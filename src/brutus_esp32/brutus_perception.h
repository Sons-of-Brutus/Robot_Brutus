#ifndef BRUTUS_PERCEPTION__H
#define BRUTUS_PERCEPTION__H

enum BrutusPerceptionSides {
  RIGHT = 0,
  LEFT,
  FRONT
};

struct BrutusPerception {
  float right_dist;
  float left_dist;
  float front_dist;
};

struct UltrasonicSensorPins {
  int trigger;
  int echo;
};

#endif // BRUTUS_PERCEPTION__H