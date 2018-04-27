


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_profiling.h"

#include "oam_ext_if.h"

#include "mac_data.h"

#include "dmac_ext_if.h"
#include "dmac_tx_complete.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_blockack.h"
#include "dmac_psm_ap.h"
#include "dmac_uapsd.h"
#include "dmac_chan_mgmt.h"
#include "dmac_ext_if.h"
#include "dmac_dft.h"
#include "mac_data.h"
#include "dmac_p2p.h"
#include "dmac_beacon.h"

#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_test_main.h"
#include "dmac_test_sch.h"
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
#include "mac_board.h"
#include "dmac_device.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "dmac_ftm.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
#include "dmac_smps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
#include "dmac_resource.h"
#endif

#ifdef _PRE_WLAN_11K_STAT
#include "dmac_stat.h"
#endif

#ifdef _PRE_WLAN_INIT_PTK_TX_PN
#include "dmac_11i.h"
#endif

#include "dmac_power.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TX_COMPLETE_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_INIT_PTK_TX_PN

//与AC68网卡跑流压测断流，ping不通，此时用VI或者VO重新跑流可以跑通，或者通过修改MAC寄存器(0x2001a104(msb) 0x2001a108(lsb))将我们发送的PN号
//改大可以恢复跑流，一开始怀疑是我们自己的PN号跳变了，导致后续的数据帧的PN号都小于跳变值，从而导致不通的，但是增加维测断流时并未出现我们PN好
//跳变的打印信息，所以暂时的结论是AC68自己保存的PN号跳变导致的，为了降低PN号跳变导致不通的概率，将发送端的邋PN号初始化为一个很大的值

dmac_tx_dscr_pn_stru g_ul_prev_iv_word[WLAN_CHIP_MAX_NUM][DMAC_MAX_RA_LUT_INDEX] = {
{   {0, (PN_MSB_INIT_VALUE<<4),0x20000000}, {1, (PN_MSB_INIT_VALUE<<4),0x20000000}, {2, (PN_MSB_INIT_VALUE<<4),0x20000000}, {3, (PN_MSB_INIT_VALUE<<4),0x20000000}, {4, (PN_MSB_INIT_VALUE<<4),0x20000000},
    {5, (PN_MSB_INIT_VALUE<<4),0x20000000}, {6, (PN_MSB_INIT_VALUE<<4),0x20000000}, {7, (PN_MSB_INIT_VALUE<<4),0x20000000}, {8, (PN_MSB_INIT_VALUE<<4),0x20000000}, {9, (PN_MSB_INIT_VALUE<<4),0x20000000},
    {10,(PN_MSB_INIT_VALUE<<4),0x20000000}, {11,(PN_MSB_INIT_VALUE<<4),0x20000000}, {12,(PN_MSB_INIT_VALUE<<4),0x20000000}, {13,(PN_MSB_INIT_VALUE<<4),0x20000000}, {14,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {15,(PN_MSB_INIT_VALUE<<4),0x20000000}, {16,(PN_MSB_INIT_VALUE<<4),0x20000000}, {17,(PN_MSB_INIT_VALUE<<4),0x20000000}, {18,(PN_MSB_INIT_VALUE<<4),0x20000000}, {19,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {20,(PN_MSB_INIT_VALUE<<4),0x20000000}, {21,(PN_MSB_INIT_VALUE<<4),0x20000000}, {22,(PN_MSB_INIT_VALUE<<4),0x20000000}, {23,(PN_MSB_INIT_VALUE<<4),0x20000000}, {24,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {25,(PN_MSB_INIT_VALUE<<4),0x20000000}, {26,(PN_MSB_INIT_VALUE<<4),0x20000000}, {27,(PN_MSB_INIT_VALUE<<4),0x20000000}, {28,(PN_MSB_INIT_VALUE<<4),0x20000000}, {29,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {30,(PN_MSB_INIT_VALUE<<4),0x20000000}, {31,(PN_MSB_INIT_VALUE<<4),0x20000000}  },

{   {0, (PN_MSB_INIT_VALUE<<4),0x20000000}, {1, (PN_MSB_INIT_VALUE<<4),0x20000000}, {2, (PN_MSB_INIT_VALUE<<4),0x20000000}, {3, (PN_MSB_INIT_VALUE<<4),0x20000000}, {4, (PN_MSB_INIT_VALUE<<4),0x20000000},
    {5, (PN_MSB_INIT_VALUE<<4),0x20000000}, {6, (PN_MSB_INIT_VALUE<<4),0x20000000}, {7, (PN_MSB_INIT_VALUE<<4),0x20000000}, {8, (PN_MSB_INIT_VALUE<<4),0x20000000}, {9, (PN_MSB_INIT_VALUE<<4),0x20000000},
    {10,(PN_MSB_INIT_VALUE<<4),0x20000000}, {11,(PN_MSB_INIT_VALUE<<4),0x20000000}, {12,(PN_MSB_INIT_VALUE<<4),0x20000000}, {13,(PN_MSB_INIT_VALUE<<4),0x20000000}, {14,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {15,(PN_MSB_INIT_VALUE<<4),0x20000000}, {16,(PN_MSB_INIT_VALUE<<4),0x20000000}, {17,(PN_MSB_INIT_VALUE<<4),0x20000000}, {18,(PN_MSB_INIT_VALUE<<4),0x20000000}, {19,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {20,(PN_MSB_INIT_VALUE<<4),0x20000000}, {21,(PN_MSB_INIT_VALUE<<4),0x20000000}, {22,(PN_MSB_INIT_VALUE<<4),0x20000000}, {23,(PN_MSB_INIT_VALUE<<4),0x20000000}, {24,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {25,(PN_MSB_INIT_VALUE<<4),0x20000000}, {26,(PN_MSB_INIT_VALUE<<4),0x20000000}, {27,(PN_MSB_INIT_VALUE<<4),0x20000000}, {28,(PN_MSB_INIT_VALUE<<4),0x20000000}, {29,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {30,(PN_MSB_INIT_VALUE<<4),0x20000000}, {31,(PN_MSB_INIT_VALUE<<4),0x20000000}  } };

dmac_tx_dscr_pn_stru g_ul_max_iv_word[WLAN_CHIP_MAX_NUM][DMAC_MAX_RA_LUT_INDEX] = {
{   {0, (PN_MSB_INIT_VALUE<<4),0x20000000}, {1, (PN_MSB_INIT_VALUE<<4),0x20000000}, {2, (PN_MSB_INIT_VALUE<<4),0x20000000}, {3, (PN_MSB_INIT_VALUE<<4),0x20000000}, {4, (PN_MSB_INIT_VALUE<<4),0x20000000},
    {5, (PN_MSB_INIT_VALUE<<4),0x20000000}, {6, (PN_MSB_INIT_VALUE<<4),0x20000000}, {7, (PN_MSB_INIT_VALUE<<4),0x20000000}, {8, (PN_MSB_INIT_VALUE<<4),0x20000000}, {9, (PN_MSB_INIT_VALUE<<4),0x20000000},
    {10,(PN_MSB_INIT_VALUE<<4),0x20000000}, {11,(PN_MSB_INIT_VALUE<<4),0x20000000}, {12,(PN_MSB_INIT_VALUE<<4),0x20000000}, {13,(PN_MSB_INIT_VALUE<<4),0x20000000}, {14,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {15,(PN_MSB_INIT_VALUE<<4),0x20000000}, {16,(PN_MSB_INIT_VALUE<<4),0x20000000}, {17,(PN_MSB_INIT_VALUE<<4),0x20000000}, {18,(PN_MSB_INIT_VALUE<<4),0x20000000}, {19,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {20,(PN_MSB_INIT_VALUE<<4),0x20000000}, {21,(PN_MSB_INIT_VALUE<<4),0x20000000}, {22,(PN_MSB_INIT_VALUE<<4),0x20000000}, {23,(PN_MSB_INIT_VALUE<<4),0x20000000}, {24,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {25,(PN_MSB_INIT_VALUE<<4),0x20000000}, {26,(PN_MSB_INIT_VALUE<<4),0x20000000}, {27,(PN_MSB_INIT_VALUE<<4),0x20000000}, {28,(PN_MSB_INIT_VALUE<<4),0x20000000}, {29,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {30,(PN_MSB_INIT_VALUE<<4),0x20000000}, {31,(PN_MSB_INIT_VALUE<<4),0x20000000}  },

{   {0, (PN_MSB_INIT_VALUE<<4),0x20000000}, {1, (PN_MSB_INIT_VALUE<<4),0x20000000}, {2, (PN_MSB_INIT_VALUE<<4),0x20000000}, {3, (PN_MSB_INIT_VALUE<<4),0x20000000}, {4, (PN_MSB_INIT_VALUE<<4),0x20000000},
    {5, (PN_MSB_INIT_VALUE<<4),0x20000000}, {6, (PN_MSB_INIT_VALUE<<4),0x20000000}, {7, (PN_MSB_INIT_VALUE<<4),0x20000000}, {8, (PN_MSB_INIT_VALUE<<4),0x20000000}, {9, (PN_MSB_INIT_VALUE<<4),0x20000000},
    {10,(PN_MSB_INIT_VALUE<<4),0x20000000}, {11,(PN_MSB_INIT_VALUE<<4),0x20000000}, {12,(PN_MSB_INIT_VALUE<<4),0x20000000}, {13,(PN_MSB_INIT_VALUE<<4),0x20000000}, {14,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {15,(PN_MSB_INIT_VALUE<<4),0x20000000}, {16,(PN_MSB_INIT_VALUE<<4),0x20000000}, {17,(PN_MSB_INIT_VALUE<<4),0x20000000}, {18,(PN_MSB_INIT_VALUE<<4),0x20000000}, {19,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {20,(PN_MSB_INIT_VALUE<<4),0x20000000}, {21,(PN_MSB_INIT_VALUE<<4),0x20000000}, {22,(PN_MSB_INIT_VALUE<<4),0x20000000}, {23,(PN_MSB_INIT_VALUE<<4),0x20000000}, {24,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {25,(PN_MSB_INIT_VALUE<<4),0x20000000}, {26,(PN_MSB_INIT_VALUE<<4),0x20000000}, {27,(PN_MSB_INIT_VALUE<<4),0x20000000}, {28,(PN_MSB_INIT_VALUE<<4),0x20000000}, {29,(PN_MSB_INIT_VALUE<<4),0x20000000},
    {30,(PN_MSB_INIT_VALUE<<4),0x20000000}, {31,(PN_MSB_INIT_VALUE<<4),0x20000000}  } };
#endif
#if (defined(_PRE_DEBUG_MODE) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE))
oal_uint32 g_ul_desc_check_count[HAL_TX_QUEUE_BUTT] = {0};
oal_uint32 g_ul_desc_addr[HAL_TX_QUEUE_BUTT] = {0};
#endif

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
dmac_dyn_cali_tx_pow_modul_diff_stru  g_ast_dyn_tx_power_modul_diff[WLAN_BAND_BUTT] = {{0}};  /*  OFDM功率补偿值  */
#endif

extern oal_uint32  dmac_config_11i_add_key_set_reg(mac_vap_stru *pst_mac_vap, oal_uint8 uc_key_index, oal_uint8 *puc_mac_addr);

/* 静态函数声明 */
OAL_STATIC oal_uint32  dmac_tx_complete_normal_buffer(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr);
#ifdef _PRE_WLAN_FEATURE_AMPDU
OAL_STATIC oal_uint32  dmac_tx_complete_ampdu_buffer(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr);
#endif
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
OAL_STATIC oal_uint32  dmac_tx_complete_ampdu_buffer_hw(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr);
#endif

OAL_STATIC OAL_INLINE oal_uint32  dmac_tx_update_alg_param(hal_tx_dscr_stru *pst_dscr,
                                                  mac_tx_ctl_stru                *pst_cb,
                                                  hal_tx_dscr_ctrl_one_param *pst_tx_dscr_one,
                                                  hal_to_dmac_device_stru * pst_hal_device);

#ifdef _PRE_DEBUG_MODE_USER_TRACK

OAL_STATIC OAL_INLINE oal_uint32  dmac_tx_complete_check_protocol(
                                        hal_to_dmac_device_stru *pst_hal_device,
                                        dmac_user_stru   *pst_dmac_user,
                                        hal_tx_dscr_stru *pst_dscr)
{
    wlan_phy_protocol_enum_uint8   en_protocol = WLAN_PHY_PROTOCOL_BUTT;

    hal_tx_get_protocol_mode(pst_hal_device, pst_dscr, &en_protocol);

    dmac_user_check_txrx_protocol_change(pst_dmac_user, en_protocol, OAM_USER_INFO_CHANGE_TYPE_TX_PROTOCOL);

    return OAL_SUCC;
}
#endif


OAL_STATIC OAL_INLINE oal_void dmac_keepalive_timestamp_update(mac_vap_stru  *pst_mac_vap,
                                                           dmac_user_stru   *pst_dmac_user,
                                                           oal_uint8     *puc_frame_hdr,
                                                           oal_uint8      uc_dscr_status)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        return;
    }
#endif

    if ((WLAN_DATA_BASICTYPE != mac_frame_get_type_value(puc_frame_hdr)) ||
        (uc_dscr_status != DMAC_TX_SUCC))
    {
        return;
    }

    /* 更新用户时间戳 */
    pst_dmac_user->ul_last_active_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

}


oal_void dmac_tx_delete_ba_fail_process(dmac_user_stru *pst_dmac_user)
{
    hal_to_dmac_device_stru     *pst_hal_device = OAL_PTR_NULL;
    oal_uint8                    uc_tid;
    mac_user_stru               *pst_mac_user = &(pst_dmac_user->st_user_base_info);

    /* 通知算法把协议模式升回原协议模式 */
    dmac_alg_delete_ba_fail_notify(pst_mac_user);

    pst_hal_device = dmac_user_get_hal_device(pst_mac_user);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_delete_ba_fail_process::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_user->uc_vap_id);
        return;
    }

    for (uc_tid = 0; uc_tid < WLAN_TID_MAX_NUM; uc_tid++)
    {
        if (OAL_PTR_NULL != pst_dmac_user->ast_tx_tid_queue[uc_tid].pst_ba_tx_hdl)
        {
            /* 未删除ba成功，恢复TID队列 */
            dmac_tid_resume(pst_hal_device,&pst_dmac_user->ast_tx_tid_queue[uc_tid], DMAC_TID_PAUSE_RESUME_TYPE_BA);
        }
    }

}

oal_void dmac_tx_delete_ba(dmac_user_stru *pst_dmac_user)
{
    oal_uint8                    uc_tid;
    mac_user_stru               *pst_mac_user = &(pst_dmac_user->st_user_base_info);
    frw_event_mem_stru          *pst_event_mem;
    frw_event_stru              *pst_event;
    dmac_to_hmac_ctx_event_stru *pst_del_ba_event;

    for (uc_tid = 0; uc_tid < WLAN_TID_MAX_NUM; uc_tid++)
    {
        if (OAL_PTR_NULL != pst_dmac_user->ast_tx_tid_queue[uc_tid].pst_ba_tx_hdl)
        {
            /* 需要暂停对应TID队列,待BA删除后再恢复 */
            dmac_tid_pause(&pst_dmac_user->ast_tx_tid_queue[uc_tid], DMAC_TID_PAUSE_RESUME_TYPE_BA);
        }
    }

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_to_hmac_ctx_event_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        /* 删除ba失败的异常处理 */
        dmac_tx_delete_ba_fail_process(pst_dmac_user);
        OAM_ERROR_LOG0(pst_mac_user->uc_vap_id, OAM_SF_BA, "{dmac_tx_delete_ba::alloc event failed!}");
        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_SDT_REG,
                       DMAC_TO_HMAC_DEL_BA,
                       OAL_SIZEOF(dmac_to_hmac_ctx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_user->uc_chip_id,
                       pst_mac_user->uc_device_id,
                       pst_mac_user->uc_vap_id);

    pst_del_ba_event = (dmac_to_hmac_ctx_event_stru *)(pst_event->auc_event_data);

    pst_del_ba_event->us_user_index     = pst_mac_user->us_assoc_id;
    pst_del_ba_event->uc_tid            = 0xFF;     /* 通知HMAC删除所有TID的BA会话 */
    pst_del_ba_event->uc_vap_id         = pst_mac_user->uc_vap_id;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    pst_del_ba_event->uc_cur_protocol   = pst_mac_user->en_cur_protocol_mode;
#endif

    if(OAL_SUCC != frw_event_dispatch_event(pst_event_mem))
    {
        /* 删除ba失败的异常处理 */
        dmac_tx_delete_ba_fail_process(pst_dmac_user);
        OAM_ERROR_LOG0(pst_mac_user->uc_vap_id, OAM_SF_TX, "{dmac_tx_delete_ba::post event failed}");
    }

    FRW_EVENT_FREE(pst_event_mem);

    return;
}

#ifdef _PRE_DEBUG_MODE
OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_debug_dscr(hal_to_dmac_device_stru *pst_dmac_device,
                                                                       hal_tx_dscr_stru *pst_base_dscr)
{
    oal_mempool_tx_dscr_addr  *pst_tx_dscr_addr   = OAL_PTR_NULL;
    oal_dlist_head_stru       *pst_dscr_entry;
    hal_tx_dscr_stru          *pst_dscr;
    oal_uint16                 us_dscr_idx_in_list = 0;
    oal_uint16                 us_tx_dscr_cnt_dn = 0;
    mac_tx_ctl_stru           *pst_tx_cb;

    if (1 == oal_mem_get_stop_flag())
    {
       return ;
    }

    pst_tx_dscr_addr = oal_mem_get_tx_dscr_addr();

    if ((pst_tx_dscr_addr->us_rcd_rls_stop_flag > 0) && (pst_tx_dscr_addr->us_rcd_rls_stop_flag < OAL_TX_DSCR_RCD_TAIL_CNT))
    {
        pst_tx_dscr_addr->us_rcd_rls_stop_flag++;
    }

    us_tx_dscr_cnt_dn = pst_tx_dscr_addr->us_tx_dscr_cnt_dn;
    pst_tx_dscr_addr->ast_tx_dscr_info[us_tx_dscr_cnt_dn].ul_dn_intr_ts = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    pst_tx_dscr_addr->ast_tx_dscr_info[us_tx_dscr_cnt_dn].ul_tx_dscr_in_dn_intr = (oal_uint32)pst_base_dscr;
    pst_tx_dscr_addr->ast_tx_dscr_info[us_tx_dscr_cnt_dn].uc_q_num = pst_base_dscr->uc_q_num;
    pst_tx_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_base_dscr->pst_skb_start_addr);
    if (OAL_PTR_NULL != pst_tx_cb)
    {
        pst_tx_dscr_addr->ast_tx_dscr_info[us_tx_dscr_cnt_dn].uc_mpdu_num = MAC_GET_CB_MPDU_NUM(pst_tx_cb);
    }

    OAL_MEMZERO(pst_tx_dscr_addr->ast_tx_dscr_info[us_tx_dscr_cnt_dn].ul_tx_dscr_in_q, OAL_SIZEOF(oal_uint32) * OAL_MAX_TX_DSCR_CNT_IN_LIST);
    OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, &(pst_dmac_device->ast_tx_dscr_queue[pst_base_dscr->uc_q_num].st_header))
    {
        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        if (us_dscr_idx_in_list < OAL_MAX_TX_DSCR_CNT_IN_LIST)
        {
            pst_tx_dscr_addr->ast_tx_dscr_info[us_tx_dscr_cnt_dn].ul_tx_dscr_in_q[us_dscr_idx_in_list++] = (oal_uint32)pst_dscr;
        }
        else
        {
            pst_tx_dscr_addr->ast_tx_dscr_info[us_tx_dscr_cnt_dn].bit_dscr_is_overflow = OAL_TRUE;
            break;
        }
    }
    pst_tx_dscr_addr->us_tx_dscr_cnt_dn = (us_tx_dscr_cnt_dn + 1) % OAL_TX_DSCR_ITEM_NUM;

}


OAL_STATIC OAL_INLINE oal_void  dmac_tx_complete_vip_frame_check(mac_tx_ctl_stru *pst_cb, oal_netbuf_stru *pst_buf, oal_bool_enum_uint8 en_is_vipframe, oal_uint8 uc_tid, oal_uint8 uc_vap_id)
{
    if (OAL_TRUE == en_is_vipframe)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer:: VIP frame is aggregated. tidno = %u}", uc_tid);
        oam_report_80211_frame(BROADCAST_MACADDR, (oal_uint8 *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb),MAC_GET_CB_FRAME_HEADER_LENGTH(pst_cb),oal_netbuf_payload(pst_buf),
                           MAC_GET_CB_FRAME_HEADER_LENGTH(pst_cb) + MAC_GET_CB_MPDU_LEN(pst_cb), OAM_OTA_FRAME_DIRECTION_TYPE_TX);
    }
}

#endif

#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
/* 对于重传帧无需统计 */
OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_normal_pkt_num(oal_netbuf_stru *pst_buf, hal_tx_dscr_ctrl_one_param  *pst_tx_dscr_one, oal_uint16 us_index)
{
    mac_tx_ctl_stru             *pst_cb;

    pst_cb = (mac_tx_ctl_stru*)oal_netbuf_cb(pst_buf);

    /* 排除非host下传的帧 */
    if (MAC_GET_CB_ALG_TAGS(pst_cb) & DMAC_CB_ALG_TAGS_MUCTRL_MASK)
    {
        pst_tx_dscr_one->uc_normal_pkt_num++;

    #ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        if (OAL_TRUE == MAC_GET_CB_IS_LARGE_SKB_AMSDU(pst_cb))
        {
            pst_tx_dscr_one->uc_normal_pkt_num++;
        }
    #endif
    }
}
#endif

#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rwdata = "BTCM", code ="ATCM", zidata = "BTCM", rodata = "ATCM"
#endif

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

oal_int32 dmac_dyn_cali_get_tx_pow(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr,
                                              hal_pdet_info_stru *pst_pdet_info, oal_int16 *ps_tx_pow)
{
    hal_tx_dscr_ctrl_one_param          st_tx_dscr_one = {0};

    hal_tx_get_dscr_ctrl_one_param(pst_hal_device, pst_tx_dscr, &st_tx_dscr_one);

    hal_get_target_tx_power_by_tx_dscr(pst_hal_device, &st_tx_dscr_one, pst_pdet_info, ps_tx_pow);

    //OAM_WARNING_LOG1(0, OAM_SF_CALIBRATE, "dmac_dyn_cali_get_tx_pow[%d]", *ps_tx_pow);

    return OAL_SUCC;
}


oal_void dmac_tx_cali_realtime_handler(hal_tx_complete_event_stru *pst_tx_comp_event)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_tx_ctl_stru                    *pst_tx_cb;
    dmac_user_stru                     *pst_dmac_user;
    oal_int16                           s_tx_pow        = 0;
    oal_uint8                           uc_bw;
#endif
    oal_uint8                           uc_vap_id;
    mac_vap_stru                       *pst_mac_vap     = OAL_PTR_NULL;
    hal_pdet_info_stru                  st_pdet_info;
    hal_tx_dscr_stru                   *pst_base_dscr   = pst_tx_comp_event->pst_base_dscr; /* 保存上报上来的第一个描述符地址 */
    hal_to_dmac_device_stru            *pst_hal_device  = pst_tx_comp_event->pst_hal_device;
    hal_dyn_cali_usr_record_stru           *pst_user_pow     = OAL_PTR_NULL;

    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CALIBRATE, "dmac_tx_cali_realtime_handler::pst_hal_device is null[%p]", pst_tx_comp_event);
        return;
    }

    /* 51在hal做过滤判断，这里不需要 */
//#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#if 0
    /* pdet检测使能的情况下，对上报值进行过滤，待实测后对上报值的取值范围进行限定 */
    if (pst_tx_comp_event->s_pdet_val >= DMAC_DYN_CALI_PDET_MAX_LIMIT ||
        pst_tx_comp_event->s_pdet_val <= DMAC_DYN_CALI_PDET_MIN_LIMIT)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CALIBRATE, "{dmac_tx_cali_realtime_handler:illegal value s_pdet_val=[%d].}",
                           pst_tx_comp_event->s_pdet_val);
        return;
    }
#endif

    dmac_tx_get_vap_id(pst_hal_device,pst_base_dscr, &uc_vap_id);
    pst_mac_vap = mac_res_get_mac_vap(uc_vap_id);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CALIBRATE, "{dmac_tx_cali_realtime_handler::pst_mac_vap is NULL}");
        return;
    }

    st_pdet_info.s_real_pdet         = pst_tx_comp_event->s_pdet_val;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    pst_tx_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_base_dscr->pst_skb_start_addr);

    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_tx_cb));
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CALIBRATE, "{dmac_tx_cali_realtime_handler::user is NULL}");
        return;
    }

    pst_user_pow = &pst_dmac_user->ast_dyn_cali_pow[pst_tx_comp_event->uc_chain_num];

    /* pdet异常保护，防止pdet上报值不变导致upc一直变化 */
    if (st_pdet_info.s_real_pdet == pst_user_pow->s_real_pdet)
    {
        OAM_INFO_LOG1(0, 0, "dmac_tx_cali_realtime_handler::pdet_val = %d does not change,no nd dyn cali", st_pdet_info.s_real_pdet);
        return;
    }
