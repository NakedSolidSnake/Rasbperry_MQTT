#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <led.h>
#include "MQTTClient.h"

#define ADDRESS     "localhost"
#define CLIENTID    "555334"
#define STATE_TOPIC    "/led/state"

static int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message);

static LED_t led =
    {
        .gpio.pin = 0,
        .gpio.eMode = eModeOutput
    };

int main(int argc, char *argv[])
{
    if (LED_init(&led))
        return EXIT_FAILURE;

    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    MQTTClient_subscribe(client, STATE_TOPIC, 0);

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