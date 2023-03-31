#include "ProcessCID.h"
#include <Arduino.h>
#include "GPIO.h"
#include "../../include/Comandos.h"

#include <RingBuf.h>
RingBuf *queueEventCID;

unsigned long R1currentTime=0; //Cuando está en alto, se activa el contador para enviar los estados en ../../ST
unsigned long R1previousTime=0;
byte newR1ReceptionPart1=0;
byte newR1ReceptionPart2=0;
bool flagR1=0;

bool flagD1=0;

String eventCID;
String comando;

dscAdapter* DSC;

/*
Inicializo
*/
void pcid_setup(dscAdapter& panelDsc){
    DSC = &panelDsc;
    #if defined(ESP32)
    xProcessEvent = xSemaphoreCreateMutex();
    #endif
    pcid_setup_queues();
}

/*
Inicializa las colas
*/
void pcid_setup_queues(){
    queueEventCID = RingBuf_new(sizeof(String), SIZE_CID_RING_BUFFER);
    if(queueEventCID == NULL){
        Serial.println("Error creating the queue");
        ESP.restart();
    }
}

/*
CONVOCADO POR LA CAPA SW DEL PANEL, RECIBE LOS EVENTOS DEL MISMO Y LOS ALMACENA EN EL RINGBUFFER EN FORMATO CONTACT ID
*/
void pcid_callback(char qualifier,const String event,const String partition,const String zone_user) {//recibe los eventos del panel y los envía
    static String eventDSC;
    static String zoneuPrevPart1 = "000";
    static String zoneuPrevPart2 = "000";
    //EVENTOS DE ESTADO (S)
    R1currentTime=millis();
    if(qualifier == charS){
        if(flagR1){
            if(event == readyID){ //Que mande nomás el cambio de estado de ready/not ready de la partición
                if( partition.charAt(1) == '1'){
                    if(zone_user == zoneuPrevPart1 && !newR1ReceptionPart1) return;
                    //Se evita que se mande más de una vez el mismo mensaje
                    //Esto se hace porque a veces el panel manda 3 listos seguidos, cuando con mandar uno solo, es suficiente
                    else zoneuPrevPart1 = zone_user;
                }
                if( partition.charAt(1) == '2'){
                    if(zone_user == zoneuPrevPart2 && !newR1ReceptionPart2) return;
                    //Se evita que se mande más de una vez el mismo mensaje
                    //Esto se hace porque a veces el panel manda 3 listos seguidos, cuando con mandar uno solo, es suficiente
                    else zoneuPrevPart2 = zone_user;
                }
            }
            //newR1Reception va a tener un valor igual al numero de particiones activas, para poder dar el listo correctamente
            //problema: ¿Si no sé cuántas particiones activas va a tener la alarma que se
            if(newR1ReceptionPart1>0) newR1ReceptionPart1--;
            if(newR1ReceptionPart2>0) newR1ReceptionPart2--;
        }
        else return;
    }
    eventDSC = qualifier + event + partition + zone_user;
    queueEventCID->add(queueEventCID,&eventDSC);
}
/*
Convocada por ahora solamente por la función callback de mqtt, almacena el payload recibido
*/
void pcid_recibComand(String arriveComand){
    comando = arriveComand;
}
/*
Regula el tiempo de actividad de paso de eventos de caracter secundario (.../.../ES)
*/
void pcid_loop(){
    if(flagR1){
        R1currentTime=millis();
        if(R1currentTime-R1previousTime > R1windowTime){ //Si el tiempo desde que se ejecutó el comando R1 es mayor al minuto, se frena el envío de mensajes 'S'
            flagR1=0;
            Serial.println("Finaliza el trafico mqtt de eventos con calificador 'S'");
        }
    }
}
/*
Transmite los comandos rebidos, además agrega algunos extras que no estan contemplados en la capa del panel
*/
byte pcid_transmitComand(){
    static byte ret = 3;
    //char pgm [SIZE_TOPIC];
    // Si el mensaje recibido es R1, se habilita el paso a los eventos de calificador 'S' emitidos por el panel.
    // Además ejecuta sobre el panel *#. Haciendo esto, el panel responde indicando si está listo para armar o no.
    // El saber si está listo, evento de claificador S, le aporta una información muy útil a la APP
    // El paso de eventos de estado (S), se dará por un minuto, o hasta que se ejecute R0 
    if(comando == charR1){
        R1previousTime=millis();
        newR1ReceptionPart1=1;
        newR1ReceptionPart2=2;
        flagR1=1;
        Serial.println("Comienza el envío de eventos de estados");
        //publish(LOGTopic,"Comienza el envío de eventos de estados"); 
        ret=llamanteMetodo(DSC,readyPanel,strlen(readyPanel));
        return ret;
    }
    // Si llega un mensaje "R0", habiendo ejecutado previamente R1, se frena el paso de eventos de estado
    else if(comando == charR0){
        Serial.println("Finaliza el envío de eventos de estados");
        //publish(LOGTopic,"Finaliza el envío de eventos de estados");
        R1previousTime=0;
        R1currentTime=0;
        flagR1=0;
    }
    else if(comando.substring(0,2) == charD1){// si llega un mensaje "D1X"
        flagD1=1;
        Serial.print("Turn on GPIO ");
        //Serial.println(arrivedPayload[2]);//solo se contempla el caso de GPIO 0-9,en caso de usar GPIO mayor a 10 arrivedPayload[2]arrivedPayload[3]
        #if defined(useRev2)
        if(comando.charAt(2) == '1'){//solo se hizo con el GPIO5 de prueba
            gpio_cambiar_salida(Output1,HIGH);        
        }
        if(comando.charAt(2) == '2'){//solo se hizo con el GPIO5 de prueba
            gpio_cambiar_salida(Output2,HIGH);     
        }
        #endif
    }
    else if(comando.substring(0,2) == charD0){// si llega un mensaje "D0X"
        flagD1=0;
        Serial.print("Turn off GPIO ");
        #if defined(useRev2)
        Serial.println(comando.charAt(2));
        //snprintf(LOGpublish,sizeof (LOGpublish),"Turn off GPIO %c",arrivedPayload[2]);
        //publish(LOGTopic,LOGpublish);
        if(comando.charAt(2) == '1'){
            gpio_cambiar_salida(Output1,LOW);    
        }
        if(comando.charAt(2) == '2'){//solo se hizo con el GPIO5 de prueba
            gpio_cambiar_salida(Output2,LOW); 
        }
        #endif
    }
    // El panel tiene entradas/salidas digitales, este comando permite indicar el estado de las mismas
    // Estas i/o son llamadas pgm, y este panel en total tiene 3
    else if(comando.charAt(0) == charM){
        if(comando.length() == 6){
            for(byte i=1;i<6;i++){
                if(!(comando.charAt(i)>= 0x30 && comando.charAt(i) <= 0x39)){
                    Serial.println("PGM no valido");
                        return 2;
                }
            }
        }
        /*comando[0]='7';
        Serial.print("Turn on PGM");
        Serial.println(comando[1]);
        snprintf(pgm,sizeof(pgm),"%s%s",pgmstring,comando); // Mandando T*7X, levantamos el pgm X
        Serial.println(pgm);
        ret=llamanteMetodo(DSC,pgm,strlen(pgm));
        return ret; */
    }
    else{
        // El resto de mensajes que lleguen serán directamente mandados a procesar por la capa inferior
        Serial.print("The command arrived is ");
        Serial.println(comando);
        ret=llamanteMetodo(DSC,comando.c_str(),strlen(comando.c_str()));
        return ret;      
    }
    return ret;
}
/*
SIEMPRE QUE EL BUFFER DE EVENTOS EMITIDOS POR EL PANEL NO ESTÉ VACÍO, ESTE MÉTODO PUBLICARÁ EL EVENTO VÍA MQTT
*/
bool pcid_transmitEvent(){
    //Buffer de eventos Contact ID
    if(!queueEventCID->isEmpty(queueEventCID)) { // la cola NO está vacía
        queueEventCID->pull(queueEventCID, &eventCID);
        return true;
    }
    return false;
}