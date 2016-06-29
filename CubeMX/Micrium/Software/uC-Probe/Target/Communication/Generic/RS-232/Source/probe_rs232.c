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
* Filename      : probe_rs232.c
* Version       : V2.00
* Programmer(s) : BAN
* Note(s)       : (1) The abbreviations RX and TX refer to communication from the target's perspective.
*
*                 (2) The abbreviations RD and WR refer to reading data from the target memory and
*                     writing data to the target memory, respectively.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#define   PROBE_RS232_MODULE
#include  <probe_com.h>
#include  <probe_rs232.h>

#if (PROBE_COM_METHOD_RS232 == DEF_ENABLED)

/*
*********************************************************************************************************
*                                              LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          RS-232 PACKET FORMAT
*
* Note(s):  (1) All packets include the following parts:
*
*                   (A)  4 1-byte start delimiters, forming the ASCII representation of "uCPr".  These
*                        are the constants PROBE_RS232_PROTOCOL_RX_SD0-PROBE_RS232_PROTOCOL_?X_SD4;
*                   (B)  1 2-byte length, the length of the data segment;
*                   (C)  1 2-byte padding, unused;
*                   (D)  n   bytes of data; and
*                   (E)  1 1-byte checksum; and
*                   (F)  1 1-byte end delimiter, the character '/', which is the constant PROBE_RS232_PROTOCOL_?X_ED.
*
*                                       +-------------------+-------------------+
*                                       |   'u'   |   'C'   |   'P'   |   'r'   |
*                                       +-------------------+-------------------+
*                                       |       Length      |     Padding       |
*                                       +-------------------+-------------------+
*                                       |                  Data                 |   The data segment does not need to end on
*                                       |                   .                   |   a four-byte boundary, as might be inferred
*                                       |                   .                   |   from this diagram.
*                                       |                   .                   |
*                                       +-------------------+-------------------+
*                                       | Checksum|   '/'   |
*                                       +-------------------+
*********************************************************************************************************
*/

                                                                /* ------------- INBOUND PACKET DELIMITERS ------------ */
#define  PROBE_RS232_PROTOCOL_RX_SD0                    0x75    /* Start delimiters.                                    */
#define  PROBE_RS232_PROTOCOL_RX_SD1                    0x43
#define  PROBE_RS232_PROTOCOL_RX_SD2                    0x50
#define  PROBE_RS232_PROTOCOL_RX_SD3                    0x72
#define  PROBE_RS232_PROTOCOL_RX_ED                     0x2F    /* End   delimiter.                                     */

                                                                /* ------------ OUTBOUND PACKET DELIMITERS ------------ */
#define  PROBE_RS232_PROTOCOL_TX_SD0                    0x75    /* Start delimiters.                                    */
#define  PROBE_RS232_PROTOCOL_TX_SD1                    0x43
#define  PROBE_RS232_PROTOCOL_TX_SD2                    0x50
#define  PROBE_RS232_PROTOCOL_TX_SD3                    0x72
#define  PROBE_RS232_PROTOCOL_TX_ED                     0x2F    /* End   delimiter.                                     */

                                                                /* ----------- RECEIVE STATE MACHINE STATES ----------- */
#define  PROBE_RS232_RX_STATE_SD0                          0    /* Waiting for start first  start delimiter (SD0).      */
#define  PROBE_RS232_RX_STATE_SD1                          1    /* Waiting for start second start delimiter (SD1).      */
#define  PROBE_RS232_RX_STATE_SD2                          2    /* Waiting for start third  start delimiter (SD2).      */
#define  PROBE_RS232_RX_STATE_SD3                          3    /* Waiting for start fourth start delimiter (SD3).      */
#define  PROBE_RS232_RX_STATE_LEN1                         4    /* Waiting for length,  first  byte.                    */
#define  PROBE_RS232_RX_STATE_LEN2                         5    /* Waiting for length,  second byte.                    */
#define  PROBE_RS232_RX_STATE_PAD1                         6    /* Waiting for padding, first  byte.                    */
#define  PROBE_RS232_RX_STATE_PAD2                         7    /* Waiting for padding, second byte.                    */
#define  PROBE_RS232_RX_STATE_DATA                         8    /* Waiting for data.                                    */
#define  PROBE_RS232_RX_STATE_CHKSUM                       9    /* Waiting for checksum.                                */
#define  PROBE_RS232_RX_STATE_ED                          10    /* Waiting for end delimiter.                           */

                                                                /* ---------- TRANSMIT STATE MACHINE STATES ----------- */
