
#define LED 13
#define SENSOR_INPUT 12


void setup()
{
  pinMode(SENSOR_INPUT, INPUT);
  pinMode(LED, OUTPUT);
}


void loop()
{
  if (digitalRead(SENSOR_INPUT) == LOW)
  {
    digitalWrite(LED, HIGH);
  }
  else
  {
    digitalWrite(LED, LOW);
  }
  
  /*
  This could be rewritten as...
  digitalWrite(LED, !digitalRead(SENSOR_INPUT));
  */
}

