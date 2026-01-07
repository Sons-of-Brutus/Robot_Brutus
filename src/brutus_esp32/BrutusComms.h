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

    BrutusPose dbg_pose_;
    float dbg_float_;
    float dbg_float_2_;
  
    /*
    void upload_data(){
      xSemaphoreTake(data_mutex_, portMAX_DELAY);
      brutus_->getCommsData(&data_);
      xSemaphoreGive(data_mutex_);
    }
    */


    /**
    * @brief Static function that serves as a wrapper to run the communications task.
    * 
    * This function allows a FreeRTOS task to execute the `commsTask()` method of
    * a BrutusComms instance.
    * 
    * - Casts the `param` argument to a BrutusComms pointer.
    * - Calls the `commsTask()` method of the instance.
    * 
    * @param param Pointer to the BrutusComms instance passed to the FreeRTOS task.
    */

    static void commsTask_static(void* param) {
      Serial.println("commsTask_static");
      BrutusComms* instance = static_cast<BrutusComms*>(param);
      instance->commsTask();
    }


    /**
    * @brief Main communication task for the Brutus robot.
    * 
    * This function runs as a FreeRTOS task and is responsible for:
    * - Maintaining the MQTT connection and reconnecting if necessary.
    * - Periodically publishing status messages (heartbeat), pose, and distances.
    * - Updating the client and checking for new messages using client.loop().
    * 
    * @note It is assumed that `client` (PubSubClient) and `brutus_` have already been initialized.
    * @note `data_mutex_` is used to protect access to shared data.
    */

    void
    commsTask()
    {
      TickType_t last_wake_time = xTaskGetTickCount();

      while(true) {
        //Serial.println("<COMMS>");
        
        uint32_t start_time = micros();
        
        if (!client.connected()) {
          reconnect();
        }
        client.loop();

        char heartbeat_msg[MSG_BUFFER];
        char pose_msg[MSG_BUFFER];
        char dist_msg[MSG_BUFFER];
        char float_msg[MSG_BUFFER];
        char float2_msg[MSG_BUFFER];

        // HEARTBEAT
        create_msg(STATE_HEARTBEAT, nullptr, heartbeat_msg, MSG_BUFFER);

        if (!client.publish(TOPIC_STATE_HEARTBEAT, heartbeat_msg)) {
          Serial.println("Error publishing STATE_HEARTBEAT");
        }

        BrutusPerception perception = brutus_->get_perception_data();
        //BrutusPose pose = brutus_->check_pose(true);

        //upload_data();
        xSemaphoreTake(data_mutex_, portMAX_DELAY);
        //create_msg(DIST_LEFT, &data_.left_us , left_msg, MSG_BUFFER);
        create_msg(POSE, &dbg_pose_, pose_msg, MSG_BUFFER);
        //create_msg(DIST_FRONT, &(data_.front_us), front_msg, MSG_BUFFER);
        //create_msg(DIST_RIGHT, &(data_.right_us), right_msg, MSG_BUFFER);
        //create_msg(DIST_LEFT, &(data_.left_us) , left_msg, MSG_BUFFER);
        create_msg(DIST, &(perception), dist_msg, MSG_BUFFER);

        snprintf(float_msg, MSG_BUFFER, "{\"debug float\":%f}", dbg_float_);
        snprintf(float2_msg, MSG_BUFFER, "{\"debug float 2\":%f}", dbg_float_2_);

        xSemaphoreGive(data_mutex_);
        
        if (!client.publish("brutus/data/dbg_float", float_msg)) {
          Serial.println("Error publishiug DEBUG FLOAT");
        }

        if (!client.publish("brutus/data/dbg_float2", float2_msg)) {
          Serial.println("Error publishiug DEBUG FLOAT 2");
        }

        // POSE
        if (!client.publish(TOPIC_POSE, pose_msg)) {
          Serial.println("Error publishing POSE");
        }

        // DISTANCES
        if (!client.publish(TOPIC_DIST, dist_msg)) {
          Serial.println("Error publishing DISTANCES");
        }

        uint32_t end_time = micros();
        uint32_t execution_time_us = end_time - start_time;
        //Serial.print("[COMMS] Tiempo de CPU activo: ");
        //Serial.print(execution_time_us / 1000.0);
        //Serial.println(" ms");

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(comms_task_period_));
      }
    }


    /**
    * @brief Callback estático de MQTT que redirige los mensajes a la instancia de BrutusComms.
    * 
    * Esta función se registra en el cliente MQTT (`client.setCallback`) y actúa como
    * intermediario para invocar el método `callback()` de la instancia de BrutusComms.
    * 
    * @param topic Puntero al string que indica el tópico del mensaje recibido.
    * @param payload Puntero al buffer que contiene los datos del mensaje.
    * @param length Longitud del payload recibido.
    */

    static void mqtt_callback_static(char* topic, byte* payload, unsigned int length) {
      if (static_instance) {
          static_instance->callback(topic, payload, length);
      }
    }


    /**
    * @brief Callback executed upon receiving an MQTT message.
    * 
    * This function processes incoming messages from the MQTT broker and
    * forwards them to the appropriate handler based on the received topic.
    * 
    * - Checks if the message is too long and discards it if so.
    * - Iterates through the subscribed topics (`topicHandlers_sub_`) and calls
    *   the appropriate handler based on the command type:
    *     - CMD_POSE -> handleCmdPose()
    *     - CMD_VEL  -> handleCmdVel()
    *     - CMD_MODE -> handleCmdMode()
    * - If the topic does not match any known topic, it prints a warning message.
    * 
    * @note This function is not called directly. It is automatically invoked
    *       by `client.loop()` from PubSubClient when an MQTT message is received.
    * 
    * @param topic Pointer to the string representing the topic of the received message.
    * @param payload Pointer to the buffer containing the message data.
    * @param length Length of the received payload.
    */
  
    void
    callback(char* topic, byte* payload, unsigned int length) {
      
      char message[MSG_BUFFER];

      if (length >= sizeof(message)) {
        Serial.print("Message received too long: ");
        return;
      }

      memcpy(message, payload, length);
      message[length] = '\0';
      //Serial.printf("Message arrived on topic: %s. Message: %s\n", topic, message);
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


    /**
    * @brief Generates a JSON message for MQTT publication based on the data type.
    * 
    * This function constructs a JSON string in the `msg` buffer from the value
    * provided in `val`, depending on the message type indicated by `topic`.
    * 
    * - STATE_HEARTBEAT: generates a message with the current timestamp.
    * - POSE: generates a message with the shoulder and elbow angles of the four legs.
    * - DIST: generates a message with the distances measured by the front, right, and left sensors.
    * 
    * @param topic Type of message to generate. Can be:
    *              - STATE_HEARTBEAT
    *              - POSE
    *              - DIST
    * @param val Pointer to the value to include in the message. Interpreted according to `topic`:
    *            - POSE: pointer to BrutusPose
    *            - DIST: pointer to BrutusPerception
    *            - STATE_HEARTBEAT: ignored (can be nullptr)
    * @param msg Buffer where the generated JSON message will be stored.
    * @param msg_size Size of the `msg` buffer.
    */

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
        case DIST:
          const BrutusPerception* p = static_cast<const BrutusPerception*>(val);
          snprintf(msg, msg_size, "{\"front\":%.2f,\"right\":%.2f,\"left\":%.2f}", p->front_dist, p->right_dist, p->left_dist);
          break;
      }
    }


    // ----------- HANDLERS -----------

    /**
    * @brief Processes a message received on the POSE topic and updates cmd_.pose.
    * 
    * This function receives a JSON message containing the shoulder and elbow angles
    * of the Brutus robot's four legs. It deserializes the JSON and updates the
    * `cmd_.pose` structure, protected by the `cmd_mutex_` semaphore.
    * 
    * The expected JSON format is:
    * {
    *   "fr": {"shoulder": <float>, "elbow": <float>},
    *   "fl": {"shoulder": <float>, "elbow": <float>},
    *   "br": {"shoulder": <float>, "elbow": <float>},
    *   "bl": {"shoulder": <float>, "elbow": <float>}
    * }
    * 
    * @param msg Pointer to the JSON message received with the pose values.
    * 
    * @note If the JSON cannot be deserialized correctly, an error is printed
    *       and `cmd_.pose` is not modified.
    * @note `xSemaphoreTake` and `xSemaphoreGive` are used to protect concurrent
    *       access to `cmd_`.
    */


    void
    handleCmdPose(const char* msg) {
        //Serial.print("msg: ");
        //Serial.println(msg);
        StaticJsonDocument<JSON_BUFFER> doc;
        
        if (deserializeJson(doc, msg)) {
            Serial.println("JSON parsing error");
            return;
        }

        JsonObject fr = doc["fr"];
        JsonObject fl = doc["fl"];
        JsonObject br = doc["br"];
        JsonObject bl = doc["bl"];

        BrutusLegState fr_state {fr[SHOULDER], fr[ELBOW]};
        BrutusLegState fl_state {fl[SHOULDER], fl[ELBOW]};
        BrutusLegState br_state {br[SHOULDER], br[ELBOW]};
        BrutusLegState bl_state {bl[SHOULDER], bl[ELBOW]};

        BrutusPose pos{fr_state,
                       fl_state,
                       br_state,
                       bl};

        xSemaphoreTake(cmd_mutex_, portMAX_DELAY);
        cmd_.pose = pos;
        //Serial.println("Recived Pose Values:");
        //Serial.printf("FR Shoulder: %f, FR Elbow: %f\n", cmd_.pose.fr_leg_state.shoulder_angle, cmd_.pose.fr_leg_state.elbow_angle);
        //Serial.printf("FL Shoulder: %f, FL Elbow: %f\n", cmd_.pose.fl_leg_state.shoulder_angle, cmd_.pose.fl_leg_state.elbow_angle);
        //Serial.printf("BR Shoulder: %f, BR Elbow: %f\n", cmd_.pose.br_leg_state.shoulder_angle, cmd_.pose.br_leg_state.elbow_angle);
        //Serial.printf("BL Shoulder: %f, BL Elbow: %f\n", cmd_.pose.bl_leg_state.shoulder_angle, cmd_.pose.bl_leg_state.elbow_angle);
        xSemaphoreGive(cmd_mutex_);
    }


    /**
    * @brief Processes a message received on the velocity topic and updates cmd_.v and cmd_.w.
    * 
    * This function receives a JSON message containing the linear (vx) and angular (wz)
    * velocities for the Brutus robot. It deserializes the JSON and updates the `cmd_`
    * structure, protected by the `cmd_mutex_` semaphore.
    * 
    * The expected JSON format is:
    * {
    *   "vx": <float>,  // Linear velocity in X
    *   "wz": <float>   // Angular velocity around Z
    * }
    * 
    * @param msg Pointer to the JSON message received with the velocity values.
    * 
    * @note If the JSON cannot be deserialized correctly, an error is printed
    *       and `cmd_.v` and `cmd_.w` are not modified.
    * @note `xSemaphoreTake` and `xSemaphoreGive` are used to protect concurrent
    *       access to `cmd_`.
    */

    void
    handleCmdVel(const char* msg) {
      //Serial.print("handleCmdVel called with message: ");
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

      //Serial.print("vel_x = ");
      //Serial.print(cmd_.v);
      //Serial.print("  vel_z = ");
      //Serial.println(cmd_.w);
      xSemaphoreGive(cmd_mutex_);
    }


    /**
    * @brief Processes a message received on the mode topic and updates cmd_.mode.
    * 
    * This function receives a message containing a numeric mode value for the robot.
    * It converts the message to an integer and updates `cmd_.mode`, protected by the
    * `cmd_mutex_` semaphore.
    * 
    * @param msg Pointer to the message received representing the robot's mode.
    * 
    * @note The message `msg` is expected to contain a valid integer as a string.
    * @note `xSemaphoreTake` and `xSemaphoreGive` are used to protect concurrent
    *       access to `cmd_`.
    */

    void
    handleCmdMode(const char* msg) {
      Serial.print("handleCmdMode called with message: ");
      xSemaphoreTake(cmd_mutex_, portMAX_DELAY);
      cmd_.mode = atoi(msg);
      xSemaphoreGive(cmd_mutex_);
      Serial.println(cmd_.mode);
    }


    /**
    * @brief Attempts to reconnect to the MQTT broker until the connection is established.
    * 
    * This function enters a loop while the MQTT client (`client`) is not connected.
    * - It tries to connect using `client.connect(CLIENT_ID)`.
    * - If the connection succeeds, it subscribes to the command topics:
    *     - TOPIC_CMD_POSE
    *     - TOPIC_CMD_VEL
    *     - TOPIC_CMD_MODE
    * - If the connection fails, it prints the error code and waits `RECONNECT_WAIT` milliseconds before retrying.
    * 
    * @note This function blocks until a connection with the broker is successfully established.
    */

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

  public:

    /**
    * @brief Constructor of the BrutusComms class.
    * 
    * Initializes the class members:
    * - `brutus_` as a null pointer.
    * - `cmd_` with the default value `START_CMD`.
    * - `data_` with the default value `START_DATA`.
    * - `client` using `espClient` as the network client for MQTT.
    */

    BrutusComms():
      brutus_(nullptr),
      cmd_(START_CMD),
      data_(START_DATA),
      client(espClient)
    {};


    // TODO borrar esta función
    void 
    set_debug_pose(BrutusPose pose)
    {
      xSemaphoreTake(data_mutex_, portMAX_DELAY);
      this->dbg_pose_ = pose;
      xSemaphoreGive(data_mutex_);
    }

    // TODO borrar esta función
    void 
    set_debug_float(float dbg_f)
    {
      xSemaphoreTake(data_mutex_, portMAX_DELAY);
      this->dbg_float_ = dbg_f;
      xSemaphoreGive(data_mutex_);
    }

    void 
    set_debug_float_2(float dbg_f)
    {
      xSemaphoreTake(data_mutex_, portMAX_DELAY);
      this->dbg_float_2_ = dbg_f;
      xSemaphoreGive(data_mutex_);
    }
    
    
    /**
    * @brief Initializes the communication for the Brutus robot.
    * 
    * This function performs the necessary initial setup for BrutusComms to work:
    * - Initializes the serial communication at the speed defined by `BAUD`.
    * - Creates the `cmd_mutex_` and `data_mutex_` semaphores to protect access to shared data.
    * - Connects the ESP32 to the WiFi network. The network depends on the `IS_EDUROAM` macro defined in `comms_params.h`.
    * - Configures the MQTT client with the specified server and port.
    * - Registers the static callback `mqtt_callback_static` to process incoming messages.
    *   @note The `mqtt_callback_static` callback will internally call the `callback()` function of the BrutusComms instance to handle messages.
    * 
    * @param b Pointer to the Brutus object that will be associated with this BrutusComms instance.
    */
   
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

    /**
    * @brief Creates and launches the Brutus communications task on a specific core.
    * 
    * This function sets the task period and uses FreeRTOS's `xTaskCreatePinnedToCore`
    * to start the `commsTask_static` task.
    * 
    * - The task will execute `commsTask_static`, which in turn calls the `commsTask()`
    *   method of this BrutusComms instance.
    * - Stores the task handle in `comms_task_handle_`.
    * 
    * @param core ESP32 core where the task should be executed.
    * 
    * @note `comms_task_period_` is initialized with the value of `COMMS_PERIOD`.
    */

    void
    create_comms_task(int core)
    {
      comms_task_period_ = COMMS_PERIOD;

      xTaskCreatePinnedToCore(
        (TaskFunction_t)BrutusComms::commsTask_static,
        "commsTask",
        6000,
        this,
        1,
        &this->comms_task_handle_,
        core
      );
      //Serial.println("TASK CREATE");
    }

    /**
    * @brief Retrieves a safe copy of the current Brutus commands.
    * 
    * This function returns a copy of the `cmd_` structure, protecting access
    * using the `cmd_mutex_` semaphore to prevent race conditions.
    * 
    * @return BrutusCommsCmd A copy of the current commands (`cmd_`).
    */

    BrutusCommsCmd
    getCmd() {
      BrutusCommsCmd cmd_copy;
      xSemaphoreTake(cmd_mutex_, portMAX_DELAY);
      cmd_copy = cmd_;
      xSemaphoreGive(cmd_mutex_);
      return cmd_copy;
    }
};

BrutusComms* BrutusComms::static_instance = nullptr;

#endif // BRUTUS_COMMS__H