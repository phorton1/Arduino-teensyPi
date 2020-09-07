#include "myDebug.h"
#include "rpiSerialMonitor.h"

#define WITH_SERIAL2   1


#define PIN_ONBOARD_LED     13
#define BLINK_ON_TIME       50
#define BLINK_OFF_TIME      950

elapsedMillis blink_time = 0;
int blink_state = 0;
rpiSerialMonitor monitor(0,0);


void setup()
{
    Serial.begin(115200);   // 115200);   // 921600);
    Serial1.begin(115200);

    pinMode(PIN_ONBOARD_LED,OUTPUT);

    delay(600);
    display(0,"teensyPi v1.1 started",0);

    #if WITH_SERIAL2
        display(0,"opening Serial2",0);
        Serial2.begin(115200);
    #endif

    monitor.rebootPi();
        // if both are started at the same time,
        // the teensy supresses the pi bootstrap

}


void loop()
{
    monitor.task();

    #if WITH_SERIAL2
        // If anything on the coming over serial 2
        // write it to the PI AND as a message over the main Serial
        if (Serial2.available())
        {
            int c = Serial2.read();
            Serial1.write(c);
            display(0,"serial2 received chr(%d) and sent it to Serial1",c);
        }
    #endif


    if (blink_time > (blink_state ? BLINK_ON_TIME : BLINK_OFF_TIME))
    {
        blink_time = 0;
        blink_state = !blink_state;
        digitalWrite(PIN_ONBOARD_LED,blink_state);
    }
}
