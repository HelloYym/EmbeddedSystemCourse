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
*                                         Communication: Generic
*
* Filename      : probe_com.c
* Version       : V2.00
* Programmer(s) : BAN
* Note(s)       : (1) This file contains code to respond to generic (non protocol-dependent)
*                     commands received by the target.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define   PROBE_COM_MODULE
#include  <probe_com.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             DATA FORMATS
*
* Note(s):  (1) The first word in all TX data segments is identical:
*
*               (A)  A 2-byte format;
*               (B)  A 1-byte status;
*               (C)  A 1-byte modifier, currently unused.
*
*
*           (2) The first two bytes in all RX data segments is identical:
*               (A)  A 2-byte format;
*
*           (3) The following data formats are currently defined:
*
*               (A)  PROBE_COM_FMT_?X_QUERY.  The RX request queries the target about a particular setup
*                    parameter or capability.
*
*               (B)  PROBE_COM_FMT_?X_SIMPLE_RD.  The RX request instructs the target to send data read
*                    from its memory, for a certain {memory address, data length} pair (which is given in
*                    the request).
*
*               (C)  PROBE_COM_FMT_?X_SIMPLE_WR.  The RX request instructs the target to write certain
*                    data into its memory, for a certain {memory address, data length, data} triplet
*                    (which is given in the request).
*
*               (D)  PROBE_COM_FMT_?X_MULTIPLE_RD.  The RX request instructs the target to send data read
*                    from its memory, for a certain set of {memory address, data length} pairs (which are
*                    given in the request).
*
*               (E)  PROBE_COM_FMT_?X_STR_RD.  The RX request instructs the target to return a string
*                    that the user has stored in the target's string buffer.
**********************************************************************************************************
*/

#define  PROBE_COM_FMT_TX_ERROR                       0x8000

#define  PROBE_COM_FMT_RX_QUERY                       0x0001
#define  PROBE_COM_FMT_TX_QUERY                       0x8001

#define  PROBE_COM_FMT_RX_SIMPLE_RD                   0x0002
#define  PROBE_COM_FMT_TX_SIMPLE_RD                   0x8002

#define  PROBE_COM_FMT_RX_SIMPLE_WR                   0x0003
#define  PROBE_COM_FMT_TX_SIMPLE_WR                   0x8003

#define  PROBE_COM_FMT_RX_INFO                        0x0004
#define  PROBE_COM_FMT_TX_INFO                        0x8004

#define  PROBE_COM_FMT_RX_MULTIPLE_RD                 0x0007
#define  PROBE_COM_FMT_TX_MULTIPLE_RD                 0x8007

#define  PROBE_COM_FMT_TX_MULTIPLE_RD_LO                0x07
#define  PROBE_COM_FMT_TX_MULTIPLE_RD_HI                0x80

#define  PROBE_COM_FMT_RX_MULTIPLE_WR                 0x0008
#define  PROBE_COM_FMT_TX_MULTIPLE_WR                 0x8008

#define  PROBE_COM_FMT_RX_STR_RD                      0x0009
#define  PROBE_COM_FMT_TX_STR_RD                      0x8009

#define  PROBE_COM_FMT_RX_STR_WR                      0x000A
#define  PROBE_COM_FMT_TX_STR_WR                      0x800A

/*
*********************************************************************************************************
*                                             STATUS CONSTANTS
*
* Note(s):  (1) The following status constants are currently defined:
*
*               (A)  PROBE_COM_STATUS_OK.  The target was able to respond to the command.
*
*               (B)  PROBE_COM_STATUS_STR_NONE.  A PROBE_COM_FMT_RX_STR_GET packet is received, but the
*                    target has no string to send.
*
*               (C)  PROBE_COM_STATUS_UNKNOWN_REQUEST.  A request packet with an unknown type was
*                    received.
*
*               (D)  PROBE_COM_STATUS_QUERY_NOT_SUPPORTED.  A PROBE_COM_FMT_RX_QUERY packet is received,
*                    but the query is not supported.
*
*               (E)  PROBE_COM_STATUS_TX_PKT_TOO_LARGE.  The response to the request would be too large
*                    to fit into the target.
*
*               (F)  PROBE_COM_STATUS_RX_PKT_WRONG_SIZE.  The request packet is not the expected size.
*
*               (G)  PROBE_COM_STATUS_FAIL.  Another error occurred.
**********************************************************************************************************
*/

#define  PROBE_COM_STATUS_OK                            0x01
#define  PROBE_COM_STATUS_TELEMETRY_NONE                0xF7
#define  PROBE_COM_STATUS_STR_NONE                      0xF8
#define  PROBE_COM_STATUS_UNKNOWN_REQUEST               0xF9
#define  PROBE_COM_STATUS_QUERY_NOT_SUPPORTED           0xFC
#define  PROBE_COM_STATUS_TX_PKT_TOO_LARGE              0xFD
#define  PROBE_COM_STATUS_RX_PKT_WRONG_SIZE             0xFE
#define  PROBE_COM_STATUS_FAIL                          0xFF

/*
*********************************************************************************************************
*                                                QUERIES
*
* Note(s):  (1) The following queries are currently defined:
*
*               (A)  PROBE_COM_QUERY_MAX_RX_SIZE.  The target responds with the data size of the largest
*                    packet it can receive.
*
*               (B)  PROBE_COM_QUERY_MAX_TX_SIZE.  The target responds with the data size of the largest
*                    packet it can send.
*
*               (C)  PROBE_COM_ENDIANNESS_TEST.  The target responds with a 4-byte data value.  On
*                    little-endian CPUs, this will be received as 0x12345678; on big-endian CPUs, this
*                    will be received as 0x87654321.
*
*               (D)  PROBE_COM_QUERY_FMT_SUPPORT.  The target responds with a list of the formats the
*                    target can respond to.
*
*               (E)  PROBE_COM_QUERY_VERSION.  The target responds with the communication module version.
**********************************************************************************************************
*/

                                                                /* ------------------- CONFIGURATION ------------------ */
#define  PROBE_COM_QUERY_MAX_RX_SIZE                  0x0101
#define  PROBE_COM_QUERY_MAX_TX_SIZE                  0x0102

                                                                /* ----------------- TARGET PROPERTIES ---------------- */
#define  PROBE_COM_QUERY_ENDIANNESS_TEST              0x0201

                                                                /* ------------- COMMUNICATION CAPABILITIES ----------- */
#define  PROBE_COM_QUERY_FMT_SUPPORT                  0x1001
#define  PROBE_COM_QUERY_VERSION                      0x1002

/*
*********************************************************************************************************
*                                            INFO PACKET TYPES
*
* Note(s):  (1) The following info packet types are currently defined:
*
*               (A)  PROBE_COM_INFO_PKT_SIZE.  Probe supplies the size of the next packet.
**********************************************************************************************************
*/

#define  PROBE_COM_INFO_PKT_SIZE                      0x0001

/*
*********************************************************************************************************
*                                               MODIFIERS
*
* Note(s):  (1) The following modifiers are currently defined:
*
*               (A)  PROBE_COM_MODIFIER_NONE.  This is the generic modifier.
*
*               (B)  PROBE_COM_MODIFIER_STR_HAVE.  The target indicates that it has a string to transmit.
**********************************************************************************************************
*/

#define  PROBE_COM_MODIFIER_NONE                        0x00
#define  PROBE_COM_MODIFIER_STR_HAVE                    0x01

