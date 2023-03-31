#include "UDPClient.h"
#include "ProcessCID.h"
#include "GPIO.h"
#include "../../include/ContactID.h"

extern String eventCID;

UDPClient::UDPClient(){};

//udpPort: Puerto que se debe escuchar
void UDPClient::udp_setup(String serverUdp1, String serverUdp2, uint16_t serverUdpPort, uint16_t minutes){
    portLitsen = random(40000,55000);
    server1 = serverUdp1;
    server2 = serverUdp2;
    portServer = serverUdpPort;
    mins = minutes;
    byte randNum = random(0,200);
    serverActivo = (randNum % 2) + 1;
    if(serverActivo == 1) Serial.println("Host actual: " + serverUdp1);
    if(serverActivo == 2) Serial.println("Host actual: " + serverUdp2);
    if(begin(portLitsen) != 1){
        Serial.println("Connexión a UDPClient fallida a puerto local: " + String(portLitsen));
        Serial.println("Se prueba un nuevo puerto");
        udp_setup(serverUdp1, serverUdp2, serverUdpPort, mins);
    }
    else {
        Serial.println("Conexión UDPClient exitosa a puerto: " + String(portLitsen));
        pcid_callback(califRest,comunicationActive,PARTITION_OFF,ZONE_OFF);
        digitalWrite(LED_BUILTIN,HIGHLED); //Dejo prendido el LED para indicar que está conectado
    }
}
void UDPClient::udp_setupAccount(const char* business, const char* subscriber){
    company = String(business);
    for(byte i = 0; i < strlen(subscriber); i++ ){
        account += udp_inttohex(subscriber[i]);
    }
    Serial.print("Cuenta: ");
    Serial.println(subscriber);
    Serial.print("Empresa: ");
    Serial.println(business);
}

void UDPClient::udp_send(String msg){
    if(sendIntens >= 3){
        Serial.print("Se cambia de host por cant de intentos fallidos: ");
        if(serverActivo == 1) {
            serverActivo = 2;
            sendIntens = 0;
            Serial.println(server2);
        }
        else if(serverActivo == 2) {
            serverActivo = 1;
            sendIntens = 0;
            Serial.println(server1);
        }
    }
    if(serverActivo == 1) beginPacket(server1.c_str(), portServer);
    if(serverActivo == 2) beginPacket(server2.c_str(), portServer);
    #if defined(ESP32)
    print(msg.c_str());
    #elif defined(ESP8266)
    write(msg.c_str());
    #endif
    endPacket();
}

//Debe estar dentro de un loop
bool UDPClient::udp_get(){
    delay(10);
    int packetSize = parsePacket();
    if (packetSize)
    {
        // read the packet into packetBufffer
        read(incomingPacket, UDP_RX_PACKET_MAX_SIZE);
        Serial.println();
        Serial.print("Received packet of size ");
        Serial.print(packetSize);
        Serial.print(" from ");
        Serial.print(remoteIP());
        Serial.print(":");
        Serial.println(remotePort());
        Serial.print("Payload: ");
        Serial.print(incomingPacket);
        Serial.println();
        return true;
    }
    else return false;
}

void UDPClient::udp_loop(){
    static unsigned int currentTime = 0, previousTime=0;
    udp_keepAlive();
    if(!eventReadyToSend){
        if(pcid_transmitEvent() && eventCID.charAt(0) != UDP_STATUS_QUALIFER){
            eventReadyToSend = 1;
            udp_buildPayload();
            udp_send(eventToSoftguard);
            currentTime = millis();
            previousTime = currentTime;
        }
    }
    else{
        udp_get();
        if(currentTime - previousTime < UDP_TIME_OUT){
            currentTime = millis();
            if(!strcmp(incomingPacket, validation)){
                Serial.println("Arrivo exitoso de evento a Softguard. SG: ACK");
                strcpy(incomingPacket,"0");
                eventReadyToSend = 0;
                sendIntens = 0;
            }
        }
        else {
            mefStatusNoACK();
            sendIntens++;
            previousTime = currentTime;
            Serial.println("Softguard no respondió, se reintenta");
            udp_send(eventToSoftguard);
        }
    }
}

char UDPClient::udp_inttohex(char input){//expresa un numero de int a HEX
    char output = '0';
    switch (input){
    case '0':
        output=0x0A;
        break;
    case '1':
        output=0x01;
        break;
    case '2':
        output=0x02;
        break;
    case '3':
        output=0x03;
        break;
    case '4':
        output=0x04;
        break;
    case '5':
        output=0x05;
        break;
    case '6':
        output=0x06;
        break;
    case '7':
        output=0x07;
        break;
    case '8':
        output=0x08;
        break;
    case '9':
        output=0x09;
        break;
    case 'B':
        output=0x0B;
        break;
    case 'C':
        output=0x0C;
        break;
    case 'D':
        output=0x0D;
        break;
    case 'E':
        output=0x0E;
        break;
    case 'F':
        output=0x0F;
        break;
    }
	return output;
}

void UDPClient::udp_buildPayload(){
    if(eventCID.charAt(0) == UDP_STATUS_QUALIFER) return; // que solo mande los E y R que son los CID para SoftGuard
    Serial.print("El evento es: ");
    Serial.println(eventCID);
    eventToSoftguard = '@';
    eventToSoftguard += company + "  ";
    eventToSoftguard += account + UDP_HEX_ONE + UDP_HEX_EIGHT;
    if(eventCID[0] == 'E') eventToSoftguard += UDP_HEX_ONE;
    if(eventCID[0] == 'R') eventToSoftguard += UDP_HEX_THREE;
    for(byte i = 1; i < eventCID.length(); i++) eventToSoftguard += udp_inttohex(eventCID[i]);
    eventToSoftguard += UDP_HEX_RELLENO * 4;
}

void UDPClient::udp_keepAlive(){
    static unsigned long currentTimeKA = 0;
    static unsigned long previousTimeKA = 0;
    currentTimeKA = millis();
    if(currentTimeKA - previousTimeKA > mins*60*1000){
        previousTimeKA = currentTimeKA;
        pcid_callback(califEv,testComunicador,PARTITION_OFF,ZONE_OFF);
        Serial.println("Se envía keepAlive a Softguard");
    }
}

#if defined(ESP8266)
void UDPClient::mefStatusNoACK(){
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
#endif