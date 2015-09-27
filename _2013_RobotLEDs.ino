#include <LPD8806.h>

/*
Wildstang Lights 2013
By: Josh Smith and Steve Garward
*/

// Since the cRIO operates at 38.4kHz for its I2C clock, we need to set this manually here.
// This accesses the low level TWI library files found in /Wire/utility/
// This needs to be defined before we include as there is a check for a define in twi.c
#define TWI_FREQ 38400L

#include <LPD8806.h>
#include "SPI.h"
#include <Wire.h>
#include "definitions.h"

// Uncomment the following line if you want to enable debugging messages over serial
//#define WS_DEBUG

// This will use the following pins (SPI):
// Data (SDA):  11
// Clock (SCL): 13
LPD8806 strip = LPD8806(STRIP_LENGTH);

unsigned long trailPattern[TRAIL_LENGTH + 1];

// This the address to use for the arduino in slave mode when talking to the cRIO.
// This number may differ from the address that the cRIO expects. Sniff the I2C lines
// to check what address correlates to the actual integer given here
static const byte i2cAddress = 82;

// This boolean gets set when we have new data that has been verified to be correct following our checks
boolean dataChanged = false;

// Create the variables that are used for raw data packets
// These should not be altered as they are handled internally by the recieveData function
unsigned char dataByte1 = 0;
unsigned char dataByte2 = 0;
unsigned char dataByte3 = 0;
unsigned char dataByte4 = 0;
unsigned char dataByte5 = 0;

// Create the variables that are actually used within the light functions.
// These are the actual variables that should be compared with but they should not be manually altered
unsigned char commandByte  = 0;
unsigned char payloadByte1 = 0;
unsigned char payloadByte2 = 0;

// This is used for the initial 30 second wait period where our robot would calibrate the gyro and could not be touched.
// Just switch this to "false" if you do not want this initial wait period.
boolean firstRun = true;

/****************************** PICK YOUR ALLIANCE ******************************/
/*                             (uncomment alliance)                             */

#define RED_ALLIANCE
//#define BLUE_ALLIANCE

// 1 for red, 2 for blue
byte alliance = 0;

/******************************************************************************/

// Appendage States
boolean climbOut = false;
boolean intakeOn = false;

void setup()
{
   // This reads the noise off of Analog input 2 and seeds the random() function
   randomSeed(analogRead(2));

   // Start the LED strip
   strip.begin();

   // Update the strip to ensure that all the LEDs are all off at the beginning
   strip.show();
  
   // Begin I2C communications as a SLAVE. receiveData() will be called when new data arrives.
   // We call this last to avoid a nasty bug involving the LED initialization code
   Wire.begin(i2cAddress);
   Wire.onReceive(receiveData);
}



void loop()
{

// Since we don't know what alliace we are on yet (due to the bootup time of the cRIO), we use the
// preprocessor to define what will be run. This will eventually be ported over to a digitalRead with a switch
#ifdef RED_ALLIANCE   
   if (firstRun == true)
   {
      gyroCalibrate(5, 50, 120, 127, 0, 0);
      initShootingTrailPattern(127, 0, 0);
      for(byte loops = 0; loops < 20; loops++)
      {
         faderRed(30);
      }
      firstRun = false;
   }
#endif

#ifdef BLUE_ALLIANCE   
   if (firstRun == true)
   {
      gyroCalibrate(5, 50, 120, 0, 0, 127);
      initShootingTrailPattern(0, 0, 127);
      for(byte loops = 0; loops < 20; loops++)
      {
         faderBlue(30);
      }
      firstRun = false;
   }
#endif

   // With the initial bootup run out of the way, we now enter the main chunk of the loop.
   // The program will sit in the "else" statement until a valid set of bytes are received
   if (
   (commandByte == 0x02) &&
   (payloadByte1 == 0x2F) &&
   (payloadByte2 == 0x12))
   {
      dataChanged = false;
      autonomous();
   }
   else if (
   (commandByte == 0x05) &&
   (payloadByte1 == 0x13) &&
   (payloadByte2 == 0x14))
   {
      dataChanged = false;
      shoot(10, 1000);
   }
   else if (
   (commandByte == 0x06) &&
   (payloadByte1 == 0x11) &&
   (payloadByte2 == 0x12))
   {
      dataChanged = false;
      climb(20, 20, 4, 50, 120, 800);
   }
   else if (
   (commandByte == 0x07) &&
   (payloadByte1 == 0x11) &&
   (payloadByte2 == 0x12))
   {
      dataChanged = false;
      if (intakeOn == false)
      {
         intakeOn = true;
         while (dataChanged == false)
         {
            // Check these values!
            intake(10, 500);
         }
      }
      else
      {
         intakeOn = false;
         setDrivingState();
      }
   }
   else if (
   (commandByte == 0x08) &&
   (payloadByte1 == 0x34) &&
   (payloadByte2 == 0x45))
   {
      dataChanged = false;
      if (alliance == 1)
      {
         scanner(127, 0, 0, 20, true);
      }
      else if (alliance == 2)
      {
         scanner(0, 0, 127, 20, true);
      }
      rainbowWheel(3);
   }
   else if (
   (commandByte == 0x04))
   {
      dataChanged = false;
      allianceSelection(6);
   }
   else
   {
      dataChanged = false;
      rainbowWheel(3);
   }
}

