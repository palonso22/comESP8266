#include "AP.h" 
#include "../../include/Constants.h"
#include "LittleFS.h"
#include <ESP8266WebServer.h>
#include "WifiWay.h"
#include "GPIO.h"

unsigned long WiFiPrevTime = 0;
unsigned long previousAPonTime = 0;
unsigned long currentAPonTime= 0;

unsigned short statusCode;
static unsigned char datosValidos=1;//no entiendo porque me da error si lo defino dentro de create web server

String st;
String content;//contiene el código HTML
String wsid;//variables que escriben en la SPIFFS
String wpass;
String wmqttServer1;
String wmqttServer2;
String wmqttPort;
String wbrokerPass;
String wbusiness;
String wsubscriber;
String wminutesKeepAlive;


//VARIABLES

char rsid[32];//variables que leen la memoria SPIFFS
char rpass[64];
char rmqttServer1[32];
char rmqttServer2[32];
char rbrokerPass[32];
char rmqttPort[8];
char rbusiness[8];
char rsubscriber[8];
char rminutesKeepAlive[8];

ESP8266WebServer server(80);


#if defined(useMQTT)
void checkViaMQTT(bool isConnected){
    byte setFail = 0;
    if (WiFi.status() != WL_CONNECTED){
        Serial.println("Fallo de conexion al wifi.");
        setFail++;
    }
    else{
        if(!wifi_pingTest(rmqttServer1)) setFail++;
        if(!wifi_pingTest(rmqttServer2)) setFail++;
        if(!isConnected){
            Serial.println("Fallo en la conexión con el broker.");
            setFail++;
        }
    }
    if(setFail==0) return; //Conexión exitosa
    Serial.println("Acces Point activo");
    WiFiPrevTime=millis();
    setupAP();// Setea punto de acceso
    previousAPonTime=millis();
    currentAPonTime=millis();
    while ((WiFi.status() != WL_CONNECTED) && (currentAPonTime - previousAPonTime < APonTime))//levanta el punto de acceso en caso de que no se pueda conectar y este en la vantana de tiempo
    {
        currentAPonTime=millis();
        serverActive();
        server.handleClient();//mantiene levantado el punto de acceso
    }
    //en caso de que paso la ventana de tiempo y no se conecto tirar el web server e intentar arrancar con los parametros seteados
    Serial.println("Stop Server Web");
    server.stop();
    Serial.println("Stop AP");
    WiFi.softAPdisconnect (true);
    wifi_init(rsid,rpass); //Si llego acá es porque algun problema hubo al iniciar wifi, es remendable que el primer reintento sea con begin
}
#elif defined(useUDP)
void checkViaMQTT(){
    if (WiFi.status() != WL_CONNECTED){
        Serial.println("Fallo de conexion al wifi.");
    }
    else{
        Serial.println("Conectado exitosamente.");
        return;
    }
    Serial.println("Acces Point activo");
    WiFiPrevTime=millis();
    setupAP();// Setea punto de acceso
    previousAPonTime=millis();
    currentAPonTime=millis();
    while ((WiFi.status() != WL_CONNECTED) && (currentAPonTime - previousAPonTime < APonTime))//levanta el punto de acceso en caso de que no se pueda conectar y este en la vantana de tiempo
    {
        currentAPonTime=millis();
        serverActive();
        server.handleClient();//mantiene levantado el punto de acceso
    }
    //en caso de que paso la ventana de tiempo y no se conecto tirar el web server e intentar arrancar con los parametros seteados
    Serial.println("Stop Server Web");
    server.stop();
    Serial.println("Stop AP");
    WiFi.softAPdisconnect (true);
    wifi_init(rsid,rpass); //Si llego acá es porque algun problema hubo al iniciar wifi, es remendable que el primer reintento sea con begin
}
#endif

