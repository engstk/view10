


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "mac_ie.h"
#include "mac_vap.h"
#include "dmac_vap.h"
#include "dmac_user.h"
#include "dmac_main.h"
#include "dmac_blockack.h"
#include "dmac_alg.h"
#include "dmac_beacon.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_tx_complete.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_mgmt_sta.h"
#include "dmac_uapsd.h"
#include "dmac_psm_ap.h"
#include "dmac_fcs.h"
#include "dmac_chan_mgmt.h"

#ifdef _PRE_WIFI_DMT
#include "dmt_stub.h"
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_sta_pm.h"
#endif
#include "dmac_device.h"
#include "mac_device.h"
#include "dmac_config.h"
#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
#include "dmac_resource.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11K
#include "dmac_11k.h"
#endif

#ifdef _PRE_WLAN_11K_STAT
#include "dmac_stat.h"
#endif

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
#include "dmac_tx_qos_enhance.h"
#endif


#include "dmac_m2s.h"

#include "dmac_scan.h"

#include "dmac_power.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_VAP_C

OAL_STATIC oal_void dmac_sta_bw_switch_fsm_init_entry(oal_void *p_ctx);
OAL_STATIC oal_void dmac_sta_bw_switch_fsm_normal_entry(oal_void *p_ctx);
OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_init_event(oal_void  *p_ctx, oal_uint16  us_event, oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_normal_event(oal_void  *p_ctx, oal_uint16  us_event, oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_verify_20m_event(oal_void  *p_ctx, oal_uint16  us_event, oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_verify_40m_event(oal_void  *p_ctx, oal_uint16  us_event, oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_invalid_event(oal_void  *p_ctx, oal_uint16  us_event, oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_uint32 dmac_bw_fsm_trans_to_state(dmac_sta_bw_switch_fsm_info_stru *pst_bw_fsm, oal_uint8 uc_state);


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
dmac_vap_stru g_ast_dmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
#endif
oal_fsm_state_info g_sta_bw_switch_fsm_info[] = {
    {
        DMAC_STA_BW_SWITCH_FSM_INIT,
        "INIT",
        dmac_sta_bw_switch_fsm_init_entry,
        OAL_PTR_NULL,
        dmac_sta_bw_switch_fsm_init_event,
    },
    {
        DMAC_STA_BW_SWITCH_FSM_NORMAL,
        "NORMAL",
        dmac_sta_bw_switch_fsm_normal_entry,
        OAL_PTR_NULL,
        dmac_sta_bw_switch_fsm_normal_event,
    },
    {
        DMAC_STA_BW_SWITCH_FSM_VERIFY20M,
        "VERIFY20M",
        OAL_PTR_NULL,
        OAL_PTR_NULL,
        dmac_sta_bw_switch_fsm_verify_20m_event,
    },
    {
        DMAC_STA_BW_SWITCH_FSM_VERIFY40M,
        "VERIFY40M",
        OAL_PTR_NULL,
        OAL_PTR_NULL,
        dmac_sta_bw_switch_fsm_verify_40m_event,
    },
    {
        DMAC_STA_BW_SWITCH_FSM_INVALID,
        "INVALID",
        OAL_PTR_NULL,
        OAL_PTR_NULL,
        dmac_sta_bw_switch_fsm_invalid_event,
    }
};

dmac_vap_rom_stru  g_dmac_vap_rom[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_VOWIFI

void dmac_vap_vowifi_init(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint64  ull_timestamp_ms;

    if (WLAN_VAP_MODE_BSS_STA != pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        return;
    }
    if (OAL_PTR_NULL == pst_dmac_vap->pst_vowifi_status)
    {
        pst_dmac_vap->pst_vowifi_status = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_vowifi_status_stru), OAL_TRUE);
        if (OAL_PTR_NULL == pst_dmac_vap->pst_vowifi_status)
        {
            OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_VOWIFI, "{dmac_vap_vowifi_init::pst_vowifi_status alloc null, size[%d].}", OAL_SIZEOF(mac_vowifi_status_stru));
            return ;
        }
    }

    OAL_MEMZERO((oal_uint8 *)(pst_dmac_vap->pst_vowifi_status), OAL_SIZEOF(mac_vowifi_status_stru));
    ull_timestamp_ms = OAL_TIME_GET_STAMP_MS();
    pst_dmac_vap->pst_vowifi_status->ull_rssi_timestamp_ms = ull_timestamp_ms;
    pst_dmac_vap->pst_vowifi_status->ull_arp_timestamp_ms  = ull_timestamp_ms;
}
#endif /* _PRE_WLAN_FEATURE_VOWIFI */


oal_uint32  dmac_vap_init(
                dmac_vap_stru              *pst_dmac_vap,
                oal_uint8                   uc_chip_id,
                oal_uint8                   uc_device_id,
                oal_uint8                   uc_vap_id,
                mac_cfg_add_vap_param_stru *pst_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint32  ul_ret;
#endif
    oal_uint8          uc_qid;
    mac_device_stru   *pst_mac_device;
    dmac_device_stru  *pst_dmac_device;


    pst_mac_device = mac_res_get_dev(uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{dmac_vap_init::pst_mac_device[%d] null!}", uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_device))
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{dmac_vap_init::pst_dmac_device[%d] null!}", uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* DMAC VAP 部分初始清零 */
    OAL_MEMZERO(((oal_uint8 *)pst_dmac_vap) + OAL_SIZEOF(mac_vap_stru), OAL_SIZEOF(dmac_vap_stru) - OAL_SIZEOF(mac_vap_stru));
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

    /* 统计信息清零 */
    oam_stats_clear_vap_stat_info(uc_vap_id);
#endif

    pst_dmac_vap->_rom = &g_dmac_vap_rom[uc_vap_id];
    OAL_MEMZERO(pst_dmac_vap->_rom, OAL_SIZEOF(dmac_vap_rom_stru));

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 在非OFFLOAD情况下，这些操作HMAC都已经做过了 */
    /* 初始化mac vap */
    ul_ret = mac_vap_init(&pst_dmac_vap->st_vap_base_info,
                           uc_chip_id,
                           uc_device_id,
                           uc_vap_id,
                           pst_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG,
                         "{dmac_vap_init::dmac_vap_init failed[%d].", ul_ret);
        return ul_ret;
    }
    pst_dmac_vap->bit_beacon_timeout_times = 0;
#endif

#ifdef _PRE_WLAN_FEATURE_VOWIFI
    if (WLAN_LEGACY_VAP_MODE == pst_param->en_p2p_mode)
    {
        dmac_vap_vowifi_init(pst_dmac_vap);
    }
#endif
    pst_dmac_vap->en_multi_user_multi_ac_flag   = OAL_FALSE;
    pst_dmac_vap->uc_traffic_type               = OAL_TRAFFIC_NORMAL;

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)&&((_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT) || (_PRE_TARGET_PRODUCT_TYPE_5630HERA == _PRE_CONFIG_TARGET_PRODUCT)))
    /* ONT需求双通道发送beacon，11b由于芯片限制，改为两通道轮流发送*/
    pst_dmac_vap->en_beacon_tx_policy           = DMAC_BEACON_TX_POLICY_SWITCH;
#else
    /* 初始化特性标识中BEACON帧不轮流发送 */
    pst_dmac_vap->en_beacon_tx_policy           = DMAC_BEACON_TX_POLICY_SINGLE;
#endif

    /* 速率结构体赋初值 */
    /* 按照PHY给出的初始值 ul_value = 0x00800211 */
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_tx_count = 1;
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV))
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_stbc_mode = 0;
#endif
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_tx_chain_selection = 1;
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_tx_antenna         = 2;

    /* FPGA zhangyu Debug 11n */
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs = 0;
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;

    /* 默认是HOST VAP */
    pst_dmac_vap->en_is_host_vap = OAL_TRUE;

    /* 默认天线组合 */
    pst_dmac_vap->uc_default_ant_bitmap = 0xFF;

    /* 初始化节能相关参数 */
    pst_dmac_vap->uc_ps_user_num = 0;
    pst_dmac_vap->uc_dtim_count  = WLAN_DTIM_DEFAULT;
#ifdef _PRE_WLAN_CHIP_TEST
    pst_dmac_vap->pst_wow_probe_resp = OAL_PTR_NULL;
    pst_dmac_vap->pst_wow_null_data  = OAL_PTR_NULL;
#endif
    pst_dmac_vap->uc_sw_retry_limit  = DMAC_MAX_SW_RETRIES;

    if (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        if (OAL_PTR_NULL != pst_dmac_vap->puc_tim_bitmap)
        {
            OAL_MEM_FREE(pst_dmac_vap->puc_tim_bitmap, OAL_TRUE);
            pst_dmac_vap->puc_tim_bitmap = OAL_PTR_NULL;
        }

        /* +7除以8，表示按照字节补齐， +2，单独2个字节放置其他内容 */
        pst_dmac_vap->uc_tim_bitmap_len = (oal_uint8)(2 + ((mac_board_get_max_user() + 7) >> 3));
        pst_dmac_vap->puc_tim_bitmap = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, pst_dmac_vap->uc_tim_bitmap_len, OAL_TRUE);
        if (OAL_PTR_NULL == pst_dmac_vap->puc_tim_bitmap)
        {
            OAM_ERROR_LOG1(uc_vap_id, OAM_SF_ANY, "{dmac_vap_init::alloc tim_bitmap memory(length:%d)fail!}", pst_dmac_vap->uc_tim_bitmap_len);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        OAL_MEMZERO(pst_dmac_vap->puc_tim_bitmap, pst_dmac_vap->uc_tim_bitmap_len);
        /* TIM bitmap len is default 1*/
        pst_dmac_vap->puc_tim_bitmap[0] = 1;

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
        dmac_tx_qos_enhance_attach(pst_dmac_vap);
#endif

    }
    if(WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
#ifdef _PRE_WLAN_FEATURE_DFS
        mac_mib_set_SpectrumManagementImplemented(&pst_dmac_vap->st_vap_base_info, OAL_TRUE);
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        pst_dmac_vap->uc_sta_keepalive_cnt  = 0;
#endif
    }

