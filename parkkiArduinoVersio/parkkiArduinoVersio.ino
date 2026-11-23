#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <Ultrasonic.h>
#include <SPI.h>
#include <Wire.h>
#include <SeeedOLED.h>


// =========================
// LEDit
// =========================

// Paikka 1
const int spot1RedLed = 6; // Tarkista vielä pinnit kaikista
const int spot1GreenLed = 7;

// Paikka 2
const int spot2RedLed = 8;
const int spot2GreenLed = 9;



// =========================
// WIFI
// =========================

const char* ssid = "HUAWEI P30 lite";
const char* password = "19863cpn89";

// =========================
// MQTT
// =========================

const char* mqtt_server = "10.60.207.207";
const int mqtt_port = 1883;

const char* mqtt_user = "iot_user";
const char* mqtt_pass = "19863cpn89";

const char* sensorTopic = "parking/sensor";
const char* displayTopic = "parking/display";

const char* gateControlTopic = "parking/gate/control";

// =========================
// Grove-ultrasonic Portti
// =========================

Ultrasonic ultrasonic(3);

// =========================
// Grove-ultrasonic Ranger Paikka1
// =========================

Ultrasonic ultrasonic2(4);

// =========================
 //Grove-ultrasonic Ranger Paikka2
// =========================

Ultrasonic ultrasonic3(5);


/*
// =========================
// HC-SR04 Anturi 2
// =========================

const int trigPin2 = 18;
const int echoPin2 = 19;

// =========================
// HC-SR04 Anturi 3
// =========================

const int trigPin3 = 21;
const int echoPin3 = 22;
*/

// =========================
// LED-ohjaus
// =========================
void updateLEDs(bool spot1Occupied, bool spot2Occupied)
{
    // Paikka 1
    if (spot1Occupied)
    {
        digitalWrite(spot1RedLed, HIGH);
        digitalWrite(spot1GreenLed, LOW);
    }
    else
    {
        digitalWrite(spot1RedLed, LOW);
        digitalWrite(spot1GreenLed, HIGH);
    }

    // Paikka 2
    if (spot2Occupied)
    {
        digitalWrite(spot2RedLed, HIGH);
        digitalWrite(spot2GreenLed, LOW);
    }
    else
    {
        digitalWrite(spot2RedLed, LOW);
        digitalWrite(spot2GreenLed, HIGH);
    }
}


//==========================
// Näyttö
//==========================
void updateDisplay(bool spot1Occupied, bool spot2Occupied)
{
    int freePlaces = 0;

    if (!spot1Occupied)
        freePlaces++;

    if (!spot2Occupied)
        freePlaces++;

    SeeedOled.clearDisplay();

    SeeedOled.setTextXY(0,0);
    SeeedOled.putString("SMART PARKING");

    if (freePlaces == 2)
    {
        SeeedOled.setTextXY(3,0);
        SeeedOled.putString("2 PAIKKAA");

        SeeedOled.setTextXY(5,0);
        SeeedOled.putString("VAPAANA");
    }
    else if (freePlaces == 1)
    {
        SeeedOled.setTextXY(3,0);
        SeeedOled.putString("1 PAIKKA");

        SeeedOled.setTextXY(5,0);
        SeeedOled.putString("VAPAANA");
    }
    else
    {
        SeeedOled.setTextXY(3,0);
        SeeedOled.putString("TAYNNA");
    }
}


// =========================
// Servo
//==========================

Servo gateServo;


//==========================
//Auton tunnistus portti
//==========================
bool carStillPresent()
{
    int hits = 0;

    for (int i = 0; i < 10; i++)
    {
        long d = ultrasonic.MeasureInCentimeters();

        if (d > 0 && d < 50)
        {
            hits++;
        }

        delay(50);
    }

    return hits >= 7;
}



//==========================
//==========================
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSend = 0;

// =========================
// WIFI
// =========================

void connectWiFi()
{
    Serial.print("Yhdistetaan WiFiin ");

    while (WiFi.begin(ssid, password) != WL_CONNECTED)
    {
        Serial.print(".");
        delay(5000);
    }

    Serial.println();
    Serial.println("WiFi yhdistetty");

    IPAddress ip = WiFi.localIP();

    Serial.print("Arduino IP: ");
    Serial.println(ip);
}


// =========================
// MQTT Callback
// =========================

