#ifndef _LOCK_MEM_H_
#define _LOCK_MEM_H_

#include <EEPROM.h>
#include <Arduino.h>

#define EEPROM_SIZE 806

#define ID_ADRESS 0
#define KEY_START_ADRESS 4
#define KEY_END_ADRESS 403
#define MESSAGE_START_ADRESS 404
#define MESSAGE_END_ADRESS 803

namespace LockMem
{
    void clearMem();
    void printMem(); //! Serial must be init
    int readId();
    void writeId(int Id);
    void pushKey(byte uid[4]);
    boolean findKey(byte uid[4]);
    void deleteKeys();
    void pushMessage(byte uid[4]);
    void popMessage(byte uid[4]);
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

void LockMem::pushKey(byte uid[4])
{
    EEPROM.begin(EEPROM_SIZE);
    int adress = KEY_START_ADRESS;
    while (adress < KEY_END_ADRESS)
    {
        if (*(EEPROM.getConstDataPtr() + adress) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 1) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 2) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 3) == 0)
        {
            EEPROM.put(adress, uid[0]);
            EEPROM.put(adress + 1, uid[1]);
            EEPROM.put(adress + 2, uid[2]);
            EEPROM.put(adress + 3, uid[3]);
            break;
        }
        adress += 4;
    }
    EEPROM.commit();
    EEPROM.end();
}

boolean LockMem::findKey(byte uid[4])
{
    EEPROM.begin(EEPROM_SIZE);
    int adress = KEY_START_ADRESS;
    while (adress < KEY_END_ADRESS)
    {
        if (*(EEPROM.getConstDataPtr() + adress) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 1) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 2) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 3) == 0)
        {
            return false;
        }
        if (*(EEPROM.getConstDataPtr() + adress) == uid[0] &&
            *(EEPROM.getConstDataPtr() + adress + 1) == uid[1] &&
            *(EEPROM.getConstDataPtr() + adress + 2) == uid[2] &&
            *(EEPROM.getConstDataPtr() + adress + 3) == uid[3])
        {
            return true;
        }
        adress += 4;
    }
    EEPROM.end();
    return false;
}

void LockMem::deleteKeys()
{
    EEPROM.begin(EEPROM_SIZE);
    int adress = KEY_START_ADRESS;
    while (adress < KEY_END_ADRESS && !(*(EEPROM.getConstDataPtr() + adress) == 0 &&
                                        *(EEPROM.getConstDataPtr() + adress + 1) == 0 &&
                                        *(EEPROM.getConstDataPtr() + adress + 2) == 0 &&
                                        *(EEPROM.getConstDataPtr() + adress + 3) == 0))
    {

        EEPROM.put(adress, byte(0));
        EEPROM.put(adress + 1, byte(0));
        EEPROM.put(adress + 2, byte(0));
        EEPROM.put(adress + 3, byte(0));
        adress += 4;
    }
    EEPROM.commit();
    EEPROM.end();
}

void LockMem::pushMessage(byte uid[4])
{
    EEPROM.begin(EEPROM_SIZE);
    int adress = MESSAGE_START_ADRESS;
    while (adress < MESSAGE_END_ADRESS)
    {
        if (*(EEPROM.getConstDataPtr() + adress) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 1) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 2) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 3) == 0)
        {
            EEPROM.put(adress, uid[0]);
            EEPROM.put(adress + 1, uid[1]);
            EEPROM.put(adress + 2, uid[2]);
            EEPROM.put(adress + 3, uid[3]);
            break;
        }

        adress += 4;
    }
    EEPROM.commit();
    EEPROM.end();
}

void LockMem::popMessage(byte uid[4])
{
    EEPROM.begin(EEPROM_SIZE);
    int adress = MESSAGE_START_ADRESS;
    while (adress < MESSAGE_END_ADRESS)
    {
        if (*(EEPROM.getConstDataPtr() + adress) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 1) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 2) == 0 &&
            *(EEPROM.getConstDataPtr() + adress + 3) == 0)
        {
            if (adress != MESSAGE_START_ADRESS)
            {
                adress -= 4;
                EEPROM.get(adress, uid[0]);
                EEPROM.get(adress + 1, uid[1]);
                EEPROM.get(adress + 2, uid[2]);
                EEPROM.get(adress + 3, uid[3]);
                EEPROM.put(adress, byte(0));
                EEPROM.put(adress + 1, byte(0));
                EEPROM.put(adress + 2, byte(0));
                EEPROM.put(adress + 3, byte(0));
                EEPROM.end();
            }
            break;
        }
        adress += 4;
    }
    EEPROM.commit();
    EEPROM.end();
}

#endif // _LOCK_MEM_H_