/*
*********************************************************************************************************
*                                               HEADER SIZES
*
* Note(s):  (1) Every RX packet has a 2-byte "header".
*
*           (2) Every TX packet has a 4-byte "header".
**********************************************************************************************************
*/

#define  PROBE_COM_SIZE_RX_HDR                             2
#define  PROBE_COM_SIZE_TX_HDR                             4

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

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
static  CPU_INT16U                 ProbeCom_StrBufWrIx;
static  CPU_INT16U                 ProbeCom_StrBufRdIx;
static  CPU_CHAR                   ProbeCom_StrBuf[PROBE_COM_STR_BUF_SIZE];
static  PROBE_COM_STR_HDNLR_FNCT   ProbeCom_StrHndlr;
#endif

static  CPU_INT32U                 ProbeCom_EndiannessTest;

static  PROBE_COM_INFO_HDNLR_FNCT  ProbeCom_InfoHndlr;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  CPU_INT08U   ProbeCom_PktModifier  (void);

static  CPU_INT16U   ProbeCom_CmdErr       (CPU_INT08U   *ptx_buf,
                                            CPU_INT08U    pcomm_err);

static  CPU_INT16U   ProbeCom_CmdQuery     (CPU_INT08U   *prx_buf,
                                            CPU_INT08U   *ptx_buf,
                                            CPU_INT16U    rx_pkt_size,
                                            CPU_INT16U    tx_buf_size);

static  CPU_INT16U   ProbeCom_CmdInfo      (CPU_INT08U   *prx_buf,
                                            CPU_INT08U   *ptx_buf,
                                            CPU_INT16U    rx_pkt_size,
                                            CPU_INT16U    tx_buf_size);

static  CPU_INT16U   ProbeCom_CmdSimpleRd  (CPU_INT08U   *prx_buf,
                                            CPU_INT08U   *ptx_buf,
                                            CPU_INT16U    rx_pkt_size,
                                            CPU_INT16U    tx_buf_size);

static  CPU_INT16U   ProbeCom_CmdMultipleRd(CPU_INT08U   *prx_buf,
                                            CPU_INT08U   *ptx_buf,
                                            CPU_INT16U    rx_pkt_size,
                                            CPU_INT16U    tx_buf_size);

#if (PROBE_COM_SUPPORT_WR == DEF_TRUE)
static  CPU_INT16U   ProbeCom_CmdSimpleWr  (CPU_INT08U   *prx_buf,
                                            CPU_INT08U   *ptx_buf,
                                            CPU_INT16U    rx_pkt_size,
                                            CPU_INT16U    tx_buf_size);

static  CPU_INT16U   ProbeCom_CmdMultipleWr(CPU_INT08U   *prx_buf,
                                            CPU_INT08U   *ptx_buf,
                                            CPU_INT16U    rx_pkt_size,
                                            CPU_INT16U    tx_buf_size);
#endif

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
static  CPU_INT16U   ProbeCom_CmdStrRd     (CPU_INT08U   *prx_buf,
                                            CPU_INT08U   *ptx_buf,
                                            CPU_INT16U    rx_pkt_size,
                                            CPU_INT16U    tx_buf_size);

static  CPU_INT16U   ProbeCom_CmdStrWr     (CPU_INT08U   *prx_buf,
                                            CPU_INT08U   *ptx_buf,
                                            CPU_INT16U    rx_pkt_size,
                                            CPU_INT16U    tx_buf_size);
#endif


                                                                /* ------------------- RD FROM RX PKT ----------------- */
static  CPU_INT08U   ProbeCom_GetINT8U     (CPU_INT08U  **pbuf);

static  CPU_INT16U   ProbeCom_GetINT16U    (CPU_INT08U  **pbuf);

static  CPU_INT32U   ProbeCom_GetINT32U    (CPU_INT08U  **pbuf);

                                                                /* -------------------- WR TO TX BUF ----------------- */
static  void         ProbeCom_StoINT8U     (CPU_INT08U  **pbuf,
                                            CPU_INT08U    data);

static  void         ProbeCom_StoINT16U    (CPU_INT08U  **pbuf,
                                            CPU_INT16U    data);

#if 0
static  void         ProbeCom_StoINT32U    (CPU_INT08U  **pbuf,
                                            CPU_INT32U    data);
#endif

                                                                /* -------------- DETERMINE PKT MODIFIER -------------- */
#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
static  CPU_BOOLEAN  ProbeCom_StrRdy       (void);
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           ProbeCom_Init()
*
* Description : Initialize the module.
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

void  ProbeCom_Init (void)
{
#if (PROBE_COM_SUPPORT_STR  == DEF_TRUE)
    ProbeCom_StrBufWrIx     = 0;
    ProbeCom_StrBufRdIx     = 0;

    ProbeCom_OS_Init();
#endif

#if (PROBE_COM_STAT_EN      == DEF_ENABLED)
    ProbeCom_RxPktCtr       = 0;
    ProbeCom_TxPktCtr       = 0;
    ProbeCom_TxSymCtr       = 0;
    ProbeCom_TxSymByteCtr   = 0;
    ProbeCom_ErrPktCtr      = 0;

#if (PROBE_COM_SUPPORT_STR  == DEF_TRUE)
    ProbeCom_TxStrCtr       = 0;
#endif

#if (PROBE_COM_SUPPORT_WR   == DEF_TRUE)
    ProbeCom_RxSymCtr       = 0;
    ProbeCom_RxSymByteCtr   = 0;
#endif
#endif

    ProbeCom_EndiannessTest = 0x12345678L;

    ProbeCom_InfoHndlr      = (PROBE_COM_INFO_HDNLR_FNCT)0;
#if (PROBE_COM_SUPPORT_STR  == DEF_TRUE)
    ProbeCom_StrHndlr       = (PROBE_COM_STR_HDNLR_FNCT )0;
#endif
}


