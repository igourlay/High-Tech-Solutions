// AUTHOR: IVAN GOURLAY June 10, 2016

#include <SPI.h>
#include "RF24.h"

RF24 radio(7, 8);

byte addresses[][6] = {"1Node", "2Node"};

float pressure, voltage;
int x_coord, y_coord, confirmation;
int keepGoing;

int TRUE = 1;
int FALSE = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("HID communications to begin..."));

  pinMode(A0, INPUT); // pressure transducer
  pinMode(A1, INPUT); // x-coordinate
  pinMode(A2, INPUT); // y-coordinate

  keepGoing = TRUE;

  radio.begin();

  radio.setPALevel(RF24_PA_LOW);

  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  radio.startListening();
}

void loop() {
  Serial.println("Begin sending movement and pressure data");
  
  while (keepGoing) {
    radio.stopListening();

    pressure = analogRead(A0);
    voltage = (pressure / 1024.0) * 5.0;

    x_coord = analogRead(A1);
    y_coord = analogRead(A2);

    if(voltage > 2.64){                   // turning off transmission NOTE: actual delay in data transmission but in reality
      delay(2000);                        // this may be seen as the device simply just turning off
      pressure = analogRead(A0);
      voltage = (pressure / 1024.0) * 5.0;
        if(voltage > 2.64){
          keepGoing = FALSE; 
        }
    }
    
    radio.write(&voltage, sizeof(float));

    radio.write(&x_coord, sizeof(int));

    radio.write(&y_coord, sizeof(int));

    radio.startListening();

    if(radio.available()){      
      radio.read(&confirmation, sizeof(unsigned long));

      Serial.print("Confirmation: ");
      Serial.println(confirmation);
      
    }

    delay(100);

  }

  Serial.println("Terminate sending movement and pressure data");
  delay(2000);
  exit(0);
}