#endif

    hal_tx_get_protocol_mode(pst_hal_device, pst_base_dscr, &st_pdet_info.en_cur_protocol);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 同步带宽信息 */
    st_pdet_info.un_record_para.st_rf_core_para.en_freq         = pst_mac_vap->st_channel.en_band;
    st_pdet_info.un_record_para.st_rf_core_para.uc_channel      = pst_mac_vap->st_channel.uc_chan_number;
    st_pdet_info.un_record_para.st_rf_core_para.uc_pdet_chain   = pst_tx_comp_event->uc_chain_num;

    hal_tx_get_bw_mode(pst_hal_device, pst_base_dscr, &uc_bw);
    st_pdet_info.un_record_para.st_rf_core_para.en_band_width = uc_bw;

    dmac_dyn_cali_get_tx_pow(pst_hal_device, pst_base_dscr, &st_pdet_info, &s_tx_pow);

    /*  超过期望功率过滤，限制范围(当前为0-22db)  */
    if((s_tx_pow < DMAC_DYN_CALI_MIN_POW) || (s_tx_pow > DMAC_DYN_CALI_MAX_POW))
    {
        OAM_INFO_LOG3(0, 0, "dmac_tx_cali_realtime_handler::illegal target_power, pst_tx_comp_event->uc_pdet_enable = %d,s_pdet_val = %d,s_tx_pow= %d}",
                                                 pst_tx_comp_event->en_pdet_enable, st_pdet_info.s_real_pdet, s_tx_pow);
        return;
    }
    st_pdet_info.un_record_para.st_rf_core_para.uc_tx_pow = (oal_uint8)s_tx_pow;

    OAM_INFO_LOG3(0, OAM_SF_CALIBRATE, "{dmac_tx_cali_realtime_handler::pst_tx_comp_event->uc_pdet_enable = %d,s_pdet_val = %d,s_tx_pow= %d}",
                                                 pst_tx_comp_event->en_pdet_enable, st_pdet_info.s_real_pdet, s_tx_pow);
#else
    st_pdet_info.en_freq             = pst_mac_vap->st_channel.en_band;
    st_pdet_info.uc_cur_channel      = pst_mac_vap->st_channel.uc_chan_number;
    st_pdet_info.uc_pdet_chain       = pst_tx_comp_event->uc_chain_num;
    st_pdet_info.en_cur_band_width   = pst_mac_vap->st_channel.en_bandwidth;


    /* 51记录配置功率,以0.1dBm为单位 */
    st_pdet_info.s_tx_pow = (oal_int16)pst_mac_vap->uc_tx_power * 10;
#endif

    /* 补偿使能 */
    if (OAL_TRUE == pst_hal_device->st_dyn_cali_val.en_realtime_cali_adjust)
    {
        /* 动态校准钩子函数,51不使用pst_user_pow */
        hal_rf_cali_realtime_entrance(pst_hal_device, &st_pdet_info, pst_user_pow);

    #ifndef _PRE_WLAN_FEATURE_TPC_OPT
        /* 刷新发送功率 */
        dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_REFRESH);
    #endif
    }

    /* 动态校准处理完成设置完成标志位TRUE */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    pst_hal_device->st_dyn_cali_val.en_realtime_cali_comp_flag = OAL_TRUE;
#endif

    return;
}
#endif


OAL_STATIC OAL_INLINE oal_void  dmac_tx_complete_notify(hal_to_dmac_device_stru *pst_hal_device,
                                                                mac_user_stru                  *pst_user,
                                                                oal_netbuf_stru                *pst_buf,
                                                                hal_tx_dscr_ctrl_one_param     *pst_tx_dscr_one)
{

#ifdef _PRE_WLAN_FEATURE_MWO_DET
    /* 上报微波炉周期 寄存器的计数 值给算法*/
    pst_tx_dscr_one->ul_tx_comp_mwo_cyc_time = pst_hal_device->ul_tx_comp_mwo_cyc_time;
#endif

#ifdef _PRE_WLAN_FEATURE_PF_SCH
    pst_tx_dscr_one->ul_tx_consumed_airtime = pst_hal_device->ul_tx_consumed_airtime;
#endif

    /* 通知算法 */
    dmac_alg_tx_complete_notify(pst_user, pst_buf, pst_tx_dscr_one);
}


oal_uint32  dmac_tx_complete_event_handler(frw_event_mem_stru *pst_event_mem)
{
    hal_tx_dscr_stru                *pst_base_dscr;          /* 保存上报上来的第一个描述符地址 */
    hal_to_dmac_device_stru         *pst_hal_device;
    frw_event_stru                  *pst_event;
    hal_tx_complete_event_stru      *pst_tx_comp_event;      /* tx complete事件 */
    hal_tx_dscr_stru                *pst_curr_dscr;         /*dev发送队列中， 当前指向描述符*/
    oal_uint32                       ul_ret = OAL_SUCC;
    hal_tx_queue_type_enum           uc_queue_num;           /* 描述符队列号 */
    oal_dlist_head_stru             *pst_header = OAL_PTR_NULL;
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint32                       ul_tx_q_status = 0;
#endif /* defined(_PRE_PRODUCT_ID_HI110X_DEV) */
#if defined(_PRE_DEBUG_MODE)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint16                       us_report_seqnum;
    oal_uint16                       us_curr_seqnum;
    OAL_STATIC  oal_uint8            uc_int_loss_cnt = 0;
#endif
    dmac_user_stru                  *pst_dmac_user;
    mac_tx_ctl_stru                 *pst_tx_cb;
    oal_dlist_head_stru             *pst_dscr_entry;
    dmac_tid_stru                   *pst_tid_queue;
#endif
#ifdef _PRE_PROFILING_MODE
    oal_uint                         ul_irq_flag;
#endif
#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
    dmac_device_stru                *pst_dmac_device;
#endif

    OAL_MIPS_STATISTIC_IRQ_SAVE();
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_COMP_DMAC_START);
/*51测试mips时，需要锁中断  */
#ifdef _PRE_PROFILING_MODE
    oal_irq_save(&ul_irq_flag, OAL_5115IRQ_PROFILING);
#endif
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TX_COMP_DMAC_START);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_complete_event_handler::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 从事件结构体中获取硬件上报的描述符地址和发送描述符个数 */
    pst_event         = frw_get_event_stru(pst_event_mem);
    pst_tx_comp_event = (hal_tx_complete_event_stru *)pst_event->auc_event_data;
    pst_base_dscr     = pst_tx_comp_event->pst_base_dscr;
    pst_hal_device    = pst_tx_comp_event->pst_hal_device;
    uc_queue_num      = (hal_tx_queue_type_enum)pst_base_dscr->uc_q_num;
    pst_header        = &(pst_hal_device->ast_tx_dscr_queue[uc_queue_num].st_header);

#ifdef _PRE_WLAN_FEATURE_FTM
    pst_hal_device->ull_t1 = pst_tx_comp_event->ull_t1;
    pst_hal_device->ull_t4 = pst_tx_comp_event->ull_t4;
#endif

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    if((!(GET_HAL_DYN_CALI_DISABLE(pst_hal_device))) && HAL_DEVICE_WORK_STATE == GET_HAL_DEVICE_STATE(pst_hal_device))
    {
        /* 动态校准只对首帧做处理 */
        if ((OAL_PTR_NULL == pst_base_dscr->pst_skb_start_addr) ||
               (OAL_FALSE == pst_tx_comp_event->en_pdet_enable) ||
               (OAL_FALSE == pst_base_dscr->bit_is_first))
        {
            OAM_INFO_LOG3(0, OAM_SF_CALIBRATE, "dmac_tx_cali_realtime_handler::abort for pst_base_dscr[0x%x] pdet_en[%d] is_firt frame[%d]",
                             pst_base_dscr, pst_tx_comp_event->en_pdet_enable, pst_base_dscr->bit_is_first);

        }
        else
        {
            dmac_tx_cali_realtime_handler(pst_tx_comp_event);
        }

        /* 常发不需要后面处理 */
        if(HAL_ALWAYS_TX_RF == pst_hal_device->bit_al_tx_flag)
        {
            return OAL_TRUE;
        }
    }
#endif

#ifdef _PRE_DEBUG_MODE
    /* 检查queue_num是否正常*/
    if(OAL_UNLIKELY((uc_queue_num != pst_base_dscr->uc_q_num) || (uc_queue_num >= HAL_TX_QUEUE_BUTT)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_TX, "dmac_tx_complete_event_handler: report_queue_num=%d, dscr_queue_num=%d, dscr_addr=0x%x", uc_queue_num, pst_base_dscr->uc_q_num, pst_base_dscr);
        oal_mem_stop_rcd_rls();

        oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_base_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);

        return OAL_FAIL;
    }

    if(OAL_PTR_NULL == pst_base_dscr->pst_skb_start_addr)
    {
        return OAL_FAIL;
    }

    /* 检查是否是ppdu的第一个mpdu*/
    if (OAL_UNLIKELY(OAL_FALSE == pst_base_dscr->bit_is_first))
    {
        oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_base_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
        OAM_WARNING_LOG1(0, OAM_SF_TX, "dmac_tx_complete_event_handler: is not the first mpud of ppdu, dscr_addr=0x%x", pst_base_dscr);
        oal_mem_stop_rcd_rls();
        /* OTA上报当前tx队列里面的描述符 */
        OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, pst_header)
        {
            pst_curr_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
            oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_curr_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
        }
        return OAL_FAIL;
    }

    /* 检查上报的描述符的个数合法性 , 检查出错后不返回，继续处理*/
    pst_tx_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_base_dscr->pst_skb_start_addr);
    if (OAL_UNLIKELY(((OAL_TRUE == pst_base_dscr->bit_is_ampdu) && (MAC_GET_CB_MPDU_NUM(pst_tx_cb) != pst_tx_comp_event->uc_dscr_num))
         || ((OAL_FALSE == pst_base_dscr->bit_is_ampdu) && (1 != pst_tx_comp_event->uc_dscr_num))))
    {
        oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_base_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
        OAM_WARNING_LOG4(0, OAM_SF_TX, "dmac_tx_complete_event_handler: tid=%d, irq_mpdu_num=%d, cb_mpdu_num=%d, dscr_addr=0x%x",
                              MAC_GET_CB_WME_TID_TYPE(pst_tx_cb), pst_tx_comp_event->uc_dscr_num, MAC_GET_CB_MPDU_NUM(pst_tx_cb), pst_base_dscr);
        oal_mem_stop_rcd_rls();
        /* OTA上报当前tx队列里面的描述符 */
        OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, pst_header)
        {
            pst_curr_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
            oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_curr_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
        }
    }

    /*检查描述符地址是否在dev发送队列中*/
    OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, pst_header)
    {
        pst_curr_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        if (pst_curr_dscr == pst_base_dscr)
        {
            break;
        }
    }

    if (OAL_UNLIKELY(pst_dscr_entry == pst_header))
    {
        OAM_WARNING_LOG2(0, OAM_SF_TX, "dmac_tx_complete_event_handler: not find this dscr 0x%x in dev_tx_q(q_type =%d)", pst_base_dscr, uc_queue_num);
        oal_mem_stop_rcd_rls();
        oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_base_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
        /* 打印当前tx队列中的描述符地址 */
        OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, pst_header)
        {
            pst_curr_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
            OAM_WARNING_LOG3(0, OAM_SF_TX, "{tx_q[%d] dscr::[0x%x].}", uc_queue_num, MAC_GET_CB_WME_TID_TYPE(pst_tx_cb), pst_curr_dscr);
        }
        /* 打印当前tid队列里面的描述符地址 */
        pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_tx_cb));
        if (OAL_PTR_NULL == pst_dmac_user)
        {
            return OAL_FAIL;
        }
        pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[MAC_GET_CB_WME_TID_TYPE(pst_tx_cb)]);
    #ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        /* 先遍历重传队列 */
        OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, &pst_tid_queue->st_retry_q)
        {
            pst_curr_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
            OAM_WARNING_LOG4(0, OAM_SF_TX, "{retry Q info:user[%d]tid[%d] dscr::[0x%x] retry_mpdu[%d].}", MAC_GET_CB_TX_USER_IDX(pst_tx_cb), MAC_GET_CB_WME_TID_TYPE(pst_tx_cb), pst_curr_dscr, pst_tid_queue->uc_retry_num);
        }
        OAM_WARNING_LOG4(0, OAM_SF_TX, "{netbuf Q info:user[%d]tid[%d] netbuf num::[%d] mpdu[%d].}", MAC_GET_CB_TX_USER_IDX(pst_tx_cb), MAC_GET_CB_WME_TID_TYPE(pst_tx_cb), pst_tid_queue->st_buff_head.ul_num, pst_tid_queue->us_mpdu_num);

    #else
        OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, &pst_tid_queue->st_hdr)
        {
            pst_curr_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
            OAM_WARNING_LOG3(0, OAM_SF_TX, "{user[%d]tid[%d] dscr::[0x%x].}", MAC_GET_CB_TX_USER_IDX(pst_tx_cb), MAC_GET_CB_WME_TID_TYPE(pst_tx_cb), pst_curr_dscr);
        }
    #endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
        return OAL_FAIL;
    }

    /*描述符调试记录*/
    dmac_tx_complete_debug_dscr(pst_hal_device, pst_base_dscr);

    if ((pst_base_dscr->uc_q_num < HAL_TX_QUEUE_HI))
    {
        g_ast_tx_complete_stat[pst_hal_device->uc_mac_device_id].ul_tx_complete_bh1_num++;
        OAM_INFO_LOG1(0, OAM_SF_TX, "dmac_tx_complete_event_handler:ul_tx_complete_bh1_num = %d", g_ast_tx_complete_stat[pst_hal_device->uc_mac_device_id].ul_tx_complete_bh1_num);
    }
#endif //#ifdef _PRE_DEBUG_MODE

#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
    pst_dmac_device = dmac_res_get_mac_dev(pst_event->st_event_hdr.uc_device_id);
    /* 只有混合抓包下才会走tx抓包函数 */
    if ((OAL_PTR_NULL != pst_dmac_device) && (DMAC_PKT_CAP_MIX == pst_dmac_device->st_pkt_capture_stat.uc_capture_switch))
    {
        /* 新增发送抓包处理函数 */
        ul_ret = dmac_pkt_cap_tx(&pst_dmac_device->st_pkt_capture_stat, pst_dmac_device->past_hal_device[0]);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_PKT_CAP, "{dmac_tx_complete_event_handler::dmac_pkt_cap_tx err return 0x%x.}", ul_ret);
        }
    }
