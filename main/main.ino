/**
 * @file main.ino
 * @brief Must be compiled with Arduino IDE
 * @version 0.1
 * @date 2021-06-05
 * 
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#define SWITCH_ON   LOW
#define SWITCH_OFF  HIGH

#define HEARTBEAT_PERIOD 30000u  // in ms

const char *on = "ON";
const char *off = "OFF";

// WiFi
const char *ssid = "CEF51_MaxisBroadband"; // Enter your WiFi name
const char *password = "23230457";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "90smobsters.com";

const char *topic_lampu_upright_0 = "/home/bedroom/lampu/0";
const char *topic_lampu_upright_1 = "/home/bedroom/lampu/1";
const char *topic_bedroom_status_state = "/home/bedroom/status/state";
const char *topic_bedroom_status_timestamp = "/home/bedroom/status/timestamp";

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
AsyncWebServer server(80);

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
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

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
    client.publish(topic_bedroom_status_state, "ACTIVE");
    client.subscribe(topic_lampu_upright_0);
    client.subscribe(topic_lampu_upright_1);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP8266v3.");
    });

    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
    Serial.println("HTTP server started");
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

    if( strcmp( topic, topic_lampu_upright_0 ) == 0 )
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
    else if( strcmp( topic, topic_lampu_upright_1 ) == 0 )
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
    static unsigned long lastHeartbeatMs = 0u;
    if( (millis() - lastHeartbeatMs) >= HEARTBEAT_PERIOD )
    {
        lastHeartbeatMs = millis();
        String timestamp = String(lastHeartbeatMs);
        unsigned int str_len = timestamp.length() + 1;
        char char_array[str_len];
        timestamp.toCharArray(char_array, str_len);

        client.publish(topic_bedroom_status_state, "ACTIVE");
        client.publish(topic_bedroom_status_timestamp, char_array);
    }
    client.loop();
    AsyncElegantOTA.loop();
}
