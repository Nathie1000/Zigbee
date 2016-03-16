#ifndef _LAMP_H
#define _LAMP_H


#include "hal_types.h"

void initLamp(uint8 port, uint8 pin);
void setLamp(bool onOff);
void lampOn(void);
void lampOff(void);


#endif // _LAMP_H