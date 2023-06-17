#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Arduino.h>

#include "ConfigSecret.h"
#include "Config.h"

// MySQL credentials
IPAddress serverIP(SERVER_IP); // Replace with your computer's IP address
char login_db[] = LOGIN_DB;
char password_db[] = PASSWORD_DB;
WiFiClient client;
MySQL_Connection conn((Client *)&client);
MySQL_Cursor *cur_mem = NULL;
const char query[] = "SELECT email, password FROM security.user WHERE email='Tom@demo.com' LIMIT 1;";

void setup()
{
    Serial.begin(BAUDRATE);

    // Connect to Wi-Fi
    WiFi.begin(SSID, PASSWORD_WIFI);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("My IP address is: ");
    Serial.println(WiFi.localIP());

    Serial.println("Connecting...");
    if (conn.connect(serverIP, 3306, login_db, password_db))
    {
        delay(1000);
    }
    else
        Serial.println("Connection failed.");
}

void loop()
{
    delay(2000);

    Serial.println("\nRunning SELECT and printing results\n");

    if (WiFi.status() == WL_CONNECTED && conn.connected())
    {
        // Initiate the query class instance
        cur_mem = new MySQL_Cursor(&conn);
        // Execute the query
        cur_mem->execute(query, true);
        // Fetch the columns and print them
        column_names *cols = cur_mem->get_columns();
        for (int f = 0; f < cols->num_fields; f++)
        {
            Serial.print(cols->fields[f]->name);
            if (f < cols->num_fields - 1)
            {
                Serial.print(", ");
            }
        }
        Serial.println();
        // Read the rows and print them
        row_values *row = NULL;
        do
        {
            row = cur_mem->get_next_row();
            if (row != NULL)
            {
                for (int f = 0; f < cols->num_fields; f++)
                {
                    Serial.print(row->values[f]);
                    if (f < cols->num_fields - 1)
                    {
                        Serial.print(", ");
                    }
                }
                Serial.println();
            }

        } while (row != NULL);
        // Deleting the cursor also frees up memory used
        delete cur_mem;
    }
    else
    {
        Serial.println("Disconnected");
    }
}