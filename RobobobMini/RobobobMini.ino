/*Activates a relay driving a high-voltage step-up converter
triggered by PWM-input signal from RC-reciever */

/*-----------------------------------------------------------------------------
Config
-----------------------------------------------------------------------------*/

#define RELAY_PIN 2
#define SHOCK_PWM 3
#define SHOCK_THRESHOLD 1500

/*----------------------------------------------------------------------------*/

void setup()
{
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SHOCK_PWM, INPUT);
  digitalWrite(RELAY_PIN, HIGH); //Set shock OFF
  Serial.begin(115200);
  Serial.println("Setup Complete");
}

void loop()
{
   int shock_signal = pulseIn(SHOCK_PWM, HIGH);
   Serial.println(shock_signal);
   if (shock_signal < SHOCK_THRESHOLD)
   {
     digitalWrite(RELAY_PIN, HIGH);
     Serial.println("Shock disabled"); 
   }
   else
   {
     digitalWrite(RELAY_PIN, LOW);
     Serial.println("Shock delivered");
   }
   delay(50);  
}
