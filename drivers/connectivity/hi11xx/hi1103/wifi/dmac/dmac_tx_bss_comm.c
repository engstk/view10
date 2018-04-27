


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_util.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "oal_sdio.h"

#include "hal_ext_if.h"

#include "mac_frame.h"
#include "mac_data.h"

#include "dmac_tx_bss_comm.h"
#include "dmac_blockack.h"
#include "dmac_tx_complete.h"
#include "dmac_psm_ap.h"
#include "dmac_uapsd.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_11w.h"
#include "dmac_11i.h"
#include "dmac_dft.h"
#include "dmac_alg.h"
#include "dmac_fcs.h"

#ifdef _PRE_WLAN_FEATURE_DBAC
#include "mac_device.h"
#include "dmac_device.h"
#endif

#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_test_main.h"
#include "dmac_lpm_test.h"
#include "dmac_test_sch.h"
#include "dmac_config.h"
#endif

#ifdef _PRE_WIFI_DMT
#include "hal_witp_dmt_if.h"
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#include "pm_extern.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif

#include "oal_profiling.h"
#include "dmac_config.h"

#include "dmac_auto_adjust_freq.h"
#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
#include "dmac_vap.h"
#include "dmac_resource.h"
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
#include "oal_hcc_slave_if.h"
#endif
#ifdef _PRE_WLAN_FEATURE_AP_PM
#include "mac_pm.h"
#endif

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#include "dmac_user_extend.h"
#endif

#ifdef _PRE_WLAN_11K_STAT
#include "dmac_stat.h"
#endif

#include "dmac_power.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#include "hisi_customize_wifi.h"
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
#include "dmac_ftm.h"
#endif
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TX_BSS_COMM_C

/*****************************************************************************
  2 函数原型声明
*****************************************************************************/
OAL_STATIC oal_uint32  dmac_tx_update_protection_txop_alg(dmac_vap_stru *pst_dmac_vap, hal_tx_txop_alg_stru *pst_txop_alg, oal_uint8 uc_do_default_cfg);
OAL_STATIC OAL_INLINE oal_void dmac_tx_hw_send(hal_to_dmac_device_stru *pst_hal_device,
                                                      hal_tx_dscr_stru  *pst_tx_dscr,
                                                      mac_tx_ctl_stru   *pst_tx_ctl,
                                                      oal_dlist_head_stru *pst_tx_dscr_list_hdr,
                                                      oal_uint8          uc_ppdu_cnt);

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
/* 默认的MAC协议到PHY协议的映射关系 */
oal_uint8 g_auc_default_mac_to_phy_protocol_mapping[WLAN_PROTOCOL_BUTT] =
{
    WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE,     /* WLAN_LEGACY_11A_MODE            = 0 */
    WLAN_11B_PHY_PROTOCOL_MODE,             /* WLAN_LEGACY_11B_MODE            = 1 */
    WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE,     /* WLAN_LEGACY_11G_MODE            = 2 */
    WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE,     /* WLAN_MIXED_ONE_11G_MODE         = 3 */
    WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE,     /* WLAN_MIXED_TWO_11G_MODE         = 4 */
    WLAN_HT_PHY_PROTOCOL_MODE,              /* WLAN_HT_MODE                    = 5 */
    WLAN_VHT_PHY_PROTOCOL_MODE,             /* WLAN_VHT_MODE                   = 6 */
    WLAN_HT_PHY_PROTOCOL_MODE,              /* WLAN_HT_ONLY_MODE               = 7 */
    WLAN_VHT_PHY_PROTOCOL_MODE,             /* WLAN_VHT_ONLY_MODE              = 8 */
    WLAN_HT_PHY_PROTOCOL_MODE,              /* WLAN_HT_11G_MODE                = 9 */
};
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/*用于统计dmac接收帧信息*/
dmac_tx_pkts_stat_stru   g_tx_pkts_stat;
#endif

/*****************************************************************************
  4 函数实现
*****************************************************************************/

oal_uint32  dmac_tx_excp_free_netbuf(oal_netbuf_stru *pst_netbuf)
{
    oal_netbuf_stru             *pst_buf_next = OAL_PTR_NULL;
    mac_tx_ctl_stru             *pst_tx_ctl = OAL_PTR_NULL;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    mac_ieee80211_frame_stru    *pst_frame_header = OAL_PTR_NULL;
#endif

#if defined(_PRE_MEM_DEBUG_MODE) && (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
#ifdef SW_DEBUG
    oal_uint32 ul_return_addr      = (oal_uint32)__return_address();
#endif
#endif
    if (OAL_PTR_NULL == pst_netbuf)
    {
        return OAL_FAIL;
    }

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    pst_frame_header = (mac_ieee80211_frame_stru *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
#ifndef _PRE_PRODUCT_ID_HI110X_DEV
    if (OAL_PTR_NULL == pst_frame_header)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_excp_free_netbuf::pst_frame_header null.}");

        return OAL_FAIL;
    }
#endif
#endif
    MAC_SET_CB_FRAME_HEADER_ADDR(pst_tx_ctl, OAL_PTR_NULL);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    /*如果802.11mac头不在skb中，则释放其占用的内存*/
    if((0 == MAC_GET_CB_80211_MAC_HEAD_TYPE(pst_tx_ctl)) && (WLAN_DATA_BASICTYPE == MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_ctl)))
    {
        if (OAL_SUCC != OAL_MEM_FREE((oal_void *)pst_frame_header, OAL_TRUE))
        {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_excp_free_netbuf::frame header free failed.}");

            return OAL_FAIL;
        }
    }
#endif
    /* 只有al tx 才会释放这么大的内存,只释放一片内存 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    if(WLAN_LARGE_NETBUF_SIZE < MAC_GET_CB_MPDU_LEN(pst_tx_ctl))
    {
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_excp_free_netbuf::al tx netbuff free!.}");
        OAL_MEM_MULTI_NETBUF_FREE(pst_netbuf);
        return OAL_SUCC;
    }
#endif
#endif
    while (OAL_PTR_NULL != pst_netbuf)
    {
        pst_buf_next = oal_get_netbuf_next(pst_netbuf);

    #ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
        pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        dmac_alg_flowctl_netbuff_examination(pst_netbuf, pst_tx_ctl);
    #endif

        if(OAL_ERR_CODE_OAL_MEM_ALREADY_FREE==oal_netbuf_free(pst_netbuf))
        {
    #if defined(_PRE_MEM_DEBUG_MODE) && (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
    #ifdef SW_DEBUG
            OAL_IO_PRINT("double free caller[%x]!\r\n",ul_return_addr);
    #endif
    #endif
        }

        pst_netbuf = pst_buf_next;
    }

    return OAL_SUCC;
}


oal_void  dmac_tx_excp_free_dscr(oal_dlist_head_stru *pst_tx_dscr_list_hdr, hal_to_dmac_device_stru *pst_hal_device)
{
    oal_netbuf_stru     *pst_netbuf = OAL_PTR_NULL;
    hal_tx_dscr_stru    *pst_dscr;
    oal_dlist_head_stru *pst_dscr_node;

    while (OAL_FALSE == oal_dlist_is_empty(pst_tx_dscr_list_hdr))
    {
        pst_dscr_node = oal_dlist_delete_head(pst_tx_dscr_list_hdr);
        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_node, hal_tx_dscr_stru, st_entry);

        pst_netbuf = pst_dscr->pst_skb_start_addr;
        pst_dscr->pst_skb_start_addr = OAL_PTR_NULL;
        if (OAL_SUCC != OAL_MEM_FREE((oal_void *)pst_dscr, OAL_TRUE))
        {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_excp_free_dscr::pst_dscr null.}");
        }

        dmac_tx_excp_free_netbuf(pst_netbuf);
    }

    return;
}


oal_void  dmac_tx_pause_info(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_tx_ctl_stru            *pst_tx_ctl_first;
    dmac_user_stru             *pst_dmac_user;
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
    dmac_user_query_stats_stru *pst_query_stats;
    mac_device_stru            *pst_mac_device;
    mac_rate_info_stru          st_txrate;
#endif
    dmac_tid_stru              *pst_tid_queue;
    dmac_ba_tx_stru            *pst_tid_ba_hdl;

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_netbuf_stru            *pst_netbuf_tmp;
    oal_dlist_head_stru        *pst_entry_tmp;
    oal_uint32                  ul_count = 0;
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    oal_uint32                 ul_mode_sel;
#endif
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) && defined(_PRE_WLAN_DFT_STAT))
    OAL_STATIC oal_uint32      ul_reginfo_count = 1;
#endif

    pst_tx_ctl_first  = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    if(OAL_PTR_NULL == pst_tx_ctl_first)
    {
        return;
    }

    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_tx_ctl_first));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        return;
    }

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        return;
    }
#endif

    /* 取TID队列0信息 */
    pst_tid_queue = &pst_dmac_user->ast_tx_tid_queue[0];
#if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1151)
    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_pause_info::uc_is_paused = %d, uc_num_dq = %d, uc_retry_num = %d, us_mpdu_num = %d",
      pst_tid_queue->uc_is_paused, pst_tid_queue->uc_num_dq, pst_tid_queue->uc_retry_num, pst_tid_queue->us_mpdu_num);
#else
    OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_pause_info::user idx = %d, uc_is_paused = %d, uc_num_dq = %d, uc_retry_num = %d, us_mpdu_num = %d", 5,
                          pst_tid_queue->us_user_idx, pst_tid_queue->uc_is_paused, pst_tid_queue->uc_num_dq, pst_tid_queue->uc_retry_num, pst_tid_queue->us_mpdu_num);
#endif

    pst_tid_ba_hdl = pst_tid_queue->pst_ba_tx_hdl;

    if (pst_tid_ba_hdl != OAL_PTR_NULL)
    {
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_pause_info::start = %d, last_seq_num = %d, size = %d, baw_head = %d",
          pst_tid_ba_hdl->us_baw_start, pst_tid_ba_hdl->us_last_seq_num, pst_tid_ba_hdl->us_baw_size, pst_tid_ba_hdl->us_baw_head);
    }

    /* 加入判断，当前低功耗以及算法都依赖于mpdu和retry数，mpdu个数包含retry num数，如果与队列中数据不匹配会出错 */
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    ul_count = 0;
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry_tmp, &pst_tid_queue->st_retry_q)
    {
        ul_count++;
    }
    if (ul_count != pst_tid_queue->uc_retry_num)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_pause_info::warning!!! retry num:%d != retry q element num.}", ul_count);
    }

    OAL_NETBUF_SEARCH_FOR_EACH(pst_netbuf_tmp, &pst_tid_queue->st_buff_head)
    {
        ul_count++;
    }
    if (ul_count != pst_tid_queue->us_mpdu_num)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_pause_info::warning!!! mpdu num:%d != retry q and netbuf q element num.", ul_count);
    }
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
    pst_query_stats = &pst_dmac_user->st_query_stats;

    /* 定时打印统计，信息上报字段st_query_stats，1102专用 */
    /*
    ul_drv_rx_pkts：    接收数据(硬件上报，包含rx描述符不成功的帧)数目，仅仅统计数据帧
    ul_rx_dropped_misc：接收失败(决定丢弃的帧)次数

    tx_total：          发送数据帧总数，不包括软件重传
    ul_hw_tx_pkts：     发送成功帧数，(针对AMPDU需要根据BA MAP) ，仅仅统计数据帧
    ul_tx_failed：      发送失败最终丢弃的次数，仅仅统计数据帧
    ul_hw_tx_failed:    发送完成中断上报发送失败的个数，仅仅统计数据帧
    tx_retry_cnt：      硬件重传总次数+软件重传总次数，硬件重传针对AMPDU= pkt_cnt×retry_cnt
    ul_HccPktMiss:         流控丢包统计。
    */
    OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_pause_info:TX_pkts: total = %u, tx_dropped = %u, tx_succ = %u, total_retry = %u, hw_tx_fail = %u, replay = %u, replay_droped = %u.}", 7,
        pst_query_stats->ul_tx_total, pst_query_stats->ul_tx_failed, pst_query_stats->ul_hw_tx_pkts, pst_query_stats->ul_tx_retries, pst_query_stats->ul_hw_tx_failed, pst_query_stats->ul_rx_replay,
        pst_query_stats->ul_rx_replay_droped);
    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_pause_info:RSSI = %d ; RX_pkts: total = %u, drv_dropped = %u, hcc_flowctrl_miss = %u.}",
        oal_get_real_rssi(pst_dmac_vap->st_query_stats.s_signal), pst_query_stats->ul_drv_rx_pkts, pst_query_stats->ul_rx_dropped_misc, hcc_get_rx_pktmiss_count());

    dmac_config_get_tx_rate_info(&(pst_dmac_vap->st_tx_alg), &(pst_mac_device->st_mac_rates_11g[0]), &st_txrate);
    OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_pause_info: tx rate info legacy = %d, mcs = %d, flags = %d, nss = %d, rx rate(kbps) = %d}", 5,
        st_txrate.legacy, st_txrate.mcs, st_txrate.flags, st_txrate.nss, pst_dmac_vap->pst_hal_device->ul_rx_rate);

    OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_pause_info: hardware Q inf be= %d, vi = %d, vo = %d, mgmt = %d, netbuf(1544)free = %d}", 5,
        pst_hal_device->ast_tx_dscr_queue[WLAN_WME_AC_BE].uc_ppdu_cnt,
        pst_hal_device->ast_tx_dscr_queue[WLAN_WME_AC_VI].uc_ppdu_cnt,
        pst_hal_device->ast_tx_dscr_queue[WLAN_WME_AC_VO].uc_ppdu_cnt,
        pst_hal_device->ast_tx_dscr_queue[WLAN_WME_AC_MGMT].uc_ppdu_cnt,
        g_st_netbuf_pool.ast_subpool_table[OAL_MEM_NETBUF_POOL_ID_LARGE_PKT].us_free_cnt);

#ifdef _PRE_WLAN_DFT_STAT
#ifdef _PRE_WLAN_FEATURE_STA_PM
    /* mac发送接收关键信息打印 */
    dmac_mac_key_statis_info(pst_dmac_vap,pst_mac_device);
#endif
#endif /* _PRE_WLAN_DFT_STAT */

#ifdef _PRE_WLAN_PHY_BUGFIX_IMPROVE_CE_TH
    dmac_rx_compatibility_show_stat(pst_dmac_user);
#endif

    /* 异常发送完成时间的统计信息 */
    if (pst_hal_device->st_tx_excp_info.us_tx_excp_cnt)
    {
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                    "{dmac_tx_pause_info:: Excp_tx_cnt %d; The max_offset_tsf %d, with mpdu_len %d, q_num %d}",
                    pst_hal_device->st_tx_excp_info.us_tx_excp_cnt,
                    pst_hal_device->st_tx_excp_info.us_max_offset_tsf,
                    pst_hal_device->st_tx_excp_info.us_mpdu_len,
                    pst_hal_device->st_tx_excp_info.uc_q_num);
        OAL_MEMZERO((oal_uint8 *)&pst_hal_device->st_tx_excp_info, OAL_SIZEOF(hal_tx_excp_info_stru));
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_get_btcoex_pa_status(pst_hal_device, &ul_mode_sel);
    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_COEX, "{dmac_tx_pause_info:: mode select: 0x%x}", ul_mode_sel);
#endif

    /* 进入此函数，每隔一段时间输出寄存器OTA信息和低功耗统计信息 */
    if (0 == (++(pst_hal_device->st_hal_dev_dft_stat.uc_fe_print_ctrl) % 3))
    {
#ifdef _PRE_WLAN_DFT_STAT
        if (ul_reginfo_count)
        {
            pst_dmac_vap->pst_hal_device->uc_reg_info_flag = 0x10;
            ul_reginfo_count = 0;
        }
        else
        {
            pst_dmac_vap->pst_hal_device->uc_reg_info_flag = 0xF;
            ul_reginfo_count = 1;
        }
        dmac_dft_report_all_ota_state(&pst_dmac_vap->st_vap_base_info);
        pst_dmac_vap->pst_hal_device->uc_reg_info_flag = 0x1F;
#endif  /* _PRE_WLAN_DFT_STAT */
        /* 打印sta低功耗维测信息 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
        /* 协议的计数 */
        dmac_pm_key_info_dump(pst_dmac_vap);

        /* 平台的计数*/
        pfn_wlan_dumpsleepcnt();
#endif  /* _PRE_WLAN_FEATURE_STA_PM */

    }
#endif  /* #if (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102_DEV) */

    return;
}



#ifdef _PRE_WLAN_FEATURE_AP_PM

OAL_STATIC OAL_INLINE oal_void dmac_tx_data_in_wow_mode(hal_to_dmac_device_stru *pst_hal_device,
                                                                  dmac_vap_stru *pst_dmac_vap,
                                                                  oal_dlist_head_stru *pst_tx_dscr_list_hdr)
{
    mac_pm_arbiter_stru         *pst_pm_arbiter;
    mac_device_stru             *pst_mac_device;

    pst_mac_device = (mac_device_stru*)mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_data_in_wow_mode::pst_mac_device null.");
        return;
    }
    pst_pm_arbiter = (mac_pm_arbiter_stru*)(pst_mac_device->pst_pm_arbiter);

    if ((pst_pm_arbiter != OAL_PTR_NULL)
        && (OAL_TRUE == pst_mac_device->en_pm_enable)
        && (DEV_PWR_STATE_WOW == pst_pm_arbiter->uc_cur_state))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_data_in_wow_mode::send data when device in wow.");
        dmac_tx_dump_data(pst_hal_device, pst_tx_dscr_list_hdr);
    }
}
#endif


oal_uint32  dmac_tx_update_protection_all_txop_alg(dmac_vap_stru *pst_dmac_vap)
{
    /*更新单播数据帧txop alg参数*/
    dmac_tx_update_protection_txop_alg(pst_dmac_vap, &(pst_dmac_vap->st_tx_alg), OAL_TRUE);

    /*更新组播数据帧txop alg参数*/
    dmac_tx_update_protection_txop_alg(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_mcast), OAL_TRUE);

     /*更新广播数据帧txop alg参数*/
    dmac_tx_update_protection_txop_alg(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_bcast), OAL_TRUE);

    /*更新单播管理帧txop alg参数*/
    dmac_tx_update_protection_txop_alg(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G]), OAL_TRUE);

    dmac_tx_update_protection_txop_alg(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G]), OAL_TRUE);

    /*更新组播广播管理帧txop alg参数*/
    dmac_tx_update_protection_txop_alg(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G]), OAL_TRUE);

    dmac_tx_update_protection_txop_alg(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G]), OAL_TRUE);

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_tx_get_amsdu_params(
                                        oal_netbuf_stru  *pst_netbuf,
                                        mac_tx_ctl_stru  *pst_tx_ctl,
                                        hal_tx_mpdu_stru *pst_mpdu)
{
    oal_netbuf_stru     *pst_netbuf_next;
    mac_tx_ctl_stru     *pst_tx_ctl_next;
    oal_uint16           us_msdu_len;

    pst_netbuf_next = oal_get_netbuf_next(pst_netbuf);
    if (OAL_PTR_NULL == pst_netbuf_next)
    {
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        /* 如果是AMSDU帧类型,需要去掉SKB起始的ETHER HEAD */
        if (OAL_TRUE == MAC_GET_CB_HAS_EHTER_HEAD(pst_tx_ctl))
        {
            pst_mpdu->ast_msdu_addr[0].ul_msdu_addr0 = (oal_uint32)(oal_netbuf_payload(pst_netbuf) + WLAN_LARGE_SKB_AMSDU_PAYLOAD_OFFSET);
        }
        else
#endif
        {
            pst_mpdu->ast_msdu_addr[0].ul_msdu_addr0 = (oal_uint32)oal_netbuf_payload(pst_netbuf);
        }

        pst_mpdu->ast_msdu_addr[0].us_msdu0_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl);
        pst_mpdu->ast_msdu_addr[0].ul_msdu_addr1 = 0;
        pst_mpdu->ast_msdu_addr[0].us_msdu1_len = 0;
        /* 只有一个MSDU */
        pst_mpdu->st_mpdu_mac_hdr.uc_num_sub_msdu = 1;
    }
    else
    {
        pst_mpdu->ast_msdu_addr[0].ul_msdu_addr0 = (oal_uint32)oal_netbuf_payload(pst_netbuf) + 2;
        /* 对于AMSDU帧,hmac保留了ETHER HEAD,但未统计HEAD LEN在MPDU LEN中,此处需要加ETHER HEAD */
        us_msdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl) + ETHER_HDR_LEN;
        /* 打pad对齐 */
        pst_mpdu->ast_msdu_addr[0].us_msdu0_len = OAL_ROUND_UP(us_msdu_len, 4);

        pst_tx_ctl_next = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf_next);
        pst_mpdu->ast_msdu_addr[0].ul_msdu_addr1 = (oal_uint32)oal_netbuf_payload(pst_netbuf_next) + 2;
        pst_mpdu->ast_msdu_addr[0].us_msdu1_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl_next) + ETHER_HDR_LEN;

        /* 两个MSDU组成的AMSDU */
        pst_mpdu->st_mpdu_mac_hdr.uc_num_sub_msdu = 2;
    }

    return OAL_SUCC;
}
#else

oal_uint32  dmac_tx_get_amsdu_params(
                                        oal_netbuf_stru  *pst_netbuf,
                                        mac_tx_ctl_stru  *pst_tx_ctl,
                                        hal_tx_mpdu_stru *pst_mpdu)
{
    oal_uint8            uc_msdu_num;                /* MPDU中子msdu的个数 */
    oal_uint8            uc_loop;
    oal_netbuf_stru     *pst_next_buf;
    oal_uint8           *puc_netbuf_data;            /* netbuf的data指针 */

    if (OAL_UNLIKELY((OAL_TRUE == MAC_GET_CB_IS_AMSDU(pst_tx_ctl)) &&
                     (OAL_TRUE != MAC_GET_CB_IS_FIRST_MSDU(pst_tx_ctl))))
    {
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_get_amsdu_params::not first MSDU in MPDU.");
        return OAL_FAIL;
    }

    uc_msdu_num = MAC_GET_CB_NETBUF_NUM(pst_tx_ctl);
    if((uc_msdu_num >> 1) >= WLAN_DSCR_SUBTABEL_MAX_NUM)
    {
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_get_amsdu_params::buffer overflow.");
        return OAL_FAIL;
    }

    for (uc_loop = 0; uc_loop < uc_msdu_num; uc_loop++)
    {
        pst_next_buf = oal_get_netbuf_next(pst_netbuf);

        /* 获取SUB-MSDU的头指针 */
        puc_netbuf_data   = oal_netbuf_payload(pst_netbuf);

        /* 填写子帧发送参数 */
        if (!(uc_loop & 0x1))
        {
            pst_mpdu->ast_msdu_addr[uc_loop >> 1].ul_msdu_addr0 = (oal_uint32)puc_netbuf_data;
            pst_mpdu->ast_msdu_addr[uc_loop >> 1].us_msdu0_len  = (oal_uint16)oal_netbuf_get_len(pst_netbuf);
        }
        else
        {
            pst_mpdu->ast_msdu_addr[uc_loop >> 1].ul_msdu_addr1 = (oal_uint32)puc_netbuf_data;
            pst_mpdu->ast_msdu_addr[uc_loop >> 1].us_msdu1_len  = (oal_uint16)oal_netbuf_get_len(pst_netbuf);
        }

        pst_netbuf = pst_next_buf;
    }

    /* 如果子帧个数是奇数，则最后填的子表的第二段不存在，清零 */
    if (uc_msdu_num & 0x1)
    {
        pst_mpdu->ast_msdu_addr[uc_msdu_num >> 1].ul_msdu_addr1 = 0;
        pst_mpdu->ast_msdu_addr[uc_msdu_num >> 1].us_msdu1_len  = 0;
    }

    return OAL_SUCC;
}
#endif



OAL_STATIC OAL_INLINE oal_void dmac_tx_set_basic_ctrl_dscr(hal_tx_dscr_stru *pst_mgmt_dscr, mac_tx_ctl_stru *pst_tx_ctl, hal_tx_mpdu_stru *pst_mpdu)
{
    oal_uint8 uc_ac;

    pst_mgmt_dscr->us_original_mpdu_len = pst_mpdu->us_mpdu_len;
    uc_ac = MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl);
    pst_mgmt_dscr->uc_q_num = HAL_AC_TO_Q_NUM(uc_ac);

    /* 更新发送帧的长度 */
    pst_mpdu->ast_msdu_addr[0].us_msdu0_len  = pst_mpdu->us_mpdu_len - pst_mpdu->st_mpdu_mac_hdr.uc_mac_hdr_len;

    return;
}

#ifdef _PRE_WLAN_FEATURE_STA_PM

OAL_STATIC OAL_INLINE oal_void  dmac_tx_set_pwr_mgmt(dmac_vap_stru  *pst_dmac_vap, dmac_user_stru *pst_dmac_user,mac_ieee80211_frame_stru *pst_mac_header, mac_tx_ctl_stru *pst_tx_ctl)
{
    oal_uint16  us_user_idx = 0;

    if (OAL_SUCC != mac_vap_find_user_by_macaddr(&(pst_dmac_vap->st_vap_base_info), pst_mac_header->auc_address1, &us_user_idx))
    {
        return;
    }

    /* pst_dmac_user通过tx cb内的user idx获取,如果填非法值,在发送完成内该包会提前释放,进不到低功耗的处理,故置位时需保证tx cb user idx合法 */
    if (OAL_PTR_NULL != pst_dmac_user)
    {
        /* 不能改变null帧的pm位,否则低功耗状态会出错 */
        if ((WLAN_NULL_FRAME !=  MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl)->st_frame_control.bit_sub_type)
            &&(WLAN_QOS_NULL_FRAME != MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl)->st_frame_control.bit_sub_type)
            && !MAC_GET_CB_IS_SMPS_FRAME(pst_tx_ctl)
            && !MAC_GET_CB_IS_OPMODE_FRAME(pst_tx_ctl))
        {
            dmac_psm_tx_set_power_mgmt_bit(pst_dmac_vap, pst_tx_ctl);
        }
    }
    else
    {
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_set_pwr_mgmt:staut send mgmt type[%d][%d] to [%d]user,but tx cb user idx[%d] invaild!!!}",
                pst_mac_header->st_frame_control.bit_type,pst_mac_header->st_frame_control.bit_sub_type,us_user_idx,MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
    }
}


