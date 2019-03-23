
/*Activates a transistor driving a high-voltage step-up converter
triggered by PWM-input signal from RC-reciever */

//#include <Servo.h>
#include "Hyper_Neon.h"

/*-----------------------------------------------------------------------------
Config
-----------------------------------------------------------------------------*/

// Serial debug
#define debug_baud 9600

// State variables
volatile int LED_state;

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
    pinMode(weapon_pin, OUTPUT);
    pinMode(heat_pin, OUTPUT);
    
    // Turn leds off
    analogWrite(t_pin_red, 255);
    analogWrite(t_pin_green, 255);
    analogWrite(t_pin_blue, 255);

    digitalWrite(weapon_pin, LOW);
    digitalWrite(heat_pin, LOW);

    // Debugging
    Serial.begin(debug_baud);
    Serial.println("Setup Complete");
}

void loop() {
    /* Command and control
     * Concept: Get new signals, do 1 cycle of operations
     */
    // Defer to controller
    get_inputs();
    do_cycle();
    //get_inputs();  
}

/* Collapse pwm signal to a set of discrete states */
void get_inputs() {
    /* Weapon control, should be non-blocking */
    if (bNewThrottleSignalA) {
        // Empty for now
        bNewThrottleSignalA = false;
    }
    /* LED control, should be blocking */
    if (bNewThrottleSignalB) {
        if (nThrottleInB > NEUTRAL_THROTTLE_B + DEADZONE_B) {
            // stick back
            LED_state = 0;
            //generic_gradient();
        } else if (nThrottleInB < NEUTRAL_THROTTLE_B - DEADZONE_B){ 
            // stick forward
            LED_state = 2;
        } else {
            // stick middle
            LED_state = 1;
        }
        //Serial.print("LED state: ");
        //Serial.println(LED_state);
        bNewThrottleSignalB = false;
    }
    
}

void do_cycle()  {
    /* LED control, should be blocking */
    switch(LED_state) {
        case 0:
            //Serial.println("B_MODE_0");
            seizure(5, 500, all_colors, 5); 
            break;
        case 1:
            //Serial.println("B_MODE_1");
            generic_gradient(); //TODO: Replace with 3rd mode
            break;
        case 2:
            //Serial.println("B_MODE_2");
            seizure(5, 100, all_colors, 5);
            break;
    }
}

/* Some LED modes of operation */

void generic_gradient() {
    if (do_gradient(red_max, green_max, 4000, 50) == -1) {
        return;
    }
    if (do_gradient(green_max, blue_max, 4000, 50) == -1) {
        return;
    }
    if (do_gradient(blue_max, red_max, 4000, 50) == -1) {
        return;  
    }
}

/*
void standby(RGB colors[], int colors_len) {
    RGB col = colors[random(0, colors_len)];
    
    if (do_gradient(off, col, 5000, 100) == -1) {
        return;
    }  
    
    if (do_gradient(col, off, 5000, 100) == -1) {
        return;
    }   
    
} */

/* Generic method for applying a color to LEDs */
void set_leds(RGB color) {
    analogWrite(t_pin_red, color.r);
    analogWrite(t_pin_green, color.g);
    analogWrite(t_pin_blue, color.b);
}

/* Basic LED transitions. If they loop with delays, they should get_inputs and break after change.

/* Do a gradient transition from color 1 to color 2*/
int do_gradient(RGB color_1, RGB color_2, int milli_secs, int step_millis) {
    // Save current states in case there is a change
    int LED_state_now = LED_state;
    
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
        get_inputs();
        if (LED_state_now != LED_state) {
            return -1; 
        }
        local_color.r += step_r;
        local_color.g += step_g;
        local_color.b += step_b;
        set_leds(local_color);
        delay(step_millis);
        step++;
    }

    // Compensate for arithmetic / rounding errors
    set_leds(color_2);
    return 0;
}

void seizure(int blinks, int period, RGB colors[], int colors_len) {
    // Save current states in case there is a change
    int LED_state_now = LED_state;
    for (int i = 0; i< blinks; i++) {
        get_inputs();
        if (LED_state_now != LED_state) {
            break; 
        }
        set_leds(off);
        digitalWrite(onboard_led_pin, LOW);
        delay(period);
        set_leds(colors[random(0, colors_len)]);
        digitalWrite(onboard_led_pin, HIGH);
        delay(period);
    }
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
            //Serial.print("A:");
            //Serial.println(nThrottleInA);
        }
    }
    if (nThrottleInA < NEUTRAL_THROTTLE_A - DEADZONE_A) {
        // stick forward
        digitalWrite(weapon_pin, HIGH);
        //digitalWrite(onboard_led_pin, LOW);
    } else if (nThrottleInA > NEUTRAL_THROTTLE_A + DEADZONE_A) {
        // stick back
        digitalWrite(heat_pin, LOW);
        //digitalWrite(onboard_led_pin, HIGH);
    } else {
        // stick middle
        digitalWrite(weapon_pin, LOW);
        digitalWrite(heat_pin, HIGH);
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
            //Serial.print("B:");
            //Serial.println(nThrottleInB);
        }
    }
}
