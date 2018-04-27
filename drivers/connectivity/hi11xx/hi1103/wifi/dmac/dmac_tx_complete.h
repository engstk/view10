

#ifndef __DMAC_TX_COMPLETE_H__
#define __DMAC_TX_COMPLETE_H__

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
#include "hal_ext_if.h"
#ifdef _PRE_WLAN_ALG_ENABLE
#include "alg_ext_if.h"
#endif
#include "mac_device.h"
#include "mac_vap.h"
#include "dmac_ext_if.h"
#include "dmac_user.h"
#include "dmac_vap.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_stat.h"
#include "dmac_main.h"
#include "oam_ext_if.h"
#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
#include "dmac_device.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TX_COMPLETE_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef _PRE_WLAN_INIT_PTK_TX_PN
#define DMAC_MAX_RA_LUT_INDEX   32
#define WLAN_CHIP_MAX_NUM       2
#endif

#define DMAC_DYN_CALI_MIN_POW   80
#define DMAC_DYN_CALI_MAX_POW   210

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
#define DMAC_DYN_CALI_PDET_MAX_LIMIT     400
#define DMAC_DYN_CALI_PDET_MIN_LIMIT    -512
#endif

/*****************************************************************************
  3 枚举定义
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
#ifdef _PRE_WLAN_INIT_PTK_TX_PN
typedef struct
{
    oal_uint8   uc_pn_peer_idx;     /* 对端peer索引,0~31 */
    oal_uint32  ul_pn_msb;          /* 发送描述符pn值的高32位 */
    oal_uint32  ul_pn_lsb;          /* 发送描述符pn值的低32位 */
}dmac_tx_dscr_pn_stru;

typedef struct
{
    oal_uint32  ul_pn_msb;          /* 发送描述符pn值的高32位 */
    oal_uint32  ul_pn_lsb;          /* 发送描述符pn值的低32位 */
    oal_uint32  ul_phy_mode_one;    /* 发送描述符phy mode字段 */
    oal_uint16  us_seq_num;    /* 发送描述符seq num */
    hal_tx_dscr_ctrl_one_param st_tx_dscr_one; /* 发送描述符中的ctrl one param字段 */
}dmac_tx_dscr_mac_phy_param_stru;
#endif

typedef struct
{
    oal_uint8 uc_tx_chiper_type;
    oal_uint8 uc_chiper_key_id;
}dmac_tx_chiper_stat_stru;

