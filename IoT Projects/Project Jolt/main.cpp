#include "brewFunc.h"

int startBrewingFlag = D0;
int checkWaterInput = D1;
int checkCoffeeInput = D2;
int checkCarafeInput = D3;

//ACCESS
String id;
String token;

//VARIABLES
bool waterLevel = false;
bool coffeeLevel = false;
bool coffeeCarafe = false;

int i = 0;

class CoffeeMaker{
	public:
		CoffeeMaker(){
			Particle.function("brew", &CoffeeMaker::brew, this);
		}

		bool checkWater(){
			if(D1 == HIGH){
				return true;
			}else{
				return false;
			}
		}

		bool checkCoffee(){
			if(D2 == HIGH){
				return true;
			}else{
				return false;
			}
		}

		bool checkCarafe(void){
			if(D3 == HIGH){
				return true;
			}else{
				return false;
			}
		}

		int startBrew(){
	
    bool waterPresent = checkWater();
    bool coffeePresent = checkCoffee();
    bool carafePresent = checkCarafe();
    
    if (waterPresent == true){
        if (coffeePresent == true){
			if(carafePresent == true){
                digitalWrite(startBrewing, HIGH);
				Particle.publish("brewStatus", "started", 60, PRIVATE);
				Serial.println("Coffee is brewing now...");
                return 1;
            }else{
				Particle.publish("brewStatus", "noCarafe", 60, PRIVATE);
				Serial.println("Carafe is not present...");
                return 0;
            }           
        }else{
			Particle.publish("brewStatus", "noCoffee", 60, PRIVATE);
			Serial.println("Coffee is not present...");
            return 0;
        }
	}else{
		Particle.publish("brewStatus", "noWater", 60, PRIVATE);
		Serial.println("Water is not present...");
        return 0;
    }
}



void myHandler(const char *event, const char *data){
    i++;
    Serial.print(i);
    Serial.print(event);
    Serial.print(", data: ");
    if(data){
        Serial.println(data);
    }else{
        Serial.println("NULL");
    }
}

void setup() {

Serial.begin(9600);

pinMode(startBrewing, OUTPUT);
pinMode(checkWaterInput, INPUT);
pinMode(checkCoffeeInput, INPUT);
pinMode(checkCarafeInput, INPUT);

digitalWrite(startBrewing, LOW);

}

void loop() 
{
	
	Particle.function("brew", startBrew);
	
}    
    //listen for request for coffee to be brewed
    //find API to listen for data
    
