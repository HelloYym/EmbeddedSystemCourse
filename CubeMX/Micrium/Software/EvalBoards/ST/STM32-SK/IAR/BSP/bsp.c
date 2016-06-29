/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2007; Micrium, Inc.; Weston, FL
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
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   IAR STM32-SK Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.10
* Programmer(s) : BAN
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_MODULE
#include <bsp.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
                                                                /* -------------------- GPIOA PINS -------------------- */
#define  BSP_GPIOA_PB_WAKEUP                     DEF_BIT_00
#define  BSP_GPIOA_MIC_IN                        DEF_BIT_01
#define  BSP_GPIOA_UART2_TX                      DEF_BIT_02
#define  BSP_GPIOA_UART2_RX                      DEF_BIT_03
#define  BSP_GPIOA_LED1                          DEF_BIT_04
#define  BSP_GPIOA_LED2                          DEF_BIT_05
#define  BSP_GPIOA_LED3                          DEF_BIT_06
#define  BSP_GPIOA_LED4                          DEF_BIT_07
#define  BSP_GPIOA_PWM_OUT                       DEF_BIT_08
#define  BSP_GPIOA_LED5                          DEF_BIT_09
#define  BSP_GPIOA_LED6                          DEF_BIT_10

#if (BSP_CFG_LED_SPI1_EN == DEF_ENABLED)
#define  BSP_GPIOA_LEDS        (BSP_GPIOA_LED1 | BSP_GPIOA_LED2 | BSP_GPIOA_LED3 | BSP_GPIOA_LED4 | BSP_GPIOA_LED5 | BSP_GPIOA_LED6)
#else
#define  BSP_GPIOA_LEDS        (BSP_GPIOA_LED5 | BSP_GPIOA_LED6)
#endif

                                                                /* -------------------- GPIOB PINS -------------------- */
#define  BSP_GPIOB_LCD_LIGHT                     DEF_BIT_00
#define  BSP_GPIOB_PB2                           DEF_BIT_05
#define  BSP_GPIOB_LED7                          DEF_BIT_06
#define  BSP_GPIOB_LED8                          DEF_BIT_07
#define  BSP_GPIOB_CAN_RX                        DEF_BIT_08
#define  BSP_GPIOB_CAN_TX                        DEF_BIT_09
#define  BSP_GPIOB_LED9                          DEF_BIT_10
#define  BSP_GPIOB_LED10                         DEF_BIT_11
#define  BSP_GPIOB_LED11                         DEF_BIT_12
#define  BSP_GPIOB_LED12                         DEF_BIT_13
#define  BSP_GPIOB_LED13                         DEF_BIT_14
#define  BSP_GPIOB_LED14                         DEF_BIT_15

#if (BSP_CFG_LED_SPI2_EN == DEF_ENABLED)
#define  BSP_GPIOB_LEDS        (BSP_GPIOB_LED7 | BSP_GPIOB_LED8 | BSP_GPIOB_LED9 | BSP_GPIOB_LED10 | BSP_GPIOB_LED11 | BSP_GPIOB_LED12 | BSP_GPIOB_LED13 | BSP_GPIOB_LED14)
#else
#define  BSP_GPIOB_LEDS        (BSP_GPIOB_LED7 | BSP_GPIOB_LED8 | BSP_GPIOB_LED9 | BSP_GPIOB_LED10)
#endif

                                                                /* -------------------- GPIOC PINS -------------------- */
