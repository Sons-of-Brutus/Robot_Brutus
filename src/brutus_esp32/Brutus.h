#ifndef BRUTUS__H
#define BRUTUS__H

#include "brutus_params.h"
#include "BrutusLegInterface.h"
#include "BrutusPose.h"
#include "brutus_perception.h"
#include "BrutusDistanceSensor.h"

#define JOINTS_PER_LEG 2
#define N_LEGS 4

/**
 * @enum BrutusMotionControlMode
 * @brief Motion control modes supported by Brutus's motion task.
 */
enum BrutusMotionControlMode {
  POSE_CONTROL = 0,   /**< Direct pose control */
  SPEED_CONTROL = 1   /**< Velocity-based gait control */
};

/**
 * @class Brutus
 * @brief Main control/interface class for the Brutus quadruped robot.
 *
 * This class manages:
 * - Four leg interfaces
 * - FreeRTOS motion task for motion control (pose and speed based)
 * - RGB eye status indicators
 *
 * Brutus supports both direct pose control and speed-based gait generation.
 */
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
  enum BrutusMotionControlMode motion_mode_ = POSE_CONTROL;

  TaskHandle_t perception_task_handle_;
  SemaphoreHandle_t perception_mutex_;
  BrutusPerception perception_data_ = {0.0, 0.0, 0.0};
  bool perception_is_setup_ = false;
  int perception_task_period_;

  BrutusDistanceSensor right_us_, left_us_, front_us_;

