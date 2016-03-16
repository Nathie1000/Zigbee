#ifndef _LIGHT_SENSOR_H
#define _LIGHT_SENSOR_H

#include "hal_types.h"

#define LOW_LIGHT_LEVEL 1000

void initLightSensor(uint8 port, uint8 pin);
bool isLightLevelLow(void);

#endif // _LIGHT_SENSOR_H