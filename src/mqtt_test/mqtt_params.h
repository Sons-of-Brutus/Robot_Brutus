#ifndef MQTT_PARAMS__H
#define MQTT_PARAMS__H

// ----------- CLIENT ID -----------
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

// ----------- WAITS -----------
#define WIFI_WAIT 500
#define RECONNECT_WAIT 3000

// ----------- MODES -----------
#define IS_EDUROAM false // true to connect to eduroam, false to connect to normal wifi

#define TEST_WITH_RANDOM_MSG true // true to publish random data without a controller
#define PUB_FREQ 1000 // Frequency of publishing random data in ms

// ----------- RANGES FOR TEST_WITH_RANDOM_MSG-----------
#define MIN_LEG_DEG 0
#define MAX_LEG_DEG 181

#define MIN_DIST 0
#define MAX_DIST 200

// ----------- JOINTS -----------
#define SHOULDER 0
#define ELBOW 1

// ----------- OTHER -----------
#define MSG_BUFFER 254
#define BAUD 115200

#endif
