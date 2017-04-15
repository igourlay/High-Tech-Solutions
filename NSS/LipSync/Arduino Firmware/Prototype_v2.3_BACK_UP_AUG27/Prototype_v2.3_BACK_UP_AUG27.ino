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

//AUTHOR: Ivan Gourlay 22 June 2016

/*
 * REVISION HISTORY:
 * 27 Jun 2016
 * 08 Jul 2016
 * 11 Jul 2016
 * 15 Jul 2016
 * 18 Jul 2016
 * 29 Jul 2016
 * 02 Aug 2016
 * 03 Aug 2016
 * 16 Aug 2016
 * 18 Aug 2016
 * 25 Aug 2016
 */

#include <EEPROM.h>

//***DEFINITIONS AND VARIABLE DECLARATIONS***//

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
int speed_counter = 0;                            // cursor speed counter
int cursor_click_status = 0;                      // value indicator for click status, ie. tap, back and drag
int comm_mode = 0;                                // 0: USB Communications or 1: Bluetooth Communications
int i = 0u;
int config_done = 0;
unsigned long puff_count, sip_count;              // long puff and long sip incremental counter
float nominal_cursor_value, sip_threshold, puff_threshold, cursor_click, cursor_back;

int single = 0;

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

  Mouse.begin();                                  // Mouse.move(x,y,?), Mouse.release(), Mouse.click(),

  delay(1000);

  //communication mode select (USB or Bluetooth)
  if (digitalRead(MODE_SELECT) == LOW) {
    comm_mode = 0;                                // USB communication
    delay(500);
    Serial.println("comm_mode = 0");
  } else if (digitalRead(MODE_SELECT) == HIGH) {
    comm_mode = 1;                                // Bluetooth communication
    delay(500);
    Serial.println("comm_mode = 1");
  }



  blink(6, 100, 3);

  if (comm_mode == 1) {
    config_done = config_status();                    // check if Bluetooth has previously been configured
    delay(100);
    if (config_done == 0) {                           // if Bluetooth has not been configured then execute configuration sequence
      BT_CommandMode();                               // enter Bluetooth command mode
      BT_Configure();                                 // send configuarion data to Bluetooth module
      BT_configAOK();                                 // returns diagnostic responses from Bluetooth
      delay(200);
    } else {
      Serial.println("Bluetooth configuration has previously been completed.");
      delay(200);
    }
  }

  x_center = analogRead(X_DIRECTION);             // initial resting x position of joystick
  y_center = analogRead(Y_DIRECTION);             // initial resting y position of joystick

  // creating a center box which the joystick values must exceed in order to move the cursor
  // TROUBLESHOOT: if the cursor moves in one direction automatically this may indicate that the center position needs to be
  // reset in which case you can simply unplug then plug it back in, or potentially use a reset button

  box_delta = 25;                                 // value for creating the boundary box around cursor movement

  x_box_right = x_center + box_delta;             // create right x threshold
  x_box_left = x_center - box_delta;              // create left x threshold

  y_box_up = y_center + box_delta;                // create upper y threshold
  y_box_down = y_center - box_delta;              // create lower y threshold

  nominal_cursor_value = (((float)analogRead(PRESSURE_CURSOR)) / 1024.0) * 5.0; //read initial pressure sensor value
  delay(20);
  sip_threshold = nominal_cursor_value + 0.15;    //create sip pressure threshold value ***REVERSED
  delay(20);
  puff_threshold = nominal_cursor_value - 0.15;   //create puff pressure threshold value ***REVERSED
  delay(20);
  cursor_delta = 20;                              //reciprocal proportionality amount cursor travels per iterated loop
  delay(250);

  blink(3, 500, 1);

  //***START REMOVAL
  Serial.print("pressure sensor threshold: ");    //REMOVE
  Serial.println(nominal_cursor_value);           //REMOVE
  Serial.print("sip_threshold: ");                //REMOVE
  Serial.println(sip_threshold);                  //REMOVE
  Serial.print("puff_threshold: ");               //REMOVE
  Serial.println(puff_threshold);                 //REMOVE
  //***STOP REMOVAL
}

//***START OF LOOP***//

