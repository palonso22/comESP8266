#ifndef __AP_H__
#define __AP_H__


#define Server1 "Servidor1"
#define Server2 "Servidor2"
#define PuertoMon "PuertoMQTT"
#define BrokPass "BrokerPass"
#define idWifi "SSID"
#define passwWifi "PASS"
#define Empr "Empresa"
#define Subs "Subscriptor"
#define apnGprs "APN"
#define userApn "UserAPN"
#define passApn "PassAPN"
#define minKA "keepAliveUDP"

#define APonTime 1000*240 //tiempo acción del punto de acceso

#if defined(useMQTT)
void checkViaMQTT(bool);//revisa que este conectada a wifi y  mqtt
#elif defined(useUDP)
void checkViaMQTT();
#endif
void launchWeb();// iniciar el servidor web en 192.168.4.1
void setupAP(); //seteo parametros del punto de acceso y lo inicia
void readSPIFFS();//lee de la flash
void writeSPIFFS();//escribe en al flash
void createWebServer();//crea el servidor web,aqui esta HTML
void serverActive();
#endif // __AP_H__