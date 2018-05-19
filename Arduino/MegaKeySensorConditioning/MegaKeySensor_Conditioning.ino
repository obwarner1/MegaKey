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

// STANDARD ANALOGREAD SAMPLES AT 100KSAMPLES PER SECOND
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
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

void loop() {
  // put your main code here, to run repeatedly:
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
}