#ifdef _PRE_WLAN_FEATURE_11K
    pst_dmac_vap->pst_rrm_info = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_rrm_info_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_dmac_vap->pst_rrm_info)
    {
        if ((WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode) && (OAL_PTR_NULL != pst_dmac_vap->puc_tim_bitmap))
        {
            OAL_MEM_FREE(pst_dmac_vap->puc_tim_bitmap, OAL_TRUE);
            pst_dmac_vap->puc_tim_bitmap = OAL_PTR_NULL;
        }
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_ANY, "{dmac_vap_init::pst_rrm_info null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    OAL_MEMZERO(pst_dmac_vap->pst_rrm_info, OAL_SIZEOF(mac_rrm_info_stru));
    oal_dlist_init_head(&(pst_dmac_vap->pst_rrm_info->st_meas_rpt_list));

    pst_dmac_vap->bit_bcn_table_switch  = OAL_TRUE; //宏打开时默认使能，可修改
    pst_dmac_vap->bit_11k_enable        = OAL_TRUE;
    pst_dmac_vap->bit_11v_enable        = OAL_TRUE;
#endif //_PRE_WLAN_FEATURE_11K
#ifdef _PRE_WLAN_FEATURE_11R
    pst_dmac_vap->bit_11r_enable        = OAL_TRUE;
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    pst_dmac_vap->pst_ip_addr_info = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_vap_ip_entries_stru), OAL_TRUE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_ip_addr_info))
    {
        if ((WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode) && (OAL_PTR_NULL != pst_dmac_vap->puc_tim_bitmap))
        {
            OAL_MEM_FREE(pst_dmac_vap->puc_tim_bitmap, OAL_TRUE);
            pst_dmac_vap->puc_tim_bitmap = OAL_PTR_NULL;
        }
#ifdef _PRE_WLAN_FEATURE_11K
        if(OAL_PTR_NULL != pst_dmac_vap->pst_rrm_info)
        {
            OAL_MEM_FREE(pst_dmac_vap->pst_rrm_info, OAL_TRUE);
            pst_dmac_vap->pst_rrm_info = OAL_PTR_NULL;
        }
#endif
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_PWR, "{dmac_vap_init::Alloc memory for IP address record array failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    oal_memset(pst_dmac_vap->pst_ip_addr_info, 0, OAL_SIZEOF(dmac_vap_ip_entries_stru));

#endif
    /* 初始化RSSI值，作平滑用 */
    pst_dmac_vap->st_query_stats.s_signal   = OAL_RSSI_INIT_MARKER;
    pst_dmac_vap->st_query_stats.c_snr_ant0 = OAL_SNR_INIT_VALUE;
    pst_dmac_vap->st_query_stats.c_snr_ant1 = OAL_SNR_INIT_VALUE;
    pst_dmac_vap->us_beacon_timeout  = DMAC_DEFAULT_STA_BEACON_WAIT_TIME;
    if (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        pst_dmac_vap->us_in_tbtt_offset  = DMAC_DEFAULT_AP_INTER_TBTT_OFFSET;
    }
    else
    {
        pst_dmac_vap->us_in_tbtt_offset  = DMAC_DEFAULT_STA_INTER_TBTT_OFFSET;
    }
    pst_dmac_vap->us_ext_tbtt_offset = DMAC_DEFAULT_EXT_TBTT_OFFSET;
    pst_dmac_vap->uc_bcn_tout_max_cnt = DMAC_DEFAULT_STA_BCN_TOUT_MAX_CNT;

    pst_dmac_vap->uc_psm_dtim_period = 0;
    pst_dmac_vap->us_psm_listen_interval = 0;

    pst_dmac_vap->en_non_erp_exist = OAL_FALSE;

    /* 初始化记录ap在切换到20M之前的带宽为BUTT */
    pst_dmac_vap->en_40M_bandwidth = WLAN_BAND_WIDTH_BUTT;

    /* 初始化虚假发送队列 */
    for (uc_qid = 0; uc_qid < HAL_TX_QUEUE_NUM; uc_qid++)
    {
        oal_dlist_init_head(&(pst_dmac_vap->ast_tx_dscr_queue_fake[uc_qid].st_header));

        pst_dmac_vap->ast_tx_dscr_queue_fake[uc_qid].en_queue_status  = HAL_DSCR_QUEUE_VALID;
        pst_dmac_vap->ast_tx_dscr_queue_fake[uc_qid].uc_ppdu_cnt      = 0;
    }

#ifdef _PRE_WLAN_11K_STAT
    dmac_stat_init_vap(pst_dmac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    dmac_vap_init_sensing_bssid_list(pst_dmac_vap);
#endif

    //dmac_pow_init_vap_info(pst_dmac_vap);

#ifdef _PRE_WLAN_FEATURE_TXBF_HW
    pst_dmac_vap->puc_vht_bfee_buff = OAL_PTR_NULL;
#endif

    return OAL_SUCC;
}


oal_void dmac_vap_init_tx_data_rate_ucast(dmac_vap_stru *pst_dmac_vap,oal_uint8 uc_protocol_mode, oal_uint8 uc_legacy_rate)
{
    oal_uint32          ul_data_rate = 0;

    pst_dmac_vap->st_tx_alg.ast_per_rate[0].ul_value = 0x0;

    /* 单播数据帧参数 */
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_tx_count       = 3;
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV))
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_stbc_mode      = 0;
#endif
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_tx_chain_selection =
                                                          pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain;
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_tx_antenna     = 1;

    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate   = uc_legacy_rate;
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = uc_protocol_mode;
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable     = 0;
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_txbf_mode          = 0;

    /* 11b 1M速率使用长前导码 */
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_preamble_mode  = HAL_PHY_11B_1M_RATE(uc_protocol_mode, uc_legacy_rate) ? 1 : 0;
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_short_gi_enable    = 0;
    pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_reserve            = 0;

    ul_data_rate = pst_dmac_vap->st_tx_alg.ast_per_rate[0].ul_value;

    pst_dmac_vap->st_tx_alg.ast_per_rate[1].ul_value = ul_data_rate;
    pst_dmac_vap->st_tx_alg.ast_per_rate[1].rate_bit_stru.bit_tx_count = 0;

    pst_dmac_vap->st_tx_alg.ast_per_rate[2].ul_value = ul_data_rate;
    pst_dmac_vap->st_tx_alg.ast_per_rate[2].rate_bit_stru.bit_tx_count = 0;

    pst_dmac_vap->st_tx_alg.ast_per_rate[3].ul_value = ul_data_rate;
    pst_dmac_vap->st_tx_alg.ast_per_rate[3].rate_bit_stru.bit_tx_count = 0;

    pst_dmac_vap->st_tx_alg.st_rate.bit_lsig_txop = OAL_FALSE;
}



oal_void dmac_vap_init_tx_data_rate_bmcast(dmac_vap_stru  *pst_dmac_vap, hal_tx_txop_alg_stru  *pst_tx_data_cast)
{
    oal_uint8       uc_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    oal_uint8       uc_legacy_rate   = 0xB;
    oal_uint32      ul_data_rate;

    switch (pst_dmac_vap->st_vap_base_info.en_protocol)
    {
        /* 11b 1M */
        case WLAN_LEGACY_11B_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_HT_MODE:
            if ((WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)
#ifdef _PRE_WLAN_FEATURE_P2P
                && (IS_LEGACY_VAP(&(pst_dmac_vap->st_vap_base_info)))
#endif //#ifdef _PRE_WLAN_FEATURE_P2P
            )
            {
                /* P2P 设备不能发送11b 速率的帧 */
                uc_protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
                uc_legacy_rate   = 0;
            }
            else
            {
                uc_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
                uc_legacy_rate   = 0xB;
            }
            break;

        /* OFDM 6M */
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
        case WLAN_VHT_MODE:
    #ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_MODE:
    #endif
        case WLAN_HT_11G_MODE:
            uc_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            uc_legacy_rate   = 0xB;
            break;

        /* OFDM 24M */
        case WLAN_HT_ONLY_MODE:
        case WLAN_VHT_ONLY_MODE:
            uc_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            uc_legacy_rate   = 0x9;
            break;

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                             "{dmac_vap_init_tx_data_rate_bmcast::invalid en_protocol[%d].}", pst_dmac_vap->st_vap_base_info.en_protocol);
            return;
    }


    pst_tx_data_cast->ast_per_rate[0].ul_value = 0x0;

    /* 广播、组播数据帧参数 */
    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_tx_count           = 3;
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV))
    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_stbc_mode          = 0;
#endif

    if (WLAN_11B_PHY_PROTOCOL_MODE == uc_protocol_mode)
    {
        pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_tx_chain_selection =
                                                   pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_single_tx_chain;
    }
    else
    {
        pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_tx_chain_selection =
                                                   pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain;
    }

    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_tx_antenna     = 1;

    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate   = uc_legacy_rate;
    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = uc_protocol_mode;
    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable     = 0;
    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_txbf_mode          = 0;

    /* 11b 1M速率使用长前导码 */
    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_preamble_mode = HAL_PHY_11B_1M_RATE(uc_protocol_mode, uc_legacy_rate) ? 1 : 0;
    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_short_gi_enable    = 0;
    pst_tx_data_cast->ast_per_rate[0].rate_bit_stru.bit_reserve            = 0;

    ul_data_rate = pst_tx_data_cast->ast_per_rate[0].ul_value;

    pst_tx_data_cast->ast_per_rate[1].ul_value = ul_data_rate;
    pst_tx_data_cast->ast_per_rate[1].rate_bit_stru.bit_tx_count = 0;

    pst_tx_data_cast->ast_per_rate[2].ul_value = ul_data_rate;
    pst_tx_data_cast->ast_per_rate[2].rate_bit_stru.bit_tx_count = 0;

    pst_tx_data_cast->ast_per_rate[3].ul_value = ul_data_rate;
    pst_tx_data_cast->ast_per_rate[3].rate_bit_stru.bit_tx_count = 0;

    pst_tx_data_cast->st_rate.bit_lsig_txop = OAL_FALSE;
}


oal_void  dmac_vap_init_tx_mgmt_rate(dmac_vap_stru *pst_dmac_vap, hal_tx_txop_alg_stru *pst_tx_mgmt_cast)
{
    oal_uint32 ul_value;

    /* 初始化2.4G参数 */
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_tx_count             = 3;
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV))
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_stbc_mode            = 0;
#endif
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_tx_antenna           = 1;
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable       = 0;
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_txbf_mode            = 0;
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_short_gi_enable      = 0;
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_reserve              = 0;
    pst_tx_mgmt_cast[WLAN_BAND_2G].st_rate.bit_lsig_txop = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_P2P
    if (!IS_LEGACY_VAP(&(pst_dmac_vap->st_vap_base_info)))
    {
        /* P2P 设备不能发送11b 速率的帧,采用OFDM 模式发送 */
        pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate   = 0xb;  /* 6M */
        pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
        pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_preamble_mode                            = 0;
        pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_tx_chain_selection   =
                                                         pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_single_tx_chain;
    }
    else
