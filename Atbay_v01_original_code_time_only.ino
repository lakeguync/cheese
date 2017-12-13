#include <TimeLib.h>
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>

#define STEP 2
#define ITER_DELAY 20
#define ITER_DELAY_REWARD 40

#define UBT_HR 23
#define UBT_MN 00
#define SBT_HR 1
#define SBT_MN 0
#define REM_HR 0
#define REM_MN 0

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

const int UBT_TIME = UBT_HR*60 + UBT_MN;                // 1,380
const int SBT_TIME = UBT_TIME - SBT_HR*60 + SBT_MN;     // 1,320
const int REM_TIME = SBT_TIME - REM_HR*60 + REM_MN;     // 1,320

enum Mode {
  OFF,
  REMINDER,
  SBT
};

Mode mode = OFF;
int docked = 0;
int fadeStep = STEP;                // 2
int delayTime = ITER_DELAY;         // 20

Scheduler ts;

//Callback method prototypes
void readSwitches();
void controlLED();
void modeCheck();

//Tasks
Task led(delayTime, TASK_FOREVER, &controlLED);
Task sw(50, TASK_FOREVER, &readSwitches);
Task setMode(TASK_MINUTE, TASK_FOREVER, &modeCheck);

void setup() {
  //setup time
  Serial.begin(9600);
//  setSyncProvider(requestSync);  //set function to call when sync required
//  Serial.println("Set current time");
//  while (timeStatus()== timeNotSet) {
//    if (Serial.available()) {
//      processSyncMessage();
//    }
//  }
  //set current time to Jan 1, 2017 22:00
  const unsigned long DEFAULT_TIME = 1483308000; // Jan 1 2013
  setTime(DEFAULT_TIME); // Sync Arduino clock to the time received on the serial port
  Serial.println("Time Set!");
  digitalClockDisplay();

  //setup scheduler
  ts.init();                                                //starts task scheduler
//  Serial.println("Initialized scheduler");

  ts.addTask(sw);
  ts.addTask(led);
  ts.addTask(setMode);

  //sw.enable();
  //led.enable();
  setMode.enable();
}

void loop() {
  ts.execute();
}

void controlLED() {
  static int state = 0;

  switch (state) {
    case 0:   //OFF state
              if ( mode == REMINDER || mode == SBT ) {
                if ( DEBUG ) {
                  Serial.println("Switching to REMINDER");
                  digitalClockDisplay();
                }
                state = 1;
                fadeStep = STEP;
                brightness = BRIGHTNESS_MIN;
                delayTime = ITER_DELAY;         //20       **************************************************
              } else {
                rgb(off,0);
              }
              break;
    case 1:   //breathing white light
              if ( mode == OFF ) {
                if ( DEBUG ) {
                  Serial.println("Switching to OFF");
                  digitalClockDisplay();
                }
                state = 0;
                rgb(off,0);
                led.disable();
              } else if ( docked == 1 ) {
                if ( DEBUG ) {
                  Serial.println("Switching to REWARD");
                  digitalClockDisplay();
                }
                state = 2;
                fadeStep = STEP;
                brightness = BRIGHTNESS_MIN_REWARD;
                delayTime = ITER_DELAY_REWARD;        //40        *********************************************************************
              } else {
                if ( (brightness + fadeStep) <= BRIGHTNESS_MIN || (brightness + fadeStep) >= BRIGHTNESS_MAX ) {
                  fadeStep = -fadeStep;
                }
                brightness = brightness + fadeStep;
                rgb(white, brightness);
              }
              break;
    case 2:   //breathing orange reward
              if ( mode == OFF ) {
                if ( DEBUG ) {
                  Serial.println("Switching to OFF");
                  digitalClockDisplay();
                }
                state = 0;
                rgb(off,0);
                led.disable();
              } else if ( docked == 0 && mode == SBT ) {
                if ( DEBUG ) {
                  Serial.println("Switching to CHEAT");
                  digitalClockDisplay();
                }
                state = 3;
                brightness = 0;
                fadeStep = STEP;
                delayTime = 20;           //20       **************************************************
              } else if ( docked == 0 && mode == REMINDER ) {
                if ( DEBUG ) {
                  Serial.println("Switching to REMINDER");
                  digitalClockDisplay();
                }
                state = 1;
                fadeStep = STEP;
                brightness = BRIGHTNESS_MIN;
                delayTime = ITER_DELAY;     //20       **************************************************
              } else {
                //rgb(orange,255);
                if ( (brightness + fadeStep) <= BRIGHTNESS_MIN_REWARD || (brightness + fadeStep) >= BRIGHTNESS_MAX_REWARD ) {
                  fadeStep = -fadeStep;
                }
                brightness = brightness + fadeStep;
                rgb(orange, brightness);
              }
              break;
    case 3:   //cheating - 1 red pulse
              if ( mode == OFF ) {
                if ( DEBUG ) {
                  Serial.println("Switching to OFF");
                  digitalClockDisplay();
                }
                state = 0;
                rgb(off,0);
                led.disable();
              } else if ( docked == 1 ) {
                if ( DEBUG ) {
                  Serial.println("Switching to REWARD");
                  digitalClockDisplay();
                }
                state = 2;
                fadeStep = STEP;
                brightness = BRIGHTNESS_MIN_REWARD;
                delayTime = ITER_DELAY_REWARD;          //40       **************************************************
              } else {
                rgb(red,brightness);
                if ( 255 - brightness < fadeStep ) {
                  fadeStep = -fadeStep;
                }
                brightness = brightness + fadeStep;
                if ( brightness == 0 ) {
                  //done with pulse. move to REMINDER.
                  if ( DEBUG ) {
                    Serial.println("Switching to REMINDER");
                    digitalClockDisplay();
                  }
                  state = 1;
                  fadeStep = STEP;
                  brightness = BRIGHTNESS_MIN;
                  delayTime = ITER_DELAY;             //20       **************************************************
                }
              }
              break;
    default:  //goto state 0
              state = 0;
              if ( DEBUG ) {
                Serial.println("Switching to OFF (default)");
                digitalClockDisplay();
              }
  }
  led.setInterval(delayTime);     //       ************************************************** ??
}

void modeCheck() {
  time_t t = now();
  int t_mins = hour(t)*60 + minute(t);
  if ( t_mins < REM_TIME ) {
    mode = OFF;
    if ( DEBUG ) {
      Serial.println("OFF");
      digitalClockDisplay();
    }
  } else if ( t_mins < SBT_TIME ) {
    mode = REMINDER;
    sw.enableIfNot();
    led.enableIfNot();
    if ( DEBUG ) {
      Serial.println("REMINDER");
      digitalClockDisplay();
    }
  } else if ( t_mins < UBT_TIME ) {
    mode = SBT;
    sw.enableIfNot();
    led.enableIfNot();
    if ( DEBUG ) {
      Serial.println("SBT");
      digitalClockDisplay();
    }
  } else {
    mode = OFF;
    sw.disable();
  }
}
