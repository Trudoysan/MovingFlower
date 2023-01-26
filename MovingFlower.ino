#include <Servo.h>
#include <Adafruit_NeoPixel.h>

static const int servoPin1 = 17;
static const int servoPin2 = 16;

static const int ledPin = 22;

Servo servo1;
Servo servo2;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(5, ledPin, NEO_GRB + NEO_KHZ800);
int touchVal = 0;
int up;

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(50);
  strip.show();  // Initialize all pixels to 'off'
  for (int pokus = 0; pokus < 20; pokus++) {
    touchVal += touchRead(T3);
    Serial.println(touchVal);
    delay(100);
  }
  touchVal /= 20;
  Serial.print("Done ");
  Serial.println(touchVal);
  delay(200);
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  delay(200);

  servo1.write(90);
  servo2.write(150);
  up = 150;
}

void loop() {
  int touchValNow = touchRead(T3);
  Serial.println(touchValNow);  // get value using T0
  if (touchValNow < touchVal - 1) {
    //strip.fill(strip.Color(255, 0, 0));
    strip.show();
    if (up == 150)
      for (int i = 150; i >= 90; i--) {
        strip.fill(strip.Color(0, 0, 0));
        strip.setPixelColor(i % 5, strip.Color(255, 0, 0));
        strip.show();
        servo2.write(i);
        delay(200);
      }
    up = 90;
    Serial.println("89");
  } else {
    strip.fill(strip.Color(0, 255, 0));
    strip.show();
    if (up == 90)
      for (int i = 90; i <= 150; i++) {
        servo2.write(i);
        delay(50);
      }
    up = 150;
    Serial.println("90");
  }

  delay(100);
  /* for (int posDegrees = 0; posDegrees <= 5; posDegrees++) {
      servo1.write(posDegrees);
      servo2.write(posDegrees);
      // Serial.println(posDegrees);
      int color = posDegrees % 5;
      strip.setPixelColor(color, strip.Color(255, 0, 0));
      if (color == 4)
        strip.fill(strip.Color(0, 0, 0));
      strip.show();
      delay(20);
    }

    for (int posDegrees = 5; posDegrees >= 0; posDegrees--) {
      servo1.write(posDegrees);
      servo2.write(posDegrees);
      // Serial.println(posDegrees);
      delay(20);
    }
  }*/
}