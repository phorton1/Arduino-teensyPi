#include "myDebug.h"
#include "rpiSerialMonitor.h"

#define PIN_ONBOARD_LED     13
#define BLINK_ON_TIME       50
#define BLINK_OFF_TIME      950

elapsedMillis blink_time = 0;
int blink_state = 0;
rpiSerialMonitor monitor(0,0);


void setup()
{
    Serial.begin(115200);   // 115200);   // 921600);
    delay(2000);
    display(0,"teensyPi v1.4 started",0);

    Serial1.begin(115200);
    display(0,"opening Serial2",0);
    Serial2.begin(115200);

    pinMode(PIN_ONBOARD_LED,OUTPUT);

    monitor.init();
    monitor.rebootPi();
        // if both are started at the same time,
        // the teensy supresses the pi bootstrap

}


void loop()
{
    monitor.task();

    // My midi protocol
    //
    // is 4 byte packets that look just like USB midi packets
    //
    //     CIN           |  MIDI0   |  MIDI1   |  MIDI2   |
    //    byte0          |  byte1   |  byte2   |  byte3   |
    //-------------------------------------------------------
    // cable  |  code    |          |          |          |
    // number |  index   |          |          |          |
    //        |  number  |          |          |          |
    //
    // The "cable number" is always zero.
    //
    // The CIN may be one of the standard values from 0x00 to 0x0F,
    // EXCEPT that 0xA and 0xD, which are seldom used MIDI packets
    // for "poly-keypress" and "channel pressure" ARE NOT USED.
    // Those values are "reserved" for TEXT carriage returns and
    // linefeeds, so that this protocol can be sent "within" a
    // potential text streem (i.e. debugging output).
    //
    // Thus, when we see a byte 0x0 thru 0xF (except 0xA and 0xD)
    // we presume there is a 4 byte (32 bit) midi packet present,
    // which we pass onto the raspberry pi via the main serial
    // port.
    //
    // The protocol is bi-directional.

    // So, if we identify a packet, we pass it on to the rPI.
    // otherwise, we echo the output to the teensyPi USB port
    // and see what happens.  To make debugging output look right,
    // we have to buffer, and break on carriage returns,
    // and present a "header" before the line oriented degug output

    #define MAX_LINE_BUFFER 512
    static int midi_ptr = 0;
    static int line_ptr = 0;
    static unsigned char midi_buffer[4];
    static unsigned char line_buffer[MAX_LINE_BUFFER];

    if (Serial2.available())
    {
        unsigned char c = Serial2.read();
        // warning(0,"teensyPI got SERIAL2(0x%02x == %d) '%c'",c,c,(c>32?c:32));

        if (midi_ptr)
        {
            midi_buffer[midi_ptr++] = c;
            // display(0,"teensyPI adding to midi packet(0x%02x == %d)",c,c);
            if (midi_ptr == 4)
            {
                // display_bytes(0,"teensyPi sending packet to rPI",midi_buffer,4);
                Serial1.write(midi_buffer,4);
                midi_ptr = 0;
            }
        }
        else if (c <= 0xf && c != 0xA && c != 0xD)
        {
            midi_buffer[midi_ptr++] = c;
            // display(0,"teensyPI starting midi packet with (0x%02x == %d)",c,c);
        }
        else if (c == 0xA || c == 0xD || line_ptr >= MAX_LINE_BUFFER-3)
        {
            if (line_ptr)
            {
                display(0,"TE: %s",line_buffer);
            }
            line_buffer[0] = 0;
            line_ptr = 0;
        }
        else
        {
            line_buffer[line_ptr++] = c;
            line_buffer[line_ptr] = 0;
        }
    }

    if (blink_time > (blink_state ? BLINK_ON_TIME : BLINK_OFF_TIME))
    {
        blink_time = 0;
        blink_state = !blink_state;
        digitalWrite(PIN_ONBOARD_LED,blink_state);
    }
}
