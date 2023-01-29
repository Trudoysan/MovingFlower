#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "Hlava";
const char* password = "xxxxx";

static const int leafs = 8;
static const int ledInLeaf = 4;

static const int servoPin[leafs] = { 16, 17, 5, 18, 19, 21, 3, 1 };
static const int touchPin[leafs] = { T9, T3, T4, T5, T6, T7, T8, T0 };
static const int ledPin = 23;

Servo servo[leafs];
int touchVal[leafs] = { 0 };
int up[leafs] = { 156, 155, 153, 151, 140, 144, 168, 155 };
int upNow[leafs];
int upTarget[leafs];
static const int moveAngle = 60;
int lastLeaf = -1;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(leafs * ledInLeaf, ledPin, NEO_GRB + NEO_KHZ800);

static const int maxTricks = 5;
static const int maxColours = 12;
int tricks[leafs];
int colours[leafs];

uint32_t selectColour(int col, float intensity = 1.0);

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(50);
  strip.fill(strip.Color(0, 0, 255));
  strip.show();  // Initialize all pixels to 'off'
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    strip.fill(strip.Color(255, 0, 0));
    delay(5000);
    ESP.restart();
  }
  strip.fill(strip.Color(0, 0, 0));
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Serial.print("Done ");
  //Serial.println(touchVal);
  //delay(200);
  for (int i = 0; i < leafs; i++) {
    //  Serial.print("leaf ");
    //  Serial.print(i);
    //  Serial.print(" pin ");
    //delay(100);
    //  Serial.println(servoPin[i]);
    servo[i].attach(servoPin[i]);
  }
  delay(200);
  //Serial.println("4 ");

  for (int i = 0; i < leafs; i++) {
    servo[i].write(up[i]);
    upNow[i] = up[i];
    upTarget[i] = up[i];
  }
  delay(500);
  for (int pokus = 0; pokus < 10; pokus++) {
    for (int i = 0; i < leafs; i++) {
      touchVal[i] += touchRead(touchPin[i]);
      //Serial.println(touchVal);
    }
    delay(50);
  }

  for (int i = 0; i < leafs; i++)
    touchVal[i] /= 10;
  //Serial.println("Setup Done ");
  randomSeed(analogRead(26));
}

void loop() {
  ArduinoOTA.handle();
  /* for (int i = 0; i < leafs; i++) {
    strip.setPixelColor(4*i+1, strip.Color(0, 255, 0));
    strip.setPixelColor(4*i, strip.Color(255, 0, 0));
    strip.show();
    //Serial.print(i);
    servo[i].write(up[i] - moveAngle);
    delay(1000);
    servo[i].write(up[i]);
    delay(1000);
  }
  return;*/

  for (int i = 0; i < leafs; i++) {
    if (upNow[i] == up[i]) {  // je nahore, nacti touch
      int touchValNow = touchRead(touchPin[i]);
      //Serial.println(touchValNow);
      if (touchValNow < touchVal[i] - 3) {
        /*Serial.print(i);
        Serial.print(" ");
        Serial.print(touchValNow);
        Serial.print(" ");
        Serial.print(touchVal[i]);*/


        if (lastLeaf == i && upNow[i] == up[i]) {
          specialita(i);
        } else {
          upTarget[i] = up[i] - moveAngle;
          tricks[i] = (int)random(maxTricks);
          colours[i] = (int)random(maxColours);
          lastLeaf = i;
        }
      }
    }
    if (upTarget[i] != up[i] || upNow[i] != up[i]) {  // je v pohybu
      int step = 1;
      if (upTarget[i] < upNow[i])
        step = -1;
      upNow[i] += step;
      servo[i].write(upNow[i]);
      //for (int j = 0; j < ledInLeaf; j++) {
      //  strip.setPixelColor(i * ledInLeaf + j, strip.Color(step > 0 ? 255 : 0, step > 0 ? 0 : 255, 0));
      //}
      colourTricks(i * ledInLeaf, up[i] - upNow[i], step, tricks[i], colours[i]);
      //colourTricks(i * ledInLeaf, up[i] - upNow[i], step, 3, colours[i]);
      if (upTarget[i] == upNow[i]) {  //obrat nebo koec
        if (upTarget[i] == up[i]) {   //konec, zhasni
          for (int j = 0; j < ledInLeaf; j++) {
            strip.setPixelColor(i * ledInLeaf + j, strip.Color(0, 0, 0));
          }
        } else  //obrat
          upTarget[i] = up[i];
      }
    }
  }
  strip.show();
  delay(40);
}