#endif

    /* 从dev中的发送队列头部开始循环处理， 直到处理完当前上报的描述符，保证中断丢失无影响 */
    do
    {
        /*发送队列为空则直接跳出*/
        if (OAL_UNLIKELY(oal_dlist_is_empty(pst_header)))
        {
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
            /* TBD:问题跟踪，无复现后续日志可降级 */
            OAM_ERROR_LOG4(0, OAM_SF_ANY, "{dmac_tx_complete_event_handler:: TX Q(%d) IS EMPTY, dscr addr[0x%x],skb[0x%x],first[%d].}",
                uc_queue_num, pst_base_dscr, pst_base_dscr->pst_skb_start_addr, pst_base_dscr->bit_is_first);

            hal_get_all_tx_q_status(pst_hal_device, &ul_tx_q_status);
            OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_tx_complete_event_handler::ppdu_cnt(%d), tx_q_status(%u).}",
                pst_hal_device->ast_tx_dscr_queue[uc_queue_num].uc_ppdu_cnt,
                ul_tx_q_status);
#else
            OAM_WARNING_LOG4(0, OAM_SF_ANY, "{dmac_tx_complete_event_handler:: TX Q(%d) IS EMPTY, dscr addr[0x%x],skb[0x%x],first[%d].}",
                uc_queue_num, pst_base_dscr, pst_base_dscr->pst_skb_start_addr, pst_base_dscr->bit_is_first);
#endif /* defined(_PRE_PRODUCT_ID_HI110X_DEV) */
            pst_hal_device->ast_tx_dscr_queue[uc_queue_num].uc_ppdu_cnt = 0;
            break;
        }

        pst_curr_dscr = OAL_DLIST_GET_ENTRY(pst_header->pst_next, hal_tx_dscr_stru, st_entry);
    #ifdef _PRE_DEBUG_MODE
    #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /*丢中断维测*/
        if (OAL_UNLIKELY(pst_curr_dscr != pst_base_dscr))
        {
            hal_tx_get_dscr_seq_num(pst_hal_device, pst_base_dscr, &us_report_seqnum);  /*获取上报描述符seqnum*/
            hal_tx_get_dscr_seq_num(pst_hal_device, pst_curr_dscr, &us_curr_seqnum);      /*获取当前处理描述符seqnum*/
            OAM_WARNING_LOG4(0, OAM_SF_ANY, "{dmac_tx_complete_event_handler::hw_report_seqnum=%d pst_base_dscr=0x%p, sw_current_seqnum=%d pst_curr_dscr=0x%p.}", us_report_seqnum, pst_base_dscr, us_curr_seqnum, pst_curr_dscr);
            oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_base_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
            oam_report_dscr(BROADCAST_MACADDR,(oal_uint8 *)pst_curr_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
            uc_int_loss_cnt++;
            if (uc_int_loss_cnt > 1)
            {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_tx_complete_event_handler::continuous loss int[%d].}", uc_int_loss_cnt);
            }
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_tx_complete_event_handler::single int loss[%d].}", uc_int_loss_cnt);
        }
        else
        {
            uc_int_loss_cnt = 0;
        }
    #endif

    #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        /* 检查上报的描述符地址和alg判断 ppdu number异常时候记录的是否一致 */
        if(g_ul_desc_addr[uc_queue_num] == (oal_uint32)pst_curr_dscr)
        {
            OAM_WARNING_LOG3(0, OAM_SF_ANY, "{dmac_tx_complete_event_handler::BH after scheduler, noted queue[%d] descriptor address=%08x",
                uc_queue_num, &(pst_hal_device->ast_tx_dscr_queue[uc_queue_num]), pst_curr_dscr);
            g_ul_desc_addr[uc_queue_num] = 0;
            g_ul_desc_check_count[uc_queue_num] = 0;
        }
        else if(g_ul_desc_check_count[uc_queue_num]++ > 100)
        {
            g_ul_desc_check_count[uc_queue_num] = 0;
            g_ul_desc_addr[uc_queue_num] = 0;
        }
    #endif
    #endif

    #ifdef _PRE_WLAN_FEATURE_MWO_DET
        /*上报微波炉周期寄存器的计数值给算法*/
        pst_hal_device->ul_tx_comp_mwo_cyc_time = pst_tx_comp_event->ul_tx_comp_mwo_cyc_time;
    #endif
    #ifdef _PRE_WLAN_FEATURE_PF_SCH
        pst_hal_device->ul_tx_consumed_airtime = pst_tx_comp_event->ul_tx_consumed_airtime;
    #endif

        ul_ret = dmac_tx_complete_buff(pst_hal_device, pst_curr_dscr);
        if (OAL_UNLIKELY(OAL_SUCC != ul_ret)) /*仅打印， 不返回， 保证调度正常进行*/
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_event_handler::dmac_tx_complete failed[%d].", ul_ret);
        }

    } while(pst_curr_dscr != pst_base_dscr) ;/*处理完本次中断上报的描述符后结束*/

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TX_COMP_DMAC_END);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_COMP_DMAC_END);
    OAL_MIPS_STATISTIC_IRQ_RESTORE();
#ifdef _PRE_PROFILING_MODE
    oal_irq_restore(&ul_irq_flag, OAL_5115IRQ_PROFILING);
#endif

#ifdef _PRE_WLAN_PROFLING_MIPS
    oal_profiling_stop_tx_save();
#endif

#ifdef _PRE_WLAN_PROFLING_MIPS
    if (g_mips_tx_statistic.uc_flag & BIT0)
    {
        return OAL_SUCC;
    }
#endif

    ul_ret = dmac_tx_complete_schedule(pst_hal_device, HAL_Q_NUM_TO_AC(uc_queue_num));

    return ul_ret;
}


oal_uint32  dmac_tx_complete_buff(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr)
{
    oal_uint32          ul_ret = OAL_SUCC;

    /* 打印发送描述符 */
    dmac_tx_complete_dump_dscr(pst_hal_device, pst_dscr);

    if ((OAL_TRUE == pst_dscr->bit_is_ampdu) || (OAL_TRUE == pst_dscr->bit_is_rifs))
    {
#ifdef _PRE_WLAN_FEATURE_AMPDU
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
        if(OAL_TRUE == pst_hal_device->en_ampdu_tx_hw_en)
        {
            ul_ret = dmac_tx_complete_ampdu_buffer_hw(pst_hal_device, pst_dscr);
        }
        else
#endif
        {
            ul_ret = dmac_tx_complete_ampdu_buffer(pst_hal_device, pst_dscr);
        }
#endif
    }
    else
    {
        ul_ret = dmac_tx_complete_normal_buffer(pst_hal_device, pst_dscr);
    }

    return ul_ret;
}



oal_void  dmac_tx_complete_free_dscr_list(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr, oal_uint8 uc_dscr_num, oal_uint8 en_set_flag)
{
    oal_uint8            uc_idx;
    hal_tx_dscr_stru    *pst_dscr_next;

    for (uc_idx = 0; uc_idx < uc_dscr_num; uc_idx++)
    {
        if (OAL_PTR_NULL == pst_dscr)
        {
           OAM_WARNING_LOG2(0, OAM_SF_TX, "{dmac_tx_complete_free_dscr_list::pst_buf null(dscr_num=%d, idx=%d). }", uc_dscr_num, uc_idx);
           break;
        }

        pst_dscr_next = OAL_DLIST_GET_ENTRY(pst_dscr->st_entry.pst_next, hal_tx_dscr_stru, st_entry);
        dmac_tx_complete_free_dscr(pst_dscr);
        pst_dscr = pst_dscr_next;
    }
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_void  dmac_tx_complete_sdt_stat(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_seq, oal_uint8 *puc_sdt_permit)
{
    if (++pst_hal_device->st_hal_dev_sdt_stat.us_last_sn == us_seq)
    {
        if (3 == ++pst_hal_device->st_hal_dev_sdt_stat.uc_print_ctl)
        {
            pst_hal_device->st_hal_dev_sdt_stat.uc_print_ctl = 0;
            *puc_sdt_permit = OAL_TRUE;
        }
    }
    else
    {
        pst_hal_device->st_hal_dev_sdt_stat.us_last_sn = us_seq;
        *puc_sdt_permit = OAL_TRUE;
        pst_hal_device->st_hal_dev_sdt_stat.uc_print_ctl = 0;
    }
}
#endif


OAL_STATIC OAL_INLINE oal_void  dmac_tx_set_retry_dscr(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr, oal_uint8 uc_tx_status, dmac_tx_chiper_stat_stru *pst_chiper_stat)
{
    oal_uint32 ul_iv_ls_word = 0xFF;
    oal_uint32 ul_iv_ms_word = 0xFF;

    /* 1103重传报文发送描述符清0 1两行 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    hal_tx_retry_clear_dscr(pst_hal_device, pst_dscr);
#endif

    /* 标志为重传包 */
    pst_dscr->bit_is_retried = OAL_TRUE;
    hal_tx_set_dscr_status(pst_hal_device, pst_dscr, DMAC_TX_INVALID);

#ifdef _PRE_WLAN_MAC_BUGFIX_PN
    if ((uc_tx_status == DMAC_TX_TIMEOUT)
       || (uc_tx_status == DMAC_TX_RTS_FAIL)
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
       || (uc_tx_status == DMAC_TX_RTS_FAIL_CTS_ERROR)
#elif (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
       || (uc_tx_status == DMAC_TX_FAIL_BW_TOO_BIG)
#endif
       || (uc_tx_status == DMAC_TX_SOFT_PSM_BACK))
    {
        if (OAL_TRUE == pst_dscr->bit_is_first)
        {
            hal_tx_get_dscr_iv_word(pst_dscr, &ul_iv_ms_word, &ul_iv_ls_word, pst_chiper_stat->uc_tx_chiper_type, pst_chiper_stat->uc_chiper_key_id);
            pst_dscr->bit_is_first = OAL_FALSE;
            hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_dscr, 0);
            return;
        }

        hal_tx_get_dscr_iv_word(pst_dscr, &ul_iv_ms_word, &ul_iv_ls_word, pst_chiper_stat->uc_tx_chiper_type, pst_chiper_stat->uc_chiper_key_id);
        if (0 == ul_iv_ls_word)
        {
            hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_dscr, 0);
        }
        else
        {
            hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_dscr, 1);
        }
    }
    else if ((uc_tx_status == DMAC_TX_KEY_SEARCH_FAIL)
       || (uc_tx_status == DMAC_TX_AMPDU_MISMATCH)
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
       || (uc_tx_status == DMAC_TX_FAIL_ABORT)
       || (uc_tx_status == DMAC_TX_FAIL_STATEMACHINE_PHY_ERROR)
#endif
       || (uc_tx_status == DMAC_TX_PENDING))
    {
        pst_dscr->bit_is_first = OAL_FALSE;
        hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_dscr, 0);
        return;
    }
    else
    {
        hal_tx_get_dscr_iv_word(pst_dscr, &ul_iv_ms_word, &ul_iv_ls_word, pst_chiper_stat->uc_tx_chiper_type, pst_chiper_stat->uc_chiper_key_id);
        pst_dscr->bit_is_first = OAL_FALSE;
        if (0 == ul_iv_ls_word)
        {
            hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_dscr, 0);
        }
        else
        {
            hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_dscr, 1);
        }

    }

#else
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
#ifdef _PRE_WLAN_1103_PILOT
    /* 如果是带宽超过硬件工作带宽能力,进行维测打印，聚合帧只看首帧 */
    if (DMAC_TX_FAIL_BW_TOO_BIG == uc_tx_status && OAL_TRUE == pst_dscr->bit_is_first)
    {
        hal_get_tx_dscr_field(pst_hal_device, pst_dscr);
    }
#endif
#endif

    if(OAL_TRUE == pst_hal_device->en_ampdu_tx_hw_en)
    {
        /* 王浩:硬件聚合重传帧, 当做新帧处理 */
        hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_dscr, 0);
    }
    else
    {
        pst_dscr->bit_is_first = OAL_FALSE;
        hal_tx_get_dscr_iv_word(pst_dscr, &ul_iv_ms_word, &ul_iv_ls_word, pst_chiper_stat->uc_tx_chiper_type, pst_chiper_stat->uc_chiper_key_id);
        if (0 == ul_iv_ls_word)
        {
            hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_dscr, 0);
        }
        else
        {
            /* 设置描述符重传标记 */
            hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_dscr, 1);
        }
    }

#endif //_PRE_WLAN_MAC_BUGFIX_PN

}

OAL_STATIC OAL_INLINE oal_uint32 dmac_tx_complete_dscr_back(dmac_vap_stru      *pst_dmac_vap,
                                                                       dmac_user_stru          *pst_dmac_user,
                                                                       oal_uint8                uc_tid,
                                                                       oal_dlist_head_stru     *pst_pending_q,
                                                                       oal_uint8                uc_retry_num)
{
    oal_uint32 ul_ret = OAL_SUCC;
    oal_uint16 us_seq_num = 0;
    oal_dlist_head_stru *pst_dscr_entry = OAL_PTR_NULL;
    hal_tx_dscr_stru    *pst_tx_dscr    = OAL_PTR_NULL;
    hal_to_dmac_device_stru *pst_hal_device;
    dmac_tid_stru            *pst_tid_queue;

    if (oal_dlist_is_empty(pst_pending_q))
    {
        return OAL_SUCC;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;

    pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[uc_tid]);

    ul_ret = dmac_tid_tx_queue_enqueue_head(&pst_dmac_user->st_user_base_info, pst_tid_queue, pst_pending_q, uc_retry_num);
    if (OAL_SUCC == ul_ret)
    {
        pst_tid_queue->uc_retry_num += uc_retry_num;
        dmac_alg_tid_update_notify(pst_tid_queue);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE) && defined(_PRE_WLAN_DFT_STAT)
        if (OAL_PTR_NULL != pst_tid_queue->pst_tid_stats)
        {
            DMAC_TID_STATS_INCR(pst_tid_queue->pst_tid_stats->ul_tid_retry_enqueue_cnt, uc_retry_num);
        }
#endif
    }
    else
    {
        /*amdpu聚合时候，回退tid队列不成功需要释放描述符并移窗*/
        if ((DMAC_TX_MODE_NORMAL != pst_tid_queue->en_tx_mode) && (OAL_PTR_NULL != pst_tid_queue->pst_ba_tx_hdl))
        {
            OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, pst_pending_q)
            {
                pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
                hal_tx_get_dscr_seq_num(pst_hal_device, pst_tx_dscr, &us_seq_num);
                dmac_ba_update_baw(pst_tid_queue->pst_ba_tx_hdl, us_seq_num);
            }
        }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        pst_dmac_vap->st_vap_base_info.st_vap_stats.ul_tx_dropped_packets += uc_retry_num;
#endif
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::dmac_tid_tx_queue_enqueue failed[%d].", ul_ret);

        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_dscr(pst_pending_q, pst_hal_device);
    }

    return OAL_SUCC;
}


oal_void dmac_tx_hw_back_to_queue(hal_to_dmac_device_stru *pst_hal_device,
                                                hal_tx_dscr_stru      *pst_tx_first_dscr,
                                                mac_tx_ctl_stru       *pst_cb,
                                                oal_dlist_head_stru   *pst_tx_dscr_list_hdr)
{

    oal_uint8                  uc_vap_id;
    dmac_vap_stru             *pst_dmac_vap;
    dmac_user_stru            *pst_dmac_user;
    oal_dlist_head_stru       *pst_dscr_entry;
    oal_uint8                  uc_retry_num = 0;
    hal_tx_dscr_stru          *pst_tx_dscr  = OAL_PTR_NULL;

    dmac_tx_get_vap_id(pst_hal_device, pst_tx_first_dscr, &uc_vap_id);
    pst_dmac_vap = mac_res_get_dmac_vap(uc_vap_id);
    if(OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_TX,"dmac_tx_hw_back_to_queue::pst_dmac_vap null,vap id=%d", uc_vap_id);
        return ;
    }

    OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, pst_tx_dscr_list_hdr)
    {
        pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        pst_tx_dscr->bit_is_retried = OAL_TRUE;

        hal_tx_set_dscr_status(pst_hal_device, pst_tx_dscr, DMAC_TX_INVALID);
        if(OAL_TRUE == pst_hal_device->en_ampdu_tx_hw_en)
        {
            hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_tx_dscr, 0);
        }
        else
        {
            hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_tx_dscr, 1);
        }

        uc_retry_num++;
    }

    /* 一个ampdu归属同一个用户 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_cb));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_hw_back_to_queue::pst_dmac_user[%d] null.}",
            MAC_GET_CB_TX_USER_IDX(pst_cb));
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_tx_first_dscr, uc_retry_num, OAL_TRUE);
        return ;
    }

    dmac_tx_complete_dscr_back(pst_dmac_vap, pst_dmac_user, MAC_GET_CB_WME_TID_TYPE(pst_cb), pst_tx_dscr_list_hdr, uc_retry_num);
    return ;

}


OAL_STATIC OAL_INLINE  oal_uint8 dmac_tx_comp_mgmt_retry_check(mac_tx_ctl_stru *pst_cb,
                                                          mac_ieee80211_frame_stru  *pst_mac_hdr)
{
    oal_uint8                           uc_frame_sub_type;
    oal_uint8                           uc_is_mgmt = OAL_FALSE;

    uc_frame_sub_type = mac_get_frame_sub_type((oal_uint8 *)pst_mac_hdr);
    switch (uc_frame_sub_type)
    {
        /* 只处理管理报文 */
        case WLAN_FC0_SUBTYPE_AUTH:
        case WLAN_FC0_SUBTYPE_ASSOC_REQ:
        case WLAN_FC0_SUBTYPE_ASSOC_RSP:
        case WLAN_FC0_SUBTYPE_REASSOC_REQ:
        case WLAN_FC0_SUBTYPE_REASSOC_RSP:
        case WLAN_FC0_SUBTYPE_ACTION:
            uc_is_mgmt = OAL_TRUE;
            break;

        default:
            uc_is_mgmt = OAL_FALSE;
            break;
    }

    if (OAL_TRUE == uc_is_mgmt)
    {
        if ((MAC_GET_CB_RETRIED_NUM(pst_cb)) < DMAC_MGMT_MAX_SW_RETRIES)
        {
            return OAL_TRUE;
        }
        else
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_comp_mgmt_retry_check::drop mgmt,frame type[%d].", uc_frame_sub_type);
        }
    }

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_uint8 dmac_tx_comp_data_retry_check(dmac_vap_stru *pst_dmac_vap, mac_tx_ctl_stru *pst_cb)
{
    if ((MAC_GET_CB_RETRIED_NUM(pst_cb) < pst_dmac_vap->uc_sw_retry_limit)
       && (OAL_FALSE == OAL_GET_THRUPUT_BYPASS_ENABLE(OAL_TX_HAL_HARDWARE_BYPASS)) /* bypass, 不发送 */
       && (DMAC_USER_ALG_SMARTANT_NULLDATA_PROBE != MAC_GET_CB_IS_PROBE_DATA(pst_cb)))//智能天线训练帧不重传
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

oal_void dmac_tx_sw_retry(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buf, hal_tx_dscr_stru *pst_dscr)
{
    oal_uint16        us_original_mpdu_len = 0;

    if ((OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_buf) || (OAL_PTR_NULL == pst_dscr))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_sw_retry: null ptr!!!}");
        return;
    }

    us_original_mpdu_len = pst_dscr->us_original_mpdu_len;

    /* 释放描述符，不释放net_buff */
    if (OAL_SUCC != OAL_MEM_FREE(pst_dscr, OAL_TRUE))
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_sw_retry::tx dscr free fail, addr=0x%x.}", pst_dscr);
    }

    /* 调用管理报文接口发送 */
    if (OAL_SUCC != dmac_tx_mgmt(pst_dmac_vap, pst_buf, us_original_mpdu_len))
    {
        oal_netbuf_free(pst_buf);
    }
}



