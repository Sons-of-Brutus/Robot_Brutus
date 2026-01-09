/**
 * @file brutus_perception.h
 * @brief Definition of Perception Structures for Brutus.
 *
 * Define the structures and enumerations used for handling perception data
 */

#ifndef BRUTUS_PERCEPTION__H
#define BRUTUS_PERCEPTION__H

/**
 * @enum BrutusPerceptionSides
 * @brief Perception directions of Brutus.
 *
 * Defines the different perception directions
 * (where the ultrasonic sensors are oriented) of Brutus.
 */

enum BrutusPerceptionSides {
  RIGHT = 0, /**< Right perception */
  LEFT, /**< Left perception */
  FRONT /**< Front perception */
};

/**
 * @struct BrutusPerception
 * @brief Ultrasonic configuration for Brutus.
 *
 * Groups the distances measured by the different ultrasonic sensors of Brutus.
 */
struct BrutusPerception {
  float right_dist; /**< Distance measured by the right ultrasonic sensor */
  float left_dist; /**< Distance measured by the left ultrasonic sensor */
  float front_dist; /**< Distance measured by the front ultrasonic sensor */
};

/**
 * @struct UltrasonicSensorPins
 * @brief Configuration of ultrasonic sensor pins.
 *
 * Groups the trigger and echo pins of an ultrasonic sensor.
 */
struct UltrasonicSensorPins {
  int trigger;  /**< Trigger pin of the sensor */
  int echo;   /**< Echo pin of the sensor */
};

#endif // BRUTUS_PERCEPTION__H