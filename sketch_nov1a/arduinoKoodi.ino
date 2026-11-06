#include <Adafruit_LiquidCrystal.h>

// C++ code
//
Adafruit_LiquidCrystal lcd(0);
int rLed = 7;
int gLed = 8;
int pingPin = 11;
int cm = 0;
int duration = 0;
bool taynna = false;


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rLed, OUTPUT);
  pinMode (gLed, OUTPUT);
  Serial.begin(9600);
  pinMode(pingPin, OUTPUT);
  Serial.println("LCD Character Backpack IC Test");
  
  
    if (!lcd.begin(16,2)) {
  Serial.println("Could not init backpack. Check wiring.");
    while(1);
  }
  
}

void loop()
{
  //Set the cursor to column 0 line 1
  lcd.setCursor(0,1);
  
  lcd.setBacklight(HIGH);


  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);
  cm = duration / 29 / 2; // Muunnos senttimetreiksi
  Serial.print(cm);
  Serial.println(" cm");
  pinMode(pingPin, OUTPUT);
  delay(200);

    if (cm < 50){
    digitalWrite(rLed, HIGH);
    digitalWrite(gLed, LOW);
      if (!taynna){
            lcd.clear();
      lcd.setCursor(0,0); 
      lcd.print("taynna");
      taynna = true;
      }  

    }
    else{
      digitalWrite(gLed, HIGH);
      digitalWrite(rLed, LOW);
      //lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("paikkoja vapaana");
      taynna = false;
    }

}