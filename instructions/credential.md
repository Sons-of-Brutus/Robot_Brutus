# Network and MQTT configuration

The file credential.h contains all the network credentials and communication parameters required for the ESP32 to connect to a WiFi network and to the MQTT broker.
This file is intentionally separated from the rest of the code in order to keep sensitive information (usernames, passwords, and network names) isolated and easy to modify without changing the main program.

This approach also allows each user to configure their own credentials without affecting the shared project code.

### WiFi configuration

The system supports two different WiFi connection modes:
- **Eduroam network**, which requires a username and password instead of only an SSID and a WiFi key
- **Home / Office WiFi network**, in which only an SSID and a password are required

### Network selection

In the main firmware, a configuration parameter determines whether the ESP32 should connect to the Eduroam network or to a standard WiFi network.

Depending on this parameter, the corresponding set of credentials (SSID_EDUROAM + USERNAME_EDUROAM + PASSWORD_EDUROAM, or SSID + PASSWORD) is used to establish the connection.

This design allows the same firmware to be used in different environments (university, home, office, etc.) without modifying the core program logic.

### MQTT configuration

The file also defines the parameters used to connect to the MQTT broker, the values selected in the files specify the address and port of the MQTT server used for communication between the robot and external systems.

### Security considerations

Since this file contains private credentials, it should not be shared publicly or uploaded to version control repositories with real data.
Each user should keep their own local copy of credential.h with their personal WiFi and Eduroam credentials.
