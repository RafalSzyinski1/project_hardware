#include <Arduino.h>
#include <ezButton.h>

#include "ConfigSecret.h"
#include "Config.h"

int state = 0;

ezButton button(BUTTON_PIN);

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
    button.loop();
    if (button.isReleased())
        state = !state;

    if (state)
        digitalWrite(YELLOW_LED, HIGH);
    else
        digitalWrite(YELLOW_LED, LOW);

    delay(20);
}