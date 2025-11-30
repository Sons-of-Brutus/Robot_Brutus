#ifndef BRUTUS__H
#define BRUTUS__H

#include "brutus_params.h"
#include "BrutusLegInterface.h"
#include "BrutusPose.h"

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

  bool brutus_is_setup = false;

  bool fr_leg_is_setup = false;
  bool fl_leg_is_setup = false;
  bool br_leg_is_setup = false;
  bool bl_leg_is_setup = false;

  bool pca_active = false;

  BrutusPose standing_pose = STANDING_POSE;

public:
  Brutus(Adafruit_PWMServoDriver* pca,
         int pca_oe_pin,
         int eyes_r_pin,
         int eyes_b_pin,
         int eyes_g_pin,
         int pwm_servo_frequency)
  : front_right_leg(pca),
    back_right_leg(pca),
    front_left_leg(pca),
    back_left_leg(pca)
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

    delay(100);

    this->eyes_blue();

    activate_motors();
    delay(100);

    pca_active = true;
    brutus_is_setup = false;
  }

  void
  start()
  {
    if (pca_active && fr_leg_is_setup && fl_leg_is_setup && br_leg_is_setup && bl_leg_is_setup) {
      brutus_is_setup = true;
    }

    if (brutus_is_setup) {
      this->set_pose(standing_pose);
      this->eyes_magenta();
    } else {
      this->eyes_red();
    }
  }

  // --------- LEGS -----------

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

    this->front_right_leg.setup_elbow(elbow_pca_pin,
                                      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
                                      elbow_min_servo_angle, elbow_max_servo_angle);

    fr_leg_is_setup = true;
  }

  void
  setup_front_left_leg(int shoulder_pca_pin, int elbow_pca_pin,
                       int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                       int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                       int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                       int elbow_min_servo_angle, int elbow_max_servo_angle)
  {
    this->front_left_leg.setup_shoulder(shoulder_pca_pin,
                                        shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
                                        shoulder_min_servo_angle, shoulder_max_servo_angle);

    this->front_left_leg.setup_elbow(elbow_pca_pin,
                                     elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
                                     elbow_min_servo_angle, elbow_max_servo_angle);

    fl_leg_is_setup = true;
  }

  void
  setup_back_right_leg(int shoulder_pca_pin, int elbow_pca_pin,
                       int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                       int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                       int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                       int elbow_min_servo_angle, int elbow_max_servo_angle)
  {
    this->front_right_leg.setup_shoulder(shoulder_pca_pin,
                                         shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
                                         shoulder_min_servo_angle, shoulder_max_servo_angle);

    this->front_right_leg.setup_elbow(elbow_pca_pin,
                                      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
                                      elbow_min_servo_angle, elbow_max_servo_angle);

    br_leg_is_setup = true;
  }

  void
  setup_back_left_leg(int shoulder_pca_pin, int elbow_pca_pin,
                      int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                      int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                      int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                      int elbow_min_servo_angle, int elbow_max_servo_angle)
  {
    this->back_left_leg.setup_shoulder(shoulder_pca_pin,
                                        shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
                                        shoulder_min_servo_angle, shoulder_max_servo_angle);

    this->back_left_leg.setup_elbow(elbow_pca_pin,
                                     elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
                                     elbow_min_servo_angle, elbow_max_servo_angle);

    bl_leg_is_setup = true;
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

  // ---------- POSES -----------
  void
  set_pose(BrutusPose & pose)
  {
    this->front_right_leg.set_leg_state(pose.fr_leg_state);
    this->front_left_leg.set_leg_state(pose.fl_leg_state);
    this->back_right_leg.set_leg_state(pose.br_leg_state);
    this->back_left_leg.set_leg_state(pose.bl_leg_state);
  }

  BrutusPose
  check_pose()
  {
    auto fr_state = front_right_leg.get_leg_state();
  }

  void
  change_standing_pose(BrutusPose & pose)
  {
    this->standing_pose.fr_leg_state = pose.fr_leg_state;
    this->standing_pose.fl_leg_state = pose.fl_leg_state;
    this->standing_pose.br_leg_state = pose.br_leg_state;
    this->standing_pose.bl_leg_state = pose.bl_leg_state;
  }
};




#endif // BRUTUS__H