typedef struct dmac_dyn_cali_tx_pow_modul_diff       /*OFDM调制补偿结构体，只对20M以外带宽进行功率补偿*/
{
    oal_int16  s_dsss_20M_diff;
    oal_int16  s_ofdm_40M_diff;
    oal_int16  s_ofdm_80M_diff;
    oal_int16  s_ofdm_160M_diff;
}dmac_dyn_cali_tx_pow_modul_diff_stru;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern oal_uint8 g_uc_aggr_num_switch;     /* 设置最大AMPDU聚合个数开关 */
extern oal_uint8 g_uc_max_aggr_num;     /* 设置最大AMPDU聚合个数 */

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
#ifdef _PRE_DEBUG_MODE
OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_oam_report(dmac_user_stru* pst_dmac_user, hal_tx_dscr_stru* pst_dscr, oal_uint8 uc_dscr_num, oal_uint8 uc_dscr_status)
{
    hal_tx_dscr_stru            *pst_dscr_debug;
    hal_tx_dscr_stru            *pst_dscr_next_debug;
    oal_netbuf_stru             *pst_buf_debug;
    mac_tx_ctl_stru             *pst_cb_debug;
    oal_uint8                    uc_loop_debug;
    oal_switch_enum_uint8    en_frame_switch = 0;
    oal_switch_enum_uint8    en_cb_switch = 0;
    oal_switch_enum_uint8    en_dscr_switch = 0;
    oal_switch_enum_uint8    en_ota_switch = 0;
    oal_uint32               ul_ret;

    pst_dscr_debug = pst_dscr;
    pst_buf_debug = pst_dscr_debug->pst_skb_start_addr;
    pst_cb_debug  = (mac_tx_ctl_stru*)OAL_NETBUF_CB(pst_buf_debug);
    en_ota_switch = oam_report_data_get_global_switch(OAM_OTA_FRAME_DIRECTION_TYPE_TX);

    if (OAL_SWITCH_ON == en_ota_switch)
    {
        ul_ret = dmac_tx_dump_get_switch(OAM_USER_TRACK_FRAME_TYPE_DATA,
                                &en_frame_switch,
                                &en_cb_switch,
                                &en_dscr_switch,
                                pst_cb_debug);
        if(OAL_SUCC != ul_ret)
        {
            return;
        }
        if (OAL_SWITCH_ON == en_dscr_switch)
        {
            oam_report_dscr(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                            (oal_uint8*)pst_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
        }
    }

    pst_dscr_debug = pst_dscr;

    for (uc_loop_debug = 0; uc_loop_debug < uc_dscr_num; uc_loop_debug++)
    {
        pst_buf_debug = pst_dscr_debug->pst_skb_start_addr;
        pst_cb_debug  = (mac_tx_ctl_stru*)OAL_NETBUF_CB(pst_buf_debug);

        if (DMAC_TX_AMPDU_MISMATCH == uc_dscr_status)
        {
            oam_report_dscr(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                            (oal_uint8*)pst_dscr_debug, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
        }

        if ((OAL_SWITCH_ON == en_ota_switch)  && (OAL_SWITCH_ON == en_frame_switch))
        {
            oam_report_80211_frame(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                                   (oal_uint8*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb_debug),
                                   MAC_GET_CB_FRAME_HEADER_LENGTH(pst_cb_debug),
                                   oal_netbuf_payload(pst_buf_debug),
                                   MAC_GET_CB_FRAME_HEADER_LENGTH(pst_cb_debug) + MAC_GET_CB_MPDU_LEN(pst_cb_debug),
                                   OAM_OTA_FRAME_DIRECTION_TYPE_TX);
        }

        pst_dscr_next_debug = OAL_DLIST_GET_ENTRY(pst_dscr_debug->st_entry.pst_next, hal_tx_dscr_stru, st_entry);
        pst_dscr_debug      = pst_dscr_next_debug;
    }

    return;
}


OAL_STATIC OAL_INLINE oal_void  dmac_tx_complete_get_ampdu_len(
                                        hal_to_dmac_device_stru * pst_hal_device,
                                        hal_tx_dscr_stru *pst_dscr,
                                        oal_uint32       *pul_ampdu_len)
{
    hal_tx_get_ampdu_len(pst_hal_device, pst_dscr, pul_ampdu_len);
}

#endif

OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_dft_stat_fail_incr(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_uint8 uc_tid)
{
    DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.uc_tx_successive_mpdu_fail_num, 1);
    DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_ampdu_fail_num, 1);

    OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_ampdu_fail_num, 1);
    OAM_STAT_TID_INCR(pst_dmac_user->st_user_base_info.us_assoc_id, uc_tid, tx_ampdu_fail_num, 1);

    return;
}

OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_dft_stat_succ_incr(dmac_vap_stru* pst_dmac_vap, dmac_user_stru* pst_dmac_user, hal_tx_dscr_stru *pst_dscr, oal_uint8 uc_dscr_num, oal_uint8 uc_tid)
{
#ifdef _PRE_DEBUG_MODE
    oal_uint32                   ul_ampdu_len;
#endif

    DMAC_VAP_DFT_STATS_PKT_SET_ZERO(pst_dmac_vap->st_query_stats.uc_tx_successive_mpdu_fail_num);
    DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_ampdu_succ_num, 1);
    DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_mpdu_in_ampdu, uc_dscr_num);
#ifdef _PRE_WLAN_DFT_STAT

    if ((uc_dscr_num >= DMAC_AMPDU_LENGTH_COUNT_LEVEL_1) && (uc_dscr_num <= DMAC_AMPDU_LENGTH_COUNT_LEVEL_14))
    {
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.aul_tx_count_per_apmpdu_length[DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_0], 1);
    }
    else if ((uc_dscr_num >= DMAC_AMPDU_LENGTH_COUNT_LEVEL_15) && (uc_dscr_num <= DMAC_AMPDU_LENGTH_COUNT_LEVEL_17))
    {
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.aul_tx_count_per_apmpdu_length[DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_1], 1);
    }
    else if ((uc_dscr_num >= DMAC_AMPDU_LENGTH_COUNT_LEVEL_18) && (uc_dscr_num <= DMAC_AMPDU_LENGTH_COUNT_LEVEL_30))
    {
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.aul_tx_count_per_apmpdu_length[DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_2], 1);
    }
    else if ((uc_dscr_num >= DMAC_AMPDU_LENGTH_COUNT_LEVEL_31) && (uc_dscr_num <= DMAC_AMPDU_LENGTH_COUNT_LEVEL_32))
    {
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.aul_tx_count_per_apmpdu_length[DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_3], 1);
    }

#endif