#define  PROBE_RS232_TX_STATE_SD0                          0    /* Waiting to send start first  start delim. (SD0).     */
#define  PROBE_RS232_TX_STATE_SD1                          1    /* Waiting to send start second start delim. (SD1).     */
#define  PROBE_RS232_TX_STATE_SD2                          2    /* Waiting to send start third  start delim. (SD2).     */
#define  PROBE_RS232_TX_STATE_SD3                          3    /* Waiting to send start fourth start delim. (SD3).     */
#define  PROBE_RS232_TX_STATE_LEN1                         4    /* Waiting to send length,  first  byte.                */
#define  PROBE_RS232_TX_STATE_LEN2                         5    /* Waiting to send length,  second byte.                */
#define  PROBE_RS232_TX_STATE_PAD1                         6    /* Waiting to send padding, first  byte.                */
#define  PROBE_RS232_TX_STATE_PAD2                         7    /* Waiting to send padding, second byte.                */
#define  PROBE_RS232_TX_STATE_DATA                         8    /* Waiting to send data.                                */
#define  PROBE_RS232_TX_STATE_CHKSUM                       9    /* Waiting to send checksum.                            */
#define  PROBE_RS232_TX_STATE_ED                          10    /* Waiting to send end delimiter.                       */

#define  PROBE_RS232_USE_CHECKSUM                  DEF_FALSE    /* DO NOT CHANGE                                        */

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

                                                                /* ---------------- RX STATE VARIABLES ---------------- */
static  CPU_INT08U   ProbeRS232_RxState;                        /* Current state of RX state machine.                   */
static  CPU_INT16U   ProbeRS232_RxRemainLen;                    /* Remaining bytes of data to read.                     */

                                                                /* ----------------- RX PKT VARIABLES ----------------- */
static  CPU_INT16U   ProbeRS232_RxLen;                          /* Length  of data in current pkt.                      */
static  CPU_INT08U   ProbeRS232_RxBuf[PROBE_RS232_RX_BUF_SIZE]; /* Data    of current pkt.                              */
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
static  CPU_INT08U   ProbeRS232_RxChkSum;                       /* Checksum of current pkt.                             */
#endif

                                                                /* --------------- RX DATA BUF VARIABLE --------------- */
static  CPU_INT16U   ProbeRS232_RxBufWrIx;                      /* Index of next write; also number of bytes in buf.    */

                                                                /* ---------------- TX STATE VARIABLES ---------------- */
static  CPU_INT08U   ProbeRS232_TxState;                        /* Current state of TX state machine.                   */
static  CPU_BOOLEAN  ProbeRS232_TxActiveFlag;                   /* Indicates TX is currently active.                    */

                                                                /* ----------------- TX PKT VARIABLES ----------------- */
static  CPU_INT16U   ProbeRS232_TxLen;                          /* Length  of data in current pkt.                      */
static  CPU_INT08U   ProbeRS232_TxBuf[PROBE_RS232_TX_BUF_SIZE]; /* Data    of current pkt.                              */
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
static  CPU_INT08U   ProbeRS232_TxChkSum;                       /* Checksum of current pkt.                             */
#endif

                                                                /* --------------- TX DATA BUF VARIABLES -------------- */
static  CPU_BOOLEAN  ProbeRS232_TxBufInUse;                     /* Indicates TX buf currently holds a pkt.              */
static  CPU_INT16U   ProbeRS232_TxBufRdIx;                      /* Index of next read.                                  */



/*
*********************************************************************************************************
*                                        LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  CPU_INT16U  ProbeRS232_ParseRxPkt(void);

static  void        ProbeRS232_RxPkt     (void);

static  void        ProbeRS232_RxStoINT8U(CPU_INT08U  rx_data);

static  void        ProbeRS232_RxBufClr  (void);

static  void        ProbeRS232_TxStart   (void);


/*
*********************************************************************************************************
*                                       LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            ProbeRS232_Init()
*
* Description : Initialize the RS-232 communication module.
*
* Argument(s) : baud_rate       The RS-232 baud rate which will be passed to the hardware initialization.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_Init (CPU_INT32U baud_rate)
{
    ProbeRS232_RxState      = PROBE_RS232_RX_STATE_SD0;         /* Setup Rx and Tx state machines.                      */
    ProbeRS232_TxState      = PROBE_RS232_TX_STATE_SD0;

    ProbeRS232_TxLen        = 0;
    ProbeRS232_TxActiveFlag = DEF_FALSE;
    ProbeRS232_TxBufInUse   = DEF_FALSE;

