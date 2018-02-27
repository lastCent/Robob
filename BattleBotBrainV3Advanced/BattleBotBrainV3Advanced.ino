// Advanced battle bot wheel control.
// Debug serial output left on, since some tweaking may be needed for redeployment 

// TODO: SEPARATE SHIT, MAKE CONFIG SECTION
// UNFUCK VARIABLE NAMES
// 4 space indent

// Forward-Backward steering
#define THROTTLE_SIGNAL_IN_A 0		// INTERRUPT 0 = DIGITAL PIN 2 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_A_PIN 2	// INTERRUPT 0 = DIGITAL PIN 11 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE_A 1488		// this is the duration in microseconds of neutral throttle on an electric RC Car

volatile int nThrottleInA = NEUTRAL_THROTTLE_A;	// volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriodA = 0;	// set in the interrupt
volatile boolean bNewThrottleSignalA = false;	// set in the interrupt and read in the loop

// Left-Right steering
#define THROTTLE_SIGNAL_IN_B 1		// INTERRUPT 1 = DIGITAL PIN 3 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_B_PIN 3 	// INTERRUPT 1 = DIGITAL PIN 3 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE_B 1496 	// This value may require changing the DX4e (Reading 1420 as neutral

volatile int nThrottleInB = NEUTRAL_THROTTLE_B;	// volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriodB = 0;	// set in the interrupt
volatile boolean bNewThrottleSignalB = false;	// set in the interrupt and read in the loop

// Flags
bool forward, backward, left, right; 
// Wheel variables
float throttlePercent;
float steerPercent;                     // How much left or right robot should go
const int maxForward = 1116;            // Must be tweaked
const int maxBack = 1792;               // Use control to test values
const int maxLeft = 1048 ;              // and find values from the Serial Monitor
const int maxRight = 2000;

const int IN_1=5;  	// HBridge IN_1
const int IN_2=6;  	// HBridge IN_2
const int IN_3=7;  	// HBridge IN_3
const int IN_4=8;  	// HBridge IN_4
const int pinPWM_A=9; 	// HBridge EN_A  (venstre?)
const int pinPWM_B=10;  // HBrigde EN_B (høyre?)

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
    forward, backward, left, right = false;
}

void loop () {
    delay(50); // For testing
    //Handle new speed value
    handleThrottle();
    //Handle new turn value
    handleTurn();
    // Write new values to engines
    updateSpeeds();
}

void handleThrottle() {
    if (bNewThrottleSignalA) {
        if (nThrottleInA > NEUTRAL_THROTTLE_A-100 && nThrottleInA < NEUTRAL_THROTTLE_A+100) {
            //NO FORWARD/BACKWARD
            throttlePercent = 0;
            forward, backward = false;
        }
        else if (nThrottleInA < NEUTRAL_THROTTLE_A){
            // Going Forward
            throttlePercent = ((NEUTRAL_THROTTLE_A-nThrottleInA)/(NEUTRAL_THROTTLE_A-maxForward));
            forward = true; 
            backward = false;
        } else {
            // Going Backward
            throttlePercent = ((NEUTRAL_THROTTLE_A-nThrottleInA)/(NEUTRAL_THROTTLE_A-maxBack));
            backward = true;
            forward = false;
        }
    bNewThrottleSignalA = false;
    }    
}

void handleTurn() {
  if (bNewThrottleSignalB) {
    if (nThrottleInB > NEUTRAL_THROTTLE_B-100 && nThrottleInB < NEUTRAL_THROTTLE_B+100) {
      // NO LEFT OR RIGHT
      steerPercent = 0;
      left, right = false;
    } else if (nThrottleInB < NEUTRAL_THROTTLE_B) {
      // Going left 
      steerPercent = (float)(NEUTRAL_THROTTLE_B-nThrottleInB)/(NEUTRAL_THROTTLE_B-maxLeft);
      left = true;
      right = false;
    } else {
      //Going Right 
      steerPercent =(float)(NEUTRAL_THROTTLE_B-nThrottleInB)/(NEUTRAL_THROTTLE_B-maxRight) ; 
      right = true;
      left = false; 
    }
    bNewThrottleSignalB = false;
  }    
}
    
void updateSpeeds() {
  // Stand in place (and turn)
  if (!(forward || backward)) {
    // Do nothing
    if (!(left || right)) {
      setHBridge(LOW, LOW, LOW, LOW, 0, 0);
    }
    // Turn right
    else if (right) {
      setHBridge(LOW, LOW, LOW, LOW, 255*steerPercent, 255*steerPercent);
    }
    // Turn left
    else if (left){
      setHBridge(HIGH, HIGH, HIGH, HIGH,255*steerPercent, 255*steerPercent);  
    }
  }
  // Move forward
  else if (forward) {
    // Straight
    if (!(left || right)) {
        setHBridge(HIGH, LOW, HIGH, LOW, 255*throttlePercent,255*throttlePercent);
    }
    // Right
    else if (right) {
      setHBridge(HIGH, LOW, HIGH, LOW, 255*throttlePercent, 255*throttlePercent*(1-steerPercent));
    } 
    // Left
    else if (left) {
      setHBridge(HIGH, LOW, HIGH, LOW, 255*throttlePercent*(1-steerPercent), 255*throttlePercent);  
    }
  }
  // Move backward
  else {
    // Straight
    if (!(left || right)) {
        setHBridge(LOW, HIGH, LOW, HIGH, 255*throttlePercent, 255*throttlePercent);
    }
    // Right
    else if (right) {
        setHBridge(LOW, HIGH, LOW, HIGH, 255*throttlePercent, 255*throttlePercent*(1-steerPercent));
    }
    // Left
    else if (left) {
        setHBridge(LOW, HIGH, LOW, HIGH, 255*throttlePercent*(1-steerPercent), 255*throttlePercent);  
    }
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

void setHBridge(uint8_t one, uint8_t two, uint8_t three, uint8_t four, uint8_t A, uint8_t B) {
    digitalWrite(IN_1, one);
    digitalWrite(IN_2, two);
    digitalWrite(IN_3, three);
    digitalWrite(IN_4, four);
    analogWrite(pinPWM_A, A);
    analogWrite(pinPWM_B, B);
}


