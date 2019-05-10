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

// IR transmitter and receiver setup
  const byte IRLED = 11;  // Timer 2 "A" output: OC2A
  const byte IRSensor = 8; // from receiver output
  const byte onboard_LED = 13; // onboard indicator LED
  boolean BeamBroke;

void setup() {
    // Serial.begin(9600);
    pinMode (IRSensor, INPUT);
    pinMode (onboard_LED, OUTPUT);
  
  // LED matrix setup
    delay(3000);
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setBrightness(BRIGHTNESS);
    // Initialize all LEDs to black (off)
    for(uint8_t i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0,0,0);
    }
    
  // 38kHz LED setup
    pinMode (IRLED, OUTPUT);
    // set 38kHz square wave on Timer 2, from http://forum.arduino.cc/index.php/topic,102430.0.html
    TCCR2A = _BV (COM2A0) | _BV(WGM21);  // CTC, toggle OC2A on Compare Match
    TCCR2B = _BV (CS20);   // No prescaler
    OCR2A =  209;          // compare A register value (210 * clock speed (1/16MHz))
    //  = 13.125 uS , so frequency is 1 / (2 * 13.125) = 38095
}

// This function will return the right 'led index number' for 
// a given set of X and Y coordinates on your matrix.  
uint16_t XY( uint8_t row, uint8_t column) {
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

unsigned long UpdateState(boolean beambroke, uint8_t Input[3], unsigned long t) {
  uint8_t state = Input[2];
  unsigned long tnow = millis();
  
  if (beambroke == HIGH) {
    if (state == 0) {
      Change1LED(Input,R);
      state = 1; // in use
    }
    else if (state >= 2) {
      if (state >= 5) { // if beam being repeatedly broken & restored, assume in use
        state = 1;
      }
      else if ((unsigned long)(tnow - t) >= 5000) { // if beam broken after 5sec, set as in use
        state = 1;
      }
      else {
        state = state + 1; // count number of times beam is broken after first break and before wait interval is up
      }
    }
  }
  else {
    if (state == 1) { // if beam restored after being broken, enter intermediate state
      t = tnow; // save time instance
      state = 2; // intermediate
    }
    else if (state >= 2) {
      if ((unsigned long)(tnow - t) >= 5000) { // if 5sec have passed since restored without being broken, set as not in use
        Change1LED(Input,x);
        state = 0; // not in use
      }
    }
  }
  Input[2] = state;
  return t;
}

void loop() {
  // Equipment to LED location definitions
  // Use like 'uint8_t Variable[2] = {row,column,state}'
  // State: 0 = not in use, 1 = in use, 2 >= between states
  static uint8_t Bike[3] = {2,4,1};
  static unsigned long tBike = 0;
  // Serial.println(Bike[2]);
  BeamBroke = digitalRead(IRSensor);

  tBike = UpdateState(BeamBroke, Bike, tBike);

  if (BeamBroke == HIGH) { // beam interrupted
    digitalWrite(onboard_LED, LOW); // green onboard LED off    
  }
  else { // beam detected
    digitalWrite(onboard_LED, HIGH); // green LED on
  }
}