#endif  /* _PRE_WLAN_FEATURE_P2P */
    {
        /* 2.4G初始化为11b 1M, long preable */
        pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate   = 0;
        pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
        pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_preamble_mode                            = 1;
        pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_tx_chain_selection   =
                                                                 pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_single_tx_chain;
    }

    ul_value = pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[0].ul_value;
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[1].ul_value = ul_value;
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[2].ul_value = ul_value;
    pst_tx_mgmt_cast[WLAN_BAND_2G].ast_per_rate[3].ul_value = ul_value;

    /* 初始化5G参数 */
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_tx_count             = 3;
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV))
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_stbc_mode            = 0;
#endif
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_tx_antenna           = 1;
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable       = 0;
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_txbf_mode            = 0;
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_short_gi_enable      = 0;
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_reserve              = 0;
    pst_tx_mgmt_cast[WLAN_BAND_5G].st_rate.bit_lsig_txop = OAL_FALSE;

    /* 5G初始化为OFDM 6M, short preable */
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate   = 0xb;  /* 6M */
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_preamble_mode                            = 0;
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_tx_chain_selection   =
                                                                  pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain;
    ul_value = pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[0].ul_value;
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[1].ul_value = ul_value;
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[2].ul_value = ul_value;
    pst_tx_mgmt_cast[WLAN_BAND_5G].ast_per_rate[3].ul_value = ul_value;
}


oal_uint32  dmac_vap_init_tx_frame_params(dmac_vap_stru *pst_dmac_vap, oal_bool_enum_uint8  en_mgmt_rate_init_flag)
{

    if (OAL_TRUE == en_mgmt_rate_init_flag)
    {
        /* 初始化单播管理帧参数 */
        dmac_vap_init_tx_mgmt_rate(pst_dmac_vap, pst_dmac_vap->ast_tx_mgmt_ucast);

        /* 初始化组播、广播管理帧参数 */
        dmac_vap_init_tx_mgmt_rate(pst_dmac_vap, pst_dmac_vap->ast_tx_mgmt_bmcast);
    }

    /* 初始化组播数据帧发送参数 */
    dmac_vap_init_tx_data_rate_bmcast(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_mcast));

    /* 初始化广播数据帧发送参数 */
    dmac_vap_init_tx_data_rate_bmcast(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_bcast));

    return OAL_SUCC;
}


oal_uint32  dmac_vap_init_tx_ucast_data_frame(dmac_vap_stru *pst_dmac_vap)
{

    switch(pst_dmac_vap->st_vap_base_info.en_protocol)
    {
        /* BPSK 1M */
        case WLAN_LEGACY_11B_MODE:
        case WLAN_MIXED_ONE_11G_MODE:
             dmac_vap_init_tx_data_rate_ucast(pst_dmac_vap, WLAN_11B_PHY_PROTOCOL_MODE, 0);
            break;

        /* OFDM 6M*/
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
            dmac_vap_init_tx_data_rate_ucast(pst_dmac_vap, WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE, 0xB);
            break;

        /* HT MCS0*/
        case WLAN_HT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
        case WLAN_HT_MODE:
            dmac_vap_init_tx_data_rate_ucast(pst_dmac_vap, WLAN_HT_PHY_PROTOCOL_MODE, 0);
        break;

        /* VHT MCS0 */
        case WLAN_VHT_MODE:
        case WLAN_VHT_ONLY_MODE:
            dmac_vap_init_tx_data_rate_ucast(pst_dmac_vap, WLAN_VHT_PHY_PROTOCOL_MODE, 0);
             break;
    #ifdef _PRE_WLAN_FEATURE_11AX
        case WLAN_HE_MODE:
            dmac_vap_init_tx_data_rate_ucast(pst_dmac_vap, WLAN_HE_SU_FORMAT ,0);
            break;
    #endif
        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                             "{dmac_vap_init_tx_ucast_data_frame::invalid en_protocol[%d].}", pst_dmac_vap->st_vap_base_info.en_protocol);
        return OAL_FAIL;
    }

    OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                     "{dmac_vap_init_tx_ucast_data_frame::en_protocol[%d], bit_protocol_mode:%d.}",
                     pst_dmac_vap->st_vap_base_info.en_protocol,
                     pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode);

    return OAL_SUCC;
}
#if 0

oal_uint8  dmac_vap_get_default_ant(mac_vap_stru *pst_vap)
{
    return ((dmac_vap_stru *)pst_vap)->uc_default_ant_bitmap;
}
#endif

oal_uint32  dmac_vap_sta_reset(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8                       auc_bssid[WLAN_MAC_ADDR_LEN] = {0, 0, 0, 0, 0, 0};
    mac_device_stru *pst_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_vap_sta_reset::pst_device[%d] null!}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 恢复帧过滤寄存器 */
    //hal_disable_non_frame_filter(pst_dmac_vap->pst_hal_device);

    /* 删除BSSID */
    hal_set_sta_bssid(pst_dmac_vap->pst_hal_vap, auc_bssid);

    /* 停止STA tsf */
    hal_disable_tsf_tbtt(pst_dmac_vap->pst_hal_vap);

    /* 恢复slottime类型为short */
    hal_cfg_slottime_type(pst_dmac_vap->pst_hal_device, 0);

    /* 恢复TXOP不使能状态 */
    if (pst_device->en_txop_enable)
    {
        pst_device->en_txop_enable = OAL_FALSE;
        hal_vap_set_machw_txop_limit_bkbe(pst_dmac_vap->pst_hal_vap,
                                          (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_BE),
                                          (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_BK));
    }
    return OAL_SUCC;
}

oal_uint32  dmac_vap_save_tx_queue(mac_vap_stru *pst_mac_vap)
{
    hal_tx_dscr_queue_header_stru   *pst_fake_queue;
    hal_to_dmac_device_stru         *pst_hal_device;

    pst_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_vap);
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);

    dmac_tx_save_tx_queue(pst_hal_device, pst_fake_queue);

    return OAL_SUCC;
}

oal_uint32  dmac_vap_restore_tx_queue(mac_vap_stru *pst_mac_vap)
{
    hal_tx_dscr_queue_header_stru   *pst_fake_queue;
    hal_to_dmac_device_stru         *pst_hal_device;

    pst_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_vap);
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);

    if (OAL_FALSE == dmac_vap_is_fakeq_empty(pst_mac_vap))
    {
        dmac_tx_restore_tx_queue(pst_hal_device, pst_fake_queue);
    }
    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_same_channel_down_vap_handle_fakeq(hal_to_dmac_device_stru *pst_hal_device, mac_vap_stru *pst_down_vap, mac_vap_stru *pst_up_vap)
{
    oal_int8                        c_q_id;
    oal_bool_enum_uint8             en_down_fakeq_empty;
    oal_bool_enum_uint8             en_up_fakeq_empty;
    oal_uint8                       uc_mac_vap_id;
    mac_vap_stru                   *pst_mac_dscr_vap;
    mac_device_stru                *pst_mac_device;
    hal_tx_dscr_queue_header_stru  *pst_fake_queue      = OAL_PTR_NULL;
    oal_dlist_head_stru            *pst_dlist_entry_pos = OAL_PTR_NULL;
    oal_dlist_head_stru            *pst_dlist_entry_n   = OAL_PTR_NULL;
    hal_tx_dscr_stru               *pst_tx_dscr         = OAL_PTR_NULL;
    hal_tx_dscr_stru               *pst_tx_dscr_tmp     = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev(pst_down_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_the_same_channel_down_switch_fakeq::pst_mac_device[%d] null.}", pst_down_vap->uc_device_id);
        return;
    }

    en_up_fakeq_empty   = dmac_vap_is_fakeq_empty(pst_up_vap);
    en_down_fakeq_empty = dmac_vap_is_fakeq_empty(pst_down_vap);

    /* 如果都空,不需要后续处理 */
    if ((OAL_TRUE == en_up_fakeq_empty) && (OAL_TRUE == en_down_fakeq_empty))
    {
        return;
    }

    /* 不可能存在两个fakeq都有dscr */
    if ((en_up_fakeq_empty != OAL_TRUE) && (en_down_fakeq_empty != OAL_TRUE))
    {
        dmac_device_check_fake_queues_empty(pst_down_vap->uc_device_id);
        return;
    }

    pst_mac_dscr_vap = (en_up_fakeq_empty != OAL_TRUE) ? pst_up_vap : pst_down_vap;
    pst_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_dscr_vap);

    OAM_WARNING_LOG4(0, OAM_SF_SCAN, "{dmac_the_same_channel_down_switch_fakeq::up vap[%d]up fakeq empty[%d],down vap[%d]down fakeq empty[%d].}",
                        pst_up_vap->uc_vap_id, en_up_fakeq_empty, pst_down_vap->uc_vap_id, en_down_fakeq_empty);

    /* 遍历6个发送队列 一定要先处理高优先级队列防止普通优先级队列发送完成产生管理帧入错队列 */
    for (c_q_id = HAL_TX_QUEUE_BUTT - 1; c_q_id >= 0; c_q_id--)
    {
        OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_dlist_entry_pos, pst_dlist_entry_n, &(pst_fake_queue[(oal_uint8)c_q_id].st_header))
        {
            pst_tx_dscr = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_entry_pos, hal_tx_dscr_stru, st_entry);
            dmac_tx_get_vap_id(pst_hal_device, pst_tx_dscr, &uc_mac_vap_id);

            /* down vap的包释放 */
            if (uc_mac_vap_id == pst_down_vap->uc_vap_id)
            {
                pst_tx_dscr_tmp  = pst_tx_dscr;
                do
                {
                    dmac_tx_complete_free_dscr(pst_tx_dscr_tmp);
                    hal_get_tx_dscr_next(pst_hal_device, pst_tx_dscr_tmp, &pst_tx_dscr_tmp);
                }while(OAL_PTR_NULL != pst_tx_dscr_tmp);

                pst_fake_queue[(oal_uint8)c_q_id].uc_ppdu_cnt--;
            }
        }
    }

    /* 描述符都挂在down的vap上需要放到up的vap上 */
    if (pst_mac_dscr_vap == pst_down_vap)
    {
        dmac_vap_fake_queue_empty_assert(pst_up_vap, THIS_FILE_ID);
        dmac_tx_switch_tx_queue(pst_fake_queue, DMAC_VAP_GET_FAKEQ(pst_up_vap));
    }
}


