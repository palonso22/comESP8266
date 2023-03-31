#include <Arduino.h>
#include "Constants.h"


void print_version(){	
	Serial.printf("Iniciando %s %s\n",SoftwareName,SoftwareVersion);	
}



void setup_baud_rate(){
	Serial.begin(115200);			
}

#if defined(useMQTT)
#include "GPIO.h"
#include "AP.h"
#include "WifiWay.h"
#include "dscAdapter.h"
#include "ProcessCID.h"
#include "MQTTClient.h"

WiFiClient espClient;

extern char rsid[32];//variables que leen la memoria EEPROM
extern char rpass[64];
extern char rmqttServer1[32];
extern char rmqttServer2[32];
extern char rmqttPort[8];
extern char rbusiness[8];
extern char rsubscriber[8];
extern char rbrokerPass[32];

MQTTClient cliente(espClient);
dscAdapter dsc(dscClockPin, dscReadPin, dscWritePin);

void setup() {
	setup_baud_rate();
	print_version();	
	gpio_setup(); //Inicializo GPIO
	pcid_setup(dsc);
	readSPIFFS();
	wifi_init(rsid,rpass); //Se inicializa la conexión wifi
	cliente.setupTopics(rbusiness,rsubscriber);
	cliente.setupMQTT(rmqttServer1,rmqttServer2,rmqttPort,rbrokerPass); //Se inicializa la conexión Mqtt
	checkViaMQTT(cliente.connected());
	dsc.setupDSC();
	Serial.println("Comunicación vía MQTT");
}

void loop() {
	//Verifica si el ESP esta conectado a la RED WIFI
	if(wifi_isConnected()){ 
		//Verifica si Cliente MQTT esta conectado al broker
		if(cliente.connected()){
			cliente.handle(); //Ejecuta recibe comandos y envia mensajes por MQTT, 
			if(wifiWaitForAction(WiFiSignalTime))cliente.publishWifiSignal(wifiSignal()); 
			//Transmite los comandos recibidos por MQTT al panel
			cliente.transmitCommand();
		}
		//Se reconecta al broker
		else cliente.reconnect();
	}
	else wifi_reconnect(); //Se reconecta a la RED WIFI
	for(byte i=0;i<50;i++) dsc.ReaderDSC(); //CLAVE EN ESP8266: Esto es para desencolar todos los eventos cuando se pierde conexión
	gpio_loop();
}
#elif defined(useUDP)
#include "GPIO.h"
#include "AP.h"
#include "WifiWay.h"
#include "dscAdapter.h"
#include "ProcessCID.h"
#include "UDPClient.h"

WiFiClient espClient;

extern char rsid[32];//variables que leen la memoria EEPROM
extern char rpass[64];
extern char rmqttServer1[32];
extern char rmqttServer2[32];
extern char rmqttPort[8];
extern char rbusiness[8];
extern char rsubscriber[8];
extern char rminutesKeepAlive[8];

UDPClient cliente;
dscAdapter dsc(dscClockPin, dscReadPin, dscWritePin);

void setup() {
	gpio_setup(); //Inicializo GPIO
	pcid_setup(dsc);
	readSPIFFS();
	wifi_init(rsid,rpass); //Se inicializa la conexión wifi
	cliente.udp_setupAccount(rbusiness,rsubscriber);
	cliente.udp_setup(rmqttServer1, rmqttServer2, atoi(rmqttPort), atoi(rminutesKeepAlive)); //Se inicializa la conexión Mqtt
	Serial.println("Comunicación vía UDP");
	checkViaMQTT();
	dsc.setupDSC();
}

void loop() {
	if(WiFi.status() == WL_CONNECTED){ //Si la ESP32 está conectada a la red, ejecuta la siguiente linea. Verifica que esté conectado
		cliente.udp_loop();
		cliente.udp_get();
	}
	else wifi_reconnect(); //En caso de perder la conexión wifi, se reconecta
	for(byte i=0;i<50;i++) dsc.ReaderDSC(); //CLAVE EN ESP8266: Esto es para desencolar todos los eventos cuando se pierde conexión
	gpio_loop();
}
#endif
