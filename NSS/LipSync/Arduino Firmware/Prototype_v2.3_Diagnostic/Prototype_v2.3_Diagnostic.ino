/*
----------------------------------------------------------
=    //        //            ///               ///       =
=    ////      //         ///  ///         ///    ///    =
=    // //     //         ///              ///           =
=    //  //    //          ///              ///          =
=    //   //   //            ///              ///        =
=    //    //  //              ///              ///      =
=    //     // //               ///              ///     =
=    //      ////        ///   ///         ///   ///     =
=    //       ///           ///               ///        =
----------------------------------------------------------
*/

//AUTHOR: Ivan Gourlay 30 August 2016

/*
 * Revision History:
 * Sept 08 2016
 * Sept 10 2016
 */

#include <EEPROM.h>

#define MODE_SELECT 12                            // LipSync Mode Select - USB or Bluetooth Communications - digital input pin 12
#define PUSH_BUTTON_UP 8                          // push button 1: UP - digital input pin 8
#define PUSH_BUTTON_DOWN 7                        // push button 2: DOWN - digital input pin 7
#define LED_1 5                                   // LipSync LED Color1 - digital output pin 5
#define LED_2 4                                   // LipSync LED Color2 - digital outputpin 4

#define PRESSURE_CURSOR A5                        // pressure sensor pin - analog input pin A5
#define PIO4 A4                                   // PIO4 pin - digital output pin A4
#define TRANS_CONTROL A3                          // transistor pin - digital output pin A3
#define Y_DIRECTION A2                            // joystick x-direction pin - analog input pin A2
#define X_DIRECTION A1                            // joystick y-direction pin - analog input pin A1

int x, y;                                         // the joystick x and y values
int x_center, y_center;                           // the relative x,y resting center of the joystick
int x_box_right, x_box_left, y_box_up, y_box_down;// boundaries for which x,y values must exceed to produce cursor movement
int box_delta;                                    // the delta value for the boundary range in all 4 directions about the x,y center
int cursor_delta;                                 // amount cursor moves in some single or combined direction
int speed_counter = 2;                           // cursor speed counter
int cursor_click_status = 0;                      // value indicator for click status, ie. tap, back and drag
int comm_mode = 0;                                // 0 == USB Communications or 1 == Bluetooth Communications
int config_done = 0;                              // Binary check of completed Bluetooth configuration
unsigned long puff_count, sip_count;              // long puff and long sip incremental counter

float nominal_cursor_value, sip_threshold, puff_threshold, cursor_back;

int delta_values[7] = {35, 30, 25, 20, 15, 10, 5};

int single = 0;
int puff1, puff2;

//***DIAGNOSTIC VARIABLES***//

int test_number = 1;
int flag1 = 0, flag2 = 0;

float pressure_value;
int joystick_value;
int cursor_value;
int speed_value;

int i = 3;


//***MICROCONTROLLER AND PERIPHERAL MODULES CONFIGURATIONS***//

void setup() {

  pinMode(LED_1, OUTPUT);                         // visual feedback #1
  pinMode(LED_2, OUTPUT);                         // visual feedback #2
  pinMode(TRANS_CONTROL, OUTPUT);                 // transistor pin output
  pinMode(PIO4, OUTPUT);                          // command mode pin output

  pinMode(PRESSURE_CURSOR, INPUT);                // pressure sensor pin input
  pinMode(X_DIRECTION, INPUT);                    // joystick x pin input
  pinMode(Y_DIRECTION, INPUT);                    // joystick y pin input

  pinMode(MODE_SELECT, INPUT_PULLUP);             // LOW: USB (default with jumper in) HIGH: Bluetooth (jumper removed)
  pinMode(PUSH_BUTTON_UP, INPUT_PULLUP);          // increase cursor speed button
  pinMode(PUSH_BUTTON_DOWN, INPUT_PULLUP);        // decrease cursor speed button

  Serial.begin(115200);                           // setting baud rate for serial coms for diagnostic data return from Bluetooth and microcontroller ***MAY REMOVE LATER***
  Serial1.begin(115200);                          // setting baud rate for Bluetooth module

  delay(1000);

  Joystick_Initialization();                        // home joystick and generate movement threshold boundaries
  delay(100);
  Pressure_Sensor_Initialization();                 // register nominal air pressure and generate activation threshold boundaries
  delay(100);
  Communication_Mode_Status();                      // identify the selected communication mode
  delay(100);
  Mouse_Configure();                                // conditionally activate the HID mouse functions
  delay(100);
  BT_Configure();                                   // conditionally configure the Bluetooth module [WHAT IF A NEW BT MODULE IS INSTALLED?]
  delay(100);

  EEPROM.put(2,i);        // assign cursor value of 3 for testing
  
}

