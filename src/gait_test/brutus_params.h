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

/* TODO
  Poner a que sensor de ultrasonidos corresponde cada par de pines.
  1,2,3 --> FRONT, RIGHT, LEFT
*/

#define US_1_TRIG 15
#define US_1_ECHO 2

#define US_2_TRIG 4
#define US_2_ECHO 16

#define US_3_TRIG 17
#define US_3_ECHO 5

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

constexpr BrutusLegState FRONT_FORWARD_UP_STATE {60,0};
constexpr BrutusLegState FRONT_FORWARD_DOWN_STATE {60,-20};

constexpr BrutusLegState BACK_FORWARD_UP_STATE {-25,0};
constexpr BrutusLegState BACK_FORWARD_DOWN_STATE {-25,-20};

constexpr BrutusLegState FRONT_BACK_UP_STATE {30,0};
constexpr BrutusLegState FRONT_BACK_DOWN_STATE {30,-20};

constexpr BrutusLegState BACK_BACK_UP_STATE {-50,0};
constexpr BrutusLegState BACK_BACK_DOWN_STATE {-50,-20};

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

#define GAIT_TURN_STEP 0 // GAIT_1
// -----------------------

// ---------- SPEEDS -----------
#define MIN_W -1.0f
#define MAX_W 1.0f

#define MIN_V -1.0f
#define MAX_V 1.0f
// -----------------------------

#endif // BRUTUS_PARAMS__H
