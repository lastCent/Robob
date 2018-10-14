/*-----------------------------------------------------------------------------
Description
-----------------------------------------------------------------------------*/
/*
 * Code for the sattelite bot. 
 * It drives independently from the mother bot, 
 * and is designed to apply all the volts to the enemy
 */

/* Steering: PWM-based variable speed. Can turn both while moving and in-place.*/

/* Weapon: Activates a relay driving a high-voltage step-up converter
triggered by PWM-input signal from RC-reciever */

/*
 *  Hardware requirements:
 *  Arduino Pro Micro  (change pins for other layouts)
 *      - Must have 3+ interrupt pins
 */
 
/*TODO list: 
  * Hvis våpen aktiv i mer enn x sekunder uten input endrin, skru av
  * Gøre våpen interrupt basert
*/
/*-----------------------------------------------------------------------------
Config
-----------------------------------------------------------------------------*/


// Serial debug
#define debug_baud 115200

// Shock weaponry
#define RELAY_PIN 4
#define SHOCK_PWM 5
#define SHOCK_THRESHOLD 1500

// Forward-Backward steering
#define THROTTLE_SIGNAL_IN_A 1               // INTERRUPT 0 = DIGITAL PIN 2 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_A_PIN 2      // INTERRUPT 0 = DIGITAL PIN 11 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE_A 1488             // this is the duration in microseconds of neutral throttle on an electric RC Car
#define maxForward 1064
#define maxBack 1908 

// Left-Right steering
#define THROTTLE_SIGNAL_IN_B 0                // INTERRUPT 1 = DIGITAL PIN 3 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_B_PIN 3      // INTERRUPT 1 = DIGITAL PIN 3 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE_B 1496             // This value may require changing the DX4e (Reading 1420 as neutral
#define maxLeft 1068
#define maxRight 1920

// HBridge
#define pinPWM_A 9
#define pinPWM_B 10
#define IN_1 18                     // HBridge IN_1
#define IN_2 19                     // HBridge IN_2
#define IN_3 20                     // HBridge IN_3
#define IN_4 21                     // HBridge IN_4

/*-----------------------------------------------------------------------------
Definitions
-----------------------------------------------------------------------------*/

// Forward-Backward steering
volatile int nThrottleInA = NEUTRAL_THROTTLE_A;    // volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriodA = 0;  // set in the interrupt
volatile boolean bNewThrottleSignalA = false;   // set in the interrupt and read in the loop

// Left-Right steering
volatile int nThrottleInB = NEUTRAL_THROTTLE_B;    // volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriodB = 0;  // set in the interrupt
volatile boolean bNewThrottleSignalB = false;   // set in the interrupt and read in the loop

// Flags
bool forward, backward, left, right; 

// Wheel variables
float throttlePercent;
float steerPercent;                     // How much left or right robot should go

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

    // Steering
    pinMode(IN_1,OUTPUT);
    pinMode(IN_2,OUTPUT);
    pinMode(IN_3,OUTPUT);
    pinMode(IN_4,OUTPUT);
    pinMode(pinPWM_A,OUTPUT);
    pinMode(pinPWM_B,OUTPUT);
    forward, backward, left, right = false;

    // Shock 
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(SHOCK_PWM, INPUT);
    digitalWrite(RELAY_PIN, HIGH); //Set shock OFF

    // Debug
    Serial.begin(debug_baud);
    Serial.println("Setup Complete");
}

void loop() {
    handleThrottle();   // Handle new speed value
    handleTurn();            // Handle new turn value
    updateSpeeds();       // Write new values to engines
    operateShock();       // Use the weapon

    delay(50);  
}

void operateShock() {
    int shock_signal = pulseIn(SHOCK_PWM, HIGH);
    Serial.println(shock_signal);
    if (shock_signal < SHOCK_THRESHOLD) {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("Shock disabled"); 
    }
    else {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("Shock delivered");
    }
}

void handleThrottle() {
    if (bNewThrottleSignalA) {
        // Check that no values are out of valid range
        throttlePercent = min(throttlePercent, 1.0);
        throttlePercent = max(throttlePercent, 0.0);
        if (nThrottleInA > NEUTRAL_THROTTLE_A-150 && nThrottleInA < NEUTRAL_THROTTLE_A+150) {
            //NO FORWARD/BACKWARD
            throttlePercent = 0;
            forward = false;
            backward = false;
        }
        else if (nThrottleInA < NEUTRAL_THROTTLE_A){
            // Going Forward
            throttlePercent = (float)(NEUTRAL_THROTTLE_A-nThrottleInA)/(NEUTRAL_THROTTLE_A-maxForward);
            forward = true; 
            backward = false;
        } else {
            // Going Backward
            throttlePercent = (float)(NEUTRAL_THROTTLE_A-nThrottleInA)/(NEUTRAL_THROTTLE_A-maxBack);
            backward = true;
            forward = false;
        }
    bNewThrottleSignalA = false;
    }    
}

void handleTurn() {
    if (bNewThrottleSignalB) {
            // Check that no values are out of valid range
            steerPercent = min(steerPercent, 1.0);
            steerPercent= max(steerPercent, 0.0);
        if (nThrottleInB > NEUTRAL_THROTTLE_B-100 && nThrottleInB < NEUTRAL_THROTTLE_B+100) {
            // NO LEFT OR RIGHT
            steerPercent = 0;
            left = false;
            right = false;
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
            setHBridge(HIGH, LOW, LOW, HIGH, 255*steerPercent, 255*steerPercent);
        }
        // Turn left
        else if (left){
            setHBridge(LOW, HIGH, HIGH, LOW,255*steerPercent, 255*steerPercent);  
        }
    }
    // Move forward
    else if (forward) {
        // Straight
        if (!(left || right)) {
            setHBridge(HIGH, LOW, HIGH, LOW, 255*throttlePercent,255*throttlePercent);
            Serial.println(255*throttlePercent);
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
            Serial.println(255*throttlePercent);
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
