#ifndef BRUTUS_LEG_INTERFACE__H
#define BRUTUS_LEG_INTERFACE__H

#include "Pca9685Servo.h"
#include "brutus_params.h"
#include "BrutusLegState.h"

#define N_SERVOS_AT_LEG 2


/**
 * @class BrutusLegInterface
 * @brief Interface class to control a single Brutus robot leg.
 *
 * This class abstracts the control of a leg composed of two servos:
 * a shoulder servo and an elbow servo, both driven by a PCA9685 controller.
 * It provides methods to configure the servos, read the current leg state,
 * and command a desired leg state, optionally handling angle inversion.
 */
class BrutusLegInterface
{

private:
  /**
   * @brief Pointer to the PCA9685 PWM driver used by the leg servos.
   */
  Adafruit_PWMServoDriver* pca_;

  /**
   * @brief Elbow servo controller.
   */
  Pca9685Servo elbow_;

  /**
   * @brief Shoulder servo controller.
   */
  Pca9685Servo shoulder_;

  /**
   * @brief Indicates whether the shoulder servo angle is inverted.
   */
  bool shoulder_is_inverted_;

  /**
   * @brief Indicates whether the elbow servo angle is inverted.
   */
  bool elbow_is_inverted_;


  /**
   * @brief Applies angle inversion if required.
   *
   * @param is_inverted True if the angle must be inverted.
   * @param angle Input angle in radians or degrees (depending on configuration).
   * @return float The resulting angle after optional inversion.
   */
  float
  apply_angle_inversion(bool is_inverted, float angle)
  {
    if (is_inverted) {
      angle = -angle;
    }

    return angle;
  }

public:
  /**
   * @brief Default constructor.
   *
   * Initializes the leg's interface with no PCA driver attached.
   */
  BrutusLegInterface()
  : pca_(NULL),
    elbow_(),
    shoulder_()
  {}

  /**
   * @brief Assigns the PCA9685 driver to the leg interface.
   *
   * @param pca Pointer to an initialized Adafruit_PWMServoDriver.
   */
  void
  setup(Adafruit_PWMServoDriver* pca)
  {
    this->pca_ = pca;
  }

  /**
   * @brief Configures the ELBOW servo.
   *
   * @param pca_idx PCA9685 channel/pin number where the servo is connected.
   * @param min_pwm_pulse_period Minimum PWM pulse period. [ms]
   * @param max_pwm_pulse_period Maximum PWM pulse width. [ms]
   * @param min_angle Minimum allowed servo angle. [degrees]
   * @param max_angle Maximum allowed servo angle. [degrees]
   * @param is_inverted True if the servo rotation direction must be inverted.
   */
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

  /**
   * @brief Configures the SHOULDER servo.
   *
   * @param pca_idx PCA9685 channel/pin number where the servo is connected.
   * @param min_pwm_pulse_period Minimum PWM pulse period. [ms]
   * @param max_pwm_pulse_period Maximum PWM pulse width. [ms]
   * @param min_angle Minimum allowed servo angle. [degrees]
   * @param max_angle Maximum allowed servo angle. [degrees]
   * @param is_inverted True if the servo rotation direction must be inverted.
   */
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

  /**
   * @brief Returns the current leg state.
   *
   * Reads the current angles of the shoulder and elbow servos.
   *
   * @param apply_inversion If true, angle inversion is applied before returning.
   * @return BrutusLegState struct containing the current leg state.
   */
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

  /**
   * @brief Commands the leg's servos a desired leg state angles.
   *
   * @param leg_state Desired leg state containing shoulder and elbow angles.
   * @param apply_inversion If true, angle inversion is applied before commanding.
   */
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

    elbow_.set_angle(elbow_angle);
    shoulder_.set_angle(shoulder_angle);
  }
};

#endif // BRUTUS_LEG_INTERFACE__H
