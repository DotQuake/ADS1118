
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
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
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include "dwt_stm32_delay.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
uint16_t  data[3];
uint8_t   rxBuffer,startStream=0;
/*
 * readCommand Function
 * Parameters: 	config ----> CONFIG COMMAND TO SEND
 * 				discardData ---> Discard The return Data from ADS1118
 *
 * 	Basic Idea of Function:
 * 		Sends 16 bit length of config command to all three ADS1118.
 * 		At the same time, if discardData==false, stores the 3 16 bit
 * 		return data of all ADS1118 to data variable
 *
 * 		For Deeper information for why does the function works this
 * 		way is to read Datasheet or Ask MEEE :D :D
 *
 */
void readCommand(uint16_t config,uint8_t discardData)
{
	uint16_t read;
	int16_t counter=15;
	if(discardData==0)
	{
		data[0]=0x0000;
		data[1]=0x0000;
		data[2]=0x0000;
	}
	DWT_Delay_us(1);
	while(counter>=0)
	{
		read=config>>counter;
		read=read&0x0001;
		read=read==0x0001?GPIO_PIN_SET:GPIO_PIN_RESET;
		HAL_GPIO_WritePin(DOUT_GPIO_Port,DOUT_Pin,read);
		HAL_GPIO_WritePin(SCLK_GPIO_Port,SCLK_Pin,GPIO_PIN_SET);
		DWT_Delay_us(1);
		HAL_GPIO_WritePin(SCLK_GPIO_Port,SCLK_Pin,GPIO_PIN_RESET);
		if(discardData==0)
		{
			read=HAL_GPIO_ReadPin(DIN_X_GPIO_Port,DIN_X_Pin)==GPIO_PIN_SET?0x01:0x00;
			read=read<<counter;
			data[0]=data[0]|read;
			read=HAL_GPIO_ReadPin(DIN_Y_GPIO_Port,DIN_Y_Pin)==GPIO_PIN_SET?0x01:0x00;
			read=read<<counter;
			data[1]=data[1]|read;
			read=HAL_GPIO_ReadPin(DIN_Z_GPIO_Port,DIN_Z_Pin)==GPIO_PIN_SET?0x01:0x00;
			read=read<<counter--;
			data[2]=data[2]|read;
		}else{
			DWT_Delay_us(2);
			counter--;
		}
	}
	//HAL_GPIO_WritePin(CS_GPIO_Port,CS_Pin,GPIO_PIN_SET);
	return;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	/*
	 * UART RECEIVE Callback
	 * All receive byte are received in this function
	 * Read Byte will determine the active action of STM32 below
	 */
  if(rxBuffer=='P'){
	  HAL_UART_Transmit(&huart1,"Pong",4,1);
  }else if(rxBuffer=='D'){
	  startStream=startStream==0?1:0;
  }
  HAL_UART_Receive_DMA(&huart1, &rxBuffer, 1);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  //Initialize Pins
  HAL_GPIO_WritePin(READY_GPIO_Port,READY_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(CS_GPIO_Port,CS_Pin,GPIO_PIN_SET);
  HAL_GPIO_WritePin(SCLK_GPIO_Port,SCLK_Pin,GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DOUT_GPIO_Port,DOUT_Pin,GPIO_PIN_RESET);
  DWT_Delay_Init();
  HAL_Delay(1000);
  HAL_GPIO_WritePin(CS_GPIO_Port,CS_Pin,GPIO_PIN_RESET);


  /* Update Config Reg 0x04EB, refer to datasheet @ page 25 on Table 7
  	 For more information about Config Register
 	 For Data Communication Please refer to Page 24 to see
  	 How does data transfer to each other
   */
  readCommand(0x04EB,1);
  readCommand(0x04EB,1);

  /*
   * LED LIGHT ON to determine that initialization is complete
   * The color of the led is Yellowish and is found on the
   * STM32 board
   */
  HAL_GPIO_WritePin(READY_GPIO_Port,READY_Pin,GPIO_PIN_RESET);

  /*
   * Set DMA(Direct Memory Access) Receive mode for UART
   * and Callback HAL_UART_RxCpltCallback is called everytime
   * a Byte is received
   */
  HAL_UART_Receive_DMA(&huart1,&rxBuffer,1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /*
	   * Checks if the 3 ADS1118 Out Pin is low
	   * if all out pins are low start reading the three data
	   */
	  if(HAL_GPIO_ReadPin(DIN_X_GPIO_Port,DIN_X_Pin)==GPIO_PIN_RESET)
	  {
		  if(HAL_GPIO_ReadPin(DIN_Y_GPIO_Port,DIN_Y_Pin)==GPIO_PIN_RESET)
		  {
			  if(HAL_GPIO_ReadPin(DIN_Z_GPIO_Port,DIN_Z_Pin)==GPIO_PIN_RESET)
			  {
				  readCommand(0x04EB,0);//Send Command and Read The 16 bit Data
				  readCommand(0x04EB,1);//Send Command and Discard the return Data
	  	  		  if(startStream==1){
	  	  			  /*
	  	  			   * Sends The data through bluetooth
	  	  			   * Please refer to HAL_UART_RxCpltCallback method
	  	  			   * above to check how does android and stm32
	  	  			   * communicates
	  	  			   */
	  	  			  HAL_UART_Transmit(&huart1,data,6,1);
				  }
			  }
		  }
	  }

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 1382400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(READY_GPIO_Port, READY_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CS_Pin|DOUT_Pin|SCLK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : READY_Pin */
  GPIO_InitStruct.Pin = READY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(READY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_Pin DOUT_Pin SCLK_Pin */
  GPIO_InitStruct.Pin = CS_Pin|DOUT_Pin|SCLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : DIN_X_Pin */
  GPIO_InitStruct.Pin = DIN_X_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DIN_X_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DIN_Y_Pin DIN_Z_Pin */
  GPIO_InitStruct.Pin = DIN_Y_Pin|DIN_Z_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
