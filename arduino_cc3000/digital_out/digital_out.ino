/***************************************************
  Adafruit MQTT Library CC3000 Example

  Designed specifically to work with the Adafruit WiFi products:
  ----> https://www.adafruit.com/products/1469

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  Adafruit IO example additions by Todd Treece.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <Adafruit_SleepyDog.h>
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_CC3000.h"

/****************************** Pins ******************************************/

#define LAMP                  8  // power switch tail pin
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  5  // VBAT & CS can be any digital pins.
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "...your SSID..."  // can't be longer than 32 characters!
#define WLAN_PASS       "...your password..."
#define WLAN_SECURITY   WLAN_SEC_WPA2  // Can be: WLAN_SEC_UNSEC, WLAN_SEC_WEP,
                                       //         WLAN_SEC_WPA or WLAN_SEC_WPA2

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "...your AIO username (see https://accounts.adafruit.com)..."
#define AIO_KEY         "...your AIO key..."

/************ Global State (you don't need to change this!) ******************/

// Setup the main CC3000 class, just like a normal CC3000 sketch.
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT);

// Store the MQTT server, client ID, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM  = __TIME__ AIO_USERNAME;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the CC3000 MQTT class by passing in the CC3000 class and MQTT server and login details.
Adafruit_MQTT_CC3000 mqtt(&cc3000, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

// CC3000connect is a helper function that sets up the CC3000 and connects to
// the WiFi network. See the cc3000helper.cpp tab above for the source!
boolean CC3000connect(const char* wlan_ssid, const char* wlan_pass, uint8_t wlan_security);

/****************************** Feeds ***************************************/

// Setup a feed called 'lamp' for subscribing to changes.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
const char LAMP_FEED[] PROGMEM = AIO_USERNAME "/feeds/lamp";
Adafruit_MQTT_Subscribe lamp = Adafruit_MQTT_Subscribe(&mqtt, LAMP_FEED);

/*************************** Sketch Code ************************************/

void setup() {

  // set power switch tail pin as an output
  pinMode(LAMP, OUTPUT);

  Serial.begin(115200);

  Serial.println(F("Adafruit IO Example:"));
  Serial.print(F("Free RAM: ")); Serial.println(getFreeRam(), DEC);

  // Initialise the CC3000 module
  Serial.print(F("\nInit the CC3000..."));
  if (! cc3000.begin())
    halt("Failed");

  // attempt wifi connection
  while (! CC3000connect(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Retrying WiFi"));
    while(1);
  }
  Serial.println(F("Connected to WiFi!"));

  // listen for events on the lamp feed
  mqtt.subscribe(&lamp);

  // connect to adafruit io
  connect();

}

void loop() {

  Adafruit_MQTT_Subscribe *subscription;

  // Make sure to reset watchdog every loop iteration!
  Watchdog.reset();

  // ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

  // this is our 'wait for incoming subscription packets' busy subloop
  while (subscription = mqtt.readSubscription(1000)) {

    // we only care about the lamp events
    if (subscription == &lamp) {

      // convert mqtt ascii payload to int
      char *value = (char *)lamp.lastread;
      Serial.print(F("Received: "));
      Serial.println(value);
      int current = atoi(value);

      // write the current state to the power switch tail
      digitalWrite(LAMP, current == 1 ? HIGH : LOW);

    }

  }

}

// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default:
        Serial.println(F("Connection failed"));
        CC3000connect(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
        break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}

