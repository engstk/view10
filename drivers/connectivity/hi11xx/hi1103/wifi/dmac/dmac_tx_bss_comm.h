

#ifndef __DMAC_TX_BSS_COMM_H__
#define __DMAC_TX_BSS_COMM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hal_ext_if.h"
#include "oal_ext_if.h"
#ifdef _PRE_WLAN_ALG_ENABLE
#include "alg_ext_if.h"
#endif
#include "dmac_ext_if.h"
#include "dmac_vap.h"
#include "dmac_user.h"
#include "dmac_main.h"
#include "dmac_alg.h"
#include "wlan_types.h"
#include "oal_net.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TX_BSS_COMM_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMAC_MAX_FIFO_PUT       10  /* = FIFO DEHPTH + PREFETCH */
#define DMAC_MAX_PENDING_RETRY  6
#define DMAC_MAX_PROBE_RSP_PENDING_RETRY 1

#define DMAC_GET_USER_CURRENT_CHANNEL_BW(_pst_user) \
    ((WLAN_BW_CAP_20M == (_pst_user)->en_cur_bandwidth) ?  WLAN_BAND_ASSEMBLE_20M :   \
     (WLAN_BW_CAP_40M == (_pst_user)->en_cur_bandwidth) ?  WLAN_BAND_ASSEMBLE_40M :   \
     (WLAN_BW_CAP_80M == (_pst_user)->en_cur_bandwidth) ?  WLAN_BAND_ASSEMBLE_80M :   \
     (WLAN_BW_CAP_160M == (_pst_user)->en_cur_bandwidth) ? WLAN_BAND_ASSEMBLE_160M : \
     WLAN_BAND_ASSEMBLE_BUTT)

typedef oal_uint32 (*p_dmac_tx_parse_mpdu_func)(oal_netbuf_stru *pst_netbuf, hal_tx_msdu_address_params *past_msdu);

/* 是否满足发送调度条件 */
#define HAL_NEED_TX_SCHEDULE(hal_device, tid_queue, q)   \
    ((1 >= hal_device->ast_tx_dscr_queue[q].uc_ppdu_cnt) && (0 == tid_queue->uc_is_paused))

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#define DMAC_RCV_PKT_STAT_CNT(_tx_cnt)   \
    (g_tx_pkts_stat.ul_rcv_pkts += (_tx_cnt))

#define DMAC_SND_PKT_STAT_CNT(_tx_cnt)   \
    (g_tx_pkts_stat.ul_snd_pkts += (_tx_cnt))
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
/* 2用于4字节对齐 */
#define WLAN_LARGE_SKB_AMSDU_PAYLOAD_OFFSET    (ETHER_HDR_LEN + 2)
#endif
/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

#ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
extern oal_uint8 g_auc_default_mac_to_phy_protocol_mapping[WLAN_PROTOCOL_BUTT];
#endif

typedef oal_uint32 (*p_dmac_tx_excp_free_netbuf_cb)(oal_netbuf_stru *pst_netbuf);
typedef oal_void  (*p_dmac_tx_excp_free_dscr_cb)(oal_dlist_head_stru *pst_tx_dscr_list_hdr, hal_to_dmac_device_stru *pst_hal_device);

typedef struct
{
    p_dmac_tx_excp_free_netbuf_cb                p_dmac_tx_excp_free_netbuf;
    p_dmac_tx_excp_free_dscr_cb                  p_dmac_tx_excp_free_dscr;
}dmac_tx_bss_comm_rom_cb;

