/*
*********************************************************************************************************
*                                     uC/Probe uC/OS-II Plug-in
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
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
*                                         Plug-in for uC/OS-II
*
* Filename      : os_probe.c
* Version       : V2.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#define  OS_PROBE_MODULE
#include <os_probe.h>


/*
*********************************************************************************************************
*                                             LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
static  OS_STK  OSProbe_TaskStk[OS_PROBE_TASK_STK_SIZE];
#endif


/*
*********************************************************************************************************
*                                        LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
static  void  OSProbe_InitOS(void);
static  void  OSProbe_Task  (void  *p_arg);
#endif


/*
*********************************************************************************************************
*                                       LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             OSProbe_Init()
*
* Description : Initialize the Probe Plug-In for uC/OS-II.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  OSProbe_Init (void)
{
#if (OS_PROBE_TASK > 0)
    OSProbe_SetDelay(100);
    OSProbe_SetCallback((void (*)(void))0);                     /* Force terminal callback function to 'nothing'.       */

    OSProbe_InitOS();
#endif

#if (OS_PROBE_HOOKS_EN > 0)
    OSProbe_TmrInit();

    OSProbe_CyclesCtr   = 0;
    OSProbe_TmrCntsPrev = 0;
#endif
}

/*
*********************************************************************************************************
*                                          OSProbe_SetCallback()
*
* Description : Set the callback function which will be invoked in OSProbe_Task().
*
* Argument(s) : call_back   Pointer to the callback function.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
void  OSProbe_SetCallback (void (*call_back)(void))
{
    OSProbe_CallbackFnct = call_back;
}
#endif


/*
*********************************************************************************************************
*                                           OSProbe_SetDelay()
*
* Description : Set the delay used in OSProbe_Task().
*
* Argument(s) : delay       Delay, in milliseconds.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
void  OSProbe_SetDelay (INT16U  delay)
{
    OSProbe_Delay = delay;
}
#endif


/*
*********************************************************************************************************
*                                         OSProbe_TimeGetCycles()
*
* Description : Get time as accurately as possible, stored in a 32-bit variable.
*
* Argument(s) : none.
*
* Return(s)   : A 32-bit representation of time.
*
* Caller(s)   : OSProbe_TaskSwHook(),
*               OSProbe_TaskCreateHook().
*
* Note(s)     : (1) Since the cycles count returned by this function will eventually overflow a 32-bit
*                   integer, it should only be used for comparative time lapse measurements (e.g., to
*                   determine a time lapse between two events which can be compared to similarly
*                   calculated time lapses).  In such a measurement, the difference between two cycle
*                   counts will be computed.  The application MUST guarantee that this difference does
*                   not overflow a 32-bit integer.  For example, if the underlying timer increments at a
*                   rate of 100MHz, then the maximum time lapse that can be measured is
*
*                                2^32 - 1
*                       tmax = ------------ s = 42.9497 s
*                               100 * 10^6
*
*               (2) When using a 16-bit timer, this function MUST be called with sufficient frequency
*                   that timer overflows do not occur.  If necessary, the timer should be configured with
*                   a sufficient prescaler in order to decrease the probability of timer overflows.
*
*                   For example, a 16-bit timer incrementing at 48-MHz with a prescaler of 128 will
*                   require that this function be called at
*
*                                   48 * 10^6
*                       freqmin = ------------- Hz = 5.72 Hz
*                                  128 * 65536
*
*                   A possible solution is that this would be called from the tick handler of the
*                   application's OS (assuming the tick rate is greater than 5.72 Hz).
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
INT32U  OSProbe_TimeGetCycles (void)
{
    INT32U     cycles;
#if (OS_PROBE_TMR_32_BITS > 0)
    INT32U     cnts32;
    INT32U     cnts32_delta;
#else
    INT16U     cnts16;
    INT16U     cnts16_delta;
#endif
#if (OS_CRITICAL_METHOD == 3)                                   /* Allocate storage for CPU status register.            */
    OS_CPU_SR  cpu_sr = 0;
#endif


    OS_ENTER_CRITICAL();
#if (OS_PROBE_TMR_32_BITS > 0)
    cnts32               = OSProbe_TmrRd();                     /* Read current counts of the free running counter.     */
    cnts32_delta         = cnts32 - OSProbe_TmrCntsPrev;
    OSProbe_TmrCntsPrev  = cnts32;                              /* Save current counts for next time.                   */
    OSProbe_CyclesCtr   += cnts32_delta;
