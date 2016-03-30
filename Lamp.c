#include "Lamp.h"
#include "hal_board_cfg.h"

static uint8 lampPort = 0;
static uint8 lampPin  = 1;
static bool lampStatus = FALSE;

/* MACROS are not variable configurable.
*  To work around this all the MARCOS have be defined in a swith case, not ideal but it works.
*/

void initLamp(uint8 port, uint8 pin){
  lampPort = port;
  lampPin = pin;
  
  if(lampPort == 0){
    switch(lampPin){
    case 0 : MCU_IO_DIR_OUTPUT(0, 0); break;
    case 1 : MCU_IO_DIR_OUTPUT(0, 1); break;
    case 2 : MCU_IO_DIR_OUTPUT(0, 2); break;
    case 3 : MCU_IO_DIR_OUTPUT(0, 3); break;
    case 4 : MCU_IO_DIR_OUTPUT(0, 4); break;
    case 5 : MCU_IO_DIR_OUTPUT(0, 5); break;
    case 6 : MCU_IO_DIR_OUTPUT(0, 6); break;
    case 7 : MCU_IO_DIR_OUTPUT(0, 7); break;
    }
  }
  else{
    switch(lampPin){
    case 0 : MCU_IO_DIR_OUTPUT(1, 0); break;
    case 1 : MCU_IO_DIR_OUTPUT(1, 1); break;
    case 2 : MCU_IO_DIR_OUTPUT(1, 2); break;
    case 3 : MCU_IO_DIR_OUTPUT(1, 3); break;
    case 4 : MCU_IO_DIR_OUTPUT(1, 4); break;
    case 5 : MCU_IO_DIR_OUTPUT(1, 5); break;
    case 6 : MCU_IO_DIR_OUTPUT(1, 6); break;
    case 7 : MCU_IO_DIR_OUTPUT(1, 7); break;
    }
  }
  
  //Turn lamp default to the defualt setting.
  setLamp(lampStatus);
}

void setLamp(bool onOff){
    lampStatus = onOff;
  
    if(lampPort == 0){
    switch(lampPin){
    case 0 : MCU_IO_SET(0, 0, onOff); break;
    case 1 : MCU_IO_SET(0, 1, onOff); break;
    case 2 : MCU_IO_SET(0, 2, onOff); break;
    case 3 : MCU_IO_SET(0, 3, onOff); break;
    case 4 : MCU_IO_SET(0, 4, onOff); break;
    case 5 : MCU_IO_SET(0, 5, onOff); break;
    case 6 : MCU_IO_SET(0, 6, onOff); break;
    case 7 : MCU_IO_SET(0, 7, onOff); break;
    }
  }
  else{
    switch(lampPin){
    case 0 : MCU_IO_SET(1, 0, onOff); break;
    case 1 : MCU_IO_SET(1, 1, onOff); break;
    case 2 : MCU_IO_SET(1, 2, onOff); break;
    case 3 : MCU_IO_SET(1, 3, onOff); break;
    case 4 : MCU_IO_SET(1, 4, onOff); break;
    case 5 : MCU_IO_SET(1, 5, onOff); break;
    case 6 : MCU_IO_SET(1, 6, onOff); break;
    case 7 : MCU_IO_SET(1, 7, onOff); break;
    }
  }
}

bool isLampOn(){
  return lampStatus;
}

void toggleLamp(void){
   setLamp(!lampStatus);
}