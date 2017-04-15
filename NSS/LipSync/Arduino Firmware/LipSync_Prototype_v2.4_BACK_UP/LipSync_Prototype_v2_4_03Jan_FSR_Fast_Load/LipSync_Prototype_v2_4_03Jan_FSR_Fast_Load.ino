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
   REVISION HISTORY:
   27 Jun 2016
   08 Jul 2016
   11 Jul 2016
   15 Jul 2016
   18 Jul 2016
   29 Jul 2016
   02 Aug 2016
   03 Aug 2016
   16 Aug 2016
   18 Aug 2016
   25 Aug 2016
   26 Aug 2016
   27 Aug 2016
   29 Aug 2016
   30 Aug 2016
   04 Sept 2016
   13 Sept 2016
   14 Sept 2016
   19 Sept 2016
   26 Sept 2016
   27 Sept 2016
   29 Sept 2016
   06 Oct 2016
   12 Oct 2016
   17 Oct 2016
   25 Oct 2016
   04 Nov 2016
   07 Nov 2016
   ---
   30 Nov 2016
   02 Dec 2016
   12 Dec 2016
   13 Dec 2016
   14 Dec 2016
   15 Dec 2016
   20 Dec 2016
   23 Dec 2016
   24 Dec 2016
   30 Dec 2016
   03 Jan 2017
*/

#include <EEPROM.h>
#include <Mouse.h>
#include <math.h>

//***PIN ASSIGNMENTS***//

#define MODE_SELECT 12                            // LipSync Mode Select - USB mode (comm_mode = 0; jumper on) or Bluetooth mode (comm_mode = 1; jumper off) - digital input pin 12 (internally pulled-up)
#define PUSH_BUTTON_UP 8                          // Cursor Control Button 1: UP - digital input pin 8 (internally pulled-up)
#define PUSH_BUTTON_DOWN 7                        // Cursor Control Button 2: DOWN - digital input pin 7 (internally pulled-up)
#define LED_1 5                                   // LipSync LED Color1 - digital output pin 5
#define LED_2 4                                   // LipSync LED Color2 - digital outputpin 4

#define TRANS_CONTROL A3                          // Bluetooth Transistor Control Pin - digital output pin A3
#define PIO4 A4                                   // Bluetooth PIO4 Command Pin - digital output pin A4

#define PRESSURE_CURSOR A5                        // Sip & Puff Pressure Transducer Pin - analog input pin A5
#define X_DIR_HIGH A0                             // X Direction High (Cartesian positive x : right) - analog input pin A0
#define X_DIR_LOW A1                              // X Direction Low (Cartesian negative x : left) - digital output pin A1
#define Y_DIR_HIGH A2                             // Y Direction High (Cartesian positive y : up) - analog input pin A2
#define Y_DIR_LOW A10                             // Y Direction Low (Cartesian negative y : down) - analog input pin A10

//***VARIABLE DECLARATION***//

int xh, yh, xl, yl;                               // xh: x-high, yh: y-high, xl: x-low, yl: y-low
int x_center, y_center;                           // the relative x,y resting center of the joystick
int x_box_right, x_box_left, y_box_up, y_box_down;// boundaries for which x,y values must exceed to produce cursor movement
int box_delta;                                    // the delta value for the boundary range in all 4 directions about the x,y center
int cursor_delta;                                 // amount cursor moves in some single or combined direction
int speed_counter = 2;                            // cursor speed counter
int cursor_click_status = 0;                      // value indicator for click status, ie. tap, back and drag
int comm_mode = 0;                                // 0 == USB Communications or 1 == Bluetooth Communications
int config_done;                                  // Binary check of completed Bluetooth configuration
unsigned long puff_count, sip_count;              // long puff and long sip incremental counter

int poll_counter = 0;                             // cursor poll counter
int init_counter = 0;                             // serial port initialization counter

int cursor_delay;
float cursor_factor;
int cursor_max_speed;

float sip_threshold, puff_threshold, cursor_click, cursor_back;

typedef struct {
  int _delay;
  float _factor;
  int _max_speed;
} _cursor;