/*
*********************************************************************************************************
*                                         ProbeCom_ParseRxPkt()
*
* Description : Parse a packet & formulate a response.
*
* Argument(s) : prx_pkt         Pointer to the receive  packet buffer
*
*               ptx_pkt         Pointer to the transmit packet buffer
*
*               rx_pkt_size     Size of the received packet
*
*               tx_pkt_size     Size of the transmit packet buffer
*
* Return(s)   : The number of bytes in the data segment of the packet to transmit in response.
*
* Caller(s)   : Tasks/receive handlers in communications-specific drivers, e.g., probe_rs232,
*               probe_usb, probe_tcpip, etc.).
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT16U  ProbeCom_ParseRxPkt (void        *prx_pkt,
                                 void        *ptx_pkt,
                                 CPU_INT16U   rx_pkt_size,
                                 CPU_INT16U   tx_buf_size)
{
    CPU_INT16U   tx_buf_wr;
    CPU_INT16U   format;
    CPU_INT08U  *prx_pkt_08;
    CPU_INT08U  *ptx_pkt_08;


    if (rx_pkt_size < 2) {
        return (0);
    }

    prx_pkt_08  = (CPU_INT08U *)prx_pkt;
    ptx_pkt_08  = (CPU_INT08U *)ptx_pkt;
    format      = (prx_pkt_08[1] << 8) + prx_pkt_08[0];
    prx_pkt_08 += 2;

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    ProbeCom_RxPktCtr++;
    ProbeCom_TxPktCtr++;
#endif

    switch (format) {
        case PROBE_COM_FMT_RX_QUERY:
             tx_buf_wr = ProbeCom_CmdQuery(       prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_INFO:
             tx_buf_wr = ProbeCom_CmdInfo(        prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_SIMPLE_RD:
             tx_buf_wr = ProbeCom_CmdSimpleRd(    prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_MULTIPLE_RD:
             tx_buf_wr = ProbeCom_CmdMultipleRd(  prx_pkt_08, ptx_pkt_08,  rx_pkt_size, tx_buf_size);
             break;

#if (PROBE_COM_SUPPORT_WR == DEF_TRUE)
        case PROBE_COM_FMT_RX_SIMPLE_WR:
             tx_buf_wr = ProbeCom_CmdSimpleWr(    prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_MULTIPLE_WR:
             tx_buf_wr = ProbeCom_CmdMultipleWr(  prx_pkt_08, ptx_pkt_08, rx_pkt_size, tx_buf_size);
             break;
#endif

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
        case PROBE_COM_FMT_RX_STR_RD:
             tx_buf_wr = ProbeCom_CmdStrRd(       prx_pkt_08, ptx_pkt_08,  rx_pkt_size, tx_buf_size);
             break;

        case PROBE_COM_FMT_RX_STR_WR:
             tx_buf_wr = ProbeCom_CmdStrWr(       prx_pkt_08, ptx_pkt_08,  rx_pkt_size, tx_buf_size);
             break;
#endif

        default:
             tx_buf_wr = ProbeCom_CmdErr(                     ptx_pkt_08,  PROBE_COM_STATUS_UNKNOWN_REQUEST);
             break;
    }

    return (tx_buf_wr);
}


/*
*********************************************************************************************************
*                                         ProbeCom_InfoHndlrSet()
*
* Description : Set the handler that will be invoked when an info packet is received.
*
* Argument(s) : hndlr           The handler that will be invoked.
*
* Return(s)   : none.
*
* Caller(s)   : Application or communications-specific driver.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  ProbeCom_InfoHndlrSet (PROBE_COM_INFO_HDNLR_FNCT  hndlr)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR  cpu_sr;
#endif


    CPU_CRITICAL_ENTER();
    ProbeCom_InfoHndlr = hndlr;
    CPU_CRITICAL_EXIT();
}


/*
*********************************************************************************************************
*                                         ProbeCom_StrHndlrSet()
*
* Description : Set the handler that will be invoked when an string write packet is received.
*
* Argument(s) : hndlr           The handler that will be invoked.
*
* Return(s)   : none.
*
* Caller(s)   : Application or communications-specific driver.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
void  ProbeCom_StrHndlrSet (PROBE_COM_STR_HDNLR_FNCT  hndlr)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR  cpu_sr;
#endif


    CPU_CRITICAL_ENTER();
    ProbeCom_StrHndlr = hndlr;
    CPU_CRITICAL_EXIT();
}
#endif


/*
*********************************************************************************************************
*                                            ProbeCom_TxStr()
*
* Description : Append a string in the string buffer.
*
* Argument(s) : pstr        Pointer to the string to send.
*
*               dly         Delay time (in milliseconds).  If this value is zero, then
*                           the function will return after queueing in the buffer the portion that fits
*                           immediately.  Otherwise, the function will delay for a certain number of
*                           milliseconds until the entire string has been queued in the buffer.
*
* Return(s)   : DEF_TRUE   if the entire string was queued in the buffer.
*               DEF_FALSE  if the entire string could not be queued in the buffer.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) The string buffer is implemented as a circular buffer.  This function is one of two
*                   points of access for this buffer, the other being in the task or ISR which forms the
*                   tx packets.  Only this function should modify the global current write index
*                   (ProbeCom_StrBufWrIx); only the task or ISR which forms the packets should modify the
*                   global current read index (ProbeCom_StrBufRdIx).
*
*               (2) The global current write index (ProbeCom_StrBufWrIx) is the index of the next location
*                   in the buffer to write.  The global current read index (ProbeCom_StrBufRdIx) is the
*                   index of the next location in the buffer to read.
*
*               (3) The string buffer, an array of PROBE_COM_STR_BUF_SIZE bytes, can only hold
*                   (PROBE_COM_STR_BUF_SIZE - 1) bytes so that the condition
*
*                                        ProbeCom_StrBufWrIx == ProbeCom_StrBufRdIx
*
*                   will be true if and only if the buffer is empty.  Consequently, this function
*                   always leaves an empty space in the buffer.
*
*               (4) If called from an ISR, dly MUST be 0.
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
CPU_BOOLEAN  ProbeCom_TxStr (CPU_CHAR    *pstr,
                             CPU_INT16U   dly)
{
    CPU_BOOLEAN  ret;
    CPU_INT32U   len;
    CPU_INT16U   wr_ix;
    CPU_INT16U   rd_ix;
    CPU_INT16U   wr_ix_n;

    CPU_INT16U   nbytes_free;
    CPU_INT16U   nbytes_wr;


    if (dly == 0) {
        ret = ProbeCom_OS_Pend(DEF_FALSE);
    } else {
        ret = ProbeCom_OS_Pend(DEF_TRUE);
    }

    if (ret == DEF_FALSE) {
        return (DEF_FALSE);
    }

    len = (CPU_INT32U)Str_Len(pstr);                            /* Determine length of the string (without NULL byte).  */

    while (len > 0) {
        rd_ix = ProbeCom_StrBufRdIx;
        wr_ix = ProbeCom_StrBufWrIx;

        if (rd_ix > wr_ix) {                                    /* If rd_ix > wr_ix, store str at                       */
            nbytes_free = rd_ix - wr_ix - 1;                    /*           buf interval [wr_ix, rd_ix - 1).           */
        } else {
            if (rd_ix == 0) {                                   /* If rd_ix <= wr_ix && rd_ix == 0, store str at        */
                                                                /*           buf interval [wr_ix, end_ix - 1).          */
                nbytes_free = PROBE_COM_STR_BUF_SIZE - wr_ix - 1;
            } else {                                            /* If rd_ix <= wr_ix && rd_ix != 0, store str at        */
                nbytes_free = PROBE_COM_STR_BUF_SIZE - wr_ix;   /*           buf interval [wr_ix, end_ix).              */
            }
        }

        if (nbytes_free == 0) {                                 /* If the buf is full ...                               */
            if (dly == 0) {                                     /* (a) Rtn if dly = 0.                                  */
                ProbeCom_OS_Post();
                return (DEF_FALSE);
            } else {                                            /* (b) Call OS fnct to dly and continue.                */
                ProbeCom_OS_Dly(dly);
            }
        } else {
            if (nbytes_free > len) {                            /* If str is shorter than free space.                   */
                nbytes_wr = len;
            } else {
                nbytes_wr = nbytes_free;
            }

            wr_ix_n = wr_ix + nbytes_wr;                        /* Assign wr ix after wr.                               */

            if (wr_ix_n == PROBE_COM_STR_BUF_SIZE) {            /* Wrap buf ix around.                                  */
                wr_ix_n = 0;
            }

            Mem_Copy((void     *)&ProbeCom_StrBuf[wr_ix],       /* Copy str to buf.                                     */
                     (void     *) pstr,
                     (CPU_SIZE_T) nbytes_wr);

            ProbeCom_StrBufWrIx  = wr_ix_n;                     /* Assign new global wr ix.                             */
            pstr               += nbytes_wr;                    /* Inc str ptr.                                         */
            len                -= nbytes_wr;                    /* Dec str len.                                         */
        }
    }

    ProbeCom_OS_Post();
    return (DEF_TRUE);                                          /* Rtn TRUE to indicate success.                        */
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
*                                            ProbeCom_StrRdy()
*
* Description : Check if a string is ready for transmission.
*
* Argument(s) : none.
*
* Return(s)   : DEF_TRUE   if a  string is in the buffer for transmission.
*               DEF_FALSE  if no string is in the buffer for transmission.
*
* Caller(s)   : ProbeCom_PktModifier().
*
* Note(s)     : (1) See Notes for 'ProbeCom_TxStr()'.
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
static  CPU_BOOLEAN  ProbeCom_StrRdy (void)
{
    CPU_BOOLEAN  rdy;
    CPU_INT16U   wr_ix;
    CPU_INT16U   rd_ix;


    wr_ix = ProbeCom_StrBufWrIx;
    rd_ix = ProbeCom_StrBufRdIx;

    if (wr_ix == rd_ix) {
        rdy = DEF_FALSE;
    } else {
        rdy = DEF_TRUE;
    }

    return (rdy);
}
#endif


