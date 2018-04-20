

#include "/Users/andrewfyfe/Documents/Arduino/ArduinoClasses/ArduinoCircularBuffer/ArduinoCircularBufferClass.h"
#include "/Users/andrewfyfe/Documents/Arduino/ArduinoClasses/ArduinoMedianFilter/ArduinoMedianFilter.h"


void setup() {
   //put your setup code here, to run once:
    Serial.begin(115200);
    ArduinoCircularBuffer<double> circle(10);
    Serial.println(circle.size());
    
    Serial.println("Adding 9 values\n");
    for(uint32_t i = 0; i < circle.size(); i++)
    {
        circle.put(i);
    }
    
    double* life = circle.getBuffer();
    
    for(uint32_t i = 0; i < circle.size(); i++)
    {
        Serial.println(circle.get());
    }
    
    double medianValue = findMedian(&life[0], circle.size());
    Serial.println(medianValue);

}

void loop() {
  // put your main code here, to run repeatedly:

}
