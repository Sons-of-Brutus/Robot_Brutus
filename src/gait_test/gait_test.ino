#include "Brutus.h"

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

Brutus brutus;

void
setup()
{
  Serial.begin(9600);

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
                              MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);

  brutus.setup_front_left_leg(SHOULDER_FRONT_LEFT_PCA_PIN, ELBOW_FRONT_LEFT_PCA_PIN,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                              MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);

  brutus.setup_back_right_leg(SHOULDER_BACK_RIGHT_PCA_PIN, ELBOW_BACK_RIGHT_PCA_PIN,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                              MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                              MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);

  brutus.setup_back_left_leg(SHOULDER_BACK_LEFT_PCA_PIN, ELBOW_BACK_LEFT_PCA_PIN,
                             MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                             MIN_SERVO_PWM_PULSE_US, MAX_SERVO_PWM_PULSE_US,
                             MIN_SERVO_ANGLE, MAX_SERVO_ANGLE,
                             MIN_SERVO_ANGLE, MAX_SERVO_ANGLE);

  delay(3000);

  brutus.start();
}

void loop() {
  auto pose = brutus.check_pose();
  Serial.println("Pose:");

  Serial.print("  [FR] ");
  Serial.print(pose.fr_leg_state.shoulder_angle);
  Serial.print(" ");
  Serial.println(pose.fr_leg_state.elbow_angle);

  Serial.print("  [FL] ");
  Serial.print(pose.fl_leg_state.shoulder_angle);
  Serial.print(" ");
  Serial.println(pose.fl_leg_state.elbow_angle);

  Serial.print("  [BR] ");
  Serial.print(pose.br_leg_state.shoulder_angle);
  Serial.print(" ");
  Serial.println(pose.br_leg_state.elbow_angle);

  Serial.print("  [BL] ");
  Serial.print(pose.bl_leg_state.shoulder_angle);
  Serial.print(" ");
  Serial.println(pose.bl_leg_state.elbow_angle);

  delay(1000);
}