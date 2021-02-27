<p align="center">
  <img src="https://moriohcdn.b-cdn.net/b18b836ce0.png">
</p>

# MQTT
## Introdução
## Implementação
### mqtt_connection.h
```c
#ifndef MQTT_CONNECTION_H_
#define MQTT_CONNECTION_H_

typedef struct 
{
    char *id;
    char *address;
    char *topic;
} MQTT_Connection;

#endif /* MQTT_CONNECTION_H_ */
```
### button_publish.c
```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <button.h>
#include <json.h>
#include "mqtt_connection.h"
#include "MQTTClient.h"

#define _1ms    1000

static Button_t button =
{
    .gpio.pin = 7,
    .gpio.eMode = eModeInput,
    .ePullMode = ePullModePullUp,
    .eIntEdge = eIntEdgeFalling,
    .cb = NULL
};

static void wait_press(void);
static void publishState(MQTTClient client);
static void publish(MQTTClient client, char* topic, char* payload);

MQTT_Connection mqtt = {0};

IHandler iButton[] =
    {
        {.token = "id", .data = &mqtt.id, .type = eType_String, .child = NULL},
        {.token = "address", .data = &mqtt.address, .type = eType_String, .child = NULL},
};

IHandler iMQTT[] =
    {
        {.token = "button", .data = NULL, .type = eType_Object, .child = iButton, .size = getItems(iButton)},
        {.token = "topics", .data = &mqtt.topic, .type = eType_String, .child = NULL},
};

int main(int argc, char *argv[])
{
    char buffer[512] = {0};
    
    if(!getJsonFromFile("mqtt_properties.json", buffer, 512))
        return EXIT_FAILURE;

    processJson(buffer, iMQTT, getItems(iMQTT));

    MQTTClient client;
    MQTTClient_create(&client, mqtt.address, mqtt.id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    
    if (Button_init(&button))
        return EXIT_FAILURE;

    while(1)
    {
        wait_press();
        publishState(client);
    }

    return 0;
}


static void wait_press(void)
{
    while (1)
    {
        if (!Button_read(&button))
        {
            usleep(_1ms * 40);
            while (!Button_read(&button))
                ;                
            usleep(_1ms * 40);
            break;
        }
        else
        {
            usleep(_1ms);
        }
    }
}
static void publishState(MQTTClient client)
{
    static eState_t state = eStateLow;
    char buffer[10] = {0};
    memset(buffer, 0, 10);
    state = state ? eStateLow : eStateHigh;
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
```
### led_subscriber.c
```c
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
```
## Conclusão