void loop() {
  // INTERACTIVE TEST SUITE
  /*
  while (!serial.available()) {
    // wait for user response
  }

  if (Serial.available() > 0) {
    char c = toUpperCase(Serial.read());
    if ( c == 'Y') {
      //proceed with some function
    } else if ( c == 'N') {
      //proceed with some function
    } else {
      Serial.println("Invalid input, please re-enter response (Y/N)...");
    }
  }
  */

  Serial.println("---");
  Serial.println("Commencing LipSync Prototype Diagnostic Program...");
  Serial.println("---");

  Serial.println("");
  Serial.println("");

  switch (test_number) {
    case 1: {
        // test 1: positive pressure test
        flag1 = 0;
        flag2 = 0;

        Serial.println("");
        Serial.println("");

        Serial.println("Pressure sensor diagnostic. Follow on-screen prompts...");
        delay(2000);
        Serial.println("When the LED turns from GREEN to RED exhale into the mouthpiece until the LED turns GREEN again...");
        blink(1, 1500, 1);
        digitalWrite(LED_2, HIGH);
        delay(500);
        for ( int i = 0; i < 10; i++) {
          pressure_value = (((float)analogRead(PRESSURE_CURSOR)) / 1023.0) * 5.0;
          delay(100);
        }
        digitalWrite(LED_2, LOW);
        blink(1, 1000, 1);
        Serial.println("Stop exhaling...");
        Serial.println(" --- ");
        if (pressure_value < puff_threshold) {
          Serial.println("A postive pressure was registered by the pressure sensor module. Test : PASS");
          Serial.println(" --- ");
          switch (2);
        } else {
          Serial.println("A positive pressure WAS NOT registered by the pressure sensor module. Test : FAIL");

          while (flag1 == 0) {
            Serial.println("Did you exhale into the mouthpiece? (Y/N)...");
            while (!Serial.available()) {
              // wait for user response
            }
            if (Serial.available() > 0) {
              char c = toUpperCase(Serial.read());
              if ( c == 'Y') {
                Serial.println("There may be a problem with the your pressure sensor module:");
                Serial.print("Positive pressure threshold: ");
                Serial.println(puff_threshold);
                Serial.print("Pressure value: ");
                Serial.println(pressure_value);
                Serial.println("Solutions: check that the tubing is making a tight seals with the pressure sensor and mouthpiece.");
                Serial.println("If all seals are tight there may be a problem with your pressure sensor module. Check all solder joints.");
                Serial.println("If all solder joints are making good contact then consider replacing your pressure sensor.");
                delay(3500);
                Serial.println("To continue pressure sensor diagnostic, press any key");

                while (!Serial.available()) {
                  // wait for user response
                }
                switch (4);
              } else if ( c == 'N') {
                while (flag2 == 0) {
                  Serial.println("Would you like to repeat the positive pressure test?");
                  while (!Serial.available()) {
                    // wait for user response
                  }
                  if (Serial.available() > 0) {
                    char c = toUpperCase(Serial.read());
                    if ( c == 'Y') {
                      switch (1);
                    } else if ( c == 'N') {
                      Serial.println("Continuing pressure sensor diagnostic...");
                      switch (2);
                    } else {
                      Serial.println("Invalid input, please re-enter response (Y/N)...");
                      flag2 = 0;
                    }
                  }
                }
              } else {
                Serial.println("Invalid input, please re-enter response (Y/N)...");
                flag1 = 0;
              }
            }
          }
        }
      }
    case 2: {
        // test 2 : negative pressure test
        flag1 = 0;
        flag2 = 0;

        Serial.println("");
        Serial.println("");

        Serial.println("Continuing pressure sensor diagnostic. Follow on-screen prompts...");
        delay(2000);
        Serial.println("When the LED turns from GREEN to RED inhale from the mouthpiece until the LED turns GREEN again...");
        blink(1, 1500, 1);
        digitalWrite(LED_2, HIGH);
        delay(500);
        for ( int i = 0; i < 10; i++) {
          pressure_value = (((float)analogRead(PRESSURE_CURSOR)) / 1023.0) * 5.0;
          delay(100);
        }
        digitalWrite(LED_2, LOW);
        blink(1, 1000, 1);
        Serial.println("Stop exhaling...");
        Serial.println(" --- ");
        if (pressure_value > sip_threshold) {
          Serial.println("A negative pressure was registered by the pressure sensor module. Test : PASS");
          Serial.println(" --- ");
          switch (3);
        } else {
          Serial.println("A negative pressure WAS NOT registered by the pressure sensor module. Test : FAIL");

          while (flag1 == 0) {
            Serial.println("Did you inhale from the mouthpiece? (Y/N)...");

            while (!Serial.available()) {
              // wait for user response
            }
            if (Serial.available() > 0) {
              char c = toUpperCase(Serial.read());
              if ( c == 'Y') {
                Serial.println("There may be a problem with the your pressure sensor module:");
                Serial.print("Negative pressure threshold: ");
                Serial.println(sip_threshold);
                Serial.print("Pressure value: ");
                Serial.println(pressure_value);
                Serial.println("Solutions: check that the tubing is making a tight seals with the pressure sensor and mouthpiece.");
                Serial.println("If all seals are tight there may be a problem with your pressure sensor module. Check all solder joints.");
                Serial.println("If all solder joints are making good contact then consider replacing your pressure sensor.");
                delay(3500);
                Serial.println("To continue diagnostic, press any key");

                while (!Serial.available()) {
                  // wait for user response
                }
                switch (3);
              } else if ( c == 'N') {
                while (flag2 == 0) {
                  Serial.println("Would you like to repeat the negative pressure test?");
                  while (!Serial.available()) {
                    // wait for user response
                  }
                  if (Serial.available() > 0) {
                    char c = toUpperCase(Serial.read());
                    if ( c == 'Y') {
                      switch (2);
                    } else if ( c == 'N') {
                      Serial.println("Continuing diagnostic...");
                      switch (3);
                    } else {
                      Serial.println("Invalid input, please re-enter response (Y/N)...");
                      flag2 = 0;
                    }
                  }
                }
              } else {
                Serial.println("Invalid input, please re-enter response (Y/N)...");
                flag1 = 0;
              }
            }
          }
        }
      }
    case 3: {
        // test 3 : high vertical joystick test
        flag1 = 0;
        flag2 = 0;

        Serial.println("");
        Serial.println("");

        Serial.println("Joystick diagnostic. Follow on-screen prompts...");
        delay(2000);
        Serial.println("When the LED turns from GREEN to RED use your hand and move the joystick up, in the positive vertical direction, until the LED turns GREEN again...");
        blink(1, 1500, 1);
        digitalWrite(LED_2, HIGH);
        delay(500);
        for ( int i = 0; i < 10; i++) {
          joystick_value = analogRead(Y_DIRECTION);
          delay(100);
        }
        digitalWrite(LED_2, LOW);
        blink(1, 1000, 1);
        Serial.println("Release joystick...");
        Serial.println(" --- ");
        if (joystick_value > y_center) {
          Serial.println("A VALID joystick value was registered by the microcontroller. Test : PASS");
          Serial.println(" --- ");
          switch (4);
        } else {
          Serial.println("An INVALID joystick value was registered by the microcontroller. Test : FAIL");

          while (flag1 == 0) {
            Serial.println("Did you move the joystick up in the positive vertical direction? (Y/N)...");
            while (!Serial.available()) {
              // wait for user response
            }
            if (Serial.available() > 0) {
              char c = toUpperCase(Serial.read());
              if ( c == 'Y') {
                Serial.println("There may be a problem with the your joystick:");
                Serial.print("Vertical y-center value: ");
                Serial.println(y_center);
                Serial.print("Joystick value: ");
                Serial.println(joystick_value);
                Serial.println("Solutions: check that the soldered joints on the headers are making good electrical contact with the joystick board");
                Serial.println("If all solder joints are making good contact then consider replacing your joystick.");
                delay(3500);
                Serial.println("To continue joystick diagnostic, press any key");

                while (!Serial.available()) {
                  // wait for user response
                }
                switch (4);
              } else if ( c == 'N') {
                while (flag2 == 0) {
                  Serial.println("Would you like to repeat the up, positive vertical joystick test?");
                  while (!Serial.available()) {
                    // wait for user response
                  }
                  if (Serial.available() > 0) {
                    char c = toUpperCase(Serial.read());
                    if ( c == 'Y') {
                      switch (3);
                    } else if ( c == 'N') {
                      Serial.println("Continuing joystick diagnostic...");
                      switch (4);
                    } else {
                      Serial.println("Invalid input, please re-enter response (Y/N)...");
                      flag2 = 0;
                    }
                  }
                }
              } else {
                Serial.println("Invalid input, please re-enter response (Y/N)...");
                flag1 = 0;
              }
            }
          }
        }
      }
    case 4: {
        // test 4 : low vertical joystick test
        flag1 = 0;
        flag2 = 0;

        Serial.println("");
        Serial.println("");

        Serial.println("Continuing joystick diagnostic. Follow on-screen prompts...");
        delay(2000);
        Serial.println("When the LED turns from GREEN to RED use your hand and move the joystick down, in the negative vertical direction, until the LED turns GREEN again...");
        blink(1, 1500, 1);
        digitalWrite(LED_2, HIGH);
        delay(500);
        for ( int i = 0; i < 10; i++) {
          joystick_value = analogRead(Y_DIRECTION);
          delay(100);
        }
        digitalWrite(LED_2, LOW);
        blink(1, 1000, 1);
        Serial.println("Release joystick...");
        Serial.println(" --- ");
        if (joystick_value < y_center) {
          Serial.println("A VALID joystick value was registered by the microcontroller. Test : PASS");
          Serial.println(" --- ");
          switch (5);
        } else {
          Serial.println("An INVALID joystick value was registered by the microcontroller. Test : FAIL");

          while (flag1 == 0) {
            Serial.println("Did you move the joystick down in the negative vertical direction? (Y/N)...");
            while (!Serial.available()) {
              // wait for user response
            }
            if (Serial.available() > 0) {
              char c = toUpperCase(Serial.read());
              if ( c == 'Y') {
                Serial.println("There may be a problem with the your joystick:");
                Serial.print("Vertical y-center value: ");
                Serial.println(y_center);
                Serial.print("Joystick value: ");
                Serial.println(joystick_value);
                Serial.println("Solutions: (from left to right) check the electrical continuinty of the solder joint on header 3 on the main interface board and ");
                Serial.println("the solder joint on header 4 on the joystick board. If all solder joints are making good contact then consider replacing your joystick.");
                delay(3500);
                Serial.println("To continue joystick diagnostic, press any key");

                while (!Serial.available()) {
                  // wait for user response
                }
                switch (5);
              } else if ( c == 'N') {
                while (flag2 == 0) {
                  Serial.println("Would you like to repeat the down, negative vertical joystick test?");
                  while (!Serial.available()) {
                    // wait for user response
                  }
                  if (Serial.available() > 0) {
                    char c = toUpperCase(Serial.read());
                    if ( c == 'Y') {
                      switch (4);
                    } else if ( c == 'N') {
                      Serial.println("Continuing joystick diagnostic...");
                      switch (5);
                    } else {
                      Serial.println("Invalid input, please re-enter response (Y/N)...");
                      flag2 = 0;
                    }
                  }
                }
              } else {
                Serial.println("Invalid input, please re-enter response (Y/N)...");
                flag1 = 0;
              }
            }
          }
        }
      }
    case 5: {
        // test 5 : high horizontal joystick test
        flag1 = 0;
        flag2 = 0;

        Serial.println("");
        Serial.println("");

        Serial.println("Continuing joystick diagnostic. Follow on-screen prompts...");
        delay(2000);
        Serial.println("When the LED turns from GREEN to RED use your hand and move the joystick right, in the positive horizontal direction, until the LED turns GREEN again...");
        blink(1, 1500, 1);
        digitalWrite(LED_2, HIGH);
        delay(500);
        for ( int i = 0; i < 10; i++) {
          joystick_value = analogRead(X_DIRECTION);
          delay(100);
        }
        digitalWrite(LED_2, LOW);
        blink(1, 1000, 1);
        Serial.println("Release joystick...");
        Serial.println(" --- ");
        if (joystick_value > x_center) {
          Serial.println("A VALID joystick value was registered by the microcontroller. Test : PASS");
          Serial.println(" --- ");
          switch (6);
        } else {
          Serial.println("An INVALID joystick value was registered by the microcontroller. Test : FAIL");

          while (flag1 == 0) {
            Serial.println("Did you move the joystick right in the positive horizontal direction? (Y/N)...");
            while (!Serial.available()) {
              // wait for user response
            }
            if (Serial.available() > 0) {
              char c = toUpperCase(Serial.read());
              if ( c == 'Y') {
                Serial.println("There may be a problem with the your joystick:");
                Serial.print("Horizontal x-center value: ");
                Serial.println(x_center);
                Serial.print("Joystick value: ");
                Serial.println(joystick_value);
                Serial.println("Solutions: (from left to right) check the electrical continuinty of the solder joint on header 4 on the main interface board and ");
                Serial.println("the solder joint on header 3 on the joystick board. If all solder joints are making good contact then consider replacing your joystick.");
                delay(3500);
                Serial.println("To continue joystick diagnostic, press any key");

                while (!Serial.available()) {
                  // wait for user response
                }
                switch (6);
              } else if ( c == 'N') {
                while (flag2 == 0) {
                  Serial.println("Would you like to repeat the right, positive horizontal joystick test?");
                  while (!Serial.available()) {
                    // wait for user response
                  }
                  if (Serial.available() > 0) {
                    char c = toUpperCase(Serial.read());
                    if ( c == 'Y') {
                      switch (5);
                    } else if ( c == 'N') {
                      Serial.println("Continuing joystick diagnostic...");
                      switch (6);
                    } else {
                      Serial.println("Invalid input, please re-enter response (Y/N)...");
                      flag2 = 0;
                    }
                  }
                }
              } else {
                Serial.println("Invalid input, please re-enter response (Y/N)...");
                flag1 = 0;
              }
            }
          }
        }
      }
    case 6: {
        // test 6 : low horizontal joystick test
        flag1 = 0;
        flag2 = 0;

        Serial.println("");
        Serial.println("");

        Serial.println("Continuing joystick diagnostic. Follow on-screen prompts...");
        delay(2000);
        Serial.println("When the LED turns from GREEN to RED use your hand and move the joystick left, in the negative horizontal direction, until the LED turns GREEN again...");
        blink(1, 1500, 1);
        digitalWrite(LED_2, HIGH);
        delay(500);
        for ( int i = 0; i < 10; i++) {
          joystick_value = analogRead(X_DIRECTION);
          delay(100);
        }
        digitalWrite(LED_2, LOW);
        blink(1, 1000, 1);
        Serial.println("Release joystick...");
        Serial.println(" --- ");
        if (joystick_value < x_center) {
          Serial.println("A VALID joystick value was registered by the microcontroller. Test : PASS");
          Serial.println(" --- ");
          switch (7);
        } else {
          Serial.println("An INVALID joystick value was registered by the microcontroller. Test : FAIL");

          while (flag1 == 0) {
            Serial.println("Did you move the joystick right in the positive horizontal direction? (Y/N)...");
            while (!Serial.available()) {
              // wait for user response
            }
            if (Serial.available() > 0) {
              char c = toUpperCase(Serial.read());
              if ( c == 'Y') {
                Serial.println("There may be a problem with the your joystick:");
                Serial.print("Horizontal x-center value: ");
                Serial.println(x_center);
                Serial.print("Joystick value: ");
                Serial.println(joystick_value);
                Serial.println("Solutions: (from left to right) check the electrical continuinty of the solder joint on header 4 on the main interface board and ");
                Serial.println("the solder joint on header 3 on the joystick board. If all solder joints are making good contact then consider replacing your joystick.");
                delay(3500);
                Serial.println("To continue joystick diagnostic, press any key");

                while (!Serial.available()) {
                  // wait for user response
                }
                switch (7);
              } else if ( c == 'N') {
                while (flag2 == 0) {
                  Serial.println("Would you like to repeat the right, positive horizontal joystick test?");
                  while (!Serial.available()) {
                    // wait for user response
                  }
                  if (Serial.available() > 0) {
                    char c = toUpperCase(Serial.read());
                    if ( c == 'Y') {
                      switch (6);
                    } else if ( c == 'N') {
                      Serial.println("Continuing diagnostic...");
                      switch (7);
                    } else {
                      Serial.println("Invalid input, please re-enter response (Y/N)...");
                      flag2 = 0;
                    }
                  }
                }
              } else {
                Serial.println("Invalid input, please re-enter response (Y/N)...");
                flag1 = 0;
              }
            }
          }
        }
      }
    case 7: {
        // test 7 : cursor increase speed control button test
        flag1 = 0;
        flag2 = 0;

        Serial.println("");
        Serial.println("");

        Serial.println("Cursor speed control diagnostic. Follow on-screen prompts...");
        delay(2000);
        EEPROM.get(2,cursor_value);
        Serial.println("When the LED turns from GREEN to RED depress the upper cursor speed control button twice (2 times)");
        blink(1, 1500, 1);
        digitalWrite(LED_2, HIGH);
        EEPROM.get(2,speed_value);
        delay(2000);
        digitalWrite(LED_2, LOW);
        blink(1, 1000, 1);
        Serial.println("Push button should be released...");
        Serial.println(" --- ");
        
        if (speed_value == cursor_value+2) {
          Serial.println("A VALID cursor speed value was registered by the microcontroller. Test : PASS");
          Serial.println(" --- ");
          switch (8);
        } else {
          Serial.println("An INVALID cursor speed value was registered by the microcontroller. Test : FAIL");

          while (flag1 == 0) {
            Serial.println("Did you press the increase cursor speed button twice (2 times)? (Y/N)...");
            while (!Serial.available()) {
              // wait for user response
            }
            if (Serial.available() > 0) {
              char c = toUpperCase(Serial.read());
              if ( c == 'Y') {
                Serial.println("There may be a problem with the your increase cursor speed push button:");
                Serial.print("Original cursor speed value: ");
                Serial.println(cursor_value);
                Serial.print("Increased cursor speed value: ");
                Serial.println(speed_value);
                Serial.println("Solutions: check the solder joints on the increase cursor speed push button located on the main interface board if ");
                Serial.println("the solder joints are making good electrical contact then consider replacing this push button.");
                delay(3500);
                Serial.println("To continue cursor speed control diagnostic, press any key");

                while (!Serial.available()) {
                  // wait for user response
                }
                switch (8);
              } else if ( c == 'N') {
                while (flag2 == 0) {
                  Serial.println("Would you like to repeat the increase cursor speed push button test?");
                  while (!Serial.available()) {
                    // wait for user response
                  }
                  if (Serial.available() > 0) {
                    char c = toUpperCase(Serial.read());
                    if ( c == 'Y') {
                      switch (7);
                    } else if ( c == 'N') {
                      Serial.println("Continuing cursor speed control diagnostic...");
                      switch (8);
                    } else {
                      Serial.println("Invalid input, please re-enter response (Y/N)...");
                      flag2 = 0;
                    }
                  }
                }
              } else {
                Serial.println("Invalid input, please re-enter response (Y/N)...");
                flag1 = 0;
              }
            }
          }
        }
      }
    case 8: {
        // test 8 : decrease cursor speed control test
        flag1 = 0;
        flag2 = 0;

        EEPROM.put(2,i);
        delay(500);

        Serial.println("");
        Serial.println("");
        
        Serial.println("Continuing cursor speed control diagnostic. Follow on-screen prompts...");
        delay(2000);
        Serial.println("When the LED turns from GREEN to RED depress the lower cursor speed control button twice (2 times)");
        blink(1, 1500, 1);
        digitalWrite(LED_2, HIGH);
        EEPROM.get(2,speed_value);
        delay(2000);
        digitalWrite(LED_2, LOW);
        blink(1, 1000, 1);
        Serial.println("Push button should be released...");
        Serial.println(" --- ");
        
        if (speed_value == cursor_value-2) {
          Serial.println("A VALID cursor speed value was registered by the microcontroller. Test : PASS");
          Serial.println(" --- ");
          switch (9);
        } else {
          Serial.println("An INVALID cursor speed value was registered by the microcontroller. Test : FAIL");

          while (flag1 == 0) {
            Serial.println("Did you press the decrease cursor speed button twice (2 times)? (Y/N)...");
            while (!Serial.available()) {
              // wait for user response
            }
            if (Serial.available() > 0) {
              char c = toUpperCase(Serial.read());
              if ( c == 'Y') {
                Serial.println("There may be a problem with the your decrease cursor speed push button:");
                Serial.print("Original cursor speed value: ");
                Serial.println(cursor_value);
                Serial.print("Decreased cursor speed value: ");
                Serial.println(speed_value);
                Serial.println("Solutions: check the solder joints on the decrease cursor speed push button located on the main interface board if ");
                Serial.println("the solder joints are making good electrical contact then consider replacing this push button.");
                delay(3500);
                Serial.println("To continue diagnostic, press any key");

                while (!Serial.available()) {
                  // wait for user response
                }
                switch (9);
              } else if ( c == 'N') {
                while (flag2 == 0) {
                  Serial.println("Would you like to repeat the decrease cursor speed push button test?");
                  while (!Serial.available()) {
                    // wait for user response
                  }
                  if (Serial.available() > 0) {
                    char c = toUpperCase(Serial.read());
                    if ( c == 'Y') {
                      switch (8);
                    } else if ( c == 'N') {
                      Serial.println("Continuing diagnostic...");
                      switch (9);
                    } else {
                      Serial.println("Invalid input, please re-enter response (Y/N)...");
                      flag2 = 0;
                    }
                  }
                }
              } else {
                Serial.println("Invalid input, please re-enter response (Y/N)...");
                flag1 = 0;
              }
            }
          }
        }
      }
    case 9: {
        // test 6
      }
    case 10: {
        // test 6
      }
    case 11: {
        // test 6
      }
    case 12: {
        // test 6
      }
    case 13: {
        // test 6
      }
    case 14: {
        // test 6
      }
    case 15: {
        // test 6
      }
    case 16: {
        // test 6
      }
    case 17: {
        // test 6
      }
    case 18: {
        // test 6
      }
    case 19: {
        // test 6
      }
    case 20: {
        // test 6
      }
  }

  //test_number++;
}

