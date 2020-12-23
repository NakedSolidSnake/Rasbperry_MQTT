#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <button.h>
#include "MQTTClient.h"

#define ADDRESS     "localhost"
#define CLIENTID    "555333"
#define STATE_TOPIC    "/led/state"

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

int main(int argc, char *argv[])
{
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
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
    publish(client, STATE_TOPIC, buffer);
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