oal_void dmac_vap_down_notify(mac_vap_stru *pst_down_vap)
{
    mac_vap_stru                *pst_up_vap;
    mac_device_stru             *pst_device;
    hal_to_dmac_device_stru     *pst_hal_device;

    pst_device      = mac_res_get_dev(pst_down_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
       OAM_ERROR_LOG1(pst_down_vap->uc_vap_id, OAM_SF_DBAC, "{dmac_vap_down_notify::pst_device[%d] is NULL!}", pst_down_vap->uc_device_id);
       return;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_down_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_down_vap->uc_vap_id, OAM_SF_DBAC, "{dmac_vap_down_notify::pst_hal_device[%d] is NULL!}", pst_down_vap->uc_device_id);
        return;
    }

    OAM_WARNING_LOG3(pst_down_vap->uc_vap_id, OAM_SF_DBAC, "{dmac_vap_down_notify::vap[%d] vap mode[%d],p2p mode[%d]is down!!!}",
                    pst_down_vap->uc_vap_id, pst_down_vap->en_vap_mode, pst_down_vap->en_p2p_mode);

    pst_up_vap = mac_device_find_another_up_vap(pst_device, pst_down_vap->uc_vap_id);

    /* scan abort的逻辑和CCA的逻辑应该从DBAC中挪出来,不然没有启动DBAC的产品形态，AP+STA形态会在前一个判断返回，TBD */
    
    if ((mac_is_dbac_enabled(pst_device)) && (MAC_SCAN_STATE_RUNNING == pst_device->en_curr_scan_state))
    {
        OAM_WARNING_LOG2(pst_down_vap->uc_vap_id, OAM_SF_SCAN, "dmac_vap_down_notify::device is scanning[%d],mode[%d]. stop scan",
                                pst_device->en_curr_scan_state, pst_device->st_scan_params.en_scan_mode);

        if (pst_up_vap != OAL_PTR_NULL)
        {
            /* 同信道,提前剥离down vap的描述符 */
            if (OAL_TRUE == mac_fcs_is_same_channel(&(pst_down_vap->st_channel), &(pst_up_vap->st_channel)))
            {
                dmac_same_channel_down_vap_handle_fakeq(pst_hal_device, pst_down_vap, pst_up_vap);
            }
        }

        dmac_scan_abort(pst_device);
    }

    /* 清空down的vap的虚假队列 */
    dmac_vap_clear_fake_queue(pst_down_vap);

#ifdef _PRE_WLAN_FEATURE_DBDC
     dmac_vap_dbdc_stop(pst_device, pst_down_vap);
#endif

    /* 通知算法DBAC/DBDC */
    dmac_alg_vap_down_notify(pst_down_vap);

    if (pst_up_vap != OAL_PTR_NULL)
    {
        /*中间经过dbdc的处理,up vap如果在辅路此时已经切回主路重新获取hal device */
        pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_down_vap);
        /* 有up/pause vap,vap信道不一致或者带宽不一致都重新设置信道,这里是不是应该通知算法? */
        /* 目前只会是5x 双ap时才会进入,命令关闭dbac */
        if ((pst_up_vap->st_channel.uc_chan_number != pst_hal_device->uc_current_chan_number) ||
            (pst_up_vap->st_channel.en_bandwidth != pst_hal_device->st_wifi_channel_status.en_bandwidth))
        {
            OAM_WARNING_LOG4(pst_down_vap->uc_vap_id, OAM_SF_DBAC, "dmac_vap_down_notify::has up vap, its bw[%d] chan[%d]!=hal_bw[%d] chan[%d],need switch chan",
                     pst_up_vap->st_channel.en_bandwidth,
                     pst_up_vap->st_channel.uc_chan_number,
                     pst_hal_device->st_wifi_channel_status.en_bandwidth,
                     pst_hal_device->uc_current_chan_number);

            pst_hal_device->st_hal_scan_params.st_home_channel = pst_up_vap->st_channel;
            dmac_scan_switch_channel_back(pst_device, pst_hal_device);
        }
    }
    dmac_vap_fake_queue_empty_assert(pst_down_vap, THIS_FILE_ID);
}
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)

oal_void  dmac_one_packet_send_null_data(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_ps)
{
    mac_fcs_mgr_stru               *pst_fcs_mgr;
    mac_fcs_cfg_stru               *pst_fcs_cfg;
    hal_one_packet_status_stru      st_status;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_fcs_mgr = dmac_fcs_get_mgr_stru(pst_mac_device);
    pst_fcs_cfg = MAC_DEV_GET_FCS_CFG(pst_mac_device);
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        return;
    }

    OAL_MEMZERO(pst_fcs_cfg, OAL_SIZEOF(mac_fcs_cfg_stru));
    dmac_fcs_prepare_one_packet_cfg(pst_mac_vap, &pst_fcs_cfg->st_one_packet_cfg, 20);
    dmac_fcs_send_one_packet_start(pst_fcs_mgr, &pst_fcs_cfg->st_one_packet_cfg, pst_hal_device, &st_status, en_ps);

    /* 开关PA只需要在02的产品中打开 */
    hal_disable_machw_phy_and_pa(pst_hal_device);

    /* 1102需要软复位mac，防止one packet和bt同时发生， 1103不需要 */
#if defined(_PRE_WLAN_FEATURE_BTCOEX) && defined(_PRE_PRODUCT_ID_HI1102_DEV)
    if(OAL_TRUE == DMAC_VAP_GET_BTCOEX_STATUS(pst_mac_vap)->un_bt_status.st_bt_status.bit_bt_on)
    {
        hal_reset_phy_machw(pst_hal_device, HAL_RESET_HW_TYPE_MAC, HAL_RESET_MAC_LOGIC, OAL_FALSE, OAL_FALSE);
    }
#endif

    hal_one_packet_stop(pst_hal_device);

    hal_recover_machw_phy_and_pa(pst_hal_device);
}
#ifdef _PRE_WLAN_FEATURE_DBAC

oal_void dmac_vap_restart_dbac(dmac_vap_stru  *pst_dmac_vap)
{
    mac_device_stru            *pst_mac_device;
    mac_vap_stru               *pst_mac_vap;
    mac_vap_stru               *pst_another_mac_vap;
    oal_uint8                   uc_another_mac_vap_id;
    oal_uint32                  ul_ret;

    pst_mac_vap    = &pst_dmac_vap->st_vap_base_info;
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_DBAC, "{dmac_vap_restart_dbac:: mac_res_get_dev fail[%d]!!!}", pst_mac_vap->uc_device_id);
        return;
    }

    if (OAL_FALSE == pst_mac_device->en_dbac_running)
    {
        return;
    }

    ul_ret  = hal_device_find_another_up_vap(pst_dmac_vap->pst_hal_device, pst_mac_vap->uc_vap_id, &uc_another_mac_vap_id);
    if (ul_ret!= OAL_SUCC)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_DBAC, "{dmac_vap_restart_dbac:: hal_device_find_another_up_vap fail[%d]!!!}", ul_ret);
        return;
    }

    pst_another_mac_vap = mac_res_get_mac_vap(uc_another_mac_vap_id);
    if (OAL_PTR_NULL == pst_another_mac_vap)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_DBAC, "{dmac_vap_restart_dbac:: mac_res_get_mac_vap[%d] fail!!!}", uc_another_mac_vap_id);
        return;
    }

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_DBAC, "{dmac_vap_restart_dbac::now vap[%d]another vap[%d]restart dbac!!!}",pst_mac_vap->uc_vap_id, uc_another_mac_vap_id);

    /* DBAC运行需要更新beacon周期时,此vap up是工作vap需要将硬件队列包搬到虚假队列,再交由DBAC处理 */
    dmac_dbac_switch_channel_off(pst_mac_device, pst_mac_vap, pst_another_mac_vap, &(pst_mac_vap->st_channel), 20);

    /* 通知算法DBAC/DBDC */
    dmac_alg_vap_down_notify(pst_mac_vap);
    dmac_alg_vap_up_notify(pst_mac_vap);
}
#endif
#endif /* _PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV */



oal_void  dmac_vap_resume_tx_by_chl(mac_device_stru *pst_device, hal_to_dmac_device_stru *pst_hal_device,  mac_channel_stru *pst_dst_channel)
{
    dmac_vap_stru                  *pst_dmac_vap;
    oal_uint8                       uc_vap_idx;
    oal_uint8                       uc_up_vap_num;
    oal_uint8                       auc_mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    dmac_user_stru                 *pst_dmac_user;

#ifdef _PRE_WLAN_FEATURE_STA_PM
    mac_sta_pm_handler_stru        *pst_sta_pm_handler;
#endif

    /* 只看自己的hal device上找 */
    uc_up_vap_num = hal_device_find_all_up_vap(pst_hal_device, auc_mac_vap_id);
    for (uc_vap_idx = 0; uc_vap_idx < uc_up_vap_num; uc_vap_idx++)
    {
        pst_dmac_vap = mac_res_get_dmac_vap(auc_mac_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_dmac_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_vap_resume_tx_by_chl::pst_dmac_vap null, vap id is %d.}",
                           auc_mac_vap_id[uc_vap_idx]);
            continue;
        }

        if (OAL_TRUE == mac_fcs_is_same_channel(&(pst_dmac_vap->st_vap_base_info.st_channel), pst_dst_channel))
        {
#ifdef _PRE_WLAN_FEATURE_ROAM
            /* 当回到漫游vap所在的信道时，不需要切vap的状态。*/
            if (MAC_VAP_STATE_ROAMING == pst_dmac_vap->st_vap_base_info.en_vap_state)
            {
                pst_dmac_user = mac_res_get_dmac_user(pst_dmac_vap->st_vap_base_info.us_assoc_vap_id);
                if(OAL_PTR_NULL == pst_dmac_user)
                {
                    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_DBAC, "{dmac_vap_resume_tx_by_chl::mac_res_get_dmac_user failed! us_assoc_vap_id=%d, vap_mode:%d, p2p_mode:%d}",
                                            pst_dmac_vap->st_vap_base_info.us_assoc_vap_id,
                                            pst_dmac_vap->st_vap_base_info.en_vap_mode,
                                            mac_get_p2p_mode(&(pst_dmac_vap->st_vap_base_info)));
                    return;
                }
            #ifdef _PRE_WLAN_FEATURE_STA_PM
                pst_sta_pm_handler = &pst_dmac_vap->st_sta_pm_handler;
                if (GET_PM_STATE(pst_sta_pm_handler) == STA_PWR_SAVE_STATE_ACTIVE)
            #endif
                {
            #if defined(_PRE_PRODUCT_ID_HI110X_DEV)
                    dmac_one_packet_send_null_data(pst_device, &pst_dmac_vap->st_vap_base_info, OAL_FALSE);
            #else
                    dmac_psm_send_null_data(pst_dmac_vap, pst_dmac_user, OAL_FALSE);
            #endif /* */
                }
                continue;
            }
