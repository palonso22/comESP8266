#define pgmstring "T*"
#define readyPanel "T*#" //Al ejecturarse, el panel devuelve si esta o no está listo

#define charM 'M' //Pensado para usarse con PGM
#define charR1 "R1" //Activa por un minuto el paso de de eventos de estado
#define charR0 "R0"

#define charD1 "D1" //Activa salida digital
#define charD0 "D0"//Desactiva salida digital
#define charD 'D'

#define restCommand "RESET"
#define ethActive "RETH1"
#define ethDesactive "RETH0"
#define gprsActive "RGPRS1"
#define gprsDesactive "RGPRS0"
#define logActive "LOG1"
#define logDesactive "LOG0"
#define version "V"

#define COMANDO_ARM_INT 'S'
#define COMANDO_ARM_EXT 'W'
#define COMANDO_IN_DESIN 'I' //permite inhibir/desinhibit una zona
#define COMANDO_INCENDIO 'F'
#define COMANDO_AUXILIO 'A'
#define COMANDO_PANICO 'P'
#define COMANDO_DESARMADO 'X' 
#define COMANDO_CAMBIO_PART 'L' //permite el cambio de partición
#define COMANDO_COMODIN 'T' //permite realizar comandos como si se estuviera interactuando directamente con el reclado