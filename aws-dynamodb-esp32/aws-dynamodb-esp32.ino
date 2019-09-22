#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* ssid = "Your-ssid";
const char* password = "Your-password";
const char* mqttServer = "Your-endpoint";
const int mqttPort = 8883;

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

#define BUFFER_LEN  256
long lastMsg = 0;
char msg[BUFFER_LEN];
uint64_t chipId;

const char* root = \
                   "-----BEGIN CERTIFICATE-----\n"\
                   "................................................................\n"\
                   "-----END CERTIFICATE-----\n";
const char* cert = \
                   "-----BEGIN CERTIFICATE-----\n"\
                   "................................................................\n"\
                   "-----END CERTIFICATE-----\n";
const char* privateKey = \
                         "-----BEGIN RSA PRIVATE KEY-----\n"\
                         "................................................................\n"\
                         "-----END RSA PRIVATE KEY-----\n";
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random mqttClient ID
    String mqttClientId = "ESP32-";
    mqttClientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(mqttClientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  wifiClient.setCACert(root);
  wifiClient.setCertificate(cert);
  wifiClient.setPrivateKey(privateKey);

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  chipId = ESP.getEfuseMac();
}

void loop() {

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    String chipIdStr = String((uint16_t)(chipId>>32), HEX) + String((uint32_t)chipId, HEX);
    uint8_t randomNumber = random(20, 50);
    String randomString = String(random(0xffff), HEX);
    snprintf (msg, BUFFER_LEN, "{\"chip_id\" : \"%s\", \"random_number\" : %d, \"random_string\" : \"%s\"}", chipIdStr.c_str(), randomNumber, randomString.c_str());

    Serial.print("Publish message: ");
    Serial.println(msg);
    mqttClient.publish("outTopic", msg);
  }
}
