#include "MQTTClient.h"
#include "GPIO.h"
#include <ESP8266WiFi.h>
#include "ProcessCID.h"
#include "../../include/Comandos.h"
#include "../../include/ContactID.h"
#include "../../include/Constants.h"

//LOS ATRIBUTOS DE CARACTER ESTÁTICOS, DEBEN SER LLAMADOS EN .CPP DE ESTA MANERA PARA PODER SER UTILIZADOS
bool MQTTClient::flagNewPayload=0;
String MQTTClient::arrivedPayload;
String MQTTClient::arrivedTopic;
String MQTTClient::LOGpublish;
String MQTTClient::topicLOG;
String MQTTClient::topicC;

static String eventDSCrx;
extern String eventCID; //Declarada en Process CID

MQTTClient::MQTTClient(Client& client):PubSubClient(client){}
MQTTClient::MQTTClient(Client& client, void (*f)(bool)):PubSubClient(client){
    callback=f;
}

void llamanteLog(MQTTClient* mqtt, String msgLog){
    mqtt->logSend(msgLog);
}

/*
ACÁ SE SETEAN LOS PARÁMETROS DE CONEXIÓN, HAY QUE ASEGURARSE DE QUE ESTE CONECTADO A LA RED ANTES DE LLAMARLA
*/
void MQTTClient::setupMQTT(String mqttserverHost1,String mqttserverHost2, String mqttPortString, String mqttBrokPass){
    uint16_t mqttPort=atoi(mqttPortString.c_str());
    setKeepAlive(KeepAlive); //Tiempo en el cual le va a mandar un pinrequest al broker para que el mismo sepa si sigue conectado este cliente
    setCallback(mqttCallback);
    byte randNum = random(0,200);
    serverActivo = (randNum % 2) + 1;
    serverHost1 = mqttserverHost1;
    serverHost2 = mqttserverHost2;
    if(serverActivo == 1) setServer(serverHost1.c_str(),mqttPort);
    if(serverActivo == 2) setServer(serverHost2.c_str(),mqttPort);
    Port = mqttPort;
    brokerPassw = mqttBrokPass;
    if(!mqttConnect()){ //Acá se conecta, si devuelve verrdadero es porqe se pudo conectar exitosamente, falsto en caso contrario
        if(serverActivo == 1){
            serverActivo = 2;
            setServer(serverHost1.c_str(),mqttPort);
        }
        if(serverActivo == 2){
            serverActivo = 1;
            setServer(serverHost2.c_str(),mqttPort);
        }
        Serial.println("Intento de conexión al primer host fallido, pruebo en el otro");
        mqttConnect();
    }
    mqttPreviousTime = millis();
}
/*
CONECTA CON EL BROKER
*/
bool MQTTClient::mqttConnect() {
    if (connect(ID.c_str(),ID.c_str(),brokerPassw.c_str(),topicST.c_str(),QOS1,Retain,StOFFLINE)) {
        subscribe(topicST.c_str());
        unsigned long preTime,currTime=millis();
        preTime = millis();
        Serial.println("Espero máximo 10 segundo para recibir el estado previo de conexión");
        while(currTime - preTime < 10000 && arrivedPayload != StONLINE && arrivedPayload != StOFFLINE){
            loop();
            currTime = millis();
        }
        if(arrivedPayload != "TONLINE"){
            publish(topicST.c_str(),StONLINE,Retain);
            Serial.println("Se publica TONLINE");
        }
        else Serial.println("No se publica TONLINE");
        publishWifiSignal(WiFi.RSSI());
        Serial.print("Conexión MQTTClient online. Servidor: ");
        if(serverActivo == 1) Serial.print(serverHost1);
        if(serverActivo == 2) Serial.print(serverHost2);
        Serial.print(". Puerto: ");
        Serial.println(Port);
        subscribe(topicC.c_str(),QOS1);
        subscribe(topicEP.c_str(),QOS1);
        digitalWrite(LED_BUILTIN,HIGHLED); //Dejo prendido el LED para indicar que está conectado
    }
    else {
        Serial.print("Conexion MQTTClient fallida a: ");
        if(serverActivo == 1) Serial.print(serverHost1);
        if(serverActivo == 2) Serial.print(serverHost2);
        Serial.print(". Puerto: ");
        Serial.println(Port);
    }
    return connected();
}
/*
SETEO DE TÓPICOS
*/
void MQTTClient::setupTopics(String business, String subscriber){
    // ID para el broker
    ID = business + subscriber;
    // Tópicos de pub y sub
    topicST = business + "/" + subscriber + "/" + subTopicST;// XXX/NNNN/ST
    topicEP = business + "/" + subscriber + "/" + subTopicEP; // XXX/NNNN/SP
    topicES = business + "/" + subscriber + "/" + subTopicES;// XXX/NNNN/ES
    topicC = business + "/" + subscriber + "/" + subTopicC;// XXX/NNNN/C
    topicLOG = business + "/" + subscriber + "/" + subTopicLOG;// XXX/NNNN/LOG
    wifiTopic = business + "/" + subscriber + "/" + subTopicWifi;// XXX/NNNN/WiFi
}
/*
Retorna false, para indicar que falló en reconectarse
*/
byte MQTTClient::reconnect(){
    mefStatusNoReconnect();
    mqttCurrentTime = millis(); //Si no está conectado, guarda el tiempo en que perdió la conexión
    if (mqttCurrentTime - mqttPreviousTime > ReconnectMQTTTime) { //Cuando pase un tiempo ReconnectMQTTTime, intenta reconectarse
        Serial.println(mqttCurrentTime - mqttPreviousTime);
        mqttPreviousTime = mqttCurrentTime;
        Serial.println("Reconectando MQTTClient...");
        if (mqttConnect()) {
            logSend("Reconexion a MQTTClient exitosa");
            reconFallida = 0;
            return 1;
        }
        else {
            delay(5000);
            if(reconFallida > 2){ //cambio de host en caso de fallar 3 veces con 1
                if(serverActivo == 1) {
                    setServer(serverHost2.c_str(),Port);
                    serverActivo = 2;
                    reconFallida = 0;
                }
                else if(serverActivo == 2) {
                    setServer(serverHost1.c_str(),Port);
                    serverActivo = 1;
                    reconFallida = 0;
                }
            }
            reconFallida ++;
            return 0;
        }
    }
    return 2;
}
/*
RECONECTA EN CASO DE QUE SE PIERDA LA CONEXIÓN, HAY QUE ASEGURARSE QUE ESTÉ CONECTADO A LA RED ANTES DE CONVOCARLO
*/
void MQTTClient::handle() {
    transmitEvent();
    loop(); //En caso de estar conectado con el broker, llama a callback. Además, permite mandar el pin request cada KeepAlive segundos.
    pcid_loop();
}
/*
FUNCIÓN CALLBACK QUE RECIBE COMANDOS, CUANDO ES CONVOCADO
*/
void MQTTClient::mqttCallback(char* topic, byte* payload, unsigned int length){
    unsigned char i;
    if(length > 9) return; //Si el payload es mayor, rompe el programa, además no hay ninguno mayor a este tamaño, ya se de +/+/C o +/+/EP
    arrivedPayload = (char)payload[0];
    for (i = 1; i < length; i++) {
        arrivedPayload += (char)payload[i]; //Se almacena el payload
    }
    arrivedTopic = topic;
    Serial.print("Message arrived from -> ");
    Serial.print(arrivedTopic);
    Serial.print(" Payload -> ");
    Serial.println(arrivedPayload);
    flagNewPayload=1;// Cada vez que llega un mensaje llamo a transmitCommand() para ejecutar el comando sobre el panel
    if(arrivedTopic == topicC){
        pcid_recibComand(arrivedPayload);
    } 
}
/*
LLAMADO POR POLLING EN EL MAIN, CADA VEZ QUE HAYA UN NUEVO COMANDO QUE HAYA RECIBIDO EL COMUNICADOR, ESTA FUNCIÓN
LLAMARÁ A LA CAPA SW DEL PANEL PARA REALIZAR SU EJECUCIÓN 
*/
void MQTTClient::transmitCommand(){
    if(flagNewPayload ==1 && (arrivedTopic == topicC)){
        logSend("Comando recibido");
        if(arrivedPayload == logActive){
            Serial.println("Activo logs");
            logsOn = true;
            logSend("Activo logs");
            flagNewPayload = 0;
            return;
        }
        if(arrivedPayload == logDesactive){
            Serial.println("Desactivo logs");
            logsOn = false;
            flagNewPayload = 0;
            return;
        }
        if(arrivedPayload == restCommand){
            logSend("Reseteando comunicador");
            delay(1000);
            Serial.println("Reseteando comunicador");
            ESP.restart();
        }
        if(arrivedPayload == version){
            Serial.println("Devuelvo versión");
            publishSucceedPub=publish(topicLOG.c_str(),SoftwareVersion, false);
            flagNewPayload = 0;
            return;
        }
        flagNewPayload=0;
        byte succesExecute = pcid_transmitComand();
        if(succesExecute == 0) LOGpublish = "Comando ejecutado exitosamente";
        if(succesExecute == 1) LOGpublish = "Comando no reconocido";
        if(succesExecute == 2) LOGpublish = "Formato de comando incorrecto exitosamente";
        logSend(LOGpublish);
    }
}
/*
SIEMPRE QUE EL BUFFER DE EVENTOS EMITIDOS POR EL PANEL NO ESTÉ VACÍO, ESTE MÉTODO PUBLICARÁ EL EVENTO VÍA MQTTClient
*/
bool MQTTClient::transmitEvent(){ 
    static byte waitArrivedTopicEP = 0;
    if(connected() && !waitArrivedTopicEP && pcid_transmitEvent()){
        eventDSCrx = eventCID;
        if(eventDSCrx.charAt(0) == charE || eventDSCrx.charAt(0) == charR){
            Serial.print("Publish message MQTTClient Contact ID. ");
            Serial.print("Topic: ");
            Serial.print(topicEP);
            Serial.print(" Payload: ");
            Serial.println(eventDSCrx);
            logSend(eventDSCrx);
            waitArrivedTopicEP = 1;
        }
        if(eventDSCrx.charAt(0) == charS){
            Serial.println("Publish message MQTTClient status");
            Serial.print("Topic: ");
            Serial.println(topicES);
            Serial.print("Payload: ");
            Serial.println(eventDSCrx);
            logSend(eventDSCrx);
            publishSucceedPub=publish(topicES.c_str(),eventDSCrx.c_str(), false);
        }
        return publishSucceedPub;
    }
    if(waitArrivedTopicEP){
        if(pubQoS1(topicEP)) waitArrivedTopicEP = 0;
    }
    return false;
}
/*
Publica el nivel de intensidad wifi cada 1 hora
*/
void MQTTClient::publishWifiSignal(int signal){    
    Serial.println("Publicando señal de wifi: "+String(signal));
    String payload = String(signal);//publico cada cierto tiempo la señal wifi
    publish(wifiTopic.c_str(),payload.c_str()); 
}


