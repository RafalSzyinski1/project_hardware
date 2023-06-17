#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#include "LockMem.h"
#include "ConfigSecret.h"
#include "Config.h"

void setupPins()
{
    pinMode(YELLOW_LED, OUTPUT);
    digitalWrite(YELLOW_LED, LOW);
}

void setupWifi()
{
    WiFi.begin(SSID, PASSWORD_WIFI);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.print("Connected to WiFi IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(YELLOW_LED, HIGH);
}

IPAddress serverIP(SERVER_IP);
char login_db[] = LOGIN_DB;
char password_db[] = PASSWORD_DB;
WiFiClient client;
MySQL_Connection conn((Client *)&client);
MySQL_Cursor *cur_mem = NULL;

void setupMySQL()
{
    Serial.println("Connecting to database...");
    if (conn.connect(serverIP, PORT_DB, login_db, password_db))
        delay(1000);
    else
    {
        Serial.println("Connection failed. Restart");
        delay(2000);
        ESP.restart();
    }
}

void setupLock()
{
    int id = LockMem::readId();
    if (id == 0)
    {
        Serial.println("Detect new lock");
        LockMem::clearMem();
        Serial.println("Enter new lock name: ");
        while (!Serial.available())
            ;
        String name = Serial.readStringUntil('\n');
        if (WiFi.status() == WL_CONNECTED && conn.connected())
        {
            Serial.println("Adding to database");
            cur_mem = new MySQL_Cursor(&conn);
            String query = "INSERT INTO security.lock (name) VALUES ('" + name + "');";
            cur_mem->execute(query.c_str(), true);
            LockMem::writeId(cur_mem->get_last_insert_id());
            Serial.println("Succesful add lock to database");
            delete cur_mem;
        }
        else
        {
            Serial.println("Cannot add this lock to database");
            ESP.restart();
        }
    }
    else
    {
        if (WiFi.status() == WL_CONNECTED && conn.connected())
        {
            Serial.print("Verifying lock with id: ");
            Serial.println(id);
            cur_mem = new MySQL_Cursor(&conn);
            String query = "SELECT id FROM security.lock WHERE id=" + String(id) + ";";
            cur_mem->execute(query.c_str(), true);

            if (cur_mem->get_columns() != NULL)
            {
                row_values *row = cur_mem->get_next_row();

                if (row != NULL)
                {
                    Serial.println("Lock ok");
                    while (row != NULL)
                        row = cur_mem->get_next_row();
                }
                else
                {
                    Serial.println("Lock not found. Restarting lock");
                    LockMem::clearMem();
                    ESP.restart();
                }
            }
            else
            {
                Serial.println("Cannot verify lock");
                ESP.restart();
            }

            delete cur_mem;
        }
        else
        {
            Serial.println("Cannot verify lock");
            ESP.restart();
        }
    }
}

MFRC522 rfid(SS_PIN, RST_PIN);

void setupRFID()
{
    SPI.begin();
    rfid.PCD_Init();
    Serial.println();
    Serial.print(F("Reader :"));
    rfid.PCD_DumpVersionToSerial();
    Serial.println("Enter card");
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    setupPins();
    setupWifi();
    setupMySQL();
    setupLock();
    setupRFID();
}

void loop()
{
    delay(20);

    if (WiFi.status() == WL_CONNECTED && conn.connected())
    {
        digitalWrite(YELLOW_LED, HIGH);
    }
    else
    {
        WiFi.reconnect();
        setupMySQL();
        digitalWrite(YELLOW_LED, LOW);
    }

    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!rfid.PICC_IsNewCardPresent())
        return;

    // Verify if the NUID has been readed
    if (!rfid.PICC_ReadCardSerial())
        return;

    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.println(rfid.PICC_GetTypeName(piccType));

    // Check is the PICC of Classic MIFARE type
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K)
    {
        Serial.println(F("Your tag is not of type MIFARE Classic."));
        return;
    }

    String uid;
    for (int i = 0; i < rfid.uid.size; ++i)
        uid = uid + String(rfid.uid.uidByte[i], HEX);
    Serial.print("UID: ");
    Serial.println(uid);

    if (WiFi.status() == WL_CONNECTED && conn.connected())
    {
        Serial.println("Verifying key");
        cur_mem = new MySQL_Cursor(&conn);
        String query = "SELECT * FROM security.key WHERE uid='" + uid + "';";
        cur_mem->execute(query.c_str(), true);

        if (cur_mem->get_columns() != NULL)
        {
            row_values *row = cur_mem->get_next_row();

            if (row != NULL)
            {
                Serial.print("Key exist. Key id = ");
                Serial.println(row->values[0]);
                while (row != NULL)
                    row = cur_mem->get_next_row();
                Serial.println("Enter card");
            }
            else
            {
                Serial.println("Key doesn't exist. Adding key");
                Serial.println("Enter name of the key: ");
                while (!Serial.available())
                    ;
                String name = Serial.readStringUntil('\n');
                query = "INSERT INTO security.key (name, uid) VALUES ('" + name + "', '" + uid + "');";
                cur_mem->execute(query.c_str(), true);
                if (cur_mem->get_last_insert_id() > 0)
                {
                    Serial.println("Key added successfuly");
                    Serial.println("Enter card");
                }
                else
                {
                    Serial.println("Something worng. Try again");
                }
            }
        }

        delete cur_mem;
    }
    else
    {
        Serial.println("Cannot establish connection. Try again");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}