void loop() {

  /*if (single == 0) {
    int var = 0;
    EEPROM.put(0, var);
    delay(100);
    single++;
  }*/

  x = analogRead(X_DIRECTION);                    // read x-direction value
  y = analogRead(Y_DIRECTION);                    // read y-direction value

  //cursor speed control push button functions below

  if (digitalRead(PUSH_BUTTON_UP) == LOW) {
    increase_cursor_speed();      // increase cursor speed with push button up
    delay(250);                   // software debounce
  }
  if (digitalRead(PUSH_BUTTON_DOWN) == LOW) {
    decrease_cursor_speed();      // decrease cursor speed with push button down
    delay(250);                   // software debounce
  }

  //pressure sensor sip/puff functions below

  cursor_click = (((float)analogRead(PRESSURE_CURSOR)) / 1023.0) * 5.0;

  if (cursor_click < puff_threshold) {
    while (cursor_click < puff_threshold) {
      cursor_click = (((float)analogRead(PRESSURE_CURSOR)) / 1023.0) * 5.0;
      puff_count++;         // NEED TO FIGURE OUT ROUGHLY HOW LONG ONE CYCLE OF THIS WHILE LOOP IS -> COUNT THRESHOLD
      delay(5);
    }
    Serial.println(puff_count);             //REMOVE

    if (puff_count < 100 && cursor_click_status != 1) { //short tap (smartphone) or left mouse click (PC)
      if (comm_mode == 0) {
        Mouse.click(MOUSE_LEFT);
        cursor_click_status = 0;
        delay(250);
      } else {
        cursor_click_status = 1;
        mouseCommand(cursor_click_status, 0, 0, 0);
        mouseClear();
        cursor_click_status = 0;
        delay(250);
      }
    } else if (puff_count < 100 && cursor_click_status == 1) {  //drag release (smartphone) or left mouse click release (PC)
      if (comm_mode == 0) {
        Mouse.release(MOUSE_LEFT);
        cursor_click_status = 0;
        delay(250);
      } else {
        cursor_click_status = 0;
        delay(250);
      }
    } else if (puff_count < 300 && puff_count > 100 && cursor_click_status != 1) {   //drag (double check function) or left mouse click hold (PC)
      if (comm_mode == 0) {
        Mouse.press(MOUSE_LEFT);
        cursor_click_status = 1;
        delay(250);
      } else {
        cursor_click_status = 1;
        delay(250);
      }
    } else {  //long tap and drag (smartphone) and left mouse click and drag (PC) (***double check function)
      if (comm_mode == 0) {
        Mouse.press(MOUSE_LEFT);
        cursor_click_status = 1;
        delay(750);
      } else {
        cursor_click_status = 1;
        mouseCommand(cursor_click_status, 0, 0, 0);
        delay(750);
      }
    }
    puff_count = 0;
  }
  if (cursor_click > sip_threshold) {
    while (cursor_click > sip_threshold) {
      cursor_click = (((float)analogRead(PRESSURE_CURSOR)) / 1023.0) * 5.0;
      sip_count++;         // NEED TO FIGURE OUT ROUGHLY HOW LONG ONE CYCLE OF THIS WHILE LOOP IS -> COUNT THRESHOLD
      delay(5);
    }
    Serial.println(sip_count);             //REMOVE

    if (sip_count < 100 && cursor_click_status != 2) { //back button (smartphone) or right mouse click (PC)
      if (comm_mode = 0) {
        Mouse.click(MOUSE_RIGHT);
        cursor_click_status = 2;
        delay(250);
      } else {
        cursor_click_status = 2;
        mouseCommand(cursor_click_status, 0, 0, 0);
        mouseClear();
        cursor_click_status = 0;
        delay(250);
      }
    } else if (sip_count < 100 && cursor_click_status == 2) { //enable 2nd function
      if (comm_mode == 0) {
        //disengage secondary function with Arduino HID
        delay(250);
      } else {
        //disengage secondary function with Bluetooth HID
        delay(250);
      }
    } else if (sip_count > 100 && cursor_click_status != 2) {
      if (comm_mode == 0) {
        //engage seconday function with Arduino HID
        delay(250);
      } else {
        //engage secondary function with Bluetooth HID
        delay(250);
      }
    }
    sip_count = 0;
  }

  //cursor movement control functions below

  if (x > x_box_right || x < x_box_left || y > y_box_up || y < y_box_down) {

    if (x > x_box_right && (y < y_box_up && y > y_box_down)) {
      if (comm_mode == 0) {
        Mouse.move(cursor_movement_high(x), 0, 0);
        delay(20);
      } else {
        mouseCommand(cursor_click_status, cursor_movement_high(x), 0, 0);
        delay(20);
      }
    } else if (x < x_box_left && (y < y_box_up && y > y_box_down)) {
      if (comm_mode == 0) {
        Mouse.move(-1 * cursor_movement_low(x), 0, 0);
        delay(20);
      } else {
        mouseCommand(cursor_click_status, -1 * cursor_movement_low(x), 0, 0);
        delay(20);
      }
    } else if (y > y_box_up && (x < x_box_right && x > x_box_left)) {
      if (comm_mode == 0) {
        Mouse.move(0, -1 * cursor_movement_high(y), 0);
        delay(20);
      } else {
        mouseCommand(cursor_click_status, 0, -1 * cursor_movement_high(y), 0);
        delay(20);
      }
    } else if (y < y_box_down && (x < x_box_right && x > x_box_left)) {
      if (comm_mode == 0) {
        Mouse.move(0, cursor_movement_low(y), 0);
        delay(20);
      } else {
        mouseCommand(cursor_click_status, 0, cursor_movement_low(y), 0);
        delay(20);
      }
    } else if (x > x_box_right && y > y_box_up) {
      if (comm_mode == 0) {
        Mouse.move(cursor_movement_high(x), -1 * cursor_movement_high(y), 0);
        delay(20);
      } else {
        mouseCommand(cursor_click_status, cursor_movement_high(x), -1 * cursor_movement_high(y), 0);
        delay(20);
      }
    } else if (x > x_box_right && y < y_box_down) {
      if (comm_mode == 0) {
        Mouse.move(cursor_movement_high(x), cursor_movement_low(y), 0);
        delay(20);
      } else {
        mouseCommand(cursor_click_status, cursor_movement_high(x), cursor_movement_low(y), 0);
        delay(20);
      }
    } else if (x < x_box_left && y > y_box_up) {
      if (comm_mode == 0) {
        Mouse.move(-1 * cursor_movement_low(x), -1 * cursor_movement_high(y), 0);
        delay(20);
      } else {
        mouseCommand(cursor_click_status, -1 * cursor_movement_low(x), -1 * cursor_movement_high(y), 0);
        delay(20);
      }
    } else if (x < x_box_left && y < y_box_down) {
      if (comm_mode == 0) {
        Mouse.move(-1 * cursor_movement_low(x), cursor_movement_low(y), 0);
        delay(20);
      } else {
        mouseCommand(cursor_click_status, -1 * cursor_movement_low(x), cursor_movement_low(y), 0);
        delay(20);
      }
    }
  }
}

