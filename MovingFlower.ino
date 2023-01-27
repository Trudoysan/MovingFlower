#include <Servo.h>
#include <Adafruit_NeoPixel.h>
static const int leafs = 8;
static const int ledInLeaf = 4;

static const int servoPin[leafs] = { 16, 17, 5, 18, 19, 21, 3, 1 };
static const int touchPin[leafs] = { T2, T3, T4, T5, T6, T7, T8, T9 };
static const int ledPin = 22;

Servo servo[leafs];
int touchVal[leafs] = { 0 };
int up[leafs] = { 90, 90, 90, 90, 90, 90, 90, 90 };
int upNow[leafs];
int upTarget[leafs];
static const int moveAngle = 50;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(leafs * ledInLeaf, ledPin, NEO_GRB + NEO_KHZ800);


static const int maxTricks = 4;
static const int maxColours = 12;
int tricks[leafs];
int colours[leafs];

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(50);
  strip.show();  // Initialize all pixels to 'off'

  for (int pokus = 0; pokus < 10; pokus++) {
    for (int i = 0; i < leafs; i++)
      touchVal[i] += touchRead(touchPin[i]);
    //Serial.println(touchVal);
    delay(100);
  }
  for (int i = 0; i < leafs; i++)
    touchVal[i] /= 10;

  //Serial.print("Done ");
  //Serial.println(touchVal);
  delay(200);
  for (int i = 0; i < leafs; i++)
    servo[i].attach(servoPin[i]);
  delay(200);
  for (int i = 0; i < leafs; i++) {
    servo[i].write(up[i]);
    upNow[i] = up[i];
    upTarget[i] = up[i];
  }
  randomSeed(analogRead(26));
}

void loop() {
  for (int i = 0; i < leafs; i++) {
    if (upNow[i] == up[i]) {  // je nahore, nacti touch
      int touchValNow = touchRead(touchPin[i]);
      //Serial.println(touchValNow);
      if (touchValNow < touchVal[i] - 1){
          upTarget[i] = up[i] - moveAngle;
          tricks[i] = random(maxTricks);
          colours[i] = random(maxColours);
      }
    }
    if (upTarget[i] != up[i] || upNow[i] != up[i]) {  // je v pohybu
      int step = 1;
      if (upTarget[i] < upNow[i])
        step = -1;
      upNow[i] += step;
      servo[i].write(upNow[i]);
      for (int j = 0; j < ledInLeaf; j++) {
        strip.setPixelColor(i * ledInLeaf + j, strip.Color(step > 0 ? 255 : 0, step > 0 ? 0 : 255, 0));
      }
      //colourTricks(i * ledInLeaf, up[i] - upNow[i],step, tricks[i],colours[i]);
      if (upTarget[i] == upNow[i]) {  //obrat nebo koec
        if (upTarget[i] == up[i]) {   //konec, zhasni
          for (int j = 0; j < ledInLeaf; j++) {
            strip.setPixelColor(i * ledInLeaf + j, strip.Color(0, 0, 0));
          }
        } else  //obrat
          upTarget[i] = up[i];
      }
    }
    strip.show();
  }
  delay(50);
}

void colourTricks(int firstLED,int move, int stp, int trick, int colour) {
  switch(trick){
    case 0:
        for (int j = 0; j < ledInLeaf; j++) {
          strip.setPixelColor(firstLED + j, selectColour(step > 0 ? colour : -colour));
        }
       break;

    case 1:
        for (int j = 0; j < ledInLeaf; j++) {
          strip.setPixelColor(firstLED + j, selectColour(colour,(move-j)%5));
        }
        break;
   
    case 2:
        for (int j = 0; j < ledInLeaf; j++) {
          strip.setPixelColor(firstLED + j, selectColour(move/4));
        }
       break;

    case 3:
        for (int j = 0; j < ledInLeaf; j++) {
          strip.setPixelColor(firstLED + j, selectColour(colour,(move/5)%2 ? 1 : 32000));
        }
       break;

  }
}

int fader(int numLED){
  switch(numLED){
    case 0:
      return 4;
    case 1:
      return 1;
    case 2:
      return 4;
    case 3:
      return 32000;
    case 4:
      return 32000;
  }
} 

uint32_t selectColour(int col, int intensity = 1){
  if(col<0)
  col += maxColours/2;
  if(col>maxColours)
  col -=  maxColours;

  switch(col){
    case :
     return strip.Color(255/intensity,0,0);
    case :
     return strip.Color(255/intensity,128/intensity,0);
         case :
     return strip.Color(255/intensity,255/intensity,0);; 
     case :
     return strip.Color(128/intensity,255/intensity,0);    
     case :
     return strip.Color(0,255/intensity,0);    
     case :
     return strip.Color(0,255/intensity,128/intensity);    
     case :
     return strip.Color(0,255/intensity,255/intensity);    
     case :
     return strip.Color(0,128/intensity,255/intensity);	    
     case :
     return strip.Color(0,0,255/intensity);	    
     case :
     return strip.Color(128/intensity,0,255/intensity);    
     case :
     return strip.Color(255/intensity,0,255/intensity);    
     case :
     return strip.Color(255/intensity,0,128/intensity);
  }
}