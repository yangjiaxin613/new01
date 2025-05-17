/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "dht11.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
uint8_t PIR_state;
uint8_t Light_state;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId Vibration_PIR_THandle;
osThreadId Light_TaskHandle;
osThreadId DHT11_TaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartVibration_PIR_Task(void const * argument);
void StartLight_Task(void const * argument);
void StartDHT11_Task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Vibration_PIR_T */
  osThreadDef(Vibration_PIR_T, StartVibration_PIR_Task, osPriorityNormal, 0, 128);
  Vibration_PIR_THandle = osThreadCreate(osThread(Vibration_PIR_T), NULL);

  /* definition and creation of Light_Task */
  osThreadDef(Light_Task, StartLight_Task, osPriorityNormal, 0, 256);
  Light_TaskHandle = osThreadCreate(osThread(Light_Task), NULL);

  /* definition and creation of DHT11_Task */
  osThreadDef(DHT11_Task, StartDHT11_Task, osPriorityBelowNormal, 0, 256);
  DHT11_TaskHandle = osThreadCreate(osThread(DHT11_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartVibration_PIR_Task */
/**
  * @brief  Function implementing the Vibration_PIR_T thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartVibration_PIR_Task */
void StartVibration_PIR_Task(void const * argument)
{
  /* USER CODE BEGIN StartVibration_PIR_Task */
		static uint8_t last_vibration_state = GPIO_PIN_SET;
		char sensor_buff[20] = {0};
  /* Infinite loop */
  for(;;)
  {
 // 处理振动传感器
    uint8_t current_vib_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);
    
    if (current_vib_state != last_vibration_state)
    {
        if (current_vib_state == GPIO_PIN_RESET)
        {
						osDelay(100);
						//printf("震动\r\n");
            HAL_UART_Transmit(&huart3, (uint8_t*)"V:down\r\n", strlen("V:down\r\n"), 2000);
        }
        last_vibration_state = current_vib_state;
    }
    
    // 处理PIR传感器
    if (HAL_GPIO_ReadPin(PIR_SENSOR_GPIO_PORT, PIR_SENSOR_PIN) == GPIO_PIN_SET)
    {
        PIR_state = 1;
				//printf("有人\r\n");
        snprintf(sensor_buff, sizeof(sensor_buff), "P:%d\r\n", PIR_state);
        HAL_UART_Transmit(&huart3, (uint8_t*)sensor_buff, strlen(sensor_buff), 20000);
        Buzzer_Beep(1000);
    }
    else
    {
        PIR_state = 0;
        snprintf(sensor_buff, sizeof(sensor_buff), "P:%d\r\n", PIR_state);
        HAL_UART_Transmit(&huart3, (uint8_t*)sensor_buff, strlen(sensor_buff), 20000);
    }
    
    osDelay(100); // 使用最短的延迟，因为这是两个传感器中需要的
  }
  /* USER CODE END StartVibration_PIR_Task */
}

/* USER CODE BEGIN Header_StartLight_Task */
/**
* @brief Function implementing the Light_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLight_Task */
void StartLight_Task(void const * argument)
{
  /* USER CODE BEGIN StartLight_Task */
		
		//printf("hello\r\n");
		char Light_buff[10] = {0};
  /* Infinite loop */
    for (;;)
    {
        uint32_t lightVoltage = Get_Light_Voltage_ADC2();

        if (lightVoltage < 1533)
        {
						Light_state=0;
            //printf("环境光线较暗: %d\r\n", lightVoltage);
            Buzzer_Beep(500);
        }
        else
        {
						Light_state=1;
            //printf("环境光线充足: %d\r\n", lightVoltage);
        }
				
				snprintf(Light_buff, sizeof(Light_buff), "L:%d\r\n",lightVoltage);
				HAL_UART_Transmit(&huart3, (uint8_t*)Light_buff, strlen(Light_buff), 20000);
				
        osDelay(1000);
    }
  /* USER CODE END StartLight_Task */
}

/* USER CODE BEGIN Header_StartDHT11_Task */
/**
* @brief Function implementing the DHT11_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDHT11_Task */
void StartDHT11_Task(void const * argument)
{
  /* USER CODE BEGIN StartDHT11_Task */
  /* Infinite loop */
	osDelay(1000); // 延迟1秒等待系统稳定
		char buffer[64];
    uint16_t len;
    char message_error[] = "DHT11 error !\r\n";
    for (;;)
    {
//									printf("Raw DHT11 Data: %d.%d°C, %d.%d%%\r\n",
//                   data_dht11[0], data_dht11[1], data_dht11[2], data_dht11[3]);
			
        if (DHT11_READ_DATA() == 1)
        {
					
            len = snprintf(buffer, sizeof(buffer), "T:%d.%d,H:%d.%d\r\n",
                           data_dht11[0], data_dht11[1], data_dht11[2], data_dht11[3]);
            HAL_UART_Transmit(&huart3, (uint8_t*)buffer, len, 20000);
        }
        else
        {
            HAL_UART_Transmit(&huart3, (uint8_t*)message_error, strlen(message_error), 20000);
        }

        osDelay(2000); // 每3秒读取一次
    }
  /* USER CODE END StartDHT11_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

