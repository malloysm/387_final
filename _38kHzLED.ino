
// IR transmitter and receiver setup
  const byte IRLED = 11;  // Timer 2 "A" output: OC2A
  const byte IRSensor = 8; // from receiver output
  const byte onboard_LED = 13; // onboard indicator LED
  boolean BeamBroke;

void setup() {
  // 38kHz LED setup
    pinMode (onboard_LED, OUTPUT);
    pinMode (IRLED, OUTPUT);
    pinMode (IRSensor, INPUT);
    // set 38kHz square wave on Timer 2, from http://forum.arduino.cc/index.php/topic,102430.0.html
    TCCR2A = _BV (COM2A0) | _BV(WGM21);  // CTC, toggle OC2A on Compare Match
    TCCR2B = _BV (CS20);   // No prescaler
    OCR2A =  209;          // compare A register value (210 * clock speed (1/16MHz))
    //  = 13.125 uS , so frequency is 1 / (2 * 13.125) = 38095
}

void loop() {
//  BeamBroke = digitalRead (IRSensor);
//  if (BeamBroke == HIGH) { // beam interrupted
//    digitalWrite(onboard_LED, LOW); // green onboard LED off
//    delay(1000);
//  }
//  else { // beam detected
//    digitalWrite(onboard_LED, HIGH); // green LED on
//  }
}
