#include "myDebug.h"
#include "rpiSerialMonitor.h"


#define PIN_ONBOARD_LED  13

#define BLINK_ON_TIME   50
#define BLINK_OFF_TIME 950
elapsedMillis blink_time = 0;
int blink_state = 0;


rpiSerialMonitor monitor(0,0);


void setup() 
{ 
    Serial.begin(115200);   // 115200);   // 921600);
    Serial1.begin(115200);
    delay(1000);
    display(0,"teensyPi started",0);
    pinMode(PIN_ONBOARD_LED, OUTPUT);
}



void loop() 
{
    if (blink_time > (blink_state ? BLINK_ON_TIME : BLINK_OFF_TIME))
    {
        blink_time = 0;
        blink_state = !blink_state;
        digitalWrite(PIN_ONBOARD_LED,blink_state);
    }
    monitor.task();
}
