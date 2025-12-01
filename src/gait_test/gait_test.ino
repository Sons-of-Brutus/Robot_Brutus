#include "Brutus.h"

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

TaskHandle_t TaskMov;

Brutus* brutus;

void
setup()
{
  brutus = new Brutus(&pca,
                      PCA9685_OE,
                      R_PIN,
                      B_PIN,
                      G_PIN,
                      PWM_SERVO_FREQ);

  brutus->setup_front_right_leg(SHOULDER_FRONT_RIGHT_PCA_PIN, ELBOW_FRONT_RIGHT_PCA_PIN,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_ANGLE, MIN_SERVO_ANGLE,
                              MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  brutus->setup_front_left_leg(SHOULDER_FRONT_LEFT_PCA_PIN, ELBOW_FRONT_LEFT_PCA_PIN,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_ANGLE, MIN_SERVO_ANGLE,
                              MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  brutus->setup_back_right_leg(SHOULDER_BACK_RIGHT_PCA_PIN, ELBOW_BACK_RIGHT_PCA_PIN,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_ANGLE, MIN_SERVO_ANGLE,
                              MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  brutus->setup_back_left_leg(SHOULDER_BACK_LEFT_PCA_PIN, ELBOW_BACK_LEFT_PCA_PIN,
                             MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                             MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                             MIN_SERVO_ANGLE, MIN_SERVO_ANGLE,
                             MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  brutus->start();
}

void loop() {}