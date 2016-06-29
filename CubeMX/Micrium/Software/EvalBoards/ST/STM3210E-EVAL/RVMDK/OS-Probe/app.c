/*
*********************************************************************************************************
*                                              EXAMPLE CODE
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
*                                            EXAMPLE CODE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210E-EVAL Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_LINE_0                                        0
#define  APP_LINE_1                                       24
#define  APP_LINE_2                                       48
#define  APP_LINE_3                                       72
#define  APP_LINE_4                                       96
#define  APP_LINE_5                                      120
#define  APP_LINE_6                                      144
#define  APP_LINE_7                                      168
#define  APP_LINE_8                                      192
#define  APP_LINE_9                                      216

#define  APP_COLOR_WHITE                              0xFFFF
#define  APP_COLOR_BLACK                              0x0000
#define  APP_COLOR_BLUE                               0x001F
#define  APP_COLOR_BLUE2                              0x051F
#define  APP_COLOR_RED                                0xF800
#define  APP_COLOR_MAGENTA                            0xF81F
#define  APP_COLOR_GREEN                              0x07E0
#define  APP_COLOR_CYAN                               0x7FFF
#define  APP_COLOR_YELLOW                             0xFFE0

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK         App_TaskStartStk[APP_TASK_START_STK_SIZE];
static  OS_STK         App_TaskUserIFStk[APP_TASK_USER_IF_STK_SIZE];
static  OS_STK         App_TaskKbdStk[APP_TASK_KBD_STK_SIZE];

static  OS_EVENT      *App_UserIFMbox;

static  CPU_CHAR       App_LCDLine0[21];
static  CPU_CHAR       App_LCDLine1[21];
static  CPU_CHAR       App_LCDLine2[21];
static  CPU_CHAR       App_LCDLine3[21];
static  CPU_CHAR       App_LCDLine4[21];
static  CPU_CHAR       App_LCDLine5[21];
static  CPU_CHAR       App_LCDLine6[21];
static  CPU_CHAR       App_LCDLine7[21];
static  CPU_CHAR       App_LCDLine8[21];
static  CPU_CHAR       App_LCDLine9[21];

#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (APP_PROBE_COM_EN  == DEF_ENABLED) && \
     (PROBE_COM_STAT_EN == DEF_ENABLED))
static  CPU_FP32       App_ProbeComRxPktSpd;
static  CPU_FP32       App_ProbeComTxPktSpd;
static  CPU_FP32       App_ProbeComTxSymSpd;
static  CPU_FP32       App_ProbeComTxSymByteSpd;

static  CPU_INT32U     App_ProbeComRxPktLast;
static  CPU_INT32U     App_ProbeComTxPktLast;
static  CPU_INT32U     App_ProbeComTxSymLast;
static  CPU_INT32U     App_ProbeComTxSymByteLast;

static  CPU_INT32U     App_ProbeComCtrLast;
#endif

#if (APP_OS_PROBE_EN == DEF_ENABLED)
static  CPU_INT32U     App_ProbeCounts;
static  CPU_BOOLEAN    App_ProbeB1;
static  CPU_BOOLEAN    App_ProbeJoystickLeft;
static  CPU_BOOLEAN    App_ProbeJoystickRight;
static  CPU_BOOLEAN    App_ProbeJoystickUp;
static  CPU_BOOLEAN    App_ProbeJoystickDown;
static  CPU_BOOLEAN    App_ProbeJoystickCenter;
#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  App_TaskCreate       (void);
static  void  App_EventCreate      (void);

static  void  App_TaskStart        (void        *p_arg);
static  void  App_TaskUserIF       (void        *p_arg);
static  void  App_TaskKbd          (void        *p_arg);

static  void  App_DispScr_SignOn   (void);
static  void  App_DispScr_TaskNames(void);

#if ((APP_PROBE_COM_EN == DEF_ENABLED) || \
     (APP_OS_PROBE_EN  == DEF_ENABLED))
static  void  App_InitProbe        (void);
#endif

#if (APP_OS_PROBE_EN == DEF_ENABLED)
static  void  App_ProbeCallback    (void);
#endif


static  void  App_FormatDec        (CPU_INT08U  *pstr,
                                    CPU_INT32U   value,
                                    CPU_INT08U   digits);


/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/

int  main (void)
{
    CPU_INT08U  os_err;


    BSP_IntDisAll();                                            /* Disable all ints until we are ready to accept them.  */

    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel".         */

    os_err = OSTaskCreateExt((void (*)(void *)) App_TaskStart,  /* Create the start task.                               */
                             (void          * ) 0,
                             (OS_STK        * )&App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],
                             (INT8U           ) APP_TASK_START_PRIO,
                             (INT16U          ) APP_TASK_START_PRIO,
                             (OS_STK        * )&App_TaskStartStk[0],
                             (INT32U          ) APP_TASK_START_STK_SIZE,
                             (void          * )0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