extern dmac_tx_bss_comm_rom_cb g_st_dmac_tx_bss_comm_rom_cb;



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
    hal_to_dmac_device_stru *pst_hal_device;
    mac_tx_ctl_stru         *pst_tx_cb;
    hal_tx_dscr_stru        *pst_tx_dscr;
    oal_netbuf_stru         *pst_netbuf;
    oal_uint8               *puc_mac_hdr_addr;
    oal_uint8               *puc_mac_payload_addr;
    oal_uint8                auc_user_macaddr[WLAN_MAC_ADDR_LEN];
    oal_uint16               us_mac_frame_len;      /* 帧头+帧体长度 */
    oal_uint8                uc_mac_hdr_len;
    oal_switch_enum_uint8    en_frame_switch;
    oal_switch_enum_uint8    en_cb_switch;
    oal_switch_enum_uint8    en_dscr_switch;
}dmac_tx_dump_param_stru;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
typedef struct
{
    oal_bool_enum_uint8 en_stat_en;
    oal_bool_enum_uint8 en_stat_rate_start;  /* 统计均速 */
    oal_uint16          us_pkt_len;   /* 每帧长度 */
    oal_uint32          ul_rcv_pkts;  /* HOST来帧量统计 */
    oal_uint32          ul_snd_pkts;  /* 驱动实际发送帧统计 */
    oal_uint32          ul_start_time; /* 均速统计开始时间 */
}dmac_tx_pkts_stat_stru;
extern dmac_tx_pkts_stat_stru g_tx_pkts_stat;
#endif
/*****************************************************************************
  8 UNION定义
*****************************************************************************/
extern oal_uint32  dmac_tx_mgmt(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf_mgmt, oal_uint16 us_len);
extern oal_uint32  dmac_tx_process_data_event(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_tx_process_data(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_uint32  dmac_tid_tx_queue_remove(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap, dmac_tid_stru *pst_tid_queue, oal_uint8 uc_dscr_num);
extern oal_uint32  dmac_tid_tx_queue_remove_ampdu(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_user, dmac_tid_stru *pst_tid_queue, oal_uint8 uc_mpdu_num);
extern oal_void  dmac_tx_excp_free_dscr(oal_dlist_head_stru *pst_tx_dscr_list_hdr, hal_to_dmac_device_stru *pst_hal_device);
extern oal_netbuf_stru* dmac_tx_dequeue_first_mpdu(oal_netbuf_head_stru  *pst_netbuf_head);
extern oal_uint32  dmac_tx_excp_free_netbuf(oal_netbuf_stru *pst_netbuf);
extern oal_uint32  dmac_flush_txq_to_tid_of_vo(hal_to_dmac_device_stru *pst_hal_device);
extern oal_void  dmac_tx_get_txop_alg_params(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user,  mac_tx_ctl_stru *pst_tx_ctl, hal_tx_txop_alg_stru **ppst_txop_alg);
extern oal_void dmac_tx_mgmt_get_txop_para(dmac_vap_stru *pst_dmac_vap, hal_tx_txop_alg_stru **ppst_txop_alg, mac_tx_ctl_stru *pst_tx_ctl);
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
extern hal_tx_dscr_stru* dmac_tx_dscr_alloc(hal_to_dmac_device_stru   *pst_hal_device, dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
#endif
extern oal_uint32  dmac_tx_get_amsdu_params(oal_netbuf_stru *pst_netbuf, mac_tx_ctl_stru *pst_tx_ctl, hal_tx_mpdu_stru *pst_mpdu);
#if defined (_PRE_WLAN_FEATURE_UAPSD) || defined (_PRE_WLAN_FEATURE_STA_PM)
extern oal_uint32 dmac_send_qosnull(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_uint8 uc_ac, oal_bool_enum_uint8 en_ps);
#endif
#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
extern oal_void dmac_vap_update_rsp_frm_rate(dmac_device_stru *pst_dmac_dev, oal_uint8 uc_protocol_mode, oal_uint8 uc_bandwidth, oal_uint8 uc_ref_rate);
#endif
extern oal_uint32  dmac_tx_dump_get_switch(oam_user_track_frame_type_enum_uint8     en_frame_type,
                                    oal_uint8                               *pen_frame_switch,
                                    oal_uint8                               *pen_cb_switch,
                                    oal_uint8                               *pen_dscr_switch,
                                    mac_tx_ctl_stru                         *pst_tx_cb);
extern oal_uint32  dmac_tx_dump_get_user_macaddr(mac_tx_ctl_stru *pst_tx_cb,
                                                         oal_uint8 auc_user_macaddr[]);
extern oal_void dmac_tx_dump_data(hal_to_dmac_device_stru     *pst_hal_device,
                            oal_dlist_head_stru         *pst_tx_dscr_list_hdr);
extern oal_void  dmac_tx_dump_mgmt(hal_to_dmac_device_stru *pst_hal_device,
                                oal_netbuf_stru *pst_netbuf_mgmt,
                                hal_tx_mpdu_stru *pst_mpdu,
                                hal_tx_dscr_stru *pst_mgmt_dscr);
extern oal_void  dmac_free_tx_dscr_queue(dmac_vap_stru *pst_dmac_vap, oal_dlist_head_stru *pst_dscr_head);
extern oal_void dmac_tx_set_first_cb(mac_tx_ctl_stru *pst_cb_first, dmac_user_stru *pst_user);
extern oal_void dmac_post_soft_tx_complete_event(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_base_dscr, oal_uint8 uc_dscr_num);

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
oal_void  dmac_tx_data_always_tx(hal_to_dmac_device_stru *pst_hal_device,
                                                             dmac_vap_stru *pst_dmac_vap,
                                                             hal_tx_dscr_stru *pst_tx_dscr);
#endif
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


OAL_STATIC OAL_INLINE oal_void  dmac_tx_set_htc_field(
                                    hal_to_dmac_device_stru  *pst_hal_device,
                                    hal_tx_dscr_stru         *pst_tx_dscr,
                                    mac_tx_ctl_stru          *pst_tx_ctl,
                                    hal_tx_ppdu_feature_stru *pst_ppdu_feature)
{
    mac_ieee80211_qos_htc_frame_addr4_stru  *pst_mac_ieee80211_qos_htc_frame_addr4;
    mac_ieee80211_qos_htc_frame_stru        *pst_mac_ieee80211_qos_htc_frame;
    oal_uint8                                uc_mac_header_length;

    /* frame control B15(Order)改为1，加上HTC部分，并且HTC字段的B22B23值是11, 改写描述符 */
    /*lint -save -e506 */
    /*lint -save -e774 */
    if (OAL_TRUE == MAC_GET_CB_IS_4ADDRESS(pst_tx_ctl))
    {
        pst_mac_ieee80211_qos_htc_frame_addr4  = (mac_ieee80211_qos_htc_frame_addr4_stru *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);;
        pst_mac_ieee80211_qos_htc_frame_addr4->ul_htc |= (BIT22 | BIT23);
        pst_mac_ieee80211_qos_htc_frame_addr4->st_frame_control.bit_order = 1;
        uc_mac_header_length                   = OAL_SIZEOF(mac_ieee80211_qos_htc_frame_addr4_stru);
    }
    else
    {
        pst_mac_ieee80211_qos_htc_frame = (mac_ieee80211_qos_htc_frame_stru *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
        pst_mac_ieee80211_qos_htc_frame->ul_htc |= (BIT22 | BIT23);
        pst_mac_ieee80211_qos_htc_frame->st_frame_control.bit_order = 1;
        uc_mac_header_length                   = OAL_SIZEOF(mac_ieee80211_qos_htc_frame_stru);
    }
    /*lint -restore */
    /*lint -restore */
    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl) = uc_mac_header_length;

    pst_tx_dscr->us_original_mpdu_len = uc_mac_header_length + MAC_GET_CB_MPDU_LEN(pst_tx_ctl);

    if (pst_ppdu_feature->uc_ampdu_enable == OAL_TRUE)
    {
        pst_ppdu_feature->ul_ampdu_length  += (OAL_SIZEOF(mac_ieee80211_qos_htc_frame_stru) - OAL_SIZEOF(mac_ieee80211_qos_frame_stru));
    }

    /* 修改描述符中mac 帧头长度 */
    hal_tx_set_dscr_modify_mac_header_length(pst_hal_device,pst_tx_dscr, uc_mac_header_length);

}


OAL_STATIC OAL_INLINE oal_void  dmac_tx_unset_htc_field(
                                    hal_to_dmac_device_stru  *pst_hal_device,
                                    hal_tx_dscr_stru         *pst_tx_dscr,
                                    mac_tx_ctl_stru          *pst_tx_ctl,
                                    hal_tx_ppdu_feature_stru *pst_ppdu_feature)
{
    oal_uint8   uc_mac_header_length;
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    uc_mac_header_length = MAC_80211_QOS_FRAME_LEN;
#else
    if (OAL_TRUE == MAC_GET_CB_IS_4ADDRESS(pst_tx_ctl))
    {
        uc_mac_header_length = MAC_80211_QOS_4ADDR_FRAME_LEN;
    }
    else
    {
        uc_mac_header_length = MAC_80211_QOS_FRAME_LEN;
    }
#endif
    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl) = uc_mac_header_length;

    pst_tx_dscr->us_original_mpdu_len = uc_mac_header_length + MAC_GET_CB_MPDU_LEN(pst_tx_ctl);

    if (pst_ppdu_feature->uc_ampdu_enable == OAL_TRUE)
    {
        pst_ppdu_feature->ul_ampdu_length  -= (MAC_80211_QOS_HTC_FRAME_LEN - MAC_80211_QOS_FRAME_LEN);
    }

    /* 修改描述符中mac 帧头长度 */
    hal_tx_set_dscr_modify_mac_header_length(pst_hal_device, pst_tx_dscr, uc_mac_header_length);
}


OAL_STATIC OAL_INLINE oal_void  dmac_free_tx_dscr(hal_tx_dscr_stru *pst_tx_dscr)
{
    oal_netbuf_stru   *pst_netbuf = OAL_PTR_NULL;

    pst_netbuf = pst_tx_dscr->pst_skb_start_addr;
    pst_tx_dscr->pst_skb_start_addr = OAL_PTR_NULL;

    OAL_MEM_FREE(pst_tx_dscr, OAL_TRUE);
    if (OAL_PTR_NULL != pst_netbuf)
    {
        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_netbuf);
    }
}

#ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE

OAL_STATIC OAL_INLINE oal_void dmac_tx_get_spec_mode_ucast_data_params(
                                                   dmac_vap_stru         *pst_dmac_vap,
                                                   dmac_user_stru        *pst_dmac_user,
                                                   mac_tx_ctl_stru       *pst_tx_ctl,
                                                   hal_tx_txop_alg_stru  **ppst_txop_alg)
{
    wlan_phy_protocol_enum_uint8 en_phy_protocol = g_auc_default_mac_to_phy_protocol_mapping[pst_dmac_user->st_user_base_info.en_avail_protocol_mode];

    /* 若当前user采用的protocol所对应的配置参数valid, 则采用该协议下的ucast data发送参数 */
    if ((WLAN_VHT_PHY_PROTOCOL_MODE == en_phy_protocol) && (1 == pst_dmac_vap->un_mode_valid.st_spec_mode.bit_vht_param_vaild))
    {
        *ppst_txop_alg = &pst_dmac_vap->st_tx_alg_vht;
    }
    else if ((WLAN_HT_PHY_PROTOCOL_MODE == en_phy_protocol) && (1 == pst_dmac_vap->un_mode_valid.st_spec_mode.bit_ht_param_vaild))
    {
        *ppst_txop_alg = &pst_dmac_vap->st_tx_alg_ht;
    }
    else if ((WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE == en_phy_protocol) && (1 == pst_dmac_vap->un_mode_valid.st_spec_mode.bit_11ag_param_vaild))
    {
        *ppst_txop_alg = &pst_dmac_vap->st_tx_alg_11ag;
    }
    else if ((WLAN_11B_PHY_PROTOCOL_MODE == en_phy_protocol) && (1 == pst_dmac_vap->un_mode_valid.st_spec_mode.bit_11b_param_vaild))
    {
        *ppst_txop_alg = &pst_dmac_vap->st_tx_alg_11b;
    }
    /* 否则, 还是采用默认的ucast data发送参数 */
    else
    {
        *ppst_txop_alg = &pst_dmac_vap->st_tx_alg;
    }

    return ;
}
#endif


OAL_STATIC OAL_INLINE oal_void  dmac_tx_init_ppdu_feature(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_uint8 uc_mpdu_num, hal_tx_ppdu_feature_stru *pst_ppdu_feature)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    hal_to_dmac_device_stru   *pst_hal_device_base;

    pst_hal_device_base = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device_base))
    {
      return;
    }
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_hal_vap))
    {
      return;
    }

    /* zero ppdu feature */
    OAL_MEMZERO(pst_ppdu_feature, OAL_SIZEOF(hal_tx_ppdu_feature_stru));

    /* 单包/管理帧 发送这些字段无效初始化为0 */
    pst_ppdu_feature->uc_mpdu_num = uc_mpdu_num;

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (mac_vap_is_vsta(&pst_dmac_vap->st_vap_base_info))
    {
      /* 如果启用Repeater功能，Proxy STA的发送描述符中的TX VAP index需要填为4(和普通sta是一样的) */
      pst_ppdu_feature->st_ppdu_addr_index.uc_tx_vap_index = WLAN_STA0_HAL_VAP_ID;
      pst_ppdu_feature->st_ppdu_addr_index.uc_ori_tx_vap_index = pst_dmac_vap->pst_hal_vap->uc_vap_id;
    }
    else
    {
      pst_ppdu_feature->st_ppdu_addr_index.uc_tx_vap_index = pst_dmac_vap->pst_hal_vap->uc_vap_id;
      pst_ppdu_feature->st_ppdu_addr_index.uc_ori_tx_vap_index = 0;
    }
