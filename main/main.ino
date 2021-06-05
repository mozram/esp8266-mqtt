/**
 * @file main.ino
 * @brief Must be compiled with Arduino IDE
 * @version 0.1
 * @date 2021-06-05
 * 
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SWITCH_ON   LOW
#define SWITCH_OFF  HIGH

const char *on = "ON";
const char *off = "OFF";

// WiFi
const char *ssid = "Ramli_wifi"; // Enter your WiFi name
const char *password = "78933476";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "90smobsters.com";

const char *topic_kipas = "/home/bedroom/kipas/0";
const char *topic_lampu = "/home/bedroom/lampu/0";

const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

// Pin Config
const int kipas0Pin = 4;
bool kipas0State = SWITCH_OFF;

const int lampu0Pin = 5;
bool lampu0State = SWITCH_OFF;

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
    // Set software serial baud to 115200;
    Serial.begin(115200);

    // Setup pin
    pinMode(kipas0Pin, OUTPUT);
    digitalWrite(kipas0Pin, kipas0State);
    pinMode(lampu0Pin, OUTPUT);
    digitalWrite(lampu0Pin, lampu0State);

    // connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");

    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected())
    {
        String client_id = "esp8266-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
        {
            Serial.println("Public mqtt broker connected");
        } 
        else
        {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }

    // publish and subscribe
    client.publish(topic_lampu, "hello emqx");
    client.subscribe(topic_kipas);
    client.subscribe(topic_lampu);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");

    for (int i = 0; i < length; i++)
    {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");

    char * command;
    command = (char *) malloc(length);
    memcpy(command, payload, length);

    if( strcmp( topic, topic_kipas ) == 0 )
    {
        if( memcmp( command, on, 2 ) == 0 )
        {
            kipas0State = SWITCH_ON;
            digitalWrite( kipas0Pin, kipas0State );
        }
        else if( memcmp( command, off, 3 ) == 0 )
        {
            kipas0State = SWITCH_OFF;
            digitalWrite( kipas0Pin, kipas0State );
        }
    }
    else if( strcmp( topic, topic_lampu ) == 0 )
    {
        if( memcmp( command, on, 2 ) == 0 )
        {
            lampu0State = SWITCH_ON;
            digitalWrite( lampu0Pin, lampu0State );
        }
        else if( memcmp( command, off, 3 ) == 0 )
        {
            lampu0State = SWITCH_OFF;
            digitalWrite( lampu0Pin, lampu0State );
        }
    }

    if(command != NULL) free(command);
}

void loop()
{
    client.loop();
}