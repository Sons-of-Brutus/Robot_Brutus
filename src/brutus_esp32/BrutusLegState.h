/**
* @file BrutusLegState.h
* @brief Definition of the struct BrutusLegState.
*/

#ifndef BRUTUS_LEG_STATE__H
#define BRUTUS_LEG_STATE__H

/**
 * @struct BrutusLegState
 * @brief State of a robot leg.
 */
struct BrutusLegState
{
  float shoulder_angle; /**< Shoulder joint angle */
  float elbow_angle;   /**< Elbow joint angle */
};

#endif // BRUTUS_LEG_STATE__H