#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <button_interface.h>
#include <mqtt_connection.h>
#include "MQTTClient.h"
#include <json/json.h>

#define _1ms    1000

static void publishState(MQTTClient client);
static void publish(MQTTClient client, char* topic, char* payload);

static MQTT_Connection mqtt = {0};

IHandler iButton[] =
    {
        {.token = "id", .data = &mqtt.id, .type = eType_String, .child = NULL},
        {.token = "address", .data = &mqtt.address, .type = eType_String, .child = NULL},
};

static IHandler iMQTT[] =
    {
        {.token = "button", .data = NULL, .type = eType_Object, .child = iButton, .size = getItems(iButton)},
        {.token = "topics", .data = &mqtt.topic, .type = eType_String, .child = NULL},
};

bool Button_Run(void *object, char **argv, Button_Interface *button)
{
    char buffer[512] = {0};
    
    if(!JSON_GetFromFile("mqtt_properties.json", buffer, 512))
        return EXIT_FAILURE;

    JSON_Process(buffer, iMQTT, getItems(iMQTT));

    MQTTClient client;
    MQTTClient_create(&client, mqtt.address, mqtt.id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    if(button->Init(object) == false)
        return EXIT_FAILURE;

    while (true)
    {
        while (true)
        {
            if (!button->Read(object))
            {
                usleep(_1ms * 100);
                break;
            }
            else
            {
                usleep(_1ms);
            }
        }

        publishState(client);
    }
    
    return true;
}

static void publishState(MQTTClient client)
{
    static bool state = false;
    char buffer[10] = {0};
    memset(buffer, 0, 10);
    state = state ? false : true;
    snprintf(buffer, 10, "%d", state);
    publish(client, mqtt.topic, buffer);
}

static void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 0;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);    
}