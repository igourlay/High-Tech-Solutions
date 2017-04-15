float x, y, x_voltage, y_voltage;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:

  y = analogRead(A0);
  x = analogRead(A1);
  
  x_voltage = (x/1024.0)*5.0;
  y_voltage = (y/1024.0)*5.0;

  Serial.print("X voltage: ");
  Serial.print(x_voltage);
  Serial.print("\t");
  Serial.print("Y voltage: ");
  Serial.println(y_voltage);

}
