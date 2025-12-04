#ifndef BRUTUS_COMMS__H
#define BRUTUS_COMMS__H

#include <WiFi.h>
#include <PubSubClient.h>
#include <iostream>
#include "ArduinoJson.h"
#include "WiFiClientSecure.h"

#include "Brutus.h"
#include "comms_params.h"
#include "credential.h"
/*
TODO

Leeros esto antes de tirar codigo, porfis. Es todo lo que hablamos ayer en clase, necesidades por concurrencias, necesidades
por la clase Brutus y alguna idea que se me ha ocurrido.

BrutusComms.h:

  - Struct BrutusCommsCmd:
    · [int] mode: Modo de funcionamiento (aquí es un entero, en brutus_esp32.ino se parsea en un enum,
                                          pero ese enum estaría genial que estuviera abstraido de esta clase,
                                          ya que ese enum contiene los estados de la lógica principal)
    · [float] v: Velocidad lineal.
    · [float] w: Velocidad angular.
    · [BrutusPose] pose: La pose que quefremos que le llegue al robot.

  - Struct BrutusCommsData:
    · [BrutusPose] pose: La pose que el robot envía al servidor.
    · [float] front_us: Distancia detectada por el ultrasonidos frontal.
    · [float] left_us: Distancia detectada por el ultrasonidos derecho.
    · [float] right_us: Distancia detectada por el ultrasonidos izquierdo.

  - Clase BrutusComms (Esto es lo necesario, que hablamos ayer, que no tiene que ver con MQTT, que eso ya es cosa vuestra):
    · Atributos que sea un puntero a un objeto de clase Brutus:
      - Puntero a un objeto de clase Brutus.
      - Un struct BrutusCommsCmd.
      - Un struct BrutusCommsData.
      (Esto de separar BrutusCommsCmd y BrutusCommsData es un poco una
       idea que os doy para separar lo que llegue de los topics brutus/cmd y lo
       que se envíe a los topics brutus/data. Si veis que es más util abordarlo
       de otra forma, adelante con ello. Pero ***SÍ ES NECESARIO*** algo para guardar
       lo que os llegue por los topics brutus/cmd)

    · Aquí también necesitais hacer lo del constructor vacío y una función setup, como en la clase Brutus, por el puntero a la clase Brutus.
    · Función de creación de la task [argumentos: core, prioridad, periodo de trabajo].
    · Función que ejecuta la propia task. (Os recomiendo inspiraros en lo que he hecho del wrapper
                                           con la motion task en la clase brutus)
    · El resto de cosas las veis vosotros según vuestras necesidades de software.

    ! La task utiliza las funciones thread-safe del atributo puntero de tipo Brutus.
    ! Los callbacks de MQTT ***NO PUEDEN*** utilizar las funciones thread-safe de la clase Brutus, para eso está la struct de BrutusCommsCmd.

Cosas a tener en cuenta en brutus_esp32.ino:
  - Llamar al setup de la global de BrutusComms, como mínimo, después de brutus.start()
  - El core que utilice la task del BrutusComms debe ser la constante LOGIC_CORE (que está definida en brutus_params.h).
  - La prioridad de la task intentad que sea la mínima (0).
  - El enum BrutusMotionMode no es lo de standing, ex1, ex2,...  Eso lo tendremos en cuenta cuando programemos la lógica.
    Ahora, con que llegue el numero del modo por MQTT basta (que ese número si es lo de standing, ex1, ex2,...).
    Al enum de la lógica lo voy a llamar en este comentario LogicMode, por decir algo.

Cosas importantes de la clase Brutus:
  - Métodos thread-safe:
    · [void] set_linear_speed_ts(float v)
    · [float] get_linear_speed_ts()
    · [void] set_angular_speed_ts(float w)
    · [float] get_angular_speed()
    · [void] change_target_pose(BrutusPose pose): Este no creo que os vaya a hacer falta para algo referente a las comunicaciones.
                                                  Pero, lo que si que quiero que tengáis en cuenta, es que está función solo
                                                  puede tener efecto directo si el LogicMode es el de controlar las piernas. De
                                                  todos modos, os lo explicaré mejor cuando nos pongamos con la lógica.
    · [void] change_motion_mode(enum BrutusMotionMode mode): Este no creo que os vaya a hacer falta para algo referente a las
                                                             comunicaciones.

  - La tarea de locomoción no está hecha todavía, la haré cuando pueda y tenga el robot. Ahora mismo, lo unico que hace es poner la pose target_pose_.
    Así que al menos podéis probar las barras deslizadoras para mover los brazos utilizando el método change_target_pose.
  
  - Lo otro que tenía que decir se me ha olvidado así que os leeis el código muertos.


A meterle bien duro chavales.

*/


class BrutusComms {
  
  private:
    Brutus* brutus_;
    BrutusCommsCmd cmd_;
    BrutusCommsData data_;
    static BrutusComms* static_instance;
    
