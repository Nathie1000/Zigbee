#ifndef _LIGHT_SENSOR_H
#define _LIGHT_SENSOR_H

#include "hal_types.h"

void initLightSensor(uint8 channel, uint16 threshold);
bool isLightLevelLow(void);

#endif // _LIGHT_SENSOR_H