#else
    cnts16               = (INT16U)OSProbe_TmrRd();             /* Read current counts of the free running counter.     */
    cnts16_delta         = cnts16 - OSProbe_TmrCntsPrev;
    OSProbe_TmrCntsPrev  = cnts16;                              /* Save current counts for next time.                   */
    OSProbe_CyclesCtr   += (INT32U)cnts16_delta;
#endif
    cycles               = OSProbe_CyclesCtr;
    OS_EXIT_CRITICAL();

    return (cycles);
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               TASK HOOKS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        OSProbe_TaskCreateHook()
*
* Description : This function is called when a task is created.
*
* Argument(s) : ptcb        Pointer to the task control block of the task being created.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskCreateHook().
*
* Note(s)     : (1) Interrupts are disabled during this call.
*
*               (2) This MUST be called from applications's task create hook function App_TaskCreateHook().
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
void  OSProbe_TaskCreateHook (OS_TCB *ptcb)
{
    ptcb->OSTCBCyclesStart = OSProbe_TimeGetCycles();           /* Get the current start time for this task.            */
    ptcb->OSTCBCyclesTot   = 0;                                 /* Update the task's total execution time.              */
}
#endif


/*
*********************************************************************************************************
*                                          OSProbe_TaskSwHook()
*
* Description : This function is called when a task switch is performed.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskSwHook().
*
* Note(s)     : (1) Interrupts are disabled during this call.
*
*               (2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                   task being switched out (i.e. the preempted task).
*
*               (3) This MUST be called from application's task switch hook function App_TaskSwHook().
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
void  OSProbe_TaskSwHook (void)
{
    INT32U  cycles;


    cycles                         = OSProbe_TimeGetCycles();   /* This task is done.                                   */
    OSTCBCur->OSTCBCyclesTot      += cycles - OSTCBCur->OSTCBCyclesStart;
    OSTCBHighRdy->OSTCBCyclesStart = cycles;                    /* Save absolute #cycles at task activation.            */
}
#endif