/*
*********************************************************************************************************
*                                         ProbeCom_PktModifier()
*
* Description : Get packet modfier byte.
*
* Argument(s) : none.
*
* Return(s)   : The modifier byte.
*
* Caller(s)   : Various command handlers (ProbeCom_Cmd????()).
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT08U  ProbeCom_PktModifier (void)
{
    CPU_INT08U  mod;


    mod = PROBE_COM_MODIFIER_NONE;

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
    if (ProbeCom_StrRdy() == DEF_TRUE) {
        DEF_BIT_SET(mod, PROBE_COM_MODIFIER_STR_HAVE);
    }
#endif

    return (mod);
}


/*
*********************************************************************************************************
*                                           ProbeCom_CmdErr()
*
* Description : Formulate error response when the target receives a request it cannot handle.
*
* Argument(s) : ptx_buf      Pointer to the transmit buffer
*
*               comm_err     Error that occurred
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : Various command handlers (ProbeCom_Cmd????()).
*
* Note(s)     : (1) The TX format:
*
*                   (A) A 2-byte format  , indicating the data segment format;
*                   (B) A 1-byte constant, PROBE_RS232_OK,   if the location can    be read;
*                       A 1-byte constant, PROBE_RS232_FAIL, if the location cannot be read.
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*********************************************************************************************************
*/

static  CPU_INT16U  ProbeCom_CmdErr (CPU_INT08U  *ptx_buf,
                                     CPU_INT08U   comm_err)
{
#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    ProbeCom_ErrPktCtr++;
#endif

    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_ERROR);       /* TX pkt format.                                  */
    ProbeCom_StoINT8U( &ptx_buf, comm_err);                     /* Target status.                                  */
    ProbeCom_StoINT8U( &ptx_buf, ProbeCom_PktModifier());       /* Modifier.                                       */
    return (PROBE_COM_SIZE_TX_HDR);                             /* Rtn TX data segment size =  4 (= Tx hdr size).  */
}


/*
*********************************************************************************************************
*                                           ProbeCom_CmdQuery()
*
* Description : Parse the FMT_QUERY command & formulate response.  This command asks the target
*               about its capabilities, which are returned in response.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte query , indicating the query type.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |          Query          |
*                         +-------------------------+-------------------------+
*
*              (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier;
*                   (E) A n-byte answer, the answer to the query.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*                         |                       Answer                      |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*********************************************************************************************************
*/

static  CPU_INT16U  ProbeCom_CmdQuery (CPU_INT08U  *prx_buf,
                                       CPU_INT08U  *ptx_buf,
                                       CPU_INT16U   rx_pkt_size,
                                       CPU_INT16U   tx_buf_size)
{
    CPU_INT16U  query;
    CPU_INT16U  nbytes;
    CPU_INT32U  version;


    query  = 0;
    nbytes = 0;

                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size =  2 (= Rx header size)                */
                                                                /*               +  2 (= Query         ).               */
    if (rx_pkt_size != 4) {
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
    }

                                                                /* ------------------- HANDLE QUERY ------------------- */
    query = ProbeCom_GetINT16U(&prx_buf);                       /* Read the query.                                      */

    switch (query) {
        case PROBE_COM_QUERY_MAX_RX_SIZE:
        case PROBE_COM_QUERY_MAX_TX_SIZE:
        case PROBE_COM_QUERY_FMT_SUPPORT:
        case PROBE_COM_QUERY_ENDIANNESS_TEST:
             break;

        default:
             return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_QUERY_NOT_SUPPORTED));
    }

                                                                /* Store TX pkt header :                                */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_QUERY);       /*  (a) TX pkt format.                                  */
    ProbeCom_StoINT8U( &ptx_buf, PROBE_COM_STATUS_OK);          /*  (b) Target status.                                  */
    ProbeCom_StoINT8U( &ptx_buf, ProbeCom_PktModifier());       /*  (c) Modifier.                                       */

                                                                /* Store TX data segment data.                          */
    switch (query) {

        case PROBE_COM_QUERY_MAX_RX_SIZE:
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_RX_MAX_SIZE);
             ProbeCom_StoINT16U(&ptx_buf, 0);
             nbytes  = 4;
             break;

        case PROBE_COM_QUERY_MAX_TX_SIZE:
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_TX_MAX_SIZE);
             ProbeCom_StoINT16U(&ptx_buf, 0);
             nbytes  = 4;
             break;

        case PROBE_COM_QUERY_FMT_SUPPORT:
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_QUERY      );
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_INFO       );
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_SIMPLE_RD  );
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_MULTIPLE_RD);
             nbytes  = 8;
#if (PROBE_COM_SUPPORT_WR == DEF_TRUE)
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_SIMPLE_WR  );
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_MULTIPLE_WR);
             nbytes += 4;
#endif
#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_STR_RD);
             ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_RX_STR_WR);
             nbytes += 4;
#endif
             break;

        case PROBE_COM_QUERY_ENDIANNESS_TEST:
             ProbeCom_EndiannessTest = 0x12345678;
             Mem_Copy((void     *) ptx_buf,
                      (void     *)&ProbeCom_EndiannessTest,
                      (CPU_SIZE_T) 4);
             nbytes  = 4;
             break;


        case PROBE_COM_QUERY_VERSION:
             version = PROBE_COM_VERSION;
             Mem_Copy((void     *) ptx_buf,
                      (void     *)&version,
                      (CPU_SIZE_T) 4);
             nbytes  = 4;
             break;

        default:
             nbytes  = 0;
             break;
    }

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    return ((CPU_INT16U)(nbytes + PROBE_COM_SIZE_TX_HDR));      /* TX pkt size = nbytes (= Tx data   size)              */
                                                                /*             + 4      (= Tx header size).             */
}


/*
*********************************************************************************************************
*                                           ProbeCom_CmdInfo()
*
* Description : Parse the FMT_INFO command & formulate response.  This command supplies information
*               to the target about the Windows program's capabilitied & configuration.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte info  , indicating the type of the info packet;
*                   (C) A 4-byte data  , the packet data.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |          Info           |
*                         +-------------------------+-------------------------+
*                         |                       Data                        |
*                         +---------------------------------------------------+
*
*              (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*                         |                       Answer                      |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*********************************************************************************************************
*/