#else
    pst_ppdu_feature->st_ppdu_addr_index.uc_tx_vap_index = pst_dmac_vap->pst_hal_vap->uc_vap_id;
#endif



    if (OAL_PTR_NULL != pst_dmac_user)
    {
        /* addba req需要填写正确的ra lut index 以获取正确的ssn */
        pst_ppdu_feature->st_ppdu_addr_index.uc_ra_lut_index = pst_dmac_user->uc_lut_index; /* 用户结构体下的lut index */
    }

    pst_ppdu_feature->en_addba_ssn_hw_bypass    = OAL_TRUE;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    if(OAL_SWITCH_ON == pst_hal_device_base->bit_al_tx_flag)
    {
        pst_ppdu_feature->en_seq_ctl_hw_bypass      = OAL_TRUE;
        pst_ppdu_feature->en_duration_hw_bypass     = OAL_TRUE;
        pst_ppdu_feature->en_retry_flag_hw_bypass   = OAL_TRUE;
    }
#endif

 }



OAL_STATIC OAL_INLINE oal_void  dmac_get_mgmt_mpdu_param(oal_netbuf_stru *pst_netbuf_mgmt, mac_tx_ctl_stru *pst_tx_ctl,  oal_uint16 us_len, hal_tx_mpdu_stru *pst_mpdu)
{
    MAC_GET_CB_NETBUF_NUM(pst_tx_ctl) = 1;              /* 管理帧只有一个 */
    MAC_GET_CB_EVENT_TYPE(pst_tx_ctl) = FRW_EVENT_TYPE_WLAN_CTX;
    MAC_GET_CB_MPDU_NUM(pst_tx_ctl)   = 1;              /* 管理帧只有一个 */
    MAC_GET_CB_IS_AMSDU(pst_tx_ctl)              = OAL_FALSE; /* 管理帧不做amsdu聚合 */
    MAC_SET_CB_FRAME_HEADER_ADDR(pst_tx_ctl, (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf_mgmt));
    MAC_GET_CB_IS_ROAM_DATA(pst_tx_ctl) = OAL_FALSE;

    /* 填写MPDU基本参数 */
    pst_mpdu->st_wmm.uc_tid_no = 0;
    pst_mpdu->st_wmm.uc_qos_enable = OAL_FALSE;
    if (WLAN_CONTROL == MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_ctl))
    {
        pst_mpdu->st_wmm.uc_tid_no = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);
        pst_mpdu->st_mpdu_mac_hdr.uc_mac_hdr_len = MAC_80211_CTL_HEADER_LEN;
    }
    else
    {
        pst_mpdu->st_mpdu_mac_hdr.uc_mac_hdr_len = MAC_80211_FRAME_LEN;
    }

    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl)  = pst_mpdu->st_mpdu_mac_hdr.uc_mac_hdr_len;

    pst_mpdu->st_mpdu_mac_hdr.uc_num_sub_msdu = 1;

    pst_mpdu->st_mpdu_addr.ul_mac_hdr_start_addr = (oal_uint32)OAL_NETBUF_HEADER(pst_netbuf_mgmt);
    pst_mpdu->st_mpdu_addr.pst_skb_start_addr    = pst_netbuf_mgmt;
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_mpdu->ast_msdu_addr[0].ul_msdu_addr0 = (oal_uint32)((oal_uint8 *)OAL_NETBUF_PAYLOAD(pst_netbuf_mgmt));
#else
    pst_mpdu->ast_msdu_addr[0].ul_msdu_addr0 = (oal_uint32)((oal_uint8 *)OAL_NETBUF_HEADER(pst_netbuf_mgmt) + pst_mpdu->st_mpdu_mac_hdr.uc_mac_hdr_len);
