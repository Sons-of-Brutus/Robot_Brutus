#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define PWM_SERVO_FREQ      50
#define PCA9685_RESOLUTION  4096
#define PWM_PERIOD_US       20000.0f

#define MIN_MG90S_ANGLE     0.0
#define MAX_MG90S_ANGLE     180.0

#define MIN_MG90S_PULSE_US  500
#define MAX_MG90S_PULSE_US  2500

#define N_SERVOS 8

#define R_PIN 32
#define G_PIN 25
#define B_PIN 33

#define PCA9685_OE 19

#define ECHO_PIN 18
#define TRIG_PIN 19

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
setup_servo(PcaServo *s, int idx, float min_angle, float max_angle)
{
  s->pca_idx = idx;
  s->min_pwm = us_to_pwm(MIN_MG90S_PULSE_US);
  s->max_pwm = us_to_pwm(MAX_MG90S_PULSE_US);
  s->init_pwm = angle_to_pwm(90, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE, s->min_pwm, s->max_pwm);
}

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;
TaskHandle_t Task4;

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

PcaServo servo_lfr,
         servo_lfl,
         servo_lbr,
         servo_lbl,
         servo_cfr,
         servo_cfl,
         servo_cbr,
         servo_cbl;

PcaServo* servos_list[] = {&servo_lfr,
                           &servo_lfl,
                           &servo_lbr,
                           &servo_lbl,
                           &servo_cfr,
                           &servo_cfl,
                           &servo_cbr,
                           &servo_cbl};

void setup() {
  Serial.begin(115200);

  pca.begin();
  pca.setPWMFreq(PWM_SERVO_FREQ);

  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(PCA9685_OE, INPUT_PULLDOWN); 
  pinMode(PCA9685_OE, OUTPUT);

  digitalWrite(PCA9685_OE, LOW);

  delay(100);

  setup_servo(&servo_lfr, 12, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE);
  setup_servo(&servo_lfl, 8, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE);
  setup_servo(&servo_lbr, 3, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE);
  setup_servo(&servo_lbl, 1, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE);
  setup_servo(&servo_cfr, 13, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE);
  setup_servo(&servo_cfl, 9, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE);
  setup_servo(&servo_cbr, 2, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE);
  setup_servo(&servo_cbl, 0, MIN_MG90S_ANGLE, MAX_MG90S_ANGLE);
  delay(100);

  xTaskCreatePinnedToCore(
    TaskServos,
    "TaskServos",
    2048,
    NULL,
    1,
    &Task1,
    0
  );

  xTaskCreatePinnedToCore(
    TaskUltrasonic,
    "TaskUltrasonic",
    2048,
    NULL,
    1,
    &Task2,
    1
  );

  xTaskCreatePinnedToCore(
    TaskLed,
    "TaskLed",
    2048,
    NULL,
    2,
    &Task4,
    1
  );
}

void loop() {}

void
TaskServos(void *pvParameters)
{
  int angle_idx = 41;
  bool up = true;
  int min, max;

  float positions[][2] = {{70, 130}, {110, 50}, {110, 50}, {70, 130},{80,80},{80,80},{80,80},{80,80}};

  while (true) {
    for (int i = 0; i < N_SERVOS; i++) {
      auto actual_servo = servos_list[i];

      pca.setPWM(actual_servo->pca_idx, 0, angle_to_pwm(positions[i][angle_idx%2], MIN_MG90S_ANGLE, MAX_MG90S_ANGLE, actual_servo->min_pwm, actual_servo->max_pwm));
    }


    if (angle_idx >= 140 || angle_idx <= 40) {
      up = !(up);
    }

    if (up) {
      angle_idx += 1;
    } else {
      angle_idx -= 1;
    }

    vTaskDelay(pdMS_TO_TICKS(2000)); // HAbria que añdir timer
  }
}

void
TaskLed(void *pvParameters)
{
  uint8_t rgbTable[8][3] = {
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {1, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {0, 1, 1},
    {1, 1, 1}
  };

  while (true) {
    for (uint8_t i = 0; i < 8; i++) {
      digitalWrite(R_PIN, rgbTable[i][0]);
      digitalWrite(G_PIN, rgbTable[i][1]);
      digitalWrite(B_PIN, rgbTable[i][2]);

      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}

void
TaskUltrasonic(void *pvParameters)
{
  while (true) {
    long duration;
    float distance_cm;

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 30000);
    distance_cm = (duration / 2.0) * 0.0343;

    Serial.print("Distance: ");
    Serial.print(distance_cm);
    Serial.println(" cm");

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}