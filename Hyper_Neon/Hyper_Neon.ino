
/*Activates a transistor driving a high-voltage step-up converter
triggered by PWM-input signal from RC-reciever */

//#include <Servo.h>
#include "Hyper_Neon.h"

/*-----------------------------------------------------------------------------
Config
-----------------------------------------------------------------------------*/

// Serial debug
#define debug_baud 9600

#define weapon_pin 6
#define t_pin_red 9
#define t_pin_green 10
#define t_pin_blue 11
#define onboard_led_pin 13 // For debugging purposes

#define WEAPON_SIGNAL_IN_A 1       // INTERRUPT 0 = DIGITAL PIN 2 - use the interrupt number in attachInterrupt
#define WEAPON_SIGNAL_IN_A_PIN 2   // INTERRUPT 0 = DIGITAL PIN 11 - use the PIN number in digitalRead#define NEUTRAL_THROTTLE_A 1460      // this is the duration in microseconds of neutral throttle on an electric RC Car
#define NEUTRAL_THROTTLE_A 1460      // this is the duration in microseconds of neutral throttle on an electric RC Car
#define maxForward 1144
#define maxBack 1820 
#define DEADZONE_A 200

#define LED_SIGNAL_IN_B 0          // INTERRUPT 1 = DIGITAL PIN 3 - use the interrupt number in attachInterrupt
#define LED_SIGNAL_IN_B_PIN 3      // INTERRUPT 1 = DIGITAL PIN 3 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE_B 1212             // This value may require changing the DX4e (Reading 1420 as neutral
#define maxLeft 868
#define maxRight 1760
#define DEADZONE_B 200

// Forward-Backward steering
volatile int nThrottleInA = NEUTRAL_THROTTLE_A;    // volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriodA = 0;  // set in the interrupt
volatile boolean bNewThrottleSignalA = false;   // set in the interrupt and read in the loop

// Left-Right steering
volatile int nThrottleInB = NEUTRAL_THROTTLE_B;    // volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriodB = 0;  // set in the interrupt
volatile boolean bNewThrottleSignalB = false;   // set in the interrupt and read in the loop

// State variables
int weapon_state;
int LED_state;

//-----------------------------------------------------------------------------

void setup() {
    // Enable interupts
    attachInterrupt(digitalPinToInterrupt(WEAPON_SIGNAL_IN_A_PIN),calcInputA,CHANGE);
    delay(10); 
    attachInterrupt(digitalPinToInterrupt(LED_SIGNAL_IN_B_PIN),calcInputB,CHANGE);

    // Set pins as input
    pinMode(WEAPON_SIGNAL_IN_A_PIN, INPUT);
    pinMode(LED_SIGNAL_IN_B_PIN, INPUT);
    
    // set  the pins as output:
    pinMode(t_pin_red, OUTPUT);
    pinMode(t_pin_green, OUTPUT);
    pinMode(t_pin_blue, OUTPUT);
    pinMode(onboard_led_pin, OUTPUT);

    // Turn leds off
    analogWrite(t_pin_red, 255);
    analogWrite(t_pin_green, 255);
    analogWrite(t_pin_blue, 255);

    // Debugging
    Serial.begin(debug_baud);
    Serial.println("Setup Complete");
}

void loop() {
    /* Command and control
     * Concept: Get new signals, do 1 cycle of operations
     */

    // Check interrupt flag
    get_inputs();
    // Defer to controller
    do_cycle();  
}

