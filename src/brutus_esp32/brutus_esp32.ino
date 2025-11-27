#include "brutus.h"

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

BrutusLegController fr_leg = BrutusLegController();
BrutusLegController fl_leg = BrutusLegController();
BrutusLegController br_leg = BrutusLegController();
BrutusLegController bl_leg = BrutusLegController();

void setup() {
  init_brutus(&pca);
}