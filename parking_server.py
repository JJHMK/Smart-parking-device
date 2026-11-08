import paho.mqtt.client as mqtt
import json
from datetime import datetime

# --- Broker Configuration ---
BROKER_ADDRESS = "192.168.1.151"  # Use your computer's IP if the ESP32 is on another machine
PORT = 1883
TOPIC = "parking/#"           # The '#' wildcard now listens to sensors AND the gate
USERNAME = "iot_user"        
PASSWORD = "your_secure_password" # Remember to change this to your real password!

# --- Mock Database ---
# Nyt tietokanta tallentaa tilan lisäksi myös saapumisajan
parking_database = {
    "spot_1": {"status": "Vacant", "parked_at": None},
    "spot_2": {"status": "Vacant", "parked_at": None},
    "spot_3": {"status": "Vacant", "parked_at": None}
}

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Successfully connected to Mosquitto MQTT Broker.")
        client.subscribe(TOPIC)
        print(f"Subscribed to topic: {TOPIC}")
    else:
        print(f"Connection failed with return code {rc}")

def on_message(client, userdata, msg):
    # Sivuutetaan omat näytölle lähetetyt viestit (estää JSON-virheen / kaikuefektin)
    if msg.topic == "parking/display":
        return
        
    try:
        raw_payload = msg.payload.decode()
        data = json.loads(raw_payload)
        
        # --- SCENARIO 1: A parking spot sensor sends a distance update ---
        if msg.topic.startswith("parking/sensor/"):
            distance_cm = data.get("distance_cm")
            
            if distance_cm is not None:
                is_occupied = True if distance_cm < 100 else False
                new_status = "Occupied" if is_occupied else "Vacant"
                spot_name = msg.topic.split("/")[-1]
                
                # Haetaan paikan vanha tila tietokannasta
                old_status = parking_database[spot_name]["status"]
                
                # Tehdään asioita vain, jos paikan tila oikeasti muuttuu
                if new_status != old_status:
                    parking_database[spot_name]["status"] = new_status
                    now = datetime.now()
                    timestamp = now.strftime("%H:%M:%S")
                    
                    if new_status == "Occupied":
                        # Auto saapuu, tallennetaan aloitusaika
                        parking_database[spot_name]["parked_at"] = now
                        print(f"[{timestamp}] Sensor update: {spot_name} is now OCCUPIED.")
                    else:
                        # Auto lähtee, lasketaan kuinka kauan se oli ruudussa
                        parked_time = parking_database[spot_name]["parked_at"]
                        if parked_time:
                            duration = now - parked_time
                            minutes = int(duration.total_seconds() // 60)
                            seconds = int(duration.total_seconds() % 60)
                            print(f"[{timestamp}] Sensor update: {spot_name} freed. Total parking time: {minutes} min {seconds} sec.")
                        else:
                             # Varmistus jos auto poistuu ennen kuin järjestelmä ehti rekisteröidä saapumisen
                             print(f"[{timestamp}] Sensor update: {spot_name} is now Vacant.")
                        
                        # Nollataan aika seuraavaa autoa varten
                        parking_database[spot_name]["parked_at"] = None

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
                for spot, spot_data in parking_database.items():
                    if spot_data["status"] == "Vacant":
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

print("Initializing local parking server with Gate Logic & Time Tracking...")
client.connect(BROKER_ADDRESS, PORT, 60)
client.loop_forever()