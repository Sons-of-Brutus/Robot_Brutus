#ifndef COMMS_STRUCT__H
#define COMMS_STRUCT__H

#include "BrutusPose.h" 

struct BrutusCommsCmd {
  int mode;          // Modo de funcionamiento
  float v;          // Velocidad lineal
  float w;          // Velocidad angular
  BrutusPose pose;  // La pose que queremos que le llegue al robot
};

struct BrutusCommsData {
  BrutusPose pose;  // La pose que el robot envía al servidor
  float front_us;   // Distancia detectada por el ultrasonidos frontal
  float left_us;    // Distancia detectada por el ultrasonidos derecho
  float right_us;   // Distancia detectada por el ultrasonidos izquierdo
};

struct TopicHandlerSub {
  const char* topic;
  void (*handler)(const char* msg);
};

struct TopicHandlerPub {
  int n;
  const char* topic;
};
#endif