OAL_STATIC oal_void  dmac_tx_process_psm_sta(dmac_vap_stru  *pst_dmac_vap, dmac_user_stru *pst_dmac_user,mac_ieee80211_frame_stru *pst_mac_header, mac_tx_ctl_stru *pst_tx_ctl)
{
    /* STA模式下低功耗处理 */
    if (WLAN_VAP_MODE_BSS_STA != pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        return;
    }

    /* STA模式下根据节能模式设置单播管理帧power_mgmt位 */
    dmac_tx_set_pwr_mgmt(pst_dmac_vap, pst_dmac_user, pst_mac_header, pst_tx_ctl);

    /* cl || legacy sta抛事件恢复前端, 扫描时不需要切换低功耗状态来打开前端 */
    if (!IS_P2P_DEV(&pst_dmac_vap->st_vap_base_info) && (HAL_DEVICE_SCAN_STATE != GET_HAL_DEVICE_STATE(pst_dmac_vap->pst_hal_device)))
    {
        /* fast ps开启切协议状态到active,fast ps没开启直接切hal device自状态打开前端 */
        if (OAL_TRUE == dmac_is_sta_fast_ps_enabled(&pst_dmac_vap->st_sta_pm_handler))
        {
            dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_TX_MGMT, 0, OAL_PTR_NULL);
        }
        else
        {
            hal_device_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), HAL_DEVICE_EVENT_VAP_CHANGE_TO_ACTIVE, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
        }
    }
}
#endif

#ifdef _PRE_WLAN_FEATURE_DFR

OAL_STATIC OAL_INLINE oal_void  dmac_tx_process_dfr(hal_to_dmac_device_stru *pst_hal_device)
{
    if (OAL_FALSE == pst_hal_device->en_dfr_enable)
    {
        return;
    }

    /* 启动无发送完成中断检测定时器 */
    if (OAL_FALSE == pst_hal_device->st_dfr_tx_prot.st_tx_prot_timer.en_is_enabled)
    {
        FRW_TIMER_RESTART_TIMER(&pst_hal_device->st_dfr_tx_prot.st_tx_prot_timer, WLAN_TX_PROT_TIMEOUT, OAL_TRUE);
    }
}
#endif

#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP

oal_void dmac_vap_update_rsp_frm_rate(dmac_device_stru *pst_dmac_dev, oal_uint8 uc_protocol_mode, oal_uint8 uc_bandwidth, oal_uint8 uc_ref_rate)
{
    /* 数据速率为11ac */
    if (WLAN_VHT_PHY_PROTOCOL_MODE == uc_protocol_mode)
    {
        /* 80M */
        if (uc_bandwidth >= WLAN_BAND_ASSEMBLE_80M)
        {
            /* 响应帧使用24M速率 */
            if (WLAN_PHY_RATE_24M != pst_dmac_dev->uc_rsp_frm_rate_val)
            {
                pst_dmac_dev->uc_rsp_frm_rate_val = WLAN_PHY_RATE_24M;
                hal_set_rsp_rate((oal_uint32)pst_dmac_dev->uc_rsp_frm_rate_val);
            }
        }
        /* 40M */
        else if (uc_bandwidth >= WLAN_BAND_ASSEMBLE_40M)
        {
            if (WLAN_VHT_MCS0 == uc_ref_rate)
            {
                if (WLAN_PHY_RATE_12M != pst_dmac_dev->uc_rsp_frm_rate_val)
                {
                    pst_dmac_dev->uc_rsp_frm_rate_val = WLAN_PHY_RATE_12M;
                    hal_set_rsp_rate((oal_uint32)pst_dmac_dev->uc_rsp_frm_rate_val);
                }
            }
            else
            {
                /* 响应帧使用24M速率 */
                if (WLAN_PHY_RATE_24M != pst_dmac_dev->uc_rsp_frm_rate_val)
                {
                    pst_dmac_dev->uc_rsp_frm_rate_val = WLAN_PHY_RATE_24M;
                    hal_set_rsp_rate((oal_uint32)pst_dmac_dev->uc_rsp_frm_rate_val);
                }
            }
        }
        /* 20M带宽 */
        else
        {
            if (uc_ref_rate >= WLAN_VHT_MCS3)
            {
                /* 24M */
                if (WLAN_PHY_RATE_24M != pst_dmac_dev->uc_rsp_frm_rate_val)
                {
                    pst_dmac_dev->uc_rsp_frm_rate_val = WLAN_PHY_RATE_24M;
                    hal_set_rsp_rate((oal_uint32)pst_dmac_dev->uc_rsp_frm_rate_val);
                }
            }
            else if (uc_ref_rate >= WLAN_VHT_MCS1)
            {
                /* 12M */
                if (WLAN_PHY_RATE_12M != pst_dmac_dev->uc_rsp_frm_rate_val)
                {
                    pst_dmac_dev->uc_rsp_frm_rate_val = WLAN_PHY_RATE_12M;
                    hal_set_rsp_rate((oal_uint32)pst_dmac_dev->uc_rsp_frm_rate_val);
                }
            }
            else
            {
                /* 6M */
                if (WLAN_PHY_RATE_6M != pst_dmac_dev->uc_rsp_frm_rate_val)
                {
                    pst_dmac_dev->uc_rsp_frm_rate_val = WLAN_PHY_RATE_6M;
                    hal_set_rsp_rate((oal_uint32)pst_dmac_dev->uc_rsp_frm_rate_val);
                }
            }
        }


    }
    /* 11a */
    else if (WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE == uc_protocol_mode)
    {
        if (WLAN_PHY_RATE_6M != pst_dmac_dev->uc_rsp_frm_rate_val)
        {
            pst_dmac_dev->uc_rsp_frm_rate_val = WLAN_PHY_RATE_6M;
            hal_set_rsp_rate((oal_uint32)pst_dmac_dev->uc_rsp_frm_rate_val);
        }
    }
    /* DBAC场景，1个VAP为11b时 */
    else if (WLAN_11B_PHY_PROTOCOL_MODE == uc_protocol_mode)
    {
        if (WLAN_11B_PHY_PROTOCOL_MODE != pst_dmac_dev->uc_rsp_frm_rate_val)
        {
            pst_dmac_dev->uc_rsp_frm_rate_val = WLAN_11B_PHY_PROTOCOL_MODE;
            hal_set_rsp_rate((oal_uint32)pst_dmac_dev->uc_rsp_frm_rate_val);
        }
    }
}


OAL_STATIC OAL_INLINE oal_void  dmac_tx_data_update_rsp_frame(hal_to_dmac_device_stru *pst_hal_device,
                                                                        dmac_vap_stru *pst_dmac_vap,
                                                                        hal_tx_txop_alg_stru *pst_txop_alg,
                                                                        oal_uint8  uc_ac)
{
    dmac_device_stru            *pst_dmac_dev = OAL_PTR_NULL;

    pst_dmac_dev = dmac_res_get_mac_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_dmac_dev)
    {
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_data_update_rsp_frame::pst_dmac_dev null.dev id [%d]}", pst_hal_device->uc_mac_device_id);
        return ;
    }

    if ((uc_ac < WLAN_WME_AC_BUTT)
        && (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
        && (OAL_TRUE == pst_dmac_dev->en_state_in_sw_ctrl_mode))
    {
        dmac_vap_update_rsp_frm_rate(pst_dmac_dev,
                                    pst_txop_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode,
                                    pst_txop_alg->st_rate.en_channel_bandwidth,
                                    pst_txop_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs);
    }

}
#endif

#ifdef _PRE_DEBUG_MODE
OAL_STATIC OAL_INLINE oal_void  dmac_tx_debug_print(hal_to_dmac_device_stru *pst_hal_device,
                                                          dmac_vap_stru *pst_dmac_vap,
                                                          dmac_user_stru *pst_dmac_user,
                                                          hal_tx_dscr_stru *pst_tx_dscr,
                                                          mac_tx_ctl_stru *pst_tx_ctl,
                                                          oal_uint8        uc_ppdu_cnt)
{
    oal_uint8                    uc_tid;
    dmac_tid_stru               *pst_tid_queue = OAL_PTR_NULL;

    g_ast_tx_complete_stat[pst_hal_device->uc_mac_device_id].ul_tx_data_num += uc_ppdu_cnt;

    uc_tid = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);
    pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[uc_tid]);

    if ((pst_tx_dscr->bit_is_ampdu == 0) && (DMAC_TX_MODE_AGGR == pst_tid_queue->en_tx_mode))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_data::sending none_ampdu while ba is setup.}");
    }
    else if ((pst_tx_dscr->bit_is_ampdu == 1) && (DMAC_TX_MODE_NORMAL == pst_tid_queue->en_tx_mode))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_data::sending ampdu while ba is not setup.}");
    }

}
#if 0
OAL_STATIC OAL_INLINE oal_void dmac_tx_dscr_trace_debug(oal_dlist_head_stru *pst_dscr_head)
{
    oal_dlist_head_stru        *pst_dscr_tra_entry  = OAL_PTR_NULL;
    hal_tx_dscr_stru           *pst_trace_dscr      = OAL_PTR_NULL;

    OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_tra_entry, pst_dscr_head)
    {
        pst_trace_dscr = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dscr_tra_entry, hal_tx_dscr_stru, st_entry);
        OAL_MEM_TRACE(pst_trace_dscr, OAL_FALSE);
    }

    return;
}
#endif
#endif




OAL_STATIC OAL_INLINE oal_void dmac_tx_mgmt_set_txop_security_param(dmac_vap_stru  *pst_dmac_vap,
                                                      mac_tx_ctl_stru *pst_tx_ctl, hal_tx_txop_feature_stru *pst_txop_feature,
                                                      oal_netbuf_stru *pst_netbuf, hal_tx_mpdu_stru *pst_mpdu)
{
    mac_user_stru                *pst_multi_user;
    mac_ieee80211_frame_stru     *pst_mac_header;

    pst_mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf);

    /* 管理帧默认不加密*/
    pst_txop_feature->st_security.en_cipher_key_type      = WLAN_KEY_TYPE_TX_GTK;
    pst_txop_feature->st_security.en_cipher_protocol_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
    pst_txop_feature->st_security.uc_cipher_key_id = 0;

    /* 漫游状态时，管理帧不加密 */
#ifdef _PRE_WLAN_FEATURE_ROAM
    if (MAC_VAP_STATE_ROAMING == pst_dmac_vap->st_vap_base_info.en_vap_state)
    {
        return ;
    }
#endif

    /* 如果报文中的加密位置1了,需要设置发送描述符为加密*/
    /* 需要区别如下2种情况下*/
    /* 1、WEP加密套件下，认证的第三帧,此时，用户还没有关联成功*/
    /* 2、WPA2加密套件下，协商11W成功后，11W规定的部分管理帧*/
    if (1 == mac_is_protectedframe((oal_uint8 *)pst_mac_header))
    {
        pst_multi_user = mac_res_get_mac_user(pst_dmac_vap->st_vap_base_info.us_multi_user_idx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_multi_user))
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_set_txopps_security_param::pst_multi_user[%d] is NULL!}",
                pst_dmac_vap->st_vap_base_info.us_multi_user_idx);
            return ;
        }

        pst_txop_feature->st_security.en_cipher_protocol_type = pst_multi_user->st_user_tx_info.st_security.en_cipher_protocol_type;
        pst_txop_feature->st_security.en_cipher_key_type      = pst_multi_user->st_user_tx_info.st_security.en_cipher_key_type;

        if (OAL_TRUE == mac_is_wep_enabled(&pst_dmac_vap->st_vap_base_info))

        {
            /* TBD非法值处理 */
            pst_txop_feature->st_security.uc_cipher_key_id = mac_vap_get_default_key_id(&pst_dmac_vap->st_vap_base_info);
        }
    }

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    if (OAL_FALSE == ETHER_IS_MULTICAST(pst_mac_header->auc_address1))
    {
        /* 设置PMF管理帧单播加密位 */
        dmac_11w_set_ucast_mgmt_frame(pst_dmac_vap, pst_tx_ctl, pst_txop_feature, pst_netbuf, pst_mac_header);
    }
    else
    {
        /* 设置PMF组播管理帧加密位 */
        dmac_11w_set_mcast_mgmt_frame(pst_dmac_vap, pst_txop_feature, pst_netbuf, pst_mpdu);
    }
#endif

    return ;

}



#if defined(_PRE_PRODUCT_ID_HI110X_DEV)

OAL_STATIC oal_void dmac_sta_tsf_restore(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint32                      ul_bank3_bcn_period;

    /* 只1102需要tsf_restore逻辑 */
    if ((WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode))
    {
        /* 发送时如果发现staut bank3 的beacon周期为0 需要将ap的tsf同步给sta */
        hal_vap_get_beacon_period(pst_dmac_vap->pst_hal_vap, &ul_bank3_bcn_period);
        if (0 == ul_bank3_bcn_period)
        {
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,"tx mgmt staut bank3 bcn period 0 ap sync tsf to sta");
#else
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,"tx mgmt staut bank3 bcn period 0 ap sync tsf to sta");
#endif

#ifdef _PRE_WLAN_MAC_BUGFIX_TSF_SYNC
            hal_sta_tsf_restore(pst_dmac_vap->pst_hal_vap);
#endif
        }
    }
}
#endif

#ifdef  _PRE_WLAN_FEATURE_TSF_SYNC

oal_uint16 dmac_sync_tx_dscr_tsf(hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 us_tsf)
{
    oal_uint16      us_tmp_tsf_lo = 0;
    dmac_vap_stru   *pst_dmac_vap;


    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_hal_vap->uc_mac_vap_id);
    if(OAL_PTR_NULL == pst_dmac_vap)
    {
        return us_tmp_tsf_lo;
    }

    us_tmp_tsf_lo = us_tsf - pst_dmac_vap->us_sync_tsf_value; /*sync the tsf*/
    if (pst_dmac_vap->us_sync_tsf_value != 0)
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "dmac_sync_tx_dscr_tsf:origi tsf:[%d],after sync us_tmp_tsf_lo:[%d]",us_tsf, us_tmp_tsf_lo);
    }
    return us_tmp_tsf_lo;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_void dmac_tx_updata_probe_rsp_para(dmac_vap_stru *pst_dmac_vap, hal_tx_txop_alg_stru *pst_txop_alg, mac_ieee80211_frame_stru *pst_mac_header)
{
    if ((WLAN_MANAGEMENT == pst_mac_header->st_frame_control.bit_type) &&
        (WLAN_PROBE_RSP == pst_mac_header->st_frame_control.bit_sub_type))
    {
        pst_txop_alg->ast_per_rate[0].rate_bit_stru.bit_tx_count = 3;
        pst_txop_alg->ast_per_rate[1].rate_bit_stru.bit_tx_count = 0;
        pst_txop_alg->ast_per_rate[2].rate_bit_stru.bit_tx_count = 0;
        pst_txop_alg->ast_per_rate[3].rate_bit_stru.bit_tx_count = 0;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        if (DMAC_VAP_GET_BTCOEX_STATUS(pst_dmac_vap)->un_bt_status.st_bt_status.bit_bt_sco)
        {
            pst_txop_alg->ast_per_rate[0].rate_bit_stru.bit_tx_count = 1;
        }
#endif
    }

    return;
}
#endif





#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rwdata = "BTCM", code ="ATCM", zidata = "BTCM", rodata = "ATCM"
#endif



OAL_STATIC oal_void dmac_tx_get_timestamp(hal_to_dmac_device_stru *pst_hal_device, hal_to_dmac_vap_stru *pst_hal_vap, oal_uint16 *pus_tsf)
{
    oal_uint32                   ul_tsf = 0;
#if (defined(_PRE_WLAN_FEATURE_PROXYSTA) || defined(_PRE_WLAN_PRODUCT_1151V200))
    hal_to_dmac_vap_stru        *pst_hal_vap_sta0;

    /* proxysta 没有tsf寄存器，如果取Proxy STA的寄存器，用sta的tsf寄存器代替,这里将Proxy STA的hal_vap换成sta0的hal_vap */
#if (defined(_PRE_WLAN_FEATURE_PROXYSTA) && defined(_PRE_WLAN_PRODUCT_1151V200))
    if (((pst_hal_vap->uc_vap_id >= WLAN_PROXY_STA_START_ID) && (pst_hal_vap->uc_vap_id <= WLAN_PROXY_STA_END_ID))
       || (pst_hal_vap->uc_vap_id == 28))
#else
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if ((pst_hal_vap->uc_vap_id >= WLAN_PROXY_STA_START_ID) && (pst_hal_vap->uc_vap_id <= WLAN_PROXY_STA_END_ID))
#endif
#ifdef _PRE_WLAN_PRODUCT_1151V200
    if (pst_hal_vap->uc_vap_id == 28)
#endif
#endif
    {
        hal_get_hal_vap(pst_hal_device, WLAN_STA0_HAL_VAP_ID, &pst_hal_vap_sta0);
        hal_vap_tsf_get_32bit(pst_hal_vap_sta0, &ul_tsf);
    }
    else
#endif
    {
        hal_vap_tsf_get_32bit(pst_hal_vap, &ul_tsf);
    }

#if ((_PRE_OS_VERSION_RAW == _PRE_OS_VERSION) &&defined(_PRE_PRODUCT_ID_HI110X_DEV))
    /*Zourong add for debug,make sure not read tsf when inner tsf is being stopped*/
    if(0!=(READW(0x50000400)&(1<<3)))
    {
        OAM_ERROR_LOG0(pst_hal_vap->uc_vap_id,OAM_SF_TX, "{dmac_tx_get_timestamp::Read TSF When ext tsf is working}");
    }
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
#ifdef _PRE_WLAN_PRODUCT_1151V200
    *pus_tsf = ((ul_tsf >> 10) & 0xffff); /* 将微妙换算成毫秒，取16位 */
#else
    *pus_tsf = ((ul_tsf >> 10) & 0xffff) - 1; /* 将微妙换算成毫秒，取16位 */
#endif
#else
    *pus_tsf = ((ul_tsf >> 10) & 0xffff); /* 将微妙换算成毫秒，取16位 */

#ifdef  _PRE_WLAN_FEATURE_TSF_SYNC
    *pus_tsf = dmac_sync_tx_dscr_tsf(pst_hal_vap, *pus_tsf);
#endif

#endif
}

#if ((WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS) && defined(_PRE_WLAN_FEATURE_TXBF_HT))

OAL_STATIC oal_uint32  dmac_tx_bf_add_ht_control_field(
                                    hal_to_dmac_device_stru  *pst_hal_device,
                                    mac_tx_ctl_stru          *pst_tx_ctl,
                                    oal_dlist_head_stru      *pst_tx_dscr_list_hdr,
                                    hal_tx_txop_alg_stru     *pst_txop_alg,
                                    hal_tx_ppdu_feature_stru *pst_ppdu_feature)
{
    hal_tx_dscr_stru                        *pst_tx_dscr;
    oal_dlist_head_stru                     *pst_dscr_entry;
    oal_uint8                                uc_htc_hdr_len;

    if (OAL_TRUE != MAC_GET_CB_IS_QOS_DATA(pst_tx_ctl))
    {
        /* 非QoS帧，不涉及HTC字段，直接返回 */
        return OAL_SUCC;
    }

    if (OAL_TRUE == MAC_GET_CB_IS_4ADDRESS(pst_tx_ctl))
    {
        uc_htc_hdr_len = MAC_80211_QOS_HTC_4ADDR_FRAME_LEN;
    }
    else
    {
        uc_htc_hdr_len = MAC_80211_QOS_HTC_FRAME_LEN;
    }

    if ((WLAN_STAGGERED_SOUNDING == pst_txop_alg->st_rate.en_sounding_mode)
         && (uc_htc_hdr_len != MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl)))
    {
        /* 如果是sounding并且没有设置过HTC，添加HTC头 */

        pst_dscr_entry = pst_tx_dscr_list_hdr->pst_next;
        pst_tx_dscr    = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);

        dmac_tx_set_htc_field(pst_hal_device, pst_tx_dscr, pst_tx_ctl, pst_ppdu_feature);
    }
    else if ((WLAN_STAGGERED_SOUNDING != pst_txop_alg->st_rate.en_sounding_mode)
        && (uc_htc_hdr_len == MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl)))
    {
        /* 如果不是sounding并且有设置过HTC，去掉HTC头 */

        pst_dscr_entry = pst_tx_dscr_list_hdr->pst_next;
        pst_tx_dscr    = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);

        dmac_tx_unset_htc_field(pst_hal_device, pst_tx_dscr, pst_tx_ctl, pst_ppdu_feature);
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC OAL_INLINE oal_uint32 dmac_tx_dscr_queue_add_dscr_list(
                hal_to_dmac_device_stru *       pst_hal_device,
                oal_uint8                       uc_qid,
                oal_dlist_head_stru            *pst_tx_dscr_list_hdr,
                oal_uint8                       uc_ppdu_cnt)
{
    /* 将指定的发送描述符链表加入发送描述符队列 */
    oal_dlist_join_tail(&(pst_hal_device->ast_tx_dscr_queue[uc_qid].st_header), pst_tx_dscr_list_hdr);
    pst_hal_device->ast_tx_dscr_queue[uc_qid].uc_ppdu_cnt += uc_ppdu_cnt;

    return OAL_SUCC;
}


#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT

OAL_STATIC oal_void dmac_tx_data_flush_cache(mac_tx_ctl_stru *pst_tx_ctl, oal_dlist_head_stru *pst_tx_dscr_list_hdr)
{
    oal_dlist_head_stru     *pst_dlist_node;
    oal_netbuf_stru         *pst_netbuf = OAL_PTR_NULL;
    hal_tx_dscr_stru        *pst_dscr   = OAL_PTR_NULL;

    oal_uint16               us_frame_len = 0;     /* 帧头加帧体的长度 */
    oal_uint8               *puc_frame_head;       /* 帧头指针 */
     oal_uint8              *puc_frame_data;       /* 帧体指针 */
    oal_uint8                uc_frame_head_len = 0;

    oal_uint8                              uc_msdu_num_in_amsdu;
    oal_uint8                              uc_idx;
    oal_uint8                              uc_msdu_num;
    oal_uint8                              uc_max_msdu;
    oal_uint16                             us_msdu_length;
    oal_uint8                             *puc_msdu_data;
    hal_tx_msdu_address_params            *pst_tx_dscr_msdu_subtable;
    /* 将数据帧同步到Memory */
    pst_dlist_node = pst_tx_dscr_list_hdr->pst_next;
    while (pst_dlist_node != pst_tx_dscr_list_hdr)
    {
        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dlist_node, hal_tx_dscr_stru, st_entry);
        pst_netbuf = pst_dscr->pst_skb_start_addr;

        if(OAL_PTR_NULL != pst_netbuf)
        {
            pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
            us_frame_len = (oal_uint16)MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl) + (oal_uint16)oal_netbuf_get_len(pst_netbuf);
            #if 0
            if(1 == pst_tx_ctl->bit_80211_mac_head_type)  /* 表示mac header和data在一起 */
            {
                oal_dma_map_single(NULL, OAL_NETBUF_DATA(pst_netbuf), us_frame_len, OAL_TO_DEVICE);
            }
            else  /* MAC头和data指针不连续场景 */
            #endif
            /* Hi1151 E5场景，无论bit_80211_mac_head_type值是什么类型，mac header和data指针都不在一起，所以需要header和data分别刷新*/
            {
                puc_frame_head = (oal_uint8 *)(MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl));
                uc_frame_head_len = MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl);
                oal_dma_map_single(NULL, puc_frame_head, uc_frame_head_len, OAL_TO_DEVICE);   /* MAC头 */

                if(1 == MAC_GET_CB_NETBUF_NUM(pst_tx_ctl)) /* 非AMSDU情况 */
                {
                    puc_frame_data = oal_netbuf_payload(pst_netbuf);
                    oal_dma_map_single(NULL, puc_frame_data, us_frame_len - uc_frame_head_len, OAL_TO_DEVICE);  /* payload */
                }
                else
                {
                    hal_get_tx_msdu_address_params(pst_dscr, &pst_tx_dscr_msdu_subtable, &uc_msdu_num_in_amsdu);

                    uc_msdu_num = 0;
                    uc_max_msdu = (uc_msdu_num_in_amsdu >> 1) + 1;

                    for (uc_idx = 0; uc_idx < uc_max_msdu; uc_idx++)
                    {
                        if (uc_msdu_num < uc_msdu_num_in_amsdu)
                        {
                            puc_msdu_data  = ((oal_uint8 *)OAL_PHY_TO_VIRT_ADDR(pst_tx_dscr_msdu_subtable->ul_msdu_addr0));
                            us_msdu_length = pst_tx_dscr_msdu_subtable->us_msdu0_len;
                            oal_dma_map_single(NULL, puc_msdu_data, us_msdu_length, OAL_TO_DEVICE);  /* payload */
                            uc_msdu_num++;
                        }

                        if (uc_msdu_num < uc_msdu_num_in_amsdu)
                        {
                            puc_msdu_data  = ((oal_uint8 *)OAL_PHY_TO_VIRT_ADDR(pst_tx_dscr_msdu_subtable->ul_msdu_addr1));
                            us_msdu_length = pst_tx_dscr_msdu_subtable->us_msdu1_len;
                            oal_dma_map_single(NULL, puc_msdu_data, us_msdu_length, OAL_TO_DEVICE);  /* payload */
                            uc_msdu_num++;
                        }
                        pst_tx_dscr_msdu_subtable++;
                    }
                }
            }
        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_data_flush_cache::pst_netbuf null.}");
        }

        pst_dlist_node = pst_dlist_node->pst_next;
    }
}
#endif  /* _PRE_WLAN_CACHE_COHERENT_SUPPORT */