OAL_STATIC oal_uint8  dmac_tx_softretry_check(
                dmac_vap_stru               *pst_dmac_vap,
                dmac_user_stru              *pst_dmac_user,
                hal_tx_dscr_stru            *pst_dscr,
                hal_tx_dscr_ctrl_one_param  *pst_tx_dscr_one)
{
    hal_to_dmac_device_stru            *pst_hal_device;
    oal_netbuf_stru                    *pst_buf;
    oal_dlist_head_stru                 st_pending_q;
    mac_tx_ctl_stru                    *pst_cb;
    oal_uint8                           uc_dscr_status;
    mac_ieee80211_frame_stru           *pst_mac_hdr;
    dmac_tx_chiper_stat_stru            st_chiper_stat;
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8           en_trace_pkt_type;
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    oal_uint8                           *puc_data;
#endif

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG2(0, OAM_SF_TX, "{dmac_tx_check_softretry: pst_hal_device is null, vap id[%d], user assoc id[%d].}",
                       pst_dmac_vap->st_vap_base_info.uc_vap_id,
                       pst_dmac_user->st_user_base_info.us_assoc_id);
        return OAL_FALSE;
    }

    pst_buf = pst_dscr->pst_skb_start_addr;
    pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
    pst_mac_hdr = MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb);

#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
    /* 清cache，读取mac头判断是否重传 */
    oal_dma_map_single(NULL, pst_mac_hdr, 24, OAL_FROM_DEVICE);
#endif

    /* 获取发送状态位 */
    hal_tx_get_dscr_status(pst_hal_device, pst_dscr, &uc_dscr_status);

    /* 判断管理帧/控制帧是否需要重传  */
    if (WLAN_DATA_BASICTYPE != MAC_GET_CB_WLAN_FRAME_TYPE(pst_cb))
    {
        if(WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
        {
            if((OAL_TRUE == pst_dmac_vap->en_auth_received)
                &&((WLAN_FC0_SUBTYPE_AUTH|WLAN_FC0_TYPE_MGT) == mac_get_frame_type_and_subtype((oal_uint8 *)pst_mac_hdr)))
            {
                pst_dmac_vap->en_auth_received = OAL_FALSE;
                OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_softretry_check::auth is received, not retry antuh].}");
                return OAL_FALSE;
            }

            if((OAL_TRUE == pst_dmac_vap->en_assoc_rsp_received)
                &&((WLAN_FC0_SUBTYPE_ASSOC_REQ|WLAN_FC0_TYPE_MGT) == mac_get_frame_type_and_subtype((oal_uint8 *)pst_mac_hdr)))
            {
                pst_dmac_vap->en_assoc_rsp_received = OAL_FALSE;
                OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_softretry_check::assoc_rsp is received, not retry assoc_req].}");
                return OAL_FALSE;
            }
        }

        /* 不满足重传条件 */
        if (OAL_TRUE != dmac_tx_comp_mgmt_retry_check(pst_cb, pst_mac_hdr))
        {
            return OAL_FALSE;
        }

#ifdef _PRE_WLAN_FEATURE_FTM
        /* FTM帧不重传 */
        /* 获取帧体指针 */
        puc_data = oal_netbuf_payload(pst_buf);
        if((WLAN_ACTION == pst_mac_hdr->st_frame_control.bit_sub_type)
          &&(MAC_ACTION_CATEGORY_PUBLIC == (puc_data[MAC_ACTION_OFFSET_CATEGORY]))
          &&(MAC_PUB_FTM == (puc_data[MAC_ACTION_OFFSET_ACTION])))
        {
            return OAL_FALSE;
        }
#endif

        /* 从硬件队列删除 */
        oal_dlist_delete_entry(&pst_dscr->st_entry);
        MAC_GET_CB_RETRIED_NUM(pst_cb)++;

        /* 如果是节能回退帧 */
        if (DMAC_TX_SOFT_PSM_BACK == uc_dscr_status)
        {
            /* 释放描述符 */
            OAL_MEM_FREE(pst_dscr, OAL_TRUE);
            dmac_psm_ps_enqueue(pst_dmac_vap, pst_dmac_user, pst_buf);
        }
        else
        {
            dmac_tx_sw_retry(pst_dmac_vap, pst_buf, pst_dscr);
        }

        return OAL_TRUE;
    }
    else/* 数据帧是否需要重传 */
    {
        /* 需要重传的帧和节能回退的帧才进行重传, 03在带宽能力异常时也要进行重传 */
        if ((OAL_TRUE != MAC_GET_CB_IS_NEEDRETRY(pst_cb)) && (DMAC_TX_SOFT_PSM_BACK != uc_dscr_status)
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        && (DMAC_TX_FAIL_BW_TOO_BIG != uc_dscr_status)
#endif
        )
        {
            return OAL_FALSE;
        }

        if (OAL_TRUE == mac_frame_is_null_data(pst_buf))
        {
            /* NULL报文不作重传处理 */
            return OAL_FALSE;
        }

        /* 不满足重传条件 */
        if (OAL_TRUE != dmac_tx_comp_data_retry_check(pst_dmac_vap, pst_cb))
        {
            return OAL_FALSE;
        }

        /* 从硬件队列删除 */ /* 放入用户重传队列 */
        oal_dlist_delete_entry(&pst_dscr->st_entry);
        MAC_GET_CB_RETRIED_NUM(pst_cb)++;

        /* 是否需要通知算法 */
        if ((MAC_GET_CB_IS_DATA_FRAME(pst_cb)) &&
            (!MAC_GET_CB_IS_VIPFRAME(pst_cb))&&
            (OAL_FALSE == MAC_GET_CB_IS_MCAST(pst_cb)))
        {
            dmac_tx_update_alg_param(pst_dscr, pst_cb, pst_tx_dscr_one, pst_hal_device);
            /* 通知算法 */
            dmac_tx_complete_notify(pst_hal_device, &(pst_dmac_user->st_user_base_info), pst_buf, pst_tx_dscr_one);

            /* 算法降协议删除ba会话时将此标志置为true */
            if (OAL_TRUE == pst_dmac_user->en_delete_ba_flag)
            {
                pst_dmac_user->en_delete_ba_flag = OAL_FALSE;
                /* 抛事件到HMAC执行删除动作 */
                dmac_tx_delete_ba(pst_dmac_user);
            }
        }

        /* 加入重传队列 */
        oal_dlist_init_head(&st_pending_q);
        oal_dlist_add_tail(&pst_dscr->st_entry, &st_pending_q);
        hal_tx_get_dscr_chiper_type(pst_dscr, &st_chiper_stat.uc_tx_chiper_type, &st_chiper_stat.uc_chiper_key_id);
        dmac_tx_set_retry_dscr(pst_hal_device, pst_dscr, uc_dscr_status, &st_chiper_stat);
        dmac_tx_complete_dscr_back(pst_dmac_vap, pst_dmac_user, MAC_GET_CB_WME_TID_TYPE(pst_cb), &st_pending_q, 1);

        /* AP模式,并且节能回退则更新bitmap */
        if (DMAC_TX_SOFT_PSM_BACK == uc_dscr_status && IS_AP(&pst_dmac_vap->st_vap_base_info))
        {
            /* 更新bitmap */
            dmac_psm_update_beacon(pst_dmac_vap, pst_dmac_user);
        }
#ifdef _PRE_DEBUG_MODE
        else
        {
            //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
            en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_buf), MAC_NETBUFF_PAYLOAD_SNAP);
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_check_softretry::type%d send fail, drop[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
        }
#endif

        return OAL_TRUE;
    }
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW

oal_void dmac_tx_restore_txq_to_tid(hal_to_dmac_device_stru *pst_hal_device,
                                             dmac_user_stru    *pst_dmac_user,
                                             oal_uint8          uc_tid)
{
    hal_tx_dscr_stru             *pst_tx_dscr        = OAL_PTR_NULL;
    mac_tx_ctl_stru              *pst_cb             = OAL_PTR_NULL;
    oal_dlist_head_stru          *pst_dlist_pos      = OAL_PTR_NULL;
    oal_dlist_head_stru          *pst_dlist_n        = OAL_PTR_NULL;
    dmac_vap_stru                *pst_dmac_vap;
    oal_netbuf_stru              *pst_netbuf;
    //oal_netbuf_head_stru          st_netbuf_head;
    //oal_netbuf_stru              *pst_netbuf_tmp;
    dmac_tid_stru                *pst_tid_queue = OAL_PTR_NULL;
    oal_uint8                     uc_q_idx;
    //oal_uint8                    uc_valid;
    //oal_uint16                   us_seq;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_tx_restore_txq_to_tid:: dmac vap null, vap idnex[%d].}", pst_dmac_user->st_user_base_info.uc_vap_id);
        return;
    }

    //oal_netbuf_list_head_init(&st_netbuf_head);

    uc_q_idx = WLAN_WME_TID_TO_AC(uc_tid);
    pst_tid_queue = &pst_dmac_user->ast_tx_tid_queue[uc_tid];

    /* 将发送队列的帧回收 */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_dlist_pos, pst_dlist_n, &pst_hal_device->ast_tx_dscr_queue[uc_q_idx].st_header)
    {
        pst_tx_dscr  = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_pos, hal_tx_dscr_stru, st_entry);
        pst_netbuf   = pst_tx_dscr->pst_skb_start_addr;
        pst_cb       = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);

        /* 从首帧获取用户 */
        if (OAL_TRUE == pst_tx_dscr->bit_is_first)
        {
        #if 0
            /* 入队 */
            while (!oal_netbuf_list_empty(&st_netbuf_head))
            {
                pst_netbuf_tmp = oal_netbuf_tail(&st_netbuf_head);
                oal_netbuf_delete(pst_netbuf_tmp, &st_netbuf_head);
                dmac_tid_tx_enqueue_tid_head(&pst_dmac_user->st_user_base_info, pst_tid_queue, pst_netbuf_tmp, 1);
            }

            oal_netbuf_list_head_init(&st_netbuf_head);
         #endif
            /* 更新dmac user */
            pst_dmac_user = mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_cb));
            if (OAL_PTR_NULL == pst_dmac_user)
            {
                OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_tx_restore_txq_to_tid:: dmac user null, idnex[%d].}", MAC_GET_CB_TX_USER_IDX(pst_cb));
                dmac_tx_complete_free_dscr(pst_tx_dscr);
                continue;
            }

            pst_dmac_vap = mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
            if (OAL_PTR_NULL == pst_dmac_vap)
            {
                OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_tx_restore_txq_to_tid:: dmac vap null, vap idnex[%d].}", pst_dmac_user->st_user_base_info.uc_vap_id);
                dmac_tx_complete_free_dscr(pst_tx_dscr);
                continue;
            }

            pst_tid_queue = &pst_dmac_user->ast_tx_tid_queue[MAC_GET_CB_WME_TID_TYPE(pst_cb)];
        }

        //hal_tx_get_dscr_seqnum(pst_tx_dscr, &us_seq, &uc_valid);
        //OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_tx_restore_txq_to_tid:: seq num[%d] valid[%d].}",us_seq,uc_valid);

        /* 硬件队列回收的未被确认的帧都做新帧处理 */
        oal_dlist_delete_entry(&pst_tx_dscr->st_entry);
        OAL_MEM_FREE(pst_tx_dscr, OAL_TRUE);
        //oal_netbuf_add_to_list_tail(pst_netbuf, &st_netbuf_head);
        dmac_tid_tx_enqueue_tid_head(&pst_dmac_user->st_user_base_info, pst_tid_queue, pst_netbuf);
    }

    pst_hal_device->ast_tx_dscr_queue[uc_q_idx].uc_ppdu_cnt = 0;
    /* 清空MAC FIFO */
    hal_clear_hw_fifo(pst_hal_device);
}
#endif

#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rodata, code, rwdata, zidata  // return to default placement
#endif


OAL_STATIC OAL_INLINE oal_uint32  dmac_al_tx_free_long_frame(hal_tx_dscr_stru   *pst_dscr, oal_netbuf_stru *pst_buf)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    mac_tx_ctl_stru   *pst_tx_ctl;
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
    if(WLAN_LARGE_NETBUF_SIZE < MAC_GET_CB_MPDU_LEN(pst_tx_ctl))
    {
        //OAM_INFO_LOG1(0, OAM_SF_TX, "MAC_GET_CB_MPDU_BYTES(pst_tx_ctl) %u", MAC_GET_CB_MPDU_BYTES(pst_tx_ctl));
        oal_dlist_delete_entry(&pst_dscr->st_entry);
        OAL_MEM_FREE(pst_dscr, OAL_TRUE);
        OAL_MEM_MULTI_NETBUF_FREE(pst_buf);
        return OAL_SUCC;
    }
#endif
#endif
    return OAL_FAIL;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#if 0

OAL_STATIC oal_uint32  dmac_psm_enqueue_psm_back_netbuf_tx_complete(
    hal_to_dmac_device_stru *pst_hal_device,
    hal_tx_dscr_stru  *pst_tx_dscr,
    dmac_vap_stru *pst_dmac_vap,
    dmac_user_stru *pst_dmac_user,
    oal_netbuf_stru *pst_net_buf)
{
    hal_tx_queue_type_enum_uint8 en_queue_num;

    if ((OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
        || (OAL_UNLIKELY(OAL_PTR_NULL == pst_tx_dscr))
        || (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
        || (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
        || (OAL_UNLIKELY(OAL_PTR_NULL == pst_net_buf))
        )
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_enqueue_psm_back_netbuf_tx_complete::ptr is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_queue_num = pst_tx_dscr->uc_q_num;


    /* 释放描述符 */
    oal_dlist_delete_entry(&pst_tx_dscr->st_entry);
    OAL_MEM_FREE(pst_tx_dscr, OAL_TRUE);
    /* 放到节能队列，此时节能队列为空，enqueue操作后，数据排序同硬件队列的先后顺序 */

#ifdef _PRE_WLAN_FEATURE_UAPSD
    if((pst_dmac_user->uc_uapsd_flag) & MAC_USR_UAPSD_EN)
    {
        dmac_uapsd_tx_enqueue(pst_dmac_vap, pst_dmac_user, pst_net_buf);
    }
    else
#endif
    {
        dmac_psm_enqueue(pst_dmac_vap, pst_dmac_user, pst_net_buf);
    }
    /* 从硬件队列删除ppdu后，需要将ppducnt减1 */
    pst_hal_device->ast_tx_dscr_queue[en_queue_num].uc_ppdu_cnt
        = OAL_SUB(pst_hal_device->ast_tx_dscr_queue[en_queue_num].uc_ppdu_cnt, 1);

    return OAL_SUCC;

}
#endif
#endif


#ifdef _PRE_WLAN_DFT_STAT
static oal_void dmac_performance_show_mpdu_info(oal_uint8 uc_vap_id,oal_uint8 uc_dscr_status,hal_tx_dscr_ctrl_one_param *pst_tx_dscr_one)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    if(DFX_GET_PERFORMANCE_LOG_SWITCH_ENABLE(DFX_PERFORMANCE_TX_LOG))
    {/*lint -e437*/
        OAM_WARNING_LOG_ALTER(uc_vap_id, OAM_SF_TX,
                            "{performance debug::uc_dscr_status[%d],mpdu_num[%d],mpdu_len[%d],ac[%d],bw[%d],rate[%d],long_retry[%d],cts_fail[%d].}",
                            8,
                            uc_dscr_status,
                            pst_tx_dscr_one->uc_mpdu_num,
                            pst_tx_dscr_one->us_mpdu_len,
                            pst_tx_dscr_one->uc_ac,
                            pst_tx_dscr_one->uc_bandwidth,
                            pst_tx_dscr_one->ast_per_rate[0],
                            pst_tx_dscr_one->uc_long_retry,
                            pst_tx_dscr_one->uc_cts_failure);/*lint +e437*/
    }
#endif
}
#endif

#ifdef _PRE_WLAN_DFT_STAT
static oal_void dmac_successive_fail_dump_info(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, hal_tx_dscr_stru *pst_dscr)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    if(pst_dmac_vap->st_query_stats.uc_tx_successive_mpdu_fail_num <= DMAC_MAX_TX_SUCCESSIVE_FAIL_PRINT_THRESHOLD)
    {
        return;
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    if((OAL_TRUE == DMAC_VAP_GET_HAL_CHIP(pst_dmac_vap)->st_btcoex_btble_status.un_bt_status.st_bt_status.bit_bt_on) &&
        (pst_dmac_vap->st_query_stats.uc_tx_successive_mpdu_fail_num <= DMAC_MAX_TX_SUCCESSIVE_FAIL_PRINT_THRESHOLD_BTCOEX))
    {
        return;
    }
#endif

    oam_report_dscr(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                     (oal_uint8 *)pst_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);

    dmac_dft_report_all_ota_state(&pst_dmac_vap->st_vap_base_info);
    DMAC_VAP_DFT_STATS_PKT_SET_ZERO(pst_dmac_vap->st_query_stats.uc_tx_successive_mpdu_fail_num);
#endif
}
#else
#define dmac_successive_fail_dump_info(pst_dmac_vap, pst_dmac_user, pst_dscr)
#endif

#ifdef _PRE_WLAN_FEATURE_VOWIFI

oal_void dmac_update_vowifi_tx_cnt(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{

    /* 外部保证pst_dmac_vap和pst_dmac_user不为空!  */
    if (OAL_PTR_NULL == pst_dmac_vap->pst_vowifi_status)
    {
        return;
    }

    pst_dmac_vap->pst_vowifi_status->ul_tx_total  = pst_dmac_user->st_query_stats.ul_tx_total;
    pst_dmac_vap->pst_vowifi_status->ul_tx_failed = pst_dmac_user->st_query_stats.ul_tx_failed;

}

#endif //_PRE_WLAN_FEATURE_VOWIFI
#if defined(_PRE_WLAN_FEATURE_DBAC) && defined(_PRE_PRODUCT_ID_HI110X_DEV)

oal_void dmac_tx_complete_disasoc_frame_process(hal_to_dmac_device_stru *pst_hal_device, oal_netbuf_stru *pst_buf)
{
    mac_ieee80211_frame_stru           *pst_mac_header;
    oal_uint8                           uc_mgmt_subtype;
    oal_uint8                           uc_mgmt_type;

    pst_mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_buf);
    uc_mgmt_type    = mac_frame_get_type_value((oal_uint8 *)pst_mac_header);
    uc_mgmt_subtype = mac_frame_get_subtype_value((oal_uint8 *)pst_mac_header);

    if (WLAN_MANAGEMENT == uc_mgmt_type && WLAN_DISASOC == uc_mgmt_subtype)
    {
        dmac_alg_dbac_resume(pst_hal_device, OAL_FALSE);
    }
}
#endif /* defined(_PRE_WLAN_FEATURE_DBAC) && defined(_PRE_PRODUCT_ID_HI110X_DEV) */


#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rwdata = "BTCM", code ="ATCM", zidata = "BTCM", rodata = "ATCM"
#endif


/*lint -save -e438 */
OAL_STATIC oal_uint32  dmac_tx_complete_normal_buffer(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr)
{
    mac_tx_ctl_stru                    *pst_cb;
    dmac_user_stru                     *pst_dmac_user;
    hal_tx_dscr_ctrl_one_param          st_tx_dscr_one = {0};
    oal_uint8                           uc_dscr_status = DMAC_TX_INVALID;
    oal_uint32                          ul_seq    = 0;
    oal_uint8                           uc_data_tx_cnt = 0;
    oal_uint8                           uc_is_qos;
    oal_uint16                          us_user_idx      = 0;
    mac_device_stru                    *pst_mac_device;
    dmac_vap_stru                      *pst_dmac_vap;
    mac_vap_stru                       *pst_mac_vap;
    hal_to_dmac_vap_stru               *pst_hal_vap;                                       /* hal vap结构 */
    oal_uint32                          ul_ret;
    oal_uint8                           uc_q_num;
    oal_netbuf_stru                    *pst_buf;
    oal_uint8                           uc_vap_id;
    oal_uint8                           uc_mem_state = 0;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8                           uc_print_permit = 0;
#endif
#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    oal_uint8                           uc_tid;
#else
    oal_uint8                           uc_data_tx_rate0 = 0;
#endif /* (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) */
#ifdef _PRE_WLAN_PERFORM_STAT
    oal_uint8                           uc_hw_retry_num    = 0;
    oal_uint8                           uc_rate_index      = 0;
#endif /* _PRE_WLAN_PERFORM_STAT */

#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8      en_trace_pkt_type;
#endif

#ifdef _PRE_WLAN_11K_STAT
    oal_time_us_stru                        st_time;

    /* 获取系统时间 */
    oal_time_get_stamp_us(&st_time);
#endif

    uc_q_num = pst_dscr->uc_q_num;

    pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt
                = OAL_SUB(pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt, 1);

    pst_buf = pst_dscr->pst_skb_start_addr;
    if (OAL_PTR_NULL == pst_buf)
    {
        OAL_GET_MEM_FLAG(pst_dscr, &uc_mem_state);
        if (OAL_MEM_STATE_FREE != uc_mem_state)
        {
            dmac_tx_complete_free_dscr(pst_dscr);
        }
        OAM_ERROR_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::pst_buf null,mem state[%d].}", uc_mem_state);
        return OAL_ERR_CODE_PTR_NULL;
    }

    dmac_tx_get_vap_id(pst_hal_device, pst_dscr, &uc_vap_id);

    pst_cb   = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);

#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    uc_tid   = MAC_GET_CB_WME_TID_TYPE(pst_cb);
#endif

    OAL_MEM_TRACE(pst_dscr, OAL_FALSE);

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::pst_mac_device null. device_id=%d.}", pst_hal_device->uc_mac_device_id);
        dmac_tx_complete_free_dscr(pst_dscr);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_DFT_STAT
    /* 硬件发送完成的管理帧统计 */
    if (HAL_TX_QUEUE_HI == uc_q_num)
    {
        dmac_dft_mgmt_stat_incr(pst_mac_device, oal_netbuf_header(pst_buf), MAC_DEV_MGMT_STAT_TYPE_TX_COMPLETE);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    /* 由于02裸系统，需要使用multi_free释放大内存 */
    if(OAL_SUCC == dmac_al_tx_free_long_frame(pst_dscr, pst_buf))
    {
        return OAL_SUCC;
    }
#endif

    if (OAL_UNLIKELY(0 == uc_vap_id))
    {
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::vap id is 0, mac vap had been deleted.}");

        dmac_tx_complete_free_dscr(pst_dscr);
        return OAL_SUCC;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        dmac_tx_complete_free_dscr(pst_dscr);
        return OAL_SUCC;
    }
    pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);

    /* 获取发送状态位 */
    hal_tx_get_dscr_status(pst_hal_device, pst_dscr, &uc_dscr_status);

