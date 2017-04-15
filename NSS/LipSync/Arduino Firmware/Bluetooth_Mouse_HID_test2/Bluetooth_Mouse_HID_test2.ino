//AUTHOR: Ivan Gourlay 22 June 2016

/*
 * REVISION HISTORY:
 * 27 June 2016
 * 08 July 2016
 * 11 July 2016
 */

#define TRANS_CONTROL 12                          //transistor pin
#define PRESSURE_CURSOR A0                        //pressure sensor pin
#define X_DIRECTION A2                            //joystick x-direction pin
#define Y_DIRECTION A1                            //joystick y-direction pin
#define PIO4 11                                   //PIO4 pin
#define LED_PIN 13                                //on-board LED

int x, y;
int x_center, y_center;
int x_box_right, x_box_left, y_box_up, y_box_down, box_delta;
int speed_counter;
int cursor_delta;
int cursor_click_status = 0;
float nominal_cursor_value, sip_threshold, puff_threshold, cursor_click, cursor_back;

int tick, took = 0;                               //Testing variables
String done = "";                                 //Testing string

void setup() {

  pinMode(LED_PIN, OUTPUT);                       // used for checking status of data transmission and start up sequence
  pinMode(PRESSURE_CURSOR, INPUT);                // used for the pressure sensor data
  pinMode(X_DIRECTION, INPUT);                    // used for the joystick x data
  pinMode(Y_DIRECTION, INPUT);                    // used for the joystick y data
  pinMode(TRANS_CONTROL, OUTPUT);                 // transistor pin :: output
  pinMode(PIO4, OUTPUT);                          // command mode pin :: output

  // pinMode(toggle_up_pin, INPUT_PULLUP);
  // pinMode(toggle_down_pin, INPUT_PULLUP);

  Serial.begin(115200);                           //setting baud rate for serial comms for testing ***REMOVE LATER***
  Serial1.begin(115200);                          //setting baud rate for BT module

  BT_CommandMode();                               //enter BT command mode
  BT_Configure();                                 //send configuarion data to BT
  BT_configAOK();                                 //returns responses of configuration
  delay(200);

  Serial.println("");
  Serial.println("Configuration complete.");
  //Serial.println("To exit command mode press X");

  x_center = analogRead(X_DIRECTION);             //read initial x-coord of resting joystick
  y_center = analogRead(Y_DIRECTION);             //read initial y-coord of resting joystick

  Serial.print("x center: ");
  Serial.println(x_center);

  // creating a center box which the joystick values must exceed in order to move the cursor
  // TROUBLESHOOT: if the cursor moves in one direction automatically this may indicate that the center position needs to be
  // reset in which case you can simply unplug then plug it back in, or potentially use a reset button

  box_delta = 25;                                 //value for creating the boundary box around cursor movement

  x_box_right = x_center + box_delta;             //create right x threshold
  x_box_left = x_center - box_delta;              //create left x threshold

  y_box_up = y_center + box_delta;                //create upper y threshold
  y_box_down = y_center - box_delta;              //create lower y threshold

  nominal_cursor_value = (((float)analogRead(PRESSURE_CURSOR)) / 1024.0) * 5.0; //read initial pressure sensor value
  delay(20);
  Serial.print("pressure sensor threshold: ");
  Serial.println(nominal_cursor_value);           //REMOVE
  sip_threshold = nominal_cursor_value - 0.10;    //create sip pressure threshold value
  delay(20);
  Serial.print("sip_threshold: ");                //REMOVE
  Serial.println(sip_threshold);                  //REMOVE
  puff_threshold = nominal_cursor_value + 0.10;   //create puff pressure threshold value
  delay(20);
  Serial.print("puff_threshold: ");               //REMOVE
  Serial.println(puff_threshold);                 //REMOVE

  speed_counter = 0;                              //initialize speed counter as unadjusted value
  cursor_delta = 11;                              //amount cursor travels per loop
  delay(250);
}