#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
OAL_STATIC OAL_INLINE oal_uint32  dmac_tid_tx_queue_enqueue(
                mac_device_stru    *pst_device,
                mac_vap_stru       *pst_vap,
                dmac_tid_stru      *pst_tid_queue,
                oal_uint8           uc_mpdu_num,
                oal_netbuf_stru    *pst_netbuf)
#else
OAL_STATIC OAL_INLINE oal_uint32  dmac_tid_tx_queue_enqueue(
                mac_device_stru    *pst_device,
                mac_vap_stru       *pst_vap,
                dmac_tid_stru      *pst_tid_queue,
                hal_tx_dscr_stru   *pst_tx_dscr,
                oal_uint8           uc_mpdu_num,
                oal_netbuf_stru    *pst_netbuf)
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    oal_time_us_stru            st_timestamp_us = {0, 0};
#else
    oal_uint32                  ul_timestamp = 0;
#endif
    oal_netbuf_stru            *pst_netbuf_tmp;
    mac_tx_ctl_stru            *pst_cb;
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8   en_trace_pkt_type;
#endif
#ifdef _PRE_WLAN_FEATURE_DBAC
    oal_uint32                  ul_ret;
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_DFT_STAT
    dmac_tid_stats_stru   *pst_tid_stats;
    pst_tid_stats = pst_tid_queue->pst_tid_stats;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tid_stats))
    {
        OAM_ERROR_LOG4(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue::tid_stats is null. tid:%d,is paused:%d,mpdu_num:%d,user idx:%d}",
                       pst_tid_queue->uc_tid,
                       pst_tid_queue->uc_is_paused,
                       pst_tid_queue->us_mpdu_num,
                       pst_tid_queue->us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
#endif

    /* 更新device结构体下的统计信息 */
    if (OAL_UNLIKELY((pst_device->us_total_mpdu_num + 1) > WLAN_TID_MPDU_NUM_LIMIT))
    {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue::us_total_mpdu_num exceed[%d].}",pst_device->us_total_mpdu_num);
        DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_enqueue_full_cnt, uc_mpdu_num);

        return OAL_FAIL;
    }

    pst_netbuf_tmp = pst_netbuf;
    while (OAL_PTR_NULL != pst_netbuf_tmp)
    {
        pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf_tmp);
#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf_tmp), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue::type%d into tid queue[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif

#ifdef _PRE_WLAN_FEATURE_PF_SCH
        /* 标记该帧是由tid队列调度发送的，供调度算法使用*/
        MAC_GET_CB_ALG_TAGS(pst_cb) |= DMAC_CB_ALG_TAGS_TIDSCH_MASK;
#endif
        /* VO/VI 队列需要打时间戳，算法需要*/
        if ((WLAN_WME_AC_VI == MAC_GET_CB_WME_AC_TYPE(pst_cb)) || (WLAN_WME_AC_VO == MAC_GET_CB_WME_AC_TYPE(pst_cb)))
        {
            /*只需要获取一次时间戳，所有的netbuf使用同一个时间戳*/
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
            if ((0 == st_timestamp_us.i_sec) && (0 ==st_timestamp_us.i_usec))
            {
                dmac_timestamp_get(&st_timestamp_us);
            }

            MAC_GET_CB_TIMESTAMP(pst_cb) = (oal_uint32)DMAC_TIME_USEC_INT64(&st_timestamp_us);
#else
            /* us级别, 降低mips处理 */
            if (0 == ul_timestamp)
            {
                ul_timestamp = (oal_uint32)(OAL_TIME_GET_STAMP_MS() * 1000);
            }

            MAC_GET_CB_TIMESTAMP(pst_cb) = ul_timestamp;
#endif
        }
        pst_netbuf_tmp = oal_get_netbuf_next(pst_netbuf_tmp);
    }

    dmac_tid_tx_enqueue_update(pst_device, pst_tid_queue, uc_mpdu_num);

    DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_enqueue_total_cnt, uc_mpdu_num);

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    dmac_alg_flowctl_backp_notify(pst_vap, pst_device->us_total_mpdu_num, pst_device->aus_ac_mpdu_num);
#endif

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    dmac_tx_queue_mpdu(pst_netbuf, &pst_tid_queue->st_buff_head);
#else
    OAL_MEM_TRACE(pst_tx_dscr, OAL_FALSE);
    oal_dlist_add_tail(&pst_tx_dscr->st_entry, &pst_tid_queue->st_hdr);
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

#ifdef _PRE_WLAN_FEATURE_DBAC
    ul_ret = dmac_alg_enqueue_tid_notify(pst_vap, pst_tid_queue, uc_mpdu_num);
    if (OAL_UNLIKELY(OAL_SUCC !=  ul_ret))
    {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue::dmac_alg_enqueue_tid_notify failed[%d].}", ul_ret);
        return ul_ret;
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  dmac_tx_is_tx_force (dmac_vap_stru *pst_dmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    /* 常发下返回状态OK */
    if((OAL_SWITCH_ON == pst_dmac_vap->st_vap_base_info.bit_al_tx_flag))
    {
        return OAL_TRUE;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 漫游状态时，强制发送该数据帧 */
    if (MAC_VAP_STATE_ROAMING == pst_dmac_vap->st_vap_base_info.en_vap_state)
    {
        return OAL_TRUE;
    }
#endif

    return OAL_FALSE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  dmac_is_vap_state_ok (dmac_vap_stru *pst_dmac_vap)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (OAL_UNLIKELY(!((MAC_VAP_STATE_UP == pst_dmac_vap->st_vap_base_info.en_vap_state) ||
    (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state))))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
        "{dmac_tx_process_data_event:vap state[%d] != MAC_VAP_STATE_{UP|PAUSE}!}\r\n",
        pst_dmac_vap->st_vap_base_info.en_vap_state);
        return OAL_FALSE;
    }
#endif
    return OAL_TRUE;

}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  dmac_is_user_state_ok (dmac_user_stru *pst_dmac_user)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if(MAC_USER_STATE_BUTT == pst_dmac_user->st_user_base_info.en_user_asoc_state)
    {
        /*data frames stored in the hmac/dmac pipeline when dmac offload*/
        OAM_WARNING_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_TX,
                         "{dmac_is_user_state_ok::user(id[%d]) state is MAC_USER_STATE_BUTT! ", pst_dmac_user->st_user_base_info.us_assoc_id);
        return OAL_FALSE;
    }
#endif
    return OAL_TRUE;
}


OAL_STATIC OAL_INLINE oal_bool_enum_uint8  dmac_tx_need_enqueue_tid(
                                                   mac_device_stru *pst_mac_device,
                                                   dmac_vap_stru   *pst_dmac_vap,
                                                   dmac_tid_stru   *pst_tid_queue,
                                                   oal_uint8        uc_q_num)
{
    hal_to_dmac_device_stru *pst_hal_device;

    pst_hal_device = pst_dmac_vap->pst_hal_device;

    /* 强制发送帧不入队列 */
    if(OAL_TRUE == dmac_tx_is_tx_force(pst_dmac_vap))
    {
        return OAL_FALSE;
    }

    /* 非WMM AC不入队列 */
    if (HAL_TX_QUEUE_VO < uc_q_num)
    {
        return OAL_FALSE;
    }

    /* (1)TX mode为聚合
       (2)TID被暂停
       (3)如果硬件队列有包
       (4)若VAP暂停则需要入队
       (5)对应的TID缓存队列不为空 */
    if ((DMAC_TX_MODE_NORMAL != pst_tid_queue->en_tx_mode) ||
        (pst_tid_queue->uc_is_paused > 0) ||
        (0 != pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt) ||
        (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state)||
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        (OAL_FALSE == oal_dlist_is_empty(&pst_tid_queue->st_retry_q)) ||
        (OAL_FALSE == oal_netbuf_list_empty(&pst_tid_queue->st_buff_head)))
#else
        (OAL_FALSE == oal_dlist_is_empty(&pst_tid_queue->st_hdr)))
#endif
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

hal_tx_dscr_stru* dmac_tx_dscr_alloc(hal_to_dmac_device_stru   *pst_hal_device,
                                                        dmac_vap_stru *pst_dmac_vap,
                                                      oal_netbuf_stru *pst_netbuf)
{
    oal_uint32                 ul_ret;
    mac_tx_ctl_stru           *pst_tx_ctl_first;
    hal_tx_mpdu_stru           st_mpdu;
    oal_uint16                 us_tx_dscr_len;
    hal_tx_dscr_stru          *pst_tx_dscr;
    mac_device_stru           *pst_mac_device;
    oal_uint8                  uc_ac;
    oal_uint8                  uc_q_num;
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_mem_pool_id_enum_uint8 en_mem_poo_id;
#endif

    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_dscr_alloc::tx pkt is dropped, pst_netbuf null.}");
        return OAL_PTR_NULL;
    }
    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_dscr_alloc::tx pkt is dropped, pst_mac_device null.}");
        return OAL_PTR_NULL;
    }

    pst_tx_ctl_first  = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    if(OAL_PTR_NULL == pst_tx_ctl_first)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_dscr_alloc::tx pkt is dropped, tx ctl null.}");
        return OAL_PTR_NULL;
    }

    uc_ac       = MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl_first);
    uc_q_num    = HAL_AC_TO_Q_NUM(uc_ac);

    /* 获取MPDU发送参数 */
    ul_ret = dmac_tx_get_mpdu_params(pst_netbuf, pst_tx_ctl_first, &st_mpdu);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_dscr_alloc::tx pkt is dropped, dmac_tx_get_mpdu_params failed[%d].", ul_ret);
        OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_mpdu_dropped, 1);

        return OAL_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 设置为漫游数据，标志以便发送完成时算法不进行统计 */
    if (MAC_VAP_STATE_ROAMING == pst_dmac_vap->st_vap_base_info.en_vap_state)
    {
        MAC_GET_CB_IS_ROAM_DATA(pst_tx_ctl_first) = OAL_TRUE;
    }
    else
    {
        MAC_GET_CB_IS_ROAM_DATA(pst_tx_ctl_first) = OAL_FALSE;
    }
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    if (MAC_GET_CB_NETBUF_NUM(pst_tx_ctl_first) == 1)
    {
        us_tx_dscr_len = WLAN_MEM_SHARED_TX_DSCR_SIZE1;
        en_mem_poo_id = OAL_MEM_POOL_ID_TX_DSCR_1;
    }
    else
    {
        us_tx_dscr_len = WLAN_MEM_SHARED_TX_DSCR_SIZE2;
        en_mem_poo_id = OAL_MEM_POOL_ID_TX_DSCR_2;
    }

    /* 申请发送描述符内存 */
    pst_tx_dscr = (hal_tx_dscr_stru *)OAL_MEM_ALLOC(en_mem_poo_id, us_tx_dscr_len, OAL_FALSE);
#else
    us_tx_dscr_len = (MAC_GET_CB_NETBUF_NUM(pst_tx_ctl_first) == 1) ? WLAN_MEM_SHARED_TX_DSCR_SIZE1 : WLAN_MEM_SHARED_TX_DSCR_SIZE2;
    pst_tx_dscr = (hal_tx_dscr_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_SHARED_DSCR, us_tx_dscr_len, OAL_FALSE);
#endif
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tx_dscr))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_dscr_alloc::tx pkt is dropped, pst_tx_dscr alloc failed.}");

        OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_mpdu_dropped, 1);
#ifdef _PRE_WLAN_DFT_STAT
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        dmac_dft_print_mac_phy_rf(pst_hal_device);
#endif
#endif
        return OAL_PTR_NULL;
    }

    OAL_MEM_TRACE(pst_tx_dscr, OAL_FALSE);
    OAL_MEMZERO(pst_tx_dscr, us_tx_dscr_len);

    /* 填写描述符ac */
    pst_tx_dscr->uc_q_num = uc_q_num;

    /* 填写mpdu长度 包括帧头 */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    if (OAL_PTR_NULL == oal_get_netbuf_next(pst_netbuf))
    {
        pst_tx_dscr->us_original_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl_first) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl_first);
    }
    else
    {
        pst_tx_dscr->us_original_mpdu_len = st_mpdu.st_mpdu_mac_hdr.uc_mac_hdr_len + st_mpdu.ast_msdu_addr[0].us_msdu0_len + st_mpdu.ast_msdu_addr[0].us_msdu1_len;
    }
#else
    pst_tx_dscr->us_original_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl_first) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl_first);
#endif

    hal_tx_fill_basic_ctrl_dscr(pst_hal_device, pst_tx_dscr, &st_mpdu);

    return pst_tx_dscr;
}
#ifdef _PRE_WLAN_FEATURE_VOWIFI

OAL_STATIC oal_void dmac_vowifi_update_arp_timestamp(dmac_vap_stru *pst_dmac_vap, oal_uint16 us_idx)
{
    dmac_user_stru *pst_dmac_user;

    if ((!IS_STA(&pst_dmac_vap->st_vap_base_info)) || (!IS_LEGACY_VAP(&pst_dmac_vap->st_vap_base_info)))
    {
        return ;
    }

    if (OAL_PTR_NULL == pst_dmac_vap->pst_vowifi_status)
    {
        return ;
    }

    /* 非业务用户的帧 */
    if (us_idx != pst_dmac_vap->st_vap_base_info.us_assoc_vap_id)
    {
        return ;
    }

    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(us_idx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_VOWIFI,
                    "{dmac_vowifi_update_arp_timestamp::pst_dmac_user[%d] null!}", us_idx);
        return;
    }

    pst_dmac_vap->pst_vowifi_status->ull_arp_timestamp_ms = OAL_TIME_GET_STAMP_MS();
    pst_dmac_vap->pst_vowifi_status->ul_arp_rx_succ_pkts  = (pst_dmac_user->st_query_stats.ul_drv_rx_pkts >= pst_dmac_user->st_query_stats.ul_rx_dropped_misc)?
                                                     (pst_dmac_user->st_query_stats.ul_drv_rx_pkts - pst_dmac_user->st_query_stats.ul_rx_dropped_misc):
                                                     0;
}
#endif /* _PRE_WLAN_FEATURE_VOWIFI */


oal_uint32  dmac_tx_process_data_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru            *pst_event;
    frw_event_hdr_stru        *pst_event_hdr;
    hal_to_dmac_device_stru   *pst_hal_device;
    dmac_vap_stru             *pst_dmac_vap;
    dmac_tx_event_stru        *pst_tx_event;
    oal_netbuf_stru           *pst_netbuf;
    mac_tx_ctl_stru           *pst_tx_ctl;
    oal_netbuf_stru           *pst_netbuf_next;
    oal_uint32                 ul_ret;
    oal_uint8                  uc_data_type;

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_DMAC_START);
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_DMAC_TX_START);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_process_data_event::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event     = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &pst_event->st_event_hdr;
    pst_tx_event  = (dmac_tx_event_stru *)pst_event->auc_event_data;

    /* 获取vap结构信息 */
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data_event::mac_res_get_dmac_vap fail, null ptr.}");
        dmac_tx_excp_free_netbuf(pst_tx_event->pst_netbuf);
        OAM_STAT_VAP_INCR(pst_event_hdr->uc_vap_id, tx_abnormal_mpdu_dropped, 1);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data_event::pst_hal_device null.}");
        dmac_tx_excp_free_netbuf(pst_tx_event->pst_netbuf);
        OAM_STAT_VAP_INCR(pst_event_hdr->uc_vap_id, tx_abnormal_mpdu_dropped, 1);
        return OAL_SUCC;
    }

    /* 发送pps统计 */
    dmac_auto_freq_pps_count(1);

    pst_netbuf = pst_tx_event->pst_netbuf;

    /* 循环处理分片报文 */
    while (pst_netbuf != OAL_PTR_NULL)
    {
        pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        /* 排除amsdu的情况amsdu不进行循环，分片报文每个报文的next必须为NULL */
        if (OAL_TRUE != MAC_GET_CB_IS_AMSDU(pst_tx_ctl))
        {
            pst_netbuf_next = oal_get_netbuf_next(pst_netbuf);
            oal_set_netbuf_next(pst_netbuf, OAL_PTR_NULL);
        }
        else
        {
            pst_netbuf_next = OAL_PTR_NULL;
        }

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        /* 如果是AMSDU帧类型,需要去掉SKB起始的ETHER HEAD */
        if (OAL_TRUE == MAC_GET_CB_HAS_EHTER_HEAD(pst_tx_ctl))
        {
            MAC_GET_CB_MPDU_LEN(pst_tx_ctl) -= (ETHER_HDR_LEN+2);
        }
#endif

        /* 维测，输出一个关键帧打印 */
        if(MAC_GET_CB_IS_VIPFRAME(pst_tx_ctl))
        {
            uc_data_type = MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            if(MAC_DATA_ARP_REQ >= uc_data_type)
#else
            if(MAC_DATA_EAPOL >= uc_data_type)
#endif
            {
                OAM_WARNING_LOG3(pst_event_hdr->uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data_event::datatype==%u, qos==%u, usridx=%d}[0:dhcp 1:eapol 2:arp_rsp 3:arp_req]",
                    uc_data_type , MAC_GET_CB_IS_QOS_DATA(pst_tx_ctl), MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
                dmac_tx_pause_info(pst_hal_device, pst_dmac_vap, pst_netbuf);
            }

#ifdef _PRE_WLAN_FEATURE_VOWIFI
            dmac_vowifi_update_arp_timestamp(pst_dmac_vap, MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
#endif
        }

        ul_ret = dmac_tx_process_data(pst_hal_device, pst_dmac_vap, pst_netbuf);
        if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
        {
            //OAM_INFO_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data_event::dmac_tx_process_data fail[%u].}", ul_ret);
            dmac_tx_excp_free_netbuf(pst_netbuf);
        }

        pst_netbuf = pst_netbuf_next;
    }

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_DMAC_END);

    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rodata, code, rwdata, zidata  // return to default placement
#endif


oal_bool_enum_uint8 dmac_is_need_enqueue_fake(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device, mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_netbuf)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_ieee80211_frame_stru       *pst_mac_header;
    oal_uint8                      *puc_sa = OAL_PTR_NULL;
#endif

    /* DBAC/DBDC运行时判断管理帧是否需要缓存 */
    if ((mac_is_dbac_running(pst_mac_device)) || (mac_is_dbdc_running(pst_mac_device)))
    {
        if (pst_hal_device->uc_current_chan_number != pst_mac_vap->st_channel.uc_chan_number)
        {
            return OAL_TRUE;
        }

#ifdef _PRE_WLAN_FEATURE_DBAC
        if (dmac_alg_dbac_is_pause(pst_hal_device))
        {
        #ifdef _PRE_WLAN_FEATURE_P2P
            if (MAC_SCAN_FUNC_P2P_LISTEN == pst_mac_device->st_scan_params.uc_scan_func)
            {
                pst_mac_header  = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf);
                mac_rx_get_sa(pst_mac_header, &puc_sa);

                if (!oal_compare_mac_addr(puc_sa, mac_mib_get_p2p0_dot11StationID(pst_mac_vap)))
                {
                    return OAL_FALSE;
                }
            }
        #endif

            return OAL_TRUE;
        }
#endif
    }

    /* 漫游期间，强制发送该数据帧, 需在dbac之后判断 */
#ifdef _PRE_WLAN_FEATURE_ROAM
    if (MAC_VAP_STATE_ROAMING == pst_mac_vap->en_vap_state)
    {
        return OAL_FALSE;
    }
#endif

    /* 扫描期间的管理帧是否需要缓存 */
    if (MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state &&
        MAC_SCAN_FUNC_P2P_LISTEN != pst_mac_device->st_scan_params.uc_scan_func &&
        WLAN_SCAN_MODE_FOREGROUND != pst_mac_device->st_scan_params.en_scan_mode &&
        WLAN_SCAN_MODE_BACKGROUND_PNO != pst_mac_device->st_scan_params.en_scan_mode)
    {
        /* 在背景扫描时,中间会切回工作信道,此时管理帧不需要缓存,必须是pause的vap,防止进了p2p device的虚假队列出不来 */
        if (MAC_VAP_STATE_PAUSE == pst_mac_vap->en_vap_state)
        {
            return OAL_TRUE;
        }

        return OAL_FALSE;
    }

    /* 非上述场景p2p go是否需要缓存 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if ((WLAN_P2P_GO_MODE == pst_mac_vap->en_p2p_mode) &&(pst_hal_device->uc_current_chan_number != pst_mac_vap->st_channel.uc_chan_number)
            && (MAC_VAP_STATE_PAUSE == pst_mac_vap->en_vap_state))
    {
        return OAL_TRUE;
    }
#endif


    if (MAC_VAP_STATE_PAUSE == pst_mac_vap->en_vap_state)
    {
#ifdef  _PRE_WLAN_FEATURE_M2S
        if (OAL_TRUE == pst_hal_device->en_m2s_excute_flag)
        {
            return OAL_TRUE;
        }
#endif /* #ifdef _PRE_WLAN_FEATURE_M2S */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        if (HAL_BTCOEX_SW_POWSAVE_WORK == GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_is_need_enqueue_fake::BT ps is working.}");
            return OAL_TRUE;
        }
#endif /* _PRE_WLAN_FEATURE_BTCOEX */
    }

    return OAL_FALSE;
}

oal_void  dmac_tx_restore_tx_queue(hal_to_dmac_device_stru     *pst_hal_device,
                                                hal_tx_dscr_queue_header_stru *pst_fake_queue)
{
    oal_uint8                        uc_q_idx;
    oal_dlist_head_stru             *pst_dscr_entry;
    oal_dlist_head_stru              st_dscr_head;
    hal_tx_dscr_stru                *pst_tx_next_dscr;
    oal_dlist_head_stru             *pst_dscr_next_entry;
    mac_tx_ctl_stru                 *pst_tx_current_ctl;
    hal_tx_dscr_stru                *pst_tx_current_dscr;

    if (OAL_PTR_NULL == pst_fake_queue)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_restore_tx_queue::pst_fake_queue null.}");
        return;
    }

    for (uc_q_idx = 0; uc_q_idx < HAL_TX_QUEUE_BUTT; uc_q_idx++)
    {
        /* 从fake队列轮个插入硬件队列队尾，put硬件发送寄存器 */
        if (OAL_TRUE == oal_dlist_is_empty(&(pst_fake_queue[uc_q_idx].st_header)))
        {
            continue;
        }

        if (pst_hal_device->ast_tx_dscr_queue[uc_q_idx].uc_ppdu_cnt != 0)
        {
            OAM_WARNING_LOG2(0, OAM_SF_TX, "{dmac_tx_restore_tx_queue::q[%d], ppdu[%d]!=0.}", uc_q_idx, pst_hal_device->ast_tx_dscr_queue[uc_q_idx].uc_ppdu_cnt);
        }

        oal_dlist_init_head(&st_dscr_head);
        pst_dscr_next_entry = pst_fake_queue[uc_q_idx].st_header.pst_next;
        while(pst_dscr_next_entry != (&pst_fake_queue[uc_q_idx].st_header))
        {
            /* 从fake队列单独拎出来一个包 */
            pst_dscr_entry      = oal_dlist_delete_head(&(pst_fake_queue[uc_q_idx].st_header));
            pst_tx_current_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
            pst_tx_current_ctl  = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_tx_current_dscr->pst_skb_start_addr);

            pst_dscr_next_entry = pst_fake_queue[uc_q_idx].st_header.pst_next;

            if (OAL_TRUE == pst_tx_current_dscr->bit_is_first)
            {
                oal_dlist_add_tail(pst_dscr_entry, &st_dscr_head);
                if (OAL_FALSE == pst_tx_current_dscr->bit_is_ampdu)
                {
                    /* 最后一个包的pst next dscr指针需要为空 */
                    hal_tx_ctrl_dscr_unlink(pst_hal_device, pst_tx_current_dscr);

                    /* 单个mpdu包，直接发送 */
                    dmac_tx_hw_send(pst_hal_device, pst_tx_current_dscr, pst_tx_current_ctl, &st_dscr_head, 1);
                    oal_dlist_init_head(&st_dscr_head);
                    continue;
                }
                /* ampdu第一个包暂存在st_dscr_head中，等待后续ampdu剩余包全部提取完成一起挂硬件队列 */
            }
            else
            {
                /* ampdu非第一个的包继续添加在st_dscr_head中缓存 */
                oal_dlist_add_tail(pst_dscr_entry, &st_dscr_head);
            }

            if (OAL_TRUE == oal_dlist_is_empty(&st_dscr_head))
            {
                continue;
            }

            /* 检查ampdu包是否组装完整 */
            if (OAL_TRUE == oal_dlist_is_empty(&pst_fake_queue[uc_q_idx].st_header))
            {
                /* fake队列所有包处理完毕，清理环境 */
                pst_tx_current_dscr = OAL_DLIST_GET_ENTRY(st_dscr_head.pst_next, hal_tx_dscr_stru, st_entry);
                pst_tx_current_ctl  = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_tx_current_dscr->pst_skb_start_addr);
                dmac_tx_hw_send(pst_hal_device, pst_tx_current_dscr, pst_tx_current_ctl, &st_dscr_head, 1);
                oal_dlist_init_head(&st_dscr_head);
                continue;
            }
            else
            {
                pst_tx_next_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_next_entry, hal_tx_dscr_stru, st_entry);
                if (OAL_TRUE == pst_tx_next_dscr->bit_is_first)
                {
                    /* 最后一个包的pst next dscr指针需要为空 */
                    hal_tx_ctrl_dscr_unlink(pst_hal_device, pst_tx_current_dscr);

                    /* 此ampdu包已获取完毕，可以将完整的ampdu包挂入硬件队列 */
                    pst_tx_current_dscr = OAL_DLIST_GET_ENTRY(st_dscr_head.pst_next, hal_tx_dscr_stru, st_entry);
                    pst_tx_current_ctl  = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_tx_current_dscr->pst_skb_start_addr);
                    dmac_tx_hw_send(pst_hal_device, pst_tx_current_dscr, pst_tx_current_ctl, &st_dscr_head, 1);
                    oal_dlist_init_head(&st_dscr_head);
                    continue;
                }
            }

        }

        oal_dlist_init_head(&pst_fake_queue[uc_q_idx].st_header);
        pst_fake_queue[uc_q_idx].uc_ppdu_cnt = 0;

    }
}