#if defined(_PRE_WLAN_FEATURE_DBAC) && defined(_PRE_PRODUCT_ID_HI110X_DEV)
    dmac_tx_complete_disasoc_frame_process(pst_hal_device, pst_buf);
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
    if (uc_dscr_status == DMAC_TX_SUCC)
    {
        /* 发送ftm帧完成，记录t1和t4 */
        if(mac_check_ftm_enable(pst_mac_vap))
        {
            dmac_check_tx_ftm(pst_dmac_vap, pst_buf);
        }
    }
#endif

    /* 获取DMAC模块用户结构体 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_cb));
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        /* 检查cb字段 bit_need_rsp, 上报hmac 传送状态*/
        if (OAL_TRUE == MAC_GET_CB_IS_NEED_RESP(pst_cb))
        {
            /* mgmt tx 结束上报 */
            dmac_mgmt_tx_complete(&(pst_dmac_vap->st_vap_base_info), MAC_GET_CB_MGMT_FRAME_ID(pst_cb), uc_dscr_status, MAC_GET_CB_TX_USER_IDX(pst_cb));
        }

        dmac_tx_complete_free_dscr(pst_dscr);
        return OAL_SUCC;
    }

    /* 从mac地址获取用户索引，没找到的话，说明这个mac用户不存在，直接丢弃 */
    /* 用户索引idx为全f，表示用户已经删除或者用户idx与cb字段中不一致，表示用户曾经关联又关联上后，原来的数据发送处理都直接丢弃 */
    if (MAC_GET_CB_TX_USER_IDX(pst_cb) != pst_mac_vap->us_multi_user_idx)
    {
        ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_dmac_user->st_user_base_info.auc_user_mac_addr, &us_user_idx);
        if((OAL_SUCC != ul_ret)|| (us_user_idx != MAC_GET_CB_TX_USER_IDX(pst_cb)))
        {
            dmac_tx_complete_free_dscr(pst_dscr);
            return OAL_SUCC;
        }
    }

#ifdef _PRE_DEBUG_MODE_USER_TRACK
    dmac_tx_complete_check_protocol(pst_hal_device, pst_dmac_user, pst_dscr);
#endif

    if (uc_dscr_status != DMAC_TX_SUCC)
    {
        dmac_vap_linkloss_threshold_decr(pst_dmac_vap);

        /* 统计连续发送失败的的次数 */
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.uc_tx_successive_mpdu_fail_num, 1);
        dmac_successive_fail_dump_info(pst_dmac_vap, pst_dmac_user, pst_dscr);
        /* 获取用户指定TID的单播帧的发送完成的次数 */
        uc_is_qos = MAC_GET_CB_IS_QOS_DATA(pst_cb);
        if( OAL_TRUE == uc_is_qos)
        {
            hal_tx_get_dscr_seq_num(pst_hal_device, pst_dscr, (oal_uint16 *)&ul_seq);
        }
        else
        {
            pst_hal_vap = pst_dmac_vap->pst_hal_vap;
            if (OAL_PTR_NULL != pst_hal_vap)
            {
                hal_get_tx_sequence_num(pst_hal_device, 0, 0, 0,pst_hal_vap->uc_vap_id, &ul_seq);
            }
        }

#ifdef _PRE_WLAN_FEATURE_P2P
        if (!(OAL_TRUE == pst_mac_device->st_p2p_info.en_p2p_ps_pause && DMAC_TX_SOFT_PSM_BACK == uc_dscr_status))
#endif
        {
            hal_tx_get_dscr_tx_cnt(pst_hal_device, pst_dscr, &uc_data_tx_cnt);

        #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            dmac_tx_complete_sdt_stat(pst_hal_device, (oal_uint16)ul_seq, &uc_print_permit);
            if (OAL_TRUE == uc_print_permit)
        #endif
            {
            #if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1151)
                OAM_WARNING_LOG4(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::tx dscr status=%d,tx_cnt=%d,ul_seq=%d, isqos=%d.}",
                            uc_dscr_status,uc_data_tx_cnt, ul_seq, uc_is_qos);
            #else
                hal_tx_dscr_get_rate3(pst_hal_device, pst_dscr, &uc_data_tx_rate0);
                OAM_WARNING_LOG_ALTER(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::tx dscr status=%d,tx_cnt=%d,ul_seq=%d, isqos=%d, rate3=%d Mbps/mcs.}", 5,
                            uc_dscr_status, uc_data_tx_cnt, ul_seq, uc_is_qos, uc_data_tx_rate0);
            #endif

                OAM_WARNING_LOG4(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::cb user idx=%d,user idx=%d, vap multi useridx=%d, vap id=%d.}",
                            MAC_GET_CB_TX_USER_IDX(pst_cb), us_user_idx, pst_mac_vap->us_multi_user_idx, pst_dmac_user->st_user_base_info.uc_vap_id);

            }

        }

#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_buf), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::type%d send fail, retry[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif

        st_tx_dscr_one.uc_error_mpdu_num = 1;
        OAM_STAT_VAP_INCR(uc_vap_id, tx_mpdu_fail_num, 1);
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_mpdu_fail_num, 1);

        #if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
        OAM_STAT_TID_INCR(pst_dmac_user->st_user_base_info.us_assoc_id,
                          uc_tid, tx_mpdu_fail_num, 1);
        #endif

        /* 查看是否需要重传，如果是不需要释放，直接返回，如果不是继续后续处理 */
        ul_ret = dmac_tx_softretry_check(pst_dmac_vap, pst_dmac_user, pst_dscr, &st_tx_dscr_one);
        /* 不是以太网来的不统计 */
        if (FRW_EVENT_TYPE_HOST_DRX == MAC_GET_CB_EVENT_TYPE(pst_cb))
        {
            oal_uint8 uc_retries;

            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_tx_failed,((OAL_TRUE == ul_ret)? 0:1 ));
            DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_failed, ((OAL_TRUE == ul_ret)? 0:1 ));
            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_tx_retries,st_tx_dscr_one.uc_long_retry);
            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_tx_retries,st_tx_dscr_one.uc_short_retry);
            DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_retries,st_tx_dscr_one.uc_long_retry);
            DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_retries,st_tx_dscr_one.uc_short_retry);
            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_hw_tx_failed,1);

            uc_retries = st_tx_dscr_one.uc_long_retry + st_tx_dscr_one.uc_short_retry;
            if (uc_retries > 0)
            {
                uc_retries -= 1;    /* 减掉传输成功的1次 */
            }

            OAM_STAT_USER_INCR(pst_dmac_user->st_user_base_info.us_assoc_id, tx_ppdu_retries, uc_retries);
        }
#ifdef _PRE_WLAN_DFT_STAT
        DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_cts_fail,st_tx_dscr_one.uc_cts_failure);
#endif
        if (OAL_TRUE == ul_ret)
        {
            pst_dmac_user->st_ps_structure.en_is_pspoll_rsp_processing = OAL_FALSE;
            return OAL_SUCC;
        }
        else
        {
#ifdef _PRE_WLAN_11K_STAT
            dmac_user_stat_tx_mpdu_info(pst_dmac_user, pst_dmac_vap, pst_cb, uc_dscr_status, OAL_TRUE);
#endif
        }
    }
    else
    {
        /* 统计发送成功的MPDU个数 */
        DMAC_VAP_DFT_STATS_PKT_SET_ZERO(pst_dmac_vap->st_query_stats.uc_tx_successive_mpdu_fail_num);
        OAM_STAT_VAP_INCR(uc_vap_id, tx_mpdu_succ_num, 1);
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_mpdu_succ_num, 1);
        #if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
        OAM_STAT_TID_INCR(pst_dmac_user->st_user_base_info.us_assoc_id, uc_tid, tx_mpdu_succ_num, 1);
        OAM_STAT_TID_INCR(pst_dmac_user->st_user_base_info.us_assoc_id, uc_tid, tx_mpdu_bytes, MAC_GET_CB_MPDU_LEN(pst_cb));
        #endif

#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_buf), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::type%d send succuss[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif

        /* 不是以太网来的不统计 */
        if (FRW_EVENT_TYPE_HOST_DRX == MAC_GET_CB_EVENT_TYPE(pst_cb))
        {
            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_hw_tx_pkts,MAC_GET_CB_NETBUF_NUM(pst_cb));
            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_hw_tx_bytes,MAC_GET_CB_MPDU_LEN(pst_cb));
            DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_hw_tx_pkts, MAC_GET_CB_NETBUF_NUM(pst_cb));
            DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_hw_tx_bytes, MAC_GET_CB_MPDU_LEN(pst_cb));
      #ifdef _PRE_WLAN_FEATURE_VOWIFI
            dmac_update_vowifi_tx_cnt(pst_dmac_vap, pst_dmac_user);
      #endif
        }

        /* 发送成功 清空linkloss计数 */
        if (MAC_VAP_STATE_UP == pst_dmac_vap->st_vap_base_info.en_vap_state)
        {
            dmac_vap_linkloss_clean(pst_dmac_vap);
            dmac_vap_linkloss_threshold_incr(pst_dmac_vap);
        }

#ifdef _PRE_WLAN_FEATURE_SMPS
        //dmac_smps_check_rx_action(pst_hal_device, pst_buf);
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
        dmac_m2s_send_action_complete_check(pst_mac_vap, pst_cb);
#endif
    }

#ifdef _PRE_WLAN_11K_STAT
    dmac_user_stat_tx_mpdu_info(pst_dmac_user, pst_dmac_vap, pst_cb, uc_dscr_status, OAL_FALSE);
#endif
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    dmac_user_stat_tx_mcs_count(pst_dmac_user, pst_dmac_vap, pst_cb, &st_tx_dscr_one);
#endif

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TX_COMP_GET_DSCR_STAT);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_COMP_GET_DSCR_STAT);

    /* 更新keepalive计数器 */
    dmac_keepalive_timestamp_update(pst_mac_vap, pst_dmac_user, (oal_uint8 *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb), uc_dscr_status);

    /* 更新mib库信息 */
    dmac_tx_update_alg_param(pst_dscr, pst_cb, &st_tx_dscr_one, pst_hal_device);
#ifdef _PRE_WLAN_DFT_STAT
    dmac_performance_show_mpdu_info(uc_vap_id, uc_dscr_status, &st_tx_dscr_one);
#endif
#ifdef _PRE_WLAN_FEATURE_UAPSD
    /*UAPSD 检查EOSP位，结束一个USP*/
    dmac_uapsd_tx_complete(pst_dmac_user,pst_cb);
#endif

    /* STA侧pspoll低功耗处理 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        dmac_psm_tx_complete_sta(pst_dmac_vap, pst_dscr, pst_buf);
    }
#endif

#if defined(_PRE_WLAN_FEATURE_20_40_80_COEXIST) && defined(_PRE_WLAN_FEATURE_DFS)
    dmac_chan_tx_complete_suspend_tx(pst_mac_device, pst_mac_vap, pst_hal_device, pst_buf);
#elif defined(_PRE_WLAN_FEATURE_20_40_80_COEXIST) && !defined(_PRE_WLAN_FEATURE_DFS)
    dmac_chan_tx_complete_2040_coexist(pst_mac_device, pst_hal_device, pst_buf);
#endif /* end of _PRE_WLAN_FEATURE_DFS */

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TX_COMP_PROCESS_FEATURE);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_COMP_PROCESS_FEATURE);

    /* 将ps_poll标志清除，表明此次ps-poll已经处理完成，如果再接收到ps-poll可以处理 */
    pst_dmac_user->st_ps_structure.en_is_pspoll_rsp_processing = OAL_FALSE;

    /* 调管理帧发送完成处理钩子 */
    dmac_tx_complete_mgmt_notify(pst_hal_device, pst_dmac_user, pst_dscr, pst_buf);
    if ((MAC_GET_CB_IS_DATA_FRAME(pst_cb)) &&
        (!MAC_GET_CB_IS_VIPFRAME(pst_cb)) &&
        (OAL_FALSE == MAC_GET_CB_IS_MCAST(pst_cb)))
    {
#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
        dmac_tx_complete_normal_pkt_num(pst_buf, &st_tx_dscr_one, us_user_idx);
#endif

        /* 通知算法 */
        dmac_tx_complete_notify(pst_hal_device, &(pst_dmac_user->st_user_base_info), pst_buf, &st_tx_dscr_one);

        /* 算法降协议删除ba会话时将此标志置为true */
        if (OAL_TRUE == pst_dmac_user->en_delete_ba_flag)
        {
            pst_dmac_user->en_delete_ba_flag = OAL_FALSE;

            /* 抛事件到HMAC执行删除动作 */
            dmac_tx_delete_ba(pst_dmac_user);
        }
    }
    else if((pst_dmac_user->bit_is_rx_eapol_key_open == OAL_FALSE)
            && (OAL_TRUE == mac_is_eapol_key_ptk_4_4(pst_buf)))
    {
        /* 如果用户需要加密EAPOL-KEY, 且4/4 EAPOL-KEY 发送成功，则设置秘钥 */
        pst_dmac_user->bit_eapol_key_4_4_tx_succ = OAL_TRUE;
        if (pst_dmac_user->bit_ptk_need_install == OAL_TRUE)
        {
            /* 更新单播秘钥 */
            ul_ret = dmac_config_11i_add_key_set_reg(&(pst_dmac_vap->st_vap_base_info),
                                                    pst_dmac_user->bit_ptk_key_idx,
                                                    pst_dmac_user->st_user_base_info.auc_user_mac_addr);
            OAM_WARNING_LOG4(0, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::set ptk succ.ret %d, key_idx %d. %02X:XX:XX:XX:XX:%02X",
                    ul_ret,
                    pst_dmac_user->bit_ptk_key_idx,
                    pst_dmac_user->st_user_base_info.auc_user_mac_addr[0],
                    pst_dmac_user->st_user_base_info.auc_user_mac_addr[5]);

            pst_dmac_user->bit_ptk_need_install      = OAL_FALSE;
            pst_dmac_user->bit_eapol_key_4_4_tx_succ = OAL_FALSE;
            pst_dmac_user->bit_ptk_key_idx           = 0;
        }
    }
#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
    else
    {
        if (MAC_GET_CB_ALG_TAGS(pst_cb) & DMAC_CB_ALG_TAGS_MUCTRL_MASK)
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::trace_muctl");
        }
    }
#endif

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TX_COMP_MGMT_NOTIFY);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_COMP_MGMT_NOTIFY);

#ifdef _PRE_WLAN_11K_STAT
    dmac_user_stat_tx_frm_info(pst_dmac_user, pst_dmac_vap, st_tx_dscr_one.uc_tid ,&st_tx_dscr_one, OAL_FALSE);

    /* 获取系统时间 */
    oal_time_get_stamp_us(&st_time);
    if (DMAC_TX_SUCC == uc_dscr_status)
    {
        dmac_user_stat_tx_delay(pst_dmac_user, pst_dmac_vap, st_tx_dscr_one.uc_tid, pst_cb, &st_time);
    }
#endif

#ifdef _PRE_WLAN_PERFORM_STAT
    dmac_stat_tx_thrpt(pst_dmac_user, st_tx_dscr_one.uc_tid ,st_tx_dscr_one);

    if (DMAC_TX_SUCC != uc_dscr_status)
    {
        dmac_stat_tid_per(&(pst_dmac_user->st_user_base_info), st_tx_dscr_one.uc_tid, 0, 1, DMAC_STAT_PER_SW_RETRY_MPDU);
    }

    dmac_stat_tid_per(&(pst_dmac_user->st_user_base_info), st_tx_dscr_one.uc_tid, st_tx_dscr_one.uc_mpdu_num, st_tx_dscr_one.uc_error_mpdu_num, DMAC_STAT_PER_MAC_TOTAL);
    dmac_stat_tid_per(&(pst_dmac_user->st_user_base_info), st_tx_dscr_one.uc_tid, st_tx_dscr_one.uc_rts_succ + st_tx_dscr_one.uc_cts_failure, st_tx_dscr_one.uc_cts_failure, DMAC_STAT_PER_RTS_FAIL);

    /* 统计mac层per，包括硬件及软件重传 */
    for (uc_rate_index = 0; uc_rate_index <= st_tx_dscr_one.uc_last_rate_rank; uc_rate_index++)
    {
        uc_hw_retry_num += st_tx_dscr_one.ast_per_rate[uc_rate_index].rate_bit_stru.bit_tx_count;
    }
    dmac_stat_tid_per(&(pst_dmac_user->st_user_base_info),
                        st_tx_dscr_one.uc_tid,
                        (st_tx_dscr_one.uc_mpdu_num * uc_hw_retry_num),
                        (st_tx_dscr_one.uc_mpdu_num * uc_hw_retry_num - 1) + st_tx_dscr_one.uc_error_mpdu_num,
                        DMAC_STAT_PER_HW_SW_FAIL);
#endif

#ifdef _PRE_WLAN_CHIP_TEST
    DMAC_CHIP_TEST_CALL(dmac_test_sch_stat_tx_mpdu_num(pst_dmac_user, st_tx_dscr_one.uc_tid ,st_tx_dscr_one, pst_cb));
#endif

#ifdef _PRE_DEBUG_MODE
    if ((pst_dscr->uc_q_num < HAL_TX_QUEUE_HI))
    {
        g_ast_tx_complete_stat[pst_hal_device->uc_mac_device_id].ul_tx_complete_bh3_num++;
        OAM_INFO_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_normal_buffer::ul_tx_complete_bh3_num = %d.", g_ast_tx_complete_stat[pst_hal_device->uc_mac_device_id].ul_tx_complete_bh3_num);
    }
#endif

    dmac_tx_complete_free_dscr(pst_dscr);

    return OAL_SUCC;
}
/*lint -restore */

#ifdef _PRE_WLAN_INIT_PTK_TX_PN
#if 0

