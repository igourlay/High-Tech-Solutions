// AUTHOR: IVAN GOURLAY June 10, 2016

#include <SPI.h>
#include "RF24.h"

RF24 radio(7,8);

byte addresses[][6] = {"1Node", "2Node"};

float voltage;
int x_coord, y_coord, confirmation;

int TRUE = 1;

void setup() {
  Serial.begin(115200);
  Serial.println(F("HID communcations to begin..."));

  confirmation = 0000;

  radio.begin();
  
  radio.setPALevel(RF24_PA_LOW);

  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);

  radio.startListening();

  // SETUP HID STUFF

}

void loop() {

  Serial.println("Begin receiving movement and pressure data");

  while(TRUE) {

      if(radio.available()){

        while(radio.available()){
          radio.read(&voltage, sizeof(float));
          delay(20);
          radio.read(&x_coord, sizeof(int));
          delay(20);
          radio.read(&y_coord, sizeof(int));
          
          confirmation = 1111;        // transmission confirmation
        }

        radio.stopListening();
        
        radio.write(&confirmation, sizeof(int));
        delay(35);
        
        radio.startListening();
        
        confirmation = 0000;

        Serial.print("Voltage: ");
        Serial.print(voltage);
        Serial.print("\t");
        Serial.print("X-coordinate: ");
        Serial.print(x_coord);
        Serial.print("   ");
        Serial.print("Y-coordinate: ");
        Serial.println(y_coord);
        
        delay(50);
      }
  
  }
}