#endif


    pst_mpdu->ast_msdu_addr[0].us_msdu0_len  = us_len - MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl);

    pst_mpdu->ast_msdu_addr[0].us_msdu1_len  = 0;
    pst_mpdu->ast_msdu_addr[0].ul_msdu_addr1 = 0;

    MAC_GET_CB_MPDU_LEN(pst_tx_ctl)  = us_len - MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl);

    pst_mpdu->us_mpdu_len = us_len;
}


OAL_STATIC OAL_INLINE oal_uint32  dmac_tx_get_mpdu_params(
                oal_netbuf_stru           *pst_netbuf,
                mac_tx_ctl_stru           *pst_tx_ctl,
                hal_tx_mpdu_stru          *pst_mpdu)
{
    /* 获取MPDU相关参数 */
    pst_mpdu->st_mpdu_mac_hdr.uc_mac_hdr_len   = MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl);

    pst_mpdu->st_wmm.en_ack_policy = MAC_GET_CB_ACK_POLACY(pst_tx_ctl);
    pst_mpdu->st_wmm.uc_tid_no     = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);
    pst_mpdu->st_wmm.uc_qos_enable = MAC_GET_CB_IS_QOS_DATA(pst_tx_ctl);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    pst_mpdu->st_wmm.uc_nonqos_seq_bypass = mac_get_cb_is_seq_ctrl_bypass(pst_tx_ctl);
