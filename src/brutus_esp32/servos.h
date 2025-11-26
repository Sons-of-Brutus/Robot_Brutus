#ifndef SERVOS_H
#define SERVOS_H

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define PWM_SERVO_FREQ      50
#define PCA9685_RESOLUTION  4096
#define PWM_PERIOD_US       20000.0f

#define MIN_MG90S_ANGLE     0.0
#define MAX_MG90S_ANGLE     180.0

#define MIN_MG90S_PULSE_US  500
#define MAX_MG90S_PULSE_US  2500

#define N_SERVOS 8

uint16_t
us_to_pwm(float pulse_width_us)
{
  float counts = (pulse_width_us / PWM_PERIOD_US) * PCA9685_RESOLUTION;

  if (counts < 0) counts = 0;
  if (counts > (PCA9685_RESOLUTION - 1)) counts = PCA9685_RESOLUTION - 1;

  return (uint16_t)counts;
}

uint16_t
angle_to_pwm(float angle,
             float min_angle, float max_angle,
             uint16_t min_pwm, uint16_t max_pwm)
{
  if (angle < min_angle) angle = min_angle;
  if (angle > max_angle) angle = max_angle;

  float slope = (float)(max_pwm - min_pwm) / (max_angle - min_angle);

  return (uint16_t)(min_pwm + slope * (angle - min_angle));
}

float
pwm_to_angle(uint16_t pwm,
             uint16_t min_pwm, uint16_t max_pwm,
             float min_angle, float max_angle)
{
  if (pwm < min_pwm) pwm = min_pwm;
  if (pwm > max_pwm) pwm = max_pwm;

  float slope = (max_angle - min_angle) / (float)(max_pwm - min_pwm);

  return min_angle + slope * (pwm - min_pwm);
}

struct PcaServo {
  int pca_idx;        // PCA's pin where the servo is connected

  float min_angle;    // The lowest angle value that the servo can reach
  uint16_t min_pwm;   // PWM value correspondant to min_angle

  float max_angle;    // The highest angle value that the servo can reach
  uint16_t max_pwm;   // PWM value corresponant to max_angle

  uint16_t init_pwm;  // PWM value for the angle in the middle

  uint16_t pwm_offset; // PWM value for the servo's angle offset with the same 
                       // init_pwm as other servos with the same parameters
};

/*
PcaServo example:
pca_idx = 0
min_pwm = pwm equivalent to -π rad
max_pwm = pwm equivalent to π rad
init_pwm = pwm equivalent to 0 rad
*/

/**
* @brief Initializes a PcaServo struct
*
* @param s                PcaServo struct pointer
* @param pca_idx          PCA's pin where the servo is connected
* @param min_pwm_pulse_t  The lowest pwm pulse period that the servo can use  [unit: us]
* @param max_pwm_pulse_t  The highest pwm pulse period that the servo can use [unit: us]
*/
void
setup_pca_servo(PcaServo *s, 
                int pca_idx,
                int min_pwm_pulse_period,
                int max_pwm_pulse_period,
                float min_angle,
                float max_angle)
{
  s->pca_idx = pca_idx;

  s->min_pwm = us_to_pwm(min_pwm_pulse_period);
  s->max_pwm = us_to_pwm(max_pwm_pulse_period);

  s->min_angle = min_angle;
  s->max_angle = max_angle;

  float init_angle = (s->min_angle + s->max_angle) * 0.5f; 

  s->init_pwm = angle_to_pwm(init_angle, s->min_angle, s->max_angle, s->min_pwm, s->max_pwm);
}


#endif // SERVOS_H