static  CPU_INT16U  ProbeCom_CmdInfo (CPU_INT08U  *prx_buf,
                                      CPU_INT08U  *ptx_buf,
                                      CPU_INT16U   rx_pkt_size,
                                      CPU_INT16U   tx_buf_size)
{
    CPU_INT16U                 info;
    CPU_INT32U                 data;
    PROBE_COM_INFO_HDNLR_FNCT  hndlr;
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR                     cpu_sr;
#endif


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx header size)                 */
                                                                /*               + 2 (= Info          )                 */
                                                                /*               + 4 (= Data          ).                */
    if (rx_pkt_size != 8) {
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
    }
                                                                /* ------------------- HANDLE INFO -------------------- */
    CPU_CRITICAL_ENTER();
    hndlr = ProbeCom_InfoHndlr;
    CPU_CRITICAL_EXIT();
    if (hndlr != (PROBE_COM_INFO_HDNLR_FNCT)0) {                /* Invoke callback if callback is set.                  */
        info = ProbeCom_GetINT16U(&prx_buf);
        data = ProbeCom_GetINT32U(&prx_buf);
        hndlr(info, data);
    }

                                                                /* Store TX pkt hdr :                                   */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_INFO);        /*  (a) TX pkt format.                                  */
    ProbeCom_StoINT8U( &ptx_buf, PROBE_COM_STATUS_OK);          /*  (b) Target status.                                  */
    ProbeCom_StoINT8U( &ptx_buf, ProbeCom_PktModifier());       /*  (c) Modifier.                                       */

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    return ((CPU_INT16U)(PROBE_COM_SIZE_TX_HDR));               /* Tx pkt size = 4 (= Tx header size).                  */
}


/*
*********************************************************************************************************
*                                         ProbeCom_CmdSimpleRd()
*
* Description : Parse the FMT_SIMPLE_RD request & formulate response.  This command causes the target
*               to send data read from its memory for a certain {memory address, data length} pair (which
*               is given in the request).
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format , indicating the data segment format;
*                   (B) A 2-byte length , indicating the number of bytes to read;
*                   (C) A 4-byte address, the starting address of the data to read.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |     Number of bytes     |
*                         +-------------------------+-------------------------+
*                         |                      Address                      |
*                         +-------------------------+-------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier;
*                   (D) The memory data.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*                         |                        Data                       |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*********************************************************************************************************
*/

static  CPU_INT16U  ProbeCom_CmdSimpleRd (CPU_INT08U  *prx_buf,
                                          CPU_INT08U  *ptx_buf,
                                          CPU_INT16U   rx_pkt_size,
                                          CPU_INT16U   tx_buf_size)
{
    CPU_ADDR    addr;
    CPU_INT16U  nbytes;


    addr   = 0;
    nbytes = 0;

                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx header size )                */
                                                                /*               + 2 (= Number of bytes)                */
                                                                /*               + 4 (= Address        ).               */
    if (rx_pkt_size != 8) {
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
    }


                                                                /* -------------------- HANDLE RD --------------------- */
    nbytes = ProbeCom_GetINT16U(&prx_buf);                      /* Get nbr of bytes to read.                            */

                                                                /* Get read addr.                                       */
#if ((!defined(CPU_CFG_ADDR_SIZE)) || \
     ((defined(CPU_CFG_ADDR_SIZE)) && \
              (CPU_CFG_ADDR_SIZE   != CPU_WORD_SIZE_16)))
    addr   = (CPU_ADDR)ProbeCom_GetINT32U(&prx_buf);
#else
    addr   = (CPU_ADDR)ProbeCom_GetINT16U(&prx_buf);
#endif

    if (nbytes + PROBE_COM_SIZE_TX_HDR > tx_buf_size) {         /* If TX pkt will NOT fit in buf, rtn err.            */
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_TX_PKT_TOO_LARGE));
    }

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    ProbeCom_TxSymCtr++;                                        /* Increment transmit symbol counter.                   */
    ProbeCom_TxSymByteCtr += nbytes;
#endif

                                                                /* Store TX pkt hdr :                                   */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_SIMPLE_RD);   /*  (a) TX pkt format.                                  */
    ProbeCom_StoINT8U( &ptx_buf, PROBE_COM_STATUS_OK);          /*  (b) Target status.                                  */
    ProbeCom_StoINT8U( &ptx_buf, ProbeCom_PktModifier());       /*  (c) Modifier.                                       */

    Mem_Copy((void     *)ptx_buf,                               /* Save TX data segment data.                           */
             (void     *)addr,
             (CPU_SIZE_T)nbytes);

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    return ((CPU_INT16U)(nbytes + PROBE_COM_SIZE_TX_HDR));      /* TX pkt size = nbytes (= Tx data   size)              */
                                                                /*             + 4      (= Tx header size).             */
}


/*
*********************************************************************************************************
*                                        ProbeCom_CmdMultipleRd()
*
* Description : Parse the FMT_MULTIPLE_RD request & formulate respnse.  This command causes the target
*               to send data read from its memory for a certain set of {memory address, data length}
*               pairs (which are given in the request).
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format,          indicating the data segment format;
*                   (B) A 5-byte item descriptor, for each item in the list, consisting of:
*
*                       (1) A 4-byte address, the starting address of the data to read;
*                       (2) A 1-byte length,  indicating the number of bytes to read.
*
*                         +-------------------------+------------+------------+
*                         |          Format         | Num. bytes |         Addr       ---
*                         +-------------------------+------------+------------+        |    Item 1
*                         ress                                   | Num. bytes |       ---
*                         +--------------------------------------+------------+        |
*                         |                      Address                      |        |    Item 2
*                         +------------+--------------------------------------+       ---
*                         | Num. bytes |                                   Addr        |    Item 3
*                         +------------+--------------------------------------+        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         +--------------------------------------+------------+        .
*                         ress                                   | Num. bytes |       ---
*                         ---------------------------------------+------------+        |   Item n
*                         |                      Address                      |        |
*                         +---------------------------------------------------+       ---
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier;
*                   (D) For each item, the following is sent:
*
*                       (1) The memory data.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+       ---
*                         |                        Data                       |        |    Item 1
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         +---------------------------------------------------+       ---
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         +---------------------------------------------------+       ---
*                         |                        Data                       |        |    Item n
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         +---------------------------------------------------+       ---
*
*********************************************************************************************************
*/

