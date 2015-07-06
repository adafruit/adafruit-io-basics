#include <Adafruit_CC3000.h>
#include <SPI.h>

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

