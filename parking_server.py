import paho.mqtt.client as mqtt
import json
from datetime import datetime

# --- Broker Configuration ---
BROKER_ADDRESS = "localhost"  # Use your computer's IP if the ESP32 is on another machine
PORT = 1883
TOPIC = "parking/sensor/#"    # The '#' wildcard listens to all parking sensors
USERNAME = "iot_user"         # The authenticated user created in Mosquitto
PASSWORD = "your_secure_password"

# Callback when the client successfully connects to the broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Successfully connected to Mosquitto MQTT Broker.")
        client.subscribe(TOPIC)
        print(f"Subscribed to topic: {TOPIC}")
    else:
        print(f"Connection failed with return code {rc}")

# Callback when a message is received from the ESP32
def on_message(client, userdata, msg):
    try:
        # 1. Receive the raw payload from the microcontroller
        raw_payload = msg.payload.decode()
        data = json.loads(raw_payload)
        
        # 2. Extract the raw variables sent by the ESP32
        sensor_id = data.get("sensor_id", "unknown")
        distance_cm = data.get("distance_cm", 0)
        
        # 3. Edit and enrich the data on the server side
        # Calculate occupancy: if an object is closer than 100cm, the spot is occupied
        is_occupied = True if distance_cm < 100 else False
        
        data["status"] = "Occupied" if is_occupied else "Vacant"
        data["server_timestamp"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        data["action_required"] = "Turn LED Red" if is_occupied else "Turn LED Green"
        
        # 4. Output the edited data
        print(f"\n--- Data Received on {msg.topic} ---")
        print(json.dumps(data, indent=4))
        
    except json.JSONDecodeError:
        print(f"Failed to parse JSON from payload: {msg.payload.decode()}")
    except Exception as e:
        print(f"Error processing message: {e}")

# --- Initialize and Run the Client ---
client = mqtt.Client()
client.username_pw_set(USERNAME, PASSWORD)
client.on_connect = on_connect
client.on_message = on_message

print("Initializing local parking server...")
client.connect(BROKER_ADDRESS, PORT, 60)

# Run the loop continuously to process incoming data
client.loop_forever()