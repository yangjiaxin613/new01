#include "onenet.h"
#include "esp8266.h"
#include "stdio.h"

char MQTT_ClientID[100]; //MQTT_�ͻ���ID
char MQTT_UserName[100]; //MQTT_�û���
char MQTT_PassWord[200]; //MQTT_����

uint8_t *mqtt_rxbuf;
uint8_t *mqtt_txbuf;
uint16_t mqtt_rxlen;
uint16_t mqtt_txlen;
uint8_t _mqtt_txbuf[512];//�������ݻ�����
uint8_t _mqtt_rxbuf[512];//�������ݻ�����

typedef enum
{
    //����         ֵ             ������������     ����
    M_RESERVED1    =0    ,    //    ��ֹ    ����
    M_CONNECT        ,    //    �ͻ��˵������    �ͻ����������ӷ����
    M_CONNACK        ,    //    ����˵��ͻ���    ���ӱ���ȷ��
    M_PUBLISH        ,    //    ������������    ������Ϣ
    M_PUBACK        ,    //    ������������    QoS 1��Ϣ�����յ�ȷ��
    M_PUBREC        ,    //    ������������    �����յ�����֤������һ����
    M_PUBREL        ,    //    ������������    �����ͷţ���֤�����ڶ�����
    M_PUBCOMP        ,    //    ������������    QoS 2��Ϣ������ɣ���֤������������
    M_SUBSCRIBE        ,    //    �ͻ��˵������    �ͻ��˶�������
    M_SUBACK        ,    //    ����˵��ͻ���    ����������ȷ��
    M_UNSUBSCRIBE    ,    //    �ͻ��˵������    �ͻ���ȡ����������
    M_UNSUBACK        ,    //    ����˵��ͻ���    ȡ�����ı���ȷ��
    M_PINGREQ        ,    //    �ͻ��˵������    ��������
    M_PINGRESP        ,    //    ����˵��ͻ���    ������Ӧ
    M_DISCONNECT    ,    //    �ͻ��˵������    �ͻ��˶Ͽ�����
    M_RESERVED2        ,    //    ��ֹ    ����
}_typdef_mqtt_message;

//���ӳɹ���������Ӧ 20 02 00 00
//�ͻ��������Ͽ����� e0 00
const uint8_t parket_connetAck[] = {0x20,0x02,0x00,0x00};
const uint8_t parket_disconnet[] = {0xe0,0x00};
const uint8_t parket_heart[] = {0xc0,0x00};
const uint8_t parket_heart_reply[] = {0xc0,0x00};
const uint8_t parket_subAck[] = {0x90,0x03};

/*
��������: ��ʼ���������������������ĵ�¼����
*/


//����
//����֮ǰ�����ݸ�ʽ:  clientId*deviceName*productKey#
// *�滻ΪDeviceName  #�滻ΪProductKey  ������Կ��DeviceSecret  ���ܷ�ʽ��HmacSHA1  
//PassWord����=  clientIdiot_devicedeviceNameiot_deviceproductKeya1VMIfYeEEE
//hmacsha1������վ��http://encode.chahuo.com/
//���ܵ���Կ��DeviceSecret

void mqtt_login_init(char *ProductKey,char *DeviceName,char *DeviceSecret)
{
//    sprintf(MQTT_ClientID,"%s.%s|securemode=2,signmethod=hmacsha256,timestamp=1695871022945|",ProductKey,DeviceName);
//    sprintf(MQTT_UserName,"%s&%s",DeviceName,ProductKey);
//    sprintf(MQTT_PassWord,"%s","a8921500839307ec3fedbbcd8c0cbc19f133f68c831dcad41fe13d92dc90b89d");
    sprintf(MQTT_ClientID,"%s", DeviceName);
    sprintf(MQTT_UserName,"%s", ProductKey);
    sprintf(MQTT_PassWord,"version=2018-10-31&res=products%%2F%s%%2Fdevices%%2F%s&et=2017881776&method=sha1&sign=%s",ProductKey,DeviceName,DEVICE_SECRET);
}

