#ifndef BRUTUS__H
#define BRUTUS__H

#include "Pca9685Servo.h"
#include "BrutusLegInterface.h"
#include "brutus_params.h"

struct BrutusGaitState {
  // TODO La representación de las velocidades debe ser checkeada con el movimiento de las patas
  float lin_speed; // Desired linear speed
  float ang_speed; // Desired angular speed

  BrutusLegState fr_leg_state; // Desired front right leg state
  BrutusLegState fl_leg_state; // Desired front left leg state
  BrutusLegState br_leg_state; // Desired back right leg state
  BrutusLegState bl_leg_state; // Desired back left leg state
};

void init_brutus(Adafruit_PWMServoDriver* pca);

void activate_motors();
void deactivate_motors();

void eyes_red();
void eyes_blue();
void eyes_green();
void eyes_magenta();
void eyes_cyan();
void eyes_yellow();
void eyes_white();
void eyes_off();

void gait_task(void *pvParameters);

// -------- SETUP --------
void
init_brutus(Adafruit_PWMServoDriver* pca)
{
  pinMode(US_1_TRIG, OUTPUT);
  pinMode(US_1_ECHO, INPUT);

  pinMode(US_2_TRIG, OUTPUT);
  pinMode(US_2_ECHO, INPUT);
  
  pinMode(US_3_TRIG, OUTPUT);
  pinMode(US_3_ECHO, INPUT);

  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);

  pinMode(PCA9685_OE, INPUT_PULLDOWN); 
  pinMode(PCA9685_OE, OUTPUT);

  pca->begin();
  pca->setPWMFreq(PWM_SERVO_FREQ);
  delay(100);

  activate_motors();
  delay(100);

  eyes_magenta();
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

// ---------- GAIT -------------
void
gait_task(void *pvParameters)
{
  
}


#endif // BRUTUS__H