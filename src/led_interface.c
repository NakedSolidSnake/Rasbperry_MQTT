#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <led_interface.h>
#include <json/json.h>
#include <mqtt_connection.h>
#include "MQTTClient.h"

typedef struct 
{
    void *object;
    LED_Interface *led;
} MQTT_Context;

static int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message);

static MQTT_Connection mqtt = {0};

IHandler iLED[] =
    {
        {.token = "id", .data = &mqtt.id, .type = eType_String, .child = NULL},
        {.token = "address", .data = &mqtt.address, .type = eType_String, .child = NULL},
};

static IHandler iMQTT[] =
    {
        {.token = "led", .data = NULL, .type = eType_Object, .child = iLED, .size = getItems(iLED)},
        {.token = "topics", .data = &mqtt.topic, .type = eType_String, .child = NULL},
};

bool LED_Run(void *object, char **argv, LED_Interface *led)
{
     char buffer[512] = {0};
     MQTT_Context context = 
     {
         .object = object,
         .led = led
     };
    
    if(!JSON_GetFromFile("mqtt_properties.json", buffer, 512))
        return EXIT_FAILURE;

    JSON_Process(buffer, iMQTT, getItems(iMQTT));


    if(led->Init(object) == false)
        return EXIT_FAILURE;

    MQTTClient client;
    MQTTClient_create(&client, mqtt.address, mqtt.id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_setCallbacks(client, &context, NULL, on_message, NULL);

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    MQTTClient_subscribe(client, mqtt.topic, 0);

    while(1)
        sleep(1);    

    return 0;
}

static int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    bool state;
    MQTT_Context *_context = (MQTT_Context *)context;
    char* payload = message->payload;
    state = (bool)atoi(payload);
    _context->led->Set(_context->object, state);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}