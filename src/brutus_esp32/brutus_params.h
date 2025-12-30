#ifndef BRUTUS_PARAMS__H
#define BRUTUS_PARAMS__H

#include "BrutusPose.h"

// --------- SERVOS CONFIG ----------

// Actual configuration is for MG90s servos
#define PWM_SERVO_FREQ 50

#define MIN_SERVO_ANGLE -90.0
#define MAX_SERVO_ANGLE 90.0

#define MIN_SERVO_PWM_PULSE_US 500
#define MAX_SERVO_PWM_PULSE_US 2500

// ---------------------------



// ---------- GPIOs -----------

#define US_R_TRIG 17
#define US_R_ECHO 5

#define US_L_TRIG 4
#define US_L_ECHO 16

#define US_F_TRIG 15
#define US_F_ECHO 2

#define R_PIN 32
#define G_PIN 25
#define B_PIN 33

#define PCA9685_OE 19

// ---------------------------



// --------- LEGS ----------

#define SHOULDER_BACK_LEFT_PCA_PIN 0
#define ELBOW_BACK_LEFT_PCA_PIN    1

#define SHOULDER_BACK_RIGHT_PCA_PIN 2
#define ELBOW_BACK_RIGHT_PCA_PIN    3

#define SHOULDER_FRONT_LEFT_PCA_PIN 9
#define ELBOW_FRONT_LEFT_PCA_PIN    8

#define SHOULDER_FRONT_RIGHT_PCA_PIN 13
#define ELBOW_FRONT_RIGHT_PCA_PIN    12



#define SHOULDER_BACK_LEFT_ANGLE_OFFSET -2.0
#define ELBOW_BACK_LEFT_ANGLE_OFFSET    -5.0

#define SHOULDER_BACK_RIGHT_ANGLE_OFFSET 3.0
#define ELBOW_BACK_RIGHT_ANGLE_OFFSET    0.0

#define SHOULDER_FRONT_LEFT_ANGLE_OFFSET -10.0
#define ELBOW_FRONT_LEFT_ANGLE_OFFSET    -12.0

#define SHOULDER_FRONT_RIGHT_ANGLE_OFFSET -2.0
#define ELBOW_FRONT_RIGHT_ANGLE_OFFSET    -20.0


#define SHOULDER_BACK_LEFT_INVERTED false
#define ELBOW_BACK_LEFT_INVERTED    true

#define SHOULDER_BACK_RIGHT_INVERTED true
#define ELBOW_BACK_RIGHT_INVERTED    false

#define SHOULDER_FRONT_LEFT_INVERTED false
#define ELBOW_FRONT_LEFT_INVERTED    false

#define SHOULDER_FRONT_RIGHT_INVERTED true
#define ELBOW_FRONT_RIGHT_INVERTED    true

// ---------------------------



// -------- FreeRTOS -----------
#define MOTION_CORE  0
#define DEFAULT_MOTION_PERIOD 300 // [ms]

#define LOGIC_CORE 1

#define MODES_PERIOD 200 // SAME PERIOD AS ALL LOGIC CORE TASKS
// -----------------------------


// ------- POSES ---------
constexpr BrutusLegState STANDING_FR_STATE {20, -20};
constexpr BrutusLegState STANDING_FL_STATE {20, -20};
constexpr BrutusLegState STANDING_BR_STATE {-20, -20};
constexpr BrutusLegState STANDING_BL_STATE {-20, -20};

constexpr BrutusPose STANDING_POSE {STANDING_FR_STATE,
                                    STANDING_FL_STATE,
                                    STANDING_BR_STATE,
                                    STANDING_BL_STATE};

#define ELBOW_UP 0
#define ELBOW_DOWN -20

#define FRONT_FORWARD_SHOULDER 60.0
#define FRONT_BACK_SHOULDER 30.0
#define FRONT_MID_SHOULDER (FRONT_FORWARD_SHOULDER + FRONT_BACK_SHOULDER) / 2.0

#define BACK_FORWARD_SHOULDER -25.0
#define BACK_BACK_SHOULDER -50.0
#define BACK_MID_SHOULDER (BACK_FORWARD_SHOULDER + BACK_BACK_SHOULDER) / 2.0

constexpr BrutusLegState FRONT_FORWARD_UP_STATE {FRONT_FORWARD_SHOULDER, ELBOW_UP};
constexpr BrutusLegState FRONT_FORWARD_DOWN_STATE {FRONT_FORWARD_SHOULDER, ELBOW_DOWN};

constexpr BrutusLegState BACK_FORWARD_UP_STATE {BACK_FORWARD_SHOULDER, ELBOW_UP};
constexpr BrutusLegState BACK_FORWARD_DOWN_STATE {BACK_FORWARD_SHOULDER, ELBOW_DOWN};