/* 
Publico evento prioritarios con QoS = 1, es decir que me aseguro que siempre arrive a monitoreo todos mensajes
*/    
bool MQTTClient::pubQoS1(String topic){
    static byte state = 0;
    static String payloadPriority;
    static String topicPriority;
    static String arrivedPayloadVerify;
    static unsigned long previousTime = 0;
    switch(state){
        case 0:{
            topicPriority = topic;
            state = 1;
            break;
        }
        case 1:{
            payloadPriority = eventDSCrx;
            state = 2;
            break;
        }
        case 2:{
            if(publish(topicPriority.c_str(),payloadPriority.c_str(),false)){
                previousTime = millis();
                state = 3;
            }
            else state=0;
            break;
            
        }
        case 3:{
            arrivedPayloadVerify = arrivedPayload;
            if(payloadPriority == arrivedPayloadVerify){
                state = 1;
                Serial.println("ACK DE EVENTO PRIORITARIO RECIBIDO");
                return true;
            }
            else{
                if(millis() - previousTime > 15000){
                    previousTime = millis();
                    if(!connected()) break;
                    state = 4;
                }
            }
            break;
        }
        case 4:{
            Serial.println("DISCONECT");
            disconnect();
            state = 0;
            break;
        }
    }
    return false;
}
/*
ESTE ES UN LOG PARA VER A TRAVÉS DE UN SUBSCRIPTOR, ÚTIL PARA SUJETOS QUE ESTE INVOLUCRADOS EN EL PROYECTO SIN VER ESTE CÓDIGO
*/
void MQTTClient::logSend(String payload){
    if(logsOn){
        LOGpublish = payload;
        if(payload.substring(0,4) == EmerMedicaEv) LOGpublish = "Emergencia medica ON";
        if(payload.substring(0,4) == EmerMedicaRest) LOGpublish = "Emergencia medica OFF";
        if(payload.substring(0,4) == IncendioEv) LOGpublish = "Incendio ON";
        if(payload.substring(0,4) == IncendioRest) LOGpublish = "Incendio OFF";
        if(payload.substring(0,4) == PanicoEv) LOGpublish = "Panico ON";
        if(payload.substring(0,4) == PanicoRest) LOGpublish = "Panico OFF";
        if(payload.substring(0,4) == CoaccionCodEv) LOGpublish = "CODIGO DE COACCION EJECUTADO";
        if(payload.substring(0,4) == AlarmaEv) LOGpublish = "Alarma en particion " + String(payload.charAt(5)) + " por activacion de zona: " + String(payload.charAt(7)) + String(payload.charAt(8));
        if(payload.substring(0,4) == AlarmaRest) LOGpublish = "Alarma RESTAURADA en particion " +  String(payload.charAt(5)) + " por activacion de zona: " + String(payload.charAt(7)) + String(payload.charAt(8));
        if(payload.substring(0,4) == Perdida220Ev) LOGpublish = "Perdida de alimentacion de red (220V)";
        if(payload.substring(0,4) == Perdida220Rest) LOGpublish = "Recuperacion de alimentacion de red (220V)";
        if(payload.substring(0,4) == BatBajaEv) LOGpublish = "Perdida de bateria auxiliar";
        if(payload.substring(0,4) == BatBajaRest) LOGpublish = "Recuperacion de bateria auxiliar";
        if(payload.substring(0,4) == CampanaProblEv) LOGpublish = "Problema en campanilla";
        if(payload.substring(0,4) == CampanaProblRest) LOGpublish = "Problema en campanilla restaurado";
        if(payload.substring(0,4) == ArmDesarmEv) LOGpublish = "Desarmado de particion " + String(payload.charAt(5)) + " por usuario " + String(payload.charAt(7)) + String(payload.charAt(8));
        if(payload.substring(0,4) == ArmDesarmRest) LOGpublish = "Armado Ausente de particion " + String(payload.charAt(5)) + " por usuario " + String(payload.charAt(7)) + String(payload.charAt(8));
        if(payload.substring(0,4) == ArmParcialEv) LOGpublish ="Armado Parcial de particion" + String(payload.charAt(5));
        if(payload.substring(0,4) == ArmStayRest) LOGpublish = "Armado Presente de particion " + String(payload.charAt(5)) + " por usuario " + String(payload.charAt(7)) + String(payload.charAt(8));
        if(payload.substring(0,4) == ByPassEv) LOGpublish = "Bypass de zona "+ String(payload.charAt(7)) + String(payload.charAt(8));
        if(payload.substring(0,4) == TestPeriodEv) LOGpublish = "Test periodico";
        if(payload.substring(0,4) == ModProgEv) LOGpublish = "Entrada a modo de programacion";
        if(payload.substring(0,4) == ModProgRest) LOGpublish = "Salida de modo de programacion";
        if(payload.substring(0,4) == outputCIDEv) LOGpublish = "Salida digital " + String(payload.charAt(8)) + " ON";
        if(payload.substring(0,4) == outputCIDRest) LOGpublish = "Salida digital " + String(payload.charAt(8)) + " OFF";
        if(payload.substring(0,4) == input1CIDEv) LOGpublish = "Entrada digital 1 ON";
        if(payload.substring(0,4) == input1CIDRest) LOGpublish = "Entrada digital 1 OFF";
        if(payload.substring(0,4) == input2CIDEv) LOGpublish = "Entrada digital 2 ON";
        if(payload.substring(0,4) == input2CIDRest) LOGpublish = "Entrada digital 2 OFF";
        if(payload.substring(0,4) == input3CIDEv) LOGpublish = "Entrada digital 3 ON";
        if(payload.substring(0,4) == input3CIDRest) LOGpublish = "Entrada digital 3 OFF";
        if(payload.substring(0,4) == PGMEv) LOGpublish = "PGM " + String(payload.charAt(8)) + " ON";
        if(payload.substring(0,4) == PGMRest) LOGpublish= "PGM " + String(payload.charAt(8)) + " OFF";
        if(payload.substring(0,4) == statusArmed && payload.charAt(8) == '1') LOGpublish = "La particion " + String(payload.charAt(5)) +  " esta armada en Modo Ausente";
        if(payload.substring(0,4) == statusArmed && payload.charAt(8) == '0') LOGpublish = "La particion " + String(payload.charAt(5)) +  " esta armada en Modo Presente";
        if(payload.substring(0,4) == partitionReady && payload.charAt(8) == '1') LOGpublish = "Particion " + String(payload.charAt(5)) +  " lista para armar";
        if(payload.substring(0,4) == partitionReady && payload.charAt(8) == '0') LOGpublish = "Particion " + String(payload.charAt(5)) +  " no se encuentra lista para armar";
        if(payload.substring(0,4) == invalidAcces) LOGpublish = "Codigo de acceso invailido";
        if(payload.substring(0,4) == EvSabSist) LOGpublish = "Sabotaje del sistema. Slot = " +  String(payload.charAt(7)) + String(payload.charAt(8));
        if(payload.substring(0,4) == RestSabSist) LOGpublish = "Restauracion de Sabotaje del sistema. Slot = " + String(payload.charAt(7)) + String(payload.charAt(8));
        if(payload.substring(0,4) == PerdDCModExp) LOGpublish = "Perdida de alimentacion auxiliar DC de modulo expansor";
        if(payload.substring(0,4) == RecupDCModExp) LOGpublish = "Recuperacion de alimentacion auxiliar DC de modulo expansor";
        if(payload.substring(0,4) == PerdACModExp) LOGpublish = "Perdida de alimentación auxiliar AC de modulo expansor";
        if(payload.substring(0,4) == RecupACModExp) LOGpublish ="Recuperacion de alimentacion auxiliar AC de modulo expansor";
        if(payload.substring(0,4) == openZonesLog) LOGpublish = "Zonas abiertas";
        if(payload.substring(0,4) == partitionExitDelay && payload.charAt(8) == '1') LOGpublish = "Particion " + String(payload.charAt(5)) + " en exit delay";
        if(payload.substring(0,4) == partitionExitDelay && payload.charAt(8) == '0') LOGpublish = "Fin de exit delay de la particion " + String(payload.charAt(5));


        publish(topicLOG.c_str(),LOGpublish.c_str());
    }
}
/*
Indicación de pérdida de comunicación MQTTClient mediante LED
*/
void MQTTClient::mefStatusNoReconnect(){
    static byte cont = 0;
    static byte ledOn = 0; //Conmienza con el LED apagado
    while(cont < 16){
        if(ledOn){
            digitalWrite(LED_BUILTIN, HIGHLED);
            ledOn = 0;
                delay(100);
        }
        else{
            digitalWrite(LED_BUILTIN, LOWLED);
            ledOn = 1;
                delay(100);
            }
        cont ++;
    }
    cont = 0;
    digitalWrite(LED_BUILTIN, HIGHLED);
}