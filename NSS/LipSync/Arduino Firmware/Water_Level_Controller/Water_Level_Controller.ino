#define trigPin 8 //trigger pin
#define echoPin 5 //echo pin
#define pumpController 3 //motor transistor pin

long duration, distance;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pumpController, OUTPUT);
}

void loop() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);  

  distance = duration/58.2;

  Serial.println(distance);

  delay(20);

  digitalWrite(13,HIGH);

  if(distance > 15.0)
  {
    digitalWrite(13, HIGH);
    analogWrite(3,255);
    delay(25000);
  }
  else
  {
    digitalWrite(13, LOW);
    analogWrite(3,0);
    delay(3000);
  }

}
