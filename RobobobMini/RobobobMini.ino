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
  * Fix var names
*/

/*-----------------------------------------------------------------------------
Config
-----------------------------------------------------------------------------*/
#include <Servo.h>

// Serial debug
#define debug_baud 115200

// Shock weaponry
#define RELAY_PIN 4
#define SHOCK_PWM 5
#define SHOCK_THRESHOLD 1750

// Forward-Backward steering
#define THROTTLE_SIGNAL_IN_A 1               // INTERRUPT 0 = DIGITAL PIN 2 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_A_PIN 2      // INTERRUPT 0 = DIGITAL PIN 11 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE_A 1488             // this is the duration in microseconds of neutral throttle on an electric RC Car
#define maxForward 1144
#define maxBack 1820 
#define DEADZONE_A 150

// Left-Right steering
#define THROTTLE_SIGNAL_IN_B 0                // INTERRUPT 1 = DIGITAL PIN 3 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_B_PIN 3      // INTERRUPT 1 = DIGITAL PIN 3 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE_B 1300             // This value may require changing the DX4e (Reading 1420 as neutral
#define maxLeft 868
#define maxRight 1760
#define DEADZONE_B 100

// Servos
#define pinPWM_A 9
#define pinPWM_B 10
#define LEFT_FORWARD 0
#define LEFT_BACK 180
#define STOP 90
#define RIGHT_FORWARD 180
#define RIGHT_BACK 0
 
Servo left, right;

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
    //pinMode(pinPWM_A,OUTPUT);
    //pinMode(pinPWM_B,OUTPUT);
    left.attach(pinPWM_A);
    right.attach(pinPWM_B);

    // Shock 
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(SHOCK_PWM, INPUT);
    digitalWrite(RELAY_PIN, HIGH); //Set shock OFF

    // Debug
    Serial.begin(debug_baud);
    Serial.println("Setup Complete");
}

void loop() {
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

void updateSpeeds() {
    // Move forward
    if (nThrottleInA < NEUTRAL_THROTTLE_A - DEADZONE_A) {
        // Turn right
        if (nThrottleInB > NEUTRAL_THROTTLE_B + DEADZONE_B) {
            set_servos(
                map(nThrottleInA, NEUTRAL_THROTTLE_A, maxForward, STOP, LEFT_FORWARD),
                max(STOP, map(nThrottleInA, NEUTRAL_THROTTLE_A, maxForward, STOP, RIGHT_FORWARD) -
                map(nThrottleInB, NEUTRAL_THROTTLE_B, maxRight, RIGHT_BACK, STOP))
            );
                
        }
        // Turn left
        else if (nThrottleInB < NEUTRAL_THROTTLE_B - DEADZONE_B){ 
            set_servos( 
                min(STOP, map(nThrottleInA, NEUTRAL_THROTTLE_A, maxForward, STOP, LEFT_FORWARD) +
                map(nThrottleInB, NEUTRAL_THROTTLE_B, maxLeft, LEFT_FORWARD, STOP)), 
                map(nThrottleInA, NEUTRAL_THROTTLE_A, maxForward, STOP, RIGHT_FORWARD)
            ); 
        }
        // Move straight
        else {
            set_servos(
                map(nThrottleInA, NEUTRAL_THROTTLE_A, maxForward, STOP, LEFT_FORWARD),
                map(nThrottleInA, NEUTRAL_THROTTLE_A, maxForward, STOP, RIGHT_FORWARD)
            );
        }
    }
    // Move backward
    else if (nThrottleInA > NEUTRAL_THROTTLE_A + DEADZONE_A) {
        // Turn right
        if (nThrottleInB > NEUTRAL_THROTTLE_B + DEADZONE_B) {
            set_servos(
                map(nThrottleInA, NEUTRAL_THROTTLE_A, maxBack, STOP, LEFT_BACK),
                min(STOP, map(nThrottleInA, NEUTRAL_THROTTLE_A, maxBack, STOP, RIGHT_BACK) +
                map(nThrottleInB, NEUTRAL_THROTTLE_B, maxRight, RIGHT_BACK, STOP))
            );
        }
        // Turn left
        else if (nThrottleInB < NEUTRAL_THROTTLE_B - DEADZONE_B){ 
            set_servos(
                max(STOP, map(nThrottleInA, NEUTRAL_THROTTLE_A, maxForward, STOP, LEFT_BACK) -
                map(nThrottleInB, NEUTRAL_THROTTLE_B, maxLeft, LEFT_FORWARD, STOP)),
                map(nThrottleInA, NEUTRAL_THROTTLE_A, maxBack, STOP, RIGHT_BACK)
            );
        }
        // Move straight
        else {
            set_servos(
                map(nThrottleInA, NEUTRAL_THROTTLE_A, maxBack, STOP, LEFT_BACK),
                map(nThrottleInA, NEUTRAL_THROTTLE_A, maxBack, STOP, RIGHT_BACK)
            );
        }
    }
    // Stand in place (and turn)
    else {
        // Turn right
        if (nThrottleInB > NEUTRAL_THROTTLE_B + DEADZONE_B) {
            set_servos(
                map(nThrottleInB, NEUTRAL_THROTTLE_B, maxRight, STOP, LEFT_FORWARD),
                map(nThrottleInB, NEUTRAL_THROTTLE_B, maxRight, STOP, RIGHT_BACK)
            );
        }
        // Turn left
        else if (nThrottleInB < NEUTRAL_THROTTLE_B - DEADZONE_B){ 
            set_servos(
                map(nThrottleInB, NEUTRAL_THROTTLE_B, maxLeft, STOP, LEFT_BACK),
                map(nThrottleInB, NEUTRAL_THROTTLE_B, maxLeft, STOP, RIGHT_FORWARD)
            );
        }
        // Do nothing
        else {
            set_servos(STOP,STOP);
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

void set_servos(int leftVal, int rightVal) {
    left.write(leftVal);
    right.write(rightVal);
}
