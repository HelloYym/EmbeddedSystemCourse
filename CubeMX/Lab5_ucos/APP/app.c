#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "ucos_ii.h"
#include "dht11.h"


void SysTick_Handler(void)
{
		OS_CPU_SR cpu_sr;
		OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR */
		OSIntNesting++;
		OS_EXIT_CRITICAL();
		OSTimeTick(); /* Call uC/OS-II's OSTimeTick() */
		OSIntExit();
}

void GPIO_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_DeInit();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13
                                | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11
                                | GPIO_Pin_12;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0
                                | GPIO_Pin_1
                                | GPIO_Pin_2
                                | GPIO_Pin_3
                                | GPIO_Pin_4
                                | GPIO_Pin_5
                                | GPIO_Pin_6
                                | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void Delay_ms(int times){
    OSTimeDly(OS_TICKS_PER_SEC / 1000 * times);
}

void digit_select(int index){
    BitAction v[4];
    int i;
    for (i=0; i<4; i++){
        if (index == i){
            v[i] = Bit_SET;
        }else{
            v[i] = Bit_RESET;
        }
    }
    GPIO_WriteBit(GPIOA, GPIO_Pin_11, v[0]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_12, v[1]);
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, v[2]);
    GPIO_WriteBit(GPIOC, GPIO_Pin_14, v[3]);
}
void digit_show(int dight, int point){
    int segment, i, base;
    BitAction v[8];
    switch (dight){
        case    0    :    segment = 0xee; break; // 0b11101110  0 -> 7
        case    1    :    segment = 0x24; break; // 0b00100100
        case    2    :    segment = 0xba; break; // 0b10111010
        case    3    :    segment = 0xb6; break; // 0b10110110
        case    4    :    segment = 0x74; break; // 0b01110100
        case    5    :    segment = 0xd6; break; // 0b11010110
        case    6    :    segment = 0xde; break; // 0b11011110
        case    7    :    segment = 0xa4; break; // 0b10100100
        case    8    :    segment = 0xfe; break; // 0b11111110
        case    9    :    segment = 0xf6; break; // 0b11110110
        default      :    segment = 0xda; break; // 0b11011010 error state
    }
    segment |= point != 0; // ???????
    base = 1 << 8;
    for (i=0; i<8; i++){
        base >>= 1;
        // segment????1????,?????????
        if ((segment & base )== 0){
            v[i] = Bit_SET;
        }else{
            v[i] = Bit_RESET;
        }
    }
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, v[0]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, v[1]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, v[2]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, v[3]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, v[4]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_5, v[5]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_6, v[6]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_7, v[7]);
}

void led_show(int digit){
    static int index = -1;
    int i;
    int base = 1000;
    index = (index + 1) % 4;
    for (i=0; i<index; i++){
        base /= 10;
    }
    digit = (digit / base) % 10;
    digit_select(index);
    digit_show(digit, 0);
}

int ledValue = 0; 
OS_CPU_SR cpu_sr=0;

void LED0_task(void* pdata){
		int state;
		int cnt = 0;
    while (1){
				OS_ENTER_CRITICAL();
				state = read_dht11();
				OS_EXIT_CRITICAL(); 
			
				switch (state) {
					case DHTLIB_OK:
						if (cnt)
							ledValue = get_temperature();
						else
							ledValue = get_humidity();
						break;
					case DHTLIB_ERROR_CHECKSUM:
						ledValue = 9998;
						break;
					case DHTLIB_ERROR_TIMEOUT:
						ledValue = 9999;
						break;
				}
				
				cnt = 1 - cnt;
        Delay_ms(5000);
    }
}

void LED1_task(void* pdata){
    while(1){
        led_show(ledValue);
        Delay_ms(3);
    }
}

#define STK_Size 100
int LED0_Task_STK[STK_Size];
int LED1_Task_STK[STK_Size];
int Task_STK[STK_Size];


int main()
{
    GPIO_Configuration();
    OSInit();
    OS_CPU_SysTickInit();
		OSTaskCreate(LED0_task, (void *)0, (OS_STK *)&LED0_Task_STK[STK_Size-1], 1);
    OSTaskCreate(LED1_task, (void *)0, (OS_STK *)&LED1_Task_STK[STK_Size-1], 2);
    OSStart();
    return 0;
}