#define  BSP_GPIOC_LCD_DB4                       DEF_BIT_00
#define  BSP_GPIOC_LCD_DB5                       DEF_BIT_01
#define  BSP_GPIOC_LCD_DB6                       DEF_BIT_02
#define  BSP_GPIOC_LCD_DB7                       DEF_BIT_03
#define  BSP_GPIOC_PB3                           DEF_BIT_04
#define  BSP_GPIOC_POT                           DEF_BIT_05
#define  BSP_GPIOC_LED15                         DEF_BIT_06
#define  BSP_GPIOC_LED16                         DEF_BIT_07
#define  BSP_GPIOC_LCD_RS                        DEF_BIT_08
#define  BSP_GPIOC_LCD_RW                        DEF_BIT_09
#define  BSP_GPIOC_USART3_TX                     DEF_BIT_10
#define  BSP_GPIOC_USART3_RX                     DEF_BIT_11
#define  BSP_GPIOC_LCD_E                         DEF_BIT_12
#define  BSP_GPIOC_PB1                           DEF_BIT_13
#define  BSP_GPIOC_LCD_DB      (BSP_GPIOC_LCD_DB4 | BSP_GPIOC_LCD_DB5 | BSP_GPIOC_LCD_DB6 | BSP_GPIOC_LCD_DB7)
#define  BSP_GPIOC_LEDS        (BSP_GPIOC_LED15   | BSP_GPIOC_LED16)

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


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  BSP_LED_Init      (void);
static  void  BSP_PB_Init       (void);
static  void  BSP_LCD_LightInit (void);
static  void  BSP_ADCS_Init     (void);

#ifdef DISP_MODULE_PRESENT
static  void  BSP_DispE_High    (void);
static  void  BSP_DispE_Low     (void);
static  void  BSP_DispRW_Low    (void);
#endif

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*********************************************************************************************************
*/

void  BSP_Init (void)
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    RCC_WaitForHSEStartUp();


    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    FLASH_SetLatency(FLASH_Latency_2);
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
    RCC_PLLCmd(ENABLE);

    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {
        ;
    }

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    while (RCC_GetSYSCLKSource() != 0x08) {
        ;
    }

    BSP_LED_Init();                                             /* Initialize the I/Os for the LED      controls.       */
    BSP_PB_Init();                                              /* Initialize the I/Os for the PB       controls.       */
    BSP_LCD_LightInit();                                        /* Initialize the I/Os for the LCD backlight.           */
    BSP_ADCS_Init();                                            /* Initialize the ADC  for the potentiometer.           */
}


/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    RCC_ClocksTypeDef  rcc_clocks;


    RCC_GetClocksFreq(&rcc_clocks);

    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         OS CORTEX-M3 FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         OS_CPU_SysTickClkFreq()
*
* Description : Get system tick clock frequency.
*
* Argument(s) : none.
*
* Return(s)   : Clock frequency (of system tick).
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

INT32U  OS_CPU_SysTickClkFreq (void)
{
    INT32U  freq;


    freq = BSP_CPU_ClkFreq();
    return (freq);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               PB FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*                                              BSP_PB_Init()
*
* Description : Initialize the board's PB.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_PB_Init (void)
{
    GPIO_InitTypeDef  gpio_init;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    gpio_init.GPIO_Pin  = BSP_GPIOA_PB_WAKEUP;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin  = BSP_GPIOB_PB2;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio_init);

    gpio_init.GPIO_Pin  = BSP_GPIOC_PB1 | BSP_GPIOC_PB3;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &gpio_init);

}


