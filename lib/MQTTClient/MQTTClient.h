#ifndef __INTERFACEMQTT_H__
#define __INTERFACEMQTT_H__


#include <PubSubClient.h>

#define SIZE_TOPIC 30 //tamaño del tópico
#define SIZE_PAYLOAD_EXTEND 60
#define SIZE_CID_PAYLOAD 11 //tamaño del payload en CID ejemplo E10000000

#define Retain 1
#define QOS1 1
#define StOFFLINE "TOFFLINE" //mensaje LWT
#define StONLINE "TONLINE" //mensaje cuando conecta a MQTTClient
#define KeepAlive 15*60*1000 // keepalive de MQTTClient en segundos. Se setea en 20, para que cuando se pierda la conexión, se notifique a los 30
#define ReconnectMQTTTime 5000 //tiempo que intento reconectar a MQTTClient

//TÓPICOS
#define subTopicST "ST" 
#define subTopicEP "EP" 
#define subTopicES "ES" 
#define subTopicC "C" 
#define subTopicLOG "LOG"
#define subTopicWifi "WIFI"

#define WiFiSignalTime 60 *1000*240 //Cada 240 minutos se publica la intensidad de la señal wifi,gprs

//Necesario para evitar hacer #include "dscAdapter.h" debido a que este .h ya esta includio MQTTClient.h
//Este tipo de erroe lo resolví leyendo: https://stackoverflow.com/questions/7696022/c-includes-in-a-cycle
//"fordward declaration" segun la gente de recuros teconlógicos
//El compilador ya incluyo la clase, por eso se evita el problema en esta declaración
//class dscAdapter;

//Esta función es clabe para poder usar un método de la clase declarada como fordward (dscAdapter)
//bool llamanteMetodo(dscAdapter* dsc,const char* cadena,byte largo);

class MQTTClient : public PubSubClient {
public:
    MQTTClient(Client& client);
    MQTTClient(Client& client, void (*f)(bool));
    void setupMQTT(String mqttServer1,String mqttServer2, String mqttPortString, String mqttBrokPass);
    bool mqttConnect();
    byte reconnect();
    void setupTopics(String business, String subscriber);//arma los tópicos
    void handle();
    static void mqttCallback(char* topic, unsigned char* payload, unsigned int length);//si no se declara como estática, tira error
    #ifdef useEth
    void mqttEthernetReconnect();
    #endif
    bool transmitEvent(); //Desencola el evento del buffer y lo transmite
    void transmitCommand();//ejecuta comando recibido sobre el panel DSC, tiene que recibir de argumento un objeto dscKeybusInterface o derivados
    bool pubQoS1(String topic);
    void logSend(String payload);    
    void publishWifiSignal(int signalQuality);
private:
    #if defined(ESP8266)
    void mefStatusNoReconnect(); //Estado de led que indica la falta de conexion vía mqtt
    #endif
    void (*callback)(bool);
    String serverHost1;
    String serverHost2;
    String brokerPassw;
    uint16_t Port;
    String ID;

    bool logsOn = 0;

    //CONTADORES PARA RECONECTAR CON EL BROKER EN CASO DE PERDERSE LA CONEXIÓN//
    unsigned long mqttPreviousTime = 0;
    unsigned long mqttCurrentTime = 0;

    //Usados por setUpTopics para poder colocar la empresa y subscriptor en los tópicos
    String topicST;//Topico XXX/NNNN/ST
    static String topicC;//Topico XXX/NNNN/C
    String topicES;//Topico XXX/NNNN/ES
    String topicEP;//Topico XXX/NNNN/EP
    String wifiTopic;//Topico XXX/NNNN/wifi
    String gprsTopic;//Topico XXX/NNNN/gprs
    static String topicLOG;//Topico XXX/NNNN/LOG
    

    bool publishSucceedPub;

    //Estos atributos son usadas por una funcion estática, en consecuencia deben ser declarados estáticos
    //La función estática es la callback
    static bool flagNewPayload;
    static String arrivedPayload;
    static String arrivedTopic;
    static String LOGpublish;

    #ifdef useEth
    byte noReconect=0;
    #endif
    
    byte serverActivo = 0;
    byte reconFallida = 0;
};

#endif // __INTERFACEMQTT_H__