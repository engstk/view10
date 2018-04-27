

#ifndef __DMAC_M2S_H__
#define __DMAC_M2S_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "mac_frame.h"
#include "mac_ie.h"
#include "dmac_device.h"
#include "dmac_user.h"
#include "dmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_M2S_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
#define M2S_MIMO_RX_CNT_THRESHOLD          2
#define M2S_SISO_RX_CNT_THRESHOLD          8
#define M2S_RX_UCAST_CNT_THRESHOLD         10

#define M2S_ACTION_SENT_CNT_THRESHOLD      3

#define M2S_RX_STATISTICS_START_TIME          1000

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


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_void  dmac_m2s_mgr_param_info(hal_to_dmac_device_stru *pst_hal_device);
extern wlan_nss_enum_uint8  dmac_m2s_get_bss_max_nss(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_netbuf,
                                                 oal_uint16 us_frame_len, oal_bool_enum_uint8 en_assoc_status);
extern oal_uint8 dmac_m2s_scan_get_num_sounding_dim(oal_netbuf_stru *pst_netbuf, oal_uint16 us_frame_len);
extern oal_bool_enum_uint8 dmac_m2s_get_bss_support_opmode(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_frame_body, oal_uint16 us_frame_len);
extern oal_uint32 dmac_m2s_d2h_device_info_syn(mac_device_stru *pst_mac_device);
extern oal_uint32 dmac_m2s_d2h_vap_info_syn(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_m2s_send_action_complete_check(mac_vap_stru *pst_mac_vap, mac_tx_ctl_stru *pst_tx_ctl);
extern oal_void  dmac_m2s_update_vap_capbility(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap);
extern oal_void  dmac_m2s_update_user_capbility(mac_user_stru *pst_mac_user, mac_vap_stru *pst_mac_vap);
extern oal_void  dmac_m2s_switch_update_vap_capbility(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void  dmac_m2s_switch_update_device_capbility(hal_to_dmac_device_stru *pst_hal_device, mac_device_stru *pst_mac_device);
extern oal_void dmac_m2s_switch_same_channel_vaps_begin(hal_to_dmac_device_stru *pst_hal_device,
                   mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap1, mac_vap_stru *pst_mac_vap2);
#ifdef _PRE_WLAN_FEATURE_DBAC
extern oal_void dmac_m2s_switch_dbac_vaps_begin(hal_to_dmac_device_stru *pst_hal_device,
                     mac_device_stru  *pst_mac_device, mac_vap_stru  *pst_mac_vap1, mac_vap_stru *pst_mac_vap2);
#ifdef _PRE_WLAN_FEATURE_DBDC
extern oal_void dmac_dbdc_switch_vaps_begin(hal_to_dmac_device_stru *pst_hal_device,
                     mac_device_stru  *pst_mac_device, mac_vap_stru  *pst_mac_vap1, mac_vap_stru *pst_mac_vap2);
extern oal_void dmac_dbdc_switch_vap_back(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap);
extern oal_void  dmac_dbdc_switch_vaps_back(mac_device_stru *pst_mac_device, mac_vap_stru *pst_keep_vap, mac_vap_stru *pst_shift_vap);
extern oal_uint32 dmac_dbdc_vap_reg_shift(dmac_device_stru *pst_dmac_device, mac_vap_stru *pst_shift_vap,
                        hal_to_dmac_device_stru  *pst_shift_hal_device, hal_to_dmac_vap_stru *pst_shift_hal_vap);
extern oal_uint32 dmac_dbdc_vap_hal_device_shift(dmac_device_stru  *pst_dmac_device, mac_vap_stru *pst_shift_vap);
extern oal_void dmac_dbdc_handle_stop_event(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void dmac_dbdc_start_renew_dev(hal_to_dmac_device_stru  *pst_master_hal_device);
extern oal_void dmac_dbdc_switch_vap_to_slave(dmac_device_stru  *pst_dmac_device, mac_vap_stru *pst_shift_mac_vap);
extern oal_void dmac_dbdc_renew_pm_tbtt_offset(dmac_dbdc_state_enum_uint8 en_dbdc_state);
#endif
#endif
extern oal_void dmac_m2s_show_blacklist_in_list(hal_to_dmac_device_stru *pst_hal_device);
extern oal_bool_enum_uint8 dmac_m2s_assoc_vap_find_in_device_blacklist(hal_to_dmac_device_stru *pst_hal_device);
extern oal_uint32 dmac_m2s_disassoc_state_syn(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_m2s_switch_protect_trigger(dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_m2s_send_action_frame(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_m2s_rx_rate_nss_process (mac_vap_stru *pst_vap, dmac_rx_ctl_stru *pst_cb_ctrl, mac_ieee80211_frame_stru *pst_frame_hdr);
extern oal_bool_enum_uint8  dmac_m2s_switch_apply_and_confirm(hal_to_dmac_device_stru *pst_hal_device,
    oal_uint16 us_m2s_type, wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode);
extern oal_uint32 dmac_m2s_switch_vap_off(hal_to_dmac_device_stru *pst_hal_device,
    mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap);
extern oal_uint32 dmac_m2s_switch_device_begin(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device);
extern oal_void  dmac_m2s_switch_device_end(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void dmac_m2s_nss_and_bw_alg_notify(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,
    oal_bool_enum_uint8 en_nss_change, oal_bool_enum_uint8 en_bw_change);
extern oal_uint32  dmac_m2s_switch(hal_to_dmac_device_stru       *pst_hal_device,
                                             wlan_nss_enum_uint8 en_nss,
                                             hal_m2s_event_tpye_uint16 en_m2s_event,
                                             oal_bool_enum_uint8   en_hw_switch);
extern oal_void dmac_m2s_mimo_to_miso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event);
extern oal_void dmac_m2s_mimo_to_siso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event);
extern oal_void dmac_m2s_siso_to_mimo(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event);
extern oal_void dmac_m2s_siso_to_miso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event);
extern oal_void dmac_m2s_miso_to_mimo(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event);
extern oal_void dmac_m2s_miso_to_siso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event);
extern oal_void dmac_m2s_siso_to_siso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event);
extern oal_uint32 dmac_m2s_handle_event(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_type, oal_uint16 us_datalen, oal_uint8* pst_data);
extern oal_void dmac_m2s_fsm_attach(hal_to_dmac_device_stru   *pst_hal_device);
extern oal_void dmac_m2s_fsm_detach(hal_to_dmac_device_stru   *pst_hal_device);
extern oal_void dmac_m2s_mgmt_switch(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_single_tx_chain);

#endif /* end of _PRE_WLAN_FEATURE_M2S */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_m2s.h */

