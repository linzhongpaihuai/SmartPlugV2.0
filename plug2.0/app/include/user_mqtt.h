/*
 * user_mqtt.h
 *
 *  Created on: 2019��8��4��
 *      Author: lenovo
 */

#ifndef __USER_MQTT_H__
#define __USER_MQTT_H__


#include "user_common.h"
#include "mqtt/MQTTClient.h"

#define MQTT_ADDR_LEN      		64
#define MQTT_CLIENTID_LEN  		64
#define MQTT_USERNAME_LEN  		64
#define MQTT_PASSWD_LEN    		64
#define MQTT_PASSWD_LEN    		64
#define MQTT_TOKEN_LEN     		40
#define MQTT_SOFTWAREVER_LEN    10
#define MQTT_TOPIC_NUM          3    // ���Ҫ���� MAX_MESSAGE_HANDLERS, ���ȷʵ��Ҫ�����޸�MAX_MESSAGE_HANDLERS�����Ȼ�����±���mqtt��������
#define MQTT_TOPIC_LEN          100
#define MQTT_URL_LEN            500  // �¹̼������ص�ַ

typedef enum tagMQTT_UPGRADE_STATUS{
	MQTT_UPGRADE_IDEA = 0,            // ����״̬
	MQTT_UPGRADE_DOWNLOADING,         // �������ع̼�
	MQTT_UPGRADE_BURNING,             // ������д�̼�
	MQTT_UPGRADE_DONE,                // �������
	MQTT_UPGRADE_TIMEOUT,             // ������ʱ

	MQTT_UPGRADE_BUFF,
}MQTT_UPGRADE_STATUS;

typedef enum{
	MQTT_CONSTATUS_Unknown = 0,
	MQTT_CONSTATUS_Connectting,
	MQTT_CONSTATUS_Connected,
	MQTT_CONSTATUS_Failed,

	MQTT_CONSTATUS_Buff,
}MQTT_CONSTATUS_E;

typedef struct tagMQTT_CTX{
	MQTTPacket_connectData      stConnectData;
	Network 					stNetwork;
	MQTTClient 					stClient;
	UINT                        uiTimeOut;       	// ����ͳ�ʱ�䣬��λms
	UINT						uiRecvBufSize;	    // ���ջ�������С
	UINT						uiSendBufSize;	    // ���ͻ�������С
	CHAR*                       pcRecvBuf;			// ���ջ�����
	CHAR*						pcSendBuf;          // ���ͻ�����

	INT                         iMqttPort;         // mqtt�������˿�
	CHAR						szMqttAddr[MQTT_ADDR_LEN];  		// mqtt��������ַ
	CHAR						szClientID[MQTT_CLIENTID_LEN];  	// mqtt clientID
	CHAR						szUserName[MQTT_USERNAME_LEN];  	// mqtt username
	CHAR						szPassWord[MQTT_PASSWD_LEN];  		// mqtt password
	CHAR                        szToken[MQTT_TOKEN_LEN];            // smartconfig����ʱ��token

	CHAR                        szCurSoftWareVer[MQTT_SOFTWAREVER_LEN];      // ��ǰ���еĹ̼��汾
	CHAR                        szUpdateSoftWareVer[MQTT_SOFTWAREVER_LEN];   // Ҫ�����Ĺ̼��汾
	CHAR                        szSoftWareUrl[MQTT_URL_LEN];                 // �̼����ص�ַ
	UINT                        uiSoftWareSize;                              // �̼���С
	UINT                        uiDownloadSize;                              // �����ع̼���С
	MQTT_UPGRADE_STATUS         enUpgradeStatus;                             // ��ǰ������״̬
	INT                         iDownloadProcess;                            // �̼����ؽ���

	MQTT_CONSTATUS_E            eConnectStatus;                             // �Ƿ����ӵ�������
	CHAR                        aszSubscribeTopic[MQTT_TOPIC_NUM][MQTT_TOPIC_LEN];// ��Ŷ��ĵ�����

}MQTT_CTX;

typedef struct tagWriteSoftWarePara{
	HTTP_CLIENT_S* pstCli;
	MQTT_CTX* pstMqttCtx;
}MQTT_WriteSoftWarePara_S;


#endif /* __USER_MQTT_H__ */
