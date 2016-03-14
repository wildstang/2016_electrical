
#include <Adafruit_DotStar.h>
#include <SPI.h>
#include <Wire.h>
#define TWI_FREQ 38400L

#define NUMPIXELS 98
#define DATAPIN    4
#define CLOCKPIN   6
Adafruit_DotStar strip = Adafruit_DotStar(
                           NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

#define ALLIANCE_RED 0x04
#define ALLIANCE_BLUE 0x47
#define AUTONOMOUS 0x02
#define SHOOT 0x03
#define INTAKE 0x11
#define DISABLED 0x33
#define ANTITURBO 0x06
#define TURBO 0x05

// This the address to use for the arduino in slave mode when talking to the cRIO.
// This number may differ from the address that the cRIO expects. Sniff the I2C lines
// to check what address correlates to the actual integer given here
static const byte i2cAddress = 0x10;

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
boolean firstRun = false;

void setup()
{
  // This reads the noise off of Analog input 2 and seeds the random() function
  randomSeed(analogRead(2));
  Serial.begin(9600);

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1);
#endif
  Serial.println("setup");
  strip.begin();
  strip.show();

  // Begin I2C communications as a SLAVE. receiveData() will be called when new data arrives.
  // We call this last to avoid a nasty bug involving the LED initialization code
  Wire.begin(i2cAddress);
  Wire.onReceive(receiveData);
}


