/**************************************************************************************************
  Filename:       DemoCoordinator.c

  Description:    Coordinator application for the sensor demo utilizing Simple API.

                  The collector node can be set in a state where it accepts
                  incoming reports from the sensor nodes, and can send the reports
                  via the UART to a PC tool. The collector node in this state
                  functions as a gateway. The collector nodes that are not in the
                  gateway node function as routers in the network.


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
#include "OSAL_Nv.h"
#include "sapi.h"
#include "hal_key.h"
#include "hal_led.h"
#include "hal_lcd.h"
#include "hal_uart.h"
#include "hal_adc.h"
#include "DemoApp.h"
#include "Console.h"
#include "Clusters.h"
#include "Lamp.h"
#include "LightSensor.h"
#include "Lock.h"

/******************************************************************************
 * CONSTANTS
 */

// General UART frame offsets
#define FRAME_SOF_OFFSET                    0
#define FRAME_LENGTH_OFFSET                 1
#define FRAME_CMD0_OFFSET                   2
#define FRAME_CMD1_OFFSET                   3
#define FRAME_DATA_OFFSET                   4

// ZB_RECEIVE_DATA_INDICATION offsets
#define ZB_RECV_SRC_OFFSET                  0
#define ZB_RECV_CMD_OFFSET                  2
#define ZB_RECV_LEN_OFFSET                  4
#define ZB_RECV_DATA_OFFSET                 6
#define ZB_RECV_FCS_OFFSET                  8

// ZB_RECEIVE_DATA_INDICATION frame length
#define ZB_RECV_LENGTH                      15

// PING response frame length and offset
#define SYS_PING_RSP_LENGTH                 7
#define SYS_PING_CMD_OFFSET                 1

// Stack Profile
#define ZIGBEE_2007                         0x0040
#define ZIGBEE_PRO_2007                     0x0041

#ifdef ZIGBEEPRO
#define STACK_PROFILE                       ZIGBEE_PRO_2007
#else
#define STACK_PROFILE                       ZIGBEE_2007
#endif

#define CPT_SOP                             0xFE
#define SYS_PING_REQUEST                    0x0021
#define SYS_PING_RESPONSE                   0x0161
#define ZB_RECEIVE_DATA_INDICATION          0x8746

// Application States


// Application osal event identifiers
#define SENSOR_TASK_EVENT         0x0001
#define SENSOR_POLL_INTERVAL      1000

/******************************************************************************
 * TYPEDEFS
 */


/******************************************************************************
 * LOCAL VARIABLES
 */
static bool isLightEndPointBound = FALSE;
static bool isLockEndPointBound = FALSE;


/******************************************************************************
 * LOCAL FUNCTIONS
 */

static void reportLightSensor(void);
static void reportLockStatus(void);

/******************************************************************************
 * GLOBAL VARIABLES
 */

// Inputs and Outputs for Collector device
#define NUM_OUT_CMD_COLLECTOR           2
#define NUM_IN_CMD_COLLECTOR            2

// List of output and input commands for Collector device
const cId_t zb_InCmdList[NUM_IN_CMD_COLLECTOR] =
{
  LOCK_DATA_CMD_ID,
  LAMP_DATA_CMD_ID
};

const cId_t zb_OutCmdList[NUM_IN_CMD_COLLECTOR] =
{
  LOCK_DATA_CMD_ID,
  LAMP_DATA_CMD_ID
};

// Define SimpleDescriptor for Collector device
const SimpleDescriptionFormat_t zb_SimpleDesc =
{
  MY_ENDPOINT_ID,             //  Endpoint
  MY_PROFILE_ID,              //  Profile ID
  DEV_ID_COLLECTOR,           //  Device ID
  DEVICE_VERSION_COLLECTOR,   //  Device Version
  0,                          //  Reserved
  NUM_IN_CMD_COLLECTOR,       //  Number of Input Commands
  (cId_t *) zb_InCmdList,     //  Input Command List
  NUM_OUT_CMD_COLLECTOR,      //  Number of Output Commands
  (cId_t *) zb_OutCmdList     //  Output Command List
};


/******************************************************************************
 * FUNCTIONS
 */

/******************************************************************************
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
  if( event & ZB_ENTRY_EVENT ){
    // Initialise UART
    initUart(uartRxCB);
    initApp();
    
    println("System ENTRY");
    // blind LED 1 to indicate starting/joining a network
    HalLedBlink ( HAL_LED_1, 0, 50, 500 );
    HalLedSet( HAL_LED_2, HAL_LED_MODE_OFF );

    // Start the device
    zb_StartRequest();
  }
  else if(event & SENSOR_TASK_EVENT){
    reportLightSensor();
    reportLockStatus();
    osal_start_timerEx( sapi_TaskID, SENSOR_TASK_EVENT, SENSOR_POLL_INTERVAL);
  }
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
  //Print some debug info when key 2 is pressed.
  if ( keys & HAL_KEY_SW_2 ) {
    uint16 val = HalAdcRead(HAL_ADC_CHANNEL_3, HAL_ADC_RESOLUTION_12);
    print("LDR = ");
    println(itoa(val));
    print("Light level: ");
    println(itoa(isLightLevelLow()));
    print("LOCK = ");
    println(itoa(isLockOpen()));
    println("*************************");
  }
}

/******************************************************************************
 * @fn          initApp
 *
 * @brief       Initial function call to initialize the application. 
 *              This function is called after the ZB_ENTRY_EVENT OS call.
 *              
 * @param       none
 *                    
 *                      
 * @return      none
 */