void setupAP(void){
    unsigned char n;
    unsigned char i;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    n = WiFi.scanNetworks();//se escanean todas las redes wifi disponibles y devuelve la cantidad de redes escaneadas
    if (n == 0)
        Serial.println("no networks found");
    else{
        Serial.print(n);
        Serial.println(" networks found");
        for (i = 0; i < n; ++i){
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println("");
            delay(10);
        }
    }
    Serial.println("");
    st = "<ol>";
    for (i = 0; i < n; ++i){
        // Print SSID and RSSI for each network found
        st += "<li>";
        st += WiFi.SSID(i);
        st += " (";
        st += WiFi.RSSI(i); 
        st += ")";
        Serial.println("");
        //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
        st += "</li>";
    }
    st += "</ol>";
    delay(100);
    WiFi.softAP("AccesPointRecursos", "");
    Serial.print("Connect to AP");
    Serial.println("AccesPointRecursos");
    launchWeb();
}

void launchWeb(){
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED) Serial.println("AP levantado");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("GO to the SoftAP IP in a browser");
    Serial.print("SoftAP IP: ");
    Serial.println(WiFi.softAPIP());//setea la IP 192.168.4.1
    createWebServer();
    server.begin();//levanta el servidor
    Serial.println("WebServer started");
}
void createWebServer(){
    //unsigned char datosValidos=1;
    server.on("/", []() {
        IPAddress ip= WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>\r\n<html>";        
        content += String("<h1>")+  String(SoftwareName)+ " "+ String(SoftwareVersion) +String("</h1>");
        content += "<h2>Por favor, introduzca los parametros de inicializacion para una correcta comunicacion con monitoreo </h2>";        
        content += ipStr;
        content += "<p>";
        content += st;
        content += "</p><form method='post' action='setting'>";
        #ifdef useGprs
            content += "<label>APN: </label><input name='apn' length=50><br><label>User: </label><input name='user' length=20><br><label>Pass: </label><input name='passW' length=20><br>";
        #endif
        content += "<label>SSID: </label><input name='ssid' length=32><br><label>Password: </label><input name='pass' length=64><br>";
        content += "<label>Servidor de monitoreo 1: </label><input name='mqttServer1' length=32><br><label>Servidor de monitoreo 2: </label><input name='mqttServer2' length=32><br><label>Puerto: </label><input name='mqttPort' length=64><br>";
        #if defined(useMQTT)
        content += "<label>Pass del Servidor: </label><input name='brokerPass' length=32><br>";
        #elif defined(useUDP)
        content += "<label>Tiempo de Keep Alive (minutos): </label><input name='keepAliveMins' length=8><br>";
        #endif
        //content +=  "<label>Empresa: </label><input name='business' length=64><br><input type='submit'></form>";
        content +=  "<label>Cuenta: </label><input name='subscriber' length=64><br><label>Empresa: </label><input name='business' length=64><br><input type='submit'></form>";
        content += "</html>";
        server.send(200, "text/html", content);
    });    
    server.on("/setting", []() {
        wmqttServer1 = server.arg("mqttServer1"); //32
        wmqttServer2 = server.arg("mqttServer2"); //32
        wmqttPort =  server.arg("mqttPort"); //8
        #if defined(useMQTT)
        wbrokerPass = server.arg("brokerPass");
        #elif defined(useUDP)
        wminutesKeepAlive = server.arg("keepAliveMins");
        #endif
        wbusiness = server.arg("business"); //8
        wsubscriber = server.arg("subscriber");//8
        wsid = server.arg("ssid");
        wpass = server.arg("pass");
        if(!(wsid.length() > 0 && wpass.length() > 0)){
            datosValidos=0;
        }
        #if defined(useMQTT)
        if(!(wbrokerPass.length()> 0)) datosValidos = 0;
        #elif defined(useUDP)
        if(!(wminutesKeepAlive.length()> 0)) datosValidos = 0;
        #endif
        if (wmqttServer1.length()> 0 && wmqttServer2.length()> 0 && wmqttPort.length()> 0  && wbusiness.length()> 0 && wsubscriber.length()> 0 && datosValidos==1) {
            writeSPIFFS();
            content = "<!DOCTYPE HTML>\r\n<html>";                
            content += "<p>";
            content += "Seteo de parametros almacenados en la memoria flash con exito...";        
            content += "</p>";        
            content += "</html>";            
            server.send(200, "text/html", content);
            delay(10000);
            Serial.println("");
            Serial.println("Stop Server Web");
            server.stop();
            Serial.println("Stop Acces Point");
            WiFi.softAPdisconnect (true);
            Serial.println("Reseteando comunicador...");
            ESP.restart();
            return;
        } 

        content = "<!DOCTYPE HTML>\r\n<html>ERROR";                
        content += "<p>";
        content += "Alguno de los campos no fue completado correctamente por favor resetee el comunicador...";        
        content += "</p>";        
        content += "</html>";            
        server.send(200, "text/html", content);                
    }); 
}

