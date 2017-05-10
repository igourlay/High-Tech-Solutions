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
 * 27 June 2016
 * 08 July 2016
 * 11 July 2016
 * 15 July 2016
 * 18 July 2016
 * 29 July 2016
 * 02 Aug 2016
 * 03 Aug 2016
 * 09 Aug 2016
 * 10 Aug 2016
 * 16 Aug 2016
 * 26 Aug 2016
 */

#include <EEPROM.h>

//***DEFINITIONS AND VARIABLE DECLARATIONS***//

#define TRANS_CONTROL A3                          // transistor pin - digital pin 12
#define PIO4 A4                                   // PIO4 pin - digital pin 11

int config_done = 0;
int i = 0u;
int single = 0;

//***MICROCONTROLLER AND PERIPHERAL MODULES CONFIGURATIONS***//

void setup() {

  pinMode(TRANS_CONTROL, OUTPUT);                 // transistor pin output
  pinMode(PIO4, OUTPUT);                          // command mode pin output

  Serial.begin(115200);                           // setting baud rate for serial coms for diagnostic data return from Bluetooth and microcontroller ***MAY REMOVE LATER***
  Serial1.begin(115200);                          // setting baud rate for Bluetooth module
  delay(200);

  //*** BEGIN TESTING SEQUENCE ***//

  config_done = config_status();
  //config_done = 0;
  delay(100);

  if (config_done == 0) {
    BT_CommandMode();                               // enter Bluetooth command mode
    BT_Configure();                                 // send configuarion data to Bluetooth module
    BT_configAOK();                                 // returns diagnostic responses from Bluetooth

  } else {
    Serial.println("Bluetooth configuration previously completed.");
    //BT_CommandMode();
    //BT_Connect();
  }

  //*** END TESTING SEQUENCE ***//
}

//----------------------------------------------------------------------------------------------------------//

//****** START OF LOOP ******//

void loop() {

  while (Serial1.available() > 0) {
    Serial.print((char)Serial1.read());
  }

  while (Serial.available() > 0) {
    Serial1.println((char)Serial.read());
  }

  if (single == 0) {
    int var = 0;
    EEPROM.put(0, var);
    delay(100);
    single++;
  }
}
//****** END OF LOOP ******//

//-----------------------------------------------------------------------------------------------------------//

//***BLUETOOTH CONFIGURATION AND STATUS FUNCTIONS***//

int config_status(void) {
  EEPROM.get(0, i);
  //Serial.println(i);
  delay(500);
  return i;
}

void BT_Connect(void) {
  // MUST BE IN COMMAND MODE
  Serial1.println("C");
  delay(500);
}

void BT_CommandMode(void) {                 //*** TRANSISTOR CONTROL CHANGED TO PNP TYPE TRANSISTOR ***//
  digitalWrite(TRANS_CONTROL, HIGH);        // transistor base pin HIGH to ensure Bluetooth module is off
  digitalWrite(PIO4, HIGH);                 // command pin high
  delay(500);

  digitalWrite(TRANS_CONTROL, LOW);         // transistor base pin LOW (PNP type transistor) to power on Bluetooth module
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
  delay(1000);                              // time delay to visual inspect the red LED is flashing at 10Hz which indicates the Bluetooth module is in Command Mode
}

void BT_Configure(void) {                   // change all Serial instructions to Serial1
  Serial1.println("ST,255");                // turn off the 60 sec timer for command mode
  delay(100);
  Serial1.println("SA,1");                  // ***NEW ADDITION - Authentication Values 0: no authentication (security risk?)
  delay(100);
  Serial1.println("SX,1");                  // ***NEW ADDITION - Bonding enabled: accepts connections to paired devices???
  delay(100);
  Serial1.println("SN,LipSyncBT_Autoload1");// change name of BT module
  delay(100);
  Serial1.println("SM,6");                  // ***NEW ADDITION - "SM,6" from DTR mode "SM,4": this should allow for auto-connect //Works when the LipSync and phone are initially paired up together, the phone BT turns off, phone BT turns back on, then the LipSync will
  delay(100);                               // auto-connect back to the phone. Still trying to figure out how to enable auto-connect if LipSync and phone are both turned off then back on.
  Serial1.println("SH,0220");               // configure device as HID mouse
  delay(100);
  Serial1.println("S~,6");                  // activate HID profile
  delay(100);

  //Serial1.println("SW,<hex value>);         // sniff mode conserves power by polling the radio ***POWER CONSERVATION
  //delay(100);
  //Serial1.println("SY,<hex value>);       // set transmist power settings ***POWER CONSERVATION

  Serial1.println("R,1");                   // reboot BT module **Maybe you don't need to reboot the BT perhaps it wipes the addresses??
  delay(100);

  //*** SPECIAL SECTION ***//
  int val = 1;
  EEPROM.put(0, val);                         // use the EEPROM.get and EEPROM.put CORRECTLY
  delay(100);

}

void BT_configAOK(void) {                    // diagnostic feedback from Bluetooth
  while (Serial1.available() > 0) {
    Serial.print((char)Serial1.read());
  }
  Serial.println("");
  Serial.println("Initial Bluetooth configuration complete.");
}
