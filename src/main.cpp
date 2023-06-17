#include <Arduino.h>

#include "LockMem.h"
#include "Config.h"

void setup()
{
    Serial.begin(BAUDRATE);
    LockMem::clearMem();
    LockMem::printMem();

    byte key[4] = {0b00010011, 0b01101001, 0b11110111, 0b10100110};
    LockMem::pushKey(key);

    LockMem::printMem();
    if (LockMem::findKey(key))
        Serial.println("Found");

    LockMem::pushMessage(1);
    LockMem::pushMessage(2);
    LockMem::pushMessage(3);

    LockMem::printMem();

    LockMem::popMessage();
    LockMem::printMem();
}

void loop()
{
}
