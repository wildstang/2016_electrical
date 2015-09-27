#include <Adafruit_DotStar.h>
#include <SPI.h>
 
#define DATAPIN 11
#define CLOCKPIN 13
#define nLED 18

Adafruit_DotStar strip = Adafruit_DotStar(nLED, DATAPIN, CLOCKPIN);

 void setup()
 {
   strip.begin();
   strip.show();
 }
 
  void loop()
 {
   rainbowFade();
 }

uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   //Red down
      g = WheelPos % 128;      // Green up
      b = 0;                  //blue off
      break; 
    case 1:
      g = 127 - WheelPos % 128;  //green down
      b = WheelPos % 128;      //blue up
      r = 0;                  //red off
      break; 
    case 2:
      b = 127 - WheelPos % 128;  //blue down 
      r = WheelPos % 128;      //red up
      g = 0;                  //green off
      break; 
  }
  return(strip.Color(r,g,b));
}
 
 void rainbowCycle()
 {
   
 int i, j;
 uint32_t color; 
   for (j=0; j < 384 * 5; j+=75)
   {
       for (int i = 0; i < 6; i++)
       {
          color = Wheel( ((i * 384 / strip.numPixels()) + j) % 384);
          
          strip.setPixelColor(i ,  color );  
          strip.setPixelColor(11 - i ,  color );
          strip.setPixelColor(12 + i ,  color );
          strip.show(); 
 }
 
 
 void rainbowFade()
{
 int i, j;
 uint32_t color; 
   for (j=0; j < 384 * 5; j+=75)
   {
       for (int i = 0; i < 6; i++)
       {
          color = Wheel( ((i * 384 / strip.numPixels()) + j) % 384);
          
          strip.setPixelColor(i ,  color );  
          strip.setPixelColor(11 - i ,  color );
          strip.setPixelColor(12 + i ,  color );
          strip.show();

          color = 0;
delay(50);          
          strip.setPixelColor(i ,  color );  
          strip.setPixelColor(11 - i ,  color );
          strip.setPixelColor(12 + i ,  color );
          strip.show();
          
          delay(50);
      }
   }
}

void rainbowChase()
{
 int i, j;
 uint32_t color; 
   for (j=0; j < 384 * 5; j+=5)
   {
       for (int i = 0; i < nLED; i++)
       {
          color = Wheel( ((i * 384 / strip.numPixels()) + j) % 384);
          
          strip.setPixelColor(i ,  color );
         // strip.setPixelColor(i-5 ,  strip.Color(0,0,0));  
          strip.show();
       }
   }
}