void colorChase(unsigned long c, byte wait)
{
   unsigned int i;

   for (i=0; i < strip.numPixels(); i++)
   {
      strip.setPixelColor(i, 0);
   }

   for (i=0; i < strip.numPixels(); i++)
   {
      strip.setPixelColor(i, c);
      if (i == 0)
      { 
         strip.setPixelColor(strip.numPixels()-1, 0);
      }
      else
      {
         strip.setPixelColor(i-1, 0);
      }
      strip.show();
      if (true == timedWait(wait))
      {
         return;
      }
   }
}

// Sets all pixels off to blank the entire strip.
void blankStrip()
{
   for (unsigned int i = 0; i < strip.numPixels(); i++)
   {
      strip.setPixelColor(i, 0);
   }
   strip.show();
}

// Turns off a specified range of pixels
void blankRange(unsigned int p_start, unsigned int p_end)
{
   for (unsigned int i = p_start; i < p_end; i++)
   {
      strip.setPixelColor(i, 0);
   }
   strip.show();
}

void testArrows()
{
   unsigned long color = 0;
 
   blankStrip();
  
   for (byte i = 0; i < 3; i++)
   {
      // Pick the color
      switch (i)
      {
         case 0: color = strip.Color(127, 0, 0);
         break;
         case 1: color = strip.Color(127, 40, 0);
         break;
         case 2: color = strip.Color(0, 127, 0);
         break;
      }
      
      // Now light up each arrow in turn
      for (byte arrow = 1; arrow <= 4; arrow++)
      {
         for (int pixel = 0; pixel < arrow * ARROW_LENGTH; pixel++)
         {
            strip.setPixelColor(ARROWS_START + pixel, color);
         }
         strip.show();
         if (true == timedWait(100))
         {
            return;
         }
      }
   }
   
   if (true == timedWait(1000))
   {
      return;
   }

   for (int j = 0; j < 384 * 5; j++)
   {     // 5 cycles of all 384 colors in the wheel
      for (int i = 0; i < ARROW_STRIP_LENGTH; i++)
      {
         // tricky math! we use each pixel as a fraction of the full 384-color wheel
         // (thats the i / strip.numPixels() part)
         // Then add in j which makes the colors go around per pixel
         // the % 384 is to make the wheel cycle around
         strip.setPixelColor(ARROWS_START + i, Wheel( ((i * 384 / ARROW_STRIP_LENGTH) + j) % 384) );
      }
      strip.show();   // write all the pixels out
      if (true == timedWait(0))
      {
         return;
      }
   }
}

// Runs a rainbow cycle through only the arrows
void arrowRainbow()
{
   for (int j = 0; j < 384 * 5; j++)
   {     
      // 5 cycles of all 384 colors in the wheel
      setArrow1Colour(Wheel( ((0 * 384 / ARROW_STRIP_LENGTH) + j) % 384) );
      setArrow2Colour(Wheel( ((1 * 384 / ARROW_STRIP_LENGTH) + j) % 384) );
      setArrow3Colour(Wheel( ((2 * 384 / ARROW_STRIP_LENGTH) + j) % 384) );
      setArrow4Colour(Wheel( ((3 * 384 / ARROW_STRIP_LENGTH) + j) % 384) );
         
      strip.show();
      if (true == timedWait(20))
      {
         return;
      }
   }

}

void setArrow1Colour(byte red, byte green, byte blue)
{
   setArrow1Colour(strip.Color(red, green, blue));
}

void setArrow2Colour(byte red, byte green, byte blue)
{
   setArrow2Colour(strip.Color(red, green, blue));
}

void setArrow3Colour(byte red, byte green, byte blue)
{
   setArrow3Colour(strip.Color(red, green, blue));
}

void setArrow4Colour(byte red, byte green, byte blue)
{
   setArrow4Colour(strip.Color(red, green, blue));
}

void setArrow1Colour(unsigned long color)
{
   for (byte i = 0; i < ARROW_LENGTH; i++)
   {
      strip.setPixelColor(ARROW_1_START + i, color);
   }
}

void setArrow2Colour(unsigned long color)
{
   for (byte i = 0; i < ARROW_LENGTH; i++)
   {
      strip.setPixelColor(ARROW_2_START + i, color);
   }
}

void setArrow3Colour(unsigned long color)
{
   for (byte i = 0; i < ARROW_LENGTH; i++)
   {
      strip.setPixelColor(ARROW_3_START + i, color);
   }
}

void setArrow4Colour(unsigned long color)
{
   for (byte i = 0; i < ARROW_LENGTH; i++)
   {
      strip.setPixelColor(ARROW_4_START + i, color);
   }
}

void initShootingTrailPattern(byte red, byte green, byte blue)
{
   byte dimRed = red / TRAIL_LENGTH;
   byte dimGreen = green / TRAIL_LENGTH;
   byte dimBlue = blue / TRAIL_LENGTH;

   // Set up the trail pattern
   trailPattern[0] = 0;
   for (unsigned int i = 1; i <= TRAIL_LENGTH; i++)
   {
      int index = TRAIL_LENGTH + 1 - i;
      trailPattern[index] = strip.Color(max(red - (dimRed * (i - 1)), 0), max(green - (dimGreen * (i - 1)), 0), max(blue - (dimBlue * (i - 1)), 0));
   }

}

