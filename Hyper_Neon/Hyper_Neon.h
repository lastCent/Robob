/* Header file for Hyper_Neon 
 * ---------------------------------------------------------------------------------
 */

// Output pins
#define weapon_pin 6
#define t_pin_red 9
#define t_pin_green 10
#define t_pin_blue 11
#define onboard_led_pin 13 // For debugging purposes

// Weapon related values
#define WEAPON_SIGNAL_IN_A 1       // INTERRUPT 0 = DIGITAL PIN 2 - use the interrupt number in attachInterrupt
#define WEAPON_SIGNAL_IN_A_PIN 2   // INTERRUPT 0 = DIGITAL PIN 11 - use the PIN number in digitalRead#define NEUTRAL_THROTTLE_A 1460      // this is the duration in microseconds of neutral throttle on an electric RC Car
#define NEUTRAL_THROTTLE_A 1460      // this is the duration in microseconds of neutral throttle on an electric RC Car
#define maxForward 1144
#define maxBack 1820 
#define DEADZONE_A 200

// LED related values
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


// Colour struct
struct RGB { 
    float r;
    float g;
    float b;
};

/* Predefined colors
 * Remember that high->no color and low->much color
 */
 
RGB red_max = {0,255,255};
RGB green_max = {255, 0, 255};
RGB blue_max = {255, 255, 0};

RGB yellow_max = {0, 0, 255};
RGB purple_max = {0, 255, 0};
RGB teal_max = {255, 0, 0};

RGB white_max = {0, 0, 0};
RGB off = {255, 255, 255};

static RGB all_colors[] = {
    red_max,
    yellow_max,
    green_max,
    purple_max,
    blue_max,
    teal_max    
};
