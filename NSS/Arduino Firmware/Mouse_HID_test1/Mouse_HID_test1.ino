void setup() {

  pinMode(13, OUTPUT);
  Serial1.begin(115200); //setting baud rate for BT module
  delay(500);
  powerUpSequence();
}

void loop() {
  digitalWrite(13,LOW);
  mouseCommand(0, -20, -20, 0);
  digitalWrite(13,HIGH);
  delay(500);
  Serial.println("Working");
  //mouseClear();
  delay(20);
  mouseCommand(0, 20, 20, 0);
  digitalWrite(13,LOW);
  delay(500);
  //mouseClear();
  delay(20);
}

void powerUpSequence(void) {
  for (int i = 0; i < 10; i++) {
    digitalWrite(13, LOW);
    delay(20);
    digitalWrite(13, HIGH);
    delay(20);
  }
}

void mouseCommand(int buttons, int x, int y, int scroll) {
  Serial1.write(0xFD);
  Serial1.write(0x05);
  Serial1.write(0x02);
  Serial1.write(buttons); // left click is 0x01 and right click is 0x02 I THINK?!?!
  Serial1.write(x);
  Serial1.write(y);
  Serial1.write(scroll);
  delay(50);
}

void mouseClear(void){
  
  int value = 0x00;
  
  Serial1.write(0xFD);
  Serial1.write(0x05);
  Serial1.write(0x02);
  Serial1.write(value);
  Serial1.write(value);
  Serial1.write(value);
  Serial1.write(value);
  delay(50);
}

