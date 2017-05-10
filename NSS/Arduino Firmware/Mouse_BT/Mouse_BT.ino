
int led = 13;

void setup() {                
  pinMode(led, OUTPUT); 
  digitalWrite(led, HIGH);
  Serial.begin(115200);
  delay(500);
  digitalWrite(led, LOW);
}

void loop() {
    digitalWrite(led, LOW);
    mouseDataSend(0,10,0,0);
    delay(500);    
    mouseDataClear();
}
   
void mouseDataSend(int button,int x,int y,int scroll)
  {
  int value=0x00;
  Serial.write(0xFD);
  Serial.write(0x05);
  Serial.write(0x02);
  Serial.write(button);
  Serial.write(x);
  Serial.write(y);
  Serial.write(scroll);
  delay(20);
  }
  
void mouseDataClear(void)
  {
  int value=0x00;
  Serial.write(0xFD);
  Serial.write(0x05);
  Serial.write(0x02);
  Serial.write(value);
  Serial.write(value);
  Serial.write(value);
  Serial.write(value);
  delay(20);
  }

