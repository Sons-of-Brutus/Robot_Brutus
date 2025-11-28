#include "brutus.h"

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

BrutusLegInterface fr_leg = BrutusLegInterface(&pca);
BrutusLegInterface fl_leg = BrutusLegInterface(&pca);
BrutusLegInterface br_leg = BrutusLegInterface(&pca);
BrutusLegInterface bl_leg = BrutusLegInterface(&pca);

void setup() {
  // Setup brutus
  init_brutus(&pca);

  // Setup front right leg
  fr_leg.setup_shoulder(SHOULDER_FRONT_RIGHT_PCA_PIN,
                        MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                        MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  fr_leg.setup_elbow(SHOULDER_FRONT_RIGHT_PCA_PIN,
                     MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                     MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  // Setup front left leg
  fl_leg.setup_shoulder(SHOULDER_FRONT_LEFT_PCA_PIN,
                        MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                        MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  fl_leg.setup_elbow(SHOULDER_FRONT_LEFT_PCA_PIN,
                     MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                     MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  // Setup back right leg
  br_leg.setup_shoulder(SHOULDER_BACK_RIGHT_PCA_PIN,
                        MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                        MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  br_leg.setup_elbow(SHOULDER_BACK_RIGHT_PCA_PIN,
                     MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                     MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  // Setup back left leg
  bl_leg.setup_shoulder(SHOULDER_BACK_LEFT_PCA_PIN,
                        MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                        MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);

  bl_leg.setup_elbow(SHOULDER_BACK_LEFT_PCA_PIN,
                     MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                     MIN_SERVO_ANGLE, MIN_SERVO_ANGLE);
}

void loop() {}