/**
 * @file BrutusDistanceSensor.h
 * @brief Ultrasonic distance sensor class.
 *
 * This class implements a distance sensor using a TRIG pin to emit
 * an ultrasonic pulse and an ECHO pin to measure the return time.
 * The echo duration is measured using interrupts and FreeRTOS
 * synchronization primitives.
 */

#ifndef BRUTUS_DISTANCE_SENSOR__H
#define BRUTUS_DISTANCE_SENSOR__H

/**
 * @class BrutusDistanceSensor
 * @brief Ultrasonic distance sensor handler.
 *
 * Manages distance measurements using a trigger pin (TRIG) and an
 * echo pin (ECHO). The echo pulse duration is measured via an
 * interrupt service routine (ISR) and synchronized with a FreeRTOS
 * semaphore.
 */
class BrutusDistanceSensor {
private:
  int trig_pin_, echo_pin_;
  volatile uint32_t start_micros_ = 0;
  volatile uint32_t duration_micros_ = 0;
  uint32_t timeout_ms_ = 30;
  SemaphoreHandle_t echo_semaphore_ = NULL;

  /**
  * @brief Echo pin interrupt service routine.
  *
  * This ISR is triggered on both rising and falling edges of the ECHO
  * pin. On the rising edge, the start time is stored. On the falling
  * edge, the pulse duration is calculated and the waiting task is
  * notified via a FreeRTOS semaphore.
  *
  * @param arg Pointer to the BrutusDistanceSensor instance.
  */
  static void echo_isr_handler(void* arg) {
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

public:
  BrutusDistanceSensor() {}

  /**
  * @brief Initialize the distance sensor.
  *
  * Configures the trigger and echo pins, initializes the timeout
  * value, creates the semaphore if needed, and attaches the echo
  * interrupt.
  *
  * @param timeout Maximum waiting time for the echo (ms).
  * @param trigger GPIO pin used as TRIG.
  * @param echo GPIO pin used as ECHO.
  */
  void setup(uint32_t timeout, int trigger, int echo) {
    trig_pin_ = trigger;
    echo_pin_ = echo;
    timeout_ms_ = timeout;

    pinMode(trig_pin_, OUTPUT);
    pinMode(echo_pin_, INPUT);

    if (echo_semaphore_ == NULL) {
      echo_semaphore_ = xSemaphoreCreateBinary();
    }

    attachInterruptArg(digitalPinToInterrupt(echo_pin_), echo_isr_handler, this, CHANGE);
  }

  /**
  * @brief Measure the distance to an object.
  *
  * Sends a trigger pulse and waits for the echo response.
  * If the echo is received within the timeout, the distance
  * is calculated based on the speed of sound.
  *
  * @return Distance in centimeters, or -1.0 if the timeout expires.
  */
  float read_distance() {
    digitalWrite(trig_pin_, LOW);
    delayMicroseconds(2);
    digitalWrite(trig_pin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin_, LOW);

    if (xSemaphoreTake(echo_semaphore_, pdMS_TO_TICKS(timeout_ms_)) == pdTRUE) {
      return duration_micros_ * 0.0343f / 2.0f;
    }
    return -1.0f;
  }
};

#endif
