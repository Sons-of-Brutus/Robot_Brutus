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

  int EX1_ITERATIONS = 3;
  int EX1_TURN_ITERATIONS = 2;
  int N_MEASUREMENTS = 10;
  float DIST_DIFF_THRESHOLD = 20.0;
  float EX1_W = 0.2;
  float EX1_STRAIGHT_THRESHOLD = 3.0;

  //float EX1_KP = 3.33;
  //float EX1_EXP_DIST = 30.0;  

  float error = 0.0;
  float distance_errors[N_MEASUREMENTS];
  float errors_m = 0.0;
  float last_errors_m = 0.0;
  int w_sign = 0;

  float dist = 0.0;
  float right_distances[N_MEASUREMENTS];
  float left_distances[N_MEASUREMENTS];
  float mean_dist = 0.0;

  bool must_sum_iterations = true;
  int ex1_iterations = 0;
  float last_dist = 0.0;
  float exp_dist;
  float right_mean = 0.0;
  float left_mean = 0.0;
  bool right_side = true;
  

  BrutusPose pose = brutus.check_pose(true);
  int i = 0;

  float fr_elbow = 0.0;
  float fl_elbow = 0.0;
  float br_elbow = 0.0;
  float bl_elbow = 0.0;

  BrutusPose dbg_pose;

  float w = 0.0;

  while (true) {
    //Serial.println("<MODES>");
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

        brutus.change_target_pose(interp_pose_norm(cmd.pose));
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
          i = 0;

          w = 0.0;
          w_sign = 0;
          
          //last_errors_m = 0.0;
          //errors_m = 0.0;
          
          must_sum_iterations = true;
          ex1_iterations = 0;

          right_mean = 0.0;
          left_mean = 0.0;
        }

        perception_data = brutus.get_perception_data();

        if (ex1_iterations < N_MEASUREMENTS) {
          right_distances[ex1_iterations] = perception_data.right_dist;
          //left_distances[ex1_iterations] = perception_data.left_dist;
        } else if (ex1_iterations == N_MEASUREMENTS) {
          right_mean = median(right_distances, N_MEASUREMENTS);
          brutus_comms.set_debug_float_2(right_distances[0]);
          brutus_comms.set_debug_float_2(right_distances[1]);
          brutus_comms.set_debug_float_2(right_distances[2]);
          brutus_comms.set_debug_float_2(right_distances[3]);
          brutus_comms.set_debug_float_2(right_distances[4]);
          brutus_comms.set_debug_float_2(right_distances[5]);
          brutus_comms.set_debug_float_2(right_distances[6]);
          brutus_comms.set_debug_float_2(right_distances[7]);
          brutus_comms.set_debug_float_2(right_distances[8]);
          brutus_comms.set_debug_float_2(right_distances[9]);
          brutus_comms.set_debug_float(right_mean);
          //left_mean = median(left_distances, N_MEASUREMENTS);

          //if (right_mean < left_mean) {
          //  exp_dist = right_mean;
          //  right_side = true;
          //} else {
          //  exp_dist = left_mean;
          //  right_side = false;
          //}

          right_side = true;
          exp_dist = right_mean;

          last_dist = exp_dist;
        }
        
        if (ex1_iterations%EX1_ITERATIONS == EX1_TURN_ITERATIONS) {
          w = 0.0;
        }

        pose = brutus.check_pose(true);
        fr_elbow = pose.fr_leg_state.elbow_angle;
        fl_elbow = pose.fl_leg_state.elbow_angle;
        br_elbow = pose.br_leg_state.elbow_angle;
        bl_elbow = pose.bl_leg_state.elbow_angle;

        if (right_side) {
          dist = perception_data.right_dist;
        } else {
          dist = perception_data.left_dist;
        }

        if (fabs(dist - last_dist) > DIST_DIFF_THRESHOLD) {
          dist = last_dist;
        } else {
          last_dist = dist;
        }

        if (ex1_iterations != 0 && (ex1_iterations%EX1_ITERATIONS == 0) && fabs(ELBOW_DOWN-(fr_elbow - ELBOW_FRONT_RIGHT_ANGLE_OFFSET)) <= MIN_ELBOW_DIFF && fabs(ELBOW_DOWN-(fl_elbow - ELBOW_FRONT_LEFT_ANGLE_OFFSET)) <= MIN_ELBOW_DIFF && fabs(ELBOW_DOWN-(br_elbow - ELBOW_BACK_RIGHT_ANGLE_OFFSET)) <= MIN_ELBOW_DIFF && fabs(ELBOW_DOWN-(bl_elbow - ELBOW_BACK_LEFT_ANGLE_OFFSET)) <= MIN_ELBOW_DIFF) {
          error = dist - exp_dist;

          if (fabs(error) < EX1_STRAIGHT_THRESHOLD) {
            w_sign = 0;
          } else if (dist < exp_dist) {
            if (right_side) {
              w_sign = -1;
            } else {
              w_sign = 1;
            }
            
          } else if (dist > exp_dist) {
            if (right_side) {
              w_sign = 1;
            } else {
              w_sign = -1;
            }
          }

          w = EX1_W * w_sign;

          /*
          if (i > 0) {
            mean_dist = 0.0;

            for (int k = 0; k < N_MEASUREMENTS; k++) {
              mean_dist += distances[k];
            }

            mean_dist = mean_dist/((float)N_MEASUREMENTS);
          } else {
            mean_dist = dist;
          }

          error = dist - mean_dist;

          error = dist - distances[(i-1)%N_MEASUREMENTS];

          if (i < N_MEASUREMENTS) {
            for (int k = N_MEASUREMENTS-1; k > i; k--) {
              distances[k] = dist;
            }
          } else {
            distances[i%N_MEASUREMENTS] = dist;
          }

          //if (fabs(fabs(error) - distance_errors[(i-1)%N_MEASUREMENTS]) > 10){
          //  error = distance_errors[(i-1)%N_MEASUREMENTS];
          //}
          
          distance_errors[i%N_MEASUREMENTS] = fabs(error);

          errors_m = calc_slope(distance_errors, N_MEASUREMENTS);
          errors_m = mapf_constrained(errors_m, -100.0, 100.0, MIN_W, MAX_W);


          if (i%N_MEASUREMENTS == N_MEASUREMENTS-1) {
            //Serial.println("KLK");
            if (errors_m > last_errors_m) {
              //Serial.print("Changing direction: ");
              w_sign *= -1;
              //Serial.println(w_sign);
            }
            last_errors_m = errors_m;
            
          }

          i++;

          brutus.eyes_white();
          */

          ex1_iterations++;

        }

        if (ex1_iterations >= N_MEASUREMENTS) {
          if (w_sign < 0) {
            brutus.eyes_blue();
          } else if (w_sign > 0) {
            brutus.eyes_red();
          } else {
            brutus.eyes_magenta();
          }

          brutus.set_angular_speed_ts(w);
          brutus.set_linear_speed_ts(1.0);
        } else {
          brutus.set_angular_speed_ts(0.0);
          brutus.set_linear_speed_ts(0.0);
        }

        //brutus_comms.set_debug_float((float)ex1_iterations);
        brutus_comms.set_debug_float_2(dist);
        brutus_comms.set_debug_float_2(exp_dist);

        if (ex1_iterations != EX1_ITERATIONS) {
          ex1_iterations++;
        }

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
    //Serial.print("[MODES] Tiempo de CPU activo: ");
    //Serial.print(execution_time_us / 1000.0);
    //Serial.println(" ms");

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

  // Copiar valores
  for (int i = 0; i < n; i++) {
    tmp[i] = values[i];
  }

  // Ordenación simple (inserción)
  for (int i = 1; i < n; i++) {
    float key = tmp[i];
    int j = i - 1;
    while (j >= 0 && tmp[j] > key) {
      tmp[j + 1] = tmp[j];
      j--;
    }
    tmp[j + 1] = key;
  }

  // Mediana
  if (n % 2 == 0) {
    return 0.5f * (tmp[n/2 - 1] + tmp[n/2]);
  } else {
    return tmp[n/2];
  }
}