void shoot(unsigned int shotSpeed, unsigned int waitAfter)
{
   int currentPixel, lastStart = 0;
   unsigned int h;
   
   for(h=0; h < STRIP_LENGTH; h++)
   {
      strip.setPixelColor(h, 0);
   }

   // Fill in colours
   for (unsigned int i = SHOOT_TRAIL_START; i <= SHOOT_TRAIL_END; i++)
   {
      lastStart = i - TRAIL_LENGTH;

      for (unsigned int j = 0; j <= TRAIL_LENGTH; j++)
      {
         currentPixel = lastStart + j;
         
         if (currentPixel < 0)
         {
            // Work out position at end
            currentPixel = SHOOT_TRAIL_END + 1 + currentPixel;  // subtracts from length to get index
         }
         if (currentPixel >= SHOOT_TRAIL_START)
         {
            strip.setPixelColor(currentPixel, trailPattern[j]);
            strip.setPixelColor(STRIP_LENGTH - 1 - currentPixel, trailPattern[j]);
         }
      }

      if ((i - SHOOT_TRAIL_START) >= ARROW_TRIGGER_1)
      {
         setArrow1Colour(127, 127, 0);
      }
      if ((i - SHOOT_TRAIL_START) >= ARROW_TRIGGER_2)
      {
         setArrow2Colour(127, 127, 0);
      }
      if ((i - SHOOT_TRAIL_START) >= ARROW_TRIGGER_3)
      {
         setArrow3Colour(127, 127, 0);
      }
      if ((i - SHOOT_TRAIL_START) >= ARROW_TRIGGER_4)
      {
         setArrow4Colour(127, 127, 0);
      }

      strip.show();
      if (true == timedWait(shotSpeed))
      {
         return;
      }
   }
   
   // Clean out the strip after a shot
   for(h=0; h < STRIP_LENGTH; h++)
   {
      strip.setPixelColor(h, 0);
   }
   
   if (true == timedWait(waitAfter))
   {
      return;
   }
   
   setDrivingState();
}


void twinkle(byte times, byte numLit)
{
   int pixels[STRIP_LENGTH] = {0};

   for (byte i = 0; i < times; i++)
   {
      for (byte i = 0; i < numLit; i++)
      {
         pixels[random(STRIP_LENGTH)] = 1;
      }

      for (unsigned int i=0; i < strip.numPixels(); i++)
      {
         if (pixels[i])
         {
            strip.setPixelColor(i, strip.Color(127, 127, 127));
         }
         else
         {
            strip.setPixelColor(i, strip.Color(0, 0, 0));
         }
      }  
      strip.show();
      if (true == timedWait(50))
      {
         return;
      }
   }
}

void twinkle(byte times, byte numLit, byte bgred, byte bggreen, byte bgblue, byte fgred, byte fggreen, byte fgblue, unsigned int wait)
{
   int pixels[STRIP_LENGTH] = {0};
   
   for (byte i = 0; i < times; i++)
   {
      for (byte i = 0; i < numLit; i++)
      {
         pixels[random(STRIP_LENGTH)] = 1;
      }

      for (unsigned int i=0; i < strip.numPixels(); i++)
      {
         if (pixels[i])
         {
            strip.setPixelColor(i, strip.Color(fgred, fggreen, fgblue));
         }
         else
         {
            strip.setPixelColor(i, strip.Color(bgred, bggreen, bgblue));
         }
      }  
      strip.show();
      if (true == timedWait(wait))
      {
         return;
      }
   }
}


void colorFlowDownShimmer(byte red, byte green, byte blue)
{
   blankStrip();
   
   int height;
//   int pixels[4];
//   int shimmerRow1[4];
//   int shimmerRow2[4];
   
   for (int count = HALF_STRIP_LENGTH; count >= 0; count--)
   {
      for (height = count; height <= HALF_STRIP_LENGTH; height++)
      {
         // Loop through pixels returned
         strip.setPixelColor(height, strip.Color(red, green, blue));
         strip.setPixelColor(STRIP_LENGTH - 1 - height, strip.Color(red, green, blue));

         strip.show();

         if (height == count || height == count + 1)
         {
   
            for (byte j = 1; j < 3; j++)
            {
               if (random(6) > 3)
               {
                  strip.setPixelColor(count, strip.Color(127, 127, 127));
                  strip.setPixelColor(STRIP_LENGTH - 1 - count, strip.Color(127, 127, 127));

                  strip.setPixelColor(count + 1, strip.Color(0, 0, 0));
                  strip.setPixelColor(STRIP_LENGTH - 1 - (count + 1), strip.Color(0, 0, 0));
               }
               else
               {
                  strip.setPixelColor(count, strip.Color(0, 0, 0));
                  strip.setPixelColor(STRIP_LENGTH - 1 - count, strip.Color(0, 0, 0));

                  strip.setPixelColor(count + 1, strip.Color(127, 127, 127));
                  strip.setPixelColor(STRIP_LENGTH - 1 - (count + 1), strip.Color(127, 127, 127));
               }            
            strip.show();
            }
         }
      }
   }
}
   

void colorFlowDown(byte red, byte green, byte blue)
{
   blankStrip();
 
   for (int count = HALF_STRIP_LENGTH; count >= 0; count--)
   {
      // Loop through pixels returned
      strip.setPixelColor(count, strip.Color(red, green, blue));
      strip.setPixelColor(STRIP_LENGTH - 1 - count, strip.Color(red, green, blue));

      strip.show();
      if (true == timedWait(20))
      {
         return;
      }
   }

}
   

