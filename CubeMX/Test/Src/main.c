/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define BUFFSIZE	512
#define MEMSIZE		1024
#define ENTER	'\r'

struct uart_circular_queue {  
    uint8_t *rear;
    uint8_t *front;
    uint8_t buffer[BUFFSIZE];  
} uart_rcv;

unsigned int Memory[MEMSIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void ptrInc(uint8_t **ptr, uint8_t* base, int len) {
    *ptr += 1;
    if (*ptr >= base + len)
        *ptr = base;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {  
    uint8_t ret = HAL_OK;


    char c = *uart_rcv.rear;
		putchar(c);
    if (c == '\r') putchar(c);
	

    ptrInc(&uart_rcv.rear, uart_rcv.buffer, BUFFSIZE);
    if (uart_rcv.rear == uart_rcv.front)
      ptrInc(&uart_rcv.front, uart_rcv.buffer, BUFFSIZE);

    do{  
        ret = HAL_UART_Receive_IT(UartHandle, uart_rcv.rear, 1);  
    } while(ret != HAL_OK);

    // while (HAL_UART_Receive_IT(UartHandle, uart_rcv.rear, 1) != HAL_OK){
    // }
}


int8_t uart_readline(uint8_t *buf, uint16_t maxlen) {  
    int count = 0;

    while(count < maxlen - 1) {
        if(uart_rcv.front != uart_rcv.rear){
						*buf = *uart_rcv.front;
            ptrInc(&uart_rcv.front, uart_rcv.buffer, BUFFSIZE);

            if (*buf == ENTER) break;
            buf++;
            count++;
        }
    }
		
    *buf = '\0';
    return count;  
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
	printf("Hello STM32 !!!\n\r");
	
  uart_rcv.front = uart_rcv.rear = uart_rcv.buffer;

  HAL_UART_Receive_IT(&huart1, (uint8_t*)uart_rcv.rear, 1);
	
	char str[100];
	sprintf(str, "Memory Addr: %p Len: %d\r\n", Memory, 1024);
	printf(str);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
	  int count = 0;
		char str[100];
    char s[100];
		char cmd[100];
		
    printf("STM32 > ");
    count = uart_readline((uint8_t*)str, 100);
    count = sprintf(s, "\r\nReadin: %d -%s-", count, str);
    printf("%s\n\r", s);
		
		
    sscanf(str, "%s", cmd);
    if (strcmp(cmd, "PEEK") == 0){
			int addr = 0, readAns;
 
      readAns = sscanf(str + 5, "%x %s", &addr, s);
			if (readAns == 1){
				sprintf(s, "PEEK: %x %x\r\n", addr, *((int*)addr));
				printf(s);
      }else{
				printf("ERROR");
      }
		}else if(strcmp(cmd, "POKE") == 0){
			int addr = 0, data = 0, readAns;
           
			readAns = sscanf(str + 5, "%x %x %s", &addr, &data, s);
			if (readAns == 2){
				*((int*)addr) = data;
				sprintf(s, "POKE: %x %x\r\n", addr, *((int*)addr));
        printf(s);
      }else{
				printf("ERROR");
      }
		}
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);

}

/** Pinout Configuration
*/
void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f) {
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}


/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
