#ifndef BRUTUS_LEG_INTERFACE__H
#define BRUTUS_LEG_INTERFACE__H

#include "Pca9685Servo.h"
#include "brutus_params.h"
#include "BrutusLegState.h"

#define N_SERVOS_AT_LEG 2

class BrutusLegInterface
{

private:
  Adafruit_PWMServoDriver* pca_;

  Pca9685Servo elbow_;
  Pca9685Servo shoulder_;

  bool shoulder_is_inverted_;
  bool elbow_is_inverted_;

  float
  apply_angle_inversion(bool is_inverted, float angle)
  {
    if (is_inverted) {
      angle = -angle;
    }

    return angle;
  }

public:
  BrutusLegInterface()
  : pca_(NULL),
    elbow_(),
    shoulder_()
  {}

  void
  setup(Adafruit_PWMServoDriver* pca)
  {
    this->pca_ = pca;
  }

  void
  setup_elbow(int pca_idx,
              int min_pwm_pulse_period,
              int max_pwm_pulse_period,
              float min_angle,
              float max_angle,
              bool is_inverted)
  {
    elbow_.init(this->pca_, pca_idx, min_pwm_pulse_period, max_pwm_pulse_period, min_angle, max_angle);
    this->elbow_is_inverted_ = is_inverted;
  }

  void
  setup_shoulder(int pca_idx,
                 int min_pwm_pulse_period,
                 int max_pwm_pulse_period,
                 float min_angle,
                 float max_angle,
                 bool is_inverted)
  {
    shoulder_.init(this->pca_, pca_idx, min_pwm_pulse_period, max_pwm_pulse_period, min_angle, max_angle);
    this->shoulder_is_inverted_ = is_inverted;
  }

  // TODO Needs test
  BrutusLegState
  get_leg_state(bool apply_inversion)
  {
    BrutusLegState leg_state;

    leg_state.shoulder_angle = shoulder_.get_angle();
    leg_state.elbow_angle = elbow_.get_angle();

    if (apply_inversion) {
      leg_state.shoulder_angle = apply_angle_inversion(shoulder_is_inverted_, leg_state.shoulder_angle);
      leg_state.elbow_angle = apply_angle_inversion(elbow_is_inverted_, leg_state.elbow_angle);
    }

    return leg_state;
  }

  void
  set_leg_state(BrutusLegState & leg_state, bool apply_inversion)
  {
    float shoulder_angle, elbow_angle;

    if (apply_inversion) {
      shoulder_angle = apply_angle_inversion(shoulder_is_inverted_, leg_state.shoulder_angle);
      elbow_angle = apply_angle_inversion(elbow_is_inverted_, leg_state.elbow_angle);
    } else {
      shoulder_angle = leg_state.shoulder_angle;
      elbow_angle = leg_state.elbow_angle;
    }

    shoulder_.set_angle(shoulder_angle);
    elbow_.set_angle(elbow_angle);
  }
};

#endif // BRUTUS_LEG_INTERFACE__H
