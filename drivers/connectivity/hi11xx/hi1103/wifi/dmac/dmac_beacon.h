

#ifndef __DMAC_BEACON_H__
#define __DMAC_BEACON_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oam_ext_if.h"
#include "dmac_vap.h"
#include "mac_resource.h"
#include "oal_ext_if.h"
#include "mac_vap.h"
#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
#include "dmac_device.h"
#endif


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_BEACON_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMAC_WMM_QOS_PARAMS_HDR_LEN 8
#define DMAC_MAX_WAIT_BEACON_TIMES      5

#define WLAN_PROTECTION_NON_ERP_AGING_THRESHOLD        (60)   /*ERP 老化计数最大值*/
#define WLAN_PROTECTION_NON_HT_AGING_THRESHOLD         (60)   /*HT 老化计数最大值*/

/*接收或者发送成功门限增值*/
#define LINKLOSS_THRESHOLD_INCR 1
/*用于计算linkloss门限的最大beacon周期*/
#define LINKLOSS_THRESHOLD_BEACON_MAX_INTVAL 400

#define RX_DATA_RATE                            (50000) /* 50M */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern OAL_CONST mac_freq_channel_map_stru g_dmac_ast_freq_map_2g[MAC_CHANNEL_FREQ_2_BUTT];


typedef oal_uint8  (*chan_event)(frw_event_mem_stru *pst_event, mac_vap_stru *pst_mac_vap, dmac_set_chan_stru *pst_set_chan);

typedef struct
{
    chan_event   chan_event_cb;

}dmac_beacon_cb;
extern dmac_beacon_cb   g_st_beacon_rom_cb;

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* beacon帧需要更新的信息元素偏移量 */
typedef struct
{
    oal_uint16   us_cap_offset;
    oal_uint16   us_bss_load_offset;
    oal_uint16   us_wmm_offset;
    oal_uint16   us_ht_cap_offset;
    oal_uint16   us_ht_operation_offset;
    oal_uint16   us_erp_offset;
    oal_uint16   us_pwr_constrain_offset;
    oal_uint16   us_tpc_report_offset;
    oal_uint16   us_tim_offset;
    oal_uint8    uc_wmm_update_flag;
    oal_uint8    auc_resv[1];
}dmac_beacon_ie_offset_stru;


/* 仅用于beacon测试 */
#ifdef _PRE_WLAN_CHIP_TEST
typedef struct
{
    oal_uint8 uc_test_type;
    oal_uint8 uc_test_flag;
    oal_uint8 uc_host_sleep;
    oal_uint8 uc_opmode_in_beacon;
    oal_uint8 uc_opmode;
    oal_uint8 uc_csa_in_beacon;
    oal_uint8 uc_announced_channel;
    oal_uint8 uc_switch_cnt;
}dmac_beacon_test_stru;