/*
*********************************************************************************************************
*                                           OSProbe_TickHook()
*
* Description : This function is called every tick.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TimeTickHook().
*
* Note(s)     : (1) Interrupts may or may not be ENABLED during this call.
*
*               (2) This MUST be called from user's time tick hook function App_TimeTickHook().
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
void  OSProbe_TickHook (void)
{
    (void)OSProbe_TimeGetCycles();
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            OSProbe_InitOS()
*
* Description : Create the task for the Probe Plug-In for uC/OS-II.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : OSProbe_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
static  void  OSProbe_InitOS (void)
{
    INT8U  err;


#if (OS_TASK_CREATE_EXT_EN > 0)
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreateExt((void (*)(void *)) OSProbe_Task,
                          (void          * ) 0,
                          (OS_STK        * )&OSProbe_TaskStk[OS_PROBE_TASK_STK_SIZE - 1],
                          (INT8U           ) OS_PROBE_TASK_PRIO,
                          (INT16U          ) OS_PROBE_TASK_PRIO,
                          (OS_STK        * )&OSProbe_TaskStk[0],
                          (INT32U          ) OS_PROBE_TASK_STK_SIZE,
                          (void          * ) 0,
                          (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
    #else
    err = OSTaskCreateExt((void (*)(void *)) OSProbe_Task,
                          (void          * ) 0,
                          (OS_STK        * )&OSProbe_TaskStk[0],
                          (INT8U           ) OS_PROBE_TASK_PRIO,
                          (INT16U          ) OS_PROBE_TASK_PRIO,
                          (OS_STK        * )&OSProbe_TaskStk[OS_PROBE_TASK_STK_SIZE - 1],
                          (INT32U          ) OS_PROBE_TASK_STK_SIZE,
                          (void          * ) 0,
                          (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
    #endif
#else
    #if (OS_STK_GROWTH == 1)
    err = OSTaskCreate((void (*)(void *)) OSProbe_Task,
                       (void          * ) 0,
                       (OS_STK        * )&OSProbe_TaskStk[OS_PROBE_TASK_STK_SIZE - 1],
                       (INT8U           ) OS_PROBE_TASK_PRIO);
    #else
    err = OSTaskCreate((void (*)(void *)) OSProbe_Task,
                       (void          * ) 0,
                       (OS_STK        * )&OSProbe_TaskStk[0],
                       (INT8U           ) OS_PROBE_TASK_PRIO);
    #endif
#endif

#if (OS_TASK_NAME_SIZE > 15)
    OSTaskNameSet(OS_PROBE_TASK_PRIO, (INT8U *)"Probe OS PlugIn", &err);
#endif

    (void)&err;
}
#endif


/*
*********************************************************************************************************
*                                             OSProbe_Task()
*
* Description : Updates task CPU usage and task stack usage statistics and calls a user-specified
*               callback functions, if the user sets this function.
*
* Argument(s) : p_arg       Argument passed to OSProbe_Task() by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (OS_PROBE_TASK > 0)
static  void  OSProbe_Task (void *p_arg)
{
            OS_TCB  *ptcb;
            INT16U   i;
            INT32U   cycles_tot;
    static  INT32U   cycles_dif[OS_MAX_TASKS];
    static  INT32U   cycles_tot_last[OS_MAX_TASKS];
#if (OS_PROBE_USE_FP == 0)
            INT32U   max;
#endif


    (void)p_arg;

                                                                /* Initialize stored CyclesTot values.                  */
    for (i = 0; i < OS_MAX_TASKS; i++) {
        cycles_tot_last[i]      = 0;
        OSProbe_TaskStkUsage[i] = 0;
        OSProbe_TaskCPUUsage[i] = 0;
    }

    while (1) {
        OSTimeDlyHMSM(0, 0, 0, OSProbe_Delay);
        if (OSProbe_CallbackFnct != (void (*)(void))0) {
            OSProbe_CallbackFnct();
        }

                                                                /* Update task CPU usage                                */
        i          = 0;
        cycles_tot = 0;
        ptcb       = &OSTCBTbl[0];                              /*  ... Get pointer to first TCB ...                    */

        while ((i    < OS_MAX_TASKS) &&
               (ptcb != (OS_TCB *)0) &&
               (ptcb != (OS_TCB *)1)) {
                                                                /*  ... Calculate new CyclesDif, the number of cycles   */
                                                                /*  ... used by the task since the last reading.  Half  */
                                                                /*  ... the previous value is added to provide some     */
                                                                /*  ... hysteresis, thereby reducing the natural        */
                                                                /*  ... "jitter" in the data.                           */
            cycles_dif[i]       = (ptcb->OSTCBCyclesTot - cycles_tot_last[i]) / 2 + (cycles_dif[i] / 2);
            cycles_tot_last[i]  = ptcb->OSTCBCyclesTot;
            cycles_tot         += cycles_dif[i];

            if (ptcb->OSTCBStkSize == 0) {
                OSProbe_TaskStkUsage[i] = 0;
            } else {
#if (OS_PROBE_USE_FP > 0)
#if (OS_STK_GROWTH == 1)
                OSProbe_TaskStkUsage[i] = (FP32)(((INT32U)(ptcb->OSTCBStkBase) - (INT32U)(ptcb->OSTCBStkPtr))  * 100)
                                        / ((ptcb->OSTCBStkSize) * sizeof (OS_STK));
#else
                OSProbe_TaskStkUsage[i] = (FP32)(((INT32U)(ptcb->OSTCBStkPtr)  - (INT32U)(ptcb->OSTCBStkBase)) * 100)
                                        / ((ptcb->OSTCBStkSize) * sizeof (OS_STK));
#endif
#else
                max = ((ptcb->OSTCBStkSize) * sizeof (OS_STK)) / 100L;

#if (OS_STK_GROWTH == 1)
                OSProbe_TaskStkUsage[i] = (INT16U)(((INT32U)(ptcb->OSTCBStkBase) - (INT32U)(ptcb->OSTCBStkPtr))  / max);
#else
                OSProbe_TaskStkUsage[i] = (INT16U)(((INT32U)(ptcb->OSTCBStkPtr)  - (INT32U)(ptcb->OSTCBStkBase)) / max);
#endif
#endif
            }

            ptcb = ptcb->OSTCBPrev;

            i++;
        }

#if (OS_PROBE_USE_FP == 0)
        max = cycles_tot / 100L;
#endif
                                                                /*  ... For each task, calculate percent CPU usage.     */
        for (i = 0; i < OS_MAX_TASKS; i++) {
#if (OS_PROBE_USE_FP > 0)
            OSProbe_TaskCPUUsage[i] = (FP32)(cycles_dif[i] * 100) / cycles_tot;
#else
            OSProbe_TaskCPUUsage[i] = (INT16U)(cycles_dif[i] / max);
#endif
        }
    }
}
#endif
