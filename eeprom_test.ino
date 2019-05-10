// http://www.avrfreaks.net/forum/how-readwrite-float-eeprom

#include <EEPROM.h>
// EEPROM variables
uint16_t  EEMEM myvalStore;
uint16_t  EEMEM myval2Store;

uint8_t a = 0;

void setup() {
  // Writing & Reading EEPROM:
  Serial.begin(9600);
  uint16_t f1 = 960;
  uint16_t f2 = 25;

  // if this is the first time a variable will be used, initialize to zero
  // i.e. first time running this code on your Arduino, want to reset stored data, adding more equipment
  // if adding new equipment statistics to save, only initialize those variables to zero to keep other values intact
  // Upload and run the code on the Arduino once, then comment out the code and reupload
//  uint16_t zero = 0;
  eeprom_update_block(&f1, &myvalStore, sizeof(f1));
  eeprom_update_block(&f2, &myval2Store, sizeof(f2));

  delay(10000);
}

void DoSave(uint16_t *p) {
  uint16_t doval = 0;
  eeprom_read_block(&doval, p, sizeof(doval));
  Serial.print("Before: ");
  Serial.println(doval); // prints myvalStore value
  doval = doval + 5;
  eeprom_update_block(&doval, p, sizeof(doval)); // writes new doval to myvalStore
  eeprom_read_block(&doval, p, sizeof(doval)); //should read myvalStore (should be 5 greater than previous value printed)
    Serial.print("After: ");
    Serial.println(doval);
    Serial.println(" ");
}

void loop() {
  static uint16_t myval = 0;
  static uint16_t myval2 = 0;
  
  if (a < 3) {
    Serial.println("myval");
    DoSave(&myvalStore);
//    eeprom_read_block(&myval, &myvalStore, sizeof(myval)); //should read myvalStore (should be 5 greater than previous value printed)
//    Serial.print("myval After: ");
//    Serial.println(myval);
//    Serial.println(" ");
    delay(5000);
  }
  if (a >= 1 && a < 4) {
    Serial.println("myval2");
    DoSave(&myval2Store);
//    eeprom_read_block(&myval2, &myval2Store, sizeof(myval2)); //should read myvalStore (should be 5 greater than previous value printed)
//    Serial.print("myval2 After: ");
//    Serial.println(myval2);
//    Serial.println(" ");
    delay(5000);
  }
  if (a >= 4) {
    a = 5;
  }
  a = a + 1;
}