//***LED FEEDBACK FUNCTION***//

void blink(int num_Blinks, int delay_Blinks, int LED_number ) {
  if (num_Blinks < 0) num_Blinks *= -1;

  if (LED_number == 1) {
    for (int i = 0; i < num_Blinks; i++) {
      digitalWrite(LED_1, HIGH);
      delay(delay_Blinks);
      digitalWrite(LED_1, LOW);
      delay(delay_Blinks);
    }
  }
  if (LED_number == 2) {
    for (int i = 0; i < num_Blinks; i++) {
      digitalWrite(LED_2, HIGH);
      delay(delay_Blinks);
      digitalWrite(LED_2, LOW);
      delay(delay_Blinks);
    }
  }
  if (LED_number == 3) {
    for (int i = 0; i < num_Blinks; i++) {
      digitalWrite(LED_1, HIGH);
      delay(delay_Blinks);
      digitalWrite(LED_1, LOW);
      delay(delay_Blinks);
      digitalWrite(LED_2, HIGH);
      delay(delay_Blinks);
      digitalWrite(LED_2, LOW);
      delay(delay_Blinks);
    }
  }
}


//***CURSOR SPEED FUNCTIONS***//

void Cursor_speed_value(void) {
  int var;
  EEPROM.get(2, var);
  delay(250);
  speed_counter = var;
}

