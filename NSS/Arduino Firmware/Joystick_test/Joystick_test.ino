int x1,y1, x_start, x_high, x_low;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(A0, INPUT);
  //pinMode(A1, INPUT);

  new_config();
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:

  x1 = analogRead(A0);
  //y1 = analogRead(A1);
  
  //Serial.println(x1);
  //Serial.print("\t");
  //Serial.println(y1);

  if(x1 > x_high){
    Serial.println("Moving up");
    delay(100);
  }else if(x1 < x_low){
    Serial.println("Moving down");
    delay(100);
  }
  
}

void new_config(void){
  x_start = analogRead(A0);
  delay(50);
  x_high = x_start + 5;
  delay(50);
  x_low = x_start - 5;
  delay(50);
}

