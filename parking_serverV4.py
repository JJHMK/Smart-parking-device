import paho.mqtt.client as mqtt
import json
import csv
import time
import traceback
from datetime import datetime

# ==================================================
# MQTT ASETUKSET
# ==================================================

BROKER_ADDRESS = "10.60.207.207"
PORT = 1883

USERNAME = "iot_user"
PASSWORD = "19863cpn89"

TOPIC = "parking/#"

# ==================================================
# PARKKIPAIKKOJEN TIETOKANTA
# ==================================================

parking_database = {}

# ==================================================
# MQTT YHDISTETTY
# ==================================================

def on_connect(client, userdata, flags, rc):

    if rc == 0:
        print("MQTT Connected")
        client.subscribe(TOPIC)

    else:
        print("MQTT connection failed:", rc)

# ==================================================
# MQTT VIESTI VASTAANOTETTU
# ==================================================

def on_message(client, userdata, msg):

    # Ei käsitellä näyttöviestejä uudelleen
    if msg.topic == "parking/display":
        return

    try:

        data = json.loads(msg.payload.decode())

        # ==========================================
        # PARKKIPAIKKA-ANTURIT
        # ==========================================
        
        if msg.topic == "parking/gate/sensor":

            distance_cm = data.get("distance_cm")

            if distance_cm is None:
                return

            if distance_cm < 50:

                client.publish("parking/gate/control", json.dumps({"command": "open"}) )

                print("Gate OPEN command sent")

        return
        
        

        if msg.topic == "parking/sensor":

            spot_name = data.get("sensor_id")
            distance_cm = data.get("distance_cm")

            if spot_name is None:
                return

            if distance_cm is None:
                return

            # Luodaan paikka ensimmäisellä kerralla
            if spot_name not in parking_database:

                parking_database[spot_name] = {
                    "status": "Vacant",
                    "parked_at": None
                }

            old_status = parking_database[spot_name]["status"]

            # ======================================
            # HYSTEREESI
            #
            # < 50 cm  = Occupied
            # > 80 cm  = Vacant
            # 50-80 cm = pidä vanha tila
            # ======================================

            old_status = parking_database[spot_name]["status"]

            print(f"{spot_name} distance={distance_cm} old={old_status}"
            )

            if distance_cm < 50:
                new_status = "Occupied"

            elif distance_cm > 80:
                new_status = "Vacant"

            else:
                new_status = old_status

            print(f"{spot_name} -> {new_status}")

            # ======================================
            # TALLENNETAAN CSV:HEN
            # ======================================

            with open("parking_data.csv", "a", newline="") as file:
                
                now = datetime.now()
                writer = csv.writer(file)

                writer.writerow([
                    now.strftime("%Y-%m-%d %H:%M:%S"),
                    spot_name,
                    new_status
                ])

            # ======================================
            # AUTO SAAPUI
            # ======================================

            if new_status == "Occupied":

                parking_database[spot_name]["parked_at"] = now

                print(f"\n{spot_name} OCCUPIED")
                print(f"Distance: {distance_cm} cm")

                distance_cm = data.get("distance_cm")    
                    
                # Spot 1 ohjaa porttia
                if spot_name == "spot_1":

                    client.publish(
                        "parking/gate/control",
                        json.dumps({"command": "open"})
                    )

                    print("Gate OPEN command sent")



            # ======================================
            # AUTO POISTUI
            # ======================================

            else:

                parked_at = parking_database[spot_name]["parked_at"]

                if parked_at:

                    duration = now - parked_at
                    minutes = int(duration.total_seconds() / 60)

                    print(
                        f"\n{spot_name} released after {minutes} min")

                    parking_database[spot_name]["parked_at"] = None

            # ======================================
            # TULOSTETAAN TILA
            # ======================================

            print("\n===== PARKING STATUS =====")

            for spot, info in parking_database.items():

                print(f"{spot}: {info['status']}")

                print("==========================")

                # ==========================================
                # PORTIN TILATAPAHTUMA
                # ==========================================

        elif msg.topic == "parking/gate/status":

                        if data.get("event") == "car_arrived":

                            available_spot = "TAYNNA"

                            for spot, info in parking_database.items():

                                if info["status"] == "Vacant":

                                    number = spot.split("_")[1]
                                    available_spot = f"Paikka {number}"

                                    break

        client.publish("parking/display",available_spot)

        print(f"Assigned parking spot: {available_spot}")

    except Exception:

        print("\n===== ERROR =====")
        traceback.print_exc()
        print("=================\n")

# ==================================================
# MQTT CLIENT
# ==================================================

client = mqtt.Client()

client.username_pw_set(
    USERNAME,
    PASSWORD
)

client.on_connect = on_connect
client.on_message = on_message

# ==================================================
# KÄYNNISTYS
# ==================================================

print("Parking server started")

client.connect(
    BROKER_ADDRESS,
    PORT,
    60
)

client.loop_forever()



