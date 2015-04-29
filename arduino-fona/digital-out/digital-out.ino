#include <SoftwareSerial.h>
#include <Adafruit_FONA.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
#define LIGHT 8

// replace xxxxxxxxx with your Adafruit IO key
#define AIO_KEY "xxxxxxxxx"
#define AIO_URL "http://io.adafruit.com/api/feeds/%s/data/last.txt?X-AIO-Key=%s"

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

void setup() {

  // set power switch pin as an output
  pinMode(LIGHT, OUTPUT);

  // wait until Serial is available
  while (!Serial);

  // Serial init
  Serial.begin(115200);
  Serial.println(F("Initializing....(May take a few seconds)"));

  // SoftwareSerial init
  fonaSS.begin(4800);

  // bail if we can't connect to the FONA
  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA"));
    while(1);
  }

  // turn on GPRS
  fona.enableGPRS(true);

}

void loop() {

  char* data = getData("Light");
  int state = atoi(data);

  digitalWrite(LIGHT, state == 1 ? HIGH : LOW);

  // wait 1 second
  delay(1000);

}

char* getData(char *feed) {

  uint16_t statuscode;
  int16_t length;
  int16_t i = 0;

  char url[200];
  sprintf(url, AIO_URL, feed, AIO_KEY);

  if(! fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length))
    return "";

  char data[length];

  while(length > 0) {

    if(fona.available()) {

      data[i] = fona.read();
      length--;
      i++;

    }

  }

  fona.HTTP_GET_end();

  return data;

}
