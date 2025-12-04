# mqtt_README

Brief guide to install Mosquitto and test MQTT topics (public broker: `test.mosquitto.org`).

## Requirements
- Debian/Ubuntu based system
- Internet connection (a public broker is used for testing)

## 1. Install Mosquitto and clients
```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients -y
```

## 2. Subscribe to individual topics
Open a terminal for each subscriber (or multiplexer if you prefer).

ESP (topics published by the ESP32):
```bash
mosquitto_sub -h test.mosquitto.org -t brutus/data/heartbeat
mosquitto_sub -h test.mosquitto.org -t brutus/data/pose
mosquitto_sub -h test.mosquitto.org -t brutus/data/distance/front
mosquitto_sub -h test.mosquitto.org -t brutus/data/distance/right
mosquitto_sub -h test.mosquitto.org -t brutus/data/distance/left
```

Controller (topics received/monitored by the controller):
```bash
mosquitto_sub -h test.mosquitto.org -t brutus/state/mode
mosquitto_sub -h test.mosquitto.org -t brutus/cmd/mode
mosquitto_sub -h test.mosquitto.org -t brutus/cmd/velocity
mosquitto_sub -h test.mosquitto.org -t brutus/cmd/pose
```

## 3. Subscribe to all topics
Use the wildcard `#` to capture everything under `brutus/`:
```bash
mosquitto_sub -h test.mosquitto.org -t "brutus/#"
```

## 4. Publish test messages
Send test messages to verify callbacks and subscriptions.

Messages from the ESP (simulated):
```bash
mosquitto_pub -h test.mosquitto.org -t brutus/data/heartbeat -m "heartbeat"
mosquitto_pub -h test.mosquitto.org -t brutus/data/pose -m "data_pose"
mosquitto_pub -h test.mosquitto.org -t brutus/data/distance/front -m "sensor_front"
mosquitto_pub -h test.mosquitto.org -t brutus/data/distance/right -m "sensor_right"
mosquitto_pub -h test.mosquitto.org -t brutus/data/distance/left -m "sensor_left"
```

Messages from the controller:
```bash
mosquitto_pub -h test.mosquitto.org -t brutus/state/mode -m "state_pose"
mosquitto_pub -h test.mosquitto.org -t brutus/cmd/mode -m "cmd_mode"
mosquitto_pub -h test.mosquitto.org -t brutus/cmd/velocity -m "cmd_vel"
mosquitto_pub -h test.mosquitto.org -t brutus/cmd/pose -m "cmd_pose"
```