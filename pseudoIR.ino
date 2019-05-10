// Define gym open/close times
  // if Arduino not loses power during gym operation times data will not be saved for that day, operate off battery or backup to ensure no outages occur
  // TURN ARDUINO ON/OFF at times stated, or data may not be saved and may be inaccurate. - use a wall outlet timer
  // if 24hr gym, need to redefine start time to new power up time.
  // use 24hr clock, {hr, mins}
  // if 24hr gym set to 'GymOpen' to the intitial power on time, ignore 'GymClose'
  uint8_t GymOpen[2] = {6,0}; //6:00am, this is when the Arduino thinks it is being powered on
  uint8_t GymClose[2] = {6,2}; //10:30pm
  // define if Arduino is turned on/off at closing/opening (true) or is on 24/7 (false)
  boolean ArduinoPower = true;  
  unsigned long savetime;

// Define statistic variables to save to EEPROM
  #include <EEPROM.h>
  uint16_t EEMEM daycountStore;
  float EEMEM BikeAvgUsesPerDayStore; // rounded to whole number
  float EEMEM BikeAvgUseTimePerDayStore; // in minutes

// IR transmitter and receiver setup
  const byte onboard_LED = 13; // onboard indicator LED
  boolean BeamBroke;

void setup() {
  Serial.begin(9600);

  // Statistic variables to save setup
    // if this is the first time a variable will be used, initialize to zero
    // i.e. first time running this code on your Arduino, want to reset stored data, adding more equipment
    // if adding new equipment statistics to save, only initialize those variables to zero to keep other values intact
    // Upload and run the code on the Arduino once, then comment out the code and reupload
     uint16_t zero = 0;
     float fzero = 0;
     eeprom_update_block(&zero, &daycountStore, sizeof(zero));
     eeprom_update_block(&fzero, &BikeAvgUsesPerDayStore, sizeof(fzero));
     eeprom_update_block(&fzero, &BikeAvgUseTimePerDayStore, sizeof(fzero));
    
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

    
  // 38kHz LED setup
    pinMode (onboard_LED, OUTPUT);
}


void UpdateState(boolean beambroke, uint8_t Input[4], unsigned long t[3]) {
  uint8_t state = Input[2];
  unsigned long tnow = millis();
  
  if (beambroke == HIGH) {
    if (state == 0) {
      state = 1; // in use?
      t[1] = tnow; //save start instance of time for calculating use time
    }
    else if (state == 1) {
      if ((unsigned long)(tnow - t[1]) >= 10000) { // if beam still broken after 10sec, set as in use
        state = 2; // in use.
        Input[3] = Input[3] + 1; //increment uses of equipment
      }
    }
    else if (state >= 3) {
      if (state >= 7) { // if beam being repeatedly broken & restored, assume in use
        state = 2;
      }
      else if ((unsigned long)(tnow - t[0]) >= 5000) { // if beam broken after 10sec, set as in use
        state = 2;
      }
      else {
        state = state + 1; // count number of times beam is broken after first break and before wait interval is up
      }
    }
  }
  else {
    if (state == 2) { // if beam restored after being broken, enter intermediate state
      t[0] = tnow; // save time instance
      state = 3; // intermediate
    }
    else if (state == 1) {
      if ((unsigned long)(tnow - t[1]) >= 10000) { // if beam restored after initial break, assume mistakenly triggered & actually not in use
        state = 0; // not in use
      }
    }
    else if (state >= 3) {
      if ((unsigned long)(tnow - t[0]) >= 5000) { // if 10sec have passed since restored without being broken, set as not in use
        state = 0; // not in use
        t[2] = t[2] + (tnow - t[1]) - 5000; //save and calculate use time based on end time and start time, -5000 to adjust for delay
      }
    }
  }
  Input[2] = state;
}


void DoSave(uint8_t Input[4], unsigned long t[3], float *AvgUsesPerDayStore, float *AvgUseTimePerDayStore) { // add another arg - probably array of stats
  uint16_t daycount;
  float AvgUsesPerDay = 0;
  float AvgUseTimePerDay = 0;
  float NumberUsesToday = (float)Input[3];
  float AvgUseTimeToday = ((float)t[2]/(float)1000)/NumberUsesToday; // in seconds for testing purposes
  
  // retrieve stored EEPROM Data
  eeprom_read_block(&daycount, &daycountStore, sizeof(daycount));
  eeprom_read_block(&AvgUsesPerDay, AvgUsesPerDayStore, sizeof(AvgUsesPerDay));
  eeprom_read_block(&AvgUseTimePerDay, AvgUseTimePerDayStore, sizeof(AvgUseTimePerDay));

  Serial.print("          Avg Before: ");
  Serial.print(AvgUsesPerDay);
  Serial.print(", ");
  Serial.print(AvgUseTimePerDay);
  Serial.println("secs");
        
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

  Serial.print("          Avg After: ");
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

void loop() {
  // Initialize save time variables
  static uint8_t start = 0;
  static unsigned long prevtime = millis();
  uint16_t daycount;

  
  // Equipment to LED location definitions
  // Use like 'uint8_t Variable[2] = {row,column,state,total usage amount}'
  // State: 0 = not in use, 1 = between states, 2 = in use, 3 >= between states
  static uint8_t Bike[4] = {2,4,0,0};
  // Use like 'unsigned long tVariable[2] = {turn off delay, turn on time (use begins), total use time}'
  static unsigned long tBike[3] = {0,0,0};
  //Serial.println(Bike[2]);
  if (((millis() >= 10000) && (millis() <= 40000)) || ((millis() >= 50000) && (millis() <= 70000)) || ((millis() >= 85000) && (millis() <= 105000))) {
    BeamBroke = HIGH;
  }
  else if (((millis() >= 160000) && (millis() <= 220000)) || ((millis() >= 228000) && (millis() <= 255000))){
    BeamBroke = HIGH;
  }
  else if (((millis() >= 310000) && (millis() <= 350000)) || ((millis() >= 360000) && (millis() <= 374000))){
    BeamBroke = HIGH;
  }
  else {
    BeamBroke = LOW;
  }

  UpdateState(BeamBroke, Bike, tBike);

  // LED to show immediate IR sensor state
  if (BeamBroke == HIGH) { // beam interrupted
    digitalWrite(onboard_LED, LOW); // green onboard LED off    
  }
  else { // beam detected
    digitalWrite(onboard_LED, HIGH); // green LED on
  }

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
      Serial.println("Bike");
      DoSave(Bike, tBike, &BikeAvgUsesPerDayStore, &BikeAvgUseTimePerDayStore);
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
      Bike[3] = 0;
      tBike [2] = 0;
    }
    prevtime = millis();
  }
}
