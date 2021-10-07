#include <WS2812Serial.h>

const int numled = 100;
const int pin = 1;

// Usable pins:
//   Teensy LC:   1, 4, 5, 24
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
//   Teensy 3.5:  1, 5, 8, 10, 26, 32, 33, 48
//   Teensy 3.6:  1, 5, 8, 10, 26, 32, 33
//   Teensy 4.0:  1, 8, 14, 17, 20, 24, 29, 39
//   Teensy 4.1:  1, 8, 14, 17, 20, 24, 29, 35, 47, 53

byte drawingMemory[numled * 3];         //  3 bytes per LED
DMAMEM byte displayMemory[numled * 12]; // 12 bytes per LED

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRB);

#define RED 0xFF0000
#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define YELLOW 0xFFFF00
#define PINK 0xFF1088
#define ORANGE 0xE05800
#define WHITE 0xFFFFFF

#define ARRAYSIZE(x)  (sizeof(x) / sizeof(x[0]))


class Mode
{
public:
    virtual void update(){}
};

class Rainbow : public Mode
{
public:
    int offset = 0;
    int offset_multiplier = 15;
    int offset_sign = 1;

    int time_delay = 1000;

    int prev_time = 0;
    int switch_time = 1000;

    int rgb_min = 0;

    Rainbow(int rgb_min_input)
    {
        rgb_min = rgb_min_input;
    }

    void update()
    {
        Serial.println("Rainbow");
        
        for (int i = 0; i < leds.numPixels(); i++)
        {
            uint32_t color = getRainbowColor(i * offset_multiplier + offset, rgb_min, 255);
            leds.setPixel(i, color);
        }

        leds.show();
        delayMicroseconds(time_delay);

        if (millis() - prev_time > switch_time)
        {
            prev_time = millis();

            switch_time = random(1000, 10000);
            time_delay = random(1000, 10000);

            offset_sign = -offset_sign;
        }
        else
        {
            offset += offset_sign;
            if (offset > 255 * 6)
                offset = 0;
            if (offset <= 0)
                offset = 255 * 6;
        }
    }

    uint32_t getRainbowColor(int count, int min_bright, int max_bright)
    {
        int r, g, b;

        count %= 255 * 6;

        //value of crescendo increases from 0 to max_bright - min_bright as count goes from 0 to 255
        int crescendo = (int)((count % 255) / 255.0 * (max_bright - min_bright));
        if (crescendo < 0)
            crescendo = 0;

        if (count < 255)
        {
            r = max_bright;
            g = min_bright + crescendo;
            b = min_bright;
        }
        else if (count < 255 * 2)
        {
            r = max_bright - crescendo;
            g = max_bright;
            b = min_bright;
        }
        else if (count < 255 * 3)
        {
            r = min_bright;
            g = max_bright;
            b = min_bright + crescendo;
        }
        else if (count < 255 * 4)
        {
            r = min_bright;
            g = max_bright - crescendo;
            b = max_bright;
        }
        else if (count < 255 * 5)
        {
            r = min_bright + crescendo;
            g = min_bright;
            b = max_bright;
        }
        else if (count < 255 * 6)
        {
            r = max_bright;
            g = min_bright;
            b = max_bright - crescendo;
        }

        b *= 0.5;

        uint32_t color = 0;
        color |= g;
        color |= (b << 8);
        color |= (r << 16);
        return color;
    }
};

class Halloween : public Mode
{
public:
    void update(){
        Serial.println("Halloween");

        for (int i = 0; i < leds.numPixels(); i++)
        {
            uint32_t color;
            if(millis() % 1000 < 500){
                color = RED;
            }else{
                color = ORANGE;
            }
            leds.setPixel(i, color);
        }
        leds.show();
    }
};

boolean lastbutton10 = false;

int mode_id = 0;

Rainbow rainbow0 = Rainbow(0);
Rainbow rainbow1 = Rainbow(75);
Halloween halloween = Halloween();

Mode* modes[3];

void setup()
{
  modes[0] = &rainbow0;
  modes[1] = &rainbow1;
  modes[2] = &halloween;
    leds.begin();

    Serial.begin(9600);

    pinMode(5, INPUT_PULLUP);
    pinMode(10, INPUT_PULLUP);
}

void loop()
{

    if (digitalRead(5))
    {
      int32_t modeptr = modes[mode_id];
        Serial.println(modeptr);
        int32_t rainbowptr = &rainbow1;
        Serial.println(rainbowptr);

        //if toggle switch changes states
        if (digitalRead(10) != lastbutton10)
        {
            Serial.println("switch mode");

            //increment color state, loop around if too big
            mode_id++;
            mode_id %= ARRAYSIZE(modes);

            
        }
        lastbutton10 = digitalRead(10);

        //do whatever each mode does

//        rainbow1.update();
        //halloween.update();
        modes[mode_id]->update();
    }
    else
    {
        Serial.println("off");

        //turn leds off, wait 1 second before checking if on again
        for (int i = 0; i < leds.numPixels(); i++)
        {
            leds.setPixel(i, 0x000000);
        }
        leds.show();
        delay(1000);
    }
}
