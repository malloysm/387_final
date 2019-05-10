// LED Matrix Setup
  #include <FastLED.h>
  #define LED_PIN  3 // Connect the data wire from your LED strip to this pin. Use a resistor!
  #define COLOR_ORDER GRB
  #define CHIPSET     WS2812B
  #define NUM_LEDS    150 // Edit this to reflect the number of LEDS you are using.
  #define BRIGHTNESS 16 // 0 to 255
  CRGB leds[NUM_LEDS];
  // Params for width and height, edit these to reflect your setup.
  const uint8_t NumberOfColumns = 15; 
  const uint8_t NumberOfRows = 10;

// Color variable definitions
  uint8_t R[3] = {255,0,0}; //red
  uint8_t Y[3] = {255,255,0}; //yellow
  uint8_t B[3] = {0,0,255}; //blue
  uint8_t x[3] = {0,0,0}; //off aka black

// Define gym open/close times
  // if Arduino not loses power during gym operation times data will not be saved for that day, operate off battery or backup to ensure no outages occur
  // TURN ARDUINO ON/OFF at times stated, or data may not be saved and may be inaccurate. - use a wall outlet timer
  // if 24hr gym, need to redefine start time to new power up time.
  // use 24hr clock, {hr, mins}
  // if 24hr gym set to 'GymOpen' to the intitial power on time, ignore 'GymClose'
  uint8_t GymOpen[2] = {6,2}; //6:02am, this is when the Arduino thinks it is being powered on
  uint8_t GymClose[2] = {6,3}; //6:03am
  // define if Arduino is turned on/off at closing/opening (true) or is on 24/7 (false)
  boolean ArduinoPower = true;  
  unsigned long savetime;

// Define statistic variables to save to EEPROM
  #include <EEPROM.h>
  uint16_t EEMEM daycountStore;
  float EEMEM Bike1AvgUsesPerDayStore; // 
  float EEMEM Bike1AvgUseTimePerDayStore; // in minutes
  float EEMEM Bike2AvgUsesPerDayStore; // 
  float EEMEM Bike2AvgUseTimePerDayStore; // in minutes
  float EEMEM TreadAvgUsesPerDayStore; // 
  float EEMEM TreadAvgUseTimePerDayStore; // in minutes

// IR transmitter and receiver setup
  const byte IRLED = 11;  // Timer 2 "A" output: OC2A
  const byte IRBike1 = 4; // from receiver output
  const byte IRBike2 = 7; // from receiver output
  const byte IRTread = 8; // from receiver output
  const byte LEDBike1 = 13; // red indicator LED
  const byte LEDBike2 = 12; // green indicator LED
  const byte LEDTread = 10; // yellow indicator LED
  boolean BeamBike1;
  boolean BeamBike2;
  boolean BeamTread;