void loop() {

  x = analogRead(X_DIRECTION);                             //read x-direction value
  y = analogRead(Y_DIRECTION);                             //read y-direction value

  //code below is to allow maker to read any pertinent BT module information before exiting command mode

  if (Serial1.available() && Serial1.peek() != '!') {                      // If the bluetooth sent any characters
    Serial.print((char)Serial1.read());           // Send any characters the bluetooth prints to the serial monitor
  }
  if (Serial.available()) {                       // If stuff was typed in the serial monitor
    char check = toUpperCase((char)Serial.read());// Send any characters the Serial monitor prints to the bluetooth

    if (check == 'X') {
      BT_ExitCommandMode();                       //exit BT configuration mode - cursor will not work while in config mode
    } else if (check == 'C') {
      changeCursorSpeed();                        //changes the cursor speed by cursor delta value == "increased cursor speed"
      Serial.print("Speed changed:");             //replace with visual confirmation with final design
      Serial.println(cursor_delta);
      check = 'P';
    } else {
      Serial1.print(check);
    }
  }

  //code below controls the pressure sensor sip/puff functions

  cursor_click = (((float)analogRead(PRESSURE_CURSOR)) / 1024.0) * 5.0;

  if (cursor_click > puff_threshold) {
    cursor_click_status = 1;
    mouseCommand(cursor_click_status, 0, 0, 0);
    mouseClear();
    cursor_click_status = 0;
    delay(250);
  } else if (cursor_click < sip_threshold) {
    cursor_click_status = 2;
    mouseCommand(cursor_click_status, 0, 0, 0);
    mouseClear();
    cursor_click_status = 0;
    delay(250);
  }

  // Build into the code something that will allow for long sip and long puff   //***NEEDS TO BE DONE

  //cursor direction change functions

  if (x > x_box_right || x < x_box_left || y > y_box_up || y < y_box_down) {

    if (x > x_box_right && (y < y_box_up && y > y_box_down)) {
      mouseCommand(cursor_click_status, cursor_movement_high(x), 0, 0);
      delay(20);
    } else if (x < x_box_left && (y < y_box_up && y > y_box_down)) {
      mouseCommand(cursor_click_status, -1 * cursor_movement_low(x), 0, 0);
      delay(20);
    } else if (y > y_box_up && (x < x_box_right && x > x_box_left)) {
      mouseCommand(cursor_click_status, 0, cursor_movement_high(y), 0);
      delay(20);
    } else if (y < y_box_down && (x < x_box_right && x > x_box_left)) {
      mouseCommand(cursor_click_status, 0, -1 * cursor_movement_low(y), 0);
      delay(20);
    } else if (x > x_box_right && y > y_box_up) {
      mouseCommand(cursor_click_status, cursor_movement_high(x), cursor_movement_high(y), 0);
      delay(20);
    } else if (x > x_box_right && y < y_box_down) {
      mouseCommand(cursor_click_status, cursor_movement_high(x), -1 * cursor_movement_low(y), 0);
      delay(20);
    } else if (x < x_box_left && y > y_box_up) {
      mouseCommand(cursor_click_status, -1 * cursor_movement_low(x), cursor_movement_high(y), 0);
      delay(20);
    } else if (x < x_box_left && y < y_box_down) {
      mouseCommand(cursor_click_status, -1 * cursor_movement_low(x), -1 * cursor_movement_low(y), 0);
      delay(20);
    }

  }

}

int cursor_movement_high(int j) {
  int k = (int)(((((float)(j - 512)) / 512.0) * 127) / ((float)cursor_delta));
  return k;
}

int cursor_movement_low(int j) {
  int k = (int)(((((float)(512 - j)) / 512.0) * 127) / ((float)cursor_delta));
  return k;
}

void statusLED(int numberOfBlinks, int flashDelay) {
  for (int i = 0; i < numberOfBlinks; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(flashDelay);
    digitalWrite(LED_PIN, LOW);
    delay((int)(flashDelay / 2));
  }
}

void mouseCommand(int buttons, int x, int y, int scroll) {
  Serial1.write(0xFD);
  Serial1.write(0x05);
  Serial1.write(0x02);
  Serial1.write(buttons); // 0x01 is left click 0x02 is right click on CPU. 0x01 is a tap and 0x02 is back on mobile device.
  Serial1.write(x);
  Serial1.write(y);
  Serial1.write(scroll);
  //delay(20);
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

void changeCursorSpeed(void) {

  if (speed_counter == 0) {
    cursor_delta -= 5;
    speed_counter++;
    statusLED(1, 500);
  } else if (speed_counter == 1) {
    cursor_delta -= 5;
    speed_counter++;
    statusLED(2, 500);
  } else {
    cursor_delta += 10;
    speed_counter = 0;
    statusLED(10, 50);
  }

}

void BT_CommandMode(void) {
  digitalWrite(TRANS_CONTROL, LOW);         //Transistor base low to ensure Bluetooth module is off
  digitalWrite(PIO4, HIGH);                 //Command pin high
  delay(1000);

  digitalWrite(TRANS_CONTROL, HIGH);        //Transistor base high to power on Bluetooth module
  delay(1000);

  for (int i = 0; i < 3; i++) {             //Cycle PIO4 pin high-low 3 times with 1 sec delay between each transition
    digitalWrite(PIO4, HIGH);
    delay(1000);
    digitalWrite(PIO4, LOW);
    delay(1000);
  }

  digitalWrite(PIO4, LOW);                  //Drive PIO4 pin low as per command mode instructions
  delay(500);
  Serial1.print("$$$");                     //Enter Bluetooth command mode
  //"$$$": CANNOT be Serial.println("$$$") ONLY Serial.print("$$$")
  delay(2000);
}

void BT_Configure(void) {                   //Change all Serial instructions to Serial1
  Serial1.println("ST,255");                 //Turn off the 60 sec timer for command mode
  delay(100);
  Serial1.println("SN,LipSyncBT_Autoload1"); //Change name of BT module
  delay(100);
  Serial1.println("SM,4");                   //Set DTR mode //Check for other modes to use
  delay(100);
  Serial1.println("SH,0220");                //Configure device as HID mouse
  delay(100);
  Serial1.println("S~,6");                   //Activate HID profile
  delay(100);
  Serial1.println("R,1");                    //Reboot BT module !!!REMOVED DURING TESTING PHASE!!!
  delay(100);
}

void BT_configAOK(void) {                    //FIX LATER FOR DIAGNOSTIC AND TESTING PURPOSES FOR TEAM AND MAKERS
  while (Serial1.available() > 0) {
    Serial.print((char)Serial1.read());
  }
}

void BT_FactoryReset(void) {                //RESET CONFIGURATION SETTINGS TO FACTORY SETTINGS

  BT_CommandMode();
  Serial1.println("SF,1");
  delay(100);
  Serial1.println("R,1");
  delay(1000);
}

void BT_ExitCommandMode(void) {             //EXIT COMMAND MODE
  Serial1.println("---");
  delay(1000);
}