#endif

    pst_mpdu->st_mpdu_addr.ul_mac_hdr_start_addr = (oal_uint32)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
    pst_mpdu->st_mpdu_addr.pst_skb_start_addr    = pst_netbuf;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    dmac_tx_get_amsdu_params(pst_netbuf, pst_tx_ctl, pst_mpdu);
#else
    pst_mpdu->st_mpdu_mac_hdr.uc_num_sub_msdu  = MAC_GET_CB_NETBUF_NUM(pst_tx_ctl);

    if (OAL_FALSE == MAC_GET_CB_IS_AMSDU(pst_tx_ctl))
    {
        pst_mpdu->ast_msdu_addr[0].ul_msdu_addr0 = (oal_uint32)oal_netbuf_payload(pst_netbuf);
        pst_mpdu->ast_msdu_addr[0].us_msdu0_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl);
        pst_mpdu->ast_msdu_addr[0].ul_msdu_addr1 = 0;
        pst_mpdu->ast_msdu_addr[0].us_msdu1_len = 0;
    }
    else
    {
        return dmac_tx_get_amsdu_params(pst_netbuf, pst_tx_ctl, pst_mpdu);
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_uint32  dmac_tx_get_ampdu_max_len(dmac_user_stru *pst_user,dmac_tid_stru *pst_tid_queue)
{
    if ((WLAN_VHT_MODE == pst_user->st_user_base_info.en_cur_protocol_mode)
      || (WLAN_VHT_ONLY_MODE == pst_user->st_user_base_info.en_cur_protocol_mode))
    {
        return pst_tid_queue->st_ht_tx_hdl.ul_ampdu_max_size_vht;
    }
    else
    {
        return pst_tid_queue->st_ht_tx_hdl.us_ampdu_max_size;
    }
}


OAL_STATIC OAL_INLINE oal_void  dmac_tx_ampdu_calculate_total_len(hal_tx_ppdu_feature_stru *pst_ppdu_feature, oal_uint16 us_mpdu_len, oal_uint16 us_pad_len, oal_uint16 us_null_len)
{
    /* 给ampdu总长度加上前一个帧的pad和null data这个帧的长度 */
    pst_ppdu_feature->ul_ampdu_length += us_pad_len + us_null_len + us_mpdu_len;
}


OAL_STATIC OAL_INLINE oal_void  dmac_tx_ampdu_calculate_pad_len(hal_tx_ppdu_feature_stru *pst_ppdu_feature, oal_uint16 us_mpdu_len, oal_uint16 *us_pad_len, oal_uint16 *us_null_len)
{
    oal_uint16 us_roundup_len;

    /* 计算 pad len，mpdu的长度需要4字节对齐*/
    *us_pad_len = (4 - (us_mpdu_len & 0x3)) & 0x3;
    /* 4字节对齐后mpdu length */
    us_roundup_len = us_mpdu_len + *us_pad_len;
    /* mpdu长小于协议规定最短长时需要补齐空pad字节 */
    *us_null_len = OAL_MAX(us_roundup_len, pst_ppdu_feature->us_min_mpdu_length) - us_roundup_len;
}




OAL_STATIC OAL_INLINE oal_uint16  dmac_tx_get_baw_remain(hal_to_dmac_device_stru *pst_hal_device, dmac_ba_tx_stru *pst_tx_ba_handle)
{
    oal_uint16                  us_baw_end;

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    if (OAL_TRUE == pst_hal_device->en_ampdu_tx_hw_en)
    {
        return pst_tx_ba_handle->us_baw_size;
    }
#endif

    us_baw_end = DMAC_BA_SEQNO_ADD(pst_tx_ba_handle->us_baw_start, pst_tx_ba_handle->us_baw_size);
    us_baw_end = DMAC_BA_SEQNO_SUB(us_baw_end, 1);

    /*获取本次可以新发送mpdu的最大数目*/
    return DMAC_BA_SEQNO_SUB(us_baw_end, pst_tx_ba_handle->us_last_seq_num);
}


OAL_STATIC OAL_INLINE oal_uint16  dmac_tx_get_mpdu_ext_len(dmac_user_stru *pst_user)
{
    oal_uint8    uc_ext_len = (WLAN_HDR_FCS_LENGTH + WLAN_DELIMETER_LENGTH);

    /* 判断是否加密增加加密字节数 */
    switch (pst_user->st_user_base_info.st_key_info.en_cipher_type)
    {
        case WLAN_80211_CIPHER_SUITE_CCMP:
            uc_ext_len += WLAN_CCMP_ENCRYP_LEN;
            break;

        case WLAN_80211_CIPHER_SUITE_CCMP_256:
        case WLAN_80211_CIPHER_SUITE_GCMP:
        case WLAN_80211_CIPHER_SUITE_GCMP_256:
            uc_ext_len += WLAN_CCMP256_GCMP_ENCRYP_LEN;
            break;

        default:
            break;
    }

    return uc_ext_len;
}

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT

OAL_STATIC OAL_INLINE oal_void  dmac_tx_queue_mpdu(oal_netbuf_stru *pst_netbuf, oal_netbuf_head_stru *pst_head)
{
    oal_netbuf_stru    *pst_netbuf_tmp;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    mac_tx_ctl_stru    *pst_cb;
    oal_uint8           uc_netbuf_num_in_mpdu;
#endif

    pst_netbuf_tmp = pst_netbuf;
    /* 需要将所有的netbuf链加入 */
    while (OAL_PTR_NULL != pst_netbuf_tmp)
    {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        /* 从每一个mpdu中第一个net_buf的CB字段获取该mpdu一共包含几个net_buff */
        pst_cb = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf_tmp);
        uc_netbuf_num_in_mpdu = MAC_GET_CB_NETBUF_NUM(pst_cb);
#endif
        /* 将该mpdu的每一个net_buff加入到节能队列中 */
        pst_netbuf = pst_netbuf_tmp;
        while (
        #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
            (0 != uc_netbuf_num_in_mpdu) &&
        #endif
            (OAL_PTR_NULL != pst_netbuf))
        {
            pst_netbuf_tmp = oal_get_netbuf_next(pst_netbuf);

            oal_netbuf_add_to_list_tail(pst_netbuf, pst_head);

            pst_netbuf = pst_netbuf_tmp;

        #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
            uc_netbuf_num_in_mpdu--;
        #endif
        }
    }
}