void colorFill(byte red, byte green, byte blue)
{
   for (unsigned int i = 0; i < STRIP_LENGTH; i++)
   {
      strip.setPixelColor(i, strip.Color(red, green, blue));
   }
   
   strip.show();
}

// Credit to adafruit for this function. This is used in calculating a
// "color wheel" for the rainbow function
unsigned long Wheel(unsigned int WheelPos)
{
  byte r=0, g=0, b=0;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   // Red down
      g = WheelPos % 128;      // Green up
      b = 0;                  // Blue off
      break; 
    case 1:
      g = 127 - WheelPos % 128;  // Green down
      b = WheelPos % 128;      // Blue up
      r = 0;                  // Red off
      break; 
    case 2:
      b = 127 - WheelPos % 128;  // Blue down 
      r = WheelPos % 128;      // Red up
      g = 0;                  // Green off
      break; 
  }
  return(strip.Color(r,g,b));
}

// This gets called every time we receive new data over the I2C lines
// See I2Cdesign.md for a complete explanation of our utilization
void receiveData(int byteCount)
{
  // Check the byte count to ensure that we are recieving a 5 byte packet
  if (5 == byteCount)
  {
    // Strip off the last byte and read the value
    dataByte1 = (0x000000FF & Wire.read()); //Command Byte
    dataByte2 = (0x000000FF & Wire.read()); //Payload Data
    dataByte3 = (0x000000FF & Wire.read()); //Payload Data
    dataByte4 = (0x000000FF & Wire.read()); //Flipped Version of byte 2
    dataByte5 = (0x000000FF & Wire.read()); //Flipped version of byte 3
    
    // Check if the payload bytes and the flipped counterparts are indeed opposite using XOR
    // If so, then set the bytes we actually use in loop()
    if ((0xFF == (dataByte2 ^ dataByte4)) &&
        (0xFF == (dataByte3 ^ dataByte5)))
    {
//      Since data is being sent repeatedly to signify the start and end of a given state, this check
//      cannot be used. Uncomment these three lines and the closing parenthesis if you want to
//      check if the data is the same.
       
       // Check to see if the new data is the same as the old data after verifying that it is correct
//       if((commandByte != dataByte1) || (payloadByte1 != dataByte2) || (payloadByte2 != dataByte3))
//       {
       // Finally set the data to the variables we actually use in loop() 
       commandByte = dataByte1;
       payloadByte1 = dataByte2;
       payloadByte2 = dataByte3;
       
       // Set the flag to say that we have new data
       dataChanged = true;
//      }
    }    
  }
  // This should clear out any packets that are bigger than the required 5 bytes
  else if (byteCount > 5) 
  {
    // Keep on reading the bytes from the buffer until they are gone. They are simply not used and thus
    // will be thrown away.
    while (Wire.available() > 0)
    {
      Wire.read();
    }
  }
}

// Wait function (Specified Time)
// This is called when we wait to wait in between events that are occuring in our functions.
// Much better than using delay() because we can interrupt the parent function when new data is received.
boolean timedWait(unsigned int waitTime)
{
   unsigned long previousMillis = millis();
   unsigned long currentMillis = millis();

   for(previousMillis; (currentMillis - previousMillis) < waitTime; currentMillis = millis())
   {
      // This may appear to have to effect and the compiler will even throw a warning about it.
      // However, dataChanged is set even when in this loop by the receiveData() function
      if(dataChanged == true)
      {
         return true;
      }
   }
   return false;
}

// Wait function (infinite)
// This is called when we wait to wait in between events that are occuring in our functions.
// Much better than using delay() because we can interrupt the parent function when new data is received.
// We sit in this function until dataChanged becomes true.
boolean infiniteWaitFunction()
{
   while(dataChanged == false)
   {
      // Do nothing. We just sit in here until new data is recieved.
   }
   // If we break out of the while loop, then dataChanged must be true so we can return true
   return true;
}