void setup() {
  Serial.begin(9600);

  // Statistic variables to save setup
    // if this is the first time a variable will be used, initialize to zero
    // i.e. first time running this code on your Arduino, want to reset stored data, adding more equipment
    // if adding new equipment statistics to save, only initialize those variables to zero to keep other values intact
    // Upload and run the code on the Arduino once, then comment out the code and reupload
//     uint16_t zero = 0;
//     float fzero = 0;
//     eeprom_update_block(&zero, &daycountStore, sizeof(zero));
//     eeprom_update_block(&fzero, &Bike1AvgUsesPerDayStore, sizeof(fzero));
//     eeprom_update_block(&fzero, &Bike1AvgUseTimePerDayStore, sizeof(fzero));
//     eeprom_update_block(&fzero, &Bike2AvgUsesPerDayStore, sizeof(fzero));
//     eeprom_update_block(&fzero, &Bike2AvgUseTimePerDayStore, sizeof(fzero));
//     eeprom_update_block(&fzero, &TreadAvgUsesPerDayStore, sizeof(fzero));
//     eeprom_update_block(&fzero, &TreadAvgUseTimePerDayStore, sizeof(fzero));
    
  // Calculate 'savetime' in milliseconds from constant hours+mins entered above
    if (ArduinoPower == true) {
      if (GymOpen[0] > GymClose[0]) {
        if (GymOpen[1] >= GymClose[1]) {
          savetime = 60000*((60*(24-(GymOpen[0]-GymClose[0])))-(GymOpen[1]-GymClose[1]));
        }
        else {
          savetime = 60000*((60*(24-(GymOpen[0]-GymClose[0])))+(GymClose[1]-GymOpen[1]));
        }
      }
      else if (GymOpen[0] == GymClose[0]) {
        if (GymOpen[1] >= GymClose[1]) {
          savetime = 60000*((60*24)-(GymOpen[1]-GymClose[1]));
        }
        else {
          savetime = 60000*(GymClose[1]-GymOpen[1]);
        }
      }
      else {
        if (GymOpen[1] >= GymClose[1]) {
          savetime = 60000*((60*(GymClose[0]-GymOpen[0]))-(GymOpen[1]-GymClose[1]));
        }
        else {
          savetime = 60000*((60*(GymClose[0]-GymOpen[0]))+(GymClose[1]-GymOpen[1]));
        }
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
    
  // IR receiver and indicator LED setup
    pinMode (LEDBike1, OUTPUT);
    pinMode (LEDBike2, OUTPUT);
    pinMode (LEDTread, OUTPUT);
    pinMode (IRBike1, INPUT);
    pinMode (IRBike2, INPUT);
    pinMode (IRTread, INPUT);
    
  // 38kHz LED setup
    pinMode (IRLED, OUTPUT);
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
void Change1LED(uint8_t Equip[6], uint8_t *color) {
  // Set Color and get the LED index number using the 'XY' function.
  leds[XY(Equip[0], Equip[1])] = CRGB(*color,*(color+1),*(color+2));
  FastLED.show();
}

void UpdateState(boolean beambroke, uint8_t Input[6], unsigned long t[3], float *AvgUseTimePerDayStore) {
  uint8_t state = Input[2];
  uint8_t count = Input[5];
  unsigned long tnow = millis();
  
  if (beambroke == HIGH) {
    if (state == 0) {
      state = 1; // in use?
      t[1] = tnow; //save start instance of time for calculating use time
    }
    else if (state == 1) {
      if ((unsigned long)(tnow - t[1]) >= 5000) { // if beam still broken after 5sec, set as in use
        if (count <= 50) { // if beam being broken & restored multiple times, assume in use
          Change1LED(Input,R);
          Input[4] = 1; // color is red
          state = 2; // in use.
          Input[3] = Input[3] + 1; //increment uses of equipment
          count = 0;
        }
        else { // if beam broken & restored too frequently to be human caused, assume jitter and not in use
          state = 0;
          count = 0;
        }
      }
    }
    else if (state >= 3) {
      if ((unsigned long)(tnow - t[0]) >= 5000) { // if beam broken after 5sec
        if (state >= 7 && state <= 50) { // if beam being broken & restored multiple times, assume in use
          state = 2;
        }
        else { // if beam broken & restored too frequently to be human caused, assume jitter and not in use
          Change1LED(Input,x);
          Input[4] = 0; // off
          state = 0;
          t[2] = t[2] + ((tnow - t[1]) - 5000); //save and calculate use time based on end time and start time, -5000 to adjust for delay
        }
      }
      else {
        if (state <= 250) {
          state = state + 1; // count number of times beam is broken after first break and before wait interval is up
        }
      }
    }
  }
  else {
    if (state == 2) { // if beam restored after being broken, enter intermediate state
      t[0] = tnow; // save time instance
      state = 3; // intermediate
    }
    else if (state == 1) {
      if ((unsigned long)(tnow - t[1]) >= 5000) { // if beam restored after initial break, assume mistakenly triggered & actually not in use
        state = 0; // not in use
      }
      else {
        if (count <= 250) {
          count = count + 1; // count number of times beam is broken after first break and before wait interval is up
        }
      }
    }
    else if (state >= 3) {
      if ((unsigned long)(tnow - t[0]) >= 5000) { // if 5sec have passed since restored without being broken
        if (state >= 7 && state <= 50) { // if beam being broken & restored multiple times, assume in use
          state = 2;
        }
        else { // if beam broken & restored too frequently to be human caused, assume jitter and not in use
          Change1LED(Input,x);
          Input[4] = 0; // off
          state = 0; // not in use
          t[2] = t[2] + ((tnow - t[1]) - 5000); //save and calculate use time based on end time and start time, -5000 to adjust for delay
        }
      }
    }
  }

  Input[2] = state;
  Input[5] = count;

  //Change LED color based on state
  if (Input[4] >= 1) {
    float AvgUseTimePerDay = 0;
    eeprom_read_block(&AvgUseTimePerDay, AvgUseTimePerDayStore, sizeof(AvgUseTimePerDay));
    unsigned long AvgUsage = (unsigned long) AvgUseTimePerDay;
    unsigned long AvgUsage75 = (AvgUsage - (AvgUsage/4))*1000; //convert to ms, will change 1000 to 60000 once change avg from secs to mins
    unsigned long AvgUsage125 = (AvgUsage + (AvgUsage/4))*1000; //convert to ms
    if ((Input[4] == 1) && ((tnow - t[1]) >= AvgUsage75) && ((tnow - t[1]) <= AvgUsage125)) {
      Change1LED(Input,Y);
      Input[4] = 2;
    }
    else if ((Input[4] == 2) && ((tnow - t[1]) > AvgUsage125)) {
      Change1LED(Input,B);
      Input[4] = 0;
    }
  }
}


void DoSave(uint8_t Input[6], unsigned long t[3], float *AvgUsesPerDayStore, float *AvgUseTimePerDayStore) { // add another arg - probably array of stats
  uint16_t daycount;
  float AvgUsesPerDay = 0;
  float AvgUseTimePerDay = 0;
  float NumberUsesToday = (float)Input[3];
  float AvgUseTimeToday;
  if (Input[3] == 0) {
    AvgUseTimeToday = 0;
  }
  else {
    AvgUseTimeToday = ((float)t[2]/(float)1000)/NumberUsesToday; // in seconds for testing purposes, will change 1000 to 60000 once change avg from secs to mins
  }
  
  // retrieve stored EEPROM Data
  eeprom_read_block(&daycount, &daycountStore, sizeof(daycount));
  eeprom_read_block(&AvgUsesPerDay, AvgUsesPerDayStore, sizeof(AvgUsesPerDay));
  eeprom_read_block(&AvgUseTimePerDay, AvgUseTimePerDayStore, sizeof(AvgUseTimePerDay));
        
  // Print out daily statistics to Serial Monitor
  Serial.print("          Today: ");
  Serial.print(NumberUsesToday);
  Serial.print(", ");
  Serial.print(AvgUseTimeToday);
  Serial.println("secs");
        
  // do updating of avgs stats, doesn't save if program didn't run whole day (i.e. day not counted)
  if (Input[3] > AvgUsesPerDay) {
    AvgUsesPerDay = AvgUsesPerDay + ((NumberUsesToday - AvgUsesPerDay)/(float)daycount);
  }
  else {
    AvgUsesPerDay = AvgUsesPerDay - ((AvgUsesPerDay - NumberUsesToday)/(float)daycount);
  }
  if (AvgUseTimeToday > AvgUseTimePerDay) {
    AvgUseTimePerDay = AvgUseTimePerDay + ((AvgUseTimeToday - AvgUseTimePerDay)/(float)daycount);
  }
  else {
    AvgUseTimePerDay = AvgUseTimePerDay - ((AvgUseTimePerDay - AvgUseTimeToday)/(float)daycount);
  }

  Serial.print("          Avg: ");
  Serial.print(AvgUsesPerDay);
  Serial.print(", ");
  Serial.print(AvgUseTimePerDay);
  Serial.println("secs");
  
  // save variables
  eeprom_update_block(&AvgUsesPerDay, AvgUsesPerDayStore, sizeof(AvgUsesPerDay));
  eeprom_update_block(&AvgUseTimePerDay, AvgUseTimePerDayStore, sizeof(AvgUseTimePerDay));

  // reset daily variables
  Input[3] = 0;
  t[2] = 0;
}

// Use LEDs to show immediate IR receiver state
void ShowRcvr(boolean BeamBroke, const byte LED) {
  if (BeamBroke == HIGH) { // beam interrupted
    digitalWrite(LED, LOW); // LED off    
  }
  else { // beam detected
    digitalWrite(LED, HIGH); // LED on
  }
}

void loop() {
  // Initialize save time variables
  static uint8_t start = 0;
  static unsigned long prevtime = millis();
  uint16_t daycount;

  
  // Equipment to LED location definitions
  // Use like 'uint8_t Variable[2] = {row,column,state,total usage amount,color,count}'
  // Color 0 = off, 1 = Red, 2 = Yellow, 3 = Blue
  // State: 0 = not in use, 1 = between states, 2 = in use, 3 >= between states
  static uint8_t Bike1[6] = {2,4,0,0,0,0};
  static uint8_t Bike2[6] = {2,5,0,0,0,0};
  static uint8_t Tread[6] = {3,4,0,0,0,0};
  // Use like 'unsigned long tVariable[2] = {turn off delay, turn on time (use begins), total use time}'
  static unsigned long tBike1[3] = {0,0,0};
  static unsigned long tBike2[3] = {0,0,0};
  static unsigned long tTread[3] = {0,0,0};
  
  BeamBike1 = digitalRead(IRBike1);
  BeamBike2 = digitalRead(IRBike2);
  BeamTread = digitalRead(IRTread);

  UpdateState(BeamBike1, Bike1, tBike1, &Bike1AvgUseTimePerDayStore);
  UpdateState(BeamBike2, Bike2, tBike2, &Bike2AvgUseTimePerDayStore);
  UpdateState(BeamTread, Tread, tTread, &TreadAvgUseTimePerDayStore);

  ShowRcvr(BeamBike1, LEDBike1);
  ShowRcvr(BeamBike2, LEDBike2);
  ShowRcvr(BeamTread, LEDTread);

//  if (BeamBike1 == HIGH) { // beam interrupted
//    digitalWrite(LEDBike1, LOW); // LED off    
//  }
//  else { // beam detected
//    digitalWrite(LEDBike1, HIGH); // LED on
//  }


  // Perform Saving
  if ((unsigned long)(millis() - prevtime) >= savetime) {
    if (ArduinoPower == true || start == 1) {
      eeprom_read_block(&daycount, &daycountStore, sizeof(daycount));
      daycount = daycount + 1;
      eeprom_update_block(&daycount, &daycountStore, sizeof(daycount)); // Update the day count
      Serial.print("Day: ");
      Serial.println(daycount);
      Serial.println();
      
      // Put Variables to save for equipment here:
      Serial.println("Bike1");
      DoSave(Bike1, tBike1, &Bike1AvgUsesPerDayStore, &Bike1AvgUseTimePerDayStore);
      Serial.println();

      Serial.println("Bike2");
      DoSave(Bike2, tBike2, &Bike2AvgUsesPerDayStore, &Bike2AvgUseTimePerDayStore);
      Serial.println();

      Serial.println("Tread");
      DoSave(Tread, tTread, &TreadAvgUsesPerDayStore, &TreadAvgUseTimePerDayStore);
      Serial.println();


      
      if (ArduinoPower == true) {
        Serial.println("Paused");
        //delay(86400000-savetime); // 'pause' program until gym opens again if forgot to turn off
        delay(30000); // used for testing purposes
        Serial.println("Back in Action");
        Serial.println();
        // if powered on not at gym opening and also forget to turn off, the stats will be messed up
      }
    }
    else {
      start = 1;
      savetime = 86400000;

      // reset daily variables
      Bike1[3] = 0;
      tBike1 [2] = 0;
      Bike2[3] = 0;
      tBike2[2] = 0;
      Tread[3] = 0;
      tTread[2] = 0;
    }
    prevtime = millis();
  }
}