/*
*********************************************************************************************************
*                                           BSP_PB_GetStatus()
*
* Description : Get the status of a push button on the board.
*
* Argument(s) : pb      The ID of the push button to probe
*
*                       1    probe the PB1    push button
*                       2    probe the PB2    push button
*                       3    probe the PB3    push button
*                       4    probe the wakeup push button
*
* Return(s)   : DEF_FALSE   if the push button is pressed.
*               DEF_TRUE    if the push button is not pressed.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_PB_GetStatus (CPU_INT08U pb)
{
    CPU_BOOLEAN  status;
    CPU_INT32U   pin;


    status = DEF_FALSE;

    switch (pb) {
        case 1:
             pin = GPIO_ReadInputDataBit(GPIOC, BSP_GPIOC_PB1);
             if (pin > 0) {
                 status = DEF_TRUE;
             }
             break;

        case 2:
             pin = GPIO_ReadInputDataBit(GPIOB, BSP_GPIOB_PB2);
             if (pin > 0) {
                 status = DEF_TRUE;
             }
             break;

        case 3:
             pin = GPIO_ReadInputDataBit(GPIOC, BSP_GPIOC_PB3);
             if (pin > 0) {
                 status = DEF_TRUE;
             }
             break;

        case 4:
             pin = GPIO_ReadInputDataBit(GPIOA, BSP_GPIOA_PB_WAKEUP);
             if (pin > 0) {
                 status = DEF_TRUE;
             }
             break;

        default:
             break;
    }

    return (status);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                              LED FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             BSP_LED_Init()
*
* Description : Initialize the I/O for the LEDs
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_LED_Init (void)
{
    GPIO_InitTypeDef  gpio_init;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    gpio_init.GPIO_Pin   = BSP_GPIOA_LEDS;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin   = BSP_GPIOB_LEDS;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &gpio_init);

#if (BSP_CFG_LED_PIOC_EN == DEF_ENABLED)
    gpio_init.GPIO_Pin   = BSP_GPIOC_LEDS;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &gpio_init);
#endif
}


/*
*********************************************************************************************************
*                                             BSP_LED_On()
*
* Description : Turn ON any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turns ON ALL the LEDs
*                       1    turns ON user LED1  on the board
*                       2    turns ON user LED2  on the board
*                             .
*                             .
*                             .
*                       16   turns ON user LED16 on the board
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_On (CPU_INT08U led)
{
    switch (led) {
        case 0:
             GPIO_ResetBits(GPIOA, BSP_GPIOA_LEDS);
             GPIO_ResetBits(GPIOB, BSP_GPIOB_LEDS);
#if (BSP_CFG_LED_PIOC_EN == DEF_ENABLED)
             GPIO_ResetBits(GPIOC, BSP_GPIOC_LEDS);
#endif
             break;

#if (BSP_CFG_LED_SPI1_EN == DEF_ENABLED)
        case 1:
             GPIO_ResetBits(GPIOA, BSP_GPIOA_LED1);
             break;

        case 2:
             GPIO_ResetBits(GPIOA, BSP_GPIOA_LED2);
             break;

        case 3:
             GPIO_ResetBits(GPIOA, BSP_GPIOA_LED3);
             break;

        case 4:
             GPIO_ResetBits(GPIOA, BSP_GPIOA_LED4);
             break;
#endif

        case 5:
             GPIO_ResetBits(GPIOA, BSP_GPIOA_LED5);
             break;

        case 6:
             GPIO_ResetBits(GPIOA, BSP_GPIOA_LED6);
             break;

        case 7:
             GPIO_ResetBits(GPIOB, BSP_GPIOB_LED7);
             break;

        case 8:
             GPIO_ResetBits(GPIOB, BSP_GPIOB_LED8);
             break;

        case 9:
             GPIO_ResetBits(GPIOB, BSP_GPIOB_LED9);
             break;

        case 10:
             GPIO_ResetBits(GPIOB, BSP_GPIOB_LED10);
             break;

#if (BSP_CFG_LED_SPI2_EN == DEF_ENABLED)
        case 11:
             GPIO_ResetBits(GPIOB, BSP_GPIOB_LED11);
             break;

        case 12:
             GPIO_ResetBits(GPIOB, BSP_GPIOB_LED12);
             break;

        case 13:
             GPIO_ResetBits(GPIOB, BSP_GPIOB_LED13);
             break;

        case 14:
             GPIO_ResetBits(GPIOB, BSP_GPIOB_LED14);
             break;
#endif

#if (BSP_CFG_LED_PIOC_EN == DEF_ENABLED)
        case 15:
             GPIO_ResetBits(GPIOC, BSP_GPIOC_LED15);
             break;

        case 16:
             GPIO_ResetBits(GPIOC, BSP_GPIOC_LED16);
             break;
#endif

        default:
             break;
    }
}


/*
*********************************************************************************************************
*                                              BSP_LED_Off()
*
* Description : Turn OFF any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turns OFF ALL the LEDs
*                       1    turns OFF user LED1  on the board
*                       2    turns OFF user LED2  on the board
*                             .
*                             .
*                             .
*                       16   turns OFF user LED16 on the board
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Off (CPU_INT08U led)
{
    switch (led) {
        case 0:
             GPIO_SetBits(GPIOA, BSP_GPIOA_LEDS);
             GPIO_SetBits(GPIOB, BSP_GPIOB_LEDS);
#if (BSP_CFG_LED_PIOC_EN == DEF_ENABLED)
             GPIO_SetBits(GPIOC, BSP_GPIOC_LEDS);
#endif
             break;

#if (BSP_CFG_LED_SPI1_EN == DEF_ENABLED)
        case 1:
             GPIO_SetBits(GPIOA, BSP_GPIOA_LED1);
             break;

        case 2:
             GPIO_SetBits(GPIOA, BSP_GPIOA_LED2);
             break;

        case 3:
             GPIO_SetBits(GPIOA, BSP_GPIOA_LED3);
             break;

        case 4:
             GPIO_SetBits(GPIOA, BSP_GPIOA_LED4);
             break;
#endif

        case 5:
             GPIO_SetBits(GPIOA, BSP_GPIOA_LED5);
             break;

        case 6:
             GPIO_SetBits(GPIOA, BSP_GPIOA_LED6);
             break;

        case 7:
             GPIO_SetBits(GPIOB, BSP_GPIOB_LED7);
             break;

        case 8:
             GPIO_SetBits(GPIOB, BSP_GPIOB_LED8);
             break;

        case 9:
             GPIO_SetBits(GPIOB, BSP_GPIOB_LED9);
             break;

        case 10:
             GPIO_SetBits(GPIOB, BSP_GPIOB_LED10);
             break;

#if (BSP_CFG_LED_SPI2_EN == DEF_ENABLED)
        case 11:
             GPIO_SetBits(GPIOB, BSP_GPIOB_LED11);
             break;

        case 12:
             GPIO_SetBits(GPIOB, BSP_GPIOB_LED12);
             break;

        case 13:
             GPIO_SetBits(GPIOB, BSP_GPIOB_LED13);
             break;

        case 14:
             GPIO_SetBits(GPIOB, BSP_GPIOB_LED14);
             break;
#endif

#if (BSP_CFG_LED_PIOC_EN == DEF_ENABLED)
        case 15:
             GPIO_SetBits(GPIOC, BSP_GPIOC_LED15);
             break;

        case 16:
             GPIO_SetBits(GPIOC, BSP_GPIOC_LED16);
             break;
#endif

        default:
             break;
    }
}


/*
*********************************************************************************************************
*                                            BSP_LED_Toggle()
*
* Description : TOGGLE any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    TOGGLE ALL the LEDs
*                       1    TOGGLE user LED1  on the board
*                       2    TOGGLE user LED2  on the board
*                             .
*                             .
*                             .
*                       16   TOGGLE user LED16 on the board
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Toggle (CPU_INT08U led)
{
    CPU_INT32U  pins;

    switch (led) {
        case 0:
             pins =  GPIO_ReadOutputData(GPIOA);
             pins ^= BSP_GPIOA_LEDS;
             GPIO_SetBits(  GPIOA,   pins  & BSP_GPIOA_LEDS);
             GPIO_ResetBits(GPIOA, (~pins) & BSP_GPIOA_LEDS);

             pins =  GPIO_ReadOutputData(GPIOB);
             pins ^= BSP_GPIOB_LEDS;
             GPIO_SetBits(  GPIOB,   pins  & BSP_GPIOB_LEDS);
             GPIO_ResetBits(GPIOB, (~pins) & BSP_GPIOB_LEDS);

#if (BSP_CFG_LED_PIOC_EN == DEF_ENABLED)
             pins =  GPIO_ReadOutputData(GPIOA);
             pins ^= BSP_GPIOC_LEDS;
             GPIO_SetBits(  GPIOC,   pins  & BSP_GPIOC_LEDS);
             GPIO_ResetBits(GPIOC, (~pins) & BSP_GPIOC_LEDS);
#endif
             break;

#if (BSP_CFG_LED_SPI1_EN == DEF_ENABLED)
        case 1:
        case 2:
        case 3:
        case 4:
            pins = GPIO_ReadOutputData(GPIOA);
            if ((pins & (1 << (led + 3))) == 0) {
                 GPIO_SetBits(  GPIOA, (1 << (led + 3)));
             } else {
                 GPIO_ResetBits(GPIOA, (1 << (led + 3)));
             }
            break;
#endif

        case 5:
        case 6:
            pins = GPIO_ReadOutputData(GPIOA);
            if ((pins & (1 << (led + 4))) == 0) {
                 GPIO_SetBits(  GPIOA, (1 << (led + 4)));
             } else {
                 GPIO_ResetBits(GPIOA, (1 << (led + 4)));
             }
            break;

        case 7:
        case 8:
            pins = GPIO_ReadOutputData(GPIOB);
            if ((pins & (1 << (led - 1))) == 0) {
                 GPIO_SetBits(  GPIOB, (1 << (led - 1)));
             } else {
                 GPIO_ResetBits(GPIOB, (1 << (led - 1)));
             }


        case 9:
        case 10:
#if (BSP_CFG_LED_SPI2_EN == DEF_ENABLED)
        case 11:
        case 12:
        case 13:
        case 14:
#endif
             pins = GPIO_ReadOutputData(GPIOB);
             if ((pins & (1 << (led + 1))) == 0) {
                 GPIO_SetBits(  GPIOB, (1 << (led + 1)));
             } else {
                 GPIO_ResetBits(GPIOB, (1 << (led + 1)));
             }
            break;

#if (BSP_CFG_LED_PIOC_EN == DEF_ENABLED)
        case 15:
        case 16:
             pins = GPIO_ReadOutputData(GPIOC);
             if ((pins & (1 << (led - 9))) == 0) {
                 GPIO_SetBits(  GPIOC, (1 << (led - 9)));
             } else {
                 GPIO_ResetBits(GPIOC, (1 << (led - 9)));
             }
            break;
#endif

        default:
             break;
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LCD LIGHT FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           BSP_LCD_LightInit()
*
* Description : Initialize the LCD backlight.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_LCD_LightInit (void)
{
    GPIO_InitTypeDef  gpio_init;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gpio_init.GPIO_Pin   = BSP_GPIOB_LCD_LIGHT;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &gpio_init);
}


/*
*********************************************************************************************************
*                                            BSP_LCD_LightOn()
*
* Description : Turn on the LCD backlight.
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

void  BSP_LCD_LightOn (void)
{
    GPIO_ResetBits(GPIOB, BSP_GPIOB_LCD_LIGHT);
}


/*
*********************************************************************************************************
*                                           BSP_LCD_LightOff()
*
* Description : Turn off the LCD backlight.
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

void  BSP_LCD_LightOff (void)
{
    GPIO_SetBits(GPIOB, BSP_GPIOB_LCD_LIGHT);
}


/*
*********************************************************************************************************
*                                          BSP_LCD_LightToggle()
*
* Description : Toggle the LCD backlight
*
* Arguments   : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LCD_LightToggle (void)
{
    CPU_INT32U  pins;


    pins = GPIO_ReadOutputData(GPIOB);
    if ((pins & BSP_GPIOB_LCD_LIGHT) == 0) {
        GPIO_SetBits(  GPIOB, BSP_GPIOB_LCD_LIGHT);
    } else {
        GPIO_ResetBits(GPIOB, BSP_GPIOB_LCD_LIGHT);
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                              ADC FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             BSP_ADCS_Init()
*
* Description : Initialize the board's ADC
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_ADCS_Init (void)
{
    ADC_InitTypeDef   adc_init;
    GPIO_InitTypeDef  gpio_init;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    gpio_init.GPIO_Pin  = GPIO_Pin_5;
    gpio_init.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &gpio_init);

    adc_init.ADC_Mode               = ADC_Mode_Independent;
    adc_init.ADC_ScanConvMode       = DISABLE;
    adc_init.ADC_ContinuousConvMode = ENABLE;
    adc_init.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    adc_init.ADC_DataAlign          = ADC_DataAlign_Right;
    adc_init.ADC_NbrOfChannel       = 1;
    ADC_Init(ADC1, &adc_init);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_13Cycles5);
    ADC_Cmd(ADC1, ENABLE);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


/*
*********************************************************************************************************
*                                           BSP_ADC_GetStatus()
*
* Description : This function initializes the board's ADC
*
* Argument(s) : adc             ID of the ADC to probe.  For this board, the only legitimate value is 1.
*
* Return(s)   : The numerator of the binary fraction representing the result of the latest ADC conversion.
*               This value will be a 12-bit value between 0x0000 and 0x0FFF, inclusive.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT16U  BSP_ADC_GetStatus (CPU_INT08U  adc)
{
    CPU_INT16U  result;


    result = 0;

    if (adc == 1) {
        result = ADC_GetConversionValue(ADC1);
    }

    return (result);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                              LCD FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              DispDataWr()
*
* Description : Send a single byte to the display device.
*
* Arguments   : data            Byte to send to the display device.
*
* Returns     : none.
*
* Caller(s)   : uC/LCD
*
* Notes       : none.
*********************************************************************************************************
*/

