# Comms

## Comms struct

The **communications structure** is based on the **ESP32 itself**, which sends and receives messages via **MQTT**, and on a **graphical interface**, which is also capable of sending and receiving messages. In our case, **two possibilities** are considered for this interface: a **terminal-based graphical interface** or an **Android application developed in Android Studio**.

## Wifi

The **MQTT connection** can be established through **two different types of WiFi networks** by using the `credential.h` file: a **conventional WiFi network** and the **eduroam network**. To control which type of network the system connects to, the **`IS_EDUROAM` parameter** is used, which is defined in the **`comms_params.h` file**.


## Message types

### ESP > Interface
The ESP sends to the interface all the required information obtained from Brutus and stored in the `BrutusCommsData` structure, which is transmitted through different MQTT topics for later visualization in the interface. These messages include a **heartbeat**, sent periodically by the robot to indicate that it is still running; the **current pose of the robot**, defined by its four legs, with each leg described by the angles of its shoulder and elbow joints; and the **distance measured by each ultrasonic sensor**.

### Interface > ESP
From the interface to the ESP, different messages are sent and initially stored in separate structures, which are later grouped into a global structure called `BrutusCommsCmd`. These messages include the commanded **linear and angular velocities** (v and w), the **target pose** to be applied to the robot, and the different **operating modes**, including EX1 and EX2 to select the exercise to be executed, *Standing* to keep the robot standing without moving, *Leg_Control* to control it using pose messages sent from the interface, and *VEL_CONTROL* to control it through velocity messages.