_cursor setting1 = {5, -1.0, 100}; // 5,-1.0,10
_cursor setting2 = {5, -1.2, 100}; // 5,-1.2,10
_cursor setting3 = {5, -1.4, 100}; //        10
_cursor setting4 = {5, -1.6, 100}; //        10
_cursor setting5 = {5, -1.8, 100}; //        10

_cursor cursor_params[5] = {setting1, setting2, setting3, setting4, setting5};

//float cursor_factor = {-1.0, -1.2, -1.4, -1.6, -1.8, -2.0};

int single = 0;
int puff1, puff2;

//-----------------------------------------------------------------------------------------------------------------------------------

//***MICROCONTROLLER AND PERIPHERAL MODULES CONFIGURATION***//

void setup() {

  Serial.begin(115200);                           // setting baud rate for serial coms for diagnostic data return from Bluetooth and microcontroller ***MAY REMOVE LATER***
  Serial1.begin(115200);                          // setting baud rate for Bluetooth module

  //pinMode(LED_1, OUTPUT);                         // visual feedback #1
  //pinMode(LED_2, OUTPUT);                         // visual feedback #2
  //pinMode(TRANS_CONTROL, OUTPUT);                 // transistor pin output
  //pinMode(PIO4, OUTPUT);                          // command mode pin output

  pinMode(PRESSURE_CURSOR, INPUT);                // pressure sensor pin input
  pinMode(X_DIR_HIGH, INPUT);   // redefine the pins when all has been finalized
  pinMode(X_DIR_LOW, INPUT);    // ditto above
  pinMode(Y_DIR_HIGH, INPUT);   // ditto above
  pinMode(Y_DIR_LOW, INPUT);    // ditto above

  pinMode(MODE_SELECT, INPUT_PULLUP);             // LOW: USB (default with jumper in) HIGH: Bluetooth (jumper removed)
  pinMode(PUSH_BUTTON_UP, INPUT_PULLUP);          // increase cursor speed button
  pinMode(PUSH_BUTTON_DOWN, INPUT_PULLUP);        // decrease cursor speed button

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  //pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);

  Serial_Initialization();

  /*while (!Serial) {
    // need to wait
    }

    while (!Serial1) {
    // need to wait
    }*/

  //delay(2000);

  Set_Default();                                  // should only occur once per initialization of a new microcontroller
  delay(10);
  Pressure_Sensor_Initialization();               // register nominal air pressure and generate activation threshold boundaries
  delay(10);
  Joystick_Initialization();                      // home joystick and generate movement threshold boundaries
  delay(10);
  Communication_Mode_Status();                    // identify the selected communication mode
  delay(10);
  Mouse_Configure();                              // conditionally activate the HID mouse functions
  delay(10);
  BT_Configure();                                 // conditionally configure the Bluetooth module [WHAT IF A NEW BT MODULE IS INSTALLED?]
  delay(10);
  cursor_speed_value();                           // reads saved cursor speed parameter from EEPROM
  delay(10);

  int exec_time = millis();
  Serial.print("Configuration time: ");
  Serial.println(exec_time);

  blink(1, 50, 3);                               // end initialization visual feedback

  cursor_delay = cursor_params[speed_counter]._delay;
  cursor_factor = cursor_params[speed_counter]._factor;
  cursor_max_speed = cursor_params[speed_counter]._max_speed;

  // functions below are for diagnostic feedback only

  Serial.print("config_done: ");
  Serial.println(EEPROM.get(0, puff1));
  delay(5);
  Serial.print("speed_counter: ");
  Serial.println(EEPROM.get(2, puff2));
  delay(5);
  Serial.print("cursor_delay: ");
  Serial.println(cursor_params[puff2]._delay);
  delay(5);
  Serial.print("cursor_factor: ");
  Serial.println(cursor_params[puff2]._factor);
  delay(5);
  Serial.print("cursor_max_speed: ");
  Serial.println(cursor_params[puff2]._max_speed);
  delay(5);

}

//-----------------------------------------------------------------------------------------------------------------------------------

//***START OF INFINITE LOOP***//

