
/*Activates a transistor driving a high-voltage step-up converter
triggered by PWM-input signal from RC-reciever */

/*TODO: 
 * check values from PWM-input 
 * activate transistor when RC-switch is HIGH
 * deactivate when RC-switch is LOW
*/

const int transistorPin = 5;    // connected to the base of the transistor
const byte PWM_PIN = 3;         // pwm input-pin

int pwm_value;                  //value (time signal HIGH) in ms

void setup() {
   // set  the pins as output:
   pinMode(transistorPin, OUTPUT);
   pinMode(PWM_PIN, INPUT);
   Serial.begin(115200);
 }

 void loop() {
   
   //Test transistor by turning off and on
   /*
   digitalWrite(transistorPin, HIGH);
   delay(1000);
   digitalWrite(transistorPin, LOW);
   delay(1000);
   */
   
   pwm_value = pulseIn(PWM_PIN, HIGH);
   Serial.println(pwm_value);
   
   
 }
