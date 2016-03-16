/******************************************************************************
*  PUBLIC STRUCTS     
*/
#ifndef _CLUSTERS_H
#define _CLUSTERS_H

#include "hal_types.h"

//Lock data sector command id.
#define LOCK_DATA_CMD_ID        4
#define LAMP_DATA_CMD_ID        5

//Lock data sub-commands.
#define BIND_BACK_REQUEST       0x00    
#define STATUS_REQUEST          0x01  
#define BUTTON_PRESSED_REQUEST  0x02    
#define STATUS_RESPONSE         0x04   

typedef struct{
  uint8 cmd;
  uint8 status;  //Only set when cmd = STATUS_RESPONSE
} Data;

bool isLockDataCommand(uint16 commandId);
bool isLampDataCommand(uint16 commandId);

void bindDevice(uint16 commandId);
void requestBindBack(uint16 commandId);
bool isBindBackRequest(uint8* data);

void sendStatus(uint16 commandId, uint8 status);
void requestStatus(uint16 commandId);
bool isStatusRequest(uint8* data);
bool isStatusResponse(uint8* data);
uint8 getStatus(uint8* data);

void sendButtonPressedRequest(uint16 commandId);
bool isButtonPressedRequest(uint8* data);


#endif //_CLUSTERS_H