#if (OS_TASK_NAME_SIZE >= 11)
    OSTaskNameSet(APP_TASK_START_PRIO, (CPU_INT08U *)"Start Task", &os_err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II).  */

    return (0);
}


/*
*********************************************************************************************************
*                                          App_TaskStart()
*
* Description : The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
*
* Argument(s) : p_arg       Argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_TaskStart (void *p_arg)
{
    CPU_INT32U  i;
    CPU_INT32U  j;


    (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions.                            */
    OS_CPU_SysTickInit();                                       /* Initialize the SysTick.                              */

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                               /* Determine CPU capacity.                              */
#endif

#if ((APP_PROBE_COM_EN == DEF_ENABLED) || \
     (APP_OS_PROBE_EN  == DEF_ENABLED))
    App_InitProbe();
#endif

    App_EventCreate();                                          /* Create application events.                           */
    App_TaskCreate();                                           /* Create application tasks.                            */

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
        for (j = 0; j < 4; j++) {
            for (i = 1; i <= 4; i++) {
                BSP_LED_On(i);
                OSTimeDlyHMSM(0, 0, 0, 50);
                BSP_LED_Off(i);
                OSTimeDlyHMSM(0, 0, 0, 50);
            }

            for (i = 3; i >= 2; i--) {
                BSP_LED_On(i);
                OSTimeDlyHMSM(0, 0, 0, 50);
                BSP_LED_Off(i);
                OSTimeDlyHMSM(0, 0, 0, 50);
            }
        }

        for (i = 0; i < 4; i++) {
            BSP_LED_On(0);
            OSTimeDlyHMSM(0, 0, 0, 200);
            BSP_LED_Off(0);
            OSTimeDlyHMSM(0, 0, 0, 200);
        }
    }
}



/*
*********************************************************************************************************
*                                             App_EventCreate()
*
* Description : Create the application events.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskStart().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_EventCreate (void)
{
#if (OS_EVENT_NAME_SIZE > 12)
    CPU_INT08U  os_err;
#endif


    App_UserIFMbox = OSMboxCreate((void *)0);                   /* Create MBOX for communication between Kbd and UserIF.*/
#if (OS_EVENT_NAME_SIZE > 12)
    OSEventNameSet(App_UserIFMbox, "User IF Mbox", &os_err);
#endif
}