OAL_STATIC oal_bool_enum dmac_check_iv_word(oal_uint32 ul_iv_ms_word, oal_uint32 ul_iv_ls_word, oal_uint8 uc_chip_id,  oal_uint8 uc_ra_lut_index)
{
    /* 如果iv word 全零则不做比较 */
    if ((ul_iv_ms_word == 0) && (ul_iv_ls_word == 0))
    {
        return OAL_TRUE;
    }

    /* ra lut index 不对不检查 */
    if (OAL_UNLIKELY(uc_ra_lut_index != g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].uc_pn_peer_idx))
    {
        OAM_WARNING_LOG2(0, OAM_SF_TX, "{dmac_check_iv_word::uc_ra_lut_index[%d] != g_ul_prev_iv_word.uc_pn_peer_idx[%d].}",
                                            uc_ra_lut_index, g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].uc_pn_peer_idx);
        return OAL_TRUE;
    }

    /* 首先检查iv ms word，iv ms word检查往大和往小两种情况;
       在iv ms word相等的情况下，再检查iv ls word，差值大于1000的情况则上报；
       并假设iv word增长到FFFFFFFFFFFF的情况不永远不会发生 */
    if ((ul_iv_ms_word < g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_msb) &&
        (g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_msb - ul_iv_ms_word > 1))
    {
        return OAL_FALSE;
    }

    if ((ul_iv_ms_word > g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_msb) &&
        (ul_iv_ms_word - g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_msb > 1))
    {
        return OAL_FALSE;
    }

    if (ul_iv_ms_word == g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_msb)
    {
        if ((ul_iv_ls_word > g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_lsb) &&
            (ul_iv_ls_word - g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_lsb > 1000))
        {
            return OAL_FALSE;
        }

        if ((ul_iv_ls_word < g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_lsb) &&
            (g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_lsb - ul_iv_ls_word > 1000))
        {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}


OAL_STATIC oal_bool_enum dmac_update_iv_word(oal_uint32 ul_iv_ms_word, oal_uint32 ul_iv_ls_word, oal_uint8 uc_chip_id, oal_uint8 uc_ra_lut_index)
{

    if (OAL_UNLIKELY(uc_ra_lut_index != g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].uc_pn_peer_idx) ||
        OAL_UNLIKELY(uc_ra_lut_index != g_ul_max_iv_word[uc_chip_id][uc_ra_lut_index].uc_pn_peer_idx))
    {
        OAM_WARNING_LOG3(0, OAM_SF_TX, "{dmac_update_iv_word::uc_ra_lut_index[%d] != g_ul_prev_iv_word.uc_pn_peer_idx[%d] or g_ul_max_iv_word.uc_pn_peer_idx[%d].}",
                                                    uc_ra_lut_index, g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].uc_pn_peer_idx, g_ul_max_iv_word[uc_chip_id][uc_ra_lut_index].uc_pn_peer_idx);
        return OAL_FALSE;
    }

    if ((ul_iv_ms_word != 0) || (ul_iv_ls_word != 0))
    {
        g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_msb = ul_iv_ms_word;
        g_ul_prev_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_lsb = ul_iv_ls_word;

        if ((ul_iv_ms_word > g_ul_max_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_msb) ||
            ((ul_iv_ms_word == g_ul_max_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_msb) &&
             (ul_iv_ls_word > g_ul_max_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_lsb)))
        {
            g_ul_max_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_msb = ul_iv_ms_word;
            g_ul_max_iv_word[uc_chip_id][uc_ra_lut_index].ul_pn_lsb = ul_iv_ls_word;
        }
    }

    return OAL_TRUE;
}
#endif


oal_bool_enum dmac_init_iv_word_lut(hal_to_dmac_device_stru *pst_hal_device, hal_security_key_stru *pst_security_key,oal_uint32 ul_pn_msb)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_init_iv_word_lut::pst_hal_device null.}\r\n");
        return OAL_FALSE;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_security_key))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_init_iv_word_lut::pst_security_key null.}\r\n");
        return OAL_FALSE;
    }

    if ((WLAN_KEY_TYPE_PTK != pst_security_key->en_key_type) ||
        (WLAN_80211_CIPHER_SUITE_CCMP != pst_security_key->en_cipher_type))
    {
        return OAL_FALSE;
    }

    if (OAL_UNLIKELY( pst_security_key->uc_lut_idx != g_ul_prev_iv_word[pst_hal_device->uc_chip_id][pst_security_key->uc_lut_idx].uc_pn_peer_idx) ||
        OAL_UNLIKELY( pst_security_key->uc_lut_idx != g_ul_max_iv_word[pst_hal_device->uc_chip_id][pst_security_key->uc_lut_idx].uc_pn_peer_idx))
    {
        OAM_WARNING_LOG3(0, OAM_SF_TX, "{dmac_init_iv_word_lut::uc_ra_lut_index[%d] != g_ul_prev_iv_word.uc_pn_peer_idx[%d] or g_ul_max_iv_word.uc_pn_peer_idx[%d].}",
                                            pst_security_key->uc_lut_idx, g_ul_prev_iv_word[pst_hal_device->uc_chip_id][pst_security_key->uc_lut_idx].uc_pn_peer_idx, g_ul_max_iv_word[pst_hal_device->uc_chip_id][pst_security_key->uc_lut_idx].uc_pn_peer_idx);
        return OAL_FALSE;
    }

    g_ul_prev_iv_word[pst_hal_device->uc_chip_id][pst_security_key->uc_lut_idx].ul_pn_msb = ul_pn_msb;
    g_ul_prev_iv_word[pst_hal_device->uc_chip_id][pst_security_key->uc_lut_idx].ul_pn_lsb = 0x20000000;

    g_ul_max_iv_word[pst_hal_device->uc_chip_id][pst_security_key->uc_lut_idx].ul_pn_msb = ul_pn_msb;
    g_ul_max_iv_word[pst_hal_device->uc_chip_id][pst_security_key->uc_lut_idx].ul_pn_lsb = 0x20000000;

    return OAL_TRUE;
}
#endif

#ifdef _PRE_WLAN_PERFORM_STAT
OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_perform_stat(dmac_user_stru *pst_dmac_user, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_one)
{
    oal_uint8                    uc_hw_retry_num    = 0;
    oal_uint8                    uc_rate_index      = 0;

    /* 性能统计日志 */
    dmac_stat_tid_per(&(pst_dmac_user->st_user_base_info), pst_tx_dscr_one->uc_tid, pst_tx_dscr_one->uc_mpdu_num, pst_tx_dscr_one->uc_error_mpdu_num, DMAC_STAT_PER_MAC_TOTAL);
    dmac_stat_tid_per(&(pst_dmac_user->st_user_base_info), pst_tx_dscr_one->uc_tid, pst_tx_dscr_one->uc_rts_succ + pst_tx_dscr_one->uc_cts_failure, pst_tx_dscr_one->uc_cts_failure, DMAC_STAT_PER_RTS_FAIL);

    for (uc_rate_index = 0; uc_rate_index <= pst_tx_dscr_one->uc_last_rate_rank; uc_rate_index++)
    {
        uc_hw_retry_num += pst_tx_dscr_one->ast_per_rate[uc_rate_index].rate_bit_stru.bit_tx_count;
    }

    dmac_stat_tid_per(&(pst_dmac_user->st_user_base_info),
                        pst_tx_dscr_one->uc_tid,
                        (pst_tx_dscr_one->uc_mpdu_num * uc_hw_retry_num),
                        (pst_tx_dscr_one->uc_mpdu_num * (uc_hw_retry_num - 1) + pst_tx_dscr_one->uc_error_mpdu_num),
                        DMAC_STAT_PER_HW_SW_FAIL);

    dmac_stat_tx_thrpt(pst_dmac_user, pst_tx_dscr_one->uc_tid, (*pst_tx_dscr_one));
}
#endif

/*lint -save -e438 */
OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_retry_stat(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, mac_tx_ctl_stru *pst_cb,
                                                                    hal_tx_dscr_ctrl_one_param st_tx_dscr_one, oal_uint8 uc_dscr_num, oal_uint8 uc_retry_num)
{
    oal_uint8 uc_retries;

    /* 不是以太网来的不统计 */
    if (FRW_EVENT_TYPE_HOST_DRX == MAC_GET_CB_EVENT_TYPE(pst_cb))
    {

        /* 重传帧统计 : ampdu硬件整体重传次数*mpdu个数 + 现在准备重传的mpdu个数 */
        DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_tx_retries,(st_tx_dscr_one.uc_long_retry * uc_dscr_num + uc_retry_num));
        DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_tx_retries,st_tx_dscr_one.uc_short_retry);
        DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_retries,(st_tx_dscr_one.uc_long_retry * uc_dscr_num + uc_retry_num));
        DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_retries,st_tx_dscr_one.uc_short_retry);

        uc_retries = st_tx_dscr_one.uc_long_retry + st_tx_dscr_one.uc_short_retry;
        if (uc_retries > 0)
        {
            uc_retries -= 1;    /* 减掉传输成功的1次 */
        }

        OAM_STAT_USER_INCR(pst_dmac_user->st_user_base_info.us_assoc_id, tx_ppdu_retries, uc_retries);
    }
#ifdef _PRE_WLAN_DFT_STAT
    DMAC_VAP_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_cts_fail, st_tx_dscr_one.uc_cts_failure);
#endif
#ifdef _PRE_WLAN_CHIP_TEST
    DMAC_CHIP_TEST_CALL(dmac_test_sch_stat_tx_mpdu_num(pst_dmac_user, st_tx_dscr_one.uc_tid, st_tx_dscr_one, pst_cb));
#endif
}
/*lint -restore */


OAL_STATIC OAL_INLINE oal_void dmac_tx_comlete_retry_pkt_loss_stat(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, mac_tx_ctl_stru *pst_cb, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_one)
{
    dmac_tx_complete_fail_stat(pst_dmac_vap, pst_dmac_user, pst_cb);

    DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_mpdu_fail_in_ampdu, 1);

    OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_mpdu_fail_in_ampdu, 1);
    OAM_STAT_TID_INCR(pst_dmac_user->st_user_base_info.us_assoc_id,  pst_tx_dscr_one->uc_tid, tx_mpdu_fail_in_ampdu, 1);
#ifdef _PRE_WLAN_PERFORM_STAT
    dmac_stat_tid_per(&(pst_dmac_user->st_user_base_info), pst_tx_dscr_one->uc_tid, 0, 1, DMAC_STAT_PER_SW_RETRY_SUB_AMPDU);
#endif
#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    pst_dmac_vap->st_vap_base_info.st_vap_stats.ul_tx_dropped_packets++;
#endif
}


OAL_STATIC OAL_INLINE oal_void dmac_tx_complete_seq_wrong_stat(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, mac_tx_ctl_stru *pst_cb, oal_netbuf_stru *pst_buf,
                                                                            hal_tx_dscr_stru *pst_dscr, dmac_ba_tx_stru *pst_ba_hdl, oal_uint16 us_seq_num, oal_uint8 uc_tid)
{
    oam_report_dscr(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                    (oal_uint8*)pst_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);

    oam_report_80211_frame(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                           (oal_uint8*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb),
                           MAC_GET_CB_FRAME_HEADER_LENGTH(pst_cb),
                           oal_netbuf_payload(pst_buf),
                           MAC_GET_CB_FRAME_HEADER_LENGTH(pst_cb) + MAC_GET_CB_MPDU_LEN(pst_cb),
                           OAM_OTA_FRAME_DIRECTION_TYPE_TX);

    dmac_tx_complete_fail_stat(pst_dmac_vap, pst_dmac_user, pst_cb);

    DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_tx_mpdu_fail_in_ampdu, 1);

    OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_mpdu_fail_in_ampdu, 1);

    OAM_STAT_TID_INCR(pst_dmac_user->st_user_base_info.us_assoc_id, uc_tid, tx_mpdu_fail_in_ampdu, 1);
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
oal_void  dmac_tx_update_dscr_para_hw(
                hal_tx_dscr_stru               *pst_dscr,
                hal_tx_dscr_ctrl_one_param           *pst_tx_dscr_one,
                hal_to_dmac_device_stru        *pst_hal_device)
{
    /* 是否使能MAC部分帧重传 */
    if (OAL_FALSE == pst_hal_device->en_ampdu_partial_resnd)
    {
        return;
    }

    hal_tx_update_dscr_para_hw(pst_hal_device, pst_dscr, pst_tx_dscr_one);

}



OAL_STATIC oal_uint32  dmac_tx_complete_ampdu_buffer_hw(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr)
{
    oal_uint8                    uc_dscr_index;
    oal_uint8                    uc_dscr_status = DMAC_TX_INVALID;
    oal_uint8                    uc_mem_state = OAL_MEM_STATE_FREE;
    oal_uint8                    uc_retry_num   = 0;
    oal_uint8                    uc_q_num;
    oal_uint8                    uc_dscr_num;
    oal_uint8                    uc_vap_id;
    oal_uint8                    uc_tid;
    oal_uint16                   us_user_idx    = MAC_INVALID_USER_ID;
    oal_uint32                   ul_ret         = OAL_SUCC;
    mac_tx_ctl_stru             *pst_cb;
    dmac_user_stru              *pst_dmac_user;
    hal_tx_dscr_ctrl_one_param   st_tx_dscr_one = {0};
    hal_tx_dscr_stru            *pst_dscr_next;
    dmac_tid_stru               *pst_tid_queue;
    dmac_ba_tx_stru             *pst_ba_hdl;
    oal_dlist_head_stru          st_pending_q;
    dmac_vap_stru               *pst_dmac_vap;
    mac_vap_stru                *pst_mac_vap;
    oal_netbuf_stru             *pst_buf;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8                    uc_pktno;
#else
    oal_uint32                   ul_pktno;
#endif
#ifdef _PRE_WLAN_FEATURE_PF_SCH
    oal_uint32                   ul_fstpkt_timestrap;
#endif
    oal_bool_enum_uint8          en_is_vipframe = OAL_FALSE;
    dmac_user_alg_probe_enum_uint8  en_is_probe_data;
    dmac_tx_chiper_stat_stru     st_chiper_stat;
    oal_uint8                    en_dscr_tx_succ = OAL_FALSE;

    uc_q_num = pst_dscr->uc_q_num;
    pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt =
                     OAL_SUB(pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt, 1);

    pst_buf = pst_dscr->pst_skb_start_addr;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_buf))
    {
        OAL_GET_MEM_FLAG(pst_dscr, &uc_mem_state);
        if (OAL_MEM_STATE_FREE != uc_mem_state)
        {
            dmac_tx_complete_free_dscr(pst_dscr);
        }
        OAM_ERROR_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw::pst_buf null,mem state[%d].}", uc_mem_state);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cb          = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
    uc_dscr_num     = MAC_GET_CB_MPDU_NUM(pst_cb);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    uc_pktno        = MAC_GET_CB_ALG_PKTNO(pst_cb);
#else
    ul_pktno        = MAC_GET_CB_ALG_PKTNO(pst_cb);
#endif
    en_is_probe_data= MAC_GET_CB_IS_PROBE_DATA(pst_cb);
    en_is_vipframe  = MAC_GET_CB_IS_VIPFRAME(pst_cb);
#ifdef _PRE_WLAN_FEATURE_PF_SCH
    ul_fstpkt_timestrap = MAC_GET_CB_TIMESTAMP(pst_cb);
#endif

    dmac_tx_get_vap_id(pst_hal_device, pst_dscr, &uc_vap_id);


    if (OAL_UNLIKELY(0 == uc_vap_id))
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw::vap id is 0, mac vap had been deleted.}");
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_SUCC;
    }

    /* 获取DMAC模块用户结构体 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_cb));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw::pst_dmac_user[%d] null.}",
            MAC_GET_CB_TX_USER_IDX(pst_cb));
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw::pst_dmac_vap null.}");
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);

    /* 从mac地址获取用户索引，用户不存在，直接删除 */
    /* 用户索引idx为全f，表示用户已经删除或者用户idx与cb字段中不一致，表示用户曾经关联又关联上后，原来的数据发送处理都直接丢弃 */
    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_dmac_user->st_user_base_info.auc_user_mac_addr, &us_user_idx);
    if(OAL_UNLIKELY((OAL_SUCC != ul_ret) || (us_user_idx != MAC_GET_CB_TX_USER_IDX(pst_cb))))
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw::drop tx dscr normally.}\r\n");
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_SUCC;
    }

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_AMPDU_PREPARE);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_AMPDU_PREPARE);

    /* 获取发送状态位 */
    hal_tx_get_dscr_status(pst_hal_device, pst_dscr, &uc_dscr_status);
    hal_tx_get_dscr_chiper_type(pst_dscr, &st_chiper_stat.uc_tx_chiper_type, &st_chiper_stat.uc_chiper_key_id);
    dmac_tx_update_alg_param(pst_dscr, pst_cb, &st_tx_dscr_one, pst_hal_device);
#ifdef _PRE_WLAN_DFT_STAT
    dmac_performance_show_mpdu_info(uc_vap_id, uc_dscr_status, &st_tx_dscr_one);
#endif
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_UPDATE_MIB);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_UPDATE_MIB);

    /* 将ps_poll标志清除，表明此次ps-poll已经处理完成，如果再接收到ps-poll可以处理 */
    pst_dmac_user->st_ps_structure.en_is_pspoll_rsp_processing = OAL_FALSE;

    /* 获取用户的特定TID队列 */
    uc_tid    = MAC_GET_CB_WME_TID_TYPE(pst_cb);
    pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[uc_tid]);
    /* 获取发送BA会话参数 */
    pst_ba_hdl = pst_tid_queue->pst_ba_tx_hdl;

    /* ba handle指针判断 */
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_ba_hdl) )
    {
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(DMAC_BA_COMPLETE != pst_ba_hdl->en_ba_conn_status))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw::ba status[%d] error.}", pst_ba_hdl->en_ba_conn_status);
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_UAPSD
    /* 更新keepalive计数器 */
    dmac_keepalive_timestamp_update(pst_mac_vap, pst_dmac_user, (oal_uint8 *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb), uc_dscr_status);
#endif

    /* STA侧pspoll低功耗处理 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        dmac_psm_tx_complete_sta(pst_dmac_vap, pst_dscr, pst_buf);
    }
#endif

    oal_dlist_init_head(&st_pending_q);
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_EXTRACT_BA);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_EXTRACT_BA);

#if 0
    /* 定位序号乱序维测 */
    dmac_tx_complete_hw_seq_num_err(pst_hal_device, pst_dscr, uc_dscr_num, uc_q_num);