// An experimental function which will scale a given RGB color up and down in a linear manner
void faderRGB(byte r, byte g, byte b, unsigned int wait)
{
   byte i = 0;
   unsigned int q = 0, p = 0, incrementR = 0, incrementG = 0, incrementB = 0;
   // This array stores both the general increment and the remainder in the format of
   // [inc (r), inc (g), inc (b), rem (r), rem (g), rem (b)]
   unsigned int colorIncrement[6];
   
   colorIncrement[0] = 127 / r;
   colorIncrement[1] = 127 / g;
   colorIncrement[2] = 127 / b;
   colorIncrement[3] = 127 % r;
   colorIncrement[4] = 127 % g;
   colorIncrement[5] = 127 % b;
   
   for (i=0; i <= 127; i++)
   {
      if (i % colorIncrement[0] == 0 && i < r)
      {
         incrementR++;
      }
      else if ((incrementR + colorIncrement[3]) - 127 >= 0 && i < r)
      {
         incrementR++;
      }
      if (i % colorIncrement[1] == 0 && i < g)
      {
         incrementG++;
      }
      else if ((incrementG + colorIncrement[4]) - 127 >= 0 && i < g)
      {
         incrementG++;
      }
      if (i % colorIncrement[2] == 0 && i < b)
      {
         incrementB++;
      }
      else if ((incrementB + colorIncrement[5]) - 127 >= 0 && i < b)
      {
         incrementB++;
      }
      for (p=0; p < strip.numPixels(); p++)
      {
         strip.setPixelColor(p, strip.Color(incrementR, incrementG, incrementB));
      }
      strip.show();

      if (true == timedWait(wait))
      {
         break;
      }
   }
   for (q=127; q >= 0; q--)
   {
      if (q % colorIncrement[0] == 0 && q > 0)
      {
         incrementR--;
      }
      else if ((incrementR + colorIncrement[3]) - 127 >= 0 && q > 0)
      {
         incrementR--;
      }
      if (q % colorIncrement[1] == 0 && q > 0)
      {
         incrementG--;
      }
      else if ((incrementG + colorIncrement[4]) - 127 >= 0 && q > 0)
      {
         incrementG--;
      }
      if (q % colorIncrement[2] == 0 && q > 0)
      {
         incrementB--;
      }
      else if ((incrementB + colorIncrement[5]) - 127 >= 0 && q > 0)
      {
         incrementB--;
      }
      for (p=0; p < strip.numPixels(); p++)
      {
         strip.setPixelColor(p, strip.Color(incrementR, incrementG, incrementB));
      }
      strip.show();

      if (true == timedWait(wait))
      {
         break;
      }
   }
}

// This is a simple fader that scales on and off (RED)
void faderRed(unsigned int wait)
{
   byte i = 0;
   unsigned int p, q;

   for (i=0; i <= 120; i+=5)
   {
      for (p=0; p < strip.numPixels(); p++)
      {
         strip.setPixelColor(p, strip.Color(i,0,0));
      }
      strip.show();

      if (true == timedWait(wait))
      {
         break;
      }
   }
   for (q=i; q >= 0; q-=5)
   {
      for (p=0; p < strip.numPixels(); p++)
      {
         strip.setPixelColor(p, strip.Color(q,0,0));
      }
      strip.show();

      if (true == timedWait(wait))
      {
         break;
      }
   }
}

// This is a simple fader that scales on and off (GREEN)
void faderGreen(unsigned int wait)
{
   byte i = 0;
   unsigned int p, q;

   for (i=0; i <= 120; i+=5)
   {
      for (p=0; p < strip.numPixels(); p++)
      {
         strip.setPixelColor(p, strip.Color(0,i,0));
      }
      strip.show();

      if (true == timedWait(wait))
      {
         break;
      }
   }
   for (q=i; q >= 0; q-=5)
   {
      for (p=0; p < strip.numPixels(); p++)
      {
         strip.setPixelColor(p, strip.Color(0,q,0));
      }
      strip.show();

      if (true == timedWait(wait))
      {
         break;
      }
   }
}

// This is a simple fader that scales on and off (BLUE)
void faderBlue (unsigned int wait)
{
   unsigned int i, p, q;
   for (i=0; i <= 120; i+=5)
   {
      for (p=0; p < strip.numPixels(); p++)
      {
         strip.setPixelColor(p, strip.Color(0,0,i));
      }
      strip.show();
      if (true == timedWait(wait))
      {
         return;
      }
   }
   for (q=i; q >= 0; q-=5)
   {
      for (p=0; p < strip.numPixels(); p++)
      {
         strip.setPixelColor(p, strip.Color(0,0,q));
      }
      strip.show();

      if (true == timedWait(wait))
      {
         return;
      }
   }
}

// Cycle through a rainbow of colors throughout the whole strip
void rainbowWheel(unsigned int wait)
{
  unsigned int i, j;

  for (j=0; j < 384 * 5; j++)
  {     // 5 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++)
    {
      // tricky math! we use each pixel as a fraction of the full 384-color
      // wheel (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 384 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel(((i * 384 / strip.numPixels()) + j) % 384));
    }
    strip.show();   // write all the pixels out
   
    if (true == timedWait(wait))
    {
      break;
    }
  }
}

// Send a small block of lights down the strip and optionally bounce them back
void scanner(byte r, byte g, byte b, unsigned int wait, boolean bounce)
{
   unsigned int h = 0, i = 0;
   char j = 0;
   int pos = 0, dir = 1;

   // Erase the strip initially to be sure that we do not leave
   // LEDs on from previous functions
   for (h=0; h < strip.numPixels(); h++)
   {
      strip.setPixelColor(h, 0);
   }

   for (i=0; i<((strip.numPixels()-1) * 8); i++)
   {
      // Draw 5 pixels centered on pos.  setPixelColor() will clip
      // any pixels off the ends of the strip, no worries there.
      // we'll make the colors dimmer at the edges for a nice pulse
      // look
      strip.setPixelColor(pos - 2, strip.Color(r/4, g/4, b/4));
      strip.setPixelColor(pos - 1, strip.Color(r/2, g/2, b/2));
      strip.setPixelColor(pos, strip.Color(r, g, b));
      strip.setPixelColor(pos + 1, strip.Color(r/2, g/2, b/2));
      strip.setPixelColor(pos + 2, strip.Color(r/4, g/4, b/4));

      strip.show();
      // Wait function with interrupt
      if (true == timedWait(wait))
      {
         break;
      }

      // If we wanted to be sneaky we could erase just the tail end
      // pixel, but it's much easier just to erase the whole thing
      // and draw a new one next time.
      for (j=-2; j<= 2; j++)
      {
         strip.setPixelColor(pos+j, strip.Color(0,0,0));
      }
      // Bounce off ends of strip
      pos += dir;
      if (pos < 0)
      {
         pos = 1;
         dir = -dir;
      }
      else if (pos >= strip.numPixels())
      {
         if (bounce == true)
         {
            pos = strip.numPixels() - 2;
            dir = -dir;
         }
         else
         {
            pos = 0;
         } 
      }
   }
}

