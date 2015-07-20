/***************************************************
  Adafruit IO Trigger Example
  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board:
  ----> https://www.adafruit.com/product/2471

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Todd Treece for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_IO_Client.h"

// door gpio pin
#define DOOR 13

// wifi credentials
#define WLAN_SSID       "...your SSID..."
#define WLAN_PASS       "...your password..."

// Configure Adafruit IO access.
#define AIO_KEY         "...your AIO key..."

WiFiClient client;

// Create an Adafruit IO Client instance.  Notice that this needs to take a
// WiFiClient object as the first parameter, and as the second parameter a
// default Adafruit IO key to use when accessing feeds (however each feed can
// override this default key value if required, see further below).
Adafruit_IO_Client aio = Adafruit_IO_Client(client, AIO_KEY);

// Finally create instances of Adafruit_IO_Feed objects, one per feed.  Do this
// by calling the getFeed function on the Adafruit_IO_FONA object and passing
// it at least the name of the feed, and optionally a specific AIO key to use
// when accessing the feed (the default is to use the key set on the
// Adafruit_IO_Client class).
Adafruit_IO_Feed door = aio.getFeed("door");

void setup() {

  pinMode(DOOR, INPUT_PULLUP);

  // if door isn't open, we don't need to send anything
  if(digitalRead(DOOR) == LOW) {
    // sleep a couple seconds before checking again
    ESP.deepSleep(2000000, WAKE_RF_DISABLED);
    return;
  }

  // wifi init
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  // wait for connection
  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  // AIO init
  aio.begin();

  // send value to AIO
  door.send("1");

  // sleep for a couple seconds
  ESP.deepSleep(2000000, WAKE_RF_DISABLED);

}

// noop
void loop() {}

