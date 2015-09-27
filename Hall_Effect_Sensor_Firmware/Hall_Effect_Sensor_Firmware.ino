#include "PinChangeInt.h"
#include "Wire.h"

#define SENSOR_1 2
#define SENSOR_2 3
#define SENSOR_3 4
#define SENSOR_4 5
#define SENSOR_5 6
#define SENSOR_6 7
#define SENSOR_7 8
#define SENSOR_8 9
#define SENSOR_9 10

#define SENSOR_HOLD_TIME 50

const int I2C_ADDRESS = 0x10;   // Set the I2C slave address here

byte activeSensor = 0;
byte lastActiveSensor = 0;
long lastchange = 0;
long sensorDeactivatedTime = 0;

void setup()
{
  pinMode(SENSOR_1, INPUT);
  pinMode(SENSOR_2, INPUT);
  pinMode(SENSOR_3, INPUT);
  pinMode(SENSOR_4, INPUT);
  pinMode(SENSOR_5, INPUT);
  pinMode(SENSOR_6, INPUT);
  pinMode(SENSOR_7, INPUT);
  pinMode(SENSOR_8, INPUT);
  pinMode(SENSOR_9, INPUT);
  
  PCintPort::attachInterrupt(SENSOR_1, sensor1Interrupt, CHANGE);
  PCintPort::attachInterrupt(SENSOR_2, sensor2Interrupt, CHANGE);
  PCintPort::attachInterrupt(SENSOR_3, sensor3Interrupt, CHANGE);
  PCintPort::attachInterrupt(SENSOR_4, sensor4Interrupt, CHANGE);
  PCintPort::attachInterrupt(SENSOR_5, sensor5Interrupt, CHANGE);
  PCintPort::attachInterrupt(SENSOR_6, sensor6Interrupt, CHANGE);
  PCintPort::attachInterrupt(SENSOR_7, sensor7Interrupt, CHANGE);
  PCintPort::attachInterrupt(SENSOR_8, sensor8Interrupt, CHANGE);
  PCintPort::attachInterrupt(SENSOR_9, sensor9Interrupt, CHANGE);
 
 
    // Configure for I2C communications
   Wire.begin(I2C_ADDRESS);
   Wire.onRequest(requestHandler);
}


void loop()
{
  // Reset the active sensor number to 0 once the sensor has been active for a certain period
  if ((sensorDeactivatedTime - lastchange) > SENSOR_HOLD_TIME)
  {
    activeSensor = 0;
  }
}



void requestHandler()
{
   Wire.write((byte)activeSensor);
}


void activateSensor(int sensorNumber)
{
  lastActiveSensor = activeSensor;
  activeSensor = sensorNumber;
  lastchange = millis();
}

void deactivateSensor()
{
  if (activeSensor > 0)
  {
    activeSensor = 0;
    sensorDeactivatedTime = millis();
  }
}

void sensor1Interrupt()
{
  if (digitalRead(SENSOR_1) == LOW)
  {
    activateSensor(1);
  }
  else
  {
    deactivateSensor();
  }
}


void sensor2Interrupt()
{
  if (digitalRead(SENSOR_2) == LOW)
  {
    activateSensor(2);
  }
  else
  {
    deactivateSensor();
  }
}

void sensor3Interrupt()
{
  if (digitalRead(SENSOR_3) == LOW)
  {
    activateSensor(3);
  }
  else
  {
    deactivateSensor();
  }
}

void sensor4Interrupt()
{
  if (digitalRead(SENSOR_4) == LOW)
  {
    activateSensor(4);
  }
  else
  {
    deactivateSensor();
  }
}

void sensor5Interrupt()
{
  if (digitalRead(SENSOR_5) == LOW)
  {
    activateSensor(5);
  }
  else
  {
    deactivateSensor();
  }
}

void sensor6Interrupt()
{
  if (digitalRead(SENSOR_6) == LOW)
  {
    activateSensor(6);
  }
  else
  {
    deactivateSensor();
  }
}

void sensor7Interrupt()
{
  if (digitalRead(SENSOR_7) == LOW)
  {
    activateSensor(7);
  }
  else
  {
    deactivateSensor();
  }
}

void sensor8Interrupt()
{
  if (digitalRead(SENSOR_8) == LOW)
  {
    activateSensor(8);
  }
  else
  {
    deactivateSensor();
  }
}

void sensor9Interrupt()
{
  if (digitalRead(SENSOR_9) == LOW)
  {
    activateSensor(9);
  }
  else
  {
    deactivateSensor();
  }
}

