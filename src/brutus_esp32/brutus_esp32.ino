#include "Brutus.h"
#include "BrutusComms.h"

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

Brutus brutus;
BrutusComms brutus_comms;
TaskHandle_t mqtt_task_handle;

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

  delay(3000);

  brutus.start();
  brutus_comms.start(&brutus);
  delay(100);

  Serial.println("<START>");
  
  brutus.create_motion_task(MOTION_PERIOD, MOTION_CORE);
  brutus_comms.create_comms_task(COMMS_CORE);
}

void loop() {}