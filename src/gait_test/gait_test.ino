#include "brutus.h"

typedef struct BrutusMovementState BrutusMovementState;
typedef struct BrutusData BrutusData;

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

BrutusLegInterface fr_leg = BrutusLegInterface(&pca);
BrutusLegInterface fl_leg = BrutusLegInterface(&pca);
BrutusLegInterface br_leg = BrutusLegInterface(&pca);
BrutusLegInterface bl_leg = BrutusLegInterface(&pca);

BrutusLegState fr_leg_state = {0,0};
BrutusLegState fl_leg_state = {0,0};
BrutusLegState br_leg_state = {0,0};
BrutusLegState bl_leg_state = {0,0};
BrutusMovementState mov_state = {0.0, 0.0, &fr_leg_state, &fl_leg_state, &br_leg_state, &bl_leg_state};

TaskHandle_t TaskMov;

SemaphoreHandle_t x_movement_mutex;

BrutusData brutus

void
setup()
{

  mov_state.ang_speed = 0.0;
  mov_state.lin_speed = 0.0;

  x_movement_mutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    movement_task,
    "movement_task",
    2048,
    (void*)x_movement_mutex,
    0,
    &TaskMov,
    MOVEMENT_CORE
  );
}

void loop() {}

void
movement_task(void *pvParameters)
{
  SemaphoreHandle_t mutex = (SemaphoreHandle_t)pvParameters;

  TickType_t exec_period = pdMS_TO_TICKS(MOVEMENT_PERIOD);
  TickType_t last_wake = xTaskGetTickCount();

  float v, w;

  while(true) {


    if 
  }
}

void
set_brutus_linear_speed(float linear_speed, )
{

}