/* Collaps pwm signal to a set of discrete states */
void get_inputs() {
    /* Weapon control, should be non-blocking */
    if (bNewThrottleSignalA) {
        // Mode 1
        if (nThrottleInA < NEUTRAL_THROTTLE_A - DEADZONE_A) {
            weapon_state = 0;
        // Mode 3
        } else if (nThrottleInA > NEUTRAL_THROTTLE_A + DEADZONE_A) {
            weapon_state = 2;
        // Mode 2
        } else {
            weapon_state = 1;
        }
        bNewThrottleSignalA = false;
    }
    /* LED control, should be blocking */
    if (bNewThrottleSignalB) {
        
        if (nThrottleInB > NEUTRAL_THROTTLE_B + DEADZONE_B) {
            LED_state = 0;
            generic_gradient();
        } else if (nThrottleInB < NEUTRAL_THROTTLE_B - DEADZONE_B){ 
            LED_state = 2;
        } else {
            LED_state = 1;
        }
        bNewThrottleSignalB = false;
    }
}

void do_cycle()  {
    /* Weapon control, should be non-blocking */
    switch(weapon_state) {
        case 0:
            // Gradient activate 
            byte out = map(nThrottleInA, NEUTRAL_THROTTLE_A - DEADZONE_A , maxForward, 255, 0);
            analogWrite(weapon_pin, out); // TODO: TWEAK
            Serial.print("Weapon gradient: ");
            Serial.println(out);
            break;
        case 1:
            // Alt lavt 
            analogWrite(weapon_pin, 255);
            Serial.println("Weapon off");
            break;
        case 2:
            // pin høy
            analogWrite(weapon_pin, 0);
            Serial.println("Weapon on");
            break;
    }
    /* LED control, should be blocking */
    switch(LED_state) {
        case 0:
            Serial.println("B_MODE_0");
            generic_gradient(); //TODO: Replace with 3rd mode
            //seizure(10, 100, all_colors, 5);
            break;
        case 1:
            Serial.println("B_MODE_1");
            generic_gradient(); 
            break;
        case 2:
            Serial.println("B_MODE_2");
            seizure(10, 500, all_colors, 5); 
            break;
    }
}

/* Some LED modes of operation */

void generic_gradient() {
    do_gradient(red_max, green_max, 4000, 50);
    do_gradient(green_max, blue_max, 4000, 50);
    do_gradient(blue_max, red_max, 4000, 50);  
}

void seizure(int blinks, int period, RGB colors[], int len) {
    for (int i = 0; i< blinks; i++) {
        set_leds(off);
        digitalWrite(onboard_led_pin, LOW);
        delay(period);
        set_leds(colors[random(0, len)]);
        digitalWrite(onboard_led_pin, HIGH);
        delay(period);
    }
}

/* Generic method for applying a color to LEDs */
void set_leds(RGB color) {
    analogWrite(t_pin_red, color.r);
    analogWrite(t_pin_green, color.g);
    analogWrite(t_pin_blue, color.b);
}

/* Do a gradient transition from color 1 to color 2*/
void do_gradient(RGB color_1, RGB color_2, int milli_secs, int step_millis) {
    // Go from color 1 to color 2 in a number of milli secs
    // Let me invite you to a game of "The floor is floating point values"
    RGB local_color = {color_1.r, color_1.g, color_1.b};
    set_leds(local_color);

    int step = 0;
    int total_steps = milli_secs / step_millis; 
        
    // Calculate fixed point values
    int delta_r = color_2.r - color_1.r;   // Color channel change
    int delta_g = color_2.g - color_1.g;
    int delta_b = color_2.b - color_2.b;

    float step_r = delta_r / total_steps;
    float step_g = delta_g / total_steps;
    float step_b = delta_b / total_steps;

    while(step < total_steps) {
        local_color.r += step_r;
        local_color.g += step_g;
        local_color.b += step_b;
        set_leds(local_color);
        delay(step_millis);
        step++;
    }

    // Compensate for arithmetic / rounding errors
    set_leds(color_2);
}

void do_blink(RGB color_1, RGB color_2, int milli_duration) {
    set_leds(color_2);
    delay(milli_duration);
    set_leds(color_1);
}

/* Calculate pwm input signals */

void calcInputA() {
    if(digitalRead(WEAPON_SIGNAL_IN_A_PIN) == HIGH){
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
    if(digitalRead(LED_SIGNAL_IN_B_PIN) == HIGH){
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
