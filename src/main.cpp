#include <Arduino.h>
#include <EEPROM.h>
#define EEPROM_SIZE 12

void setup()
{
    // Init Serial USB
    Serial.begin(115200);
    Serial.println(F("Initialize System"));
    // Init EEPROM
    EEPROM.begin(EEPROM_SIZE);

    // Write data into eeprom
    int address = 0;

    // Read data from eeprom
    address = 0;
    int readId;
    EEPROM.get(address, readId);
    Serial.print("Read Id = ");
    Serial.println(readId);
    address += sizeof(readId); // update address value

    float readParam;
    EEPROM.get(address, readParam); // readParam=EEPROM.readFloat(address);
    Serial.print("Read param = ");
    Serial.println(readParam);

    EEPROM.end();
}

void loop() {}