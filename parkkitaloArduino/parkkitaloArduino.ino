#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ---------- WiFi ----------
const char* ssid = "HUAWEI P30 lite";
const char* password = "19863cpn89";

// ---------- MQTT ----------
const char* mqtt_server = "192.168.1.151";
const int mqtt_port = 1883;
const char* mqtt_user = "iot_user";
const char* mqtt_pass = "your_secure_password";

// ---------- MQTT Topics ----------
const char* sensorTopic = "parking/sensor/spot_1";
const char* displayTopic = "parking/display";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSend = 0;

// ---------- HC-SR04 ----------
const int trigPin = 5;
const int echoPin = 18;

void connectWiFi() {

  Serial.println();
  Serial.println("Aloitetaan WiFi-yhteys");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int count = 0;

  while (WiFi.status() != WL_CONNECTED) {

    delay(1000);

    Serial.print(".");
    Serial.print(" Status: ");
    Serial.println(WiFi.status());

    count++;

    if (count > 30) {
      Serial.println("WiFi yhteys epäonnistui!");
      return;
    }
  }

  Serial.println();
  Serial.println("WiFi yhdistetty!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("===============");
  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Viesti: ");
  Serial.println(message);
}

void reconnect() {

  while (!client.connected()) {

    Serial.print("Yhdistetään MQTT brokeriin...");

    if (client.connect(
          "ESP32_SPOT1",
          mqtt_user,
          mqtt_pass)) {

      Serial.println(" OK");

      client.subscribe(displayTopic);

      Serial.print("Tilattu: ");
      Serial.println(displayTopic);

    } else {

      Serial.print("Virhe: ");
      Serial.println(client.state());

      delay(5000);
    }
  }
}

long readDistance() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);

  long distance = duration * 0.034 / 2;

  return distance;
}

void setup() {

  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  connectWiFi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if (millis() - lastSend > 5000) {

    lastSend = millis();

    long distance_cm = readDistance();

    DynamicJsonDocument doc(256);

    doc["sensor_id"] = "spot_1";
    doc["distance_cm"] = distance_cm;

    char buffer[256];
    serializeJson(doc, buffer);

    client.publish(sensorTopic, buffer);

    Serial.print("Lähetetty: ");
    Serial.println(buffer);
  }
}
