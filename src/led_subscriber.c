#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <led.h>
#include <json.h>
#include "mqtt_connection.h"
#include "MQTTClient.h"

static int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message);

static LED_t led =
    {
        .gpio.pin = 0,
        .gpio.eMode = eModeOutput
    };

MQTT_Connection mqtt = {0};

IHandler iLED[] =
    {
        {.token = "id", .data = &mqtt.id, .type = eType_String, .child = NULL},
        {.token = "address", .data = &mqtt.address, .type = eType_String, .child = NULL},
};

IHandler iMQTT[] =
    {
        {.token = "led", .data = NULL, .type = eType_Object, .child = iLED, .size = getItems(iLED)},
        {.token = "topics", .data = &mqtt.topic, .type = eType_String, .child = NULL},
};

int main(int argc, char *argv[])
{
    char buffer[512] = {0};
    
    if(!getJsonFromFile("mqtt_properties.json", buffer, 512))
        return EXIT_FAILURE;

    processJson(buffer, iMQTT, getItems(iMQTT));


    if (LED_init(&led))
        return EXIT_FAILURE;

    MQTTClient client;
    MQTTClient_create(&client, mqtt.address, mqtt.id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

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
    eState_t state;
    char* payload = message->payload;
    state = (eState_t)atoi(payload);
    LED_set(&led, state);    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}