void increase_cursor_speed(void) {
  speed_counter++;

  if (speed_counter == 7) {
    blink(6, 50, 3);     // twenty very fast blinks
    speed_counter = 6;
  } else {
    blink(speed_counter, 250, 1);
    cursor_delta = delta_values[speed_counter];
    EEPROM.put(2, speed_counter);
    delay(250);
    Serial.println("+");
  }
}

void decrease_cursor_speed(void) {
  speed_counter--;

  if (speed_counter == -1) {
    blink(6, 50, 3);     // twenty very fast blinks
    speed_counter = 0;
  } else if (speed_counter == 0) {
    blink(1, 500, 2);
    cursor_delta = delta_values[speed_counter];
    EEPROM.put(2, speed_counter);
    delay(250);
    Serial.println("-");
  } else {
    blink(speed_counter, 250, 1);
    cursor_delta = delta_values[speed_counter];
    EEPROM.put(2, speed_counter);
    delay(250);
    Serial.println("-");
  }
}

//***CURSOR MOVEMENT FUCNTIONS***//

int cursor_movement_high(int j) {
  int k = (int)(((((float)(j - 512)) / 512.0) * 127) / ((float)cursor_delta));
  return k;
}

int cursor_movement_low(int j) {
  int k = (int)(((((float)(512 - j)) / 512.0) * 127) / ((float)cursor_delta));
  return k;
}

