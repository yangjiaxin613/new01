/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "dht11.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rxbuf[4] = {0};
uint8_t text[20] = {0};
uint8_t flag3 = 0;
uint8_t i = 0;
uint8_t data_dht11[5] = {0};

#define THRESHOLD_VOLTAGE 3300 // 根据实际电压调整阈值（单位：mV）
static uint8_t last_state = GPIO_PIN_SET;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Buzzer_On(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
}

void Buzzer_Off(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}

void Buzzer_Beep(uint32_t duration_ms)
{
    Buzzer_On();
    osDelay(duration_ms); // 使用 FreeRTOS 延时
    Buzzer_Off();
}

uint32_t Get_Light_Voltage_ADC2(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < 10; i++)
    {
        HAL_ADC_Start(&hadc2);
        if (HAL_ADC_PollForConversion(&hadc2, 100) == HAL_OK)
        {
            sum += HAL_ADC_GetValue(&hadc2);
        }
        HAL_ADC_Stop(&hadc2);
        osDelay(1); // 提高稳定性
    }

    uint32_t adc_value = sum / 10;
    return (adc_value * 3300UL) / 4095UL; // 返回电压值（mV）
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
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, (uint8_t *)text, sizeof(text));
  Buzzer_Off();
//printf("环境光线较暗\r\n");
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
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
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//void MyDefaultTask(void *argument)
//{
//    for (;;)
//    {
//        osDelay(1000);
//    }
//}

//void DHT11_Task(void *argument)
//{
//		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
//    char message_error[] = "DHT11 error !\r\n";
//    char buffer[64];
//    uint16_t len;

//    for (;;)
//    {
//        if (DHT11_READ_DATA() == 1)
//        {
//            len = snprintf(buffer, sizeof(buffer), "%d.%d,%d.%d\n",
//                           data_dht11[0], data_dht11[1], data_dht11[2], data_dht11[3]);
//            HAL_UART_Transmit(&huart3, (uint8_t*)buffer, len, 20000);
//        }
//        else
//        {
//            HAL_UART_Transmit(&huart3, (uint8_t*)message_error, strlen(message_error), 20000);
//        }

//        osDelay(3000); // 每3秒读取一次
//    }
//}

//void Vibration_Task(void *argument)
//{
//    static uint8_t last_state = GPIO_PIN_SET;

//    for (;;)
//    {
//        uint8_t current_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);

//        if (current_state != last_state)
//        {
//            if (current_state == GPIO_PIN_RESET)
//            {
//                HAL_UART_Transmit(&huart1, (uint8_t*)"震动\r\n", strlen("震动\r\n"), 2000);
//            }
//            last_state = current_state;
//        }

//        osDelay(100);
//    }
//}


//void Light_Task(void *argument)
//{
//    for (;;)
//    {
//        uint32_t lightVoltage = Get_Light_Voltage_ADC2();

//        if (lightVoltage < 1533)
//        {
//            printf("环境光线较暗: %d\r\n", lightVoltage);
//            Buzzer_Beep(500);
//        }
//        else
//        {
//            printf("环境光线充足: %d\r\n", lightVoltage);
//        }

//        osDelay(1000);
//    }
//}

//void PIR_Task(void *argument)
//{
//    for (;;)
//    {
//        if (HAL_GPIO_ReadPin(PIR_SENSOR_GPIO_PORT, PIR_SENSOR_PIN) == GPIO_PIN_SET)
//        {
//            char *pirDetected = "Human detected!\r\n";
//            HAL_UART_Transmit(&huart1, (uint8_t*)pirDetected, strlen(pirDetected), 2000);
//            Buzzer_Beep(1000);
//        }
//        else
//        {
//            char *noHuman = "No human detected.\r\n";
//            HAL_UART_Transmit(&huart1, (uint8_t*)noHuman, strlen(noHuman), 2000);
//        }

//        osDelay(1000);
//    }
//}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
