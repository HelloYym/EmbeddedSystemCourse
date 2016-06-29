/*
*********************************************************************************************************
*                                         uC/Probe Communication
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
*                                                uC/Probe
*
*                                         Communication: RS-232
*
* Filename      : probe_rs232.h
* Version       : V2.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               PROBE_RS232 present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  PROBE_RS232_PRESENT                                    /* See Note #1.                                         */
#define  PROBE_RS232_PRESENT


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef    PROBE_RS232_MODULE
#define   PROBE_RS232_EXT
#else
#define   PROBE_RS232_EXT  extern
#endif


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include  <probe_com.h>
#include  <probe_rs232c.h>


#if (PROBE_COM_METHOD_RS232 == DEF_ENABLED)

/*
*********************************************************************************************************
*                                                 DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
                                                                    /* ---------------- Byte counters --------------------- */
PROBE_RS232_EXT  CPU_INT32U  ProbeRS232_RxCtr;                      /*  ... Number of bytes received                        */
PROBE_RS232_EXT  CPU_INT32U  ProbeRS232_TxCtr;                      /*  ... Number of bytes transmitted                     */
#endif


/*
*********************************************************************************************************
*                                                 MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  ProbeRS232_Init          (CPU_INT32U   baud_rate);            /* Initialize uC/Probe RS-232 Communication Module      */
void  ProbeRS232_RxHandler     (CPU_INT08U   rx_data);              /* Processor independent Rx handler                     */
void  ProbeRS232_TxHandler     (void);                              /* Processor independent Tx handler                     */

#if (PROBE_RS232_PARSE_TASK == DEF_TRUE)
void  ProbeRS232_Task          (void        *p_arg);
#endif


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*                                    DEFINED IN PORT's probe_rs232c.c
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget    (CPU_INT32U   baud_rate);            /* Initialize the target specifics                      */

void  ProbeRS232_RxTxISRHandler(void);                              /* Code to process Rx or Tx character                   */

void  ProbeRS232_RxISRHandler  (void);                              /* Code to process character received                   */
void  ProbeRS232_RxIntEn       (void);                              /* Enable  Rx interrupts                                */
void  ProbeRS232_RxIntDis      (void);                              /* Disable Rx interrupts                                */

void  ProbeRS232_TxISRHandler  (void);                              /* Code to process next character to send               */
void  ProbeRS232_TxIntEn       (void);                              /* Enable  Tx interrupts                                */
void  ProbeRS232_TxIntDis      (void);                              /* Disable Tx interrupts                                */
void  ProbeRS232_Tx1           (CPU_INT08U   c);                    /* Function to send a single character                  */


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*                                    DEFINED IN OS's probe_rs232_os.c
*********************************************************************************************************
*/

#if (PROBE_RS232_PARSE_TASK == DEF_TRUE)
void  ProbeRS232_OS_Init       (void);
void  ProbeRS232_OS_Pend       (void);
void  ProbeRS232_OS_Post       (void);
#endif


/*
*********************************************************************************************************
*                                          CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_PARSE_TASK
  #error  "PROBE_RS232_PARSE_TASK            not #define'd in 'probe_com_cfg.h'     "
  #error  "                           [MUST be  DEF_TRUE   Packet parsed in task   ]"
  #error  "                           [     ||  DEF_FALSE  Packet parsed in ISR    ]"

#elif    ((PROBE_RS232_PARSE_TASK != DEF_TRUE ) && \
          (PROBE_RS232_PARSE_TASK != DEF_FALSE))
  #error  "PROBE_RS232_PARSE_TASK      illegally #define'd in 'probe_com_cfg.h'     "
  #error  "                           [MUST be  DEF_TRUE   Packet parsed in task   ]"
  #error  "                           [     ||  DEF_FALSE  Packet parsed in ISR    ]"

#elif     (PROBE_RS232_PARSE_TASK == DEF_TRUE)

#ifndef    PROBE_RS232_TASK_PRIO
  #error  "PROBE_RS232_TASK_PRIO             not #define'd in 'probe_com_cfg.h'     "
#endif

#ifndef    PROBE_RS232_TASK_STK_SIZE
  #error  "PROBE_RS232_TASK_STK_SIZE         not #define'd in 'probe_com_cfg.h'     "
#endif

#endif



#ifndef    PROBE_RS232_RX_BUF_SIZE
  #error  "PROBE_RS232_RX_BUF_SIZE           not #define'd in 'probe_com_cfg.h'     "
#endif



#ifndef    PROBE_RS232_TX_BUF_SIZE
  #error  "PROBE_RS232_TX_BUF_SIZE           not #define'd in 'probe_com_cfg.h'     "
#endif


#ifndef    PROBE_RS232_COMM_SEL
  #error  "PROBE_RS232_COMM_SEL              not #define'd in 'probe_com_cfg.h'     "
#endif


#endif


/*
*********************************************************************************************************
*                                              MODULE END
*
* Note(s) : See 'MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of PROBE_COM_RS232 module include (see Note #1). */
