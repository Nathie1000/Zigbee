/**************************************************************************************************
  Filename:       DemoSensor.c

  Description:    Sensor application for the sensor demo utilizing the Simple API.

                  The sensor node is a ZigBee end device.
                  The sensor application binds to a gateway and will periodically
                  read temperature and supply voltage from the ADC and send report
                  towards the gateway node.


  Copyright 2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/******************************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "OSAL.h"
#include "sapi.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "DemoApp.h"
#include "Clusters.h"
#include "Lamp.h"

/******************************************************************************
 * CONSTANTS
 */

/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */

// Inputs and Outputs for Sensor device
#define NUM_OUT_CMD_SENSOR                1
#define NUM_IN_CMD_SENSOR                 1

// List of output and input commands for Collector device  
 const cId_t zb_InCmdList[NUM_IN_CMD_SENSOR] =
{
  LOCK_DATA_CMD_ID
};  
   
// List of output and input commands for Sensor device
const cId_t zb_OutCmdList[NUM_OUT_CMD_SENSOR] =
{
  LOCK_DATA_CMD_ID
};

// Define SimpleDescriptor for Sensor device
const SimpleDescriptionFormat_t zb_SimpleDesc =
{
  MY_ENDPOINT_ID,             //  Endpoint
  MY_PROFILE_ID,              //  Profile ID
  DEV_ID_SENSOR,              //  Device ID
  DEVICE_VERSION_SENSOR,      //  Device Version
  0,                          //  Reserved
  NUM_IN_CMD_SENSOR,          //  Number of Input Commands
  (cId_t *) zb_InCmdList,     //  Input Command List
  NUM_OUT_CMD_SENSOR,         //  Number of Output Commands
  (cId_t *) zb_OutCmdList     //  Output Command List
};

/******************************************************************************
 * LOCAL FUNCTIONS
 */

/*****************************************************************************
 * @fn          zb_HandleOsalEvent
 *
 * @brief       The zb_HandleOsalEvent function is called by the operating
 *              system when a task event is set
 *
 * @param       event - Bitmask containing the events that have been set
 *
 * @return      none
 */
void zb_HandleOsalEvent( uint16 event ){
  if( event & ZB_ENTRY_EVENT ) {
    // blind LED 1 to indicate joining a network
    initApp();
    HalLedBlink ( HAL_LED_1, 0, 50, 500 );
    // Start the device
    zb_StartRequest();
  }
}

void initApp(void){
  HalLedSet(HAL_LED_1, HAL_LED_MODE_OFF );
  HalLedSet(HAL_LED_2, HAL_LED_MODE_OFF );
  HalLedSet(HAL_LED_3, HAL_LED_MODE_OFF );
  initLamp(1,2);
}

/******************************************************************************
 * @fn      zb_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 EVAL_SW4
 *                 EVAL_SW3
 *                 EVAL_SW2
 *                 EVAL_SW1
 *
 * @return  none
 */
void zb_HandleKeys( uint8 shift, uint8 keys ){
  //Test for press on key 2.
  if( keys & HAL_KEY_SW_2 ) {
    //Send a data request to bound device indicating button pressed.
    sendButtonPressedRequest(LOCK_DATA_CMD_ID);
  }
}

/******************************************************************************
 * @fn          zb_StartConfirm
 *
 * @brief       The zb_StartConfirm callback is called by the ZigBee stack
 *              after a start request operation completes
 *
 * @param       status - The status of the start operation.  Status of
 *                       ZB_SUCCESS indicates the start operation completed
 *                       successfully.  Else the status is an error code.
 *
 * @return      none
 */
void zb_StartConfirm( uint8 status ){
  //If the device sucessfully started.
  if ( status == ZB_SUCCESS ) { 
    //Set LED 1 to indicate that node is operational on the network.
    HalLedSet(HAL_LED_1, HAL_LED_MODE_ON );
    //Blink LED 2 to indicate discovery and binding.
    HalLedBlink( HAL_LED_2, 0, 50, 500 );
    //Find and bind to a collector device.
    bindDevice(LOCK_DATA_CMD_ID);
  }
}

/******************************************************************************
 * @fn          zb_SendDataConfirm
 *
 * @brief       The zb_SendDataConfirm callback function is called by the
 *              ZigBee after a send data operation completes
 *
 * @param       handle - The handle identifying the data transmission.
 *              status - The status of the operation.
 *
 * @return      none
 */
void zb_SendDataConfirm( uint8 handle, uint8 status ){
 (void)handle;
 (void)status;
}

/******************************************************************************
 * @fn          zb_BindConfirm
 *
 * @brief       The zb_BindConfirm callback is called by the ZigBee stack
 *              after a bind operation completes.
 *
 * @param       commandId - The command ID of the binding being confirmed.
 *              status - The status of the bind operation.
 *
 * @return      none
 */
void zb_BindConfirm( uint16 commandId, uint8 status ){
  if(isLockDataCommand(commandId)){
    //Test if bind succeded.
    if(status == ZB_SUCCESS ) {
       HalLedSet( HAL_LED_2, HAL_LED_MODE_ON );
       HalLedBlink(HAL_LED_3, 0, 50, 500 );
       //Allow this device to bind.
       zb_AllowBind( 0xFF );
       //Request to other device to bind to this device.
       requestBindBack(LOCK_DATA_CMD_ID);
    }
    else{
       //Try to bind again.
       bindDevice(LOCK_DATA_CMD_ID);
    }
  }
}

/******************************************************************************
 * @fn          zb_AllowBindConfirm
 *
 * @brief       Indicates when another device attempted to bind to this device
 *
 * @param
 *
 * @return      none
 */
void zb_AllowBindConfirm( uint16 source ){
  (void)source;
  //Turn of bind mode, we only bind to 1 device ever.
  zb_AllowBind(0x00);
  HalLedSet( HAL_LED_3, HAL_LED_MODE_ON );
  //Request the inital status of the lamp.
  requestStatus(LOCK_DATA_CMD_ID);
}

/******************************************************************************
 * @fn          zb_FindDeviceConfirm
 *
 * @brief       The zb_FindDeviceConfirm callback function is called by the
 *              ZigBee stack when a find device operation completes.
 *
 * @param       searchType - The type of search that was performed.
 *              searchKey - Value that the search was executed on.
 *              result - The result of the search.
 *
 * @return      none
 */
void zb_FindDeviceConfirm( uint8 searchType, uint8 *searchKey, uint8 *result ){
  (void)searchType;
  (void)searchKey;
  (void)result;
}

/******************************************************************************
 * @fn          zb_ReceiveDataIndication
 *
 * @brief       The zb_ReceiveDataIndication callback function is called
 *              asynchronously by the ZigBee stack to notify the application
 *              when data is received from a peer device.
 *
 * @param       source - The short address of the peer device that sent the data
 *              command - The commandId associated with the data
 *              len - The number of bytes in the pData parameter
 *              pData - The data sent by the peer device
 *
 * @return      none
 */
void zb_ReceiveDataIndication( uint16 source, uint16 command, uint16 len, uint8 *pData){
  (void)source;
  (void)command;
  (void)len;
  //Test data respone.
  if(isStatusResponse(pData)){
    //Update the lamp status.
    setLamp(getStatus(pData));
  }
}

/******************************************************************************
 * @fn          uartRxCB
 *
 * @brief       Callback function for UART
 *
 * @param       port - UART port
 *              event - UART event that caused callback
 *
 * @return      none
 */
void uartRxCB( uint8 port, uint8 event ){
  (void)port;
  (void)event;
}

