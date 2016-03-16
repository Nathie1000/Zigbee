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
#define APP_INIT                            0
#define APP_START                           2

// Application osal event identifiers
#define MY_START_EVT                        0x0001

/******************************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint16              source;
  uint16              parent;
  uint8               temp;
  uint8               voltage;
} gtwData_t;

/******************************************************************************
 * LOCAL VARIABLES
 */

static uint8 appState =             APP_INIT;
static uint8 myStartRetryDelay =    10;          // milliseconds
static gtwData_t gtwData;

/******************************************************************************
 * LOCAL FUNCTIONS
 */

static uint8 calcFCS(uint8 *pBuf, uint8 len);
static void sysPingReqRcvd(void);
static void sysPingRsp(void);
static void sendGtwReport(gtwData_t *gtwData);

/******************************************************************************
 * GLOBAL VARIABLES
 */

// Inputs and Outputs for Collector device
#define NUM_OUT_CMD_COLLECTOR           0
#define NUM_IN_CMD_COLLECTOR            2

// List of output and input commands for Collector device
const cId_t zb_InCmdList[NUM_IN_CMD_COLLECTOR] =
{
  SENSOR_REPORT_CMD_ID,
  SENSOR_BUTTON_CMD_ID
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
  (cId_t *) NULL              //  Output Command List
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
void zb_HandleOsalEvent( uint16 event )
{
  if( event & SYS_EVENT_MSG ){
  }

  if( event & ZB_ENTRY_EVENT )
  {
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

  if ( event & MY_START_EVT )
  {
    zb_StartRequest();
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
void zb_HandleKeys( uint8 shift, uint8 keys )
{
  if ( keys & HAL_KEY_SW_1 ){
    MCU_IO_SET_LOW(0,4);
  }
  
  if ( keys & HAL_KEY_SW_2 ) {
    uint16 val = HalAdcRead(HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_12);
    print("LDR = ");
    println(itoa(val));
    
    MCU_IO_SET_HIGH(0,4);
  }
}

/******************************************************************************
 * @fn          initApp
 *
 * @brief       Initial function call to initialize the application this function
 *              is triggerd after the ZB_ENTRY_EVENT OS call.
 * @param       none
 *                    
 *                      
 * @return      none
 */
void initApp(void){
   MCU_IO_DIR_OUTPUT(0,4);
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
void zb_StartConfirm( uint8 status )
{
  // If the device sucessfully started, change state to running
  if ( status == ZB_SUCCESS )
  {
     println("System START SUCCESS");
    // Set LED 1 to indicate that node is operational on the network
    HalLedSet( HAL_LED_1, HAL_LED_MODE_ON );
    
    // Turn ON Allow Bind mode infinitly
    zb_AllowBind( 0xFF );
    HalLedSet( HAL_LED_2, HAL_LED_MODE_ON );
    println("System Ready to bind");

    // Change application state
    //appState = APP_START;
  }
  else
  {
     println("System START FAIL");
    // Try again later with a delay
    osal_start_timerEx( sapi_TaskID, MY_START_EVT, myStartRetryDelay );
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
void zb_SendDataConfirm( uint8 handle, uint8 status )
{
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
void zb_BindConfirm( uint16 commandId, uint8 status )
{
  
  print("System Bind confirmed (commandID = ");
  print(itoa(commandId));
  print(" status = ");
  print(itoa(status));
  println(")");
  
  (void)commandId;
  (void)status;
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
void zb_AllowBindConfirm( uint16 source )
{
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
void zb_FindDeviceConfirm( uint8 searchType, uint8 *searchKey, uint8 *result )
{
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
  print("Network Incoming data (id = ");
  print(itoa(command));
  print(" len = ");
  print(itoa(len));
  print(" data = ");
  print(pData);
  println(")");
  
  if(command == SENSOR_REPORT_CMD_ID){
    (void)len;

    gtwData.parent = BUILD_UINT16(pData[SENSOR_PARENT_OFFSET+ 1], pData[SENSOR_PARENT_OFFSET]);
    gtwData.source = source;
    gtwData.temp = *pData;
    gtwData.voltage = *(pData+1);

    // Flash LED 2 once to indicate data reception
    HalLedSet ( HAL_LED_2, HAL_LED_MODE_FLASH );
  
    //MCU_IO_SET_HIGH(0, 4);

    // Send gateway report
    //sendGtwReport(&gtwData);
    //println("");
  }
  else if(command == SENSOR_BUTTON_CMD_ID){
    //println("Toggle LED");
    static bool isOn = false;
    if(isOn){
      MCU_IO_SET_HIGH(0,4);
    }
    else{
      MCU_IO_SET_LOW(0,4);
    }
    isOn = !isOn;
    
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
void uartRxCB( uint8 port, uint8 event )
{
  (void)port;

  uint8 pBuf[RX_BUF_LEN];
  uint16 cmd;
  uint16 len;

  if ( event != HAL_UART_TX_EMPTY )
  {
    // Read from UART
    len = HalUARTRead( HAL_UART_PORT_0, pBuf, RX_BUF_LEN );

    if ( len > 0 )
    {
      cmd = BUILD_UINT16(pBuf[SYS_PING_CMD_OFFSET + 1], pBuf[SYS_PING_CMD_OFFSET]);

      if( (pBuf[FRAME_SOF_OFFSET] == CPT_SOP) && (cmd == SYS_PING_REQUEST) )
      {
        sysPingReqRcvd();
      }
    }
  }
}

/******************************************************************************
 * @fn          sysPingReqRcvd
 *
 * @brief       Ping request received
 *
 * @param       none
 *
 * @return      none
 */
static void sysPingReqRcvd(void)
{
   sysPingRsp();
}

/******************************************************************************
 * @fn          sysPingRsp
 *
 * @brief       Build and send Ping response
 *
 * @param       none
 *
 * @return      none
 */
static void sysPingRsp(void)
{
  uint8 pBuf[SYS_PING_RSP_LENGTH];

  // Start of Frame Delimiter
  pBuf[FRAME_SOF_OFFSET] = CPT_SOP;

  // Length
  pBuf[FRAME_LENGTH_OFFSET] = 2;

  // Command type
  pBuf[FRAME_CMD0_OFFSET] = LO_UINT16(SYS_PING_RESPONSE);
  pBuf[FRAME_CMD1_OFFSET] = HI_UINT16(SYS_PING_RESPONSE);

  // Stack profile
  pBuf[FRAME_DATA_OFFSET] = LO_UINT16(STACK_PROFILE);
  pBuf[FRAME_DATA_OFFSET + 1] = HI_UINT16(STACK_PROFILE);

  // Frame Check Sequence
  pBuf[SYS_PING_RSP_LENGTH - 1] = calcFCS(&pBuf[FRAME_LENGTH_OFFSET], (SYS_PING_RSP_LENGTH - 2));

  // Write frame to UART
  HalUARTWrite(HAL_UART_PORT_0,pBuf, SYS_PING_RSP_LENGTH);
}

/******************************************************************************
 * @fn          sendGtwReport
 *
 * @brief       Build and send gateway report
 *
 * @param       none
 *
 * @return      none
 */
static void sendGtwReport(gtwData_t *gtwData)
{
  uint8 pFrame[ZB_RECV_LENGTH];

  // Start of Frame Delimiter
  pFrame[FRAME_SOF_OFFSET] = CPT_SOP; // Start of Frame Delimiter

  // Length
  pFrame[FRAME_LENGTH_OFFSET] = 10;

  // Command type
  pFrame[FRAME_CMD0_OFFSET] = LO_UINT16(ZB_RECEIVE_DATA_INDICATION);
  pFrame[FRAME_CMD1_OFFSET] = HI_UINT16(ZB_RECEIVE_DATA_INDICATION);

  // Source address
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_SRC_OFFSET] = LO_UINT16(gtwData->source);
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_SRC_OFFSET+ 1] = HI_UINT16(gtwData->source);

  // Command ID
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_CMD_OFFSET] = LO_UINT16(SENSOR_REPORT_CMD_ID);
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_CMD_OFFSET+ 1] = HI_UINT16(SENSOR_REPORT_CMD_ID);

  // Length
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_LEN_OFFSET] = LO_UINT16(4);
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_LEN_OFFSET+ 1] = HI_UINT16(4);

  // Data
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_DATA_OFFSET] = gtwData->temp;
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_DATA_OFFSET+ 1] = gtwData->voltage;
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_DATA_OFFSET+ 2] = LO_UINT16(gtwData->parent);
  pFrame[FRAME_DATA_OFFSET + ZB_RECV_DATA_OFFSET+ 3] = HI_UINT16(gtwData->parent);

  // Frame Check Sequence
  pFrame[ZB_RECV_LENGTH - 1] = calcFCS(&pFrame[FRAME_LENGTH_OFFSET], (ZB_RECV_LENGTH - 2) );

  // Write report to UART
  HalUARTWrite(HAL_UART_PORT_0,pFrame, ZB_RECV_LENGTH);
}

/******************************************************************************
 * @fn          calcFCS
 *
 * @brief       This function calculates the FCS checksum for the serial message
 *
 * @param       pBuf - Pointer to the end of a buffer to calculate the FCS.
 *              len - Length of the pBuf.
 *
 * @return      The calculated FCS.
 ******************************************************************************
 */
static uint8 calcFCS(uint8 *pBuf, uint8 len)
{
  uint8 rtrn = 0;

  while ( len-- )
  {
    rtrn ^= *pBuf++;
  }

  return rtrn;
}