// This blinks all the LEDs to the rgb values passed in. Time can be changed for on and off with onWait and offWait when calling.
void colorBlink(unsigned long onWait, unsigned long offWait, byte r, byte g, byte b)
{
  unsigned int i;
  // First Pulse
  for (i=0; i < strip.numPixels(); i++)
  {

     strip.setPixelColor(i, strip.Color(r,g,b));
  }
  strip.show();
  
  if (true == timedWait(onWait))
  {
     return;
  }

  for (i=0; i < strip.numPixels(); i++)
  {
     strip.setPixelColor(i, 0);
  }
  strip.show();
  // Wait function with interrupt
  if (true == timedWait(offWait))
  {
     return;  
  }
}

// This is a unique function which is used only when the robot first boots up. It allows for us to easily see
// when 8 seconds has passed
void gyroCalibrate(byte flashes, int blinkTime, int blinkTime2, byte red, byte green, byte blue)
{
   unsigned int pixel, h;
   byte q;
   // Erase the strip initially to be sure that we do not leave
   // LEDs on from previous functions
   for(h=0; h < strip.numPixels(); h++)
   {
      strip.setPixelColor(h, 0);
   }
   strip.show();
   if (true == timedWait(2000))
   {
      return;
   }
   strip.setPixelColor(0, strip.Color(127, 0, 0));
   strip.setPixelColor(1, strip.Color(127, 0, 0));
   strip.setPixelColor(83, strip.Color(127, 0, 0));
   strip.setPixelColor(82, strip.Color(127, 0, 0));
   for (pixel = 0; pixel < 1 * ARROW_LENGTH; pixel++)
   {
      strip.setPixelColor(ARROWS_START + pixel, strip.Color(red, green, blue));
   }
   strip.show();
   if (true == timedWait(1000))
   {
      return;
   }
   strip.setPixelColor(2, strip.Color(127, 127, 0));
   strip.setPixelColor(3, strip.Color(127, 127, 0));
   strip.setPixelColor(81, strip.Color(127, 127, 0));
   strip.setPixelColor(80, strip.Color(127, 127, 0));
   strip.show();
   if (true == timedWait(1000))
   {
      return;
   }
   strip.setPixelColor(4, strip.Color(0, 127, 0));
   strip.setPixelColor(5, strip.Color(0, 127, 0));
   strip.setPixelColor(79, strip.Color(0, 127, 0));
   strip.setPixelColor(78, strip.Color(0, 127, 0));
   for (pixel = 0; pixel < 2 * ARROW_LENGTH; pixel++)
   {
      strip.setPixelColor(ARROWS_START + pixel, strip.Color(red, green, blue));
   }
   strip.show();
   if (true == timedWait(1000))
   {
      return;
   }
   strip.setPixelColor(6, strip.Color(0, 127, 127));
   strip.setPixelColor(7, strip.Color(0, 127, 127));
   strip.setPixelColor(77, strip.Color(0, 127, 127));
   strip.setPixelColor(76, strip.Color(0, 127, 127));
   strip.show();
   if (true == timedWait(1000))
   {
      return;
   }
   strip.setPixelColor(8, strip.Color(0, 0, 127));
   strip.setPixelColor(9, strip.Color(0, 0, 127));
   strip.setPixelColor(75, strip.Color(0, 0, 127));
   strip.setPixelColor(74, strip.Color(0, 0, 127));
   for (pixel = 0; pixel < 3 * ARROW_LENGTH; pixel++)
   {
      strip.setPixelColor(ARROWS_START + pixel, strip.Color(red, green, blue));
   }
   strip.show();
   if (true == timedWait(1000))
   {
      return;
   }
   strip.setPixelColor(10, strip.Color(127, 0, 127));
   strip.setPixelColor(11, strip.Color(127, 0, 127));
   strip.setPixelColor(73, strip.Color(127, 0, 127));
   strip.setPixelColor(72, strip.Color(127, 0, 127));
   strip.show();
   if (true == timedWait(1000))
   {
      return;
   }
   strip.setPixelColor(12, strip.Color(127, 127, 127));
   strip.setPixelColor(13, strip.Color(127, 127, 127));
   strip.setPixelColor(71, strip.Color(127, 127, 127));
   strip.setPixelColor(70, strip.Color(127, 127, 127));
   for (pixel = 0; pixel < 4 * ARROW_LENGTH; pixel++)
   {
      strip.setPixelColor(ARROWS_START + pixel, strip.Color(red, green, blue));
   }
   strip.show();
   if (true == timedWait(1000))
   {
      return;
   }
   for (byte i = 0; i < flashes; i++)
   {
      for (q = 0; q < 14; q++)
      {
         strip.setPixelColor(q, strip.Color(red, green, blue));
      }
      for (q = 83; q > 69; q--)
      {
         strip.setPixelColor(q, strip.Color(red, green, blue));
      }
      for (pixel = 0; pixel < 4 * ARROW_LENGTH; pixel++)
      {
         strip.setPixelColor(ARROWS_START + pixel, strip.Color(red, green, blue));
      }
      strip.show();
      if (true == timedWait(blinkTime))
      {
         return;
      }
      for (q = 0; q < 14; q++)
      {
         strip.setPixelColor(q, 0);
      }
      for (q = 83; q > 69; q--)
      {
         strip.setPixelColor(q, 0);
      }
      for (pixel = 0; pixel < 4 * ARROW_LENGTH; pixel++)
      {
         strip.setPixelColor(ARROWS_START + pixel, 0);
      }
      strip.show();
      if (true == timedWait(blinkTime2))
      {
         return;
      }
   }
}

