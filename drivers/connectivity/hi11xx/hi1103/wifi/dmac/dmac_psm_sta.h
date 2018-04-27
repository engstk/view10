

#ifndef __DMAC_PSM_STA_H__
#define __DMAC_PSM_STA_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
#include "dmac_uapsd_sta.h"
#else
#include "dmac_sta_pm.h"
#include "dmac_ext_if.h"
#include "dmac_tx_bss_comm.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_PSM_STA_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMAC_DEFAULT_DTIM_LISTEN_DIFF 8        // DTIM LISTEN INTERVAL 比较中间量
#define DMAC_DEFAULT_LISTEN_INTERVAL  4        // 默认listen interval

/* 第一档超时TIMEOUT * CNT 必须大于100ms，否则丢包率高,体验差 */
#define DMAC_PSM_TIMER_FAST_TIMEOUT     20
#define DMAC_PSM_TIMER_FAST_CNT         2
#define DMAC_PSM_TIMER_NORMAL_CNT       10

#define DMAC_PSM_TIMER_IDLE_TIMEOUT     DMAC_PSM_TIMER_FAST_TIMEOUT
#define DMAC_PSM_TIMER_IDLE_CNT         DMAC_PSM_TIMER_FAST_CNT

#define DMAC_PSM_TIMER_BUSY_TIMEOUT     50
#define DMAC_PSM_TIMER_BUSY_CNT         8


#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
/* 单通道接收beacon的RSSI门限 */
#define DMAC_PSM_SINGLE_BCN_RX_CHAIN_TH  -73
#endif

#define DMAC_PSM_MAX_SKIP_BCN_CNT       5
/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern oal_uint32 g_device_wlan_pm_timeout;
extern oal_uint32 g_pm_timer_restart_cnt;
extern oal_uint32 g_ps_fast_check_cnt;
extern oal_uint8  g_uc_max_powersave;
extern oal_uint16 g_us_download_rate_limit_pps;

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef oal_void (*p_dmac_psm_update_dtime_period_cb)(mac_vap_stru *pst_mac_vap, oal_uint8 uc_mib_dtim_period,oal_uint32 ul_beacon_period);
typedef oal_void (*p_dmac_psm_update_keepalive_cb)(dmac_vap_stru *pst_dmac_vap);

typedef struct
{
    p_dmac_psm_update_dtime_period_cb                p_dmac_psm_update_dtime_period;
    p_dmac_psm_update_keepalive_cb                   p_dmac_psm_update_keepalive;
}dmac_psm_sta_rom_cb;

extern dmac_psm_sta_rom_cb g_st_dmac_psm_sta_rom_cb;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void dmac_psm_sta_incr_activity_cnt(mac_sta_pm_handler_stru *pst_sta_pm_handle)
{
    pst_sta_pm_handle->ul_tx_rx_activity_cnt++;
}

OAL_STATIC OAL_INLINE oal_uint8 dmac_psm_get_more_data_sta(mac_ieee80211_frame_stru *pst_frame_hdr)
{
    return (oal_uint8)(pst_frame_hdr->st_frame_control.bit_more_data);

}

OAL_STATIC OAL_INLINE oal_void dmac_psm_sta_reset_activity_cnt(mac_sta_pm_handler_stru *pst_sta_pm_handle)
{
    pst_sta_pm_handle->ul_tx_rx_activity_cnt = 0;
}

