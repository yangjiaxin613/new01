#ifndef __ESP8266_H__
#define __ESP8266_H__

#include <stdint.h>
#include "usart.h"

extern UART_HandleTypeDef g_uart_handle;

/* 引脚定义 */
#define ESP8266_UART_TX_GPIO_PORT           GPIOA
#define ESP8266_UART_TX_GPIO_PIN            GPIO_PIN_2
#define ESP8266_UART_TX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0) /* PC口时钟使能 */

#define ESP8266_UART_RX_GPIO_PORT           GPIOA
#define ESP8266_UART_RX_GPIO_PIN            GPIO_PIN_3
#define ESP8266_UART_RX_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0) /* PD口时钟使能 */

#define ESP8266_UART_INTERFACE              USART2
#define ESP8266_UART_IRQn                   USART2_IRQn
//#define ESP8266_UART_IRQHandler             USART2_IRQHandler
#define ESP8266_UART_CLK_ENABLE()           do{ __HAL_RCC_USART2_CLK_ENABLE(); }while(0) /* UART2时钟使能 */

/* UART收发缓冲大小 */
#define ESP8266_UART_RX_BUF_SIZE            512
#define ESP8266_UART_TX_BUF_SIZE            64

/* 错误代码 */
#define ESP8266_EOK                         0   /* 没有错误 */
#define ESP8266_ERROR                       1   /* 通用错误 */
#define ESP8266_ETIMEOUT                    2   /* 超时错误 */
#define ESP8266_EINVAL                      3   /* 参数错误 */

/* 工作模式 */
#define ESP8266_STA_MODE                    1
#define ESP8266_AP_MODE                     2
#define ESP8266_STA_AP_MODE                 3

#define WIFI_SSID                           "KD"
#define WIFI_PWD                            "20521388481"

#define TCP_SERVER_IP                       "mqtts.heclouds.com"
#define TCP_SERVER_PORT                     "1883"


uint8_t esp8266_init(uint32_t baudrate);
void esp8266_clear(void);
void esp8266_uart_printf(char *fmt, ...);
void esp8266_send_data(char *data, uint16_t len);
uint8_t esp8266_wait_receive(void);
uint16_t esp8266_copy_rxdata(char *data_receive);
#endif

