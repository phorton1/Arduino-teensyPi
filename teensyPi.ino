#include "myDebug.h"
#include "myKeyPad.h"
#include "rpiSerialMonitor.h"

#define TEST_KEYPAD   0
#define TEST_LOOPER_BUTTONS  0

#if TEST_LOOPER_BUTTONS
    #define PIN_BUTTON1    18
    #define PIN_BUTTON2    19
    #define PIN_VOLUME1    20
    #define PIN_VOLUME2    21
#endif


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
    pinMode(PIN_ONBOARD_LED,OUTPUT);
    
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

    #if TEST_LOOPER_BUTTONS
        pinMode(PIN_BUTTON1,INPUT_PULLUP);
        pinMode(PIN_BUTTON2,INPUT_PULLUP);
        pinMode(PIN_VOLUME1,INPUT_PULLDOWN);
        pinMode(PIN_VOLUME2,INPUT_PULLDOWN);
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

    #if TEST_LOOPER_BUTTONS
        #define VOLUME_DRIFT 2
        static int volume1 = 0;
        static int volume2 = 0;
        static uint8_t button1_state = 0;
        static uint8_t button2_state = 0;
        static elapsedMillis vol_check = 0;
        
        if (myButtonPressed(PIN_BUTTON1,&button1_state))
            display(0,"Loop Button 1 pressed",0);
        if (myButtonPressed(PIN_BUTTON2,&button2_state))
            display(0,"Loop Button 2 pressed",0);
        
        if (vol_check > 100)
        {
            vol_check = 0;
            int vol1 = map(analogRead(PIN_VOLUME1),0,1023,0,101)-2;
            if (vol1 < 0) vol1 = 0;
            if (abs(vol1-volume1) > VOLUME_DRIFT)
            {
                volume1 = vol1;
                display(0,"Volume1 == %d",volume1);
            }
            
            int vol2 = map(analogRead(PIN_VOLUME2),0,1023,0,101)-2;
            if (vol2 < 0) vol2 = 0;
            if (abs(vol2-volume2) > VOLUME_DRIFT)
            {
                volume2 = vol2;
                display(0,"Volume2 == %d",volume2);
            }
        }
    #endif
    
    if (blink_time > (blink_state ? BLINK_ON_TIME : BLINK_OFF_TIME))
    {
        blink_time = 0;
        blink_state = !blink_state;
        digitalWrite(PIN_ONBOARD_LED,blink_state);
    }
}
