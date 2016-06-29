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
*                                      Port for the ST STM32
*
* Filename      : probe_rs232c.c
* Version       : V1.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <probe_rs232.h>
#include  <includes.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
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


/*
*********************************************************************************************************
*                                      LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_RS232_COMM_SEL

  #error  "PROBE_RS232_COMM_SEL              not #define'd in 'probe_com_cfg.h'     "
  #error  "                                  [MUST be  PROBE_RS232_UART_1   ]       "
  #error  "                                  [     ||  PROBE_RS232_UART_2   ]       "
  #error  "                                  [     ||  PROBE_RS232_UART_3   ]       "

#elif     (PROBE_RS232_COMM_SEL != PROBE_RS232_UART_1) && \
          (PROBE_RS232_COMM_SEL != PROBE_RS232_UART_2) && \
          (PROBE_RS232_COMM_SEL != PROBE_RS232_UART_3)

  #error  "PROBE_RS232_COMM_SEL        illegally #define'd in 'probe_com_cfg.h'     "
  #error  "                                  [MUST be  PROBE_RS232_UART_1   ]       "
  #error  "                                  [     ||  PROBE_RS232_UART_2   ]       "
  #error  "                                  [     ||  PROBE_RS232_UART_3   ]       "
#endif

#ifndef    PROBE_RS232_UART_1_REMAP

  #error  "PROBE_RS232_UART_1_REMAP          not #define'd in 'probe_com_cfg.h'     "
  #error  "                                  [MUST be  DEF_TRUE   ]                 "
  #error  "                                  [     ||  DEF_FALSE  ]                 "
#endif

#ifndef    PROBE_RS232_UART_2_REMAP

  #error  "PROBE_RS232_UART_2_REMAP          not #define'd in 'probe_com_cfg.h'     "
  #error  "                                  [MUST be  DEF_TRUE   ]                 "
  #error  "                                  [     ||  DEF_FALSE  ]                 "
#endif

#ifndef    PROBE_RS232_UART_3_REMAP_PARTIAL

  #error  "PROBE_RS232_UART_3_REMAP_PARTIAL  not #define'd in 'probe_com_cfg.h'     "
  #error  "                                  [MUST be  DEF_TRUE   ]                 "
  #error  "                                  [     ||  DEF_FALSE  ]                 "
#endif

#ifndef    PROBE_RS232_UART_3_REMAP_FULL

  #error  "PROBE_RS232_UART_3_REMAP_FULL     not #define'd in 'probe_com_cfg.h'     "
  #error  "                                  [MUST be  DEF_TRUE   ]                 "
  #error  "                                  [     ||  DEF_FALSE  ]                 "
#endif

/*
*********************************************************************************************************
*********************************************************************************************************
*                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      ProbeRS232_InitTarget()
*
* Description : Initialize the UART for uC/Probe communication.
*
* Argument(s) : baud_rate   Intended baud rate of the RS-232.
*
* Return(s)   : none.
*
* Note(s )    : (1) The following constants control the GPIO remap for the USART control lines:
*
*                        PROBE_RS232_UART_1_REMAP
*                        PROBE_RS232_UART_2_REMAP
*                        PROBE_RS232_UART_3_REMAP_PARTIAL
*                        PROBE_RS232_UART_3_REMAP_FULL
*
*                    Though the #error directives in "Local Configuration Errors" will require that
*                    all are defined, the value of those bearing on the USART not used will have no
*                    effect.
*
*                (2) PROBE_RS232_UART_3_REMAP_PARTIAL has precedence over PROBE_RS232_UART_3_REMAP_FULL,
*                    if both are defined to DEF_TRUE.
*********************************************************************************************************
*/