void mouseCommand(int buttons, int x, int y, int scroll) {
  Serial1.write(0xFD);
  Serial1.write(0x05);
  Serial1.write(0x02);
  Serial1.write(buttons);                  // 0x01 is left click 0x02 is right click on CPU. 0x01 is a tap and 0x02 is back on mobile device.
  Serial1.write(x);
  Serial1.write(y);
  Serial1.write(scroll);
  delay(20);
}

void mouseClear(void) {

  int value = 0x00;

  Serial1.write(0xFD);
  Serial1.write(0x05);
  Serial1.write(0x02);
  Serial1.write(value);
  Serial1.write(value);
  Serial1.write(value);
  Serial1.write(value);
  delay(20);
}

//***COMMUNICATION MODE STATUS***//

void Communication_Mode_Status(void) {
  if (digitalRead(MODE_SELECT) == LOW) {
    comm_mode = 0;                                // 0 == USB communication
    Serial.println("comm_mode = 0");
    BT_Low_Power_Mode();
    delay(250);
    blink(6, 125, 1);
  } else if (digitalRead(MODE_SELECT) == HIGH) {
    comm_mode = 1;                                // 1 == Bluetooth communication
    delay(500);
    Serial.println("comm_mode = 1");
    blink(6, 125, 2);
  }
}

