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

  Adafruit_PWMServoDriver* pca;

public:
  BrutusLegController(Adafruit_PWMServoDriver* pca)
  {
    this->pca = pca;
  }

  void setup_elbow(int pca_idx,
                   int min_pwm_pulse_period,
                   int max_pwm_pulse_period,
                   float min_angle,
                   float max_angle)
  {
    elbow = Pca9685Servo(this->pca, pca_idx, min_pwm_pulse_period, max_pwm_pulse_period, min_angle, max_angle);
  }

  void setup_shoulder(int pca_idx,
                      int min_pwm_pulse_period,
                      int max_pwm_pulse_period,
                      float min_angle,
                      float max_angle)
  {
    shoulder = Pca9685Servo(this->pca, pca_idx, min_pwm_pulse_period, max_pwm_pulse_period, min_angle, max_angle);
  }



}

#endif // BRUTUS_LEG_CONTROLLER__H