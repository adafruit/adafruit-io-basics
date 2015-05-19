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

  fona.enableGPRS(false);
  while(!fona.enableGPRS(true)) {
    Serial.println(F("gprs init failed... trying again"));
    delay(2000);
  }
  
  Serial.println(F("gprs enabled"));

}

void loop() {

  // get current state and convert it to an int
  char* data = getData("Light");
  int state = atoi(data);

  // turn light on or off
  digitalWrite(LIGHT, state == 1 ? HIGH : LOW);

  // wait 5 seconds
  delay(5000);

}

char* getData(char *feed) {

  // init return vars
  uint16_t statuscode;
  int16_t length;
  int16_t i = 0;

  // build URL
  char url[200];
  sprintf(url, AIO_URL, feed, AIO_KEY);

  // dump url for debugging
  Serial.println(url);

  // make request
  if(! fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length))
    return "";

  // init char array based on length of response body
  char data[length];

  // read data into char array
  while(length > 0) {

    if(fona.available()) {

      data[i] = fona.read();
      length--;
      i++;

    }

  }

  // finish request
  fona.HTTP_GET_end();

  // terminate char array
  data[i] = '\0';

  // dump data for debugging
  Serial.print("Data: ");
  Serial.println(data);

  return data;

}