//***BLUETOOTH CONFIGURATION STATUS***//

int BT_Config_Status(void) {
  int i;
  EEPROM.get(0, i);
  delay(500);
  Serial.println(i);
  return i;
}

//***BLUETOOTH CONFIGURATION FUNCTIONS***//

void BT_Configure(void) {
  if (comm_mode == 1) {
    config_done = BT_Config_Status();                    // check if Bluetooth has previously been configured
    delay(100);
    if (config_done == 0) {                           // if Bluetooth has not been configured then execute configuration sequence
      BT_Command_Mode();                               // enter Bluetooth command mode
      BT_Config_Sequence();                           // send configuarion data to Bluetooth module
      BT_configAOK();                                 // returns diagnostic responses from Bluetooth
      delay(200);
    } else {
      Serial.println("Bluetooth configuration has previously been completed.");
      delay(200);
    }
  }
}

void BT_Command_Mode(void) {                 //***CHANGE THE TRANSISTOR CONTROLS ONCE THE PNP IS SWITCHED IN FOR THE NPN***
  digitalWrite(TRANS_CONTROL, HIGH);         // transistor base pin HIGH to ensure Bluetooth module is off
  digitalWrite(PIO4, HIGH);                 // command pin high
  delay(250);

  digitalWrite(TRANS_CONTROL, LOW);        // transistor base pin LOW to power on Bluetooth module
  delay(250);

  for (int i = 0; i < 3; i++) {             // cycle PIO4 pin high-low 3 times with 1 sec delay between each level transition
    digitalWrite(PIO4, HIGH);
    delay(1000);
    digitalWrite(PIO4, LOW);
    delay(1000);
  }

  digitalWrite(PIO4, LOW);                  // drive PIO4 pin low as per command mode instructions
  delay(250);
  Serial1.print("$$$");                     // enter Bluetooth command mode :: "$$$" CANNOT be Serial.println("$$$") ONLY Serial.print("$$$")
  delay(250);                              // time delay to visual inspect the red LED is flashing at 10Hz which indicates the Bluetooth module is in Command Mode
  Serial.println("Bluetooth Command Mode Activated");
}

