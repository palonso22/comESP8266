#include <Arduino.h>

#define SIZE_CID_RING_BUFFER 50 //tamaño del RingBuffer

#define R1windowTime 60000 //tiempo sino llega un R0,ventana de tiempo para mandar estados

#define charD 'D'
#define charS 'S'
#define charR 'R'
#define charE 'E'
#define charM 'M'
#define charR1 "R1"
#define charR0 "R0"
#define charD1 "D1"
#define charD0 "D0"

#define pgmstring "T*"
#define readyPanel "T*#"
#define readyID "999"

class dscAdapter;

#if defined(useLOGS)
class MQTTClient;

void llamanteLog(MQTTClient* mqtt, String msgLog);
#endif

void pcid_setup(dscAdapter& panelDsc);
void pcid_setup_queues();
void pcid_callback(char qualifier,const String event,const String partition,const String zone_user);//recibe los eventos del panel y los envía
void pcid_recibComand(String arrivComand);
byte pcid_transmitComand();
void pcid_loop();
bool pcid_transmitEvent();
//Esta función es clabe para poder usar un método de la clase declarada como fordward (dscAdapter)
bool llamanteMetodo(dscAdapter* dsc,const char* cadena,byte largo);