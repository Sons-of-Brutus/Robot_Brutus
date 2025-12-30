#ifndef MQTT_PARAMS__H
#define MQTT_PARAMS__H

#include "BrutusPose.h"
#include "comms_structs.h"
#include "brutus_params.h"


// ----------- CLIENT ID -----------
#define CLIENT_ID "brutus_esp"

// ----------- TOPICS SUBS-----------

#define TOPIC_CMD_MODE   "brutus/cmd/mode"
#define TOPIC_CMD_VEL   "brutus/cmd/velocity"
#define TOPIC_CMD_POSE   "brutus/cmd/pose"

#define CMD_MODE   0
#define CMD_VEL    1
#define CMD_POSE   2

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

// ----------- WAITS -----------
#define WIFI_WAIT 500
#define RECONNECT_WAIT 3000

// ----------- MODES -----------
#define IS_EDUROAM false // true to connect to eduroam, false to connect to normal wifi

// ----------- RANGES FOR TEST_WITH_RANDOM_MSG-----------
#define MIN_LEG_DEG 0
#define MAX_LEG_DEG 181

#define MIN_DIST 0
#define MAX_DIST 200

// ----------- JOINTS -----------
#define SHOULDER "0"
#define ELBOW "1"

// ----------- BUFFERS -----------
#define MSG_BUFFER 254
#define JSON_BUFFER 300

// ----------- PERIEODS -----------
#define COMMS_PERIOD 150
#define COMMS_CORE LOGIC_CORE
#define BAUD 115200

// ----------- START CMD AND DATA -----------
constexpr BrutusCommsCmd START_CMD {0, 0.0, 0.0, STANDING_POSE};
constexpr BrutusCommsData START_DATA {STANDING_POSE, 0.0, 0.0, 0.0};

#endif