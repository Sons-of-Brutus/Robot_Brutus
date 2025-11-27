#include "brutus.h"

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

void setup() {
  init_brutus(&pca);
}