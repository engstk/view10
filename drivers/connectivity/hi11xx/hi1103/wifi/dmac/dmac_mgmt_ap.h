

#ifndef __DMAC_MGMT_AP_H__
#define __DMAC_MGMT_AP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "dmac_vap.h"
#include "dmac_chan_mgmt.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_MGMT_AP_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
#define MAC_SENT_PROBE_RESPONSE_CNT 50
#endif


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
typedef oal_uint16  (*encap_probe_rsp)(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf, oal_uint8 *puc_data);


typedef struct
{
    encap_probe_rsp encap_probe_rsp_cb;

}dmac_mgmt_ap_cb;
extern dmac_mgmt_ap_cb g_st_dmac_mgmt_ap_rom_cb;
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
extern oal_uint32  dmac_ap_up_rx_probe_req(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf,oal_uint8 *puc_addr,oal_int8 c_rssi);
extern oal_uint16  dmac_mgmt_encap_probe_response(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf, oal_uint8 *puc_ra, oal_bool_enum_uint8 en_is_p2p_req);
extern oal_void  dmac_ap_up_rx_2040_coext(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_void  dmac_chan_multi_switch_to_20MHz_ap(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32 dmac_send_notify_chan_width(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_data);
extern oal_bool_enum_uint8  dmac_ap_check_probe_req(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_probe_req, mac_ieee80211_frame_stru *pst_frame_hdr);

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
extern oal_bool_enum_uint8  dmac_ap_check_probe_ratio(dmac_vap_stru *pst_dmac_vap, mac_ieee80211_frame_stru *pst_frame_hdr);
extern oal_bool_enum_uint8 dmac_ap_check_probe_rej_sta(dmac_vap_stru *pst_dmac_vap, mac_ieee80211_frame_stru *pst_frame_hdr);
#endif

extern oal_uint32  dmac_chan_select_channel_for_operation(
                                            mac_vap_stru                        *pst_mac_vap,
                                            oal_uint8                           *puc_new_channel,
                                            wlan_channel_bandwidth_enum_uint8   *pen_new_bandwidth);
extern oal_bool_enum_uint8  dmac_chan_is_40MHz_scb_allowed(
                mac_vap_stru                 *pst_mac_vap,
                dmac_eval_scan_report_stru   *pst_chan_scan_report,
                oal_uint8                     uc_pri_chan_idx,
                mac_sec_ch_off_enum_uint8     en_user_chan_offset);
extern oal_void  dmac_chan_init_chan_scan_report(
                mac_vap_stru                 *pst_mac_vap,
                dmac_eval_scan_report_stru   *pst_chan_scan_report,
                oal_uint8                     uc_num_supp_chan);
extern oal_bool_enum_uint8  dmac_chan_is_40MHz_sca_allowed(
                mac_vap_stru                 *pst_mac_vap,
                dmac_eval_scan_report_stru   *pst_chan_scan_report,
                oal_uint8                     uc_pri_chan_idx,
                mac_sec_ch_off_enum_uint8     en_user_chan_offset);
extern oal_uint16  dmac_chan_get_cumulative_networks(
                mac_device_stru                     *pst_mac_device,
                wlan_channel_bandwidth_enum_uint8    en_band,
                oal_uint8                            uc_pri_chan_idx);

/*****************************************************************************
  11 inline函数定义
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void  dmac_chan_initiate_switch_to_20MHz_ap(mac_vap_stru *pst_mac_vap)
{

    /* 设置VAP带宽模式为20MHz */
    pst_mac_vap->st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;

    /* 设置带宽切换状态变量，表明在下一个DTIM时刻切换至20MHz运行 */
    pst_mac_vap->st_ch_switch_info.en_bw_switch_status = WLAN_BW_SWITCH_40_TO_20;

}





#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_mgmt_ap.h */