static  CPU_INT16U  ProbeCom_CmdMultipleRd (CPU_INT08U  *prx_buf,
                                            CPU_INT08U  *ptx_buf,
                                            CPU_INT16U   rx_pkt_size,
                                            CPU_INT16U   tx_buf_size)
{
    CPU_INT08U  *ptx_buf_start;
    CPU_INT16U   tx_len;

    CPU_ADDR     addr;
    CPU_INT16U   nbytes;
    CPU_INT16U   rx_pkt_ix;

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    CPU_INT16U   sym_ctr;
    CPU_INT16U   sym_byte_ctr;
#endif


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size >= 2 (= Rx header size )               */
                                                                /*                + 5 (= 1 item descriptor).            */
    if (rx_pkt_size  < 7) {
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
    }

                                                                /* -------------------- HANDLE RD --------------------- */
    ptx_buf_start = ptx_buf;                                    /* Save ptr to TX buf in case pkt is too long for buf.  */
    tx_len        = PROBE_COM_SIZE_TX_HDR;                      /* Initial TX pkt len = 4 = size of hdr.                */

                                                                /* Store TX pkt hdr :                                   */
    ptx_buf[0]    = PROBE_COM_FMT_TX_MULTIPLE_RD_LO;            /* (a) TX pkt format.                                   */
    ptx_buf[1]    = PROBE_COM_FMT_TX_MULTIPLE_RD_HI;
    ptx_buf[2]    = PROBE_COM_STATUS_OK;                        /* (b) Target status.                                   */
    ptx_buf[3]    = ProbeCom_PktModifier();                     /* (c) Modifier.                                        */
    ptx_buf      += PROBE_COM_SIZE_TX_HDR;

    rx_pkt_ix     = 7;                                          /* Recv pkt ix after 1st item = 2 (= Rx header size   ) */
                                                                /*                            + 5 (= 1 item descriptor).*/

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    sym_ctr      = 0;
    sym_byte_ctr = 0;
#endif
                                                                /* Store data for each item.                            */
    while (rx_pkt_ix <= rx_pkt_size) {
        nbytes     =  prx_buf[0];                               /* (a) Get nbr of bytes to read.                        */

                                                                /* (b) Get read addr.                                   */
#if ((!defined(CPU_CFG_ADDR_SIZE)) || \
     ((defined(CPU_CFG_ADDR_SIZE)) && \
              (CPU_CFG_ADDR_SIZE   != CPU_WORD_SIZE_16)))
        addr       = (prx_buf[4] << 8) + prx_buf[3];
        addr       = (addr       << 8) + prx_buf[2];
        addr       = (addr       << 8) + prx_buf[1];
#else
        addr       = (prx_buf[2] << 8) + prx_buf[1];
#endif

        prx_buf   += 5;
        rx_pkt_ix += 5;

        tx_len    += nbytes;                                    /* (c) Add nbr of bytes to pkt len.                     */

        if (tx_len > tx_buf_size) {                             /* (d) Will pkt be too long for TX buf? ...             */
            ptx_buf = ptx_buf_start;                            /*     ... rtn err.                                     */
            return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_TX_PKT_TOO_LARGE));
        }

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
        sym_ctr++;                                              /* (e) Inc local sym ctr.                               */
        sym_byte_ctr += nbytes;
#endif
                                                                /* (f) Otherwise, save TX data.                         */
        Mem_Copy((void     *)ptx_buf,
                 (void     *)addr,
                 (CPU_SIZE_T)nbytes);

        ptx_buf += nbytes;
    }

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    ProbeCom_TxSymCtr     += sym_ctr;                           /* Inc global sym ctr.                                  */
    ProbeCom_TxSymByteCtr += sym_byte_ctr;
#endif

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    return (tx_len);
}


/*
*********************************************************************************************************
*                                         ProbeCom_CmdSimpleWr()
*
* Description : Parse the FMT_SIMPLE_WR request & formulate response.  This command causes the target
*               to write certain data into its memroy, for a certain {memory address, data length, data}
*               triplet (which is given in the request).
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format,  indicating the data segment format;
*                   (B) A 2-byte length,  indicating the number of bytes to write;
*                   (C) A 4-byte address, the starting address at which the data should be written;
*                   (D) The memory data.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |      Number of bytes    |
*                         +-------------------------+-------------------------+
*                         |                      Address                      |
*                         +---------------------------------------------------+
*                         |                        Data                       |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_WR == DEF_TRUE)
static  CPU_INT16U  ProbeCom_CmdSimpleWr (CPU_INT08U  *prx_buf,
                                          CPU_INT08U  *ptx_buf,
                                          CPU_INT16U   rx_pkt_size,
                                          CPU_INT16U   tx_buf_size)
{
    CPU_INT16U  nbytes;
    CPU_ADDR    addr;
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr;
#endif


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size >= 2      (= Rx header size )          */
                                                                /*                + 2      (= Number of bytes)          */
                                                                /*                + 4      (= Address        )          */
                                                                /*                + nbytes (= Data           ).         */
    if (rx_pkt_size < 8) {
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
    }

                                                                /* -------------------- HANDLE WR --------------------- */
    nbytes = ProbeCom_GetINT16U(&prx_buf);                      /* Get nbr of bytes to write.                           */

                                                                /* Get write addr.                                      */
#if ((!defined(CPU_CFG_ADDR_SIZE)) || \
     ((defined(CPU_CFG_ADDR_SIZE)) && \
              (CPU_CFG_ADDR_SIZE   != CPU_WORD_SIZE_16)))
    addr   = (CPU_ADDR)ProbeCom_GetINT32U(&prx_buf);
#else
    addr   = (CPU_ADDR)ProbeCom_GetINT16U(&prx_buf);
#endif

    if (rx_pkt_size != (8 + nbytes)) {                          /* If RX data segment is NOT expected size ... rtn err. */
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
    }

                                                                /* Copy data into memory.                               */
    CPU_CRITICAL_ENTER();
    Mem_Copy((void     *)addr,
             (void     *)prx_buf,
             (CPU_SIZE_T)nbytes);
    CPU_CRITICAL_EXIT();

                                                                /* Store TX pkt hdr :                                   */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_SIMPLE_WR);   /* (a) TX pkt format.                                   */
    ProbeCom_StoINT8U( &ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT8U( &ptx_buf, ProbeCom_PktModifier());       /* (c) Modifier.                                        */

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    return ((CPU_INT16U)(PROBE_COM_SIZE_TX_HDR));               /* Tx pkt size = 4 (= Tx header size).                  */
}
#endif


/*
*********************************************************************************************************
*                                        ProbeCom_CmdMultipleWr()
*
* Description : Parse the FMT_MULTIPLE_WR request & formulate response.  This command causes the target
*               to write certain data into its memroy, for a certain number of {memory address, data
*               length, data} triplets (which are given in the request).
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format,  indicating the data segment format;
*                   (B) A n-byte item descriptor, for each item in the list, consiting of the following:
*
*                       (1) A 1-byte length,  indicating the number of bytes to write;
*                       (2) A 4-byte address, the starting address at which the data should be written;
*                       (3) The memory data.
*
*                         +-------------------------+------------+------------+
*                         |          Format         | Num. bytes |         Addr       ---
*                         +-------------------------+------------+------------+        |
*                         ress                                   |            |        |
*                         +--------------------------------------+            |        |    Item 1
*                         |                       Data                        |        |
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         +------------+--------------------------------------+       ---
*                         | Num. bytes |                                   Addr        |    Item 2
*                         +------------+--------------------------------------+        .
*                         |                         .                         |        .
*                         |                         .                         |        .
*                         |                         .            +------------+        .
*                         |                         .            | Num. bytes |       ---
*                         ---------------------------------------+------------+        |
*                         |                      Address                      |        |
*                         +---------------------------------------------------+        |    Item n
*                         |                       Data                        |        |
*                         |                         .                         |        |
*                         |                         .                         |        |
*                         +------------+--------------------------------------+       ---
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_WR == DEF_TRUE)
static  CPU_INT16U  ProbeCom_CmdMultipleWr (CPU_INT08U  *prx_buf,
                                            CPU_INT08U  *ptx_buf,
                                            CPU_INT16U   rx_pkt_size,
                                            CPU_INT16U   tx_buf_size)
{
    CPU_INT16U  nbytes;
    CPU_ADDR    addr;
    CPU_INT16U  rx_pkt_ix;
#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    CPU_INT16U  sym_ctr;
    CPU_INT16U  sym_byte_ctr;
#endif
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR      cpu_sr;
#endif

                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size >= 2      (= Rx header size )          */
                                                                /*                + 1      (= Number of bytes)          */
                                                                /*                + 4      (= Address        )          */
                                                                /*                + nbytes (= Data           ).         */
    if (rx_pkt_size < 7) {
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
    }

                                                                /* -------------------- HANDLE WR --------------------- */
    rx_pkt_ix  = 7;                                             /* Recv pkt ix after 1st item = 2 (= Rx header size   ) */
                                                                /*                            + 5 (= 1 item descriptor).*/

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    sym_ctr      = 0;
    sym_byte_ctr = 0;
