#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// Missing 10k 1 turn trim pot Digikey P/N: D4AA14-ND replaced by
// TSR-3386 10k trim pot

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define OFF 0x0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7



void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600); //debugging output
  
  lcd.begin(16,2); // set up the LCD's number of columns and rows

  /*
   * Print a message to the LCD. We track how long it takes since
   * this library has been optimized
   */

   int time = millis();
   lcd.print("Hello, Aarun!");
   time = millis() - time;
   Serial.print("Took: ");
   Serial.print(time);
   Serial.println(" ms");
   lcd.setBacklight(WHITE);
}

void loop() {
  // put your main code here, to run repeatedly:

  lcd.setCursor(0,1);

  lcd.print(millis()/1000);
  
  uint8_t buttons = lcd.readButtons();

  if (buttons) {
    lcd.clear();
    lcd.setCursor(0,0);
    
    if (buttons & BUTTON_UP) {
      lcd.print("Mathematics");
    }
    
    if (buttons & BUTTON_DOWN) {
      lcd.print("Physics");
    }

  }
}
