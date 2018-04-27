

#ifndef __DMAC_BTCOEX_H__
#define __DMAC_BTCOEX_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hal_ext_if.h"
#include "mac_resource.h"
#include "mac_board.h"
#include "mac_frame.h"
#include "dmac_ext_if.h"
#include "dmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_BTCOEX_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define WLAN_TIDNO_COEX_QOSNULL     WLAN_TIDNO_ANT_TRAINING_HIGH_PRIO   /* 发送qos null复用智能天线高优先级训练帧 */

#define BTCOEX_RSSI_THRESHOLD               (WLAN_FAR_DISTANCE_RSSI)

#define BTCOEX_RX_LOW_RATE_TIME             (5000)
#define BTCOEX_SCO_CALCULATE_TIME           (500)

#define BTCOEX_PRIO_TIMEOUT_150MS           (150)    //软件定时器操作精度是ms
#define BTCOEX_PRIO_TIMEOUT_100MS           (100)    //软件定时器操作精度是ms
#define BTCOEX_PRIO_TIMEOUT_60MS            (60)

#define BTCOEX_PRIO_TIMEOUT_ALWAYS_ON       (0xffff) //持续拉高
#define BTCOEX_PRIO_TIMEOUT_20MS            (20000)  //硬件单位为us
#define BTCOEX_PRIO_TIMEOUT_10MS            (10000)
#define BTCOEX_PRIO_TIMEOUT_5MS             (5000)
#define BTCOEX_PRIO_TIMEOUT_ALWAYS_OFF      (0x0)    //拉低

#define ALL_MID_PRIO_TIME                   (10)    // 5s / BTCOEX_SCO_CALCULATE_TIME
#define ALL_HIGH_PRIO_TIME                  (4)     // 2s / BTCOEX_SCO_CALCULATE_TIME

#define BTCOEX_LINKLOSS_OCCUPIED_NUMBER     (8)
#define BTCOEX_POW_SAVE_CNT                 (5)    /* 连续处于ps=1状态次数，超过时，wifi强制恢复(用于低功耗唤醒时) */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

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
    mac_header_frame_control_stru   st_frame_control;
    oal_uint16                      bit_duration_value      : 15,           /* duration/id */
                                    bit_duration_flag       : 1;
    oal_uint8                       auc_address1[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address2[WLAN_MAC_ADDR_LEN];
    oal_uint8                       auc_address3[WLAN_MAC_ADDR_LEN];
    oal_uint16                      bit_sc_frag_num     : 4,                /* sequence control */
                                    bit_sc_seq_num      : 12;
    oal_uint8                       bit_qc_tid          : 4,
                                    bit_qc_eosp         : 1,
                                    bit_qc_ack_polocy   : 2,
                                    bit_qc_amsdu        : 1;

} dmac_btcoex_qosnull_frame_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32 dmac_btcoex_init(oal_void);
extern oal_uint32 dmac_btcoex_exit(oal_void);
extern oal_void dmac_btcoex_set_mgmt_priority(mac_vap_stru *pst_mac_vap, oal_uint16 us_timeout_ms);
extern oal_void dmac_btcoex_perf_param_show(oal_void);
extern oal_void dmac_btcoex_perf_param_update(mac_btcoex_mgr_stru *pst_btcoex_mgr);
extern oal_void dmac_btcoex_set_occupied_period(dmac_vap_stru *pst_dmac_vap, oal_uint16 us_occupied_period);
extern oal_void dmac_btcoex_set_freq_and_work_state_hal_device(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void dmac_btcoex_init_preempt(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_void dmac_btcoex_set_wlan_priority(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_value, oal_uint8 uc_timeout_ms);
extern oal_uint32 dmac_btcoex_wlan_priority_timeout_callback(oal_void *p_arg);
extern oal_uint32 dmac_btcoex_mgmt_priority_timeout_callback(oal_void *p_arg);
extern oal_void dmac_btcoex_rx_mgmt_occupied_check(mac_ieee80211_frame_stru *pst_frame_hdr, dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_btcoex_tx_mgmt_process(dmac_vap_stru *pst_dmac_vap, mac_ieee80211_frame_stru *pst_mac_header);
extern oal_void dmac_btcoex_change_state_syn(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_btcoex_vap_up_handle(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_btcoex_user_spatial_stream_change_notify(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_void dmac_btcoex_delba_trigger(mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_need_delba, dmac_user_btcoex_delba_stru  *pst_dmac_user_btcoex_delba);
extern oal_void dmac_btcoex_release_rx_prot(mac_vap_stru *pst_mac_vap, oal_uint8 uc_data_type);
extern oal_void dmac_btcoex_tx_mgmt_frame(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_netbuf_mgmt);
extern oal_void dmac_btcoex_tx_vip_frame(mac_vap_stru *pst_mac_vap, oal_dlist_head_stru *pst_tx_dscr_list_hdr);
extern oal_uint32 dmac_btcoex_wlan_occupied_timeout_callback(oal_void *p_arg);
extern oal_void dmac_btcoex_tx_addba_rsp_check(oal_netbuf_stru *pst_netbuf, mac_user_stru *pst_mac_user);
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
extern oal_uint32 dmac_config_set_device_freq(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
extern oal_uint32 dmac_btcoex_rx_data_bt_acl_check(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_btcoex_update_ba_size(mac_vap_stru *pst_mac_vap,
    dmac_user_btcoex_delba_stru *pst_dmac_user_btcoex_delba, hal_btcoex_btble_status_stru *pst_btble_status);
extern oal_uint32 dmac_btcoex_rx_rate_statistics_flag_callback(oal_void *p_arg);
extern oal_uint32 dmac_btcoex_sco_rx_rate_statistics_flag_callback(oal_void *p_arg);
extern oal_void dmac_btcoex_update_rx_rate_threshold (mac_vap_stru *pst_mac_vap, dmac_user_btcoex_delba_stru *pst_dmac_user_btcoex_delba);
extern oal_void dmac_btcoex_linkloss_init(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_btcoex_linkloss_update_threshold(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_btcoex_beacon_occupied_handler(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_btcoex_beacon_miss_handler(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void dmac_btcoex_linkloss_occupied_process(
      hal_to_dmac_chip_stru *pst_hal_chip, hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_btcoex_rx_rate_process_check(mac_vap_stru *pst_mac_vap,
      oal_uint8 uc_frame_subtype, oal_uint8 uc_data_type, oal_bool_enum_uint8 en_ampdu);
extern oal_uint32 dmac_config_btcoex_assoc_state_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_uint32 dmac_config_btcoex_disassoc_state_syn(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_btcoex_roam_succ_handler(hal_to_dmac_device_stru *pst_hal_device, mac_vap_stru *pst_mac_vap);
extern oal_void dmac_btcoex_clear_fake_queue_check(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_btcoex_ps_stop_check_and_notify(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void dmac_btcoex_ps_pause_check_and_notify(hal_to_dmac_device_stru *pst_hal_device);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
extern oal_uint32 dmac_btcoex_pow_save_callback(oal_void *p_arg);
extern oal_void dmac_btcoex_ps_powerdown_recover_handle(hal_to_dmac_device_stru *pst_hal_device);
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_BTCOEX */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_data_acq.h */
