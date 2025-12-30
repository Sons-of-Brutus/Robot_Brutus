#ifndef BRUTUS_DISTANCE_SENSOR__H
#define BRUTUS_DISTANCE_SENSOR__H

#include <Arduino.h>

void IRAM_ATTR brutus_echo_isr_handler(void* arg);

class BrutusDistanceSensor {
private:
  int trig_pin_, echo_pin_;
  volatile uint32_t start_micros_ = 0;
  volatile uint32_t duration_micros_ = 0;
  uint32_t timeout_ms_ = 30;
  SemaphoreHandle_t echo_semaphore_ = NULL;

  friend void IRAM_ATTR brutus_echo_isr_handler(void* arg);

public:
  BrutusDistanceSensor() {}

  void setup(uint32_t timeout, int trigger, int echo) {
    trig_pin_ = trigger;
    echo_pin_ = echo;
    timeout_ms_ = timeout;

    pinMode(trig_pin_, OUTPUT);
    pinMode(echo_pin_, INPUT);

    if (echo_semaphore_ == NULL) {
      echo_semaphore_ = xSemaphoreCreateBinary();
    }

    attachInterruptArg(digitalPinToInterrupt(echo_pin_), brutus_echo_isr_handler, this, CHANGE);
  }

  float read_distance() {

    digitalWrite(trig_pin_, LOW);
    delayMicroseconds(2);
    digitalWrite(trig_pin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin_, LOW);

    // Cuando salta la interrupción, si estaba en una tarea de menor prioridad que la
    // de percepción, si es necesario que se ejecute la percepción, lo hará, ya que se
    // lo estamos permitiendo porque el dato de duración ya se ha tomado.
    // Porque le cede ya el semáforo en el que se había bloqueado la task de percepción
    // al no tener el dato.

    if (xSemaphoreTake(echo_semaphore_, pdMS_TO_TICKS(timeout_ms_)) == pdTRUE) {
      return duration_micros_ * 0.0343f / 2.0f;
    }
    return -1.0f;
  }
};

void IRAM_ATTR brutus_echo_isr_handler(void* arg) {
  BrutusDistanceSensor* sensor = static_cast<BrutusDistanceSensor*>(arg);

  if (digitalRead(sensor->echo_pin_) == HIGH) {
    sensor->start_micros_ = micros();
  } else {
    sensor->duration_micros_ = micros() - sensor->start_micros_;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sensor->echo_semaphore_, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
  }
}

#endif