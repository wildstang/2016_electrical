#include <Adafruit_DotStar.h>
#include <SPI.h>

#define DATAPIN 11
#define CLOCKPIN 13
#define nLED 90

Adafruit_DotStar strip = Adafruit_DotStar(nLED, DATAPIN, CLOCKPIN);

int loopcount = 0;
int z=1;

int allianceRed;
int allianceBlue;
int allianceColor;   

void setup() 
  {
    allianceRed = strip.Color(0,255,0);
    allianceBlue = strip.Color(0,0,255);
    allianceColor = allianceBlue;
    
    strip.begin();
    strip.show(); //Sets all pixels to off
    Serial.begin(9600);
  }
  
void loop()
{
  rainbowCycle();
}



/* ======================================================== */
    // Reset Loopcount Variable to 1
    
void blankLoopCount()
{
  loopcount = 0;
}

/* ======================================================== */
    // Reset Strand or Turns Off
    
void blankWipe()
{
  int i;
  
   for (i = 0; i < nLED; i++)
  {
  strip.setPixelColor(i, 0);
  }

  strip.show();
}



/* ======================================================== */
    // Reset Strand or Turns Off

void blankFade()
{
  int i;
  
   for (i = 0; i < nLED; i++)
  {
  strip.setPixelColor(i, 0);
  strip.show();
  }
}

/* ======================================================== */
    // Solid Blue
    
void solidColor()
{
   int i;
   for(i=0; i < nLED; i++) 
   {
     strip.setPixelColor(i, allianceColor );
     strip.show();
   }
}

/* ======================================================== */
    // Fill Blue
    
void fillColor()
{
  int i;
  
  for (i=0; i< nLED; i++)
  {
    strip.setPixelColor(i, allianceColor);
    strip.show();
  }
}

/* ======================================================== */
    // Chase Blue

void chaseColor()
{
 int i;
 
    for(i=0; i < nLED+1; i++)
     {
      strip.setPixelColor(i, allianceColor);
      strip.show();
      delay(10);
      
      strip.setPixelColor(i-1, strip.Color(0,0,0));
      strip.show();
      delay(10);
     } 
   loopcount++;
 
}

/* ======================================================== */
    // Chase Blue 2
    
void chaseColor2()
{
      uint32_t blank = strip.Color(0,0,0);
  
        for (int i = 0; i <60; i++ ) 
        {
          if ((loopcount + i) % 2 == 0)
           {
           strip.setPixelColor(i, allianceColor);
            }
           else if ((loopcount + i) % 3 == 0)
           {
            strip.setPixelColor(i, blank);
           }
        }
      
    //blank();
      
      strip.show(); 
   
      loopcount++;
      delay(75);  // wait 1 second
      strip.show();
    
}

/* ======================================================== */
    // Loopy Blue
  
void loopyColor()
{
  while (loopcount <=10)
    {
      loopyRunColor();
      loopyReturnColor();
      loopcount++;
    }
}

  void loopyRunColor()
  {
   int i;
  
    for(i=0; i < nLED+1; i++)
     {
      strip.setPixelColor(i, allianceColor);
      strip.show();
      delay(10);
      
      strip.setPixelColor(i-3, strip.Color(0,0,0));
      strip.show();
      delay(10);
     } 
  }
  
  void loopyReturnColor()
  {
    int i;
     for(i=0 ; i < nLED; i++)
     {
      strip.setPixelColor(56-i  , allianceColor); 
      strip.show();
      delay(10);
      
      strip.setPixelColor(59-i, strip.Color(0,0,0));
      strip.show();
      delay(10);
     } 
  }

/* ======================================================== */
    // Pulse Blue
    
void pulseColor()
{
 int i, j;
  
  if ( allianceColor = allianceRed)
  {
    for(j=0; j < 255; j++)
      {
       strip.show();
       for(i=0; i < nLED; i++) 
         {
           strip.setPixelColor(i, strip.Color(j,0,0));
         }
      }
    
    for(j=255; j > 0; j--)
      {
       strip.show();
       for(i=0; i < nLED; i++) 
         {
           strip.setPixelColor(i, strip.Color(j,0,0));
         }
      }
  }
  
  else
  {
    for(j=0; j < 255; j++)
      {
       strip.show();
       for(i=0; i < nLED; i++) 
         {
           strip.setPixelColor(i, strip.Color(0,0,j));
         }
      }
    
    for(j=255; j > 0; j--)
      {
       strip.show();
       for(i=0; i < nLED; i++) 
         {
           strip.setPixelColor(i, strip.Color(0,0,j));
         }
      }
  }
}


/* ======================================================== */
    // Flash Blue
    