// Simply alternates the pixels that are lit up over a given wait period
void alternatingColor(byte red1, byte green1, byte blue1, byte red2, byte green2, byte blue2, unsigned int wait1, unsigned int wait2, unsigned int times)
{
   unsigned int h, i;
   for (h=0; h < strip.numPixels(); h++)
   {
      strip.setPixelColor(h, 0);
   }
   for (i=0; i < times; i++)
   {
      for (h=0; h < strip.numPixels(); h++)
      {
        strip.setPixelColor(h, 0);
      }
      for (h=0; h < strip.numPixels(); h=h+2)
      {
        strip.setPixelColor(h, strip.Color(red1, green1, blue1));
      }
      strip.show();
      if (true == timedWait(wait1))
      {
         return;
      }
      for (h=0; h < strip.numPixels(); h++)
      {
        strip.setPixelColor(h, 0);
      }
      for (h=1; h < strip.numPixels(); h=h+2)
      {
        strip.setPixelColor(h, strip.Color(red2, green2, blue2));
      }
      strip.show();
      if (true == timedWait(wait2))
      {
         return;
      }
   }
}

// This is called when we initially receive what alliance we are on along with what station we are
// The robot will fade the number of times which corresponds to the station
void allianceSelection(byte times)
{
   byte p, q = 0;
   if (payloadByte2 > 3)
   {
      return;
   }
   if (payloadByte1 == 0x52)
   {
      alliance = 1;
      for (q=0; q < times; q++)
      {
         for (p=0; p < payloadByte2; p++)
         {
            faderRed(30);
         }
         if (true == timedWait(600))
         {
            return;
         }
      }
   }
   else if (payloadByte1 == 0x47)
   {
      alliance = 2;
      for (q=0; q < times; q++)
      {
         for (p=0; p < payloadByte2; p++)
         {
            faderBlue(30);
         }
         if (true == timedWait(600))
         {
            return;
         }
      }
   }
   setDrivingState();
}

void autonomous()
{
   for (int i=0; i < 5; i++)
   {
      scanner(0, 0, 127, 20, true);
   }
   
   alternatingColor(0, 0, 127, 0, 0, 127, 150, 150, 20);
}

void climb(unsigned int extensionTime, unsigned int retractionTime, unsigned int flashes, unsigned int blinkTime, unsigned int blinkTime2, unsigned int holdTime)
{
   unsigned int pixel, h;
   byte q;
   
   if (climbOut == false)
   {
      climbOut = true; 
      // Erase the strip initially to be sure that we do not leave
      // LEDs on from previous functions
      for (h=0; h < strip.numPixels(); h++)
      {
        strip.setPixelColor(h, 0);
      }
      strip.show();
      for (byte i=0; i < 14; i++)
      {
         strip.setPixelColor(i, allianceColor());
         strip.setPixelColor(STRIP_LENGTH - i, allianceColor());
         
         // Set the arrows at the periodic intervals
         if (i==1 || i==5 || i==9 || i==13)
         {
            switch (i)
            {
               case 1:
                  for (pixel = 0; pixel < 1 * ARROW_LENGTH; pixel++)
                  {
                     strip.setPixelColor(ARROWS_START + pixel, allianceColor());
                  }
                  break;
               case 5:
                  for (pixel = 0; pixel < 2 * ARROW_LENGTH; pixel++)
                  {
                     strip.setPixelColor(ARROWS_START + pixel, allianceColor());
                  }
                  break;
               case 9:
                  for (pixel = 0; pixel < 3 * ARROW_LENGTH; pixel++)
                  {
                     strip.setPixelColor(ARROWS_START + pixel, allianceColor());
                  }
                  break;
               case 13:
                  for (pixel = 0; pixel < 4 * ARROW_LENGTH; pixel++)
                  {
                     strip.setPixelColor(ARROWS_START + pixel, allianceColor());
                  }
                  break;
            }
         }
         strip.show();
         if (true == timedWait(extensionTime))
         {
            return;
         }
      }
      if (true == timedWait(30000))
      {
         return;
      }
   }
   else
   {
      climbOut = false;
      for (byte i=13; i > 0; i--)
      {
         strip.setPixelColor(i, strip.Color(127, 0, 127));
         strip.setPixelColor(STRIP_LENGTH - i, strip.Color(127, 0, 127));
         
         //Set the arrows at the periodic intervals
         if (i==1 || i==5 || i==9 || i==13)
         {
            switch (i)
            {
               case 1:
                  for (pixel = 0; pixel < 1 * ARROW_LENGTH; pixel++)
                  {
                     strip.setPixelColor(ARROWS_START + pixel, strip.Color(127, 0, 127));
                  }
                  break;
               case 5:
                  for (pixel = 0; pixel < 2 * ARROW_LENGTH; pixel++)
                  {
                     strip.setPixelColor(ARROWS_START + pixel, strip.Color(127, 0, 127));
                  }
                  break;
               case 9:
                  for (pixel = 0; pixel < 3 * ARROW_LENGTH; pixel++)
                  {
                     strip.setPixelColor(ARROWS_START + pixel, strip.Color(127, 0, 127));
                  }
                  break;
               case 13:
                  for (pixel = 0; pixel < 4 * ARROW_LENGTH; pixel++)
                  {
                     strip.setPixelColor(ARROWS_START + pixel, strip.Color(127, 0, 127));
                  }
                  break;
            }
         }
         strip.show();
         if (true == timedWait(retractionTime))
         {
            return;
         }
      }
      if (true == timedWait(holdTime))
      {
         return;
      }
      for (byte i = 0; i < flashes; i++)
      {
         for (q = 0; q < 14; q++)
         {
            strip.setPixelColor(q, allianceColor());
         }
         for (q = 83; q > 69; q--)
         {
            strip.setPixelColor(q, allianceColor());
         }
         for (pixel = 0; pixel < 4 * ARROW_LENGTH; pixel++)
         {
            strip.setPixelColor(ARROWS_START + pixel, allianceColor());
         }
         strip.show();
         if (true == timedWait(blinkTime))
         {
            return;
         }
         for (q = 0; q < 14; q++)
         {
            strip.setPixelColor(q, 0);
         }
         for (q = 83; q > 69; q--)
         {
            strip.setPixelColor(q, 0);
         }
         for (pixel = 0; pixel < 4 * ARROW_LENGTH; pixel++)
         {
            strip.setPixelColor(ARROWS_START + pixel, 0);
         }
         strip.show();
         if (true == timedWait(blinkTime2))
         {
            return;
         }
      }
      if (true == timedWait(3000))
      {
         return;
      }
      setDrivingState();
   }
}