void  ProbeRS232_InitTarget (CPU_INT32U baud_rate)
{
    GPIO_InitTypeDef        gpio_init;
    USART_InitTypeDef       usart_init;
    USART_ClockInitTypeDef  usart_clk_init;


                                                                /* ----------------- INIT USART STRUCT ---------------- */
    usart_init.USART_BaudRate            = baud_rate;
    usart_init.USART_WordLength          = USART_WordLength_8b;
    usart_init.USART_StopBits            = USART_StopBits_1;
    usart_init.USART_Parity              = USART_Parity_No ;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;


    usart_clk_init.USART_Clock           = USART_Clock_Disable;
    usart_clk_init.USART_CPOL            = USART_CPOL_Low;
    usart_clk_init.USART_CPHA            = USART_CPHA_2Edge;
    usart_clk_init.USART_LastBit         = USART_LastBit_Disable;


#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_1)
    BSP_PeriphEn(BSP_PERIPH_ID_USART1);

                                                                /* ----------------- SETUP USART1 GPIO ---------------- */
#if (PROBE_RS232_UART_1_REMAP > 0)
    BSP_PeriphEn(BSP_PERIPH_ID_IOPB);
    BSP_PeriphEn(BSP_PERIPH_ID_IOPD);
    BSP_PeriphEn(BSP_PERIPH_ID_AFIO);

    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);

                                                                /* Configure GPIOB.6 as push-pull                       */
    gpio_init.GPIO_Pin   = GPIO_Pin_6;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &gpio_init);

                                                                /* Configure GPIOB.7 as input floating                  */
    gpio_init.GPIO_Pin   = GPIO_Pin_7;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio_init);
#else
    BSP_PeriphEn(BSP_PERIPH_ID_IOPA);

                                                                /* Configure GPIOA.9 as push-pull                       */
    gpio_init.GPIO_Pin   = GPIO_Pin_9;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio_init);

                                                                /* Configure GPIOA.10 as input floating                 */
    gpio_init.GPIO_Pin   = GPIO_Pin_10;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_init);
#endif

                                                                /* ------------------ SETUP USART1 -------------------- */
    USART_Init(USART1, &usart_init);
    USART_ClockInit(USART1, &usart_clk_init);
    USART_Cmd(USART1, ENABLE);

    BSP_IntVectSet(BSP_INT_ID_USART1, ProbeRS232_RxTxISRHandler);
    BSP_IntEn(BSP_INT_ID_USART1);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_2)
    BSP_PeriphEn(BSP_PERIPH_ID_USART2);

                                                                /* ----------------- SETUP USART2 GPIO ---------------- */
#if (PROBE_RS232_UART_2_REMAP > 0)
    BSP_PeriphEn(BSP_PERIPH_ID_IOPD);
    BSP_PeriphEn(BSP_PERIPH_ID_AFIO);
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);

                                                                /* Configure GPIOD.4 as push-pull                       */
    gpio_init.GPIO_Pin   = GPIO_Pin_4;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &gpio_init);

                                                                /* Configure GPIOD.3 as input floating                  */
    gpio_init.GPIO_Pin   = GPIO_Pin_3;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &gpio_init);
#else
    BSP_PeriphEn(BSP_PERIPH_ID_IOPA);

                                                                /* Configure GPIOA.2 as push-pull                       */
    gpio_init.GPIO_Pin   = GPIO_Pin_2;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio_init);

                                                                /* Configure GPIOA.3 as input floating                  */
    gpio_init.GPIO_Pin   = GPIO_Pin_3;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_init);
#endif

                                                                /* ------------------ SETUP USART2 -------------------- */
    USART_Init(USART2, &usart_init);
    USART_ClockInit(USART2, &usart_clk_init);
    USART_Cmd(USART2, ENABLE);

    BSP_IntVectSet(BSP_INT_ID_USART2, ProbeRS232_RxTxISRHandler);
    BSP_IntEn(BSP_INT_ID_USART2);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_3)
    BSP_PeriphEn(BSP_PERIPH_ID_USART3);

                                                                /* ----------------- SETUP USART3 GPIO ---------------- */
#if (PROBE_RS232_UART_3_REMAP_PARTIAL > 0)
    BSP_PeriphEn(BSP_PERIPH_ID_IOPC);
    BSP_PeriphEn(BSP_PERIPH_ID_AFIO);
    GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);

                                                                /* Configure GPIOC.10 as push-pull                      */
    gpio_init.GPIO_Pin   = GPIO_Pin_10;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &gpio_init);

                                                                /* Configure GPIOC.11 as input floating                 */
    gpio_init.GPIO_Pin   = GPIO_Pin_11;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &gpio_init);

#elif (PROBE_RS232_UART_3_REMAP_FULL > 0)
    BSP_PeriphEn(BSP_PERIPH_ID_IOPD);
    BSP_PeriphEn(BSP_PERIPH_ID_AFIO);
    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);

                                                                /* Configure GPIOD.8 as push-pull                       */
    gpio_init.GPIO_Pin   = GPIO_Pin_8;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &gpio_init);

                                                                /* Configure GPIOD.9 as input floating                  */
    gpio_init.GPIO_Pin   = GPIO_Pin_9;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &gpio_init);

