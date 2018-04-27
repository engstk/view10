

#ifndef __DMAC_STS_PM_H__
#define __DMAC_STS_PM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "mac_device.h"
#include "mac_frame.h"
#include "mac_pm.h"
#include "dmac_vap.h"
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_STA_PM_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WLAN_NBBY      8
//#define WLAN_TIM_ISSET(a,i) ((a)[(i)/WLAN_NBBY] & (1<<((i)%WLAN_NBBY)))
#define WLAN_TIM_ISSET(a,i) ((a)[(i)>>3] & (1<<((i)&0x7)))
#define LISTEN_INTERVAL_TO_DITM_TIMES   4
#define MIN_ACTIVITY_TIME_OUT 20000   /* 500 ms */
#define MAX_ACTIVITY_TIME_OUT 20000 /* 10 sec */
#define WLAN_PS_KEEPALIVE_MAX_NUM   300 /* 300 DTIM1 interval*/
#define WLAN_MAX_NULL_SENT_NUM      10  /* NULL帧最大重传次数 */

#define DMAC_TIMER_DOZE_TRANS_FAIL_NUM        10
#define DMAC_STATE_DOZE_TRANS_FAIL_NUM        2          //连续N次切doze失败输出维测
#define DMAC_BEACON_DOZE_TRANS_FAIL_NUM       2
#define DMAC_BEACON_TIMEOUT_MAX_TIME          1000 //收不到beacon最大睡眠时间,单位ms
#define DMAC_RECE_MCAST_TIMEOUT               20   //接收广播组播帧超时--没收到

#if defined(_PRE_DEBUG_MODE)
#define DMAC_STA_UAPSD_STATS_INCR(_member)    ((_member)++)
#define DMAC_STA_PSM_STATS_INCR(_member)      ((_member)++)
#elif defined(_PRE_PSM_DEBUG_MODE)
#define DMAC_STA_PSM_STATS_INCR(_member)    ((_member)++)
#define DMAC_STA_UAPSD_STATS_INCR(_member)
#else
#define DMAC_STA_UAPSD_STATS_INCR(_member)
#define DMAC_STA_PSM_STATS_INCR(_member)
#endif
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* TIM processing result */
typedef enum
{
    DMAC_TIM_IS_SET  = 1,
    DMAC_DTIM_IS_SET = 2,

    DMAC_TIM_PROC_BUTT
} dmac_tim_proc_enum;

typedef enum {
    STA_PWR_SAVE_STATE_ACTIVE = 0,         /* active状态 */
    STA_PWR_SAVE_STATE_DOZE,               /* doze状态 */
    STA_PWR_SAVE_STATE_AWAKE,              /* wake状态*/

    STA_PWR_SAVE_STATE_BUTT                /*最大状态*/
} sta_pwr_save_state_info;

typedef enum {
    STA_PWR_EVENT_TX_DATA = 0,              /* DOZE状态下的发送事件 */
    STA_PWR_EVENT_TBTT,                     /* DOZE状态下的TBTT事件 */
    STA_PWR_EVENT_FORCE_AWAKE,              /* DOZE状态下后tbtt唤醒还没上来,手动强制唤醒 */
    STA_PWR_EVENT_RX_UCAST,                 /* AWAKE状态下接收单播  */
    STA_PWR_EVENT_LAST_MCAST = 4,           /* AWAKE状态下最后一个组播/广播 */
    STA_PWR_EVENT_TIM,                      /* AWAKE状态下的TIM事件 */
    STA_PWR_EVENT_DTIM,                     /* AWAKE状态下的DTIM事件 */
    STA_PWR_EVENT_NORMAL_SLEEP,
    STA_PWR_EVENT_BEACON_TIMEOUT  = 8,      /* AWAKE状态下睡眠事件 */
    STA_PWR_EVENT_SEND_NULL_SUCCESS,        /* 三种状态下都有NULL帧发送成功事件 */
    STA_PWR_EVENT_TIMEOUT,                  /* ACTIVE/AWAKE状态下超时事件 */
    STA_PWR_EVENT_KEEPALIVE,                /* ACTIVE状态下非节能模式下keepalive事件 */
    STA_PWR_EVENT_NO_POWERSAVE = 12,        /* DOZE/AWAKE状态下非节能模式 */
    STA_PWR_EVENT_P2P_SLEEP,                /* P2P SLEEP */
    STA_PWR_EVENT_P2P_AWAKE,                /* P2P AWAKE */
    STA_PWR_EVENT_DETATCH,                  /* 销毁低功耗状态机事件 */
    STA_PWR_EVENT_DEASSOCIATE,              /* 去关联*/
    STA_PWR_EVENT_TX_MGMT,                  /* 发送管理帧开启前端 */
    STA_PWR_EVENT_TX_COMPLETE,              /* 发送完成事件 */
    STA_PWR_EVENT_ENABLE_FRONT_END,         /* 打开前端事件，状态机切到active状态 */
    STA_PWR_EVENT_RX_BEACON,                /* doze状态下收到beacon */
    STA_PWR_EVENT_RX_DATA,                  /* doze状态下收到数据帧 */

#ifdef _PRE_WLAN_DOWNLOAD_PM
    STA_PWR_EVENT_NOT_EXCEED_MAX_SLP_TIME,  /* 限流模式下距离上次开始丢弃接收帧的时间未超过最大允许睡眠时间事件 */
#endif
    STA_PWR_EVENT_NO_PS_FRM,
    STA_PWR_EVENT_BUTT
}sta_pwr_save_event;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern    oal_uint32   g_lightsleep_fe_awake_cnt;
extern    oal_uint32   g_deepsleep_fe_awake_cnt;
/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

typedef enum
{
    STA_PS_DEEP_SLEEP   = 0,
    STA_PS_LIGHT_SLEEP  = 1
}ps_mode_state_enum;

typedef  enum
{
    DMAC_SP_NOT_IN_PROGRESS         = 0,
    DMAC_SP_WAIT_START              = 1,
    DMAC_SP_IN_PROGRESS             = 2,
    DMAC_SP_UAPSD_STAT_BUTT
}dmac_uapsd_sp_stat_sta_enum;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_void dmac_pm_sta_state_trans(mac_sta_pm_handler_stru* pst_handler,oal_uint8 uc_state, oal_uint16 us_event);
extern oal_uint32 dmac_pm_sta_post_event(dmac_vap_stru *pst_dmac_vap, oal_uint16 us_type, oal_uint16 us_datalen, oal_uint8* pst_data);
extern oal_void dmac_pm_sta_attach(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_pm_sta_detach(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_sta_initialize_psm_globals(mac_sta_pm_handler_stru *p_handler);
extern oal_void dmac_pm_key_info_dump(dmac_vap_stru  *pst_dmac_vap);
extern oal_void dmac_pm_change_to_active_state(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_event);
extern oal_void dmac_pm_sta_wait_for_mcast(dmac_vap_stru *pst_dmac_vap, mac_sta_pm_handler_stru *pst_mac_sta_pm_handle);
#endif /* _PRE_WLAN_FEATURE_STA_PM*/
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_sts_pm.h */
