# Display Gym Equipment Usage
Final Project for ECE 387.

This project interfaces a [W2812B LED Strip Matrix](https://github.com/malloysm/387_midterm), IR Beams, and an Arduino to track Gym Machine usage and statistics.

Click the image to watch the demo in action:


### Descriptions of Files Uploaded

The following is the approximate order in which the files were created. Some files are conglomerates of multiple different code files and were edited well past their creation date.

*_38kHzLED.ino*         - Drives an IR LED at 38 kHz and checks the beam received by the IR Receiver with an indicator LED.

*UpdateMatrix.ino*      - Updates the LED Matrix based on the IR beam's state. Incorporates delays and counters to determine if beam is actually broken or if it was just a mistake.

*Statistics.ino*        - Incorporates the gym's opening and closing time using 'millis()', so statistics can be saved at the end of the day. This allows the system to be shut down at the end of the gym day, or, if it is a 24hr gym, lets the user select when the data should be saved.

*eeprom_test.ino*       - Tests storing and retrieving values from the Arduino's EEPROM memory. First instance of using a function with references and pointers to do the saving, allowing multiple variables to be saved easily.

*Statistics_2.ino*      - Combination of all of the above code, starts keeping track of the variables being saved (avg uses and avg use times, per day and total)

*pseudoIR.ino*          -

*ColorVsUse.ino*        -

*_3sensor_38kHzLED.ino* -

*_3sensors.ino*         -
