/*
 * user_infrared.h
 *
 *  Created on: 2018��11��17��
 *      Author: lenovo
 */

#ifndef __USER_INFRARE_H__
#define __USER_INFRARE_H__

#define INFRARED_MAX             10
#define INFRARED_NAME_MAX_LEN    32

#define INFRARED_ALL            (INFRARED_MAX + 1)

typedef struct tagINFRARED_VALUE                            /* ��������  */
{
    UINT                uiNum;                            /* ��ʱ����� */
    CHAR                szName[INFRARED_NAME_MAX_LEN];    /* ����         */
    BOOL                bEnable;                        /* ʹ��         */
    UINT                uiOnValue;                        /* ������ֵ     */
    UINT                uiOffValue;                        /* �ر���ֵ     */
}INFRARED_VALUE_S;

typedef struct tagINFRARED_Set
{
    BOOL                bIsSetting;                    /* ��set����get    */
    BOOL                bIsRefresh;                    /* �Ƿ��ȡ���µ�ֵ     */
    UINT                uiValue;                    /* ��ֵ             */
}INFRARED_SET_S;

extern INFRARED_SET_S g_stinfrared_Set;

extern VOID INFRARED_InfraredInit( VOID );
extern VOID INFRARED_JudgeInfrared( VOID );

INFRARED_VALUE_S* INFRARED_GetInfraredData( UINT8 ucNum );
UINT32 INFRARED_GetInfraredDataSize();
UINT INFRARED_SaveInfraredData( INFRARED_VALUE_S* pstData );
UINT INFRARED_SetInfrared( UINT8 ucNum, UINT8 ucSwitch, UINT uiTimeOut_s );
VOID INFRARED_InfraredDataDeInit( VOID );


VOID INFRA_InfraredInit( VOID );

#endif /* __USER_INFRARE_H__ */
