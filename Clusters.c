#include "Clusters.h"
#include "hal_types.h"
#include "sapi.h"

void send(uint16 commandId, uint8 subCommand, uint8 status);
uint8 parseCmd(uint8* pData);
uint8 parseStatus(uint8* pData);

void send(uint16 commandId, uint8 subCommand, uint8 status){
  Data data;
  data.cmd = subCommand;
  data.status = status;
  zb_SendDataRequest(0xFFFE, commandId, sizeof(Data), (uint8*)&data, 0, AF_TX_OPTIONS_NONE, 0);
}

uint8 parseCmd(uint8* pData){
  Data data = *((Data*)pData);
  return data.cmd;
}

uint8 parseStatus(uint8* pData){
  Data data = *((Data*)pData);
  return data.status;
}

///////////////////////////////////////////////////////////////////////////////
bool isLockDataCommand(uint16 commandId){
  return commandId == LOCK_DATA_CMD_ID;
}

bool isLampDataCommand(uint16 commandId){
  return commandId == LAMP_DATA_CMD_ID;
}

////////////////////////////////////////////////////////////////////////////////
void bindDevice(uint16 commandId){
   zb_BindDevice(TRUE, commandId, (uint8*)NULL);
}

void requestBindBack(uint16 commandId){
    send(commandId, BIND_BACK_REQUEST, 0);
}

bool isBindBackRequest(uint8* data){
  return parseCmd(data) == BIND_BACK_REQUEST;
}

///////////////////////////////////////////////////////////////////////////////
void sendStatus(uint16 commandId, uint8 status){
  send(commandId, STATUS_RESPONSE, status);
}

void requestStatus(uint16 commandId){
  send(commandId, STATUS_REQUEST, 0);
}

bool isStatusRequest(uint8* data){
  return parseCmd(data) == STATUS_REQUEST;
}

bool isStatusResponse(uint8* data){
  return parseCmd(data) == STATUS_RESPONSE;
}

uint8 getStatus(uint8* data){
  return parseStatus(data);
}

///////////////////////////////////////////////////////////////////////////////
void sendButtonPressedRequest(uint16 commandId){
  send(commandId, BUTTON_PRESSED_REQUEST, 0);
}

bool isButtonPressedRequest(uint8* data){
  return parseCmd(data) == BUTTON_PRESSED_REQUEST;
}
