#ifndef BRUTUS_POSE__H
#define BRUTUS_POSE__H

#include "BrutusLegState.h"

float
interpolate_values(float v0, float vf, float alpha)
{
  return v0 + alpha * (vf - v0);
}

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

  BrutusPose
  interpolate(const BrutusPose & p_f, float alpha) const
  {
    alpha = constrain(alpha, 0.0f, 1.0f);

    BrutusLegState fr_leg = {
      interpolate_values(fr_leg_state.shoulder_angle,
                         p_f.fr_leg_state.shoulder_angle, alpha),
      interpolate_values(fr_leg_state.elbow_angle,
                         p_f.fr_leg_state.elbow_angle, alpha)
    };

    BrutusLegState fl_leg = {interpolate_values(fl_leg_state.shoulder_angle, p_f.fl_leg_state.shoulder_angle, alpha),
                             interpolate_values(fl_leg_state.elbow_angle, p_f.fl_leg_state.elbow_angle, alpha)};

    BrutusLegState br_leg = {interpolate_values(br_leg_state.shoulder_angle, p_f.br_leg_state.shoulder_angle, alpha),
                             interpolate_values(br_leg_state.elbow_angle, p_f.br_leg_state.elbow_angle, alpha)};

    BrutusLegState bl_leg = {interpolate_values(bl_leg_state.shoulder_angle, p_f.bl_leg_state.shoulder_angle, alpha),
                             interpolate_values(bl_leg_state.elbow_angle, p_f.bl_leg_state.elbow_angle, alpha)};

    BrutusPose pose = {fr_leg, fl_leg, br_leg, bl_leg};

    return pose;
  }
};

#endif // BRUTUS_POSE__H