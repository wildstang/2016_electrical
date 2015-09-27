#include "Wire.h"

  signed int RAxH, RAyH, RAzH, RHeadingH, RGxH, RGyH, RGzH, RAxL, RAyL, RAzL, RHeadingL, RGxL, RGyL, RGzL;
  int ExpectedBytes = 14;
  
  int number = 0;
  
  float RAx, RAy, RAz, RHeading, RGx, RGy, RGz;
  
void setup()
{
  Wire.begin();
  Serial.begin(9600);
}

void loop()
{
  ReadIMU();
  Serial.println(number);
  Serial.print("Accel X: "); Serial.print(RAx); Serial.print(" ");
  Serial.print("  \tY: "); Serial.print(RAy);       Serial.print(" ");
  Serial.print("  \tZ: "); Serial.print(RAz);     Serial.println("  \tm/s^2");

  // print out heading
  Serial.print("Heading = "); Serial.print(RHeading); Serial.println(" deg");
  
  // print out gyroscopic data
  Serial.print("Gyro  X: "); Serial.print(RGx); Serial.print(" ");
  Serial.print("  \tY: "); Serial.print(RGy);       Serial.print(" ");
  Serial.print("  \tZ: "); Serial.print(RGz);     Serial.println("  \tdps");

number++;
}

void ReadIMU()
{
  int x = 0;
  signed int imuReturn[14];
  
  Wire.requestFrom(0x6F, ExpectedBytes);
while (Wire.available())
{
  for (x; x < ExpectedBytes; x++)
  {
    imuReturn[x] = Wire.read();
  }
  
  RAxH = imuReturn[0] << 8;
  RAxL = imuReturn[1];
  RAyH = imuReturn[2] << 8;
  RAyL = imuReturn[3];
  RAzH = imuReturn[4] << 8;
  RAzL = imuReturn[5];
  RHeadingH = imuReturn[6] << 8;
  RHeadingL = imuReturn[7];
  RGxH = imuReturn[8] << 8;
  RGxL = imuReturn[9];
  RGyH = imuReturn[10] << 8;
  RGyL = imuReturn[11];
  RGzH = imuReturn[12] << 8;
  RGzL = imuReturn[13];
}
dataArrange();
}

void dataArrange()
{
  RAx = (RAxH + RAxL) / 10;
  RAy = (RAyH + RAyL) / 10;
  RAz = (RAzH + RAzL) / 10;
  RHeading = (RHeadingH + RHeadingL);
  RGx = (RGxH + RGxL) / 10;
  RGy = (RGyH + RGyL) / 10;
  RGz = (RGzH + RGzL) / 10;
}
