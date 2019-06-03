/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ws2812b.h"
#include "nRF24L01.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRANSMITTER_MODE 0x00
#define RECEIVER_MODE 0x01
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t * payload;
uint8_t mode = RECEIVER_MODE;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == RF_IRQ_Pin && mode == RECEIVER_MODE)
	{
	HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);
	rf_clear_interrupt_flags();
	read_payload();

		for(int i =0; i < DIODES_NUMBER; i++)
		{
			set_diode_color(i, payload[1],payload[2], payload[3]);
		}
		send_data_to_spi();
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_SPI1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
	init_spi(&hspi1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);
	uint8_t * payload = get_payload();
	rf_initialize(&hspi2, GPIOB, GPIO_PIN_12);
	
	uint8_t address[5] = {0x0A,0x0A,0x0A,0x0A,0x0A};
	rf_initialize_address(address,sizeof(address));

	if(mode == TRANSMITTER_MODE)
	{
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		configure_as_transmitter();
  while (1)
  {
		HAL_Delay(500);
		rf_clear_interrupt_flags();
		payload[1] = 0xff; payload[2] = 0x00; payload[3] = 0x00;
		write_payload();
		
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);
		
	  HAL_Delay(500);
		rf_clear_interrupt_flags();
		payload[1] = 0x00; payload[2] = 0xff; payload[3] = 0x00;
		write_payload();
		
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);
		
		HAL_Delay(500);
		rf_clear_interrupt_flags();
		payload[1] = 0x00; payload[2] = 0x00; payload[3] = 0xff;
		write_payload();
		
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);
	}
}

else
	{
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		configure_as_receiver();
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
	rf_clear_interrupt_flags();
	read_payload();
	HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);
  while (1)
  {
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
	  HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);
	rf_clear_interrupt_flags();
	if(status() & (1<<6))
	read_payload();

	for(int i =0; i < DIODES_NUMBER; i++)
	{
		set_diode_color(i, payload[1],payload[2], payload[3]);
	}
	send_data_to_spi();
  }
}
	
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
