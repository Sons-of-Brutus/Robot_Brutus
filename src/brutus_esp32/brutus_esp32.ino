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

  //Serial.println("<START>");
  
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

  float ex1_kp = 0.20;
  float exp_dist = 40.0;
  float error = 0.0;
  float dist = 0.0;
  float last_dist = 0.0;
  float last_last_dist = 0.0;

  BrutusPose pose = brutus.check_pose(true);
  int i = 0;

  float fr_elbow = 0.0;
  float fl_elbow = 0.0;
  float br_elbow = 0.0;
  float bl_elbow = 0.0;


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

        fr_elbow = pose.fr_leg_state.elbow_angle;
        fl_elbow = pose.fl_leg_state.elbow_angle;
        br_elbow = pose.br_leg_state.elbow_angle;
        bl_elbow = pose.bl_leg_state.elbow_angle;


        perception_data = brutus.get_perception_data();
        dist = perception_data.right_dist;

        if (i < 1){
          last_dist = dist;
          last_last_dist = dist;
          i++;
        }


        if (dist >= last_dist+10){
          dist = last_dist;
        }

        error = dist - exp_dist;

        //if (abs(ELBOW_DOWN-(fr_elbow - ELBOW_FRONT_RIGHT_ANGLE_OFFSET)) <= 2.5 && abs(ELBOW_DOWN-(fl_elbow - ELBOW_FRONT_LEFT_ANGLE_OFFSET)) <= 2.5 && abs(ELBOW_DOWN-(br_elbow - ELBOW_BACK_RIGHT_ANGLE_OFFSET)) <= 2.5 && abs(ELBOW_DOWN-(bl_elbow - ELBOW_BACK_LEFT_ANGLE_OFFSET)) <= 2.5){
        if (abs(0) <= 2.5 && abs(ELBOW_DOWN-(fl_elbow - ELBOW_FRONT_LEFT_ANGLE_OFFSET)) <= 2.5 && abs(ELBOW_DOWN-(br_elbow - ELBOW_BACK_RIGHT_ANGLE_OFFSET)) <= 2.5 && abs(ELBOW_DOWN-(bl_elbow - ELBOW_BACK_LEFT_ANGLE_OFFSET)) <= 2.5){
          //brutus.set_angular_speed_ts(ex1_kp*error);
          //brutus_comms.publish_w(ex1_kp*error);
          Serial.print(ex1_kp*error);
          Serial.println(" cm");
        }
        


        //brutus.set_linear_speed_ts(1.0);

        last_last_dist = last_dist;
        last_dist = dist;
        
        

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