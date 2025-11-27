#ifndef BRUTUS_LEG_CONTROLLER__H
#define BRUTUS_LEG_CONTROLLER__H

#include "Pca9685Servo.h"
#include "brutus_params.h"

#define N_SERVOS_AT_LEG 2

class BrutusLegController
{

private:
  Pca9685Servo elbow;
  Pca9685Servo shoulder;

  float elbow_offset;
  float shoulder_offset;

public:
  BrutusLegController()
  {
  }

  void setup_elbow(Adafruit_PWMServoDriver* pca,
                   int pca_idx,
                   int min_pwm_pulse_period,
                   int max_pwm_pulse_period,
                   float min_angle,
                   float max_angle.
                   float angle_offset)
  {
    elbow = Pca9685Servo(pca, pca_idx, min_pwm_pulse_period, max_pwm_pulse_period, min_angle, max_angle);
    elbow_offset = angle_offset;
  }

  void setup_shoulder(Adafruit_PWMServoDriver* pca,
                      int pca_idx,
                      int min_pwm_pulse_period,
                      int max_pwm_pulse_period,
                      float min_angle,
                      float max_angle.
                      float angle_offset)
  {
    shoulder = Pca9685Servo(pca, pca_idx, min_pwm_pulse_period, max_pwm_pulse_period, min_angle, max_angle);
    shoulder_offset = angle_offset;
  }

}

#endif // BRUTUS_LEG_CONTROLLER__H