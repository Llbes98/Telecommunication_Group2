#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTPubSubClient.h>

//Started SoftwareSerial at D2 and D1 pin of ESP8266/NodeMCU

// WiFi
const char* ssid = "Pixel";
const char* pass = "12345678";

WiFiClientSecure client;
MQTTPubSubClient mqtt;

void setup() {
  // Serial initialization
  Serial.begin(115200);      // Serial monitor

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  Serial.println("");
  Serial.print("Connecting to wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" connected!");

  // Connect to MQTT
  Serial.print("connecting to HiveMQ...");
  client.setInsecure();  // skip verification
  while (!client.connect("a911c54c25cb45a4bbd445a491c83ccb.s2.eu.hivemq.cloud", 8883)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" connected!");

  // initialize mqtt client
  mqtt.begin(client);

  Serial.print("Logging in...");
  while (!mqtt.connect("ESPBoi", "gruppe2ktane", "explodey")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" connected!");

  // subscribe topic and callback which is called when topic has new payload
  mqtt.subscribe("MQTT_IN", [](const String& payload, const size_t size) {
    String msg = payload;  // set msg variable to equal payload (the payload is a const int, and therefore cant be changed)
    msg.trim();            // trim msg variable since there is invisible characters from the payload

    // Sends data to arduino Mega
    Serial.println((String) "From MQTT: " + msg);
  });
}

void loop() {
  mqtt.update();  // should be called


  if (Serial.available() > 0) {
    String dataFromMega;
    dataFromMega = Serial.readStringUntil('\n');  //Read the serial data and store it
    dataFromMega.trim();                              // Trim invisible characters
    Serial.println((String) "Got data from MEGA. To MQTT: " + dataFromMega);
    mqtt.publish("MQTT_OUT", dataFromMega);  // Send data to Nodered by MQTT
  }
}