#endif
                                                                /* Store data for each item.                            */
    while (rx_pkt_ix <= rx_pkt_size) {
        nbytes  =  prx_buf[0];                                  /* (a) Get nbr of bytes to write.                       */

                                                                /* (b) Get write addr.                                  */
#if ((!defined(CPU_CFG_ADDR_SIZE)) || \
     ((defined(CPU_CFG_ADDR_SIZE)) && \
              (CPU_CFG_ADDR_SIZE   != CPU_WORD_SIZE_16)))
        addr     = (prx_buf[4] << 8) + prx_buf[3];
        addr     = (addr       << 8) + prx_buf[2];
        addr     = (addr       << 8) + prx_buf[1];
#else
        addr     = (prx_buf[2] << 8) + prx_buf[1];
#endif

        prx_buf += 5;

        if (rx_pkt_ix + nbytes > rx_pkt_size) {
            return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
        }

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
        sym_ctr++;                                              /* (c) Incr local sym ctr.                              */
        sym_byte_ctr += nbytes;
#endif
                                                                /* (d) Store data into pkt.                             */
        CPU_CRITICAL_ENTER();
        Mem_Copy((void     *)addr,
                 (void     *)prx_buf,
                 (CPU_SIZE_T)nbytes);
        CPU_CRITICAL_EXIT();

        prx_buf   += nbytes;
        rx_pkt_ix += 5 + nbytes;
    }

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    ProbeCom_RxSymCtr     += sym_ctr;                           /* Inc global sym ctr.                                  */
    ProbeCom_RxSymByteCtr += sym_byte_ctr;
#endif

                                                                /* Store TX pkt hdr :                                   */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_MULTIPLE_WR); /* (a) TX pkt format.                                   */
    ProbeCom_StoINT8U( &ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT8U( &ptx_buf, ProbeCom_PktModifier());       /* (c) Modifier.                                        */

                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    return ((CPU_INT16U)(PROBE_COM_SIZE_TX_HDR));               /* Tx pkt size = 4 (= Tx header size).                  */
}
#endif


/*
*********************************************************************************************************
*                                           ProbeCom_CmdStrRd()
*
* Description : Parse the FMT_STR_RD command & formulate response.  This command asks the target to send
*               a string that it is currently storing.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte padding.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |         Padding         |
*                         +-------------------------+-------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier;
*                   (D) A n-byte string, the string which is stored in the target's buffer.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*                         |                       String                      |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*               (3) See Notes for 'ProbeCom_TxStr()'.
*
*               (4) uC/Probe requires that a NULL byte ends the string.  Consequently, if the data
*                   section of the TX packet is n bytes long, then only (n - 1) bytes have been read from
*                   the string buffer.
*
*               (5) A NULL modifier is returned so that string transmission will not monopolize the
*                   transmission channel.  If more string is queued in the string buffer, additional
*                   string bytes will be sent following the response to the next received packet.
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
static  CPU_INT16U  ProbeCom_CmdStrRd (CPU_INT08U  *prx_buf,
                                       CPU_INT08U  *ptx_buf,
                                       CPU_INT16U   rx_pkt_size,
                                       CPU_INT16U   tx_buf_size)
{
    CPU_INT16U  wr_ix;
    CPU_INT16U  rd_ix;
    CPU_INT16U  nbytes;
    CPU_INT16U  nbytes_rd;
    CPU_INT16U  rd_ix_n;
    CPU_INT16U  tx_max;


    wr_ix  = ProbeCom_StrBufWrIx;
    rd_ix  = ProbeCom_StrBufRdIx;
    nbytes = 0;

                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx header size)                 */
                                                                /*               + 2 (= Padding       ).                */
    if (rx_pkt_size != 4) {
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
    }

                                                                /* ------------------- HANDLE STR --------------------- */
    if (wr_ix == rd_ix) {                                       /* If there is NO string ... rtn err.                   */
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_STR_NONE));
    }

                                                                /* Store TX pkt hdr :                                   */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_STR_RD);      /* (a) TX pkt format.                                   */
    ProbeCom_StoINT8U( &ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT8U( &ptx_buf, 0);                            /* (c) Modifier (see Notes #5).                         */

                                                                /* ------------------- STORE STR ---------------------- */
    tx_max = tx_buf_size - PROBE_COM_SIZE_TX_HDR - 1;           /* (a) Calculate size of largest tx string.             */

    if (wr_ix > rd_ix) {                                        /* (b) If wr_ix > rd_ix, store the bytes [rd_ix, wr_ix).*/
        nbytes_rd = wr_ix - rd_ix;                              /*     (1) Calculate nbr bytes to read.                 */
                                                                /*     (2) If too much to fit in pkt ...                */
                                                                /*     ... Store only [rd_ix,rd_ix + tx_buf_size - 5)   */
                                                                /*     ... Where 5 = 1 (NULL byte)                      */
        if (nbytes_rd >= tx_max) {                              /*                 + 4 (TX hdr   ).                     */
            nbytes_rd = tx_max;
            rd_ix_n   = rd_ix + tx_max;
        } else {
            rd_ix_n   = wr_ix;
        }

        nbytes = nbytes_rd;                                     /*     (3) Store nbr of bytes read.                     */
        Mem_Copy((void     *) ptx_buf,                          /*     (4) Read from the string buf.                    */
                 (void     *)&ProbeCom_StrBuf[rd_ix],
                 (CPU_SIZE_T) nbytes_rd);
       *(ptx_buf + nbytes)  = 0;                                /*     (5) Store NULL byte.                             */
    } else {                                                    /*  (c) If wr_ix < rd_ix ...                            */
                                                                /*  ... Store in pkt the bytes [rd_ix, end_ix) ...      */
                                                                /*  ... Then store in pkt the bytes [0, wr_ix).         */

        nbytes_rd = PROBE_COM_STR_BUF_SIZE - rd_ix;             /*      (1) Calculate nbr bytes to read.                */

        if (nbytes_rd >= tx_max) {                              /*      (2) If too much to fit in pkt ...               */
                                                                /*      ... Store only [rd_ix, rd_ix + tx_buf_size)     */
            nbytes  = tx_max;                                   /*          (A) Store nbr of bytes read.                */
            rd_ix_n = rd_ix + tx_max;
            Mem_Copy((void     *) ptx_buf,                      /*          (B) Read from string buf.                   */
                     (void     *)&ProbeCom_StrBuf[rd_ix],
                     (CPU_SIZE_T) nbytes);
           *(ptx_buf + nbytes)  = 0;                            /*          (C) Store NULL byte.                        */
        } else {                                                /*     (3) If not too much for pkt :                    */

                                                                /*           ------- [rd_ix, end_ix) ------             */

            nbytes    = nbytes_rd;                              /*          (A) Store nbr of bytes read.                */
            rd_ix_n   = 0;                                      /*          (B) Buf read ix after read.                 */
            Mem_Copy((void     *) ptx_buf,                      /*          (C) Read from the string buf.               */
                     (void     *)&ProbeCom_StrBuf[rd_ix],
                     (CPU_SIZE_T) nbytes_rd);
            rd_ix     = rd_ix_n;                                /*          (D) Update local read index.                */

                                                                /*           --------- [0, wr_ix) ---------             */
            ptx_buf   += nbytes_rd;
            nbytes_rd  = wr_ix;                                 /*          (E) Number of bytes to read.                */
            if (nbytes_rd + nbytes >= tx_max) {                 /*          (F) Too much to fit in pkt ...              */
                nbytes_rd = tx_max - nbytes;                    /*          ... Store only [0, tx_buf_size - 5].        */
            }
            nbytes  += nbytes_rd;                               /*          (G) Update total nbr bytes read.            */
            rd_ix_n  = rd_ix + nbytes_rd;                       /*          (H) Buf read ix after read.                 */
            Mem_Copy((void     *) ptx_buf,                      /*          (I) Read from the string buf.               */
                     (void     *)&ProbeCom_StrBuf[rd_ix],
                     (CPU_SIZE_T) nbytes_rd);
           *(ptx_buf + nbytes_rd) = 0;                           /*          (J) Store NULL byte.                        */
        }
    }

    ProbeCom_StrBufRdIx = rd_ix_n;                              /*  (d) Update global read ix.                          */

#if (PROBE_COM_STAT_EN == DEF_ENABLED)
    ProbeCom_TxStrCtr += nbytes;
#endif
                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    return ((CPU_INT16U)(PROBE_COM_SIZE_TX_HDR + nbytes + 1));  /* Tx pkt size = 4      (= Tx header size)              */
                                                                /*             + nbytes (= String    size)              */
                                                                /*             + 1      (= NULL byte     ).             */
}
#endif


