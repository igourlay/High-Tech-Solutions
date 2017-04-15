
int ledPin = 13;
int configPin = 10;  //config pin
int modeVal = 0;      // low is operating mode and high is config mode

void blink3Times() {
      for (int i=0; i<=3; i++) {
        digitalWrite(ledPin, HIGH);
        delay(1000);
        digitalWrite(ledPin, LOW);
        delay(1000);      
    }
}

void configBT(void) {
    digitalWrite(ledPin, HIGH);
    Serial.println("ST,255");         //Turn off 60 seconds timer
    delay(100); 
    Serial.println("SN,LipSyncBT");   //Change name to LipSyncBT
    delay(100);
    Serial.println("S-,LipSyncBT");   //Change Serialized name to LipSyncBT
    delay(100);
    Serial.println("SM,4");           // DTR mode
    delay(100);
    Serial.println("SH,0220");        //Configured device as mouse
    delay(100);
    Serial.println("S~,6");          //Active HID profile
    delay(100);
    Serial.println("R,1");            //Reboot
}



void setup() {                
  pinMode(ledPin, OUTPUT);
  pinMode(configPin, INPUT); 
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);
  delay(1000);
  Serial.print("$$$");
  delay(200);
  modeVal = digitalRead(configPin); 
  if (modeVal == HIGH) {             //if pin 10 is high in power on then go to config mode
    blink3Times();                   //blink led 3 times to indicate it is starting config mode
    configBT();                      //call the config function and make led stay high
    blink3Times();                  //blink led 3 times to indicate it has finished the process in config mode
    modeVal=LOW;
  }
  else{
    digitalWrite(ledPin, LOW);       //if pin 10 is low in power on then operate lipsync 
    Serial.println("C");    
  }
}




void loop() {
    digitalWrite(ledPin, LOW);
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