//***END OF LOOP***//

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

void increase_cursor_speed(void) {
  speed_counter++;
  if (speed_counter == 0) {
    blink(10, 100, 1);     // ten very fast blinks
    cursor_delta = 20;  // redundant
    speed_counter = 0;  // redundant
  } else if (speed_counter == 4) {
    blink(5, 100, 1);      // five very fast blinks
    cursor_delta = 5;
    speed_counter = 3;
  } else {
    blink(speed_counter, 300, 1);
    cursor_delta -= 5;
  }
}

void decrease_cursor_speed(void) {
  speed_counter--;
  if (speed_counter == 0) {
    blink(10, 100, 1);     // ten very fast blinks
    cursor_delta = 20;  // redundant
    speed_counter = 0;  // redundant
  } else if (speed_counter == -4) {
    blink(5, 100, 1);      // five very fast blinks
    cursor_delta = 35;
    speed_counter = -3;
  } else {
    blink(speed_counter, 300, 1);
    cursor_delta += 5;
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

//***BLUETOOTH CONFIGURATION STATUS***//

int config_status(void) {
  EEPROM.get(0, i);
  delay(500);
  return i;
}

//***BLUETOOTH CONFIGURATION FUNCTIONS***//

void BT_CommandMode(void) {                 //***CHANGE THE TRANSISTOR CONTROLS ONCE THE PNP IS SWITCHED IN FOR THE NPN***
  digitalWrite(TRANS_CONTROL, HIGH);         // transistor base pin HIGH to ensure Bluetooth module is off
  digitalWrite(PIO4, HIGH);                 // command pin high
  delay(500);

  digitalWrite(TRANS_CONTROL, LOW);        // transistor base pin LOW to power on Bluetooth module
  delay(500);

  for (int i = 0; i < 3; i++) {             // cycle PIO4 pin high-low 3 times with 1 sec delay between each level transition
    digitalWrite(PIO4, HIGH);
    delay(1000);
    digitalWrite(PIO4, LOW);
    delay(1000);
    Serial.println("Done.");
  }

  digitalWrite(PIO4, LOW);                  // drive PIO4 pin low as per command mode instructions
  delay(500);
  Serial1.print("$$$");                     // enter Bluetooth command mode :: "$$$" CANNOT be Serial.println("$$$") ONLY Serial.print("$$$")
  delay(2000);                              // time delay to visual inspect the red LED is flashing at 10Hz which indicates the Bluetooth module is in Command Mode
}

void BT_Configure(void) {                    // change all Serial instructions to Serial1
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

  //*** SPECIAL SECTION ***//
  int val = 1;
  EEPROM.put(0, val);
  delay(100);

}

void BT_configAOK(void) {                    // diagnostic feedback from Bluetooth configuration
  while (Serial1.available() > 0) {
    Serial.print((char)Serial1.read());
  }
  Serial.println("");
  Serial.println("Configuration complete.");
}
