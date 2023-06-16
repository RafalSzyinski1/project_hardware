#include <Arduino.h>

#include "Config.h"

void setup()
{
  Serial.begin(BAUDRATE);

  while (!Serial)
    ;

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  Serial.println("ok");
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
