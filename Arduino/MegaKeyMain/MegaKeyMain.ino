#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <StandardCplusplus.h>
#include <vector>
#include "StaticForceController.h"

//Initialising dependencies
int analogPin[12]={A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11};
int valFilt[12];
double diff = 0.0;
int interupt;
int interupt2;
double valSens[12];
double valFin = 0.0;
boolean toggle0 = 0;
boolean toggle1 = 0;
double minSens0 = 250.0; // minimum sensor value
double maxSens0 = 650.0; // maximum sensor value 
double velocity[12];
double accel[12];

const int solPullPin = 6;    // H-bridge leg 1 (pin 2, 1A)
const int solPushPin = 5;    // H-bridge leg 2 (pin 7, 2A)
const int enablePin = 7;    // H-bridge enable pin
const int buttonPin1 = 10;
const int buttonPin2 = 9;
const int buttonPin3 = 8;    
const int ledPin1 =  13; 
const int ledPin2 =  12; 
const int ledPin3 =  11;
const int pianoMode = 0;
const int harpsiMode = 1; 
const int organMode = 2; 
 
double output;
int pwmPin, outForce;
float position;
float radius = 0.150;
float inertia = 0.001497;
float force = 0.0;
float accel = 0.0;
float angAcc = 0.0;
float torque = 0.0;
uint8_t distance2 = 0;
uint8_t distance = 0;
uint8_t finalDistance = 0;
int accelMidi, accelMidiNew;
bool noteState, isOn, button1, button2, button3, buttonCheck1, buttonCheck2, buttonCheck3 ;
int noteON = 144;//144 = 10010000 in binary, note on command
int noteOFF = 128;//128 = 10000000 in binary, note off command
int midiNotes[12] = {60,61,62,63,64,65,66,67,68,69,70,71}
long start = 0, stop, time;


//Normalization Function
double normalize(int sensVal, double minVal, double maxVal){
  double diff = maxVal-minVal;
  double normalSens = (sensVal-minVal)/diff;
  if (normalSens < 0){
    normalSens = 0;
  }
  return normalSens;
}

//Rate of Vel Change Function
double rateOfVelChange(int inrupt, double Value){
  static double rateChange = 0.0;
  static double v1 = 0.0;
  static double v2 = 0.0;
  static int lastInterupt;
  if (lastInterupt != inrupt){
    Serial.print("Last Interupt ");
    Serial.println(lastInterupt);
    if (inrupt > lastInterupt){
      v1 = Value;
      Serial.print("Distance 1 ");
      Serial.println(v1);  
    }
    else{
      v2 = Value;
      Serial.print("Distance 2 ");
      Serial.println(v2); 
    }
    rateChange = abs((v2-v1)/0.0005);     
  }
  lastInterupt = inrupt;
  return rateChange;
}

//Rate of Accel Change Function
double rateOfAccelChange(int inrupt, double Value){
  static double rateAccelChange = 0.0;
  static double a1 = 0.0;
  static double a2 = 0.0;
  static int lastInterupt;
  if (lastInterupt != inrupt){
    if (inrupt > lastInterupt){
      a1 = Value; 
    }
    else{
      a2 = Value;
    }
    rateAccelChange = abs((a2-a1)/0.001);      
  }
  lastInterupt = inrupt;
  return rateAccelChange;
}

void calcInertialForce(double accel)
{
    angAcc = accel/radius;
    torque = inertia*angAcc;
    force = (torque/0.200);
    if (force < 0 ) {
      force = force*-1;
    }
    if(force > 5){
      force = 0;
    }
    force = 64*force; // scaling value for pwm
}


void setup() {
  
  pinMode(solPushPin, OUTPUT);
  pinMode(solPullPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  digitalWrite(enablePin, HIGH);
  Serial.begin(31250);
  Wire.begin();
  Wire.setClock(400000);
  pwm.begin();
  pwm.setPWMFreq(1600);  // This is the maximum PWM frequency
  isOn = true;
  button1 = LOW;
  button2 = LOW;
  button3 = LOW;
  buttonCheck1 = LOW;
  buttonCheck2 = LOW;
  buttonCheck3 = LOW;
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);

  analogReadResolution(12);

  cli();//stop interrupts

//set timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 124;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

  //set timer1 interrupt at 1kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set timer count for 1khz increments
  OCR1A = 1999;// = (16*10^6) / (1000*8) - 1
  //had to use 16 bit timer1 for this bc 1999>255, but could switch to timers 0 or 2 with larger prescaler
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
 
}

