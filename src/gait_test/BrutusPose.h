#ifndef BRUTUS_POSE__H
#define BRUTUS_POSE__H

#include "BrutusLegState.h"

struct BrutusPose {
  BrutusLegState fr_leg_state;
  BrutusLegState fl_leg_state;
  BrutusLegState br_leg_state;
  BrutusLegState bl_leg_state;
};
typedef struct BrutusPose BrutusPose;

#endif // BRUTUS_POSE__H