#else
    BSP_PeriphEn(BSP_PERIPH_ID_IOPB);

                                                                /* Configure GPIOB.10 as push-pull                      */
    gpio_init.GPIO_Pin   = GPIO_Pin_10;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &gpio_init);

                                                                /* Configure GPIOB.11 as input floating                 */
    gpio_init.GPIO_Pin   = GPIO_Pin_11;
    gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio_init);
#endif

                                                                /* ------------------ SETUP USART3 -------------------- */
    USART_Init(USART3, &usart_init);
    USART_ClockInit(USART3, &usart_clk_init);
    USART_Cmd(USART3, ENABLE);

    BSP_IntVectSet(BSP_INT_ID_USART3, ProbeRS232_RxTxISRHandler);
    BSP_IntEn(BSP_INT_ID_USART3);
#endif
}

/*
*********************************************************************************************************
*                                     ProbeRS232_RxTxISRHandler()
*
* Description: Handle Rx and Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*
* Note(s)    : (1) This ISR handler handles the interrupt entrance/exit as expected by
*                  by uC/OS-II v2.85.  If you are using a different RTOS (or no RTOS), then this
*                  procedure may need to be modified or eliminated.  However, the logic in the handler
*                  need not be changed.
*********************************************************************************************************
*/

void  ProbeRS232_RxTxISRHandler (void)
{
    USART_TypeDef  *usart;
    CPU_INT08U      rx_data;
    CPU_SR          cpu_sr;


    CPU_CRITICAL_ENTER();                                       /* Tell uC/OS-II that we are starting an ISR            */
    OSIntNesting++;
    CPU_CRITICAL_EXIT();


#if   (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_1)
    usart = USART1;
#elif (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_2)
    usart = USART2;
#elif (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_3)
    usart = USART3;
#else
    OSIntExit();
    return;
#endif

    if (USART_GetITStatus(usart, USART_IT_RXNE) != RESET) {
        rx_data = USART_ReceiveData(usart) & 0xFF;              /* Read one byte from the receive data register         */
        ProbeRS232_RxHandler(rx_data);

        USART_ClearITPendingBit(usart, USART_IT_RXNE);          /* Clear the USART1 Receive interrupt                   */
    }

    if (USART_GetITStatus(usart, USART_IT_TXE) != RESET) {
        ProbeRS232_TxHandler();

        USART_ClearITPendingBit(usart, USART_IT_TXE);           /* Clear the USART1 transmit interrupt                  */
    }

    OSIntExit();                                                /* Tell uC/OS-II that we are leaving the ISR            */
}

/*
*********************************************************************************************************
*                                    ProbeRS232_RxISRHandler()
*
* Description: Handle Rx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*
* Note(s)    : This function is empty because Rx interrupts are handled by ProbeRS232_RxTxISRHandler()
*********************************************************************************************************
*/

void  ProbeRS232_RxISRHandler (void)
{
}


/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntDis()
*
* Description: Disable Rx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntDis (void)
{
#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_1)
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_2)
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_3)
    USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
#endif
}


/*
*********************************************************************************************************
*                                      ProbeRS232_RxIntEn()
*
* Description: Enable Rx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxIntEn (void)
{
#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_1)
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_2)
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_3)
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#endif
}


/*
*********************************************************************************************************
*                                    ProbeRS232_TxISRHandler()
*
* Description: Handle Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*
* Note(s)    : This function is empty because Tx interrupts are handled by ProbeRS232_RxTxISRHandler()
*********************************************************************************************************
*/

void  ProbeRS232_TxISRHandler (void)
{
}


/*
*********************************************************************************************************
*                                      ProbeRS232_Tx1()
*
* Description: Transmit one byte.
*
* Argument(s): c            The byte to transmit.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_Tx1 (CPU_INT08U c)
{
#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_1)
    USART_SendData(USART1, c);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_2)
    USART_SendData(USART2, c);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_3)
    USART_SendData(USART3, c);
#endif
}


/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntDis()
*
* Description: Disable Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntDis (void)
{
#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_1)
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_2)
    USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_3)
    USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
#endif
}

/*
*********************************************************************************************************
*                                      ProbeRS232_TxIntEn()
*
* Description: Enable Tx interrupts.
*
* Argument(s): none.
*
* Return(s)  : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxIntEn (void)
{
#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_1)
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_2)
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
#endif

#if (PROBE_RS232_COMM_SEL == PROBE_RS232_UART_3)
    USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
#endif
}