void callback(char* topic, byte* payload, unsigned int length)
{
    String message;

    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.println();
    Serial.println("========== MQTT ==========");
    Serial.print("Topic: ");
    Serial.println(topic);

    Serial.print("Message: ");
    Serial.println(message);

    if (String(topic) == gateControlTopic)
    {
        DynamicJsonDocument doc(128);

        DeserializationError error =
            deserializeJson(doc, message);

        if (!error)
        {
            String command = doc["command"];

            if (command == "open")
            {
                Serial.println("PORTTI AUKI");

                gateServo.write(90);

                while (ultrasonic.MeasureInCentimeters() > 50)
                {
                    delay(100);
                }

                Serial.println("Auto havaittu");

                while (ultrasonic.MeasureInCentimeters() < 50)
                {
                    delay(100);
                }

                Serial.println("Auto meni sisaan");

                delay(1000);

                gateServo.write(0);

                Serial.println("PORTTI KIINNI");
            }
        }
    }

    Serial.println("==========================");
}
// =========================
// MQTT Reconnect
// =========================

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Yhdistetään MQTT brokeriin...");

        if (client.connect("ESP32_PARKING", mqtt_user, mqtt_pass))
        {
            Serial.println(" OK");

            client.subscribe(displayTopic);
            client.subscribe(gateControlTopic);

            Serial.print("Tilattu topic: ");
            Serial.println(displayTopic);

            Serial.println("Tilattu topic");
            Serial.println(gateControlTopic);
        }
        else
        {
            Serial.print("Virhe: ");
            Serial.println(client.state());

            delay(5000);
        }
    }
}


// =========================
// HC-SR04 Mittaus
// =========================
/*
long readDistance(int trigPin, int echoPin)
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);

    if (duration == 0)
    {
        return 999;
    }

    long distance = duration * 0.034 / 2;

    return distance;
}
*/
// =========================
// Setup
// =========================

void setup()
{
    Serial.begin(9600);
    delay(2000);

    Serial.println("START");
    /*
    //pinMode(trigPin1, OUTPUT);
    //pinMode(echoPin1, INPUT);

    pinMode(trigPin2, OUTPUT);
    pinMode(echoPin2, INPUT);

    pinMode(trigPin3, OUTPUT);
    pinMode(echoPin3, INPUT);
    */
    connectWiFi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    gateServo.attach(2);
    gateServo.write(0); // Portti kiinni aluksi

//====================================
// Näyttö=============================
//====================================




    Wire.begin();
    SeeedOled.init();  //initialze SEEED OLED display

    SeeedOled.clearDisplay();          //clear the screen and set start position to top left corner
    SeeedOled.setNormalDisplay();      //Set display to normal mode (i.e non-inverse mode)
    SeeedOled.setPageMode();           //Set addressing mode to Page Mode
    SeeedOled.setTextXY(0,0);          //Set the cursor to Xth Page, Yth Column  
    SeeedOled.putString("Hello World!"); //Print the String
}

//==================LED===========
pinMode(spot1RedLed, OUTPUT);
pinMode(spot1GreenLed, OUTPUT);

pinMode(spot2RedLed, OUTPUT);
pinMode(spot2GreenLed, OUTPUT);

// =========================
// Loop
// =========================

void loop()
{

    if (!client.connected())
    {
        reconnect();
    }

    client.loop();


    if (millis() - lastSend > 2000)
    {
        lastSend = millis();
        // Portin anturi


        // Parkkipaikka 1
        long distance1 = ultrasonic2.MeasureInCentimeters();

        // Parkkipaikka 2
        long distance2 = ultrasonic3.MeasureInCentimeters();

    
        // HC-SR4 Anturit
        //long distance2 = readDistance(trigPin2, echoPin2);
        //delay(50);
        //long distance3 = readDistance(trigPin3, echoPin3);

        //=========Grove Ultrasound


        bool spot1Occupied = (distance1 < 50);
        bool spot2Occupied = (distance2 < 50);


        DynamicJsonDocument doc(256);

        char buffer[256];

        // ===================
        // SPOT 1
        // ===================

        doc.clear();
        doc["sensor_id"] = "spot_1";
        doc["distance_cm"] = distance1;

        serializeJson(doc, buffer);
        client.publish(sensorTopic, buffer);
        
        // ===================
        // SPOT 2
        // ===================

        doc.clear();
        doc["sensor_id"] = "spot_2";
        doc["distance_cm"] = distance2;

        serializeJson(doc, buffer);
        client.publish(sensorTopic, buffer);


        long gateDistance = ultrasonic.MeasureInCentimeters();
        
        doc.clear();
        doc["distance_cm"] = gateDistance;

        serializeJson(doc, buffer);

        client.publish("parking/gate/sensor",buffer);


        
        // ===================
        // SPOT 3
        // ===================
        /*
        doc.clear();
        doc["sensor_id"] = "spot_3";
        doc["distance_cm"] = distance3;
        */

        /*
        serializeJson(doc, buffer);
        client.publish(sensorTopic, buffer);
        */
        // ===================
        // SERIAL MONITOR
        // ===================

        Serial.println();
        Serial.println("===== PARKING SENSOR DATA =====");

        Serial.print("Spot 1: ");
        Serial.print(distance1);
        Serial.println(" cm");

        Serial.print("Spot 2: ");
        Serial.print(distance2);
        Serial.println(" cm");
        /*
        Serial.print("Spot 3: ");
        Serial.print(distance3);
        Serial.println(" cm");
        */
        Serial.println("===============================");
        Serial.print("Portti: ");
        Serial.print(gateDistance);
        Serial.println(" cm");


        //==========================
        //Näyttö
        //==========================
        updateDisplay(spot1Occupied, spot2Occupied); 

        //==========LED==============
        updateLEDs(spot1Occupied, spot2Occupied);



    }
}