//Velocity Interupt
ISR(TIMER0_COMPA_vect){//timer0 interrupt 2kHz toggle 0
//generates pulse wave of frequency 2kHz/2 = 1kHz (takes two cycles for full wave- toggle high then toggle low)
  if (toggle0){
    toggle0 = 0;
  }
  else{
    toggle0 = 1;
  }
}

//Acceleration Interupt
ISR(TIMER1_COMPA_vect){//timer1 interrupt 1kHz toggle 1
//generates pulse wave of frequency 1kHz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
  if (toggle0){
    toggle1 = 0;
  }
  else{
    toggle1 = 1;
  }
}


//USER INTERFACE FEEL SWITCHING
void switchResponseGrandPiano(){
  for ( int pinNum = 0; pinNum < 12; pinNum++)
  {
    pwmnum = pinNum;
    calcInertialForce(accel[pinNum]);
    if (position == 1){
      pwmPin = solPullPin;
      processPianoForceDescend(valSens[pinNum]);
    }
    else if(position == 0){
      pwmPin = solPushPin;
      processPianoForceAscend(valSens[pinNum]);
    }
  }
}

void switchResponseHarps()
{
  for ( int pinNum = 0; pinNum < 12; pinNum++)
  {
    pwmnum = pinNum;
    calcInertialForce(accel[pinNum]);
    if (position == 1){
      pwmPin = solPullPin;
      processHarpsForceDescend(valSens[pinNum]);
    }
    else if(position == 0){
      pwmPin = solPushPin;
      processHarpsForceAscend(valSens[pinNum]);
    }
  }
}

void switchResponseOrgan()
{
  for ( int pinNum = 0; pinNum < 12; pinNum++)
  {
    pwmnum = pinNum;
    calcInertialForce(accel[pinNum]);
    if (position == 1){
      pwmPin = solPullPin;
      processOrganForceDescend(valSens[pinNum]);
    }
    else if(position == 0){
      pwmPin = solPushPin;
      processOrganForceAscend(valSens[pinNum]);
    }
  }
}

//PIANO FEEL PROCESSING DESCENDING
void processPianoForceDescend(double finalDistance){
    if (finalDistance < 0.1){
      output = 14;
      outForce = 0;
      pwm.setPWM(pwmnum, 0, outForce );

    }else if (finalDistance < 0.3){
      output = -0.05*finalDistance + 9.65;
      outForce = output*5 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }else if (finalDistance < 0.6){
      output = 0.13*finalDistance + 10.79;
      outForce = output*5 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }else if (finalDistance < 0.95){
      output = 0.6*finalDistance - 0.4;
      outForce = output*6.6 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }else { 
      output = 24;
      outForce = 120 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }

}

//PIANO FEEL PROCESSING ASCENDING
void processPianoForceAscend(double finalDistance){
    if (finalDistance < 0.1){
      output = 14;
      outForce = 54;
      pwm.setPWM(pwmnum, 0, outForce );
    }else if (finalDistance < 0.3){
      output = -0.05*finalDistance + 9.65;
      outForce = output*5 + force;
      pwm.setPWM(pwmnum, 0, outForce ); 
    }else if (finalDistance < 0.6){
      output = 0.13*finalDistance + 10.79;
      outForce = output*5 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }else if (finalDistance < 0.95){
      output = 0.6*finalDistance - 0.4;
      outForce = output*6.6 + force;
      pwm.setPWM(pwmnum, 0, outForce ); 
    }else { 
      output = 24;
      outForce = 160 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }
}

//HARPSICHORD FEEL PROCESSING DESCENDING
void processHarpsForceDescend(double finalDistance){

    if (finalDistance < 0.1){
      output = 14;
      outForce = 0;
      pwm.setPWM(pwmnum, 0, outForce );
    }else if (finalDistance < 0.3){
      output = -0.05*finalDistance + 9.65;
      outForce = output*4 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }else if (finalDistance < 0.6){
      output = 0.13*finalDistance + 10.79;
      outForce = output*10 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }else if (finalDistance < 0.95){
      output = 0.6*finalDistance - 0.4;
      outForce = output*2 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }else { 
      output = 24;
      outForce = 0 + force;
      pwm.setPWM(pwmnum, 0, outForce ); 
    }

}

