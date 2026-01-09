/**
* @file Pca9685Servo.h
* @brief Definition of the Pca9685Servo class.
*/

#ifndef PCA_9685_SERVO__H
#define PCA_9685_SERVO__H

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define PCA9685_RESOLUTION  4096
#define PWM_PERIOD_US       20000.0f

class Pca9685Servo {

private:
  Adafruit_PWMServoDriver* pca; // PCA's controller object reference
  int pca_idx;                  // PCA's pin where the servo is connected

  float min_angle;              // The lowest angle value that the servo can reach
  uint16_t min_pwm;             // PWM value correspondant to min_angle

  float max_angle;              // The highest angle value that the servo can reach
  uint16_t max_pwm;             // PWM value corresponant to max_angle

  uint16_t init_pwm;            // PWM value for the angle in the middle

public:

  /**
   * @brief Constructor
   */
  Pca9685Servo() {}

  /**
    * @brief Initializes a Pca9685Servo object with the given parameters.
    *
    * This method sets up the servo to use the specified PCA9685 controller,
    * configures the PWM pin, pulse width limits, and angle limits. It also
    * calculates the initial PWM value corresponding to the midpoint angle.
    *
    * @param pca Pointer to the Adafruit_PWMServoDriver instance controlling the PCA9685.
    * @param pca_idx The PCA9685 channel/pin number where the servo is connected.
    * @param min_pwm_pulse_period Minimum PWM pulse width in microseconds for the servo.
    * @param max_pwm_pulse_period Maximum PWM pulse width in microseconds for the servo.
    * @param min_angle Minimum angle the servo can reach (degrees).
    * @param max_angle Maximum angle the servo can reach (degrees).
    */
    void 
    init(Adafruit_PWMServoDriver* pca,
         int pca_idx,
         int min_pwm_pulse_period,
         int max_pwm_pulse_period,
         float min_angle,
         float max_angle)
    {
      this->pca = pca;
      this->pca_idx = pca_idx;

      this->min_pwm = Pca9685Servo::us_to_pwm(min_pwm_pulse_period);
      this->max_pwm = Pca9685Servo::us_to_pwm(max_pwm_pulse_period);

      this->min_angle = min_angle;
      this->max_angle = max_angle;

      float init_angle = (this->min_angle + this->max_angle) * 0.5f; 

      this->init_pwm = Pca9685Servo::angle_to_pwm(init_angle, min_angle, max_angle, this->min_pwm, this->max_pwm);
    }

  /**
   * @brief Sends a PWM pulse corresponding to an angle to the specified servo
   * 
   * @param angle   Desired angle, normalized to [this->min_angle, this->max_angle]
   * @param on_tick The tick when the PCA send the pulse to the servo
   */
  void set_angle(float angle,
                 uint16_t on_tick = 0)
  {
    float ang = this->normalize_to_servo_range(angle);

    uint16_t pwm_value = Pca9685Servo::angle_to_pwm(ang,
                                                    this->min_angle,
                                                    this->max_angle,
                                                    this->min_pwm,
                                                    this->max_pwm);
    this->pca->setPWM(this->pca_idx,
                      on_tick,
                      pwm_value);
  }

  /**
   * @return Actual servo's angle.
   */
  float
  get_angle()
  {
    uint16_t on_tick = this->pca->getPWM(this->pca_idx, false);
    uint16_t off_tick = this->pca->getPWM(this->pca_idx, true);

    uint16_t pwm_val;
    if (off_tick >= on_tick) {
      pwm_val = off_tick - on_tick;
    } else {
      pwm_val = off_tick + 4096 - on_tick;
    }

    float angle = Pca9685Servo::pwm_to_angle(pwm_val,
                                             this->min_pwm,
                                             this->max_pwm,
                                             this->min_angle,
                                             this->max_angle);

    return angle;
  }

  /**
   * @brief Wraps an angle into the servo's valid range.
   *
   * Normalizes the input angle so it fits within [min_angle, max_angle].
   *
   * @param angle Input angle in degrees.
   * @return float Normalized angle within the servo range.
   */
  float
  normalize_to_servo_range(float angle) {
    float range = this->max_angle - this->min_angle;
    float a = fmod(angle - this->min_angle, range);
    if (a < 0) a += range;
    return a + this->min_angle;
  }

  /**
   * @brief Converts a pulse width in microseconds to a PWM value for the PCA9685.
   *
   * The PCA9685 uses a 12-bit resolution (0-4095) to generate the PWM signal.
   * This function converts a pulse width in microseconds to the corresponding
   * register value to send to the PCA9685.
   *
   * @param pulse_width_us Desired pulse width in microseconds (us).
   * @return uint16_t PWM value corresponding to the pulse width, clamped to [0, 4095].
   *
   * @note Values below 0 are clamped to 0.
   * @note Values above 4095 are clamped to the maximum allowed.
   */
  static uint16_t
  us_to_pwm(float pulse_width_us)
  {
    float counts = (pulse_width_us / PWM_PERIOD_US) * PCA9685_RESOLUTION;

    if (counts < 0) counts = 0;
    if (counts > (PCA9685_RESOLUTION - 1)) counts = PCA9685_RESOLUTION - 1;

    return (uint16_t)counts;
  }

  /**
   * @brief Converts an angle to a PWM value for the PCA9685.
   *
   * This function maps the servo angle linearly to the corresponding PWM value.
   * It is useful to send position commands in human-understandable units (degrees).
   *
   * @param angle Desired angle for the servo.
   * @param min_angle Minimum achievable angle of the servo.
   * @param max_angle Maximum achievable angle of the servo.
   * @param min_pwm PWM value corresponding to the minimum angle.
   * @param max_pwm PWM value corresponding to the maximum angle.
   * @return uint16_t PWM value corresponding to the desired angle.
   *
   * @note Angles below min_angle are clamped to min_angle.
   * @note Angles above max_angle are clamped to max_angle.
   * @note The angle-to-PWM mapping is linear.
   */
  static uint16_t
  angle_to_pwm(float angle,
               float min_angle, float max_angle,
               uint16_t min_pwm, uint16_t max_pwm)
  {
    if (angle < min_angle) angle = min_angle;
    if (angle > max_angle) angle = max_angle;

    float slope = (float)(max_pwm - min_pwm) / (max_angle - min_angle);

    return (uint16_t)(min_pwm + slope * (angle - min_angle));
  }

  /**
   * @brief Converts a PWM value from the PCA9685 back to the corresponding angle.
   *
   * This is the inverse of angle_to_pwm, allowing you to estimate the actual
   * servo angle from a PWM value sent or read.
   *
   * @param pwm Current PWM value.
   * @param min_pwm PWM value corresponding to the minimum angle.
   * @param max_pwm PWM value corresponding to the maximum angle.
   * @param min_angle Minimum achievable angle of the servo.
   * @param max_angle Maximum achievable angle of the servo.
   * @return float Angle corresponding to the provided PWM value.
   *
   * @note PWM values below min_pwm are clamped to min_pwm.
   * @note PWM values above max_pwm are clamped to max_pwm.
   * @note The PWM-to-angle mapping is linear.
   */
  static float
  pwm_to_angle(uint16_t pwm,
               uint16_t min_pwm, uint16_t max_pwm,
               float min_angle, float max_angle)
  {
    if (pwm < min_pwm) pwm = min_pwm;
    if (pwm > max_pwm) pwm = max_pwm;

    float slope = (max_angle - min_angle) / (float)(max_pwm - min_pwm);

    return min_angle + slope * (pwm - min_pwm);
  }

};

#endif // PCA_9685_SERVO__H