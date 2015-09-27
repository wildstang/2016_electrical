#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS0.h>
//#include <Adafruit_Simple_AHRS.h>
#include "Average.h"
#include "math.h"
/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.
   
   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.

   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example).
   
   Connections (For default I2C)
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 5V DC
   Connect GROUND to common ground

   History
   =======
   2014/JULY/25  - First version (KTOWN)
*/
   
/* Assign a unique base ID for this sensor */   
//Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0(1000);  // Use I2C, ID #1000

/* Or, use Hardware SPI:
  SCK -> SPI CLK
  SDA -> SPI MOSI
  G_SDO + XM_SDO -> tied together to SPI MISO
  then select any two pins for the two CS lines:
*/

#define LSM9DS0_XM_CS 10
#define LSM9DS0_GYRO_CS 9
//Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0(LSM9DS0_XM_CS, LSM9DS0_GYRO_CS, 1000);

/* Or, use Software SPI:
  G_SDO + XM_SDO -> tied together to the MISO pin!
  then select any pins for the SPI lines, and the two CS pins above
*/

#define LSM9DS0_SCLK 13
#define LSM9DS0_MISO 12
#define LSM9DS0_MOSI 11
//(SPI) 
Adafruit_LSM9DS0 lsm = Adafruit_LSM9DS0(LSM9DS0_SCLK, LSM9DS0_MISO, LSM9DS0_MOSI, LSM9DS0_XM_CS, LSM9DS0_GYRO_CS, 1000);

int number = 1;
int wait = 5;

const int CNT = 5;

float avgAX[CNT];
float avgAY[CNT];
float avgAZ[CNT];

float avgGX[CNT];
float avgGY[CNT];
float avgGZ[CNT];

float avgMX[CNT];
float avgMY[CNT];
float avgMZ[CNT];

boolean Failure = false;

float aX, aY, aZ, gX, gY, gZ, mX, mY, mZ = 0;

signed int HeadingH, AxH, AyH, AzH, GxH, GyH, GzH, HeadingL, AxL, AyL, AzL, GxL, GyL, GzL;

signed int IAx, IAy, IAz, IHeading, IGx, IGy, IGz;

//int RioAddress = 1;

byte imuArray[14];


