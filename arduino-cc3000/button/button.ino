#include <Adafruit_CC3000.h>
#include <PubSubClient.h>
#include <SPI.h>

#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
#define BUTTON                2

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
#define MQTT_TOPIC "api/feeds/%s/data/send.json"

// button state
int current = 0;
int last = -1;

void setup() {

  // set button pin as an input
  pinMode(BUTTON, INPUT_PULLUP);

  // wait until serial is available
  while (!Serial);

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

  // grab the current state of the button
  current = digitalRead(BUTTON);

  // return if the value hasn't changed
  if(current == last)
    return;

  // send the proper value to AIO depending on button state
  if(current == LOW)
    sendData("Button", "1");
  else
    sendData("Button", "0");

  // save the button state
  last = current;

}

bool sendData(char *feed, char *value) {

  // allows for ~60 char feed name
  char topic[85];

  // build mqtt topic
  sprintf(topic, MQTT_TOPIC, feed);

  // push data
  client.publish(topic, value);

  Serial.print(F("Sent: "));
  Serial.println(topic);
  Serial.println(value);
  Serial.println(F(""));

}

void mqtt_connect() {

  char client_name[30];

  // generate new client name
  sprintf(client_name, "adafruit-cc3000-%ul", micros());

  // attempt connection
  if (client.connect(client_name, AIO_KEY, NULL)) {
    Serial.println(F("Connected to AIO"));
  } else {
    Serial.println(F("AIO connect failed"));
  }

}

void mqtt_callback (char* topic, byte* payload, unsigned int length) {

  // dump topic and payload from subscriptions
  Serial.print(F("Receieved: "));
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println(topic);

}
