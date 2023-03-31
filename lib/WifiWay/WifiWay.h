#ifndef __WIFILAYER_H__
#define __WIFILAYER_H__


#include <ESP8266WiFi.h>

#define WIFI_TIME_RECONNECT 10000// tiempo de espera para volver a intentar la reconexión de WiFi a parte de los 25 segundos de espera necesarios que ejecuta wifi_connect
#define WIFI_TIME_WAIT_REQUEST 500
#define urlPing "www.google.com"

void wifi_init(char* ssid, char* pass);
bool wifi_connect();
void wifi_reconnect();
bool wifi_isConnected();
void wifi_statusNoReconnectWifi();
bool wifi_pingTest(String host);
int  wifiSignal();
bool wifiWaitForAction(const unsigned long);
#endif // __WIFILAYER_H__