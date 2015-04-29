#include <Adafruit_CC3000.h>
#include <PubSubClient.h>
#include <SPI.h>

#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
#define LIGHT                 8

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS,
                                         ADAFRUIT_CC3000_IRQ,
                                         ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER);

Adafruit_CC3000_Client cc3000_client = Adafruit_CC3000_Client();
PubSubClient client("io.adafruit.com", 1883, mqtt_callback, cc3000_client);

// replace with your WiFi connection info
#define WLAN_SSID "test"
#define WLAN_PASS "secretpass"
#define WLAN_SECURITY WLAN_SEC_WPA2

// replace xxxxxxxxx with your Adafruit IO key
#define AIO_KEY "xxxxxxxxx"
#define MQTT_TOPIC "api/feeds/Light/data/receive.json"

// light state
int state = 0;

void setup() {

  // set power switch tail pin as an output
  pinMode(LIGHT, OUTPUT);

  // Serial init
  Serial.begin(115200);
  Serial.println(F("Initializing....(May take a few seconds)"));

  // cc3300 init
  if (! cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }

  Serial.print(F("\nAttempting to connect to ")); 
  Serial.println(WLAN_SSID);

  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }

  Serial.println(F("Connected!"));

  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) {
    delay(100);
  }

  // connect to AIO
  mqtt_connect();

}

void loop() {

  // required for MQTT connection
  client.loop();

  // reconnect if we lost connection to AIO
  if(! client.connected()) {
    Serial.print(F("AIO connection dropped. Attempting reconnect"));
    mqtt_connect();
  }

  // write the state to the relay
  digitalWrite(LIGHT, state == 1 ? HIGH : LOW);

}

void mqtt_connect() {

  char client_name[30];

  // generate new client name
  sprintf(client_name, "adafruit-cc3000-%ul", micros());

  // attempt connection
  if (client.connect(client_name, AIO_KEY, NULL)) {
    Serial.println(F("Connected to AIO"));
    client.subscribe(MQTT_TOPIC);
    client.loop();
  } else {
    Serial.println(F("AIO connect failed"));
  }

}

void mqtt_callback (char* topic, byte* payload, unsigned int length) {

  // dump topic and payload from subscription
  Serial.print(F("Received: "));
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println("");

  // make sure we received a message from the correct feed
  if(strcmp(topic, MQTT_TOPIC) != 0)
    return;

  // convert the MQTT payload to int
  int value = to_int(payload, length);

  // make sure we have a 0 or 1
  if(value != 0 && value != 1)
    return;

  // save the new state
  state = value;

}

int to_int(byte* payload, int length) {

  int i;
  char val[10];

  // convert payload to char
  for(i = 0; i < length; i++) {
    val[i] = payload[i];
  }

  val[i] = '\0';

  // convert char to int and return
  return atoi(val);

}
