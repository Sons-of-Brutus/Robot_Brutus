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
