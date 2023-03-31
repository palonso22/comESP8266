#include "dscAdapter.h"
#include "ProcessCID.h"
#include "../../include/ContactID.h"
#include "../../include/Comandos.h"

String dscAdapter::parti = "01";

/*
Esta función no pertenece a la clase dscAdapter y sirve para que la capa superior de comunicación que no va a incluir
"dscAdapter.h", pueda comunicarse con un método evitando el cycle include
Le paso un puntero a objeto que es el que esta en el campo privado que fue inicializado en setup
*/
bool llamanteMetodo(dscAdapter* clase,const char* cadena, byte largo){
  return clase->adapter_command(cadena , largo);
}

/*
Constructor del objeto dscAdapter, los parámetros son los pines
*/
dscAdapter::dscAdapter(byte setClockPin, byte setReadPin, byte setWritePin):dscKeybusInterface(setClockPin,setReadPin,setWritePin){}

/*
Inicializa la comunicación con el panel para un correcto procesamiento de datos
*/
void dscAdapter::setupDSC(){
  begin();
  Serial.println("DSC Keybus Interface is online.");
  processModuleData = true;
}

/*
- Debe ejecutarse en el loop ínfinito del main.
- loop() desencola los eventos capturados por el bus de datos, para ver qué eventos encola mirar ejemplo KeybusReader
- El evento capturado en el loop queda almacenado en en panelData o moduleData
- ReaderDSC encola todos los eventos disponibles usando loop
*/
void dscAdapter::ReaderDSC(){
  if(loop()){
    if (!keybusConnected){
      Serial.println("Keybus no conectado, verfique la conexion");
      delay(10000);
      return;
    }
    //panelData contiene la información proveniente del panel
    switch (panelData[0]) {
      case 0x27:{
        break;
      }
      case 0x58: {
        if ((moduleData[2] & 0x03) == 0) modMEF(15);
          //stream->print(F("PC5204: Battery restored "));
        if ((moduleData[2] & 0x0C) == 0) modMEF(14);
          //stream->print(F("PC5204: Battery trouble "));
        if ((moduleData[2] & 0x30) == 0) modMEF(17);
          //stream->print(F("PC5204: AC power restored "));
        if ((moduleData[2] & 0xC0) == 0) modMEF(16);
          //stream->print(F("PC5204: AC power trouble "));
        if(moduleData[12] == 0x3F ) modMEF(14);
        if(moduleData[12] == 0xCF ) modMEF(15);
        if(moduleData[12] == 0xF3 ) modMEF(16);
        if(moduleData[12] == 0xFC ) modMEF(17);
        break;
      }
      case 0x4C:{
        modMEF(35);
        break;
      }
      case 0x05: {
        if(panelData[3] == 0x05) modMEF(36); //estado de armado ausente particion 1
        if(panelData[3] == 0x04) modMEF(37); //estado de armado presente particion 1
        if(panelData[4] == 0x8A) modMEF(43); //estado de armado presente particion 2
        if(panelData[4] == 0x82) modMEF(42); //estado de armado ausente particion 2
        if (!(panelData[2] == 0)) modMEF(6); //partition 1 Ready or not
        if(panelData[3]==0x08) modMEF(7); //partition 1 exit delay
        if(!(panelData[5] == 0xC7)){ //partition 2 status
          if (!(panelData[4] == 0)) modMEF(8); //partition 2 Ready or not 
          if(panelData[5]==0x08) modMEF(9); //partition 2 exit delay
        }
        if (!(panelData[7] == 0xC7)) {
          if (!(panelData[6] == 0)) modMEF(10); //partition 3 Ready or not 
          //if(panelData[7]==0x08) modMEF(11); //partition 3 exit delay
        }
        if (!(panelData[9] == 0xC7)) {
          if (!(panelData[8] == 0)) modMEF(12); //partition 4 Ready or not 
          //if(panelData[9]==0x08) modMEF(13); //partition 4 exit delay
        }
        break;
      }
      case  0x0F:{
        break;
      }
      case  0x0A: {
        break;
      }
      case 0xA5: {
        switch (panelData[3] >> 6) {
          case 0x01: parti = "01"; partitionAct = 0; break;
          case 0x02: parti = "02"; partitionAct = 1; break;
        }
        switch((panelData[5] & 0x03)){
          case 0x00: {
            switch (panelData[6]){
              case 0x49: modMEF(27); break; //Código de coacción accionado
              case 0x4E: modMEF(19); break; //Incendio activado 'E'
              case 0x4F: modMEF(23); break; //Emergencia medica activada 'E'
              case 0x50: modMEF(25); break; //Pánico activado 'E'
              case 0x52: modMEF(20); break; //Incendio restaurado 'R'
              case 0x53: modMEF(24); break; //Emergencia médica restaurada 'R'
              case 0x54: modMEF(26); break; //Pánico restaurado 'R'
              case 0xBE: modMEF(5); break; //Armado parcial + Zonas inhibidas
              case 0xE9: modMEF(33); break; //Problema de campanilla 'E'
              case 0xF1: modMEF(34); break; //Problema de campanilla 'R'
              case 0xFE: modMEF(38); break; //Test diario
              case 0xBF:{
                if(armedAway[partitionAct]) modMEF(31);
                if(armedStay[partitionAct]) modMEF(32);
                break; //Armado especial (sin código de usuario)
              }
              case 0xE6: modMEF(18); break; //Desarmado especial, sin cód de usuario
            }
            if(panelData[6]>=0x99 && panelData[6]<=0xBD) modMEF(1); //Exit delay y Armado con código de usuario
            if(panelData[6]>=0xC0 && panelData[6]<=0xE4) modMEF(2); //Desarmado con código de usuario
            return;
          }
          case 0x01: {
            if(panelData[6] >= 0xB0 && panelData[6] <= 0xCF){ //bypass de zonas
              if(armed[0] || armed[1]) modMEF(4);
            }
            switch (panelData[6]){
              case 0xAC: modMEF(22); break; //Salida de modo de programación 'R'
              case 0xAD: modMEF(21); break; //Entrada a modo de programación 'E' 
              default: return;
            }
            return;
          }
          case 0x05: {
            if (panelData[6] <= 0x39) { //armado con coaccion
              modMEF(28);
            }
            if (panelData[6] >= 0x3A && panelData[6] <= 0x73) { //desarmado con coaccion
              modMEF(29);
            }
            break;
          }
        }
        break;
      }
      //E and R events of all 8 partitions
      /*case 0xEB: {
        //Store partition
        if(!(panelData[2]==0)){
          for (byte bit = 0; bit < 8; bit++) {
            if (bitRead(panelData[2],bit)) {
              parti = ZERO + String(1 + bit);
              partitionAct = bit;
            }
          }
        }
        switch (panelData[7]) {
          case 0x00: {
            switch (panelData[8]){
              case 0x49: modMEF(27); break; //Código de coacción accionado
              case 0x4E: modMEF(19); break; //Incendio activado 'E'
              case 0x4F: modMEF(23); break; //Emergencia medica activada 'E'
              case 0x50: modMEF(25); break; //Pánico activado 'E'
              case 0x52: modMEF(20); break; //Incendio restaurado 'R'
              case 0x53: modMEF(24); break; //Emergencia médica restaurada 'R'
              case 0x54: modMEF(26); break; //Pánico restaurado 'R'
              case 0xBE: modMEF(5); break; //Armado parcial + Zonas inhibidas
              case 0xE9: modMEF(33); break; //Problema de campanilla 'E'
              case 0xF1: modMEF(34); break; //Problema de campanilla 'R'
              case 0xBF:{
                if(armedAway[partitionAct]) modMEF(31);
                if(armedStay[partitionAct]) modMEF(32);
                break; //Armado especial (sin código de usuario)
              }
              case 0xE6: modMEF(18); break; //Desarmado especial, sin cód de usuario
            }
            if(panelData[8]>=0x99 && panelData[8]<=0xBD) modMEF(1); //Exit delay y Armado con código de usuario
            if(panelData[8]>=0xC0 && panelData[8]<=0xE4) modMEF(2); //Desarmado con código de usuario
            return;
          }
          case 0x01: {
            if(panelData[8] >= 0xB0 && panelData[8] <= 0xCF){ 
              if(armed[0] || armed[1]) modMEF(4);
            }
            switch (panelData[8]){
              case 0xAC: modMEF(22); break; //Salida de modo de programación 'R'
              case 0xAD: modMEF(21); break; //Entrada a modo de programación 'E' 
              default: return;
            }
            return;
          }
          case 0x05: {
            if (panelData[8] <= 0x39) { //armado con coaccion
              modMEF(28);
            }
            if (panelData[8] >= 0x3A && panelData[8] <= 0x73) { //desarmado con coaccion
              modMEF(29);
            }
            break;
          }
        }
      }*/
    }
    if(statusChanged){
      statusChanged = false;
      for (byte partition = 0; partition < dscPartitions; partition++) {
        //Alarma en zona evento/restauración de las 64 zonas
        if (alarmChanged[partition]){
          parti = ZERO + String(partition+1);
          modMEF(3);
        }
        // Checks disabled status
        if (disabledChanged[partition]) {
          disabledChanged[partition] = false;
          if (disabled[partition]) {
            Serial.print(F("Partition "));
            Serial.print(partition + 1);
            Serial.println(F(" disabled"));
          }
        }
         // Publishes the current partition message
        switch (status[partition]) {
          case 0x8F: modMEF(30); break;
        }
      }
      //EXTRA QUE AGREGO PARA DEBUGGING
      if (bufferOverflow) {
        Serial.println("Keybus buffer overflow");
        bufferOverflow = false;
      }
      //Problema en la alimentación de red AC 220V E/R
      if (powerChanged){
        powerChanged = false;
        modMEF(40);
      }
      //Problema de batería E/R
      if (batteryChanged){
        modMEF(41);
        batteryChanged = false;
      }
      // Publishes PGM outputs 1-14 status in a separate topic per zone
      // PGM status is stored in the pgmOutputs[] and pgmOutputsChanged[] arrays using 1 bit per PGM output:
      //   pgmOutputs[0] and pgmOutputsChanged[0]: Bit 0 = PGM 1 ... Bit 7 = PGM 8
      //   pgmOutputs[1] and pgmOutputsChanged[1]: Bit 0 = PGM 9 ... Bit 5 = PGM 14
      if (pgmOutputsStatusChanged) {
        pgmOutputsStatusChanged = false;  // Resets the PGM outputs status flag
        for (byte pgmGroup = 0; pgmGroup < 1; pgmGroup++) {
          for (byte pgmBit = 0; pgmBit < 2; pgmBit++) {
            if (bitRead(pgmOutputsChanged[pgmGroup], pgmBit)) {  // Checks an individual PGM output status flag
              bitWrite(pgmOutputsChanged[pgmGroup], pgmBit, 0);  // Resets the individual PGM output status flag
              pgm = ZERO + ZERO + String(pgmBit + 1 + (pgmGroup * 8));
              if (bitRead(pgmOutputs[pgmGroup], pgmBit)) {
                //pcid_callback(califEv,PGM,parti,pgm);           // PGM enabled
              }
              else ;//pcid_callback(califRest,PGM,parti,pgm);        // PGM disabled
            }
          }
        }
      }
      if (openZonesStatusChanged) {
        openZonesStatusChanged = false;                           // Resets the open zones status flag
        for (byte zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
          for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
            if (bitRead(openZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual open zone status flag
              bitWrite(openZonesChanged[zoneGroup], zoneBit, 0);  // Resets the individual open zone status flag
              if (bitRead(openZones[zoneGroup], zoneBit)) {
                statusOpenZones[zoneBit  + (zoneGroup * 8)]=1;
                savePayloadOpenZons();
              }
              else {
                statusOpenZones[zoneBit  + (zoneGroup * 8)]=0;         // Zone closed
                savePayloadOpenZons();
              }
            }
          }
        }
      }
    }
  }
}
/*
Llamado por ReaderDSC cuando corresponda, enviará a la capa superior el evento en formato contact id
*/
void dscAdapter::modMEF(byte event){
  switch (event){
    //ARMADO DE PARTICIÓN POR CÓDIGO DE USUARIO
    //ARMADO STAY (INTERIOR) S11234 S33668 (CID: 441)
    //ARMADO AUSENTE (AWAY) W11234 W27899 (CID 400)
    //DEBERÍA SERVIR PARA TODAS LAS PARTICIONES
    case 1:{
      byte dscCode = panelData[6] - 0x98;
      if (dscCode >= 35) dscCode += 5;
      if(dscCode > 9) zona_us = ZERO + dscCode;
      else zona_us = ZERO +  ZERO + dscCode;
      Serial.println("EL CODIGO DE USUARIO ES: " + String(dscCode)); 
      if(armedStay[partitionAct]){
        pcid_callback(califRest,ArmStay,parti,zona_us);
        Serial.print("Armado presente con Cod de usuario ");
        Serial.print(zona_us);
        Serial.print(" de la particion ");
        Serial.println(parti);
        if(partitionAct + 1 == 1) modMEF(39);
        if(partitionAct + 1 == 2) modMEF(44);
      }
      else{
        pcid_callback(califRest,ArmDesarm,parti,zona_us); 
        Serial.print("Armado ausente con Cod de usuario ");
        Serial.print(zona_us);
        Serial.print(" de la particion ");
        Serial.println(parti);
      }
      protecArm=1;
      break;
    }
    //DESARMADO
    case 2:{
      byte dscCode = panelData[6] - 0xBF;
      if (dscCode >= 35) dscCode += 5;
      if(dscCode > 9) zona_us = ZERO + dscCode;
      else zona_us = ZERO +  ZERO + dscCode;
      pcid_callback(califEv,ArmDesarm,parti,zona_us);
      Serial.print("Desarmado con Cod de usuario ");
      Serial.print(zona_us);
      Serial.print(" de la particion ");
      Serial.println(parti);
      protecArm=0;
      break;
    }
    //ALARMA POR SENSIBILIZACIÓN DE ZONA
    case 3:{
      alarmZonesStatusChanged = false;                          // Resets the alarm zones status flag
      for (byte zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
        for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
          if (bitRead(alarmZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual alarm zone status flag
            bitWrite(alarmZonesChanged[zoneGroup], zoneBit, 0);  // Resets the individual alarm zone status flag
            if (bitRead(alarmZones[zoneGroup], zoneBit)) {       // Zone alarm
              byte zonAlarm = zoneBit + 1 + (zoneGroup * 8);
              if(zonAlarm > 9) zona_us =  ZERO + String(zonAlarm);
              else zona_us = ZERO +  ZERO + String(zoneBit + 1 + (zoneGroup * 8));
              pcid_callback(califEv,Alarma,parti,zona_us);
              Serial.print("Alarma en zona: ");
              Serial.println(zona_us);
            }
            else {
              byte zonAlarm = zoneBit + 1 + (zoneGroup * 8);
              if(zonAlarm > 9) zona_us =  ZERO + String(zonAlarm);
              else zona_us = ZERO +  ZERO + String(zoneBit + 1 + (zoneGroup * 8));
              pcid_callback(califRest,Alarma,parti,zona_us);
              Serial.print("Alarma restaurada de zona: ");
              Serial.println(zona_us);
            }
          }
        }
      }
      break;
    }
    //NUEVA ZONA INHIBIDA POST ARMADO
    case 4:{
      if(armed[0] || armed[1]){
        byte zonInt = panelData[6] - 175;
        if(zonInt > 64 || zonInt < 0) break;
        String zona = String(zonInt);
        if(zonInt < 10) zona = ZERO + ZERO + zona;
        else zona = ZERO + zona;
        Serial.println("zona inhibida: " + zona);
        pcid_callback(califEv,ByPass,parti,zona);
      }
      break;
    }
    //INHIBIR/DESHINIBIR ZONAS, ARMADO PARCIAL
    //DEBO VERIFICAR QUE LA PARTICION ENVIADA SEA LA CORRECTA
    case 5:{
      pcid_callback(califEv,ArmParcial,parti,ZONE_OFF);
      Serial.print("Armado parcial: Zonas inhibidas.");
      Serial.print(" De la particion ");
      Serial.println(parti);
      armPacialFlag = 1;
      break;
    }
    //ESTADO READY/NOTREADY DE LA PARTICION 1
    //ZONAS ABIERTAS DE LA PARTICION 1 CUANDO NO ESTÁ LISTA
    case 6:{
      if (bitRead(panelData[2],0)){
        Serial.println("Particion 1 Listo");
        pcid_callback(califEstado,readyNotReady,PART_1,ZONE_ON); 
      }else {
        if(armed[0]) return; //Si la partición está armada, no sigue ejecutando este caso
        Serial.println("No está listo la particion 1");
        pcid_callback(califEstado,readyNotReady,PART_1,ZONE_OFF);
        pcid_callback(califEstado,openZonesID,"",zonOpen);
      }
      break;
    }
    //ESTADO DE EXIT DELAY DE LA PARTICION 1 EN ON
    case 7:{
      Serial.print("La partición 1");
      Serial.println(" ésta en estado de Exit delay");
      pcid_callback(califEstado,exitDelayCID,PART_1,ZONE_ON);
      break;
    }
    //ESTADO READY/NOTREADY DE LA PARTICION 2
    //ZONAS ABIERTAS DE LA PARTICION 2 CUANDO NO ESTÁ LISTA
    case 8:{
      if (bitRead(panelData[4],0)){
        Serial.println("Particion 2 Listo");
        pcid_callback(califEstado,readyNotReady,PART_2,ZONE_ON); 
      }else {
        if(armed[1]) return; //Si la partición está armada, no sigue ejecutando este caso
        Serial.println("No está listo la particion 2");
        pcid_callback(califEstado,readyNotReady,PART_1,ZONE_OFF);
        pcid_callback(califEstado,openZonesID,"",zonOpen);
      }
      break;
    }
    // EXIT DELAY PARTICION 2
    case 9:{
      Serial.print("La partición 2");
      Serial.println(" ésta en estado de Exit delay");
      pcid_callback(califEstado,exitDelayCID,PART_2,ZONE_ON);
      break;
    }
    // READY/NOTREADY Y ZONAS ABIERTAS DE PARTICION 3
    case 10:{
      if (bitRead(panelData[6],0)){
        Serial.println("Listo");
        pcid_callback(califEstado,readyNotReady,PART_3,ZONE_ON); 
      }
      else {
        Serial.println("No está listo la particion 3");
        if(armed[2]) return; //Si la partición está armada, no sigue ejecutando este caso
        pcid_callback(califEstado,readyNotReady,PART_1,ZONE_OFF);
        pcid_callback(califEstado,openZonesID,"",zonOpen);
      }
      break;
    }
    // EXIT DELAY PARTICION 3
    case 11:{
      Serial.print("La partición 3");
      Serial.println(" ésta en estado de Exit delay");
      break;
    }
    // READY/NOTREADY Y ZONAS ABIERTAS DE PARTICION 4
    case 12:{
      if (bitRead(panelData[8],0)){
        Serial.println("Listo");
        pcid_callback(califEstado,readyNotReady,PART_4,ZONE_ON); 
      }
      else {
        Serial.println("No está listo la particion 4");
        if(armed[3]) return; //Si la partición está armada, no sigue ejecutando este caso
        pcid_callback(califEstado,readyNotReady,PART_1,ZONE_OFF);
        pcid_callback(califEstado,openZonesID,"",zonOpen);
      }
      break;
    }
    //ESTADO DE EXIT DELAY DE LA PARTICION 4 EN ON
    case 13:{
      Serial.print("La partición 4");
      Serial.println(" ésta en estado de Exit delay");
      break;
    }
    case 14:{
      Serial.println("Pérdida de 220 de módulo expansor");
      //pcid_callback(califEv,PerididaAccModuloExp,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 15:{
      Serial.println("Recuperación de 220 de módulo expansor");
      //pcid_callback(califRest,PerididaAccModuloExp,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 16:{
      Serial.println("Pérdida de batería DC de módulo expansor");
      //pcid_callback(califEv,PerdidaBatModuloExp,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 17:{
      Serial.println("Recuperación de batería DC de módulo expansor");
      //pcid_callback(califRest,PerdidaBatModuloExp,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 18:{
      Serial.println("Desarmado sin código de usuario");
      pcid_callback(califEv,ArmDesarm,PARTITION_OFF,ZONE_OFF);
      protecArm=0;
      break;
    }
    case 19:{
      Serial.println("Incendio accionado por teclado");
      pcid_callback(califEv,Incendio,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 20:{
      Serial.println("Incendio accionado por teclado restaurado");
      pcid_callback(califRest,Incendio,PARTITION_OFF,ZONE_OFF);
      break;
    }
    //ENTRADA A MODO PROGRAMACION
    case 21:{
      pcid_callback(califEv,ModProg,PARTITION_OFF,ZONE_OFF);
      Serial.println("Salida de modo de programación");
      break;
    }
    //SALIDA DE MODO PROGRAMACION
    case 22:{
      pcid_callback(califRest,ModProg,PARTITION_OFF,ZONE_OFF);
      Serial.println("Entrada de modo de programación");
      break;
    }
    case 23:{
      Serial.println("Emergencia médica (teclado) accionada");
      pcid_callback(califEv,EmerMedica,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 24:{
      Serial.println("Emergencia médica (teclado) restaurado");
      pcid_callback(califRest,EmerMedica,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 25:{
      Serial.println("Pánico accionado por teclado");
      pcid_callback(califEv,Panico,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 26:{
      Serial.println("Pánico accionado por teclado restaurado");
      pcid_callback(califRest,Panico,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 27:{
      pcid_callback(califEv,CoaccionCod,parti,ZONE_OFF);
      Serial.print("Código coaccion activado en la partición ");
      Serial.println(parti);
      break;
    }
    // Armado que se da mediante codigo de coaccion
    case 28:{
      byte dscCode = panelData[8] + 0x23;
      if(dscCode > 9) zona_us = ZERO + dscCode;
      else zona_us = ZERO +  ZERO + dscCode;
      if(armedStay[partitionAct]){
          pcid_callback(califRest,ArmStay,parti,zona_us);
          Serial.print("Armado presente con Cod de usuario ");
          Serial.print(zona_us);
          Serial.print(" de la particion ");
          Serial.println(parti);
        }
        else{
          pcid_callback(califRest,ArmDesarm,parti,zona_us); 
          Serial.print("Armado ausente con Cod de usuario ");
          Serial.print(zona_us);
          Serial.print(" de la particion ");
          Serial.println(parti);
        }
        if(partitionAct + 1 == 1) modMEF(39);
        if(partitionAct + 1 == 2) modMEF(44);
        protecArm=1;
      break;
    }
    // // Desrmado que se da mediante codigo de coaccion
    case 29:{
      byte dscCode = panelData[8] - 0x17;
      if(dscCode > 9) zona_us = ZERO + dscCode;
      else zona_us = ZERO +  ZERO + dscCode;
      pcid_callback(califEv,ArmDesarm,parti,zona_us);
      Serial.print("Desarmado con Cod de usuario ");
      Serial.print(zona_us);
      Serial.print(" de la particion ");
      Serial.println(parti);
      protecArm=0;
      break;
    }
    case 30:{
      //pcid_callback(califEv,"461",PART_1,ZONE_OFF);
      pcid_callback(califEstado,invalidAccesCode,"","");
      Serial.println("Código de acceso inválido");
      break;
    }
    case 31:{
      pcid_callback(califRest,ArmDesarm,parti,ZONE_OFF);
      Serial.println("Armado ausente sin código de usuario");
      if(partitionAct + 1 == 1) modMEF(39);
      if(partitionAct + 1 == 2) modMEF(44);
      protecArm=1;
      break;
    }
    case 32:{
      pcid_callback(califRest,ArmStay,parti,ZONE_OFF);
      Serial.println("Armado presente sin código de usuario");
      if(partitionAct + 1 == 1) modMEF(39);
      if(partitionAct + 1 == 2) modMEF(44);
      protecArm=1;
      break;
    }
    case 33:{
      pcid_callback(califEv,CampanaProbl,PARTITION_OFF,ZONE_OFF);
      Serial.println("(Evento) problema con la campanilla");
      break;
    }
    case 34:{
      pcid_callback(califRest,CampanaProbl,PARTITION_OFF,ZONE_OFF);
      Serial.println("(restauración) problema con la campanilla");
      break;
    }
    case 35:{ //Sabotaje general del sistema
      if (printModuleSlotsDSC(255, 2, 5, 0xC0, 0, 4, 0)){
        printModuleSlotsDSC(1, 2, 5, 0xC0, 0, 4, 0);
        Serial.printf("Tecladp Tamper slot: %s\n",slot.c_str());
        //pcid_callback(califEv,SabotajeSistema,PARTITION_OFF,slot.c_str());
      }
      if (printModuleSlotsDSC(255, 2, 5, 0xF0, 0, 4, 0x0C)){
        printModuleSlotsDSC(1, 2, 5, 0xF0, 0, 4, 0x0C);
        Serial.printf("Recuperacion Tamper teclado slot: %s\n",slot.c_str());
        //pcid_callback(califRest,SabotajeSistema,PARTITION_OFF,slot.c_str());
      }
      if (printModuleSlotsDSC(255, 6, 8, 0xC0, 0, 4, 0)){
        printModuleSlotsDSC(9, 6, 8, 0xC0, 0, 4, 0);
        Serial.printf("Tamper modulo slot: %s\n",slot.c_str());
        //pcid_callback(califEv,SabotajeSistema,PARTITION_OFF,slot.c_str());
      }
      if (printModuleSlotsDSC(255, 6, 8, 0xF0, 0, 4, 0x0C)){
        printModuleSlotsDSC(9, 6, 8, 0xF0, 0, 4, 0x0C);
        Serial.printf("Recuperacion Tamper modulo slot: %s\n",slot.c_str());
        //pcid_callback(califRest,SabotajeSistema,PARTITION_OFF,slot.c_str());
      }
      if ((moduleData[9] & 0xC0) == 0) {
        Serial.println("RF5132: Tamper ");
        //pcid_callback(califEv,SabotajeSistema,PARTITION_OFF,ZONE_OFF);
      }

      if ((moduleData[9] & 0xF0) == 0xC0) {
        Serial.println("RF5132: Tamper restored ");
        //pcid_callback(califRest,SabotajeSistema,PARTITION_OFF,ZONE_OFF);
      }

      if ((moduleData[9] & 0x0C) == 0) {
        Serial.println("PC5208: Tamper ");
        //pcid_callback(califEv,SabotajeSistema,PARTITION_OFF,ZONE_OFF);
      }

      if ((moduleData[9] & 0x0F) == 0x0C) {
        Serial.println("PC5208: Tamper restored ");
        //pcid_callback(califRest,SabotajeSistema,PARTITION_OFF,ZONE_OFF);
      }

      if ((moduleData[10] & 0xC0) == 0) {
        Serial.println("PC5204: Tamper ");
        //pcid_callback(califEv,SabotajeSistema,PARTITION_OFF,ZONE_OFF);
      }

      if ((moduleData[10] & 0xF0) == 0xC0) {
        Serial.println("PC5204: Tamper restored ");
        //pcid_callback(califRest,SabotajeSistema,PARTITION_OFF,ZONE_OFF);
      }
      break;
    }
    case 36:{
      pcid_callback(califEstado,armedStatus,PART_1,ZONE_ON);  //Armado ausente status
      break;
    }
    case 37:{
      pcid_callback(califEstado,armedStatus,PART_1,ZONE_OFF); //Armado presente status
      break;
    }
    //TEST PERIÓDICO
    case 38:{
      Serial.println("Test Periódico");
      pcid_callback(califEv,TestPeriod,PARTITION_OFF,ZONE_OFF);
      break;
    }
    case 39:{
      Serial.println("Finaliza el tiempo de exit delay de la partición 1");
      pcid_callback(califEstado,exitDelayCID,PART_1,ZONE_OFF);
      break;
    }
    case 40:{
      if (powerTrouble){
        Serial.println("Problema en la tensión de red AC");
        pcid_callback(califEv,Perdida220,PARTITION_OFF,ZONE_OFF);
      }
      else{
        pcid_callback(califRest,Perdida220,PARTITION_OFF,ZONE_OFF);
        Serial.println("Problema en la tensión de red AC restaurado");
      }
      break;;
    }
    case 41:{//Se detecta correctamente
      batteryChanged = false;  // Resets the battery trouble status flag
      if (batteryTrouble){
        Serial.println("Problema on la batería del panel (batería baja)");
        pcid_callback(califEv,BatBaja,PARTITION_OFF,ZONE_OFF);
      }
      else{
        Serial.println("Problema on la batería del panel restaurado");
        pcid_callback(califRest,BatBaja,PARTITION_OFF,ZONE_OFF);
      }
      break;
    }
    case 42:{
      pcid_callback(califEstado,armedStatus,PART_2,ZONE_ON);  //Armado ausente status
      break;
    }
    case 43:{
      pcid_callback(califEstado,armedStatus,PART_2,ZONE_OFF);  //Armado presente status
      break;
    }
    case 44:{
      Serial.println("Finaliza el tiempo de exit delay de la partición 2");
      pcid_callback(califEstado,exitDelayCID,PART_2,ZONE_OFF);
      break;
    }
    case 45:{/*
      int indexWrite=0; // el final de la cola, posición se escritura 
      cantZon2=0;
      indexWrite = 0;
      for (byte panelByte = 4; panelByte < 8; panelByte++) {
        if (panelData[panelByte] != 0) {
          for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
            if (bitRead(panelData[panelByte],zoneBit)) {
              zon=(zoneBit + 1) + ((panelByte-4) *  8);
              Serial.println(zon);
              if(zon > 9) zona_us = ZERO + String(zon);
              else zona_us = ZERO +  ZERO + String(zon);
              strcpy(ZonInBuf2[indexWrite],zona_us.c_str());
              Serial.println("Zona inhibida: "+ zona_us);
              indexWrite++; // incrementar el índice
              if ( indexWrite >= 64){
                indexWrite = 0; 
              }
              if (cantZon2 < 64){ // incrementar la cantidad de zonas inhibidas
                cantZon2++; 
              }
            }
          }
        }
      }
      break;*/
    }
    case 46:{
      break;
    }
    default: return;
  }
}
/*
Almacena en un buffer las zonas abiertas
*/
void dscAdapter::savePayloadOpenZons(){
  byte j=0, i;
  for(i=0;i<32;i++){
    if(statusOpenZones[i]){//Si la zona esta abierta, entra al if
      zonOpen[j]=',';
      sprintf(nroZona,"%02d",i+1);
      zonOpen[j+1]=nroZona[0];
      zonOpen[j+2]=nroZona[1];
      j=j+3;
    }
  }
  
  zonOpen[j]='\0';
  if(j==0){ 
    Serial.println("Todas las zonas cerradas");
    return;
  }
  Serial.print("Las zonas abiertas son: ");
  Serial.println(zonOpen);
}
/*
Llamada por la capa superior, permite ejecutar un comando sobre el panel
*/
byte dscAdapter::adapter_command (const char* comando, byte longitud){

  byte i;

  //Error de formato de comando.
  if(longitud <= 0) {
    Serial.println("Error de formato de comando");
    return 2;
  }

  //Validación y ejecución de comando recibido (en caso de pasar la  validación).
  //En caso de que no pase la validación, saldrá del switch y llegará retornará 1 al final
  // del programa donde se indicará que el comando no fue reconocido.
  switch (comando[0]){
    case COMANDO_ARM_INT: //Armar interior.
    {
      partitionAct = comando[1] - 0x30;
      if(armed[partitionAct-1]){
        Serial.println("Este comando no puede ejecutarse estando armado");
        parti = ZERO + String(partitionAct);
        pcid_callback(califEstado,armedStatus,parti,"");
        break;;
      } 
      
      //Validacion código de usuario
      if(longitud == 6){ 
        if(!(comando[1]>= 0x31 && comando[1] <= 0x38)){
          Serial.println("Particion inválida");
          return 1;
        }
        for(i=2;i<6;i++){
          if(!(comando[i]>= 0x30 && comando[i] <= 0x39)){
            Serial.println("Código inválido");
            return 1;
          }
        }
        //En caso de estar armada la partición, se la desarma.
        //CUANDO ATACO EL PANEL, REESCRIBIR CONSTANTEMENTE LA PARTICIÓN HACE QUE SE RESETEE
        partitionAct = comando[1] - 0x30;
        writePartition = partitionAct;    // Sets writes to the partition number.
        write('#');  // Salgo de configuración
        write('s');  // Keypad stay arm.
        for(i=2;i<6;i++){ //p arranca en 2
          delay(50);
          write(comando[i]);
        }
        Serial.println(" Se ha ejecutado la solictud de armado/desarmado por código de usuario");
        return 0;
      }
      if(longitud == 2){
        if(!(comando[1]>= 0x31 && comando[1] <= 0x38)){
          Serial.println("Particion inválida");
          return 1;
        }
        partitionAct = comando[1] - 0x30;
        writePartition = partitionAct;    // Sets writes to the partition number.
        write('#');  // Salgo de configuración
        write('s');  // Keypad stay arm.
        Serial.println("Se ejecuta la solicitud de armado presente sin codigo de usuario");
        return 0;
      }
      break;
    }
    case COMANDO_ARM_EXT: //Armado exterior
    {  
      partitionAct = comando[1] - 0x30;
      if(armed[partitionAct-1]){
        Serial.println("Este comando no puede ejecutarse estando armado");
        parti = ZERO + String(partitionAct);
        pcid_callback(califEstado,armedStatus,parti,"");
        break;;
      } 
      
      //Validacion código de usuario
      if(longitud == 6){
        if(!(comando[1]>= 0x31 && comando[1] <= 0x38)){
          Serial.println("Código inválido");
          return 1;
        }
        for(i=2;i<6;i++){
          if(!(comando[i]>= 0x30 && comando[i] <= 0x39)){
            Serial.println("Código inválido");
            return 1;
          }
        }
        //En caso de estar armada la partición, se la desarma.
        //CUANDO ATACO EL PANEL, REESCRIBIR CONSTANTEMENTE LA PARTICIÓN HACE QUE SE RESETEE
        writePartition = partitionAct;    // Sets writes to the partition number.
        write('#');  // Keypad stay arm.
        write('w');  // Keypad stay arm.
        for(i=2;i<6;i++){ //p arranca en 2
          delay(50);
          write(comando[i]);
          delay(20);
        }
        Serial.println(" Se ha ejecutado la solictud de armado/desarmado por código de usuario");
        return 0;
      }
      if(longitud == 2){
        if(!(comando[1]>= 0x31 && comando[1] <= 0x38)){
          Serial.println("Particion inválida");
          return 1;
        }
        writePartition = partitionAct;    // Sets writes to the partition number.
        write('#');  // Salgo de configuración
        write('w');  // Keypad stay arm.
        Serial.println("Se ejecuta la solicitud de armado presente sin codigo de usuario");
        return 0;
      }
      break;
    }
    case COMANDO_IN_DESIN: //Inhbición de zonas.
    {
      //No permito inhibir zonas si alguna partición esta armada
      if(armed[0]) break;
      if(armed[1]) break;

      byte zonaInt = 0;

      if(longitud>1 && longitud<=3){ //Debe indicarse mínimo una zona a inhibir/deshinibir
        i=1;
        //Validación del resto de comando
        if(!(comando[i]>= 0x30 && comando[i] <= 0x39)){ //En caso de no ser alguno de estos el comando no es válido
          Serial.println("El comando para inhibiir tiene un formato incorrecto");
          return 1;
        }
        //Almacenamiento de zona a inhibir
        if(comando[i]>= 0x30 && comando[i] <= 0x39) { 
          //Se ingresó una zona de 2 cifras o màs, si es más se invalidará el comando
          if((i+1)<longitud){
            if(comando[i+1]>= 0x30 && comando[i+1] <= 0x39){ 
              //Zona de 2 cifras ingresada en el comando
              //Almacenamiento de zona
              zona_us = comando[i];
              zona_us += comando[i+1];
            }
            else {
              Serial.println("Comando no reconocido");
              return 1;
            }
          } 
          //En caso de no ser de 2 cifras la zona ingresada en el comando se chekea si es de una cifra
          //En caso de serlo se la almacena
          else if(i+1 == longitud){
            zona_us = comando[i];
          }
          zonaInt = atoi (zona_us.c_str());
          if(zonaInt>64 || (zonaInt == 0)){ //Se invalidan zonas de 1 y 2 cifras fuera de este rango
            Serial.println("La zona es inexistente");
            return 1;
          }
        }
        if (zonaInt > 9) zona_us = String(zonaInt);
        else zona_us = ZERO + String(zonaInt);
        write('#'); //Salgo de config
        //Mando a inhibir las zonas como si ejecutara el comando sobre el teclado
        delay(50);
        write('*');
        delay(50);
        write('1');
        delay(50);
        write(zona_us.charAt(0));
        delay(50);
        write(zona_us.charAt(1));
        delay(50);
        write('#');
        Serial.print("Se inhibió/deshinibió la zona: ");
        Serial.println(zona_us);
      
        return 0;
      }
      break;
    }
    case COMANDO_INCENDIO: //Incendio
    {
      if(longitud > 1) return 2;
      write('f');  // Keypad fire
      Serial.println("Ejecución de incendio!!");
      return 0;
    }
    case COMANDO_AUXILIO: //Auxilio
    {
      if(longitud > 1) return 2;
      write('a');  // Keypad aux
      Serial.println("Ejecución de solicitud de emergencia!");
      return 0;
    }     
    case COMANDO_PANICO: //Pánico
    {
      if(longitud > 1) return 2; 
      write('p');  // Keypad panic
      Serial.println("Ejecución de pánico!!");
      return 0;
    }
    case COMANDO_DESARMADO:{
      //Armado y desarmado por código
      //Validacion código de usuario
      if(longitud == 6){
        if(!(comando[1]>= 0x31 && comando[1] <= 0x38)){
          Serial.println("Código inválido");
          return 1;
        }
        for(i=2;i<6;i++){
          if(!(comando[i]>= 0x30 && comando[i] <= 0x39)){
            Serial.println("Código inválido");
            return 1;
          }
        }
        //En caso de estar armada la partición, se la desarma.
        //CUANDO ATACO EL PANEL, REESCRIBIR CONSTANTEMENTE LA PARTICIÓN HACE QUE SE RESETEE
        partitionAct = comando[1] - 0x30;
        writePartition = partitionAct;    // Sets writes to the partition number.
        write('#');  // Keypad stay arm.
        for(i=2;i<6;i++){ //p arranca en 2
          delay(50);
          write(comando[i]);
          delay(20);
        }
        Serial.println(" Se ha ejecutado la solictud de armado/desarmado por código de usuario");
        return 0;
      }
      break;
    }
    case COMANDO_COMODIN:{//Modo instalador (comodín) ESTE MODO HAY QUE VERLO                                                                
      for(i=1;i<longitud;i++){
        write(comando[i]);
        delay(20);
      }
      Serial.print("Se uso el comodín, el comando ejecutado fue: ");
      Serial.println(comando);

      return 0;
    }
    case COMANDO_CAMBIO_PART:{//Cambio de partición activa
      if(comando[1] == '1' || comando[1] == '2'){
        writePartition = comando[1] - 0x30;
        Serial.println("Cambio de partición activa");
        return 0;
      }
      else Serial.println("Partición inválida");
      break;
    }
  }

  //Si llegó acá es porque no se reconoció el comando
  //Pudo nunca haber entrado al switch o al if
  Serial.println("Comando no reconocido");
  return 1;
}

bool dscAdapter::printModuleSlotsDSC(byte outputNumber, byte startByte, byte endByte, byte startMask, byte endMask, byte bitShift, byte matchValue) {
  for (byte testByte = startByte; testByte <= endByte; testByte++) {
    byte matchShift = 8 - bitShift;
    for (byte testMask = startMask; testMask != 0; testMask >>= bitShift) {
      if (testByte == endByte && testMask < endMask) return false;

      byte testData = moduleData[testByte];

      if ((testData & testMask) >> matchShift == matchValue) {
        if (outputNumber == 255) return true;
        else {
          if(outputNumber < 10) slot = ZERO + ZERO + String(outputNumber);
          else slot = ZERO + String(outputNumber);
        }
      }

      if (outputNumber != 255) outputNumber++;
      matchShift -= bitShift;
    }
  }

  return false;
}