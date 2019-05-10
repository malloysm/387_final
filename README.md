# Display Gym Equipment Usage
Final Project for ECE 387.

This project interfaces a [W2812B LED Strip Matrix](https://github.com/malloysm/387_midterm), IR Beams, and an Arduino to track Gym Machine usage and statistics.

Click the image to watch the demo in action:


### Descriptions of Files Uploaded

The following is the approximate order in which the files were created. Some files are conglomerates of multiple different code files and were edited well past their creation date.

`38kHzLED.ino`         - Drives an IR LED at 38 kHz and checks the beam received by the IR Receiver with an indicator LED.

`UpdateMatrix.ino`      - Updates the LED Matrix based on the IR beam's state. Incorporates delays and counters to determine if beam is actually broken or if it was just a mistake.

`Statistics.ino`        - Incorporates the gym's opening and closing time using 'millis()', so statistics can be saved at the end of the day. This allows the system to be shut down at the end of the gym day, or, if it is a 24hr gym, lets the user select when the data should be saved.

`eeprom_test.ino`       - Tests storing and retrieving values from the Arduino's EEPROM memory. First instance of using a function with references and pointers to do the saving, allowing multiple variables to be saved easily.

`Statistics_2.ino`      - Combination of all of the above code, starts keeping track of the variables being saved (avg uses and avg use times, per day and total).

`pseudoIR.ino`          - Same as 'Statistics_2.ino' but without IR sensing. Beams were directly manipulated with timers in order to better evaluate the accuracy of the statistic variables.

`ColorVsUse.ino`        - Code was added to change the color of a gym machine's corresponding LED based on it's average use time.

`3sensor_38kHzLED.ino`  - `38kHzLED.ino` expanded to include more IR Receivers.

`3sensors.ino`          - Combination of all the above code to create a system that checks 3 machine's usage, and updates it to a LED. matrix.

`Final.ino`             - Improved version with bug fixes of `3sensors.ino`.
