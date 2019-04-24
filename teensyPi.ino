#include "myDebug.h"
#include "myKeyPad.h"
#include "rpiSerialMonitor.h"

#define TEST_KEYPAD   0


#define PIN_ONBOARD_LED     13
#define BLINK_ON_TIME       50
#define BLINK_OFF_TIME      950
elapsedMillis blink_time = 0;
int blink_state = 0;


rpiSerialMonitor monitor(0,0);

#if TEST_KEYPAD

    myKeyPad keypad;
    
    void downHandler(void *param, int row, int col, uint8_t state)
    {
        display(0,"downHandler(%d,%d) param(%d) state(0x%02X)",row,col,(int)param,state);
    }
    void upHandler(void *param, int row, int col, uint8_t state)
    {
        display(0,"upHandler(%d,%d) param(%d) state(0x%02X)",row,col,(int)param,state);
    }
    void shortHandler(void *param, int row, int col, uint8_t state)
    {
        display(0,"shortHandler(%d,%d) param(%d) state(0x%02X)",row,col,(int)param,state);
    }
    void longHandler(void *param, int row, int col, uint8_t state)
    {
        display(0,"longHandler(%d,%d) param(%d) state(0x%02X)",row,col,(int)param,state);
    }

#endif  // TEST_KEYPAD


void setup() 
{ 
    Serial.begin(115200);   // 115200);   // 921600);
    Serial1.begin(115200);
    
    #if TEST_KEYPAD
        for (int i=0; i<KP_NUM_ROWS*KP_NUM_COLS; i++)
        {
            keyPadButton *button = keypad.getButton(i);
            button->registerHandler((void *)237,KP_STATE_DOWN,&downHandler);
            button->registerHandler((void *)237,KP_STATE_UP,&upHandler);
            button->registerHandler((void *)237,KP_STATE_SHORT,&shortHandler);
            button->registerHandler((void *)237,KP_STATE_LONG,&longHandler);
        }
    #endif

    delay(1200);
    display(0,"teensyPi started",0);
}



void loop() 
{
    monitor.task();
    
    #if TEST_KEYPAD
        keypad.task();
    #endif
    
    if (blink_time > (blink_state ? BLINK_ON_TIME : BLINK_OFF_TIME))
    {
        blink_time = 0;
        blink_state = !blink_state;
        digitalWrite(PIN_ONBOARD_LED,blink_state);
    }
}
