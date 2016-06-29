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
*                                      Communication: Generic
*
* Filename      : probe_com_os.c
* Version       : V2.00
* Programmer(s) : BAN
* Note(s)       : (1) This file is the uC/OS-II layer for the uC/Probe Generic Communication Module.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <probe_com.h>
#include  <ucos_ii.h>


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

#if (PROBE_COM_SUPPORT_STR > 0) && (OS_SEM_EN > 0)
static  OS_EVENT  *ProbeCom_OS_Sem;
#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR > 0) && (OS_SEM_EN == 0)
  #error  "If PROBE_COM_SUPPORT_STR is set to DEF_TRUE, then semaphores MUST be enabled."
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
*                                          ProbeCom_OS_Init()
*
* Description : Create a semaphore to lock access to the string buffer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR > 0)
void  ProbeCom_OS_Init (void)
{
#if (OS_EVENT_NAME_SIZE > 10) && (OS_SEM_EN > 0)
    CPU_INT08U  err;
#endif


#if   (OS_SEM_EN > 0)
    ProbeCom_OS_Sem = OSSemCreate(1);
#if   (OS_EVENT_NAME_SIZE > 13)
    OSEventNameSet(ProbeCom_OS_Sem, (CPU_INT08U *)"uC/Probe Com", &err);
#elif (OS_EVENT_NAME_SIZE > 10)
    OSEventNameSet(ProbeCom_OS_Sem, (CPU_INT08U *)"Probe Com", &err);
#endif
#endif
}
#endif

/*
*********************************************************************************************************
*                                      ProbeCom_OS_Dly()
*
* Description : Delay for a certain number of milliseconds.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR > 0)
void  ProbeCom_OS_Dly (CPU_INT16U  dly)
{
    if (dly >= 1000) {
        OSTimeDlyHMSM(0, 0, 1, 0);
    } else {
        OSTimeDlyHMSM(0, 0, 0, dly);
    }
}
#endif


/*
*********************************************************************************************************
*                                       ProbeCom_OS_Pend()
*
* Description : Obtain write access to the string buffer, waiting (if required) until it the
*               buffer has been released by another task.
*
* Argument(s) : wait         Specify whether the function should wait until the semaphore is available
*                            or just check if the semaphore is available and, if it is, then accept it.
*
* Return(s)   : DEF_TRUE  if the semaphore was     obtained
*               DEF_FALSE if the semaphore was NOT obtained
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR > 0)
CPU_BOOLEAN  ProbeCom_OS_Pend (CPU_BOOLEAN wait)
{
#if (OS_SEM_EN > 0)
    CPU_INT08U  tmp;


    if (wait == DEF_TRUE) {
        OSSemPend(ProbeCom_OS_Sem, 0, &tmp);                        /* Wait for string buffer to be released                */

        if (tmp != OS_NO_ERR) {
            return (DEF_FALSE);
        } else {
            return (DEF_TRUE);
        }
    } else {
        tmp = OSSemAccept(ProbeCom_OS_Sem);                         /* Wait for string buffer to be released                */

        if (tmp == 0) {
            return (DEF_FALSE);
        } else {
            return (DEF_TRUE);
        }
    }
#else
    return (DEF_TRUE);
#endif
}
#endif


/*
*********************************************************************************************************
*                                       ProbeCom_OS_Post()
*
* Description : Release the lock on write access to the string buffer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR > 0)
void  ProbeCom_OS_Post (void)
{
#if (OS_SEM_EN > 0)
    OSSemPost(ProbeCom_OS_Sem);                                     /* String buffer is being released                      */
#endif
}
#endif
