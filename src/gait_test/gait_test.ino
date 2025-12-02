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
}

void loop() {
  if (Serial.available() > 0)
  {
    String input = Serial.readStringUntil('\n');

    input.replace(",", " ");
    input.trim();

    float vals[N_LEGS * JOINTS_PER_LEG];
    int index = 0;

    char *token = strtok((char*)input.c_str(), " ");
    while (token != NULL && index < 8)
    {
      vals[index] = atof(token);
      index++;
      token = strtok(NULL, " ");
    }

    if (index == N_LEGS * JOINTS_PER_LEG)
    {
      BrutusPose new_pose;

      new_pose.fr_leg_state.shoulder_angle = vals[0];
      new_pose.fr_leg_state.elbow_angle    = vals[1];

      new_pose.fl_leg_state.shoulder_angle = vals[2];
      new_pose.fl_leg_state.elbow_angle    = vals[3];

      new_pose.br_leg_state.shoulder_angle = vals[4];
      new_pose.br_leg_state.elbow_angle    = vals[5];

      new_pose.bl_leg_state.shoulder_angle = vals[6];
      new_pose.bl_leg_state.elbow_angle    = vals[7];

      brutus.set_pose(new_pose);

      Serial.println("Nueva pose aplicada");
    }

    auto pose = brutus.check_pose(true);
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
  }

  delay(2000);
}