#ifdef DISP_MODULE_PRESENT
void  DispDataWr (CPU_INT08U data)
{
    CPU_INT32U  value;


    BSP_DispRW_Low();                                           /* Set R/W write LOW to write to the LCD module         */

    BSP_DispE_High();                                           /* Write the UPPER nibble to the LCD module             */

    value =  ((data >> 4) & 0x0000000F);
    GPIO_SetBits(GPIOC, value);
    value = (~(data >> 4) & 0x0000000F);
    GPIO_ResetBits(GPIOC, value);

    DispDly_uS(1000);
    BSP_DispE_Low();

    DispDly_uS(1000);                                           /* Write the LOWER nibble to the LCD module             */
    BSP_DispE_High();

    value =  (data & 0x0000000F);
    GPIO_SetBits(GPIOC, value);
    value = (~data & 0x0000000F);
    GPIO_ResetBits(GPIOC, value);

    DispDly_uS(1000);
    BSP_DispE_Low();
}

#if (DISP_BUS_WIDTH == 4)
void  DispDataWrOneNibble (CPU_INT08U data)
{
    CPU_INT32U  value;


    BSP_DispRW_Low();                                           /* Set R/W write LOW to write to the LCD module         */

    BSP_DispE_High();                                           /* Write the UPPER nibble to the LCD module             */

    value =  ((data >> 4) & 0x0F);
    GPIO_SetBits(GPIOC, value);
    value = (~(data >> 4) & 0x0F);
    GPIO_ResetBits(GPIOC, value);

    DispDly_uS(1000);
    BSP_DispE_Low();
}
#endif

