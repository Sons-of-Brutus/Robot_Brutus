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
    SemaphoreHandle_t cmd_mutex_;
    SemaphoreHandle_t data_mutex_;

  public:

    BrutusComms():
      brutus_(nullptr),
      cmd_(START_CMD),
      data_(START_DATA),
      client(espClient)
    {};
    
    void start(Brutus *b){
      Serial.begin(BAUD);
      cmd_mutex_ = xSemaphoreCreateMutex();
      data_mutex_ = xSemaphoreCreateMutex();

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
        vTaskDelay(pdMS_TO_TICKS(WIFI_WAIT));
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
      Serial.println("commsTask_static");
      BrutusComms* instance = static_cast<BrutusComms*>(param);
      instance->commsTask();
    }

    void
    create_comms_task(int core)
    {
      xTaskCreatePinnedToCore(
        (TaskFunction_t)BrutusComms::commsTask_static,
        "commsTask",
        6000,
        this,
        10,
        &this->comms_task_handle_,
        core
      );
      Serial.println("TASK CREATE");
      comms_task_period_ = COMMS_PERIOD;
    }

    void
    commsTask()
    {
      while(true) {
        // Serial.println("commsTask");
        if (!client.connected()) {
          reconnect();
        }
        client.loop();

        char heartbeat_msg[MSG_BUFFER];
        char pose_msg[MSG_BUFFER];
        char front_msg[MSG_BUFFER];
        char right_msg[MSG_BUFFER];
        char left_msg[MSG_BUFFER];

        // HEARTBEAT
        create_msg(STATE_HEARTBEAT, nullptr, heartbeat_msg, MSG_BUFFER);

        if (!client.publish(TOPIC_STATE_HEARTBEAT, heartbeat_msg)) {
          Serial.println("Error publishing STATE_HEARTBEAT");
        }
        
        xSemaphoreTake(data_mutex_, portMAX_DELAY);
        create_msg(DIST_LEFT, &data_.left_us , left_msg, MSG_BUFFER);
        create_msg(POSE, &(data_.pose), pose_msg, MSG_BUFFER);
        create_msg(DIST_FRONT, &(data_.front_us), front_msg, MSG_BUFFER);
        create_msg(DIST_RIGHT, &(data_.right_us), right_msg, MSG_BUFFER);
        create_msg(DIST_LEFT, &(data_.left_us) , left_msg, MSG_BUFFER);
        xSemaphoreGive(data_mutex_);
        

        // POSE
        if (!client.publish(TOPIC_POSE, pose_msg)) {
          Serial.println("Error publishing POSE");
        }

        // DISTANCES
        
        // FRONT DIST
        if (!client.publish(TOPIC_DIST_FRONT, front_msg)) {
          Serial.println("Error publishing DIST_FRONT");
        }

        // RIGHT DIST
        if (!client.publish(TOPIC_DIST_RIGHT, right_msg)) {
          Serial.println("Error publishing DIST_RIGHT");
        }

        // LEFT DIST
        if (!client.publish(TOPIC_DIST_LEFT, left_msg)) {
          Serial.println("Error publishing DIST_LEFT");
        }
        vTaskDelay(pdMS_TO_TICKS(comms_task_period_));
      }
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
      Serial.printf("Message arrived on topic: %s. Message: %s\n", topic, message);
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
            "{\"fr\":{\"%s\":%f,\"%s\":%f},\"fl\":{\"%s\":%f,\"%s\":%f},"
            "\"br\":{\"%s\":%f,\"%s\":%f},\"bl\":{\"%s\":%f,\"%s\":%f}}",
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

        BrutusLegState STANDING_FR_STATE {fr[SHOULDER], fr[ELBOW]};
        BrutusLegState STANDING_FL_STATE {fl[SHOULDER], fl[ELBOW]};
        BrutusLegState STANDING_BR_STATE {br[SHOULDER], br[ELBOW]};
        BrutusLegState STANDING_BL_STATE {bl[SHOULDER], bl[ELBOW]};

        BrutusPose pos{STANDING_FR_STATE,
                       STANDING_FL_STATE,
                       STANDING_BR_STATE,
                       STANDING_BL_STATE};

        xSemaphoreTake(cmd_mutex_, portMAX_DELAY);
        cmd_.pose = pos;
        Serial.println("Recived Pose Values:");
        Serial.printf("FR Shoulder: %f, FR Elbow: %f\n", cmd_.pose.fr_leg_state.shoulder_angle, cmd_.pose.fr_leg_state.elbow_angle);
        Serial.printf("FL Shoulder: %f, FL Elbow: %f\n", cmd_.pose.fl_leg_state.shoulder_angle, cmd_.pose.fl_leg_state.elbow_angle);
        Serial.printf("BR Shoulder: %f, BR Elbow: %f\n", cmd_.pose.br_leg_state.shoulder_angle, cmd_.pose.br_leg_state.elbow_angle);
        Serial.printf("BL Shoulder: %f, BL Elbow: %f\n", cmd_.pose.bl_leg_state.shoulder_angle, cmd_.pose.bl_leg_state.elbow_angle);
        xSemaphoreGive(cmd_mutex_);
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

      xSemaphoreTake(cmd_mutex_, portMAX_DELAY);
      cmd_.v = doc["vx"] | 0.0;
      cmd_.w = doc["wz"] | 0.0;

      Serial.print("vel_x = ");
      Serial.print(cmd_.v);
      Serial.print("  vel_z = ");
      Serial.println(cmd_.w);
      xSemaphoreGive(cmd_mutex_);
    }

    void
    handleCmdMode(const char* msg) {
      Serial.print("handleCmdMode called with message: ");
      xSemaphoreTake(cmd_mutex_, portMAX_DELAY);
      cmd_.mode = atoi(msg);
      Serial.println(cmd_.mode);
      xSemaphoreGive(cmd_mutex_);
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
          vTaskDelay(pdMS_TO_TICKS(RECONNECT_WAIT));
        }
      }
    }

};

BrutusComms* BrutusComms::static_instance = nullptr;

#endif // BRUTUS_COMMS__H