public:
  /**
   * @brief Default constructor.
   *
   * Methods that need to be called for using Brutus class:
   * 1. setup
   * 2. All legs setup
   * 3. start
   */
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

  /**
   * @brief Initializes hardware and internal subsystems.
   *
   * @param pca Pointer to an initialized PCA9685 driver.
   * @param pca_oe_pin Output enable pin for the PCA9685.
   * @param eyes_r_pin Red LED pin.
   * @param eyes_b_pin Blue LED pin.
   * @param eyes_g_pin Green LED pin.
   * @param pwm_servo_frequency Servo PWM frequency.
   */
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
    this->perception_mutex_ = xSemaphoreCreateMutex();

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

  /**
   * @brief Initializes the perception sensors hardware.
   *
   * This method configures the GPIO pins used by the distance sensors
   * (e.g. ultrasonic sensors) located on the right, left and front sides
   * of the robot. Trigger pins are configured as outputs and echo pins
   * as inputs.
   *
   * After calling this method, the perception subsystem is configured
   * at the hardware level but not yet activated.
   *
   * @param right_trig_pin GPIO pin connected to the right sensor trigger.
   * @param right_echo_pin GPIO pin connected to the right sensor echo.
   * @param left_trig_pin GPIO pin connected to the left sensor trigger.
   * @param left_echo_pin GPIO pin connected to the left sensor echo.
   * @param front_trig_pin GPIO pin connected to the front sensor trigger.
   * @param front_echo_pin GPIO pin connected to the front sensor echo.
   */
  void
  setup_perception(int right_trig_pin,
                   int right_echo_pin,
                   int left_trig_pin,
                   int left_echo_pin,
                   int front_trig_pin,
                   int front_echo_pin,
                   uint32_t timeout)
  {
    right_us_.setup(timeout, right_trig_pin, right_echo_pin);
    left_us_.setup(timeout, left_trig_pin, left_echo_pin);
    front_us_.setup(timeout, front_trig_pin, front_echo_pin);

    perception_is_setup_ = true;
  }

  /**
   * @brief Finalizes setup and moves the robot to the standing pose.
   *
   * Correct previous initialization: Magenta eyes
   * Incorrect previous intialization: Red eyes
   */
  void
  start()
  {
    if (pca_active_ && fr_leg_is_setup_ && fl_leg_is_setup_ && br_leg_is_setup_ && bl_leg_is_setup_ && perception_is_setup_) {
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

  /**
   * @brief Configures the FRONT RIGHT LEG.
   *
   * @param shoulder_pca_pin PCA channel/pin for shoulder servo.
   * @param elbow_pca_pin PCA channel/pin for elbow servo.
   * @param shoulder_min_servo_pwm_pulse_us Minimum shoulder PWM pulse period. [μs]
   * @param shoulder_max_servo_pwm_pulse_us Maximum shoulder PWM pulse period. [μs]
   * @param elbow_min_servo_pwm_pulse_us Minimum elbow PWM pulse period. [μs]
   * @param elbow_max_servo_pwm_pulse_us Maximum elbow PWM pulse period. [μs]
   * @param shoulder_min_servo_angle Minimum shoulder angle. [degrees]
   * @param shoulder_max_servo_angle Maximum shoulder angle. [degrees]
   * @param elbow_min_servo_angle Minimum elbow angle. [degrees]
   * @param elbow_max_servo_angle Maximum elbow angle. [degrees]
   * @param shoulder_offset Shoulder angle offset. [degrees]
   * @param elbow_offset Elbow angle offset. [degrees] 
   * @param shoulder_is_inverted Shoulder inversion flag.
   * @param elbow_is_inverted Elbow inversion flag.
   */
  void setup_front_right_leg(int shoulder_pca_pin, int elbow_pca_pin,
                             int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                             int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                             int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                             int elbow_min_servo_angle, int elbow_max_servo_angle,
                             float shoulder_offset, float elbow_offset,
                             bool shoulder_is_inverted, bool elbow_is_inverted)
  {
    front_right_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle,
      shoulder_is_inverted
    );

    this->fr_offsets_.shoulder_angle = shoulder_offset;

    front_right_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle,
      elbow_is_inverted
    );

    this->fr_offsets_.elbow_angle = elbow_offset;

    fr_leg_is_setup_ = true;
  }

  /**
   * @brief Configures the FRONT LEFT LEG.
   *
   * @param shoulder_pca_pin PCA channel/pin for shoulder servo.
   * @param elbow_pca_pin PCA channel/pin for elbow servo.
   * @param shoulder_min_servo_pwm_pulse_us Minimum shoulder PWM pulse period. [μs]
   * @param shoulder_max_servo_pwm_pulse_us Maximum shoulder PWM pulse period. [μs]
   * @param elbow_min_servo_pwm_pulse_us Minimum elbow PWM pulse period. [μs]
   * @param elbow_max_servo_pwm_pulse_us Maximum elbow PWM pulse period. [μs]
   * @param shoulder_min_servo_angle Minimum shoulder angle. [degrees]
   * @param shoulder_max_servo_angle Maximum shoulder angle. [degrees]
   * @param elbow_min_servo_angle Minimum elbow angle. [degrees]
   * @param elbow_max_servo_angle Maximum elbow angle. [degrees]
   * @param shoulder_offset Shoulder angle offset. [degrees]
   * @param elbow_offset Elbow angle offset. [degrees] 
   * @param shoulder_is_inverted Shoulder inversion flag.
   * @param elbow_is_inverted Elbow inversion flag.
   */
  void setup_front_left_leg(int shoulder_pca_pin, int elbow_pca_pin,
                            int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                            int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                            int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                            int elbow_min_servo_angle, int elbow_max_servo_angle,
                            float shoulder_offset, float elbow_offset,
                            bool shoulder_is_inverted, bool elbow_is_inverted)
  {
    front_left_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle,
      shoulder_is_inverted
    );

    this->fl_offsets_.shoulder_angle = shoulder_offset;

    front_left_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle,
      elbow_is_inverted
    );

    this->fl_offsets_.elbow_angle = elbow_offset;

    fl_leg_is_setup_ = true;
  }

  /**
   * @brief Configures the BACK RIGHT LEG.
   *
   * @param shoulder_pca_pin PCA channel/pin for shoulder servo.
   * @param elbow_pca_pin PCA channel/pin for elbow servo.
   * @param shoulder_min_servo_pwm_pulse_us Minimum shoulder PWM pulse period. [μs]
   * @param shoulder_max_servo_pwm_pulse_us Maximum shoulder PWM pulse period. [μs]
   * @param elbow_min_servo_pwm_pulse_us Minimum elbow PWM pulse period. [μs]
   * @param elbow_max_servo_pwm_pulse_us Maximum elbow PWM pulse period. [μs]
   * @param shoulder_min_servo_angle Minimum shoulder angle. [degrees]
   * @param shoulder_max_servo_angle Maximum shoulder angle. [degrees]
   * @param elbow_min_servo_angle Minimum elbow angle. [degrees]
   * @param elbow_max_servo_angle Maximum elbow angle. [degrees]
   * @param shoulder_offset Shoulder angle offset. [degrees]
   * @param elbow_offset Elbow angle offset. [degrees] 
   * @param shoulder_is_inverted Shoulder inversion flag.
   * @param elbow_is_inverted Elbow inversion flag.
   */
  void setup_back_right_leg(int shoulder_pca_pin, int elbow_pca_pin,
                            int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                            int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                            int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                            int elbow_min_servo_angle, int elbow_max_servo_angle,
                            float shoulder_offset, float elbow_offset,
                            bool shoulder_is_inverted, bool elbow_is_inverted)
  {
    back_right_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle,
      shoulder_is_inverted
    );

    this->br_offsets_.shoulder_angle = shoulder_offset;

    back_right_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle,
      elbow_is_inverted
    );

    this->br_offsets_.elbow_angle = elbow_offset;

    br_leg_is_setup_ = true;
  }

  /**
   * @brief Configures the BACK LEFT LEG.
   *
   * @param shoulder_pca_pin PCA channel/pin for shoulder servo.
   * @param elbow_pca_pin PCA channel/pin for elbow servo.
   * @param shoulder_min_servo_pwm_pulse_us Minimum shoulder PWM pulse period. [μs]
   * @param shoulder_max_servo_pwm_pulse_us Maximum shoulder PWM pulse period. [μs]
   * @param elbow_min_servo_pwm_pulse_us Minimum elbow PWM pulse period. [μs]
   * @param elbow_max_servo_pwm_pulse_us Maximum elbow PWM pulse period. [μs]
   * @param shoulder_min_servo_angle Minimum shoulder angle. [degrees]
   * @param shoulder_max_servo_angle Maximum shoulder angle. [degrees]
   * @param elbow_min_servo_angle Minimum elbow angle. [degrees]
   * @param elbow_max_servo_angle Maximum elbow angle. [degrees]
   * @param shoulder_offset Shoulder angle offset. [degrees]
   * @param elbow_offset Elbow angle offset. [degrees] 
   * @param shoulder_is_inverted Shoulder inversion flag.
   * @param elbow_is_inverted Elbow inversion flag.
   */
  void setup_back_left_leg(int shoulder_pca_pin, int elbow_pca_pin,
                           int shoulder_min_servo_pwm_pulse_us, int shoulder_max_servo_pwm_pulse_us,
                           int elbow_min_servo_pwm_pulse_us, int elbow_max_servo_pwm_pulse_us,
                           int shoulder_min_servo_angle, int shoulder_max_servo_angle,
                           int elbow_min_servo_angle, int elbow_max_servo_angle,
                           float shoulder_offset, float elbow_offset,
                           bool shoulder_is_inverted, bool elbow_is_inverted)
  {
    back_left_leg_.setup_shoulder(
      shoulder_pca_pin,
      shoulder_min_servo_pwm_pulse_us, shoulder_max_servo_pwm_pulse_us,
      shoulder_min_servo_angle, shoulder_max_servo_angle,
      shoulder_is_inverted
    );

    this->bl_offsets_.shoulder_angle = shoulder_offset;

    back_left_leg_.setup_elbow(
      elbow_pca_pin,
      elbow_min_servo_pwm_pulse_us, elbow_max_servo_pwm_pulse_us,
      elbow_min_servo_angle, elbow_max_servo_angle,
      elbow_is_inverted
    );

    this->bl_offsets_.elbow_angle = elbow_offset;

    bl_leg_is_setup_ = true;
  }

  // -------- PCA9685 --------

  /**
   * @brief Enables servo outputs.
   */
  void
  activate_motors()
  {
    digitalWrite(this->pca_oe_pin_, LOW);
  }

  /**
   * @brief Disables servo outputs.
   */
  void
  deactivate_motors()
  {
    digitalWrite(this->pca_oe_pin_, HIGH);
  }

  // ---------- MOTION -------------

  /**
   * @brief FreeRTOS wrapper for the motion task.
   *
   * @param pvParameters Pointer to the Brutus instance.
   */
  static void 
  motion_task_wrapper(void *pvParameters)
  {
    Brutus *const self = static_cast<Brutus *>(pvParameters);

    self->motion_task();
  }

  /**
   * @brief Creates the motion control task.
   *
   * @param task_period Task execution period. [ms]
   * @param core ESP32's CPU core to pin the task to.
   */
  void
  create_motion_task(int task_period, int core)
  {
    xTaskCreatePinnedToCore(
      (TaskFunction_t)Brutus::motion_task_wrapper,
      "MotionTask",
      MOTION_TASK_SIZE,
      (void*)this,
      configMAX_PRIORITIES - 2,
      &this->motion_task_handle_,
      core
    );

    motion_task_period_ = task_period;
  }

  /**
   * @brief Sets the motion control mode in a thread-safe way.
   *
   * @param new_mode Desired motion control mode.
   */
  void
  set_motion_control_mode(enum BrutusMotionControlMode new_mode)
  {
    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    motion_mode_ = new_mode;
    xSemaphoreGive(motion_mutex_);
  }

  /**
   * @brief Gets the current motion control mode.
   *
   * @return Current motion control mode.
   */
  enum BrutusMotionControlMode
  get_motion_control_mode()
  {
    enum BrutusMotionControlMode ctrl_mode;
    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    ctrl_mode = motion_mode_;
    xSemaphoreGive(motion_mutex_);
    return ctrl_mode;
  }
  
  /**
   * @brief Main motion control loop.
   *
   * Runs periodically as a FreeRTOS task and updates the robot pose
   * according to the selected control mode.
   */
  void
  motion_task()
  {
    TickType_t last_wake_time;

    int i = 0;

    BrutusPose pose;

    last_wake_time = xTaskGetTickCount();
    
    float v, w;

    int period;

    enum BrutusMotionControlMode ctrl_mode = this->get_motion_control_mode(), last_mode = ctrl_mode;

    while (true) {
      ctrl_mode = this->get_motion_control_mode();

      switch(ctrl_mode) {
        case POSE_CONTROL:
          if (last_mode == SPEED_CONTROL) {
            last_mode = ctrl_mode;
          }

          this->set_pose(target_pose_,true);
          break;
        
        case SPEED_CONTROL:
          if (last_mode == POSE_CONTROL) {
            i = 0;
            last_mode = ctrl_mode;
          }

          //this->check_pose(true).print();
          //Serial.println(i);

          v = constrain(this->get_linear_speed_ts(), MIN_V, MAX_V);
          w = constrain(this->get_angular_speed_ts(), MIN_W, MAX_W);

          //GAIT_STEPS[i%N_GAIT_STEPS].print();
          //pose.print();

          if (v > 0) {
            pose = process_angular_speed(GAIT_STEPS[i%N_GAIT_STEPS], w);
          } else if (v < 0) {
            pose = process_angular_speed(BACKWARD_GAIT_STEPS[i%N_GAIT_STEPS], w);
          } else {
            if (w > 0) {
              pose = process_angular_speed(SPIN_STEPS[i%N_SPIN_STEPS], w);
            } else if (w < 0) {
              pose = process_angular_speed(CCW_SPIN_STEPS[i%N_SPIN_STEPS], w);
            } else {
              pose = GAIT_STEPS[GAIT_STAY_STEP];
            }
          }

          this->change_target_pose(pose);
          this->set_pose(target_pose_,true);
          //this->front_left_leg_.set_leg_state(target_pose_.fl_leg_state,true);
          
          i++;
          break;
      }

      vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(motion_task_period_));
    }
  }

  /**
   * @brief Performs a linear interpolation between two values.
   *
   * @param v0 Initial value.
   * @param vf Final value.
   * @param alpha Interpolation factor in the range [0.0, 1.0].
   * @return Interpolated value.
   */
  float
  linear_interpolation(float v0, float vf, float alpha)
  {
    return v0 + alpha * (vf - v0);
  }

  /**
   * @brief Modifies a pose according to the desired angular speed.
   *
   * This method scales the shoulder angles of the legs to simulate
   * turning while walking. Inner legs reduce their stride while
   * outer legs increase it, depending on the angular velocity sign.
   *
   * @param pose_0 Base pose to be modified.
   * @param w Angular speed command.
   * @return BrutusPose adjusted for angular motion.
   */
  BrutusPose
  process_angular_speed(BrutusPose pose_0, float w)
  {
    w = constrain(w, MIN_W, MAX_W);

    BrutusPose pose = pose_0;

    float turn = fabs(w);

    float inner_scale = 1.0f - turn;
    float outer_scale = 1.0f + turn;

    float right_scale, left_scale;

    if (w > 0) {
      right_scale = inner_scale;
    } else {
      right_scale = outer_scale;
    }

    if (w < 0) {
      left_scale = inner_scale;
    } else {
      left_scale = outer_scale;
    }

    // FRONT
    pose.fr_leg_state.shoulder_angle =
      linear_interpolation(FRONT_MID_SHOULDER,
                           pose_0.fr_leg_state.shoulder_angle,
                           right_scale);

    pose.fl_leg_state.shoulder_angle =
      linear_interpolation(FRONT_MID_SHOULDER,
                           pose_0.fl_leg_state.shoulder_angle,
                           left_scale);

    // BACK
    pose.br_leg_state.shoulder_angle =
      linear_interpolation(BACK_MID_SHOULDER,
                           pose_0.br_leg_state.shoulder_angle,
                           right_scale);

    pose.bl_leg_state.shoulder_angle =
      linear_interpolation(BACK_MID_SHOULDER,
                           pose_0.bl_leg_state.shoulder_angle,
                           left_scale);

    return pose;
  }

  // ---------- POSES -----------

  /**
   * @brief Updates the internal target pose. (thread-safe)
   *
   * Copies the given pose into the internal target pose
   * used by the motion task.
   *
   * @param pose New target pose.
   */
  void
  change_target_pose(BrutusPose pose)
  {
    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    BrutusLegState fr = {pose.fr_leg_state.shoulder_angle,
                         pose.fr_leg_state.elbow_angle};
    
    BrutusLegState fl = {pose.fl_leg_state.shoulder_angle,
                         pose.fl_leg_state.elbow_angle};

    BrutusLegState br = {pose.br_leg_state.shoulder_angle,
                         pose.br_leg_state.elbow_angle};
    
    BrutusLegState bl = {pose.bl_leg_state.shoulder_angle,
                         pose.bl_leg_state.elbow_angle};
 
    this->target_pose_.fr_leg_state = fr;
    this->target_pose_.fl_leg_state = fl;
    this->target_pose_.br_leg_state = br;
    this->target_pose_.bl_leg_state = bl;

    xSemaphoreGive(motion_mutex_);
  }

  /**
   * @brief Commands the robot to adopt a given pose.
   *
   * Joint offsets are applied before sending commands to the servos.
   * Optionally, joint inversion can also be applied.
   *
   * @param pose Desired pose.
   * @param apply_inversion If true, joint inversion is applied.
   */
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

    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    for (int i = 0; i < 4; i++) {
      legs[i]->set_leg_state(states[i], apply_inversion);
    }
    xSemaphoreGive(motion_mutex_);

  }

  /**
   * @brief Reads the current robot pose from the servos.
   *
   * @param apply_inversion If true, joint inversion is applied to the read values.
   * @return BrutusPose Current measured pose.
   */
  BrutusPose
  check_pose(bool apply_inversion)
  {
    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    auto fr_state = front_right_leg_.get_leg_state(apply_inversion);
    auto fl_state = front_left_leg_.get_leg_state(apply_inversion);
    auto br_state = back_right_leg_.get_leg_state(apply_inversion);
    auto bl_state = back_left_leg_.get_leg_state(apply_inversion);
    xSemaphoreGive(motion_mutex_);

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

  // ---------- SPEED -----------

  /**
   * @brief Sets the linear speed command.
   *
   * @note This method is NOT thread-safe, so it should not be used
   *       while motion task is running.
   *
   * @param v Linear speed command.
   */
  void
  set_linear_speed(float v)
  {
    lin_speed_ = v;
  }

  /**
   * @brief Sets the linear speed command in a thread-safe manner.
   *
   * @note This method IS thread-safe, so it can be used
   *       while motion task is running.
   *
   * @param v Linear speed command.
   */
  void
  set_linear_speed_ts(float v)
  {
    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    lin_speed_ = v;
    xSemaphoreGive(motion_mutex_);
  }

  /**
   * @brief Gets the linear speed command.
   *
   * @note This method is NOT thread-safe, so it should not be used
   *       while motion task is running.
   *
   * @return Linear speed command.
   */
  float
  get_linear_speed()
  {
    return lin_speed_;
  }

  /**
   * @brief Gets the linear speed command in a thread-safe manner.
   *
   * @note This method IS thread-safe, so it can be used
   *       while motion task is running.
   *
   * @return Linear speed command.
   */
  float
  get_linear_speed_ts()
  {
    float v;

    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    v = lin_speed_;
    xSemaphoreGive(motion_mutex_);

    return v;
  }



  /**
   * @brief Sets the angular speed command.
   *
   * @note This method is NOT thread-safe, so it should not be used
   *       while motion task is running.
   *
   * @param w Angular speed command.
   */
  void
  set_angular_speed(float w)
  {
    ang_speed_ = w;
  }

  /**
   * @brief Sets the angular speed command in a thread-safe manner.
   *
   * @note This method IS thread-safe, so it can be used
   *       while motion task is running.
   *
   * @param w Angular speed command.
   */
  void
  set_angular_speed_ts(float w)
  {
    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    ang_speed_ = w;
    xSemaphoreGive(motion_mutex_);
  }

  /**
   * @brief Gets the angular speed command.
   *
   * @note This method is NOT thread-safe, so it should not be used
   *       while motion task is running.
   *
   * @return Angular speed command.
   */
  float
  get_angular_speed()
  {
    return ang_speed_;
  }

  /**
   * @brief Gets the angular speed command in a thread-safe manner.
   *
   * @note This method IS thread-safe, so it can be used
   *       while motion task is running.
   *
   * @return Angular speed command.
   */
  float
  get_angular_speed_ts()
  {
    float w;

    xSemaphoreTake(motion_mutex_, portMAX_DELAY);
    w = ang_speed_;
    xSemaphoreGive(motion_mutex_);

    return w;
  }

  // -------- Perception ---------

  /**
  * @brief Task example to read multiple ultrasonic sensors.
  */
  void
  perception_task()
  {
    float dist_right = 0.0, dist_left = 0.0, dist_front = 0.0;

    TickType_t last_wake_time = xTaskGetTickCount();

    //Serial.println("PERCEPTION TASK START!");

    while (true) {
      //Serial.println("<PERCEPTION>");
      
      uint32_t start_time = micros();

      dist_right = this->right_us_.read_distance();
      dist_left  = this->left_us_.read_distance();
      dist_front = this->front_us_.read_distance();

      xSemaphoreTake(perception_mutex_, portMAX_DELAY);

      if (dist_right >= 0.0) {
        this->perception_data_.right_dist = dist_right;
      }

      if (dist_left >= 0.0) {
        this->perception_data_.left_dist = dist_left;
      }

      if (dist_front >= 0.0) {
        this->perception_data_.front_dist = dist_front;
      }

      xSemaphoreGive(perception_mutex_);

      uint32_t end_time = micros();
      uint32_t execution_time_us = end_time - start_time;
      //Serial.print("[PERCEPTION] Tiempo de CPU activo: ");
      //Serial.print(execution_time_us / 1000.0);
      //Serial.println(" ms");

      vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(perception_task_period_));
    }
  }

  BrutusPerception
  get_perception_data()
  {
    BrutusPerception data;
    
    xSemaphoreTake(perception_mutex_, portMAX_DELAY);
    data = this->perception_data_;
    xSemaphoreGive(perception_mutex_);
    
    return data;
  }

  /**
   * @brief FreeRTOS wrapper for the perception task.
   *
   * @param pvParameters Pointer to the Brutus instance.
   */
  static void 
  perception_task_wrapper(void *pvParameters)
  {
    Brutus *const self = static_cast<Brutus *>(pvParameters);

    self->perception_task();
  }

  /**
   * @brief Creates the perception task.
   *
   * @param task_period Task execution period. [ms]
   * @param core ESP32's CPU core to pin the task to.
   */
  void
  create_perception_task(int task_period, int core, int priority)
  {
    perception_task_period_ = task_period;

    xTaskCreatePinnedToCore(
      (TaskFunction_t)Brutus::perception_task_wrapper,
      "PerceptionTask",
      PERCEPTION_TASK_SIZE,
      (void*)this,
      priority,
      &this->perception_task_handle_,
      core
    );
  }

  // -------- Eyes color --------

  /**
   * @brief Sets eyes LED to RED.
   */
  void
  eyes_red()
  {
    digitalWrite(this->eyes_r_pin_, HIGH);
    digitalWrite(this->eyes_b_pin_, LOW);
    digitalWrite(this->eyes_g_pin_, LOW);
  }
  
  /**
   * @brief Sets eyes LED to BLUE.
   */
  void
  eyes_blue()
  {
    digitalWrite(this->eyes_r_pin_, LOW);
    digitalWrite(this->eyes_b_pin_, HIGH);
    digitalWrite(this->eyes_g_pin_, LOW);
  }

  /**
   * @brief Sets eyes LED to GREEN.
   */
  void
  eyes_green()
  {
    digitalWrite(this->eyes_r_pin_, LOW);
    digitalWrite(this->eyes_b_pin_, LOW);
    digitalWrite(this->eyes_g_pin_, HIGH);
  }

  /**
   * @brief Sets eyes LED to MAGENTA.
   */
  void
  eyes_magenta()
  {
    digitalWrite(this->eyes_r_pin_, HIGH);
    digitalWrite(this->eyes_b_pin_, HIGH);
    digitalWrite(this->eyes_g_pin_, LOW);
  }

  /**
   * @brief Sets eyes LED to CYAN.
   */
  void
  eyes_cyan()
  {
    digitalWrite(this->eyes_r_pin_, LOW);
    digitalWrite(this->eyes_b_pin_, HIGH);
    digitalWrite(this->eyes_g_pin_, HIGH);
  }

  /**
   * @brief Sets eyes LED to YELLOW.
   */
  void
  eyes_yellow()
  {
    digitalWrite(this->eyes_r_pin_, HIGH);
    digitalWrite(this->eyes_b_pin_, LOW);
    digitalWrite(this->eyes_g_pin_, HIGH);
  }

  /**
   * @brief Sets eyes LED to WHITE.
   */
  void
  eyes_white()
  {
    digitalWrite(this->eyes_r_pin_, HIGH);
    digitalWrite(this->eyes_b_pin_, HIGH);
    digitalWrite(this->eyes_g_pin_, HIGH);
  }

  /**
   * @brief Turn off eyes LED.
   */
  void
  eyes_off()
  {
    digitalWrite(this->eyes_r_pin_, LOW);
    digitalWrite(this->eyes_b_pin_, LOW);
    digitalWrite(this->eyes_g_pin_, LOW);
  }

};


#endif // BRUTUS__H