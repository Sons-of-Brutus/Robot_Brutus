# `brutus_params.h` – Robot configuration and motion parameters

The file brutus_params.h contains all the global configuration parameters that define the electrical, mechanical and motion behavior of the Brutus robot.
It centralizes all tunable values so that the robot can be calibrated, adapted and modified without changing the core control logic.

This file covers:

- Servo configuration

- GPIO pin mapping

- Leg geometry and calibration

- FreeRTOS task configuration

- Motion poses and gait definitions

- Speed limits

- Perception system timing

## Servo configuration

```cpp
#define PWM_SERVO_FREQ 50
#define MIN_SERVO_ANGLE -90.0
#define MAX_SERVO_ANGLE 90.0
#define MIN_SERVO_PWM_PULSE_US 500
#define MAX_SERVO_PWM_PULSE_US 2500
```

These parameters define how the PWM signals are generated for the servos:

- `PWM_SERVO_FREQ` defines the PWM frequency (50 Hz), which is standard for RC servos.

- `MIN_SERVO_ANGLE` and `MAX_SERVO_ANGLE` define the angular range used by the software.

- `MIN_SERVO_PWM_PULSE_US` and `MAX_SERVO_PWM_PULSE_US` define the electrical PWM pulse width corresponding to the minimum and maximum angles.

## GPIO configuration

The GPIO section defines how the ESP32 is electrically connected to the sensors and actuators:
```cpp
#define US_R_TRIG 17
#define US_R_ECHO 5

#define US_L_TRIG 4
#define US_L_ECHO 16

#define US_F_TRIG 15
#define US_F_ECHO 2

#define R_PIN 32
#define G_PIN 25
#define B_PIN 33

#define PCA9685_OE 19
```

These pins correspond to:

- Three ultrasonic distance sensors (front, left and right).

- An RGB LED (R, G, B channels).

- The Output Enable (OE) pin of the PCA9685 servo controller.

## Leg servo mapping

Each leg has two servos:

- A shoulder (horizontal movement)

- An elbow (vertical movement)

The file defines which PCA9685 channel controls each servo:

```cpp
#define SHOULDER_BACK_LEFT_PCA_PIN 0
#define ELBOW_BACK_LEFT_PCA_PIN    1

#define SHOULDER_BACK_RIGHT_PCA_PIN 2
#define ELBOW_BACK_RIGHT_PCA_PIN    3

#define SHOULDER_FRONT_LEFT_PCA_PIN 9
#define ELBOW_FRONT_LEFT_PCA_PIN    8

#define SHOULDER_FRONT_RIGHT_PCA_PIN 13
#define ELBOW_FRONT_RIGHT_PCA_PIN    12
```

## Servo calibration

Mechanical assembly imperfections mean that servos are never perfectly aligned.
To compensate for this, angular offsets are defined for each joint:

```cpp
#define SHOULDER_BACK_LEFT_ANGLE_OFFSET -2.0
#define ELBOW_BACK_LEFT_ANGLE_OFFSET    -5.0

#define SHOULDER_BACK_RIGHT_ANGLE_OFFSET 3.0
#define ELBOW_BACK_RIGHT_ANGLE_OFFSET    0.0

#define SHOULDER_FRONT_LEFT_ANGLE_OFFSET -10.0
#define ELBOW_FRONT_LEFT_ANGLE_OFFSET    -12.0

#define SHOULDER_FRONT_RIGHT_ANGLE_OFFSET -2.0
#define ELBOW_FRONT_RIGHT_ANGLE_OFFSET    -20.0
```

In addition, because of the colocation of some legs, some servos are mounted in inverted orientations. This is handled by:

```cpp
#define SHOULDER_BACK_LEFT_INVERTED false
#define ELBOW_BACK_LEFT_INVERTED    true

#define SHOULDER_BACK_RIGHT_INVERTED true
#define ELBOW_BACK_RIGHT_INVERTED    false

#define SHOULDER_FRONT_LEFT_INVERTED false
#define ELBOW_FRONT_LEFT_INVERTED    false

#define SHOULDER_FRONT_RIGHT_INVERTED true
#define ELBOW_FRONT_RIGHT_INVERTED    true
```

## FreeRTOS task configuration

The robot software runs on FreeRTOS using two CPU cores:
```cpp
#define MOTION_CORE  0
#define LOGIC_CORE   1
```

- The motion core handles gait generation and servo control.

- The logic core handles perception, communication and high-level decision making.

Task timing is controlled by:

```cpp
#define DEFAULT_MOTION_PERIOD 300
#define MODES_PERIOD 200
```

These values define how often motion and logic tasks are executed.

## Pose and gait definition

The robot motion is defined using poses, represented by the `BrutusPose` structure:

```cpp
struct BrutusPose {
  BrutusLegState fr_leg_state;
  BrutusLegState fl_leg_state;
  BrutusLegState br_leg_state;
  BrutusLegState bl_leg_state;
};
```
Each pose contains the desired joint angles for the four legs.

Each pose contains the desired joint angles for the four legs.

A **gait** is defined as a sequence of poses:

```cpp
constexpr BrutusPose GAIT_STEPS[6] {GAIT_1, GAIT_2, ... , GAIT_6};
```
These poses describe a walking cycle.
A backward gait is also defined by reversing this sequence.

Spin-in-place motions are defined using the `SPIN_STEPS` arrays.

This approach makes it easy to design and tune locomotion by editing poses rather than writing complex motion equations.
## Speed limits