#endif


/*
*********************************************************************************************************
*                                              DispDly_uS()
*
* Description : Delay for the specified number of microseconds.
*
* Argument(s) : us          Number of microseconds
*
* Return(s)   : none.
*
* Caller(s)   : uC/LCD
*
* Notes       : none.
*********************************************************************************************************
*/

#ifdef DISP_MODULE_PRESENT
void  DispDly_uS (CPU_INT32U us)
{
    CPU_INT32U  us_per_tick;
    CPU_INT32U  ticks;


    us_per_tick = BSP_CPU_ClkFreq() / OS_TICKS_PER_SEC;
    ticks       = us / us_per_tick + 2;
    OSTimeDly(ticks);
}
#endif


/*
*********************************************************************************************************
*                                             DispInitPort()
*
* Description : Initialize the I/O ports used by the display driver.
*
* Arguments   : none.
*
* Returns     : none.
*
* Caller(s)   : DispInit().
*
* Notes       : none.
*********************************************************************************************************
*/

#ifdef DISP_MODULE_PRESENT
void  DispInitPort (void)
{
    GPIO_InitTypeDef  gpio_init;


    gpio_init.GPIO_Pin   = BSP_GPIOC_LCD_DB4 | BSP_GPIOC_LCD_DB5 | BSP_GPIOC_LCD_DB6 | BSP_GPIOC_LCD_DB7
                         | BSP_GPIOC_LCD_RS  | BSP_GPIOC_LCD_RW  | BSP_GPIOC_LCD_E;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &gpio_init);
}
#endif


