#ifndef BRUTUS_PARAMS__H
#define BRUTUS_PARAMS__H

// --------- SERVOS CONFIG ----------

// Actual configuration is for MG90s servos
#define PWM_SERVO_FREQ      50

#define MIN_SERVO_ANGLE 0.0
#define MAX_SERVO_ANGLE 180.0

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



#define SHOULDER_BACK_LEFT_ANGLE_OFFSET 0.0
#define ELBOW_BACK_LEFT_ANGLE_OFFSET    0.0

#define SHOULDER_BACK_RIGHT_ANGLE_OFFSET 0.0
#define ELBOW_BACK_RIGHT_ANGLE_OFFSET    0.0

#define SHOULDER_FRONT_LEFT_ANGLE_OFFSET 0.0
#define ELBOW_FRONT_LEFT_ANGLE_OFFSET    0.0

#define SHOULDER_FRONT_RIGHT_ANGLE_OFFSET 0.0
#define ELBOW_FRONT_RIGHT_ANGLE_OFFSET    0.0



#define N_LEGS 4

// ---------------------------

#endif // BRUTUS_PARAMS__H