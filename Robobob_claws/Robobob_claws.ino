
/*Activates a transistor driving a high-voltage step-up converter
triggered by PWM-input signal from RC-reciever */

#include <Servo.h>

// TODO: Safety exit if servo read pins not connected
// TODO: Tweak servo position degrees
// TODO: Check safety delay
// TODO: Negative angle writing untested, replace with (180-x)?
// TODO: Make claws start as open

/*-----------------------------------------------------------------------------
Config
-----------------------------------------------------------------------------*/

const int transistor_pin = 12;    	// Shock weapon transistor gate
const int shock_PWM_pin = 9;         	// Shock PWM input-pin
const int servo_PWM_pin = 10; 		// Servo PWM input-pin
const int servo_ctrl_pin_l = 3;		// Left servo control
const int servo_ctrl_pin_r = 5;		// Right servo control
const int servo_start_deg = 0;		// Claws neutral position
const int servo_close_deg = 90;		// Claws closed position
const int toggle_threshold = 1500;	// PWM threshold of toggling signal
const int safety_delay = 1000;		// Delay before shock, in micros
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
    Serial.begin(115200);
    Serial.println("Setup Complete");       //DEBUG
}

void loop() {
    do_servos();
    do_shock();
    delay(50);      //TODO: Adjust?
}

// Test that claws are closed before shock
bool safety_check() {
    int current_time = micros();
    if (claws_closed && current_time - last_toggle > safety_delay) {
        Serial.println("Safety check passed");      //DEBUG
        return true;
    } else {
        Serial.println("Safety check failed");      //DEBUG
        return false;
    }
}

// Open/close claws
// Returns 1 if closed, -1 if opened, and 0 if no action taken
int do_servos() {
    int servo_PWM = pulseIn(servo_PWM_pin, HIGH);
    //Serial.println(servo_PWM);             //DEBUG
    if (!claws_closed && servo_PWM > toggle_threshold) {
        servo_l.write(servo_close_deg);
	      servo_r.write(servo_start_deg);
        last_toggle = micros();
        claws_closed = true;   
        Serial.println("Claws closed");         //DEBUG
        return 1;
    } else if (claws_closed && servo_PWM < toggle_threshold) {
        servo_l.write(servo_start_deg);
        servo_r.write(servo_close_deg);
        last_toggle = micros();
        claws_closed = false;
        Serial.println("Claws opened");         //DEBUG
        return -1;
    } else {
        return 0;
    }
}

// Activate shock if safety measures met
bool do_shock() {
    int shock_PWM = pulseIn(shock_PWM_pin, HIGH);
    //Serial.println(shock_PWM);
    if (!safety_check()) {
        digitalWrite(transistor_pin, LOW);
        return false;
    }
    if (shock_PWM > toggle_threshold) {
        digitalWrite(transistor_pin, HIGH);
        Serial.println("Shock delivered");          //DEBUG
        return true;
    } else {
        digitalWrite(transistor_pin, LOW);
        Serial.println("Shock disabled");           //DEBUG
        return false;
    }
}
