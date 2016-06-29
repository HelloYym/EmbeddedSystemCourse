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
*                                                uC/Probe
*
*                                         Communication: Generic
*
* Filename      : probe_com.h
* Version       : V2.00
* Programmer(s) : BAN
* Note(s)       : (1) This file contains code to respond to generic (non protocol-dependent)
*                     commands received by the target.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               PROBE_COM present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  PROBE_COM_PRESENT                                      /* See Note #1.                                         */
#define  PROBE_COM_PRESENT


/*
*********************************************************************************************************
*                                PROBE COMMUNICATION MODULE VERSION NUMBER
*
* Note(s) : (1) (a) The Probe communication module software version is denoted as follows :
*
*                       Vx.yy
*
*                           where
*                                  V     denotes 'Version' label
*                                  x     denotes major software version revision number
*                                  yy    denotes minor software version revision number
*
*               (b) The Probe communication module software version label #define is formatted as follows :
*
*                       ver = x.yy * 100
*
*                           where
*                                  ver   denotes software version number scaled as
*                                        an integer value
*                                  x.yy  denotes software version number
*********************************************************************************************************
*/

#define  PROBE_COM_VERSION         200u                         /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef    PROBE_COM_MODULE
#define   PROBE_COM_EXT
#else
#define   PROBE_COM_EXT  extern
#endif


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <cpu.h>

#include  <lib_def.h>
#include  <lib_mem.h>

#include  <probe_com_cfg.h>

#if (PROBE_COM_SUPPORT_STR == DEF_ENABLED)
#include  <lib_str.h>
#endif


/*
*********************************************************************************************************
*                                                 DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               DATA TYPES
*********************************************************************************************************
*/

typedef  void  (*PROBE_COM_INFO_HDNLR_FNCT) (CPU_INT16U   info,
                                             CPU_INT32U   data);

typedef  void  (*PROBE_COM_STR_HDNLR_FNCT)  (CPU_CHAR    *pstr,
                                             CPU_INT16U   len);

/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

                                                                /* ------------------- COUNTERS ----------------------- */
#if (PROBE_COM_STAT_EN == DEF_ENABLED)
PROBE_COM_EXT  CPU_INT32U  ProbeCom_RxPktCtr;                   /* Number of packets received                           */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_TxPktCtr;                   /* Number of packets tranmitted                         */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_TxSymCtr;                   /* Number of symbols transmitted                        */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_TxSymByteCtr;               /* Number of symbol bytes transmitted                   */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_ErrPktCtr;                  /* Number of error packets transmitted                  */

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
PROBE_COM_EXT  CPU_INT32U  ProbeCom_TxStrCtr;                   /* Number of string characters transmitted              */
#endif

#if (PROBE_COM_SUPPORT_WR == DEF_TRUE)
PROBE_COM_EXT  CPU_INT32U  ProbeCom_RxSymCtr;                   /* Number of symbols received.                          */
PROBE_COM_EXT  CPU_INT32U  ProbeCom_RxSymByteCtr;               /* Number of symbol bytes received.                     */
#endif
#endif


/*
*********************************************************************************************************
*                                               MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         ProbeCom_Init        (void);                                   /* Init comm module.                        */

CPU_INT16U   ProbeCom_ParseRxPkt  (void                       *prx_pkt,     /* Parse rx'd pkt & fmt resp.               */
                                   void                       *ptx_pkt,
                                   CPU_INT16U                  rx_pkt_size,
                                   CPU_INT16U                  tx_buf_size);

void         ProbeCom_InfoHndlrSet(PROBE_COM_INFO_HDNLR_FNCT   hndlr);      /* Set hndlr for info pkt's.                */

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
CPU_BOOLEAN  ProbeCom_TxStr       (CPU_CHAR                   *pstr,        /* Tx str.                                  */
                                   CPU_INT16U                  dly);

void         ProbeCom_StrHndlrSet (PROBE_COM_STR_HDNLR_FNCT    hndlr);      /* Set hdnlr for str wr pkt's.              */
#endif

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*                                    DEFINED IN OS's probe_com_os.c
*********************************************************************************************************
*/