OAL_STATIC oal_bool_enum_uint8 dmac_tx_mgmt_buffer_proc(
                mac_device_stru     *pst_mac_device,
                dmac_vap_stru       *pst_dmac_vap,
                hal_tx_dscr_stru    *pst_mgmt_dscr,
                oal_netbuf_stru     *pst_netbuf_mgmt)
{
    hal_tx_dscr_queue_header_stru  *pst_tx_dscr_queue_fake;
    hal_to_dmac_device_stru        *pst_hal_device = pst_dmac_vap->pst_hal_device;
    mac_tx_ctl_stru                *pst_tx_ctl;

    if (dmac_is_need_enqueue_fake(pst_mac_device, pst_hal_device, &(pst_dmac_vap->st_vap_base_info), pst_netbuf_mgmt))
    {
        pst_tx_dscr_queue_fake = DMAC_VAP_GET_FAKEQ(pst_dmac_vap);
        pst_tx_ctl             = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf_mgmt);

        /* opmode和smps帧需要放到虚假队列的头部 */
        if (MAC_GET_CB_IS_OPMODE_FRAME(pst_tx_ctl) || MAC_GET_CB_IS_SMPS_FRAME(pst_tx_ctl))
        {
            oal_dlist_add_head(&pst_mgmt_dscr->st_entry, &(pst_tx_dscr_queue_fake[pst_mgmt_dscr->uc_q_num].st_header));
        }
        else
        {
            oal_dlist_add_tail(&pst_mgmt_dscr->st_entry, &(pst_tx_dscr_queue_fake[pst_mgmt_dscr->uc_q_num].st_header));
        }

        pst_tx_dscr_queue_fake[pst_mgmt_dscr->uc_q_num].uc_ppdu_cnt++;

        OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_DBAC,
                            "dmac_tx_mgmt_buffer_proc::buffered into fakeq. hal device chan num[%d],vap chan num[%d],fake q vap_id:%d",
                            pst_hal_device->uc_current_chan_number, pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number, pst_dmac_vap->st_vap_base_info.uc_vap_id);

        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC oal_bool_enum_uint8 dmac_tx_mgmt_buff_process(
                                            mac_device_stru     *pst_mac_device,
                                            dmac_vap_stru       *pst_dmac_vap,
                                            hal_tx_dscr_stru    *pst_mgmt_dscr,
                                            oal_netbuf_stru     *pst_netbuf_mgmt)
{
    mac_ieee80211_frame_stru       *pst_mac_header;
    oal_uint8                       uc_mgmt_subtype;
    oal_uint8                       uc_mgmt_type;
#if defined(_PRE_PRODUCT_ID_HI110X_DEV) && defined(_PRE_WLAN_FEATURE_DBAC)
    mac_vap_stru                   *pst_mac_vap1;           /* 已经UP的1个VAP */
    mac_vap_stru                   *pst_mac_vap2;           /* 已经UP的1个VAP */
    mac_vap_stru                   *pst_current_chan_vap;   /* 当前信道上的VAP */
    mac_fcs_mgr_stru               *pst_fcs_mgr;
    mac_fcs_cfg_stru               *pst_fcs_cfg;
#endif

    pst_mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf_mgmt);
    uc_mgmt_type    = mac_frame_get_type_value((oal_uint8 *)pst_mac_header);
    uc_mgmt_subtype = mac_frame_get_subtype_value((oal_uint8 *)pst_mac_header);

    if (WLAN_MANAGEMENT == uc_mgmt_type && WLAN_PROBE_REQ == uc_mgmt_subtype)
    {
        /* probe req在任何情况下都不需要缓存 */
        return OAL_FALSE;
    }

#if defined(_PRE_PRODUCT_ID_HI110X_DEV) && defined(_PRE_WLAN_DFT_STAT)
    if (WLAN_MANAGEMENT == uc_mgmt_type && WLAN_DISASOC == uc_mgmt_subtype)
    {
        dmac_dft_report_all_ota_state(&(pst_dmac_vap->st_vap_base_info));
    }
#endif

    /* 1102 sta+p2p共存引入的优化，1151 DBAC不需要 */
#if defined(_PRE_PRODUCT_ID_HI110X_DEV) && defined(_PRE_WLAN_FEATURE_DBAC)
    if ((WLAN_MANAGEMENT == uc_mgmt_type && WLAN_DISASOC == uc_mgmt_subtype)
         && mac_is_dbac_running(pst_mac_device)
         && (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state))
    {
        /* 暂停两个VAP的发送 */
        if (OAL_SUCC == mac_device_find_2up_vap(pst_mac_device, &pst_mac_vap1, &pst_mac_vap2))
        {
            pst_fcs_mgr = dmac_fcs_get_mgr_stru(pst_mac_device);
            pst_fcs_cfg = MAC_DEV_GET_FCS_CFG(pst_mac_device);
            OAL_MEMZERO(pst_fcs_cfg, OAL_SIZEOF(mac_fcs_cfg_stru));
            dmac_vap_pause_tx(pst_mac_vap1);
            dmac_vap_pause_tx(pst_mac_vap2);
            /* 暂停DBAC切信道 */
            dmac_alg_dbac_pause(pst_dmac_vap->pst_hal_device);
            if (pst_dmac_vap->pst_hal_device->uc_current_chan_number == pst_mac_vap1->st_channel.uc_chan_number)
            {
                pst_current_chan_vap = pst_mac_vap1;
            }
            else
            {
                pst_current_chan_vap = pst_mac_vap2;
            }

            pst_fcs_cfg->st_src_chl = pst_current_chan_vap->st_channel;
            pst_fcs_cfg->st_dst_chl = pst_dmac_vap->st_vap_base_info.st_channel;
            pst_fcs_cfg->pst_hal_device = pst_dmac_vap->pst_hal_device;

            pst_fcs_cfg->pst_src_fake_queue = DMAC_VAP_GET_FAKEQ(pst_current_chan_vap);
            dmac_fcs_prepare_one_packet_cfg(pst_current_chan_vap, &(pst_fcs_cfg->st_one_packet_cfg), 20);
            dmac_fcs_start(pst_fcs_mgr, pst_fcs_cfg, 0);
            mac_fcs_release(pst_fcs_mgr);
        }
        /* DBAC场景下，去关联帧不需要缓存 */
        return OAL_FALSE;
    }
#endif

    return dmac_tx_mgmt_buffer_proc(pst_mac_device, pst_dmac_vap, pst_mgmt_dscr, pst_netbuf_mgmt);
}


/*lint -save -e438 */
OAL_STATIC oal_uint32  dmac_tx_rifs_process(hal_to_dmac_device_stru *pst_hal_dev,
                                      dmac_vap_stru *pst_dmac_vap,
                                      dmac_tid_stru *pst_tid_queue,
                                      hal_tx_ppdu_feature_stru *pst_ppdu_feature,
                                      oal_dlist_head_stru *pst_dscr_list)
{
    hal_tx_dscr_stru    *pst_first_dscr;
    hal_tx_dscr_stru    *pst_last_dscr;
    hal_tx_dscr_stru    *pst_bar_dscr;
    oal_netbuf_stru     *pst_bar_buf;
    dmac_ba_tx_stru     *pst_tx_ba_handle;
    mac_tx_ctl_stru     *pst_tx_ctl;
    oal_uint16           us_bar_len;
    hal_tx_mpdu_stru     st_mpdu;
    oal_dlist_head_stru *pst_dscr_entry;
    hal_tx_dscr_stru    *pst_dscr;
    mac_ieee80211_qos_frame_addr4_stru *pst_4addr_hdr;
    mac_ieee80211_qos_frame_stru       *pst_hdr;


    pst_tx_ba_handle = pst_tid_queue->pst_ba_tx_hdl;

    /* 申请bar描述符 */
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_bar_dscr = (hal_tx_dscr_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_TX_DSCR_1, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAL_TRUE);
#else
    pst_bar_dscr = (hal_tx_dscr_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_SHARED_DSCR, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAL_TRUE);
#endif
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_bar_dscr))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_rifs_process::pst_bar_dscr null.}");

        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    OAL_MEM_TRACE(pst_bar_dscr, OAL_FALSE);
    OAL_MEMZERO(pst_bar_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1);

    /* 申请bar帧体 */
    pst_bar_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    OAL_MEM_NETBUF_TRACE(pst_bar_buf, OAL_FALSE);
    if (OAL_PTR_NULL == pst_bar_buf)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_rifs_process::alloc netbuff failed.}");

        OAL_MEM_FREE(pst_bar_dscr, OAL_TRUE);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_bar_buf, OAL_TRUE);

    /* 组bar帧 */
    us_bar_len = dmac_ba_encap_blockack_req(pst_dmac_vap, pst_bar_buf, pst_tx_ba_handle, pst_tid_queue->uc_tid);
    if (0 == us_bar_len)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_rifs_process::us_bar_len=0.}");
        OAL_MEM_FREE(pst_bar_dscr, OAL_TRUE);
        oal_netbuf_free(pst_bar_buf);

        return OAL_FAIL;
    }

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_bar_buf);
    OAL_MEMZERO(pst_tx_ctl, OAL_NETBUF_CB_SIZE());

    /* init mpdu basic para */
    dmac_get_mgmt_mpdu_param(pst_bar_buf, pst_tx_ctl, us_bar_len, &st_mpdu);

    MAC_GET_CB_EVENT_TYPE(pst_tx_ctl) = FRW_EVENT_TYPE_WLAN_CTX;
    MAC_SET_CB_IS_BAR(pst_tx_ctl, OAL_TRUE);
    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl) = MAC_80211_CTL_HEADER_LEN;
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl)            = us_bar_len;

    oal_set_netbuf_prev(pst_bar_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_bar_buf, OAL_PTR_NULL);

    /* 填写bar描述符基本信息 */
    hal_tx_fill_basic_ctrl_dscr(pst_hal_dev, pst_bar_dscr, &st_mpdu);

    pst_last_dscr = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dscr_list->pst_prev, hal_tx_dscr_stru, st_entry);

    hal_tx_ctrl_dscr_link(pst_hal_dev, pst_last_dscr, pst_bar_dscr);
    hal_tx_ctrl_dscr_unlink(pst_hal_dev, pst_bar_dscr);

    /* 将bar串到rifs burst的最后 */
    oal_dlist_add_tail(&pst_bar_dscr->st_entry, pst_dscr_list);

    OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, pst_dscr_list)
    {
        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);

        pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_dscr->pst_skb_start_addr);

        /*lint -save -e774 */
        /*lint -save -e506 */
        if (OAL_TRUE == MAC_GET_CB_IS_4ADDRESS(pst_tx_ctl))
        {
            pst_4addr_hdr = (mac_ieee80211_qos_frame_addr4_stru *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
            pst_4addr_hdr->bit_qc_ack_polocy = WLAN_TX_NO_ACK;
        }
        else
        {
            pst_hdr = (mac_ieee80211_qos_frame_stru *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
            pst_hdr->bit_qc_ack_polocy = WLAN_TX_NO_ACK;
        }
        /*lint -restore */
        /*lint -restore */
    }

    pst_ppdu_feature->uc_ampdu_enable       = OAL_FALSE;
    pst_ppdu_feature->uc_rifs_enable        = OAL_TRUE;
    pst_ppdu_feature->ul_ampdu_length       = 0;
    pst_ppdu_feature->us_min_mpdu_length    = 0;

    pst_ppdu_feature->uc_mpdu_num++;

    pst_first_dscr = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dscr_list->pst_next, hal_tx_dscr_stru, st_entry);
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_first_dscr->pst_skb_start_addr);

    MAC_SET_CB_BAR_DSCR_ADDR(pst_tx_ctl, pst_bar_dscr);
    return OAL_SUCC;
}
/*lint -restore */



oal_void dmac_tx_save_tx_queue(hal_to_dmac_device_stru       *pst_hal_device,
                                            hal_tx_dscr_queue_header_stru *pst_fake_queue)
{
    oal_uint8            uc_q_id            = 0;
    oal_dlist_head_stru *pst_dlist_entry    = OAL_PTR_NULL;
    hal_tx_dscr_stru    *pst_tx_dscr        = OAL_PTR_NULL;
    hal_tx_dscr_stru    *pst_tx_dscr_tmp    = OAL_PTR_NULL;
    oal_uint8            uc_tx_status;
    mac_tx_ctl_stru     *pst_cb;
    oal_netbuf_stru     *pst_buf;


    /*
     * 遍历6个发送队列 一定要先处理非管理帧队列，防止普通优先级队列发送完成产生管理帧(BAR)入错队列
     * 保险起见，另启循环独立save。
     *
     * 此处有两种情形:
     *  1、发送完成事件已经入队列，若该AMPDU发送失败，则会在flush event时发送BAR，
     *     BAR直接入硬件队列(应为还未切信道)，而后被save
     *  2、save过程中，发现发送失败的AMPDU，强调irq loss，产生BAR，该BAR直接入硬件队列，
     *     而后被本函数最后的交换操作save，因此必须最后处理管理帧队列。
     */
    for (uc_q_id = 0; uc_q_id < HAL_TX_QUEUE_BUTT; uc_q_id++)
    {
        while(!oal_dlist_is_empty(&pst_hal_device->ast_tx_dscr_queue[uc_q_id].st_header))
        {
            pst_dlist_entry = pst_hal_device->ast_tx_dscr_queue[uc_q_id].st_header.pst_next;

            pst_tx_dscr     = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_entry, hal_tx_dscr_stru, st_entry);
            /*lint -save -e774 */
            if(OAL_PTR_NULL == pst_tx_dscr)
            {
                OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_save_tx_queue::pst_tx_dscr poiter is null.}");
                break;
            }
            /*lint -restore */
            hal_tx_get_dscr_status(pst_hal_device, pst_tx_dscr, &uc_tx_status);

            // 据王浩介绍，聚合帧中各子帧的status并不一定都是DMAC_TX_PENDING，因此需要各个处理
            // 另外，假如软件不清除某个单帧的status，那么硬件不会为之更新seq number
            if (DMAC_TX_PENDING == uc_tx_status)
            {
                pst_buf = pst_tx_dscr->pst_skb_start_addr;
                pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
                (MAC_GET_CB_RETRIED_NUM(pst_cb))++;

            #if defined(_PRE_PRODUCT_ID_HI110X_DEV)
                if (MAC_GET_CB_RETRIED_NUM(pst_cb) > DMAC_MAX_PENDING_RETRY)
            #else
                /* 限制probe response 帧的重传最大次数为1 超过100ms丢弃 其余帧保持不变*/
                if (((WLAN_MANAGEMENT == pst_cb->en_frame_type) &&
                    (WLAN_PROBE_RSP == pst_cb->pst_frame_header->st_frame_control.bit_sub_type) &&
                    (MAC_GET_CB_RETRIED_NUM(pst_cb) > DMAC_MAX_PROBE_RSP_PENDING_RETRY))
                    || (MAC_GET_CB_RETRIED_NUM(pst_cb) > DMAC_MAX_PENDING_RETRY))
            #endif
                {
                    OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_save_tx_queue::retry cnt=%d, break}", MAC_GET_CB_RETRIED_NUM(pst_cb));
                    dmac_tx_complete_buff(pst_hal_device, pst_tx_dscr);
                    break;
                }

                pst_tx_dscr_tmp  = pst_tx_dscr;
                do
                {
                    pst_tx_dscr_tmp->bit_is_retried = OAL_TRUE;

                    hal_tx_set_dscr_seqno_sw_generate(pst_hal_device, pst_tx_dscr_tmp, 0);

                    hal_tx_set_dscr_status(pst_hal_device, pst_tx_dscr_tmp, DMAC_TX_INVALID);
                    hal_get_tx_dscr_next(pst_hal_device, pst_tx_dscr_tmp, &pst_tx_dscr_tmp);
                }while(OAL_PTR_NULL != pst_tx_dscr_tmp);

                break; // 认定不可能同时传输两个帧
            }
            else if (DMAC_TX_INVALID != uc_tx_status)
            {
                dmac_tx_complete_buff(pst_hal_device, pst_tx_dscr);

                continue;
            }
            else
            {
                break;
            }
        }
    }

    for (uc_q_id = 0; uc_q_id < HAL_TX_QUEUE_BUTT; uc_q_id++)
    {
        /* 将硬件队列里的内容放到虚假队列中去 */
        oal_dlist_join_head(&pst_fake_queue[uc_q_id].st_header, &pst_hal_device->ast_tx_dscr_queue[uc_q_id].st_header);
        pst_fake_queue[uc_q_id].uc_ppdu_cnt += pst_hal_device->ast_tx_dscr_queue[uc_q_id].uc_ppdu_cnt;

        oal_dlist_init_head(&pst_hal_device->ast_tx_dscr_queue[uc_q_id].st_header);
        pst_hal_device->ast_tx_dscr_queue[uc_q_id].uc_ppdu_cnt = 0;
    }

    return;
}



#if defined (_PRE_WLAN_FEATURE_UAPSD) || defined (_PRE_WLAN_FEATURE_STA_PM)

oal_uint32 dmac_send_qosnull(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_uint8 uc_ac, oal_bool_enum_uint8 en_ps)
{
    oal_netbuf_stru                 *pst_net_buf;
    mac_tx_ctl_stru                 *pst_tx_ctrl;
    oal_uint32                       ul_ret;
    mac_ieee80211_qos_frame_stru    *pst_mac_header;
    oal_uint16                       us_tx_direction = WLAN_FRAME_FROM_AP;
    /* 入参检查 */
    if ((OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_KEEPALIVE, "{dmac_uapsd_send_qosnull:: func input  is null.}");
        return OAL_ERR_CODE_KEEPALIVE_PTR_NULL;
    }
    if (uc_ac >= MAC_AC_PARAM_LEN)
    {
        OAM_ERROR_LOG1(0, OAM_SF_KEEPALIVE, "{dmac_uapsd_send_qosnull:: uc_ac %d is too large.}", uc_ac);
        return OAL_FAIL;
    }
    /* 申请net_buff */
    pst_net_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_net_buf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_KEEPALIVE, "{dmac_uapsd_send_qosnull::pst_net_buf failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    OAL_MEM_NETBUF_TRACE(pst_net_buf, OAL_TRUE);
    oal_set_netbuf_prev(pst_net_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_net_buf, OAL_PTR_NULL);
    /* null帧发送方向From AP || To AP */
    us_tx_direction = (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode) ? WLAN_FRAME_FROM_AP : WLAN_FRAME_TO_AP;
    /* 填写帧头 */
    OAL_MEMZERO(OAL_NETBUF_HEADER(pst_net_buf), OAL_SIZEOF(mac_ieee80211_qos_frame_stru));
    mac_null_data_encap(OAL_NETBUF_HEADER(pst_net_buf),
                        ((oal_uint16)(WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS_NULL) | us_tx_direction),
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                        mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info));
    pst_mac_header = (mac_ieee80211_qos_frame_stru*)OAL_NETBUF_HEADER(pst_net_buf);
    pst_mac_header->bit_qc_tid = WLAN_WME_AC_TO_TID(uc_ac);
    pst_mac_header->bit_qc_eosp = 0;
    pst_mac_header->st_frame_control.bit_power_mgmt = en_ps;

    /*协议规定单播的QOS NULL DATA只允许normal ack*/
    pst_mac_header->bit_qc_ack_polocy = WLAN_TX_NORMAL_ACK;
    /* 填写cb字段 */
    pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
    OAL_MEMZERO(pst_tx_ctrl, OAL_SIZEOF(mac_tx_ctl_stru));
    /* 填写tx部分 */
    MAC_SET_CB_IS_QOS_DATA(pst_tx_ctrl, OAL_TRUE);
    MAC_GET_CB_ACK_POLACY(pst_tx_ctrl)     = WLAN_TX_NORMAL_ACK;
    MAC_GET_CB_EVENT_TYPE(pst_tx_ctrl)      = FRW_EVENT_TYPE_WLAN_DTX;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctrl) = uc_ac;
    MAC_GET_CB_RETRIED_NUM(pst_tx_ctrl)      = 0;
    MAC_GET_CB_WME_TID_TYPE(pst_tx_ctrl)= WLAN_WME_AC_TO_TID(uc_ac);
    MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctrl)     = pst_dmac_vap->st_vap_base_info.uc_vap_id;
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctrl)      = pst_dmac_user->st_user_base_info.us_assoc_id;
    /* 填写tx rx公共部分 */
    MAC_SET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl, (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_net_buf));
    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl)     = OAL_SIZEOF(mac_ieee80211_qos_frame_stru);
    MAC_GET_CB_MPDU_NUM(pst_tx_ctrl)                = 1;
    MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl)              = 1;
    MAC_GET_CB_MPDU_LEN(pst_tx_ctrl)                = 0;
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctrl)             = pst_dmac_user->st_user_base_info.us_assoc_id;
    //MAC_GET_CB_IS_NEEDRETRY(pst_tx_ctrl)        = 1;/* NULL帧软重传10次 */
    ul_ret = dmac_tx_process_data(pst_dmac_vap->pst_hal_device, pst_dmac_vap, pst_net_buf);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_KEEPALIVE,
                       "{dmac_uapsd_send_qosnull::dmac_tx_process_data failed[%d].}", ul_ret);
        dmac_tx_excp_free_netbuf(pst_net_buf);
        return ul_ret;
    }
    return OAL_SUCC;
}
#endif

#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rwdata = "BTCM", code ="ATCM", zidata = "BTCM", rodata = "ATCM"
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 dmac_tx_enable_in_roaming(hal_to_dmac_device_stru *pst_hal_device,
                                                                            dmac_vap_stru *pst_dmac_vap)
{
    if (MAC_VAP_STATE_ROAMING == pst_dmac_vap->st_vap_base_info.en_vap_state)
    {
        if (pst_hal_device->uc_current_chan_number != pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number)
        {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}
#endif


OAL_STATIC OAL_INLINE oal_void dmac_tx_data_statistics(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user,
                                                            mac_tx_ctl_stru *pst_tx_ctl, hal_tx_dscr_stru *pst_tx_dscr)
{
    /* 不是以太网来的不统计 */
    if (FRW_EVENT_TYPE_HOST_DRX == MAC_GET_CB_EVENT_TYPE(pst_tx_ctl) && (OAL_TRUE != pst_tx_dscr->bit_is_retried))
    {
#ifdef _PRE_WLAN_DFT_STAT
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_drv_tx_pkts,
                                    MAC_GET_CB_NETBUF_NUM(pst_tx_ctl));
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_drv_tx_bytes,
                                    MAC_GET_CB_MPDU_LEN(pst_tx_ctl));
#endif
        DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_query_stats.ul_tx_total, MAC_GET_CB_NETBUF_NUM(pst_tx_ctl));
    }
}

oal_void dmac_tx_set_first_cb(mac_tx_ctl_stru *pst_cb_first, dmac_user_stru *pst_user)
{
    MAC_GET_CB_TX_USER_IDX(pst_cb_first)  = pst_user->st_user_base_info.us_assoc_id;

    /***** 临时维测 *****/
    if (MAC_GET_CB_IS_VIPFRAME(pst_cb_first))
    {
        MAC_GET_CB_FRAME_SUBTYPE(pst_cb_first)  = MAC_DATA_BUTT;
        MAC_GET_CB_IS_NEEDRETRY(pst_cb_first) = OAL_FALSE;
        MAC_GET_CB_RETRIED_NUM(pst_cb_first)      = 0;
    }

    return;
}


OAL_STATIC OAL_INLINE oal_void dmac_save_frag_seq(dmac_user_stru     *pst_dmac_user,
                                                         mac_tx_ctl_stru    *pst_tx_ctl)
{
    oal_uint8                   uc_tid = 0;
    mac_ieee80211_frame_stru   *pst_mac_header = OAL_PTR_NULL;

    pst_mac_header = (mac_ieee80211_frame_stru   *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
    uc_tid         = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);

    /* first frame for frag frame */
    if (pst_mac_header->st_frame_control.bit_more_frag != 0 && 0 == pst_mac_header->bit_frag_num)
    {
        pst_dmac_user->aus_txseqs_frag[uc_tid] = pst_dmac_user->aus_txseqs[uc_tid];
        pst_dmac_user->aus_txseqs[uc_tid] = DMAC_BA_SEQ_ADD(pst_dmac_user->aus_txseqs[uc_tid], 1);
    }
}


OAL_STATIC OAL_INLINE oal_void dmac_tx_seqnum_set(hal_to_dmac_device_stru *pst_hal_device, dmac_user_stru *pst_dmac_user, mac_tx_ctl_stru *pst_tx_ctl, hal_tx_dscr_stru *pst_tx_dscr)
{
    oal_uint8                   uc_tid = 0;
    mac_ieee80211_frame_stru   *pst_mac_header = OAL_PTR_NULL;

    if (OAL_TRUE != (oal_bool_enum_uint8)MAC_GET_CB_IS_QOS_DATA(pst_tx_ctl))
    {
        return;
    }

    uc_tid  = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    /* 开启聚合硬化功能, 非聚合帧的seq num也由硬件维护 */
    if(OAL_TRUE == pst_hal_device->en_ampdu_tx_hw_en)
    {
        pst_dmac_user->aus_txseqs[uc_tid] = DMAC_BA_SEQ_ADD(pst_dmac_user->aus_txseqs[uc_tid], 1);
        return;
    }
#endif

    hal_tx_seqnum_set_dscr(pst_tx_dscr, pst_dmac_user->aus_txseqs[uc_tid]);

    pst_mac_header = (mac_ieee80211_frame_stru *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);

    if (OAL_LIKELY((0 == pst_mac_header->st_frame_control.bit_more_frag ) && (0 == pst_mac_header->bit_frag_num)))
    {
        pst_mac_header->bit_seq_num    = pst_dmac_user->aus_txseqs[uc_tid];
        MAC_GET_CB_SEQ_NUM(pst_tx_ctl) = pst_dmac_user->aus_txseqs[uc_tid];
        pst_dmac_user->aus_txseqs[uc_tid] = DMAC_BA_SEQ_ADD(pst_dmac_user->aus_txseqs[uc_tid], 1);
    }
    else
    {
        pst_mac_header->bit_seq_num = pst_dmac_user->aus_txseqs_frag[uc_tid];
    }

}