void mqtt_init(void)
{
    mqtt_login_init(PRODUCT_KEY,DEVICE_NAME,DEVICE_SECRET);
    //��������ֵ
    mqtt_rxbuf = _mqtt_rxbuf;
    mqtt_rxlen = sizeof(_mqtt_rxbuf);
    mqtt_txbuf = _mqtt_txbuf;
    mqtt_txlen = sizeof(_mqtt_txbuf);
    memset(mqtt_rxbuf,0,mqtt_rxlen);
    memset(mqtt_txbuf,0,mqtt_txlen);
    
    //�������������Ͽ�
    mqtt_disconnect();
    HAL_Delay(100);
    mqtt_disconnect();
    HAL_Delay(100);
}

/*
��������: ��¼������
��������ֵ: 0��ʾ�ɹ� 1��ʾʧ��
*/
uint8_t mqtt_connect(char *ClientID,char *Username,char *Password)
{
//    uint8_t i;
    uint8_t j;
    int ClientIDLen = strlen(ClientID);
    int UsernameLen = strlen(Username);
    int PasswordLen = strlen(Password);
    int DataLen;
    mqtt_txlen=0;
    //�ɱ䱨ͷ+Payload  ÿ���ֶΰ��������ֽڵĳ��ȱ�ʶ
    DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);
    
    //�̶���ͷ
    //���Ʊ�������
    mqtt_txbuf[mqtt_txlen++] = 0x10;        //MQTT Message Type CONNECT
    //ʣ�೤��(�������̶�ͷ��)
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( DataLen > 0 )
            encodedByte = encodedByte | 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    }while ( DataLen > 0 );
        
    //�ɱ䱨ͷ
    //Э����
    mqtt_txbuf[mqtt_txlen++] = 0;            // Protocol Name Length MSB    
    mqtt_txbuf[mqtt_txlen++] = 4;           // Protocol Name Length LSB    
    mqtt_txbuf[mqtt_txlen++] = 'M';            // ASCII Code for M    
    mqtt_txbuf[mqtt_txlen++] = 'Q';            // ASCII Code for Q    
    mqtt_txbuf[mqtt_txlen++] = 'T';            // ASCII Code for T    
    mqtt_txbuf[mqtt_txlen++] = 'T';            // ASCII Code for T    
    //Э�鼶��
    mqtt_txbuf[mqtt_txlen++] = 4;                // MQTT Protocol version = 4    
    //���ӱ�־
    mqtt_txbuf[mqtt_txlen++] = 0xc2;            // conn flags 
    mqtt_txbuf[mqtt_txlen++] = 0;                // Keep-alive Time Length MSB    
    mqtt_txbuf[mqtt_txlen++] = 100;            // Keep-alive Time Length LSB  100S������  
    
    mqtt_txbuf[mqtt_txlen++] = BYTE1(ClientIDLen);// Client ID length MSB    
    mqtt_txbuf[mqtt_txlen++] = BYTE0(ClientIDLen);// Client ID length LSB      
    memcpy(&mqtt_txbuf[mqtt_txlen],ClientID,ClientIDLen);
    mqtt_txlen += ClientIDLen;
    
    if(UsernameLen > 0)
    {   
        mqtt_txbuf[mqtt_txlen++] = BYTE1(UsernameLen);        //username length MSB    
        mqtt_txbuf[mqtt_txlen++] = BYTE0(UsernameLen);        //username length LSB    
        memcpy(&mqtt_txbuf[mqtt_txlen],Username,UsernameLen);
        mqtt_txlen += UsernameLen;
    }
    
    if(PasswordLen > 0)
    {    
        mqtt_txbuf[mqtt_txlen++] = BYTE1(PasswordLen);        //password length MSB    
        mqtt_txbuf[mqtt_txlen++] = BYTE0(PasswordLen);        //password length LSB  
        memcpy(&mqtt_txbuf[mqtt_txlen],Password,PasswordLen);
        mqtt_txlen += PasswordLen; 
    }    
    
//    for(i=0;i<10;i++)
//    {
        memset(mqtt_rxbuf,0,mqtt_rxlen);
        mqtt_send_data(mqtt_txbuf,mqtt_txlen);
