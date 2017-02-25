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
//#include <Adafruit_SleepyDog.h>
#include "clangdata.h"

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define SAMPLE_RATE 8000
volatile uint16_t sample;
byte lastSample;
int sampleID = 0; //0-3
const unsigned char* const sounddata_table[] PROGMEM = {clang1_data, clang2_data, clang3_data, clang4_data};

uint8_t clangColors[] = {32,64,128,192};

// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
// likely up to 127 is valid
#define CLICKTHRESHHOLD 80
bool isClicked = true;
bool wasClicked = false;

void setup() {
  // Setup serial port for debugging
  delay(1000);
  //Serial.begin(115200);
  //Serial.println("Circuit Playground Interactive Sword");
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
}

void loop() {
  // Check if slide switch is on the left (false) and go to sleep.
  while (!CircuitPlayground.slideSwitch()) {
    // Turn off the pixels, then go into deep sleep for three seconds.
    CircuitPlayground.clearPixels();
    delay(3000);
    //Watchdog.sleep(3000);
  }

  if(!isClicked)
  {
    uint8_t click = CircuitPlayground.lis.getClick();
    //if (click == 0) { return; }
    //if (! (click & 0x30)) return;
    //Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
    if (click & 0x10){
      //Serial.println(" single click");
      isClicked = true;
    }
    //if (click & 0x20) Serial.print(" double click");
    //Serial.println();
  }

  if(isClicked)
  {
    newClang();
    setPixelColorAll(CircuitPlayground.colorWheel(clangColors[sampleID]));
    startPlayback();
    delay(500);
    CircuitPlayground.clearPixels();
    isClicked = false;
  }
  
  //float accel = CircuitPlayground.motionY(); 
}

void newClang()
{
  sampleID = random(0,5);
}

/*
void playClang(int num){
  switch(num){
    case 1:
      setPixelColorAll(CircuitPlayground.colorWheel(CLANGCOLOR1));
      //startPlayback(clang1_data, sizeof(clang1_length));
      for(int i=0;i<clang1_length;i++){
        analogWrite(CPLAY_BUZZER,clang1_data[i]);
      }
      analogWrite(CPLAY_BUZZER,0);
    break;
    
    case 2:
      setPixelColorAll(CircuitPlayground.colorWheel(CLANGCOLOR2));
      //startPlayback(clang1_data, sizeof(clang2_length));
      for(int i=0;i<clang2_length;i++){
        analogWrite(CPLAY_BUZZER,clang2_data[i]);
      }
      analogWrite(CPLAY_BUZZER,0);
    break;
    
    case 3:
      setPixelColorAll(CircuitPlayground.colorWheel(CLANGCOLOR3));
      //startPlayback(clang1_data, sizeof(clang3_length));
      for(int i=0;i<clang3_length;i++){
        analogWrite(CPLAY_BUZZER,clang3_data[i]);
      }
      analogWrite(CPLAY_BUZZER,0);
    break;
    
    case 4:
      setPixelColorAll(CircuitPlayground.colorWheel(CLANGCOLOR4));
      //startPlayback(clang1_data, sizeof(clang4_length));
      for(int i=0;i<clang4_length;i++){
        analogWrite(CPLAY_BUZZER,clang4_data[i]);
      }
      analogWrite(CPLAY_BUZZER,0);
    break;
    
    default:
      //do nothing
    break;
  }
}
*/

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

void stopPlayback()
{
    // Disable playback per-sample interrupt.
    TIMSK1 &= ~_BV(OCIE1A);

    // Disable the per-sample timer completely.
    TCCR1B &= ~_BV(CS10);

    // Disable the PWM timer.
    TCCR3B &= ~_BV(CS10);

    digitalWrite(CPLAY_BUZZER, LOW);
}

// This is called at 8000 Hz to load the next sample.
ISR(TIMER1_COMPA_vect) {
    if (sample >= clang_lengths[sampleID]) {
        if (sample == clang_lengths[sampleID] + lastSample) {
            stopPlayback();
        }
        else {
              // Ramp down to zero to reduce the click at the end of playback.
              OCR3A = clang_lengths[sampleID] + lastSample - sample;
        }
    }
    else {
      
      OCR3A = pgm_read_byte((&sounddata_table[sampleID])[sample]);
      //OCR3A = pgm_read_byte(&sounddata_data[sample]);
    }

    ++sample;
}

void startPlayback()
{
    pinMode(CPLAY_BUZZER, OUTPUT);

    // Set up Timer 2 to do pulse width modulation on the speaker
    // pin.

    // Use internal clock (datasheet p.160)
    //ASSR &= ~(_BV(EXCLK) | _BV(AS3));

    // Set fast PWM mode  (p.157)
    TCCR3A |= _BV(WGM31) | _BV(WGM30);
    TCCR3B &= ~_BV(WGM32);

    // Do non-inverting PWM on pin OC2A (p.155)
    // On the Arduino this is pin 11.
    TCCR3A = (TCCR3A | _BV(COM3A1)) & ~_BV(COM3A0);
    TCCR3A &= ~(_BV(COM3B1) | _BV(COM3B0));
    // No prescaler (p.158)
    TCCR3B = (TCCR3B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    // Set initial pulse width to the first sample.
    OCR3A = pgm_read_byte( &(&sounddata_table[sampleID])[0]);
    //OCR3A = pgm_read_byte(&sounddata_data[0]);




    // Set up Timer 1 to send a sample every interrupt.

    cli();

    // Set CTC mode (Clear Timer on Compare Match) (p.133)
    // Have to set OCR1A *after*, otherwise it gets reset to 0!
    TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
    TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

    // No prescaler (p.134)
    TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    // Set the compare register (OCR1A).
    // OCR1A is a 16-bit register, so we have to do this with
    // interrupts disabled to be safe.
    OCR1A = F_CPU / SAMPLE_RATE;    // 16e6 / 8000 = 2000

    // Enable interrupt when TCNT1 == OCR1A (p.136)
    TIMSK1 |= _BV(OCIE1A);

    //OCR3A = pgm_read_byte(&(&sounddata_table[0])[sample]);
    lastSample = pgm_read_byte(&(&sounddata_table[sampleID])[clang_lengths[sampleID]-1]);
    //lastSample = pgm_read_byte(&sounddata_data[sounddata_length-1]);
    sample = 0;
    sei();
}
