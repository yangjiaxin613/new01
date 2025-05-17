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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <string.h>
#include "esp8266.h"
#include "onenet.h"
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
#define RX_BUFFER_SIZE 128
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_byte;
uint16_t rx_index = 0;
uint8_t rxbuf[50]={0};
uint8_t rxbuf_1[50]={0};
uint8_t i=0;
uint8_t senddata[4]={1,2,3,4};

int h_int,h_dec,t_int,t_dec;
uint8_t PIR_state;              // PIR 状态
uint16_t Light_value;           // 光照值
uint8_t Vibration_state;        // 振动状态

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	//HAL_NVIC_EnableIRQ(USART2_IRQn);
		HAL_UART_Receive_IT(&huart3, &rx_byte, 1); // 每次只接收一个字节
	
	//HAL_UART_Receive_IT(&huart1,rxbuf,sizeof(rxbuf));
	
	printf("    初始化ESP8266...\r\n");
	esp8266_init(115200);
	printf("    初始化MQTT...\r\n");
	mqtt_init();
	printf("    MQTT连接...\r\n");
	
	mqtt_connect(MQTT_ClientID,MQTT_UserName,MQTT_PassWord);
	
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//printf("Parsed: Humi=%d.%d%%, Temp=%d.%d°C\r\n", h_int, h_dec, t_int, t_dec);
		
		uint8_t data_send_buff[550];
		memset(data_send_buff, 0, sizeof(data_send_buff));
		
		// t_int, t_dec, h_int, h_dec
		
    sprintf((char *)data_send_buff,
            "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"CurrentTemperature\":{\"value\":%d.%d},\"CurrentHumidity\":{\"value\":%d.%d},\"Light_value\":{\"value\":%d},\"PIR\":{\"value\":%d},\"Vibration\":{\"value\":%d}}}",
            t_int, t_dec, h_int, h_dec, // 温湿度
            Light_value,                // 光照值
            PIR_state,                  // PIR 状态
            Vibration_state             // 振动状态
    );

		// 打印调试信息（可选）
		//printf("Sending Data: %s\r\n", data_send_buff);
		
		mqtt_publish_data(POST_TOPIC, (char *)data_send_buff, 0);
		HAL_Delay(3000);        //3s发送一次
		mqtt_send_heart();
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
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
if (huart == &huart3)
    {
        if (rx_byte == '\n') // 以换行符作为帧结束标志
        {
            rx_buffer[rx_index] = '\0'; // 字符串结尾

            // 解析不同数据格式
            int parsed = 0;

            if (sscanf((char *)rx_buffer, "T:%d.%d,H:%d.%d", &t_int, &t_dec, &h_int, &h_dec) == 4)
            {
                //printf("温度: %d.%d°C, 湿度: %d.%d%%\r\n", t_int, t_dec, h_int, h_dec);
                parsed = 1;
            }
            else if (sscanf((char *)rx_buffer, "L:%hu", &Light_value) == 1)
            {
                //printf("光照值: %d\r\n", Light_value);
                parsed = 1;
            }
            else if (sscanf((char *)rx_buffer, "P:%hhu", &PIR_state) == 1)
            {
                //printf("PIR状态: %s\r\n", PIR_state ? "有人" : "无人");
                parsed = 1;
            }
            else if (strstr((char *)rx_buffer, "V:down") != NULL)
            {
                //printf("检测到振动！\r\n");
                Vibration_state = 1;
                parsed = 1;
            }

            if (!parsed)
            {
                printf("未知数据: %s\r\n", rx_buffer);
            }

            // 清空缓冲区并重置索引
            rx_index = 0;
            memset(rx_buffer, 0, sizeof(rx_buffer));
        }
        else if (rx_index < RX_BUFFER_SIZE - 1)
        {
            rx_buffer[rx_index++] = rx_byte; // 存入缓冲区
        }

        // 继续接收下一个字节
        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
    }
	if(huart==&huart1){
		//HAL_UART_Transmit(&huart3, rxbuf,sizeof(rxbuf) , 0xffff);
		//HAL_UART_Receive_IT(&huart1,rxbuf,sizeof(rxbuf));//传送到串口2
	}
}
/* USER CODE END 4 */

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