OAL_STATIC OAL_INLINE oal_void dmac_tx_seqnum_set_ampdu(hal_to_dmac_device_stru *pst_hal_device, dmac_user_stru *pst_dmac_user, mac_tx_ctl_stru *pst_tx_ctl, hal_tx_dscr_stru *pst_tx_dscr)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_ieee80211_frame_stru   *pst_frame_hdr = OAL_PTR_NULL;
#endif
    oal_uint8                   uc_tid = 0;

    uc_tid  = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    if (OAL_TRUE == pst_hal_device->en_ampdu_tx_hw_en)
    {
        pst_dmac_user->aus_txseqs[uc_tid] = DMAC_BA_SEQ_ADD(pst_dmac_user->aus_txseqs[uc_tid], 1);
        return;
    }
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    pst_frame_hdr                    = MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl);
    pst_frame_hdr->bit_seq_num       = pst_dmac_user->aus_txseqs[uc_tid];
#endif

    MAC_GET_CB_SEQ_NUM(pst_tx_ctl) = pst_dmac_user->aus_txseqs[uc_tid];

    hal_tx_seqnum_set_dscr(pst_tx_dscr, pst_dmac_user->aus_txseqs[uc_tid]);

    pst_dmac_user->aus_txseqs[uc_tid] = DMAC_BA_SEQ_ADD(pst_dmac_user->aus_txseqs[uc_tid], 1);

}


oal_netbuf_stru* dmac_tx_dequeue_first_mpdu(oal_netbuf_head_stru  *pst_netbuf_head)
{
    oal_netbuf_stru        *pst_first_net_buf;
    oal_netbuf_stru        *pst_tmp_net_buf;
    oal_netbuf_stru        *pst_net_buf;
    mac_tx_ctl_stru        *pst_tx_ctrl;
    oal_uint8               uc_netbuf_num_per_mpdu;

    pst_first_net_buf = OAL_NETBUF_HEAD_NEXT(pst_netbuf_head);
    pst_tx_ctrl       = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_first_net_buf);

    /* mpdu不是a-msdu，返回第一个net_buff即可 */
    if (OAL_FALSE == MAC_GET_CB_IS_AMSDU(pst_tx_ctrl))
    {
        pst_first_net_buf = oal_netbuf_delist(pst_netbuf_head);
        return pst_first_net_buf;
    }

    /* 出现这种异常只能将所有缓存帧释放掉 */
    if (OAL_FALSE == MAC_GET_CB_IS_FIRST_MSDU(pst_tx_ctrl))
    {
       /* 这个错误只有踩内存才会出现，如果出现就无法恢复，以太网来的包无法释放，
           软件复位也没用，内存也会泄漏
        */
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_dequeue_first_mpdu::not the first msdu.}");
        /* 出现此错误时，需要将此帧出队,否则造成拥塞无法调度 */
        pst_first_net_buf = oal_netbuf_delist(pst_netbuf_head);
        return pst_first_net_buf;
    }

    /* 节能队列中的第一个mpdu是a-msdu，获取skb个数 */
    uc_netbuf_num_per_mpdu = MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl);

    /* 将第一个mpdu中的所有skb从节能队列中取出，然后组成一个net_buff链 */
    pst_first_net_buf = oal_netbuf_delist(pst_netbuf_head);
    uc_netbuf_num_per_mpdu--;

    pst_tmp_net_buf = pst_first_net_buf;
    while (0 != uc_netbuf_num_per_mpdu)
    {
        pst_net_buf = oal_netbuf_delist(pst_netbuf_head);
        oal_set_netbuf_prev(pst_net_buf, pst_tmp_net_buf);
        oal_set_netbuf_next(pst_net_buf, OAL_PTR_NULL);

        oal_set_netbuf_next(pst_tmp_net_buf, pst_net_buf);

        pst_tmp_net_buf = pst_net_buf;

        uc_netbuf_num_per_mpdu--;
    }

    return pst_first_net_buf;

}


oal_uint32  dmac_tid_tx_dequeue(hal_to_dmac_device_stru    *pst_hal_device,
                                        dmac_vap_stru      *pst_dmac_vap,
                                        dmac_tid_stru      *pst_tid_queue,
                                        hal_tx_dscr_stru   *pst_tx_dscr,
                                        mac_device_stru    *pst_dev,
                                        dmac_user_stru     *pst_user)
{
    mac_tx_ctl_stru            *pst_cb;
    oal_netbuf_stru            *pst_netbuf;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_DFT_STAT
    dmac_tid_stats_stru *pst_tid_stats;
    pst_tid_stats = pst_tid_queue->pst_tid_stats;
    if (OAL_PTR_NULL == pst_tid_stats)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tid_tx_dequeue::dequeue from netbuff Q or retry Q failed, pst_tid_stats NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
#endif

    OAL_MEM_TRACE(pst_tx_dscr, OAL_FALSE);
    pst_tx_dscr->bit_is_first = 0;

    dmac_tid_tx_dequeue_update(pst_dev, pst_tid_queue, 1);

    if ((OAL_TRUE == pst_tx_dscr->bit_is_retried) && (pst_tid_queue->uc_retry_num > 0))
    {
        pst_tid_queue->uc_retry_num--;
    }

    DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_dequeue_normal_cnt, 1);
    DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_dequeue_total_cnt, 1);


    pst_netbuf = pst_tx_dscr->pst_skb_start_addr;
    pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    if (OAL_PTR_NULL == pst_cb)
    {
        OAM_ERROR_LOG0(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_dequeue::dequeue from netbuff Q or retry Q failed, pst_cb null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    MAC_GET_CB_TX_USER_IDX(pst_cb)  = pst_user->st_user_base_info.us_assoc_id;

    /* 不是以太网来的不统计 */
    if ((FRW_EVENT_TYPE_HOST_DRX == MAC_GET_CB_EVENT_TYPE(pst_cb))
        && (OAL_TRUE != pst_tx_dscr->bit_is_retried))
    {
#ifdef _PRE_WLAN_DFT_STAT
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_drv_tx_pkts,
                               MAC_GET_CB_NETBUF_NUM(pst_cb));
        DMAC_VAP_DFT_STATS_PKT_INCR(pst_dmac_vap->st_query_stats.ul_drv_tx_bytes,
                                MAC_GET_CB_MPDU_LEN(pst_cb));
#endif
        DMAC_USER_STATS_PKT_INCR(pst_user->st_query_stats.ul_tx_total, MAC_GET_CB_NETBUF_NUM(pst_cb));
    }


    /*非重传的qos帧需要软件维护seqnum*/
    if ((OAL_FALSE == pst_tx_dscr->bit_is_retried) && (OAL_TRUE == (oal_bool_enum_uint8)MAC_GET_CB_IS_QOS_DATA(pst_cb)))
    {
        dmac_save_frag_seq(pst_user, pst_cb);
        dmac_tx_seqnum_set(pst_hal_device, pst_user, pst_cb, pst_tx_dscr);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_tx_tid_enqueue(dmac_tid_stru *pst_tid_queue, oal_dlist_head_stru *pst_dscr_head, hal_tx_dscr_stru *pst_tx_dscr)
{
    oal_dlist_head_stru        *pst_dscr_entry      = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_netbuf_stru            *pst_netbuf;
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    oal_netbuf_stru            *pst_netbuf_next;
#endif

    pst_dscr_entry = oal_dlist_delete_tail(pst_dscr_head);

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    if (OAL_TRUE == pst_tx_dscr->bit_is_retried)
    {
        /* 需要放回重传队列，从队列尾放回到重传队列头 */
        oal_dlist_add_head(pst_dscr_entry, &pst_tid_queue->st_retry_q);
        pst_tid_queue->uc_retry_num++;
    }
    else
    {
        /* 放回netbuf队列 */
        pst_tx_dscr    = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        pst_netbuf     = pst_tx_dscr->pst_skb_start_addr;

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        if (OAL_PTR_NULL != oal_get_netbuf_next(pst_netbuf))
        {
            pst_netbuf_next = pst_netbuf->next;
            oal_set_netbuf_next(pst_netbuf, OAL_PTR_NULL);
            dmac_tx_queue_mpdu_head(pst_netbuf_next, &pst_tid_queue->st_buff_head);
        }
#endif
        dmac_tx_queue_mpdu_head(pst_netbuf, &pst_tid_queue->st_buff_head);
        OAL_MEM_FREE(pst_tx_dscr, OAL_TRUE);
    }
#else
    /* 需要放回TID队列，从队列尾放回到TID队列头 */
    oal_dlist_add_head(pst_dscr_entry, &pst_tid_queue->st_hdr);
    if (OAL_TRUE == pst_tx_dscr->bit_is_retried)
    {
        pst_tid_queue->uc_retry_num++;
    }
#endif

}

OAL_STATIC oal_bool_enum_uint8 dmac_tid_tx_retry_in_baw(hal_to_dmac_device_stru   *pst_hal_device,
                                                               mac_device_stru           *pst_dev,
                                                               hal_tx_dscr_stru          *pst_tx_dscr,
                                                               dmac_tid_stru             *pst_tid_queue)
{
    oal_uint16                  us_seq_num          = 0;
    dmac_ba_tx_stru            *pst_tx_ba_handle;
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    mac_tx_ctl_stru            *pst_cb;
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_DFT_STAT
    dmac_tid_stats_stru        *pst_tid_stats;

    pst_tid_stats = pst_tid_queue->pst_tid_stats;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tid_stats))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tid_tx_retry_in_baw::tid_stats is null.}");
        return OAL_TRUE;
    }
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    /* 使能硬件聚合，不做BA窗判定 */
    if(OAL_TRUE == pst_hal_device->en_ampdu_tx_hw_en)
    {
        return OAL_TRUE;
    }
#endif

    pst_tx_ba_handle = pst_tid_queue->pst_ba_tx_hdl;

    /* 获取seq num */
    hal_tx_get_dscr_seq_num(pst_hal_device, pst_tx_dscr, &us_seq_num);

    /* 如果该包不在窗内，需要丢弃 */
    if (!DMAC_BAW_WITHIN(pst_tx_ba_handle->us_baw_start, pst_tx_ba_handle->us_baw_size, us_seq_num))
    {
        OAM_WARNING_LOG3(0, OAM_SF_TX, "{dmac_tid_tx_retry_in_baw::a retry packet shall be dropped. baw_start=%d baw_size=%d sqe_num=%d.}",
                         pst_tx_ba_handle->us_baw_start, pst_tx_ba_handle->us_baw_size, us_seq_num);
        pst_tid_queue->uc_retry_num--;

    #ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_tx_dscr->pst_skb_start_addr);
        /* AMPDU中AMSDU子帧数 */
        if (OAL_TRUE == MAC_GET_CB_IS_LARGE_SKB_AMSDU(pst_cb))
        {
            dmac_tid_tx_dequeue_update(pst_dev, pst_tid_queue, 2);
        }
        else
    #endif
        {
            dmac_tid_tx_dequeue_update(pst_dev, pst_tid_queue, 1);
        }

        oal_dlist_delete_entry(&pst_tx_dscr->st_entry);
        dmac_free_tx_dscr(pst_tx_dscr);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_DFT_STAT
        DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_retry_dequeue_cnt, 1);
        DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_dequeue_ampdu_cnt, 1);
        DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_dequeue_total_cnt, 1);
#endif
#endif
        return OAL_FALSE;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_DFT_STAT
    DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_retry_dequeue_cnt, 1);
#endif
#endif
    return OAL_TRUE;
}


oal_uint32 dmac_tx_reinit_tx_queue(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_to_dmac_dev)
{
    oal_uint8                        uc_queue_num;
    oal_uint32                       ul_all_status = 0;
    oal_bool_enum_uint8              en_empty = 0;
    oal_uint8                        uc_idx;
    oal_dlist_head_stru             *pst_dlist_entry;
    hal_tx_dscr_stru                *pst_tx_dscr;

    if (!pst_mac_device || !pst_hal_to_dmac_dev)
    {
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    FRW_TIMER_STOP_TIMER(&(pst_hal_to_dmac_dev->st_dfr_tx_prot.st_tx_prot_timer));
#endif

    /* 挂起硬件发送 */
    hal_set_machw_tx_suspend(pst_hal_to_dmac_dev);

    /* 停止mac phy接收 */
    hal_disable_machw_phy_and_pa(pst_hal_to_dmac_dev);

    // 确保发送完成事件执行完成
    for (uc_idx = 0; uc_idx < pst_mac_device->uc_vap_num; uc_idx++)
    {
        frw_event_vap_flush_event(pst_mac_device->auc_vap_id[uc_idx], FRW_EVENT_TYPE_WLAN_TX_COMP, OAL_FALSE);
    }

    /* 读硬件队列寄存器状态 */
    hal_get_all_tx_q_status(pst_hal_to_dmac_dev, &ul_all_status);

    for(uc_queue_num = 0; uc_queue_num < HAL_TX_QUEUE_NUM; uc_queue_num++)
    {
        en_empty = ul_all_status & ((oal_uint32)1 << (uc_queue_num * 2)) ? OAL_TRUE : OAL_FALSE;

        // 硬件队列为空且软件队列中有帧，则处理。当前强制丢弃，后续可以考虑塞会TID或者重新放回硬件
        if (pst_hal_to_dmac_dev->ast_tx_dscr_queue[uc_queue_num].uc_ppdu_cnt > 0)
        {
            OAM_WARNING_LOG3(0, OAM_SF_TX, "{dmac_tx_reinit_tx_queue: queue=%d empty=%d ppdu_cnt=%d",
                uc_queue_num, en_empty, pst_hal_to_dmac_dev->ast_tx_dscr_queue[uc_queue_num].uc_ppdu_cnt);
            while(!oal_dlist_is_empty(&pst_hal_to_dmac_dev->ast_tx_dscr_queue[uc_queue_num].st_header))
            {
                pst_dlist_entry = pst_hal_to_dmac_dev->ast_tx_dscr_queue[uc_queue_num].st_header.pst_next;
                pst_tx_dscr     = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_entry, hal_tx_dscr_stru, st_entry);
                /*lint -save -e774 */
                if(pst_tx_dscr)
                {
                    dmac_tx_complete_buff(pst_hal_to_dmac_dev, pst_tx_dscr);
                }
                else
                {
                    break;
                }
                /*lint -restore */
            }

            if (0 != pst_hal_to_dmac_dev->ast_tx_dscr_queue[uc_queue_num].uc_ppdu_cnt)
            {
                OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_reinit_tx_queue: ppdu exist after flush, ppdu_cnt=%d， force 0", pst_hal_to_dmac_dev->ast_tx_dscr_queue[uc_queue_num].uc_ppdu_cnt);
                pst_hal_to_dmac_dev->ast_tx_dscr_queue[uc_queue_num].uc_ppdu_cnt = 0;
            }
        }
    }

    hal_clear_hw_fifo(pst_hal_to_dmac_dev);
    hal_enable_machw_phy_and_pa(pst_hal_to_dmac_dev);
    hal_set_machw_tx_resume(pst_hal_to_dmac_dev);

    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_tx_tid_update_stat(hal_tx_ppdu_feature_stru *pst_ppdu_feature,
                                                               dmac_tid_stru            *pst_tid_queue,
                                                               mac_device_stru          *pst_dev,
                                                               oal_uint8                 uc_mpdu_idx,
                                                               oal_uint8                 uc_amsdu_cnt)
{
    oal_uint8   uc_mpdu_num;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_DFT_STAT
    dmac_tid_stats_stru        *pst_tid_stats;

    pst_tid_stats = pst_tid_queue->pst_tid_stats;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tid_stats))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tid_tx_retry_check::tid_stats is null.}");
        return;
    }
#endif
#endif

    pst_ppdu_feature->uc_mpdu_num += uc_mpdu_idx;

    uc_mpdu_num = uc_mpdu_idx + uc_amsdu_cnt;

    dmac_tid_tx_dequeue_update(pst_dev, pst_tid_queue, uc_mpdu_num);

    DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_dequeue_ampdu_cnt, uc_mpdu_num);
    DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_dequeue_total_cnt, uc_mpdu_num);

}




OAL_STATIC OAL_INLINE oal_uint32 dmac_tx_update_protection_lsig_txop(dmac_vap_stru *pst_dmac_vap, hal_tx_txop_alg_stru *pst_txop_alg, oal_uint8 uc_do_default_cfg)
{
    mac_protection_stru          *pst_protection;
    wlan_phy_protocol_enum_uint8  en_protocol_mode;

    en_protocol_mode = pst_txop_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode;
    pst_protection = &(pst_dmac_vap->st_vap_base_info.st_protection);

    /*如果VAP启用了L-SIG TXOP保护机制来做ht保护，并且发送协议模式是HT, 则需要设置发送描述符中bit_lsig_txop置1*/
    if ((OAL_SWITCH_ON == pst_protection->bit_lsig_txop_protect_mode)
         &&(WLAN_HT_PHY_PROTOCOL_MODE == en_protocol_mode))
    {
        pst_txop_alg->st_rate.bit_lsig_txop = OAL_TRUE;
    }
    else
    {
        /*在保护模式发生变化时候需要恢复lsig txop默认值为0*/
        if (OAL_TRUE == uc_do_default_cfg)
        {
            pst_txop_alg->st_rate.bit_lsig_txop = OAL_FALSE;
        }
        else /*其他情况不强制要求bit_lsig_txop置1或置0*/
        {
            /*当入参是算法提供的发送参数时候，不需要对bit_lsig_txop做修改， 保持算法提供值*/
        }
    }

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_uint32 dmac_tx_update_protection_preamble_mode(dmac_vap_stru *pst_dmac_vap, hal_tx_txop_alg_stru *pst_txop_alg, oal_uint8 uc_do_default_cfg)
{
    dmac_user_stru               *pst_dmac_user;
    mac_protection_stru          *pst_protection;
    wlan_phy_protocol_enum_uint8  en_protocol_mode;
    oal_uint8                     uc_preamble_mode = 0;
    oal_uint8                     uc_rate_index = 0;
    oal_uint8                     uc_legacy_rate = 0;

    pst_protection = &(pst_dmac_vap->st_vap_base_info.st_protection);

    for(uc_rate_index = 0; uc_rate_index < HAL_TX_RATE_MAX_NUM; uc_rate_index++)
    {
        /*如果发送count为0， 则说明本级速率无效*/
        if (0 == pst_txop_alg->ast_per_rate[uc_rate_index].rate_bit_stru.bit_tx_count)
        {
            continue;
        }

        uc_preamble_mode = 0xFF;
        en_protocol_mode = pst_txop_alg->ast_per_rate[uc_rate_index].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode;
        uc_legacy_rate   = pst_txop_alg->ast_per_rate[uc_rate_index].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate;

        /*11b协议下， 确认是否需要设置long preamble*/
        if (WLAN_11B_PHY_PROTOCOL_MODE == en_protocol_mode)
        {
            /*VAP 为 AP情况下*/
            if (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode)
            {
                /* 如果有不支持short preamble站点与AP关联， 或者AP自身不支持short preamble，或者发送率为1mpbs，则需要设置AP使用long preamble方式发送管理帧*/
                if ((0 != pst_protection->uc_sta_no_short_preamble_num)
                     || (OAL_TRUE != mac_mib_get_ShortPreambleOptionImplemented(&(pst_dmac_vap->st_vap_base_info)))
                     || (WLAN_PHY_RATE_1M == uc_legacy_rate))
                {
                    uc_preamble_mode = WLAN_LEGACY_11B_DSCR_LONG_PREAMBLE;
                }
                else
                {
                    /*在保护模式发生变化时候需要恢复为默认值short preamble*/
                    if (OAL_TRUE == uc_do_default_cfg)
                    {
                        uc_preamble_mode = WLAN_LEGACY_11B_DSCR_SHORT_PREAMBLE;
                    }
                    else /*其他情况不做强制要求*/
                    {
                        /*当入参是算法提供的发送参数时候，不需要对uc_preamble_mode做修改， 保持算法提供值*/
                    }
                }

            }
            else/*VAP 为 STA情况下*/
            {
                pst_dmac_user = mac_res_get_dmac_user(pst_dmac_vap->st_vap_base_info.us_assoc_vap_id); /*user保存的是AP的信息*/
                if (OAL_PTR_NULL == pst_dmac_user)
                {
                    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_update_protection_preamble_mode::pst_dmac_user null, assoc_vap_id [%d].}",
                                    pst_dmac_vap->st_vap_base_info.us_assoc_vap_id);
                    return OAL_ERR_CODE_PTR_NULL;
                }

                /* 如果接收到beacon帧中携带的ERP信息元素中baker preamble mode为1， 或者STA自身不支持short preamble， 则需要设置STA使用long preamble方式发送管理帧*/
                if ((OAL_TRUE == pst_dmac_user->st_user_base_info.st_cap_info.bit_barker_preamble_mode)
                      || (OAL_TRUE != mac_mib_get_ShortPreambleOptionImplemented(&(pst_dmac_vap->st_vap_base_info)))
                      || (WLAN_PHY_RATE_1M == uc_legacy_rate))
                {
                    uc_preamble_mode = WLAN_LEGACY_11B_DSCR_LONG_PREAMBLE;
                }
                else
                {
                    /*在保护模式发生变化时候需要恢复为默认值short preamble*/
                    if (OAL_TRUE == uc_do_default_cfg)
                    {
                        uc_preamble_mode = WLAN_LEGACY_11B_DSCR_SHORT_PREAMBLE;
                    }
                    else /*其他情况不做强制要求*/
                    {
                        /*当入参是算法提供的发送参数时候，不需要对uc_preamble_mode做修改， 保持算法提供值*/
                    }
                }
            }
        }
        /*HT协议下*/
        else if (WLAN_HT_PHY_PROTOCOL_MODE == en_protocol_mode)
        {
            /*启用了GF保护时候， 管理帧发送需要设置为MF前导码发送*/
            if (WLAN_PROT_GF == pst_protection->en_protection_mode)
            {
                uc_preamble_mode = OAL_FALSE;
            }
            else
            {
                /*在保护模式发生变化时候需要恢复为默认值,1151方案目前默认就是用MF前导码发送*/
                if (OAL_TRUE == uc_do_default_cfg)
                {
                    uc_preamble_mode = OAL_FALSE;
                }
                else /*其他情况不做强制要求*/
                {
                    /*当入参是算法提供的发送参数时候，不需要对uc_preamble_mode做修改， 保持算法提供值*/
                }
            }
        }
        else /*其他协议模式下preamble为0*/
        {
            uc_preamble_mode = 0;
        }

        if(0xFF != uc_preamble_mode)
        {
            pst_txop_alg->ast_per_rate[uc_rate_index].rate_bit_stru.bit_preamble_mode = uc_preamble_mode;
        }

    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_tx_update_protection_txop_alg(dmac_vap_stru *pst_dmac_vap, hal_tx_txop_alg_stru *pst_txop_alg, oal_uint8 uc_do_default_cfg)
{
    oal_uint32 ul_ret = OAL_SUCC;

    /*更新lsig txop参数*/
    dmac_tx_update_protection_lsig_txop(pst_dmac_vap, pst_txop_alg, uc_do_default_cfg);

    

    /*更新preamble参数*/
    ul_ret = dmac_tx_update_protection_preamble_mode(pst_dmac_vap, pst_txop_alg, uc_do_default_cfg);

    return ul_ret;
}


OAL_STATIC OAL_INLINE oal_void dmac_judge_forbid_rts(dmac_user_stru *pst_dmac_user, hal_tx_txop_alg_stru *pst_txop_alg)
{
    oal_uint8   uc_rate_rank;

    /* 在开启RTS存在兼容性问题时, 强制关闭RTS */
    if (OAL_TRUE == pst_dmac_user->bit_forbid_rts)
    {
        for (uc_rate_rank = 0; uc_rate_rank < HAL_TX_RATE_MAX_NUM; uc_rate_rank++)
        {
            pst_txop_alg->ast_per_rate[uc_rate_rank].rate_bit_stru.bit_rts_cts_enable = OAL_FALSE;
        }
    }
}


OAL_STATIC OAL_INLINE oal_void  dmac_tx_set_txopps_param(dmac_vap_stru  *pst_dmac_vap,
                                                      dmac_user_stru *pst_dmac_user,
                                                      hal_tx_txop_feature_stru *pst_txop_feature,
                                                      oal_bool_enum_uint8       en_ismcast)
{
    /* 以下根据Draft P802.11ac_D3.1.pdf 9.17a填写 */
    if (en_ismcast)
    {
        pst_txop_feature->st_groupid_partial_aid.uc_group_id    = 63;
        pst_txop_feature->st_groupid_partial_aid.us_partial_aid = 0;
    }
    else
    {
        pst_txop_feature->st_groupid_partial_aid.uc_group_id    = pst_dmac_user->uc_groupid;
        pst_txop_feature->st_groupid_partial_aid.us_partial_aid = pst_dmac_user->us_partial_aid;
    }

    switch (pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        case WLAN_VAP_MODE_BSS_STA:
            pst_txop_feature->st_groupid_partial_aid.uc_txop_ps_not_allowed = OAL_TRUE;
            break;

        case WLAN_VAP_MODE_BSS_AP:
            if (OAL_TRUE == mac_mib_get_txopps(&(pst_dmac_vap->st_vap_base_info)))
            {
                /* 对于AP,如果支持TXOP PS则始终允许STA睡眠 */
                pst_txop_feature->st_groupid_partial_aid.uc_txop_ps_not_allowed = OAL_FALSE;
            }
            else
            {
                pst_txop_feature->st_groupid_partial_aid.uc_txop_ps_not_allowed = OAL_TRUE;
            }
            break;

        default:
            OAM_ERROR_LOG1(0, OAM_SF_ANY,"{dmac_tx_set_txopps_param: unvalid vap mode[%d]}", pst_dmac_vap->st_vap_base_info.en_vap_mode);
            break;
    }
}


OAL_STATIC OAL_INLINE oal_void dmac_tx_data_get_txop(dmac_vap_stru  *pst_dmac_vap,
                                          dmac_user_stru *pst_dmac_user,
                                          hal_tx_txop_feature_stru *pst_txop_feature,
                                          mac_tx_ctl_stru          *pst_tx_ctl)
{
    /* 获取TXOP特性发送参数 */
    pst_txop_feature->st_security.en_cipher_key_type      = pst_dmac_user->st_user_base_info.st_user_tx_info.st_security.en_cipher_key_type;
    pst_txop_feature->st_security.en_cipher_protocol_type = pst_dmac_user->st_user_base_info.st_key_info.en_cipher_type;
    pst_txop_feature->st_security.uc_cipher_key_id        = pst_dmac_user->st_user_base_info.st_key_info.uc_default_index;

    dmac_tx_set_txopps_param(pst_dmac_vap, pst_dmac_user, pst_txop_feature, MAC_GET_CB_IS_MCAST(pst_tx_ctl));

    if (OAL_TRUE == MAC_GET_CB_IS_EAPOL_KEY_PTK(pst_tx_ctl)
        && (pst_dmac_user->bit_is_rx_eapol_key_open == OAL_TRUE))
    {
        pst_txop_feature->st_security.en_cipher_protocol_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;
    }
}


oal_void  dmac_tx_get_txop_alg_params(dmac_vap_stru         *pst_dmac_vap,
                                      dmac_user_stru        *pst_dmac_user,
                                      mac_tx_ctl_stru       *pst_tx_ctl,
                                      hal_tx_txop_alg_stru **ppst_txop_alg)
{
    hal_tx_txop_alg_stru *pst_txop_para = OAL_PTR_NULL;
    if (MAC_GET_CB_IS_VIPFRAME(pst_tx_ctl))
    {
        pst_txop_para = &pst_dmac_vap->ast_tx_mgmt_bmcast[pst_dmac_vap->st_vap_base_info.st_channel.en_band];
    }
    else if (OAL_FALSE == MAC_GET_CB_IS_MCAST(pst_tx_ctl))
    {
        /* 从算法获取单播发送数据帧参数 */
#ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        /* 若vap下未配置针对特定协议的ucast data发送参数, 则采用默认的ucast data发送参数 */
        if (0 == pst_dmac_vap->un_mode_valid.uc_mode_param_valid)
        {
            pst_txop_para = &pst_dmac_vap->st_tx_alg;
        }
        else
        {
            dmac_tx_get_spec_mode_ucast_data_params(pst_dmac_vap, pst_dmac_user, pst_tx_ctl, &pst_txop_para);
        }
#else
        pst_txop_para = &pst_dmac_vap->st_tx_alg;
#endif

        /* 设置发送功率 */
        dmac_pow_tx_power_process(pst_dmac_vap, (mac_user_stru *)pst_dmac_user, pst_tx_ctl, pst_txop_para);

        /* 从算法处获取单播数据帧速率等发送参数 */
        dmac_alg_tx_notify(pst_dmac_vap, (mac_user_stru *)pst_dmac_user, pst_tx_ctl, pst_txop_para);

    }
    else /* 软件填写组播/广播数据帧发送参数 */
    {
        pst_txop_para = &pst_dmac_vap->st_tx_data_mcast;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 漫游状态时，以最低速率发送该帧 */
    if (MAC_VAP_STATE_ROAMING == pst_dmac_vap->st_vap_base_info.en_vap_state)
    {
        dmac_tx_mgmt_get_txop_para(pst_dmac_vap, &pst_txop_para, pst_tx_ctl);
    }
#endif

    /*保护模式特性更新发送参数*/
    if (WLAN_PROT_NO != pst_dmac_vap->st_vap_base_info.st_protection.en_protection_mode)
    {
        dmac_tx_update_protection_txop_alg(pst_dmac_vap, pst_txop_para, OAL_FALSE);
    }

    /* 判断是否强制关闭RTS */
    dmac_judge_forbid_rts(pst_dmac_user, pst_txop_para);

    /* 获取发送参数 */
    *ppst_txop_alg = pst_txop_para;

    return ;
}
/*lint -save -e661 */

OAL_STATIC OAL_INLINE oal_void dmac_tx_hw_send(hal_to_dmac_device_stru *pst_hal_device,
                                                      hal_tx_dscr_stru  *pst_tx_dscr,
                                                      mac_tx_ctl_stru   *pst_tx_ctl,
                                                      oal_dlist_head_stru *pst_tx_dscr_list_hdr,
                                                      oal_uint8          uc_ppdu_cnt)
{
    hal_tx_dscr_stru    *pst_last_dscr;
    oal_dlist_head_stru *pst_last_dscr_entry;
    oal_uint32           ul_tx_q_status = 0;
    oal_uint8            uc_q_num;

/* 装备维测 */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    oal_uint8           *puc_txdscr = OAL_PTR_NULL;
    oal_uint8            uc_txdscr_idx = 0;
    oal_uint8            auc_txdscr[4] = {0};
    oal_uint8            uc_vap_id;
    dmac_vap_stru       *pst_dmac_vap;
#endif
#endif

    OAL_MEM_TRACE(pst_tx_dscr, OAL_FALSE);

    uc_q_num = pst_tx_dscr->uc_q_num;
    /*****************************************************************************
        写到硬件发送队列，有两种情况
        1) 如果硬件发送队列FIFO已满，直接挂到最后一个写入硬件的描述符队列队尾
        2) 如果不满，直接将此描述符写入硬件FIFO
    *****************************************************************************/
    /* 读取发送队列状态寄存器 */
    if (HAL_TX_QUEUE_HI >= uc_q_num)
    {
        hal_get_tx_q_status(pst_hal_device, &ul_tx_q_status, uc_q_num);
    }
#ifndef _PRE_WLAN_MAC_BUGFIX_MCAST_HW_Q
    else if (HAL_TX_QUEUE_MC == uc_q_num)
    {
        hal_get_tx_multi_q_status(pst_hal_device, &ul_tx_q_status, uc_q_num);
    }
#endif

    /* 增加判断，防止插入发送描述符队列尾溢出 */
    if(HAL_TX_QUEUE_NUM <= pst_tx_dscr->uc_q_num)
    {
        hal_tx_dscr_ctrl_one_param          st_tx_dscr_one = {0};
        OAM_WARNING_LOG2(0, OAM_SF_TX, "dmac_tx_hw_send::uc_q_num % is more than %d", pst_tx_dscr->uc_q_num, HAL_TX_QUEUE_NUM);
        hal_tx_get_dscr_ctrl_one_param(pst_hal_device, pst_tx_dscr, &st_tx_dscr_one);
        /* 需要释放描述符 */
        dmac_tx_complete_free_dscr_list(pst_hal_device, pst_tx_dscr, st_tx_dscr_one.uc_mpdu_num, 0);
        return;
    }

    /* 状态寄存器为0，表示硬件发送队列FIFO未满(深度4) */
    if (0 == ul_tx_q_status)
    {
        /* 将当前发送描述符链表插入发送描述符队列尾 */
        dmac_tx_dscr_queue_add_dscr_list(pst_hal_device, pst_tx_dscr->uc_q_num, pst_tx_dscr_list_hdr, uc_ppdu_cnt);

        if (OAL_GET_THRUPUT_BYPASS_ENABLE(OAL_TX_HAL_HARDWARE_BYPASS))
        {
            dmac_post_soft_tx_complete_event(pst_hal_device, pst_tx_dscr, MAC_GET_CB_MPDU_NUM(pst_tx_ctl));
            return ;
        }
        /* 直接写入MAC硬件 */
        hal_tx_put_dscr(pst_hal_device, uc_q_num, pst_tx_dscr);

/* 装备维测 */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
        dmac_tx_get_vap_id(pst_hal_device, pst_tx_dscr, &uc_vap_id);
        pst_dmac_vap = mac_res_get_dmac_vap(uc_vap_id);
        if(OAL_PTR_NULL == pst_dmac_vap)
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX,"dmac_tx_hw_send::pst_dmac_vap null,vap id=%d", uc_vap_id);
            return;
        }
        if (OAL_SWITCH_ON == pst_dmac_vap->st_vap_base_info.bit_al_tx_flag)
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX,
            "dmac_tx_hw_send::tx dscr original_mpdu_len=0x%04x",pst_tx_dscr->us_original_mpdu_len);

            /*tx dscr lin4~22*/
            for ( uc_txdscr_idx = 0 ; uc_txdscr_idx < 4*19 ; uc_txdscr_idx++ )
            {
                puc_txdscr = (oal_uint8*)(pst_tx_dscr->data + uc_txdscr_idx);/*lint !e662*/
                auc_txdscr[uc_txdscr_idx%4] = *puc_txdscr;
                if(3 == uc_txdscr_idx%4)
                {
                OAM_WARNING_LOG2(0, OAM_SF_TX,
                    "dmac_tx_hw_send::line=%02d, tx dscr=0x%08x", (uc_txdscr_idx>>2) + 4, ((auc_txdscr[3]<<24)|(auc_txdscr[2]<<16)|(auc_txdscr[1]<<8)|auc_txdscr[0]));
                }
            }
        }
