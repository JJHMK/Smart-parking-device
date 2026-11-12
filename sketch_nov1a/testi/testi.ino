#include <Servo.h>
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

Servo myservo;


// ultraääni sensori
int echoPin = 7;
int trikPin = 8;
int ledPin = 13;
int maxRange = 200;
int minRange = 0;



void setup() {

  // Ledit
  pinMode(rLed, OUTPUT);
  pinMode(gLed, OUTPUT);

  // Serial
  Serial.begin(9600);
  Serial.println("LCD Character Backpack IC Test");

  // Servo
  myservo.attach(9);
  myservo.write(90);

  // Ultraääni sensori setup
  pinMode(trigPin, OUTPU)
  pinMode(echoPin, INPUT)





  if (!lcd.begin(16, 2)) {
    Serial.println("Could not init backpack. Check wiring.");
    while (1)
      ;
  }
}

void loop() {




  //Set the cursor to column 0 line 1
  lcd.setCursor(0, 1);

  lcd.setBacklight(HIGH);




/* The following trigPin/echoPin cycle is used to determine the
 distance of the nearest object by bouncing soundwaves off of it. */ 

 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 
 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);

 /*/Calculate the distance (in cm) based on the speed of sound.
 distance = duration/58.2;
  Serial.print(cm);
  Serial.println(" cm");
  pinMode(pingPin, OUTPUT);
  delay(200); 
*/

  if (cm < 50) {
    digitalWrite(rLed, HIGH);
    digitalWrite(gLed, LOW);
    if (!taynna) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("taynna");
      taynna = true;
    }

  } else {
    digitalWrite(gLed, HIGH);
    digitalWrite(rLed, LOW);
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("paikkoja vapaana");
    taynna = false;
  }

  if (cm < 70) {
    myservo.write(0);
  } else {
    myservo.write(90);
  }
}
