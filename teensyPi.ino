#include "myDebug.h"
#include "rpiSerialMonitor.h"

#define SENSE_RPI   1

#if SENSE_RPI
    #define PIN_RPI_SENSE      2      // handled by Looper.ino for teensy case
    #define PIN_RPI_INDICATOR 11      // handled by Looper.ino for teensy case
#endif

#define PIN_ONBOARD_LED     13

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

    #if SENSE_RPI
        pinMode(PIN_RPI_SENSE,INPUT_PULLDOWN);
        pinMode(PIN_RPI_INDICATOR,OUTPUT);
        digitalWrite(PIN_RPI_INDICATOR,0);
    #endif
}



void loop() 
{
    #if SENSE_RPI
        static uint8_t teensy_sensed = 0;
        if (digitalRead(PIN_RPI_SENSE) != teensy_sensed)
        {
            teensy_sensed = !teensy_sensed;
            digitalWrite(PIN_RPI_INDICATOR,teensy_sensed);
        }
    #endif
    
    if (blink_time > (blink_state ? BLINK_ON_TIME : BLINK_OFF_TIME))
    {
        blink_time = 0;
        blink_state = !blink_state;
        digitalWrite(PIN_ONBOARD_LED,blink_state);
    }
    monitor.task();
}
