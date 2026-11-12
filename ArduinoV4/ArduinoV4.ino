#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// =========================
// WIFI
// =========================

const char* ssid = "HUAWEI P30 lite";
const char* password = "19863cpn89";

// =========================
// MQTT
// =========================

const char* mqtt_server = "10.29.217.207";
const int mqtt_port = 1883;

const char* mqtt_user = "iot_user";
const char* mqtt_pass = "19863cpn89";

const char* sensorTopic = "parking/sensor";
const char* displayTopic = "parking/display";

// =========================
// HC-SR04 Anturi 1
// =========================

const int trigPin1 = 5;
const int echoPin1 = 4;

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

// =========================

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSend = 0;

// =========================
// WIFI
// =========================

void connectWiFi()
{
    WiFi.begin(ssid, password);

    Serial.print("Yhdistetään WiFiin");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi yhdistetty");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
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

        if (client.connect(
                "ESP32_PARKING",
                mqtt_user,
                mqtt_pass))
        {
            Serial.println(" OK");

            client.subscribe(displayTopic);

            Serial.print("Tilattu topic: ");
            Serial.println(displayTopic);
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

// =========================
// Setup
// =========================

void setup()
{
    Serial.begin(115200);

    pinMode(trigPin1, OUTPUT);
    pinMode(echoPin1, INPUT);

    pinMode(trigPin2, OUTPUT);
    pinMode(echoPin2, INPUT);

    pinMode(trigPin3, OUTPUT);
    pinMode(echoPin3, INPUT);

    connectWiFi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

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

    if (millis() - lastSend > 5000)
    {
        lastSend = millis();

        long distance1 = readDistance(trigPin1, echoPin1);
        delay(50);

        long distance2 = readDistance(trigPin2, echoPin2);
        delay(50);

        long distance3 = readDistance(trigPin3, echoPin3);

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

        // ===================
        // SPOT 3
        // ===================

        doc.clear();
        doc["sensor_id"] = "spot_3";
        doc["distance_cm"] = distance3;

        serializeJson(doc, buffer);

        client.publish(sensorTopic, buffer);

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

        Serial.print("Spot 3: ");
        Serial.print(distance3);
        Serial.println(" cm");

        Serial.println("===============================");
    }
}