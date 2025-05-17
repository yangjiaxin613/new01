#ifndef _ONENET_H_
#define _ONENET_H_

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdint.h"


#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
    
extern char MQTT_ClientID[100]; //MQTT_�ͻ���ID
extern char MQTT_UserName[100]; //MQTT_�û���
extern char MQTT_PassWord[200]; //MQTT_����

typedef struct
{
    uint8_t topic[512];
    uint16_t topic_len;
    uint8_t payload[512];
    uint16_t payload_len;
} Mqtt_RxData_Type;

//�Ʒ��������豸֤��
#define PRODUCT_KEY "8VORdlxrZT"
#define DEVICE_NAME "temp01"
#define DEVICE_SECRET "cP5gRsFQWcnfrBo9Ryuvoajm%2ByE%3D"

//�����뷢��������
#define RELY_PUBLISH_TOPIC  "$sys/8VORdlxrZT/temp01/thing/property/set_reply"  //��������Ӧ��������,onenet studio����õ�
#define SET_TOPIC  "$sys/8VORdlxrZT/temp01/thing/property/set"
#define POST_TOPIC "$sys/8VORdlxrZT/temp01/thing/property/post"
//�¼��ϱ�����
#define EVENT_PUBLISH_TOPIC   "$sys/8VORdlxrZT/temp01/thing/event/post"  //��������,onenet studio����õ�

//�������û�����ʼ��
void mqtt_login_init(char *ProductKey,char *DeviceName,char *DeviceSecret);
//MQTTЭ����غ�������
uint8_t mqtt_publish_data(char *topic, char *message, uint8_t qos);
uint8_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether);
void mqtt_init(void);
uint8_t mqtt_connect(char *ClientID,char *Username,char *Password);
void mqtt_send_heart(void);
void mqtt_disconnect(void);
void mqtt_send_data(uint8_t *buf,uint16_t len);
void mqtt_send_response(uint8_t *id);
uint8_t mqtt_receive_handle(uint8_t *data_received, Mqtt_RxData_Type *rx_data);
#endif
