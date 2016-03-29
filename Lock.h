#ifndef _LOCK_H
#define _LOCK_H

#include "hal_types.h"

void initLock(uint8 lockPort, uint8 lockPin, uint8 lockSensorPort, uint8 lockSensorPin);
void setLock(bool openClosed);
void toggleLock(void);
bool isLockOpen(void);

#endif //_LOCK_H