OAL_STATIC OAL_INLINE oal_void  dmac_tx_queue_mpdu_head(oal_netbuf_stru *pst_netbuf, oal_netbuf_head_stru *pst_head)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_netbuf_addlist(pst_head, pst_netbuf);
#else/* 1151需要考虑mpdu占多个buf场景,暂未修改 */
    dmac_tx_queue_mpdu(pst_netbuf, pst_head);
#endif
    return;
}
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU

OAL_STATIC OAL_INLINE oal_void dmac_tx_amsdu_update_80211head(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_user, mac_tx_ctl_stru  *pst_tx_ctrl)
{
    mac_ieee80211_qos_frame_stru           *pst_qos_header;
    mac_ieee80211_qos_frame_addr4_stru     *pst_qos_4addr_header;
    mac_ieee80211_qos_htc_frame_addr4_stru *pst_hdr;

    /* 更新mac head amsdu标记 */
    if (MAC_80211_QOS_FRAME_LEN == MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl))
    {
        pst_qos_header = (mac_ieee80211_qos_frame_stru*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl);
        pst_qos_header->bit_qc_amsdu = OAL_TRUE;
    }
    else if (MAC_80211_QOS_4ADDR_FRAME_LEN == MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl))
    {
        pst_qos_4addr_header = (mac_ieee80211_qos_frame_addr4_stru*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl);
        pst_qos_4addr_header->bit_qc_amsdu = OAL_TRUE;
    }