/*
*********************************************************************************************************
*                                               DispSel()
*
* Description : Change the Register Select control line to the LCD controller.
*
* Argument(s) : sel             Indicates whether command or data register should be selected:
*
*                                   DISP_SEL_CMD_REG    Select command register.
*                                   DISP_SEL_DATA_REG   Select data    register.
*
*
* Return(s)   : none.
*
* Caller(s)   : uC/LCD
*
* Notes       : none.
*********************************************************************************************************
*/

#ifdef DISP_MODULE_PRESENT
void  DispSel (CPU_INT08U sel)
{
    if (sel == DISP_SEL_CMD_REG) {
        GPIO_ResetBits(GPIOC, BSP_GPIOC_LCD_RS);                /* Select the command register (RS low)                     */
    } else {
        GPIO_SetBits(GPIOC, BSP_GPIOC_LCD_RS);                  /* Select the data    register (RS high)                    */
    }
}
#endif


/*
*********************************************************************************************************
*                                      DISPLAY CONTROL LINE FUNCTIONS
*********************************************************************************************************
*/

#ifdef DISP_MODULE_PRESENT
static  void  BSP_DispE_High (void)
{
    GPIO_SetBits(GPIOC, BSP_GPIOC_LCD_E);
}


static  void  BSP_DispE_Low (void)
{
    GPIO_ResetBits(GPIOC, BSP_GPIOC_LCD_E);
}