#endif

    pst_dscr_next = pst_dscr;

    for (uc_dscr_index = 0; uc_dscr_index < uc_dscr_num; uc_dscr_index++)
    {
        if (OAL_UNLIKELY(oal_dlist_is_empty(&pst_hal_device->ast_tx_dscr_queue[uc_q_num].st_header)))
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw::q empty.}");
            break;
        }

        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dscr_next))
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw:: pst_dscr_next NULL .}");
            break;
        }

        pst_dscr = pst_dscr_next;
        OAL_MEM_TRACE(pst_dscr, OAL_FALSE);
        pst_dscr_next = OAL_DLIST_GET_ENTRY(pst_dscr->st_entry.pst_next, hal_tx_dscr_stru, st_entry);
        oal_dlist_delete_entry(&pst_dscr->st_entry);

        /* 获取buf */
        pst_buf = pst_dscr->pst_skb_start_addr;
        pst_cb = (mac_tx_ctl_stru*)oal_netbuf_cb(pst_buf);
        uc_tid = MAC_GET_CB_WME_TID_TYPE(pst_cb);

        /* 更新速率统计参数 */
        dmac_tx_update_dscr_para_hw(pst_dscr, &st_tx_dscr_one, pst_hal_device);

        hal_tx_get_dscr_status(pst_hal_device, pst_dscr, &uc_dscr_status);
        if (uc_dscr_status != DMAC_TX_SUCC)
        {
            dmac_tx_complete_hw_fail_stat(pst_dmac_user, pst_cb, &st_tx_dscr_one);
            /* 满足数据帧重传条件 */
            if (OAL_TRUE == dmac_tx_comp_data_retry_check(pst_dmac_vap, pst_cb))
            {
                /* 该包没超重传上限 继续重传 */
                dmac_tx_set_retry_dscr(pst_hal_device, pst_dscr, uc_dscr_status, &st_chiper_stat);
                oal_dlist_add_tail(&pst_dscr->st_entry, &st_pending_q);
                uc_retry_num++;
                MAC_GET_CB_RETRIED_NUM(pst_cb)++;

                /* 算法更新速率 */
                if (uc_dscr_index == OAL_SUB(uc_dscr_num, 1) && (OAL_FALSE == en_is_vipframe))
                {
                    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ACK);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
                    MAC_GET_CB_ALG_PKTNO(pst_cb) = uc_pktno;
#else
                    MAC_GET_CB_ALG_PKTNO(pst_cb) = ul_pktno;
#endif
                    MAC_GET_CB_IS_PROBE_DATA(pst_cb) = en_is_probe_data;
#ifdef _PRE_WLAN_FEATURE_PF_SCH
                    MAC_GET_CB_TIMESTAMP(pst_cb) = ul_fstpkt_timestrap;
#endif
                    /* 通知算法 */
                    dmac_tx_complete_notify(pst_hal_device, &(pst_dmac_user->st_user_base_info), pst_buf, &st_tx_dscr_one);
                    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ALG_NOTIFY);
                }

                continue;
            }
            else
            {
                /* 统计信息更新 */
            #ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
                dmac_tx_complete_normal_pkt_num(pst_buf, &st_tx_dscr_one, us_user_idx);
            #endif
                dmac_tx_comlete_retry_pkt_loss_stat(pst_dmac_vap, pst_dmac_user, pst_cb, &st_tx_dscr_one);
            }

        }
        else
        {
            /* 发送成功统计信息更新 */
        #ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
            dmac_tx_complete_normal_pkt_num(pst_buf, &st_tx_dscr_one, us_user_idx);
        #endif
            dmac_tx_complete_succ_stat(pst_dmac_vap, pst_dmac_user, pst_cb);
            /* 至少有一帧是发送成功的 */
            en_dscr_tx_succ = OAL_TRUE;
        }

        //oal_bit_set_bit_eight_byte(&st_tx_dscr_one.ull_ampdu_result, uc_dscr_index);

        /* 算法更新速率 */
        if ((uc_dscr_index == OAL_SUB(uc_dscr_num, 1)) && (OAL_FALSE == en_is_vipframe) && (OAL_FALSE == OAL_GET_THRUPUT_BYPASS_ENABLE(OAL_TX_HAL_HARDWARE_BYPASS)))
        {
            OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ACK);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            MAC_GET_CB_ALG_PKTNO(pst_cb) = uc_pktno;
#else
            MAC_GET_CB_ALG_PKTNO(pst_cb) = ul_pktno;
#endif
            MAC_GET_CB_IS_PROBE_DATA(pst_cb) = en_is_probe_data;
#ifdef _PRE_WLAN_FEATURE_PF_SCH
            MAC_GET_CB_TIMESTAMP(pst_cb) = ul_fstpkt_timestrap;
#endif
            /* 通知算法 */
            dmac_tx_complete_notify(pst_hal_device, &(pst_dmac_user->st_user_base_info), pst_buf, &st_tx_dscr_one);
            OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ALG_NOTIFY);
        }

        dmac_tx_complete_free_dscr(pst_dscr);
    }/*所有mpdu都确认完成*/

#ifdef _PRE_WLAN_PERFORM_STAT
    dmac_tx_complete_perform_stat(pst_dmac_user, &st_tx_dscr_one);
#endif
    dmac_tx_complete_retry_stat(pst_dmac_vap, pst_dmac_user, pst_cb, st_tx_dscr_one, uc_dscr_num, uc_retry_num);

    /* 至少有一帧是发送成功的 */
    if (OAL_LIKELY(en_dscr_tx_succ == DMAC_TX_SUCC))/* 发送成功 收到BA */
    {
#ifdef _PRE_DEBUG_MODE_USER_TRACK
        dmac_tx_complete_check_protocol(pst_hal_device, pst_dmac_user, pst_dscr);
#endif
        dmac_tx_complete_dft_stat_succ_incr(pst_dmac_vap, pst_dmac_user, pst_dscr, uc_dscr_num, uc_tid);

        OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_TX_COMP_CHECK);
        OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_TX_COMP_CHECK);

        /* 发送成功 清空linkloss计数 */
        dmac_vap_linkloss_clean(pst_dmac_vap);
        dmac_vap_linkloss_threshold_incr(pst_dmac_vap);
    }
    else  /*没有收到ba*/
    {
        dmac_vap_linkloss_threshold_decr(pst_dmac_vap);
        dmac_successive_fail_dump_info(pst_dmac_vap, pst_dmac_user, pst_dscr);

#ifdef _PRE_DEBUG_MODE
        dmac_tx_complete_oam_report(pst_dmac_user, pst_dscr, uc_dscr_num, uc_dscr_status);
#endif
        /* 发送AMPDU失败的相关统计信息 */
        dmac_tx_complete_dft_stat_fail_incr(pst_dmac_vap, pst_dmac_user, uc_tid);
    }

    dmac_tx_complete_dscr_back(pst_dmac_vap, pst_dmac_user, uc_tid, &st_pending_q, uc_retry_num);

    /* 算法降协议删除ba会话时将此标志置为true */
    if (OAL_TRUE == pst_dmac_user->en_delete_ba_flag)
    {
        pst_dmac_user->en_delete_ba_flag = OAL_FALSE;

        /* 抛事件到HMAC执行删除动作 */
        dmac_tx_delete_ba(pst_dmac_user);
    }


    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_COMP_ENQUEUE_RETRY);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ENQUEUE_RETRY);

    return OAL_SUCC;

}
#endif



#ifdef _PRE_WLAN_FEATURE_AMPDU

OAL_STATIC oal_uint32  dmac_tx_complete_ampdu_buffer(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr)
{
    oal_uint8                    uc_dscr_index;
    oal_uint8                    uc_dscr_status = DMAC_TX_INVALID;
#ifdef _PRE_WLAN_INIT_PTK_TX_PN
#if 0
    oal_uint32                   ul_iv_ms_word = 0xFF;
    oal_uint32                   ul_iv_ls_word = 0xFF;
    oal_uint32                   aul_iv_ms_word[2] = {0};
    oal_uint32                   aul_iv_ls_word[2] = {0};
    oal_uint32                   ul_phy_mode_one[2] = {0};
    oal_uint8                    uc_ra_lut_index[3] = {0,0,0};
    oal_uint16                   us_dscr_seq_num[2] = {0};
    hal_tx_dscr_ctrl_one_param   ast_tx_dscr_one[2] = {{0},{0}};
#endif
#endif
    oal_uint8                    uc_retry_num   = 0;
    oal_uint8                    uc_q_num;
    oal_uint8                    uc_dscr_num;
    oal_uint8                    uc_vap_id;
    oal_uint8                    uc_tid;
    oal_bool_enum_uint8          en_need_bar    = OAL_FALSE;
    oal_uint16                   us_ssn         = 0;
    oal_uint16                   us_seq_num     = 0;
    oal_uint16                   us_user_idx    = MAC_INVALID_USER_ID;
    oal_uint32                   ul_ret         = OAL_SUCC;
    mac_tx_ctl_stru             *pst_cb;
    dmac_user_stru              *pst_dmac_user;
    hal_tx_dscr_ctrl_one_param   st_tx_dscr_one = {0};
    hal_tx_dscr_stru            *pst_dscr_next;
    oal_uint32                   aul_ba_bitmap[DMAC_BA_BMP_SIZE >> 5] = {0};
    dmac_tid_stru               *pst_tid_queue;
    dmac_ba_tx_stru             *pst_ba_hdl;
    oal_dlist_head_stru          st_pending_q;
    dmac_vap_stru               *pst_dmac_vap;
    mac_vap_stru                *pst_mac_vap;
    oal_netbuf_stru             *pst_buf;
    oal_uint8                    uc_mem_state = 0;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8                    uc_pktno;
#else
    oal_uint32                   ul_pktno;
#endif
#ifdef _PRE_WLAN_FEATURE_PF_SCH
    oal_uint32                   ul_fstpkt_timestrap;
#endif
    oal_bool_enum_uint8          en_is_vipframe = OAL_FALSE;
    dmac_user_alg_probe_enum_uint8  en_is_probe_data;
    dmac_tx_chiper_stat_stru     st_chiper_stat;
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8    en_trace_pkt_type;
#endif

#ifdef _PRE_WLAN_11K_STAT
    oal_time_us_stru                        st_time;
#endif

    uc_q_num   = pst_dscr->uc_q_num;
    pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt =
                    OAL_SUB(pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt, 1);

    pst_buf   = pst_dscr->pst_skb_start_addr;
    if (OAL_PTR_NULL == pst_buf)
    {
        OAL_GET_MEM_FLAG(pst_dscr, &uc_mem_state);
        if (OAL_MEM_STATE_FREE != uc_mem_state)
        {
            dmac_tx_complete_free_dscr(pst_dscr);
        }
        OAM_ERROR_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw::pst_buf null,mem state[%d].}", uc_mem_state);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cb              = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);

    uc_dscr_num         = MAC_GET_CB_MPDU_NUM(pst_cb);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    uc_pktno            = MAC_GET_CB_ALG_PKTNO(pst_cb);
#else
    ul_pktno            = MAC_GET_CB_ALG_PKTNO(pst_cb);
#endif
    en_is_vipframe      = MAC_GET_CB_IS_VIPFRAME(pst_cb);
    en_is_probe_data    = MAC_GET_CB_IS_PROBE_DATA(pst_cb);
    uc_tid              = MAC_GET_CB_WME_TID_TYPE(pst_cb);
#ifdef _PRE_WLAN_FEATURE_PF_SCH
    ul_fstpkt_timestrap = MAC_GET_CB_TIMESTAMP(pst_cb);
#endif

    dmac_tx_get_vap_id(pst_hal_device, pst_dscr, &uc_vap_id);

#ifdef _PRE_DEBUG_MODE
    dmac_tx_complete_vip_frame_check(pst_cb, pst_buf, en_is_vipframe, uc_tid, uc_vap_id);
#endif


    if (OAL_UNLIKELY(0 == uc_vap_id))
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::vap id is 0,  mac vap had been deleted.}");
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_SUCC;
    }

    /* 获取DMAC模块用户结构体 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_cb));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::pst_dmac_user[%d] null.}",
            MAC_GET_CB_TX_USER_IDX(pst_cb));
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::pst_dmac_vap null.}");
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);

    /* 从mac地址获取用户索引，用户不存在，直接删除 */
    /* 用户索引idx为全f，表示用户已经删除或者用户idx与cb字段中不一致，表示用户曾经关联又关联上后，原来的数据发送处理都直接丢弃 */
    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_dmac_user->st_user_base_info.auc_user_mac_addr, &us_user_idx);
    if(OAL_UNLIKELY((OAL_SUCC != ul_ret) || (us_user_idx != MAC_GET_CB_TX_USER_IDX(pst_cb))))
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::drop tx dscr normally.}\r\n");
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_SUCC;
    }

    /* 获取用户的特定TID队列 */
    pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[uc_tid]);
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_AMPDU_PREPARE);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_AMPDU_PREPARE);

    /* 更新mib库信息 */
    dmac_tx_update_alg_param(pst_dscr, pst_cb, &st_tx_dscr_one, pst_hal_device);
#ifdef _PRE_WLAN_DFT_STAT
    dmac_performance_show_mpdu_info(uc_vap_id, uc_dscr_status, &st_tx_dscr_one);
#endif
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_UPDATE_MIB);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_UPDATE_MIB);

    /* 将ps_poll标志清除，表明此次ps-poll已经处理完成，如果再接收到ps-poll可以处理 */
    pst_dmac_user->st_ps_structure.en_is_pspoll_rsp_processing = OAL_FALSE;

    /* 获取发送BA会话参数 */
    pst_ba_hdl = pst_tid_queue->pst_ba_tx_hdl;

    /* ba handle指针判断 */
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_ba_hdl) )
    {
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(DMAC_BA_COMPLETE != pst_ba_hdl->en_ba_conn_status))
    {
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_dscr, uc_dscr_num, OAL_TRUE);
        return OAL_FAIL;
    }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    /* 针对rifs需要先释放RIFS序列最后加上的BAR */
    if (OAL_UNLIKELY(OAL_PTR_NULL != MAC_GET_CB_BAR_DSCR_ADDR(pst_cb)))
    {
        dmac_tx_complete_free_dscr_list(pst_hal_device, MAC_GET_CB_BAR_DSCR_ADDR(pst_cb), 1, OAL_FALSE);
        uc_dscr_num--;
        pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt = OAL_SUB(pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt, uc_dscr_num);
    }
#endif
    /* 获取发送状态位 */
    hal_tx_get_dscr_status(pst_hal_device, pst_dscr, &uc_dscr_status);
    hal_tx_get_dscr_chiper_type(pst_dscr, &st_chiper_stat.uc_tx_chiper_type, &st_chiper_stat.uc_chiper_key_id);

#ifdef _PRE_WLAN_FEATURE_UAPSD
    /* 更新keepalive计数器 */
    dmac_keepalive_timestamp_update(pst_mac_vap, pst_dmac_user, (oal_uint8 *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb), uc_dscr_status);
#endif

    /* STA侧pspoll低功耗处理 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        dmac_psm_tx_complete_sta(pst_dmac_vap, pst_dscr, pst_buf);
    }
#endif

    if (OAL_LIKELY(uc_dscr_status == DMAC_TX_SUCC))/* 发送成功 收到BA */
    {
#ifdef _PRE_DEBUG_MODE_USER_TRACK
        dmac_tx_complete_check_protocol(pst_hal_device, pst_dmac_user, pst_dscr);
#endif
        dmac_tx_complete_dft_stat_succ_incr(pst_dmac_vap, pst_dmac_user, pst_dscr, uc_dscr_num, uc_tid);

        OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_TX_COMP_CHECK);
        OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_TX_COMP_CHECK);

        /* 获取BA信息 ssn 和 bitmap */
        hal_tx_get_dscr_ba_ssn(pst_hal_device, pst_dscr, &us_ssn);
        hal_tx_get_dscr_ba_bitmap(pst_hal_device, pst_dscr, aul_ba_bitmap);

        /* 发送成功 清空linkloss计数 */
        dmac_vap_linkloss_clean(pst_dmac_vap);
        dmac_vap_linkloss_threshold_incr(pst_dmac_vap);
    }
    else  /*没有收到ba*/
    {
        dmac_vap_linkloss_threshold_decr(pst_dmac_vap);
        dmac_successive_fail_dump_info(pst_dmac_vap, pst_dmac_user, pst_dscr);

#ifdef _PRE_DEBUG_MODE
        dmac_tx_complete_oam_report(pst_dmac_user, pst_dscr, uc_dscr_num, uc_dscr_status);
#endif
        /* 发送AMPDU失败的相关统计信息 */
        dmac_tx_complete_dft_stat_fail_incr(pst_dmac_vap, pst_dmac_user, uc_tid);
    }

    oal_dlist_init_head(&st_pending_q);
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_EXTRACT_BA);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_EXTRACT_BA);

    pst_dscr_next = pst_dscr;

#ifdef _PRE_WLAN_11K_STAT
    /* 获取系统时间 */
    oal_time_get_stamp_us(&st_time);
#endif

    for (uc_dscr_index = 0; uc_dscr_index < uc_dscr_num; uc_dscr_index++)
    {
        if (OAL_UNLIKELY(oal_dlist_is_empty(&pst_hal_device->ast_tx_dscr_queue[uc_q_num].st_header)))
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::q empty.}");
            break;
        }

        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dscr_next))
        {
            OAM_ERROR_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer:: pst_dscr_next NULL .}");
            break;
        }

        pst_dscr = pst_dscr_next;
        OAL_MEM_TRACE(pst_dscr, OAL_FALSE);
        pst_dscr_next = OAL_DLIST_GET_ENTRY(pst_dscr->st_entry.pst_next, hal_tx_dscr_stru, st_entry);
        /* 获取buf */
        pst_buf = pst_dscr->pst_skb_start_addr;
        pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
        uc_tid = MAC_GET_CB_WME_TID_TYPE(pst_cb);
        us_seq_num = MAC_GET_CB_SEQ_NUM(pst_cb);

        oal_dlist_delete_entry(&pst_dscr->st_entry);

#ifdef _PRE_WLAN_INIT_PTK_TX_PN
#if 0
        /* 记录首帧的特征参数 */
        if (pst_dscr->bit_is_first)
        {
            hal_tx_get_dscr_seq_num(pst_hal_device, pst_dscr, &us_dscr_seq_num[0]);
            hal_tx_get_dscr_ctrl_one_param(pst_hal_device, pst_dscr, &ast_tx_dscr_one[0]);
            hal_tx_get_dscr_iv_word(pst_dscr, &aul_iv_ms_word[0], &aul_iv_ls_word[0], st_chiper_stat.uc_tx_chiper_type, st_chiper_stat.uc_chiper_key_id);
            hal_tx_get_dscr_phy_mode_one(pst_hal_device, pst_dscr,  &ul_phy_mode_one[0]);
            hal_tx_get_ra_lut_index(pst_hal_device, pst_dscr, &uc_ra_lut_index[0]);
        }

        /*记录其中的最大的pn号
          如果本次报上来的最大pn号大于保存的pn号1000以上，则输出该MPDU附近3个描述符和首帧的发送状态
          否则将该次发送的最大pn号更新至保存的最近的PN号
          需要在对端ba确认的帧中查找最大pn号 */
        hal_tx_get_dscr_iv_word(pst_dscr, &ul_iv_ms_word, &ul_iv_ls_word, st_chiper_stat.uc_tx_chiper_type, st_chiper_stat.uc_chiper_key_id);
        hal_tx_get_ra_lut_index(pst_hal_device, pst_dscr, &uc_ra_lut_index[2]);

        if ((DMAC_BA_ISSET(aul_ba_bitmap, DMAC_BA_INDEX(us_ssn, us_seq_num)))&&
            (OAL_FALSE == dmac_check_iv_word(ul_iv_ms_word, ul_iv_ls_word,pst_hal_device->uc_chip_id,uc_ra_lut_index[2])))
        {
            OAM_WARNING_LOG4(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::iv word error,first mpdu tx status is [%u] key type[%u] key id[%u] lutidx=[%u].}",
                                           uc_dscr_status,st_chiper_stat.uc_tx_chiper_type, st_chiper_stat.uc_chiper_key_id,uc_ra_lut_index[2]);
            OAM_WARNING_LOG4(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::iv word error,current mpdu iv ms word is[0x%x] ls word is[0x%x],last mpdu iv ms word is[0x%x] ls word is[0x%x].}",
                                           ul_iv_ms_word, ul_iv_ls_word, g_ul_prev_iv_word[pst_hal_device->uc_chip_id][uc_ra_lut_index[2]].ul_pn_msb, g_ul_prev_iv_word[pst_hal_device->uc_chip_id][uc_ra_lut_index[2]].ul_pn_lsb);
            oam_report_dscr(pst_dmac_user->st_user_base_info.auc_user_mac_addr,(oal_uint8 *)pst_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TYPE_TX_DSCR);
            if (OAL_PTR_NULL != pst_dscr_next)
            {
                oam_report_dscr(pst_dmac_user->st_user_base_info.auc_user_mac_addr,(oal_uint8 *)pst_dscr_next, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TYPE_TX_DSCR);
            }
            /* 输出首帧和前一帧的特征参数 */
            OAM_WARNING_LOG4(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::1st mpdu seq num[%d] iv ms word is[0x%x] iv ls word is[0x%x] ra lut index[%d].}",
                                          us_dscr_seq_num[0], aul_iv_ms_word[0], aul_iv_ls_word[0], uc_ra_lut_index[0]);
            OAM_WARNING_LOG4(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::1st mpdu long retry[%d] short retry[%d] rts succ[%d] cts failure[%d].}",
                                          ast_tx_dscr_one[0].uc_long_retry, ast_tx_dscr_one[0].uc_short_retry, ast_tx_dscr_one[0].uc_rts_succ, ast_tx_dscr_one[0].uc_cts_failure);
            OAM_WARNING_LOG4(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::prev mpdu seq num[%d] iv ms word is[0x%x] iv ls word is[0x%x] ra lut index[%d].}",
                                          us_dscr_seq_num[1], aul_iv_ms_word[1], aul_iv_ls_word[1], uc_ra_lut_index[1]);
            OAM_WARNING_LOG4(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::prev mpdu long retry[%d] short retry[%d] rts succ[%d] cts failure[%d].}",
                                          ast_tx_dscr_one[1].uc_long_retry, ast_tx_dscr_one[1].uc_short_retry, ast_tx_dscr_one[1].uc_rts_succ, ast_tx_dscr_one[1].uc_cts_failure);
            OAM_WARNING_LOG2(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::1st mpdu phy mode one[0x%x] prev mpdu ul_phy_mode_one[0x%x].}",
                                          ul_phy_mode_one[0], ul_phy_mode_one[1]);
        }

        dmac_update_iv_word(ul_iv_ms_word,ul_iv_ls_word,pst_hal_device->uc_chip_id,uc_ra_lut_index[2]);


        /* 记录PN跳变帧的前一帧的特征参数 */
        hal_tx_get_dscr_seq_num(pst_hal_device, pst_dscr, &us_dscr_seq_num[1]);
        hal_tx_get_dscr_ctrl_one_param(pst_hal_device, pst_dscr, &ast_tx_dscr_one[1]);
        hal_tx_get_dscr_iv_word(pst_dscr, &aul_iv_ms_word[1], &aul_iv_ls_word[1], st_chiper_stat.uc_tx_chiper_type, st_chiper_stat.uc_chiper_key_id);
        hal_tx_get_dscr_phy_mode_one(pst_hal_device, pst_dscr,  &ul_phy_mode_one[1]);
        hal_tx_get_ra_lut_index(pst_hal_device, pst_dscr, &uc_ra_lut_index[1]);
