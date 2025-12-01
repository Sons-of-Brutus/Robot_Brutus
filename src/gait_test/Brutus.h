#ifndef BRUTUS__H
#define BRUTUS__H

#include "brutus_params.h"
#include "BrutusLegInterface.h"
#include "BrutusPose.h"

class Brutus {

private:
  Adafruit_PWMServoDriver* pca_;
  SemaphoreHandle_t motion_mutex_;

  BrutusLegInterface front_right_leg_;
  BrutusLegInterface back_right_leg_;
  BrutusLegInterface front_left_leg_;
  BrutusLegInterface back_left_leg_;

  BrutusLegState fr_offsets_ = {0,0};
  BrutusLegState fl_offsets_ = {0,0};
  BrutusLegState br_offsets_ = {0,0};
  BrutusLegState bl_offsets_ = {0,0};

  float lin_speed_ = 0;
  float ang_speed_ = 0;

  int pca_oe_pin_;
  int eyes_r_pin_;
  int eyes_b_pin_;
  int eyes_g_pin_;

  bool brutus_is_setup_ = false;

  bool fr_leg_is_setup_ = false;
  bool fl_leg_is_setup_ = false;
  bool br_leg_is_setup_ = false;
  bool bl_leg_is_setup_ = false;

  bool pca_active_ = false;

  BrutusPose standing_pose_ = STANDING_POSE;

  BrutusPose* target_pose_ = &standing_pose_;

public:
  Brutus()
  : pca_(NULL),
    front_right_leg_(),
    back_right_leg_(),
    front_left_leg_(),
    back_left_leg_(),
    pca_oe_pin_(0),
    eyes_r_pin_(0),
    eyes_b_pin_(0),
    eyes_g_pin_(0)
  {}

  void
  setup(Adafruit_PWMServoDriver* pca,
        int pca_oe_pin,
        int eyes_r_pin,
        int eyes_b_pin,
        int eyes_g_pin,
        int pwm_servo_frequency)
  {
    this->pca_ = pca;
    
    this->pca_oe_pin_ = pca_oe_pin;
    this->eyes_r_pin_ = eyes_r_pin;
    this->eyes_b_pin_ = eyes_b_pin;
    this->eyes_g_pin_ = eyes_g_pin;

    pinMode(this->eyes_r_pin_, OUTPUT);
    pinMode(this->eyes_b_pin_, OUTPUT);
    pinMode(this->eyes_g_pin_, OUTPUT);

    this->eyes_yellow();

    this->motion_mutex_ = xSemaphoreCreateMutex();

    delay(100);

    this->eyes_green();

    pinMode(pca_oe_pin, INPUT_PULLUP);
    pinMode(pca_oe_pin, OUTPUT);

    this->eyes_red();

    pca->begin();
    pca->setPWMFreq(pwm_servo_frequency);
    delay(100);
    this->eyes_blue();

    activate_motors();
    delay(100);

    this->front_right_leg_.setup(this->pca_);
    this->back_right_leg_.setup(this->pca_);
    this->front_left_leg_.setup(this->pca_);
    this->back_left_leg_.setup(this->pca_);

    this->eyes_cyan();

    pca_active_ = true;
    brutus_is_setup_ = false;
  }

  void
  start()
  {
    if (pca_active_ && fr_leg_is_setup_ && fl_leg_is_setup_ && br_leg_is_setup_ && bl_leg_is_setup_) {
      brutus_is_setup_ = true;
    }

    if (brutus_is_setup_) {
      this->set_pose(standing_pose_);

      this->eyes_magenta();
    } else {
      this->eyes_red();
    }
  }

  // --------- LEGS -----------

  void setup_front_right_leg(int shoulder_pca_pin, int elbow_pca_pin,
                             int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                             int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                             int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                             int elbow_min_servo_angle, int elbow_max_servo_angle,
                             float elbow_offset, float shoulder_offset)
  {
    front_right_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle
    );

