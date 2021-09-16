/* WS2812Serial BasicTest Example
   Test LEDs by turning then 7 different colors.
   This example code is in the public domain. */

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

byte drawingMemory[numled*3];         //  3 bytes per LED
DMAMEM byte displayMemory[numled*12]; // 12 bytes per LED

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRB);

#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF

void setup() {
  leds.begin();

  Serial.begin(9600);

  pinMode(5, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);

}

int offset = 0;
//int offset_multiplier = 10;
int offset_multiplier = 15;
int offset_sign = 1;

int time_delay = 1000;

int prev_time = 0;
int switch_time = 1000;

//float speed_factor = 1;
//float speed_limit = 100;
//float speed_increase = 0.001;
//uint32_t time_delay = 1;

boolean lastbutton5 = false;
boolean lastbutton10 = false;

int rgb_min = 0;


int color_state = 0;
boolean on = true;


void loop() {
//  if(digitalRead(5)){
//      if(!lastbutton5){
//        on = !on;
//        Serial.println("toggle on");
//      }
//      lastbutton5 = true;
//    }else{
//      lastbutton5 = false;
//    }
  if(digitalRead(5)){
    on=true;
  }else{
    on = false;
  }


  if(on){
    


    if(digitalRead(10) != lastbutton10){
      if(color_state == 1){
        color_state = 0;
      }else{
        color_state++;
      }

      if(color_state == 0){
        rgb_min = 0;
      }
      if(color_state == 1){
        rgb_min = 75;
      }
    }
    
    lastbutton10 = digitalRead(10);


//    if(digitalRead(10)){
//      if(rgb_min == 75){
//        rgb_min = 0;
//      }else{
//        rgb_min = 75;
//      }
//    }
    
    for (int i=0; i < leds.numPixels(); i++) {
//    uint32_t color = getColor(i*offset_multiplier+offset, 50, 100 + 50*(sin((i*offset_multiplier+offset)*0.02)+1));
      uint32_t color = getColor(i*offset_multiplier+offset, rgb_min, 255);
      leds.setPixel(i, color);
    }
    leds.show();
    delayMicroseconds(time_delay);
  
  //  speed_factor += speed_increase;
  //  if (speed_factor >= speed_limit){
  //    speed_increase = -speed_increase;
  //  } else if (speed_factor < 1){
  //    speed_increase = -speed_increase;
  //    speed_factor = 1;
  //  }
  //  time_delay = (uint32_t)(100000 / speed_factor);
  
  
  
    if (millis() - prev_time > switch_time){
      prev_time = millis();
  //    switch_time = random(500,5000);
  //    time_delay = random(500,10000);
      switch_time = random(1000,10000);
      time_delay = random(1000,10000);
      
      offset_sign = -offset_sign;
    } else {
       offset+= offset_sign;
      if(offset > 255*6) offset = 0;
      if(offset <= 0) offset = 255 * 6;
    }
  }else{
    for (int i=0; i < leds.numPixels(); i++) {
      leds.setPixel(i, 0x000000);
    }
    leds.show();
    delay(1000);

//    Serial.println("off");
//    Serial.println(digitalRead(5));
//    Serial.println("ddddd");
//
//    Serial.println(digitalRead(10));
//    
  }
  

}

uint32_t getColor(int count, int min_bright, int max_bright){
  int r, g, b;

  count %= 255*6;

  //value of crescendo increases from 0 to max_bright - min_bright as count goes from 0 to 255
  int crescendo = (int) ((count%255)/255.0 * (max_bright - min_bright));
  if(crescendo<0) crescendo = 0;
  
  if(count < 255){
     r = max_bright;
     g = min_bright + crescendo;
     b = min_bright;
   }
   else if(count < 255*2){
     r = max_bright - crescendo;
     g = max_bright;
     b = min_bright;
   }
   else if(count < 255*3){
     r = min_bright;
     g = max_bright;
     b = min_bright + crescendo;
   }
   else if(count < 255*4){
     r = min_bright;
     g = max_bright - crescendo;
     b = max_bright;
   }
   else if(count < 255*5){
     r = min_bright + crescendo;
     g = min_bright;
     b = max_bright;
   }
   else if(count < 255*6){
     r = max_bright;
     g = min_bright;
     b = max_bright - crescendo;
   }

   b*= 0.5;

  uint32_t color = 0;
  color |= g;
  color |= (b << 8);
  color |= (r << 16);
  return color;
}