//        for(j=0;j<10;j++)
//            printf("%c",mqtt_txbuf[j]);
        for(j=0;j<10;j++)
        {
            HAL_Delay(50);
            if (esp8266_wait_receive() == ESP8266_EOK)
                esp8266_copy_rxdata((char *)mqtt_rxbuf);

            //CONNECT
            if(mqtt_rxbuf[0]==parket_connetAck[0] && mqtt_rxbuf[1]==parket_connetAck[1] && mqtt_rxbuf[2]==parket_connetAck[2]) //���ӳɹ�
            {
                return 0;//���ӳɹ�
            }
        }
//    }
    return 1;
}

/*
��������: MQTT����/ȡ���������ݴ������
��������:
    topic       ����   
    qos         ��Ϣ�ȼ� 0:���ַ�һ��  1: ���ٷַ�һ��  2: ���ַ�һ��
    whether     ����/ȡ����������� (1��ʾ����,0��ʾȡ������)
����ֵ: 0��ʾ�ɹ� 1��ʾʧ��
*/
uint8_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether)
{    
//    uint8_t i;
    uint8_t j;
    mqtt_txlen=0;
    int topiclen = strlen(topic);
    
    int DataLen = 2 + (topiclen+2) + (whether?1:0);//�ɱ䱨ͷ�ĳ��ȣ�2�ֽڣ�������Ч�غɵĳ���
    //�̶���ͷ
    //���Ʊ�������
    if(whether)mqtt_txbuf[mqtt_txlen++] = 0x82; //��Ϣ���ͺͱ�־����
    else    mqtt_txbuf[mqtt_txlen++] = 0xA2;    //ȡ������

    //ʣ�೤��
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( DataLen > 0 )
            encodedByte = encodedByte | 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    }while ( DataLen > 0 );    
    
    //�ɱ䱨ͷ
    mqtt_txbuf[mqtt_txlen++] = 0;            //��Ϣ��ʶ�� MSB
    mqtt_txbuf[mqtt_txlen++] = 0x01;        //��Ϣ��ʶ�� LSB
    //��Ч�غ�
    mqtt_txbuf[mqtt_txlen++] = BYTE1(topiclen);//���ⳤ�� MSB
    mqtt_txbuf[mqtt_txlen++] = BYTE0(topiclen);//���ⳤ�� LSB   
    memcpy(&mqtt_txbuf[mqtt_txlen],topic,topiclen);
    mqtt_txlen += topiclen;
    
    if(whether)
    {
       mqtt_txbuf[mqtt_txlen++] = qos;//QoS����
    }
    
//    for(i=0;i<10;i++)
//    {
        memset(mqtt_rxbuf,0,mqtt_rxlen);
        mqtt_send_data(mqtt_txbuf,mqtt_txlen);

        for(j=0;j<10;j++)
        {
            HAL_Delay(50);
            if (esp8266_wait_receive() == ESP8266_EOK)
                esp8266_copy_rxdata((char *)mqtt_rxbuf);

            if(mqtt_rxbuf[0]==parket_subAck[0] && mqtt_rxbuf[1]==parket_subAck[1]) //���ĳɹ�               
            {
                return 0;//���ĳɹ�
            }
        }
//    }
    return 1; //ʧ��
}

