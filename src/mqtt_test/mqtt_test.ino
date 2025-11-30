#include <WiFi.h>
#include <PubSubClient.h>

// ----------- WIFI CONFIGURATION -----------
#define SSID = "TU_WIFI";
#define PASSWORD = "TU_PASSWORD";

// ----------- MQTT CONFIGURATION  -----------
#define MQTT_SERVER "test.mosquitto.org
#define MQTT_PORT 1885

WiFiClient espClient;
PubSubClient client(espClient);

// ----------- Función que se ejecuta cuando llega un mensaje -----------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en ");
  Serial.print(topic);
  Serial.print(": ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
}

// ----------- Conexión MQTT -----------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar al broker MQTT...");
    if (client.connect("ESP32_mqtt_test")) {
      Serial.println("Conectado!");

      // Suscribirse a un tema
      client.subscribe("esp32/ejemplo/entrada");
    } else {
      Serial.print("Fallo. Código: ");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 3 segundos...");
      delay(3000);
    }
  }
}

// ----------- SETUP -----------
void setup() {
  Serial.begin(115200);

  // Wifi conection
  Serial.print("Conectando a ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");

  // Configuración del cliente MQTT
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
}

// ----------- LOOP PRINCIPAL -----------
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Publicar un mensaje cada 5 segundos
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    client.publish("esp32/ejemplo/salida", "Hola desde ESP32!");
    Serial.println("Mensaje publicado!");
  }
}