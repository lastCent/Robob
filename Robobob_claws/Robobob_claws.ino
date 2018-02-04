
/*Activates a transistor driving a high-voltage step-up converter
triggered by PWM-input signal from RC-reciever */

const int transistorPin = 8;    // connected to the base of the transistor
const byte PWM_PIN = 3;         // pwm input-pin
const int pwm_threshold_top = 1600;

int pwm_value;                  //value (time signal HIGH) in ms

void setup() {
   // set  the pins as output:
   pinMode(transistorPin, OUTPUT);
   pinMode(PWM_PIN, INPUT);
   Serial.begin(115200);
 }

 void loop() {
   pwm_value = pulseIn(PWM_PIN, HIGH);
      Serial.println("on");             // Debug
   Serial.println(pwm_value);    // Debug
   // Prototype toggle transist2or activation
   if (pwm_value > pwm_threshold_top) {
      digitalWrite(transistorPin, HIGH);
   } else {
      digitalWrite(transistorPin, LOW);
   }
   delay(50);
 }
