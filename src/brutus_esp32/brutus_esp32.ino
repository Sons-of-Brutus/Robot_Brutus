#include "Brutus.h"
#include "BrutusComms.h"

enum FuncMode {
  STANDING = 0,
  EXERCISE_1 = 1,
  EXERCISE_2 = 2,
  LEG_CONTROL = 3,
  VEL_CONTROL = 4
};

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

Brutus brutus;
BrutusComms brutus_comms;

TaskHandle_t mode_task_handle;

void setup() {
  Serial.begin(115200);

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

  brutus.setup_front_left_leg(SHOULDER_FRONT_LEFT_PCA_PIN, ELBOW_FRONT_LEFT_PCA_PIN,
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

  delay(3000);

  brutus.start();
  brutus_comms.start(&brutus);
  delay(100);

  Serial.println("<START>");
  
  brutus.set_linear_speed_ts(0.0);
  brutus.set_angular_speed_ts(0.0);
  brutus.change_target_pose(STANDING_POSE);

  brutus.set_motion_control_mode(POSE_CONTROL);
  brutus.create_motion_task(DEFAULT_MOTION_PERIOD, MOTION_CORE);
  brutus.create_perception_task(PERCEPTION_PERIOD, LOGIC_CORE, PERCEPTION_PRIO);
  brutus_comms.create_comms_task(COMMS_CORE);

  
  xTaskCreatePinnedToCore(
    (TaskFunction_t)mode_task,
    "ModeTask",
    2048,
    NULL,
    2,
    &mode_task_handle,
    LOGIC_CORE
  );
  
}

void loop() {}

void
mode_task(void* pvParameters)
{
  enum FuncMode mode = EXERCISE_1;
  enum FuncMode last_mode = STANDING;
  BrutusCommsCmd cmd;

  BrutusPerception perception_data;

  TickType_t last_wake_time = xTaskGetTickCount();

  float ex1_kp = 1.0;
  float exp_dist = 20.0;
  float error = 0.0;
  float dist = 0.0;

  while (true) {
    Serial.println("<MODES>");
    uint32_t start_time = micros();

    cmd = brutus_comms.getCmd();
    mode = FuncMode(cmd.mode);

    switch (mode)
    {
      case STANDING:
        if (mode != last_mode) {
          brutus.eyes_blue();
          brutus.set_motion_control_mode(POSE_CONTROL);
          last_mode = mode;
        }

        brutus.change_target_pose(STANDING_POSE);
        break;
      
      case LEG_CONTROL:
        if (mode != last_mode) {
          brutus.eyes_yellow();
          brutus.set_motion_control_mode(POSE_CONTROL);
          last_mode = mode;
        }

        brutus.change_target_pose(cmd.pose);
        break;

      case VEL_CONTROL:
        if (mode != last_mode) {
          brutus.eyes_green();
          brutus.set_motion_control_mode(SPEED_CONTROL);
          last_mode = mode;
        }

        brutus.set_linear_speed_ts(cmd.v);
        brutus.set_angular_speed_ts(cmd.w);
        break;

      case EXERCISE_1:
        if (mode != last_mode) {
          brutus.eyes_white();
          brutus.set_motion_control_mode(SPEED_CONTROL);
          last_mode = mode;
        }

        perception_data = brutus.get_perception_data();
        dist = perception_data.right_dist;

        error = exp_dist - dist;

        brutus.set_linear_speed_ts(1.0);
        brutus.set_angular_speed_ts(ex1_kp*error);

        break;

      case EXERCISE_2:
        if (mode != last_mode) {
          brutus.eyes_magenta();
          brutus.set_motion_control_mode(SPEED_CONTROL);
          last_mode = mode;
        }

        break;

      default:
        break;
    }

    uint32_t end_time = micros();
    uint32_t execution_time_us = end_time - start_time;
    Serial.print("[MODES] Tiempo de CPU activo: ");
    Serial.print(execution_time_us / 1000.0);
    Serial.println(" ms");

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MODES_PERIOD));
    
  }
}