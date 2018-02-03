#include "SD.h"
#include "TMRpcm.h"
#include "SPI.h"
#include "QList.h"
#include "QList.cpp"    // QList strange bug fix


/*
 *  A simple SD card based music player intended for a battle bot. 
 *  Connect a (battery powered) speaker to the speaker pin, upload files named "1", "2" etc
 *  to SD card and connect reader as instructed. Designed for flippable switch style buttons.
 */

// CUSTOMIZATION - Variables you might want to change
// ----------------------------------------------------------------------------------------------------------
#define SD_ChipSelectPin 10		// SD card select pin
#define SpeakerPin 9			// Output to speaker
#define VOLUME 5			// Volume 0 to 7
#define NUM_OF_SONGS 27                 // Number of sound files on SD card
#define loopDelay 1000			// Grace period between two sound-starts		
//-----------------------------------------------------------------------------------------------------------

// SD card driver pins (can't be changed here)
# define SCK 13
# define MISO 12
# define MOSI 11

// Music button A variables
#define SOUND_SIGNAL_A 0 		// INTERRUPT 0 = DIGITAL PIN 2 - use the interrupt number in attachInterrupt
#define SOUND_SIGNAL_A_PIN 2 		// INTERRUPT 0 = DIGITAL PIN 2 - use the PIN number in digitalRead
volatile boolean newSoundA= false; 	// set in the interrupt and read in the loop
int ulStartPeriodA = 0;
boolean newInterruptSignalA = false;
int valA;
boolean toggleA = false;

// Music button B variables
#define SOUND_SIGNAL_B 1 		// INTERRUPT 1 = DIGITAL PIN 3 - use the interrupt number in attachInterrupt
#define SOUND_SIGNAL_B_PIN 3 		// INTERRUPT 1 = DIGITAL PIN 3 - use the PIN number in digitalRead
volatile boolean newSoundB= false; 	// set in the interrupt and read in the loop
int ulStartPeriodB = 0;
boolean newInterruptSignalB = false;
int valB;
boolean toggleB = false;

// Queue operation
String x;			// Next track from queue
char title[20];			// Char array title
QList<String> soundQueue;	// Music queue

TMRpcm tmrpcm;			// Card sound player

void setup(){
  // Prepare pins
  pinMode(SOUND_SIGNAL_A_PIN, INPUT_PULLUP);
  pinMode(SOUND_SIGNAL_B_PIN, INPUT_PULLUP);
  // Attach interrupts:
  attachInterrupt(digitalPinToInterrupt(SOUND_SIGNAL_A_PIN),calcPWMA,CHANGE);
  attachInterrupt(digitalPinToInterrupt(SOUND_SIGNAL_B_PIN),calcPWMB,CHANGE);
  // Set up SD card
  if (!SD.begin(SD_ChipSelectPin)) {
    Serial.println("SD fail");
    return;
  }
  // Configure audio output
  tmrpcm.speakerPin = SpeakerPin;
  tmrpcm.setVolume(VOLUME);
}

void loop(){  
  // Handle flags set by interrupts:
  if (newInterruptSignalA) {
    soundQueue.push_back(String(random(1,NUM_OF_SONGS)));
    newInterruptSignalA = false;
  }
  if (newInterruptSignalB) {
    soundQueue.push_back(String(0));
    newInterruptSignalB = false;
  }
  // Play a queued song
  if (soundQueue.size()) {
    x = soundQueue[0];
    soundQueue.pop_front();
    x.toCharArray(title,20);
    tmrpcm.play(title);     
  }  
  delay(loopDelay);
 }

void calcPWMA() {
  // Calculate pwm value of signal 1, and play sound if button switch detected
  if(digitalRead(SOUND_SIGNAL_A_PIN) == HIGH){
    ulStartPeriodA = micros();
  }
  else {
    if(ulStartPeriodA && (newInterruptSignalA == false)){
      valA= (int)(micros() - ulStartPeriodA);
      ulStartPeriodA = 0;
      if (toggleA && (valA > 1600)) {
        newInterruptSignalA = true;
        toggleA = !toggleA;
      } else if (!toggleA && (valA < 1200)) {
        newInterruptSignalA = true;
        toggleA = !toggleA;
      }
    }
  }
}

void calcPWMB() {
  // Calculate pwm value of signal 2, and play sound if button switch detected
  if(digitalRead(SOUND_SIGNAL_B_PIN) == HIGH){
    ulStartPeriodB = micros();
  }
  else {
    if(ulStartPeriodB && (newInterruptSignalB == false)){
      valB= (int)(micros() - ulStartPeriodB);
      ulStartPeriodB = 0;
      if (toggleB && (valB > 1600)) {
        newInterruptSignalB = true;
        toggleB = !toggleB;
      } else if (!toggleB && (valB < 1200)) {
        newInterruptSignalB = true;
        toggleB = !toggleB;
      }
    }
  }
}


