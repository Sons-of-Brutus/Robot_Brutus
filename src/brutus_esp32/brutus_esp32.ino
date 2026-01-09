#include "Brutus.h"
#include "BrutusComms.h"

//EX1

#define W_EX1 0.45
#define V_EX1 1
#define NEAREST_DIST 13
#define FURTHER_DIST 16.0

#define INITIAL_SAMPLES 3
// STATES
#define PREPARE 0
#define START 1
#define FINISH 2

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

bool start_tasks = false;

void setup() {
  Serial.begin(BAUD);

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

  brutus_comms.start(&brutus);
  brutus.start();
  delay(100);

  //Serial.println("<START>");
  
  brutus.set_linear_speed_ts(0.0);
  brutus.set_angular_speed_ts(0.0);
  brutus.change_target_pose(STANDING_POSE);

  brutus.set_motion_control_mode(POSE_CONTROL);
  
  brutus.create_motion_task(DEFAULT_MOTION_PERIOD, MOTION_CORE);
  brutus.create_perception_task(PERCEPTION_PERIOD, LOGIC_CORE, PERCEPTION_PRIO);
  
  xTaskCreatePinnedToCore(
    (TaskFunction_t)mode_task,
    "ModeTask",
    MODES_TASK_SIZE,
    NULL,
    2,
    &mode_task_handle,
    LOGIC_CORE
  );

  brutus_comms.create_comms_task(COMMS_CORE); 
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
  

  BrutusPose pose = brutus.check_pose(true);

  float fr_elbow = 0.0;
  float fl_elbow = 0.0;
  float br_elbow = 0.0;
  float bl_elbow = 0.0;

  BrutusPose dbg_pose;

  float w = 0.0;

  float dist;

  bool is_right;
  int state_ex1 = PREPARE;
  int sign;
  int samples_taken = 0;
  float dists_right[INITIAL_SAMPLES];
  float dists_left[INITIAL_SAMPLES];
  float med_right_dist = 0;
  float med_left_dist = 0;
  while (true) {
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
          state_ex1 = PREPARE;
        }

        brutus.change_target_pose(STANDING_POSE);
        break;
      
      case LEG_CONTROL:
        if (mode != last_mode) {
          brutus.eyes_yellow();
          brutus.set_motion_control_mode(POSE_CONTROL);
          last_mode = mode;
          state_ex1 = PREPARE;
        }

        brutus.change_target_pose(interp_pose_norm(cmd.pose));
        break;

      case VEL_CONTROL:
        if (mode != last_mode) {
          brutus.eyes_green();
          brutus.set_motion_control_mode(SPEED_CONTROL);
          last_mode = mode;
          state_ex1 = PREPARE;
        }

        brutus.set_linear_speed_ts(cmd.v);
        brutus.set_angular_speed_ts(cmd.w);
        break;

      case EXERCISE_1:
        switch(state_ex1) {
          case PREPARE:
            if (mode != last_mode) {
              brutus.eyes_white();
              brutus.set_motion_control_mode(SPEED_CONTROL);
              last_mode = mode;
            }
            brutus.set_linear_speed_ts(V_EX1);

            perception_data = brutus.get_perception_data();

            dists_right[samples_taken] = perception_data.right_dist;
            dists_left[samples_taken] = perception_data.left_dist;

            if(is_right) {
              sign = 1;
            } else {
              sign = -1;
            }

            if (samples_taken >= INITIAL_SAMPLES) {
              med_right_dist = median(dists_right, INITIAL_SAMPLES);
              med_left_dist = median(dists_left, INITIAL_SAMPLES);

              is_right = (med_right_dist < med_left_dist);

              if(is_right) {
                sign = 1;
              } else {
                sign = -1;
              }
              samples_taken = 0;
              state_ex1 = START;
            } else {
              samples_taken++;
            }
            break;
          case START:
          perception_data = brutus.get_perception_data();
          if(is_right) {
              dist = perception_data.right_dist;
            } else {
              dist = perception_data.left_dist;
            }

            if (dist < NEAREST_DIST) {
              brutus.eyes_blue();
              brutus.set_angular_speed_ts(-W_EX1 * sign);
            } else if (dist > FURTHER_DIST) {
              brutus.eyes_red();
              brutus.set_angular_speed_ts(W_EX1 * sign);
            } else {
              brutus.eyes_magenta();
              brutus.set_angular_speed_ts(0);
            }

            if (perception_data.front_dist < FRONT_LIMIT_CM) {
              brutus.set_linear_speed_ts(0.0);
              brutus.set_angular_speed_ts(0.0);
              brutus.eyes_yellow();
            }
            brutus_comms.set_debug_float(dist);
          break;
        }
        break;

      case EXERCISE_2:
        if (mode != last_mode) {
          state_ex1 = PREPARE;
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

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MODES_PERIOD));
  }
}

float
calc_slope(float *values, int n_values)
{
  float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

  for (int i = 0; i < n_values; i++) {
    float x = i;
    sumX += x;
    sumY += values[i];
    sumXY += x * values[i];
    sumX2 += x * x;
  }

  float numerador = (n_values * sumXY) - (sumX * sumY);
  float denominador = (n_values * sumX2) - (sumX * sumX);
  
  return numerador / denominador;
}

float mapf_constrained(float x, float in_min, float in_max,
                       float out_min, float out_max)
{
    if (x < in_min) x = in_min;
    if (x > in_max) x = in_max;

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

BrutusPose interp_pose_norm(const BrutusPose& norm_pose)
{
  BrutusPose out;

  out.fr_leg_state.shoulder_angle = mapf_constrained(norm_pose.fr_leg_state.shoulder_angle, -1.0, 1.0,
                                                     MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);
  out.fr_leg_state.elbow_angle    = mapf_constrained(norm_pose.fr_leg_state.elbow_angle, -1.0, 1.0,
                                                     MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);

  out.fl_leg_state.shoulder_angle = mapf_constrained(norm_pose.fl_leg_state.shoulder_angle, -1.0, 1.0,
                                                     MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);
  out.fl_leg_state.elbow_angle    = mapf_constrained(norm_pose.fl_leg_state.elbow_angle, -1.0, 1.0,
                                                     MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);

  out.br_leg_state.shoulder_angle = mapf_constrained(norm_pose.br_leg_state.shoulder_angle, -1.0, 1.0,
                                                     MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);
  out.br_leg_state.elbow_angle    = mapf_constrained(norm_pose.br_leg_state.elbow_angle, -1.0, 1.0,
                                                     MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);

  out.bl_leg_state.shoulder_angle = mapf_constrained(norm_pose.bl_leg_state.shoulder_angle, -1.0, 1.0,
                                                     MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);
  out.bl_leg_state.elbow_angle    = mapf_constrained(norm_pose.bl_leg_state.elbow_angle, -1.0, 1.0,
                                                     MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);

  return out;
}

float median(float *values, int n)
{
  float tmp[n];

  for (int i = 0; i < n; i++) {
    tmp[i] = values[i];
  }

  for (int i = 1; i < n; i++) {
    float key = tmp[i];
    int j = i - 1;
    while (j >= 0 && tmp[j] > key) {
      tmp[j + 1] = tmp[j];
      j--;
    }
    tmp[j + 1] = key;
  }

  if (n % 2 == 0) {
    return 0.5f * (tmp[n/2 - 1] + tmp[n/2]);
  } else {
    return tmp[n/2];
  }
}