/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void)
{
  sensor_t accel, mag, gyro, temp;
  
  lsm.getSensor(&accel, &mag, &gyro, &temp);

  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(accel.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(accel.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(accel.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(accel.max_value); Serial.println(F(" m/s^2"));
  Serial.print  (F("Min Value:    ")); Serial.print(accel.min_value); Serial.println(F(" m/s^2"));
  Serial.print  (F("Resolution:   ")); Serial.print(accel.resolution); Serial.println(F(" m/s^2"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));

  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(mag.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(mag.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(mag.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(mag.max_value); Serial.println(F(" uT"));
  Serial.print  (F("Min Value:    ")); Serial.print(mag.min_value); Serial.println(F(" uT"));
  Serial.print  (F("Resolution:   ")); Serial.print(mag.resolution); Serial.println(F(" uT"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));

  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(gyro.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(gyro.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(gyro.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(gyro.max_value); Serial.println(F(" rad/s"));
  Serial.print  (F("Min Value:    ")); Serial.print(gyro.min_value); Serial.println(F(" rad/s"));
  Serial.print  (F("Resolution:   ")); Serial.print(gyro.resolution); Serial.println(F(" rad/s"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));

  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(temp.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(temp.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(temp.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(temp.max_value); Serial.println(F(" C"));
  Serial.print  (F("Min Value:    ")); Serial.print(temp.min_value); Serial.println(F(" C"));
  Serial.print  (F("Resolution:   ")); Serial.print(temp.resolution); Serial.println(F(" C"));  
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  
  delay(500);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureSensor(void)
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_6G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_16G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS0_MAGGAIN_2GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS0_MAGGAIN_12GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_2000DPS);
}

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void) 
{
  Wire.begin(0x6F);
  Wire.onRequest(ReturnIMU);
  while (!Serial);  // wait for flora/leonardo
  
  Serial.begin(9600);
  Serial.println(F("LSM9DS0 9DOF Sensor Test")); Serial.println("");
  
  /* Initialise the sensor */
  if(!lsm.begin())
  {
    /* There was a problem detecting the LSM9DS0 ... check your connections */
    Serial.print(F("Ooops, no LSM9DS0 detected ... Check your wiring or I2C ADDR!"));
    while(1);
  }
  Serial.println(F("Found LSM9DS0 9DOF"));
  
  /* Display some basic information on this sensor */
//  displaySensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  /* We're ready to go! */
  Serial.println("");
}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/
void loop(void) 
{  
  /* Get a new sensor event */ 
  sensors_event_t accel, mag, gyro, temp;

  lsm.getEvent(&accel, &mag, &gyro, &temp); 
  
int j = 0;
   for (j; j < CNT; j++)
      {   avgAX[j] = (accel.acceleration.x);
          avgAY[j] = (accel.acceleration.y);
          avgAZ[j] = (accel.acceleration.z);
          avgGX[j] = (gyro.gyro.x);
          avgGY[j] = (gyro.gyro.y);
          avgGZ[j] = (gyro.gyro.z);
          avgMX[j] = (mag.magnetic.x);
          avgMY[j] = (mag.magnetic.y);
          avgMZ[j] = (mag.magnetic.z);
      }
      
          aX = mean(avgAX, CNT);
          aY = mean(avgAY, CNT);
          aZ = mean(avgAZ, CNT);
          gX = mean(avgGX, CNT);
          gY = mean(avgGY, CNT);
          gZ = mean(avgGZ, CNT);
          mX = mean(avgMX, CNT);
          mY = mean(avgMY, CNT);
          mZ = mean(avgMZ, CNT); 

if(number % 100 == 0)
{
  // print out accelleration data
  Serial.println(number);
  Serial.print("Accel X: "); Serial.print(aX); Serial.print(" ");
  Serial.print("  \tY: "); Serial.print(aY);       Serial.print(" ");
  Serial.print("  \tZ: "); Serial.print(aZ);     Serial.println("  \tm/s^2");

  // print out magnetometer data
  Serial.print("Magn. X: "); Serial.print(mX); Serial.print(" ");
  Serial.print("  \tY: "); Serial.print(mY);       Serial.print(" ");
  Serial.print("  \tZ: "); Serial.print(mZ);     Serial.println("  \tgauss");
  
  // print out gyroscopic data
  Serial.print("Gyro  X: "); Serial.print(gX); Serial.print(" ");
  Serial.print("  \tY: "); Serial.print(gY);       Serial.print(" ");
  Serial.print("  \tZ: "); Serial.print(gZ);     Serial.println("  \tdps");

  // print out heading
  Serial.print("Heading = "); Serial.print(SuperCompass(mX, mY)); Serial.println(" deg");
  
  // print out temperature data
  Serial.print("Temp: "); Serial.print(temp.temperature); Serial.println(" *C");

  Serial.println("**********************\n");
}

while (Failure == true)
{
//  ReturnIMU();
}

SetWireValues();

number ++;

}

double SuperCompass(float x, float y)
{
  double Direction;
  
 if(y>0)
 {
 Direction = 90 - (atan(x/y))*180/3.14159;
 }
 
 if(y<0)
 {
 Direction = 270 - (atan(x/y))*180/3.14159;
 }
 
 if(y=0 && x<0)
 {
 Direction = 180.0;
 }
 
 if(y=0 && x>0)
 {
 Direction = 0.0;
 }
 return Direction;
}

void ReturnIMU()
{
//  Wire.beginTransmission(RioAddress);
 
 if(Wire.write(imuArray, 14) == 14)
 {
   Failure = false;
 }
 else
 {
   Wire.write(1114);
   Wire.endTransmission();
   Failure = true;
 }

}

void SetWireValues()
{
  IAx = (aX * 10);
  IAy = (aY * 10);
  IAz = (aZ * 10);
  IHeading = SuperCompass(mX, mY);
  IGx = (gX * 10);
  IGy = (gY * 10);
  IGz = (gZ * 10);
  
  AxH = highByte(IAx);
  AyH = highByte(IAy);
  AzH = highByte(IAz);
  HeadingH = highByte(IHeading);  
  GxH = highByte(IGx);
  GyH = highByte(IGy);
  GzH = highByte(IGz);
  
  AxL = lowByte(IAx);
  AyL = lowByte(IAy);
  AzL = lowByte(IAz);
  HeadingL = lowByte(IHeading);
  GxL = lowByte(IGx);
  GyL = lowByte(IGy);
  GzL = lowByte(IGz);
  
  imuArray[0] = AxH;
  imuArray[1] = AxL;
  imuArray[2] = AyH;
  imuArray[3] = AyL;
  imuArray[4] = AzH;
  imuArray[5] = AzL;
  imuArray[6] = HeadingH;
  imuArray[7] = HeadingL;
  imuArray[8] = GxH;
  imuArray[9] = GxL;
  imuArray[10] = GyH;
  imuArray[11] = GyL;
  imuArray[12] = GzH;
  imuArray[13] = GzL;
}