void initApp(void){
    initLamp(0,4);
    initLightSensor(3,1000);
    initLock(0,7,0,5);
}

/******************************************************************************
 * @fn          zb_StartConfirm
 *
 * @brief       The zb_StartConfirm callback is called by the ZigBee stack
 *              after a start request operation completes.
 *
 * @param       status - The status of the start operation.  Status of
 *                       ZB_SUCCESS indicates the start operation completed
 *                       successfully.  Else the status is an error code.
 *
 * @return      none
 */
void zb_StartConfirm( uint8 status ){
  // If the device sucessfully started, change state to running
  if ( status == ZB_SUCCESS ) {
    println("System START SUCCESS");
    // Set LED 1 to indicate that node is operational on the network
    HalLedSet( HAL_LED_1, HAL_LED_MODE_ON );
    
    // Turn ON Allow Bind mode infinitly.
    zb_AllowBind( 0xFF );
    HalLedSet( HAL_LED_2, HAL_LED_MODE_ON );
    println("System Ready to bind");
    
    //Start the poll events to periodically test the light and door sensors.
    osal_start_timerEx( sapi_TaskID,SENSOR_TASK_EVENT, SENSOR_POLL_INTERVAL);
  }
  else{
    //Debug info.
    println("System START FAIL");
  }
}

/******************************************************************************
 * @fn          zb_SendDataConfirm
 *
 * @brief       The zb_SendDataConfirm callback function is called by the
 *              ZigBee stack after a send data operation completes
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
  //Debug info
  print("System Bind confirmed (commandID = ");
  print(itoa(commandId));
  print(" status = ");
  print(itoa(status));
  println(")");
  
  //If binding to a device failed, we try again.
  if(status != ZB_SUCCESS){
    bindDevice(commandId);
  }
  //If binding succeeded, we note wich device we are bound to.
  else{
    if(isLampDataCommand(commandId)){
      isLightEndPointBound = TRUE;
    }
    else if(isLockDataCommand(commandId)){
      isLockEndPointBound = TRUE;
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
  //Only debug info.
  print("System Allow Bind confirmed (source = ");
  print(itoa(source));
  println(")");
  (void)source;
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
void zb_ReceiveDataIndication( uint16 source, uint16 command, uint16 len, uint8 *pData  ){
  //Debug info
  print("Network Incoming data (id = ");
  print(itoa(command));
  print(" len = ");
  print(itoa(len));
  print(" data = ");
  print(pData);
  println(")");
  
  //Bind back request from remote device.
  //These are the same for both devices.
  if(isBindBackRequest(pData)){
    bindDevice(command);
  }
  //Status request from remote device.
  else if(isStatusRequest(pData)){
    //Default status.
    uint8 status = 0;
    //Update the door lock status.
    if(isLockDataCommand(command)){
      status = isLockOpen();
    }
    //Update the lamp status.
    else if(isLampDataCommand(command)){
      status = isLampOn();
    }
    //Send the status back to remote devce.
    sendStatus(command, status);  
  }
  //Button pressed notify from remote device.
  else if(isButtonPressedRequest(pData)){
    //Butten pressed on lock remote device. 
    if(isLockDataCommand(command)){
       toggleLock();
       sendStatus(command, isLockOpen());
    }
    //Button pressed on lamp remote device
    else if(isLampDataCommand(command)){
      bool isLowLight = isLightLevelLow();
      if(isLowLight){
        toggleLamp();
      }
      else{
        setLamp(0);
      }
      sendStatus(command, isLampOn());
    }
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

/******************************************************************************
 * @fn          reportLightSensor
 *
 * @brief       Function periodically called to poll the light sensor and send  
 *              the new data to the remote if need be.
 *
 * @param       none
 *
 * @return      none
 */
void reportLightSensor(){
   bool isLightLevelHigh = !isLightLevelLow();
   //Test if the lamp is on.
   //No need to do anyting if it's alrdy off.
   if(isLampOn()){
      //Test if the lamp needs to be turned off.
      if(isLightLevelHigh){
        //Turn off lamp.
        setLamp(0);
         //We only need to send if there is a device bound that we can send too.
         if(isLightEndPointBound){
           sendStatus(LAMP_DATA_CMD_ID, 0);
         }
      }
   }
}

/******************************************************************************
 * @fn          reportLightSensor
 *
 * @brief       Function periodically called to poll the lock sensor and send  
 *              the new data to the remote if need be.
 *
 * @param       none
 *
 * @return      none
 */
void reportLockStatus(){
  static bool lastLockStatus = -1;
  bool currentLockStatus = isLockOpen();
  //Test if status of the lock has changed.
  if(lastLockStatus != currentLockStatus){
    //Update the lock status.
    lastLockStatus = currentLockStatus;
    //We only need to send if there is a device bound that we can send too.
    if(isLockEndPointBound){
       sendStatus(LOCK_DATA_CMD_ID, currentLockStatus);
    }
  }
}

