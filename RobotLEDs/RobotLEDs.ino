#include <Adafruit_DotStar.h>

/*
Wildstang Lights 2015
This is a change.
*/

// Since the cRIO operates at 38.4kHz for its I2C clock, we need to set this manually here.
// This accesses the low level TWI library files found in /Wire/utility/
// This needs to be defined before we include as there is a check for a define in twi.c
//#define TWI_FREQ 38400L

#include "SPI.h"
#include <Wire.h>
#include "definitions.h"

// Uncomment the following line if you want to enable debugging messages over serial
//#define WS_DEBUG

// This will use the following pins (SPI):
// Data (SDA):  11
// Clock (SCL): 13
#define NUMPIXELS 30

Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, 11, 13);

unsigned long trailPattern[TRAIL_LENGTH + 1];

#define ALLIANCE_RED 0x52
#define ALLIANCE_BLUE 0x47
#define ALLIANCE_PATTERN 0x04
#define AUTONOMOUS 0x02

// This the address to use for the arduino in slave mode when talking to the cRIO.
// This number may differ from the address that the cRIO expects. Sniff the I2C lines
// to check what address correlates to the actual integer given here
static const byte i2cAddress = 0x12;

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

   if ((commandByte == AUTONOMOUS) &&
     (payloadByte1 == 0x2F) &&
     (payloadByte2 == 0x12))
   {
      dataChanged = false;
      autonomous();
   }
   else if (commandByte == ALLIANCE_PATTERN)
   {
     if (payloadByte1 == ALLIANCE_RED)
     {
      dataChanged = false;
      allianceSelection(6);
     }
     else if (payloadByte1 == ALLIANCE_BLUE)
     {
      dataChanged = false;
      allianceSelection(6);
     }
   }
   else if ((commandByte == 0x08) &&
     (payloadByte1 == 0x34) &&
     (payloadByte2 == 0x45))
   {
      dataChanged = false;
      if (alliance == 1)
      {
         scanner(255, 0, 0, 20, true); //Hi
      }
      else if (alliance == 2)
      {
         scanner(0, 0, 255, 20, true);
      }
      rainbowWheel(3);
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


// This is called when we initially receive what alliance we are on along with what station we are
// The robot will fade the number of times which corresponds to the station
void allianceSelection(byte times)
{
   byte p, q = 0;
   if (payloadByte2 > 3)
   {
      return;
   }
   if (payloadByte1 == ALLIANCE_RED)
   {
      alliance = 1;
      for (q=0; q < times; q++)
      {
         for (p=0; p < payloadByte2; p++)
         {
           colorBlink(50, 50, 255, 0, 0); 
           //faderRed(30);
         }
         if (true == timedWait(600))
         {
            return;
         }
      }
   }
   else if (payloadByte1 == ALLIANCE_BLUE)
   {
      alliance = 2;
      for (q=0; q < times; q++)
      {
         for (p=0; p < payloadByte2; p++)
         {
            colorBlink(50, 50, 0, 0, 255); 
            //faderBlue(30);
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
      scanner(255, 255, 0, 20, true);
   }
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


