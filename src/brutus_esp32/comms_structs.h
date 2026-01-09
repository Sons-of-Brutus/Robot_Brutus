/**
* @file comms_structs.h
* @brief Definition of all the structs that are used in BrutusComms.h.
*/


#ifndef COMMS_STRUCT__H
#define COMMS_STRUCT__H

#include "BrutusPose.h" 

/**
 * @struct BrutusCommsCmd
 * @brief State of a robot leg.
 */
/**
 * @struct BrutusCommsCmd
 * @brief Command structure for the robot.
 */
struct BrutusCommsCmd {
  int mode;         /**< Operating mode */
  float v;          /**< Linear velocity */
  float w;          /**< Angular velocity */
  BrutusPose pose;  /**< Desired pose to be sent to the robot */
};

/**
 * @struct BrutusCommsData
 * @brief Data structure for the robot's state.
 */
struct BrutusCommsData {
  BrutusPose pose;  /**< Pose sent by the robot to the server */
  float front_us;   /**< Distance detected by the front ultrasonic sensor */
  float left_us;    /**< Distance detected by the left ultrasonic sensor */
  float right_us;   /**< Distance detected by the right ultrasonic sensor */
};

/**
 * @struct TopicHandler
 * @brief Structure to handle MQTT topics.
 */
struct TopicHandler{
  int n;              /**< Topic identifier */
  const char* topic; /**< Topic string */
};
#endif