#endif
#endif

    }
    else
    {
        /* 高优先级队列或组播帧队列满写入硬件的描述符队列队尾,否则放回tid */
#ifndef _PRE_WLAN_MAC_BUGFIX_MCAST_HW_Q
        if (HAL_TX_QUEUE_HI == uc_q_num || HAL_TX_QUEUE_MC == uc_q_num)
#else
        if (HAL_TX_QUEUE_HI == uc_q_num)
#endif
        {
            /* 获取发送队列的队尾一个描述符 */
            pst_last_dscr_entry = pst_hal_device->ast_tx_dscr_queue[uc_q_num].st_header.pst_prev;
            pst_last_dscr       = OAL_DLIST_GET_ENTRY(pst_last_dscr_entry, hal_tx_dscr_stru, st_entry);

            /* 将当前发送描述符链表插入发送描述符队列尾 */
            dmac_tx_dscr_queue_add_dscr_list(pst_hal_device, pst_tx_dscr->uc_q_num, pst_tx_dscr_list_hdr, uc_ppdu_cnt);

            if (OAL_GET_THRUPUT_BYPASS_ENABLE(OAL_TX_HAL_HARDWARE_BYPASS))
            {
                dmac_post_soft_tx_complete_event(pst_hal_device, pst_tx_dscr, MAC_GET_CB_MPDU_NUM(pst_tx_ctl));
                return ;
            }

             /* 将当前描述符链首个描述符链入MAC描述符链 */
            hal_tx_ctrl_dscr_link(pst_hal_device, pst_last_dscr, pst_tx_dscr);

            OAM_WARNING_LOG4(0, OAM_SF_TX, "dmac_tx_hw_send::fifo full,now link to tail! q_num = [%d], ul_tx_q_status = [%d], ppdu_cnt[%d], tid num [%d]",
                                            uc_q_num, ul_tx_q_status, pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt, MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl));

            if (pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt > DMAC_MAX_FIFO_PUT)
            {
                OAM_WARNING_LOG4(0, OAM_SF_TX, "dmac_tx_hw_send::q_num = [%d], ul_tx_q_status = [%d], ppdu_cnt[%d], tid num [%d]",
                                 uc_q_num, ul_tx_q_status, pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt, MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl));
            }
        }
        else
        {
            /* 当队列满，不应当往后继续挂发送帧 */
            hal_get_all_tx_q_status(pst_hal_device, &ul_tx_q_status);

            OAM_WARNING_LOG4(0, OAM_SF_TX, "dmac_tx_hw_send::fifo full,now put back to tid!!! q_num = [%d], all_tx_q_status = [%u], ppdu_cnt[%d], tid num [%d].",
            uc_q_num, ul_tx_q_status, pst_hal_device->ast_tx_dscr_queue[uc_q_num].uc_ppdu_cnt, MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl));

            /* 发送队列满，重新将包挂回重传队列 */
            dmac_tx_hw_back_to_queue(pst_hal_device, pst_tx_dscr, pst_tx_ctl, pst_tx_dscr_list_hdr);
        }
    }
}
/*lint -restore */
/*lint -save -e613 */

OAL_STATIC oal_uint32  dmac_tx_data(
                                    dmac_vap_stru            *pst_dmac_vap,
                                    dmac_user_stru           *pst_dmac_user,
                                    mac_tx_ctl_stru          *pst_tx_ctl,
                                    oal_dlist_head_stru      *pst_tx_dscr_list_hdr,
                                    hal_tx_ppdu_feature_stru *pst_ppdu_feature,
                                    hal_tx_txop_alg_stru     *pst_txop_alg)
{
    hal_to_dmac_device_stru     *pst_hal_device;
    hal_tx_dscr_stru            *pst_tx_dscr;
    hal_tx_txop_feature_stru     st_txop_feature;
    oal_dlist_head_stru         *pst_dscr_entry;
    hal_tx_dscr_stru            *pst_dscr_temp = OAL_PTR_NULL;
    oal_uint8                    uc_ppdu_cnt;
    mac_tx_ctl_stru             *pst_cb = OAL_PTR_NULL;
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8    en_trace_pkt_type;
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    dmac_ftm_initiator_stru     *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;
#endif

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_tx_dscr_list_hdr->pst_next, hal_tx_dscr_stru, st_entry);

    /* 维测 */
#ifdef _PRE_WLAN_FEATURE_AP_PM
    dmac_tx_data_in_wow_mode(pst_hal_device, pst_dmac_vap, pst_tx_dscr_list_hdr);
#endif

#ifdef _PRE_WLAN_FEATURE_DBAC
    if ((pst_hal_device->uc_current_chan_number != pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number) &&
         (MAC_VAP_STATE_INIT != pst_dmac_vap->st_vap_base_info.en_vap_state))
    {
        OAM_ERROR_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_DBAC, "dmac_tx_data:put tx_dscr in wrong channel. vap chan:%d, hal chan:%d. vap state:%d",
                                pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number, pst_hal_device->uc_current_chan_number, pst_dmac_vap->st_vap_base_info.en_vap_state);
        oam_report_80211_frame(BROADCAST_MACADDR,
                            (oal_uint8*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl),
                            MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl),
                            (oal_uint8*)oal_netbuf_payload(pst_tx_dscr->pst_skb_start_addr),
                            (MAC_GET_CB_MPDU_LEN(pst_tx_ctl) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl)),
                            OAM_OTA_FRAME_DIRECTION_TYPE_TX);
        oam_report_backtrace();
    }
#endif

#ifdef _PRE_WLAN_CHIP_TEST
    DMAC_CHIP_TEST_CALL(dmac_test_lpm_txopps_set_partial_aid(pst_dmac_vap, pst_dmac_user, DMAC_TEST_STUB_BEGIN));
    DMAC_CHIP_TEST_CALL(dmac_test_lpm_smps_set_rate(pst_tx_ctl,pst_txop_alg,DMAC_TEST_STUB_BEGIN));
    DMAC_CHIP_TEST_CALL(dmac_test_dfx_set_tx_cnt(pst_txop_alg));
#endif

    /*如果用户处于节能状态，则需要考虑设置当前帧的more data*/
    if (OAL_TRUE == pst_dmac_user->bit_ps_mode)
    {
        dmac_psm_tx_set_more_data(pst_dmac_user, pst_tx_ctl);
    }

#ifdef _PRE_WLAN_FEATURE_STA_PM
    /* STA模式下根据节能模式设置power_mgmt位 */
    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        dmac_psm_tx_set_power_mgmt_bit(pst_dmac_vap, pst_tx_ctl);
        /* fast ps开启切协议状态到active,fast ps没开启直接切hal device自状态打开前端 */
        if (OAL_TRUE == dmac_is_sta_fast_ps_enabled(&pst_dmac_vap->st_sta_pm_handler))
        {
            dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_TX_MGMT, 0, OAL_PTR_NULL);
        }
        else
        {
            hal_device_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), HAL_DEVICE_EVENT_VAP_CHANGE_TO_ACTIVE, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
        }
    }
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV) //待验证03无beacon 0问题时，用_PRE_WLAN_MAC_BUGFIX_TSF_SYNC宏包裹
    dmac_sta_tsf_restore(pst_dmac_vap);
#endif

    dmac_tx_get_timestamp(pst_hal_device, pst_dmac_vap->pst_hal_vap, &pst_ppdu_feature->us_tsf);

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_GET_TSF);

#if ((WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS) && defined(_PRE_WLAN_FEATURE_TXBF_HT))
    if (WLAN_STAGGERED_SOUNDING == pst_txop_alg->st_rate.en_sounding_mode)
    {
        /*AMPDU的第一个MPDU帧控制字段（或者普通MPDU帧的控制字段）B15(Order)改为1，加上HTC部分，并且HTC字段的B22B23值是11 */
        dmac_tx_bf_add_ht_control_field(pst_hal_device, pst_tx_ctl, pst_tx_dscr_list_hdr, pst_txop_alg, pst_ppdu_feature);
    }
#endif

#ifdef _PRE_WLAN_CHIP_TEST
    DMAC_CHIP_TEST_CALL(dmac_test_sch_modify_txdscp_timestamp(&pst_ppdu_feature->us_tsf, g_st_dmac_test_mng.s_ct_sch_lifetime_delay_ms));
    DMAC_CHIP_TEST_CALL(dmac_test_sch_set_long_nav_enable(pst_tx_dscr));
#endif

    /* 获取txop feature */
    dmac_tx_data_get_txop(pst_dmac_vap, pst_dmac_user, &st_txop_feature, pst_tx_ctl);

    if ((MAC_GET_CB_FRAME_TYPE(pst_tx_ctl) == WLAN_CB_FRAME_TYPE_DATA) && (MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl) == MAC_DATA_EAPOL))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,"{dmac_tx_data: trace_eapol:en_cipher_protocol_type[%d]}", st_txop_feature.st_security.en_cipher_protocol_type);
    }
#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
    dmac_tx_data_update_rsp_frame(pst_hal_device, pst_dmac_vap, pst_txop_alg, MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl));
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    if ( (OAL_TRUE == hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_DYN_BW_ENABLE))
    #if defined(_PRE_FEATURE_WAVEAPP_CLASSIFY)
        && (OAL_FALSE == pst_hal_device->en_test_is_on_waveapp_flag)
    #endif
        && (WLAN_VHT_MODE == pst_dmac_user->st_user_base_info.en_protocol_mode
        || WLAN_VHT_ONLY_MODE == pst_dmac_user->st_user_base_info.en_protocol_mode ))
    {
        pst_txop_alg->st_rate.dyn_bandwidth_in_non_ht_exist = OAL_TRUE;
    }
#endif
#endif

    /* 填写发送参数,只有ampdu可以只填写第一个描述符 */
    if (OAL_TRUE == pst_ppdu_feature->uc_ampdu_enable)
    {
        hal_tx_ucast_data_set_dscr(pst_hal_device, pst_tx_dscr, &st_txop_feature, pst_txop_alg, pst_ppdu_feature);
        uc_ppdu_cnt = 1;
    }
    else  /*MPDU每个描述都需要填写*/
    {
         OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, pst_tx_dscr_list_hdr)
         {
            pst_dscr_temp = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);

            pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_dscr_temp->pst_skb_start_addr);

            /* 组播 广播数据帧调用hal_tx_umbcast_set_dscr接口填写描述符*/
            if (OAL_TRUE == MAC_GET_CB_IS_MCAST(pst_cb))
            {
                hal_tx_non_ucast_data_set_dscr(pst_hal_device, pst_dscr_temp, &st_txop_feature, &(pst_dmac_vap->st_tx_data_mcast), pst_ppdu_feature);
            }
            else
            {
                hal_tx_ucast_data_set_dscr(pst_hal_device, pst_dscr_temp, &st_txop_feature, pst_txop_alg, pst_ppdu_feature);
            }
#ifdef _PRE_DEBUG_MODE
            en_trace_pkt_type = wifi_pkt_should_trace( pst_dscr_temp->pst_skb_start_addr, MAC_GET_CB_FRAME_HEADER_LENGTH(pst_cb));
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_data::type%d sent to hw[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
#endif
         }
        uc_ppdu_cnt = MAC_GET_CB_MPDU_NUM(pst_tx_ctl);
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 包括共存场景下, 对EAPOL帧处理 */
    dmac_btcoex_tx_vip_frame(&(pst_dmac_vap->st_vap_base_info), pst_tx_dscr_list_hdr);
#endif

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_SET_DSCR);

#ifdef _PRE_WLAN_FEATURE_DFR
    dmac_tx_process_dfr(pst_hal_device);
#endif

#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
    dmac_tx_data_flush_cache(pst_tx_ctl, pst_tx_dscr_list_hdr);
#endif

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    if (OAL_SWITCH_ON == pst_dmac_vap->st_vap_base_info.bit_al_tx_flag)
    {
        dmac_tx_data_always_tx(pst_hal_device, pst_dmac_vap, pst_tx_dscr);
#ifdef _PRE_WLAN_FEATURE_FTM
        hal_set_ftm_cali(pst_hal_device, pst_tx_dscr, past_ftm_init[0].en_cali);
#endif
    }
#endif

#ifdef _PRE_WLAN_CHIP_TEST
    /* 适配成4地址 */
    DMAC_CHIP_TEST_CALL(dmac_test_set_addr4(pst_tx_dscr));
    DMAC_CHIP_TEST_CALL(dmac_test_set_dscr_software_retry(pst_hal_device, pst_tx_dscr));
    DMAC_CHIP_TEST_CALL(dmac_test_lpm_txopps_set_partial_aid(pst_dmac_vap, pst_dmac_user, DMAC_TEST_STUB_END));
    DMAC_CHIP_TEST_CALL(dmac_test_lpm_smps_set_rate(pst_tx_ctl,pst_txop_alg,DMAC_TEST_STUB_END));
#endif

#ifdef _PRE_DEBUG_MODE
    dmac_tx_debug_print(pst_hal_device, pst_dmac_vap, pst_dmac_user, pst_tx_dscr, pst_tx_ctl, uc_ppdu_cnt);
#endif

#ifdef _PRE_WLAN_FEATURE_PF_SCH
    /* 调度使用 */
    MAC_GET_CB_TIMESTAMP(pst_tx_ctl) = (oal_uint32)OAL_TIME_GET_STAMP_TS();
#endif

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
    /* update chip active user dlist. */
    dmac_user_update_active_user_dlist(pst_dmac_user);
#endif

    if (OAL_SWITCH_ON == oam_report_data_get_global_switch(OAM_OTA_FRAME_DIRECTION_TYPE_TX))
    {
        dmac_tx_dump_data(pst_hal_device, pst_tx_dscr_list_hdr);
    }

    /* 挂硬件队列发送 */
    dmac_tx_hw_send(pst_hal_device, pst_tx_dscr, pst_tx_ctl, pst_tx_dscr_list_hdr, uc_ppdu_cnt);

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_PUT_DSCR);

    return OAL_SUCC;
}


oal_uint32  dmac_tid_tx_queue_remove_ampdu(
                hal_to_dmac_device_stru   *pst_hal_device,
                dmac_vap_stru             *pst_dmac_vap,
                dmac_user_stru            *pst_user,
                dmac_tid_stru             *pst_tid_queue,
                oal_uint8                  uc_mpdu_num)
{
    mac_device_stru            *pst_dev;
    hal_tx_dscr_stru           *pst_tx_dscr         = OAL_PTR_NULL;   /*当前处理的发送描述*/
    hal_tx_dscr_stru           *pst_tx_dscr_prev    = OAL_PTR_NULL;   /*前一个发送描述符*/
    hal_tx_txop_alg_stru       *pst_txop_alg        = OAL_PTR_NULL;
    mac_tx_ctl_stru            *pst_cb              = OAL_PTR_NULL;
    mac_tx_ctl_stru            *pst_cb_first        = OAL_PTR_NULL;
    oal_netbuf_stru            *pst_netbuf;
    hal_tx_ppdu_feature_stru    st_ppdu_feature;
    dmac_ba_tx_stru            *pst_tx_ba_handle;
    oal_uint16                  us_new_mpdu_max_num = 0;  /*本次调度可以新发送的mpdu数码*/
    oal_uint16                  us_pad_len          = 0;
    oal_uint16                  us_null_len         = 0;
    oal_uint16                  us_mpdu_len         = 0;
    oal_uint8                   uc_protocol_mode;
    oal_uint16                  us_ext_mpdu_len     = 0;  /*每个MPDU额外添加的长度*/
    oal_uint32                  ul_ampdu_length;
    oal_uint32                  ul_ret;
    oal_uint32                  ul_max_ampdu_length = 0;
    oal_uint8                   uc_retry_mpdu_num   = 0;  /*amdpu中重传mpdu的数目*/
    oal_uint8                   uc_mpdu_idx         = 0;
    oal_uint8                   uc_large_amsdu_num  = 0;
    oal_dlist_head_stru         st_dscr_head;
#ifdef _PRE_WLAN_MAC_BUGFIX_BTCOEX_SMALL_AGGR
    oal_uint32                  ul_have_short_packet = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_to_dmac_chip_stru       *pst_hal_chip        = OAL_PTR_NULL;
#endif
#endif

#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8   en_trace_pkt_type;
#endif

#ifdef _PRE_WLAN_11K_STAT
    oal_time_us_stru            st_time;
#endif

    pst_tx_ba_handle = pst_tid_queue->pst_ba_tx_hdl;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tx_ba_handle))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove_ampdu::pst_tx_ba_handle null.}");
        return OAL_FAIL;
    }

    /* 更新device结构体下的统计信息 */
    pst_dev = mac_res_get_dev(pst_user->st_user_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dev))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove_ampdu::pst_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化ppdu feature参数 */
    dmac_tx_init_ppdu_feature(pst_dmac_vap, pst_user, 0, &st_ppdu_feature);

    /* 获取最大AMPDU长度 */
    ul_max_ampdu_length = dmac_tx_get_ampdu_max_len(pst_user, pst_tid_queue);

    /*获取本次可以新发送mpdu的最大数目*/
    us_new_mpdu_max_num = dmac_tx_get_baw_remain(pst_hal_device, pst_tx_ba_handle);

    /* 获取每个MPDU的额外长度 */
    us_ext_mpdu_len = dmac_tx_get_mpdu_ext_len(pst_user);

