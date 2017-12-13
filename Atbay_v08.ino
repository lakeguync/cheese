//Libraries to include
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//#define DEBUG 1                               //Delete later? --------------------------------------

//Neopixel info
#define PIXEL_PIN    0
#define PIXEL_COUNT  16
#define NUM_LEDS 12
#define BRIGHTNESS 50

//Soft tactile button "switch"
#define SW1 2

//TRINKET_WATCH TIME CODE*****************************************
unsigned long sinceLast = millis();

int hours = 1;
int minutes = 0;
int seconds = 0;
int ms = 0;

//****************************************************************

//Time-based constants for triggering events                              //Delete later? ----------------------------
//#define STEP 2
//#define ITER_DELAY 20
//#define ITER_DELAY_REWARD 40

//Time based constants for user bedtime, smartphone bedtime, and reminder light
//***Old way: These are used later to compare to a clock running using timeLib.h
//***New way: These are used later to compare to the trinket_watc time code
#define UBT_HR 23
#define UBT_MN 00
#define SBT_HR 1
#define SBT_MN 0
#define REM_HR 0
#define REM_MN 0

//Times that are set for establishing light flow            //Delete later?--------------------------------------------------------
  //int: Integers are your primary data-type for number storage.
const int UBT_TIME = UBT_HR*60 + UBT_MN;              //1,380 minutes or 23 hours
const int SBT_TIME = UBT_TIME - SBT_HR*60 + SBT_MN;   //
const int REM_TIME = SBT_TIME - REM_HR*60 + REM_MN;


//Establishes the mode variable which can have the following states
  //It enables the programmer to create a variable that can be a selected set of values.
enum Mode {
  OFF,
  REMINDER,
  SBT
};

//Defining the mode = off referenced above
Mode mode = OFF;
int docked = 0;

//Callback method prototypes
void readSwitches();
void controlLED();
void modeCheck();

//Defining neopixel strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIXEL_PIN, NEO_GRBW + NEO_KHZ800);

//Setup button and neopixel variables ----------------------------------------------------------
bool oldState = HIGH;
int showType = 0;

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

  //Setup neopixel strip & button
  pinMode(SW1, INPUT_PULLUP);
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //Setup SW pins
  pinMode(SW1, INPUT_PULLUP);

  //TRINKET_WATCH TIME CODE*****************************************
  //Serial.begin(9600);
  char h1[] = {__TIME__[0], __TIME__[1], '\0'};
  char m1[] = {__TIME__[3], __TIME__[4], '\0'};
  char s1[] = {__TIME__[6], __TIME__[7], '\0'};
  hours = atoi(h1);
  minutes = atoi(m1);
  seconds = atoi(s1);

  int mnu = 0;                            // Not sure if this is needed? ---------------------------------------------------------
  int timeMode = 0;
  //***************************************************************


  //These 3 lines are all original code, I didn't comment anything out
  //sw.enable();
  //led.enable();
//  setMode.enable();
}

void loop() {
//TRINKET_WATCH TIME CODE***************************************************************************************
  updateInternalTime();
//**************************************************************************************************************

//New additions to loop...
  readSwitches();
  modeCheck();
  controlLED();
}

//TRINKET_WATCH TIME CODE***************************************************************************************
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
    hours = 0;
  }
}

int mins[][2] = {
                {64, 10}, {69, 11}, // :00
                {74, 12}, {79, 16}, // :05
                {83, 20}, {85, 26}, // :10
                {85, 32}, {84, 38}, // :15
                {83, 43}, {78, 49}, // :20
                {73, 52}, {68, 53}, // :25
                {64, 54}, {59, 53}, // :30
                {55, 52}, {50, 47}, // :35
                {45, 43}, {43, 38}, // :40
                {42, 32}, {44, 26}, // :45
                {45, 20}, {50, 16}, // :50
                {53, 14}, {59, 12}  // :55
};
           
int hors[][2] = {  
                {64, 21}, // 12
                {70, 23}, // 1
                {73, 27}, // 2
                {75, 32}, // 3
                {73, 37}, // 4
                {70, 42}, // 5
                {64, 43}, // 6
                {58, 40}, // 7
                {55, 37}, // 8
                {53, 32}, // 9
                {54, 27}, // 10
                {59, 24} // 11
                
};
//**********************************************************************************

