/*
 * user_bigiot.h
 *
 *  Created on: 2019��8��4��
 *      Author: lenovo
 */

#ifndef __USER_BIGIOT_H__
#define __USER_BIGIOT_H__

#include "MQTTFreeRTOS.h"

#define BIGIOT_NONE       4
#define BIGIOT_ERROR      3
#define BIGIOT_INFO       2
#define BIGIOT_DEBUG      1

#define BIGIOT_LOG(lev, arg...) LOG_OUT(lev, ##arg)


#define BIGIOT_DEVNAME_LEN     64
#define BIGIOT_EVENT_NUM       7
#define BIGIOT_CBNAME_NUM      64

typedef char* (*CallbackFun)(void *para);

typedef struct tagBigiotEvent
{
    char*             pcIfId;                                //���������Ľӿ�ID
    char              szCbName[BIGIOT_CBNAME_NUM];           //�ص���������
    CallbackFun       cb;                                    //�ص�����
    void*             cbPara;                                //�ص��������
    void*             pstCtx;                                //BIGIOT_Ctx_S

}BIGIOT_Event_S;

typedef struct tagBigiot
{
    char* pcHostName;                        //��������ƽ̨��������������www.bigiot.net
    int port;                                //��������ƽ̨�������Ķ˿ڣ�������8181
    char szDevName[BIGIOT_DEVNAME_LEN];        //��Ӧ�����������豸����
    DEVTYPE_E eDevType;                        //�豸����
    char* pcDeviceId;                        //�豸ID
    char* pcApiKey;                            //APIKEY

    int socket;                                //ʹ��tcpЭ�����ӵ�socket������
    int iTimeOut;                            //���ͺͽ��յĳ�ʱʱ�䣬������ʱ����Ϊ���ͻ����ʧ��

    xTaskHandle xEventHandle;                //����ע���¼������������
    int iAlived;                            //����ƽ̨״̬��0:δ֪״̬��1:�������ӣ�2:���ӳɹ���3������ʧ��

    BIGIOT_Event_S astEvent[BIGIOT_EVENT_NUM]; //����������״̬���¶ȡ�ʪ�ȵ��¼���ע�ᵽ����

    unsigned char ucReadLock;               // ��������ʱ������������˶�ȡ����������Ҫ��һ�������������ݱ�����������ռ��ȡ

    int  (*Read)(struct tagBigiot*, unsigned char*, unsigned int, unsigned int); //���ݽ��ܺ���
    int  (*Write)(struct tagBigiot*, const unsigned char*, unsigned int, unsigned int); //���ݷ��ͺ���
    int (*Connect)(struct tagBigiot*);        //���Ӻ���
    void (*Disconnect)(struct tagBigiot*);    //�Ͽ����Ӻ���

}BIGIOT_Ctx_S;



BIGIOT_Ctx_S* Bigiot_New( char* pcHostName, int iPort, char* pcDevId, char* pcApiKey );
void BIGIOT_Destroy( BIGIOT_Ctx_S **ppstCtx );
int Bigiot_Login( BIGIOT_Ctx_S *pstCtx );
int Bigiot_Logout( BIGIOT_Ctx_S *pstCtx );
int Bigiot_Cycle( BIGIOT_Ctx_S *pstCtx );
int Bigiot_GetBigioStatus( void );
char* Bigiot_GetBigioDeviceName( void );

#endif /* __USER_BIGIOT_H__ */
