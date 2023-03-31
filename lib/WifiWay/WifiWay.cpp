#include "WifiWay.h"
#include "GPIO.h"
#include <ESP8266Ping.h>


unsigned long wifiPreviousTime;//contador para enviar la señal wifi
unsigned long wifiCurrentTime;
byte beginOK = 0; //Se pone en uno, cuando se conecta a wifi por begin
char _ssid[32];//variables que leen la memoria SPIFFS
char _pass[64];
static byte state=0;

void pcid_callback(char qualifier,const String event,const String partition,const String zone_user);

void wifi_init(char* ssid, char* pass){
    beginOK = 0;
    WiFi.disconnect(); //Elimino cualquier configuración previa que haya quedado
    strcpy(_ssid,ssid);
    strcpy(_pass,pass);
    delay(1500);
    Serial.println();
    Serial.println("Comienza comunicación vía wifi");
    WiFi.begin(_ssid, _pass);//intenta conectarse al wifi
    if (wifi_connect()){
        Serial.print(F("Conexion exitosa con IP: "));
        Serial.println(WiFi.localIP());
        Serial.print("DNS: ");
        Serial.println(WiFi.dnsIP());
        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("Máscara de subred: ");
        Serial.println(WiFi.subnetMask());
        beginOK = 1;
    }
    else {
        Serial.println("No conectó");
        Serial.println("No logra conectarse al wifi en el primer intento");
    }
}

bool wifi_connect(void)//espera cierto tiempo para conectarse al wifi
{
    unsigned char c = 0;
    Serial.println("Espere por la conexion WiFI");
    wifiCurrentTime=millis();
    while (c<50){ //Aprox 25 segs dentro de este loop
        wifi_statusNoReconnectWifi();
        delay(50);
        wifiCurrentTime=millis();
        if(wifiCurrentTime - wifiPreviousTime > WIFI_TIME_WAIT_REQUEST){
            wifiPreviousTime = wifiCurrentTime;
            Serial.print(".#");
            if(wifi_isConnected()){
                Serial.println();
                state = 1;
                return true;
            }
            c++;
        }
    }
    Serial.println();
    Serial.println("Tiempo de espera de conexion superado WiFi");
    Serial.println("No conectado vía WiFI :(");
    return false;
}
bool wifi_isConnected(){
    static byte state=0, stateChange=0;
    if(WiFi.status() == WL_CONNECTED){
        if(!state) stateChange = 1;
        state = 1;
        if(stateChange){
            //pcid_callback('R',"360","00","000");
            stateChange = 0;
        }
        return true;
    }
    else{
        if(state) stateChange = 1;
        state = 0;
        if(stateChange){
            //pcid_callback('E',"360","00","000");
            stateChange = 0;
        }
        return false;
    }
}

void wifi_reconnect(){
    wifi_statusNoReconnectWifi();
    wifiCurrentTime=millis();
    if (wifiCurrentTime - wifiPreviousTime > WIFI_TIME_RECONNECT) {
        wifiPreviousTime = wifiCurrentTime;
        if(beginOK == 1) WiFi.reconnect();
        else{
            WiFi.disconnect();
            WiFi.begin(_ssid, _pass);
            Serial.println("Nunca conecto con el begin antes, se reintenta");
        }
        if (wifi_connect()) {
            if(!beginOK) beginOK = 1;//intenta conectarse al wifi
            Serial.println("WiFi desconectado, reconexión exitosa.");
            wifiPreviousTime = 0;
            digitalWrite(LED_BUILTIN,HIGHLED);
        }     
    }

}


bool wifi_pingTest(String host){
    bool success = Ping.ping(host.c_str(), 3);
    if(!success){
        Serial.println("Ping fallido a "+ host);
        return false;
    }
    Serial.println("Ping exitoso a " + host);
    return true;
}



int wifiSignal(){
    return WiFi.RSSI();        
}

bool wifiWaitForAction(const unsigned long timer){
    static unsigned long wifiPreviousTime=0;
    unsigned long currentTime = millis();
    if (currentTime - wifiPreviousTime > timer) {
        wifiPreviousTime = currentTime;
        return true;
    }
    else return false;
}

void wifi_statusNoReconnectWifi(){
    static byte status=0;
    static unsigned int ledPreviousTime=0;
    static unsigned int ledCurrentTime;
    
    ledCurrentTime=millis();
    
    switch (status)
    {
        case 0:
            digitalWrite(LED_BUILTIN, HIGHLED);
            if((ledCurrentTime - ledPreviousTime) > windowLed){
                ledPreviousTime = ledCurrentTime;
                status = 1;
            }
        break;
        case 1:
            digitalWrite(LED_BUILTIN, LOWLED);
            if((ledCurrentTime - ledPreviousTime) > windowLed){
                ledPreviousTime = ledCurrentTime;
                status = 2;
            }
        break;
        case 2:
            digitalWrite(LED_BUILTIN, HIGHLED);
            if((ledCurrentTime - ledPreviousTime) > windowLed){
                ledPreviousTime = ledCurrentTime;
                status = 3;
            }
        break;
        case 3:
            digitalWrite(LED_BUILTIN, LOWLED);
            if((ledCurrentTime - ledPreviousTime) > windowLed){
                ledPreviousTime = ledCurrentTime;
                status = 4;
            }
        break;
        case 4:
            digitalWrite(LED_BUILTIN, HIGHLED);
            if((ledCurrentTime - ledPreviousTime) > windowLed){
                ledPreviousTime = ledCurrentTime;
                status = 5;
            }
        break;
        case 5:
            digitalWrite(LED_BUILTIN, LOWLED);
            if((ledCurrentTime - ledPreviousTime) > windowOnOffOn){
                ledPreviousTime = ledCurrentTime;
                status = 0;
            }
        break;
    }
}