#if 1
    pst_hdr = (mac_ieee80211_qos_htc_frame_addr4_stru*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl);

    if ((WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode) &&
        (!(MAC_GET_CB_IS_4ADDRESS(pst_tx_ctrl))))
    {
        /* Set Address3 field in the WLAN Header with the BSSID */
        oal_set_mac_addr(pst_hdr->auc_address3, pst_user->st_user_base_info.auc_user_mac_addr);
    }
    else if ((WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode) &&
             (!(MAC_GET_CB_IS_4ADDRESS(pst_tx_ctrl)))) /* From AP */
    {
        /* Set Address3 field in the WLAN Header with the BSSID */
        oal_set_mac_addr(pst_hdr->auc_address3, pst_dmac_vap->st_vap_base_info.auc_bssid);
    }
    else if ((WLAN_VAP_MODE_WDS == pst_dmac_vap->st_vap_base_info.en_vap_mode) ||
             (MAC_GET_CB_IS_4ADDRESS(pst_tx_ctrl)))
    {
        /* 地址3是 BSSID */
        oal_set_mac_addr(pst_hdr->auc_address3, mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info));

        /* 地址4也是 BSSID */
        oal_set_mac_addr(pst_hdr->auc_address4, mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info));
    }
    #endif
}


OAL_STATIC OAL_INLINE void dmac_tx_tid_encap_amsdu(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_user, oal_netbuf_stru  *pst_netbuf)
{
    mac_tx_ctl_stru            *pst_tx_ctrl;

    pst_tx_ctrl  = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);

    /* 单帧 */
    if (OAL_PTR_NULL == oal_get_netbuf_next(pst_netbuf))
    {
        if (OAL_TRUE == MAC_GET_CB_IS_LARGE_SKB_AMSDU(pst_tx_ctrl))
        {
            /* 由于节能等原因回退至TID的帧,已经修改的80211 HEAD */
            /* 需要回退帧头地址 */
        }

        return;
    }

    /* 标记此帧为大包AMSDU */
    MAC_GET_CB_IS_LARGE_SKB_AMSDU(pst_tx_ctrl) = OAL_TRUE;

    /* 更新mac head amsdu标记 */
    dmac_tx_amsdu_update_80211head(pst_dmac_vap, pst_user, pst_tx_ctrl);
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8 dmac_tx_need_encap_amsdu(oal_netbuf_stru *pst_netbuf,oal_netbuf_head_stru  *pst_netbuf_head)
{
    mac_tx_ctl_stru       *pst_tx_ctl;
    oal_netbuf_stru       *pst_netbuf_next;

    /* Host处理过的帧才能AMSDU聚合 */
    pst_tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);
    if (OAL_FALSE == MAC_GET_CB_HAS_EHTER_HEAD(pst_tx_ctl))
    {
        return OAL_FALSE;
    }

    /* TID中至少有两帧 */
    pst_netbuf_next = oal_netbuf_peek(pst_netbuf_head);
    if (OAL_PTR_NULL == pst_netbuf_next)
    {
        return OAL_FALSE;
    }

    pst_tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf_next);
    if (OAL_FALSE == MAC_GET_CB_HAS_EHTER_HEAD(pst_tx_ctl))
    {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}
