float pressure, voltage;

void setup() {
  Serial.begin(9600);

  pinMode(A0, INPUT);

}

void loop() {

  pressure = analogRead(A0);

  voltage = (pressure/1024.0)*3.3;

  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.print("   ");
  Serial.print("\t");
  Serial.print("Voltage: ");
  Serial.println(voltage);

}
