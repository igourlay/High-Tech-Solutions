#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>

#include <SPI.h>


int x1, y1;

RF24 radio(9, 10);

const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL}; //these are the addresses on the transceiver
unsigned long Command = 1;

void setup() {
  Serial.begin(115200); //new baud rate

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  radio.begin();
  radio.setRetries(15,15);
  radio.openReadingPipe(1, pipes[1]);
  radio.startListening();
  radio.printDetails();
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  radio.stopListening();
}

void loop() {

  radio.stopListening();

  //x1 = analogRead(A0);
  //y1 = analogRead(A1);

  if(radio.write(&Command, sizeof(Command))){
    Serial.println("Data sent.");
  }

  radio.startListening();

  delay(1000);

  //Serial.println(x1);
  //Serial.print("\t");
  //Serial.println(y1);

  }
