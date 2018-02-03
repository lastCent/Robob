// Advanced battle bot wheel control.
// TODO: Add rotating in place

// Debug serial output left on, since some tweaking may be needed for redeployment 

// Forward-Backward steering
#define THROTTLE_SIGNAL_IN_A 0		// INTERRUPT 0 = DIGITAL PIN 2 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_A_PIN 2	// INTERRUPT 0 = DIGITAL PIN 2 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE_A 1556		// this is the duration in microseconds of neutral throttle on an electric RC Car

volatile int nThrottleInA = NEUTRAL_THROTTLE_A;	// volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriodA = 0;	// set in the interrupt
volatile boolean bNewThrottleSignalA = false;	// set in the interrupt and read in the loop

// Left-Right steering
#define THROTTLE_SIGNAL_IN_B 1		// INTERRUPT 1 = DIGITAL PIN 3 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_B_PIN 3 	// INTERRUPT 1 = DIGITAL PIN 3 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE_B 1540 	// This value may require changing the DX4e (Reading 1420 as neutral

volatile int nThrottleInB = NEUTRAL_THROTTLE_B;	// volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriodB = 0;	// set in the interrupt
volatile boolean bNewThrottleSignalB = false;	// set in the interrupt and read in the loop

// Wheel variables
int throttle;
int engineSpeed;
const float baseSteerPercent = 0.5;	// Go straight here
float steerPercent; 			// 1 Goes right, 0 left
const int maxForward = 1116; 		// Must be tweaked
const int maxBack = 1792;    		// Use control to test values
const int maxLeft = 1048 ;   		// and find values from the Serial Monitor
const int maxRight = 2000;

const int IN_1=5;  	// HBridge IN_1
const int IN_2=6;  	// HBridge IN_2
const int IN_3=7;  	// HBridge IN_3
const int IN_4=8;  	// HBridge IN_4
const int pinPWM_A=9; 	// HBridge EN_A
const int pinPWM_B=10;  // HBrigde EN_B

void setup() {
// tell the Arduino we want the function calcInput to be called whenever INT0 (digital pin 2)
// changes from HIGH to LOW or LOW to HIGH
// catching these changes will allow us to calculate how long the input pulse is
attachInterrupt(digitalPinToInterrupt(THROTTLE_SIGNAL_IN_A_PIN),calcInputA,CHANGE);

delay(10); // allows for Interrupts and I can afford to wait 100th of second to process interrupts

// tell the Arduino we want the function calcInput to be called whenever INT0 (digital pin 3)
// changes from HIGH to LOW or LOW to HIGH
// catching these changes will allow us to calculate how long the input pulse is
attachInterrupt(digitalPinToInterrupt(THROTTLE_SIGNAL_IN_B_PIN),calcInputB,CHANGE);

pinMode(IN_1,OUTPUT);
pinMode(IN_2,OUTPUT);
pinMode(IN_3,OUTPUT);
pinMode(IN_4,OUTPUT);
pinMode(pinPWM_A,OUTPUT);
pinMode(pinPWM_B,OUTPUT);
Serial.begin(9600);
}

void loop () {
  delay(20); // For testing
  //Handle new speed value
  if (bNewThrottleSignalA) {
    if (nThrottleInA > NEUTRAL_THROTTLE_A-100 && nThrottleInA < NEUTRAL_THROTTLE_A+100) {
      //NO FORWARD/BACKWARD
      throttle = NEUTRAL_THROTTLE_A;
    }
    else {
      throttle = nThrottleInA;
    }
    updateSpeeds();
    bNewThrottleSignalA = false;
  }
  //Handle new turn value
  if (bNewThrottleSignalB) {
    if (nThrottleInB > NEUTRAL_THROTTLE_B-10 && nThrottleInB < NEUTRAL_THROTTLE_B+10) {
      // NO LEFT OR RIGHT
      steerPercent = 0.5;
    }
    else {
      steerPercent = (float)(nThrottleInB - maxLeft)/(maxRight - maxLeft);
    }
    updateSpeeds();
    bNewThrottleSignalB = false;
  }
}
void updateSpeeds() {
  // Forward:
  if (throttle < NEUTRAL_THROTTLE_A) {
    digitalWrite(IN_1,HIGH);
    digitalWrite(IN_2,LOW);
    digitalWrite(IN_3,HIGH);
    digitalWrite(IN_4,LOW); 
    engineSpeed = 255*((NEUTRAL_THROTTLE_A-throttle)/(NEUTRAL_THROTTLE_A-maxForward));
  }
  // Backward
  else {
    digitalWrite(IN_1,LOW);
    digitalWrite(IN_2,HIGH);
    digitalWrite(IN_3,LOW);
    digitalWrite(IN_4,HIGH);
    engineSpeed = 255*((NEUTRAL_THROTTLE_A-throttle)/(NEUTRAL_THROTTLE_A-maxBack));
  }
  // Right
  if (steerPercent > 0.5) {
    analogWrite(pinPWM_A, engineSpeed*(1-steerPercent));
    analogWrite(pinPWM_B, engineSpeed);
  }
  // left
  else {
    analogWrite(pinPWM_A, engineSpeed);
    analogWrite(pinPWM_B, engineSpeed*steerPercent);
  }
}

void calcInputA() {
  if(digitalRead(THROTTLE_SIGNAL_IN_A_PIN) == HIGH){
    ulStartPeriodA = micros();
  }
  else {
    if(ulStartPeriodA && (bNewThrottleSignalA == false)){
      nThrottleInA = (int)(micros() - ulStartPeriodA);
      ulStartPeriodA = 0;
      bNewThrottleSignalA = true;
      Serial.print("A:");
      Serial.println(nThrottleInA);
    }
  }
}

void calcInputB() {
  if(digitalRead(THROTTLE_SIGNAL_IN_B_PIN) == HIGH){
    ulStartPeriodB = micros();
  }
  else {
    if(ulStartPeriodB && (bNewThrottleSignalB == false)){
      nThrottleInB = (int)(micros() - ulStartPeriodB);
      ulStartPeriodB = 0;
      bNewThrottleSignalB = true;
      Serial.print("B:");
      Serial.println(nThrottleInB);
      
    }
  }
}