/*
*********************************************************************************************************
*                                            App_TaskCreate()
*
* Description : Create the application tasks.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskStart().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_TaskCreate (void)
{
    CPU_INT08U  os_err;


    os_err = OSTaskCreateExt((void (*)(void *)) App_TaskUserIF,
                             (void          * ) 0,
                             (OS_STK        * )&App_TaskUserIFStk[APP_TASK_USER_IF_STK_SIZE - 1],
                             (INT8U           ) APP_TASK_USER_IF_PRIO,
                             (INT16U          ) APP_TASK_USER_IF_PRIO,
                             (OS_STK        * )&App_TaskUserIFStk[0],
                             (INT32U          ) APP_TASK_USER_IF_STK_SIZE,
                             (void          * ) 0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

#if (OS_TASK_NAME_SIZE >= 9)
    OSTaskNameSet(APP_TASK_USER_IF_PRIO, "User I/F", &os_err);
#endif

    os_err = OSTaskCreateExt((void (*)(void *)) App_TaskKbd,
                             (void          * ) 0,
                             (OS_STK        * )&App_TaskKbdStk[APP_TASK_KBD_STK_SIZE - 1],
                             (INT8U           ) APP_TASK_KBD_PRIO,
                             (INT16U          ) APP_TASK_KBD_PRIO,
                             (OS_STK        * )&App_TaskKbdStk[0],
                             (INT32U          ) APP_TASK_KBD_STK_SIZE,
                             (void          * ) 0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

#if (OS_TASK_NAME_SIZE >= 9)
    OSTaskNameSet(APP_TASK_KBD_PRIO, "Keyboard", &os_err);
#endif
}


/*
*********************************************************************************************************
*                                            App_TaskKbd()
*
* Description : Monitor the state of the push buttons and passes messages to AppTaskUserIF()
*
* Argument(s) : p_arg       Argument passed to 'App_TaskKbd()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_TaskKbd (void *p_arg)
{
    CPU_BOOLEAN  b1_prev;
    CPU_BOOLEAN  b1;
    CPU_INT08U   key;


    (void)p_arg;

    b1_prev = DEF_FALSE;
    key     = 1;

    while (DEF_TRUE) {
        b1 = BSP_PB_GetStatus(1);

        if ((b1 == DEF_TRUE) && (b1_prev == DEF_FALSE)) {
            if (key == 2) {
                key = 1;
            } else {
                key++;
            }

            OSMboxPost(App_UserIFMbox, (void *)key);
        }

        b1_prev = b1;

        OSTimeDlyHMSM(0, 0, 0, 20);
    }
}


/*
*********************************************************************************************************
*                                            App_TaskUserIF()
*
* Description : Updates LCD.
*
* Argument(s) : p_arg       Argument passed to 'App_TaskUserIF()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_TaskUserIF (void *p_arg)
{
    CPU_INT08U  *msg;
    CPU_INT08U   err;
    CPU_INT32U   nstate;
    CPU_INT32U   pstate;


    (void)p_arg;


    App_DispScr_SignOn();
    OSTimeDlyHMSM(0, 0, 1, 0);
    nstate = 1;
    pstate = 1;


    while (DEF_TRUE) {
        msg = (CPU_INT08U *)(OSMboxPend(App_UserIFMbox, OS_TICKS_PER_SEC / 10, &err));
        if (err == OS_NO_ERR) {
            nstate = (CPU_INT32U)msg;
        }

        if (nstate != pstate) {
            LCD_Clear(APP_COLOR_WHITE);
            pstate  = nstate;
        }

        switch (nstate) {
            case 2:
                App_DispScr_TaskNames();
                break;

            case 1:
            default:
                App_DispScr_SignOn();
                break;
        }
    }
}


/*
*********************************************************************************************************
*                                          App_DispScr_SignOn()
*
* Description : Display uC/OS-II system information on the LCD.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskUserIF().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_DispScr_SignOn (void)
{
    CPU_INT32U  value;


    Str_Copy(App_LCDLine0, "  Micrium uC/OS-II  ");
    Str_Copy(App_LCDLine1, "ST STM32 (Cortex-M3)");

    Str_Copy(App_LCDLine2, "                    ");

    Str_Copy(App_LCDLine3, "  uC/OS-II:  Vx.yy  ");
    value            = (CPU_INT32U)OSVersion();
    App_LCDLine3[14] =  value / 100 + '0';
    App_LCDLine3[16] = (value % 100) / 10 + '0';
    App_LCDLine3[17] = (value %  10) + '0';

    Str_Copy(App_LCDLine4, "  TickRate:   xxxx  ");
    value = (CPU_INT32U)OS_TICKS_PER_SEC;
    App_FormatDec(&App_LCDLine4[14], value, 4);

    Str_Copy(App_LCDLine5, "  CPU Usage:xx %    ");
    value            = (CPU_INT32U)OSCPUUsage;
    App_LCDLine5[12] = (value / 10) + '0';
    App_LCDLine5[13] = (value % 10) + '0';

    Str_Copy(App_LCDLine6, "  CPU Speed:xx MHz  ");
    value            = (CPU_INT32U)BSP_CPU_ClkFreq() / 1000000L;
    App_LCDLine6[12] = (value / 10) + '0';
    App_LCDLine6[13] = (value % 10) + '0';

    Str_Copy(App_LCDLine7, "  #Ticks: xxxxxxxx  ");
    value = (CPU_INT32U)OSTime;
    App_FormatDec(&App_LCDLine7[10], value, 8);

    Str_Copy(App_LCDLine8, "  #CtxSw: xxxxxxxx  ");
    value = (CPU_INT32U)OSCtxSwCtr;
    App_FormatDec(&App_LCDLine8[10], value, 8);

    Str_Copy(App_LCDLine9, "                    ");

    LCD_SetTextColor(APP_COLOR_BLUE);
    LCD_DisplayString(APP_LINE_0, App_LCDLine0);
    LCD_DisplayString(APP_LINE_1, App_LCDLine1);
    LCD_SetTextColor(APP_COLOR_BLACK);
    LCD_DisplayString(APP_LINE_2, App_LCDLine2);
    LCD_DisplayString(APP_LINE_3, App_LCDLine3);
    LCD_DisplayString(APP_LINE_4, App_LCDLine4);
    LCD_DisplayString(APP_LINE_5, App_LCDLine5);
    LCD_DisplayString(APP_LINE_6, App_LCDLine6);
    LCD_DisplayString(APP_LINE_7, App_LCDLine7);
    LCD_DisplayString(APP_LINE_8, App_LCDLine8);
}



/*
*********************************************************************************************************
*                                          App_DispScr_SignOn()
*
* Description : Display uC/OS-II system information on the LCD.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskUserIF().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_DispScr_TaskNames (void)
{
    CPU_INT08U   ix;
    OS_TCB      *ptcb;
    CPU_CHAR    *line;
    CPU_INT08U   value;


    ptcb = &OSTCBTbl[0];
    ix   = 0;

    Str_Copy(App_LCDLine0, "  Micrium uC/OS-II  ");
    Str_Copy(App_LCDLine1, "ST STM32 (Cortex-M3)");

    Str_Copy(App_LCDLine2, "  Prio   Taskname   ");

    while (ptcb != NULL) {
        value = ptcb->OSTCBPrio;

        switch (ix) {
            case 0:
                 line = App_LCDLine3;
                 break;

            case 1:
                 line = App_LCDLine4;
                 break;

            case 2:
                 line = App_LCDLine5;
                 break;

            case 3:
                 line = App_LCDLine6;
                 break;

            case 4:
                 line = App_LCDLine7;
                 break;

            case 5:
                 line = App_LCDLine8;
                 break;

            case 6:
                 line = App_LCDLine9;
                 break;

            default:
                 line = (CPU_CHAR *)0;
                 break;
        }

        if (line == (CPU_CHAR *)0) {
            break;
        }

        line[0] = ' ';
        line[1] = ' ';
        line[2] = ' ';
        line[3] = value / 10 + '0';
        line[4] = value % 10 + '0';
        line[5] = ' ';
        Str_Copy_N(line + 6, ptcb->OSTCBTaskName, 14);

        ptcb    = ptcb->OSTCBPrev;
        ix++;
    }

    if (ix < 6) {
        Str_Copy(App_LCDLine9, "                    ");
    }

    if (ix < 5) {
        Str_Copy(App_LCDLine8, "                    ");
    }

    if (ix < 4) {
        Str_Copy(App_LCDLine7, "                    ");
    }

    if (ix < 3) {
        Str_Copy(App_LCDLine6, "                    ");
    }

    LCD_SetTextColor(APP_COLOR_BLUE);
    LCD_DisplayString(APP_LINE_0, App_LCDLine0);
    LCD_DisplayString(APP_LINE_1, App_LCDLine1);
    LCD_SetTextColor(APP_COLOR_RED);
    LCD_DisplayString(APP_LINE_2, App_LCDLine2);
    LCD_SetTextColor(APP_COLOR_BLACK);
    LCD_DisplayString(APP_LINE_3, App_LCDLine3);
    LCD_DisplayString(APP_LINE_4, App_LCDLine4);
    LCD_DisplayString(APP_LINE_5, App_LCDLine5);
    LCD_DisplayString(APP_LINE_6, App_LCDLine6);
    LCD_DisplayString(APP_LINE_7, App_LCDLine7);
    LCD_DisplayString(APP_LINE_8, App_LCDLine8);
    LCD_DisplayString(APP_LINE_9, App_LCDLine9);
}


/*
*********************************************************************************************************
*                                             App_InitProbe()
*
* Description : Initialize uC/Probe target code.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : App_TaskStart().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if ((APP_PROBE_COM_EN == DEF_ENABLED) || \
     (APP_OS_PROBE_EN  == DEF_ENABLED))
static  void  App_InitProbe (void)
{
#if (APP_OS_PROBE_EN        == DEF_ENABLED)
    (void)App_ProbeCounts;
    (void)App_ProbeB1;
    (void)App_ProbeJoystickCenter;
    (void)App_ProbeJoystickDown;
    (void)App_ProbeJoystickLeft;
    (void)App_ProbeJoystickRight;
    (void)App_ProbeJoystickUp;

#if ((APP_PROBE_COM_EN      == DEF_ENABLED) && \
     (PROBE_COM_STAT_EN     == DEF_ENABLED))
    (void)App_ProbeComRxPktSpd;
    (void)App_ProbeComTxPktSpd;
    (void)App_ProbeComTxSymSpd;
    (void)App_ProbeComTxSymByteSpd;
#endif

    OSProbe_Init();
    OSProbe_SetCallback(App_ProbeCallback);
    OSProbe_SetDelay(250);
#endif

#if (APP_PROBE_COM_EN       == DEF_ENABLED)
    ProbeCom_Init();                                            /* Initialize the uC/Probe communications module.       */
