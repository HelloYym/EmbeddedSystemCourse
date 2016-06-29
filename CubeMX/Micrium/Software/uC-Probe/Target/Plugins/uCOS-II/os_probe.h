/*
*********************************************************************************************************
*                                        uC/Probe uC/OS-II Plug-in
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
*                                                uC/Probe
*
*                                          Plug-in for uC/OS-II
*
* Filename      : os_probe.h
* Version       : V2.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               PROBE_OS present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  PROBE_OS_PRESENT                                       /* See Note #1.                                         */
#define  PROBE_OS_PRESENT


/*
*********************************************************************************************************
*                                     PROBE OS MODULE VERSION NUMBER
*
* Note(s) : (1) (a) The Probe OS module software version is denoted as follows :
*
*                       Vx.yy
*
*                           where
*                                  V     denotes 'Version' label
*                                  x     denotes major software version revision number
*                                  yy    denotes minor software version revision number
*
*               (b) The Probe OS module software version label #define is formatted as follows :
*
*                       ver = x.yy * 100
*
*                           where
*                                  ver   denotes software version number scaled as
*                                        an integer value
*                                  x.yy  denotes software version number
*********************************************************************************************************
*/

#define  PROBE_OS_VERSION          200u                         /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef   OS_PROBE_MODULE
#define  OS_PROBE_EXT
#else
#define  OS_PROBE_EXT  extern
#endif


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include  <app_cfg.h>
#include  <ucos_ii.h>


/*
*********************************************************************************************************
*                                                 DEFINES
*
* Note(s)     : (1) If OS_PROBE_USE_FP is defined as 0 in user header file, then the task CPU usages
*                   and task stack usages stored in OSProbe_TaskCPUUsage[] and OSProbe_TaskStkUsage[]
*                   will be 16-bit integers. This option removes the usage of floating-point types
*                   in this code, thereby eliminating the need for a floating-point library.  If
*                   OS_PROBE_USE_FP is not defined in user header file, the value defaults to 1.
*********************************************************************************************************
*/

#ifndef  OS_PROBE_USE_FP
#define  OS_PROBE_USE_FP             1
#endif


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

OS_PROBE_EXT            void    (*OSProbe_CallbackFnct)(void);
OS_PROBE_EXT            INT16U    OSProbe_Delay;

#if (OS_PROBE_TASK   > 0)
#if (OS_PROBE_USE_FP > 0)
OS_PROBE_EXT  volatile  FP32      OSProbe_TaskCPUUsage[OS_MAX_TASKS];
OS_PROBE_EXT  volatile  FP32      OSProbe_TaskStkUsage[OS_MAX_TASKS];
#else
OS_PROBE_EXT  volatile  INT16U    OSProbe_TaskCPUUsage[OS_MAX_TASKS];
OS_PROBE_EXT  volatile  INT16U    OSProbe_TaskStkUsage[OS_MAX_TASKS];
#endif
#endif

#if (OS_PROBE_HOOKS_EN > 0)
OS_PROBE_EXT            INT32U    OSProbe_CyclesCtr;
#if (OS_PROBE_TMR_32_BITS > 0)
OS_PROBE_EXT            INT32U    OSProbe_TmrCntsPrev;
#else
OS_PROBE_EXT            INT16U    OSProbe_TmrCntsPrev;
#endif
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

void    OSProbe_Init          (void);                               /* Initialize uC/Probe uC/OS-II Plug-In.                */

#if (OS_PROBE_TASK > 0)
void    OSProbe_SetCallback   (void   (*call_back)(void));
void    OSProbe_SetDelay      (INT16U   delay);
#endif


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*                                HOOKS CALLED FROM USER'S APPLICATION CODE
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
void    OSProbe_TaskSwHook    (void);                               /* MUST be called by App_TaskSwHook().                  */
void    OSProbe_TaskCreateHook(OS_TCB  *ptcb);                      /* MUST be called by App_TaskCreateHook().              */
void    OSProbe_TickHook      (void);                               /* MUST be called by App_TimeTickHook().                */

INT32U  OSProbe_TimeGetCycles (void);
#endif


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*                                             DEFINED IN bsp.c
*********************************************************************************************************
*/

#if (OS_PROBE_HOOKS_EN > 0)
void    OSProbe_TmrInit       (void);
INT32U  OSProbe_TmrRd         (void);
#endif


/*
*********************************************************************************************************
*                                          CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if       (OS_TASK_PROFILE_EN == 0)
  #error  "OS_TASK_PROFILE_EN       illegally #define'd in 'os_cfg.h'                       "
  #error  "                [MUST be  1   Variables included in OS_TCB for profiling]        "
#endif



#ifndef    OS_PROBE_TASK
  #error  "OS_PROBE_TASK                  not #define'd in 'app_cfg.h'                      "
  #error  "                [MUST be  0  Task created to calculate CPU usage for each task]  "
  #error  "                [     ||  1  No task created                                  ]  "

#elif    ((OS_PROBE_TASK != 0 ) && \
          (OS_PROBE_TASK != 1))
  #error  "OS_PROBE_TASK            illegally #define'd in 'app_cfg.h'                      "
  #error  "                [MUST be  0  Task created to calculate CPU usage for each task]  "
  #error  "                [     ||  1  No task created                                  ]  "

#elif     (OS_PROBE_TASK == 1)

#ifndef    OS_PROBE_TASK_PRIO
  #error  "OS_PROBE_TASK_PRIO             not #define'd in 'app_cfg.h'                      "
  #error  "                [This will be the priority for the uC/OS-II Plug-in Task]        "
#endif

#ifndef    OS_PROBE_TASK_STK_SIZE
  #error  "OS_PROBE_TASK_STK_SIZE         not #define'd in 'app_cfg.h'                      "
  #error  "                [This will be the stack size for the uC/OS-II Plug-in Task]      "
#endif

#endif



#ifndef    OS_PROBE_HOOKS_EN
  #error  "OS_PROBE_HOOKS_EN              not #define'd in 'app_cfg.h'                      "
  #error  "                [MUST be  0  uC/OS-II Plug-in hooks code     included]           "
  #error  "                [     ||  1  uC/OS-II Plug-in hooks code NOT included]           "

#elif    ((OS_PROBE_HOOKS_EN != 0 ) && \
          (OS_PROBE_HOOKS_EN != 1))
  #error  "OS_PROBE_HOOKS_EN        illegally #define'd in 'app_cfg.h'                      "
  #error  "                [MUST be  0  uC/OS-II Plug-in hooks code     included]           "
  #error  "                [     ||  1  uC/OS-II Plug-in hooks code NOT included]           "

#elif     (OS_PROBE_HOOKS_EN == 1)

#ifndef    OS_PROBE_TMR_32_BITS
  #error  "OS_PROBE_TMR_32_BITS           not #define'd in 'app_cfg.h'                      "
  #error  "                [MUST be  1   Timer is 32-bit timer]                             "
  #error  "                [     ||  0   Timer is 16-bit timer]                             "

#elif    ((OS_PROBE_TMR_32_BITS != 0 ) && \
          (OS_PROBE_TMR_32_BITS != 1))
  #error  "OS_PROBE_TMR_32_BITS     illegally #define'd in 'app_cfg.h'"
  #error  "                [MUST be  1   Timer is 32-bit timer]                             "
  #error  "                [     ||  0   Timer is 16-bit timer]                             "
#endif

#endif




/*
*********************************************************************************************************
*                                              MODULE END
*
* Note(s) : See 'MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of PROBE_OS module include (see Note #1).        */
