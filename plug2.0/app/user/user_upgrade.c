/*
 * user_upgrade.c
 *
 *  Created on: 2018��12��7��
 *      Author: lenovo
 */

#include "user_common.h"
#include "esp_common.h"

/*
��makefile 23�������޸Ķ�Ӧ���ֶ�
-----------------------------------------------------------------------------------------
BOOT            none    ��ʾ��ִ�� boot.bin�ļ�����û��boot.bin�ļ�
                new     ��ʾִ�� boot.bin�ļ���ÿ��ִ�н�Ҫִ��boot.bin�ļ�
APP               1     ��ʾ���� user1.bin�ļ�    ע��������������BOOT=new ����Ч
                  2     ��ʾ���� user2.bin�ļ�    ע��������������BOOT=new ����Ч
SPI_SPEED        40     ��ʾ��¼ʱ���Ƶ��ѡ������һ��Ϊ40����Ӧ��¼����Ĭ��ֵ����
SPI_MODE        QIO     �����Ҫ��8266ģ�������̣��������ڳ������ǰ��ſɵ�8266-12F������QIO��
SPI_SIZE_MAP    1        flash=256��user1��ַ0x1000,user2��ַ0x41000     ������Ӧ��ͬ��falsh��С�̼�
                2        flash=1024��user1��ַ0x1000,user2��ַ0x81000    ������Ӧ��ͬ��falsh��С�̼�
                3        flash=2048��user1��ַ0x1000,user2��ַ0x81000    ������Ӧ��ͬ��falsh��С�̼�
                4        flash=4096��user1��ַ0x1000,user2��ַ0x81000    ������Ӧ��ͬ��falsh��С�̼�
                5        flash=2048��user1��ַ0x1000,user2��ַ0x101000   ������Ӧ��ͬ��falsh��С�̼�
                6        flash=4096��user1��ַ0x1000,user2��ַ0x101000   ������Ӧ��ͬ��falsh��С�̼�
                8        flash=8192��user1��ַ0x1000,user2��ַ0x101000   ������Ӧ��ͬ��falsh��С�̼�
                9        flash=16384��user1��ַ0x1000,user2��ַ0x101000  ������Ӧ��ͬ��falsh��С�̼�
                ������ֵ    flash=256��user1��ַ0x1000,user2��ַ0x41000        ������Ӧ��ͬ��falsh��С�̼�
-----------------------------------------------------------------------------------------
*/

const CHAR szFlashMap[][5] = {"none", "0.4MB", "1MB", "2MB", "4MB", "2MB", "4MB", "8MB", "16MB", "0.4MB" };

const UINT32 User1BinFlashSizeMap[] =
{
        0x1000,     //Flash:4Mbits. 256KBytes+256KBytes��  user1��ַ0x1000,user2��ַ0x41000
        0x1000,     //Flash:2Mbits. 256KBytes��  user1��ַ0x1000,user2��ַ0x1000
        0x1000,     //Flash:8Mbits. 512KBytes+512KBytes��  user1��ַ0x1000,user2��ַ0x81000
        0x1000,     //Flash:16Mbits.512KBytes+512KBytes��  user1��ַ0x1000,user2��ַ0x81000
        0x1000,     //Flash:32Mbits.512KBytes+512KBytes��  user1��ַ0x1000,user2��ַ0x81000
        0x1000,     //Flash:16Mbits.1024KBytes+1024KBytes��  user1��ַ0x1000,user2��ַ0x101000
        0x1000,     //Flash:32Mbits.1024KBytes+1024KBytes��  user1��ַ0x1000,user2��ַ0x101000
        0x1000,     //Flash:32Mbits don't support now
        0x1000,     //Flash:64Mbits.1024KBytes+1024KBytes��  user1��ַ0x1000,user2��ַ0x101000
        0x1000      //Flash:128Mbits.1024KBytes+1024KBytes��  user1��ַ0x1000,user2��ַ0x101000
};


const UINT32 User2BinFlashSizeMap[] =
{
        0x41000,     //Flash:4Mbits. 256KBytes+256KBytes��  user1��ַ0x1000,user2��ַ0x41000
        0x01000,     //Flash:2Mbits. 256KBytes��  user1��ַ0x1000,user2��ַ0x1000
        0x81000,     //Flash:8Mbits. 512KBytes+512KBytes��  user1��ַ0x1000,user2��ַ0x81000
        0x81000,     //Flash:16Mbits.512KBytes+512KBytes��  user1��ַ0x1000,user2��ַ0x81000
        0x81000,     //Flash:32Mbits.512KBytes+512KBytes��  user1��ַ0x1000,user2��ַ0x81000
        0x101000,    //Flash:16Mbits.1024KBytes+1024KBytes��  user1��ַ0x1000,user2��ַ0x101000
        0x101000,     //Flash:32Mbits.1024KBytes+1024KBytes��  user1��ַ0x1000,user2��ַ0x101000
        0x41000,     //Flash:32Mbits don't support now
        0x101000,     //Flash:64Mbits.1024KBytes+1024KBytes��  user1��ַ0x1000,user2��ַ0x101000
        0x101000      //Flash:128Mbits.1024KBytes+1024KBytes��  user1��ַ0x1000,user2��ַ0x101000
};