#if (PROBE_COM_METHOD_RS232 == DEF_ENABLED)
    ProbeRS232_Init(115200);
    ProbeRS232_RxIntEn();
#endif
#endif
}
#endif


/*
*********************************************************************************************************
*                                         AppProbeCallback()
*
* Description : uC/Probe OS plugin callback.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : uC/Probe OS plugin task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (APP_OS_PROBE_EN == DEF_ENABLED)
static  void  App_ProbeCallback (void)
{

#if ((APP_PROBE_COM_EN  == DEF_ENABLED) && \
     (PROBE_COM_STAT_EN == DEF_ENABLED))
    CPU_INT32U  ctr_curr;
    CPU_INT32U  rxpkt_curr;
    CPU_INT32U  txpkt_curr;
    CPU_INT32U  sym_curr;
    CPU_INT32U  symbyte_curr;
#endif
    CPU_INT32U  joystick;


    App_ProbeCounts++;

    App_ProbeB1             = BSP_PB_GetStatus(1);

    joystick                = BSP_Joystick_GetStatus();
    App_ProbeJoystickCenter = DEF_BIT_IS_SET(joystick, BSP_JOYSTICK_CENTER);
    App_ProbeJoystickDown   = DEF_BIT_IS_SET(joystick, BSP_JOYSTICK_DOWN);
    App_ProbeJoystickLeft   = DEF_BIT_IS_SET(joystick, BSP_JOYSTICK_LEFT);
    App_ProbeJoystickRight  = DEF_BIT_IS_SET(joystick, BSP_JOYSTICK_RIGHT);
    App_ProbeJoystickUp     = DEF_BIT_IS_SET(joystick, BSP_JOYSTICK_UP);


#if ((APP_PROBE_COM_EN  == DEF_ENABLED) && \
     (PROBE_COM_STAT_EN == DEF_ENABLED))
    ctr_curr     = OSTime;
    rxpkt_curr   = ProbeCom_RxPktCtr;
    txpkt_curr   = ProbeCom_TxPktCtr;
    sym_curr     = ProbeCom_TxSymCtr;
    symbyte_curr = ProbeCom_TxSymByteCtr;

    if ((ctr_curr - App_ProbeComCtrLast) >= OS_TICKS_PER_SEC) {
        App_ProbeComRxPktSpd      = ((CPU_FP32)(rxpkt_curr   - App_ProbeComRxPktLast)     / (ctr_curr - App_ProbeComCtrLast)) * OS_TICKS_PER_SEC;
        App_ProbeComTxPktSpd      = ((CPU_FP32)(txpkt_curr   - App_ProbeComTxPktLast)     / (ctr_curr - App_ProbeComCtrLast)) * OS_TICKS_PER_SEC;
        App_ProbeComTxSymSpd      = ((CPU_FP32)(sym_curr     - App_ProbeComTxSymLast)     / (ctr_curr - App_ProbeComCtrLast)) * OS_TICKS_PER_SEC;
        App_ProbeComTxSymByteSpd  = ((CPU_FP32)(symbyte_curr - App_ProbeComTxSymByteLast) / (ctr_curr - App_ProbeComCtrLast)) * OS_TICKS_PER_SEC;

        App_ProbeComCtrLast       = ctr_curr;
        App_ProbeComRxPktLast     = rxpkt_curr;
        App_ProbeComTxPktLast     = txpkt_curr;
        App_ProbeComTxSymLast     = sym_curr;
        App_ProbeComTxSymByteLast = symbyte_curr;
    }
#endif
}
#endif


/*
*********************************************************************************************************
*                                      App_FormatDec()
*
* Description : Convert a decimal value to ASCII (without leading zeros).
*
* Argument(s) : pstr            Pointer to the destination ASCII string.
*
*               value           Value to convert (assumes an unsigned value).
*
*               digits          The desired number of digits.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_FormatDec (CPU_INT08U *pstr, CPU_INT32U value, CPU_INT08U digits)
{
    CPU_INT08U   i;
    CPU_INT32U   mult;
    CPU_BOOLEAN  found;
    CPU_INT32U   nbr;


    found = DEF_NO;
    mult  = 1;
    for (i = 0; i < (digits - 1); i++) {
        mult *= 10;
    }
    while (mult > 0) {
        nbr = value / mult;
        if (found == DEF_NO) {
            if (nbr != 0) {
                found = DEF_YES;
                *pstr = nbr + '0';
            } else{
                if (mult == 1) {
                    *pstr = '0';
                } else {
                    *pstr = ' ';
                }
            }
        } else {
            *pstr = nbr + '0';
        }
        pstr++;
        value %= mult;
        mult  /= 10;
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          uC/OS-II APP HOOKS
*********************************************************************************************************
*********************************************************************************************************
*/

