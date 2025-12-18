#ifndef BRUTUS_POSE__H
#define BRUTUS_POSE__H

#include "BrutusLegState.h"

struct BrutusPose {
  BrutusLegState fr_leg_state;
  BrutusLegState fl_leg_state;
  BrutusLegState br_leg_state;
  BrutusLegState bl_leg_state;

  void
  print() const
  {
    Serial.println("---");
    Serial.println("FRONT RIGHT:");
    Serial.print("· shoulder: ");
    Serial.println(fr_leg_state.shoulder_angle);
    Serial.print("· elbow: ");
    Serial.println(fr_leg_state.elbow_angle);
    Serial.println("FRONT LEFT:");
    Serial.print("· shoulder: ");
    Serial.println(fl_leg_state.shoulder_angle);
    Serial.print("· elbow: ");
    Serial.println(fl_leg_state.elbow_angle);
    Serial.println("BACK RIGHT:");
    Serial.print("· shoulder: ");
    Serial.println(br_leg_state.shoulder_angle);
    Serial.print("· elbow: ");
    Serial.println(br_leg_state.elbow_angle);
    Serial.println("BACK LEFT:");
    Serial.print("· shoulder: ");
    Serial.println(bl_leg_state.shoulder_angle);
    Serial.print("· elbow: ");
    Serial.println(bl_leg_state.elbow_angle);
    Serial.println("---");
  }
};

#endif // BRUTUS_POSE__H