#ifdef _PRE_WLAN_11K_STAT
    /* 获取系统时间 */
    oal_time_get_stamp_us(&st_time);
#endif

    oal_dlist_init_head(&st_dscr_head);

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_GET_INFO);
    while (uc_mpdu_idx != uc_mpdu_num)
    {
        /* TID队列包出队: 出队顺序:先重传队列，再netbuf队列 */
        pst_tx_dscr = dmac_tx_dequeue(pst_hal_device, pst_dmac_vap, pst_tid_queue, &st_dscr_head);
        if (OAL_PTR_NULL == pst_tx_dscr)
        {
            break;
        }

        pst_tx_dscr->bit_is_first = 0;

        if (pst_tid_queue->uc_retry_num) /*重传包*/
        {
            if(OAL_FALSE == dmac_tid_tx_retry_in_baw(pst_hal_device, pst_dev, pst_tx_dscr, pst_tid_queue))
            {
                continue;
            }

            pst_tid_queue->uc_retry_num--;
            /* 调度出的都为重传帧，且字节长度超过协商值时，uc_retry_mpdu_num有可能大于uc_mpdu_idx*/
            uc_retry_mpdu_num++;
        }
        else /*非重传包*/
        {
            /* 如果窗口被塞满，则退出循环 */
            if ((uc_mpdu_idx - uc_retry_mpdu_num) == us_new_mpdu_max_num)
            {
                dmac_tx_tid_enqueue(pst_tid_queue, &st_dscr_head, pst_tx_dscr);
                break;
            }
        }

        us_mpdu_len = pst_tx_dscr->us_original_mpdu_len + us_ext_mpdu_len;

        /* 判断长度是否超过协商值 */
        ul_ampdu_length = st_ppdu_feature.ul_ampdu_length + us_mpdu_len + us_pad_len + us_null_len;
        if (ul_ampdu_length > ul_max_ampdu_length)
        {
            dmac_tx_tid_enqueue(pst_tid_queue, &st_dscr_head, pst_tx_dscr);
            break;
        }

#if defined(_PRE_WLAN_MAC_BUGFIX_BTCOEX_SMALL_AGGR) && (_PRE_WLAN_FEATURE_BTCOEX)
        pst_hal_chip = DMAC_VAP_GET_HAL_CHIP(pst_dmac_vap);

        /* 判断长度是否超过协商值 */
        if (us_mpdu_len < BT_WLAN_COEX_SMALL_PKT_THRES)
        {
            ul_have_short_packet = OAL_TRUE;
        }

        /* BT共存情况下 判断长度是否超过芯片门限 */
        if ((OAL_TRUE == ul_have_short_packet))
        {
            if (pst_hal_chip->st_btcoex_btble_status.un_bt_status.st_bt_status.bit_bt_on)
            {
                if (ul_ampdu_length > BT_WLAN_COEX_SMALL_FIFO_THRES)
                {
                    dmac_tx_tid_enqueue(pst_tid_queue, &st_dscr_head, pst_tx_dscr);
                    break;
                }

                if (pst_tx_dscr->us_original_mpdu_len < BT_WLAN_COEX_UNAVAIL_PAYLOAD_THRES)
                {
                    OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove_ampdu::ampdu length bug, length is %d}", pst_tx_dscr->us_original_mpdu_len);
                }
            }
        }
#endif

        pst_netbuf              = pst_tx_dscr->pst_skb_start_addr;
        pst_cb                  = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        dmac_tx_tid_encap_amsdu(pst_dmac_vap, pst_user, pst_netbuf);
#endif

#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace(OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove_ampdu::type%d ampdu send[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif

        if (0 != uc_mpdu_idx)/* 非首包link */
        {
           if ((OAL_TRUE == pst_tx_dscr->bit_is_ampdu) && (OAL_TRUE == pst_tx_dscr->bit_is_first))
           {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_tid_tx_queue_remove_ampdu::Now link a ampdu head!! mpdu_idx %d", uc_mpdu_idx);
           }
           /*描述符依次link*/
           hal_tx_ctrl_dscr_link(pst_hal_device, pst_tx_dscr_prev, pst_tx_dscr);
        }
        else/* 首包处理 */
        {
            /* 设置首包CB */
            pst_cb_first = pst_cb;
            dmac_tx_set_first_cb(pst_cb, pst_user);

            /* 获取TXOP算法发送参数 */
            dmac_tx_get_txop_alg_params(pst_dmac_vap, pst_user, pst_cb, &pst_txop_alg);

            OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_AGGR_PREPARE);
            OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_AGGR_PREPARE);

            /* 查表找min mpdu len */
            st_ppdu_feature.us_min_mpdu_length = dmac_ba_calculate_min_mpdu_len(pst_user, pst_txop_alg);

            OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_CALCULATE_MINLEN);
            OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_CALCULATE_MINLEN);
        }

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
        if(OAL_FALSE == pst_hal_device->en_ampdu_tx_hw_en)
#endif
        {
            /* 计算AMPDU总长度 */
            dmac_tx_ampdu_calculate_total_len(&st_ppdu_feature, us_mpdu_len, us_pad_len, us_null_len);

            if (OAL_TRUE == mac_frame_is_null_data(pst_netbuf))
            {
                if ((WLAN_80211_CIPHER_SUITE_CCMP == pst_user->st_user_base_info.st_key_info.en_cipher_type) ||
                    (WLAN_80211_CIPHER_SUITE_GCMP == pst_user->st_user_base_info.st_key_info.en_cipher_type) ||
                    (WLAN_80211_CIPHER_SUITE_GCMP_256 == pst_user->st_user_base_info.st_key_info.en_cipher_type) ||
                    (WLAN_80211_CIPHER_SUITE_CCMP_256 == pst_user->st_user_base_info.st_key_info.en_cipher_type))
                {
                    st_ppdu_feature.ul_ampdu_length -= us_ext_mpdu_len;
                }
            }

            /*非重传包才设置seq num并调整窗口*/
            if (OAL_FALSE == pst_tx_dscr->bit_is_retried)
            {
                dmac_tx_seqnum_set_ampdu(pst_hal_device, pst_user, pst_cb, pst_tx_dscr);
                /*发送窗口调整*/
                dmac_ba_addto_baw(pst_tx_ba_handle);
            }
        }

        /* 计算当前MPDU PAD长度 */
        dmac_tx_ampdu_calculate_pad_len(&st_ppdu_feature, us_mpdu_len, &us_pad_len, &us_null_len);

        /* 获取下一个mpdu */
        pst_tx_dscr_prev = pst_tx_dscr;
        uc_mpdu_idx++;
    #ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        /* AMPDU中AMSDU子帧数 */
        if ((OAL_FALSE == pst_tx_dscr->bit_is_retried) &&
            (OAL_TRUE == MAC_GET_CB_IS_LARGE_SKB_AMSDU(pst_cb)))
        {
            uc_large_amsdu_num++;
        }
    #endif
        dmac_tx_data_statistics(pst_dmac_vap, pst_user, pst_cb, pst_tx_dscr);
#ifdef _PRE_WLAN_11K_STAT
        dmac_user_stat_tid_delay(pst_user, pst_dmac_vap, pst_cb, &st_time);
#endif
    }

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_LINK_MPDUS);

    /*确认是否有mpdu发送*/
    if (0 == uc_mpdu_idx)
    {
        dmac_free_tx_dscr_queue(pst_dmac_vap, &st_dscr_head);
        return OAL_SUCC;
    }
    else
    {
        /* 设置队列尾dscr next为空 */
        if (OAL_PTR_NULL != pst_tx_dscr_prev)
        {
            hal_tx_ctrl_dscr_unlink(pst_hal_device, pst_tx_dscr_prev);
        }

        /*更新计数*/
        dmac_tx_tid_update_stat(&st_ppdu_feature, pst_tid_queue, pst_dev, uc_mpdu_idx, uc_large_amsdu_num);
        /*更新cb计数*/
        if (OAL_PTR_NULL != pst_cb_first)
        {
            MAC_GET_CB_MPDU_NUM(pst_cb_first) = st_ppdu_feature.uc_mpdu_num;
        }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        DMAC_SND_PKT_STAT_CNT(uc_mpdu_idx - uc_retry_mpdu_num);
#endif
    }


#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    if(OAL_FALSE == pst_hal_device->en_ampdu_tx_hw_en)
#endif
    {
        if (OAL_PTR_NULL != pst_txop_alg)
        {
            uc_protocol_mode = pst_txop_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode;
            /* 11ac AMPDU最后一个子MPDU需要有pading */
            if (WLAN_VHT_PHY_PROTOCOL_MODE == uc_protocol_mode)
            {
                st_ppdu_feature.ul_ampdu_length += us_pad_len;
            }
        }
    }

    if (DMAC_TX_MODE_RIFS != pst_tid_queue->en_tx_mode)
    {
        st_ppdu_feature.uc_ampdu_enable = OAL_TRUE;
        st_ppdu_feature.uc_tx_ampdu_session_index = pst_tid_queue->pst_ba_tx_hdl->uc_tx_ba_lut;
    }
    else
    {
        ul_ret = dmac_tx_rifs_process(pst_hal_device, pst_dmac_vap, pst_tid_queue, &st_ppdu_feature, &st_dscr_head);
        if (OAL_SUCC != ul_ret)
        {
            st_ppdu_feature.uc_ampdu_enable = OAL_TRUE;
        }
    }

    /* 在启用基于报文长度threshold的RTS策略时，根据AMPDU长度判断是否开启RTS (该策略默认不启用，根据产品需求手动开启)*/
    if ((OAL_TRUE == pst_dmac_vap->st_vap_base_info.bit_use_rts_threshold) && (OAL_PTR_NULL != pst_txop_alg))
    {
        oal_bool_enum_uint8 en_rts_enable;

        en_rts_enable = (st_ppdu_feature.ul_ampdu_length >= mac_mib_get_RTSThreshold(&pst_dmac_vap->st_vap_base_info)) ? OAL_TRUE : OAL_FALSE;
        pst_txop_alg->ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = en_rts_enable;
        pst_txop_alg->ast_per_rate[1].rate_bit_stru.bit_rts_cts_enable = en_rts_enable;
        pst_txop_alg->ast_per_rate[2].rate_bit_stru.bit_rts_cts_enable = en_rts_enable;
        pst_txop_alg->ast_per_rate[3].rate_bit_stru.bit_rts_cts_enable = en_rts_enable;
    }

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_AMPDU_REMOVE_QUEUE);
    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_AMPDU_REMOVE_QUEUE);

    dmac_tx_data(pst_dmac_vap, pst_user, pst_cb_first, &st_dscr_head, &st_ppdu_feature, pst_txop_alg);

    return OAL_SUCC;
}
/*lint -restore */

oal_uint32  dmac_tid_tx_queue_remove(hal_to_dmac_device_stru   *pst_hal_device,
                                     dmac_vap_stru             *pst_dmac_vap,
                                     dmac_tid_stru             *pst_tid_queue,
                                     oal_uint8                  uc_dscr_num)
{
    mac_device_stru            *pst_dev = OAL_PTR_NULL;
    dmac_user_stru             *pst_user = OAL_PTR_NULL;
    oal_dlist_head_stru        *pst_dscr_entry  = OAL_PTR_NULL;
    hal_tx_dscr_stru           *pst_tx_dscr = OAL_PTR_NULL;
    hal_tx_txop_alg_stru       *pst_txop_alg = OAL_PTR_NULL;
    mac_tx_ctl_stru            *pst_cb = OAL_PTR_NULL;
    oal_netbuf_stru            *pst_netbuf = OAL_PTR_NULL;
    hal_tx_ppdu_feature_stru    st_ppdu_feature;
    oal_uint8                   uc_dscr_idx;
    hal_tx_dscr_stru           *pst_tx_dscr_prev = OAL_PTR_NULL;
    oal_dlist_head_stru         st_dscr_head;
    oal_uint8                   uc_vap_id;
    oal_uint8                   uc_dscr_num_tmp;
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8   en_trace_pkt_type;
#endif
#ifndef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_dlist_head_stru        *pst_entry;
#endif

#ifdef _PRE_WLAN_11K_STAT
    oal_time_us_stru            st_time;
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_DFT_STAT
    dmac_tid_stats_stru *pst_tid_stats;
    pst_tid_stats = pst_tid_queue->pst_tid_stats;
    if (OAL_PTR_NULL == pst_tid_stats)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tid_tx_queue_remove::tid_stats is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
#endif
    uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;
    pst_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_tid_queue->us_user_idx);
    if (OAL_PTR_NULL == pst_user)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove::pst_user[%d] null.}", pst_tid_queue->us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新device结构体下的统计信息 */
    pst_dev = mac_res_get_dev(pst_user->st_user_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dev)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove::pst_dev null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TX_MPDU_REMOVE);

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        /* 11b amsdu的问题，02没开amsdu，暂时todo */
#else
    if (WLAN_LEGACY_11B_MODE == pst_user->st_user_base_info.en_cur_protocol_mode)
    {
        /* 遍历tid, 丢掉amsdu报文 */
        /* 释放TID缓存中的包 */
        pst_entry = pst_tid_queue->st_hdr.pst_next;
        if (OAL_PTR_NULL != pst_entry)
        {
            while (pst_entry != &pst_tid_queue->st_hdr)
            {
                pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_entry, hal_tx_dscr_stru, st_entry);
                pst_netbuf = pst_tx_dscr->pst_skb_start_addr;
                pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
                pst_entry = pst_entry->pst_next;

                if (OAL_TRUE == pst_cb->bit_is_amsdu)
                {
                    oal_dlist_delete_entry(&pst_tx_dscr->st_entry);
                    dmac_tid_tx_dequeue_update(pst_dev, pst_tid_queue, 1);
                    dmac_free_tx_dscr(pst_tx_dscr);
                }
            }
        }
    }
#endif

    oal_dlist_init_head(&st_dscr_head);

    uc_dscr_num_tmp = uc_dscr_num;

    /* 先从重传队列中取 */
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    for (uc_dscr_idx = 0; uc_dscr_idx < uc_dscr_num; uc_dscr_idx++)
    {
        if (OAL_TRUE == oal_dlist_is_empty(&(pst_tid_queue->st_retry_q)))
        {
            break;
        }

        pst_dscr_entry = oal_dlist_delete_head(&(pst_tid_queue->st_retry_q));
        pst_tx_dscr    = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        oal_dlist_add_tail(pst_dscr_entry, &st_dscr_head);

        if (0 != uc_dscr_idx)
        {

            if ((OAL_TRUE == pst_tx_dscr->bit_is_ampdu) && (OAL_TRUE == pst_tx_dscr->bit_is_first))
            {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_tid_tx_queue_remove::Now link a ampdu head from retry_q!! mpdu_idx %d", uc_dscr_idx);
            }
            hal_tx_ctrl_dscr_link(pst_hal_device, pst_tx_dscr_prev, pst_tx_dscr);
        }
        pst_tx_dscr_prev = pst_tx_dscr;

        dmac_tid_tx_dequeue(pst_hal_device, pst_dmac_vap, pst_tid_queue, pst_tx_dscr, pst_dev, pst_user);
        pst_netbuf = pst_tx_dscr->pst_skb_start_addr;
        pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove::type%d mpdu re send[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif
    }

    /* 减掉重传报文数 */
    uc_dscr_num -= uc_dscr_idx;
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

    /* 再从TID队列取 */
    for (uc_dscr_idx = 0; uc_dscr_idx < uc_dscr_num; uc_dscr_idx++)
    {
    #ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        if (OAL_TRUE == oal_netbuf_list_empty(&pst_tid_queue->st_buff_head))
        {
            break;
        }

        pst_netbuf  = dmac_tx_dequeue_first_mpdu(&pst_tid_queue->st_buff_head);
        pst_tx_dscr = dmac_tx_dscr_alloc(pst_hal_device, pst_dmac_vap, pst_netbuf);
        if (OAL_PTR_NULL == pst_tx_dscr)
        {
            OAM_WARNING_LOG3(uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove::put the pkt back into tid netbuf Q, alloc tx dscr failed. retry = %d, mpdu = %d, uc_mpdu_num = %d}",
                             pst_tid_queue->uc_retry_num, pst_tid_queue->us_mpdu_num, uc_dscr_num);

            dmac_tx_queue_mpdu(pst_netbuf, &pst_tid_queue->st_buff_head);
            break;
        }
        oal_dlist_add_tail(&pst_tx_dscr->st_entry, &st_dscr_head);
    #else
        /* 如果tid缓存队列为空，程序直接退出 */
        if (oal_dlist_is_empty(&(pst_tid_queue->st_hdr)))
        {
            //OAM_INFO_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove::empty tid.}\r\n");
            break;
        }

        pst_dscr_entry = oal_dlist_delete_head(&(pst_tid_queue->st_hdr));
        pst_tx_dscr    = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        oal_dlist_add_tail(pst_dscr_entry, &st_dscr_head);

    #endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
        if (0 != uc_dscr_idx)
        {

            if ((OAL_TRUE == pst_tx_dscr->bit_is_ampdu) && (OAL_TRUE == pst_tx_dscr->bit_is_first))
            {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_tid_tx_queue_remove::Now link a ampdu head from tid_queue!! mpdu_idx %d", uc_dscr_idx);
            }
            hal_tx_ctrl_dscr_link(pst_hal_device, pst_tx_dscr_prev, pst_tx_dscr);
        }

        pst_tx_dscr_prev = pst_tx_dscr;

        dmac_tid_tx_dequeue(pst_hal_device, pst_dmac_vap, pst_tid_queue, pst_tx_dscr, pst_dev, pst_user);
        pst_netbuf = pst_tx_dscr->pst_skb_start_addr;
        pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove::type%d mpdu send[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif

#ifdef _PRE_WLAN_11K_STAT
    /* 获取系统时间 */
    oal_time_get_stamp_us(&st_time);
    dmac_user_stat_tid_delay(pst_user, pst_dmac_vap, pst_cb, &st_time);
#endif
    }

    if (oal_dlist_is_empty(&st_dscr_head))
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_remove::no packets can be transmitted.}");
        return OAL_FAIL;
    }

    if (OAL_PTR_NULL != pst_tx_dscr_prev)
    {
        hal_tx_ctrl_dscr_unlink(pst_hal_device, pst_tx_dscr_prev);
    }
    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TX_INIT_PPDU);

    /* 初始化ppdu feature参数 */
    dmac_tx_init_ppdu_feature(pst_dmac_vap, pst_user, 1, &st_ppdu_feature);
    hal_tx_ctrl_dscr_unlink(pst_hal_device, pst_tx_dscr);

    if (OAL_PTR_NULL == pst_cb)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tid_tx_queue_remove::pst_cb null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    MAC_GET_CB_MPDU_NUM(pst_cb) = uc_dscr_num_tmp;

    /* 获取TXOP算法发送参数 */
    dmac_tx_get_txop_alg_params(pst_dmac_vap, pst_user, pst_cb, &pst_txop_alg);

    /* 2040共存特性更新带宽模式 */
    /* dmac_tx_update_bandwidth_mode(pst_dmac_vap, pst_user, pst_txop_alg); */

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_MPDU_REMOVE_QUEUE);

    dmac_tx_data(pst_dmac_vap, pst_user, pst_cb, &st_dscr_head, &st_ppdu_feature, pst_txop_alg);

    OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TX_MPDU_REMOVE_END);
    return OAL_SUCC;
}


oal_uint32  dmac_tx_process_data(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_tx_ctl_stru             *pst_tx_ctl_first;
    dmac_user_stru              *pst_dmac_user;
    dmac_tid_stru               *pst_tid_queue;
    hal_tx_dscr_stru            *pst_tx_dscr = OAL_PTR_NULL;
    mac_device_stru             *pst_mac_device;
    oal_dlist_head_stru          st_tx_dscr_list_hdr;
    hal_tx_ppdu_feature_stru     st_ppdu_feature;
    hal_tx_txop_alg_stru        *pst_txop_alg = OAL_PTR_NULL;
    oal_uint8                    uc_hal_q;
    wlan_wme_ac_type_enum_uint8  uc_ac;
    oal_uint32                   ul_ret = OAL_SUCC;
#ifndef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_uint16                   us_tx_dscr_len = 0;
    hal_tx_mpdu_stru             st_mpdu;
#endif

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_DMAC_DATA);

    /* 获取netbuf链中第一个netbuf的CB */
    pst_tx_ctl_first  = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    if(OAL_PTR_NULL == pst_tx_ctl_first)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_process_data:oal_netbuf_cb::skb buf poiter null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_tx_process_data::pst_mac_device[%d] null!}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取DMAC模块用户结构体 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_tx_ctl_first));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data::pst_dmac_user null.user idx [%d]}", MAC_GET_CB_TX_USER_IDX(pst_tx_ctl_first));
        OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_mpdu_dropped, 1);
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    DMAC_RCV_PKT_STAT_CNT(1);
#endif

    /* 发送检查状态 */
    if(OAL_FALSE == dmac_tx_is_tx_force(pst_dmac_vap))
    {
        if(OAL_TRUE != dmac_is_vap_state_ok(pst_dmac_vap)
        || OAL_TRUE != dmac_is_user_state_ok(pst_dmac_user))
        {
            dmac_tx_excp_free_netbuf(pst_netbuf);
            return OAL_SUCC;
        }
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    /* 非活跃用户处理 */
    if (OAL_UNLIKELY(OAL_FALSE == pst_dmac_user->bit_active_user))
    {
        ul_ret = dmac_user_tx_inactive_user_handler(pst_dmac_user);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_process_data::dmac_user_tx_inactive_user_handler failed[%d].", ul_ret);
            OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_mpdu_dropped, 1);
            return ul_ret;
        }
    }
#endif

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_DMAC_INACTIVE_USER_HDL);

    /* 调用拥塞控制的钩子 */
    ul_ret = dmac_alg_downlink_flowctl_notify(pst_mac_device, pst_dmac_vap, &pst_dmac_user->st_user_base_info, pst_netbuf);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        /* 如果拥塞控制算法判断为丢包*/
        OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_alg_process_dropped, 1);
        OAM_INFO_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                      "{dmac_tx_process_data:: alg_flowctl process return [%d], dev_mpdu_num = %d}", ul_ret, pst_mac_device->us_total_mpdu_num);
        OAM_INFO_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data:: be = %d, bk = %d, vi = %d, vo = %d}",
                      pst_mac_device->aus_ac_mpdu_num[WLAN_WME_AC_BE],
                      pst_mac_device->aus_ac_mpdu_num[WLAN_WME_AC_BK],
                      pst_mac_device->aus_ac_mpdu_num[WLAN_WME_AC_VI],
                      pst_mac_device->aus_ac_mpdu_num[WLAN_WME_AC_VO]);
#ifdef _PRE_WLAN_PERFORM_STAT
        dmac_stat_tid_per(&(pst_dmac_user->st_user_base_info), MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl_first), 1, 1, DMAC_STAT_PER_BUFF_OVERFLOW);
#endif
#ifdef _PRE_WLAN_11K_STAT
        dmac_user_stat_tx_dropped_mpdu_num(pst_dmac_user, pst_dmac_vap, pst_mac_device, MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl_first));
#endif
        return ul_ret;
    }

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_DMAC_FLOWCTRL);

#ifdef _PRE_WLAN_FEATURE_UAPSD
    /* 只有发送分片报文才会循环 */
    if(OAL_TRUE == dmac_uapsd_tx_need_enqueue(pst_dmac_vap, pst_dmac_user, pst_tx_ctl_first))
    {
        return dmac_uapsd_tx_enqueue(pst_dmac_vap, pst_dmac_user, pst_netbuf);
    }
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 判断要被发送的帧是否需要入节能缓存队列,判断条件1.AP有节能用户，2.P2P noa节能 */
    if ((0 != pst_dmac_vap->uc_ps_user_num) || ((OAL_TRUE == (oal_uint8)IS_P2P_PS_ENABLED(pst_dmac_vap))&&(OAL_TRUE == pst_mac_device->st_p2p_info.en_p2p_ps_pause)))
#else
    if (0 != pst_dmac_vap->uc_ps_user_num)
#endif
    {
        if (OAL_TRUE == dmac_psm_pkt_need_buff(pst_mac_device, pst_dmac_vap, pst_dmac_user, pst_netbuf))
        {
            return dmac_psm_ps_enqueue(pst_dmac_vap, pst_dmac_user, pst_netbuf);
        }
    }

    /* HAL Q NUM */
    uc_ac = MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl_first);
    uc_hal_q  = HAL_AC_TO_Q_NUM(uc_ac);

#ifndef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    /* 获取MPDU发送参数 */
    ul_ret = dmac_tx_get_mpdu_params(pst_netbuf, pst_tx_ctl_first, &st_mpdu);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data::dmac_tx_get_mpdu_params failed[%d].", ul_ret);
        OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_mpdu_dropped, 1);
        return OAL_FAIL;
    }

    us_tx_dscr_len = (MAC_GET_CB_NETBUF_NUM(pst_tx_ctl_first) == 1) ? WLAN_MEM_SHARED_TX_DSCR_SIZE1 : WLAN_MEM_SHARED_TX_DSCR_SIZE2;

    /* 申请发送描述符内存 */
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_tx_dscr = (hal_tx_dscr_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_TX_DSCR_1, us_tx_dscr_len, OAL_FALSE);
#else
    pst_tx_dscr = (hal_tx_dscr_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_SHARED_DSCR, us_tx_dscr_len, OAL_FALSE);
