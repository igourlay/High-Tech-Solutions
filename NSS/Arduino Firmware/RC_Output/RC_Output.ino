#define Signal 11

void setup() {
  pinMode(Signal, OUTPUT); 
}

void loop() {
tone(Signal,500);

}