void loop() {


  if (single == 0) {
    Serial.println(" ");
    Serial.println(" --- ");
    Serial.println("This is the 30 December - Stable Fast Start-Up FSR version.");
    Serial.println(" --- ");

    forceDisplayCursor();

    //mouseCommand(0,30,0,0);
    //mouseCommand(0,127,0,0);
    /*
      delay(1500);

      Mouse.move(20,0,0);

      delay(250);

      Mouse.move(20,0,0);

      delay(250);

      unsigned long time1 = micros();

      Mouse.move(20,0,0);

      unsigned long time2 = micros();

      Serial.println(time2 - time1);*/
    /*int ral3 = 3;
      EEPROM.put(4, ral3);        // testing Bluetooth config ***CAN BE REMOVED
      delay(100);*/
    single++;

    Serial.print("X high: ");
    Serial.println(analogRead(X_DIR_HIGH));
    delay(10);
    Serial.print("X low: ");
    Serial.println(analogRead(X_DIR_LOW));
    delay(10);
    Serial.print("Y high: ");
    Serial.println(analogRead(Y_DIR_HIGH));
    delay(10);
    Serial.print("Y low: ");
    Serial.println(analogRead(Y_DIR_LOW));
    delay(10);

  }


  xh = analogRead(X_DIR_HIGH);                    // A0 :: NOT CORRECT MAPPINGS
  xl = analogRead(X_DIR_LOW);                     // A1
  yh = analogRead(Y_DIR_HIGH);                    // A2
  yl = analogRead(Y_DIR_LOW);                     // A10

  //cursor movement control functions below

  if (xh > x_box_right || xl > x_box_left || yh > y_box_up || yl > y_box_down) {

    poll_counter++;

    delay(15);

    if (poll_counter >= 3) {

      if (comm_mode == 0) {
        if (xh > x_box_right && (yh < y_box_up && yl < y_box_down)) {
          Mouse.move(x_cursor_high(xh), 0, 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
        } else if (xl > x_box_left && (yh < y_box_up && yl < y_box_down)) {
          Mouse.move(x_cursor_low(xl), 0, 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
        } else if (yh > y_box_up && (xh < x_box_right && xl < x_box_left)) {
          Mouse.move(0, y_cursor_high(yh), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
        } else if (yl > y_box_down && (xh < x_box_right && xl < x_box_left)) {
          Mouse.move(0, y_cursor_low(yl), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
        } else if (xh > x_box_right && yh > y_box_up) {
          Mouse.move(x_cursor_high(xh), y_cursor_high(yh), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
        } else if (xh > x_box_right && yl > y_box_down) {
          Mouse.move(x_cursor_high(xh), y_cursor_low(yl), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
        } else if (xl > x_box_left && yh > y_box_up) {
          Mouse.move( x_cursor_low(xl), y_cursor_high(yh), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
        } else if (xl > x_box_left && yl > y_box_down) {
          Mouse.move( x_cursor_low(xl), y_cursor_low(yl), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
        }
      } else {
        /*if (xh > x_box_right && (yh < y_box_up && yl < y_box_down)) {
          mouseCommand(cursor_click_status, x_cursor_high(x), 0, 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
          } else if (xl > x_box_left && (yh < y_box_up && yl < y_box_down)) {
          mouseCommand(cursor_click_status, x_cursor_low(x), 0, 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
          } else if (yh > y_box_up && (xh < x_box_right && xl < x_box_left)) {
          mouseCommand(cursor_click_status, 0, y_cursor_high(y), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
          } else if (yl > y_box_down && (xh < x_box_right && xl < x_box_left)) {
          mouseCommand(cursor_click_status, 0, y_cursor_low(y), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
          } else if (xh > x_box_right && yh > y_box_up) {
          mouseCommand(cursor_click_status, x_cursor_high(x), y_cursor_high(y), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
          } else if (xh > x_box_right && yl > y_box_down) {
          mouseCommand(cursor_click_status, x_cursor_high(x), y_cursor_low(y), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
          } else if (xl > x_box_left && yh > y_box_up) {
          mouseCommand(cursor_click_status, x_cursor_low(x), y_cursor_high(y), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
          } else if (xl > x_box_left && yl > y_box_down) {
          mouseCommand(cursor_click_status, x_cursor_low(x), y_cursor_low(y), 0);
          delay(cursor_delay);
          poll_counter = 0;
          //delay(5);
          }*/
      }
    }
  }

  //pressure sensor sip and puff functions below

  cursor_click = (((float)analogRead(PRESSURE_CURSOR)) / 1023.0) * 5.0;

  if (cursor_click < puff_threshold) {
    while (cursor_click < puff_threshold) {
      cursor_click = (((float)analogRead(PRESSURE_CURSOR)) / 1023.0) * 5.0;
      puff_count++;         // NEED TO FIGURE OUT ROUGHLY HOW LONG ONE CYCLE OF THIS WHILE LOOP IS -> COUNT THRESHOLD
      delay(5);
    }
    Serial.println(puff_count);             //***REMOVE

    if (comm_mode == 0) {
      if (puff_count < 150) {
        Mouse.click(MOUSE_LEFT);
        delay(100);
      } else if (puff_count > 150) {
        if (Mouse.isPressed(MOUSE_LEFT)) {
          Mouse.release(MOUSE_LEFT);
        } else {
          Mouse.press(MOUSE_LEFT);
          delay(100);
        }
      } else {
        /*
            --> RE-EVALUATE THIS FUNCTION
           Mouse.press(MOUSE_LEFT)
           delay(750);
        */
      }
    } else {
      if (puff_count < 150) {
        cursor_click_status = 1; //change this stuff to hex
        mouseCommand(cursor_click_status, 0, 0, 0);
        mouseClear();
        cursor_click_status = 0;
        delay(100);
      } else if (puff_count > 150) {
        if (cursor_click_status == 0) {
          cursor_click_status = 1;
        } else if (cursor_click_status == 1) {
          cursor_click_status = 0;
        }
      } else {
        /*
            --> RE-EVALUATE THIS FUNCTION
           cursor_click_status = 1;
           mouseCommand(cursor_click_status, 0, 0, 0);
           delay(750);
        */
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
    Serial.println(sip_count);             //***REMOVE

    if (comm_mode == 0) {
      if (sip_count < 150) {
        Mouse.click(MOUSE_RIGHT);
        delay(100);
      } else if (sip_count > 150) {
        mouseScroll();
        delay(100);
      } else {
        /*
            --> RE-EVALUATE THIS FUNCTION
           Mouse.press(MOUSE_RIGHT)
           delay(750);
        */
      }
    } else {
      if (sip_count < 150) {
        cursor_click_status = 2;
        mouseCommand(cursor_click_status, 0, 0, 0);
        mouseClear();
        cursor_click_status = 0;
        delay(100);
      } else if (sip_count > 150) {
        mouseScroll();
        delay(100);
      } else {
        /*
            --> RE-EVALUATE THIS FUNCTION
           cursor_click_status = 2;
           mouseCommand(cursor_click_status, 0, 0, 0);
           delay(750);
        */
      }
    }

    sip_count = 0;
  }

  //cursor speed control push button functions below

  if (digitalRead(PUSH_BUTTON_UP) == LOW) {
    increase_cursor_speed();      // increase cursor speed with push button up
    delay(50);                   // software debounce
  }

  if (digitalRead(PUSH_BUTTON_DOWN) == LOW) {
    decrease_cursor_speed();      // decrease cursor speed with push button down
    delay(50);                   // software debounce
  }
}

//***END OF INFINITE LOOP***//

//-----------------------------------------------------------------------------------------------------------------------------------

//***LED FEEDBACK FUNCTIONS***//

void blink(int num_Blinks, int delay_Blinks, int LED_number ) {
  if (num_Blinks < 0) num_Blinks *= -1;

  switch (LED_number) {
    case 1: {
        for (int i = 0; i < num_Blinks; i++) {
          digitalWrite(LED_1, HIGH);
          delay(delay_Blinks);
          digitalWrite(LED_1, LOW);
          delay(delay_Blinks);
        }
        break;
      }
    case 2: {
        for (int i = 0; i < num_Blinks; i++) {
          digitalWrite(LED_2, HIGH);
          delay(delay_Blinks);
          digitalWrite(LED_2, LOW);
          delay(delay_Blinks);
        }
        break;
      }
    case 3: {
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
        break;
      }
  }
}

//***CURSOR SPEED FUNCTIONS***//

void cursor_speed_value(void) {
  int var;
  EEPROM.get(2, var);
  delay(5);
  speed_counter = var;
}

void increase_cursor_speed(void) {
  speed_counter++;

  if (speed_counter == 5) {
    blink(6, 50, 3);     // twelve very fast blinks
    speed_counter = 4;
  } else {
    blink(speed_counter, 100, 1);

    cursor_delay = cursor_params[speed_counter]._delay;
    cursor_factor = cursor_params[speed_counter]._factor;
    cursor_max_speed = cursor_params[speed_counter]._max_speed;

    EEPROM.put(2, speed_counter);
    delay(25);
    Serial.println("+");
  }
}

void decrease_cursor_speed(void) {
  speed_counter--;

  if (speed_counter == -1) {
    blink(6, 50, 3);     // twelve very fast blinks
    speed_counter = 0;
  } else if (speed_counter == 0) {
    blink(1, 500, 2);

    cursor_delay = cursor_params[speed_counter]._delay;
    cursor_factor = cursor_params[speed_counter]._factor;
    cursor_max_speed = cursor_params[speed_counter]._max_speed;

    EEPROM.put(2, speed_counter);
    delay(25);
    Serial.println("-");
  } else {
    blink(speed_counter, 100, 1);

    cursor_delay = cursor_params[speed_counter]._delay;
    cursor_factor = cursor_params[speed_counter]._factor;
    cursor_max_speed = cursor_params[speed_counter]._max_speed;

    EEPROM.put(2, speed_counter);
    delay(25);
    Serial.println("-");
  }
}

//***NEW CURSOR MOVEMENT FUNCTIONS***//

int y_cursor_high(int j) {

  int k = (int)(round(-1.0 * cursor_max_speed * (1.0 - pow(2.718, (cursor_factor * (((float)(j - y_box_up)) / y_box_up))))));

  //int k = (int)(round((((float)(y_center - j))/y_center) * 100)/cursor_delta);

  /*
    Serial.print("Yh:");
    Serial.print(j);
    Serial.print(", ");
    Serial.print("k:");
    Serial.println(k);
  */
  return k;

}

int y_cursor_low(int j) {

  int k = (int)(round(1.0 * cursor_max_speed * (1.0 - pow(2.718, (cursor_factor * (((float)(j - y_box_down)) / y_box_down))))));

  //int k = (int)(round((((float)(y_center - j))/y_center) * 100)/cursor_delta);

  /*
    Serial.print("Yl:");
    Serial.print(j);
    Serial.print(", ");
    Serial.print("k:");
    Serial.println(k);
  */
  return k;
}

int x_cursor_high(int j) {

  int k = (int)(round(1.0 * cursor_max_speed * (1.0 - pow(2.718, (cursor_factor * (((float)(j - x_box_right)) / x_box_right))))));

  //int k = (int)(round((((float)(j - x_center))/x_center) * 100)/cursor_delta);

  /*
    Serial.print("Xh:");
    Serial.print(j);
    Serial.print(", ");
    Serial.print("k:");
    Serial.println(k);
  */
  return k;
}

int x_cursor_low(int j) {

  int k = (int)(round(-1.0 * cursor_max_speed * (1.0 - pow(2.718, (cursor_factor * (((float)(j - x_box_left)) / x_box_left))))));

  //int k = (int)(round((((float)(j - x_center))/x_center) * 100)/cursor_delta);

  /*
    Serial.print("Xl:");
    Serial.print(j);
    Serial.print(", ");
    Serial.print("k:");
    Serial.println(k);
  */
  return k;
}

//***BLUETOOTH HID MOUSE FUNCTIONS***//

void mouseCommand(int buttons, int x, int y, int scroll) {

  byte BTcursor[7];

  BTcursor[0] = 0xFD;
  BTcursor[1] = 0x5;
  BTcursor[2] = 0x2;
  BTcursor[3] = lowByte(buttons);
  BTcursor[4] = lowByte(x);
  BTcursor[5] = lowByte(y);
  BTcursor[6] = lowByte(scroll);

  Serial1.write(BTcursor, 7);
  Serial1.flush();

  /*
    Serial1.write(0xFD);
    Serial1.write(0x5);
    Serial1.write(0x2);
    Serial1.write((byte)buttons);                  // 0x01 is left click 0x02 is right click on CPU. 0x01 is a tap and 0x02 is back on mobile device.
    Serial1.write((byte)x);
    Serial1.write((byte)y);
    Serial1.write((byte)scroll);*/
  delay(10);
}

void mouseClear(void) {

  //int value = 0x00;

  byte BTcursor[7];

  BTcursor[0] = 0xFD;
  BTcursor[1] = 0x5;
  BTcursor[2] = 0x2;
  BTcursor[3] = 0x00;
  BTcursor[4] = 0x00;
  BTcursor[5] = 0x00;
  BTcursor[6] = 0x00;

  Serial1.write(BTcursor, 7);
  //Serial1.flush();

  /*
    Serial1.write(0xFD);
    Serial1.write(0x05);
    Serial1.write(0x02);
    Serial1.write(value);
    Serial1.write(value);
    Serial1.write(value);
    Serial1.write(value);*/
  delay(10);
}

//***MOUSE SCROLLING FUNCTION***//

void mouseScroll(void) {
  while (1) {
    /*
      int scroll = analogRead(Y_DIRECTION);

      float scroll_release = (((float)analogRead(PRESSURE_CURSOR)) / 1023.0) * 5.0;

      if (comm_mode == 0) {

        if (scroll > y_box_up) {
          Mouse.move(0, 0, -1*y_cursor_high(scroll));
          delay(cursor_delay*70);
        } else if (scroll < y_box_down) {
          Mouse.move(0, 0, -1*y_cursor_low(scroll));
          delay(cursor_delay*70);
        } else if (scroll_release > sip_threshold || scroll_release < puff_threshold) {
          break;
        }
      } else {
        if (scroll > y_box_up) {
          mouseCommand(0, 0, 0, -1*y_cursor_high(scroll));
          delay(cursor_delay*70);
        } else if (scroll < y_box_down){
          mouseCommand(0, 0, 0, -1*y_cursor_low(scroll));
          delay(cursor_delay*70);
        } else if (scroll_release > sip_threshold || scroll_release < puff_threshold) {
          break;
        }
      }*/
  }
}

//***FORCE DISPLAY OF CURSOR***//

void forceDisplayCursor(void) {
  if (comm_mode == 0) {
    Mouse.move(1, 0, 0);
    delay(25);
    Mouse.move(-1, 0, 0);
    delay(25);
  } else {
    mouseCommand(0, 1, 0, 0);
    delay(25);
    mouseCommand(0, -1, 0, 0);
    delay(25);
  }
}

//***COMMUNICATION MODE STATUS***//

void Communication_Mode_Status(void) {
  if (digitalRead(MODE_SELECT) == LOW) {
    comm_mode = 0;                                // 0 == USB communication
    Serial.println("comm_mode = 0");
    BT_Low_Power_Mode();
    delay(10);
    //blink(6, 125, 1);
  } else if (digitalRead(MODE_SELECT) == HIGH) {
    comm_mode = 1;                                // 1 == Bluetooth communication
    delay(10);
    Serial.println("comm_mode = 1");
    //blink(6, 125, 2);
  }
}

//***JOYSTICK INITIALIZATION FUNCTION***//

void Joystick_Initialization(void) {
  xh = analogRead(X_DIR_HIGH);            // Initial neutral x-high value of joystick
  delay(10);
  Serial.println(xh);                     // Recommend keeping in for diagnostic purposes
  
  xl = analogRead(X_DIR_LOW);             // Initial neutral x-low value of joystick
  delay(10);
  Serial.println(xl);                     // Recommend keeping in for diagnostic purposes
  
  yh = analogRead(Y_DIR_HIGH);            // Initial neutral y-high value of joystick
  delay(10);
  Serial.println(yh);                     // Recommend keeping in for diagnostic purposes
  
  yl = analogRead(Y_DIR_LOW);             // Initial neutral y-low value of joystick
  delay(10);
  Serial.println(yl);                     // Recommend keeping in for diagnostic purposes


  box_delta = 30;                         // Threshold digital value for FSR joystick to exceed before cursor movement occurs
                                          //***CAN BE CHANGED TO CREATE MORE SENSITIVE MOVEMENT ACTIVATION *ORIGINALLY 25
                                          // A large value tends to minimize frequency of emergent cursor drift

  //^^ COULD BE INCREASED FOR PERSONS WITH SPASTIC OR INADVERTENT MOVEMENTS?

  x_box_right = xh + box_delta;             // Create horizontal right x-direction threshold
  x_box_left = xl + box_delta;              // Create horizontal left x-direction threshold

  y_box_up = yh + box_delta;                // Create vertical up y-direction threshold
  y_box_down = yl + box_delta;              // Create vertical down y-direction threshold
}

//***PRESSURE SENSOR INITIALIZATION FUNCTION***//

void Pressure_Sensor_Initialization(void) {
  float nominal_cursor_value = (((float)analogRead(PRESSURE_CURSOR)) / 1024.0) * 5.0; // Initial neutral pressure transducer analog value [0.0V - 5.0V]
  delay(10);
  
  sip_threshold = nominal_cursor_value + 1.75;    //Create sip pressure threshold value ***Larger values tend to minimize frequency of inadvertent activation
  //delay(20);
  
  puff_threshold = nominal_cursor_value - 1.75;   //Create puff pressure threshold value ***Larger values tend to minimize frequency of inadvertent activation
  //delay(20);
}

//***ARDUINO/GENUINO HID MOUSE INITIALIZATION FUNCTION***//

void Mouse_Configure(void) {
  if (comm_mode == 0) {                       // USB mode is comm_mode == 0, this is when the jumper on J13 is installed
    Mouse.begin();                            // Initialize the HID mouse functions from Mouse.h header file
    delay(25);                                // Allow extra time for initialization to take effect ***May be removed later
  }
}

//***RN-42 BLUETOOTH MODULE INITIALIZATION SECTION***//

//***BLUETOOTH CONFIGURATION STATUS FUNCTION***//

void BT_Config_Status(void) {
  int BT_EEPROM = 3;                               // Local integer variable initialized and defined for use with EEPROM GET function
  EEPROM.get(0, BT_EEPROM);                        // Assign value of EEPROM memory at index zero (0) to int variable BT_EEPROM
  delay(10);
  Serial.println(BT_EEPROM);                       // Only for diagnostics, may be removed later
  config_done = BT_EEPROM;                         // Assign value of local variable BT_EEPROM to global variable config_done 
  delay(10);
}

//***BLUETOOTH CONFIGURATION FUNCTION***//

void BT_Configure(void) {
  if (comm_mode == 1) {
    BT_Config_Status();                    // check if Bluetooth has previously been configured
    delay(10);
    if (config_done == 0) {                           // if Bluetooth has not been configured then execute configuration sequence
      BT_Command_Mode();                               // enter Bluetooth command mode
      BT_Config_Sequence();                           // send configuarion data to Bluetooth module
      BT_configAOK();                                 // returns diagnostic responses from Bluetooth
      delay(10);
    } else {
      Serial.println("Bluetooth configuration has previously been completed.");
      delay(10);
    }
  }
}

void BT_Command_Mode(void) {                 //***CHANGE THE TRANSISTOR CONTROLS ONCE THE PNP IS SWITCHED IN FOR THE NPN***
  //digitalWrite(TRANS_CONTROL, HIGH);         // transistor base pin HIGH to ensure Bluetooth module is off
  //digitalWrite(PIO4, HIGH);                 // command pin high
  delay(10);

  //digitalWrite(TRANS_CONTROL, LOW);        // transistor base pin LOW to power on Bluetooth module
  delay(10);

  for (int i = 0; i < 3; i++) {             // cycle PIO4 pin high-low 3 times with 1 sec delay between each level transition
    //digitalWrite(PIO4, HIGH);
    delay(75);
    //digitalWrite(PIO4, LOW);
    delay(75);
  }

  //digitalWrite(PIO4, LOW);                  // drive PIO4 pin low as per command mode instructions
  delay(10);
  Serial1.print("$$$");                     // enter Bluetooth command mode :: "$$$" CANNOT be Serial.println("$$$") ONLY Serial.print("$$$")
  delay(10);                              // time delay to visual inspect the red LED is flashing at 10Hz which indicates the Bluetooth module is in Command Mode
  Serial.println("Bluetooth Command Mode Activated");
}

void BT_Config_Sequence(void) {                    // change all Serial instructions to Serial1
  Serial1.println("ST,255");                 // turn off the 60 sec timer for command mode
  delay(15);
  Serial1.println("SA,1");                   // ***NEW ADDITION - Authentication Values 1: default security
  delay(15);
  Serial1.println("SX,1");                   // ***NEW ADDITION - Bonding 1: enabled - accepts connections to paired devices
  delay(15);
  Serial1.println("SN,LipSyncBT_Autoload1"); // change name of BT module
  delay(15);
  Serial1.println("SM,6");                   // ***NEW ADDITION - Pairing "SM,6": pairing auto-connect mode
  delay(15);
  Serial1.println("SH,0220");                // configure device as HID mouse
  delay(15);
  Serial1.println("S~,6");                   // activate HID profile
  delay(15);

  //Serial1.println("SW,<hex value>");       // sniff mode conserves power by polling the radio for comms ***POWER CONSERVATION
  //delay(100);
  //Serial1.println("SY,<hex value>");       // set transmit power settings ***POWER CONSERVATION

  Serial1.println("SQ,16");                  // configure for latency NOT throughput -> turn off: "SQ,0"
  delay(15);
  Serial1.println("S?,1");                   // 1:ENABLE role switch -> slave device attempts role switch -> indicates better performance for high speed data
  delay(15);
  Serial1.println("R,1");                    // reboot BT module
  delay(15);

  int val0 = 1;
  int val1 = 3;
  //delay(250);
  EEPROM.put(0, val0);                        // EEPROM address 0 gets configuration completed value (== 1)
  delay(10);
  EEPROM.put(2, val1);                        // EEPROM address 1 gets default cursor speed counter value (== 20) ***SHOULD ONLY OCCUR ONCE UNLESS THE LIPSYNC IS FACTORY RESET??
  delay(10);
  int val3;
  EEPROM.get(0, val3);  // diagnostics
  delay(10);            // diagnostics
  Serial.println(val3); // diagnostics
}

void BT_Low_Power_Mode(void) {
  BT_Command_Mode();                          // enter BT command mode
  Serial1.println('Z');                       // enter deep sleep mode (<2mA) when not connected
  delay(10);
  //digitalWrite(TRANS_CONTROL, LOW);
  Serial.println("Bluetooth Deep Sleep Mode Activated");
  delay(10);
  //blink(2, 1000, 3);                          // visual feedback
}

void BT_configAOK(void) {                    // diagnostic feedback from Bluetooth configuration
  while (Serial1.available() > 0) {
    Serial.print((char)Serial1.read());
  }
  Serial.println("");
  Serial.println("Configuration complete.");
}

//***SPECIAL INITIALIZATION OPERATIONS***//

void Serial_Initialization(void) {
  while (!Serial || !Serial1) {

    if (init_counter < 80) {
      delay(25);
      init_counter++;
    } else {
      break;
    }
  }
  Serial.println("Serial is good!");
}

void Set_Default(void) {
  int ral0;
  int ral1;
  int ral2;

  EEPROM.get(4, ral2);
  delay(10);

  if (ral2 != 1) {
    EEPROM.get(0, ral0);
    delay(10);
    EEPROM.get(2, ral1);
    delay(10);

    if (ral0 != 0) {
      ral0 = 0;
      EEPROM.put(0, ral0);
      delay(10);
    }

    if (ral1 != 3) {
      ral1 = 3;
      EEPROM.put(2, ral1);
      delay(10);
    }

    ral2 = 1;
    EEPROM.put(4, ral2);
    delay(10);

  }
}
