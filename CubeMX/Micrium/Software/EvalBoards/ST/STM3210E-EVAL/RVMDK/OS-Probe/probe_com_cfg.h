/*
*********************************************************************************************************
*                                      uC/Probe Communication
*
*                           (c) Copyright 2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                              uC/Probe
*
*                                     Communication: Configuration
*
* Filename      : probe_com_cfg.h
* Version       : V2.00
* Programmer(s) : BAN
* Note(s)       : (1) This file contains configuration constants for uC/Probe Communication Modules.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       CHOOSE COMMUNICATION METHOD
*********************************************************************************************************
*/

#define  PROBE_COM_METHOD_RS232                    DEF_TRUE
#define  PROBE_COM_METHOD_TCPIP                    DEF_FALSE
#define  PROBE_COM_METHOD_USB                      DEF_FALSE

/*
*********************************************************************************************************
*                               CONFIGURE GENERAL COMMUNICATION PARAMETERS
*********************************************************************************************************
*/

#define  PROBE_COM_RX_MAX_SIZE                           128
#define  PROBE_COM_TX_MAX_SIZE                           128

#define  PROBE_COM_SUPPORT_WR                      DEF_FALSE
#define  PROBE_COM_SUPPORT_STR                     DEF_FALSE

                                                                /* If strings are supported ...                         */
#define  PROBE_COM_STR_BUF_SIZE                          128    /*   (a) Set size of string buffer.                     */

/*
*********************************************************************************************************
*                                    CONFIGURE STATISTICS AND COUNTERS
*********************************************************************************************************
*/

#define  PROBE_COM_STAT_EN                         DEF_ENABLED

/*
*********************************************************************************************************
*                                  CONFIGURE RS-232 SPECIFIC PARAMETERS
*********************************************************************************************************
*/

                                                                /* For the RS-232 communication method ...              */

#define  PROBE_RS232_PARSE_TASK                    DEF_TRUE     /*  (a) Set whether a task will handle parsing.         */

                                                                /*  (b) If a task will handle parsing ...               */
#define  PROBE_RS232_TASK_PRIO                             8    /*       (i) Set task priority.                         */
#define  PROBE_RS232_TASK_STK_SIZE                       256    /*      (ii) Set task stack size.                       */

#define  PROBE_RS232_RX_BUF_SIZE       PROBE_COM_RX_MAX_SIZE    /*  (c) Set Rx buffer size.                             */
#define  PROBE_RS232_TX_BUF_SIZE       PROBE_COM_TX_MAX_SIZE    /*  (d) Set Tx buffer size.                             */

#define  PROBE_RS232_UART_0                                1
#define  PROBE_RS232_UART_1                                2
#define  PROBE_RS232_UART_2                                3
#define  PROBE_RS232_UART_3                                4
#define  PROBE_RS232_UART_4                                5
#define  PROBE_RS232_UART_5                                6
#define  PROBE_RS232_UART_6                                7
#define  PROBE_RS232_UART_7                                8
#define  PROBE_RS232_UART_8                                9
#define  PROBE_RS232_UART_9                               10
#define  PROBE_RS232_UART_DBG                             63
#define  PROBE_RS232_COMM_SEL             PROBE_RS232_UART_1    /*  (e) Configure UART selection.                       */

/*
*********************************************************************************************************
*                                  CONFIGURE TCP-IP SPECIFIC PARAMETERS
*********************************************************************************************************
*/

                                                                /* For the TCP-IP communication method ...              */

#define  PROBE_TCPIP_PORT                               9930    /*  (a) Set listening port of the UDP server.           */

#define  PROBE_TCPIP_RX_BUF_SIZE       PROBE_COM_RX_MAX_SIZE    /*  (b) Set Rx buffer size.                             */
#define  PROBE_TCPIP_TX_BUF_SIZE       PROBE_COM_TX_MAX_SIZE    /*  (c) Set Tx buffer size.                             */

#define  PROBE_TCPIP_TASK_PRIO                             9    /*  (d) Set task priority.                              */
#define  PROBE_TCPIP_TASK_STK_SIZE                       256    /*  (e) Set task stack size.                            */

/*
*********************************************************************************************************
*                                    CONFIGURE USB SPECIFIC PARAMETERS
*********************************************************************************************************
*/

                                                                /* For the USB communication method ...                 */

#define  PROBE_USB_RX_BUF_SIZE         PROBE_COM_RX_MAX_SIZE    /*  (a) Set Rx buffer size.                             */
#define  PROBE_USB_TX_BUF_SIZE         PROBE_COM_TX_MAX_SIZE    /*  (b) Set Tx buffer size.                             */

#define  PROBE_USB_TASK_PRIO                              10    /*  (c) Set task priority.                              */
#define  PROBE_USB_TASK_STK_SIZE                         256    /*  (d) Set task stack size.                            */

#define  PROBE_USB_INIT_STACK                      DEF_TRUE     /*  (e) The USB comm module will init stack.            */

/*
*********************************************************************************************************
*                              CONFIGURE HARDWARE SPECIFIC PARAMETERS
*********************************************************************************************************
*/

#define  PROBE_RS232_UART_1_REMAP                DEF_FALSE
#define  PROBE_RS232_UART_2_REMAP                DEF_TRUE
#define  PROBE_RS232_UART_3_REMAP_PARTIAL        DEF_FALSE
#define  PROBE_RS232_UART_3_REMAP_FULL           DEF_FALSE
