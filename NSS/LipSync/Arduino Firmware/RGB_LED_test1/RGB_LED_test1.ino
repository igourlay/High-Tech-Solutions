#define RED 11
#define GREEN 9
#define BLUE 10

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, HIGH);
  digitalWrite(BLUE, HIGH);

}

int redVal;
int blueVal;
int greenVal;

void loop() {
  /*redVal = 255;
  blueVal = 0;
  greenVal = 0;
  for(int i = 0; i < 255; i += 1){
    greenVal += 1;
    redVal -= 1;
    analogWrite(GREEN, 255 - greenVal);
    analogWrite(RED, 255 - redVal);
    delay(50);
  }

  redVal = 0;
  blueVal = 0;
  greenVal = 255;
  for(int i = 0; i < 255; i+= 1){
    blueVal =+ 1;
    greenVal -= 1;
    analogWrite(BLUE, 255 - blueVal);
    analogWrite(GREEN, 255 - greenVal);
    delay(50);
  }

  redVal = 0;
  blueVal = 255;
  greenVal = 0;
  for(int i = 0; i < 255; i += 1){
    redVal += 1;
    blueVal -= 1;
    analogWrite(RED, 255 - redVal);
    analogWrite(BLUE, 255 - blueVal);
    delay(50);
  }
  */
  for (int i = 0; i < 10; i++) {
    analogWrite(BLUE, 0);
    delay(150);
    analogWrite(BLUE, 255);
    delay(200);
  }

  for (int i = 0; i < 10; i++) {
    analogWrite(GREEN, 0);
    delay(150);
    analogWrite(GREEN, 255);
    delay(200);
  }

  for (int i = 0; i < 10; i++) {
    analogWrite(RED, 0);
    delay(150);
    analogWrite(RED, 255);
    delay(200);
  }

}
