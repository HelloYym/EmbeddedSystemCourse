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
*                                      Communication: RS-232
*
* Filename      : probe_rs232_os.c
* Version       : V2.00
* Programmer(s) : BAN
* Note(s)       : (1) This file is the uC/OS-II layer for the uC/Probe RS-232 Communication Module.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <probe_com.h>
#include  <probe_rs232.h>
#include  <ucos_ii.h>

#if (PROBE_COM_METHOD_RS232 > 0)

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (PROBE_RS232_PARSE_TASK == DEF_ENABLED)
static  OS_STK     ProbeRS232_OS_TaskStk[PROBE_RS232_TASK_STK_SIZE];    /* Probe RS-232 task stack.                         */

#if (OS_SEM_EN > 0)
static  OS_EVENT  *ProbeRS232_OS_Sem;                                   /* Packet receive signal.                           */
#endif

#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (PROBE_RS232_PARSE_TASK == DEF_ENABLED)
static  void  ProbeRS232_OS_Task(void *p_arg);                          /* Probe RS-232 task.                               */
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if (PROBE_RS232_PARSE_TASK == DEF_ENABLED) && (OS_SEM_EN == 0)
  #error  "If PROBE_RS232_PARSE_TASK is set to DEF_TRUE, then semaphores MUST be enabled."
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
**                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         ProbeRS232_OS_Init()
*
* Description : Create RTOS objects for RS-232 communication.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_PARSE_TASK == DEF_ENABLED)
void  ProbeRS232_OS_Init (void)
{
    CPU_INT08U  err;


#if (OS_SEM_EN > 0)
    ProbeRS232_OS_Sem = OSSemCreate(0);
#if   (OS_EVENT_NAME_SIZE > 16)
    OSEventNameSet(ProbeRS232_OS_Sem, (CPU_INT08U *)"uC/Probe RS-232", &err);
#elif (OS_EVENT_NAME_SIZE > 13)
    OSEventNameSet(ProbeRS232_OS_Sem, (CPU_INT08U *)"Probe RS-232", &err);
#endif
#endif

#if (OS_TASK_CREATE_EXT_EN > 0)
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreateExt( ProbeRS232_OS_Task,
                           (void *)0,
                          &ProbeRS232_OS_TaskStk[PROBE_RS232_TASK_STK_SIZE - 1],    /* Set Top-Of-Stack.                    */
                           PROBE_RS232_TASK_PRIO,
                           PROBE_RS232_TASK_PRIO,
                          &ProbeRS232_OS_TaskStk[0],                                /* Set Bottom-Of-Stack.                 */
                           PROBE_RS232_TASK_STK_SIZE,
                           (void *)0,                                               /* No TCB extension.                    */
                           OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);              /* Enable stack checking + clear stack. */
    #else
    err = OSTaskCreateExt( ProbeRS232_OS_Task,
                           (void *)0,
                          &ProbeRS232_OS_TaskStk[0],                                /* Set Top-Of-Stack.                    */
                           PROBE_RS232_TASK_PRIO,
                           PROBE_RS232_TASK_PRIO,
                          &ProbeRS232_OS_TaskStk[PROBE_RS232_TASK_STK_SIZE - 1],    /* Set Bottom-Of-Stack.                 */
                           PROBE_RS232_TASK_STK_SIZE,
                           (void *)0,                                               /* No TCB extension.                    */
                           OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);              /* Enable stack checking + clear stack. */
    #endif
#else
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreate( ProbeRS232_OS_Task,
                        (void *)0,
                       &ProbeRS232_OS_TaskStk[PROBE_RS232_TASK_STK_SIZE - 1],
                        PROBE_RS232_TASK_PRIO);
    #else
    err = OSTaskCreate( ProbeRS232_OS_Task,
                        (void *)0,
                       &ProbeRS232_OS_TaskStk[0],
                        PROBE_RS232_TASK_PRIO);
    #endif
#endif

#if   (OS_TASK_NAME_SIZE > 13)
    OSTaskNameSet(PROBE_RS232_TASK_PRIO, (CPU_INT08U *)"Probe RS-232", &err);
#endif
}
#endif


/*
*********************************************************************************************************
*                                       ProbeRS232_OS_Pend()
*
* Description : Wait for a packet to be received.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_PARSE_TASK == DEF_ENABLED)
void  ProbeRS232_OS_Pend (void)
{
#if (OS_SEM_EN > 0)
    CPU_INT08U  err;


    OSSemPend(ProbeRS232_OS_Sem, 0, &err);                          /* Wait for a packet to be received                     */
#endif
}
#endif


/*
*********************************************************************************************************
*                                       ProbeRS232_OS_Post()
*
* Description : Notify a pending task that a packet has been receieved.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_RxPkt().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_PARSE_TASK == DEF_ENABLED)
void  ProbeRS232_OS_Post (void)
{
#if (OS_SEM_EN > 0)
    OSSemPost(ProbeRS232_OS_Sem);                                   /* A packet has been received                           */
#endif
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
**                                          LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         ProbeRS232_OS_Task()
*
* Description : Task which waits for packets to be received, formalates responses, and begins transmission.
*
* Argument(s) : p_arg        Argument passed to ProbeRS232_OS_Task() by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_PARSE_TASK == DEF_ENABLED)
static  void  ProbeRS232_OS_Task (void *p_arg)
{
    ProbeRS232_Task(p_arg);
}
#endif

#endif
