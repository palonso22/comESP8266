# comunicador

Como hace mención el nombre del repositorio, este proyecto contiene todo el desarrollo del comunicador triple vía (WiFi, Ethernet y GPRS) para sistemas de alarma DSC.
Además se encuentra también disponible, el proyec

Para la utilización efectiva del mismo, el instalador, debe anular el armado rápido, lo cual permite el armado tanto presente como ausente, con código de usuario.
Esto debe realizarse tanto para el comunicador triple vía como para el wifi.

Directorios:

1. CodigoComunicador: Código del comunicador. Para una mejor legibilidad del mismo es recomendable abrirlo con Visual Studio Code mediante el pluggin de 
Platformio. En el mismo hay otro README de guía

2. Ejecutables: Contiene los ejecutables que deben subirse a los comunicadores

Comunicador WiFi:

1. Instalación:

1.1. Una vez energizado el módulo se levantará un punto de acceso por 2 minutos, levantandose una red denominada en principio como "ComunicadorTripleVia". 
El led azul del comunicador comenzará a titilar cada 200 ms.  

1.2. Con un dispositivo que disponga de un navegador web, conectarse a dicha red, y luego ingresar en el navegador la IP: 192.168.4.1

1.3. Ingresar los párametros solicitados: 
                                        - ServerMQTT: host1.centinet.com.ar
                                        - PuertoMQTT: 18331
                                        - Red Wifi: 
                                        - Pass Red Wifi:
                                        - Empresa: CNT
                                        - Nro Cuenta: 

1.4. El comunicador se reseteará, almacenará en la flash los parámetros seteados, y volverá intentará conectarse, titilando una vez cada 500 ms, si los datos fueron correctamente ingresados, se conectará exitósamente y quedará el led azul prendido. En caso de haber errado, y falle en conectarse la primera vez (después del reset), volverá a levantar el punto de acceso.

2. Posibles fallas:

2.1 Pérdida de conexión WiFi (Ejemplo: Router roto, router desenergizado o conexión a red wifi nunca establecida): Led indicador titilará 3 veces de forma cíclica.

2.2 Pérdida de conexión con el servidor (Ejemplo: Servidor caído, cable coaxil/fibra óptica/etc.. cortado): Led indicador titila 2 veces de forma cíclica.

3. Conceptos importantes del comunicador:

3.1 Los comunicadores mantendran una interconexión con el servidor regida por el protocolo de conexión MQTT. Siguiendo una infraestructura centralizada de publicación/subscripción, en donde el servidor (Mosquitto), estará en la central de monitoreo.

Los tópicos definidos son:

- CNT/6785/C  ---> Donde la app, publicará los comandos
- CNT/6785/EP ---> Donde el comunicador publicará los eventos prioritarios (Mensajes/Payloads Contacts ID). Ejemplo: E13001003 Alarma activa en la zona 3 de partición 1.
- CNT/6785/EA ---> Se publicará el mismo evento EP, pero a este estará subscripto la APP. El evento será publicado en este tópico, una vez que SoftGuard confirme la recepción.
- CNT/6785/ST ---> Se publicará con retain, el estado de conexión del comunicador. Dos estados posibles: TOFFLINE y TONLINE.
- CNT/6785/ES ---> Se publicarán estados del comunicador. Estos eventos puede ser indicaciones de zonas abiertas o de si está listo para armarse.
- CNT/6785/WIFI ---> El comunicador publicará cada 1 hora la intensisdad de la señal wifi en decibeles.
- CNT/6785/LOG ---> El comunicador usará este tópico para publicar acciones que sucedan a través del mismo que servirá como Log.

Aclaración: Puede notarse que los tópicos siguen la estrctura empresa/nro_de_cuenta/(C, EP, ST, etc...)

3.2 Comandos:

Al panel de alarmas, se lo puede comandar mediante una APP o un cliente mqtt, publicando payload válido en el tópico empresa/nro_de_cuenta/C.
Los comandos posibles son:

  - Xncccc ---> Desarmado/Armado de la partición n con código de usuario cccc. Ejemplo: X23624 (n=2 cccc=3624)
  - Sncccc ---> Armado presente de la partición n con código de usuario cccc. Ejemplo: S17771 (n=1 cccc=7771)
  - Wncccc ---> Armado ausente de la partición n con código de usuario cccc. Ejemplo: X35564 (n=3 cccc=5564)
  - Izz/Iz ---> Inhibir/Deshinibir zona zz. Ejemplo: I4 Inhibo/Deshinibo zona 4. I26 Inhibo/Deshinibo zona 26 (máx 64 zonas).
  - R1 ---> Permitirá la publicación de estados de comunicador en el empresa/nro_de_cuenta/ES por 1 minuto.
  - Dmz ---> Permite el encendido o apagado de salida digital, donde m = 1 o 0 (on/off), y z, el número de salida (máx 3 salidas digitales)
  - Mmx ---> PGM x, prendido (m=1) o apagado (m=0)
  - Txxx... ---> Comodín, permite utilizar el comunicador como teclado remoto, T*85555 (entrada a modo programacion) o cualquier otra funcionalidad
  - A ---> Emergencia médica accionada
  - P ---> Pánico accionado
  - F ---> Incendio accionado

