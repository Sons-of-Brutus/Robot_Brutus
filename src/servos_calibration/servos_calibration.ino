#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define PWM_SERVO_FREQ      50
#define PCA9685_RESOLUTION  4096
#define PWM_PERIOD_US       20000.0f

#define MIN_MG90S_ANGLE     0.0
#define MAX_MG90S_ANGLE     180.0

#define MIN_MG90S_PULSE_US  500
#define MAX_MG90S_PULSE_US  2500

#define N_SERVOS 2

#define LED1_PCA_IDX 2
#define LED2_PCA_IDX 3

struct PcaServo {
  int pca_idx;

  uint16_t min_pwm;
  uint16_t max_pwm;

  uint16_t init_pwm;
};

typedef struct PcaServo PcaServo;

uint16_t
us_to_pwm(float pulse_width_us)
{
  float counts = (pulse_width_us / PWM_PERIOD_US) * PCA9685_RESOLUTION;

  if (counts < 0) counts = 0;
  if (counts > (PCA9685_RESOLUTION - 1)) counts = PCA9685_RESOLUTION - 1;

  return (uint16_t)counts;
}

uint16_t
angle_to_pwm(float angle,
             float min_angle, float max_angle,
             uint16_t min_pwm, uint16_t max_pwm)
{
  if (angle < min_angle) angle = min_angle;
  if (angle > max_angle) angle = max_angle;

  float slope = (float)(max_pwm - min_pwm) / (max_angle - min_angle);
  return (uint16_t)(min_pwm + slope * (angle - min_angle));
}

void
setup_servo(PcaServo *s, int idx)
{
  s->pca_idx = idx;
  s->min_pwm = us_to_pwm(MIN_MG90S_PULSE_US);
  s->max_pwm = us_to_pwm(MAX_MG90S_PULSE_US);
  s->init_pwm = angle_to_pwm(90, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE, s->min_pwm, s->max_pwm);
}

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

PcaServo servo_fr;
PcaServo servo_fl;

PcaServo* servos_list[] = {&servo_fl, &servo_fr};

void
setup()
{
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  pca.begin();
  pca.setPWMFreq(PWM_SERVO_FREQ);
  delay(100);

  for (int i = 0; i < N_SERVOS; i++) {
    setup_servo(servos_list[i], i+1);
    delay(100);
  }

  /*
  xTaskCreatePinnedToCore(
    TaskServos,
    "TaskServos",
    2048,
    NULL,
    1,
    &Task1,
    0
  );
  */
}

void loop() {
  for (int i = 0; i < N_SERVOS; i++) {
    pca.setPWM(servos_list[i]->pca_idx, 0, angle_to_pwm(90, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE, servos_list[i]->min_pwm, servos_list[i]->max_pwm));
  }
}


void
TaskServos(void *pvParameters)
{
  /*
  const float angles[] = {MIN_MG90S_ANGLE, MAX_MG90S_ANGLE};
  int angle_idx = 0;
  while (true) {
    for (int i = 0; i < N_SERVOS; i++) {
      auto actual_servo = servos_list[i];

      pca.setPWM(actual_servo->pca_idx, 0, angle_to_pwm(angles[angle_idx%2], MIN_MG90S_ANGLE, MAX_MG90S_ANGLE, actual_servo->min_pwm, actual_servo->max_pwm));
    }

    angle_idx++;

    vTaskDelay(pdMS_TO_TICKS(1000)); // HAbria que añdir timer
  }
  */

  // TODO Cambiar la función de angle_to_pwm para meterle los offsets de calibración
  // FRONT LEFT +10, -35


  const float angles[] = {MIN_MG90S_ANGLE + 10, MAX_MG90S_ANGLE - 35};
  int angle_idx = 0;

  while (true) {
    pca.setPWM(servo_fl.pca_idx, 0, angle_to_pwm(angles[angle_idx%2], MIN_MG90S_ANGLE, MAX_MG90S_ANGLE, servo_fl.min_pwm, servo_fl.max_pwm));

    angle_idx++;
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}