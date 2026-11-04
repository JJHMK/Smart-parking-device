import paho.mqtt.publish as publish
import json

# --- Broker Configuration ---
HOSTNAME = "localhost"
PORT = 1883
AUTH = {'username':"iot_user", 'password':"your_secure_password"} # Change to your real password!

print("--- Smart Parking Simulator ---")
print("1: Car arrives at the gate")
print("2: Car parks in Spot 1 (Occupied)")
print("3: Car leaves Spot 1 (Vacant)")
choice = input("Enter 1, 2, or 3 and press Enter: ")

# Determine what data to send based on your choice
if choice == '1':
    topic = "parking/gate/status"
    payload = json.dumps({"event": "car_arrived"})
    print("\nSimulating: Car arrived at the gate...")
    
elif choice == '2':
    topic = "parking/sensor/spot_1"
    payload = json.dumps({"sensor_id": "test_node", "distance_cm": 45})
    print("\nSimulating: Car parked in Spot 1...")
    
elif choice == '3':
    topic = "parking/sensor/spot_1"
    payload = json.dumps({"sensor_id": "test_node", "distance_cm": 150})
    print("\nSimulating: Car left Spot 1...")
    
else:
    print("Invalid choice. Exiting.")
    exit()

# Send the chosen message to the broker
publish.single(
    topic=topic,
    payload=payload,
    hostname=HOSTNAME,
    port=PORT,
    auth=AUTH
)

print(f"Message sent successfully to topic: {topic}")