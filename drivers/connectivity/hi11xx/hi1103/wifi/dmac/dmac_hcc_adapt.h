

#ifndef __DMAC_HCC_ADAPT_H__
#define __DMAC_HCC_ADAPT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "dmac_ext_if.h"
#include "frw_ext_if.h"
#include "frw_event_main.h"
#include "hal_device.h"
#include "oal_hcc_slave_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_HCC_ADAPT_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/

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
/* 定义hcc层到驱动层的回调钩子 */
typedef oal_uint32 (*_release_tid_buffs_handle)(oal_void* arg1, oal_void* arg2, oal_uint32 cnt);


typedef struct
{
    _release_tid_buffs_handle    dmac_release_tid_buffs_func;
}dmac_hcc_adapt_handle;

extern dmac_hcc_adapt_handle    g_hcc_callback_handle;
extern dmac_hcc_adapt_handle   *g_pst_hcc_cb;


extern oal_uint32  g_hcc_sched_event_pkts[FRW_EVENT_TYPE_BUTT];

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_int32 dmac_rx_wifi_pre_action_function(oal_uint8 stype, hcc_netbuf_stru* pst_hcc_netbuf, oal_uint8  **pre_do_context);
extern oal_int32 dmac_rx_wifi_post_action_function(oal_uint8 stype, hcc_netbuf_stru* pst_hcc_netbuf, oal_uint8 *pst_context);

/*Rx适配部分*/
extern frw_event_mem_stru* dmac_process_rx_data_event_adapt_default(frw_event_mem_stru * pst_hcc_event_mem);
extern frw_event_mem_stru* dmac_hcc_rx_convert_netbuf_to_event_default(frw_event_mem_stru * pst_hcc_event_mem);
extern frw_event_mem_stru* dmac_event_config_syn_alg_rx_adapt(frw_event_mem_stru * pst_hcc_event_mem);
extern frw_event_mem_stru* dmac_scan_proc_scan_req_event_rx_adapt(frw_event_mem_stru *pst_hcc_event_mem);
extern frw_event_mem_stru *dmac_recv_event_netbuf_rx_adapt(frw_event_mem_stru *pst_hcc_event_mem);
extern frw_event_mem_stru* dmac_scan_proc_sched_scan_req_event_rx_adapt(frw_event_mem_stru *pst_hcc_event_mem);

/*Tx适配部分*/
extern oal_uint32 dmac_adapt_tx_param_and_netbuf_hdr_init(frw_event_mem_stru   *pst_event_mem, struct hcc_transfer_param* pst_param, oal_netbuf_stru  * pst_netbuf);
extern oal_uint32 dmac_proc_wlan_drx_event_tx_adapt_ram(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_proc_tkip_mic_fail_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_proc_crx_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_scan_report_scanned_bss_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_scan_proc_scan_comp_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_proc_chan_result_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_proc_init_event_process_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_proc_event_del_ba_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_hcc_tx_convert_event_to_netbuf_uint16(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_proc_event_config_syn_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_sdt_recv_reg_cmd_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_alg_ct_result_tx_adapt(frw_event_mem_stru *pst_event_mem);
//extern oal_uint32 dmac_proc_event_log_syn_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_proc_mgmt_rx_delba_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_rx_send_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_switch_to_new_chan_complete_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_dbac_status_notify_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_proc_disasoc_misc_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_cali2hmac_misc_event_tx_adapt(frw_event_mem_stru *pst_event_mem);

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
extern frw_event_mem_stru* dmac_config_update_ip_filter_rx_adapt(frw_event_mem_stru *pst_hcc_event_mem);
#endif //_PRE_WLAN_FEATURE_IP_FILTER

#ifdef _PRE_WLAN_ONLINE_DPD
extern oal_uint32 dmac_dpd_tx_adapt(frw_event_mem_stru *pst_event_mem);
#endif

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS) || defined(_PRE_WLAN_RF_AUTOCALI)
extern oal_uint32 dmac_sdt_recv_sample_cmd_tx_adapt(frw_event_mem_stru *pst_event_mem);
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
extern oal_uint32 dmac_proc_roam_trigger_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
#endif //_PRE_WLAN_FEATURE_ROAM

extern oal_uint32  dmac_alg_syn_info_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_alg_voice_aggr_adapt(frw_event_mem_stru *pst_event_mem);

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
extern oal_uint32 dmac_scan_proc_obss_scan_comp_event_tx_adapt(frw_event_mem_stru *pst_event_mem);
#endif

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
extern oal_uint32  dmac_alg_flow_tx_adapt_tx_adapt(frw_event_mem_stru * pst_event_mem);
#endif

extern oal_uint32  dmac_chan_sync_event_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_chan_protection_event_adapt(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_ch_status_event_adapt(frw_event_mem_stru *pst_event_mem);

#ifdef _PRE_WLAN_FEATURE_M2S
extern oal_uint32  dmac_m2s_sync_event_adapt(frw_event_mem_stru *pst_event_mem);
#endif
extern oal_uint32 dmac_hcc_tx_event_convert_to_netbuf(frw_event_mem_stru   *pst_event_mem,
                                               oal_netbuf_stru      *pst_event_netbuf,
                                               oal_uint32            payload_size);

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hmac_main */


