
#ifndef __DMAC_CHAN_MGMT_H__
#define __DMAC_CHAN_MGMT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "dmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CHAN_MGMT_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum
{
    DMAC_OP_ALLOWED  = BIT0,
    DMAC_SCA_ALLOWED = BIT1,
    DMAC_SCB_ALLOWED = BIT2,
}dmac_chan_op_enum;
typedef oal_uint8 dmac_chan_op_enum_uint8;

typedef enum
{
    DMAC_NETWORK_SCA = 0,
    DMAC_NETWORK_SCB = 1,

    DMAC_NETWORK_BUTT,
}dmac_network_type_enum;
typedef oal_uint8 dmac_network_type_enum_uint8;

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
    oal_uint16                 aus_num_networks[DMAC_NETWORK_BUTT];
    dmac_chan_op_enum_uint8    en_chan_op;
    oal_uint8                  auc_resv[3];
}dmac_eval_scan_report_stru;

typedef oal_uint8 (*p_dmac_dfs_radar_detect_event_cb)(frw_event_mem_stru *pst_event_mem);
typedef oal_uint8 (*p_dmac_ie_proc_ch_switch_ie_cb)(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, mac_eid_enum_uint8 en_eid_type);

typedef struct
{
    p_dmac_dfs_radar_detect_event_cb                p_dmac_dfs_radar_detect_event;
    p_dmac_ie_proc_ch_switch_ie_cb                  p_dmac_ie_proc_ch_switch_ie;
}dmac_chan_mgmt_rom_cb;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32  dmac_chan_initiate_switch_to_new_channel(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_chan_sync(frw_event_mem_stru *pst_event_mem);
extern oal_void  dmac_chan_attempt_new_chan(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_void dmac_chan_select_real_channel(mac_device_stru  *pst_mac_device, mac_channel_stru *pst_channel, oal_uint8 uc_dst_chan_num);
extern oal_void  dmac_chan_select_channel_mac(mac_vap_stru *pst_mac_vap, oal_uint8 uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_uint32  dmac_chan_disable_machw_tx_event_process(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_chan_enable_machw_tx_event_process(frw_event_mem_stru *pst_event_mem);
extern oal_void  dmac_chan_tx_complete_2040_coexist(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device, oal_netbuf_stru *pst_netbuf);
extern oal_uint32  dmac_chan_restart_network_after_switch_event(frw_event_mem_stru *pst_event_mem);
extern oal_void  dmac_chan_tx_complete_suspend_tx(mac_device_stru      *pst_mac_device,
                                             mac_vap_stru              *pst_mac_vap,
                                             hal_to_dmac_device_stru   *pst_hal_device,
                                             oal_netbuf_stru           *pst_netbuf);
extern oal_void  dmac_chan_disable_machw_tx(mac_vap_stru *pst_mac_vap);
extern oal_void  dmac_chan_enable_machw_tx(mac_vap_stru *pst_mac_vap);
extern oal_uint32 dmac_dfs_radar_detect_event(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_dfs_test(frw_event_mem_stru* pst_event_mem);
extern oal_void  dmac_switch_complete_notify(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_check_cac);
#ifdef _PRE_WLAN_FEATURE_OFFCHAN_CAC
extern oal_uint32  dmac_dfs_switch_to_offchan_event_process(frw_event_mem_stru* pst_event_mem);
extern oal_uint32  dmac_dfs_switch_back_event_process(frw_event_mem_stru* pst_event_mem);
#endif
extern oal_void  dmac_chan_update_user_bandwidth(mac_vap_stru *pst_mac_vap);
extern oal_uint32  dmac_ie_proc_csa_bw_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len);
extern oal_uint32  dmac_ie_proc_wide_bandwidth_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload);

extern oal_uint32  dmac_ie_proc_ch_switch_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, mac_eid_enum_uint8 en_eid_type);
extern oal_void    dmac_chan_update_csw_info(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len);
#ifdef _PRE_WLAN_FEATURE_DFS
extern oal_void  dmac_dfs_radar_detect_check(hal_to_dmac_device_stru *pst_hal_device, mac_device_stru *pt_mac_device, mac_vap_stru *pst_mac_vap);
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
extern oal_void hmac_chan_update_40M_intol_user(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_chan_start_40M_recovery_timer(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_chan_stop_40M_recovery_timer(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_chan_create_40M_recovery_timer(dmac_vap_stru *pst_dmac_vap);
extern oal_bool_enum_uint8  dmac_chan_get_2040_op_chan_list(mac_vap_stru *pst_mac_vap, dmac_eval_scan_report_stru *pst_chan_scan_report);
extern oal_uint32 dmac_chan_prepare_for_40M_recovery(dmac_vap_stru *pst_dmac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth);

#endif
extern oal_uint16 dmac_encap_notify_chan_width(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_data, oal_uint8 *puc_da);
extern oal_uint32 dmac_dump_chan(mac_vap_stru *pst_mac_vap, oal_uint8* puc_param);
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_chan_mgmt.h */
