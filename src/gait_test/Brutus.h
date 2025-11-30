#ifndef BRUTUS__H
#define BRUTUS__H

#include "Pca9685Servo.h"
#include "BrutusLegInterface.h"
#include "brutus_params.h"

struct BrutusPose {
  BrutusLegState & fr_leg_state;
  BrutusLegState & fl_leg_state;
  BrutusLegState & br_leg_state;
  BrutusLegState & bl_leg_state;

  BrutusPose(
    BrutusLegState & fr,
    BrutusLegState & fl,
    BrutusLegState & br,
    BrutusLegState & bl)
    : fr_leg_state(fr), fl_leg_state(fl),
      br_leg_state(br), bl_leg_state(bl)
  {}
};
typedef struct BrutusPose BrutusPose;

class Brutus {

private:
  Adafruit_PWMServoDriver* pca;
  SemaphoreHandle_t motion_mutex;
  BrutusPose target_pose;

  BrutusLegInterface front_right_leg;
  BrutusLegInterface back_right_leg;
  BrutusLegInterface front_left_leg;
  BrutusLegInterface back_left_leg;

  float lin_speed = 0;
  float ang_speed = 0;

public:
  Brutus(Adafruit_PWMServoDriver* pca,
         int pca_oe_pin,
         int eyes_r_pin,
         int eyes_b_pin,
         int eyes_g_pin,
         int pwm_servo_frequency)
  {
    pinMode(eyes_r_pin, OUTPUT);
    pinMode(eyes_b_pin, OUTPUT);
    pinMode(eyes_g_pin, OUTPUT);

    this->eyes_yellow();

    this->pca = pca;
    this->motion_mutex = xSemaphoreCreateMutex();

    pinMode(pca_oe_pin, INPUT_PULLDOWN); 
    pinMode(pca_oe_pin, OUTPUT);

    pca->begin();
    pca->setPWMFreq(pwm_servo_frequency);
    delay(100);

    this->eyes_green();

    front_right_leg = BrutusLegInterface(pca);
    back_right_leg = BrutusLegInterface(pca);
    front_left_leg = BrutusLegInterface(pca);
    back_left_leg = BrutusLegInterface(pca);

    delay(100);

    this->eyes_blue();

    activate_motors();
    delay(100);

    this->eyes_red();

    this->eyes_magenta();
    
  }

  void
  setup_front_right_leg(int shoulder_pca_pin, int elbow_pca_pin,
                        int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                        int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                        int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                        int elbow_min_servo_angle, int elbow_max_servo_angle)
  {
    this->front_right_leg.setup_shoulder(shoulder_pca_pin,
                                         shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
                                         shoulder_min_servo_angle, shoulder_max_servo_angle);
  }

  // -------- PCA9685 --------

  void
  activate_motors()
  {
    digitalWrite(PCA9685_OE, LOW);
  }

  void
  deactivate_motors()
  {
    digitalWrite(PCA9685_OE, HIGH);
  }

  // -------- Eyes color --------

  void
  eyes_red()
  {
    digitalWrite(R_PIN, HIGH);
    digitalWrite(B_PIN, LOW);
    digitalWrite(G_PIN, LOW);
  }

  void
  eyes_blue()
  {
    digitalWrite(R_PIN, LOW);
    digitalWrite(B_PIN, HIGH);
    digitalWrite(G_PIN, LOW);
  }

  void
  eyes_green()
  {
    digitalWrite(R_PIN, LOW);
    digitalWrite(B_PIN, LOW);
    digitalWrite(G_PIN, HIGH);
  }

  void
  eyes_magenta()
  {
    digitalWrite(R_PIN, HIGH);
    digitalWrite(B_PIN, HIGH);
    digitalWrite(G_PIN, LOW);
  }

  void
  eyes_cyan()
  {
    digitalWrite(R_PIN, LOW);
    digitalWrite(B_PIN, HIGH);
    digitalWrite(G_PIN, HIGH);
  }

  void
  eyes_yellow()
  {
    digitalWrite(R_PIN, HIGH);
    digitalWrite(B_PIN, LOW);
    digitalWrite(G_PIN, HIGH);
  }

  void
  eyes_white()
  {
    digitalWrite(R_PIN, HIGH);
    digitalWrite(B_PIN, HIGH);
    digitalWrite(G_PIN, HIGH);
  }

  void
  eyes_off()
  {
    digitalWrite(R_PIN, LOW);
    digitalWrite(B_PIN, LOW);
    digitalWrite(G_PIN, LOW);
  }

  // ---------- MOTION -------------
  void
  motion_task(void *pvParameters)
  {
    
  }

  const BrutusLegState standing_fr_state = {};
  const BrutusLegState standing_fl_state = {};
  const BrutusLegState standing_br_state = {};
  const BrutusLegState standing_bl_state = {};
  const BrutusPose standing_pose;
};




#endif // BRUTUS__H