    TopicHandler* topicHandlers_sub_;
    const int numTopics = 3;

    int comms_task_period_;

    TaskHandle_t comms_task_handle_;

    WiFiClient espClient;
    PubSubClient client;


  public:

    BrutusComms():
      brutus_(nullptr),
      cmd_(START_CMD),
      data_(START_DATA),
      client(espClient)
    {};
    
    void start(Brutus *b){
      Serial.begin(BAUD);
      brutus_ = b;
      this->topicHandlers_sub_ = new TopicHandler[numTopics] {
        {CMD_POSE, TOPIC_CMD_POSE},
        {CMD_VEL,  TOPIC_CMD_VEL},
        {CMD_MODE, TOPIC_CMD_MODE}
      };
      
      #if IS_EDUROAM
        WiFi.begin(SSID_EDUROAM, WPA2_AUTH_PEAP, USERNAME_EDUROAM, USERNAME_EDUROAM, PASSWORD_EDUROAM);
        Serial.print("Connecting to eduroam");
      #else
        WiFi.begin(SSID, PASSWORD);
        Serial.print("Connecting to WiFi");
      #endif

      while(WiFi.status()!=WL_CONNECTED){
        delay(WIFI_WAIT);
        Serial.print(".");
      }

      Serial.println("\nWiFi connected!");

      client.setServer(MQTT_SERVER, MQTT_PORT);
      static_instance = this;
      client.setCallback(BrutusComms::mqtt_callback_static);
    }
    
    static void mqtt_callback_static(char* topic, byte* payload, unsigned int length) {
      if (static_instance) {
          static_instance->callback(topic, payload, length);
      }
    }

    static void commsTask_static(void* param) {
      BrutusComms* instance = static_cast<BrutusComms*>(param);
      instance->commsTask();
    }

    void
    create_comms_task(int core)
    {
      xTaskCreatePinnedToCore(
        BrutusComms::commsTask_static,
        "commsTask",
        2048,
        this,
        0,
        &this->comms_task_handle_,
        core
      );
      Serial.println("TASK CREATE");
      comms_task_period_ = COMMS_PERIOD;
    }

    void
    commsTask()
    {
      Serial.println("commsTask");
      if (!client.connected()) {
        reconnect();
      }
      client.loop();

      char msg[MSG_BUFFER];

      // HEARTBEAT
      create_msg(STATE_HEARTBEAT, 0, msg, MSG_BUFFER);

      if (!client.publish(TOPIC_STATE_HEARTBEAT, msg)) {
        Serial.println("Error publishing STATE_HEARTBEAT");
      }

      // POSE
      BrutusPose l = {
        {random(MIN_LEG_DEG, MAX_LEG_DEG), random(MIN_LEG_DEG, MAX_LEG_DEG)}, // fr
        {random(MIN_LEG_DEG, MAX_LEG_DEG), random(MIN_LEG_DEG, MAX_LEG_DEG)}, // fl
        {random(MIN_LEG_DEG, MAX_LEG_DEG), random(MIN_LEG_DEG, MAX_LEG_DEG)}, // br
        {random(MIN_LEG_DEG, MAX_LEG_DEG), random(MIN_LEG_DEG, MAX_LEG_DEG)}  // bl
      };

      create_msg(POSE, &l, msg, MSG_BUFFER);
      if (!client.publish(TOPIC_POSE, msg)) {
        Serial.println("Error publishing POSE");
      }

      // DISTANCES
      int front_dist = random(MIN_DIST, MAX_DIST);
      int right_dist = random(MIN_DIST, MAX_DIST);
      int left_dist = random(MIN_DIST, MAX_DIST);
      
      // FRONT DIST
      create_msg(DIST_FRONT, &front_dist, msg, MSG_BUFFER);
      if (!client.publish(TOPIC_DIST_FRONT, msg)) {
        Serial.println("Error publishing DIST_FRONT");
      }

      // RIGHT DIST
      create_msg(DIST_RIGHT, &right_dist, msg, MSG_BUFFER);
      if (!client.publish(TOPIC_DIST_RIGHT, msg)) {
        Serial.println("Error publishing DIST_RIGHT");
      }

      // LEFT DIST
      create_msg(DIST_LEFT, &left_dist, msg, MSG_BUFFER);
      if (!client.publish(TOPIC_DIST_LEFT, msg)) {
        Serial.println("Error publishing DIST_LEFT");
      }
      vTaskDelay(pdMS_TO_TICKS(comms_task_period_));
    }

