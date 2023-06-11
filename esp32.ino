#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Ultrasonic.h>

//branch master
// Replace the variables with your Wi-Fi credentials and MQTT broker IP address
const char* ssid = "sumsung";
const char* password = "wis272777";
const char* mqtt_server = "192.168.155.150";
const int PORT = 1883;

// Replace with your MQTT topic
const char* topic = "test";
const char* topic1 = "alert";
WiFiClient espClient;
PubSubClient client(espClient);

// Initialize DHT sensor
#define DHTPIN 4    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// Initialize ultrasonic sensor
const int triggerPin = 12; // Ultrasonic sensor trigger pin
const int echoPin = 13;    // Ultrasonic sensor echo pin
Ultrasonic ultrasonic(triggerPin, echoPin);

void reconnect() {
  while (!client.connected()) {
    if (client.connect("esp32_client")) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(topic1);
    } else {
      Serial.println("Failed to connect to MQTT broker, try again in 5 seconds");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void callback(char* receivedTopic, byte* payload, unsigned int length) {
  Serial.print("Received message [");
  Serial.print(receivedTopic);
  Serial.print("]: ");

  // convert the received message into a string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, PORT);
  // Set callback function
  client.setCallback(callback);
  // Initialize DHT sensor
  dht.begin();
  // Initialize ultrasonic sensor
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read food level using ultrasonic sensor
  long distance = ultrasonic.read();
  Serial.println(distance);

  //int foodLevel = map(distance, 0, 60, 0, 100);  //pourcentage de foodLevel
  int weight = 200;

  // Create a JSON object
  StaticJsonDocument<256> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["foodLevel"] = distance;
  doc["weight"] = weight;

  // Serialize the JSON object to a string
  String payload;
  serializeJson(doc, payload);

  //Serial.println(payload);

  // Publish the payload to MQTT broker
  client.publish(topic, payload.c_str());

  // Wait a few seconds between measurements
  delay(5000);
}