#if (PROBE_COM_STAT_EN     == DEF_ENABLED)
    ProbeRS232_RxCtr        = 0;
    ProbeRS232_TxCtr        = 0;
#endif

#if (PROBE_RS232_PARSE_TASK == DEF_TRUE)
    ProbeRS232_OS_Init();
#endif

    ProbeRS232_InitTarget(baud_rate);                           /* Initialize target specific code.                     */
}


/*
*********************************************************************************************************
*                                            ProbeRS232_Task()
*
* Description : The task which parses the received packet, forms a response, and begins transmission.
*
* Argument(s) : p_arg           The argument passed to 'ProbeRS232_Task()' by  'ProbeRS232_OS_Task()'.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_OS_Task().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_RS232_PARSE_TASK == DEF_TRUE)
void  ProbeRS232_Task (void *p_arg)
{
    CPU_INT16U  len;


    (void)p_arg;

    while (DEF_TRUE) {
        ProbeRS232_OS_Pend();                                   /* Wait for a packet to be received.                    */

        len = ProbeRS232_ParseRxPkt();                          /* Parse packet and formulate a response.               */

        if (len > 0) {                                          /* If we have a response.                               */
            ProbeRS232_TxLen = len;
            ProbeRS232_TxStart();
        }
    }
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            RX & TX HANDLERS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         ProbeRS232_RxHandler()
*
* Description : Handle a received byte.
*
* Argument(s) : rx_data         Received data byte.
*
* Return(s)   : none.
*
* Caller(s)   : Rx ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_RxHandler (CPU_INT08U rx_data)
{
#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    ProbeRS232_RxCtr++;
#endif

    switch (ProbeRS232_RxState) {
        case PROBE_RS232_RX_STATE_SD0:                          /* Rx'd the start 1st start delimiter (SD0).            */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_SD0) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD1;
                 ProbeRS232_RxBufClr();
             }
             break;


        case PROBE_RS232_RX_STATE_SD1:                          /* Rx'd the start 2nd start delimiter (SD1).            */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_SD1) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD2;
             } else {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             }
             break;


        case PROBE_RS232_RX_STATE_SD2:                          /* Rx'd the start 3rd start delimiter (SD2).            */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_SD2) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD3;
             } else {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             }
             break;


        case PROBE_RS232_RX_STATE_SD3:                          /* Rx'd the start 4th start delimiter (SD3).            */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_SD3) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_LEN1;
             } else {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             }
             break;


        case PROBE_RS232_RX_STATE_LEN1:                         /* Rx'd the 1st len byte.                               */
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
             ProbeRS232_RxChkSum    = rx_data;
#endif
             ProbeRS232_RxRemainLen = rx_data;
             ProbeRS232_RxState     = PROBE_RS232_RX_STATE_LEN2;
             break;


        case PROBE_RS232_RX_STATE_LEN2:                         /* Rx'd the 2nd len byte.                               */
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
             ProbeRS232_RxChkSum    += rx_data;
#endif
             ProbeRS232_RxRemainLen |= rx_data << 8;

                                                                /* Chk len.                                             */
             if ((ProbeRS232_RxRemainLen == 0) || (ProbeRS232_RxRemainLen > PROBE_RS232_RX_BUF_SIZE)) {
                 ProbeRS232_RxState  = PROBE_RS232_RX_STATE_SD0;
             } else {
                 ProbeRS232_RxLen    = ProbeRS232_RxRemainLen;
                 ProbeRS232_RxState  = PROBE_RS232_RX_STATE_PAD1;
             }
             break;


        case PROBE_RS232_RX_STATE_PAD1:                         /* Rx'd the 1st padding byte.                           */
             ProbeRS232_RxState = PROBE_RS232_RX_STATE_PAD2;
             break;


        case PROBE_RS232_RX_STATE_PAD2:                         /* Rx'd the 2nd  padding byte.                          */
             ProbeRS232_RxState = PROBE_RS232_RX_STATE_DATA;
             break;


        case PROBE_RS232_RX_STATE_DATA:                         /* Rx'd data.                                           */
             ProbeRS232_RxStoINT8U(rx_data);
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
             ProbeRS232_RxChkSum += rx_data;