void colourTricks(int firstLED, int move, int stp, int trick, int colour) {
  switch (trick) {
    case 0:
      for (int j = 0; j < ledInLeaf; j++) {
        strip.setPixelColor(firstLED + j, selectColour(stp > 0 ? colour : (colour + (maxColours / 2))));
        //strip.setPixelColor(firstLED + j, selectColour(colour , 1));
        //strip.setPixelColor(firstLED + j, strip.Color(255, 255, 0));
      }
      break;

    case 1:
      for (int j = 0; j < ledInLeaf; j++) {
        strip.setPixelColor(firstLED + j, selectColour(colour, fader((move / 4 - j) % 5)));
      }
      break;

    case 2:
      for (int j = 0; j < ledInLeaf; j++) {
        strip.setPixelColor(firstLED + j, selectColour(move / 5));
      }
      break;

    case 3:
      for (int j = 0; j < ledInLeaf; j++) {
        strip.setPixelColor(firstLED + j, selectColour(colour, (move / 8) % 2 ? 1.0 : 0.0));
      }
      break;
    case 4:
      for (int j = 0; j < ledInLeaf; j++) {
        strip.setPixelColor(firstLED + j, selectColour(colour));
      }
      break;
  }
}

float fader(int numLED) {
  switch (numLED) {
    case 0:
      return 0.5;
    case 1:
      return 1;
    case 2:
      return 0.5;
    case 3:
      return 0.0;
    case 4:
      return 0.0;
  }
}

uint32_t selectColour(int col, float intensity) {
  if (col >= maxColours)
    col -= maxColours;
  if (intensity > 1.0)
    intensity = 1.0;
  switch (col) {
    case 0:
      return strip.Color(255 * intensity, 0, 0);
    case 1:
      return strip.Color(255 * intensity, 128 * intensity, 0);
    case 2:
      return strip.Color(255 * intensity, 255 * intensity, 0);
    case 3:
      return strip.Color(128 * intensity, 255 * intensity, 0);
    case 4:
      return strip.Color(0, 255 * intensity, 0);
    case 5:
      return strip.Color(0, 255 * intensity, 128 * intensity);
    case 6:
      return strip.Color(0, 255 * intensity, 255 * intensity);
    case 7:
      return strip.Color(0, 128 * intensity, 255 * intensity);
    case 8:
      return strip.Color(0, 0, 255 * intensity);
    case 9:
      return strip.Color(128 * intensity, 0, 255 * intensity);
    case 10:
      return strip.Color(255 * intensity, 0, 255 * intensity);
    case 11:
      return strip.Color(255 * intensity, 0, 128 * intensity);
  }
  return strip.Color(255, 255, 255);
}

int revert(int number) {
  if (number <= moveAngle)
    return number;
  return ((2 * moveAngle) - number);
}

void specialita(int verze) {
  int krokDelay = 15;
  int runs = 6;
  int leafDelay[leafs] = { -1, -1, -1, -1, -1, -1, -1, -1 };
  switch (verze) {
    case 0:
    case 3:
    case 6:
      for (int krok = 0; krok <= (krokDelay * leafs + 50) * runs; krok++) {
        for (int i = 0; i < leafs; i++) {
          int zpozdeni = krok / krokDelay;
          if (zpozdeni < leafs && leafDelay[i] == -1 && i == zpozdeni)  // nastartuj list
            leafDelay[i] = krok;
          if (krok / (krokDelay * leafs + 50) == runs - 1 && !((krok - leafDelay[i]) % (krokDelay * leafs + 50)))
            leafDelay[i] = -1;

          if (leafDelay[i] > -1) {
            int sPosunem = (krok - leafDelay[i]) % (krokDelay * leafs + 50);  //okruh 15*8=120
            if (sPosunem <= moveAngle * 2) {                                   // 2*50=100
              servo[i].write(up[i] - revert(sPosunem));
              for (int j = 0; j < ledInLeaf; j++) {
                strip.setPixelColor(i * ledInLeaf + j, selectColour(verze, 0.9 * revert(sPosunem) / (float)moveAngle));
              }
            }
          }
        }
        strip.show();
        delay(20);
      }
      break;
    case 1:
    case 4:
    case 7:

      for (int krok = 0; krok <= (moveAngle)*runs; krok++) {
        for (int i = 0; i < leafs; i++) {
          int zpozdeni = krok / moveAngle;
          if (zpozdeni < leafs && leafDelay[i] == -1 && i % 2 == zpozdeni)  // nastartuj list
            leafDelay[i] = krok;
          if (krok / (moveAngle) >= runs - 1 && !((krok - leafDelay[i]) % (moveAngle * 2)))
            leafDelay[i] = -1;

          if (leafDelay[i] > -1) {
            int sPosunem = (krok - leafDelay[i]) % (moveAngle * 2);
            if (sPosunem <= moveAngle * 2) {
              servo[i].write(up[i] - revert(sPosunem));
              for (int j = 0; j < ledInLeaf; j++) {
                strip.setPixelColor(i * ledInLeaf + j, selectColour(verze, 0.5 * revert(sPosunem) / (float)moveAngle));
              }
            }
          }
        }
        strip.show();
        delay(20);
      }
      break;
    default:
      for (int krok = 0; krok <= moveAngle * runs; krok++) {
        for (int i = 0; i < leafs; i++) {
          int sPosunem = (krok) % (moveAngle * 2);
          servo[i].write(up[i] - revert(sPosunem));
          for (int j = 0; j < ledInLeaf; j++) {
            strip.setPixelColor(i * ledInLeaf + j, selectColour(random(maxColours), 4 * revert(sPosunem) / moveAngle));
          }
        }
        strip.show();
        delay(20);
      }
  }
}