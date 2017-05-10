#include <Servo.h>

Servo servo1;
boolean keepGoing = true;
int PWMval;

void setup() {
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  servo1.attach(5);

}

void loop() {

  PWMval = 255;

  analogWrite(3,PWMval);

  while(keepGoing){
      for(int loopCounter = 0; loopCounter<5; loopCounter++){
        servo1.write(90);
        delay(2000);
        servo1.write(10);
        delay(2000);
        Serial.print("Cycle: ");
        Serial.println(loopCounter);
        PWMval = PWMval - 3;
        Serial.println(PWMval);
        analogWrite(3,PWMval); 
      }
    Serial.println("Cycles complete");
    break;
  }

  PWMval = 255;

keepGoing = false;
}