constexpr BrutusLegState FRONT_BACK_UP_STATE {FRONT_BACK_SHOULDER, ELBOW_UP};
constexpr BrutusLegState FRONT_BACK_DOWN_STATE {FRONT_BACK_SHOULDER, ELBOW_DOWN};

constexpr BrutusLegState BACK_BACK_UP_STATE {BACK_BACK_SHOULDER, ELBOW_UP};
constexpr BrutusLegState BACK_BACK_DOWN_STATE {BACK_BACK_SHOULDER, ELBOW_DOWN};

constexpr BrutusLegState FRONT_MID_UP_STATE {20,0};
constexpr BrutusLegState BACK_MID_UP_STATE {-20,20};

#define N_GAIT_STEPS 6

constexpr BrutusPose GAIT_1 {FRONT_FORWARD_UP_STATE,
                             FRONT_BACK_DOWN_STATE,
                             BACK_BACK_DOWN_STATE,
                             BACK_FORWARD_UP_STATE};

constexpr BrutusPose GAIT_2 {FRONT_FORWARD_DOWN_STATE,
                             FRONT_BACK_DOWN_STATE,
                             BACK_BACK_DOWN_STATE,
                             BACK_FORWARD_DOWN_STATE};

constexpr BrutusPose GAIT_3 {FRONT_FORWARD_DOWN_STATE,
                             FRONT_BACK_UP_STATE,
                             BACK_BACK_UP_STATE,
                             BACK_FORWARD_DOWN_STATE};

constexpr BrutusPose GAIT_4 {FRONT_BACK_DOWN_STATE,
                             FRONT_FORWARD_UP_STATE,
                             BACK_FORWARD_UP_STATE,
                             BACK_BACK_DOWN_STATE};

constexpr BrutusPose GAIT_5 {FRONT_BACK_DOWN_STATE,
                             FRONT_FORWARD_DOWN_STATE,
                             BACK_FORWARD_DOWN_STATE,
                             BACK_BACK_DOWN_STATE};

constexpr BrutusPose GAIT_6 {FRONT_BACK_UP_STATE,
                             FRONT_FORWARD_DOWN_STATE,
                             BACK_FORWARD_DOWN_STATE,
                             BACK_BACK_UP_STATE};

constexpr BrutusPose GAIT_STEPS[6] {GAIT_1, GAIT_2, GAIT_3, GAIT_4, GAIT_5, GAIT_6};

#define GAIT_STAY_STEP 1 // GAIT_2

constexpr BrutusPose BACKWARD_GAIT_STEPS[6] {GAIT_6, GAIT_5, GAIT_4, GAIT_3, GAIT_2, GAIT_1};


// TODO Fix SPIN poses

constexpr BrutusPose SPIN_1 {FRONT_FORWARD_UP_STATE,
                             FRONT_BACK_DOWN_STATE,
                             BACK_BACK_DOWN_STATE,
                             BACK_FORWARD_UP_STATE};

constexpr BrutusPose SPIN_2 {FRONT_MID_UP_STATE,
                             FRONT_MID_UP_STATE,
                             BACK_MID_UP_STATE,
                             BACK_MID_UP_STATE};

constexpr BrutusPose SPIN_3 {FRONT_BACK_DOWN_STATE,
                             FRONT_FORWARD_UP_STATE,
                             BACK_FORWARD_UP_STATE,
                             BACK_BACK_DOWN_STATE};

constexpr BrutusPose SPIN_4 {FRONT_MID_UP_STATE,
                             FRONT_MID_UP_STATE,
                             BACK_MID_UP_STATE,
                             BACK_MID_UP_STATE};

constexpr BrutusPose SPIN_5 {FRONT_FORWARD_UP_STATE,
                             FRONT_BACK_DOWN_STATE,
                             BACK_BACK_DOWN_STATE,
                             BACK_FORWARD_UP_STATE};

constexpr BrutusPose SPIN_6 {FRONT_MID_UP_STATE,
                             FRONT_MID_UP_STATE,
                             BACK_MID_UP_STATE,
                             BACK_MID_UP_STATE};

#define N_SPIN_STEPS 6
constexpr BrutusPose SPIN_STEPS[6] {SPIN_1, SPIN_2, SPIN_3, SPIN_4, SPIN_5, SPIN_6};
constexpr BrutusPose CCW_SPIN_STEPS[6] {SPIN_6, SPIN_5, SPIN_4, SPIN_3, SPIN_2, SPIN_1};



// -----------------------

// ---------- SPEEDS -----------
#define MIN_W -1.0f
#define MAX_W 1.0f

#define MIN_V -1.0f
#define MAX_V 1.0f
// -----------------------------

// ---------- PERCEPTION ----------
#define N_DISTANCE_SENSORS 3

#define PERCEPTION_PRIO 3

#define PERCEPTION_PERIOD 200 // [ms]
#define MIN_PERCEPTION_TIMEOUT 10 // [ms]
// --------------------------------

#endif // BRUTUS_PARAMS__H