UINT32 UPGRADE_GetUpgradeUserBinAddr( VOID )
{
    UINT8 ucUserBin = 0;
    UINT8 ucFlashSize = 0;

    ucUserBin = system_upgrade_userbin_check();
    ucFlashSize = system_get_flash_size_map();

    if ( ucUserBin == UPGRADE_FW_BIN1 )
    {
        //��ǰ���е���user1.bin������user2.bin������
        return User2BinFlashSizeMap[ucFlashSize];
    }
    else if ( ucUserBin == UPGRADE_FW_BIN2 )
    {
        //��ǰ���е���user2.bin������user1.bin������
        return User1BinFlashSizeMap[ucFlashSize];
    }
    else
    {
        LOG_OUT(LOGOUT_ERROR, "not support upgrade.");
        return 0;
    }
}

UINT32 UPGRADE_GetUser1BinAddr( VOID )
{
    UINT8 ucFlashSize = 0;

    ucFlashSize = system_get_flash_size_map();
    return User1BinFlashSizeMap[ucFlashSize];
}

UINT32 UPGRADE_GetUser2BinAddr( VOID )
{
    UINT8 ucFlashSize = 0;

    ucFlashSize = system_get_flash_size_map();
    return User2BinFlashSizeMap[ucFlashSize];
}

const CHAR* UPGRADE_GetFlashMap( VOID )
{
    UINT8 ucFlashSize = 0;
    ucFlashSize = system_get_flash_size_map();
    return szFlashMap[ucFlashSize];
}

VOID UPGRADE_SetUpgradeReboot()
{
    UINT32 uiAddr = 0;
    UINT8 UserBinNum = 0;

    uiAddr = UPGRADE_GetUpgradeUserBinAddr();

    LOG_OUT(LOGOUT_INFO, "system will upgrade reboot with 0x%X", uiAddr);
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);

#if IS_CHANG_XIN
    METER_RestartHandle();
#endif

    system_upgrade_reboot();
}

VOID UPGRADE_StartUpgradeRebootTimer()
{
    static xTimerHandle xUpgradeTimers = NULL;
    UINT32 uiUpgradeTimerId = 0;

    xUpgradeTimers = xTimerCreate("UPGRADE_StartUpgradeRebootTimer", 1000/portTICK_RATE_MS, FALSE, &uiUpgradeTimerId, UPGRADE_SetUpgradeReboot);
    if ( !xUpgradeTimers )
    {
        LOG_OUT(LOGOUT_ERROR, "xTimerCreate UPGRADE_SetUpgradeReboot failed.");
    }
    else
    {
        if(xTimerStart(xUpgradeTimers, 0) != pdPASS)
        {
            LOG_OUT(LOGOUT_ERROR, "xTimerCreate xUpgradeTimers start failed.");
        }
    }
}

VOID UPGRADE_SetReboot()
{
#if IS_CHANG_XIN
	METER_RestartHandle();
#endif
    system_restart();
}

VOID UPGRADE_StartRebootTimer()
{
    static xTimerHandle xRebootTimers = NULL;
    UINT32 uiRebootTimerId = 0;

    xRebootTimers = xTimerCreate("UPGRADE_StartRebootTimer", 500/portTICK_RATE_MS, FALSE, &uiRebootTimerId, UPGRADE_SetReboot);
    if ( !xRebootTimers )
    {
        LOG_OUT(LOGOUT_ERROR, "xTimerCreate UPGRADE_SetReboot failed.");
    }
    else
    {
        if(xTimerStart(xRebootTimers, 0) != pdPASS)
        {
            LOG_OUT(LOGOUT_ERROR, "xTimerCreate xRebootTimers start failed.");
        }
    }
}

VOID UPGRADE_Reset()
{
    LOG_OUT(LOGOUT_INFO, "system will reset and reboot");
    PLUG_TimerDataDeInit();
    PLUG_DelayDataDeInit();
    INFRARED_InfraredDataDeInit();
    PLUG_SystemSetDataDeInit();
    //HTTP_FileListInit();
    //PLUG_PlatformDeInit();
    METER_DeinitData();
    CONFIG_SaveConfig(PLUG_MOUDLE_BUFF);

    UPGRADE_StartRebootTimer();
}



