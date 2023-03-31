#ifndef __DSCADAPTER_H__
#define __DSCADAPTER_H__


#include <Arduino.h>
#include <dscKeybusInterface.h>

#define ARM_INT "0"
#define ARM_EXT "1"

#define ZERO String(0)

#if defined(ESP32)
//Pines en uso por esta capa
#define dscClockPin 34  // esp32: 4,13,16-39, yellow (VP en esp32 dev module)
#define dscReadPin 35   // esp32: 4,13,16-39, green (VN en esp32 dev module)
#define dscWritePin 32  // esp32: 4,13,16-33, transistor
#elif defined(ESP8266)
//Pines en uso por esta capa
#define dscClockPin D1 // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#define dscReadPin D2  // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#define dscWritePin D8 // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#endif

//Clase dscAdapter hereda de dscKeybusInterface.s
class dscAdapter : public dscKeybusInterface {
public:
	dscAdapter(byte setClockPin, byte setReadPin, byte setWritePin = 255);
	void setupDSC();
	void ReaderDSC(); //Toma los eventos generados por el panel que son de interés y los manda a a la capa UDP o MQTT para que los envíe en formato Contact ID
	byte adapter_command(const char* comando, byte longitud); //Adapta los comandos solicitados por el usuario para que sean procesados por el panel
private:
	void savePayloadOpenZons(); //arma un string con las zonas abiertas totales
	void modMEF(byte event); //Convocada por ReaderDSC, de acuerdo al evento recibido por el panel envía un mensaje con cierto formato por MQTT
	bool printModuleSlotsDSC(byte outputNumber, byte startByte, byte endByte, byte startMask, byte endMask, byte bitShift, byte matchValue);
	
	byte noEnt = 0; //Se levanta cuando el usuario acciona los comandos S o W, y permite que puedan mandarse por MQTT las zonas inhibidas, una vez hecho el armado parcial
	byte protecArm = 0; //Cuando está armado, no manda al panel los comandos S, W, N y I. Estando levantada y ejecutando uno de estos comando se retornará 1
	byte partitionAct = 1; //Variable que contiene la partición seteada para realizar la acción solicitada por el usuario
	String zona_us; //Uso general, se almacena zon o usuario
	String zona_inhibida;
	byte zon = 0; // variables auxiliares: zon se usa para almacenar las zonas con bypass 
	
	////Cola de zonas abiertas////
	byte statusOpenZones[32]; //El índice representa la zona, si el elemento es 1, la zona esta abierto, si no, está cerrada
	char nroZona[3];
	char zonOpen[3]; //Este es el arreglo que usa stringZonsOpen para armar el string con las zonas abiertas de la particion
	
	String slot = "000";

	static String parti;

	byte armPart2;
    byte armPart3;
    byte armPart4;

	byte armPacialFlag = 0;

	String pgm;
};
#endif // __DSCADAPTER_H__