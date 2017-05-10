#define push_button_up 8
#define push_button_down 7
#define LED_up 4
#define LED_down 3
#define LED_PIN 2

int push_button_up_status, push_button_down_status;
int cursor_delta = 20;
unsigned long speed_counter;

void setup() {

  pinMode(push_button_up, INPUT_PULLUP);
  pinMode(push_button_down, INPUT_PULLUP);
  pinMode(LED_up, OUTPUT);
  pinMode(LED_down, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(9600);
  digitalWrite(LED_PIN, LOW);
}

void loop() {

  if(digitalRead(push_button_up) == LOW){ // && push_button_up_status == 0){
    //push_button_up_status = 1;
    //Serial.print("push button up status is: ");
    //Serial.println(push_button_up_status);
    //digitalWrite(LED_up, HIGH);
    increase_cursor_speed();
    delay(250);
  }//else{
   // push_button_up_status = 0;
  //}

  if(digitalRead(push_button_down) == LOW){ // && push_button_down_status == 0){
    //push_button_down_status = 1;
    //Serial.print("push button down status is: ");
    //Serial.println(push_button_down_status);
    //digitalWrite(LED_down, HIGH);
    decrease_cursor_speed();
    delay(250);
  }//else{
   // push_button_down_status = 0;
  //}

}

void blink(int num_Blinks, int delay_Blinks ){
  if(num_Blinks < 0) num_Blinks *= -1;
  
  for(int i = 0; i < num_Blinks; i++){
    digitalWrite(LED_PIN, HIGH);
    delay(delay_Blinks);
    digitalWrite(LED_PIN, LOW);
    delay(delay_Blinks);
  }
}

void increase_cursor_speed(void){
  speed_counter++;
  if(speed_counter == 0){
    blink(10,100);      // single slow blink
    cursor_delta = 20;  // redundant
    speed_counter = 0;  // redundant
  }else if(speed_counter == 4){
    blink(5,100);       // six fast blinks
    cursor_delta = 5;
    speed_counter = 3;
  }else{
    blink(speed_counter, 300);
    cursor_delta -= 5;
  }
  Serial.print("Cursor delta is: ");
  Serial.println(cursor_delta);
}

void decrease_cursor_speed(void){
  speed_counter--;
  if(speed_counter == 0){
    blink(10,100);      // one slow blink
    cursor_delta = 20;  // redundant
    speed_counter = 0;  // redundant
  }else if(speed_counter == -4){
    blink(5,100);       // six fast blinks
    cursor_delta = 35;
    speed_counter = -3;
  }else{
    blink(speed_counter, 300);
    cursor_delta += 5;
  }
  Serial.print("Cursor delta is: ");
  Serial.println(cursor_delta);
}
