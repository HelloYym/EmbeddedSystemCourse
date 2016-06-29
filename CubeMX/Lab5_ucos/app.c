#include "stm32f10x.h"
#include "stm32f10x_conf.h"

void GPIO_Configuration(void){
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_DeInit();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void Delay_long(int times){
    unsigned int i, j;
    for (j=0; j<times; j++){
        for (i=0; i<0x3ffff; i++){
        }
    }
}

int main() {
    GPIO_Configuration();
    while(1){
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_SET);
        Delay_long(10);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        GPIO_WriteBit(GPIOA, GPIO_Pin_11, Bit_RESET);
        Delay_long(10);
    }
}