void writeSPIFFS(){ //escribe en la SPIFFS, cada vez que escribe borra lo que estaba anteriormente escrito
    if(LittleFS.begin()){
    }else{
        Serial.println("!An error occurred during SPIFFS mounting");
    }
    LittleFS.format();
    File file = LittleFS.open("/parametros.txt", "w");//Abro el archivo para leer y escribir, el puntero se posiciona al principio del archivo
    if(!file){
        // File not found
        Serial.println("Failed to open test file");
        return;
    } else {
        file.print(Server1);
        file.print("=");
        file.println(wmqttServer1);
        file.print(Server2);
        file.print("=");
        file.println(wmqttServer2);
        file.print(PuertoMon);
        file.print("=");
        file.println(wmqttPort);
        file.print(Empr);
        file.print("=");
        file.println(wbusiness);
        file.print(Subs);
        file.print("=");
        file.println(wsubscriber);
        file.print(idWifi);
        file.print("=");
        file.println(wsid);
        file.print(passwWifi);
        file.print("=");
        file.println(wpass);
        #if defined(useMQTT)
        file.print(BrokPass);
        file.print("=");
        file.println(wbrokerPass);
        #elif defined(useUDP)
        file.print(minKA);
        file.print("=");
        file.println(wminutesKeepAlive);
        #endif
        file.close();
    }
}

