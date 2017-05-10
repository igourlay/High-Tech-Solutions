void setup() {
  
Mouse.begin();
}

void loop() {
while(HIGH){
  Mouse.move(20,20,0);
  delay(1000);  
  Mouse.move(-20,-20,0);
  delay(2000);
}
}