//HARPSICHORD FEEL PROCESSING ASCENDING
void processHarpsForceAscend(double finalDistance){
    if (finalDistance < 0.1){
      output = 14;
      outForce = 54;
      pwm.setPWM(pwmnum, 0, outForce );
    }else if (finalDistance < 0.3){
      output = -0.05*finalDistance + 9.65;
      outForce = output*5 + force;
      pwm.setPWM(pwmnum, 0, outForce ); 
    }else if (finalDistance < 0.6){
      output = 0.13*finalDistance + 10.79;
      outForce = output*8 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }else if (finalDistance < 0.95){
      output = 0.6*finalDistance - 0.4;
      outForce = output*6.6 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }else { 
      output = 24;
      outForce = 54 + force;
      pwm.setPWM(pwmnum, 0, outForce );
    }
}

//ORGAN FEEL PROCESSING DESCENDING
void processOrganForceDescend(double finalDistance){
    output = 0.1345*finalDistance + 4.132;
    outForce = output*6 + force;
    pwm.setPWM(pwmnum, 0, outForce );
}

//ORGAN FEEL PROCESSING ASCENDING
void processOrganForceAscend(double finalDistance){
    output = 0.1345*finalDistance + 0.4132;
    outForce = output*2 + force;
    pwm.setPWM(pwmnum, 0, outForce );
}


void sendMidiData(){
  for (int pinNum = 0; pinNum < 12; pinNum++)
  {
    if (valSens[pinNum] > 0.3)
    {
      noteState = true;
      if (noteState == true  && isOn == true)
      {
        Serial.write(noteON);
        Serial.write(MIDInote[pinNum]);
        Serial.write(velocity[pinNum]);    
      }
      isOn = false;
    }
    else
    {
      noteState = false;
      if (noteState == false && isOn == false)
      {
        Serial.write(noteOFF);
        Serial.write(MIDInote[pinNum]);
        Serial.write(0);
      }
      isOn = true;
    }
  }
}


//MEGAKEY MAIN CONTROL LOOP
void loop() {

  //SENSOR CONDITIONING
  int pinNum = 0;
  for (pinNum = 0; pinNum < 12, pinNum++){
    valFilt[pinNum] = analogRead(analogPin[pinNum]);     // read the input pin 
  }
  for (pinNum = 0; pinNum < 12, pinNum++){
    valSens[pinNum] = normalize(valFilt[pinNum], minSens0, maxSens0);

  //Velocity Calc    
    interupt = toggle0;
    velocity[pinNum] = rateOfVelChange(interupt, valSens[pinNum]);
 
  //Accel Calc  
    interupt2 = toggle1;
    accel[pinNum] = rateOfAccelChange(interupt2, velocity[pinNum]);    
  } 
  

  button1 = digitalRead(buttonPin1);
  button2 = digitalRead(buttonPin2);
  button3 = digitalRead(buttonPin3);

  //FEEL MODE SWITCH

  //PIANO MODE
  if (button1 == HIGH && button2 == LOW && button3 == LOW){
    buttonCheck1 = HIGH;
    noteON = 144;
    noteOFF = 128;
  }
  if(buttonCheck1 == HIGH){
    buttonCheck2 = LOW;
    buttonCheck3 = LOW;
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    adjustStepperMotorPosition(pianoMode);
    switchResponseGrandPiano();
    sendMidiData();
  }
  //HARPSICHORD MODE
  if (button2 == HIGH && button1 == LOW && button3 == LOW){
    buttonCheck2 = HIGH;
    noteON = 145;
    noteOFF = 129;
  }
  if (buttonCheck2 == HIGH){
    buttonCheck1 = LOW;
    buttonCheck3 = LOW;
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, LOW);
    adjustStepperMotorPosition(harpsiMode);
    switchResponseHarps();
    sendMidiData();
  }
  //ORGAN MODE
  if (button3 == HIGH && button1 == LOW && button2 == LOW){
    buttonCheck3 = HIGH;
    noteON = 146;
    noteOFF = 130;
  }
  if (buttonCheck3 == HIGH){
    buttonCheck1 = LOW;
    buttonCheck2 = LOW;
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, HIGH);
    adjustStepperMotorPosition(organMode);
    switchResponseOrgan();
    sendMidiData(); 
  }   

}





