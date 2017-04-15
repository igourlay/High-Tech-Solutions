float pressure, voltage;

void setup() {
  Serial.begin(9600);

  pinMode(A5, INPUT);

}

void loop() {

  pressure = analogRead(A5);

  voltage = (pressure/1024.0)*5.0;

  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.print("   ");
  Serial.print("\t");
  Serial.print("Voltage: ");
  Serial.println(voltage);
  delay(50);

}
