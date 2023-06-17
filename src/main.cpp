#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Arduino.h>

#include "LockMem.h"
#include "ConfigSecret.h"
#include "Config.h"

void setupPins()
{
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);

    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
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
            query = "SELECT id FROM security.lock WHERE name LIKE '" + name + "%';";
            cur_mem->execute(query.c_str(), true);
            cur_mem->get_columns();
            row_values *row = cur_mem->get_next_row();

            if (row != NULL)
            {
                LockMem::writeId(atoi(row->values[0]));
                Serial.println("Succesful add lock to database");
            }
            else
            {
                Serial.println("Something wrong while adding");
                ESP.restart();
            }

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

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    setupPins();
    setupWifi();
    setupMySQL();
    setupLock();
    LockMem::printMem();
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED && conn.connected())
    {
        digitalWrite(YELLOW_LED, HIGH);
    }
    else
    {
        digitalWrite(YELLOW_LED, LOW);
    }

    delay(20);
}