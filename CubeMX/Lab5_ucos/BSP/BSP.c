#include <bsp.h>
CPU_INT32U  BSP_CPU_ClkFreq (void) {
    RCC_ClocksTypeDef  rcc_clocks;
    RCC_GetClocksFreq(&rcc_clocks);
    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}
INT32U  OS_CPU_SysTickClkFreq (void) {
    INT32U  freq;
    freq = BSP_CPU_ClkFreq();
    return (freq);
}