```cpp
#define MIN_W -1.0f
#define MAX_W 1.0f

#define MIN_V -1.0f
#define MAX_V 1.0f
```
These values define the allowed ranges for:

- Linear velocity (V)

- Angular velocity (W)

They are used by higher-level control to limit how fast the robot can move or turn.

## Perception system
```cpp
#define N_DISTANCE_SENSORS 3
#define PERCEPTION_PRIO 3
#define PERCEPTION_PERIOD 200
#define MIN_PERCEPTION_TIMEOUT 10
```

These parameters configure the ultrasonic sensor system:

- Number of sensors.

- FreeRTOS task priority.

- Update period (every 200 ms).

- Minimum allowed timeout for a measurement.

This ensures reliable obstacle detection without blocking the rest of the system.

# `comms_params.h` – MQTT communication configuration

The file comms_params.h centralizes all the configuration required for the robot’s communication layer.
It defines the MQTT client identity, the subscribed and published topics, timing parameters for connection handling, message buffers, and default command/data structures used at startup.

This file is designed to keep the MQTT interface consistent and easy to maintain. 

## MQTT client identifier

```cpp
#define CLIENT_ID "brutus_esp"
```

This is the unique MQTT client identifier used when the ESP32 connects to the broker.

## Subscribed topics (commands to the robot)

```cpp
#define TOPIC_CMD_MODE   "brutus/cmd/mode"
#define TOPIC_CMD_VEL    "brutus/cmd/velocity"
#define TOPIC_CMD_POSE   "brutus/cmd/pose"
```

The robot subscribes to these topics to receive commands from an external controller:

- brutus/cmd/mode: change the robot operating mode.

- brutus/cmd/velocity: set linear and angular velocity references.

- brutus/cmd/pose: directly command a full pose to the robot.

To simplify internal handling, each command is also mapped to an integer code:

```cpp
#define CMD_MODE   0
#define CMD_VEL    1
#define CMD_POSE   2
```

## Published topics (robot telemetry)

```cpp
#define TOPIC_STATE_HEARTBEAT "brutus/data/heartbeat"
#define TOPIC_POSE            "brutus/data/pose"
#define TOPIC_DIST_FRONT      "brutus/data/distance/front"
#define TOPIC_DIST_RIGHT      "brutus/data/distance/right"
#define TOPIC_DIST_LEFT       "brutus/data/distance/left"
```

The robot publishes telemetry data to these topics:

- `brutus/data/heartbeat`: periodic status message that indicates the robot is alive and connected.

- `brutus/data/pose`: the pose currently executed by the robot.

- `brutus/data/distance/*`: distances measured by the ultrasonic sensors (front, right, left).

Similarly, message types are mapped to integer identifiers:

```cpp
#define STATE_HEARTBEAT 0
#define POSE            1
#define DIST_FRONT      2
#define DIST_RIGHT      3
#define DIST_LEFT       4
```

## Connection timing parameters

```cpp
#define WIFI_WAIT 500
#define RECONNECT_WAIT 3000
```

- `WIFI_WAIT` sets the delay (in milliseconds) used while waiting for WiFi to connect.

- `RECONNECT_WAIT` sets the delay (in milliseconds) between MQTT reconnection attempts if the connection is lost.

These values avoid aggressive reconnection loops and help keep the system stable.

## Network mode selection (Eduroam vs normal WiFi)

```cpp
#define IS_EDUROAM false
```

This parameter selects which WiFi credentials should be used during initialization:

- true: connect using ***Eduroam*** credentials (enterprise authentication).

- false: connect using a ***standard WiFi network*** (SSID + password).

The actual credentials are defined in credential.h, while this flag determines which set of credentials will be used by the connection code.

## Ranges for test/debug messages

```cpp
#define MIN_LEG_DEG 0
#define MAX_LEG_DEG 181
#define MIN_DIST 0
#define MAX_DIST 200
```

These ranges are created in order to put a limit to the leg degrees and the distance the ultrasonic sensor can reach.

## Joint identifier strings

```cpp
#define SHOULDER "0"
#define ELBOW "1"
```

These string identifiers are used in message payloads (JSON fields) to label which joint is being referenced.

## Buffers and message sizes

```cpp
#define MSG_BUFFER 254
#define JSON_BUFFER 300
```

These values define the maximum sizes used to store incoming/outgoing MQTT payloads:

- `MSG_BUFFER`: raw MQTT message buffer length.

- `JSON_BUFFER`: buffer size for JSON serialization/deserialization.

## Task timing and serial configuration

```cpp
#define COMMS_PERIOD 200
#define COMMS_CORE LOGIC_CORE
#define BAUD 115200
```

- `COMMS_PERIOD` defines how often the communications task runs (every 200 ms).

- `COMMS_CORE` pins the task to the logic core (LOGIC_CORE), keeping communications separate from real-time motion control.

- `BAUD` sets the serial debugging baud rate.

## Default startup command and telemetry

```cpp
constexpr BrutusCommsCmd START_CMD {0, 0.0, 0.0, STANDING_POSE};
constexpr BrutusCommsData START_DATA {STANDING_POSE, 0.0, 0.0, 0.0};
```

At startup, the robot initializes both the current command and telemetry structures with safe default values:

- START_CMD initializes:

  - mode = 0 (default mode)

  - linear velocity v = 0.0

  - angular velocity w = 0.0

  - pose = STANDING_POSE

- START_DATA initializes:

  - pose = STANDING_POSE

  - ultrasonic distances set to 0.0 until valid measurements are available.


