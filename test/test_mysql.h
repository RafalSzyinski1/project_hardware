#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

#include "ConfigSecret.h"
#include "Config.h"

// MySQL credentials
IPAddress serverIP(SERVER_IP); // Replace with your computer's IP address
char login_db[] = LOGIN_DB;
char password_db[] = PASSWORD_DB;
char name_db[] = NAME_DB;
WiFiClient client;

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

    // Connect to MySQL server
    MySQL_Connection conn((Client *)&client);

    if (conn.connect(serverIP, PORT_DB, login_db, password_db, name_db))
    {
        Serial.println("Connected to MySQL server");
    }
    else
    {
        Serial.println("Connection failed to MySQL server");
    }

    // Perform SELECT query
    MySQL_Cursor *cursor = new MySQL_Cursor(&conn);
    char query[128];
    sprintf(query, "SELECT * FROM security.key");

    cursor->execute(query);

    // Fetch and process the rows
    row_values *row;
    column_names *column = cursor->get_columns();
    int rowIndex = 0;
    do
    {
        row = cursor->get_next_row();

        // Process the row data if available
        if (row != NULL)
        {
            Serial.print("Row ");
            Serial.println(rowIndex);
            for (int i = 0; i < column->num_fields; i++)
            {
                Serial.print(column->fields[i]->name);
                Serial.print(": ");
                Serial.println(row->values[i]);
            }
            Serial.println("--------------------");
            rowIndex++;
        }
    } while (row != NULL);

    delete cursor;

    // Close the connection
    conn.close();
}

void loop()
{
    // Do nothing in the loop
}