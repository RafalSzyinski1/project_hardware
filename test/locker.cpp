#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ezButton.h>

#include "LockMem.h"
#include "ConfigSecret.h"
#include "Config.h"

ezButton button(BUTTON_PIN);

void setupPins()
{
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);

    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);

    button.setDebounceTime(50);
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

void downloadKeys()
{
    int id = LockMem::readId();
    Serial.println("Downloading keys for lock");
    if (WiFi.status() == WL_CONNECTED && conn.connected())
    {
        cur_mem = new MySQL_Cursor(&conn);
        String query = "SELECT uid FROM SECURITY.key JOIN SECURITY.key_locks ON SECURITY.key.id=SECURITY.key_locks.key_id WHERE SECURITY.key_locks.lock_id=" + String(id) + ";";
        cur_mem->execute(query.c_str(), true);

        if (cur_mem->get_columns() != NULL)
        {
            row_values *row = cur_mem->get_next_row();
            LockMem::deleteKeys();
            byte uid[4];
            if (row != NULL)
            {

                while (row != NULL)
                {
                    sscanf(row->values[0], "%2hhx%2hhx%2hhx%2hhx", &uid[0], &uid[1], &uid[2], &uid[3]);
                    LockMem::pushKey(uid);
                    row = cur_mem->get_next_row();
                }
                Serial.println("Key downloaded");
            }
            else
            {
                Serial.println("No keys");
            }
        }
        else
        {
            Serial.println("Error while downloading keys from db");
        }

        delete cur_mem;
    }
    else
    {
        Serial.println("Cannot connect to wifi or db");
    }
}

void flushMessage()
{
    int id = LockMem::readId();
    if (WiFi.status() == WL_CONNECTED && conn.connected())
    {
        int key_id = LockMem::popMessage();
        cur_mem = new MySQL_Cursor(&conn);
        while (key_id != -1)
        {
            String query = "INSERT INTO security.key_usage_history (key_id, lock_id) VALUES (" + String(key_id) + "," + String(id) + ");";
            cur_mem->execute(query.c_str(), true);
            if (cur_mem->get_last_insert_id() > 0)
            {
                Serial.print("Added message to history: key(");
                Serial.print(key_id);
                Serial.print(") lock(");
                Serial.print(id);
                Serial.println(")");
            }
            else
            {
                Serial.println("Something wrong while adding message to db");
            }
            key_id = LockMem::popMessage();
        }
        delete cur_mem;
    }
    else
    {
        Serial.println("Cannot connect to wifi. Cannot flush messages");
    }
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
    downloadKeys();
    setupRFID();
}

long nr_iter = 0;
long wifi_iter = 0;
int led_state = 0;
int led_timer = 0;

void loop()
{
    delay(20);
    button.loop();

    if (WiFi.status() == WL_CONNECTED && conn.connected())
    {
        digitalWrite(YELLOW_LED, HIGH);
        flushMessage();
        wifi_iter = 0;
    }
    else
    {
        digitalWrite(YELLOW_LED, LOW);
        wifi_iter++;
    }

    if (wifi_iter >= 2000)
    {
        setupWifi();
        setupMySQL();
    }

    if (button.isReleased())
    {
        if (WiFi.status() == WL_CONNECTED && conn.connected())
        {
            downloadKeys();
            nr_iter = 0;
        }
    }

    if (nr_iter >= 20000)
    {
        if (WiFi.status() == WL_CONNECTED && conn.connected())
        {
            downloadKeys();
            nr_iter = 0;
        }
    }
    else
    {
        nr_iter++;
    }

    if (led_timer > 0)
    {
        if (led_state == 1)
        {
            led_timer--;
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(RED_LED, LOW);
        }
        else if (led_state == 2)
        {
            digitalWrite(GREEN_LED, LOW);
            digitalWrite(RED_LED, HIGH);
            led_timer--;
        }
        else
        {
            digitalWrite(GREEN_LED, LOW);
            digitalWrite(RED_LED, LOW);
            led_timer = 0;
        }
    }

    else
    {
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, LOW);
        led_timer = 0;
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

    if (LockMem::findKey(rfid.uid.uidByte))
    {
        led_state = 1;
        led_timer = 250;
        Serial.println("Access granted");
    }
    else
    {
        led_state = 2;
        led_timer = 250;
        Serial.println("Access deny");
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}