/*
*********************************************************************************************************
*                                           ProbeCom_CmdStrWr()
*
* Description : Parse the FMT_STR_WR command & formulate response.  This command sends a string to the
*               target.
*
* Argument(s) : prx_buf         Pointer to the receive  buffer
*
*               ptx_buf         Pointer to the transmit buffer
*
*               rx_pkt_size     Size of the receive  packet
*
*               tx_buf_size     Size of the transmit buffer
*
* Return(s)   : The number of bytes written to the tx buffer.
*
* Caller(s)   : ProbeCom_ParseRxPkt().
*
* Note(s)     : (1) The RX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 2-byte padding;
*                   (C) A n-byte string.
*
*                         +-------------------------+-------------------------+
*                         |          Format         |         Padding         |
*                         +-------------------------+-------------------------+
*                         |                       String                      |
*                         |                         .                         |
*                         |                         .                         |
*                         |                         .                         |
*                         +---------------------------------------------------+
*
*               (2) The TX format:
*
*                   (A) A 2-byte format, indicating the data segment format;
*                   (B) A 1-byte status, indicating the status after the request;
*                   (C) A 1-byte modifier.
*
*                         +-------------------------+------------+------------+
*                         |          Format         |   Status   |  Modifier  |
*                         +-------------------------+------------+------------+
*
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
static  CPU_INT16U  ProbeCom_CmdStrWr (CPU_INT08U  *prx_buf,
                                       CPU_INT08U  *ptx_buf,
                                       CPU_INT16U   rx_pkt_size,
                                       CPU_INT16U   tx_buf_size)
{
    PROBE_COM_STR_HDNLR_FNCT   hndlr;
    CPU_CHAR                  *pstr;
    CPU_INT16U                 len;
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR                     cpu_sr;
#endif


                                                                /* ------------------- CHK PKT SIZE ------------------- */
                                                                /* Expected size = 2 (= Rx header size)                 */
                                                                /*               + 2 (= Padding       ).                */
    if (rx_pkt_size <= 4) {
        return (ProbeCom_CmdErr(ptx_buf, PROBE_COM_STATUS_RX_PKT_WRONG_SIZE));
    }



                                                                /* ------------------- HANDLE STR --------------------- */
    CPU_CRITICAL_ENTER();
    hndlr = ProbeCom_StrHndlr;
    CPU_CRITICAL_EXIT();
    if (hndlr != (PROBE_COM_STR_HDNLR_FNCT)0) {                 /* Invoke callback if callback is set.                  */
        pstr = (CPU_CHAR *)(prx_buf     + 4);
        len  = (CPU_INT16U)(rx_pkt_size - 4);
        hndlr(pstr, len);
    }



                                                                /* ------------------ STORE RESPONSE ------------------ */
                                                                /* Store TX pkt hdr :                                   */
    ProbeCom_StoINT16U(&ptx_buf, PROBE_COM_FMT_TX_STR_WR);      /* (a) TX pkt format.                                   */
    ProbeCom_StoINT8U( &ptx_buf, PROBE_COM_STATUS_OK);          /* (b) Target status.                                   */
    ProbeCom_StoINT8U( &ptx_buf, ProbeCom_PktModifier());       /* (c) Modifier.                                        */



                                                                /* ------------------ RTN TX PKT SIZE ----------------- */
    return ((CPU_INT16U)(PROBE_COM_SIZE_TX_HDR));               /* Tx pkt size = 4 (= Tx header size)                   */
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
**                                    STATIC BUFFER FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       ProbeCom_GetINT8U()
*                                       ProbeCom_GetINT16U()
*                                       ProbeCom_GetINT32U()
*
* Description : Retrieve data from a buffer and increment buffer pointer.
*
* Argument(s) : pbuf            The pointer to the buffer pointer.
*
* Return(s)   : A 8-, 16-, or 32-bit datum, respectively.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_INT08U  ProbeCom_GetINT8U (CPU_INT08U **pbuf)
{
    return (*((*pbuf)++));
}


static  CPU_INT16U  ProbeCom_GetINT16U (CPU_INT08U **pbuf)
{
    CPU_INT16U  lowbyte;
    CPU_INT16U  highbyte;


    lowbyte  = ProbeCom_GetINT8U(pbuf);
    highbyte = ProbeCom_GetINT8U(pbuf);
    return ((CPU_INT16U)((CPU_INT16U)(highbyte << 8) | (CPU_INT16U)lowbyte));
}

static  CPU_INT32U  ProbeCom_GetINT32U (CPU_INT08U **pbuf)
{
    CPU_INT32U  highword;
    CPU_INT32U  lowword;


    lowword  = ProbeCom_GetINT16U(pbuf);
    highword = ProbeCom_GetINT16U(pbuf);
    return ((CPU_INT32U)((highword << 16) | lowword));
}


/*
*********************************************************************************************************
*                                       ProbeCom_StoINT8U()
*                                       ProbeCom_StoINT16U()
*                                       ProbeCom_StoINT32U()
*
* Description : Store data into a buffer and increment the buffer pointer.
*
* Argument(s) : pbuf            The pointer to the buffer pointer.
*
*               data            A 8-, 16-, or 32-bit datum, respectively.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  ProbeCom_StoINT8U (CPU_INT08U **pbuf, CPU_INT08U data)
{
    *((*pbuf)++) = data;
}


static  void  ProbeCom_StoINT16U (CPU_INT08U **pbuf, CPU_INT16U data)
{
    ProbeCom_StoINT8U(pbuf, (CPU_INT08U)(data & 0x00FF));
    ProbeCom_StoINT8U(pbuf, (CPU_INT08U)(data >> 8));
}


#if 0
static  void  ProbeCom_StoINT32U (CPU_INT08U **pbuf, CPU_INT32U data)
{
    ProbeCom_StoINT16U(pbuf, (CPU_INT16U)(data & 0x0000FFFFL));
    ProbeCom_StoINT16U(pbuf, (CPU_INT16U)(data >> 16));
}
#endif