#if (PROBE_COM_SUPPORT_STR == DEF_TRUE)
void         ProbeCom_OS_Init    (void);
CPU_BOOLEAN  ProbeCom_OS_Pend    (CPU_BOOLEAN                 wait);
void         ProbeCom_OS_Post    (void);
void         ProbeCom_OS_Dly     (CPU_INT16U                  dly);
#endif


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef    PROBE_COM_SUPPORT_WR
  #error  "PROBE_COM_SUPPORT_WR               not #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  DEF_TRUE   Write commands     supported]     "
  #error  "                             [     ||  DEF_FALSE  Write commands NOT supported]     "

#elif    ((PROBE_COM_SUPPORT_WR != DEF_TRUE ) && \
          (PROBE_COM_SUPPORT_WR != DEF_FALSE))
  #error  "PROBE_COM_SUPPORT_WR         illegally #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  DEF_TRUE   Write commands     supported]     "
  #error  "                             [     ||  DEF_FALSE  Write commands NOT supported]     "
#endif



#ifndef    PROBE_COM_SUPPORT_STR
  #error  "PROBE_COM_SUPPORT_STR              not #define'd in 'probe_com_cfg.h'               "
  #error  "                       [MUST be  DEF_TRUE   String commands/functions     supported]"
  #error  "                       [     ||  DEF_FALSE  String commands/functions NOT supported]"

#elif    ((PROBE_COM_SUPPORT_STR != DEF_TRUE ) && \
          (PROBE_COM_SUPPORT_STR != DEF_FALSE))
  #error  "PROBE_COM_SUPPORT_STR                      illegally #define'd in 'probe_com_cfg.h' "
  #error  "                       [MUST be  DEF_TRUE   String commands/functions     supported]"
  #error  "                       [     ||  DEF_FALSE  String commands/functions NOT supported]"

#elif     (PROBE_COM_SUPPORT_STR == DEF_TRUE)
#ifndef    PROBE_COM_STR_BUF_SIZE
  #error  "PROBE_COM_STR_BUF_SIZE             not #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  >= 32   ]                                    "
  #error  "                             [     &&  <= 65535]                                    "

#elif    ((PROBE_COM_STR_BUF_SIZE > 65535) || \
          (PROBE_COM_STR_BUF_SIZE < 32   ))
  #error  "PROBE_COM_STR_BUF_SIZE       illegally #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  >= 32   ]                                    "
  #error  "                             [     &&  <= 65535]                                    "
#endif
#endif


#ifndef    PROBE_COM_STAT_EN
  #error  "PROBE_COM_STAT_EN                  not #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  DEF_TRUE   Counters are     maintained]      "
  #error  "                             [     ||  DEF_FALSE  Counters are NOT maintained]      "

#elif    ((PROBE_COM_STAT_EN != DEF_TRUE ) && \
          (PROBE_COM_STAT_EN != DEF_FALSE))
  #error  "PROBE_COM_STAT_EN            illegally #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  DEF_TRUE   Counters are     maintained]      "
  #error  "                             [     ||  DEF_FALSE  Counters are NOT maintained]      "
#endif



#ifndef    PROBE_COM_RX_MAX_SIZE
  #error  "PROBE_COM_RX_BUF_SIZE              not #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  >= 32   ]                                    "
  #error  "                             [     &&  <= 65535]                                    "

#elif    ((PROBE_COM_RX_MAX_SIZE > 65535) || \
          (PROBE_COM_RX_MAX_SIZE < 32   ))
  #error  "PROBE_COM_RX_MAX_SIZE        illegally #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  >= 32   ]                                    "
  #error  "                             [     &&  <= 65535]                                    "
#endif




#ifndef    PROBE_COM_TX_MAX_SIZE
  #error  "PROBE_COM_TX_BUF_SIZE              not #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  >= 32   ]                                    "
  #error  "                             [     &&  <= 65535]                                    "

#elif    ((PROBE_COM_TX_MAX_SIZE > 65535) || \
          (PROBE_COM_TX_MAX_SIZE < 32   ))
  #error  "PROBE_COM_TX_MAX_SIZE        illegally #define'd in 'probe_com_cfg.h'               "
  #error  "                             [MUST be  >= 32   ]                                    "
  #error  "                             [     &&  <= 65535]                                    "

#endif



/*
*********************************************************************************************************
*                                              MODULE END
*
* Note(s) : See 'MODULE  Note #1'.
*********************************************************************************************************
*/

#endif                                                          /* End of PROBE_COM module include (see Note #1).       */