#ifdef _PRE_DEBUG_MODE
    /* 发送AMPDU成功的相关统计信息 */
    dmac_tx_complete_get_ampdu_len(pst_dmac_vap->pst_hal_device, pst_dscr, &ul_ampdu_len);
    OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_ampdu_succ_num, 1);
    OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_ampdu_bytes, ul_ampdu_len);
    OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_mpdu_in_ampdu, uc_dscr_num);
    OAM_STAT_TID_INCR(pst_dmac_user->st_user_base_info.us_assoc_id, uc_tid, tx_ampdu_succ_num, 1);
    OAM_STAT_TID_INCR(pst_dmac_user->st_user_base_info.us_assoc_id, uc_tid, tx_ampdu_bytes, ul_ampdu_len);
    OAM_STAT_TID_INCR(pst_dmac_user->st_user_base_info.us_assoc_id, uc_tid, tx_mpdu_in_ampdu, uc_dscr_num);
#endif
    return;
}

OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_succ_stat(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, mac_tx_ctl_stru *pst_cb)
{
    if (FRW_EVENT_TYPE_HOST_DRX == MAC_GET_CB_EVENT_TYPE(pst_cb))
    {
        DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_hw_tx_pkts, MAC_GET_CB_NETBUF_NUM(pst_cb));
        DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_hw_tx_bytes, MAC_GET_CB_MPDU_LEN(pst_cb));
        DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_hw_tx_pkts, MAC_GET_CB_NETBUF_NUM(pst_cb));
        DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_hw_tx_bytes, MAC_GET_CB_MPDU_LEN(pst_cb));
    }
}

OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_hw_fail_stat(dmac_user_stru *pst_dmac_user, mac_tx_ctl_stru *pst_cb, hal_tx_dscr_ctrl_one_param  *pst_tx_dscr_one)
{
    pst_tx_dscr_one->uc_error_mpdu_num++;

    if (FRW_EVENT_TYPE_HOST_DRX == MAC_GET_CB_EVENT_TYPE(pst_cb))
    {
        DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_hw_tx_failed, 1);
    }
}

OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_fail_stat(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, mac_tx_ctl_stru *pst_cb)
{
    if (FRW_EVENT_TYPE_HOST_DRX == MAC_GET_CB_EVENT_TYPE(pst_cb))
    {
        DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_tx_failed, 1);
        DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_failed,  1);
    }
}



/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_void  dmac_tx_get_vap_id(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_vap_id);
extern oal_uint32  dmac_tx_complete_event_handler(frw_event_mem_stru *pst_event_mem);
extern oal_void dmac_tx_reset_flush(hal_to_dmac_device_stru *pst_device);
#if 0
extern oal_void  dmac_tx_delete_dscr(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_delete_dscr);
#endif
extern oal_void dmac_tx_hw_back_to_queue(hal_to_dmac_device_stru *pst_hal_device,
                                                hal_tx_dscr_stru      *pst_tx_dscr,
                                                mac_tx_ctl_stru       *pst_tx_ctl,
                                                oal_dlist_head_stru   *pst_tx_dscr_list_hdr);
extern oal_uint32  dmac_tx_complete_buff(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr);
extern oal_uint32  dmac_mgmt_tx_complete(mac_vap_stru *pst_mac_vap, oal_uint8 mgmt_frame_id, oal_uint32 uc_dscr_status, oal_uint16 us_user_idx);
extern oal_void  dmac_tx_complete_free_dscr(hal_tx_dscr_stru *pst_dscr);
extern oal_void  dmac_tx_complete_free_dscr_list(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr, oal_uint8 uc_dscr_num, oal_uint8 en_set_flag);
extern oal_uint32  dmac_tx_complete_dump_dscr(hal_to_dmac_device_stru *pst_hal_device,
                                          hal_tx_dscr_stru        *pst_base_dscr);
extern oal_void  dmac_tx_delete_ba(dmac_user_stru *pst_dmac_user);

#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
extern oal_uint32  dmac_pkt_cap_tx(dmac_packet_stru *pst_dmac_packet, hal_to_dmac_device_stru *pst_hal_device);
#endif
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
extern oal_void dmac_tx_dump_dscr_list(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr);
extern oal_void dmac_tx_complete_hw_seq_num_err(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru* pst_dscr, oal_uint8 uc_dscr_num, oal_uint8 uc_q_num);
#endif
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
extern oal_void dmac_tx_restore_txq_to_tid(hal_to_dmac_device_stru *pst_hal_device,
                                             dmac_user_stru    *pst_dmac_user,
                                             oal_uint8          uc_tid);
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_tx_complete.h */
