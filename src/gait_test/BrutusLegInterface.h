#ifndef BRUTUS_LEG_INTERFACE__H
#define BRUTUS_LEG_INTERFACE__H

#include "Pca9685Servo.h"
#include "brutus_params.h"
#include "BrutusLegState.h"

#define N_SERVOS_AT_LEG 2


class BrutusLegInterface
{

private:
  Pca9685Servo elbow;
  Pca9685Servo shoulder;

  Adafruit_PWMServoDriver* pca;

public:
  BrutusLegInterface(Adafruit_PWMServoDriver* pca)
  : pca(pca),
    elbow(pca,0,0,0,0,0),
    shoulder(pca,0,0,0,0,0)
  {}

  void
  setup_elbow(int pca_idx,
              int min_pwm_pulse_period,
              int max_pwm_pulse_period,
              float min_angle,
              float max_angle)
  {
    elbow = Pca9685Servo(this->pca, pca_idx, min_pwm_pulse_period, max_pwm_pulse_period, min_angle, max_angle);
  }

  void
  setup_shoulder(int pca_idx,
                 int min_pwm_pulse_period,
                 int max_pwm_pulse_period,
                 float min_angle,
                 float max_angle)
  {
    shoulder = Pca9685Servo(this->pca, pca_idx, min_pwm_pulse_period, max_pwm_pulse_period, min_angle, max_angle);
  }

  struct BrutusLegState
  get_leg_state()
  {
    struct BrutusLegState leg_state;

    leg_state.shoulder_angle = shoulder.get_angle();
    leg_state.elbow_angle = elbow.get_angle();

    return leg_state;
  }

  void
  set_leg_state(BrutusLegState & leg_state)
  {
    shoulder.set_angle(leg_state.shoulder_angle);
    elbow.set_angle(leg_state.elbow_angle);
  }
};

#endif // BRUTUS_LEG_INTERFACE__H