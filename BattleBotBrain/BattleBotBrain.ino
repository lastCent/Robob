// Channel 1
#define THROTTLE_SIGNAL_IN 0 // INTERRUPT 0 = DIGITAL PIN 2 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_PIN 2 // INTERRUPT 0 = DIGITAL PIN 2 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLE 1500 // this is the duration in microseconds of neutral throttle on an electric RC Car

// Channel 1
volatile int nThrottleIn = NEUTRAL_THROTTLE; // volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriod = 0; // set in the interrupt
volatile boolean bNewThrottleSignal = false; // set in the interrupt and read in the loop
// we could use nThrottleIn = 0 in loop instead of a separate variable, but using bNewThrottleSignal to indicate we have a new signal
// is clearer for this first example

// Channel 2
#define THROTTLE_SIGNAL_INB 1 // INTERRUPT 1 = DIGITAL PIN 3 - use the interrupt number in attachInterrupt
#define THROTTLE_SIGNAL_IN_PINB 3 // INTERRUPT 1 = DIGITAL PIN 3 - use the PIN number in digitalRead
#define NEUTRAL_THROTTLEB 1496 // This value may require changing the DX4e (Reading 1420 as nutral

// Channel 2
volatile int nThrottleInB = NEUTRAL_THROTTLEB; // volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile unsigned long ulStartPeriodB = 0; // set in the interrupt
volatile boolean bNewThrottleSignalB = false; // set in the interrupt and read in the loop

// Wheel variables
int throttle;
float steerPercent; // 1 Goes right, 0 left
bool retningV; //1 er fram, 0 bak
bool retningH; //1 er fram, 0 bak
const float baseSteerPercent = 0.5;
const int maxForward = 1912; //Must be tweaked
const int maxBack = 1084;    //Use control to test values
const int maxLeft = 1072 ;   //and find values from the Serial Monitor
const int maxRight = 1920;

const int pin5_v=5;
const int pin6_v=6;
const int pin7_h=7;
const int pin8_h=8;
const int pinPWM_h=9;
const int pinPWM_v=10;

void setup() {
// tell the Arduino we want the function calcInput to be called whenever INT0 (digital pin 2)
// changes from HIGH to LOW or LOW to HIGH
// catching these changes will allow us to calculate how long the input pulse is
attachInterrupt(digitalPinToInterrupt(THROTTLE_SIGNAL_IN_PIN),calcInput,CHANGE);

delay(10); // allows for Interrupts and I can afford to wait 100th of second to process interrupts

// tell the Arduino we want the function calcInput to be called whenever INT0 (digital pin 3)
// changes from HIGH to LOW or LOW to HIGH
// catching these changes will allow us to calculate how long the input pulse is
attachInterrupt(digitalPinToInterrupt(THROTTLE_SIGNAL_IN_PINB),calcInputB,CHANGE);
pinMode(pin5_v,OUTPUT);
pinMode(pin6_v,OUTPUT);
pinMode(pin7_h,OUTPUT);
pinMode(pin8_h,OUTPUT);
pinMode(pinPWM_h,OUTPUT);
pinMode(pinPWM_v,OUTPUT);
Serial.begin(9600);
}

void loop () {
  delay(10); // For testing
  //Handle new speed value
  if (bNewThrottleSignal) {
    if (nThrottleIn > NEUTRAL_THROTTLE-50 && nThrottleIn < NEUTRAL_THROTTLE+50) {
      //NO FORWARD/BACKWARD
      throttle = NEUTRAL_THROTTLE;
    }
    else {
      throttle = nThrottleIn;
    }
    updateSpeeds();
    bNewThrottleSignal = false;
  }
  //Handle new turn value
  if (bNewThrottleSignalB) {
    if (nThrottleInB > NEUTRAL_THROTTLEB-10 && nThrottleInB < NEUTRAL_THROTTLEB+10) {
      // NO LEFT OR RIGHT
      steerPercent = 0.5;
    }
    else {
      steerPercent = (float)(nThrottleInB - maxLeft)/(maxRight - maxLeft);
    }
    updateSpeeds();
    bNewThrottleSignalB = false;
  }
}
void updateSpeeds() {
  if (throttle >= NEUTRAL_THROTTLE) {
    retningV=1;
    retningH=1;
  }
  else{
    retningV=0;
    retningH=0;
  }
    
    float right = 255*(((float)(throttle-NEUTRAL_THROTTLE))/((float)(maxForward-NEUTRAL_THROTTLE)));
    float left= 255*(((float)(throttle-NEUTRAL_THROTTLE))/((float)(maxForward-NEUTRAL_THROTTLE)));
    if(left<0){
      left=-left;
    }
    if(left>255){
      left=255;
    }
    if(right<0){
      right=-right;
    }
    if(right>255){
      right=255;
    }
    if(right>90){
      
      if(steerPercent>0.5){ //Det under her er til å svinge.
        right+=steerPercent*100;
      }
      else if(steerPercent<0.5){
        left+=(1-steerPercent)*100;
      }
      if(right>255){
        right-=steerPercent*100;
        left-=steerPercent*100;
      }
      if(left>255){
        left-=(1-steerPercent)*100;
        right-=(1-steerPercent)*100;
      }
      if(1){
        float temp=left;
        left=right;
        right=temp;
      }
    }
    else{
      if(steerPercent>0.7){
        retningH^=1;
      }
      else if(steerPercent<0.3){
        retningV^=1;
      }
      
    }
    
    
    //Write to motor pins + HBridge
    analogWrite(9,left); //pin 9 er farten til VENSTRE side
    analogWrite(10,right); // pin 10 er farten til HØYRE side
    if(retningV==1){
      digitalWrite(5,HIGH);  //pin 5 framover for VENTSTRE side
      digitalWrite(6,LOW);    //pin 6 bakover for VENTSRE side
    }
    else{
      digitalWrite(5,LOW);
      digitalWrite(6,HIGH);
    }
    if(retningH==1){
      digitalWrite(7, HIGH); // pin 7 er framover for HØYRE side
      digitalWrite(8,LOW);   // pin 8 er bakover for HØYRE side
    }
    else{
         digitalWrite(7,LOW);
      digitalWrite(8,HIGH);
    }

    
    Serial.println(left);
    Serial.println(right);
    Serial.println(throttle);
    Serial.println(nThrottleInB);
    Serial.println(steerPercent);
    Serial.println(retningV);
    Serial.println(retningH);
    Serial.println();
  


}
void calcInput() {
  if(digitalRead(THROTTLE_SIGNAL_IN_PIN) == HIGH){
    ulStartPeriod = micros();
  }
  else {
    if(ulStartPeriod && (bNewThrottleSignal == false)){
      nThrottleIn = (int)(micros() - ulStartPeriod);
      ulStartPeriod = 0;
      bNewThrottleSignal = true;
    }
  }
}

void calcInputB() {
  if(digitalRead(THROTTLE_SIGNAL_IN_PINB) == HIGH){
    ulStartPeriodB = micros();
  }
  else {
    if(ulStartPeriodB && (bNewThrottleSignalB == false)){
      nThrottleInB = (int)(micros() - ulStartPeriodB);
      ulStartPeriodB = 0;
      bNewThrottleSignalB = true;
    }
  }
}

