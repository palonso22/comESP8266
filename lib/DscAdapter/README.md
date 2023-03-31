# Código adaptador DSC a contact ID

Esta librería tiene por objetivo enviar la información necesaria de los eventos que provengan de un panel de alarmas de la marca DSC que son necesarios para el protocolo
Contact ID.
Es necesario utilizar este protocolo, debido a que el receptor ubicado en monitoreo, utiliza dicho protocolo para interpretar los eventos.

Cabe aclarar que no todos los eventos que son tomados del panel, son mensajes que van a ser de tipo Contact ID, sino que solamente serán de este tipo, aquellos mensajes que
son de importancia para monitoreo y que estan contemplados en la tabla de resumen de eventos Contact ID (ver pdf).

Los eventos que siguen este protocolo, son eventos prioritarios y la capa superior, en este caso mqtt, se publicarán en el tópico .../.../EP. 

Los eventos Contact ID, tendrán una longitud de 9 bytes, y comenzarán con la letra "E" o "R", seguido de 8 dígitos con valores del 0 al 9.

El resto de eventos contemplados que provengan del panel, son eventos que no son de interés para monitoreo, pero son útiles para el desarrollo de la interfaz de usuario como
lo puede ser el saber si la partición está lista o no para armar, la apertura de una zona, etc... Este tipo de eventos son los que denominamos como eventos de estado, en donde,
este tipo de mensajes, comenzarán con el caracter "S", al principio.

Por último, se provee la capacidad de ejecutar comandos sobre el panel, funcionando como un teclado virtual. Es decir que es posible realizar el mismo tipo de acciones que 
un teclado físico permitiendo la posibilidad de desarrollar una aplicación para el usuario.

1. Eventos provenientes del panel

1.1 Interpretación de eventos

Basándonos en el ejemplo KeybusReader de la dependencia dscKeybusInterface (https://github.com/taligentx/dscKeybusInterface), se llegó al desarrollo de la lógica de lectura 
que se llevo a cabo en el método ReaderDSC. De este ejemplo, notamos que los prints provienen de un archivo en el código fuente de esta librería denominado como dscKeybusPrintData.cpp. En el mismo se encontró que los paquetes
de datos leídos en el panel, se almacenan en un arreglo de bytes denominado panelData del cual realizando los filtros correctos mediante if y switch y siguiendo este ejemplo
se pudo obtener todos los eventos que fueron contemplados en el método ReaderDSC.

1.2 Eventos contemplados

- Armado y desarmado de partición:
    1. Todos los usuarios seteables en el panel
    2. Armado presente y ausente de hasta 8 particiones (partición 1 y 2 únicas testeadas)
    3. Desarmado hasta 8 particiones (partición 1 y 2 únicas testeadas)

- Alarma por zona:
    1. Hasta 64 zonas (limitado por el panel DSC que se utilice, hay de 16, 32, 64, 128, etc...)
    2. Partciones: particion 1 y 2, las demás hay que probar

- Inhibir/DesInhibir zonas:
    1. Hatas 64 zonas, también depende del panel
    2. IMPORTANTE: SI se quiere inhibir zonas que pertenecen a otra partición, se debe cambiar la particion activa con el comando L
    3. No se puede inhibir si alguna de las dos partiocnes esta armada, por proteccion.

- Particion lista/no lista para armar
    1. En principio 1 partición (más particiones se puede hacer)

- Incendio

- Auxilio

- Emergencia Médica

- Pérdida de batería auxiliar

- Pérdida de alimentación externa

- Problema de campanilla

- Test periódico del panel

- Código de coacción

2. Comandos

- A: Emergencia Médica

- P: Pánico

- F: Incendio

- Wp: Armado ausente de partición p

- Wpcccc: Armado ausente de partición p con coódigo de usuario cccc

- Sp: Armado presente de partición p

- Spcccc: Armado presente de partición p con coódigo de usuario cccc

- Iz o Izz: Inhibir/Deshinibir zona z/zz.

- Xpcccc: Desarmado de particion p con código de usuario cccc

- Lp: Cambio de particion activa a partición p

- T: Comodín, permite interactuar libremente con el panel, como si de un teclado virtual se tratase