
// IR transmitter and receiver setup
  const byte IRLED = 11;  // Timer 2 "A" output: OC2A
  const byte IRBike = 8; // from receiver output
  const byte IRTread = 7; // from receiver output
  const byte IRLat = 4; // from receiver output
  const byte LEDBike = 13; // red indicator LED
  const byte LEDTread = 6; // green indicator LED
  const byte LEDLat = 5; // yellow indicator LED
  boolean BeamBike;
  boolean BeamTread;
  boolean BeamLat;

void setup() {
  // 38kHz LED setup
    pinMode (LEDBike, OUTPUT);
    pinMode (LEDTread, OUTPUT);
    pinMode (LEDLat, OUTPUT);
    pinMode (IRBike, INPUT);
    pinMode (IRTread, INPUT);
    pinMode (IRLat, INPUT);

    pinMode (IRLED, OUTPUT);
    // set 38kHz square wave on Timer 2, from http://forum.arduino.cc/index.php/topic,102430.0.html
    TCCR2A = _BV (COM2A0) | _BV(WGM21);  // CTC, toggle OC2A on Compare Match
    TCCR2B = _BV (CS20);   // No prescaler
    OCR2A =  209;          // compare A register value (210 * clock speed (1/16MHz))
    //  = 13.125 uS , so frequency is 1 / (2 * 13.125) = 38095
}

void loop() {
  BeamBike = digitalRead (IRBike);
  if (BeamBike == HIGH) { // beam interrupted
    digitalWrite(LEDBike, LOW); // red LED off
    delay(1000);
  }
  else { // beam detected
    digitalWrite(LEDBike, HIGH); // red LED on
  }

  BeamTread = digitalRead (IRTread);
  if (BeamTread == HIGH) { // beam interrupted
    digitalWrite(LEDTread, LOW); // green LED off
    delay(1000);
  }
  else { // beam detected
    digitalWrite(LEDTread, HIGH); // green LED on
  }

  BeamLat = digitalRead (IRLat);
  if (BeamLat == HIGH) { // beam interrupted
    digitalWrite(LEDLat, LOW); // yellow LED off
    delay(1000);
  }
  else { // beam detected
    digitalWrite(LEDLat, HIGH); // yellow LED on
  }
}
