# User Sketch

## Enum Declaration

This enum is declared to be used in the state machine created in the logic task.
```c
enum FuncMode {
  STANDING = 0,
  EXERCISE_1 = 1,
  EXERCISE_2 = 2,
  LEG_CONTROL = 3,
  VEL_CONTROL = 4
};
```

## Variable Declaration

In this section, the main system objects are declared.  
The **PCA (`Adafruit_PWMServoDriver`)** is not only declared but also **initialized to enable control of the different servos**.  
Additionally, the `brutus_comms` object is declared, which is responsible for communications, and the `brutus` object, which handles both motion control and sensory processing.  
Finally, the *task handle* that manages the system logic is declared.


```c
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

Brutus brutus;
BrutusComms brutus_comms;

TaskHandle_t mode_task_handle;
```

## Setup()
### Brutus
First, Brutus is prepared for operation by calling the general setup function. In this step, the PCA controller, the LED pins, and the PWM frequency used for servo control are assigned.

Next, the setup functions for each of Brutus’ legs are called individually. In each call, the PCA pins to which the two servos of each leg (shoulder and elbow) are connected are specified, along with the minimum and maximum PWM values and the angular limits for each servomotor. Angular offsets are also defined to compensate for possible mechanical inaccuracies caused by imperfect physical calibration. Due to the physical arrangement of the legs, some servos require inversion of the rotation direction to ensure correct movement; this behavior is configured using the last two parameters of each function.

Then, the perception system is initialized, which includes the ultrasonic sensors. Once all configuration steps are completed, the function brutus.start() is called, leaving the system fully ready for use.

After this initialization, it is possible to set the robot’s desired initial configuration. The code provides an example of a possible initial setup.

Finally, the task that will manage both movement and perception is created

```c
brutus.setup(&pca,
               PCA9685_OE,
               R_PIN,
               B_PIN,
               G_PIN,
               PWM_SERVO_FREQ);

brutus.setup_front_right_leg(SHOULDER_FRONT_RIGHT_PCA_PIN, ELBOW_FRONT_RIGHT_PCA_PIN,
                            MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                            MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                            MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                            MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                            SHOULDER_FRONT_RIGHT_ANGLE_OFFSET, ELBOW_FRONT_RIGHT_ANGLE_OFFSET,
                            SHOULDER_FRONT_RIGHT_INVERTED, ELBOW_FRONT_RIGHT_INVERTED);

brutus.setup_front_left_leg(SHOULDER_FRONT_LEFT_PCA_PIN,    ELBOW_FRONT_LEFT_PCA_PIN,
                            MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                            MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                            MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                            MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                            SHOULDER_FRONT_LEFT_ANGLE_OFFSET, ELBOW_FRONT_LEFT_ANGLE_OFFSET,
                            SHOULDER_FRONT_LEFT_INVERTED, ELBOW_FRONT_LEFT_INVERTED);

brutus.setup_back_right_leg(SHOULDER_BACK_RIGHT_PCA_PIN, ELBOW_BACK_RIGHT_PCA_PIN,
                            MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                            MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                            MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                            MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                            SHOULDER_BACK_RIGHT_ANGLE_OFFSET, ELBOW_BACK_RIGHT_ANGLE_OFFSET,
                            SHOULDER_BACK_RIGHT_INVERTED, ELBOW_BACK_RIGHT_INVERTED);

brutus.setup_back_left_leg(SHOULDER_BACK_LEFT_PCA_PIN, ELBOW_BACK_LEFT_PCA_PIN,
                            MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                            MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                            MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                            MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                            SHOULDER_BACK_LEFT_ANGLE_OFFSET, ELBOW_BACK_LEFT_ANGLE_OFFSET,
                            SHOULDER_BACK_LEFT_INVERTED, ELBOW_BACK_LEFT_INVERTED);

brutus.setup_perception(US_R_TRIG,
                        US_R_ECHO,
                        US_L_TRIG,
                        US_L_ECHO,
                        US_F_TRIG,
                        US_F_ECHO,
                        MIN_PERCEPTION_TIMEOUT);

brutus.start();

brutus.set_linear_speed_ts(0.0);
brutus.set_angular_speed_ts(0.0);
brutus.change_target_pose(STANDING_POSE);

brutus.set_motion_control_mode(POSE_CONTROL);

brutus.create_motion_task(DEFAULT_MOTION_PERIOD, MOTION_CORE);
brutus.create_perception_task(PERCEPTION_PERIOD, LOGIC_CORE, PERCEPTION_PRIO);
```

### Brutus Comms
To initialize BrutusComms, simply call the start function first, and then create the task that will manage communications.

```c
brutus_comms.start(&brutus);
brutus_comms.create_comms_task(COMMS_CORE); 

```
### Logic Task
Finally, we create the task that will manage Brutus’ highest-level logic, that is, what to do in each mode.
```c
  xTaskCreatePinnedToCore(
    (TaskFunction_t)mode_task,
    "ModeTask",
    MODES_TASK_SIZE,
    NULL,
    2,
    &mode_task_handle,
    LOGIC_CORE
  );
```

## loop()
```c
void loop() {}
```
## Logic Task Definition
It is worth noting that in each mode we change the eye color to indicate the current mode, although this is not strictly necessary.

```c
void
mode_task(void* pvParameters)
{
  BrutusCommsCmd cmd;
  
  // Initial values
  enum FuncMode mode = EXERCISE_1;
  enum FuncMode last_mode = STANDING;

  BrutusPose pose = brutus.check_pose(true);

  float fr_elbow = 0.0;
  float fl_elbow = 0.0;
  float br_elbow = 0.0;
  float bl_elbow = 0.0;

  while (true) {
    // We update the command using the information received through Mosquitto communications.
    cmd = brutus_comms.getCmd();
    mode = FuncMode(cmd.mode);

    switch (mode)
    {
      case STANDING:
        if (mode != last_mode) {
          brutus.eyes_blue();
          // We set Brutus to operate in pose control mode.
          brutus.set_motion_control_mode(POSE_CONTROL);
          last_mode = mode;
        }
        // Set brutus to STANDING_POSE
        brutus.change_target_pose(STANDING_POSE);
        break;
      
      case LEG_CONTROL:
        if (mode != last_mode) {
          brutus.eyes_yellow();
          // We set Brutus to operate in pose control mode.
          brutus.set_motion_control_mode(POSE_CONTROL);
          last_mode = mode;
        }

        brutus.change_target_pose(interp_pose_norm(cmd.pose));
        break;

      case VEL_CONTROL:
        if (mode != last_mode) {
          brutus.eyes_green();
          // We set Brutus to operate in speed control mode.
          brutus.set_motion_control_mode(SPEED_CONTROL);
          last_mode = mode;
        }

        // We use the v and w recived from comms to set Brutus v and w
        brutus.set_linear_speed_ts(cmd.v);
        brutus.set_angular_speed_ts(cmd.w);
        break;

      case EXERCISE_1:
        // Logic of exercise 1
        continue;
        break;

      case EXERCISE_2:
        // Logic of exercise 2
        continue;
        break;

      default:
        break;
    }
  }
}
```
