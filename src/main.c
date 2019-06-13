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
#include "hue.h"
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
void ADC_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

struct PointRGB{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t isSet;
};

struct ColorInterpolationConfig {
	uint32_t time;
	uint32_t timeStep;
	uint8_t colorStep;
} colorInterpolation;

uint8_t calculateLightness(uint32_t reading)
{
	float calculatedValue = 0.3643f * reading - 109.2857f;
	if (calculatedValue >= 240.0f)
	{
		return 200;
	}
	else if(calculatedValue <= 0.0f)
	{
		return 0;
	}
	return (uint8_t) calculatedValue;
}

uint8_t calculateShutterPos(uint32_t reading)
{
	float calculatedValue = 0.1111111f * reading - 11.1111111f;
	if (calculatedValue >= 100.0f)
	{
		return 100;
	}
	else if(calculatedValue <= 0.0f)
	{
		return 0;
	}
	return (uint8_t) calculatedValue;
}

void set_point(struct PointRGB rgb,uint8_t R,uint8_t G,uint8_t B)
{
	rgb.r = R; rgb.g = G; rgb.b = B;
}

uint8_t * payload;
uint8_t mode = RECEIVER_MODE;
uint8_t data_ready = 0;
u_int32_t lower_limit = 100;
u_int32_t upper_limit = 400;

ADC_HandleTypeDef adc;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == RF_IRQ_Pin && mode == RECEIVER_MODE)
	{
		data_ready = 1;
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
//	hsl.H = 65;
//	hsl.S = 1.0f;
//	hsl.L = 1.0f;

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
  ADC_Config();
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
		HAL_ADC_Start(&adc);
		payload[1] = 0x00; payload[2] = 0x00;
		uint32_t delay;
  while (1)
  {
		write_tx_address();
		write_rx_address();

		uint32_t reading = HAL_ADC_GetValue(&adc);
		uint8_t color = calculateLightness(reading);
		if(color && color < 20)
			color = 20;

		payload[3] = color;
		payload[4] = color;
		payload[5] = color;

		write_payload();

		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);

		delay = isSendSuccess() ? 1000 : 1000;
		rf_clear_interrupt_flags();
		HAL_Delay(delay);


		write_tx_shutter_address();
		write_rx_shutter_address();

		reading = HAL_ADC_GetValue(&adc);
		uint8_t shutterPosition = calculateShutterPos(reading);

		payload[0] = 1;
		payload[1] = shutterPosition;

		write_payload();

		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);

		delay = isSendSuccess() ? 1000:1000;
		rf_clear_interrupt_flags();
		HAL_Delay(delay);
	}
}

else
	{
	configure_as_receiver();
	HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
	rf_clear_interrupt_flags();
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  while (1)
  {
	  if(data_ready)
	  {
			HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
			HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_RESET);
			read_payload();
			rf_clear_interrupt_flags();

				for(int i =0; i < DIODES_NUMBER; i++)
				{
					set_diode_color(i, payload[3],payload[4] , payload[5] );
				}
				send_data_to_spi();
			}
			HAL_GPIO_WritePin(GPIOA,RF_CHIP_ENABLE_Pin,GPIO_PIN_SET);
			data_ready = 0;
			HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	  }
  }
}
	
  /* USER CODE END 3 */
/**
  * @brief System Clock Configuration
  * @retval None
  */
void ADC_Config(void)
{
	__HAL_RCC_ADC1_CLK_ENABLE();
	RCC_PeriphCLKInitTypeDef clock_init;
	clock_init.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	clock_init.AdcClockSelection = RCC_ADCPCLK2_DIV8;
	HAL_RCCEx_PeriphCLKConfig(&clock_init);

	adc.Instance = ADC1;
	adc.Init.ContinuousConvMode = ENABLE;
	adc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	adc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	adc.Init.ScanConvMode = ADC_SCAN_DISABLE;
	adc.Init.NbrOfConversion = 1;
	adc.Init.DiscontinuousConvMode = DISABLE;
	adc.Init.NbrOfDiscConversion = 1;
	HAL_ADC_Init(&adc);

	HAL_ADCEx_Calibration_Start(&adc);

	ADC_ChannelConfTypeDef adc_ch;
	adc_ch.Channel = ADC_CHANNEL_0;
	adc_ch.Rank = ADC_REGULAR_RANK_1;
	adc_ch.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
	HAL_ADC_ConfigChannel(&adc, &adc_ch);
}

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
