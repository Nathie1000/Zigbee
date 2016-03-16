#include "LightSensor.h"
#include "hal_types.h"
#include "hal_board_cfg.h"
#include "hal_adc.h"

//static uint8 lampPort = 0;
//static uint8 lampPin = 0;

void initLightSensor(uint8 port, uint8 pin){
  
}

bool isLightLevelLow(void){
   uint16 light = HalAdcRead(HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_12);
   return light < LOW_LIGHT_LEVEL;
}