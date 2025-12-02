#ifndef BRUTUS__H
#define BRUTUS__H

#include "brutus_params.h"
#include "BrutusLegInterface.h"
#include "BrutusPose.h"

#define JOINTS_PER_LEG 2
#define N_LEGS 4

class Brutus {

private:
  Adafruit_PWMServoDriver* pca_;
  SemaphoreHandle_t motion_mutex_;
  bool mutex_is_locked_;

  BrutusLegInterface front_right_leg_;
  BrutusLegInterface back_right_leg_;
  BrutusLegInterface front_left_leg_;
  BrutusLegInterface back_left_leg_;

  BrutusLegState fr_offsets_ = {0,0};
  BrutusLegState fl_offsets_ = {0,0};
  BrutusLegState br_offsets_ = {0,0};
  BrutusLegState bl_offsets_ = {0,0};

  float lin_speed_ = 0.0;
  float ang_speed_ = 0.0;

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

  BrutusPose target_pose_ = standing_pose_;

  TaskHandle_t motion_task_handle_;
  int motion_task_period_;

public:
  Brutus()
  : pca_(NULL),
    mutex_is_locked_(false),
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
      this->set_pose(standing_pose_, true);

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
                             float elbow_offset, float shoulder_offset,
                             bool elbow_is_inverted, bool shoulder_is_inverted)
  {
    front_right_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle,
      shoulder_is_inverted
    );

    front_right_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle,
      elbow_is_inverted
    );

    fr_leg_is_setup_ = true;
  }

  void setup_front_left_leg(int shoulder_pca_pin, int elbow_pca_pin,
                            int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                            int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                            int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                            int elbow_min_servo_angle, int elbow_max_servo_angle,
                            float elbow_offset, float shoulder_offset,
                            bool elbow_is_inverted, bool shoulder_is_inverted)
  {
    front_left_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle,
      shoulder_is_inverted
    );

    front_left_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle,
      elbow_is_inverted
    );

    fl_leg_is_setup_ = true;
  }

  void setup_back_right_leg(int shoulder_pca_pin, int elbow_pca_pin,
                            int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                            int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                            int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                            int elbow_min_servo_angle, int elbow_max_servo_angle,
                            float elbow_offset, float shoulder_offset,
                            bool elbow_is_inverted, bool shoulder_is_inverted)
  {
    back_right_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle,
      shoulder_is_inverted
    );

    back_right_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle,
      elbow_is_inverted
    );

    br_leg_is_setup_ = true;
  }

  void setup_back_left_leg(int shoulder_pca_pin, int elbow_pca_pin,
                           int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                           int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                           int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                           int elbow_min_servo_angle, int elbow_max_servo_angle,
                           float elbow_offset, float shoulder_offset,
                           bool elbow_is_inverted, bool shoulder_is_inverted)
  {
    back_left_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle,
      shoulder_is_inverted
    );

    back_left_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle,
      elbow_is_inverted
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

  // ---------- MUTEX -------------

  // ---------- MOTION -------------
  void
  motion_task()
  {
    TickType_t last_wake_time;

    int i = 0;

    last_wake_time = xTaskGetTickCount();

    while (true) {
      this->set_pose(target_pose_,true);

      vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(motion_task_period_));
    }
  }

  static void 
  motion_task_wrapper(void *pvParameters)
  {
    Brutus *const self = static_cast<Brutus *>(pvParameters);

    self->motion_task();
  }

  void
  create_motion_task(int task_period, int core)
  {
    xTaskCreatePinnedToCore(
      (TaskFunction_t)Brutus::motion_task_wrapper,
      "MotionTask",
      2048,
      (void*)this,
      configMAX_PRIORITIES - 2,
      &this->motion_task_handle_,
      core
    );

    motion_task_period_ = MOTION_PERIOD;
  }

  // ---------- SPEED -----------

  // set_linear_speed is not thread safe
  void
  set_linear_speed(float v)
  {
    lin_speed_ = v;
  }

  // set_linear_speed_ts is the thread safe version of set_linear_speed()
  void
  set_linear_speed_ts(float v)
  {
    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    this->eyes_green();
    lin_speed_ = v;
    xSemaphoreGive(motion_mutex_);
  }

  // get_linear_speed is not thread safe
  float
  get_linear_speed()
  {
    return lin_speed_;
  }

  // get_linear_speed_ts is the thread safe version of get_linear_speed()
  float
  get_linear_speed_ts()
  {
    float v;

    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    this->eyes_cyan();
    v = lin_speed_;
    xSemaphoreGive(motion_mutex_);

    return v;
  }



  // set_angular_speed is not thread safe
  void
  set_angular_speed(float w)
  {
    ang_speed_ = w;
  }

  // set_angular_speed_ts is the thread safe version of set_angular_speed()
  void
  set_angular_speed_ts(float w)
  {
    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    this->eyes_red();
    ang_speed_ = w;
    xSemaphoreGive(motion_mutex_);
  }

  // get_angular_speed is not thread safe
  float
  get_angular_speed()
  {
    return ang_speed_;
  }

  // get_angular_speed_ts is the thread safe version of get_angular_speed()
  float
  get_angular_speed_ts()
  {
    float w;

    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    this->eyes_yellow();
    w = lin_speed_;
    xSemaphoreGive(motion_mutex_);

    return w;
  }

  // ---------- POSES -----------
  void
  change_target_pose(BrutusPose pose)
  {
    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    BrutusLegState fr = {pose.fr_leg_state.shoulder_angle + this->fr_offsets_.shoulder_angle,
                         pose.fr_leg_state.elbow_angle + this->fr_offsets_.elbow_angle};
    
    BrutusLegState fl = {pose.fl_leg_state.shoulder_angle + this->fl_offsets_.shoulder_angle,
                         pose.fl_leg_state.elbow_angle + this->fl_offsets_.elbow_angle};

    BrutusLegState br = {pose.br_leg_state.shoulder_angle + this->br_offsets_.shoulder_angle,
                         pose.br_leg_state.elbow_angle + this->br_offsets_.elbow_angle};
    
    BrutusLegState bl = {pose.bl_leg_state.shoulder_angle + this->bl_offsets_.shoulder_angle,
                         pose.bl_leg_state.elbow_angle + this->bl_offsets_.elbow_angle};
 
    this->target_pose_.fr_leg_state = fr;
    this->target_pose_.fl_leg_state = fl;
    this->target_pose_.br_leg_state = br;
    this->target_pose_.bl_leg_state = bl;

    xSemaphoreGive(motion_mutex_);
  }

  void
  set_pose(BrutusPose & pose, bool apply_inversion)
  {
    BrutusLegInterface* legs[4] = {
        &this->front_right_leg_,
        &this->front_left_leg_,
        &this->back_right_leg_,
        &this->back_left_leg_
    };

    BrutusLegState fr = {pose.fr_leg_state.shoulder_angle + this->fr_offsets_.shoulder_angle,
                         pose.fr_leg_state.elbow_angle + this->fr_offsets_.elbow_angle};
    
    BrutusLegState fl = {pose.fl_leg_state.shoulder_angle + this->fl_offsets_.shoulder_angle,
                         pose.fl_leg_state.elbow_angle + this->fl_offsets_.elbow_angle};

    BrutusLegState br = {pose.br_leg_state.shoulder_angle + this->br_offsets_.shoulder_angle,
                         pose.br_leg_state.elbow_angle + this->br_offsets_.elbow_angle};
    
    BrutusLegState bl = {pose.bl_leg_state.shoulder_angle + this->bl_offsets_.shoulder_angle,
                         pose.bl_leg_state.elbow_angle + this->bl_offsets_.elbow_angle};

    BrutusLegState states[4] = {fr,fl,br,bl};

    for (int i = 0; i < 4; i++)
    {
        legs[i]->set_leg_state(states[i], apply_inversion);
    }
  }

  BrutusPose
  check_pose(bool apply_inversion)
  {
    auto fr_state = front_right_leg_.get_leg_state(apply_inversion);
    auto fl_state = front_left_leg_.get_leg_state(apply_inversion);
    auto br_state = back_right_leg_.get_leg_state(apply_inversion);
    auto bl_state = back_left_leg_.get_leg_state(apply_inversion);

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