#endif //_PRE_WLAN_FEATURE_ROAM
            /* 存在进入这个函数,信道相同,找到的vap不是pause的吗?*/
            if (MAC_VAP_STATE_PAUSE != pst_dmac_vap->st_vap_base_info.en_vap_state)
            {
#ifdef _PRE_WLAN_FEATURE_BTCOEX
                if(HAL_BTCOEX_SW_POWSAVE_SUB_CONNECT == GET_HAL_BTCOEX_SW_PREEMPT_SUBTYPE(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap)))
                {
                    /* 状态恢复 */
                    GET_HAL_BTCOEX_SW_PREEMPT_SUBTYPE(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap)) = HAL_BTCOEX_SW_POWSAVE_SUB_ACTIVE;
                    OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_vap_resume_tx_by_chl:: vap is up because powsave when connect!.}");
                }
                else
                {
                    OAM_WARNING_LOG3(0, OAM_SF_SCAN, "{dmac_vap_resume_tx_by_chl::vap id is %d, state[%d]not pause, preempt state[%d].}",
                           pst_dmac_vap->st_vap_base_info.uc_vap_id, pst_dmac_vap->st_vap_base_info.en_vap_state, GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device));
                }
#else
                OAM_WARNING_LOG2(0, OAM_SF_SCAN, "{dmac_vap_resume_tx_by_chl::vap id is %d.state[%d]not pause.}",
                           pst_dmac_vap->st_vap_base_info.uc_vap_id, pst_dmac_vap->st_vap_base_info.en_vap_state);
#endif
                continue;
            }

            /* 将虚假队列里的帧恢复到硬件队列中去 */
            dmac_vap_restore_tx_queue(&(pst_dmac_vap->st_vap_base_info));

            mac_vap_resume_tx(&(pst_dmac_vap->st_vap_base_info));
            hal_vap_beacon_resume(pst_dmac_vap->pst_hal_vap);

            /* 判断目的信道上的vap模式, 如果是sta则需要发节能位置0的null data */
            if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
            {
                pst_dmac_user = mac_res_get_dmac_user(pst_dmac_vap->st_vap_base_info.us_assoc_vap_id);
                if(OAL_PTR_NULL == pst_dmac_user)
                {
                    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_DBAC, "{dmac_vap_resume_tx_by_chl::mac_res_get_dmac_user failed! us_assoc_vap_id=%d. vap_mode:%d, p2p_mode:%d}",
                                            pst_dmac_vap->st_vap_base_info.us_assoc_vap_id,
                                            pst_dmac_vap->st_vap_base_info.en_vap_mode,
                                            mac_get_p2p_mode(&(pst_dmac_vap->st_vap_base_info)));
                    return;
                }
            #ifdef _PRE_WLAN_FEATURE_STA_PM
                pst_sta_pm_handler = &pst_dmac_vap->st_sta_pm_handler;
                if (GET_PM_STATE(pst_sta_pm_handler) == STA_PWR_SAVE_STATE_ACTIVE)
            #endif
                {
            #if defined(_PRE_PRODUCT_ID_HI110X_DEV)
                    dmac_one_packet_send_null_data(pst_device, &pst_dmac_vap->st_vap_base_info, OAL_FALSE);
            #else
                    dmac_psm_send_null_data(pst_dmac_vap, pst_dmac_user, OAL_FALSE);
            #endif /* */
                }
            }
        }
    }

    dmac_tx_complete_schedule(pst_hal_device, WLAN_WME_AC_BE);
}

oal_void dmac_vap_update_snr_info(dmac_vap_stru  *pst_dmac_vap,
                                       dmac_rx_ctl_stru     *pst_rx_ctrl,
                                   mac_ieee80211_frame_stru *pst_frame_hdr)
{
    if (OAL_TRUE == pst_rx_ctrl->st_rx_status.bit_last_mpdu_flag)
    {
        pst_dmac_vap->st_query_stats.c_snr_ant0 = pst_rx_ctrl->st_rx_statistic.c_snr_ant0;
        pst_dmac_vap->st_query_stats.c_snr_ant1 = pst_rx_ctrl->st_rx_statistic.c_snr_ant1;
    }
}


oal_void dmac_vap_work_set_channel(dmac_vap_stru *pst_dmac_vap)
{
    mac_vap_stru    *pst_vap_up;
    mac_device_stru *pst_mac_device;
    mac_vap_stru    *pst_mac_vap = &pst_dmac_vap->st_vap_base_info;
    mac_channel_stru st_channel;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_vap_work_set_channel::pst_mac_device null.}");
        return;
    }

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        mac_device_find_up_sta(pst_mac_device, &pst_vap_up);
    }
    else if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        /* 如果有处在UP状态的VAP，此次入网切信道需要发保护帧 */
        pst_vap_up = mac_device_find_another_up_vap(pst_mac_device, pst_mac_vap->uc_vap_id);
    }
    else
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_vap_work_set_channel::error mac vap mode [%d].}", pst_mac_vap->en_vap_mode);
        return;
    }

    dmac_alg_cfg_channel_notify(pst_mac_vap, CH_BW_CHG_TYPE_MOVE_WORK);
    dmac_alg_cfg_bandwidth_notify(pst_mac_vap, CH_BW_CHG_TYPE_MOVE_WORK);

    /* 多个VAP情况下 根据实际最大的带宽设置硬件带宽 */
    st_channel = pst_dmac_vap->st_vap_base_info.st_channel;
    dmac_chan_select_real_channel(pst_mac_device, &st_channel, st_channel.uc_chan_number);

    dmac_mgmt_connect_set_channel(pst_mac_device, pst_dmac_vap->pst_hal_device, pst_vap_up, &st_channel);

    /* 初始化发送功率 */
    dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_INIT);
}


oal_void dmac_sta_bw_switch_fsm_attach(dmac_vap_stru *pst_dmac_vap)
{
    dmac_sta_bw_switch_fsm_info_stru       *pst_handler      = OAL_PTR_NULL;
    oal_uint8                               auc_fsm_name[16] = {0};
    oal_uint32                              ul_ret;

    pst_dmac_vap->pst_sta_bw_switch_fsm = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_sta_bw_switch_fsm_info_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_dmac_vap->pst_sta_bw_switch_fsm)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "{dmac_sta_bw_switch_fsm_attach::pst_sta_bw_switch_fsm null.}");
        return;
    }
    OAL_MEMZERO(pst_dmac_vap->pst_sta_bw_switch_fsm, OAL_SIZEOF(dmac_sta_bw_switch_fsm_info_stru));
    pst_handler = (dmac_sta_bw_switch_fsm_info_stru *)pst_dmac_vap->pst_sta_bw_switch_fsm;

    /* 准备一个唯一的fsmname */
    auc_fsm_name[0] = (oal_uint8)(pst_dmac_vap->st_vap_base_info.ul_core_id);
    auc_fsm_name[1] = pst_dmac_vap->st_vap_base_info.uc_chip_id;
    auc_fsm_name[2] = pst_dmac_vap->st_vap_base_info.uc_device_id;
    auc_fsm_name[3] = pst_dmac_vap->st_vap_base_info.uc_vap_id;
    auc_fsm_name[4] = pst_dmac_vap->st_vap_base_info.en_vap_mode;
    auc_fsm_name[5] = 'B';
    auc_fsm_name[6] = 'W';

    ul_ret = oal_fsm_create(pst_dmac_vap,
                            auc_fsm_name,
                            &(pst_dmac_vap->st_vap_base_info),
                            &(pst_handler->st_oal_fsm),
                            DMAC_STA_BW_SWITCH_FSM_INIT,
                            g_sta_bw_switch_fsm_info,
                            OAL_SIZEOF(g_sta_bw_switch_fsm_info)/OAL_SIZEOF(oal_fsm_state_info));

    if (OAL_SUCC != ul_ret)
    {
        pst_handler->en_is_fsm_attached = OAL_FALSE;
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "dmac_sta_bw_switch_fsm_attach::oal_fsm_create fail.");
        return;
    }

    pst_handler->en_is_fsm_attached     = OAL_TRUE;
    OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                     "dmac_sta_bw_switch_fsm_attach::vap_mode=%d p2p_mode=%d attached succ.",
                     pst_dmac_vap->st_vap_base_info.en_vap_mode, pst_dmac_vap->st_vap_base_info.en_p2p_mode);

    return;
}


oal_void dmac_sta_bw_switch_fsm_detach(dmac_vap_stru *pst_dmac_vap)
{
    dmac_sta_bw_switch_fsm_info_stru         *pst_bw_fsm    = OAL_PTR_NULL;

    pst_bw_fsm = (dmac_sta_bw_switch_fsm_info_stru *)pst_dmac_vap->pst_sta_bw_switch_fsm;
    if (OAL_PTR_NULL == pst_bw_fsm)
    {
        return;
    }

    if (OAL_FALSE == pst_bw_fsm->en_is_fsm_attached)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC, "{dmac_sta_bw_switch_fsm_detach::pst_bw_fsm not attatched}");
        return;
    }

    OAL_MEM_FREE(pst_bw_fsm, OAL_TRUE);
    pst_dmac_vap->pst_sta_bw_switch_fsm = OAL_PTR_NULL;

    return;
}