void flashColor()
{
   int i;
   
    for(i=0; i < nLED; i++) 
     {
       strip.setPixelColor(i, allianceColor); 
     }
  strip.show();
  delay(500);
  blankWipe();
  delay(500);
}
   

/* ======================================================== */
    // Twinkle Blue
    
void twinkleColor()
{
      uint32_t blank = strip.Color(0,0,0);
      uint32_t color = allianceColor;
      
//  while (loopcount <=100)
//    {
      if (loopcount%2 == 0)
      {
        for (int i = 0; i <60; i++  ) 
        {
          if ((loopcount + i) % 2 == 0)
           {
           strip.setPixelColor(i, blank);
            }
          else
           {
            strip.setPixelColor(i, color);
           }
        }
      }
      
      else
      {
        for (int i = 0; i <60; i++  ) 
        {
        if ((loopcount + i) % 2 == 0)
           {
           strip.setPixelColor(i, blank);
            }
        else
           {
            strip.setPixelColor(i, color);
           }
        }
    //blank();
      }
      strip.show(); 
   
      loopcount++;
      delay(75);  // wait 1 second
      strip.show();
   } 
//}


/* ======================================================== */
    // Random Blue
    
void randomColor()
{
  while ( loopcount <= 5 )
  {
    int i;
    
    if ( allianceColor = allianceRed)
    {
      int r = random(0,50);
      int b = random(200,255);
    
      for ( i=0; i < nLED ; i++)
      {
        strip.setPixelColor(i , r, 0, b);
        delay(3);
      }
      strip.show();
     }
     
     else
     {
       int r = random(200,255);
       int b = random(0,50);
    
        for ( i=0; i < nLED ; i++)
          {
            strip.setPixelColor(i , r, 0, b);
            delay(3);
          }
      strip.show();
     }
   }
}


/* ======================================================== */
    //  Blue Fade Purple
    
void colorFadeColor()
{
  while ( loopcount <= 5 )
  {
   int i = 1;
   int j;

     for (j = 0 ; j < nLED; j++)                       // Define the amount of pixels
     {
       if (allianceColor = allianceRed)
       {
         if (i <= 255)                                   // Define the color value
           {
             i+=2; 
             strip.setPixelColor(j, strip.Color(255,0,i));   // setPixelColor HAS to have two main values: Amount of pixels and color
             strip.show();
           }
        delay(500);
        blankFade();
      }
      
      else
       {
         if (i <= 255)                                   // Define the color value
           {
             i+=2; 
             strip.setPixelColor(j, strip.Color(i,0,255));   // setPixelColor HAS to have two main values: Amount of pixels and color
             strip.show();
           }
        delay(500);
        blankFade();
      }
    }
  }
}


/* ======================================================== */

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


/* ======================================================== */

void rainbow() {
  int i, j;
   
  for (j=0; j < 384; j++) 
  {     // 3 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, Wheel( (i + j) % 384));
    }  
    strip.show();   // write all the pixels out
    delay(100);
  }
}

/* ======================================================== */

void rainbowCycle()
{
  uint16_t i, j;
  
  for (j=0; j < 384 * 5; j++) {     // 5 cycles of all 384 colors in the wheel
                                        // There is a total of 384 colors
                                        
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 384-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 384 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 384 / strip.numPixels()) + j) % 384) );
    }  
    strip.show();   // write all the pixels out
    delay(100);
  }
}

/* ======================================================== */
    // Switch Statement
  
void pulseRainbow()
{
 int i, j;
  
  if ( allianceColor = allianceRed)
  {
    for(j=0; j < 384; j++)
      {
       strip.show();
       for(i=0; i < nLED; i++) 
         {
           strip.setPixelColor(i, strip.Color(j,0,0));
         }
      }
    
    for(j=255; j > 0; j--)
      {
       strip.show();
       for(i=0; i < nLED; i++) 
         {
           strip.setPixelColor(i, strip.Color(j,0,0));
         }
      }
  }
      
/* ======================================================== */
    // Switch Statement    
    
void switchColor()
{
  switch(z)
    {
      case 1:
        solidColor();
        blankFade();
        z++;
        break;
        
       case 2:
         fillColor();
         blankWipe();
         z++;
         break;
         
      case 3:
         chaseColor();
         z++;
         break;
         
      case 4:
         loopyColor();
         blankLoopCount();
         z++;
         break;
         
      case 5:
        pulseColor();
        pulseColor();
        blankWipe();
        z++;
        break;
        
     case 6:
         flashColor();
         flashColor();
         z++;
         break;
         
     case 7:
         twinkleColor();
         blankWipe();
         //blankLoopCount();
         z++;
         break;
         
     case 8:
         randomColor();
         blankLoopCount();
         z++;
         break;
         
     case 9:
         colorFadeColor();
         z = 1;
         break;
       
  }
}