void readSPIFFS(){
	unsigned char store=0, i=0, buff; //i: indice de posición de almacenamiento en arreglos | buff: caracter extraído de la función read
	char data[30]; //data: contiene el identificador de parámetro
	Serial.println("Cargando parámetros de FS");

    if (LittleFS.begin()) Serial.println("Montaje de parámetros exitoso.");
    else {Serial.println("Error ocurrido durante el montaje SPIFFS mounting"); ESP.restart();}
    File file = LittleFS.open("/parametros.txt", "r"); //Abro el archivo para solo lectura 
    if(!file) {Serial.println("Failed to open test file");}

	//Se lee caracter por caracter el archivo (esto es posible por poner el tercer argumento de read en 1)
	while (file.available()){
    buff = file.read();
		if(store == 0) {
			if(buff != '=') { //Se almacena el identificador de parámetro en data
				data[i]=buff;
				i++;
			}
			else { //Llegado a este punto, implica el identificador de parámetro fue complétamente almacenado
				store=1; //Se habilita el almacenamiento del valor del parámetro
				data[i]='\0';
				i=0; //Se coloca en 0 para poder reutilizar dicha variable
			}
		}

		//De acuerdo al identificador de parámetro (serverMqtt, PuertoMqtt, etc...), se almacenará el valor
		//en la variable que corresponda
		if (store == 1 && buff != '='){
			//Se almacena el ServidorMQTT 1
			if(strcmp(data,Server1)==0){
				if(buff == '\n') { //Cuando llega hasta acá, implica que se almacenó el valor completo
					rmqttServer1[i-1]='\0';
					store = 0; //Preparo para identificar nuevo parámetro
					i=0;
				}
				else{
					rmqttServer1[i]=buff;
					i++;
				}
			}
            //Se almacena el ServidorMQTT 2
			if(strcmp(data,Server2)==0){
				if(buff == '\n') { //Cuando llega hasta acá, implica que se almacenó el valor completo
					rmqttServer2[i-1]='\0';
					store = 0; //Preparo para identificar nuevo parámetro
					i=0;
				}
				else{
					rmqttServer2[i]=buff;
					i++;
				}
			}
			//Se almacena el PuertoMQTT
			if(strcmp(data,PuertoMon)==0){
				if(buff == '\n') { //Cuando llega hasta acá, implica que se almacenó el valor completo
					rmqttPort[i-1]='\0';
					store = 0; //Preparo para identificar nuevo parámetro
					i=0;
				}
				else{
                    rmqttPort[i]=buff;
					i++;
				}
			}
            #if defined(useMQTT)
            //Se almacena el pass contra mqtt
			if(strcmp(data,BrokPass)==0){
				if(buff == '\n') { //Cuando llega hasta acá, implica que se almacenó el valor completo
					rbrokerPass[i-1]='\0';
					store = 0; //Preparo para identificar nuevo parámetro
					i=0;
				}
				else{
                    rbrokerPass[i]=buff;
					i++;
				}
			}
            #elif defined(useUDP)
            //Se almacena el keep alive de udp
			if(strcmp(data,minKA)==0){
				if(buff == '\n') { //Cuando llega hasta acá, implica que se almacenó el valor completo
					rminutesKeepAlive[i-1]='\0';
					store = 0; //Preparo para identificar nuevo parámetro
					i=0;
				}
				else{
                    rminutesKeepAlive[i]=buff;
					i++;
				}
			}
            #endif
			//Se almacena el ID de wifi
			if(strcmp(data,idWifi)==0){
				if(buff == '\n') { //Cuando llega hasta acá, implica que se almacenó el valor completo
					rsid[i-1]='\0';
					store = 0; //Preparo para identificar nuevo parámetro
					i=0;
				}
				else{
					rsid[i]=buff;
					i++;
				}
			}
			//Se almacena el pass de wifi
			if(strcmp(data,passwWifi)==0){
				if(buff == '\n') { //Cuando llega hasta acá, implica que se almacenó el valor completo
					rpass[i-1]='\0';
					store = 0; //Preparo para identificar nuevo parámetro
					i=0;
				}
				else{
					rpass[i]=buff;
					i++;
				}
			}
			//Se almacena la empresa
			if(strcmp(data,Empr)==0){
				if(buff == '\n') { //Cuando llega hasta acá, implica que se almacenó el valor completo
					rbusiness[i-1]='\0';
					store = 0; //Preparo para identificar nuevo parámetro
					i=0;
				}
				else{
					rbusiness[i]=buff;
					i++;
				}
			}
      //Se almacena el susbcriptor
			if(strcmp(data,Subs)==0){
				if(buff == '\n') { //Cuando llega hasta acá, implica que se almacenó el valor completo
					rsubscriber[i-1]='\0';
					store = 0; //Preparo para identificar nuevo parámetro
					i=0;
				}
				else{
					rsubscriber[i]=buff;
					i++;
				}
			}
		}
	}
	//Cierro el archivo (libero el file descriptor)
	file.close();
	Serial.print("Servidor de monitoreo 1: ");
	Serial.println(rmqttServer1);
    Serial.print("Servidor de monitoreo 2: ");
	Serial.println(rmqttServer2);
	Serial.print("Puerto: ");
	Serial.println(rmqttPort);
    #if defined(useMQTT)
    Serial.print("Pass contra el broker: ");
	Serial.println(rbrokerPass);
    #elif defined(useUDP)
    Serial.print("Tiempo de keep alive: ");
	Serial.println(rminutesKeepAlive);
    #endif
	Serial.print("Ssid: ");
	Serial.println(rsid);
	Serial.print("Pass: ");
	Serial.println(rpass);
	Serial.print("Empresa: ");
	Serial.println(rbusiness);
	Serial.print("Cliente: ");
	Serial.println(rsubscriber);
}
void serverActive(){
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
                status = 0;
            }
        break;
    }
}
