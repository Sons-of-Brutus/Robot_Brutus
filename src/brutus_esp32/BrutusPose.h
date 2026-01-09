/**
* @file BrutusPose.h
* @brief Definition of the struct BrutusPose.
*/

#ifndef BRUTUS_POSE__H
#define BRUTUS_POSE__H

#include "BrutusLegState.h"

/**
 * @struct BrutusPose
 * @brief Pose of the Brutus robot.
 */

struct BrutusPose {
  BrutusLegState fr_leg_state; /**< Sate of the front right leg */
  BrutusLegState fl_leg_state; /**< Sate of the front left leg */
  BrutusLegState br_leg_state; /**< Sate of the back right leg */
  BrutusLegState bl_leg_state; /**< Sate of the back left leg */
};
typedef struct BrutusPose BrutusPose;

#endif // BRUTUS_POSE__H