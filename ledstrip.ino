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

// NOTE: Colors are in RBG form
#define RED 0xFF0000
#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define YELLOW 0xFFFF00
#define PINK 0xFF1088
#define ORANGE 0xE0004B
#define WHITE 0xFFFFFF

#define ARRAYSIZE(x)  (sizeof(x) / sizeof(x[0]))


class Mode
{
public:
    virtual void update(){}
    virtual uint32_t setMinInput(int rgb_min_input){}
    virtual uint32_t setMaxInput(int rgb_max_input){}
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
    int rgb_max = 0;

    Rainbow(int rgb_min_input, int rgb_max_input = 255)
    {
        setMinInput(rgb_min_input);
        setMaxInput(rgb_max_input);
    }

    void update()
    {
        //Serial.println("Rainbow");
        
        for (int i = 0; i < leds.numPixels(); i++)
        {
            uint32_t color = getRainbowColor(i * offset_multiplier + offset, rgb_min, rgb_max);
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

    uint32_t setMinInput(int rgb_min_input){
      rgb_min = rgb_min_input;
    }

    uint32_t setMaxInput(int rgb_max_input){
      rgb_max = rgb_max_input;
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

#define HALLOWEEN_TICKRATE              1000

#define HALLOWEEN_MODE_NORMAL           0
#define HALLOWEEN_MODE_LIGHTNING        1

#define HALLOWEEN_LIGHTNING_STEP_DARK   0
#define HALLOWEEN_LIGHTNING_STEP_FLASH  1

#define HALLOWEEN_LIGHTNING_CHANCE      1
#define HALLOWEEN_LIGHTNING_DELAY       50
#define HALLOWEEN_LIGHTNING_FLASH_MIN   2
#define HALLOWEEN_LIGHTNING_FLASH_MAX   5

#define HALLOWEEN_DARKNESS_DELAY_MIN    50
#define HALLOWEEN_DARKNESS_DELAY_MAX    500

#define HALLOWEEN_LIGHTNING_WIDTH_MIN   10
#define HALLOWEEN_LIGHTNING_WIDTH_MAX   50

public:
    void update(){
        if ((millis() - tickstart >= HALLOWEEN_TICKRATE) && mode == HALLOWEEN_MODE_NORMAL){
          tickstart = millis();
          if (random(100) < HALLOWEEN_LIGHTNING_CHANCE){
              mode = HALLOWEEN_MODE_LIGHTNING;
              lightning_flash_cnt = random(HALLOWEEN_LIGHTNING_FLASH_MIN, HALLOWEEN_LIGHTNING_FLASH_MAX + 1);
              darkness_delay = 1000;
              lightning_flash_num = 0;
              lightning_step = HALLOWEEN_LIGHTNING_STEP_DARK;
              starttime = millis();
              r = 0;
              g = 0;
              b = 0;
          }
        }
        //Serial.println("Halloween");
        if (mode == HALLOWEEN_MODE_NORMAL){
          normal();
        } else {
          lightning();
        }
        leds.show();

    }

private:
  uint32_t tickstart = 0;
  uint32_t ticktime;

  uint32_t starttime;
  uint32_t elapsed;
  uint32_t darkness_delay;

  uint32_t mode = HALLOWEEN_MODE_NORMAL;

  uint32_t lightning_flash_cnt;
  uint32_t lightning_flash_num;
  uint32_t lightning_step;

  uint32_t strike_width;
  uint32_t strike_position;

  uint32_t r;
  uint32_t g;
  uint32_t b;
  uint32_t w;
  uint32_t color;
  
  void normal(){
      uint32_t color;
      color = ORANGE;
      if (r < 0xE0){
        r += 1;
      }
      if (g < 0x4B){
        g += 1;
      }
      for (int i = 0; i < leds.numPixels(); i++)
      {
          color = 0;
          color += r << 16;
          color += b << 8;
          color += g << 0;
          leds.setPixel(i, color);
      }
  }
  void lightning(){
      elapsed = millis() - starttime;
      if (lightning_flash_num < lightning_flash_cnt){
          if (lightning_step == HALLOWEEN_LIGHTNING_STEP_DARK && elapsed > darkness_delay){
            starttime = millis();
            lightning_step = HALLOWEEN_LIGHTNING_STEP_FLASH;
            strike_width = random(HALLOWEEN_LIGHTNING_WIDTH_MIN, HALLOWEEN_LIGHTNING_WIDTH_MAX);
            strike_position = random(0, leds.numPixels() - strike_width);
            Serial.println("Strike");
            if (w < 205){
              w += 50;
            }
            
          } else if (lightning_step == HALLOWEEN_LIGHTNING_STEP_FLASH && elapsed > HALLOWEEN_LIGHTNING_DELAY){
            starttime = millis();
            lightning_step = HALLOWEEN_LIGHTNING_STEP_DARK;
            darkness_delay = random(HALLOWEEN_DARKNESS_DELAY_MIN, HALLOWEEN_DARKNESS_DELAY_MAX);
            lightning_flash_num ++;
            if (lightning_flash_num == lightning_flash_cnt){
              darkness_delay = (500 * lightning_flash_num);
            }
          } else {
              if (lightning_step == HALLOWEEN_LIGHTNING_STEP_FLASH){
                r = 255;
                g = 255;
                b = 255;
              } else {
                if (r > 2){
                  r -= 4;
                }
                if (g > 2){
                  g -= 4;
                }
                if (b > 2){
                  b -= 4;
                }
//                if (w >= 2){
//                  w -= 1;
//                }
              }
              for (int i = 0; i < leds.numPixels(); i++)
              {
                  color = 0;
                  if ((i > strike_position) && (i < (strike_position + strike_width))){
                    color += r << 16;
                    color += b << 8;
                    color += g << 0;
                  } else {
                    color += w << 16;
                    color += w << 8;
                    color += w << 0;
                  }
                  leds.setPixel(i, color);
              }
          }
      } else {
          if (elapsed < darkness_delay){
            if (r >= 2){
                r -= 2;
              }
              if (g >= 2){
                g -= 2;
              }
              if (b >= 2){
                b -= 2;
              }
              if (w >= 1){
                w -= 1;
              }
            for (int i = 0; i < leds.numPixels(); i++)
            {
                color = 0;
                if ((i > strike_position) && (i < (strike_position + strike_width))){
                  color += r << 16;
                  color += b << 8;
                  color += g << 0;
                } else {
                  color += w << 16;
                  color += w << 8;
                  color += w << 0;
                }
                leds.setPixel(i, color);
            }
          } else {
              mode = HALLOWEEN_MODE_NORMAL; 
              r = 0;
              g = 0;
              b = 0;
          }
      }
  }
};

boolean lastbutton10 = false;

int mode_id = 0;
int level;

Rainbow rainbow0 = Rainbow(0);
Rainbow rainbow1 = Rainbow(85);
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
    lastbutton10 = digitalRead(10);
}

void loop()
{

    if (digitalRead(5))
    {
        while (level < 255){
          level += 1;
          modes[1]->setMinInput(level/3);
          modes[mode_id]->setMaxInput(level);
          modes[mode_id]->update();
        }
        
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

        modes[mode_id]->update();
    }
    else
    {
        while (level > 0){
          level -= 1;
          modes[1]->setMinInput(level/3);
          modes[mode_id]->setMaxInput(level);
          modes[mode_id]->update();
        }
        
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