#endif
             if (--ProbeRS232_RxRemainLen == 0) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_CHKSUM;
             }
             break;


        case PROBE_RS232_RX_STATE_CHKSUM:                       /* Rx'd the checksum.                                   */
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
             ProbeRS232_RxChkSum += rx_data;
             if ((ProbeRS232_RxChkSum & 0xFF) == 0x00) {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_ED;
             } else {
                 ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             }
#else
             ProbeRS232_RxState = PROBE_RS232_RX_STATE_ED;
#endif
             break;


        case PROBE_RS232_RX_STATE_ED:                           /* Rx'd the end delimiter.                              */
             if (rx_data == PROBE_RS232_PROTOCOL_RX_ED) {
                 ProbeRS232_RxPkt();                            /* Parse rx'd pkt.                                      */
             }
             ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             break;


        default:
             ProbeRS232_RxState = PROBE_RS232_RX_STATE_SD0;
             break;
    }
}


/*
*********************************************************************************************************
*                                         ProbeRS232_TxHandler()
*
* Description : Handle a byte transmission.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Tx ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeRS232_TxHandler (void)
{
    CPU_INT08U  tx_data;


#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    if ((ProbeRS232_TxState != PROBE_RS232_TX_STATE_SD0) ||
        (ProbeRS232_TxLen    > 0                       )) {
        ProbeRS232_TxCtr++;
    }
#endif

    switch (ProbeRS232_TxState) {
        case PROBE_RS232_TX_STATE_SD0:
             if (ProbeRS232_TxLen > 0) {                        /* If pkt is waiting to be sent ...                     */
                 ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_SD0);   /*  ... tx start 1st start delimiter (SD0).             */
                 ProbeRS232_TxActiveFlag = DEF_TRUE;
                 ProbeRS232_TxState      = PROBE_RS232_TX_STATE_SD1;
                 ProbeRS232_TxBufRdIx    = 0;

             } else {                                           /* If no pkt is waiting to be sent ...                  */
                 ProbeRS232_TxActiveFlag = DEF_FALSE;
                 ProbeRS232_TxIntDis();                         /*  ... dis tx int's.                                   */
             }
             break;


        case PROBE_RS232_TX_STATE_SD1:                          /* Tx start 2nd start delimiter (SD1).                  */
             ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_SD1);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_SD2;
             break;


        case PROBE_RS232_TX_STATE_SD2:                          /* Tx start 3rd start delimiter (SD2).                  */
             ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_SD2);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_SD3;
             break;


        case PROBE_RS232_TX_STATE_SD3:                          /* Tx start 4th start delimiter (SD3).                  */
             ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_SD3);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_LEN1;
             break;


        case PROBE_RS232_TX_STATE_LEN1:                         /* Tx 1st len byte.                                     */
             tx_data             = ProbeRS232_TxLen & 0xFF;
             ProbeRS232_Tx1(tx_data);
             ProbeRS232_TxState  = PROBE_RS232_TX_STATE_LEN2;
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
             ProbeRS232_TxChkSum = tx_data;
#endif
             break;


        case PROBE_RS232_TX_STATE_LEN2:                         /* Tx 2nd len byte.                                     */
             tx_data              = ProbeRS232_TxLen >> 8;
             ProbeRS232_Tx1(tx_data);
             ProbeRS232_TxState   = PROBE_RS232_TX_STATE_PAD1;
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
             ProbeRS232_TxChkSum += tx_data;
#endif
             break;


        case PROBE_RS232_TX_STATE_PAD1:                         /* Tx 1st padding byte.                                 */
             ProbeRS232_Tx1(0);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_PAD2;
             break;


        case PROBE_RS232_TX_STATE_PAD2:                         /* Tx 2nd padding byte.                                 */
             ProbeRS232_Tx1(0);
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_DATA;
             break;


        case PROBE_RS232_TX_STATE_DATA:                         /* Tx data.                                             */
             tx_data = ProbeRS232_TxBuf[ProbeRS232_TxBufRdIx];
             ProbeRS232_Tx1(tx_data);
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
             ProbeRS232_TxChkSum += tx_data;