//This reads the switches and determines whether phone is docked=1 or undocked=2
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

    if ( (sw1_last==LOW && sw1==HIGH) && ( sw1==HIGH ) ) {
    //phone undocked.
    docked = 0;
    //if ( DEBUG ) {
    //}
  }

  sw1_last = sw1;
}

void controlLED() {
  static int state = 0;

  switch (state) {
    case 0:   //OFF state
              if ( mode == REMINDER || mode == SBT ) {
                //if ( DEBUG ) {
                  //Serial.println("Switching to OFF");
                  //digitalClockDisplay();
                //}
                state = 1;
                //delayTime = ITER_DELAY;
              } else {
                strip.show(); // Initialize all pixels to 'off'
              }
              break;
    case 1:   //breathing white light
              if ( mode == OFF ) {
                //if ( DEBUG ) {
                  //Serial.println("Switching to OFF");
                  //digitalClockDisplay();
                //}
                state = 0;
                strip.show(); // Initialize all pixels to 'off'
                //led.disable();
              } else if ( docked == 1 ) {
                //if ( DEBUG ) {
                  //Serial.println("Switching to REWARD");
                  //digitalClockDisplay();
                //}
                state = 2;
                //delayTime = ITER_DELAY_REWARD;
              } else {
                reminderLight(strip.Color(0, 0, 0, 255), 50); // Slow spinning white light
              }
              break;
    case 2:   //breathing orange reward
              if ( mode == OFF ) {
                //if ( DEBUG ) {
                  //Serial.println("Switching to OFF");
                  //digitalClockDisplay();
                //}
                state = 0;
                //rgb(off,0);
                strip.show(); // Initialize all pixels to 'off'
                //led.disable();
              } else if ( docked == 0 && mode == SBT ) {
                //if ( DEBUG ) {
                  //Serial.println("Switching to CHEAT");
                  //digitalClockDisplay();
                //}
                state = 3;
                //brightness = 0;
                //fadeStep = STEP;
                //delayTime = 20;
              } else if ( docked == 0 && mode == REMINDER ) {
                //if ( DEBUG ) {
                  //Serial.println("Switching to REMINDER");
                  //digitalClockDisplay();
                //}
                state = 1;
                //delayTime = ITER_DELAY;
              } else {
                rewardLight(10);
              }
              break;
    default:  //goto state 0
              state = 0;
              //if ( DEBUG ) {
                //Serial.println("Switching to OFF (default)");
                //digitalClockDisplay();
              //}
  }
  //led.setInterval(delayTime);------------------------------------------------------------Delete later?
}

//void modeCheck() {
//  time_t t = now();
//  int t_mins = hour(t)*60 + minute(t);
//  if ( t_mins < REM_TIME ) {
//    mode = OFF;
//    //if ( DEBUG ) {
//      //Serial.println("OFF");
//      //digitalClockDisplay();
//    //}
//  } else if ( t_mins < SBT_TIME ) {
//    mode = REMINDER;
//    sw.enableIfNot();
//    //led.enableIfNot();
//    //if ( DEBUG ) {
//      //Serial.println("REMINDER");
//      //digitalClockDisplay();
//    //}
//  } else if ( t_mins < UBT_TIME ) {
//    mode = SBT;
//    sw.enableIfNot();
//    //led.enableIfNot();
//    //if ( DEBUG ) {
//      //Serial.println("SBT");
//      //digitalClockDisplay();
//    //}
//  } else {
//    mode = OFF;
//    sw.disable();
//  }
//}

void modeCheck() {
  if ( minutes < REM_TIME ) {
    mode = OFF;
    //if ( DEBUG ) {
      //Serial.println("OFF");
      //digitalClockDisplay();
    //}
  } else if ( minutes < SBT_TIME ) {
    mode = REMINDER;
    //sw.enableIfNot();----------------------------?
    //led.enableIfNot();
    //if ( DEBUG ) {
      //Serial.println("REMINDER");
      //digitalClockDisplay();
    //}
  } else if ( minutes < UBT_TIME ) {
    mode = SBT;
    //sw.enableIfNot(); ------------------------------?
    //led.enableIfNot();
    //if ( DEBUG ) {
      //Serial.println("SBT");
      //digitalClockDisplay();
    //}
  } else {
    mode = OFF;
    //sw.disable(); -----------------------------------?
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
    // now that we've shown the leds, reset the i'th led to black
    strip.setPixelColor(i, 0);
    // Wait a little bit before we loop around and do it again
    delay(120);
  }
}
