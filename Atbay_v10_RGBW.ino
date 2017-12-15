//Libraries to include
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//Neopixel info
#define PIXEL_PIN    0
#define NUM_LEDS 12
#define BRIGHTNESS 50

//Defining neopixel strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIXEL_PIN, NEO_GRBW + NEO_KHZ800);

//Turn Neopixel showtype to OFF---------------------------------------------------------------
int showType = 0;

//Soft tactile button "switch"
#define SW1 2

//Trinket_watch time code
long last = millis();
unsigned long sinceLast = millis();

int hours = 1;
int minutes = 0;
int seconds = 0;
int ms = 0;

//Establishes the mode variable which can have the following states
  //It enables the programmer to create a variable that can be a selected set of values.
enum Mode {
  OFF,
  REMINDER,
};

//Defining the mode = off referenced above------------------------------?
Mode mode = OFF;  //---? this was in original code
int docked = 0;

//Callback method prototypes--------------------------------------------?
//void updateInternalTime();
//void modeCheck();
//void readSwitches();
//void controlLED();
//void reminderLight();
//void rewardLight();

//This is a reference table for defining neopixel brightness
byte neopix_gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

void setup() {
  //Setup Trinket 5V 16MHz
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

   //Setup SW pin... soft tactile button "switch"
  pinMode(SW1, INPUT_PULLUP);

  //Setup Neopixel strip
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //Setup trinket_watch time code ---------------------I think the following is for RTC?
  //Serial.begin(9600);
//  char h1[] = {__TIME__[0], __TIME__[1], '\0'};
//  char m1[] = {__TIME__[3], __TIME__[4], '\0'};
//  char s1[] = {__TIME__[6], __TIME__[7], '\0'};
//  hours = atoi(h1);
//  minutes = atoi(m1);
//  seconds = atoi(s1);

  //These 3 lines are all original code, I didn't comment anything out
  //sw.enable();
  //led.enable();
//  setMode.enable();
}

void loop() {
  modeCheck();
  if (mode == OFF) {
    showType=0;
    controlLED(showType);
  }
  else if (mode == REMINDER) {
    readSwitches();
    if( docked == 0){
      showType=1;
      controlLED(showType);
    }
    if( docked == 1){
      showType=2;
      controlLED(showType);
    }
  }
  else {
    showType=0;
    controlLED(showType);
  }
  updateInternalTime();
}

void updateInternalTime() {
  ms += millis() - sinceLast;
  sinceLast = millis();
  if (ms >= 1000) {
    seconds += ms / 1000;
    ms = ms % 1000;
  }
  if (seconds >= 60) {
    minutes += seconds/60;
    seconds = seconds % 60;
  }
  if (minutes >= 60) {
    hours += minutes / 60;
    minutes = minutes % 60;
  }
  if (hours >= 24) {
//    hours = 0;
      hours = 1;
  }
}

//Check the time to see what mode it is in (OFF, REMINDER)
void modeCheck() {
  int t_mins = hours*60 + minutes;
  if ( t_mins > 120 ) {
    mode = OFF;
  } else if ( t_mins <= 120 && t_mins >60 ) {
    mode = REMINDER;
  } else {
    mode = OFF;
  }
}

//Reads switches, determines if phone is docked=1 or undocked=2
void readSwitches() {
  static int sw1 = HIGH;
  static int sw1_last = HIGH;

  sw1 = digitalRead(SW1);

    if ( (sw1_last==HIGH && sw1==LOW) && (docked==0) ) {
    //phone docked.
    docked = 1;
    //if ( DEBUG ) {
    //}
  }

    if ( (sw1_last==LOW && sw1==HIGH) ) {
    //phone undocked.
    docked = 0;
    //if ( DEBUG ) {
    //}
  }
  
  sw1_last = sw1;
}

//Controls what the neopixel should do
void controlLED(int i) {
  switch (i) {
    case 0:   strip.show(); // OFF: Initialize all pixels to 'off'
              break;
    case 1:   reminderLight(strip.Color(0, 0, 0, 255), 50); // Reminder light: Slow spin white light
              break;
    case 2:   rewardLight(10); // Reward light: Orange breathing light
              break;
  }
}

//Reward light flow
void rewardLight(uint8_t wait) {
  for(int j = 0; j < 256 ; j++){
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(255,100,0, neopix_gamma[j] ) );
        }
        delay(wait);
        strip.show();
      }
  for(int j = 255; j >= 0 ; j--){
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(255,100,0, neopix_gamma[j] ) );
        }
        delay(wait);
        strip.show();
      }
}

//Reminder light flow
void reminderLight(uint32_t c, uint16_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    // Set the i'th led to color above 
    strip.setPixelColor(i, c); 
    strip.show();
    // Now that we've shown the leds, reset the i'th led to black
    strip.setPixelColor(i, 0);
    // Wait a little bit before we loop around and do it again
    delay(120);
  }
}