oal_uint8 dmac_sta_bw_switch_need_new_verify(dmac_vap_stru *pst_dmac_vap, wlan_bw_cap_enum_uint8  en_bw_becaon_new)
{
    wlan_bw_cap_enum_uint8 en_vap_bw_cap;

    if (!IS_LEGACY_STA(&pst_dmac_vap->st_vap_base_info))
    {
        return DMAC_STA_BW_VERIFY_SWITCH_BUTT;
    }

    if (DMAC_STA_BW_SWITCH_FSM_NORMAL == MAC_VAP_GET_CURREN_BW_STATE(&pst_dmac_vap->st_vap_base_info))
    {
        mac_vap_get_bandwidth_cap(&pst_dmac_vap->st_vap_base_info, &en_vap_bw_cap);

        if ((WLAN_BW_CAP_20M == en_vap_bw_cap) && (WLAN_BW_CAP_40M == en_bw_becaon_new))
        {
            return DMAC_STA_BW_VERIFY_20M_TO_40M;
        }
        else if ((WLAN_BW_CAP_40M == en_vap_bw_cap) && (WLAN_BW_CAP_20M == en_bw_becaon_new))
        {
            return DMAC_STA_BW_VERIFY_40M_TO_20M;
        }
    }
    else if (DMAC_STA_BW_SWITCH_FSM_VERIFY20M == MAC_VAP_GET_CURREN_BW_STATE(&pst_dmac_vap->st_vap_base_info))
    {
        if (WLAN_BW_CAP_40M == en_bw_becaon_new)
        {
            return DMAC_STA_BW_VERIFY_20M_TO_40M;
        }
    }
    else if (DMAC_STA_BW_SWITCH_FSM_VERIFY40M == MAC_VAP_GET_CURREN_BW_STATE(&pst_dmac_vap->st_vap_base_info))
    {
        if (WLAN_BW_CAP_20M == en_bw_becaon_new)
        {
            return DMAC_STA_BW_VERIFY_40M_TO_20M;
        }
    }

    return DMAC_STA_BW_VERIFY_SWITCH_BUTT;
}


OAL_STATIC oal_uint32 dmac_bw_fsm_trans_to_state(dmac_sta_bw_switch_fsm_info_stru *pst_bw_fsm, oal_uint8 uc_state)
{
    oal_fsm_stru   *pst_oal_fsm = &(pst_bw_fsm->st_oal_fsm);

    OAM_WARNING_LOG2(0, OAM_SF_ASSOC, "dmac_bw_fsm_trans_to_state:trans from %d to %d",
                     pst_oal_fsm->uc_cur_state, uc_state);

    return oal_fsm_trans_to_state(pst_oal_fsm, uc_state);
}


oal_void dmac_sta_bw_switch_fsm_init(dmac_vap_stru  *pst_dmac_vap)
{
    dmac_sta_bw_switch_fsm_info_stru *pst_sta_bw_switch_fsm;  /* 带宽切换状态机 */

    pst_sta_bw_switch_fsm = (dmac_sta_bw_switch_fsm_info_stru *)(pst_dmac_vap->pst_sta_bw_switch_fsm);
    pst_sta_bw_switch_fsm->uc_20M_Verify_fail_cnt = 0;
    pst_sta_bw_switch_fsm->uc_40M_Verify_fail_cnt = 0;
}


OAL_STATIC oal_void dmac_sta_bw_switch_fsm_init_entry(oal_void *p_ctx)
{
    dmac_vap_stru                    *pst_dmac_vap = (dmac_vap_stru *)p_ctx;

    dmac_sta_bw_switch_fsm_init(pst_dmac_vap);
}


OAL_STATIC oal_void dmac_sta_bw_switch_fsm_normal_entry(oal_void *p_ctx)
{
    dmac_vap_stru                    *pst_dmac_vap = (dmac_vap_stru *)p_ctx;

    dmac_sta_bw_switch_fsm_init(pst_dmac_vap);
}


OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_init_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data)
{
    dmac_vap_stru   *pst_dmac_vap  = (dmac_vap_stru *)p_ctx;
    dmac_sta_bw_switch_fsm_info_stru *pst_bw_fsm = (dmac_sta_bw_switch_fsm_info_stru *)(pst_dmac_vap->pst_sta_bw_switch_fsm);

    switch(us_event)
    {
        case DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC:
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_NORMAL);
            break;

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                                  "dmac_sta_bw_switch_fsm_init_event::get invalid event[%d]", us_event);
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_normal_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data)
{
    dmac_vap_stru   *pst_dmac_vap  = (dmac_vap_stru *)p_ctx;
    dmac_sta_bw_switch_fsm_info_stru *pst_bw_fsm = (dmac_sta_bw_switch_fsm_info_stru *)(pst_dmac_vap->pst_sta_bw_switch_fsm);

    switch(us_event)
    {
        case DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "dmac_sta_bw_switch_fsm_normal_event::event[%d] received!", DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC);
            dmac_sta_bw_switch_fsm_init(pst_dmac_vap);
            break;

        case DMAC_STA_BW_SWITCH_EVENT_BEACON_20M:
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_VERIFY20M);
            break;

        case DMAC_STA_BW_SWITCH_EVENT_BEACON_40M:
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_VERIFY40M);
            break;

        case DMAC_STA_BW_SWITCH_EVENT_USER_DEL:
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_INIT);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}


wlan_bw_cap_enum_uint8 dmac_sta_bw_rx_assemble_to_bandwith(hal_channel_assemble_enum_uint8 uc_bw)
{
    if (WLAN_BAND_ASSEMBLE_20M == uc_bw)
    {
        return WLAN_BW_CAP_20M;
    }
    else if (WLAN_BAND_ASSEMBLE_40M == uc_bw || WLAN_BAND_ASSEMBLE_40M_DUP == uc_bw)
    {
        return WLAN_BW_CAP_40M;
    }

    return WLAN_BW_CAP_BUTT;
}


OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_verify_20m_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data)
{
    dmac_vap_stru                    *pst_dmac_vap  = (dmac_vap_stru *)p_ctx;
    dmac_sta_bw_switch_fsm_info_stru *pst_bw_fsm = (dmac_sta_bw_switch_fsm_info_stru *)(pst_dmac_vap->pst_sta_bw_switch_fsm);
    oal_uint8                         uc_bw;
    dmac_rx_ctl_stru                 *pst_rx_ctl;
    dmac_user_stru                   *pst_dmac_user;

    switch(us_event)
    {
        case DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "dmac_sta_bw_switch_fsm_verify_20m_event::event[%d] received!", DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC);
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_NORMAL);
            break;

        case DMAC_STA_BW_SWITCH_EVENT_BEACON_40M:
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_VERIFY40M);
            break;

        case DMAC_STA_BW_SWITCH_EVENT_RX_UCAST_DATA_COMPLETE:
            if (p_event_data && (us_event_data_len == OAL_SIZEOF(dmac_rx_ctl_stru)))
            {
                pst_rx_ctl  = (dmac_rx_ctl_stru *)p_event_data;

                uc_bw = (oal_uint8)(pst_rx_ctl->st_rx_status.bit_freq_bandwidth_mode & 0x0F);
                pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_RX_CB_TA_USER_IDX(&pst_rx_ctl->st_rx_info));
                if (OAL_PTR_NULL == pst_dmac_user)
                {
                    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                      "{dmac_sta_bw_switch_fsm_verify_20m_event::pst_dmac_user[%d] null.}",
                                      MAC_GET_RX_CB_TA_USER_IDX(&pst_rx_ctl->st_rx_info));
                    return OAL_ERR_CODE_PTR_NULL;
                }

                if (WLAN_BW_CAP_20M == dmac_sta_bw_rx_assemble_to_bandwith(uc_bw))
                {
                    dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_NORMAL);
                    dmac_sta_set_bandwith_handler(pst_dmac_vap, WLAN_BW_CAP_20M);
                    dmac_config_d2h_user_info_syn(&pst_dmac_vap->st_vap_base_info, &pst_dmac_user->st_user_base_info);
                }
                else
                {
                    pst_bw_fsm->uc_20M_Verify_fail_cnt++;
                }

                if (pst_bw_fsm->uc_20M_Verify_fail_cnt > DMAC_BW_VERIFY_MAX_THRESHOLD)
                {
                    OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                                     "dmac_sta_bw_switch_fsm_verify_20m_event::verify counts outnumbers threshold caused invalid!");
                    dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_INVALID);
                    dmac_config_d2h_user_info_syn(&pst_dmac_vap->st_vap_base_info, &pst_dmac_user->st_user_base_info);
                }
            }
            break;

        case DMAC_STA_BW_SWITCH_EVENT_USER_DEL:
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_INIT);
            break;

        default:
            OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                              "dmac_sta_bw_switch_fsm_verify_20m_event::get invalid event[%d]", us_event);
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_verify_40m_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data)
{
    dmac_vap_stru                    *pst_dmac_vap  = (dmac_vap_stru *)p_ctx;
    dmac_sta_bw_switch_fsm_info_stru *pst_bw_fsm = (dmac_sta_bw_switch_fsm_info_stru *)(pst_dmac_vap->pst_sta_bw_switch_fsm);
    oal_uint8                         uc_bw;
    dmac_rx_ctl_stru                 *pst_rx_ctl;
    dmac_user_stru                   *pst_dmac_user;

    switch(us_event)
    {
        case DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                             "dmac_sta_bw_switch_fsm_verify_40m_event::event[%d] received!", DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC);
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_NORMAL);
            break;

        case DMAC_STA_BW_SWITCH_EVENT_BEACON_20M:
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_VERIFY20M);
            break;

        case DMAC_STA_BW_SWITCH_EVENT_RX_UCAST_DATA_COMPLETE:
            if (p_event_data && (us_event_data_len == OAL_SIZEOF(dmac_rx_ctl_stru)))
            {
                pst_rx_ctl  = (dmac_rx_ctl_stru *)p_event_data;
                uc_bw = (oal_uint8)(pst_rx_ctl->st_rx_status.bit_freq_bandwidth_mode & 0x0F);

                pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_RX_CB_TA_USER_IDX(&pst_rx_ctl->st_rx_info));
                if (OAL_PTR_NULL == pst_dmac_user)
                {
                    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                                      "{dmac_sta_bw_switch_fsm_verify_40m_event::pst_dmac_user[%d] null.}",
                                      MAC_GET_RX_CB_TA_USER_IDX(&pst_rx_ctl->st_rx_info));
                    return OAL_ERR_CODE_PTR_NULL;
                }

                if (WLAN_BW_CAP_40M == dmac_sta_bw_rx_assemble_to_bandwith(uc_bw))
                {
                    dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_NORMAL);
                    dmac_user_set_bandwith_handler(&pst_dmac_vap->st_vap_base_info, pst_dmac_user, WLAN_BW_CAP_40M);
                    dmac_config_d2h_user_info_syn(&pst_dmac_vap->st_vap_base_info, &pst_dmac_user->st_user_base_info);
                }
                else
                {
                    pst_bw_fsm->uc_40M_Verify_fail_cnt++;
                }

                if (pst_bw_fsm->uc_40M_Verify_fail_cnt > DMAC_BW_VERIFY_MAX_THRESHOLD)
                {
                    OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                                     "dmac_sta_bw_switch_fsm_verify_40m_event::verify counts outnumbers threshold caused invalid!");
                    dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_INVALID);
                    dmac_config_d2h_user_info_syn(&pst_dmac_vap->st_vap_base_info, &pst_dmac_user->st_user_base_info);
                }
            }
            break;

        case DMAC_STA_BW_SWITCH_EVENT_USER_DEL:
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_INIT);
            break;

        default:
            OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                              "dmac_sta_bw_switch_fsm_verify_40m_event::get invalid event[%d]", us_event);
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_sta_bw_switch_fsm_invalid_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data)
{
    dmac_vap_stru   *pst_dmac_vap  = (dmac_vap_stru *)p_ctx;
    dmac_sta_bw_switch_fsm_info_stru *pst_bw_fsm = (dmac_sta_bw_switch_fsm_info_stru *)(pst_dmac_vap->pst_sta_bw_switch_fsm);

    switch(us_event)
    {
        case DMAC_STA_BW_SWITCH_EVENT_USER_DEL:
            dmac_bw_fsm_trans_to_state(pst_bw_fsm, DMAC_STA_BW_SWITCH_FSM_INIT);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_DBDC

oal_uint32  dmac_up_vap_change_hal_dev(mac_vap_stru *pst_shift_mac_vap)
{
    oal_bool_enum_uint8      en_switch_alg;
    dmac_device_stru        *pst_dmac_device;

    pst_dmac_device = dmac_res_get_mac_dev(pst_shift_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(pst_shift_mac_vap->uc_device_id, OAM_SF_DBDC, "{dmac_dbdc_switch_vap::pst_dmac_device[%d] null.}", pst_shift_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 1、从原来的hal device上保护切离vap pause */
    dmac_m2s_switch_vap_off(DMAC_VAP_GET_HAL_DEVICE(pst_shift_mac_vap), pst_dmac_device->pst_device_base_info, pst_shift_mac_vap);

    /* 2、算法hal device迁移 */
    en_switch_alg = dmac_alg_dbdc_shift_alg(pst_shift_mac_vap, DMAC_ALG_DBDC_UP_VAP_SWITCH);

    /* 3、寄存器迁移,vap挂接的hal vap hal device指针切换 */
    dmac_dbdc_vap_hal_device_shift(pst_dmac_device, pst_shift_mac_vap);

    /* 4、未迁移算法需要重新注册vap */
    if (OAL_FALSE == en_switch_alg)
    {
        dmac_alg_create_vap_notify(MAC_GET_DMAC_VAP(pst_shift_mac_vap));
    }

    /* 5.只更新本vap和user能力,vap能力基于hal device能力来, user能力基于vap能力来 */
    dmac_m2s_update_vap_capbility(pst_dmac_device->pst_device_base_info, pst_shift_mac_vap);


    /* 6.恢复vap发送状态 */
    dmac_dbdc_switch_vap_back(pst_dmac_device->pst_device_base_info, pst_shift_mac_vap);

    return OAL_SUCC;
}

oal_uint32 dmac_vap_change_hal_dev_before_up(mac_vap_stru *pst_shift_vap, oal_void *pst_dmac_dev)
{
    hal_to_dmac_device_stru      *pst_shift_hal_device;
    hal_to_dmac_device_stru      *pst_ori_hal_device;
    hal_to_dmac_vap_stru         *pst_shift_hal_vap;
    hal_to_dmac_vap_stru         *pst_ori_hal_vap;
    dmac_device_stru             *pst_dmac_device = (dmac_device_stru *)pst_dmac_dev;

    pst_ori_hal_device   = DMAC_VAP_GET_HAL_DEVICE(pst_shift_vap);

    pst_shift_hal_device = dmac_device_get_another_h2d_dev(pst_dmac_device, pst_ori_hal_device);
    if (OAL_PTR_NULL == pst_shift_hal_device)
    {
        OAM_ERROR_LOG1(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_change_hal_dev_before_up::pst_shift_hal_device NULL,orig id[%d]",pst_ori_hal_device->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ori_hal_vap = DMAC_VAP_GET_HAL_VAP(pst_shift_vap);
    hal_get_hal_vap(pst_shift_hal_device, pst_ori_hal_vap->uc_vap_id, &pst_shift_hal_vap);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P 设置MAC 地址 */
    if ((WLAN_P2P_DEV_MODE == pst_shift_vap->en_p2p_mode) && (WLAN_VAP_MODE_BSS_STA == pst_shift_vap->en_vap_mode))
    {
        hal_vap_set_macaddr(pst_shift_hal_vap, pst_shift_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID);
    }
    else
    {
        /* 设置其他vap 的mac 地址 */
        hal_vap_set_macaddr(pst_shift_hal_vap, mac_mib_get_StationID(pst_shift_vap));
    }
#else
    /* 配置MAC地址 */
    hal_vap_set_macaddr(pst_shift_hal_vap, mac_mib_get_StationID(pst_shift_vap));
#endif

    /* 使能PA_CONTROL的vap_control位 */
    hal_vap_set_opmode(pst_shift_hal_vap, pst_shift_vap->en_vap_mode);

    /* 目前5g切到辅路,不需要配置 */
    if (WLAN_BAND_2G == pst_shift_vap->st_channel.en_band)
    {
        hal_set_11b_reuse_sel(pst_shift_hal_device);
    }

    /* 先通知算法去注册此vap */
    dmac_alg_delete_vap_notify(MAC_GET_DMAC_VAP(pst_shift_vap));

    /* hal device,hal vap指针替换 */
    DMAC_VAP_GET_HAL_DEVICE(pst_shift_vap) = pst_shift_hal_device;
    DMAC_VAP_GET_HAL_VAP(pst_shift_vap)    = pst_shift_hal_vap;

#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_P2P_DEV_MODE == pst_shift_vap->en_p2p_mode)
    {
        MAC_GET_DMAC_VAP(pst_shift_vap)->pst_p2p0_hal_vap = pst_shift_hal_vap;
    }
#endif /* _PRE_WLAN_FEATURE_P2P */

    /* 切过hal device指针后,算法注册此vap */
    dmac_alg_create_vap_notify(MAC_GET_DMAC_VAP(pst_shift_vap));

    dmac_m2s_update_vap_capbility(pst_dmac_device->pst_device_base_info, pst_shift_vap);

    OAM_WARNING_LOG2(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_vap_change_hal_dev_before_up::vap[%d]change hal device to[%d]!!!}",
                    pst_shift_vap->uc_vap_id, pst_shift_hal_device->uc_device_id);
    return OAL_SUCC;
}

oal_uint32  dmac_vap_change_hal_dev(mac_vap_stru *pst_shift_vap, mac_dbdc_debug_switch_stru *pst_dbdc_debug_switch)
{
    hal_to_dmac_device_stru      *pst_ori_hal_device;
    dmac_device_stru             *pst_dmac_device;
    oal_uint8                     uc_dst_hal_dev_id;

    uc_dst_hal_dev_id    = pst_dbdc_debug_switch->uc_dst_hal_dev_id;
    pst_ori_hal_device   = DMAC_VAP_GET_HAL_DEVICE(pst_shift_vap);

    if ((uc_dst_hal_dev_id == pst_ori_hal_device->uc_device_id) || (uc_dst_hal_dev_id >= HAL_DEVICE_ID_BUTT))
    {
        OAM_WARNING_LOG3(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_vap_change_hal_dev_before_up::dst hal device[%d],ori hal device[%d].but max hal dev id[%d].!!!}",
                        uc_dst_hal_dev_id, pst_ori_hal_device->uc_device_id, HAL_DEVICE_ID_SLAVE);
        return OAL_FAIL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_shift_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "dmac_config_dbdc_debug_switch::pst_dmac_device id[%d]NULL",pst_shift_vap->uc_device_id);
        return OAL_FAIL;
    }

    if (OAL_FALSE == dmac_device_is_support_double_hal_device(pst_dmac_device))
    {
        OAM_WARNING_LOG1(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_vap_change_hal_dev_before_up::mac device[%d]not support double hal device.!!}",
                        pst_shift_vap->uc_device_id);
        return OAL_FAIL;
    }

    /* wifi可能处于扫描状态，暂时直接abort */
    dmac_scan_abort(pst_dmac_device->pst_device_base_info);

    if (HAL_DEVICE_ID_SLAVE == uc_dst_hal_dev_id)
    {
        dmac_m2s_handle_event(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device), HAL_M2S_EVENT_DBDC_START, 0, OAL_PTR_NULL);
        dmac_dbdc_switch_vap_to_slave(pst_dmac_device, pst_shift_vap);//再将主路的vap切到辅路去
    }
    else if (HAL_DEVICE_ID_MASTER == uc_dst_hal_dev_id)
    {
        dmac_dbdc_handle_stop_event(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device));
        dmac_m2s_handle_event(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device), HAL_M2S_EVENT_DBDC_STOP, 0, OAL_PTR_NULL);
    }

    return OAL_SUCC;
}

