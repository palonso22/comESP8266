#include <WiFiUdp.h>
#include <Arduino.h>

#define UDP_RX_PACKET_MAX_SIZE 11
#define UDP_STATUS_QUALIFER 'S'
#define UDP_TIME_OUT 5000 //Tiempo de espera de respuesta de Softguard

#define UDP_HEX_ONE (char)0x01 //numero 1 en hexadecimal
#define UDP_HEX_THREE (char)0x03 //numero 3 en hexadecimal
#define UDP_HEX_EIGHT (char)0x08 //numero 8 en hexadecimal
#define UDP_HEX_RELLENO (char)0x11

const char validation[] = "@0";

class UDPClient : WiFiUDP {
public:
    UDPClient();
    void udp_setup(String serverUdp1, String serverUdp2, uint16_t serverUdpPort, uint16_t minutes);
    void udp_setupAccount(const char* business, const char* subscriber);
    void udp_send(String msg);
    bool udp_get();
    void udp_loop();
private:
    char udp_inttohex(char input);
    void udp_buildPayload();
    void udp_keepAlive();
    void mefStatusNoACK();

    uint16_t portLitsen, portServer;
    String server1 ,server2;
    char incomingPacket[UDP_RX_PACKET_MAX_SIZE];  // buffer for incoming packets;
    String eventToSoftguard, account, company;
    int eventReadyToSend = 0;
    uint16_t mins;
    byte sendIntens = 0;
    byte serverActivo = 0;
};