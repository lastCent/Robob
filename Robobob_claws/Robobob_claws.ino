
/*Activates a transistor driving a high-voltage step-up converter
triggered by PWM-input signal from RC-reciever */

#include <Servo.h>

// TODO: Safety exit if servo read pins not connected
// TODO: Tweak servo position degrees
// TODO: Check safety delay
// TODO: Make claws start as open

/*-----------------------------------------------------------------------------
Config
-----------------------------------------------------------------------------*/

const int transistor_pin = 12;    	// Shock weapon transistor gate
const int shock_PWM_pin = 9;         	// Shock PWM input-pin
const int servo_PWM_pin = 10; 		// Servo PWM input-pin
const int servo_ctrl_pin_l = 5;		// Left servo control
const int servo_ctrl_pin_r = 3;		// Right servo control
const int servo_l_open = 80;
const int servo_l_closed = 5;
const int servo_r_open = 0;
const int servo_r_closed = 82;
const int toggle_threshold = 1500;	// PWM threshold of toggling signal
const int safety_delay = 1000;		// Delay before shock, in millis
//-----------------------------------------------------------------------------

int last_toggle;	// Last time the claws were toggled
bool claws_closed;	// Are the claws ready to deliver shock? 
Servo servo_l;		// Left servo
Servo servo_r;		// Right servo

void setup() {
    // set  the pins as output:
    pinMode(transistor_pin, OUTPUT);
    pinMode(shock_PWM_pin, INPUT);
    pinMode(servo_PWM_pin, INPUT);
    servo_l.attach(servo_ctrl_pin_l);
    servo_r.attach(servo_ctrl_pin_r);
    // Dab sequence
    do_dab();
    Serial.begin(9600);
    Serial.println("Setup Complete");
}

void loop() {
    do_servos();        // Grab with servos
    do_dab();               // Taunt the enemy
    //do_shock();       // Donate energy
    delay(30);      //TODO: Adjust?
}

// Test that claws are closed before shock
bool safety_check() {
    int current_time = millis();
    if (claws_closed && (current_time - last_toggle) > safety_delay) {
        Serial.println("Safety check passed");      //DEBUG
        return true;
    } else {
        Serial.println("Safety check failed");      //DEBUG
        return false;
    }
}

// Open/close claws
// Returns 1 if closed, -1 if opened, and 0 if no action taken
// Commented lines need to be reactivated for safety functionality
int do_servos() {
    int servo_PWM = pulseIn(servo_PWM_pin, HIGH);
    //if (!claws_closed && servo_PWM > toggle_threshold) {
    if (servo_PWM > toggle_threshold) {
        servo_l.write(servo_l_closed);
        servo_r.write(servo_r_closed);   
        //last_toggle = millis();
        claws_closed = true;   
        //Serial.println("Claws closed");
        return 1;
    //} else if (claws_closed && servo_PWM < toggle_threshold) {
    } else if (servo_PWM < toggle_threshold) {
        servo_l.write(servo_l_open);
        servo_r.write(servo_r_open);
        //last_toggle = millis();
        claws_closed = false;
        //Serial.println("Claws opened");
        return -1;
    } else {
        return 0;
    }
}

// Activate shock if safety measures met
bool do_shock() {
    int shock_PWM = pulseIn(shock_PWM_pin, HIGH);
    if (!safety_check()) {
        digitalWrite(transistor_pin, LOW);
        return false;
    }
    if (shock_PWM > toggle_threshold) {
        digitalWrite(transistor_pin, HIGH);
        Serial.println("Shock delivered");
        return true;
    } else {
        digitalWrite(transistor_pin, LOW);
        Serial.println("Shock disabled");
        return false;
    }
}

// Unholyness
bool do_dab() {
    int shock_PWM = pulseIn(shock_PWM_pin, HIGH);
    if (shock_PWM > toggle_threshold) {
        servo_l.write(servo_l_open); 
        servo_r.write(servo_r_closed);
        delay(750);
        servo_l.write(servo_l_closed);
        servo_r.write(servo_r_open);
        delay(750);
        servo_l.write(servo_l_open);
        servo_r.write(servo_r_open);  
        return true;
    }
    return false;
}

