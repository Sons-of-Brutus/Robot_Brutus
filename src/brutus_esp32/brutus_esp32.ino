#include "brutus.h"

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

BrutusLegInterface fr_leg = BrutusLegInterface(&pca);
BrutusLegInterface fl_leg = BrutusLegInterface(&pca);
BrutusLegInterface br_leg = BrutusLegInterface(&pca);
BrutusLegInterface bl_leg = BrutusLegInterface(&pca);

float angles[][2] = {{70, 120}, {70, 90}};
int i = 0;

Pca9685Servo shoulder = Pca9685Servo(&pca,
                        SHOULDER_FRONT_RIGHT_PCA_PIN,
                        MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                        MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);

void setup() {
  Serial.begin(9600);
  
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

void loop() {
  //BrutusLegState state = {angles[0][(i%2)], angles[1][(i%2)]};
  //fr_leg.set_leg_state(state);

  shoulder.set_angle(angles[0][(i%2)]);

  i++;

  delay(1000);
  Serial.print("  [FR] ");
  Serial.println(shoulder.get_angle());
}