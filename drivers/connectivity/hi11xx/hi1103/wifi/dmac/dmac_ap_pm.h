

#ifndef __DMAC_AP_PM_H__
#define __DMAC_AP_PM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_AP_PM
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "mac_device.h"
#include "frw_ext_if.h"
#include "mac_pm.h"
#include "dmac_vap.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_AP_PM_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define AP_PWR_DEFAULT_INACTIVE_TIME    10                      /*默认inactive时间 minute*/
#define AP_PWR_DEFAULT_USR_CHECK_TIME   5000                    /*检查是否有用户在接入的定时器时间 ms*/
#define AP_PWR_MAX_USR_CHECK_COUNT      30000/AP_PWR_DEFAULT_USR_CHECK_TIME          /*30s内无用户接入切换状态*/



/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum {
    PWR_SAVE_STATE_WORK = 0,         /*工作状态*/
    PWR_SAVE_STATE_DEEP_SLEEP,      /*深睡状态*/
    PWR_SAVE_STATE_WOW,             /*WOW状态*/
    PWR_SAVE_STATE_IDLE,            /*idle状态，无用户关联*/
    PWR_SAVE_STATE_OFF,             /*下电状态*/

    PWR_SAVE_STATE_BUTT             /*最大状态*/
} ap_pwr_save_state_info;

typedef enum {
    AP_PWR_EVENT_POWER_OFF = 0,     /*下电事件*/
    AP_PWR_EVENT_POWER_ON,          /*上电事件*/
    AP_PWR_EVENT_WIFI_ENABLE,       /*使能wifi*/
    AP_PWR_EVENT_WIFI_DISABLE,      /*去使能wifi*/
    AP_PWR_EVENT_NO_USR,            /*无用户关联*/
    AP_PWR_EVENT_USR_ASSOC,         /*用户关联事件*/
    AP_PWR_EVENT_INACTIVE,          /*VAP不活跃*/
    AP_PWR_EVENT_ACTIVE,            /*VAP激活*/
    AP_PWR_EVENT_WOW_WAKE,          /* GPIO唤醒WOW */
    AP_PWR_EVENT_VAP_DOWN,          /* vap down事件 */
    AP_PWR_EVENT_STA_SCAN_CONNECT,  /* offload场景,sta 发起扫描或关联 */

    AP_PWR_EVENT_BUTT
}ap_pwr_save_event;


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct
{
    oal_uint16                       us_type;         /* 事件类型 */
    oal_uint16                       us_data_len;     /* 事件携带数据长度 */
    oal_uint32                       ul_data[1];      /* 数据长度小于等于4字节，直接承载数据,大于4字节，是一个指向数据的指针 */
}mac_pm_event_stru;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_void dmac_pm_ap_attach(dmac_vap_stru* pst_dmac_vap);
extern oal_void dmac_pm_ap_deattach(dmac_vap_stru* pst_dmac_vap);
extern oal_uint32 dmac_pm_post_event(dmac_vap_stru* pst_dmac_vap, oal_uint16 us_type, oal_uint16 us_datalen, oal_uint8* pst_data);
extern oal_uint32  ap_pm_wow_host_wake_event(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_pm_ap_sta_scan_connect_event(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);

extern oal_void dmac_pcie_ps_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_idle_state);

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hmac_fsm.h */