//MQTT�������ݴ������
//topic   ���� 
//message ��Ϣ
//qos     ��Ϣ�ȼ� 
uint8_t mqtt_publish_data(char *topic, char *message, uint8_t qos)
{  
    int topicLength = strlen(topic);    
    int messageLength = strlen(message);     
    static uint16_t id=0;
    int DataLen;
    mqtt_txlen=0;
    //��Ч�غɵĳ����������㣺�ù̶���ͷ�е�ʣ�೤���ֶε�ֵ��ȥ�ɱ䱨ͷ�ĳ���
    //QOSΪ0ʱû�б�ʶ��
    //���ݳ���             ������   ���ı�ʶ��   ��Ч�غ�
    if(qos)    DataLen = (2+topicLength) + 2 + messageLength;       
    else    DataLen = (2+topicLength) + messageLength;   

    //�̶���ͷ
    //���Ʊ�������
    mqtt_txbuf[mqtt_txlen++] = 0x30;    // MQTT Message Type PUBLISH  

    //ʣ�೤��
    do
    {
        uint8_t encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( DataLen > 0 )
            encodedByte = encodedByte | 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    }while ( DataLen > 0 );    
    
    mqtt_txbuf[mqtt_txlen++] = BYTE1(topicLength);//���ⳤ��MSB
    mqtt_txbuf[mqtt_txlen++] = BYTE0(topicLength);//���ⳤ��LSB 
    memcpy(&mqtt_txbuf[mqtt_txlen],topic,topicLength);//��������
    mqtt_txlen += topicLength;
        
    //���ı�ʶ��
    if(qos)
    {
        mqtt_txbuf[mqtt_txlen++] = BYTE1(id);
        mqtt_txbuf[mqtt_txlen++] = BYTE0(id);
        id++;
    }
    memcpy(&mqtt_txbuf[mqtt_txlen],message,messageLength);
    mqtt_txlen += messageLength;

//    int i = 0;
//    for(i=0;i<mqtt_txlen;i++)
//        printf("%02X ", mqtt_txbuf[i]);
//    printf("\r\n");
    mqtt_send_data(mqtt_txbuf,mqtt_txlen);
    return mqtt_txlen;
}

uint8_t mqtt_receive_handle(uint8_t *data_received, Mqtt_RxData_Type *rx_data)
{
    uint8_t *p;
    uint8_t encodeByte = 0;
    uint32_t multiplier = 1, Remaining_len = 0;
    uint8_t QS_level = 0;
    
    p = data_received;
    memset(rx_data, 0, sizeof(Mqtt_RxData_Type));
    
    //������������
    if((*p != 0x30)&&(*p != 0x32)&&(*p != 0x34))   //���Ƿ�������ͷ
        return 1;
    
    if(*p != 0x30) QS_level = 1;    //���qs�ȼ���Ϊ0
    
    p++;
    //��ȡʣ�����ݳ���
    do{
        encodeByte = *p++;
        Remaining_len += (encodeByte & 0x7F) * multiplier;
        multiplier *= 128;
        
        if(multiplier > 128*128*128) //����ʣ�೤�����4���ֽڵ�Ҫ��,����
            return 2;
    }while((encodeByte & 0x80) != 0);
    
    //��ȡ�������ݳ���
    rx_data->topic_len = *p++;
    rx_data->topic_len = rx_data->topic_len * 256 + *p++;
    //��ȡ����
    memcpy(rx_data->topic,p,rx_data->topic_len);
    p += rx_data->topic_len;
    
    if(QS_level != 0)  //�������ı�ʶ��
        p += 2;
    
    //��ȡpayload
    rx_data->payload_len = Remaining_len - rx_data->topic_len - 2;
    memcpy(rx_data->payload, p, rx_data->payload_len);
    
//    printf("topic: %s\r\n", rx_data->topic);
//    printf("topic_len: %d\r\n", rx_data->topic_len);
//    printf("payload: %s\r\n", rx_data->payload);
//    printf("payload_len: %d\r\n", rx_data->payload_len);

    return 0;
}

void mqtt_send_response(uint8_t *id)
{
    char buf[128] = {0};
    sprintf(buf,"{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}",id);
    
    mqtt_publish_data(RELY_PUBLISH_TOPIC,(char *)buf,0);
    
    printf("\r\n��������:\r\n");
    printf((const char *)buf);    //���������ݴ�ӡ����
    printf("\r\n");
}

void mqtt_send_heart(void)
{
    mqtt_send_data((uint8_t *)parket_heart,sizeof(parket_heart));
}

void mqtt_disconnect(void)
{
    mqtt_send_data((uint8_t *)parket_disconnet,sizeof(parket_disconnet));
}

void mqtt_send_data(uint8_t *buf,uint16_t len)
{
    esp8266_send_data((char *)buf, len);
}