#endif
             ProbeRS232_TxBufRdIx++;
             if (ProbeRS232_TxBufRdIx >= ProbeRS232_TxLen) {
                 ProbeRS232_TxState = PROBE_RS232_TX_STATE_CHKSUM;
                 ProbeRS232_TxLen   = 0;
             }
             break;


        case PROBE_RS232_TX_STATE_CHKSUM:                       /* Tx checksum.                                         */
#if (PROBE_RS232_USE_CHECKSUM == DEF_TRUE)
             ProbeRS232_Tx1(ProbeRS232_TxChkSum);
#else
             ProbeRS232_Tx1(0);
#endif
             ProbeRS232_TxState = PROBE_RS232_TX_STATE_ED;
             break;


        case PROBE_RS232_TX_STATE_ED:                           /* Tx end delimiter.                                    */
             ProbeRS232_Tx1(PROBE_RS232_PROTOCOL_TX_ED);
             ProbeRS232_TxState    = PROBE_RS232_TX_STATE_SD0;
             ProbeRS232_TxBufInUse = DEF_FALSE;
             break;


        default:
             ProbeRS232_TxState      = PROBE_RS232_TX_STATE_SD0;
             ProbeRS232_TxActiveFlag = DEF_FALSE;
             ProbeRS232_TxIntDis();                             /* No more data to send, dis tx int's.                  */
             break;
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         ProbeRS232_ParseRxPkt()
*
* Description : Parse a received packet & formulate a response.
*
* Argument(s) : none.
*
* Return(s)   : The number of bytes in the data segment of the packet to transmit in response.
*
* Caller(s)   : ProbeRS232_Task(),
*               ProbeRS232_RxPkt().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT16U  ProbeRS232_ParseRxPkt (void)
{
    CPU_INT16U  tx_len;


    if (ProbeRS232_TxBufInUse == DEF_TRUE) {                    /* Do cmds only if Tx buffer is free.                   */
        return (0);
    }

    ProbeRS232_TxBufInUse = DEF_TRUE;
    tx_len                = ProbeCom_ParseRxPkt((void     *)ProbeRS232_RxBuf,
                                                (void     *)ProbeRS232_TxBuf,
                                                (CPU_INT16U)ProbeRS232_RxLen,
                                                (CPU_INT16U)PROBE_RS232_TX_BUF_SIZE);

    return (tx_len);
}


/*
*********************************************************************************************************
*                                           ProbeRS232_RxPkt()
*
* Description : Handle a received packet.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_RxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_RxPkt (void)
{
#if (PROBE_RS232_PARSE_TASK == DEF_TRUE)
    ProbeRS232_OS_Post();                                       /* We have a whole packet, signal task to parse it.     */



#else



#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr = 0;
#endif
    CPU_INT16U  len;


    len = ProbeRS232_ParseRxPkt();                              /* We have a whole packet, parse it.                    */
    if (len > 0) {
        CPU_CRITICAL_ENTER();
        ProbeRS232_TxLen = len;
        ProbeRS232_TxStart();
        CPU_CRITICAL_EXIT();
    }
#endif
}


/*
*********************************************************************************************************
*                                         ProbeRS232_RxStoINT8U()
*
* Description : Store a byte in the receive buffer.
*
* Argument(s) : rx_data     Byte of data to store in the buffer.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_RxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_RxStoINT8U (CPU_INT08U rx_data)
{
    if (ProbeRS232_RxBufWrIx < PROBE_RS232_RX_BUF_SIZE) {
        ProbeRS232_RxBuf[ProbeRS232_RxBufWrIx++] = rx_data;
    }
}


/*
*********************************************************************************************************
*                                          ProbeRS232_RxBufClr()
*
* Description : Clear the data segment buffer write index.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_RxHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_RxBufClr (void)
{
    ProbeRS232_RxBufWrIx = 0;
}


/*
*********************************************************************************************************
*                                          ProbeRS232_TxStart()
*
* Description : Cause transmission to begin.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : ProbeRS232_Task(),
*               ProbeRS232_RxPkt().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeRS232_TxStart (void)
{
    if (ProbeRS232_TxActiveFlag == DEF_FALSE) {                 /* If no other transmission is in progress ...          */
        ProbeRS232_TxHandler();                                 /*  ... Handle transmit                    ...          */
        ProbeRS232_TxIntEn();                                   /*  ... Enable transmit interrupts.                     */
    }
}


#endif
