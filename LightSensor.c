#include "LightSensor.h"
#include "hal_types.h"
#include "hal_board_cfg.h"
#include "hal_adc.h"


static uint8 lightChannel = 0;
static uint16 lightThreshold = 1000;

void initLightSensor(uint8 channel, uint16 threshold){
  lightChannel = channel;
  lightThreshold = threshold;
}

bool isLightLevelLow(){
   uint16 light = HalAdcRead(lightChannel, HAL_ADC_RESOLUTION_12);
   return light >= lightThreshold;
}