OAL_STATIC OAL_INLINE oal_uint8 dmac_psm_sta_is_activity_cnt_zero(mac_sta_pm_handler_stru *pst_sta_pm_handle)
{
    if (0 == pst_sta_pm_handle->ul_tx_rx_activity_cnt)
    {
      return OAL_TRUE;
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE oal_uint8 dmac_is_legacy_ac_present(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8               uc_ac;
    mac_cfg_uapsd_sta_stru  st_uapsd_cfg_sta;

    st_uapsd_cfg_sta = pst_dmac_vap->st_vap_base_info.st_sta_uapsd_cfg;

    for(uc_ac = 0; uc_ac < WLAN_WME_AC_BUTT; uc_ac++)
    {
        if((st_uapsd_cfg_sta.uc_trigger_enabled[uc_ac] == 0) &&
           (st_uapsd_cfg_sta.uc_delivery_enabled[uc_ac] == 0))
           return OAL_TRUE;
    }

    return OAL_FALSE;
}



OAL_STATIC OAL_INLINE oal_uint8 dmac_is_any_legacy_ac_present(dmac_vap_stru *pst_dmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    if(OAL_TRUE == dmac_is_uapsd_capable(pst_dmac_vap))
    {
        return dmac_is_legacy_ac_present(pst_dmac_vap);
    }
#endif /* _PRE_WLAN_FEATURE_STA_UAPSD */

    return OAL_TRUE;
}

/* This function checks if a PS Poll Response is pending */
OAL_STATIC OAL_INLINE oal_uint8 dmac_is_ps_poll_rsp_pending(dmac_vap_stru *pst_dmac_vap)
{
    return pst_dmac_vap->uc_ps_poll_pending;
}

/* This function sets the PS Poll Response pending flag */
OAL_STATIC OAL_INLINE oal_void dmac_set_ps_poll_rsp_pending(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_val)
{
    pst_dmac_vap->uc_ps_poll_pending = uc_val;
}


OAL_STATIC OAL_INLINE oal_void dmac_process_rx_process_data_sta_prot(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buf)
{
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    if (OAL_TRUE == dmac_is_uapsd_capable(pst_dmac_vap))
    {
        dmac_uapsd_rx_process_data_sta(pst_dmac_vap, pst_buf);
    }
#endif /* _PRE_WLAN_FEATURE_STA_UAPSD */

#ifdef _PRE_WLAN_FEATURE_P2P
    dmac_set_ps_poll_rsp_pending(pst_dmac_vap, OAL_FALSE);
#endif

}

OAL_STATIC OAL_INLINE oal_uint8 dmac_psm_process_tx_process_data_sta_prot(dmac_vap_stru *pst_dmac_vap, mac_tx_ctl_stru *pst_tx_ctl)
{
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    if (OAL_TRUE == dmac_is_uapsd_capable(pst_dmac_vap))
    {
        return dmac_uapsd_tx_process_data_sta(pst_dmac_vap, pst_tx_ctl);
    }
#endif /* _PRE_WLAN_FEATURE_STA_UAPSD */

    return STA_PWR_SAVE_STATE_ACTIVE;
}

OAL_STATIC OAL_INLINE oal_void dmac_psm_process_tx_complete_sta_prot(dmac_vap_stru *pst_dmac_vap, oal_uint8  uc_dscr_status, oal_netbuf_stru *pst_netbuf)
{
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    if(OAL_TRUE == dmac_is_uapsd_capable(pst_dmac_vap))
    {
        dmac_uapsd_process_tx_complete_sta(pst_dmac_vap, uc_dscr_status, pst_netbuf);
    }
#endif /* _PRE_WLAN_FEATURE_STA_UAPSD */
}

OAL_STATIC OAL_INLINE oal_void dmac_psm_process_tim_set_sta_prot(dmac_vap_stru *pst_dmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    if(OAL_TRUE == dmac_is_uapsd_capable(pst_dmac_vap))
    {
        dmac_uapsd_trigger_sp_sta(pst_dmac_vap);
    }
#endif /* _PRE_WLAN_FEATURE_STA_UAPSD */
}

OAL_STATIC OAL_INLINE oal_uint8 dmac_is_sta_fast_ps_enabled(mac_sta_pm_handler_stru  *pst_sta_pm_handle)
{
    if((oal_uint8)(pst_sta_pm_handle->uc_vap_ps_mode)  >= (oal_uint8)MIN_PSPOLL_PS)
    {
        return OAL_FALSE;
    }
    else
    {
        return OAL_TRUE;
    }
}


OAL_STATIC OAL_INLINE oal_void dmac_send_ps_poll_to_ap_prot(dmac_vap_stru *pst_dmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    dmac_set_ps_poll_rsp_pending(pst_dmac_vap, OAL_TRUE);
#endif /* #if  defined(_PRE_WLAN_FEATURE_P2P)*/
}


OAL_STATIC OAL_INLINE oal_void  dmac_psm_rx_inc_pkt_cnt(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_buf)
{
    mac_sta_pm_handler_stru  *pst_mac_sta_pm_handle;
    dmac_vap_stru               *pst_dmac_vap;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_mac_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;

    if (STA_PWR_SAVE_STATE_ACTIVE == GET_PM_STATE(pst_mac_sta_pm_handle))
    {
         pst_mac_sta_pm_handle->ul_psm_pkt_cnt++;
    }

    return;
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32 dmac_psm_get_max_sleep_time(dmac_vap_stru  *pst_dmac_vap);
extern oal_void dmac_psm_process_no_powersave(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_send_null_frame_to_ap_opt(dmac_vap_stru *pst_dmac_vap, oal_uint8  uc_psm, oal_bool_enum_uint8 en_qos);
extern oal_void dmac_psm_tx_set_power_mgmt_bit(dmac_vap_stru *pst_dmac_vap, mac_tx_ctl_stru *pst_tx_ctl);
extern oal_void dmac_psm_tx_complete_sta(dmac_vap_stru *pst_dmac_vap, hal_tx_dscr_stru  *pst_dscr, oal_netbuf_stru *pst_netbuf);
extern oal_void dmac_psm_process_tbtt_sta(dmac_vap_stru *pst_dmac_vap,mac_device_stru  *pst_mac_device);
extern oal_uint32 dmac_psm_rx_process_data_sta(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buf);
extern oal_uint8 dmac_psm_tx_process_data_sta(dmac_vap_stru *pst_dmac_vap, mac_tx_ctl_stru *pst_tx_ctl);
extern oal_void dmac_psm_max_powersave_enable(mac_device_stru *pst_mac_device);
extern oal_void dmac_psm_update_dtime_period(mac_vap_stru *pst_mac_vap, oal_uint8 uc_mib_dtim_period,oal_uint32 ul_beacon_period);
extern oal_void dmac_psm_update_keepalive(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32 dmac_send_null_frame_to_ap(dmac_vap_stru *pst_dmac_vap, oal_uint8  uc_psm, oal_bool_enum_uint8 en_qos);
extern oal_uint32 dmac_send_pspoll_to_ap(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_psm_start_activity_timer(dmac_vap_stru *pst_dmac_vap, mac_sta_pm_handler_stru *pst_sta_pm_handle);
extern oal_uint32 dmac_psm_is_tim_dtim_set(dmac_vap_stru *pst_dmac_vap, oal_uint8* puc_tim_elm);
extern oal_void dmac_psm_process_tim_elm(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_void  dmac_psm_recover_ps_abb_state(oal_void);
extern oal_uint32 dmac_psm_alarm_callback(void *p_arg);
extern oal_uint8  dmac_psm_get_state(dmac_vap_stru* pst_dmac_vap);
extern oal_uint8  dmac_psm_is_tid_queues_empty(dmac_vap_stru  *pst_dmac_vap);
extern oal_uint32 dmac_psm_process_fast_ps_state_change(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_psm);
extern oal_void dmac_psm_init_null_frm_cnt(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_psm_inc_null_frm_ofdm_succ(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_psm_dec_null_frm_ofdm_succ(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_psm_inc_null_frm(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_psm_dec_null_frm(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_psm_sync_dtim_count(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_dtim_count);
#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
extern oal_void dmac_psm_update_bcn_rf_chain(dmac_vap_stru *pst_dmac_vap, oal_int8 c_rssi);
#endif
extern oal_void  dmac_psm_update_bcn_tout_max_cnt(dmac_vap_stru  *pst_dmac_vap);
extern oal_uint32 dmac_bcn_no_frm_event_hander(frw_event_mem_stru *pst_event_mem);
#endif/*_PRE_WLAN_FEATURE_STA_PM*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_psm_sta.h */