#if (OS_APP_HOOKS_EN > 0)
/*
*********************************************************************************************************
*                                      TASK CREATION HOOK (APPLICATION)
*
* Description : This function is called when a task is created.
*
* Argument(s) : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  App_TaskCreateHook (OS_TCB *ptcb)
{
#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN == DEF_ENABLED))
    OSProbe_TaskCreateHook(ptcb);
#endif
}

/*
*********************************************************************************************************
*                                    TASK DELETION HOOK (APPLICATION)
*
* Description : This function is called when a task is deleted.
*
* Argument(s) : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  App_TaskDelHook (OS_TCB *ptcb)
{
    (void)ptcb;
}

/*
*********************************************************************************************************
*                                      IDLE TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskIdleHook(), which is called by the idle task.  This hook
*               has been added to allow you to do such things as STOP the CPU to conserve power.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 251
void  App_TaskIdleHook (void)
{
}
#endif

/*
*********************************************************************************************************
*                                        STATISTIC TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskStatHook(), which is called every second by uC/OS-II's
*               statistics task.  This allows your application to add functionality to the statistics task.
*
* Argument(s) : none.
*********************************************************************************************************
*/

void  App_TaskStatHook (void)
{
}

/*
*********************************************************************************************************
*                                        TASK SWITCH HOOK (APPLICATION)
*
* Description : This function is called when a task switch is performed.  This allows you to perform other
*               operations during a context switch.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*
*               (2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                  task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

#if OS_TASK_SW_HOOK_EN > 0
void  App_TaskSwHook (void)
{
#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN == DEF_ENABLED))
    OSProbe_TaskSwHook();
#endif
}
#endif

/*
*********************************************************************************************************
*                                     OS_TCBInit() HOOK (APPLICATION)
*
* Description : This function is called by OSTCBInitHook(), which is called by OS_TCBInit() after setting
*               up most of the TCB.
*
* Argument(s) : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 204
void  App_TCBInitHook (OS_TCB *ptcb)
{
    (void)ptcb;
}
#endif

/*
*********************************************************************************************************
*                                        TICK HOOK (APPLICATION)
*
* Description : This function is called every tick.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if OS_TIME_TICK_HOOK_EN > 0
void  App_TimeTickHook (void)
{
#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN == DEF_ENABLED))
    OSProbe_TickHook();
#endif
}
#endif
#endif
