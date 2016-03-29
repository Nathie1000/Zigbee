#include "Lock.h"
#include "hal_board_cfg.h"

static uint8 inPort = 0;
static uint8 inPin = 0;

static uint8 outPort = 0;
static uint8 outPin = 0;

static bool lockStatus = TRUE;

void initLock(uint8 lockPort, uint8 lockPin, uint8 lockSensorPort, uint8 lockSensorPin){
  outPort = lockPort;
  outPin = lockPin;
  inPort = lockSensorPort;
  inPin = lockSensorPin;
  
  if(outPort == 0){
    switch(outPin){
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
    switch(inPort){
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
  
  if(outPort == 0){
    switch(inPin){
    case 0 : MCU_IO_DIR_INPUT(0, 0); break;
    case 1 : MCU_IO_DIR_INPUT(0, 1); break;
    case 2 : MCU_IO_DIR_INPUT(0, 2); break;
    case 3 : MCU_IO_DIR_INPUT(0, 3); break;
    case 4 : MCU_IO_DIR_INPUT(0, 4); break;
    case 5 : MCU_IO_DIR_INPUT(0, 5); break;
    case 6 : MCU_IO_DIR_INPUT(0, 6); break;
    case 7 : MCU_IO_DIR_INPUT(0, 7); break;
    }
  }
  else{
    switch(outPin){
    case 0 : MCU_IO_DIR_INPUT(1, 0); break;
    case 1 : MCU_IO_DIR_INPUT(1, 1); break;
    case 2 : MCU_IO_DIR_INPUT(1, 2); break;
    case 3 : MCU_IO_DIR_INPUT(1, 3); break;
    case 4 : MCU_IO_DIR_INPUT(1, 4); break;
    case 5 : MCU_IO_DIR_INPUT(1, 5); break;
    case 6 : MCU_IO_DIR_INPUT(1, 6); break;
    case 7 : MCU_IO_DIR_INPUT(1, 7); break;
    }
  }
  setLock(lockStatus);
}

void setLock(bool openClosed){
  lockStatus = openClosed;
  if(outPort == 0){
    switch(outPin){
    case 0 : MCU_IO_SET(0, 0, openClosed); break;
    case 1 : MCU_IO_SET(0, 1, openClosed); break;
    case 2 : MCU_IO_SET(0, 2, openClosed); break;
    case 3 : MCU_IO_SET(0, 3, openClosed); break;
    case 4 : MCU_IO_SET(0, 4, openClosed); break;
    case 5 : MCU_IO_SET(0, 5, openClosed); break;
    case 6 : MCU_IO_SET(0, 6, openClosed); break;
    case 7 : MCU_IO_SET(0, 7, openClosed); break;
    }
  }
  else{
    switch(outPin){
    case 0 : MCU_IO_SET(1, 0, openClosed); break;
    case 1 : MCU_IO_SET(1, 1, openClosed); break;
    case 2 : MCU_IO_SET(1, 2, openClosed); break;
    case 3 : MCU_IO_SET(1, 3, openClosed); break;
    case 4 : MCU_IO_SET(1, 4, openClosed); break;
    case 5 : MCU_IO_SET(1, 5, openClosed); break;
    case 6 : MCU_IO_SET(1, 6, openClosed); break;
    case 7 : MCU_IO_SET(1, 7, openClosed); break;
    }
  }
}

void toggleLock(){
  setLock(!lockStatus);
}

bool isLockOpen(void){
  if(inPort == 0){
    switch(inPin){
    case 0 : return MCU_IO_GET(0, 0) != 0; break;
    case 1 : return MCU_IO_GET(0, 1) != 0; break;
    case 2 : return MCU_IO_GET(0, 2) != 0; break;
    case 3 : return MCU_IO_GET(0, 3) != 0; break;
    case 4 : return MCU_IO_GET(0, 4) != 0; break;
    case 5 : return MCU_IO_GET(0, 5) != 0; break;
    case 6 : return MCU_IO_GET(0, 6) != 0; break;
    case 7 : return MCU_IO_GET(0, 7) != 0; break;
    }
  }
  else{
    switch(inPin){
    case 0 : return MCU_IO_GET(1, 0) != 0; break;
    case 1 : return MCU_IO_GET(1, 1) != 0; break;
    case 2 : return MCU_IO_GET(1, 2) != 0; break;
    case 3 : return MCU_IO_GET(1, 3) != 0; break;
    case 4 : return MCU_IO_GET(1, 4) != 0; break;
    case 5 : return MCU_IO_GET(1, 5) != 0; break;
    case 6 : return MCU_IO_GET(1, 6) != 0; break;
    case 7 : return MCU_IO_GET(1, 7) != 0; break;
    }
  }
  return -1;
}