#include <WiFi.h>
#include <PubSubClient.h>
#include "credential.h"

#define CLIENT_ID "brutus_esp"

// ----------- TOPICS SUBS-----------

#define TOPIC_STATE_MODE "brutus/state/mode"
#define TOPIC_CMD_MODE   "brutus/cmd/mode"
#define TOPIC_CMD_VEL   "brutus/cmd/velocity"
#define TOPIC_CMD_POSE   "brutus/cmd/pose"

// ----------- TOPICS PUBS-----------

#define TOPIC_STATE_HEARTBEAT "brutus/data/heartbeat"
#define TOPIC_POSE            "brutus/data/pose"
#define TOPIC_DIST_FRONT      "brutus/data/distance/front"
#define TOPIC_DIST_RIGHT      "brutus/data/distance/right"
#define TOPIC_DIST_LEFT       "brutus/data/distance/left"

#define STATE_HEARTBEAT 0
#define POSE            1
#define DIST_FRONT      2
#define DIST_RIGHT      3
#define DIST_LEFT       4

// ----------- OTHER -----------

#define MSG_BUFFER 100
#define PUB_FREQ 1000
#define BAUD 115200
#define WIFI_WAIT 500
#define RECONNECT_WAIT 3000
#define SHOULDER 0
#define ELBOW 1

// CREATE MSG
void create_msg(int topic, const void* val, char* msg);

// SUBS
void callback(char* topic, byte* payload, unsigned int length);
void handleStateMode(const char* msg);
void handleCmdMode(const char* msg);
void handleCmdVel(const char* msg);
void handleCmdPose(const char* msg);


void reconnect();

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

  // Wifi conection
  Serial.print("Conectando a ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
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

  unsigned long now = millis();
  if (now - last_msg > PUB_FREQ) {

    char msg[MSG_BUFFER];
    create_msg(STATE_HEARTBEAT, 0, msg, MSG_BUFFER);

    if (!client.publish(TOPIC_STATE_HEARTBEAT, msg)) {
      Serial.println("Error publicando STATE_HEARTBEAT");
    }

    legs l = {
      {random(0, 181), random(0, 181)}, // fr
      {random(0, 181), random(0, 181)}, // fl
      {random(0, 181), random(0, 181)}, // br
      {random(0, 181), random(0, 181)}  // bl
    };
    create_msg(POSE, &l, msg, MSG_BUFFER);

    if (!client.publish(TOPIC_POSE, msg)) {
      Serial.println("Error publicando POSE");
    }

    int front_dist = random(0, 100);
    int right_dist = random(0, 100);
    int left_dist = random(0, 100);
    create_msg(DIST_FRONT, &front_dist, msg, MSG_BUFFER);

    if (!client.publish(TOPIC_DIST_FRONT, msg)) {
      Serial.println("Error publicando DIST_FRONT");
    }

    create_msg(DIST_RIGHT, &right_dist, msg, MSG_BUFFER);

    if (!client.publish(TOPIC_DIST_RIGHT, msg)) {
      Serial.println("Error publicando DIST_RIGHT");
    }

    create_msg(DIST_LEFT, &left_dist, msg, MSG_BUFFER);

    if (!client.publish(TOPIC_DIST_LEFT, msg)) {
      Serial.println("Error publicando DIST_LEFT");
    }

    last_msg = now;
  }
}