void BT_Config_Sequence(void) {                    // change all Serial instructions to Serial1
  Serial1.println("ST,255");                 // turn off the 60 sec timer for command mode
  delay(100);
  Serial1.println("SA,1");                   // ***NEW ADDITION - Authentication Values 1: default security
  delay(100);
  Serial1.println("SX,1");                   // ***NEW ADDITION - Bonding 1: enabled - accepts connections to paired devices
  delay(100);
  Serial1.println("SN,LipSyncBT_Autoload1"); // change name of BT module
  delay(100);
  Serial1.println("SM,6");                   // ***NEW ADDITION - Pairing "SM,6": pairing auto-connect mode
  delay(100);
  Serial1.println("SH,0220");                // configure device as HID mouse
  delay(100);
  Serial1.println("S~,6");                   // activate HID profile
  delay(100);

  //Serial1.println("SW,<hex value>");       // sniff mode conserves power by polling the radio for comms ***POWER CONSERVATION
  //delay(100);
  //Serial1.println("SY,<hex value>");       // set transmit power settings ***POWER CONSERVATION

  Serial1.println("R,1");                    // reboot BT module
  delay(100);

  int val0 = 1;
  int val1 = 3;
  delay(250);
  EEPROM.put(0, val0);                        // EEPROM address 0 gets configuration completed value (== 1)
  delay(100);
  EEPROM.put(2, val1);                        // EEPROM address 1 gets default cursor speed counter value (== 20) ***SHOULD ONLY OCCUR ONCE UNLESS THE LIPSYNC IS FACTORY RESET??
  delay(100);
}

