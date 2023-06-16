#include <Arduino.h>

#include "ConfigSecret.h"
#include "Config.h"

int state = 0;

void setup()
{
    Serial.begin(BAUDRATE);

    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);

    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
}

void loop()
{
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);

    if (millis() % 1000 < 750)
        state = 0;
    if (millis() % 1000 < 500)
        state = 1;
    if (millis() % 1000 < 250)
        state = 2;

    if (state == 0)
        digitalWrite(YELLOW_LED, HIGH);
    else if (state == 1)
        digitalWrite(GREEN_LED, HIGH);
    else if (state == 2)
        digitalWrite(RED_LED, HIGH);

    delay(20);
}