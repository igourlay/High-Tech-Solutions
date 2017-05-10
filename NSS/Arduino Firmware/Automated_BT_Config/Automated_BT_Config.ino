//AUTHOR: Ivan Gourlay 6 July 2016

/*
 * REVISION HISTORY:
 * 08 July 2016
 * 11 July 2016
 */

#define TRANS_CONTROL 12                    //Transistor pin
#define PIO4 11                             //PIO4 pin
#define ob_LED 13

int tick, took = 0;                         //Testing variables
String done = "";                           //Testing string

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
  //Serial1.println("R,1");                    //Reboot BT module !!!REMOVED DURING TESTING PHASE!!!
  //delay(100);
}

void BT_configAOK(void) {                    //FIX LATER FOR DIAGNOSTIC AND TESTING PURPOSES FOR TEAM AND MAKERS
    while(Serial1.available() > 0) {
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

void setup() {

  Serial.begin(115200);
  Serial1.begin(115200);                    //Set baud rate for TX/RX port Serial1 to Bluetooth
  pinMode(TRANS_CONTROL, OUTPUT);           //Transistor pin :: output
  pinMode(PIO4, OUTPUT);                    //Command mode pin :: output
  pinMode(ob_LED, OUTPUT);                  //On-board LED

  BT_CommandMode();
  BT_Configure();
  BT_configAOK();
  
  Serial.println("");
  Serial.println("Configuration complete.");
  Serial.println("To exit command mode press X");
}

void loop() { //THE REMAINING INSTRUCTIONS ARE JUST TO OUTPUT THE BT MODULE SETTINGS TO MAKE SURE THAT EVERYTHING IS CORRECT

  if (Serial1.available()) { // If the bluetooth sent any characters
    // Send any characters the bluetooth prints to the serial monitor
    Serial.print((char)Serial1.read());
  }
  if (Serial.available()) { // If stuff was typed in the serial monitor
    // Send any characters the Serial monitor prints to the bluetooth
    char check = toUpperCase((char)Serial.read());

    if (check == 'X') {
      BT_ExitCommandMode();
    } else {
      Serial1.print(check);
    }
  }


}