void BT_Low_Power_Mode(void) {
  BT_Command_Mode();                          // enter BT command mode
  Serial1.println('Z');                       // enter deep sleep mode (<2mA) when not connected
  delay(250);
  Serial.println("Bluetooth Deep Sleep Mode Activated");
  delay(250);
  blink(2, 1000, 3);                          // visual feedback
}

void BT_configAOK(void) {                    // diagnostic feedback from Bluetooth configuration
  while (Serial1.available() > 0) {
    Serial.print((char)Serial1.read());
  }
  Serial.println("");
  Serial.println("Configuration complete.");
}

//***SPECIAL INITIALIZATION OPERATIONS***//

void Set_Default_Values(void) {                   // SHOULD ONLY OCCUR ONCE IN THE WHOLE LIFETIME OF LIPSYNC UNLESS factory_reset() FUNCTION IS CALLED
  int var0;
  EEPROM.get(0, var0);
  if (var0 != 0 && var0 != 1) {
    var0 = 0;
    EEPROM.put(0, var0);
    delay(100);
    int var1 = 3;
    EEPROM.put(2, var1);
    delay(100);
    Serial.println("this shouldn't happen");
  }
}

void Joystick_Initialization(void) {
  x_center = analogRead(X_DIRECTION);             // initial resting x position of joystick
  y_center = analogRead(Y_DIRECTION);             // initial resting y position of joystick

  box_delta = 10;                                 // ***CAN BE CHANGED TO CREATE MORE SENSITIVE MOVEMENT ACTIVATION !!!ORIGINALLY 25
  //^^ COULD BE USED FOR PERSONS WITH SPASTIC OR INADVERTENT MOVEMENTS?
  x_box_right = x_center + box_delta;             // create right x threshold
  x_box_left = x_center - box_delta;              // create left x threshold

  y_box_up = y_center + box_delta;                // create upper y threshold
  y_box_down = y_center - box_delta;              // create lower y threshold
}

void Pressure_Sensor_Initialization(void) {
  nominal_cursor_value = (((float)analogRead(PRESSURE_CURSOR)) / 1024.0) * 5.0; //read initial pressure sensor value
  delay(20);
  sip_threshold = nominal_cursor_value + 0.15;    //create sip pressure threshold value ***REVERSED
  delay(20);
  puff_threshold = nominal_cursor_value - 0.15;   //create puff pressure threshold value ***REVERSED
  delay(20);
}

void Mouse_Configure(void) {
  if (comm_mode == 0) {
    Mouse.begin();
    delay(250);
  }
}

void factory_reset(void) {
  int var0 = 3;
  EEPROM.put(0, var0);
  delay(100);
}