#endif
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tx_dscr))
    {
        if (WLAN_MEM_SHARED_TX_DSCR_SIZE1 == us_tx_dscr_len)
        {
            OAM_ERROR_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
            "{dmac_tx_process_data::pst_tx_dscr alloc failed, us_tx_dscr_len[%d], NETBUF_NUM[%d], amsdu capable[%d].}",
            us_tx_dscr_len, MAC_GET_CB_NETBUF_NUM(pst_tx_ctl_first), MAC_GET_CB_IS_AMSDU(pst_tx_ctl_first));
        }
        else
        {
            
            OAM_INFO_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
            "{dmac_tx_process_data::pst_tx_dscr alloc failed, us_tx_dscr_len[%d], NETBUF_NUM[%d], amsdu capable[%d].}",
            us_tx_dscr_len, MAC_GET_CB_NETBUF_NUM(pst_tx_ctl_first), MAC_GET_CB_IS_AMSDU(pst_tx_ctl_first));
        }

        OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_mpdu_dropped, 1);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && defined(_PRE_WLAN_DFT_STAT)
        dmac_dft_print_mac_phy_rf(pst_hal_device);
#endif
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    OAL_MEM_TRACE(pst_tx_dscr, OAL_FALSE);
    OAL_MEMZERO(pst_tx_dscr, us_tx_dscr_len);
    /* 填写mpdu长度 包括帧头 */
    pst_tx_dscr->us_original_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl_first) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl_first);

    /* 填写描述符ac */
    pst_tx_dscr->uc_q_num = uc_hal_q;

    hal_tx_fill_basic_ctrl_dscr(pst_hal_device, pst_tx_dscr, &st_mpdu);
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

    /* 获取用户的特定TID队列 */
    pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl_first)]);

    /* 判断数据帧是否需要入发送TID缓存队列 */
    if (OAL_TRUE == dmac_tx_need_enqueue_tid(pst_mac_device, pst_dmac_vap, pst_tid_queue, uc_hal_q))
    {
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        ul_ret = dmac_tid_tx_queue_enqueue(pst_mac_device, &pst_dmac_vap->st_vap_base_info, pst_tid_queue, 1, pst_netbuf);
#else
        ul_ret = dmac_tid_tx_queue_enqueue(pst_mac_device, &pst_dmac_vap->st_vap_base_info, pst_tid_queue, pst_tx_dscr, 1, pst_netbuf);
#endif
        if (OAL_LIKELY(OAL_SUCC == ul_ret))
        {
            /* 入队成功通知算法 */
            dmac_alg_tid_update_notify(pst_tid_queue);
        }
        else
        {
#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
            pst_dmac_vap->st_vap_base_info.st_vap_stats.ul_tx_dropped_packets += MAC_GET_CB_NETBUF_NUM(pst_tx_ctl_first);
#endif
#ifndef _PRE_WLAN_FEATURE_TX_DSCR_OPT
            OAL_MEM_FREE((oal_void *)pst_tx_dscr, OAL_TRUE);
#endif
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data::dmac_tid_tx_queue_enqueue fail.}");
            /* 入队失败，但是返回SUCC，因此要释放netbuf */
            dmac_tx_excp_free_netbuf(pst_netbuf);
        }

        OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_DMAC_TID_ENQUEUE);
        OAM_PROFILING_TX_STATISTIC(OAL_PTR_NULL, OAM_PROFILING_FUNC_TID_ENQUEUE);

#ifdef _PRE_WIFI_DMT
        /* AMPDU聚合由软件硬件条件判断，需要增加聚合数来模拟聚合条件开关 */
        if (HAL_NEED_TX_SCHEDULE(pst_hal_device, pst_tid_queue, uc_hal_q) && (pst_tid_queue->us_mpdu_num >= witp_dmt_get_ampdu_aggr_num()))
#else
        if (HAL_NEED_TX_SCHEDULE(pst_hal_device, pst_tid_queue, uc_hal_q))
#endif
        {
            dmac_tx_complete_schedule(pst_hal_device, MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl_first));
        }

        return OAL_SUCC;
    }

    /*如果硬件处于复位中，不发送*/
    if(OAL_TRUE == MAC_DEV_IS_RESET_IN_PROGRESS(pst_mac_device))
    {
       OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data::MAC_DEV_IS_RESET_IN_PROGRESS.}");
       OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_mpdu_dropped, 1);
#ifndef _PRE_WLAN_FEATURE_TX_DSCR_OPT
       OAL_MEM_FREE((oal_void *)pst_tx_dscr, OAL_TRUE);
#endif
       return OAL_ERR_CODE_RESET_INPROGRESS;
    }

#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 漫游状态发送帧时，如果发送帧VAP所在信道非当前工作信道则释放帧 *//* 5G发11b,后期可删 */
    if (OAL_FALSE == dmac_tx_enable_in_roaming(pst_hal_device, pst_dmac_vap))
    {
        OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_mpdu_dropped, 1);
#ifndef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        OAL_MEM_FREE((oal_void *)pst_tx_dscr, OAL_TRUE);
#endif
        return OAL_FAIL;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    pst_tx_dscr = dmac_tx_dscr_alloc(pst_hal_device, pst_dmac_vap, pst_netbuf);
    if (OAL_PTR_NULL == pst_tx_dscr)
    {
        /* null data不需要缓存 */
        if (OAL_TRUE == mac_frame_is_null_data(pst_netbuf))
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data::null data pkt is dropped, alloc tx dscr failed.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
        /* 常发模式下不入队 */
        if (OAL_SWITCH_ON == pst_dmac_vap->st_vap_base_info.bit_al_tx_flag)
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data::al tx , alloc tx dscr failed.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
#endif
        if (OAL_SUCC != dmac_tid_tx_queue_enqueue(pst_mac_device, &pst_dmac_vap->st_vap_base_info, pst_tid_queue, 1, pst_netbuf))
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data::alloc tx dscr failed, enqueue tid queue fail, pkt is dropped.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        /* 申请不到描述符，但入TID成功 */
        dmac_alg_tid_update_notify(pst_tid_queue);
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_process_data::alloc tx dscr failed, pkt need enqueue tid queue.}");
        return OAL_SUCC;
    }
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

    /* 初始化发送描述符双项链表 */
    oal_dlist_init_head(&st_tx_dscr_list_hdr);

    /* 将同一个需要发送的描述符加入描述符链表 */
    oal_dlist_add_tail(&pst_tx_dscr->st_entry, &st_tx_dscr_list_hdr);

    /* 单播帧描述符中的字段要置空 */
    hal_tx_ctrl_dscr_unlink(pst_hal_device, pst_tx_dscr);

    /* 初始化ppdu发送参数 */
    dmac_tx_init_ppdu_feature(pst_dmac_vap, pst_dmac_user, 1, &st_ppdu_feature);

    /* 获取TXOP算法发送参数 */
    dmac_tx_get_txop_alg_params(pst_dmac_vap, pst_dmac_user, pst_tx_ctl_first, &pst_txop_alg);

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    if (OAL_SWITCH_ON == pst_dmac_vap->st_vap_base_info.bit_al_tx_flag)
    {
        pst_txop_alg->st_rate.bit_lsig_txop = OAL_FALSE;
    }
#endif

    /* set mpdu num & seq num */
    MAC_GET_CB_MPDU_NUM(pst_tx_ctl_first) = 1;
    dmac_save_frag_seq(pst_dmac_user, pst_tx_ctl_first);
    dmac_tx_seqnum_set(pst_hal_device, pst_dmac_user, pst_tx_ctl_first, pst_tx_dscr);

    dmac_tx_data_statistics(pst_dmac_vap, pst_dmac_user, pst_tx_ctl_first, pst_tx_dscr);

    OAL_MIPS_TX_STATISTIC(DMAC_PROFILING_FUNC_TX_DMAC_FILL_DSCR);

    /* 正常发送流程 */
    dmac_tx_data(pst_dmac_vap, pst_dmac_user, pst_tx_ctl_first, &st_tx_dscr_list_hdr, &st_ppdu_feature, pst_txop_alg);

    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rodata, code, rwdata, zidata  // return to default placement
#endif


oal_uint32  dmac_tx_mgmt(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf_mgmt, oal_uint16 us_len)
{
    mac_tx_ctl_stru                *pst_tx_ctl;
    hal_tx_dscr_stru               *pst_mgmt_dscr;
    hal_tx_mpdu_stru                st_mpdu;
    hal_tx_txop_feature_stru        st_txop_feature     = {{0},{0}};
    hal_tx_txop_alg_stru           *pst_txop_alg = OAL_PTR_NULL;
    hal_tx_txop_alg_stru            st_txop_alg;
    hal_tx_ppdu_feature_stru        st_ppdu_feature;
    hal_to_dmac_device_stru        *pst_hal_device;
    dmac_user_stru                 *pst_dmac_user;
    mac_ieee80211_frame_stru       *pst_mac_header;
    mac_device_stru                *pst_mac_device;
    oal_dlist_head_stru             st_tx_dscr_list_hdr;
#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
    oal_uint16                     us_cache_frame_len = 0;  /* 帧头加帧体长度 */
#endif
#ifdef _PRE_WLAN_FEATURE_AP_PM
    mac_pm_arbiter_stru            *pst_pm_arbiter;
#endif
    oal_uint8                       uc_mgmt_subtype;
    oal_uint8                       uc_mgmt_type;
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8       en_trace_pkt_type;
#endif

    if (OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_netbuf_mgmt)
    {
        OAM_ERROR_LOG2(0, OAM_SF_TX, "{dmac_tx_mgmt::param null, pst_dmac_vap=%d, pst_netbuf_mgmt=%d.}", pst_dmac_vap, pst_netbuf_mgmt);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_mgmt::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_mgmt::pst_mac_device null[%d].}", pst_hal_device->uc_mac_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_TRUE == MAC_DEV_IS_RESET_IN_PROGRESS(pst_mac_device))
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_mgmt::MAC_DEV_IS_RESET_IN_PROGRESS[%d].}", pst_hal_device->uc_mac_device_id);
        return OAL_ERR_CODE_RESET_INPROGRESS;
    }

    oal_set_netbuf_prev(pst_netbuf_mgmt, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_netbuf_mgmt, OAL_PTR_NULL);
    OAL_MEM_NETBUF_TRACE(pst_netbuf_mgmt, OAL_TRUE);

    pst_mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf_mgmt);

#if defined(_PRE_WLAN_FEATURE_DBAC) && defined(_PRE_WLAN_FEATRUE_DBAC_DOUBLE_AP_MODE)
    if (mac_is_dbac_enabled(pst_mac_device) && !mac_is_dbac_running(pst_mac_device)
        && (MAC_SCAN_STATE_RUNNING != pst_mac_device->en_curr_scan_state))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_mgmt::dorp pkt while dbac enabled but not running.}");
        oal_netbuf_free(pst_netbuf_mgmt);
        return OAL_SUCC;
    }
#endif

    if(WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        if(WLAN_AUTH == pst_mac_header->st_frame_control.bit_sub_type)
        {
            pst_dmac_vap->en_auth_received = OAL_FALSE;
        }
        if(WLAN_REASSOC_REQ == pst_mac_header->st_frame_control.bit_sub_type)
        {
            pst_dmac_vap->en_assoc_rsp_received = OAL_FALSE;
        }

    }

    // when working home channel while bgscan, scanning = true
    if ((MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state)
    && (WLAN_MANAGEMENT == pst_mac_header->st_frame_control.bit_type)
    && (WLAN_PROBE_REQ != pst_mac_header->st_frame_control.bit_sub_type)
    && (WLAN_PROBE_RSP != pst_mac_header->st_frame_control.bit_sub_type)
#ifdef _PRE_WLAN_FEATURE_FTM
    && (WLAN_ACTION != pst_mac_header->st_frame_control.bit_sub_type)
#endif
    && (pst_hal_device->uc_current_chan_number != pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number))
    {
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "{dmac_tx_mgmt::drop mgmt while scan, vap_state=%d type=%d}",
                pst_dmac_vap->st_vap_base_info.en_vap_state,
                pst_mac_header->st_frame_control.bit_sub_type);
        oal_netbuf_free(pst_netbuf_mgmt);

        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_FEATURE_AP_PM
    pst_pm_arbiter = (mac_pm_arbiter_stru*)(pst_mac_device->pst_pm_arbiter);
    if ((pst_pm_arbiter != OAL_PTR_NULL) && (OAL_TRUE == pst_mac_device->en_pm_enable)
        && (DEV_PWR_STATE_WOW == pst_pm_arbiter->uc_cur_state))
    {
        OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_mgmt::dorp pkt when device in wow.bit_type[%d], sub_type[%d], us_len[%d]}\n",
                pst_mac_header->st_frame_control.bit_type, pst_mac_header->st_frame_control.bit_sub_type, us_len);
        OAM_STAT_VAP_INCR(pst_dmac_vap->st_vap_base_info.uc_vap_id, tx_abnormal_msdu_dropped, 1);
        oal_netbuf_free(pst_netbuf_mgmt);
        return OAL_SUCC;
    }
#endif

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf_mgmt);

    /* init mpdu basic para, 1.保证发送出现异常时候，帧体上报等维测能正常把帧体打印出来;2.初始化基本字段，预防后续需要使用到基本字段 */
    dmac_get_mgmt_mpdu_param(pst_netbuf_mgmt, pst_tx_ctl, us_len, &st_mpdu);

    /* 查找用户，判断帧是否需要入节能队列，如果需要，则入队，发送流程不再往下走,
       如果用户找不到了也就不涉及节能，直接略过即可 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_tx_ctl));
    if (OAL_PTR_NULL != pst_dmac_user)
    {
#ifdef _PRE_WLAN_FEATURE_UAPSD
        if(OAL_TRUE == dmac_uapsd_tx_need_enqueue(pst_dmac_vap, pst_dmac_user, pst_tx_ctl))
        {
           return dmac_uapsd_tx_enqueue(pst_dmac_vap, pst_dmac_user, pst_netbuf_mgmt);
        }
#endif

        if (OAL_TRUE == dmac_psm_pkt_need_buff(pst_mac_device, pst_dmac_vap, pst_dmac_user, pst_netbuf_mgmt))
        {
            return dmac_psm_ps_enqueue(pst_dmac_vap, pst_dmac_user, pst_netbuf_mgmt);
        }

        /* 获取group id 和 partial aid */
        dmac_tx_set_txopps_param(pst_dmac_vap, pst_dmac_user, &st_txop_feature, MAC_GET_CB_IS_MCAST(pst_tx_ctl));
    }

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_mgmt_dscr = (hal_tx_dscr_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_TX_DSCR_1, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAL_TRUE);
#else
    pst_mgmt_dscr = (hal_tx_dscr_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_SHARED_DSCR, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAL_TRUE);
#endif
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mgmt_dscr))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_mgmt::pst_mgmt_dscr null.}");
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && defined(_PRE_WLAN_DFT_STAT)
        dmac_dft_print_mac_phy_rf(pst_hal_device);
#endif
        return OAL_FAIL;
    }

    OAL_MEM_TRACE(pst_mgmt_dscr, OAL_FALSE);
    OAL_MEMZERO(pst_mgmt_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1);

    /* set mgmt frame seq num */
    dmac_tx_mgmt_set_seq_num(pst_mac_header);

    /* init ppdu feature struct */
    dmac_tx_init_ppdu_feature(pst_dmac_vap, pst_dmac_user, 1, &st_ppdu_feature);
    /* 不是发给关连用户的管理桢，场景：AP响应未关连用户的probe req;
       王浩: TX描述幅ra lut index要填一个没用过的index, 此处填最大0x1F */
    if (MAC_INVALID_USER_ID == MAC_GET_CB_TX_USER_IDX(pst_tx_ctl))
    {
        st_ppdu_feature.st_ppdu_addr_index.uc_ra_lut_index = (oal_uint8)WLAN_INVALID_RA_LUT_IDX;
    }

    /* SET txop security feature */
    dmac_tx_mgmt_set_txop_security_param(pst_dmac_vap, pst_tx_ctl, &st_txop_feature, pst_netbuf_mgmt, &st_mpdu);

    /* 填写dscr基本参数 */
    dmac_tx_set_basic_ctrl_dscr(pst_mgmt_dscr, pst_tx_ctl, &st_mpdu);

    /* 填写发送描述符参数 */
    hal_tx_fill_basic_ctrl_dscr(pst_hal_device, pst_mgmt_dscr, &st_mpdu);

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    //待验证03无beacon 0问题时，用_PRE_WLAN_MAC_BUGFIX_TSF_SYNC宏包裹
    dmac_sta_tsf_restore(pst_dmac_vap);
#endif

    dmac_tx_get_timestamp(pst_hal_device, pst_dmac_vap->pst_hal_vap, &st_ppdu_feature.us_tsf);

#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
    if (OAL_PTR_NULL != pst_dmac_user)
    {
        /* 通知算法,每次发送管理帧发送之前,根据用户rssi信息，修改resp管理帧发送功率 */
        dmac_pow_change_mgmt_power_process(&pst_dmac_vap->st_vap_base_info, oal_get_real_rssi(pst_dmac_user->s_rx_rssi));
    }
#endif

    /* 设置管理帧txop参数 */
    dmac_tx_mgmt_get_txop_para(pst_dmac_vap, &pst_txop_alg, pst_tx_ctl);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    dmac_tx_updata_probe_rsp_para(pst_dmac_vap, pst_txop_alg, pst_mac_header);
#endif

    /* 设置当前管理帧的发送参数，保存初始化的值不被覆盖参数 */
    oal_memcopy(&st_txop_alg, pst_txop_alg, OAL_SIZEOF(hal_tx_txop_alg_stru));

    /* NDPA帧需要修改当前的发送参数 */
#ifdef _PRE_WLAN_FEATURE_TXBF_HW
    if ((WLAN_CONTROL == pst_mac_header->st_frame_control.bit_type)
       && (WLAN_VHT_NDPA == pst_mac_header->st_frame_control.bit_sub_type))
    {
        /* 1103 txbf发送NDP帧参数通过描述符填写，注意修改发送速率、带宽值 */
        dmac_alg_tx_mgmt_notify(pst_dmac_user, &st_txop_alg);
    }
#endif

    /* 调用管理帧和广播、组播数据帧填写描述符接口 */
    hal_tx_non_ucast_data_set_dscr(pst_hal_device, pst_mgmt_dscr, &st_txop_feature, &st_txop_alg, &st_ppdu_feature);

    /* 设置ftm帧tx dscr*/
#ifdef _PRE_WLAN_FEATURE_FTM
    if(mac_check_ftm_enable(&pst_dmac_vap->st_vap_base_info))
    {
        dmac_tx_set_ftm_ctrl_dscr(pst_dmac_vap, pst_mgmt_dscr, pst_netbuf_mgmt);
    }
#endif

    hal_tx_ctrl_dscr_unlink(pst_hal_device, pst_mgmt_dscr);

    /*****************************************************************************
        到此发送描述符已填写完毕，接下来放到硬件队列
    *****************************************************************************/
    uc_mgmt_type    = mac_frame_get_type_value((oal_uint8 *)pst_mac_header);
    uc_mgmt_subtype = mac_frame_get_subtype_value((oal_uint8 *)pst_mac_header);

    /* 判断当前管理帧是否需要缓存 */
    if (OAL_TRUE == dmac_tx_mgmt_buff_process(pst_mac_device, pst_dmac_vap, pst_mgmt_dscr, pst_netbuf_mgmt))
    {
        OAL_MEM_TRACE(pst_mgmt_dscr, OAL_FALSE);
        /* 将缓存帧直接通过OTA上报，方便问题定位 */
        oam_report_80211_frame(BROADCAST_MACADDR,
                                (oal_uint8*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl),
                                MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl),
                                (oal_uint8*)oal_netbuf_payload(pst_netbuf_mgmt),
                                st_mpdu.us_mpdu_len,
                                OAM_OTA_FRAME_DIRECTION_TYPE_TX);
        return OAL_SUCC;
    }

    // even occurs while offload(non dbac)
    if ((pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number != 0)
        && (pst_hal_device->uc_current_chan_number != pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number)
        && (MAC_SCAN_STATE_RUNNING != pst_mac_device->en_curr_scan_state)
        && (MAC_VAP_STATE_INIT != pst_dmac_vap->st_vap_base_info.en_vap_state))
    {
        // 同频offload场景下，SYNC信道时BSS的DISASOC本身无法发送成功，此处避免虚检
        if ((WLAN_MANAGEMENT == uc_mgmt_type) && (WLAN_DISASOC == uc_mgmt_subtype))
        {
            OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "dmac_tx_mgmt:disasoc frame dropped from diff channel, vap chan:%d, hal chan:%d. vap state:%d",
                                    pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number,
                                    pst_hal_device->uc_current_chan_number,
                                    pst_dmac_vap->st_vap_base_info.en_vap_state);
        }
        else
        {
            if((WLAN_CH_SWITCH_STATUS_1 == pst_dmac_vap->st_vap_base_info.st_ch_switch_info.en_ch_switch_status)
                && (OAL_TRUE == MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_ctl)))
            {
                    oal_uint8       *puc_payload;
                    /* 如果是Channel Switch Announcement帧，只需报warning */
                    if (mac_ieeee80211_is_action(oal_netbuf_header(pst_netbuf_mgmt)))
                    {
                        puc_payload = (oal_uint8 *)mac_netbuf_get_payload(pst_netbuf_mgmt);

                        if ((MAC_ACTION_CATEGORY_SPECMGMT == puc_payload[0]) && (MAC_SPEC_CH_SWITCH_ANNOUNCE == puc_payload[1]))
                        {
                            OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "dmac_tx_mgmt:put tx_dscr in wrong channel. vap chan:%d, hal chan:%d. vap state:%d",
                                            pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number,
                                            pst_hal_device->uc_current_chan_number,
                                            pst_dmac_vap->st_vap_base_info.en_vap_state);
                        }
                    }
            }
            else
            {
                OAM_ERROR_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "dmac_tx_mgmt:put tx_dscr in wrong channel. vap chan:%d, hal chan:%d. vap state:%d",
                                    pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number,
                                    pst_hal_device->uc_current_chan_number,
                                    pst_dmac_vap->st_vap_base_info.en_vap_state);
            }
            oam_report_80211_frame(BROADCAST_MACADDR,
                                (oal_uint8*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl),
                                MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl),
                                (oal_uint8*)oal_netbuf_payload(pst_netbuf_mgmt),
                                (MAC_GET_CB_MPDU_LEN(pst_tx_ctl) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl)),
                                OAM_OTA_FRAME_DIRECTION_TYPE_TX);
            oam_report_backtrace();
        }

        oal_netbuf_free(pst_netbuf_mgmt);
        OAL_MEM_FREE(pst_mgmt_dscr, OAL_TRUE);

        return OAL_SUCC;    // 返回SUCC以避免重复释放该SKB
    }
#ifdef _PRE_WLAN_FEATURE_ROAM
    /* 漫游状态发送帧时，如果发送帧VAP所在信道非当前工作信道则释放帧 */
    if (OAL_FALSE == dmac_tx_enable_in_roaming(pst_hal_device, pst_dmac_vap))
    {
        OAL_MEM_FREE(pst_mgmt_dscr, OAL_TRUE);
        return OAL_FAIL;
    }
#endif

#ifdef _PRE_WLAN_CHIP_TEST
    /* chip test modify dscr tx cnt */
    DMAC_CHIP_TEST_CALL(dmac_test_dfx_set_tx_cnt(pst_txop_alg));
#endif

#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
    /*  将管理帧的netbuf同步到DDR，保证cache一致性 */
    us_cache_frame_len = (oal_uint16)MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl) + (oal_uint16)MAC_GET_CB_MPDU_LEN(pst_tx_ctl);
    oal_dma_map_single(NULL, OAL_NETBUF_DATA(pst_netbuf_mgmt), us_cache_frame_len, OAL_TO_DEVICE);
#endif

    /* SDT打印管理帧 */
    dmac_tx_dump_mgmt(pst_hal_device, pst_netbuf_mgmt, &st_mpdu, pst_mgmt_dscr);

#ifdef _PRE_DEBUG_MODE
    g_ast_tx_complete_stat[pst_hal_device->uc_mac_device_id].ul_tx_mgnt_num++;
#endif

#ifdef _PRE_WLAN_DFT_STAT
    /* 软件发送管理帧统计 */
    dmac_dft_mgmt_stat_incr(pst_mac_device, (oal_uint8 *)pst_mac_header, MAC_DEV_MGMT_STAT_TYPE_TX);
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    dmac_btcoex_tx_mgmt_process(pst_dmac_vap, pst_mac_header);
    dmac_btcoex_tx_mgmt_frame(&(pst_dmac_vap->st_vap_base_info), pst_netbuf_mgmt);
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
    dmac_tx_process_dfr(pst_hal_device);
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
    dmac_tx_process_psm_sta(pst_dmac_vap, pst_dmac_user, pst_mac_header, pst_tx_ctl);
#endif

    /* 将同一个需要发送的描述符加入描述符链表 */
    oal_dlist_init_head(&st_tx_dscr_list_hdr);
    oal_dlist_add_tail(&pst_mgmt_dscr->st_entry, &st_tx_dscr_list_hdr);

#ifdef _PRE_DEBUG_MODE
    //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
    en_trace_pkt_type = wifi_pkt_should_trace( pst_netbuf_mgmt, MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl));
    if( PKT_TRACE_BUTT != en_trace_pkt_type)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_mgmt::type%d sent to hw[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        oam_report_80211_frame(BROADCAST_MACADDR,
                                        (oal_uint8 *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl),
                                        MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl),
                                        (oal_uint8 *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl),
                                        (oal_uint16)(MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl) + (oal_uint16)MAC_GET_CB_MPDU_LEN(pst_tx_ctl)),
                                        OAM_OTA_FRAME_DIRECTION_TYPE_TX);
    }
#endif

    /* 挂硬件队列发送 */
    dmac_tx_hw_send(pst_hal_device, pst_mgmt_dscr, pst_tx_ctl, &st_tx_dscr_list_hdr, 1);
    return OAL_SUCC;
}



/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_tx_save_tx_queue);
oal_module_symbol(dmac_tx_restore_tx_queue);
oal_module_symbol(dmac_tx_process_data_event);
oal_module_symbol(dmac_tx_mgmt);
oal_module_symbol(dmac_alg_downlink_flowctl_notify);
oal_module_symbol(dmac_tx_reinit_tx_queue);
/*lint +e578*//*lint +e19*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