oal_bool_enum_uint8 dmac_dbdc_channel_check(mac_channel_stru *pst_channel1, mac_channel_stru  *pst_channel2)
{
    oal_uint8           uc_5g_chan_number;
    oal_uint8           uc_2g_chan_number;
    oal_bool_enum_uint8 en_can_be_together = OAL_TRUE;

    if (pst_channel1->en_band == pst_channel2->en_band)
    {
        OAM_ERROR_LOG3(0, OAM_SF_DBDC, "dmac_dbdc_channel_check::same band[%d]!!!chan1[%d]chan2[%d]",
                            pst_channel1->en_band, pst_channel1->uc_chan_number, pst_channel2->uc_chan_number);
        return OAL_TRUE;
    }

    if (WLAN_BAND_5G == pst_channel1->en_band)
    {
        uc_5g_chan_number = pst_channel1->uc_chan_number;
        uc_2g_chan_number = pst_channel2->uc_chan_number;
    }
    else
    {
        uc_2g_chan_number = pst_channel1->uc_chan_number;
        uc_5g_chan_number = pst_channel2->uc_chan_number;
    }

    if ((uc_5g_chan_number < 100) || (uc_5g_chan_number > 120))
    {
        return en_can_be_together;
    }

    /* 5G    2.4G
       100   4-11
       104   6-13
       108   8-13
       112   9-13
       116   11-13
       120   13
    这些组合不能DBDC*/
    switch (uc_5g_chan_number)
    {
        case 100:
        if ((uc_2g_chan_number >= 4) && (uc_2g_chan_number <= 11))
        {
            en_can_be_together = OAL_FALSE;
        }
        break;

        case 104:
        if ((uc_2g_chan_number >= 6) && (uc_2g_chan_number <= 13))
        {
            en_can_be_together = OAL_FALSE;
        }
        break;

        case 108:
        if ((uc_2g_chan_number >= 8) && (uc_2g_chan_number <= 13))
        {
            en_can_be_together =  OAL_FALSE;
        }
        break;

        case 112:
        if ((uc_2g_chan_number >= 9) && (uc_2g_chan_number <= 13))
        {
            en_can_be_together = OAL_FALSE;
        }
        break;

        case 116:
        if ((uc_2g_chan_number >= 11) && (uc_2g_chan_number <= 13))
        {
            en_can_be_together = OAL_FALSE;
        }
        break;

        case 120:
        if (13 == uc_2g_chan_number)
        {
            en_can_be_together = OAL_FALSE;
        }
        break;

        default:
            OAM_ERROR_LOG2(0, OAM_SF_DBDC, "dmac_dbdc_start_check_by_channel::2g channel[%d],5g channel[%d]shoule not be here!!!",uc_2g_chan_number,uc_5g_chan_number);
        break;

    }

    if (OAL_FALSE == en_can_be_together)
    {
        OAM_WARNING_LOG2(0, OAM_SF_DBDC, "dmac_dbdc_start_check_by_channel::2g channel[%d],5g channel[%d] can not start dbdc!!",uc_2g_chan_number,uc_5g_chan_number);
    }

    return en_can_be_together;
}


oal_void dmac_vap_dbdc_start(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap)
{
    dmac_device_stru             *pst_dmac_device;
    mac_vap_stru                 *pst_2g_mac_vap;     //2g需要迁移的vap
    mac_vap_stru                 *pst_5g_mac_vap;
    mac_vap_stru                 *pst_shift_mac_vap;     //需要迁移的vap
    mac_vap_stru                 *pst_another_up_vap;

    pst_another_up_vap = mac_device_find_another_up_vap(pst_mac_device, pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_another_up_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_start::only one up vap not start dbdc");
        return;
    }

    /* 同频不进dbdc逻辑 */
    if (pst_mac_vap->st_channel.en_band == pst_another_up_vap->st_channel.en_band)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_start::2vap the same band[%d]", pst_mac_vap->st_channel.en_band);
        return;
    }

    /* 静态dbdc两个业务vap不同hal device */
    if (DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap) != DMAC_VAP_GET_HAL_DEVICE(pst_another_up_vap))
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_start:2vap[%d][%d] diff hal device before dbdc",
                                    pst_mac_vap->uc_vap_id,pst_another_up_vap->uc_vap_id);
        return;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_start::pst_dmac_device id[%d]NULL",pst_mac_vap->uc_device_id);
        return;
    }

    if ((OAL_FALSE == dmac_device_is_support_double_hal_device(pst_dmac_device)) ||
            (OAL_FALSE == pst_dmac_device->en_dbdc_enable))
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_start::dmac device[%d]not support double hal device,dbdc enable[%d]",
                        pst_dmac_device->pst_device_base_info->uc_device_id, pst_dmac_device->en_dbdc_enable);
        return;
    }

    /* 选出需要迁移的vap,2G迁移,5G保留 */
    if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
    {
        pst_2g_mac_vap = pst_mac_vap;
        pst_5g_mac_vap = pst_another_up_vap;
    }
    else
    {
        pst_2g_mac_vap = pst_another_up_vap;
        pst_5g_mac_vap = pst_mac_vap;
    }

#ifdef _PRE_WLAN_1103_MPW2_BUGFIX
    pst_shift_mac_vap    = pst_5g_mac_vap; //目前只能5g最大20M到辅路

    if (pst_shift_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M)
    {
        OAM_WARNING_LOG3(pst_shift_mac_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_start::shift vap channel[%d],bw[%d]>slave max bw[%d]",
                pst_shift_mac_vap->st_channel.uc_chan_number, pst_shift_mac_vap->st_channel.en_bandwidth, WLAN_BAND_WIDTH_20M);
        return;
    }
#else
    pst_shift_mac_vap    = pst_2g_mac_vap;
#endif


    dmac_scan_abort(pst_mac_device);
    dmac_dbdc_renew_pm_tbtt_offset(DMAC_DBDC_START);
    dmac_dbdc_start_renew_dev(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device));

#ifdef _PRE_WLAN_FEATURE_ROAM
    if (MAC_VAP_STATE_ROAMING == pst_shift_mac_vap->en_vap_state)
    {
        /* 漫游先在主路,后漫游到辅路,此时没有vap down的动作,需要从原有的hal device上线去注册*/
        hal_device_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_shift_mac_vap), HAL_DEVICE_EVENT_VAP_DOWN,
                            OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(MAC_GET_DMAC_VAP(pst_shift_mac_vap)->pst_hal_vap));
    }
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    dmac_m2s_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_shift_mac_vap), HAL_M2S_EVENT_DBDC_START, 0, OAL_PTR_NULL);
#endif

    dmac_dbdc_switch_vap_to_slave(pst_dmac_device, pst_shift_mac_vap);//再将主路的vap切到辅路去

    pst_mac_device->en_dbdc_running = OAL_TRUE;

    OAM_WARNING_LOG4(pst_2g_mac_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_start::2gvap mode[%d]channel[%d],5gvap mode[%d],channel[%d]",
                pst_2g_mac_vap->en_vap_mode, pst_2g_mac_vap->st_channel.uc_chan_number,
                pst_5g_mac_vap->en_vap_mode, pst_5g_mac_vap->st_channel.uc_chan_number);

    OAM_WARNING_LOG4(pst_2g_mac_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_start::2gvap[%d] in hal dev[%d],5gvap[%d] in hal dev[%d]",
                pst_2g_mac_vap->uc_vap_id,DMAC_VAP_GET_HAL_DEVICE(pst_2g_mac_vap)->uc_device_id,
                pst_5g_mac_vap->uc_vap_id,DMAC_VAP_GET_HAL_DEVICE(pst_5g_mac_vap)->uc_device_id);

    //hal_dft_report_all_reg_state(DMAC_VAP_GET_HAL_DEVICE(pst_shift_mac_vap));
}


oal_void dmac_vap_dbdc_stop(mac_device_stru *pst_mac_device, mac_vap_stru *pst_down_vap)
{
    hal_to_dmac_device_stru *pst_down_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_down_vap);

    if (!mac_is_dbdc_running(pst_mac_device))
    {
        return;
    }

    /* 从原有hal dev上去注册1、硬件队列无包,直接切到idle释放rf资源,
    2、硬件队列有包,等待抛事件切idle,置了此hal device等待硬件队列的标记,最后一个包的释放是否有问题,减到另一路上去了 */
    hal_device_handle_event(pst_down_hal_device, HAL_DEVICE_EVENT_VAP_DOWN, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)DMAC_VAP_GET_HAL_VAP(pst_down_vap));

    pst_mac_device->en_dbdc_running = OAL_FALSE;

    /* 另一路mac pa未关,rf资源未释放不能直接切回mimo */
    if (OAL_TRUE == pst_down_hal_device->en_is_mac_pa_enabled)
    {
        OAM_WARNING_LOG2(pst_down_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_stop::down vap[%d]in hal[%d] pa is enabled not ready s->m",
                        pst_down_vap->uc_vap_id, DMAC_VAP_GET_HAL_DEVICE(pst_down_vap)->uc_device_id);
        pst_down_hal_device->en_wait_for_s2m = OAL_TRUE;
        return;
    }

    OAM_WARNING_LOG2(pst_down_vap->uc_vap_id, OAM_SF_DBDC, "dmac_vap_dbdc_stop::down vap[%d]in hal[%d]",
                               pst_down_vap->uc_vap_id, DMAC_VAP_GET_HAL_DEVICE(pst_down_vap)->uc_device_id);


    dmac_dbdc_handle_stop_event(pst_down_hal_device);

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 去关联后up的在主路需要切回到mimo */
    dmac_m2s_handle_event(pst_down_hal_device, HAL_M2S_EVENT_DBDC_STOP, 0, OAL_PTR_NULL);
#endif
}

#endif

oal_bool_enum_uint8  dmac_vap_is_host(mac_vap_stru *pst_vap)
{
    return ((dmac_vap_stru *)pst_vap)->en_is_host_vap;
}

oal_void  dmac_vap_update_bi_from_hw(mac_vap_stru *pst_mac_vap)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    dmac_vap_stru *pst_dmac_vap;
    oal_uint32     ul_bcn_period;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG1(0,OAM_SF_SCAN,"{dmac_vap_update_bi_from_hw::mac_res_get_dmac_vap fail.vap_id[%u]}",pst_mac_vap->uc_vap_id);
        return;
    }

    hal_vap_get_beacon_period(pst_dmac_vap->pst_hal_vap, &ul_bcn_period);
    mac_mib_set_BeaconPeriod(pst_mac_vap, ul_bcn_period);

#endif

}

/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_vap_resume_tx_by_chl);
oal_module_symbol(dmac_vap_restore_tx_queue);
oal_module_symbol(dmac_vap_save_tx_queue);
oal_module_symbol(dmac_vap_is_host);
oal_module_symbol(dmac_vap_update_bi_from_hw);

/*lint +e578*//*lint +e19*/









#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

