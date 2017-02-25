/*
 *  Sketch to create an interactive sword with event based outputs
 * 
 *  Hardware requirements:
 *  - Adafruit Circuit Playground
 *  
 *  Software requirements:
 *  - Adafruit Circuit Playground library
 *  
 *  
 *  Written by Matthew Swarts, 2016
 *  License: MIT License (https://opensource.org/licenses/MIT)
 */

#include <Adafruit_CircuitPlayground.h>
#include <Adafruit_SleepyDog.h>

#define TONE_MIN 220
#define TONE_MAX 1760
#define TONE_DURATION 600
int colorID = 0;
int toneID = 440;

#define RAINBOW_SPEED 10
bool doColorCycle = false;
long lastStable = 0L;
#define STABLE_DURATION 3000
#define STABLE_MIN 7
#define STABLE_MAX 11

// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
// likely up to 127 is valid
#define CLICKTHRESHHOLD 80
bool isClicked = true;



void setup() {
  // Setup serial port for debugging
  delay(1000);
  Serial.begin(115200);
  Serial.println("Circuit Playground Interactive Sword");
  CircuitPlayground.begin();
  
  //Serial.print("Range = "); Serial.print(2 << CircuitPlayground.lis.getRange());  
  //Serial.println("G"); //default is 2G
  CircuitPlayground.lis.setRange(LIS3DH_RANGE_8_G);   // 2, 4, 8 or 16 G!, default is 2G
  
  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  CircuitPlayground.lis.setClick(1, CLICKTHRESHHOLD);

  newClang();
  lastStable = millis();
}

void loop() {
  // Check if slide switch is on the left (false) and go to sleep.
  while (!CircuitPlayground.slideSwitch()) {
    // Turn off the pixels, then go into deep sleep for three seconds.
    CircuitPlayground.clearPixels();
    Watchdog.sleep(3000);
  }

  if(!isClicked)
  {
    uint8_t click = CircuitPlayground.lis.getClick();
    //if (click == 0) { return; }
    //if (! (click & 0x30)) return;
    //Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
    if (click & 0x10){
      Serial.println(" single click");
      isClicked = true;
    }
    //if (click & 0x20) Serial.print(" double click");
    //Serial.println();
  }

  if(isClicked)
  {
    newClang();
    setPixelColorAll(CircuitPlayground.colorWheel(colorID));
    CircuitPlayground.playTone(toneID,TONE_DURATION);
    delay(1000);
    CircuitPlayground.clearPixels();
    isClicked = false;
  }
  
  float accely = CircuitPlayground.motionY();
  Serial.println(accely);
  if(accely>STABLE_MIN && accely<STABLE_MAX){
    if(millis()-lastStable>STABLE_DURATION){
      doColorCycle = true;
    }
  }else
  {
    CircuitPlayground.clearPixels();
    lastStable = millis();
    doColorCycle = false;
  }

  if(doColorCycle){
    uint32_t offset = millis() / RAINBOW_SPEED;
    // Loop through each pixel and set it to an incremental color wheel value.
    for(int i=0; i<10; ++i) {
      CircuitPlayground.strip.setPixelColor(i, CircuitPlayground.colorWheel(((i * 256 / 10) + offset) & 255));
    }
    // Show all the pixels.
    CircuitPlayground.strip.show();
  }
}

void newClang()
{
  colorID = random(0,225);
  toneID = random(TONE_MIN,TONE_MAX);
}

void setPixelColorAll(uint32_t c){
  CircuitPlayground.strip.setPixelColor(0,c);
  CircuitPlayground.strip.setPixelColor(1,c);
  CircuitPlayground.strip.setPixelColor(2,c);
  CircuitPlayground.strip.setPixelColor(3,c);
  CircuitPlayground.strip.setPixelColor(4,c);
  CircuitPlayground.strip.setPixelColor(5,c);
  CircuitPlayground.strip.setPixelColor(6,c);
  CircuitPlayground.strip.setPixelColor(7,c);
  CircuitPlayground.strip.setPixelColor(8,c);
  CircuitPlayground.strip.setPixelColor(9,c);
  CircuitPlayground.strip.show();
}

/*
// Linear interpolation function
float lerp(float x, float xmin, float xmax, float ymin, float ymax) {
  if (x >= xmax) {
    return ymax;
  }
  if (x <= xmin) {
    return ymin;
  }
  return ymin + (ymax-ymin)*((x-xmin)/(xmax-xmin));
}*/
