import paho.mqtt.client as mqtt
import json
import csv
from datetime import datetime

BROKER_ADDRESS = "10.29.217.207"
PORT = 1883

USERNAME = "iot_user"
PASSWORD = "19863cpn89"

TOPIC = "parking/#"

parking_database = {}


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("MQTT Connected")
        client.subscribe(TOPIC)
    else:
        print("Connection failed:", rc)


def on_message(client, userdata, msg):

    if msg.topic == "parking/display":
        return

    try:
        data = json.loads(msg.payload.decode())

        if msg.topic == "parking/sensor":

            spot_name = data.get("sensor_id")
            distance_cm = data.get("distance_cm")

            if spot_name is None:
                return

            if spot_name not in parking_database:
                parking_database[spot_name] = {
                    "status": "Vacant",
                    "parked_at": None
                }

            new_status = "Occupied" if distance_cm < 50 else "Vacant"
            old_status = parking_database[spot_name]["status"]

            if new_status != old_status:

                parking_database[spot_name]["status"] = new_status
                now = datetime.now()

                with open("parking_data.csv", "a", newline="") as file:
                    writer = csv.writer(file)
                    writer.writerow([
                        now.strftime("%Y-%m-%d %H:%M:%S"),
                        spot_name,
                        new_status
                    ])

                if new_status == "Occupied":

                    parking_database[spot_name]["parked_at"] = now

                    print(f"{spot_name} OCCUPIED")
                    
                    # Jos anturi 1. tunnistaa auton
                    if spot_name == "spot_1":
                        client.publish("parking/gate/control", json.dumps({"command": "open"}))
                        print("Gate open command sent")

                else:

                    parked_at = parking_database[spot_name]["parked_at"]

                    if parked_at:
                        duration = now - parked_at
                        minutes = int(duration.total_seconds() / 60)

                        print(
                            f"{spot_name} released after {minutes} minutes"
                        )

                    if spot_name == "spot_1":
                        client.publish("parking/gate/control", json.dumps({"command": "close"}))
                        print("Gate close command sent")
                    
                    parking_database[spot_name]["parked_at"] = None

                print("\n===== PARKING STATUS =====")

                for spot, info in parking_database.items():
                    print(
                        f"{spot}: {info['status']}"
                    )

        elif msg.topic == "parking/gate/status":

            if data.get("event") == "car_arrived":

                available_spot = "TAYNNA"

                for spot, info in parking_database.items():

                    if info["status"] == "Vacant":

                        number = spot.split("_")[1]
                        available_spot = f"Paikka {number}"
                        break

                client.publish(
                    "parking/display",
                    available_spot
                )

                print(
                    f"Assigned: {available_spot}"
                )

    except Exception as e:
        print("Error:", e)


client = mqtt.Client()

client.username_pw_set(
    USERNAME,
    PASSWORD
)

client.on_connect = on_connect
client.on_message = on_message

print("Parking server started")

client.connect(
    BROKER_ADDRESS,
    PORT,
    60
)

client.loop_forever()