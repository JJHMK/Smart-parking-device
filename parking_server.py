import paho.mqtt.client as mqtt
import json
from datetime import datetime

# --- Broker Configuration ---
BROKER_ADDRESS = "localhost"  # Use your computer's IP if the ESP32 is on another machine
PORT = 1883
TOPIC = "parking/#"           # The '#' wildcard now listens to sensors AND the gate
USERNAME = "iot_user"         
PASSWORD = "your_secure_password" # Remember to change this to your real password!

# --- Mock Database ---
# This dictionary tracks the current status of all parking spots in your system
parking_database = {
    "spot_1": "Vacant",
    "spot_2": "Vacant",
    "spot_3": "Vacant"
}

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Successfully connected to Mosquitto MQTT Broker.")
        client.subscribe(TOPIC)
        print(f"Subscribed to topic: {TOPIC}")
    else:
        print(f"Connection failed with return code {rc}")

def on_message(client, userdata, msg):
    try:
        raw_payload = msg.payload.decode()
        data = json.loads(raw_payload)
        
        # --- SCENARIO 1: A parking spot sensor sends a distance update ---
        if msg.topic.startswith("parking/sensor/"):
            distance_cm = data.get("distance_cm")
            
            # If the payload includes distance, update the spot status
            if distance_cm is not None:
                is_occupied = True if distance_cm < 100 else False
                current_status = "Occupied" if is_occupied else "Vacant"
                
                # Extract the spot name from the topic (e.g., "spot_1")
                spot_name = msg.topic.split("/")[-1]
                parking_database[spot_name] = current_status
                
                timestamp = datetime.now().strftime("%H:%M:%S")
                print(f"[{timestamp}] Sensor update: {spot_name} is now {current_status}")

            # If it's just the ping from the stripped down ESP32 code
            elif data.get("status") == "ping":
                sensor_id = data.get("sensor_id", "unknown")
                print(f"Ping received from {sensor_id} on {msg.topic}")

        # --- SCENARIO 2: A car arrives at the gate ---
        elif msg.topic == "parking/gate/status":
            if data.get("event") == "car_arrived":
                print(f"\n--- Car arrived at the gate! ---")
                
                # Search the database for the first available spot
                available_spot = "TAYNNA" # "FULL" in Finnish, default if no spots are vacant
                for spot, status in parking_database.items():
                    if status == "Vacant":
                        # Format the spot name nicely (e.g., "spot_1" -> "Paikka 1")
                        spot_number = spot.split("_")[1]
                        available_spot = f"Paikka {spot_number}"
                        break
                
                print(f"Assigning spot: {available_spot}")
                
                # Publish the available spot back to the ESP32's OLED display
                client.publish("parking/display", available_spot)
                print(f"Sent '{available_spot}' to the display.\n")
                
    except json.JSONDecodeError:
        print(f"Failed to parse JSON: {msg.payload.decode()}")
    except Exception as e:
        print(f"Error processing message: {e}")

# --- Initialize and Run the Client ---
client = mqtt.Client()
client.username_pw_set(USERNAME, PASSWORD)
client.on_connect = on_connect
client.on_message = on_message

print("Initializing local parking server with Gate Logic...")
client.connect(BROKER_ADDRESS, PORT, 60)
client.loop_forever()