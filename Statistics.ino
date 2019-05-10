// LED Matrix Setup
  #include <FastLED.h>
  #define LED_PIN  3 // Connect the data wire from your LED strip to this pin. Use a resistor!
  #define COLOR_ORDER GRB
  #define CHIPSET     WS2812B
  #define NUM_LEDS    150 // Edit this to reflect the number of LEDS you are using.
  #define BRIGHTNESS 32 // 0 to 255
  CRGB leds[NUM_LEDS];
  // Params for width and height, edit these to reflect your setup.
  const uint8_t NumberOfColumns = 15; // Make sure this matches the number in the 'LightEmUp', 'CustomDrawColor', and 'CustomDrawMatrix' functions.
  const uint8_t NumberOfRows = 10;

// Color variable definitions
  uint8_t R[3] = {255,0,0}; //red
  uint8_t O[3] = {255,100,0}; //orange
  uint8_t Y[3] = {255,255,0}; //yellow
  uint8_t x[3] = {0,0,0}; //off aka black

// Define gym open/close times
  // if Arduino not loses power during gym operation times data will not be saved for that day, operate off battery or backup to ensure no outages occur
  // TURN ARDUINO ON/OFF at times stated, or data may not be saved and may be inaccurate. - use a wall outlet timer
  // if 24hr gym, need to redefine start time to new power up time.
  // use 24hr clock, {hr, mins}
  // if 24hr gym set to 'GymOpen' to the intitial power on time, ignore 'GymClose'
  uint8_t GymOpen[2] = {6,0}; //6:00am
  uint8_t GymClose[2] = {22,30}; //10:30pm
  // define if Arduino is turned on/off at closing/opening (true) or is on 24/7 (false)
  boolean ArduinoPower = true;  

// Define statistic variables
  float daycount = 0;
  unsigned long savetime;

// IR transmitter and receiver setup
  const byte IRLED = 11;  // Timer 2 "A" output: OC2A
  const byte IRSensor = 8; // from receiver output
  const byte onboard_LED = 13; // onboard indicator LED
  boolean BeamBroke;

void setup() {
    Serial.begin(9600);
  // Calculate 'savetime' in milliseconds from constant hours+mins entered above
    if (ArduinoPower == true) {
      if (GymOpen[0] > GymClose[0]) {
        savetime = 60000*((60*(24-(GymOpen[0]-GymClose[0])))+abs(GymClose[1]-GymOpen[1]));
      }
      else {
        savetime = 60000*((60*(GymClose[0]-GymOpen[0]))+abs(GymClose[1]-GymOpen[1]));
      }
    }
    else {
      savetime = 60000*(1440.1-((60*GymOpen[0])+GymOpen[1])); //1440.1 to ensure larger than 1440 (max day time in mins)
    }

  
  // LED matrix setup
    delay(3000);
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setBrightness(BRIGHTNESS);
    // Initialize all LEDs to black (off)
    for(uint8_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0,0,0);
    }
    
  // 38kHz LED setup
    pinMode (onboard_LED, OUTPUT);
    pinMode (IRLED, OUTPUT);
    pinMode (IRSensor, INPUT);
    // set 38kHz square wave on Timer 2, from http://forum.arduino.cc/index.php/topic,102430.0.html
    TCCR2A = _BV (COM2A0) | _BV(WGM21);  // CTC, toggle OC2A on Compare Match
    TCCR2B = _BV (CS20);   // No prescaler
    OCR2A =  209;          // compare A register value (210 * clock speed (1/16MHz))
    //  = 13.125 uS , so frequency is 1 / (2 * 13.125) = 38095

  delay(10000);
}

// This function will return the right 'led index number' for 
// a given set of X and Y coordinates on your matrix.  
uint16_t XY(uint8_t row, uint8_t column) {
  uint16_t i;
  if(row & 0x01) {
    // Odd rows run backwards
    uint8_t reverseX = (NumberOfColumns - 1) - column;
    i = (row * NumberOfColumns) + reverseX;
  } else {
    // Even rows run forwards
    i = (row * NumberOfColumns) + column;
  }

  i = i-1; // Shifts each LED by number specified.
  i = ((2*NUM_LEDS)+i)%NUM_LEDS; // If you try to assign more LEDs than defined, this wraps them back to the front
  return i;
}

// The 'Change1LED' function will assign the specified color to the
// LED at the coordinates provided and then show it.
void Change1LED(uint8_t Equip[3], uint8_t *color) {
  // Set Color and get the LED index number using the 'XY' function.
  leds[XY(Equip[0], Equip[1])] = CRGB(*color,*(color+1),*(color+2));
  FastLED.show();
}

void UpdateState(boolean beambroke, uint8_t Input[4], unsigned long t[3]) {
  uint8_t state = Input[2];
  unsigned long tnow = millis();
  
  if (beambroke == HIGH) {
    if (state == 0) {
      Change1LED(Input,R);
      state = 1; // in use
      Input[3] = Input[3] + 1; //increment uses of equipment
      t[1] = tnow; //save start instance of time for calculating use time
    }
    else if (state >= 2) {
      if (state >= 5) { // if beam being repeatedly broken & restored, assume in use
        state = 1;
      }
      else if ((unsigned long)(tnow - t[0]) >= 5000) { // if beam broken after 5sec, set as in use
        state = 1;
      }
      else {
        state = state + 1; // count number of times beam is broken after first break and before wait interval is up
      }
    }
  }
  else {
    if (state == 1) { // if beam restored after being broken, enter intermediate state
      t[0] = tnow; // save time instance
      state = 2; // intermediate
    }
    else if (state >= 2) {
      if ((unsigned long)(tnow - t[0]) >= 5000) { // if 5sec have passed since restored without being broken, set as not in use
        Change1LED(Input,x);
        state = 0; // not in use
        //save and calculate use time based on end time and start time
      }
    }
  }
  Input[2] = state;
}

void DoSave(unsigned long& prevtime, uint8_t& start) { // add another arg - probably array of stats
  if ((unsigned long)(millis() - prevtime) >= savetime) {
    if (start == 0) {
      if (ArduinoPower == true) {
        daycount = daycount + 1; // don't count partial day for 24hr gym in stats
        // do saving of stats, doesn't save if program didn't run whole day (i.e. day not counted)
        delay(86400000-savetime); // 'pause' program until gym opens again if forgot to turn off
        start = 0;
        // if powered on not at gym opening and also forget to turn off, the stats will be messed up
      }
      else {
        start = 1;
        savetime = 86400000;
      }
    }
    else {
      daycount = daycount + 1;
      // do saving of data
    }
    prevtime = millis();
  }
}

void loop() {
  // Initialize save time variables, perform saving function
  static uint8_t start = 0;
  static unsigned long prevtime = millis();
  DoSave(prevtime, start);

  
  // Equipment to LED location definitions
  // Use like 'uint8_t Variable[2] = {row,column,state,total usage amount}'
  // State: 0 = not in use, 1 = in use, 2 >= between states
  static uint8_t Bike[4] = {2,4,1,0};
  // Use like 'unsigned long tVariable[2] = {state 2 timer, turn on time (use begins), total use time}'
  static unsigned long tBike[3] = {0,0,0};
  //Serial.println(Bike[2]);
  BeamBroke = digitalRead(IRSensor);

  UpdateState(BeamBroke, Bike, tBike);

  if (BeamBroke == HIGH) { // beam interrupted
    digitalWrite(onboard_LED, LOW); // green onboard LED off    
  }
  else { // beam detected
    digitalWrite(onboard_LED, HIGH); // green LED on
  }
}
