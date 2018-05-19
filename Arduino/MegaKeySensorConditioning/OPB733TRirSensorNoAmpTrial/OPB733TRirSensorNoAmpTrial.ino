int analogPin = 0;
int analogPin1 = 1;
int valFilt = 0;
int valUnFilt = 0;

// STANDARD ANALOGREAD SAMPLES AT 100KSAMPLES PER SECOND
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  analogReadResolution(12);

}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.print(0);  // To freeze the lower limit
  Serial.print(" ");
  Serial.print(4095);  // To freeze the upper limit
  Serial.print(" ");
  valFilt = analogRead(analogPin);     // read the input pin
  Serial.print(valFilt);
  Serial.print(",");
  valUnFilt = analogRead(analogPin1);     // read the input pin
  Serial.println(valUnFilt);
}





// MAKE IT RAPID (666 KSAMPLES PER SECOND)
//int input = A0;
//int led = 13;
//int val;
//
//void setup()
//{
//  Serial.begin(115200);
//  pinMode(input,INPUT);
//  pinMode(led,OUTPUT);
//  // Setup all registers
//  pmc_enable_periph_clk(ID_ADC); // To use peripheral, we must enable clock distributon to it
//  adc_init(ADC, SystemCoreClock, ADC_FREQ_MAX, ADC_STARTUP_FAST); // initialize, set maximum posibble speed
//  adc_disable_interrupt(ADC, 0xFFFFFFFF);
//  adc_set_resolution(ADC, ADC_12_BITS);
//  adc_configure_power_save(ADC, 0, 0); // Disable sleep
//  adc_configure_timing(ADC, 0, ADC_SETTLING_TIME_3, 1); // Set timings - standard values
//  adc_set_bias_current(ADC, 1); // Bias current - maximum performance over current consumption
//  adc_stop_sequencer(ADC); // not using it
//  adc_disable_tag(ADC); // it has to do with sequencer, not using it
//  adc_disable_ts(ADC); // deisable temperature sensor
//  adc_disable_channel_differential_input(ADC, ADC_CHANNEL_7);
//  adc_configure_trigger(ADC, ADC_TRIG_SW, 1); // triggering from software, freerunning mode
//  adc_disable_all_channel(ADC);
//  adc_enable_channel(ADC, ADC_CHANNEL_7); // just one channel enabled
//  adc_start(ADC);
//}
//
//void loop() 
//{
//  while(1)
//  {
//    PIO_Set(PIOB,PIO_PB27B_TIOB0);
//    while ((adc_get_status(ADC) & ADC_ISR_DRDY) != ADC_ISR_DRDY)
//      {}; //Wait for end of conversion
//    PIO_Clear(PIOB,PIO_PB27B_TIOB0);
//    val = adc_get_latest_value(ADC); // Read ADC
//
//    Serial.print(0);  // To freeze the lower limit
//    Serial.print(" ");
//    Serial.print(4095);  // To freeze the upper limit
//    Serial.print(" ");
//    Serial.println(val);
//  }
//}
