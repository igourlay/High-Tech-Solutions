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
 * 26 Aug 2016
 * 27 Aug 2016
 * 29 Aug 2016
 * 30 Aug 2016
 * 04 Sept 2016
 * 13 Sept 2016
 * 14 Sept 2016
 * 19 Sept 2016
 * 26 Sept 2016
 * 27 Sept 2016
 * 29 Sept 2016
 * 06 Oct 2016
 * 12 Oct 2016
 * 17 Oct 2016
 * 25 Oct 2016
 * 04 Nov 2016
 * 07 Nov 2016
 */

#include <EEPROM.h>
#include <Mouse.h>
#include <math.h>

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
int speed_counter = 2;                            // cursor speed counter
int cursor_click_status = 0;                      // value indicator for click status, ie. tap, back and drag
int comm_mode = 0;                                // 0 == USB Communications or 1 == Bluetooth Communications
int config_done;                                  // Binary check of completed Bluetooth configuration
unsigned long puff_count, sip_count;              // long puff and long sip incremental counter

int cursor_delay;
float cursor_factor;
int cursor_max_speed;

float sip_threshold, puff_threshold, cursor_click, cursor_back;

typedef struct {
  int _delay;
  float _factor;
  int _max_speed;
} _cursor;

_cursor setting1 = {5, -1.0, 10}; // 5,-1.4,12
_cursor setting2 = {5, -1.2, 10}; // 5,-1.4,14
_cursor setting3 = {5, -1.4, 10}; //        16
_cursor setting4 = {5, -1.6, 10}; //        18
_cursor setting5 = {5, -1.8, 10}; //        20

_cursor cursor_params[5] = {setting1, setting2, setting3, setting4, setting5};

//float cursor_factor = {-1.0, -1.2, -1.4, -1.6, -1.8, -2.0};

int single = 0;
int puff1, puff2;

//-----------------------------------------------------------------------------------------------------------------------------------

//***MICROCONTROLLER AND PERIPHERAL MODULES CONFIGURATION***//

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

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);

  Serial.begin(115200);                           // setting baud rate for serial coms for diagnostic data return from Bluetooth and microcontroller ***MAY REMOVE LATER***
  Serial1.begin(115200);                          // setting baud rate for Bluetooth module

  delay(1000);

  

  Set_Default();                                  // should only occur once per initialization of a new microcontroller
  delay(100);
  Joystick_Initialization();                      // home joystick and generate movement threshold boundaries
  delay(100);
  Pressure_Sensor_Initialization();               // register nominal air pressure and generate activation threshold boundaries
  delay(100);
  Communication_Mode_Status();                    // identify the selected communication mode
  delay(100);
  Mouse_Configure();                              // conditionally activate the HID mouse functions
  delay(100);
  BT_Configure();                                 // conditionally configure the Bluetooth module [WHAT IF A NEW BT MODULE IS INSTALLED?]
  delay(100);
  cursor_speed_value();                           // reads saved cursor speed parameter from EEPROM
  delay(100);
  
  cursor_delay = cursor_params[speed_counter]._delay;
  cursor_factor = cursor_params[speed_counter]._factor;
  cursor_max_speed = cursor_params[speed_counter]._max_speed;
  
  delay(100);

  blink(6, 100, 3);                               // end initialization visual feedback

  // functions below are for diagnostic feedback only

  Serial.print("config_done: ");
  Serial.println(EEPROM.get(0, puff1));
  delay(100);
  Serial.print("speed_counter: ");
  Serial.println(EEPROM.get(2, puff2));
  delay(100);
  Serial.print("cursor_delay: ");
  Serial.println(cursor_params[puff2]._delay);
  delay(100);
  Serial.print("cursor_factor: ");
  Serial.println(cursor_params[puff2]._factor);
  delay(100);
  Serial.print("cursor_max_speed: ");
  Serial.println(cursor_params[puff2]._max_speed);
  delay(100);
  Serial.println(x_center);
  Serial.println(y_center);


}

//-----------------------------------------------------------------------------------------------------------------------------------

//***START OF INFINITE LOOP***//