extern dmac_beacon_test_stru g_beacon_offload_test; /*仅用于测试*/
#endif

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32 dmac_sta_set_bandwith_handler(dmac_vap_stru  *pst_dmac_vap, wlan_channel_bandwidth_enum_uint8 en_sta_new_bandwidth);
extern oal_uint32  dmac_beacon_alloc(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32  dmac_beacon_free(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32  dmac_tbtt_event_handler(frw_event_mem_stru *pst_event_mem);
extern oal_void dmac_set_tim_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len);
#ifdef _PRE_WLAN_FEATURE_P2P
extern oal_uint32  dmac_beacon_set_p2p_noa(
                mac_vap_stru           *pst_mac_vap,
                oal_uint32              ul_start_tsf,
                oal_uint32              ul_duration,
                oal_uint32              ul_interval,
                oal_uint8               uc_count);
#endif
extern oal_void  dmac_vap_linkloss_channel_clean(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_uint32  dmac_sta_up_rx_beacon(
                dmac_vap_stru   *pst_dmac_vap,
                oal_netbuf_stru *pst_netbuf,
                oal_uint8       *pen_go_on);
extern oal_void dmac_init_dtim_count_ap(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32  dmac_encap_beacon(
                dmac_vap_stru               *pst_dmac_vap,
                oal_uint8                   *puc_beacon_buf,
                oal_uint16                  *pus_beacon_len);
#ifdef _PRE_WLAN_MAC_BUGFIX_TSF_SYNC
extern oal_void dmac_get_tsf_from_bcn(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
extern oal_uint32 dmac_sta_roam_trigger_init(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32 dmac_sta_roam_trigger_check(dmac_vap_stru *pst_dmac_vap);
#endif //_PRE_WLAN_FEATURE_ROAM
extern void dmac_sta_beacon_offload_test(mac_vap_stru *pst_mac_vap, oal_uint8 *uc_host_sleep);
extern oal_uint32  dmac_beacon_timeout_event_hander(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_send_disasoc_misc_event(mac_vap_stru *pst_mac_vap, oal_uint16 us_user_idx, oal_uint16 us_disasoc_reason);
extern oal_uint32 dmac_protection_update_mib_ap(mac_vap_stru *pst_mac_vap);
extern oal_uint32 dmac_protection_update_mode_ap(mac_vap_stru *pst_mac_vap);
extern oal_uint32 dmac_protection_update_mode_sta(mac_vap_stru *pst_mac_vap_sta, dmac_user_stru *pst_dmac_user);
extern oal_void dmac_protection_obss_aging_ap(mac_vap_stru *pst_mac_vap);
extern oal_uint32 dmac_protection_del_user(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_uint32 dmac_protection_set_autoprot(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_mode);
extern oal_void dmac_protection_start_timer(dmac_vap_stru  *pst_dmac_vap);
extern oal_void dmac_protection_stop_timer(dmac_vap_stru  *pst_dmac_vap);
extern oal_uint32  dmac_protection_obss_update_timer(void *p_arg);
extern oal_uint32 dmac_set_protection_mode(mac_vap_stru *pst_mac_vap, wlan_prot_mode_enum_uint8 en_prot_mode);
extern oal_void  dmac_ap_up_rx_obss_beacon(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_void dmac_ap_wait_start_rx_obss_beacon(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_uint32  dmac_chan_sync_event(mac_vap_stru *pst_mac_vap, dmac_set_chan_stru *pst_set_chan);
extern oal_uint32  dmac_ch_status_sync_event(mac_vap_stru *pst_mac_vap, mac_ap_ch_info_stru *past_ap_channel_list);

#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
extern oal_uint32 dmac_pkt_cap_beacon(dmac_packet_stru *pst_dmac_packet, dmac_vap_stru *pst_dmac_vap);
#endif
extern oal_void dmac_vap_linkloss_threshold_incr(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_vap_linkloss_threshold_decr(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_vap_linkloss_init(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_vap_linkloss_clean(dmac_vap_stru *pst_dmac_vap);

extern oal_uint32  dmac_protection_sync_event(mac_vap_stru *pst_mac_vap, mac_h2d_protection_stru *pst_h2d_prot);
extern oal_void dmac_update_dtim_count_ap(dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_tbtt_exception_handler(hal_to_dmac_device_stru  *pst_hal_device);
extern oal_bool_enum_uint8  dmac_sta_11ntxbf_is_changed(mac_user_stru *pst_mac_user,oal_uint8 *puc_frame_body,oal_uint16  us_frame_len);
extern oal_void dmac_sta_up_update_protection_mode(dmac_vap_stru *pst_dmac_vap,mac_user_stru *pst_mac_user);
extern oal_bool_enum_uint8 dmac_sta_edca_is_changed(
                mac_vap_stru  *pst_mac_vap,
                oal_uint8     *puc_frame_body,
                oal_uint16    us_frame_len);
extern oal_void dmac_sta_update_slottime(mac_vap_stru* pst_mac_vap, mac_user_stru *pst_mac_user, oal_uint8 *puc_payload, oal_uint16 us_msg_len);
extern oal_void dmac_sta_up_process_erp_ie(
                    oal_uint8               *puc_payload,
                    oal_uint16               us_msg_len,
                    mac_user_stru           *pst_mac_user);

extern oal_uint32 dmac_protection_set_erp_protection(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_flag);
extern oal_uint32 dmac_protection_set_ht_protection(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_flag);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
extern oal_uint32  dmac_irq_tbtt_ap_isr(oal_uint8 uc_mac_vap_id);
#endif /* #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) */
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_beacon.h */