void receiveData(int byteCount)
{
  Serial.println("got data");
  // Check the byte count to ensure that we are recieving a 5 byte packet
  if (5 == byteCount)
  {
    Serial.print("byte");
    // Strip off the last byte and read the value
    dataByte1 = (0x000000FF & Wire.read()); //Command Byte
    dataByte2 = (0x000000FF & Wire.read()); //Payload Data
    dataByte3 = (0x000000FF & Wire.read()); //Payload Data
    dataByte4 = (0x000000FF & Wire.read()); //Flipped Version of byte 2
    dataByte5 = (0x000000FF & Wire.read()); //Flipped version of byte 3

    // Check if the payload bytes and the flipped counterparts are indeed opposite using XOR
    // If so, then set the bytes we actually use in loop()
    // if ((0xFF == (dataByte2 ^ dataByte4)) &&
    //    (0xFF == (dataByte3 ^ dataByte5)))
    // {
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

  for (previousMillis; (currentMillis - previousMillis) < waitTime; currentMillis = millis())
  {
    // This may appear to have to effect and the compiler will even throw a warning about it.
    // However, dataChanged is set even when in this loop by the receiveData() function
    if (dataChanged == true)
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
  while (dataChanged == false)
  {
    // Do nothing. We just sit in here until new data is recieved.
  }
  // If we break out of the while loop, then dataChanged must be true so we can return true
  return true;
}

unsigned long Wheel(unsigned int WheelPos)
{
  byte r = 0, g = 0, b = 0;
  switch (WheelPos / 128)
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
  return (strip.Color(r, g, b));
}






//strip.setPixelColor(1, strip.Color(0, 255, 0));
uint32_t off = 0x000000;
uint32_t orange = 0x00FF80;
uint32_t pink = 0x80FF00;
uint32_t blue = 0xFF0000;
uint32_t yellow = 0x00FFFF;
uint32_t red = 0x00FF00;
uint32_t green = 0x0000FF;
uint32_t white = 0xFFFFFF;
uint32_t purple = 0xFFFF00;
int lead = 0, trail = 4, intakeLead = 11, intakeTrail = 7;

int rowOne[7] = {0, 27, 28, 55, 56, 83, 84};
int rowTwo[7] = {1, 26, 29, 54, 57, 82, 85};
int rowThree[7] = {2, 25, 30, 53, 58, 81, 86};
int rowFour[7] = {3, 24, 31, 52, 59, 80, 87};
int rowFive[7] = {4, 23, 32, 51, 60, 79, 88};
int rowSix[7] = {5, 22, 33, 50, 61, 78, 89};
int rowSeven[7] = {6, 21, 34, 49, 62, 77, 90};
int rowEight[7] = {7, 20, 35, 48, 63, 76, 91};
int rowNine[7] = {8, 19, 36, 47, 64, 75, 92};
int rowTen[7] = {9, 18, 37, 46, 65, 74, 93};
int rowEleven[7] = {10, 17, 38, 45, 66, 73, 94};
int rowTwelve[7] = {11, 16, 39, 44, 67, 72, 95};
int rowThirteen[7] = {12, 15, 40, 43, 68, 71, 96};
int rowFourteen[7] = {13, 14, 41, 42, 69, 70, 97};


void loop() {
  if ((commandByte == ALLIANCE_BLUE) &&
      (payloadByte1 == 0x34) &&
      (payloadByte2 == 0x26))
  { //blue alliance
    for (lead = 0; lead <= NUMPIXELS; lead++) {
      strip.setPixelColor(lead, blue);
      strip.show();
    }
  }

  else if ((commandByte == ALLIANCE_RED) &&
           (payloadByte1 == 0x52) &&
           (payloadByte2 == 0x01))
  { //red alliance
    for (lead = 0; lead <= NUMPIXELS; lead++) {
      strip.setPixelColor(lead, red);
      strip.show();
    }
  }

  else if ((commandByte == SHOOT) &&
           (payloadByte1 == 0x21) &&
           (payloadByte2 == 0x12)) {
    /*strip.setPixelColor(lead, white); //shooting
    strip.setPixelColor(trail, pink);
    strip.show();
    delay(75);

    if (++lead >= NUMPIXELS) {
      lead = 0;
    }
    if (++trail >= NUMPIXELS) {
      trail = 0;
    } */
    /*for (lead = 0; lead <= NUMPIXELS; lead++) {
      strip.setPixelColor(lead, orange);
      strip.show();*/
      for(int i; i < 15; i++){
        setRowColor(i, pink);
        delay(1000);
      }
    }
 


  else if ((commandByte == DISABLED) &&
           (payloadByte1 == 0x55) &&
           (payloadByte2 == 0x59))
  {
    for (lead = 0; lead <= NUMPIXELS; lead++) {
      strip.setPixelColor(lead, red);
      strip.show();
    }
    delay(1000);
    for (lead = 0; lead <= NUMPIXELS; lead++) {
      strip.setPixelColor(lead, off);
      strip.show();
    }
    delay(1000);
  }


  else if ((commandByte == INTAKE) &&
           (payloadByte1 == 0x57) &&
           (payloadByte2 == 0x49))
  {
    /*strip.setPixelColor(lead, yellow); //intake
    strip.setPixelColor(trail, purple);
    strip.show();
    delay(75);

    if (intakeLead-- == 0) {
      lead = NUMPIXELS;
    }
    if (intakeTrail-- == 0) {
      trail = NUMPIXELS; */
    for (lead = 0; lead <= NUMPIXELS; lead++) {
      strip.setPixelColor(lead, purple);
      strip.show();
    }
  }

  else if ((commandByte == AUTONOMOUS) &&
           (payloadByte1 == 0x13) &&
           (payloadByte2 == 0x14))
  {
    unsigned int i, j;

    for (j = 0; j < 384 * 5; j++)
    { // 5 cycles of all 384 colors in the wheel
      for (i = 0; i < strip.numPixels(); i++)
      {
        // tricky math! we use each pixel as a fraction of the full 384-color
        // wheel (thats the i / strip.numPixels() part)
        // Then add in j which makes the colors go around per pixel
        // the % 384 is to make the wheel cycle around
        strip.setPixelColor(i, Wheel(((i * 384 / strip.numPixels()) + j) % 384));
      }
      strip.show();   // write all the pixels out
      if (true)
      {
        break;
      }
    }
  }

  else if ((commandByte == ANTITURBO) &&
           (payloadByte1 == 0x09) &&
           (payloadByte2 == 0x08))
  {
    for (lead = 0; lead <= NUMPIXELS; lead++) {
      strip.setPixelColor(lead, yellow);
      strip.show();
    }
  }

  else if ((commandByte == TURBO) &&
           (payloadByte1 == 0x20) &&
           (payloadByte2 == 0x32))
  {
    for (lead = 0; lead <= NUMPIXELS; lead++) {
      strip.setPixelColor(lead, green);
      strip.show();
    }
  }

  else
  {
    for (lead = 0; lead <= NUMPIXELS; lead++) {
      strip.setPixelColor(lead, off);
      strip.show();
    }
  }
}

void setRowColor(int row, int color) {

  if (row == 1)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowOne[i], color);
      strip.show();
    }
  }

  if (row == 2)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowTwo[i], color);
      strip.show();
    }
  }

  if (row == 3)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowThree[i], color);
      strip.show();
    }
  }

  if (row == 4)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowFour[i], color);
      strip.show();
    }
  }

  if (row == 5)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowFive[i], color);
      strip.show();
    }
  }

  if (row == 6)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowSix[i], color);
      strip.show();
    }
  }

  if (row == 7)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowSeven[i], color);
      strip.show();
    }
  }

  if (row == 8)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowEight[i], color);
      strip.show();
    }
  }

  if (row == 9)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowNine[i], color);
      strip.show();
    }
  }

  if (row == 10)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowTen[i], color);
      strip.show();
    }
  }

  if (row == 11)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowEleven[i], color);
      strip.show();
    }
  }

  if (row == 12)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowTwelve[i], color);
      strip.show();
    }
  }

  if (row == 13)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowThirteen[i], color);
      strip.show();
    }
  }

  if (row == 14)
  {
    for (int i = 0; i < 6; i++)
    {
      strip.setPixelColor(rowFourteen[i], color);
      strip.show();
    }
  }

}