void loop() {

  
    if (single == 0) {
      Serial.println(" ");
      Serial.println(" --- ");
      Serial.println("This is the 07 November - Stable version.");
      Serial.println(" --- ");

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
      single++;
    }
  

  x = analogRead(X_DIRECTION);                    // read x-direction value
  y = analogRead(Y_DIRECTION);                    // read y-direction value

  //cursor movement control functions below

  if (x > x_box_right || x < x_box_left || y > y_box_up || y < y_box_down) {

    if (comm_mode == 0) {
      if (x > x_box_right && (y < y_box_up && y > y_box_down)) {
        Mouse.move(x_cursor_high(x), 0, 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x < x_box_left && (y < y_box_up && y > y_box_down)) {
        Mouse.move(x_cursor_low(x), 0, 0);
        delay(cursor_delay);
        //delay(5);
      } else if (y > y_box_up && (x < x_box_right && x > x_box_left)) {
        Mouse.move(0, y_cursor_high(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (y < y_box_down && (x < x_box_right && x > x_box_left)) {
        Mouse.move(0, y_cursor_low(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x > x_box_right && y > y_box_up) {
        Mouse.move(x_cursor_high(x), y_cursor_high(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x > x_box_right && y < y_box_down) {
        Mouse.move(x_cursor_high(x), y_cursor_low(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x < x_box_left && y > y_box_up) {
        Mouse.move( x_cursor_low(x), y_cursor_high(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x < x_box_left && y < y_box_down) {
        Mouse.move( x_cursor_low(x), y_cursor_low(y), 0);
        delay(cursor_delay);
        //delay(5);
      }
    } else {
      if (x > x_box_right && (y < y_box_up && y > y_box_down)) {
        mouseCommand(cursor_click_status, x_cursor_high(x), 0, 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x < x_box_left && (y < y_box_up && y > y_box_down)) {
        mouseCommand(cursor_click_status, x_cursor_low(x), 0, 0);
        delay(cursor_delay);
        //delay(5);
      } else if (y > y_box_up && (x < x_box_right && x > x_box_left)) {
        mouseCommand(cursor_click_status, 0, y_cursor_high(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (y < y_box_down && (x < x_box_right && x > x_box_left)) {
        mouseCommand(cursor_click_status, 0, y_cursor_low(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x > x_box_right && y > y_box_up) {
        mouseCommand(cursor_click_status, x_cursor_high(x), y_cursor_high(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x > x_box_right && y < y_box_down) {
        mouseCommand(cursor_click_status, x_cursor_high(x), y_cursor_low(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x < x_box_left && y > y_box_up) {
        mouseCommand(cursor_click_status, x_cursor_low(x), y_cursor_high(y), 0);
        delay(cursor_delay);
        //delay(5);
      } else if (x < x_box_left && y < y_box_down) {
        mouseCommand(cursor_click_status, x_cursor_low(x), y_cursor_low(y), 0);
        delay(cursor_delay);
        //delay(5);
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
         *  --> RE-EVALUATE THIS FUNCTION
         * Mouse.press(MOUSE_LEFT)
         * delay(750);
         */
      }
    } else {
      if (puff_count < 150) {
        cursor_click_status = 1;
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
         *  --> RE-EVALUATE THIS FUNCTION
         * cursor_click_status = 1;
         * mouseCommand(cursor_click_status, 0, 0, 0);
         * delay(750);
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
        if (Mouse.isPressed(MOUSE_RIGHT)) {
          Mouse.release(MOUSE_RIGHT);
        } else {
          Mouse.press(MOUSE_RIGHT);
          delay(100);
        }
      } else {
        /*
         *  --> RE-EVALUATE THIS FUNCTION
         * Mouse.press(MOUSE_RIGHT)
         * delay(750);
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
        if (cursor_click_status == 0) {
          cursor_click_status = 2;
        } else if (cursor_click_status == 2) {
          cursor_click_status = 0;
        }
      } else {
        /*
         *  --> RE-EVALUATE THIS FUNCTION
         * cursor_click_status = 2;
         * mouseCommand(cursor_click_status, 0, 0, 0);
         * delay(750);
         */
      }
    }

    sip_count = 0;
  }

  //cursor speed control push button functions below

  if (digitalRead(PUSH_BUTTON_UP) == LOW) {
    increase_cursor_speed();      // increase cursor speed with push button up
    delay(250);                   // software debounce
  }

  if (digitalRead(PUSH_BUTTON_DOWN) == LOW) {
    decrease_cursor_speed();      // decrease cursor speed with push button down
    delay(250);                   // software debounce
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
  delay(250);
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
    delay(250);
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
    delay(250);
    Serial.println("-");
  } else {
    blink(speed_counter, 100, 1);
    
    cursor_delay = cursor_params[speed_counter]._delay;
    cursor_factor = cursor_params[speed_counter]._factor;
    cursor_max_speed = cursor_params[speed_counter]._max_speed;
    
    EEPROM.put(2, speed_counter);
    delay(250);
    Serial.println("-");
  }
}


//***1st CURSOR MOVEMENT FUNCTIONS***//
/*
int y_cursor_high(int j) {
  int k =  ((int)(((((float)(y_center - j)) / y_center) * 127.0) / (float)cursor_delta)); 
  return k;
}

int y_cursor_low(int j) {
  int k = (int)(((((float)(y_center - j)) / y_center) * 127.0) / (float)cursor_delta); 
  return k;
}

int x_cursor_high(int j) {
  int k = (int)(((((float)(j - x_center)) / x_center) * 127.0) / (float)cursor_delta); 
  return k;
}

int x_cursor_low(int j) {
  int k = ((int)(((((float)(j - x_center)) / x_center) * 127.0) / (float)cursor_delta)); 
  return k;
}
*/

//***2nd NEW CURSOR MOVEMENT FUNCTIONS***//

int y_cursor_high(int j){

  int k = (int)(round(-1.0*cursor_max_speed*(1.0-pow(2.718,(cursor_factor*(((float)(j-y_center))/y_center))))));
  
  //int k = (int)(round((((float)(y_center - j))/y_center) * 100)/cursor_delta);

  /*
  Serial.print("Y:");
  Serial.print(j);
  Serial.print(", ");
  Serial.print("k:");
  Serial.println((byte)k);
  */
  return k;

}

int y_cursor_low(int j){

  int k = (int)(round(cursor_max_speed*(1.0-pow(2.718,(cursor_factor*(((float)(y_center-j))/y_center))))));
  
  //int k = (int)(round((((float)(y_center - j))/y_center) * 100)/cursor_delta);
  
  /*
  Serial.print("Y:");
  Serial.print(j);
  Serial.print(", ");
  Serial.print("k:");
  Serial.println((byte)k);
  */
  return k;
}

int x_cursor_high(int j){

  int k = (int)(round(cursor_max_speed*(1.0-pow(2.718,(cursor_factor*(((float)(j-x_center))/x_center))))));
  
  //int k = (int)(round((((float)(j - x_center))/x_center) * 100)/cursor_delta);

  /*
  Serial.print("X:");
  Serial.print(j);
  Serial.print(", ");
  Serial.print("k:");
  Serial.println((byte)k);
  */
  return k;
}

int x_cursor_low(int j){

  int k = (int)(round(-1.0*cursor_max_speed*(1.0-pow(2.718,(cursor_factor*(((float)(x_center-j))/x_center))))));
  
  //int k = (int)(round((((float)(j - x_center))/x_center) * 100)/cursor_delta);
  
  /*
  Serial.print("X:");
  Serial.print(j);
  Serial.print(", ");
  Serial.print("k:");
  Serial.println((byte)k);
  */
  return k;
}

//***BLUETOOTH HID MOUSE FUNCTIONS***//

void mouseCommand(int buttons, int x, int y, int scroll) {

  Serial1.write(0xFD);
  Serial1.write(0x5);
  Serial1.write(0x2);
  Serial1.write((byte)buttons);                  // 0x01 is left click 0x02 is right click on CPU. 0x01 is a tap and 0x02 is back on mobile device.
  Serial1.write((byte)x);
  Serial1.write((byte)y);
  Serial1.write((byte)scroll);
  delay(5);
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
  delay(5);
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

//***JOYSTICK INITIALIZATION FUNCTION***//

void Joystick_Initialization(void) {
  x_center = analogRead(X_DIRECTION);             // initial resting x position of joystick
  delay(250);
  y_center = analogRead(Y_DIRECTION);             // initial resting y position of joystick
  delay(250);
  
  box_delta = 25;                                 // ***CAN BE CHANGED TO CREATE MORE SENSITIVE MOVEMENT ACTIVATION *ORIGINALLY 25
  
  //^^ COULD BE INCREASED FOR PERSONS WITH SPASTIC OR INADVERTENT MOVEMENTS?
  
  x_box_right = x_center + box_delta;             // create right x threshold
  x_box_left = x_center - box_delta;              // create left x threshold

  y_box_up = y_center + box_delta;                // create upper y threshold
  y_box_down = y_center - box_delta;              // create lower y threshold
}

//***PRESSURE SENSOR INITIALIZATION***//

void Pressure_Sensor_Initialization(void) {
  float nominal_cursor_value = (((float)analogRead(PRESSURE_CURSOR)) / 1024.0) * 5.0; //read initial pressure sensor value
  delay(20);
  sip_threshold = nominal_cursor_value + 0.5;    //create sip pressure threshold value ***Originally: +0.15
  delay(20);
  puff_threshold = nominal_cursor_value - 0.5;   //create puff pressure threshold value ***Originally: -0.15
  delay(20);
}

//***ARDUINO MOUSE FUNCTIONS INITIALIZATION***//

void Mouse_Configure(void) {
  if (comm_mode == 0) {
    Mouse.begin();
    delay(250);
  }
}

//***BLUETOOTH CONFIGURATION FUNCTIONS***//

void BT_Config_Status(void) {
  int val4 = 3;
  Serial.println(val4);
  EEPROM.get(0, val4);
  delay(100);
  config_done = val4;
  Serial.println(val4);
  delay(100);
  Serial.println(config_done);
  delay(100);
}

void BT_Configure(void) {
  if (comm_mode == 1) {
    BT_Config_Status();                    // check if Bluetooth has previously been configured
    delay(100);
    if (config_done == 0) {                           // if Bluetooth has not been configured then execute configuration sequence
      BT_Command_Mode();                               // enter Bluetooth command mode
      BT_Config_Sequence();                           // send configuarion data to Bluetooth module
      BT_configAOK();                                 // returns diagnostic responses from Bluetooth
      delay(100);
    } else {
      Serial.println("Bluetooth configuration has previously been completed.");
      delay(100);
    }
  }
}

void BT_Command_Mode(void) {                 //***CHANGE THE TRANSISTOR CONTROLS ONCE THE PNP IS SWITCHED IN FOR THE NPN***
  digitalWrite(TRANS_CONTROL, HIGH);         // transistor base pin HIGH to ensure Bluetooth module is off
  digitalWrite(PIO4, HIGH);                 // command pin high
  delay(100);

  digitalWrite(TRANS_CONTROL, LOW);        // transistor base pin LOW to power on Bluetooth module
  delay(100);

  for (int i = 0; i < 3; i++) {             // cycle PIO4 pin high-low 3 times with 1 sec delay between each level transition
    digitalWrite(PIO4, HIGH);
    delay(1000);
    digitalWrite(PIO4, LOW);
    delay(1000);
  }

  digitalWrite(PIO4, LOW);                  // drive PIO4 pin low as per command mode instructions
  delay(100);
  Serial1.print("$$$");                     // enter Bluetooth command mode :: "$$$" CANNOT be Serial.println("$$$") ONLY Serial.print("$$$")
  delay(100);                              // time delay to visual inspect the red LED is flashing at 10Hz which indicates the Bluetooth module is in Command Mode
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
  int val3;
  EEPROM.get(0,val3);
  Serial.println(val3);
}

void BT_Low_Power_Mode(void) {
  BT_Command_Mode();                          // enter BT command mode
  Serial1.println('Z');                       // enter deep sleep mode (<2mA) when not connected
  delay(250);
  digitalWrite(TRANS_CONTROL, LOW);
  Serial.println("Bluetooth Deep Sleep Mode Activated");
  delay(250);
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

void Set_Default(void){
  int ral0;
  int ral1;
  int ral2;

  EEPROM.get(4, ral2);
  delay(100);
  
  if(ral2 != 1){
  EEPROM.get(0, ral0);
  delay(100);
  EEPROM.get(2, ral1);
  delay(100);

  if(ral0 != 0){
    ral0 = 0;
    EEPROM.put(0, ral0);
    delay(100);
  }

  if(ral1 != 3){
    ral1 = 3;
    EEPROM.put(2, ral1);
    delay(100);
  }

  ral2 = 1;
  EEPROM.put(4,ral2);
  delay(100);
  
  }
}