    void
    callback(char* topic, byte* payload, unsigned int length) {
      char message[MSG_BUFFER];

      if (length >= sizeof(message)) {
        Serial.print("Message received too long: ");
        return;
      }

      memcpy(message, payload, length);
      message[length] = '\0';

      for (int i = 0; i < numTopics; i++) {
        if (strcmp(topic, topicHandlers_sub_[i].topic) == 0) {
          switch (topicHandlers_sub_[i].n)
          {
            case CMD_POSE:
              this->handleCmdPose(message);
              break;
            case CMD_VEL:
              this->handleCmdVel(message);
              break;
            case CMD_MODE:
              this->handleCmdMode(message);
              break;
          }
          return;
        }
      }

      Serial.print("Message received on unknown topic: ");
      Serial.println(topic);
    }

    void create_msg(int topic, const void* val, char* msg, int msg_size) {
      switch(topic) {
        case STATE_HEARTBEAT:
          snprintf(msg, msg_size, "{\"timestamp\":%lu}", millis());
          break;

        case POSE: {
          const BrutusPose* l = static_cast<const BrutusPose*>(val);

          snprintf(msg, msg_size,
            "{\"fr\":{\"%s\":%d,\"%s\":%d},\"fl\":{\"%s\":%d,\"%s\":%d},"
            "\"br\":{\"%s\":%d,\"%s\":%d},\"bl\":{\"%s\":%d,\"%s\":%d}}",
            SHOULDER, l->fr_leg_state.shoulder_angle, ELBOW, l->fr_leg_state.elbow_angle,
            SHOULDER, l->fl_leg_state.shoulder_angle, ELBOW, l->fl_leg_state.elbow_angle,
            SHOULDER, l->br_leg_state.shoulder_angle, ELBOW, l->br_leg_state.elbow_angle,
            SHOULDER, l->bl_leg_state.shoulder_angle, ELBOW, l->bl_leg_state.elbow_angle
          );

          break;
          
        }
        case DIST_FRONT:
          snprintf(msg, msg_size, "{\"front_dist\":%d}", *(int*)val);
          break;

        case DIST_RIGHT:
          snprintf(msg, msg_size, "{\"right_dist\":%d}", *(int*)val);
          break;

        case DIST_LEFT:
          snprintf(msg, msg_size, "{\"left_dist\":%d}", *(int*)val);
          break;
      }
    }


    // ----------- HANDLERS -----------
    void
    handleCmdPose(const char* msg) {
        Serial.print("msg: ");
        Serial.println(msg);
        StaticJsonDocument<JSON_BUFFER> doc;
        
        if (deserializeJson(doc, msg)) {
            Serial.println("JSON parsing error");
            return;
        }

        JsonObject fr = doc["fr"];
        JsonObject fl = doc["fl"];
        JsonObject br = doc["br"];
        JsonObject bl = doc["bl"];

        float fr_shoulder = fr[SHOULDER];
        float fr_elbow    = fr[ELBOW];

        float fl_shoulder = fl[SHOULDER];
        float fl_elbow    = fl[ELBOW];

        float br_shoulder = br[SHOULDER];
        float br_elbow    = br[ELBOW];

        float bl_shoulder = bl[SHOULDER];
        float bl_elbow    = bl[ELBOW];

        Serial.printf("FR shoulder: %2f, FR elbow: %2f, ", fr_shoulder, fr_elbow);
        Serial.printf("FL shoulder: %2f, FL elbow: %2f, ", fl_shoulder, fl_elbow);
        Serial.printf("BR shoulder: %2f, BR elbow: %2f, ", br_shoulder, br_elbow);
        Serial.printf("BL shoulder: %2f, BL elbow: %2f\n", bl_shoulder, bl_elbow);
    }


    void
    handleCmdVel(const char* msg) {
      Serial.print("handleCmdVel called with message: ");
      StaticJsonDocument<JSON_BUFFER> doc;
      
      if (deserializeJson(doc, msg)) {
          Serial.println("JSON parsing error");
          return;
      }

      float vel_x = doc["vx"] | 0.0;
      float vel_z = doc["wz"] | 0.0;

      Serial.print("vel_x = ");
      Serial.print(vel_x);
      Serial.print("  vel_z = ");
      Serial.println(vel_z);

    }

    void
    handleCmdMode(const char* msg) {
      Serial.print("handleCmdMode called with message: ");
      int cmd_mode = atoi(msg);
      Serial.println(cmd_mode);
    }

    void
    reconnect() {
      while (!client.connected()) {
        Serial.print("Attempting to connect to the MQTT broker...");

        if (client.connect(CLIENT_ID)) {
          Serial.println("Connected!");

          client.subscribe(TOPIC_CMD_POSE);
          client.subscribe(TOPIC_CMD_VEL);
          client.subscribe(TOPIC_CMD_MODE);

        } else {
          Serial.print("Failed. Code: ");
          Serial.print(client.state());
          Serial.println(" Trying again in 3 seconds...");
          delay(RECONNECT_WAIT);
        }
      }
    }

};

BrutusComms* BrutusComms::static_instance = nullptr;

#endif // BRUTUS_COMMS__H