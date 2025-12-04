#include <WiFi.h>
#include <PubSubClient.h>
#include "WiFiClientSecure.h"
#include "credential.h"
#include "mqtt_params.h"

// ----------- STRUCTS -----------
struct TopicHandlerSub {
  const char* topic;
  void (*handler)(const char* msg);
};

struct TopicHandlerPub {
  int n;
  const char* topic;
};

struct leg {
  int ang_shoulder;
  int ang_elbow;
};

struct legs {
  leg fr;
  leg fl;
  leg br;
  leg bl;
};

// CREATE MSG
void create_msg(int topic, const void* val, char* msg);

// SUBS
void callback(char* topic, byte* payload, unsigned int length);
void handleStateMode(const char* msg);
void handleCmdMode(const char* msg);
void handleCmdVel(const char* msg);
void handleCmdPose(const char* msg);

void reconnect();


TopicHandlerSub topicHandlers_sub[] = {
  { TOPIC_CMD_POSE, handleCmdPose },
  { TOPIC_STATE_MODE, handleStateMode },
  { TOPIC_CMD_MODE, handleCmdMode },
  { TOPIC_CMD_VEL, handleCmdVel }
};

const int numTopics = sizeof(topicHandlers_sub) / sizeof(topicHandlers_sub[0]);

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  char message[MSG_BUFFER];

  if (length >= sizeof(message)) {
    Serial.print("Message received too long: ");
    return;
  }

  memcpy(message, payload, length);
  message[length] = '\0';

  for (int i = 0; i < numTopics; i++) {
    if (strcmp(topic, topicHandlers_sub[i].topic) == 0) {
      topicHandlers_sub[i].handler(message);
      return;
    }
  }

  Serial.print("Message received on unknown topic: ");
  Serial.println(topic);
}

    
// ----------- HANDLERS -----------
void handleCmdPose(const char* msg) {
  Serial.print("handleCmdPose called with message: ");
  Serial.println(msg);
}

void handleStateMode(const char* msg) {
  Serial.print("handleStateMode called with message: ");
  Serial.println(msg);

}

void handleCmdVel(const char* msg) {
  Serial.print("handleCmdVel called with message: ");
  Serial.println(msg);
}

void handleCmdMode(const char* msg) {
  Serial.print("handleCmdMode called with message: ");
  Serial.println(msg);
}

void create_msg(int topic, const void* val, char* msg, int msg_size) {
  switch(topic) {
    case STATE_HEARTBEAT:
      snprintf(msg, msg_size, "{\"timestamp\":%lu}", millis());
      break;

    case POSE: {
      const legs* l = static_cast<const legs*>(val);

      snprintf(msg, msg_size, "{\"fr\":{%d:%d,%d:%d}, \"fl\":{%d:%d,%d:%d}, \"br\":{%d:%d,%d:%d}, \"bl\":{%d:%d,%d:%d}}",
                                  SHOULDER, l->fr.ang_shoulder, ELBOW, l->fr.ang_elbow,
                                  SHOULDER, l->fl.ang_shoulder, ELBOW, l->fl.ang_elbow,
                                  SHOULDER, l->br.ang_shoulder, ELBOW, l->br.ang_elbow,
                                  SHOULDER, l->bl.ang_shoulder, ELBOW, l->bl.ang_elbow);
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

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting to connect to the MQTT broker...");

    if (client.connect(CLIENT_ID)) {
      Serial.println("Connected!");

      client.subscribe(TOPIC_CMD_POSE);
      client.subscribe(TOPIC_CMD_VEL);
      client.subscribe(TOPIC_STATE_MODE);
      client.subscribe(TOPIC_CMD_MODE);

    } else {
      Serial.print("Failed. Code: ");
      Serial.print(client.state());
      Serial.println(" Trying again in 3 seconds...");
      delay(RECONNECT_WAIT);
    }
  }
}


// ----------- SETUP -----------
void setup() {
  Serial.begin(BAUD);


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
  client.setCallback(callback);
}


// ----------- LOOP PRINCIPAL -----------
unsigned long last_msg = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  #if TEST_WITH_RANDOM_MSG
    unsigned long now = millis();
    if (now - last_msg > PUB_FREQ) {

      char msg[MSG_BUFFER];
      // HEARTBEAT
      create_msg(STATE_HEARTBEAT, 0, msg, MSG_BUFFER);

      if (!client.publish(TOPIC_STATE_HEARTBEAT, msg)) {
        Serial.println("Error publishing STATE_HEARTBEAT");
      }

      // POSE
      legs l = {
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


      last_msg = now;
    }
  #endif
}

