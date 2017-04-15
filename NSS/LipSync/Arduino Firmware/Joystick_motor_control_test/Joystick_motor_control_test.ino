int x1, y1;

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  x1 = analogRead(A0);
  y1 = analogRead(A1);

  Serial.print(x1);
  Serial.print("\t");
  Serial.println(y1);
  
  if(x1>350)
  {
    digitalWrite(0, HIGH);
    digitalWrite(1, LOW);
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
    //delay(5000);
  }

  if(x1<100)
  {    
    digitalWrite(0, LOW);
    digitalWrite(1, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    //delay(5000);
  }

  if(x1>100 && x1<350)
  {
    digitalWrite(0, LOW);
    digitalWrite(1, LOW);
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
  }
  
}
