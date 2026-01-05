# Comms

## Comms struct

The communications structure is based on the ESP32 itself, which sends and receives messages via MQTT, and on a graphical interface, which is also capable of sending and receiving messages. In our case, two possibilities are considered for this interface: a terminal-based graphical interface or an Android application developed in Android Studio.

## Wifi

The MQTT connection can be established through two different types of WiFi networks by using the `credential.h` file: a conventional WiFi network and the eduroam network. To control which type of network the system connects to, the `IS_EDUROAM` parameter is used, which is defined in the `comms_params.h` file.

---

## Message types

### ESP > Interface

The data required for this message is obtained from Brutus, stored in the `BrutusCommsData` structure, and sent through the different MQTT topics.

These messages will be displayed later in the interface
#### Heartbeat
Message sent by the robot every x time to check that it is still running.
#### Pose
Current pose of the robot. The pose consists of the four legs, with each leg defined by the angles of its shoulder and elbow joints.
#### Distance
Distance detected by each ultrasonic sensor.

### Interface > ESP
Each of these messages is stored in a different structure, and all of them are later grouped into a global structure called `BrutusCommsCmd`
#### Velocities
Velocity to be commanded to the robot (v and w).
#### Pose 
Pose to be commanded to the robot.
#### Modes
There are diferents modes:
##### EX1 and EX2
To choose the exercise we want to execute.
##### Standing
To keep it standing without moving.
##### Leg_Control
To control it through the `Pose` messages sent from the interface.
##### VEL_CONTROL
To control it through the `Velocities"`messages sent from the interface.