static  void  BSP_DispRW_Low (void)
{
    GPIO_ResetBits(GPIOC, BSP_GPIOC_LCD_RW);
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           OS PROBE FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           OSProbe_TmrInit()
*
* Description : Select & initialize a timer for use with the uC/Probe Plug-In for uC/OS-II.
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

#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN == 1))
void  OSProbe_TmrInit (void)
{
    TIM_TimeBaseInitTypeDef  tim_init;


    tim_init.TIM_Period        = 0xFFFF;
    tim_init.TIM_Prescaler     = 0x00;
    tim_init.TIM_ClockDivision = 0x0;
    tim_init.TIM_CounterMode   = TIM_CounterMode_Up;

#if (OS_PROBE_TIMER_SEL == 2)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInit(TIM2, &tim_init);
    TIM_SetCounter(TIM2, 0);
    TIM_PrescalerConfig(TIM2, 256, TIM_PSCReloadMode_Immediate);
    TIM_Cmd(TIM2, ENABLE);
#elif (OS_PROBE_TIMER_SEL == 3)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInit(TIM3, &tim_init);
    TIM_SetCounter(TIM3, 0);
    TIM_PrescalerConfig(TIM3, 256, TIM_PSCReloadMode_Immediate);
    TIM_Cmd(TIM3, ENABLE);
#elif (OS_PROBE_TIMER_SEL == 4)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInit(TIM4, &tim_init);
    TIM_SetCounter(TIM4, 0);
    TIM_PrescalerConfig(TIM4, 256, TIM_PSCReloadMode_Immediate);
    TIM_Cmd(TIM4, ENABLE);
#endif
}
#endif


/*
*********************************************************************************************************
*                                            OSProbe_TmrRd()
*
* Description : Read the current counts of a 16-bit free running timer.
*
* Argument(s) : none.
*
* Return(s)   : The 16-bit counts of the timer in a 32-bit variable.
*
* Caller(s)   : OSProbe_TimeGetCycles().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN == 1))
CPU_INT32U  OSProbe_TmrRd (void)
{
#if (OS_PROBE_TIMER_SEL == 2)

    return ((CPU_INT32U)TIM_GetCounter(TIM2));

#elif (OS_PROBE_TIMER_SEL == 3)

    return ((CPU_INT32U)TIM_GetCounter(TIM3));

#elif (OS_PROBE_TIMER_SEL == 4)

    return ((CPU_INT32U)TIM_GetCounter(TIM4));

#endif
}
#endif
