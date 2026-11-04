import serial
import paho.mqtt.publish as publish
import json

# --- Settings ---
COM_PORT = "COM3"       # CHANGE THIS to match the port in Arduino IDE (e.g., COM4, COM5)
BAUD_RATE = 115200
MQTT_TOPIC = "parking/sensor/spot_1"
MQTT_HOST = "localhost"
MQTT_AUTH = {'username':"iot_user", 'password':"your_secure_password"} # Change to your real password

print(f"Connecting to ESP32 on port {COM_PORT}...")

try:
    # Open the USB serial connection to the ESP32
    esp32_serial = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print("USB connection opened! Listening for data...\n")

    while True:
        # Read data from the USB cable
        if esp32_serial.in_waiting > 0:
            raw_data = esp32_serial.readline().decode('utf-8').strip()

            # Ensure the data looks like JSON before sending
            if raw_data.startswith("{") and raw_data.endswith("}"):
                print(f"Received via USB: {raw_data}")

                # Forward the data to the local MQTT broker
                publish.single(
                    topic=MQTT_TOPIC,
                    payload=raw_data,
                    hostname=MQTT_HOST,
                    port=1883,
                    auth=MQTT_AUTH
                )
                print("Forwarded to MQTT server successfully!\n")

except serial.SerialException as e:
    print(f"Error opening port: {e}. Make sure the Arduino IDE Serial Monitor is CLOSED.")
except KeyboardInterrupt:
    print("\nBridge closed.")