#endif


OAL_STATIC OAL_INLINE hal_tx_dscr_stru* dmac_tx_dequeue(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap, dmac_tid_stru *pst_tid_queue, oal_dlist_head_stru *pst_dscr_head)
{
    oal_dlist_head_stru        *pst_dscr_entry      = OAL_PTR_NULL;
    hal_tx_dscr_stru           *pst_tx_dscr         = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_netbuf_stru            *pst_netbuf;
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    oal_netbuf_stru            *pst_netbuf_next;
#endif

    /* 出队顺序:先重传队列，再netbuf队列 */
    if (OAL_FALSE == oal_dlist_is_empty(&pst_tid_queue->st_retry_q))
    {
        /* 重传队列非空 */
        pst_dscr_entry = oal_dlist_delete_head(&pst_tid_queue->st_retry_q);
        pst_tx_dscr    = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        oal_dlist_add_tail(pst_dscr_entry, pst_dscr_head);
    }
    else if (OAL_FALSE == oal_netbuf_list_empty(&pst_tid_queue->st_buff_head))
    {
        /* netbuf队列非空 */
        pst_netbuf  = dmac_tx_dequeue_first_mpdu(&pst_tid_queue->st_buff_head);
        if (OAL_PTR_NULL == pst_netbuf)
        {
            return OAL_PTR_NULL;
        }

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        /* 使能amsdu逻辑处 */
        if (OAL_TRUE == dmac_tx_need_encap_amsdu(pst_netbuf, &pst_tid_queue->st_buff_head))
        {
            pst_netbuf_next  = dmac_tx_dequeue_first_mpdu(&pst_tid_queue->st_buff_head);
            oal_set_netbuf_next(pst_netbuf, pst_netbuf_next);
            oal_set_netbuf_next(pst_netbuf_next, OAL_PTR_NULL);
        }
#endif

        pst_tx_dscr = dmac_tx_dscr_alloc(pst_hal_device, pst_dmac_vap, pst_netbuf);
        if (OAL_PTR_NULL == pst_tx_dscr)
        {
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
            if (OAL_PTR_NULL != oal_get_netbuf_next(pst_netbuf))
            {
                pst_netbuf_next = pst_netbuf->next;
                oal_set_netbuf_next(pst_netbuf, OAL_PTR_NULL);
                dmac_tx_queue_mpdu_head(pst_netbuf_next, &pst_tid_queue->st_buff_head);
            }
#endif
            dmac_tx_queue_mpdu_head(pst_netbuf, &pst_tid_queue->st_buff_head);
            return OAL_PTR_NULL;
        }

        oal_dlist_add_tail(&pst_tx_dscr->st_entry, pst_dscr_head);
    }

    return pst_tx_dscr;
#else
    if (OAL_FALSE == oal_dlist_is_empty(&pst_tid_queue->st_hdr))
    {
        /* TID队列非空 */
        pst_dscr_entry = oal_dlist_delete_head(&pst_tid_queue->st_hdr);
        pst_tx_dscr    = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        oal_dlist_add_tail(pst_dscr_entry, pst_dscr_head);
    }
    return pst_tx_dscr;
#endif
}



OAL_STATIC OAL_INLINE oal_void dmac_tx_mgmt_set_seq_num(mac_ieee80211_frame_stru *pst_mac_header)
{
    if ((WLAN_CONTROL != pst_mac_header->st_frame_control.bit_type) ||
        (WLAN_BLOCKACK_REQ != pst_mac_header->st_frame_control.bit_sub_type))
    {
        pst_mac_header->bit_frag_num = 0;
        pst_mac_header->bit_seq_num  = 0;
    }
}

extern oal_void  dmac_tx_restore_tx_queue(hal_to_dmac_device_stru     *pst_hal_device,
                                                      hal_tx_dscr_queue_header_stru *pst_fake_queue);
extern oal_void dmac_tx_save_tx_queue(hal_to_dmac_device_stru       *pst_hal_device,
                                                  hal_tx_dscr_queue_header_stru *pst_fake_queue);
extern oal_void  dmac_clear_tx_queue(hal_to_dmac_device_stru *pst_hal_device);
extern oal_uint32  dmac_tx_switch_tx_queue(hal_tx_dscr_queue_header_stru  *pst_fake_queue1, hal_tx_dscr_queue_header_stru  *pst_fake_queue2);
extern oal_uint32  dmac_tx_update_protection_all_txop_alg(dmac_vap_stru *pst_dmac_vap);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_tx_bss_comm.h */
