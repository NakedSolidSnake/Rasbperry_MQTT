#ifndef NETWORK_H_
#define NETWORK_H_

#define SSID_LEN            32
#define PASSWORD_LEN        64

typedef struct 
{
    char ssid[SSID_LEN];
    char password[PASSWORD_LEN];
} Network;

#endif /* NETWORK_H_ */
