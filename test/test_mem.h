#include <Arduino.h>

#include "LockMem.h"
#include "Config.h"

void setup()
{
    Serial.begin(BAUDRATE);
    LockMem::clearMem();
    LockMem::printMem();
    LockMem::writeId(1);

    byte key[4] = {0b00010011, 0b01101001, 0b11110111, 0b10100110};
    LockMem::pushKey(key);
    LockMem::pushKey(key);
    LockMem::pushKey(key);

    LockMem::printMem();
    if (LockMem::findKey(key))
        Serial.println("Found");

    LockMem::pushMessage(1);
    LockMem::pushMessage(2);
    LockMem::pushMessage(3);

    LockMem::printMem();

    Serial.println(LockMem::popMessage());
    Serial.println(LockMem::popMessage());
    Serial.println(LockMem::popMessage());
    Serial.println(LockMem::popMessage());
    LockMem::printMem();
    Serial.println();
    Serial.println("----------------------------");
    LockMem::deleteKeys();
    LockMem::printMem();
}

void loop()
{
}