/******************************************* THIS NEEDS WORK!!!!! ***************************************/
void intake(unsigned int feedSpeed, unsigned int waitAfter)
{
   int currentPixel;
   unsigned int h;
   
   
//   blankRange(SHOOT_TRAIL_START, STRIP_LENGTH - SHOOT_TRAIL_START);

   for (h=0; h < STRIP_LENGTH; h++)
   {
      strip.setPixelColor(h, 0);
   }

      // reset colour array
//   for (int i = SHOOT_TRAIL_START; i <= SHOOT_TRAIL_END; i++)
//   {
//      pixels[i] = 0;
//   }

   int lastStart = 0;
   
   // Fill in colours
   for (int i = SHOOT_TRAIL_END; i >= SHOOT_TRAIL_START; i--)
   {
      lastStart = i + TRAIL_LENGTH;

      for (unsigned int j = 0; j <= TRAIL_LENGTH; j++)
      {
         currentPixel = lastStart - j;
         
//         if (currentPixel < 0)
//         {
//            // Work out position at end
//            currentPixel = SHOOT_TRAIL_END + 1 + currentPixel;  // subtracts from length to get index
//         }
         if (currentPixel >= SHOOT_TRAIL_START)
         {
            strip.setPixelColor(currentPixel, trailPattern[j]);
            strip.setPixelColor(STRIP_LENGTH - 1 - currentPixel, trailPattern[j]); //CHECK THIS
         }
      }

      if ((i - SHOOT_TRAIL_START) <= ARROW_TRIGGER_1)
      {
         setArrow1Colour(127, 0, 127);
      }
      if ((i - SHOOT_TRAIL_START) <= ARROW_TRIGGER_2)
      {
         setArrow2Colour(127, 0, 127);
      }
      if ((i - SHOOT_TRAIL_START) <= ARROW_TRIGGER_3)
      {
         setArrow3Colour(127, 0, 127);
      }
      if ((i - SHOOT_TRAIL_START) <= ARROW_TRIGGER_4)
      {
         setArrow4Colour(127, 0, 127);
      }

      strip.show();
      if (true == timedWait(feedSpeed))
      {
         return;
      }
   }
//   // Clean out the strip after a shot
//   blankRange(SHOOT_TRAIL_START, STRIP_LENGTH - SHOOT_TRAIL_START);
//   setArrow1Colour(0, 0, 0);
//   setArrow2Colour(0, 0, 0);
//   setArrow3Colour(0, 0, 0);
//   setArrow4Colour(0, 0, 0);

   for (h=0; h < STRIP_LENGTH; h++)
   {
      strip.setPixelColor(h, 0);
   }
   
   if (true == timedWait(waitAfter))
   {
      return;
   }
}

void setDrivingState()
{
   dataChanged = true;
   commandByte = 0x08;
   payloadByte1 = 0x34;
   payloadByte2 = 0x45;
}

unsigned long allianceColor()
{
   if (alliance == 1)
   {
      return strip.Color(127, 0, 0);
   }
   else if (alliance == 2)
   {
      return strip.Color(0, 0, 127);
   }
   else
   {
      return strip.Color(0, 127, 0);
   }
}
