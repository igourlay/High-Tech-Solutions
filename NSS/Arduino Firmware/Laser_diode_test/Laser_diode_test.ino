  /*
   * Keyes laser diode board
   * S : signal pin (ie. pin 9)
   * Middle pin : unconnected
   * - : GND
   */

void setup() {
  // put your setup code here, to run once:
  
  pinMode(9, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(9, HIGH);

  delay(100);

  digitalWrite(9, LOW);

  delay(100);
}