4. Limitaciones

- Por ahora no está contemplado la realización de ping por gprs ni ethernet
- No se adicionó seguridad SSL




# Código fuente tanto del comunicador Wifi como el triple vía

Directorios:
- include: No se utilizó todavía
- lib: Se encuentran los directorios que contienen el código desarrollado. Se recomienda mirar los README de cada carpeta.
- src: Se encuentra el archivo main archivo donde parte la ejecución del código
- test: No se utiliza

Legibilidad del código comunicador triple vía:
- Desde el main se ejecuta setupComunication(); correspondiente a lib/ComunicationMangnamente.h y de ahí no se ejecuta nada más del archivo main. 
- Para seguir el código hay que entrar a la definición de setupComunication en lib/ComunicationMangnamente.cpp.

Importante:
- Se discrepa entre los códigos correspondientes del comunicador triple vía y comunicador wifi mediente las siguientes sintaxis: 
    - #if defined(ESP32) -> Comunicador triple vía
    - #if defined(ESP8266) -> Comunicador wifi 
    - Se puede elegir si compilar código para que funcione con MQTT, o con UDP, con, y sin LOGS, descomentando/comentando las banderas correspondientes en platformio.ini

En caso del comunicador WiFI (ESP8266):
- titileo constante: No se pudo conectar al inicializar, punto de acceso activado
- 4 titileos rápidos: Conexion MQTT perdida
- 3 titileos: Conexion Wifi perdida

En caso del comunicador triple vía (solo apto con ESP32). Leds indicadores (titilan en forma cíclica): 
- titileo constante: No se pudo conectar al inicializar, punto de acceso activado
- 2 titileos: Conexion MQTT perdida
- 3 titileos: Conexion Wifi perdida, pero MQTT conectado por otra vía
- 4 titileos: Conexion GPRS o Ethernet perdida, pero MQTT conectado por otra vía
- 5 titileos: Conexion Wifi y GPRS o Ethernet perdida
- 6 titileos: GPRS o Ethernet on, Wifi off, MQTT off
- 7 titileos: GPRS o Ethernet off, Wifi on, MQTT off
- 8 titileos: Titila previo a ejecutar la función WiFi.begin(ssid, pass) dentro de setupWiFi(). Si vuelve a titilar 8 veces es porque resetea,

Todo el código desarrollado epuede encontrase entre las carpetas /lib /include /src

Dentro de la carpeta lib, se encuentran pueden encontrar las librerías desarrolladas en Recursos Tecnológicos. 
En la misma se encuentran las siguientes librerías:

------ Común a ambos comunicadores ------

- AP: Referido a Acces Point. Esta lib, se encarga de checkear que las conexiones mediante las vías correspondientes, y por MQTT esten activas, en caso de no estar activas se encargará de poner a la ESP32 en modo acces point, levantando una red pública de nombre "ComunicadorTripleVía". Una vez conectado, mediante un navegador, se debe acceder a la red 192.168.4.1.
Accedido a ese punto, lo siguiente que se debe hacer es setear los parámetros wifi del sitio en el que se haya instalado, los parámetros del servidor de monitoreo, y los parámetros relacionados con la APN de la red celular correspondiente, datos que varán de acuerdo a la empresa que provea los servicios. 

- dscAdapter: Esta es la capa que se encarga de traducir los mensajes que son extraídos del panel. En la misma, se utiliza la lógica proveniente del código fuente de la librería base dscKeybusInterface, para capturar la información necesaria de todos los eventos que son de interés.

- GPIO: Esta librerá se encarga de gestionar las acciones relacionadas con las Entradas/Salidas digitales del microcontrolador que se utilice, ya sea ESP8266 o ESP32.

- ProcessCID: Capa nexo entre dscAdapter y las capas de comunicación como UDP y MQTT. Todos los comandos y eventos del panel, pasan a través de la misma.

- InterfaceMQTTClient: Capa MQTT

- InterfaceUDPDSC: Capa UDP

- WifiWay: Librería que contiene el desarrollo para poder gestionar la comunicación vía wifi, desde su inicialización, sus método de reconexión, de estado y testeo.

------ Común a ambos comunicadores ------
------ Solo Triple Vía ------

- GprsWay: Librería que contiene el desarrollo para poder gestionar la comunicación vía gprs, desde su inicialización, sus método de reconexión, de estado y testeo.

- EthernetWay: Librería que contiene el desarrollo para poder gestionar la comunicación vía ethernet, desde su inicialización, sus método de reconexión, de estado y testeo.

- ComunicationMagnagement: Mediante FreeRTOS, se admite la gestión multi via utilizando diversas tareas. Se plantea una tarea para cada vía (3 en total) que esten inspeccionando constantemente si cada una está conectada. Existe otra tarea, encargada de leer constantemente el estado del panel y una última encargada de revisar manifestar el estado mediande el led indicador.

Cabe aclarar que cada una de estas librerías, a su vez, tiene un README que describe el funcionamiento en forma más detallada de las mismas, además de los comentarios que contienen cada uno de estas.

ESP8266 IMPORTANTE

En caso de subirse el firmware del PCB Rev1.0, debe comentarse en el archivo de platformio.ini la bandera -D useRev2# comESP8266
