#ifndef MQTT_CONNECTION_H_
#define MQTT_CONNECTION_H_

#define ID_LEN          12
#define ADDRESS_LEN     32
typedef struct 
{
    char id[ID_LEN];
    char address[ADDRESS_LEN];
} MQTT_Connection;

#endif /* MQTT_CONNECTION_H_ */