#endif
#endif

        if (OAL_UNLIKELY(OAL_TRUE == dmac_ba_seqnum_out_window(pst_ba_hdl, us_seq_num)))
        {
#ifdef _PRE_DEBUG_MODE
            //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
            en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_buf), MAC_NETBUFF_PAYLOAD_SNAP);
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::type%d out of window[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
#endif

            if(0 == uc_dscr_index)
            {
                /* 在BA卡死复位后，会强行移窗，改为warning,且只打印聚合帧首帧 */
                OAM_WARNING_LOG4(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::seqnum(%d) out of ba window(%d~%d), pst_ba_hdl->uc_tx_ba_lut=%d.}",
                    us_seq_num, pst_ba_hdl->us_baw_start, pst_ba_hdl->us_last_seq_num, pst_ba_hdl->uc_tx_ba_lut);
                OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::mpdu num(%d).}",
                    uc_dscr_num);
            }

#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
            dmac_tx_complete_normal_pkt_num(pst_buf, &st_tx_dscr_one, us_user_idx);
#endif
            dmac_tx_complete_seq_wrong_stat(pst_dmac_vap, pst_dmac_user, pst_cb, pst_buf, pst_dscr, pst_ba_hdl, us_seq_num, uc_tid);
        }
        /* 如果seqnum未被ba中bitmap确认或者未收到ba */
        else if ((!DMAC_BA_ISSET(aul_ba_bitmap, DMAC_BA_INDEX(us_ssn, us_seq_num))) || (uc_dscr_status != DMAC_TX_SUCC))
        {
#ifdef _PRE_DEBUG_MODE
            //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
            en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_buf), MAC_NETBUFF_PAYLOAD_SNAP);
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::type%d ampdu send fail, retry[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
#endif
            dmac_tx_complete_hw_fail_stat(pst_dmac_user, pst_cb, &st_tx_dscr_one);
            /* 满足数据帧重传条件 */
            if (OAL_TRUE == dmac_tx_comp_data_retry_check(pst_dmac_vap, pst_cb))
            {
                /* 该包没超重传上限 继续重传 */
                dmac_tx_set_retry_dscr(pst_hal_device, pst_dscr, uc_dscr_status, &st_chiper_stat);
                oal_dlist_add_tail(&pst_dscr->st_entry, &st_pending_q);
                MAC_GET_CB_RETRIED_NUM(pst_cb)++;
                uc_retry_num++;

                /* 算法更新速率 */
                if ((uc_dscr_index == OAL_SUB(uc_dscr_num, 1)) && (OAL_FALSE == en_is_vipframe))
                {
                    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ACK);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
                    MAC_GET_CB_ALG_PKTNO(pst_cb) = uc_pktno;
#else
                    MAC_GET_CB_ALG_PKTNO(pst_cb) = ul_pktno;
#endif
                    MAC_GET_CB_IS_PROBE_DATA(pst_cb) = en_is_probe_data;
#ifdef _PRE_WLAN_FEATURE_PF_SCH
                    MAC_GET_CB_TIMESTAMP(pst_cb) = ul_fstpkt_timestrap;
#endif
                    /* 通知算法 */
                    dmac_tx_complete_notify(pst_hal_device, &(pst_dmac_user->st_user_base_info), pst_buf, &st_tx_dscr_one);
                    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ALG_NOTIFY);
                }

                continue;
            }
            else
            {
#ifdef _PRE_DEBUG_MODE
                //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
                en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_buf), MAC_NETBUFF_PAYLOAD_SNAP);
                if( PKT_TRACE_BUTT != en_trace_pkt_type)
                {
                    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::type%d ampdu send fail, drop[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
                }
#endif
                /* 重传包丢包 发bar */
                en_need_bar = OAL_TRUE;
                /* 统计信息更新 */
#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
                dmac_tx_complete_normal_pkt_num(pst_buf, &st_tx_dscr_one, us_user_idx);
#endif
                dmac_tx_comlete_retry_pkt_loss_stat(pst_dmac_vap, pst_dmac_user, pst_cb, &st_tx_dscr_one);
#ifdef _PRE_WLAN_FEATURE_VOWIFI
                dmac_update_vowifi_tx_cnt(pst_dmac_vap, pst_dmac_user);
#endif

#ifdef _PRE_WLAN_11K_STAT
                dmac_user_stat_tx_mpdu_info(pst_dmac_user, pst_dmac_vap, pst_cb, uc_dscr_status, OAL_TRUE);
#endif

            }
#ifdef _PRE_WLAN_11K_STAT
            dmac_user_stat_tx_mpdu_info(pst_dmac_user, pst_dmac_vap, pst_cb, uc_dscr_status, OAL_FALSE);
#endif

        }
        else
        {
            /* 发送成功统计信息更新 */
#ifdef _PRE_DEBUG_MODE
            //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
            en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_buf), MAC_NETBUFF_PAYLOAD_SNAP);
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer::trace pkt type:%d! ampdu send succuss[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
#endif
#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
            dmac_tx_complete_normal_pkt_num(pst_buf, &st_tx_dscr_one,us_user_idx);
#endif
            dmac_tx_complete_succ_stat(pst_dmac_vap, pst_dmac_user, pst_cb);
#ifdef _PRE_WLAN_11K_STAT
            dmac_user_stat_tx_mpdu_info(pst_dmac_user, pst_dmac_vap, pst_cb, uc_dscr_status, OAL_FALSE);
            dmac_user_stat_tx_delay(pst_dmac_user, pst_dmac_vap, st_tx_dscr_one.uc_tid, pst_cb, &st_time);
#endif
        }
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
        dmac_user_stat_tx_mcs_count(pst_dmac_user, pst_dmac_vap, pst_cb, &st_tx_dscr_one);
#endif

        //oal_bit_set_bit_eight_byte(&st_tx_dscr_one.ull_ampdu_result, uc_dscr_index);
        dmac_ba_update_baw(pst_ba_hdl, us_seq_num);

        /* 算法更新速率 */
        if ((uc_dscr_index == OAL_SUB(uc_dscr_num, 1)) && (OAL_FALSE == en_is_vipframe) && (OAL_FALSE == OAL_GET_THRUPUT_BYPASS_ENABLE(OAL_TX_HAL_HARDWARE_BYPASS)))
        {
            OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ACK);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            MAC_GET_CB_ALG_PKTNO(pst_cb) = uc_pktno;
#else
            MAC_GET_CB_ALG_PKTNO(pst_cb) = ul_pktno;
#endif
            MAC_GET_CB_IS_PROBE_DATA(pst_cb) = en_is_probe_data;
#ifdef _PRE_WLAN_FEATURE_PF_SCH
            MAC_GET_CB_TIMESTAMP(pst_cb) = ul_fstpkt_timestrap;
#endif
            /* 通知算法 */
            dmac_tx_complete_notify(pst_hal_device, &(pst_dmac_user->st_user_base_info), pst_buf, &st_tx_dscr_one);
            OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ALG_NOTIFY);
        }

        dmac_tx_complete_free_dscr(pst_dscr);
    }/*所有mpdu都确认完成*/

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_COMP_ACK);

#ifdef _PRE_WLAN_PERFORM_STAT
    dmac_tx_complete_perform_stat(pst_dmac_user, &st_tx_dscr_one);
#endif

#ifdef _PRE_WLAN_11K_STAT
    dmac_user_stat_tx_frm_info(pst_dmac_user, pst_dmac_vap, uc_tid, &st_tx_dscr_one, OAL_TRUE);
#endif

    dmac_tx_complete_retry_stat(pst_dmac_vap, pst_dmac_user, pst_cb, st_tx_dscr_one, uc_dscr_num, uc_retry_num);

    dmac_tx_complete_dscr_back(pst_dmac_vap, pst_dmac_user, uc_tid, &st_pending_q, uc_retry_num);

    if (OAL_TRUE == en_need_bar)
    {
        dmac_ba_send_bar(pst_ba_hdl, pst_dmac_user, pst_tid_queue);
    }

    /* 算法降协议删除ba会话时将此标志置为true */
    if (OAL_TRUE == pst_dmac_user->en_delete_ba_flag)
    {
        pst_dmac_user->en_delete_ba_flag = OAL_FALSE;
        /* 抛事件到HMAC执行删除动作 */
        dmac_tx_delete_ba(pst_dmac_user);
    }

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_COMP_ENQUEUE_RETRY);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_COMP_ENQUEUE_RETRY);

    return OAL_SUCC;
}
#endif


OAL_STATIC OAL_INLINE oal_uint32  dmac_tx_update_alg_param(
                hal_tx_dscr_stru               *pst_dscr,
                mac_tx_ctl_stru                *pst_cb,
                hal_tx_dscr_ctrl_one_param     *pst_tx_dscr_one,
                hal_to_dmac_device_stru        *pst_hal_device)
{

    hal_tx_get_dscr_ctrl_one_param(pst_hal_device, pst_dscr, pst_tx_dscr_one);

    pst_tx_dscr_one->us_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_cb);
    pst_tx_dscr_one->uc_tid      = MAC_GET_CB_WME_TID_TYPE(pst_cb);

    return OAL_SUCC;
}


oal_uint32  dmac_tx_complete_schedule(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_ac_num)
{
    dmac_user_stru                  *pst_dmac_user = OAL_PTR_NULL;
    dmac_vap_stru                   *pst_dmac_vap = OAL_PTR_NULL;
    mac_tid_schedule_output_stru     st_schedule_ouput = {0};
    dmac_tid_stru                   *pst_dmac_tid = OAL_PTR_NULL;
    oal_uint32                       ul_ret = 0;
#ifdef _PRE_WLAN_FEATURE_DFR
    oal_uint16                       us_seq_num = 0;
#endif
    oal_uint8                        uc_tx_idx;
    oal_uint8                        uc_dscr_num;
    oal_uint8                        uc_dscr_idx;

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    mac_device_stru                 *pst_dev;
#endif

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_SCHEDULE_START);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_complete_schedule::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_SCHEDULE_START);
    ul_ret = dmac_alg_tx_schedule_notify(pst_hal_device, uc_ac_num, &st_schedule_ouput);

    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_schedule::dmac_alg_tx_schedule_notify failed[%d].", ul_ret);
        return ul_ret;
    }

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_SCHEDULE);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_SCHEDULE);


    if (0 == st_schedule_ouput.uc_mpdu_num[0])
    {
    #ifdef _PRE_WLAN_FEATURE_DFR
        /* 若BA窗未卡死，tid队列无数据，return */
        if (OAL_FALSE == st_schedule_ouput.en_ba_is_jamed)
    #endif
        {
            return OAL_SUCC;
        }
    }

    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(st_schedule_ouput.us_user_idx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_schedule::pst_dmac_user[%d] null.}", st_schedule_ouput.us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_tid  = &(pst_dmac_user->ast_tx_tid_queue[st_schedule_ouput.uc_tid_num]);
    /* OAM_INFO_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_schedule::uc_tid_num=%d.}", st_schedule_ouput.uc_tid_num); */

#ifdef _PRE_WLAN_PERFORM_STAT
    /* tid delay性能统计日志 */
    dmac_stat_tid_delay(pst_dmac_tid);
#endif

    /* 调度所给的tid可能从属不同的vap因此要在获取一次 */
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_complete_schedule::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if ((0 == st_schedule_ouput.uc_mpdu_num[0]) && (OAL_TRUE == st_schedule_ouput.en_ba_is_jamed))
    {
        /* 若BA窗卡死，则强行移窗*/
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_schedule::ba jamed!user_idx[%d],tid[%d],ssn[%d],bawsize[%d].}",
                         st_schedule_ouput.us_user_idx, st_schedule_ouput.uc_tid_num,pst_dmac_tid->pst_ba_tx_hdl->us_baw_start, pst_dmac_tid->pst_ba_tx_hdl->us_baw_size);
        us_seq_num = DMAC_BA_SEQNO_ADD(pst_dmac_tid->pst_ba_tx_hdl->us_baw_start, pst_dmac_tid->pst_ba_tx_hdl->us_baw_size);
        us_seq_num = DMAC_BA_SEQNO_SUB(us_seq_num, 1);
        dmac_move_ba_window_ahead(pst_dmac_tid->pst_ba_tx_hdl, us_seq_num);
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_schedule::flush jamed, ssn =%d, lsn=%d.}",
                            pst_dmac_tid->pst_ba_tx_hdl->us_baw_start, pst_dmac_tid->pst_ba_tx_hdl->us_last_seq_num);
        return OAL_SUCC;
    }
#endif

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_SCHEDULE_GET_TID);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_SCHEDULE_GET_TID);

    if (DMAC_TX_MODE_NORMAL == pst_dmac_tid->en_tx_mode)
    {
        uc_dscr_num = st_schedule_ouput.uc_mpdu_num[0];

#ifdef _PRE_DEBUG_MODE
        if (st_schedule_ouput.uc_mpdu_num[0] > 1)
        {
            OAM_INFO_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_schedule::not ampdu st_schedule_ouput.uc_mpdu_num=%d.}", uc_dscr_num);
        }
#endif
        for (uc_dscr_idx = 0; uc_dscr_idx < uc_dscr_num; uc_dscr_idx++)
        {
            ul_ret = dmac_tid_tx_queue_remove(pst_hal_device, pst_dmac_vap, pst_dmac_tid, 1);
        }
    }
    else
    {
        for (uc_tx_idx = 0; uc_tx_idx < DMAC_TX_QUEUE_AGGR_DEPTH; uc_tx_idx++)
        {
            uc_dscr_num = OAL_MIN(st_schedule_ouput.uc_mpdu_num[uc_tx_idx], pst_dmac_tid->pst_ba_tx_hdl->uc_ampdu_max_num);
            /*如果配置了ampdu常发模式，则选择最大的聚合报文发送*/
            if (HAL_ALWAYS_TX_AMPDU_ENABLE == pst_hal_device->bit_al_tx_flag)
            {
                uc_dscr_num = WLAN_AMPDU_TX_MAX_NUM;
            }

            if (0 == uc_dscr_num)
            {
                continue;
            }

#if defined(_PRE_PRODUCT_ID_HI110X_DEV) && defined(_PRE_WLAN_DFT_STAT)
            if(DFX_GET_PERFORMANCE_LOG_SWITCH_ENABLE(DFX_PERFORMANCE_TX_LOG))
            {
                OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                                    "{dmac_tx_complete_schedule::alg_num[%d],tid_num=[%d],retry_num[%d],tx_idx[%d],tid[%d],ssn[%d],bw_size[%d],lsn[%d].}",
                                    8,
                                    st_schedule_ouput.uc_mpdu_num[uc_tx_idx],
                                    pst_dmac_tid->us_mpdu_num,
                                    pst_dmac_tid->uc_retry_num,
                                    uc_tx_idx,
                                    pst_dmac_tid->uc_tid,
                                    pst_dmac_tid->pst_ba_tx_hdl->us_baw_start,
                                    pst_dmac_tid->pst_ba_tx_hdl->us_baw_size,
                                    pst_dmac_tid->pst_ba_tx_hdl->us_last_seq_num);
            }
#endif

#ifdef _PRE_PROFILING_MODE
            /*profiling测试ampdu时候，需要聚合最大长度*/
            uc_dscr_num = WLAN_AMPDU_TX_MAX_NUM;
#endif

            /* 指定聚合最大个数开关判断，暂时不使用debug宏，因为RL也会用到 */
            if (0 != g_uc_aggr_num_switch)
            {
                uc_dscr_num = g_uc_max_aggr_num;
            }

            ul_ret = dmac_tid_tx_queue_remove_ampdu(pst_hal_device, pst_dmac_vap, pst_dmac_user, pst_dmac_tid, uc_dscr_num);
        }
    }

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    pst_dev = (mac_device_stru *)mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dev))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_tx_complete_schedule::pst_dev[%d] null!}", pst_hal_device->uc_mac_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    dmac_alg_flowctl_backp_notify(&(pst_dmac_vap->st_vap_base_info), pst_dev->us_total_mpdu_num, pst_dev->aus_ac_mpdu_num);
#endif
    dmac_alg_tid_update_notify(pst_dmac_tid);

    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_complete_schedule::dmac_tid_tx_queue_remove or dmac_tid_tx_queue_remove_ampdu failed.}");
        return ul_ret;
    }

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_SCHEDULE_END);
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_SCHEDULE_END);

    return OAL_SUCC;
}
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rodata, code, rwdata, zidata  // return to default placement
#endif


oal_void dmac_tx_reset_flush(hal_to_dmac_device_stru *pst_device)
{
    oal_uint8                       uc_queue_num;
    hal_tx_dscr_stru                *pst_dscr;
    oal_uint32                      ul_ret;
    oal_netbuf_stru                 *pst_buf = OAL_PTR_NULL;

    /*首先清空发送完成事件队列*/
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_TX_COMP);

    /*清空发送队列*/
    for(uc_queue_num=0; uc_queue_num < HAL_TX_QUEUE_NUM; uc_queue_num++)
    {
        while(OAL_TRUE != oal_dlist_is_empty(&pst_device->ast_tx_dscr_queue[uc_queue_num].st_header))
        {
            pst_dscr = OAL_DLIST_GET_ENTRY(pst_device->ast_tx_dscr_queue[uc_queue_num].st_header.pst_next, hal_tx_dscr_stru, st_entry);
            pst_buf = pst_dscr->pst_skb_start_addr;
            if (OAL_PTR_NULL == pst_buf)
            {
                OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_reset_flush::pst_buf is null, memory has been covered.}");
                oal_dlist_delete_head(&(pst_device->ast_tx_dscr_queue[uc_queue_num].st_header));
                /* 释放发送描述符 */
                OAL_MEM_FREE((oal_void *)pst_dscr, OAL_TRUE);
                continue;
            }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            hal_tx_set_dscr_status(pst_device, pst_dscr, DMAC_TX_SOFT_PSM_BACK);
#else
            hal_tx_set_dscr_status(pst_device, pst_dscr, DMAC_TX_SOFT_RESET_BACK);
#endif
            ul_ret = dmac_tx_complete_buff(pst_device, pst_dscr);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_reset_txq_flush::dmac_tx_complete_buff failed[%d].", ul_ret);
                continue;
            }
        }
    }
    return;
}

/*lint -e19*/
#if (defined(_PRE_DEBUG_MODE) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE))
oal_module_symbol(g_ul_desc_addr);
#endif
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