    front_right_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle
    );

    fr_leg_is_setup_ = true;
  }

  void setup_front_left_leg(int shoulder_pca_pin, int elbow_pca_pin,
                            int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                            int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                            int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                            int elbow_min_servo_angle, int elbow_max_servo_angle,
                             float elbow_offset, float shoulder_offset)
  {
    front_left_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle
    );

    front_left_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle
    );

    fl_leg_is_setup_ = true;
  }

  void setup_back_right_leg(int shoulder_pca_pin, int elbow_pca_pin,
                            int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                            int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                            int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                            int elbow_min_servo_angle, int elbow_max_servo_angle,
                             float elbow_offset, float shoulder_offset)
  {
    back_right_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle
    );

    back_right_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle
    );

    br_leg_is_setup_ = true;
  }

  void setup_back_left_leg(int shoulder_pca_pin, int elbow_pca_pin,
                           int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                           int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                           int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                           int elbow_min_servo_angle, int elbow_max_servo_angle,
                             float elbow_offset, float shoulder_offset)
  {
    back_left_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle
    );

    back_left_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle
    );

    bl_leg_is_setup_ = true;
  }

  // -------- PCA9685 --------

  void
  activate_motors()
  {
    digitalWrite(this->pca_oe_pin_, LOW);
  }

  void
  deactivate_motors()
  {
    digitalWrite(this->pca_oe_pin_, HIGH);
  }

  // -------- Eyes color --------

  void
  eyes_red()
  {
    digitalWrite(this->eyes_r_pin_, HIGH);
    digitalWrite(this->eyes_b_pin_, LOW);
    digitalWrite(this->eyes_g_pin_, LOW);
  }

  void
  eyes_blue()
  {
    digitalWrite(this->eyes_r_pin_, LOW);
    digitalWrite(this->eyes_b_pin_, HIGH);
    digitalWrite(this->eyes_g_pin_, LOW);
  }

  void
  eyes_green()
  {
    digitalWrite(this->eyes_r_pin_, LOW);
    digitalWrite(this->eyes_b_pin_, LOW);
    digitalWrite(this->eyes_g_pin_, HIGH);
  }

  void
  eyes_magenta()
  {
    digitalWrite(this->eyes_r_pin_, HIGH);
    digitalWrite(this->eyes_b_pin_, HIGH);
    digitalWrite(this->eyes_g_pin_, LOW);
  }

  void
  eyes_cyan()
  {
    digitalWrite(this->eyes_r_pin_, LOW);
    digitalWrite(this->eyes_b_pin_, HIGH);
    digitalWrite(this->eyes_g_pin_, HIGH);
  }

  void
  eyes_yellow()
  {
    digitalWrite(this->eyes_r_pin_, HIGH);
    digitalWrite(this->eyes_b_pin_, LOW);
    digitalWrite(this->eyes_g_pin_, HIGH);
  }

  void
  eyes_white()
  {
    digitalWrite(this->eyes_r_pin_, HIGH);
    digitalWrite(this->eyes_b_pin_, HIGH);
    digitalWrite(this->eyes_g_pin_, HIGH);
  }

  void
  eyes_off()
  {
    digitalWrite(this->eyes_r_pin_, LOW);
    digitalWrite(this->eyes_b_pin_, LOW);
    digitalWrite(this->eyes_g_pin_, LOW);
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
    this->front_right_leg_.set_leg_state(pose.fr_leg_state);
    this->front_left_leg_.set_leg_state(pose.fl_leg_state);
    this->back_right_leg_.set_leg_state(pose.br_leg_state);
    this->back_left_leg_.set_leg_state(pose.bl_leg_state);
  }

  BrutusPose
  check_pose()
  {
    auto fr_state = front_right_leg_.get_leg_state();
    auto fl_state = front_left_leg_.get_leg_state();
    auto br_state = back_right_leg_.get_leg_state();
    auto bl_state = back_left_leg_.get_leg_state();

    BrutusPose pose = {fr_state, fl_state, br_state, bl_state};

    return pose;
  }

  void
  change_standing_pose(BrutusPose & pose)
  {
    this->standing_pose_.fr_leg_state = pose.fr_leg_state;
    this->standing_pose_.fl_leg_state = pose.fl_leg_state;
    this->standing_pose_.br_leg_state = pose.br_leg_state;
    this->standing_pose_.bl_leg_state = pose.bl_leg_state;
  }
};




#endif // BRUTUS__H