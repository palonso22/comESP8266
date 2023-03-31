//Calificadores
#define califEv 'E'
#define califRest 'R'
#define califEstado 'S'
#define charS 'S'
#define charR 'R'
#define charE 'E'

///DEFINICIONES///
//CONTACT ID codigos//
#define EmerMedica "100"   // E y R
#define Incendio "110"	   // E y R
#define Panico "120"	   // E y R
#define CoaccionCod "121"  // E //Campos en uso: partición
#define Alarma "130"	   // E y R //Campos en uso: partición y zona
#define SabotajeSistema "137" //E y R
#define Perdida220 "301"   // E y R verificado
#define BatBaja "302"	   // E y R verificado
#define CampanaProbl "321" // E y R
#define PerdidaBatModuloExp "337" //E y R
#define PerididaAccModuloExp "342" // E y R
#define comunicationActive "350" //se usa solo en UDP, en mqtt lo emite el serverApp
#define ArmDesarm "401"	   // E y R //Campos en uso: partición y usuario (en armado especial no hay usuario)
#define ArmStay "441"      // R
#define ArmParcial "456"   // E //Campos en uso: partición verificado
#define ByPass "570"	   // E //Campos en uso: zona
#define TestPeriod "602"   // E
#define ModProg "627"	   // E y R
#define testComunicador "603" //E se usa solo en UDP
#define PGM "850"          // E y R
#define lostWifiCID "360"
#define lostGprsCID "355"
#define lostEthCID "361"
#define input1CID "851"
#define input2CID "852"
#define input3CID "853"
#define outputCID "840"

//STATUS códigos//
#define invalidAccesCode "555"
#define armedStatus "666"
#define exitDelayCID "777"
#define openZonesID "888"
#define readyNotReady "999"

//Particion
#define PARTITION_OFF "00"
#define PART_1 "01"
#define PART_2 "02"
#define PART_3 "03"
#define PART_4 "04"
#define PART_5 "05"
#define PART_6 "06"
#define PART_7 "07"
#define PART_8 "08"

//Campo de zona usado como estado booleano
#define ZONE_ON "001"
#define ZONE_OFF "000"

//Definiciones publicadas en LOG 
#define EmerMedicaEv "E100" 
#define EmerMedicaRest "R100"   
#define IncendioEv "E110"
#define IncendioRest "R110"
#define PanicoEv "E120"
#define PanicoRest "R120"
#define CoaccionCodEv "E121"
#define AlarmaEv "E130"	   // E y R //Campos en uso: partición y zona
#define AlarmaRest "R130"
#define EvSabSist "E137"
#define RestSabSist "R137"
#define Perdida220Ev "E301"
#define Perdida220Rest "R301"
#define BatBajaEv "E302"
#define BatBajaRest "R302"
#define CampanaProblEv "E321"
#define CampanaProblRest "R321"
#define PerdDCModExp "E337"
#define RecupDCModExp "R337"
#define ArmDesarmEv "E401"
#define ArmDesarmRest "R401"
#define PerdACModExp "E342"
#define RecupACModExp "R342"
#define ArmStayRest "R441"
#define ArmParcialEv "E456"
#define ByPassEv "E570"
#define TestPeriodEv "E602"
#define ModProgEv "E627"
#define ModProgRest "R627"
#define outputCIDEv "E840"
#define outputCIDRest "R840"
#define PGMEv "E850"
#define PGMRest "R850"
#define input1CIDEv "E851"
#define input1CIDRest "R851"
#define input2CIDEv "E852"
#define input2CIDRest "R852"
#define input3CIDEv "E853"
#define input3CIDRest "R853"
#define invalidAcces "S555"
#define statusArmed "S666"
#define partitionExitDelay "S777"
#define openZonesLog "S888"
#define partitionReady "S999"