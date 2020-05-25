#include "myDebug.h"
#include "rpiSerialMonitor.h"


#define PIN_ONBOARD_LED     13
#define BLINK_ON_TIME       50
#define BLINK_OFF_TIME      950

elapsedMillis blink_time = 0;
int blink_state = 0;
rpiSerialMonitor monitor(0,0);


#define WITH_EXPRESSION_PEDAL  0

#if WITH_EXPRESSION_PEDAL

    #define NUM_PEDALS   2
    
    #define PIN_EXPR1    A0
    #define PIN_EXPR2    A1
    
    #define CALIBRATE_TIME              6000
    #define CALIBRATE_BLINK_ON_TIME       20
    #define CALIBRATE_BLINK_OFF_TIME     150
    
    typedef struct
    {
        int     pin;
        const   char *name;
        int     calib_min;
        int     calib_max;
        
        int     cable;
        int     channel;
        int     cc_num;
        
        int     raw_value;
        
    }   expressionPedal_t;

    expressionPedal_t pedals[NUM_PEDALS] = {
        { PIN_EXPR1, "Pedal1", 9, 1010, 0, 1, 0x0F, 0},
        { PIN_EXPR2, "Pedal2", 9, 1010, 0, 1, 0x0E, 0},
    };

    int calibrate_pedal = -1;
    elapsedMillis calibrate_time = 0;    
    
#endif



void setup() 
{ 
    Serial.begin(115200);   // 115200);   // 921600);
    Serial1.begin(115200);
    pinMode(PIN_ONBOARD_LED,OUTPUT);
    
    #if WITH_EXPRESSION_PEDAL
    
        for (int i=0; i<NUM_PEDALS; i++)
            pinMode(pedals[i].pin,INPUT);   // _PULLDOWN);
    #endif    
        
    delay(600);
    display(0,"teensyPi v1.1 started",0);
}



#if WITH_EXPRESSION_PEDAL

    #define CALIB_MARGIN 10
        // added to top and bottom to give
        // "safe" 0 and 127 values
    
    void calibrate()
    {
        int raw_value = analogRead(pedals[calibrate_pedal].pin);
        if (raw_value-CALIB_MARGIN > pedals[calibrate_pedal].calib_max)
        {
            // display(0,"max:%d",raw_value);
            pedals[calibrate_pedal].calib_max = raw_value - CALIB_MARGIN;
            if (pedals[calibrate_pedal].calib_max < 0)
                pedals[calibrate_pedal].calib_max =0;
        }
        if (raw_value + CALIB_MARGIN < pedals[calibrate_pedal].calib_min)
        {
            // display(0,"min:%d",raw_value);
            pedals[calibrate_pedal].calib_min = raw_value + CALIB_MARGIN;
            if (pedals[calibrate_pedal].calib_min > 1023)
                pedals[calibrate_pedal].calib_min = 1023;
        }
        if (blink_time > (blink_state ? CALIBRATE_BLINK_ON_TIME : CALIBRATE_BLINK_OFF_TIME))
        {
            blink_time = 0;
            blink_state = !blink_state;
            digitalWrite(PIN_ONBOARD_LED,blink_state);
        }
    }
    
    void startCalibrate(int pedal)
    {
        #if WITH_SERIAL
            display(0,"CALIBRATE %s",pedals[pedal].name);
        #endif
        calibrate_pedal = pedal;
        calibrate_time = 0;
        pedals[pedal].calib_min = 1023;
        pedals[pedal].calib_max = 0;
    }
    
    
    #define HYSTERISIS   30
    
    int getValue(int pedal)
    {
        static int direction = 0;
        static elapsedMillis settle_time = 0;
        
        int raw_value = analogRead(pedals[0].pin);
        int pedal_raw = pedals[0].raw_value;
        
        if (!direction)
        {
            if (raw_value > pedal_raw + HYSTERISIS)
            {
                direction = 1;
                pedal_raw = raw_value;
                settle_time = 0;
            }
            else if (raw_value < pedal_raw - HYSTERISIS)
            {
                direction = -1;
                pedal_raw = raw_value;
                settle_time = 0;
            }
        }
        else if (settle_time > 50)
        {
            settle_time = 0;
            direction = 0;
        }
        else if (direction > 0 && raw_value > pedal_raw)
        {
            pedal_raw = raw_value;
            settle_time = 0;
        }
        else if (direction < 0 && raw_value < pedal_raw)
        {
            pedal_raw = raw_value;
            settle_time = 0;
        }
        pedals[pedal].raw_value = pedal_raw;
        
        int value = map(pedal_raw,pedals[pedal].calib_min,pedals[pedal].calib_max,0,127);
        if (value > 127) value=127;
        if (value < 0) value=0;
        return value;
    }
    
#endif


void loop() 
{
    monitor.task();
    
    #if WITH_EXPRESSION_PEDAL
        if (monitor.rpiReady())
        {
            static int last_value = 0;
            int value = getValue(0);
            if (value != last_value)
            {
                last_value = value;
                display(0,"value=%d",value);
                
                char buf[5];
                buf[0] = pedals[0].cable;       // cable
                buf[1] = pedals[0].channel;     // channel
                buf[2] = 0x0B;                  // msg
                buf[3] = pedals[0].cc_num;      // param1
                buf[4] = value;
                Serial1.write(buf,5);
                
                #if WITH_MIDI
                    usbMIDI.sendControlChange(      // sends 0xB0 message
                        pedals[0].cc_num,           // to given cc_num
                        value,          
                        pedals[0].channel,          // to given channel
                        pedals[0].cable);           // on given cable
                #endif
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
