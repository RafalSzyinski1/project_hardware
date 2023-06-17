#ifndef _LOCK_MEM_H_
#define _LOCK_MEM_H_

#include <EEPROM.h>
#include <Arduino.h>

#define EEPROM_SIZE 32

#define ID_ADRESS 0
#define NAME_ADRESS 4

namespace LockMem
{
    void clearMem();
    void printMem(); //! Serial must be init
    int readId();
    void writeId(int Id);
};

void LockMem::clearMem()
{
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < EEPROM_SIZE; ++i)
        EEPROM.put(i, 0);
    EEPROM.commit();
    EEPROM.end();
}

void LockMem::printMem()
{
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < EEPROM_SIZE; ++i)
    {
        if (i % 8 == 0)
            Serial.println();
        Serial.print(*(EEPROM.getConstDataPtr() + i), HEX);
        Serial.print(" ");
    }

    EEPROM.end();
}

int LockMem::readId()
{
    EEPROM.begin(EEPROM_SIZE);
    int result = -1;
    EEPROM.get(ID_ADRESS, result);
    EEPROM.end();
    return result;
}

void LockMem::writeId(int Id)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(ID_ADRESS, Id);
    EEPROM.commit();
    EEPROM.end();
}

#endif // _LOCK_MEM_H_