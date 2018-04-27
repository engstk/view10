
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_ext_if.h"
#include "oal_net.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "oal_schedule.h"
#endif
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "mac_regdomain.h"
#include "mac_resource.h"
#include "mac_device.h"
#include "mac_ie.h"
#include "dmac_scan.h"
#include "dmac_main.h"
#include "dmac_fcs.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_ext_if.h"
#include "dmac_device.h"
#include "dmac_mgmt_sta.h"
#include "dmac_alg.h"
#if defined(_PRE_WLAN_CHIP_TEST) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "dmac_scan_test.h"
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_sta_pm.h"
#include "pm_extern.h"
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "dmac_dft.h"
#endif
#include "dmac_config.h"
#include "dmac_chan_mgmt.h"
#include "dmac_mgmt_classifier.h"
#ifdef _PRE_WLAN_FEATURE_11K
#include "dmac_11k.h"
#endif
#ifdef _PRE_WLAN_FEATURE_GREEN_AP
#include "dmac_green_ap.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#include "dmac_bsd.h"
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif

#include "dmac_power.h"
#include "dmac_beacon.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_csa_sta.h"

#ifdef _PRE_WLAN_FEATURE_FTM
#include "dmac_ftm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
#include "ipc_manage.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_SCAN_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
oal_uint8  g_uc_wifi_support_gscan = OAL_FALSE;   //WIFI是否支持gscan，后续删除
#endif
/* 静态函数声明 */

OAL_STATIC oal_uint32 dmac_scan_send_bcast_probe(mac_device_stru *pst_mac_device, oal_uint8 uc_band, oal_uint8  uc_index);
OAL_STATIC oal_uint32  dmac_scan_report_channel_statistics_result(hal_to_dmac_device_stru  *pst_hal_device, oal_uint8 uc_scan_idx);
OAL_STATIC oal_uint32  dmac_scan_switch_home_channel_work_timeout(void *p_arg);

OAL_STATIC oal_uint32  dmac_scan_start_pno_sched_scan_timer(void *p_arg);
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
OAL_STATIC oal_uint32  dmac_scan_check_2g_scan_results(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device, mac_vap_stru *pst_vap, wlan_channel_band_enum_uint8 en_next_band);
#endif
#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
OAL_STATIC oal_void dmac_scan_proc_scanned_bss(mac_device_stru *pst_mac_device, oal_netbuf_stru *pst_netbuf);
OAL_STATIC oal_void dmac_scan_dump_bss_list(oal_dlist_head_stru *pst_head);
OAL_STATIC oal_void dmac_scan_check_ap_bss_info(oal_dlist_head_stru *pst_head);
#endif
/*****************************************************************************
  3 函数实现
*****************************************************************************/
#if 0
OAL_STATIC oal_void dmac_scan_print_time_stamp()
{
    oal_uint32                  ul_timestamp;

    ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_print_time_stamp:: time_stamp:%d.}", ul_timestamp);

    return;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST

oal_void dmac_detect_2040_te_a_b(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_frame_body, oal_uint16 us_frame_len, oal_uint16 us_offset,oal_uint8 uc_curr_chan)
{
    oal_uint8            chan_index     = 0;
    oal_bool_enum_uint8  ht_cap         = OAL_FALSE;
    oal_uint8            uc_scan_chan_idx;
    oal_uint8            uc_real_chan   = uc_curr_chan;
    mac_device_stru     *pst_mac_device = OAL_PTR_NULL;
    oal_uint8           *puc_ie         = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_detect_2040_te_a_b::mac_res_get_dev return null.}");
        return;
    }

    uc_scan_chan_idx = GET_HAL_DEV_CURRENT_SCAN_IDX(pst_dmac_vap->pst_hal_device);

    if (us_frame_len <= us_offset)
    {
        mac_get_channel_idx_from_num((pst_mac_device->st_scan_params.ast_channel_list[uc_scan_chan_idx]).en_band,
                                      uc_real_chan, &chan_index);
        /* Detect Trigger Event - A */
        pst_dmac_vap->st_vap_base_info.st_ch_switch_info.ul_chan_report_for_te_a |= (1U << chan_index);

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_detect_2040_te_a_b::framebody_len[%d]}", us_frame_len);
        return;
    }

    us_frame_len   -= us_offset;
    puc_frame_body += us_offset;

    puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_frame_body, us_frame_len);
    if (OAL_PTR_NULL != puc_ie)
    {
        ht_cap = OAL_TRUE;

        /* Check for the Forty MHz Intolerant bit in HT-Capabilities */
        if((puc_ie[3] & BIT6) != 0)
        {
            //OAM_INFO_LOG0(0, OAM_SF_SCAN, "dmac_detect_2040_te_a_b::40 intolerant in ht cap");
            /* Register Trigger Event - B */
            pst_dmac_vap->st_vap_base_info.st_ch_switch_info.en_te_b = OAL_TRUE;
        }
    }

    puc_ie = mac_find_ie(MAC_EID_2040_COEXT, puc_frame_body, us_frame_len);
    if (OAL_PTR_NULL != puc_ie)
    {
        /* Check for the Forty MHz Intolerant bit in Coex-Mgmt IE */
        if((puc_ie[2] & BIT1) != 0)
        {
            //OAM_INFO_LOG0(0, OAM_SF_SCAN, "dmac_detect_2040_te_a_b::40 intolerant in co");
            /* Register Trigger Event - B */
            pst_dmac_vap->st_vap_base_info.st_ch_switch_info.en_te_b = OAL_TRUE;
        }

    }

    /* 只有在HT能力为False时才需要获取信道信息，其他情况下不需要查找 */
    if(OAL_FALSE == ht_cap)
    {
        puc_ie = mac_find_ie(MAC_EID_DSPARMS, puc_frame_body, us_frame_len);
        if (OAL_PTR_NULL != puc_ie)
        {
            uc_real_chan = puc_ie[2];
        }

        mac_get_channel_idx_from_num((pst_mac_device->st_scan_params.ast_channel_list[uc_scan_chan_idx]).en_band,
                            uc_real_chan, &chan_index);
        pst_dmac_vap->st_vap_base_info.st_ch_switch_info.ul_chan_report_for_te_a |= (1U << chan_index);
    }

    return;
}

#endif

oal_void  dmac_scan_proc_obss_scan_complete_event(dmac_vap_stru *pst_dmac_vap)
{
#if 0
    OAM_INFO_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                 "{dmac_scan_proc_obss_scan_complete_event::te_a:%d,te_b:%d}",
                 pst_dmac_vap->st_vap_base_info.st_ch_switch_info.ul_chan_report_for_te_a,
                 pst_dmac_vap->st_vap_base_info.st_ch_switch_info.en_te_b);
#endif

    if(!pst_dmac_vap->st_vap_base_info.st_ch_switch_info.ul_chan_report_for_te_a
       && (OAL_FALSE == pst_dmac_vap->st_vap_base_info.st_ch_switch_info.en_te_b))
    {
        return;
    }

    dmac_send_2040_coext_mgmt_frame_sta(&(pst_dmac_vap->st_vap_base_info));

    return;
}
#endif


OAL_STATIC oal_void  dmac_scan_set_vap_mac_addr_by_scan_state(mac_device_stru  *pst_mac_device,
                                                                           hal_to_dmac_device_stru *pst_hal_device,
                                                                           oal_bool_enum_uint8 en_is_scan_start)
{
    oal_uint8                      uc_orig_hal_dev_id;
    dmac_vap_stru                 *pst_dmac_vap;
    mac_vap_stru                  *pst_mac_vap;
    mac_scan_req_stru             *pst_scan_params;
    hal_scan_params_stru          *pst_hal_scan_params;

    /* 获取扫描参数 */
    pst_scan_params = &(pst_mac_device->st_scan_params);

    /* 非随机mac addr扫描，直接返回，无需重设帧过滤寄存器 */
    if (OAL_TRUE != pst_scan_params->en_is_random_mac_addr_scan)
    {
        //OAM_INFO_LOG0(0, OAM_SF_SCAN, "{dmac_scan_set_vap_mac_addr_by_scan_state:: don't need modified mac addr.}");
        return;
    }

    /* p2p扫描不支持随机mac addr */
    if (OAL_TRUE == pst_scan_params->bit_is_p2p0_scan)
    {
        //OAM_INFO_LOG0(0, OAM_SF_SCAN, "{dmac_scan_set_vap_mac_addr_by_scan_state:: p2p scan, don't need modified mac addr.}");
        return;
    }

    /* 获取dmac vap */
    pst_dmac_vap = mac_res_get_dmac_vap(pst_scan_params->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_set_vap_mac_addr_by_scan_state:: pst_dmac_vap is null.}");
        return;
    }

    pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);

    /* 判断当前非P2P场景，进行随机MAC ADDR的设置 */
    if (!IS_LEGACY_VAP(pst_mac_vap))
    {
        return;
    }

    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_set_vap_mac_addr_by_scan_state:: pst_hal_device is null.}");
        return;
    }

    pst_hal_scan_params = &(pst_hal_device->st_hal_scan_params);

    uc_orig_hal_dev_id = dmac_vap_get_hal_device_id(pst_dmac_vap);

    dmac_vap_set_hal_device_id(pst_dmac_vap, pst_hal_device->uc_device_id);

    /* 扫描开始时，重新设置帧过滤寄存器 */
    if (OAL_TRUE == en_is_scan_start)
    {
        /* 保存原先的mac addr */
        oal_set_mac_addr(pst_hal_scan_params->auc_original_mac_addr, mac_mib_get_StationID(pst_mac_vap));

        /* 设置mib和硬件的macaddr为随机mac addr */
        oal_set_mac_addr(mac_mib_get_StationID(pst_mac_vap),
                         pst_scan_params->auc_sour_mac_addr);
        hal_vap_set_macaddr(pst_dmac_vap->pst_hal_vap, pst_scan_params->auc_sour_mac_addr);

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
        OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_set_vap_mac_addr_by_scan_state::original mac addr[%02X:XX:XX:%02X:%02X:%02X], set random mac addr[%02X:XX:XX:%02X:%02X:%02X]}", 8,
                                 pst_hal_scan_params->auc_original_mac_addr[0], pst_hal_scan_params->auc_original_mac_addr[3],
                                 pst_hal_scan_params->auc_original_mac_addr[4], pst_hal_scan_params->auc_original_mac_addr[5],
                                 pst_scan_params->auc_sour_mac_addr[0], pst_scan_params->auc_sour_mac_addr[3],
                                 pst_scan_params->auc_sour_mac_addr[4], pst_scan_params->auc_sour_mac_addr[5]);
#endif
    }
    else
    {
        /* 扫描结束，恢复原先mib和硬件寄存器的mac addr */
        oal_set_mac_addr(mac_mib_get_StationID(pst_mac_vap),
                         pst_hal_scan_params->auc_original_mac_addr);
        hal_vap_set_macaddr(pst_dmac_vap->pst_hal_vap, pst_hal_scan_params->auc_original_mac_addr);

        OAM_WARNING_LOG4(0, OAM_SF_SCAN, "{dmac_scan_set_vap_mac_addr_by_scan_state:: resume original mac addr, mac_addr:%02X:XX:XX:%02X:%02X:%02X.}",
                         pst_hal_scan_params->auc_original_mac_addr[0], pst_hal_scan_params->auc_original_mac_addr[3], pst_hal_scan_params->auc_original_mac_addr[4], pst_hal_scan_params->auc_original_mac_addr[5]);

    }

    dmac_vap_set_hal_device_id(pst_dmac_vap, uc_orig_hal_dev_id);

    return;
}

#ifdef _PRE_WLAN_WEB_CMD_COMM

OAL_STATIC oal_uint32  dmac_scan_get_bss_max_rate(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf, oal_uint16 us_frame_len,
                                                                oal_uint32 *pul_max_rate_kbps, oal_uint8 *puc_max_nss, hal_channel_assemble_enum_uint8* pen_bw)
{
    hal_to_dmac_device_stru    *pst_hal_device;
    oal_uint8                  *puc_frame_body;
    oal_uint8                  *puc_ie = OAL_PTR_NULL;
    oal_uint8                  *puc_legacy_rate;
    oal_uint8                   uc_legacy_rate_num;
    oal_uint16                  us_offset;
    oal_uint8                   uc_legacy_max_rate = 0;
    wlan_nss_enum_uint8         en_nss;
    oal_uint8                  *puc_ht_mcs_bitmask;
    oal_uint8                   uc_ht_max_mcs;
    oal_uint8                   uc_mcs_bitmask;
    oal_uint16                  us_ht_cap_info;
    oal_bool_enum_uint8         en_ht_short_gi = 0;
    hal_statistic_stru          st_per_rate;
    oal_uint32                  ul_ht_max_rate_kbps;
    oal_uint16                  us_msg_idx = 0;
    oal_uint16                  us_vht_cap_filed_low;
    oal_uint16                  us_vht_cap_filed_high;
    oal_uint32                  ul_vht_cap_field;
    oal_bool_enum_uint8         en_vht_short_gi = 0;
    oal_uint16                  us_vht_mcs_map;
    oal_uint8                   uc_vht_max_mcs;
    oal_uint32                  ul_vht_max_rate_kbps;
    hal_channel_assemble_enum_uint8 en_ht_bw  = WLAN_BAND_ASSEMBLE_20M;
    hal_channel_assemble_enum_uint8 en_vht_bw = WLAN_BAND_ASSEMBLE_80M;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(&pst_dmac_vap->st_vap_base_info);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_get_bss_max_rate:: current vap, DMAC_VAP_GET_HAL_DEVICE null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取帧体起始指针 */
    puc_frame_body = (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf) + MAC_80211_FRAME_LEN;
    /* 设置Beacon帧的field偏移量 */
    us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    *pul_max_rate_kbps  = 0;
    *puc_max_nss        = 1;

    /* 获取legacy最大速率 */
    puc_ie = mac_find_ie(MAC_EID_RATES, puc_frame_body + us_offset, us_frame_len - us_offset);
    if (OAL_PTR_NULL != puc_ie)
    {
        puc_legacy_rate     = puc_ie + MAC_IE_HDR_LEN;
        uc_legacy_rate_num  = puc_ie[1];
        uc_legacy_max_rate  = puc_legacy_rate[uc_legacy_rate_num - 1] & 0x7f;
    }

    /* 获取扩展legacy最大速率 */
    puc_ie = mac_find_ie(MAC_EID_XRATES, puc_frame_body + us_offset, us_frame_len - us_offset);
    if (OAL_PTR_NULL != puc_ie)
    {
        puc_legacy_rate     = puc_ie + MAC_IE_HDR_LEN;
        uc_legacy_rate_num  = puc_ie[1];
        uc_legacy_max_rate  = OAL_MAX(uc_legacy_max_rate, puc_legacy_rate[uc_legacy_rate_num - 1] & 0x7f);
    }

    /* 更新BSS最大速率 */
    *pul_max_rate_kbps  = ((oal_uint32)uc_legacy_max_rate) * 500;

    /* 计算HT最大速率，并更新BSS最大速率 */
    puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_frame_body + us_offset, us_frame_len - us_offset);
    if (OAL_PTR_NULL != puc_ie)
    {
        /* 获取HT BW、short GI信息 */
        us_msg_idx      = MAC_IE_HDR_LEN;
        us_ht_cap_info  = OAL_MAKE_WORD16(puc_ie[us_msg_idx], puc_ie[us_msg_idx + 1]);
        /* 获取HT NSS、MCS信息 */
        us_msg_idx        += MAC_HT_CAPINFO_LEN + MAC_HT_AMPDU_PARAMS_LEN;
        puc_ht_mcs_bitmask = &puc_ie[us_msg_idx];
        for (en_nss = WLAN_SINGLE_NSS; en_nss <= WLAN_FOUR_NSS; en_nss++)
        {
            if (0 == puc_ht_mcs_bitmask[en_nss])
            {
                break;
            }
        }
        if (WLAN_SINGLE_NSS != en_nss)
        {
            en_nss--;

            uc_ht_max_mcs = 0;
            uc_mcs_bitmask = puc_ht_mcs_bitmask[en_nss] >> 1;
            while (uc_mcs_bitmask > 0)
            {
                uc_ht_max_mcs++;
                uc_mcs_bitmask >>= 1;
            }

            puc_ie = mac_find_ie(MAC_EID_HT_OPERATION, puc_frame_body + us_offset, us_frame_len - us_offset);
            if (OAL_PTR_NULL != puc_ie)
            {
                us_msg_idx      = MAC_IE_HDR_LEN;
                en_ht_bw = ((puc_ie[us_msg_idx + 1] & BIT2) ? WLAN_BAND_ASSEMBLE_40M : WLAN_BAND_ASSEMBLE_20M);
                en_ht_short_gi  = (WLAN_BAND_ASSEMBLE_20M == en_ht_bw) ?
                                  ((us_ht_cap_info & BIT5) >> 5) : ((us_ht_cap_info & BIT6) >> 6);
    //            OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_get_bss_max_rate::us_ht_cap_info: 0x%04x, shortgi: %d, nss: %d, puc_ht_mcs_bitmask: 0x%x}",
    //                us_ht_cap_info, en_ht_short_gi, en_nss, puc_ht_mcs_bitmask[en_nss]);
    //
    //            OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_get_bss_max_rate::channel: %d, ht_info: 0x%01x, en_ht_bw: %d, uc_ht_max_mcs: %d}",
    //                        puc_ie[us_msg_idx], puc_ie[us_msg_idx + 1], en_ht_bw, uc_ht_max_mcs);
            }

            /* 查询HT最大速率 */
            st_per_rate.un_nss_rate.st_ht_rate.bit_protocol_mode    = WLAN_HT_PHY_PROTOCOL_MODE;
            st_per_rate.un_nss_rate.st_ht_rate.bit_ht_mcs           = uc_ht_max_mcs;
            st_per_rate.uc_bandwidth    = en_ht_bw;
            st_per_rate.uc_short_gi     = en_ht_short_gi;
            dmac_alg_get_rate_kbps(pst_hal_device, &st_per_rate, &ul_ht_max_rate_kbps);

            *puc_max_nss        = OAL_MAX(*puc_max_nss, en_nss + 1);
            *pul_max_rate_kbps  = OAL_MAX(*pul_max_rate_kbps, ul_ht_max_rate_kbps * (en_nss + 1));
        }
    }
    *pen_bw = en_ht_bw;

    /* 计算VHT最大速率，并更新BSS最大速率 */
    puc_ie = mac_find_ie(MAC_EID_VHT_CAP, puc_frame_body + us_offset, us_frame_len - us_offset);
    if (OAL_PTR_NULL != puc_ie)
    {
        /* 解析VHT capablities info field */
        us_msg_idx              = MAC_IE_HDR_LEN;
        us_vht_cap_filed_low    = OAL_MAKE_WORD16(puc_ie[us_msg_idx], puc_ie[us_msg_idx + 1]);
        us_vht_cap_filed_high   = OAL_MAKE_WORD16(puc_ie[us_msg_idx + 2], puc_ie[us_msg_idx + 3]);
        ul_vht_cap_field        = OAL_MAKE_WORD32(us_vht_cap_filed_low, us_vht_cap_filed_high);

        /* 获取VHT NSS、MCS信息 */
        us_msg_idx    += MAC_VHT_CAP_INFO_FIELD_LEN;
        us_vht_mcs_map = OAL_MAKE_WORD16(puc_ie[us_msg_idx], puc_ie[us_msg_idx + 1]);
        uc_vht_max_mcs = 0;

        for (en_nss = WLAN_SINGLE_NSS; en_nss <= WLAN_FOUR_NSS; en_nss++)
        {
            if (WLAN_INVALD_VHT_MCS == WLAN_GET_VHT_MAX_SUPPORT_MCS(us_vht_mcs_map & 0x3))
            {
                break;
            }
            uc_vht_max_mcs = WLAN_GET_VHT_MAX_SUPPORT_MCS(us_vht_mcs_map & 0x3);
            us_vht_mcs_map >>= 2;
        }
        /* 获取VHT BW、short GI信息 */
        puc_ie = mac_find_ie(MAC_EID_VHT_OPERN, puc_frame_body + us_offset, us_frame_len - us_offset);
        if (OAL_PTR_NULL != puc_ie)
        {
            us_msg_idx      = MAC_IE_HDR_LEN;
            en_vht_bw = (puc_ie[us_msg_idx] ? WLAN_BAND_ASSEMBLE_80M : en_ht_bw);
            if (0 != (ul_vht_cap_field & (BIT3 |BIT2)))
            {
                en_vht_bw       = WLAN_BAND_ASSEMBLE_160M;
                en_vht_short_gi =((ul_vht_cap_field & BIT6) >> 6);
            }
            else if(WLAN_BAND_ASSEMBLE_80M == en_vht_bw)
            {
                en_vht_short_gi = ((ul_vht_cap_field & BIT5) >> 5);
            }
            else
            {
                en_vht_short_gi = en_ht_short_gi;
            }
            //OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_get_bss_max_rate::us_vht_cap_info: 0x%x, vht shortgi: %d, vht nss: %d, puc_vht_mcs_bitmask: 0x%x}",
            //    ul_vht_cap_field, en_vht_short_gi, en_nss, us_vht_mcs_map);
            //OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_get_bss_max_rate::vht_info: 0x%01x, en_vht_bw: %d, uc_vht_max_mcs: %d}",
            //            *(oal_uint32*)puc_ie, en_vht_bw, uc_vht_max_mcs);
        }

        /* 查询VHT最大速率 */
        st_per_rate.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode    = WLAN_VHT_PHY_PROTOCOL_MODE;
        st_per_rate.un_nss_rate.st_vht_nss_mcs.bit_nss_mode         = WLAN_SINGLE_NSS;
        st_per_rate.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs          = uc_vht_max_mcs;
        st_per_rate.uc_short_gi     = en_vht_short_gi;
        st_per_rate.uc_bandwidth    = en_vht_bw;
        dmac_alg_get_rate_kbps(pst_hal_device, &st_per_rate, &ul_vht_max_rate_kbps);

        *puc_max_nss        = OAL_MAX(*puc_max_nss, en_nss);
        *pul_max_rate_kbps  = OAL_MAX(*pul_max_rate_kbps, ul_vht_max_rate_kbps * en_nss);
        *pen_bw             = en_vht_bw;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  dmac_scan_report_scanned_bss(dmac_vap_stru *pst_dmac_vap, oal_void *p_param)
{
    frw_event_mem_stru                    *pst_event_mem;
    frw_event_stru                        *pst_event;
    mac_device_stru                       *pst_mac_device;
    dmac_tx_event_stru                    *pst_dtx_event;
    oal_netbuf_stru                       *pst_netbuf;
    dmac_rx_ctl_stru                      *pst_rx_ctrl;
#ifdef _PRE_WLAN_FEATURE_M2S
    mac_ieee80211_frame_stru              *pst_frame_hdr;
    oal_uint8                             *puc_frame_body;
#endif
    oal_uint8                             *puc_frame_body_tail;            /* 指向帧体的尾部 */
    mac_scan_req_stru                     *pst_scan_params;
    mac_scanned_result_extend_info_stru   *pst_scanned_result_extend_info;
    oal_uint16                             us_frame_len;
    oal_uint16                             us_remain_netbuf_len;
#ifdef _PRE_WLAN_WEB_CMD_COMM
    oal_uint32                             ul_ret;
#endif

    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_report_scanned_bss::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_netbuf = (oal_netbuf_stru *)p_param;

    /* 获取帧信息 */
    us_frame_len = (oal_uint16)oal_netbuf_get_len(pst_netbuf);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    us_remain_netbuf_len = oal_netbuf_get_payload_len(pst_netbuf) - (us_frame_len - MAC_80211_FRAME_LEN);
#else
    us_remain_netbuf_len = (oal_uint16)oal_netbuf_tailroom(pst_netbuf);
#endif

    if(us_remain_netbuf_len < OAL_SIZEOF(mac_scanned_result_extend_info_stru))
    {
        OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                     "{dmac_scan_report_scanned_bss::scan netbuf tailroom not enough,requet[%u],actual[%u] }",
                     us_remain_netbuf_len,
                     OAL_SIZEOF(mac_scanned_result_extend_info_stru));
        return OAL_FAIL;
    }

    /* 获取该buffer的控制信息 */
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 获取扫描参数 */
    pst_scan_params = &(pst_mac_device->st_scan_params);

    /* 每次扫描结束，上报扫描结果，抛事件到HMAC, 申请事件内存 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{dmac_scan_report_scanned_bss::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CRX,
                       DMAC_WLAN_CRX_EVENT_SUB_TYPE_EVERY_SCAN_RESULT,
                       OAL_SIZEOF(dmac_tx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_dmac_vap->st_vap_base_info.uc_chip_id,
                       pst_dmac_vap->st_vap_base_info.uc_device_id,
                       pst_dmac_vap->st_vap_base_info.uc_vap_id);

    /***********************************************************************************************/
    /*            netbuf data域的上报的扫描结果的字段的分布                                        */
    /* ------------------------------------------------------------------------------------------  */
    /* beacon/probe rsp body  |     帧体后面附加字段(mac_scanned_result_extend_info_stru)          */
    /* -----------------------------------------------------------------------------------------   */
    /* 收到的beacon/rsp的body | rssi(4字节) | channel num(1字节)| band(1字节)|bss_tye(1字节)|填充  */
    /* ------------------------------------------------------------------------------------------  */
    /*                                                                                             */
    /***********************************************************************************************/

    /* 将扩展信息拷贝到帧体的后面，上报到host侧 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    puc_frame_body_tail = (oal_uint8 *)MAC_GET_RX_PAYLOAD_ADDR(&(pst_rx_ctrl->st_rx_info), pst_netbuf) + us_frame_len - MAC_80211_FRAME_LEN;
#else
    puc_frame_body_tail = (oal_uint8 *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctrl->st_rx_info)) + us_frame_len;
#endif

    /* 指向帧体的尾部，对netbuf进行扩展，携带其它需要上报的信息 */
    pst_scanned_result_extend_info = (mac_scanned_result_extend_info_stru *)puc_frame_body_tail;

    /* 清空上报扫描结果的扩展字段信息并进行赋值 */
    OAL_MEMZERO(pst_scanned_result_extend_info, OAL_SIZEOF(mac_scanned_result_extend_info_stru));
    pst_scanned_result_extend_info->l_rssi = (oal_int32)pst_rx_ctrl->st_rx_statistic.c_rssi_dbm;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    pst_scanned_result_extend_info->c_ant0_rssi = pst_rx_ctrl->st_rx_statistic.c_ant0_rssi;
    pst_scanned_result_extend_info->c_ant1_rssi = pst_rx_ctrl->st_rx_statistic.c_ant1_rssi;
#endif
    pst_scanned_result_extend_info->en_bss_type = pst_scan_params->en_bss_type;
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    /*获取TSF TIMER时间戳*/
    hal_vap_tsf_get_32bit(pst_dmac_vap->pst_hal_vap, (oal_uint32 *)&(pst_scanned_result_extend_info->ul_parent_tsf));
    pst_scanned_result_extend_info->c_snr_ant0 = (oal_int32)pst_rx_ctrl->st_rx_statistic.c_snr_ant0;
    pst_scanned_result_extend_info->c_snr_ant1 = (oal_int32)pst_rx_ctrl->st_rx_statistic.c_snr_ant1;
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    /* 获取帧信息 */
    pst_frame_hdr  = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctrl->st_rx_info));
    puc_frame_body = MAC_GET_RX_PAYLOAD_ADDR(&(pst_rx_ctrl->st_rx_info), pst_netbuf);

    pst_scanned_result_extend_info->en_support_max_nss = dmac_m2s_get_bss_max_nss(&pst_dmac_vap->st_vap_base_info, pst_netbuf, us_frame_len, OAL_FALSE);
    pst_scanned_result_extend_info->uc_num_sounding_dim = dmac_m2s_scan_get_num_sounding_dim(pst_netbuf, us_frame_len);

    /* 只有probe rsp帧中宣称不支持OPMODE，对端才不支持OPMODE，beacon和assoc rsp帧中宣称的OPMODE信息不可信,因此当我们解析probe rsp时我们发的probe req帧中最好要带ext cap字段 */
    if(WLAN_PROBE_RSP == pst_frame_hdr->st_frame_control.bit_sub_type)
    {
        pst_scanned_result_extend_info->en_support_opmode = dmac_m2s_get_bss_support_opmode(&pst_dmac_vap->st_vap_base_info, puc_frame_body, us_frame_len);
    }
    else
    {
        pst_scanned_result_extend_info->en_support_opmode = OAL_FALSE;
    }
#endif

#ifdef _PRE_WLAN_WEB_CMD_COMM
    ul_ret = dmac_scan_get_bss_max_rate(pst_dmac_vap, pst_netbuf, us_frame_len,
                &pst_scanned_result_extend_info->ul_max_rate_kbps,
                &pst_scanned_result_extend_info->uc_max_nss,
                &pst_scanned_result_extend_info->en_bw);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
               "{dmac_scan_report_scanned_bss:: dmac_scan_get_bss_max_rate fail, ul_ret = %u}", ul_ret);
        return ul_ret;
    }
#endif

    /* 修正帧的长度为加上扩展字段的长度 */
    us_frame_len += OAL_SIZEOF(mac_scanned_result_extend_info_stru);
    oal_netbuf_put(pst_netbuf, OAL_SIZEOF(mac_scanned_result_extend_info_stru));

    /* 业务事件信息 */
    pst_dtx_event               = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_dtx_event->pst_netbuf   = pst_netbuf;
    pst_dtx_event->us_frame_len = us_frame_len;

#if 0
    /* 加入维测信息，当前信道号、信号强度、上报的netbuf长度 */
    OAM_ERROR_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                   "{dmac_scan_report_scanned_bss::rssi[%d], cb_rssi[%d], channel_num[%d], buf_len[%d].}",
                   pst_scanned_result_extend_info->l_rssi,
                   pst_rx_ctrl->st_rx_statistic.c_rssi_dbm,
                   pst_rx_ctrl->st_rx_info.uc_channel_number,
                   us_frame_len);
#endif

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_scan_check_bss_in_pno_scan(oal_uint8         *puc_frame_body,
                                                                  oal_int32          l_frame_body_len,
                                                                  mac_pno_scan_stru *pst_pno_scan_info,
                                                                  oal_int32          l_rssi)
{
    oal_uint8       *puc_ssid;
    oal_int32        l_loop;
    oal_int8         ac_ssid[WLAN_SSID_MAX_LEN];
    oal_uint8        uc_ssid_len = 0;

    /* 如果该帧的信号小于pno扫描可上报的阈值，则返回失败，此帧不上报 */
    if (l_rssi < pst_pno_scan_info->l_rssi_thold)
    {
        return OAL_FAIL;
    }

    OAL_MEMZERO(ac_ssid, OAL_SIZEOF(ac_ssid));

    /* 获取管理帧中的ssid IE信息 */
    puc_ssid = mac_get_ssid(puc_frame_body, l_frame_body_len, &uc_ssid_len);
    if ((OAL_PTR_NULL != puc_ssid) && (0 != uc_ssid_len))
    {
        oal_memcopy(ac_ssid, puc_ssid, uc_ssid_len);
        ac_ssid[uc_ssid_len] = '\0';
    }

    /* 从pno参数中查找本ssid是否存在，如果存在，则可以上报 */
    for (l_loop = 0; l_loop < pst_pno_scan_info->l_ssid_count; l_loop++)
    {
        /* 如果ssid相同，返回成功 */
        if (0 == oal_memcmp(ac_ssid, pst_pno_scan_info->ast_match_ssid_set[l_loop].auc_ssid, (uc_ssid_len + 1)))
        {
            OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_scan_check_bss_in_pno_scan::ssid match success.}");
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}


OAL_STATIC oal_uint32  dmac_scan_check_bss_type(oal_uint8 *puc_frame_body, mac_scan_req_stru *pst_scan_params)
{
    mac_cap_info_stru         *pst_cap_info;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* ----------------------------------------------------------------------*/
    /* |Timestamp|BcnInt|CapInfo|SSID|SupRates|DSParamSet|TIM  |CountryElem |*/
    /* ----------------------------------------------------------------------*/
    /* |8        |2     |2      |2-34|3-10    |3         |6-256|8-256       |*/
    /* ----------------------------------------------------------------------*/
    pst_cap_info = (mac_cap_info_stru *)(puc_frame_body + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN);

    if ((WLAN_MIB_DESIRED_BSSTYPE_INFRA == pst_scan_params->en_bss_type) &&
        (1 != pst_cap_info->bit_ess))
    {
        //OAM_INFO_LOG0(0, OAM_SF_SCAN, "{dmac_scan_check_bss_type::expect infra bss, but it's not infra bss.}\r\n");
        return OAL_FAIL;
    }

    if ((WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT == pst_scan_params->en_bss_type) &&
        (1 != pst_cap_info->bit_ibss))
    {
        //OAM_INFO_LOG0(0, OAM_SF_SCAN, "{dmac_scan_check_bss_type::expect ibss, but it's not ibss.}\r\n");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


oal_void  dmac_scan_check_assoc_ap_channel(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf,scan_check_assoc_channel_enum_uint8 en_check_mode)
{
    dmac_rx_ctl_stru                        *pst_rx_ctrl;
    mac_ieee80211_frame_stru                *pst_frame_hdr;
    oal_uint8                               *puc_frame_body;
    oal_uint16                               us_frame_len;
    oal_uint16                               us_offset =  MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    mac_cfg_ssid_param_stru                  st_mib_ssid = {0};
    oal_uint8                                uc_mib_ssid_len = 0;
    oal_uint8                                uc_frame_channel;
    oal_uint8                                uc_ssid_len = 0;
    oal_uint8                               *puc_ssid;
    oal_uint32                               ul_ret;
    oal_uint8                                uc_idx;
    oal_ieee80211_channel_sw_ie              st_csa_info;
    mac_vap_stru                            *pst_mac_vap;

    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 获取帧信息 */
    pst_frame_hdr  = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctrl->st_rx_info));
    puc_frame_body = MAC_GET_RX_PAYLOAD_ADDR(&(pst_rx_ctrl->st_rx_info), pst_netbuf);
    us_frame_len   = (oal_uint16)oal_netbuf_get_len(pst_netbuf);
    pst_mac_vap    = &(pst_dmac_vap->st_vap_base_info);

    /*非发往本机的管理帧*/
    if(OAL_MEMCMP(pst_frame_hdr->auc_address3, pst_dmac_vap->st_vap_base_info.auc_bssid, WLAN_MAC_ADDR_LEN))
    {
        return;
    }

    /*ssid信息不一致*/
    puc_ssid = mac_get_ssid(puc_frame_body, (oal_int32)(us_frame_len - MAC_80211_FRAME_LEN), &uc_ssid_len);
    mac_mib_get_ssid(&pst_dmac_vap->st_vap_base_info, &uc_mib_ssid_len, (oal_uint8 *)&st_mib_ssid);
    if((OAL_PTR_NULL == puc_ssid) || (0 == uc_ssid_len) || (st_mib_ssid.uc_ssid_len != uc_ssid_len) ||
        OAL_MEMCMP(st_mib_ssid.ac_ssid, puc_ssid, uc_ssid_len))
    {
        return;
    }

    /*管理帧中获取信道信息*/
    uc_frame_channel = mac_ie_get_chan_num(puc_frame_body, (us_frame_len - MAC_80211_FRAME_LEN),
                                       us_offset, pst_rx_ctrl->st_rx_info.uc_channel_number);
    if(0 == uc_frame_channel)
    {
        return;
    }

    ul_ret = mac_get_channel_idx_from_num(pst_dmac_vap->st_vap_base_info.st_channel.en_band, uc_frame_channel, &uc_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_check_assoc_ap_channel::Get channel idx failed, To DISASSOC! vap_channel[%d], frame_channel=[%d].}",
        pst_mac_vap->st_channel.uc_chan_number, uc_frame_channel);
        dmac_vap_linkloss_clean(pst_dmac_vap);
        dmac_sta_csa_fsm_post_event(pst_mac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OAL_PTR_NULL);
        dmac_send_disasoc_misc_event(pst_mac_vap, pst_mac_vap->us_assoc_vap_id, DMAC_DISASOC_MISC_GET_CHANNEL_IDX_FAIL);
        return;
    }

    switch (en_check_mode)
    {
        /*LinkLoss 检查*/
        case SCAN_CHECK_ASSOC_CHANNEL_LINKLOSS:
            if(pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number != uc_frame_channel)
            {/*信道发生变化*/
                /*CSA 信道切换过程中直接退出*/
                if(OAL_TRUE == dmac_sta_csa_is_in_waiting(pst_mac_vap))
                {
                    dmac_vap_linkloss_clean(pst_dmac_vap);
                    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_check_assoc_ap_channel::vap_channel=[%d],fram_channel=[%d];sta in csa process,return.}",
                    pst_mac_vap->st_channel.uc_chan_number,uc_frame_channel);
                    return;
                }

                /*触发CSA流程  */
                st_csa_info.new_ch_num                          = uc_frame_channel;
                st_csa_info.mode                                = WLAN_CSA_MODE_TX_DISABLE;
                st_csa_info.count                               = 0;
                /*带宽先启动在20M, Beacon帧返回真正带宽*/
                pst_mac_vap->st_ch_switch_info.en_new_bandwidth = WLAN_BAND_WIDTH_20M;
                OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_check_assoc_ap_channel::Trigger csa, change channle from [%d] to [%d], bw=[%d]}",
                    pst_mac_vap->st_channel.uc_chan_number, uc_frame_channel, pst_mac_vap->st_ch_switch_info.en_new_bandwidth);
                dmac_sta_csa_fsm_post_event(pst_mac_vap, WLAN_STA_CSA_EVENT_GET_IE, sizeof(st_csa_info), (oal_uint8*)&st_csa_info);
                /* 重新设置信道，vap下的带宽切到normal状态 */
                if ((IS_LEGACY_STA(pst_mac_vap)) && (OAL_TRUE == dmac_sta_csa_is_in_waiting(pst_mac_vap))
                      && (DMAC_STA_BW_SWITCH_FSM_NORMAL != MAC_VAP_GET_CURREN_BW_STATE(pst_mac_vap)))
                {
                    dmac_sta_bw_switch_fsm_post_event((dmac_vap_stru *)pst_mac_vap, DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC, 0, OAL_PTR_NULL);
                    /* 带宽切换状态机 */
                    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_ch_switch_ie::VAP CURREN BW STATE[%d].}",
                                     MAC_VAP_GET_CURREN_BW_STATE(pst_mac_vap));
                }
            }
            break;
        case SCAN_CHECK_ASSOC_CHANNEL_CSA:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CSA, "{dmac_scan_check_assoc_ap_channel::csa scan recv_probe_rsp_channel = [%d].}", uc_frame_channel);
            dmac_sta_csa_fsm_post_event(&(pst_dmac_vap->st_vap_base_info), WLAN_STA_CSA_EVENT_RCV_PROBE_RSP, sizeof(uc_frame_channel), &uc_frame_channel);
            break;
        default:
            break;
    }

    return;
}



hal_to_dmac_device_stru *dmac_scan_find_hal_device(mac_device_stru  *pst_mac_device, dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_idx)
{
    hal_to_dmac_device_stru     *pst_hal_device = OAL_PTR_NULL;
    dmac_device_stru            *pst_dmac_device;

    /* 优先返回dmac_vap所挂接的hal device */
    if (0 == uc_idx)
    {
        return pst_dmac_vap->pst_hal_device;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "dmac_scan_find_hal_device::pst_dmac_device is null");
        return pst_hal_device;

    }
    /* 是否支持并发 */
    if (OAL_TRUE == pst_dmac_device->en_is_fast_scan)
    {
        pst_hal_device = dmac_device_get_another_h2d_dev(pst_dmac_device, pst_dmac_vap->pst_hal_device);

        /* 支持并发必定存在两路hal device */
        if (OAL_PTR_NULL  == pst_hal_device)
        {
            OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_DBAC, "dmac_scan_find_hal_device::pst_hal_device NULL,chip id[%d],ori hal device id[%d]",
                            pst_dmac_vap->st_vap_base_info.uc_chip_id, pst_dmac_vap->pst_hal_device->uc_device_id);
        }

    }

    return pst_hal_device;
}

oal_void dmac_scan_check_rev_frame(mac_device_stru *pst_mac_device, dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_frame_channel_num)
{
    hal_to_dmac_device_stru         *pst_hal_device;
    oal_uint8                        uc_idx;
    oal_uint8                        uc_device_max;

    /* HAL接口获取支持device个数 */
    hal_chip_get_device_num(pst_mac_device->uc_chip_id, &uc_device_max);

    for (uc_idx = 0; uc_idx < uc_device_max; uc_idx++)
    {
        /* 优先检查dmac_vap 所挂接的hal device */
        pst_hal_device = dmac_scan_find_hal_device(pst_mac_device, pst_dmac_vap, uc_idx);
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        if ((pst_hal_device->uc_current_chan_number == uc_frame_channel_num)
            && (OAL_FALSE == pst_hal_device->st_hal_scan_params.en_working_in_home_chan))
        {
            pst_hal_device->st_hal_scan_params.en_scan_curr_chan_find_bss_flag = OAL_TRUE;
            break;
        }
    }
}

oal_uint32  dmac_scan_mgmt_filter(dmac_vap_stru *pst_dmac_vap, oal_void *p_param, oal_bool_enum_uint8 *pen_report_bss, oal_uint8 *pen_go_on)
{
    /* !!! 注意:dmac_rx_filter_mgmt 会根据pen_report_bss 决定是否需要释放netbuf,根据pen_go_on标志是否需要继续上报 */
    /* 如果pen_report_bss返回为OAL_TRUE代表扫描接口已经上报,dmac_rx_filter_mgmt则不会释放netbuf,否则根据pen_go_on标志继续上报或释放netbuf */
    /* 1151上，调用dmac_scan_report_scanned_bss之后，不可以再引用skb，因为已经被hmac释放了 */

    oal_netbuf_stru            *pst_netbuf;
    dmac_rx_ctl_stru           *pst_rx_ctrl;
    mac_ieee80211_frame_stru   *pst_frame_hdr;
    oal_uint8                  *puc_frame_body;
    mac_device_stru            *pst_mac_device;
    mac_scan_req_stru          *pst_scan_params;
    oal_uint32                  ul_ret;
    oal_uint16                  us_frame_len;
    oal_uint8                   uc_frame_channel_num;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    oal_uint16                  us_offset =  MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
#endif
#endif
    *pen_report_bss = OAL_FALSE;

    pst_mac_device  = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_mgmt_filter::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_scan_params = &(pst_mac_device->st_scan_params);

    pst_netbuf = (oal_netbuf_stru *)p_param;

    /* 获取该buffer的控制信息 */
    pst_rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 获取帧信息 */
    pst_frame_hdr  = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctrl->st_rx_info));
    puc_frame_body = MAC_GET_RX_PAYLOAD_ADDR(&(pst_rx_ctrl->st_rx_info), pst_netbuf);

    us_frame_len   = (oal_uint16)oal_netbuf_get_len(pst_netbuf);

    if ((WLAN_BEACON == pst_frame_hdr->st_frame_control.bit_sub_type) ||
        (WLAN_PROBE_RSP == pst_frame_hdr->st_frame_control.bit_sub_type))
    {
        /* 检测扫描到bss的类型 */
        if (OAL_SUCC != dmac_scan_check_bss_type(puc_frame_body, pst_scan_params))
        {
            //OAM_INFO_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_mgmt_filter::scanned bss isn't the desired one.}\r\n");
            return OAL_SUCC;
        }

        /* 如果是obss扫描，不上报扫描结果，只在从信道检测到了beacon帧或者probe rsp帧，拋事件到host，让20/40共存逻辑处理 */
        if (WLAN_SCAN_MODE_BACKGROUND_OBSS == pst_scan_params->en_scan_mode)
        {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
            dmac_detect_2040_te_a_b(pst_dmac_vap, puc_frame_body, us_frame_len, us_offset, pst_rx_ctrl->st_rx_info.uc_channel_number);
#endif
#endif
            /* OBSS扫描不需要继续上报报文内容或扫描结果到host */
            *pen_go_on = OAL_FALSE;
            return OAL_SUCC;
        }
        else
        {
            /* 处于扫描状态，且接收到的bss 信道信息与当前扫描信道相同，才标识当前信道扫描到BSS */
            uc_frame_channel_num = mac_ie_get_chan_num(puc_frame_body, (us_frame_len - MAC_80211_FRAME_LEN),
                                                                MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN,
                                                                pst_rx_ctrl->st_rx_info.uc_channel_number);

            dmac_scan_check_rev_frame(pst_mac_device, pst_dmac_vap, uc_frame_channel_num);
            if ((WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
                 && (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state))
            {
                /* STA扫描且linkloss超过门限1/4时，进行AP信道是否切换的识别 */
                if (GET_CURRENT_LINKLOSS_CNT(pst_dmac_vap) >= GET_CURRENT_LINKLOSS_THRESHOLD(pst_dmac_vap) >> 2)
                {
                    dmac_scan_check_assoc_ap_channel(pst_dmac_vap, pst_netbuf, SCAN_CHECK_ASSOC_CHANNEL_LINKLOSS);
                }

                if (WLAN_SCAN_MODE_BACKGROUND_CSA == pst_scan_params->en_scan_mode)
                {
                    dmac_scan_check_assoc_ap_channel(pst_dmac_vap, pst_netbuf, SCAN_CHECK_ASSOC_CHANNEL_CSA);
                    /* linkloss CSA扫描不需要上报报文或扫描结果到host */
                    *pen_go_on = OAL_FALSE;
                    return OAL_SUCC;
                }
            }

        #ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
            dmac_scan_proc_scanned_bss(pst_mac_device, pst_netbuf);
        #endif

            /* 如果是pno调度扫描，则需要进行rssi和ssid的过滤 */
            if (WLAN_SCAN_MODE_BACKGROUND_PNO == pst_scan_params->en_scan_mode)
            {
                /* 检测本bss是否可以上报，在pno扫描的场景下 */
                ul_ret = dmac_scan_check_bss_in_pno_scan(puc_frame_body,
                                                         (oal_int32)(us_frame_len - MAC_80211_FRAME_LEN),
                                                         &(pst_mac_device->pst_pno_sched_scan_mgmt->st_pno_sched_scan_params),
                                                         (oal_int32)pst_rx_ctrl->st_rx_statistic.c_rssi_dbm);
                if (OAL_SUCC != ul_ret)
                {
                    //OAM_INFO_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_mgmt_filter::this bss info can't report host.}");

                    /* PNO扫描,没有扫描到需要关联的AP,不需要上报报文或扫描结果到host */
                    *pen_go_on = OAL_FALSE;

                    return OAL_SUCC;
                }

                /* 如果是扫描到了第一个匹配的ssid，置扫描到了匹配的ssid标记位为真 */
                if (OAL_TRUE != pst_mac_device->pst_pno_sched_scan_mgmt->en_is_found_match_ssid)
                {
                    pst_mac_device->pst_pno_sched_scan_mgmt->en_is_found_match_ssid = OAL_TRUE;

                    /* 停止pno调度扫描定时器 */
                    dmac_scan_stop_pno_sched_scan_timer(pst_mac_device->pst_pno_sched_scan_mgmt);
                }
            }
#ifdef _PRE_WLAN_FEATURE_11K
            else if (WLAN_SCAN_MODE_RRM_BEACON_REQ == pst_scan_params->en_scan_mode)
            {
                /* WLAN_SCAN_MODE_RRM_BEACON_REQ扫描,不需要上报报文或扫描结果到host */
                *pen_go_on = OAL_FALSE;

                dmac_rrm_get_bcn_info_from_rx(pst_dmac_vap, pst_netbuf);
                return OAL_SUCC;
            }
#endif

            /* 其它模式扫描，上报扫描到的扫描结果 */
            ul_ret = dmac_scan_report_scanned_bss(pst_dmac_vap, p_param);
            if (OAL_SUCC != ul_ret)
            {
                *pen_report_bss = OAL_FALSE;
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_mgmt_filter::report scan result failed.}");
                return OAL_SUCC;
            }
            else
            {
                *pen_report_bss = OAL_TRUE;
            }
        }
    }
    else if(WLAN_ACTION == pst_frame_hdr->st_frame_control.bit_sub_type)
    {
        switch (puc_frame_body[MAC_ACTION_OFFSET_CATEGORY])
        {
            case MAC_ACTION_CATEGORY_PUBLIC:
            {
                switch (puc_frame_body[MAC_ACTION_OFFSET_ACTION])
                {
                    case MAC_PUB_FTM:
#ifdef _PRE_WLAN_FEATURE_FTM
                        *pen_go_on = OAL_FALSE;
                        if(WLAN_SCAN_MODE_FTM_REQ == pst_scan_params->en_scan_mode)
                        {
                            dmac_sta_rx_ftm(pst_dmac_vap, pst_netbuf);
                        }
#endif
                        break;

                    default:
                        break;
                }
            }
            break;

            default:
            break;
        }
    }
    return OAL_SUCC;
}


OAL_STATIC oal_uint16  dmac_scan_encap_probe_req_frame(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_mgmt_buf, oal_uint8 *puc_bssid, oal_int8 *pc_ssid)
{
    oal_uint8        uc_ie_len;
    oal_uint8       *puc_mac_header          = oal_netbuf_header(pst_mgmt_buf);
    oal_uint8       *puc_payload_addr        = mac_netbuf_get_payload(pst_mgmt_buf);
    oal_uint8       *puc_payload_addr_origin = puc_payload_addr;
    mac_device_stru *pst_mac_device          = OAL_PTR_NULL;
    oal_uint16       us_app_ie_len;
    oal_uint8        uc_dsss_channel_num;

    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_scan_encap_probe_req_frame::pst_mac_device[%d] null!}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return 0;
    }

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_PROBE_REQ);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* 设置地址1，广播地址 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, BROADCAST_MACADDR);

    /* 设置地址2的MAC地址(p2p扫描为p2p的地址，其它为本机地址，如果随机mac addr扫描开启，则为随机地址) */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, pst_mac_device->st_scan_params.auc_sour_mac_addr);

    /* 地址3，广播地址 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, puc_bssid);

    /*************************************************************************/
    /*                       Probe Request Frame - Frame Body                */
    /* --------------------------------------------------------------------- */
    /* |SSID |Supported Rates |Extended supp rates| HT cap|Extended cap      */
    /* --------------------------------------------------------------------- */
    /* |2-34 |   3-10         | 2-257             |  28   | 3-8              */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /* 设置SSID */
    /***************************************************************************
                    ----------------------------
                    |Element ID | Length | SSID|
                    ----------------------------
           Octets:  |1          | 1      | 0~32|
                    ----------------------------
    ***************************************************************************/
    if ('\0' == pc_ssid[0])    /* 通配SSID */
    {
        puc_payload_addr[0] = MAC_EID_SSID;
        puc_payload_addr[1] = 0;
        puc_payload_addr   += MAC_IE_HDR_LEN;    /* 偏移buffer指向下一个ie */
    }
    else
    {
        puc_payload_addr[0] = MAC_EID_SSID;
        puc_payload_addr[1] = (oal_uint8)OAL_STRLEN(pc_ssid);
        oal_memcopy(&(puc_payload_addr[2]), pc_ssid, puc_payload_addr[1]);
        puc_payload_addr += MAC_IE_HDR_LEN + puc_payload_addr[1];  /* 偏移buffer指向下一个ie */
    }

    /* 设置支持的速率集 */
    mac_set_supported_rates_ie(&(pst_dmac_vap->st_vap_base_info), puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 获取dsss ie内的channel num */
    uc_dsss_channel_num = dmac_get_dsss_ie_channel_num(&(pst_dmac_vap->st_vap_base_info));/* 设置dsss参数集 */

    /* 设置dsss参数集 */
    mac_set_dsss_params(&(pst_dmac_vap->st_vap_base_info), puc_payload_addr, &uc_ie_len, uc_dsss_channel_num);
    puc_payload_addr += uc_ie_len;

    /* 设置extended supported rates信息 */
    mac_set_exsup_rates_ie(&(pst_dmac_vap->st_vap_base_info), puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* PNO扫描,probe request报文只包含信道和速率集信息元素,减少发送报文长度 */
    if((MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state)
       && (WLAN_SCAN_MODE_BACKGROUND_PNO == pst_mac_device->st_scan_params.en_scan_mode))
    {
        return (oal_uint16)((puc_payload_addr - puc_payload_addr_origin) + MAC_80211_FRAME_LEN);
    }

    /* 填充HT Capabilities信息 */
    mac_set_ht_capabilities_ie(&(pst_dmac_vap->st_vap_base_info), puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充Extended Capabilities信息 */
    mac_set_ext_capabilities_ie(&(pst_dmac_vap->st_vap_base_info), puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充vht capabilities信息 */
    if((WLAN_BAND_2G != pst_dmac_vap->st_vap_base_info.st_channel.en_band))
    {
        mac_set_vht_capabilities_ie(&(pst_dmac_vap->st_vap_base_info), puc_payload_addr, &uc_ie_len);
        puc_payload_addr += uc_ie_len;
    }

    /* 填充WPS信息 */
    mac_add_app_ie((oal_void *)&(pst_dmac_vap->st_vap_base_info), puc_payload_addr, &us_app_ie_len, OAL_APP_PROBE_REQ_IE);
    puc_payload_addr += us_app_ie_len;
#ifdef _PRE_WLAN_FEATURE_HILINK
    /* 填充okc ie信息 */
    mac_add_app_ie((oal_void *)&(pst_dmac_vap->st_vap_base_info), puc_payload_addr, &us_app_ie_len, OAL_APP_OKC_PROBE_IE);
    puc_payload_addr += us_app_ie_len;
#endif

#ifdef _PRE_WLAN_FEATURE_11K
    if (OAL_TRUE == pst_dmac_vap->bit_11k_enable)
    {
        mac_set_wfa_tpc_report_ie(&(pst_dmac_vap->st_vap_base_info), puc_payload_addr, &uc_ie_len);
        puc_payload_addr += uc_ie_len;
    }
#endif //_PRE_WLAN_FEATURE_11K

    /* multi-sta特性下新增4地址ie */
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    mac_set_vender_4addr_ie((oal_void *)(&pst_dmac_vap->st_vap_base_info), puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;
#endif

    return (oal_uint16)((puc_payload_addr - puc_payload_addr_origin) + MAC_80211_FRAME_LEN);
}


oal_uint32  dmac_scan_send_probe_req_frame(dmac_vap_stru *pst_dmac_vap,
                                            oal_uint8 *puc_bssid,
                                            oal_int8 *pc_ssid)
{
    oal_netbuf_stru        *pst_mgmt_buf;
    mac_tx_ctl_stru        *pst_tx_ctl;
    oal_uint32              ul_ret;
    oal_uint16              us_mgmt_len;
    oal_uint8              *puc_mac_header = OAL_PTR_NULL;
    oal_uint8              *puc_saddr;

    /* 申请管理帧内存 */
    pst_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_SMGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_mgmt_buf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_send_probe_req_frame::alloc netbuf failed.}");
        OAL_MEM_INFO_PRINT(OAL_MEM_POOL_ID_NETBUF);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(pst_mgmt_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_mgmt_buf, OAL_PTR_NULL);

    OAL_MEM_NETBUF_TRACE(pst_mgmt_buf, OAL_TRUE);

    /* 封装probe request帧 */
    us_mgmt_len = dmac_scan_encap_probe_req_frame(pst_dmac_vap, pst_mgmt_buf, puc_bssid, pc_ssid);
    if(WLAN_SMGMT_NETBUF_SIZE < (us_mgmt_len - MAC_80211_FRAME_LEN))
    {
        OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_send_probe_req_frame::us_payload_len=[%d] over net_buf_size=[%d].}",
        us_mgmt_len - MAC_80211_FRAME_LEN, WLAN_SMGMT_NETBUF_SIZE);
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_mgmt_buf);

    OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_MGMT;

    if (!ETHER_IS_MULTICAST(puc_bssid))
    {
        /* 发送单播探测帧 */
        puc_mac_header = oal_netbuf_header(pst_mgmt_buf);
        puc_saddr = mac_vap_get_mac_addr(&(pst_dmac_vap->st_vap_base_info));
        oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, puc_bssid);
        oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, puc_saddr);
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = pst_dmac_vap->st_vap_base_info.us_assoc_vap_id;
    }
    else
    {
        /* 发动广播探测帧 */
        MAC_GET_CB_IS_MCAST(pst_tx_ctl) = OAL_TRUE;
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = pst_dmac_vap->st_vap_base_info.us_multi_user_idx; /* probe request帧是广播帧 */
    }

    /* 调用发送管理帧接口 */
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, us_mgmt_len);
    if (OAL_SUCC != ul_ret)
    {

        oal_netbuf_free(pst_mgmt_buf);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32  dmac_scan_proc_scan_complete_event(dmac_vap_stru *pst_dmac_vap,
                                               mac_scan_status_enum_uint8 en_scan_rsp_status)
{
    frw_event_mem_stru         *pst_event_mem;
    frw_event_stru             *pst_event;
    mac_device_stru            *pst_mac_device;
    oal_uint8                   uc_vap_id;
    mac_scan_rsp_stru          *pst_scan_rsp_info;
#ifdef _PRE_WLAN_FEATURE_DBAC
    mac_vap_state_enum_uint8    en_state = pst_dmac_vap->st_vap_base_info.en_vap_state;
    oal_bool_enum_uint8         en_dbac_running;
#endif
    uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;

    /* 获取device结构体 */
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_SCAN, "{dmac_scan_proc_scan_complete_event::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_DBAC
    en_dbac_running = mac_is_dbac_running(pst_mac_device);
#endif
    /* 抛扫描请求事件到DMAC, 申请事件内存 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(mac_scan_rsp_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_SCAN, "{dmac_scan_proc_scan_complete_event::alloc memory failed.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CRX,
                       DMAC_WLAN_CRX_EVENT_SUB_TYPE_SCAN_COMP,
                       OAL_SIZEOF(mac_scan_rsp_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_dmac_vap->st_vap_base_info.uc_chip_id,
                       pst_dmac_vap->st_vap_base_info.uc_device_id,
                       pst_dmac_vap->st_vap_base_info.uc_vap_id);

    pst_scan_rsp_info = (mac_scan_rsp_stru *)(pst_event->auc_event_data);

    /* 设置扫描完成时状态，是被拒绝，还是执行成功 */
    if(OAL_TRUE == pst_mac_device->st_scan_params.en_abort_scan_flag)
    {
        pst_scan_rsp_info->en_scan_rsp_status = MAC_SCAN_ABORT;
    }
    else
    {
        pst_scan_rsp_info->en_scan_rsp_status = en_scan_rsp_status;
    }

    pst_scan_rsp_info->ull_cookie         = pst_mac_device->st_scan_params.ull_cookie;

    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_proc_scan_complete_event::status:%d, vap channel:%d, cookie[%x], scan_mode[%d]}",
                        en_scan_rsp_status,
                        pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number,
                        pst_scan_rsp_info->ull_cookie,
                        pst_mac_device->st_scan_params.en_scan_mode);

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);

    // HMAC会将VAP的状态设置为扫描之前的状态，可能导致VAP误UP,此处恢复VAP的状态
    // HMAC执行流程中可能修改了DBAC工作状态，因此不可以在此处获取vap状态
#ifdef _PRE_WLAN_FEATURE_DBAC
    if (en_dbac_running
    && (pst_dmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP)
    && (pst_dmac_vap->st_vap_base_info.en_vap_state != en_state))
    {
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                "{dmac_scan_proc_scan_complete_event::restore state from %d to %d when scan while dbac running}",
                        pst_dmac_vap->st_vap_base_info.en_vap_state, en_state);
        mac_vap_state_change(&pst_dmac_vap->st_vap_base_info, en_state);
    }
#endif

    return OAL_SUCC;
}

#if 0

OAL_STATIC oal_uint32  dmac_scan_is_too_busy(mac_device_stru *pst_mac_device, mac_scan_req_stru *pst_scan_req_params)
{
    oal_uint32       ul_ret;
    oal_uint32       ul_timestamp;
    oal_uint32       ul_deltatime;

    ul_ret = mac_device_is_p2p_connected(pst_mac_device);
    if (OAL_SUCC == ul_ret)
    {
        ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
        ul_deltatime = (ul_timestamp > pst_mac_device->ul_scan_timestamp)? \
                       (ul_timestamp - pst_mac_device->ul_scan_timestamp):(0xffffffff - pst_mac_device->ul_scan_timestamp + ul_timestamp);

        if (MAC_SCAN_FUNC_P2P_LISTEN != pst_scan_req_params->uc_scan_func)
        {
            if (ul_deltatime < DMAC_SCAN_DBAC_SCAN_DELTA_TIME)
            {
                OAM_WARNING_LOG2(0, OAM_SF_DBAC, "has connected p2p. scan deltatime:%d<%d, refused", ul_deltatime, DMAC_SCAN_DBAC_SCAN_DELTA_TIME);
                return OAL_TRUE;
            }
        }
        else
        {
            if (ul_deltatime > 500 || pst_scan_req_params->us_scan_time >= DMAC_SCAN_GO_MAX_SCAN_TIME)
            {
                OAM_WARNING_LOG2(0, OAM_SF_DBAC, "has connected p2p. p2p listen deltatime:%d, scan_time:%d, refused", ul_deltatime, pst_scan_req_params->us_scan_time);
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}
#endif


oal_uint32  dmac_scan_update_channel_list(mac_device_stru    *pst_mac_device,
                                        dmac_vap_stru      *pst_dmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (mac_is_proxysta_enabled(pst_mac_device))
    {
        /* proxysta 只扫描一个信道 */
        if (mac_vap_is_vsta(&pst_dmac_vap->st_vap_base_info))
        {
            oal_uint8 uc_idx;
            oal_int32 l_found_idx = -1;

            mac_vap_stru *pst_msta = mac_find_main_proxysta(pst_mac_device);

            if (!pst_msta)
            {
                return OAL_FAIL;
            }

            for (uc_idx = 0; uc_idx < pst_mac_device->st_scan_params.uc_channel_nums; uc_idx++)
            {
                if (pst_mac_device->st_scan_params.ast_channel_list[uc_idx].uc_chan_number == pst_msta->st_channel.uc_chan_number)
                {
                    l_found_idx = (oal_int32)uc_idx;
                    break;
                }
            }

            if (l_found_idx >= 0)
            {
                pst_mac_device->st_scan_params.uc_channel_nums = 1;
                pst_mac_device->st_scan_params.ast_channel_list[0] = pst_mac_device->st_scan_params.ast_channel_list[l_found_idx];
            }
        }
    }
#endif
    return OAL_SUCC;
}

OAL_STATIC oal_void dmac_scan_set_fast_scan_flag(hal_scan_info_stru *pst_scan_info, dmac_device_stru *pst_dmac_device)
{
    /* 同时扫2g 5g,硬件支持,软件支持 */
    if ((pst_scan_info->uc_num_channels_2G != 0) && (pst_scan_info->uc_num_channels_5G != 0)
          && (OAL_TRUE == dmac_device_is_support_double_hal_device(pst_dmac_device))
          && (OAL_TRUE == pst_dmac_device->en_fast_scan_enable))
    {
        pst_dmac_device->en_is_fast_scan = OAL_TRUE;
    }
    else
    {
        pst_dmac_device->en_is_fast_scan = OAL_FALSE;
    }
}


OAL_STATIC oal_void dmac_scan_assign_chan_for_hal_device(dmac_device_stru  *pst_dmac_device, hal_scan_info_stru *pst_scan_info, dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8                               uc_idx;
    oal_uint8                               uc_dev_num_per_chip;
    hal_to_dmac_device_stru                *pst_hal_device;
    //hal_to_dmac_device_stru                *pst_work_hal_device = OAL_PTR_NULL;
    //hal_to_dmac_device_stru                *pst_other_hal_device = OAL_PTR_NULL;

    if (OAL_FALSE == pst_dmac_device->en_is_fast_scan)
    {
        /* hal device 准备扫描channel参数 */
        hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_SCAN_BEGIN, OAL_SIZEOF(hal_scan_info_stru), (oal_uint8 *)pst_scan_info);
        return;
    }

    hal_chip_get_device_num(pst_dmac_device->pst_device_base_info->uc_chip_id, &uc_dev_num_per_chip);
    for (uc_idx = 0; uc_idx < uc_dev_num_per_chip; uc_idx++)
    {
        pst_hal_device = dmac_scan_find_hal_device(pst_dmac_device->pst_device_base_info, pst_dmac_vap, uc_idx);
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        if (OAL_FALSE == pst_dmac_device->pst_device_base_info->en_dbdc_running)
        {
            /* 主路扫2g,辅路扫5g */
            pst_scan_info->en_scan_band = (OAL_TRUE == pst_hal_device->en_is_master_hal_device) ? WLAN_BAND_2G : WLAN_BAND_5G;

            /* 根据迁移的vap决定是否要将11b分配给迁移到的hal device */
            if (WLAN_BAND_2G == pst_scan_info->en_scan_band)
            {
                hal_set_11b_reuse_sel(pst_hal_device);
            }
        }
        /* DBDC模式下可以自己扫自己工作的频段,可以独立结束各hal device的扫描 */
        else
        {
            pst_scan_info->en_scan_band = pst_hal_device->st_wifi_channel_status.en_band;
        }

        /* hal device 准备扫描channel参数 */
        hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_SCAN_BEGIN, OAL_SIZEOF(hal_scan_info_stru), (oal_uint8 *)pst_scan_info);
    }
}

OAL_STATIC oal_void dmac_fscan_reorder_channel_list(mac_device_stru  *pst_mac_device, hal_scan_info_stru *pst_scan_info)
{
    oal_uint8                               uc_idx;
    oal_int8                                c_chan_idx;
    oal_uint8                               uc_chan_cnt = 0;
    oal_int8                                c_bad_chan_cnt =0;
    mac_channel_stru                        ast_bad_channel_list[6];

    OAL_MEMZERO(ast_bad_channel_list, OAL_SIZEOF(ast_bad_channel_list));

    uc_chan_cnt = pst_scan_info->uc_num_channels_2G;//从第一个5g信道开始

    /*并发扫描是将5G 100-120信道放到最后面顺序为120 116 112 108 104 100*/
    for (uc_idx = uc_chan_cnt; uc_idx < pst_mac_device->st_scan_params.uc_channel_nums; uc_idx++)
    {
        if (WLAN_BAND_2G == pst_mac_device->st_scan_params.ast_channel_list[uc_idx].en_band)
        {
            OAM_ERROR_LOG4(0, OAM_SF_DBDC, "dmac_fscan_reorder_channel_list:wrong!!!:band[%d],chan[%d]bw[%d]idx[%d]",
                        pst_mac_device->st_scan_params.ast_channel_list[uc_idx].en_band,pst_mac_device->st_scan_params.ast_channel_list[uc_idx].uc_chan_number, pst_mac_device->st_scan_params.ast_channel_list[uc_idx].en_bandwidth,
                        pst_mac_device->st_scan_params.ast_channel_list[uc_idx].uc_chan_idx);
        }
        if (((pst_mac_device->st_scan_params.ast_channel_list[uc_idx].uc_chan_number < 100)
               || (pst_mac_device->st_scan_params.ast_channel_list[uc_idx].uc_chan_number > 120)))
        {
            /* 100-120的位置由后面的信道顶替 */
            if (uc_chan_cnt != uc_idx)
            {
                pst_mac_device->st_scan_params.ast_channel_list[uc_chan_cnt] = pst_mac_device->st_scan_params.ast_channel_list[uc_idx];
            }
            uc_chan_cnt++;
        }
        /* 保存100-120的信道信息 */
        else
        {
            ast_bad_channel_list[(oal_uint8)c_bad_chan_cnt++] = pst_mac_device->st_scan_params.ast_channel_list[uc_idx];
        }
    }

    /*5G good 信道个数小于2G总的信道个数,此时必定会撞上2G影响5G信道的场景 */
    if ((pst_scan_info->uc_num_channels_5G - c_bad_chan_cnt) < pst_scan_info->uc_num_channels_2G)
    {
        OAM_WARNING_LOG0(0, OAM_SF_DBDC, "dmac_scan_reorder_channel_list::2g affect 5g");
    }

    /*再将100-120的信道放回原扫描信道列表,信道号越小限制的2g信道越多,放最后面 */
    if (c_bad_chan_cnt != 0)
    {
        for (c_chan_idx = (c_bad_chan_cnt - 1); c_chan_idx >= 0; c_chan_idx--)
        {
            pst_mac_device->st_scan_params.ast_channel_list[uc_chan_cnt++] = ast_bad_channel_list[(oal_uint8)c_chan_idx];
        }
    }
}


// TODO: 考虑将这些启动vap必须写的寄存器，在配置主路是就同时写辅路的
OAL_STATIC oal_void dmac_prepare_for_fast_scan(dmac_device_stru *pst_dmac_device, dmac_vap_stru *pst_dmac_vap, hal_scan_info_stru *pst_scan_info)
{
    hal_to_dmac_vap_stru                   *pst_ori_hal_vap;
    hal_to_dmac_vap_stru                   *pst_shift_hal_vap;
    hal_to_dmac_device_stru                *pst_ori_hal_device;
    hal_to_dmac_device_stru                *pst_shift_hal_device;

    pst_ori_hal_device   = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
    pst_shift_hal_device = dmac_device_get_another_h2d_dev(pst_dmac_device, pst_ori_hal_device);
    if (OAL_PTR_NULL  == pst_shift_hal_device)
    {
        OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_DBDC, "dmac_prepare_for_fast_scan::pst_shift_hal_device NULL, chip id[%d],ori hal device id[%d]",
                        pst_dmac_vap->st_vap_base_info.uc_chip_id, pst_ori_hal_device->uc_device_id);
        return;
    }

    pst_ori_hal_vap  = DMAC_VAP_GET_HAL_VAP(pst_dmac_vap);
    hal_get_hal_vap(pst_shift_hal_device, pst_ori_hal_vap->uc_vap_id, &pst_shift_hal_vap);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P 设置MAC 地址 */
    if ((WLAN_P2P_DEV_MODE == pst_dmac_vap->st_vap_base_info.en_p2p_mode) && (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode))
    {
        hal_vap_set_macaddr(pst_shift_hal_vap, pst_dmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID);
    }
    else
    {
        /* 设置其他vap 的mac 地址 */
        hal_vap_set_macaddr(pst_shift_hal_vap, mac_mib_get_StationID(&(pst_dmac_vap->st_vap_base_info)));
    }
#else
    /* 配置MAC地址 */
    hal_vap_set_macaddr(pst_shift_hal_vap, mac_mib_get_StationID(&(pst_dmac_vap->st_vap_base_info)));
#endif

    /* 使能PA_CONTROL的vap_control位 */
    hal_vap_set_opmode(pst_shift_hal_vap, pst_dmac_vap->st_vap_base_info.en_vap_mode);

    /* 重排序扫描信道列表 */
    dmac_fscan_reorder_channel_list(pst_dmac_device->pst_device_base_info, pst_scan_info);
}


OAL_STATIC oal_void dmac_prepare_for_scan(mac_device_stru  *pst_mac_device, dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8                               uc_idx;
    hal_scan_info_stru                      st_scan_info = {0};
    dmac_device_stru                       *pst_dmac_device;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    if (pst_mac_device->st_scan_params.en_scan_mode < WLAN_SCAN_MODE_BACKGROUND_OBSS)
    {
        pst_mac_device->uc_scan_count++;
    }
#endif

    /* 记录扫描开始时间,扫描结束后看整体扫描时间 */
    pst_mac_device->ul_scan_timestamp  = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    for (uc_idx = 0; uc_idx < pst_mac_device->st_scan_params.uc_channel_nums; uc_idx++)
    {
        if (WLAN_BAND_2G == pst_mac_device->st_scan_params.ast_channel_list[uc_idx].en_band)
        {
             st_scan_info.uc_num_channels_2G++;
        }
        else if (WLAN_BAND_5G == pst_mac_device->st_scan_params.ast_channel_list[uc_idx].en_band)
        {
             st_scan_info.uc_num_channels_5G++;
        }
        else
        {

        }
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_device->uc_device_id, OAM_SF_SCAN, "{dmac_prepare_for_scan::pst_dmac_device null.}");
        return;
    }

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
    dmac_scan_init_bss_info_list(pst_mac_device);
#endif

    /* 硬件支持并且需要扫2G和5G那么就是并发扫描 */
    dmac_scan_set_fast_scan_flag(&st_scan_info, pst_dmac_device);
    if (OAL_TRUE == pst_dmac_device->en_is_fast_scan)
    {
        dmac_prepare_for_fast_scan(pst_dmac_device, pst_dmac_vap, &st_scan_info);
    }

    st_scan_info.en_is_fast_scan               = pst_dmac_device->en_is_fast_scan;
    st_scan_info.us_scan_time                  = pst_mac_device->st_scan_params.us_scan_time;
    st_scan_info.en_scan_mode                  = pst_mac_device->st_scan_params.en_scan_mode;
    st_scan_info.uc_scan_channel_interval      = pst_mac_device->st_scan_params.uc_scan_channel_interval;
    st_scan_info.uc_max_scan_count_per_channel = pst_mac_device->st_scan_params.uc_max_scan_count_per_channel;
    st_scan_info.us_work_time_on_home_channel  = pst_mac_device->st_scan_params.us_work_time_on_home_channel;

    dmac_scan_assign_chan_for_hal_device(pst_dmac_device, &st_scan_info, pst_dmac_vap);
}

oal_uint32  dmac_scan_handle_scan_req_entry(mac_device_stru    *pst_mac_device,
                                            dmac_vap_stru      *pst_dmac_vap,
                                            mac_scan_req_stru  *pst_scan_req_params)
{
    if (WLAN_SCAN_MODE_BACKGROUND_CCA == pst_scan_req_params->en_scan_mode)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "dmac_scan_handle_scan_req_entry::cca can not be here!!!");
        return OAL_FAIL;
    }

    /* 如果处于扫描状态，则直接返回 */
    /* 如果处于常发常收状态，则直接返回 */
    if((MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state)
       || ((OAL_SWITCH_ON == pst_dmac_vap->st_vap_base_info.bit_al_tx_flag) && (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)))
    {
    #if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
        OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_handle_scan_req_entry:: device scan is running or always tx is running, cann't start scan. scan_vap_id[%d], scan_func[0x%02x], curr_scan_mode[%d], req_scan_mode[%d], scan_cookie[%x], al_tx_flag[%d].}",
                         6,
                         pst_mac_device->st_scan_params.uc_vap_id,
                         pst_mac_device->st_scan_params.uc_scan_func,
                         pst_mac_device->st_scan_params.en_scan_mode,
                         pst_scan_req_params->en_scan_mode,
                         pst_scan_req_params->ull_cookie,
                         pst_dmac_vap->st_vap_base_info.bit_al_tx_flag);
    #else
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_handle_scan_req_entry:: device scan is running or always tx is running, cann't start scan. scan_vap_id[%d], scan_func[0x%02x], scan_cookie[%x], al_tx_flag[%d].}",
                         pst_mac_device->st_scan_params.uc_vap_id,
                         pst_mac_device->st_scan_params.uc_scan_func,
                         pst_scan_req_params->ull_cookie,
                         pst_dmac_vap->st_vap_base_info.bit_al_tx_flag);
    #endif

        /* 如果是上层下发的扫描请求，直接抛扫描完成事件; OBSS扫描则返回结束，等待下一次定时器超时再发起扫描 */
        if (pst_scan_req_params->en_scan_mode < WLAN_SCAN_MODE_BACKGROUND_OBSS)
        {
            /* 更新扫描下发的cookie 值 */
            pst_mac_device->st_scan_params.ull_cookie = pst_scan_req_params->ull_cookie;

            /* 抛扫描完成事件，扫描请求被拒绝 */
            return dmac_scan_proc_scan_complete_event(pst_dmac_vap, MAC_SCAN_REFUSED);
        }
#ifdef _PRE_WLAN_FEATURE_11K
        else if (WLAN_SCAN_MODE_RRM_BEACON_REQ == pst_scan_req_params->en_scan_mode)
        {
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_handle_scan_req_entry::RRM BEACON REQ SCAN FAIL");
            return OAL_FAIL;
        }
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
        else if (WLAN_SCAN_MODE_FTM_REQ == pst_scan_req_params->en_scan_mode)
        {
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_handle_scan_req_entry::FTM SCAN FAIL");
            return OAL_FAIL;
        }
#endif
#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
        else if (WLAN_SCAN_MODE_GNSS_SCAN == pst_scan_req_params->en_scan_mode)
        {
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_handle_scan_req_entry::GNSS SCAN FAIL");
            return OAL_FAIL;
        }
#endif
        else
        {
            return OAL_SUCC;
        }
    }

    /* 设置device当前扫描状态为运行状态 */
    pst_mac_device->en_curr_scan_state = MAC_SCAN_STATE_RUNNING;

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    //扫描，bt ps和低功耗同等调度，直接暂停bt ps暂时放弃
    //dmac_btcoex_bt_acl_status_check(pst_dmac_vap);

    /* 入网开始，通知BT */
    hal_set_btcoex_soc_gpreg1(OAL_FALSE, BIT0, 0);  //清除第一次上电扫描的状态
    hal_set_btcoex_soc_gpreg1(OAL_TRUE, BIT1, 1);   // 入网流程，处于扫描状态

    if (WLAN_P2P_DEV_MODE == pst_dmac_vap->st_vap_base_info.en_p2p_mode)
    {
        hal_set_btcoex_soc_gpreg0(OAL_TRUE, BIT14, 14);   // p2p 扫描流程开始
    }

    hal_coex_sw_irq_set(HAL_COEX_SW_IRQ_BT);
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
    /* 平台的计数 */
    if (WLAN_SCAN_MODE_BACKGROUND_AP >= pst_scan_req_params->en_scan_mode)
    {
        pfn_wlan_dumpsleepcnt();
    }
#endif

    /* 竞争到扫描权限后，将扫描参数拷贝到mac deivce结构体下，此时拷贝，也是为了防止扫描参数被覆盖情况 */
    oal_memcopy(&(pst_mac_device->st_scan_params), pst_scan_req_params, OAL_SIZEOF(mac_scan_req_stru));

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P0 扫描时记录P2P listen channel */
    if (OAL_TRUE == pst_scan_req_params->bit_is_p2p0_scan)
    {
        pst_dmac_vap->st_vap_base_info.uc_p2p_listen_channel = pst_scan_req_params->uc_p2p0_listen_channel;
    }
#endif
    /* 初始化扫描信道索引 */
    dmac_prepare_for_scan(pst_mac_device, pst_dmac_vap);

    dmac_scan_update_channel_list(pst_mac_device, pst_dmac_vap);

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (mac_is_proxysta_enabled(pst_mac_device)
      && pst_mac_device->st_scan_params.uc_channel_nums == 1
      && pst_mac_device->st_scan_params.ast_channel_list[0].uc_chan_number == pst_dmac_vap->pst_hal_device->uc_current_chan_number)
    {
        OAM_INFO_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,"{dmac_scan_handle_scan_req_entry:proxysta scan working channel %d, skip switching}",
                pst_mac_device->st_scan_params.ast_channel_list[0].uc_chan_number);
    }
    else
#endif
    {
        dmac_scan_switch_channel_off(pst_mac_device);//lzhqi 函数名与实际用途有些偏差
    }

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
    dmac_alg_anti_intf_switch(pst_dmac_vap->pst_hal_device, OAL_FALSE);
#endif

#ifdef  _PRE_WLAN_FEATURE_GREEN_AP
    dmac_green_ap_switch_auto(pst_mac_device->uc_device_id, DMAC_GREEN_AP_SCAN_START);
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_scan_prepare_pno_scan_params(mac_scan_req_stru  *pst_scan_params,
                                                                  dmac_vap_stru    *pst_dmac_vap)
{
    oal_uint8           uc_chan_idx;
    oal_uint8           uc_2g_chan_num = 0;
    oal_uint8           uc_5g_chan_num = 0;
    oal_uint8           uc_chan_number;
    mac_device_stru    *pst_mac_device;

    /* 扫描请求参数清零 */
    OAL_MEMZERO(pst_scan_params, OAL_SIZEOF(mac_scan_req_stru));

    /* 设置发起扫描的vap id */
    pst_scan_params->uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_SCAN, "{dmac_scan_prepare_pno_scan_params::pst_mac_device null.}");
        return;
    }

    /* 设置初始扫描请求的参数 */
    pst_scan_params->en_bss_type    = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    pst_scan_params->en_scan_type   = WLAN_SCAN_TYPE_ACTIVE;
    pst_scan_params->en_scan_mode   = WLAN_SCAN_MODE_BACKGROUND_PNO;
    pst_scan_params->us_scan_time   = WLAN_DEFAULT_ACTIVE_SCAN_TIME * 2; /* 考虑PNO指定SSID扫描,延长每信道扫描时间为40ms */
    pst_scan_params->uc_probe_delay = 0;
    pst_scan_params->uc_scan_func   = MAC_SCAN_FUNC_BSS;   /* 默认扫描bss */
    pst_scan_params->uc_max_scan_count_per_channel           = 1;
    pst_scan_params->uc_max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;

    /* 设置扫描用的pro req的src mac地址*/
    oal_set_mac_addr(pst_scan_params->auc_sour_mac_addr, pst_mac_device->pst_pno_sched_scan_mgmt->st_pno_sched_scan_params.auc_sour_mac_addr);
    pst_scan_params->en_is_random_mac_addr_scan = pst_mac_device->pst_pno_sched_scan_mgmt->st_pno_sched_scan_params.en_is_random_mac_addr_scan;

    /* 设置扫描请求的ssid信息 */
    pst_scan_params->ast_mac_ssid_set[0].auc_ssid[0] = '\0';   /* 通配ssid */
    pst_scan_params->uc_ssid_num = 1;

    /* 设置扫描请求只指定1个bssid，为广播地址 */
    oal_set_mac_addr(pst_scan_params->auc_bssid[0], BROADCAST_MACADDR);
    pst_scan_params->uc_bssid_num = 1;

    /* 2G初始扫描信道, 全信道扫描 */
    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_chan_idx++)
    {
        /* 判断信道是不是在管制域内 */
        if (OAL_SUCC == mac_is_channel_idx_valid(WLAN_BAND_2G, uc_chan_idx))
        {
            mac_get_channel_num_from_idx(WLAN_BAND_2G, uc_chan_idx, &uc_chan_number);

            pst_scan_params->ast_channel_list[uc_2g_chan_num].uc_chan_number = uc_chan_number;
            pst_scan_params->ast_channel_list[uc_2g_chan_num].en_band        = WLAN_BAND_2G;
            pst_scan_params->ast_channel_list[uc_2g_chan_num].uc_chan_idx         = uc_chan_idx;
            pst_scan_params->uc_channel_nums++;
            uc_2g_chan_num++;
        }
    }
    //OAM_INFO_LOG1(0, OAM_SF_SCAN, "{dmac_scan_prepare_pno_scan_params::after regdomain filter, the 2g total channel num is %d", uc_2g_chan_num);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if (!hal_get_5g_enable())
    {
        return;
    }
#endif

    /* 5G初始扫描信道, 全信道扫描 */
    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_chan_idx++)
    {
        /* 判断信道是不是在管制域内 */
        if (OAL_SUCC == mac_is_channel_idx_valid(WLAN_BAND_5G, uc_chan_idx))
        {
            mac_get_channel_num_from_idx(WLAN_BAND_5G, uc_chan_idx, &uc_chan_number);

            pst_scan_params->ast_channel_list[uc_2g_chan_num + uc_5g_chan_num].uc_chan_number = uc_chan_number;
            pst_scan_params->ast_channel_list[uc_2g_chan_num + uc_5g_chan_num].en_band        = WLAN_BAND_5G;
            pst_scan_params->ast_channel_list[uc_2g_chan_num + uc_5g_chan_num].uc_chan_idx         = uc_chan_idx;
            pst_scan_params->uc_channel_nums++;
            uc_5g_chan_num++;
        }
    }
    //OAM_INFO_LOG1(0, OAM_SF_SCAN, "{dmac_scan_prepare_pno_scan_params::after regdomain filter, the 5g total channel num is %d", uc_5g_chan_num);

    return;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

/*lint -e528*/
OAL_STATIC oal_void  dmac_scan_pno_scan_timeout_fn(void *p_ptr, void *p_arg)
{
    dmac_vap_stru                       *pst_dmac_vap;
    mac_device_stru                     *pst_mac_device;
    mac_scan_req_stru                    st_scan_req_params;


    pst_dmac_vap = (dmac_vap_stru *)p_arg;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_pno_scan_timeout_fn::pst_dmac_vap null.}");
        return;
    }

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{dmac_scan_pno_scan_timeout_fn::pst_mac_device null.}");
        return;
    }

#if 0
    /* 更新pno调度扫描的次数 */
    pst_mac_device->pst_pno_sched_scan_mgmt->uc_curr_pno_sched_scan_times++;

    /* pno调度扫描到达最大重复次数，停止扫描进入低功耗 */
    if (pst_mac_device->pst_pno_sched_scan_mgmt->uc_curr_pno_sched_scan_times >= pst_mac_device->pst_pno_sched_scan_mgmt->st_pno_sched_scan_params.uc_pno_scan_repeat)
    {
        /* 停止PNO扫描，并进入低功耗 */
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{dmac_scan_pno_scan_timeout_fn:: reached max pno scan repeat times, stop pno sched scan.}");

#ifdef _PRE_WLAN_FEATURE_STA_PM
        dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_PNO_SCHED_SCAN_COMP, 0, OAL_PTR_NULL);
#endif
        /* 释放PNO管理结构体内存 */
        OAL_MEM_FREE(pst_mac_device->pst_pno_sched_scan_mgmt, OAL_TRUE);
        pst_mac_device->pst_pno_sched_scan_mgmt = OAL_PTR_NULL;
        return OAL_SUCC;
    }
#endif

    /* 初始化设置为: 未扫描到匹配的bss */
    pst_mac_device->pst_pno_sched_scan_mgmt->en_is_found_match_ssid = OAL_FALSE;

    /* 准备PNO扫描参数，准备发起扫描 */
    dmac_scan_prepare_pno_scan_params(&st_scan_req_params, pst_dmac_vap);

    /* 重新发起扫描 */
    dmac_scan_handle_scan_req_entry(pst_mac_device, pst_dmac_vap, &st_scan_req_params);

    /* 重新启动PNO调度扫描定时器 */
    dmac_scan_start_pno_sched_scan_timer((void *)pst_dmac_vap);

    return;
}
#endif



OAL_STATIC oal_uint32  dmac_scan_start_pno_sched_scan_timer(void *p_arg)
{
#if ((_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_TEST_MODE != _PRE_TEST_MODE_UT))
    dmac_vap_stru                           *pst_dmac_vap = (dmac_vap_stru *)p_arg;
    mac_device_stru                         *pst_mac_device;
    mac_pno_sched_scan_mgmt_stru            *pst_pno_mgmt;
    oal_int32                                l_ret = OAL_SUCC;

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{dmac_scan_start_pno_sched_scan_timer:: pst_mac_device is null.}");
        return OAL_FAIL;
    }

    /* 获取pno管理结构体 */
    pst_pno_mgmt = pst_mac_device->pst_pno_sched_scan_mgmt;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 判断pno调度扫描的rtc时钟定时器是否已经创建，如果未创建，则重新创建，否则，直接启动即可 */
    if (OAL_PTR_NULL == pst_pno_mgmt->p_pno_sched_scan_timer)
    {
        pst_pno_mgmt->p_pno_sched_scan_timer = (oal_void *)oal_rtctimer_create(dmac_scan_pno_scan_timeout_fn, p_arg);
        if (OAL_PTR_NULL == pst_pno_mgmt->p_pno_sched_scan_timer)
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                           "{dmac_scan_start_pno_sched_scan_timer:: create pno timer faild.}");
            return OAL_FAIL;
        }
    }

#endif

    /* 参数合法性检查，时间间隔过短，pno调度只执行一次 */
    if (0 == pst_pno_mgmt->st_pno_sched_scan_params.ul_pno_scan_interval / 100)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_start_pno_sched_scan_timer:: pno scan interval[%d] time too short.}",
                         pst_pno_mgmt->st_pno_sched_scan_params.ul_pno_scan_interval);
        return OAL_FAIL;
    }

    /* 启动定时器，上层下发的扫描间隔是毫秒级的，而定时器是100毫秒级的，因此需要除以100 */
    l_ret = oal_rtctimer_start((STIMER_STRU *)pst_pno_mgmt->p_pno_sched_scan_timer, pst_pno_mgmt->st_pno_sched_scan_params.ul_pno_scan_interval / 100);
    if (OAL_SUCC != l_ret)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_start_pno_sched_scan_timer:: start pno timer faild[%d].}",l_ret);
        return OAL_FAIL;
    }

    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                     "{dmac_scan_start_pno_sched_scan_timer:: start pno timer succ, timeout[%d].}",
                     pst_pno_mgmt->st_pno_sched_scan_params.ul_pno_scan_interval / 100);

    /* 当前PNO扫描定时器上层初始化是60s(PNO_SCHED_SCAN_INTERVAL),定时器到期后每次再延长60秒,最长不超过300s */
    pst_pno_mgmt->st_pno_sched_scan_params.ul_pno_scan_interval += (60 * 1000);
    if(pst_pno_mgmt->st_pno_sched_scan_params.ul_pno_scan_interval > (300 * 1000))
    {
        pst_pno_mgmt->st_pno_sched_scan_params.ul_pno_scan_interval = (300 * 1000);
    }

#else
    /* 1151不支持，且不会走到此处，do nothing，主要原因平台并未封装定时器相关接口 */
#endif

    return OAL_SUCC;
}


oal_uint32  dmac_scan_stop_pno_sched_scan_timer(void *p_arg)
{
#if ((_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_TEST_MODE != _PRE_TEST_MODE_UT))
    mac_pno_sched_scan_mgmt_stru    *pst_pno_mgmt;

    pst_pno_mgmt = (mac_pno_sched_scan_mgmt_stru *)p_arg;

    /* 如果定时器不存在，直接返回 */
    if (OAL_PTR_NULL == pst_pno_mgmt->p_pno_sched_scan_timer)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN,
                         "{dmac_scan_stop_pno_sched_scan_timer:: pno sched timer not create yet.}");
        return OAL_SUCC;
    }
    /* 删除定时器 */
    oal_rtctimer_delete((STIMER_STRU *)pst_pno_mgmt->p_pno_sched_scan_timer);
    pst_pno_mgmt->p_pno_sched_scan_timer = OAL_PTR_NULL;
#else
    /* 1151不支持，且不会走到此处，do nothing，主要原因平台并未封装定时器相关接口 */
#endif

    return OAL_SUCC;
}


oal_uint32  dmac_scan_proc_sched_scan_req_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    frw_event_hdr_stru         *pst_event_hdr;
    dmac_vap_stru              *pst_dmac_vap;
    mac_device_stru            *pst_mac_device;
    mac_scan_req_stru           st_scan_req_params;

    /* 参数合法性检查 */
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_proc_sched_scan_req_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件信息 */
    pst_event        = frw_get_event_stru(pst_event_mem);
    pst_event_hdr    = &(pst_event->st_event_hdr);

    /* 获取dmac vap */
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_proc_sched_scan_req_event::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_event_hdr->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_proc_sched_scan_req_event::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 准备pno扫描请求参数 */
    dmac_scan_prepare_pno_scan_params(&st_scan_req_params, pst_dmac_vap);

    /* 设置PNO调度扫描的次数为0 */
    pst_mac_device->pst_pno_sched_scan_mgmt->uc_curr_pno_sched_scan_times = 0;
    pst_mac_device->pst_pno_sched_scan_mgmt->en_is_found_match_ssid = OAL_FALSE;

    /* 调用扫描入口，执行扫描 */
    dmac_scan_handle_scan_req_entry(pst_mac_device, pst_dmac_vap, &st_scan_req_params);

    /* 启动pno调度扫描的rtc时钟定时器，可唤醒深睡的device */
    dmac_scan_start_pno_sched_scan_timer((void *)pst_dmac_vap);

    return OAL_SUCC;
}


oal_uint32  dmac_scan_proc_scan_req_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    frw_event_hdr_stru         *pst_event_hdr;
    dmac_vap_stru              *pst_dmac_vap;
    mac_device_stru            *pst_mac_device;
    mac_scan_req_stru          *pst_h2d_scan_req_params;

    /* 参数合法性检查 */
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_proc_scan_req_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件信息 */
    pst_event        = frw_get_event_stru(pst_event_mem);
    pst_event_hdr    = &(pst_event->st_event_hdr);

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_proc_scan_req_event::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_event_hdr->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_proc_scan_req_event::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取到扫描请求参数 */
    pst_h2d_scan_req_params = (mac_scan_req_stru *)frw_get_event_payload(pst_event_mem);

    /* 保存随机mac扫描开关，用于dmac发起的随机扫描 */
    pst_mac_device->en_is_random_mac_addr_scan = pst_h2d_scan_req_params->en_is_random_mac_addr_scan;

    /* host侧发起的扫描请求的处理 */
    return dmac_scan_handle_scan_req_entry(pst_mac_device, pst_dmac_vap, pst_h2d_scan_req_params);
}

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST


OAL_STATIC oal_uint32 dmac_scan_prepare_obss_scan_params(mac_scan_req_stru  *pst_scan_params,
                                                         dmac_vap_stru      *pst_dmac_vap)
{
    mac_device_stru *pst_mac_device;
    oal_uint8        uc_2g_chan_num      = 0;
    oal_uint8        uc_channel_idx      = 0;
    oal_uint8        uc_low_channel_idx  = 0;
    oal_uint8        uc_high_channel_idx = 0;
    oal_uint8        uc_channel_num      = 0;
    oal_uint8        uc_curr_channel_num = pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number;
    oal_uint8        uc_curr_band        = pst_dmac_vap->st_vap_base_info.st_channel.en_band;

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{dmac_scan_prepare_obss_scan_params::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }

    OAL_MEMZERO(pst_scan_params, OAL_SIZEOF(mac_scan_req_stru));

    /* 1.设置发起扫描的vap id */
    pst_scan_params->uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;

    /* 2.设置初始扫描请求的参数 */
    pst_scan_params->en_bss_type         = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    pst_scan_params->en_scan_type        = WLAN_SCAN_TYPE_ACTIVE;
    pst_scan_params->uc_probe_delay      = 0;
    pst_scan_params->uc_scan_func        = MAC_SCAN_FUNC_BSS;               /* 默认扫描bss */
    pst_scan_params->uc_max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;
    pst_scan_params->uc_max_scan_count_per_channel           = 1;

    /* 根据扫描类型，确定每信道扫描时间 */
    if (WLAN_SCAN_TYPE_ACTIVE == pst_scan_params->en_scan_type)
    {
        pst_scan_params->us_scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
    }
    else
    {
        pst_scan_params->us_scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
    }

    /* OBSS扫描通配ssid */
    pst_scan_params->ast_mac_ssid_set[0].auc_ssid[0] = '\0';
    pst_scan_params->uc_ssid_num = 1;
    /* OBSS扫描设置Source MAC ADDRESS */
    if((pst_mac_device->en_is_random_mac_addr_scan)
       && ((pst_mac_device->auc_mac_oui[0] != 0) || (pst_mac_device->auc_mac_oui[1] != 0) || (pst_mac_device->auc_mac_oui[2] != 0)))
    {
        pst_scan_params->auc_sour_mac_addr[0] = (pst_mac_device->auc_mac_oui[0] & 0xfe);  /*保证是单播mac*/
        pst_scan_params->auc_sour_mac_addr[1] = pst_mac_device->auc_mac_oui[1];
        pst_scan_params->auc_sour_mac_addr[2] = pst_mac_device->auc_mac_oui[2];
        pst_scan_params->auc_sour_mac_addr[3] = oal_gen_random((oal_uint32)OAL_TIME_GET_STAMP_MS(), 1);
        pst_scan_params->auc_sour_mac_addr[4] = oal_gen_random((oal_uint32)OAL_TIME_GET_STAMP_MS(), 1);
        pst_scan_params->auc_sour_mac_addr[5] = oal_gen_random((oal_uint32)OAL_TIME_GET_STAMP_MS(), 1);
        pst_scan_params->en_is_random_mac_addr_scan = OAL_TRUE;
    }
    else
    {
        oal_set_mac_addr(pst_scan_params->auc_sour_mac_addr, mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info));
    }

    /* OBSS扫描扫描只指定1个bssid，为广播地址 */
    oal_set_mac_addr(pst_scan_params->auc_bssid[0], BROADCAST_MACADDR);
    pst_scan_params->uc_bssid_num = 1;

    /* 设置扫描模式为OBSS扫描 */
    pst_scan_params->en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_OBSS;

    /* 准备OBSS扫描的信道 */
    if (WLAN_BAND_2G == uc_curr_band)
    {

        /* 从当前信道左右偏移5个信道，计算OBSS扫描信道
           1) 当前信道idx小于等于5，则从0开始，到idx+5,
           2) 大于5小于8，应该从idx-5到idx+5,
           3) 大于8，则是从idx-5到13 */
        if (uc_curr_channel_num <= 5)
        {
            uc_low_channel_idx = 0;
            uc_high_channel_idx = uc_curr_channel_num + 5;
        }
        else if (5 < uc_curr_channel_num && uc_curr_channel_num <= 8)
        {
            uc_low_channel_idx  = uc_curr_channel_num - 5;
            uc_high_channel_idx = uc_curr_channel_num + 5;
        }
        else if (8 < uc_curr_channel_num && uc_curr_channel_num <= 13)
        {
            uc_low_channel_idx = uc_curr_channel_num - 5;
            uc_high_channel_idx = 13;
        }
        else
        {
            uc_low_channel_idx  = 0;
            uc_high_channel_idx = 0;
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_update_obss_scan_params::2040M,Current channel index is %d.}",
                           uc_curr_channel_num);
        }

        /* 准备2.4G下OBSS扫描信道 */
        for(uc_channel_idx = uc_low_channel_idx; uc_channel_idx <= uc_high_channel_idx; uc_channel_idx++)
        {
            /* 判断信道是不是在管制域内 */
            if (OAL_SUCC == mac_is_channel_idx_valid(WLAN_BAND_2G, uc_channel_idx))
            {
                mac_get_channel_num_from_idx(WLAN_BAND_2G, uc_channel_idx, &uc_channel_num);

                pst_scan_params->ast_channel_list[uc_2g_chan_num].uc_chan_number = uc_channel_num;
                pst_scan_params->ast_channel_list[uc_2g_chan_num].en_band        = WLAN_BAND_2G;
                pst_scan_params->ast_channel_list[uc_2g_chan_num].uc_chan_idx         = uc_channel_idx;
                pst_scan_params->uc_channel_nums++;
                uc_2g_chan_num++;
            }
        }

        /* 更新本次扫描的信道总数 */
        pst_scan_params->uc_channel_nums = uc_2g_chan_num;
    }
#if 0
    else if (WLAN_BAND_5G == uc_curr_band)
    {
        /* 暂时不考虑5G下的obss扫描 */
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_update_obss_scan_params::5G don't do obss scan.}");
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_update_obss_scan_params::error band[%d].}", uc_curr_band);
    }
#endif
    /* 如果当前扫描信道的总数为0，返回错误，不执行扫描请求 */
    if (0 == pst_scan_params->uc_channel_nums)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_update_obss_scan_params::scan total channel num is 0, band[%d]!}", uc_curr_band);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_scan_obss_timeout_fn(void *p_arg)
{
    dmac_vap_stru          *pst_dmac_vap;
    mac_device_stru        *pst_mac_device;
    mac_scan_req_stru       st_scan_req_params;
    oal_uint32              ul_ret;
    oal_uint8               uc_scan_now = OAL_FALSE;

    //OAM_INFO_LOG0(0, OAM_SF_SCAN, "{dmac_scan_obss_timeout_fn::obss timer time out.}");

    pst_dmac_vap = (dmac_vap_stru *)p_arg;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_obss_timeout_fn::pst_dmac_vap null.}");
        return OAL_FAIL;
    }

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{dmac_scan_obss_timeout_fn::pst_mac_device null.}");
        return OAL_FAIL;
    }

    // 残留事件到期
    if (pst_mac_device->st_obss_scan_timer.p_timeout_arg != (oal_void *)pst_dmac_vap)
    {
        return OAL_FAIL;
    }

    /* 如果在obss scan timer启动期间动态修改了sta能力导致sta不支持obss扫描，
     * 则关闭obss scan timer
     * 此处是唯一中止obss扫描的地方!!!
     */
    if (OAL_FALSE == dmac_mgmt_need_obss_scan(&pst_dmac_vap->st_vap_base_info))
    {
        pst_dmac_vap->ul_obss_scan_timer_remain  = 0;
        pst_dmac_vap->uc_obss_scan_timer_started = OAL_FALSE;
        //OAM_INFO_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
        //            "{dmac_scan_obss_timeout_fn::stop obss scan timer}");
        return OAL_SUCC;
    }

    if (0 == pst_dmac_vap->ul_obss_scan_timer_remain)
    {
        uc_scan_now = OAL_TRUE;
    }

    dmac_scan_start_obss_timer(&pst_dmac_vap->st_vap_base_info);

    /* 进入扫描入口，执行obss扫描 */
    if (OAL_TRUE == uc_scan_now)
    {
        /* 准备OBSS扫描参数，准备发起扫描 */
        ul_ret = dmac_scan_prepare_obss_scan_params(&st_scan_req_params, pst_dmac_vap);
        if (OAL_SUCC != ul_ret)
        {
            //OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{dmac_scan_obss_timeout_fn::update scan params error[%d].}", ul_ret);
            return ul_ret;
        }

        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_obss_timeout_fn:: start scan}");
        return dmac_scan_handle_scan_req_entry(pst_mac_device, pst_dmac_vap, &st_scan_req_params);
    }

    return OAL_SUCC;
}


oal_void dmac_scan_start_obss_timer(mac_vap_stru *pst_mac_vap)
{
    dmac_vap_stru                 *pst_dmac_vap;
    mac_device_stru               *pst_mac_device;
    oal_uint32                    ul_new_timer;

    /* 根据发起扫描的vap id获取dmac vap */
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_scan_start_obss_timer:: pst_dmac_vap is NULL.}");
        return;
    }

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                       "{dmac_scan_start_obss_timer::pst_mac_device null.}");
        return;
    }

    /* 重置扫描定时器 */
    if (0 == pst_dmac_vap->ul_obss_scan_timer_remain)
    {
        pst_dmac_vap->ul_obss_scan_timer_remain = 1000*mac_mib_get_BSSWidthTriggerScanInterval(&pst_dmac_vap->st_vap_base_info);
    }

    ul_new_timer = pst_dmac_vap->ul_obss_scan_timer_remain > DMAC_SCAN_MAX_TIMER?
                        DMAC_SCAN_MAX_TIMER:pst_dmac_vap->ul_obss_scan_timer_remain;
    pst_dmac_vap->ul_obss_scan_timer_remain -= ul_new_timer;
    OAM_INFO_LOG2(0, OAM_SF_SCAN, "{dmac_scan_start_obss_timer::remain=%d new_timer=%d}",
                pst_dmac_vap->ul_obss_scan_timer_remain, ul_new_timer);

    FRW_TIMER_CREATE_TIMER(&(pst_mac_device->st_obss_scan_timer),
                           dmac_scan_obss_timeout_fn,
                           ul_new_timer,
                           (void *)pst_dmac_vap,
                           OAL_FALSE,
                           OAM_MODULE_ID_DMAC,
                           pst_mac_device->ul_core_id);
    pst_dmac_vap->uc_obss_scan_timer_started = OAL_TRUE;

    return;
}

oal_void dmac_scan_destroy_obss_timer(dmac_vap_stru *pst_dmac_vap)
{
    mac_vap_stru        *pst_mac_vap;
    mac_device_stru     *pst_mac_device;

    pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (!pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_scan_destroy_obss_timer:null dev ptr, id=%d}", pst_mac_vap->uc_device_id);

        return;
    }

    if ((pst_mac_device->st_obss_scan_timer.en_is_registerd)
        && pst_mac_device->st_obss_scan_timer.p_timeout_arg == (void *)pst_mac_vap) // same as dmac_vap
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_mac_device->st_obss_scan_timer);
        pst_mac_device->st_obss_scan_timer.p_timeout_arg = OAL_PTR_NULL;
        pst_dmac_vap->uc_obss_scan_timer_started = OAL_FALSE;
    }
}

oal_uint32 dmac_trigger_csa_scan(mac_vap_stru      *pst_mac_vap)
{
    oal_uint8               uc_chan_num      = 0;
    oal_uint8               uc_channel_idx      = 0;
    dmac_vap_stru          *pst_dmac_vap;
    mac_device_stru        *pst_mac_device;
    mac_scan_req_stru       st_scan_params;

    if((pst_mac_vap->st_ch_switch_info.st_old_channel.uc_chan_number == pst_mac_vap->st_channel.uc_chan_number))
    {
        dmac_sta_csa_fsm_post_event(pst_mac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OAL_PTR_NULL);
        return OAL_SUCC;
    }

    OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_trigger_csa_scan::update csa scan params.}");

    OAL_MEMZERO(&st_scan_params, OAL_SIZEOF(mac_scan_req_stru));

    /* 1.设置发起扫描的vap id */
    st_scan_params.uc_vap_id = pst_mac_vap->uc_vap_id;

    /* 2.设置初始扫描请求的参数 */
    st_scan_params.en_bss_type         = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    st_scan_params.en_scan_type        = WLAN_SCAN_TYPE_ACTIVE;
    st_scan_params.uc_probe_delay      = 0;
    st_scan_params.uc_scan_func        = MAC_SCAN_FUNC_BSS;               /* 默认扫描bss */
    st_scan_params.uc_max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;
    st_scan_params.uc_max_scan_count_per_channel           = 2;

    /* 根据扫描类型，确定每信道扫描时间 */
    if (WLAN_SCAN_TYPE_ACTIVE == st_scan_params.en_scan_type)
    {
        st_scan_params.us_scan_time = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
    }

    /* CSA扫描通配ssid */
    st_scan_params.ast_mac_ssid_set[0].auc_ssid[0] = '\0';
    st_scan_params.uc_ssid_num = 1;
    /* CSA扫描设置Source MAC ADDRESS */
    oal_set_mac_addr(st_scan_params.auc_sour_mac_addr, mac_mib_get_StationID(pst_mac_vap));

    /* CSA扫描扫描只指定1个bssid，为广播地址 */
    oal_set_mac_addr(st_scan_params.auc_bssid[0], BROADCAST_MACADDR);
    st_scan_params.uc_bssid_num = 1;

    /* 设置扫描模式为CSA扫描 */
    st_scan_params.en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_CSA;

    /* 准备扫描的信道 */
    /* 判断信道是不是在管制域内 */

    if (OAL_SUCC == mac_is_channel_num_valid(pst_mac_vap->st_ch_switch_info.st_old_channel.en_band, pst_mac_vap->st_ch_switch_info.st_old_channel.uc_chan_number))
    {
        mac_get_channel_idx_from_num(pst_mac_vap->st_ch_switch_info.st_old_channel.en_band, pst_mac_vap->st_ch_switch_info.st_old_channel.uc_chan_number, &uc_channel_idx);
        st_scan_params.ast_channel_list[uc_chan_num].uc_chan_number = pst_mac_vap->st_ch_switch_info.st_old_channel.uc_chan_number;
        st_scan_params.ast_channel_list[uc_chan_num].en_band        = pst_mac_vap->st_ch_switch_info.st_old_channel.en_band;
        st_scan_params.ast_channel_list[uc_chan_num].uc_chan_idx         = uc_channel_idx;
        st_scan_params.uc_channel_nums++;
        uc_chan_num++;
    }

    if (OAL_SUCC == mac_is_channel_num_valid(pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_chan_number))
    {
        mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_chan_number, &uc_channel_idx);
        st_scan_params.ast_channel_list[uc_chan_num].uc_chan_number = pst_mac_vap->st_channel.uc_chan_number;
        st_scan_params.ast_channel_list[uc_chan_num].en_band        = pst_mac_vap->st_channel.en_band;
        st_scan_params.ast_channel_list[uc_chan_num].uc_chan_idx         = uc_channel_idx;
        st_scan_params.uc_channel_nums++;
        uc_chan_num++;
    }


    /* 更新本次扫描的信道总数 */
    st_scan_params.uc_channel_nums = uc_chan_num;

    /* 如果当前扫描信道的总数为0，返回错误，不执行扫描请求 */
    if (0 == st_scan_params.uc_channel_nums)
    {
       OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_trigger_csa_scan::scan total channel num is 0.}");
       return OAL_FAIL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_trigger_csa_scan::pst_dmac_vap null.}");
        return OAL_FAIL;
    }

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{dmac_trigger_csa_scan::pst_mac_device null.}");
        return OAL_FAIL;
    }

    dmac_scan_handle_scan_req_entry(pst_mac_device, pst_dmac_vap, &st_scan_params);

    return OAL_SUCC;
}

#endif


OAL_STATIC oal_void  dmac_scan_switch_channel_notify_alg(hal_to_dmac_device_stru  *pst_scan_hal_device,
                                                         dmac_vap_stru     *pst_dmac_vap,
                                                         mac_channel_stru  *pst_channel)
{
    mac_channel_stru                st_channel_tmp;
    hal_to_dmac_device_stru        *pst_original_hal_device;

    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_channel) || (OAL_PTR_NULL == pst_scan_hal_device))
    {
        OAM_ERROR_LOG3(0, OAM_SF_SCAN, "{dmac_scan_switch_channel_notify_alg::pst_dmac_vap[%p], pst_channel[%p].pst_scan_hal_device[%p]}",
                       pst_dmac_vap, pst_channel, pst_scan_hal_device);
        return;
    }

    /* 记录当前vap下的信道信息 */
    st_channel_tmp = pst_dmac_vap->st_vap_base_info.st_channel;

    /* 不同的hal device扫描,通知算法不同的扫描hal device */
    pst_original_hal_device   = pst_dmac_vap->pst_hal_device;
    pst_dmac_vap->pst_hal_device = pst_scan_hal_device;

    /* 记录要切换到信道的频段，切换信道 */
    pst_dmac_vap->st_vap_base_info.st_channel.en_band        = pst_channel->en_band;
    pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number = pst_channel->uc_chan_number;
    pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_idx         = pst_channel->uc_chan_idx;
    pst_dmac_vap->st_vap_base_info.st_channel.en_bandwidth = WLAN_BAND_WIDTH_20M;

    /* 刷新发送功率 */
    dmac_pow_set_vap_tx_power(&pst_dmac_vap->st_vap_base_info, HAL_POW_SET_TYPE_INIT);

    /* 通知算法 */
    dmac_alg_cfg_channel_notify(&pst_dmac_vap->st_vap_base_info, CH_BW_CHG_TYPE_SCAN);
    dmac_alg_cfg_bandwidth_notify(&pst_dmac_vap->st_vap_base_info, CH_BW_CHG_TYPE_SCAN);

    /* 通知算法后，恢复vap原有信道信息 */
    pst_dmac_vap->st_vap_base_info.st_channel = st_channel_tmp;
    pst_dmac_vap->pst_hal_device = pst_original_hal_device;

    return;
}


oal_uint32  dmac_switch_channel_off(
                mac_device_stru     *pst_mac_device,
                mac_vap_stru        *pst_mac_vap,
                mac_channel_stru    *pst_dst_chl,
                oal_uint16           us_protect_time)
{
    mac_fcs_mgr_stru               *pst_fcs_mgr;
    mac_fcs_cfg_stru               *pst_fcs_cfg;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_switch_channel_off::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 记录切离的信道，供扫描完后切回 */
    pst_hal_device->st_hal_scan_params.st_home_channel = pst_mac_vap->st_channel;

    pst_fcs_mgr = dmac_fcs_get_mgr_stru(pst_mac_device);
    pst_fcs_cfg = MAC_DEV_GET_FCS_CFG(pst_mac_device);

    OAL_MEMZERO(pst_fcs_cfg, OAL_SIZEOF(mac_fcs_cfg_stru));

    pst_fcs_cfg->st_src_chl = pst_mac_vap->st_channel;
    pst_fcs_cfg->st_dst_chl = *pst_dst_chl;

    pst_fcs_cfg->pst_hal_device = pst_hal_device;

    pst_fcs_cfg->pst_src_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_vap);

    dmac_fcs_prepare_one_packet_cfg(pst_mac_vap, &(pst_fcs_cfg->st_one_packet_cfg), us_protect_time);

    /* 调用FCS切信道接口 保存当前硬件队列的帧到自己的虚假队列,后续DBAC不需要切换队列 */
    dmac_fcs_start(pst_fcs_mgr, pst_fcs_cfg, 0);
    mac_fcs_release(pst_fcs_mgr);

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_switch_channel_off::switch src channel[%d] to dst channel[%d].}",
                 pst_mac_vap->st_channel.uc_chan_number, pst_fcs_cfg->st_dst_chl.uc_chan_number);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_DBAC

oal_void dmac_dbac_switch_channel_off(mac_device_stru  *pst_mac_device,
                                                mac_vap_stru   *pst_mac_vap1,
                                                mac_vap_stru   *pst_mac_vap2,
                                                mac_channel_stru  *pst_dst,
                                                oal_uint16  us_protect_time)
{
    mac_fcs_mgr_stru               *pst_fcs_mgr;
    mac_fcs_cfg_stru               *pst_fcs_cfg;
    hal_to_dmac_device_stru        *pst_hal_device;
    hal_to_dmac_device_stru        *pst_hal_device2;
    mac_vap_stru                   *pst_current_chan_vap;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap1);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap1->uc_vap_id, OAM_SF_DBAC, "{dmac_dbac_switch_channel_off::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap1->uc_vap_id);
        return;
    }

    pst_hal_device2 = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap2);
    if (OAL_PTR_NULL == pst_hal_device2)
    {
        OAM_ERROR_LOG1(pst_mac_vap2->uc_vap_id, OAM_SF_DBAC, "{dmac_dbac_switch_channel_off::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap2->uc_vap_id);
        return;
    }

    if (pst_hal_device != pst_hal_device2)
    {
        OAM_ERROR_LOG2(0, OAM_SF_DBAC, "{dmac_dbac_switch_channel_off::diff pst_hal_device id [%d] [%d].}", pst_hal_device->uc_device_id, pst_hal_device2->uc_device_id);
        return;
    }

    if (pst_hal_device->uc_current_chan_number == pst_mac_vap1->st_channel.uc_chan_number)
    {
        pst_current_chan_vap = pst_mac_vap1;
    }
    else
    {
        pst_current_chan_vap = pst_mac_vap2;
    }

    /* 暂停DBAC切信道 */
    dmac_alg_dbac_pause(pst_hal_device);

    dmac_vap_pause_tx(pst_mac_vap1);
    dmac_vap_pause_tx(pst_mac_vap2);

    pst_fcs_mgr = dmac_fcs_get_mgr_stru(pst_mac_device);
    pst_fcs_cfg = MAC_DEV_GET_FCS_CFG(pst_mac_device);
    OAL_MEMZERO(pst_fcs_cfg, OAL_SIZEOF(mac_fcs_cfg_stru));

    pst_fcs_cfg->st_dst_chl     = *pst_dst;
    pst_fcs_cfg->st_src_chl = pst_current_chan_vap->st_channel;
    pst_fcs_cfg->pst_hal_device = pst_hal_device;

    pst_fcs_cfg->pst_src_fake_queue = DMAC_VAP_GET_FAKEQ(pst_current_chan_vap);

    dmac_fcs_prepare_one_packet_cfg(pst_current_chan_vap, &(pst_fcs_cfg->st_one_packet_cfg), us_protect_time);

    OAM_WARNING_LOG2(pst_current_chan_vap->uc_vap_id, OAM_SF_DBAC, "dmac_dbac_switch_channel_off::switch chan off when dbac running. curr chan num:%d, fake_q_vap_id:%d",
                    pst_current_chan_vap->st_channel.uc_chan_number, pst_current_chan_vap->uc_vap_id);


    if (pst_hal_device->uc_current_chan_number != pst_current_chan_vap->st_channel.uc_chan_number)
    {
        OAM_WARNING_LOG2(0, OAM_SF_DBAC, "dmac_dbac_switch_channel_off::switch chan off when dbac running. hal chan num:%d, curr vap chan num:%d. not same,do not send protect frame",
                        pst_hal_device->uc_current_chan_number,
                        pst_current_chan_vap->st_channel.uc_chan_number);

        pst_fcs_cfg->st_one_packet_cfg.en_protect_type = HAL_FCS_PROTECT_TYPE_NONE;
    }

    dmac_fcs_start(pst_fcs_mgr, pst_fcs_cfg, 0);
    mac_fcs_release(pst_fcs_mgr);
}

OAL_STATIC oal_void dmac_scan_dbac_switch_channel_off(mac_device_stru  *pst_mac_device,
                                                      mac_vap_stru *pst_mac_vap,
                                                      mac_vap_stru *pst_mac_vap2)
{
    oal_uint8                       uc_scan_chan_idx;
    oal_uint16                      us_protect_time;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    uc_scan_chan_idx = GET_HAL_DEV_CURRENT_SCAN_IDX(pst_hal_device);
    us_protect_time = pst_mac_device->st_scan_params.us_scan_time;

    dmac_dbac_switch_channel_off(pst_mac_device, pst_mac_vap,pst_mac_vap2,
                                 &(pst_mac_device->st_scan_params.ast_channel_list[uc_scan_chan_idx]),
                                 us_protect_time);
}

#endif

OAL_STATIC oal_uint32 dmac_the_same_channel_switch_channel_off(
                mac_device_stru     *pst_mac_device,
                mac_vap_stru        *pst_mac_vap1,
                mac_vap_stru        *pst_mac_vap2,
                mac_channel_stru    *pst_dst_chl,
                oal_uint16           us_protect_time)
{
    mac_vap_stru                   *pst_vap_sta;
    mac_fcs_mgr_stru               *pst_fcs_mgr;
    mac_fcs_cfg_stru               *pst_fcs_cfg;
    hal_to_dmac_device_stru        *pst_hal_device;
    hal_to_dmac_device_stru        *pst_hal_device2;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap1);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap1->uc_vap_id, OAM_SF_SCAN, "{dmac_the_same_channel_switch_channel_off::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",
                            pst_mac_vap1->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device2 = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap2);
    if (OAL_PTR_NULL == pst_hal_device2)
    {
        OAM_ERROR_LOG1(pst_mac_vap1->uc_vap_id, OAM_SF_SCAN, "{dmac_the_same_channel_switch_channel_off::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",
                            pst_mac_vap2->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_hal_device != pst_hal_device2)
    {
        OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{dmac_the_same_channel_switch_channel_off::diff pst_hal_device id [%d] [%d].}",
                            pst_hal_device->uc_device_id, pst_hal_device2->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_vap1->st_channel.uc_chan_number != pst_mac_vap2->st_channel.uc_chan_number)
    {
        OAM_ERROR_LOG2(0, OAM_SF_SCAN, "dmac_the_same_channel_switch_channel_off::vap1 channel num[%d] != vap2 channel num[%d].",pst_mac_vap1->st_channel.uc_chan_number,
                    pst_mac_vap2->st_channel.uc_chan_number);
        return OAL_FAIL;
    }

    /* 记录切离时最大带宽的信道，供同信道共存扫描完后切回 */
    if (pst_mac_vap1->st_channel.en_bandwidth >= pst_mac_vap2->st_channel.en_bandwidth)
    {
        pst_hal_device->st_hal_scan_params.st_home_channel = pst_mac_vap1->st_channel;
    }
    else
    {
        pst_hal_device->st_hal_scan_params.st_home_channel = pst_mac_vap2->st_channel;
    }

    /* 暂停两个VAP的发送 */
    dmac_vap_pause_tx(pst_mac_vap1);
    dmac_vap_pause_tx(pst_mac_vap2);

    pst_fcs_mgr = dmac_fcs_get_mgr_stru(pst_mac_device);
    pst_fcs_cfg = MAC_DEV_GET_FCS_CFG(pst_mac_device);
    OAL_MEMZERO(pst_fcs_cfg, OAL_SIZEOF(mac_fcs_cfg_stru));

    pst_fcs_cfg->st_dst_chl = *pst_dst_chl;
    pst_fcs_cfg->pst_hal_device = pst_hal_device;

    pst_fcs_cfg->pst_src_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_vap1);

    OAM_WARNING_LOG4(0, OAM_SF_SCAN, "{dmac_the_same_channel_switch_channel::hal device[%d], curr hal chan[%d], dst channel[%d], fakeq vap id[%d]}",
                        pst_hal_device->uc_device_id,pst_hal_device->uc_current_chan_number,
                        pst_fcs_cfg->st_dst_chl.uc_chan_number, pst_mac_vap1->uc_vap_id);

    /* 同频双STA模式，需要起两次one packet */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap1->en_vap_mode && WLAN_VAP_MODE_BSS_STA == pst_mac_vap2->en_vap_mode)
    {
        /* 准备VAP1的fcs参数 */
        pst_fcs_cfg->st_src_chl = pst_mac_vap1->st_channel;
        dmac_fcs_prepare_one_packet_cfg(pst_mac_vap1, &(pst_fcs_cfg->st_one_packet_cfg), us_protect_time);

        /* 准备VAP2的fcs参数 */
        pst_fcs_cfg->st_src_chl2 = pst_mac_vap2->st_channel;
        dmac_fcs_prepare_one_packet_cfg(pst_mac_vap2, &(pst_fcs_cfg->st_one_packet_cfg2), us_protect_time);
        pst_fcs_cfg->st_one_packet_cfg2.us_timeout = MAC_FCS_DEFAULT_PROTECT_TIME_OUT2;     /* 减小第二次one packet的保护时长，从而减少总时长 */
        pst_fcs_cfg->pst_hal_device = pst_hal_device;

        dmac_fcs_start_enhanced(pst_fcs_mgr, pst_fcs_cfg);
        mac_fcs_release(pst_fcs_mgr);
    }
    /* 同频STA+GO模式，只需要STA起一次one packet */
    else
    {
        if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap1->en_vap_mode)
        {
            pst_vap_sta = pst_mac_vap1;
        }
        else
        {
            pst_vap_sta = pst_mac_vap2;
        }

        pst_fcs_cfg->st_src_chl = pst_vap_sta->st_channel;
        dmac_fcs_prepare_one_packet_cfg(pst_vap_sta, &(pst_fcs_cfg->st_one_packet_cfg), us_protect_time);

        /* 调用FCS切信道接口 保存当前硬件队列的帧到扫描虚假队列 */
        dmac_fcs_start(pst_fcs_mgr, pst_fcs_cfg, 0);
        mac_fcs_release(pst_fcs_mgr);
    }
    return OAL_SUCC;
}
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)

OAL_STATIC hal_to_dmac_device_stru *dmac_find_the_pause_scan_hal_dev(hal_to_dmac_device_stru *pst_hal_device1, hal_to_dmac_device_stru *pst_hal_device2)
{
    if (oal_bit_get_bit_one_byte(pst_hal_device1->st_hal_scan_params.uc_scan_pause_bitmap, HAL_SCAN_PASUE_TYPE_CHAN_CONFLICT))
    {
        return  pst_hal_device1;
    }
    else if (oal_bit_get_bit_one_byte(pst_hal_device2->st_hal_scan_params.uc_scan_pause_bitmap, HAL_SCAN_PASUE_TYPE_CHAN_CONFLICT))
    {
        return  pst_hal_device2;
    }

    return OAL_PTR_NULL;
}


oal_bool_enum_uint8 dmac_check_can_start_scan(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint8                uc_scan_channel_idx;
    mac_channel_stru        *pst_scanning_channel;
    mac_channel_stru        *pst_another_scan_channel;
    hal_to_dmac_device_stru *pst_another_hal_device;
    hal_to_dmac_device_stru *pst_pause_hal_device;
    dmac_device_stru        *pst_dmac_device;
    oal_bool_enum_uint8      en_can_scan = OAL_TRUE;

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_check_can_start_scan::dmac_res_get_mac_dev[%d] is null.}", pst_mac_device->uc_device_id);
        return en_can_scan;
    }

    /* 非并发扫描不需要检查继续扫描 */
    if (OAL_FALSE == pst_dmac_device->en_is_fast_scan)
    {
        return en_can_scan;
    }

    uc_scan_channel_idx  = GET_HAL_DEV_CURRENT_SCAN_IDX(pst_hal_device);
    pst_scanning_channel = &(pst_mac_device->st_scan_params.ast_channel_list[uc_scan_channel_idx]);

    pst_another_hal_device = dmac_device_get_another_h2d_dev(pst_dmac_device, pst_hal_device);
    if (OAL_PTR_NULL == pst_another_hal_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_check_can_start_scan::ori hal dev[%d]pst_another_hal_device is null.}", pst_hal_device->uc_device_id);
        return en_can_scan;
    }
    uc_scan_channel_idx = GET_HAL_DEV_CURRENT_SCAN_IDX(pst_another_hal_device);

    /* 另一路已经完成不需要再去check直接扫描 */
    if (uc_scan_channel_idx >= pst_another_hal_device->st_hal_scan_params.uc_channel_nums)
    {
        return en_can_scan;
    }

    pst_another_scan_channel = &(pst_mac_device->st_scan_params.ast_channel_list[uc_scan_channel_idx]);

    /* 允许同时扫描时,检查是否有一路被pause,有就要恢复 */
    if (OAL_TRUE == dmac_dbdc_channel_check(pst_scanning_channel, pst_another_scan_channel))
    {
        pst_pause_hal_device = dmac_find_the_pause_scan_hal_dev(pst_hal_device, pst_another_hal_device);
        if (pst_pause_hal_device)
        {
            hal_device_handle_event(pst_pause_hal_device, HAL_DEVICE_EVENT_SCAN_RESUME_FROM_CHAN_CONFLICT, 0, OAL_PTR_NULL);
        }
        return en_can_scan;
    }

    /* 正在扫描的5G不可以继续扫,等待2.4G冲突信道扫描结束后继续 */
    if (WLAN_BAND_5G == pst_scanning_channel->en_band)
    {
        pst_pause_hal_device = pst_hal_device;
        en_can_scan = OAL_FALSE;
    }
    else
    {
        pst_pause_hal_device = pst_another_hal_device;
        en_can_scan = OAL_TRUE;
    }

    /* 暂停需要暂停扫描的dev */
    if (!oal_bit_get_bit_one_byte(pst_pause_hal_device->st_hal_scan_params.uc_scan_pause_bitmap, HAL_SCAN_PASUE_TYPE_CHAN_CONFLICT))
    {
        hal_device_handle_event(pst_pause_hal_device, HAL_DEVICE_EVENT_SCAN_PAUSE_FROM_CHAN_CONFLICT, 0, OAL_PTR_NULL);
    }

    return en_can_scan;
}
#endif

oal_void dmac_scan_do_switch_channel_off(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap)
{
    mac_channel_stru            *pst_next_scan_channel;
    oal_uint8                    uc_scan_channel_idx;
    oal_uint8                    uc_up_vap_num;
    oal_uint8                    uc_vap_idx;
    oal_uint8                    auc_mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE];
    mac_vap_stru                *pst_mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {OAL_PTR_NULL};

    /* 此路扫描完成 */
    if (MAC_SCAN_STATE_IDLE == pst_hal_device->st_hal_scan_params.en_curr_scan_state)
    {
        OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "dmac_scan_do_switch_channel_off::but this hal device[%d] is scan channel[%d]complete",
                            pst_hal_device->uc_device_id, pst_hal_device->st_hal_scan_params.uc_channel_nums);
        return;
    }

    uc_scan_channel_idx = GET_HAL_DEV_CURRENT_SCAN_IDX(pst_hal_device);
    if ((uc_scan_channel_idx > pst_mac_device->st_scan_params.uc_channel_nums) ||
            (pst_hal_device->st_hal_scan_params.uc_scan_chan_idx > pst_hal_device->st_hal_scan_params.uc_channel_nums))
    {
        OAM_ERROR_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "dmac_scan_do_switch_channel_off::hal device id[%d]scan[%d]channel,total[%d]channel,now scan channel idx[%d],",
                    pst_hal_device->uc_device_id, uc_scan_channel_idx, pst_hal_device->st_hal_scan_params.uc_channel_nums,
                    pst_hal_device->st_hal_scan_params.uc_channel_nums);
    }

    pst_next_scan_channel = &(pst_mac_device->st_scan_params.ast_channel_list[uc_scan_channel_idx]);
    //if (pst_hal_device->st_hal_scan_params.uc_channel_nums <= DMAC_SCAN_CHANENL_NUMS_TO_PRINT_SWITCH_INFO)
    {
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "dmac_scan_do_switch_channel_off::hal device[%d]switch channel to %d",
                                                    pst_hal_device->uc_device_id, pst_next_scan_channel->uc_chan_number);
    }

    dmac_scan_switch_channel_notify_alg(pst_hal_device, pst_dmac_vap, pst_next_scan_channel);

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    dmac_scan_check_2g_scan_results(pst_mac_device, pst_hal_device, &(pst_dmac_vap->st_vap_base_info), pst_next_scan_channel->en_band);
    pst_hal_device->st_hal_scan_params.uc_last_channel_band = pst_next_scan_channel->en_band;
#endif

    /* 非工作状态可以直接切离,不需要保护 */
    if (GET_HAL_DEVICE_STATE(pst_hal_device) != HAL_DEVICE_WORK_STATE)
    {
        hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF, 0, OAL_PTR_NULL);
        dmac_mgmt_switch_channel(pst_hal_device, pst_next_scan_channel, OAL_TRUE);//非工作模式可以清硬件队列，切换信道需要清fifo，传入TRUE
        return;
    }

    uc_up_vap_num = hal_device_find_all_up_vap(pst_hal_device, auc_mac_vap_id);
    for (uc_vap_idx = 0; uc_vap_idx < uc_up_vap_num; uc_vap_idx++)
    {
        pst_mac_vap[uc_vap_idx]  = (mac_vap_stru *)mac_res_get_mac_vap(auc_mac_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_mac_vap[uc_vap_idx])
        {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "dmac_scan_do_switch_channel_off::pst_mac_vap[%d] IS NULL.", auc_mac_vap_id[uc_vap_idx]);
            return;
        }
    }

    hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_OFF, 0, OAL_PTR_NULL);

    if (2 == uc_up_vap_num)
    {
#ifdef _PRE_WLAN_FEATURE_DBAC
        if (mac_is_dbac_running(pst_mac_device))
        {
            dmac_scan_dbac_switch_channel_off(pst_mac_device, pst_mac_vap[0], pst_mac_vap[1]);
        }
        else
#endif
        {
            /* 考虑和下面的归一 */
            dmac_the_same_channel_switch_channel_off(pst_mac_device, pst_mac_vap[0], pst_mac_vap[1], pst_next_scan_channel, pst_hal_device->st_hal_scan_params.us_scan_time);
        }
    }
    else
    {
        /* work状态下也有可能是0个up vap,p2p go 扫描完成状态 */
        if (uc_up_vap_num != 0)
        {
            for (uc_vap_idx = 0; uc_vap_idx < uc_up_vap_num; uc_vap_idx++)
            {
                dmac_vap_pause_tx(pst_mac_vap[uc_vap_idx]);
                dmac_switch_channel_off(pst_mac_device, pst_mac_vap[uc_vap_idx], pst_next_scan_channel, pst_hal_device->st_hal_scan_params.us_scan_time);
            }
        }
        else
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "dmac_scan_do_switch_channel_off::vap is joining can not scan!!!");
            dmac_mgmt_switch_channel(pst_hal_device, pst_next_scan_channel, OAL_TRUE);//非工作模式可以清硬件队列，切换信道需要清fifo，传入TRUE
        }
    }
}

oal_void dmac_scan_one_channel_start(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_is_scan_start)
{
    dmac_vap_stru      *pst_dmac_vap;
    mac_device_stru    *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    pst_dmac_vap   = mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_hal_device->uc_device_id, OAM_SF_SCAN, "{dmac_scan_one_channel_start::pst_dmac_vap null.}");
        return;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    /* 扫描条件检查,不符合条件不可以扫 */
    if (OAL_FALSE == dmac_check_can_start_scan(pst_mac_device, pst_hal_device))
    {
        return;
    }
#endif

    /* 切信道:包括直接切离以及保护切离 */
    dmac_scan_do_switch_channel_off(pst_mac_device, pst_hal_device, pst_dmac_vap);

    /* 扫描开始以及从home channel切走时需要配置随机mac地址 */
    if (OAL_TRUE == en_is_scan_start)
    {
        dmac_scan_set_vap_mac_addr_by_scan_state(pst_mac_device, pst_hal_device, OAL_TRUE);
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 处于ps状态下，扫描等到bt的ps结束后来执行，置scan begin状态给btcoex */
    if(HAL_BTCOEX_SW_POWSAVE_WORK == GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device))
    {
        GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device) = HAL_BTCOEX_SW_POWSAVE_SCAN_BEGIN;
    }
    else /* 在begin之前，switch_channel_off会将hal device切换到scan状态 */
#endif
    {
        dmac_scan_begin(pst_mac_device, pst_hal_device);
    }
}

oal_void  dmac_scan_switch_channel_off(mac_device_stru *pst_mac_device)
{
    wlan_scan_mode_enum_uint8       en_scan_mode;
    dmac_vap_stru                  *pst_dmac_vap;   /* 发起扫描的VAP */
    oal_uint8                       uc_device_max;
    oal_uint8                       uc_idx;
    hal_to_dmac_device_stru        *pst_hal_device;

    en_scan_mode = pst_mac_device->st_scan_params.en_scan_mode;

    if (en_scan_mode >= WLAN_SCAN_MODE_BUTT)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_switch_channel_off::scan mode[%d] is invalid.}", en_scan_mode);
        return;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_switch_channel_off::pst_dmac_vap null.}");
        return;
    }

    /* 前景扫描或者PNO发起的扫描(注: PNO只在设备未关联的状态下才发起扫描) 直接切信道 */
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (!pst_mac_device->st_scan_params.en_need_switch_back_home_channel && ((WLAN_SCAN_MODE_FOREGROUND == en_scan_mode) || (WLAN_SCAN_MODE_BACKGROUND_PNO == en_scan_mode)))
#else
    if ((WLAN_SCAN_MODE_FOREGROUND == en_scan_mode) || (WLAN_SCAN_MODE_BACKGROUND_PNO == en_scan_mode))
#endif
    {
    #ifdef _PRE_WLAN_FEATURE_PROXYSTA
        if (mac_is_proxysta_enabled(pst_mac_device) && mac_vap_is_vsta(&pst_dmac_vap->st_vap_base_info))
        {
            /* proxysta不切信道 */
            OAM_INFO_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_switch_channel_off::proxysta DO NOT switch channnel.}");
            return;
        }
    #endif
    }

    /* HAL接口获取支持device个数 */
    hal_chip_get_device_num(pst_mac_device->uc_chip_id, &uc_device_max);

    for (uc_idx = 0; uc_idx < uc_device_max; uc_idx++)
    {
        pst_hal_device = dmac_scan_find_hal_device(pst_mac_device, pst_dmac_vap, uc_idx);
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        /* 非关联状态下的扫描，保证1M dbb scaling与11M一致 */
        if (GET_HAL_DEVICE_STATE(pst_hal_device) != HAL_DEVICE_WORK_STATE)
        {
        #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
            dmac_config_update_dsss_scaling_reg(pst_hal_device, HAL_ALG_USER_DISTANCE_FAR);
        #endif  /* _PRE_PLAT_FEATURE_CUSTOMIZE */
        }

        dmac_scan_one_channel_start(pst_hal_device, OAL_TRUE);
    }
}

oal_bool_enum_uint8  dmac_scan_switch_channel_back(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device)
{
    hal_scan_params_stru           *pst_hal_scan_status;

    pst_hal_scan_status = &(pst_hal_device->st_hal_scan_params);
#ifdef _PRE_WLAN_FEATURE_DBAC
    if (mac_is_dbac_running(pst_mac_device) && dmac_alg_dbac_is_pause(pst_hal_device))
    {
        if (OAL_FALSE == pst_hal_scan_status->en_working_in_home_chan)
        {
            /* clear fifo when dbac resume*/
            hal_disable_machw_phy_and_pa(pst_hal_device);
            hal_clear_tx_hw_queue(pst_hal_device);
            hal_recover_machw_phy_and_pa(pst_hal_device);
            dmac_clear_tx_queue(pst_hal_device);
        }
        /* dbac场景只需恢复dbac，由dbac自行切到工作信道 */
        dmac_alg_dbac_resume(pst_hal_device, OAL_TRUE);
        return OAL_FALSE;
    }
#endif

    if (OAL_FALSE == pst_hal_scan_status->en_working_in_home_chan)
    {
        /* 切回工作VAP所在的信道 */
        OAM_WARNING_LOG3(0, OAM_SF_SCAN, "{dmac_scan_switch_channel_back::switch home channel[%d], band[%d], bw[%d]}",
                      pst_hal_scan_status->st_home_channel.uc_chan_number,
                      pst_hal_scan_status->st_home_channel.en_band,
                      pst_hal_scan_status->st_home_channel.en_bandwidth);

        /* 切换信道需要清fifo，传入TRUE */
        dmac_mgmt_switch_channel(pst_hal_device, &(pst_hal_scan_status->st_home_channel), OAL_TRUE);

        /* 恢复home信道上被暂停的发送,包括虚假队列包的搬移 */
        dmac_vap_resume_tx_by_chl(pst_mac_device, pst_hal_device, &(pst_hal_scan_status->st_home_channel));

        return OAL_TRUE;
    }
    return OAL_FALSE;
}

OAL_STATIC oal_bool_enum_uint8  dmac_scan_need_switch_home_channel(hal_to_dmac_device_stru  *pst_hal_device)
{
    oal_uint8                    uc_reamin_chans;
    hal_scan_params_stru        *pst_scan_params;

    pst_scan_params = &(pst_hal_device->st_hal_scan_params);

    /* 背景扫描需要切回工作信道 */
    /* en_need_switch_back_home_channel 必须最先判断 */
    if (OAL_TRUE == pst_scan_params->en_need_switch_back_home_channel)
    {
        if (0 == pst_scan_params->uc_scan_channel_interval)
        {
            OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_scan_need_switch_home_channel::scan_channel_interval is 0, set default value 6!}");
            pst_scan_params->uc_scan_channel_interval = MAC_SCAN_CHANNEL_INTERVAL_DEFAULT;
        }

        if (pst_scan_params->uc_channel_nums < (pst_scan_params->uc_scan_chan_idx + 1))
        {
            OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{dmac_scan_need_switch_home_channel::scan channel nums[%d] < scan idx[%d]!!!}",
                            pst_scan_params->uc_channel_nums, pst_scan_params->uc_scan_chan_idx);
            return OAL_FALSE;
        }

        uc_reamin_chans = pst_scan_params->uc_channel_nums - (pst_scan_params->uc_scan_chan_idx + 1);

        /* 剩余的信道数 >= uc_scan_channel_interval / 2 */
        if (uc_reamin_chans >= (pst_scan_params->uc_scan_channel_interval >> 1))
        {
            return (0 == pst_scan_params->uc_scan_chan_idx % pst_scan_params->uc_scan_channel_interval);
        }
    }

    return OAL_FALSE;
}
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)

OAL_STATIC oal_uint32  dmac_scan_check_2g_scan_results(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device, mac_vap_stru *pst_vap, wlan_channel_band_enum_uint8 en_next_band)
{
    hal_scan_params_stru          *pst_hal_scan_params = &(pst_hal_device->st_hal_scan_params);

    if (((WLAN_SCAN_MODE_FOREGROUND == pst_mac_device->st_scan_params.en_scan_mode)
         || (WLAN_SCAN_MODE_BACKGROUND_STA == pst_mac_device->st_scan_params.en_scan_mode))
         && (OAL_TRUE != pst_mac_device->st_scan_params.bit_is_p2p0_scan))
    {
        if (WLAN_BAND_2G == pst_hal_scan_params->uc_last_channel_band)
        {
            if (pst_hal_scan_params->en_scan_curr_chan_find_bss_flag == OAL_TRUE)
            {
                pst_hal_scan_params->uc_scan_ap_num_in_2p4++;
                pst_hal_scan_params->en_scan_curr_chan_find_bss_flag = OAL_FALSE;
            }

            if ((WLAN_BAND_5G == en_next_band)
               ||(MAC_SCAN_STATE_IDLE == pst_hal_scan_params->en_curr_scan_state))
            {
                if ((pst_hal_scan_params->uc_scan_ap_num_in_2p4 <= 2) || ((pst_mac_device->uc_scan_count % 30) == 0))
                {
                    OAM_WARNING_LOG2(0, OAM_SF_SCAN, "{dmac_scan_check_2g_scan_results::2.4G scan ap channel num = %d, scan_count = %d.}",
                                  pst_hal_scan_params->uc_scan_ap_num_in_2p4,
                                  pst_mac_device->uc_scan_count);
#ifdef _PRE_WLAN_DFT_STAT
                    dmac_dft_report_all_ota_state(pst_vap);
#endif
                }
            }
        }
    }

    return OAL_SUCC;
}
#endif /* _PRE_PRODUCT_ID_HI110X_DEV */


OAL_STATIC oal_void dmac_scan_update_dfs_channel_scan_param(mac_device_stru     *pst_mac_device,
                                                            hal_to_dmac_device_stru *pst_hal_device,
                                                            mac_channel_stru    *pst_mac_channel,
                                                            oal_uint16          *pus_scan_time,
                                                            oal_bool_enum_uint8 *pen_send_probe_req)
{
    mac_vap_stru           *pst_mac_vap;
    hal_scan_params_stru   *pst_scan_params = &(pst_hal_device->st_hal_scan_params);

    /* 非雷达信道，需要发送probe req，扫描时间从扫描参数中获取 */
    if (OAL_FALSE == mac_is_dfs_channel(pst_mac_channel->en_band, pst_mac_channel->uc_chan_number))
    {
        *pen_send_probe_req = OAL_TRUE;
        *pus_scan_time      = pst_scan_params->us_scan_time;
        return;
    }

    /* 如果当前为关联状态，且关联AP 信道和本次扫描信道相同，则认为该信道无雷达，直接发送probe req 发起扫描 */
    pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_device->st_scan_params.uc_vap_id, OAM_SF_SCAN,
                        "{dmac_scan_update_dfs_channel_scan_param::get vap [%d] fail.}",
                        pst_mac_device->st_scan_params.uc_vap_id);
        *pen_send_probe_req = OAL_FALSE;
        *pus_scan_time      = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
        return;
    }

    if (IS_AP(pst_mac_vap))
    {
        *pen_send_probe_req = OAL_FALSE;
        *pus_scan_time      = pst_scan_params->us_scan_time;
        return;
    }

    if ((MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state || MAC_VAP_STATE_PAUSE == pst_mac_vap->en_vap_state)
        && (pst_mac_channel->uc_chan_number == pst_mac_vap->st_channel.uc_chan_number))
    {
        *pen_send_probe_req = OAL_TRUE;
        *pus_scan_time      = pst_scan_params->us_scan_time;
        return;
    }

    /* 雷达信道第一次扫描，不发送probe req ，且在该信道暂停60ms
     * 雷达信道第二次扫描，如果在第一次扫描时候没有发现有AP，
     *                     则退出本信道扫描
     *                     如果在第一次扫描时候发现有AP，
     *                     则发送probe req ，且在该信道暂停20ms
     */
    if (pst_scan_params->uc_curr_channel_scan_count == 0)
    {
        /* 雷达信道第一次扫描，不发送probe req ，在该信道监听60ms */
        *pen_send_probe_req = OAL_FALSE;
        *pus_scan_time      = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
    }
    else
    {
        if (pst_scan_params->en_scan_curr_chan_find_bss_flag == OAL_TRUE)
        {
            /* 雷达信道第二次扫描，且第一次扫描时有发现AP,
             * 设置需要发送probe req，扫描时间从扫描参数获取
             */
            *pen_send_probe_req = OAL_TRUE;
            *pus_scan_time      = pst_scan_params->us_scan_time;
        }
        else
        {
            /* 雷达信道第二次扫描，且第一次扫描时没有发现AP,
             * 设置超时定时器为0
             */
            *pen_send_probe_req = OAL_FALSE;
            *pus_scan_time      = 0;
        }
    }
    return;
}

oal_void dmac_scan_handle_switch_channel_back(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device, hal_scan_params_stru *pst_hal_scan_params)
{
    oal_uint32                  ul_ret;
    oal_bool_enum_uint8         en_switched;
    oal_uint8                   uc_mac_vap_id;
    dmac_vap_stru              *pst_dmac_home_vap;

    en_switched = dmac_scan_switch_channel_back(pst_mac_device, pst_hal_device);
    if (OAL_TRUE == en_switched)
    {
        /* 对于切回home信道的场景需要获取home信道vap通知算法 */
        ul_ret = hal_device_find_one_up_vap(pst_hal_device, &uc_mac_vap_id);
        if (OAL_SUCC == ul_ret)
        {
            pst_dmac_home_vap  = (dmac_vap_stru *)mac_res_get_dmac_vap(uc_mac_vap_id);
            if (pst_dmac_home_vap != OAL_PTR_NULL)
            {
                if (pst_hal_scan_params->st_home_channel.uc_chan_number == pst_dmac_home_vap->st_vap_base_info.st_channel.uc_chan_number)
                {
                    dmac_scan_switch_channel_notify_alg(pst_hal_device, pst_dmac_home_vap, &(pst_hal_scan_params->st_home_channel));
                }
            }
            else
            {
                OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_handle_switch_channel_back::vap id[%d],pst_dmac_vap is null.}", uc_mac_vap_id);
            }
        }
    }
}

oal_void dmac_single_hal_device_scan_complete(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru  *pst_hal_device,
                                                         dmac_vap_stru  *pst_dmac_vap, oal_uint8 uc_scan_complete_event)
{
    oal_uint8                   uc_do_p2p_listen;
    oal_uint32                  ul_run_time;
    oal_uint32                  ul_current_timestamp;
    hal_scan_params_stru       *pst_hal_scan_params = &(pst_hal_device->st_hal_scan_params);

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
    dmac_device_stru           *pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);;
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_device->uc_device_id, OAM_SF_SCAN, "{dmac_single_hal_device_scan_complete::pst_dmac_device null.}");
        return;
    }
#endif

    if (MAC_SCAN_STATE_IDLE == pst_hal_scan_params->en_curr_scan_state)
    {
        OAM_ERROR_LOG1(pst_hal_device->uc_device_id, OAM_SF_SCAN, "{dmac_single_hal_device_scan_complete::hal dev is already scan complete[%d].}",
                        pst_hal_scan_params->en_curr_scan_state);
        return;
    }

    /* listen时修改vap信道为listen信道，listen结束后需要恢复 p2p listen不可能是并发 */
    uc_do_p2p_listen = pst_mac_device->st_scan_params.uc_scan_func & MAC_SCAN_FUNC_P2P_LISTEN;
    if (uc_do_p2p_listen)
    {
        pst_dmac_vap->st_vap_base_info.st_channel = pst_mac_device->st_p2p_vap_channel;
    }

    ul_current_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    ul_run_time = OAL_TIME_GET_RUNTIME(pst_hal_scan_params->ul_scan_timestamp, ul_current_timestamp);

    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,"{dmac_single_hal_device_scan_complete::hal device[%d]scan total time[%d]ms,scan chan[%d]need_back_home[%d]}",
                        pst_hal_device->uc_device_id, ul_run_time, pst_hal_scan_params->uc_channel_nums, pst_hal_scan_params->en_need_switch_back_home_channel);

    dmac_scan_set_vap_mac_addr_by_scan_state(pst_mac_device, pst_hal_device, OAL_FALSE);

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
    //dmac_scan_dump_bss_list(&(pst_dmac_device->st_scan_for_gnss_info.st_dmac_scan_info_list));
    dmac_scan_check_ap_bss_info(&(pst_dmac_device->st_scan_for_gnss_info.st_dmac_scan_info_list));
#endif

    /* 此路扫描完成优先回home channel工作 */
    if (OAL_TRUE == pst_hal_scan_params->en_need_switch_back_home_channel)
    {
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        /* abort的话，强制结束扫描，避免引起其他问题, abort之后恢复到idle或者work状态了，
            ps机制按照正常流程走，可以处于save或者normal状态，btcoex只是做维测，确认下频繁程度 */
        if(HAL_DEVICE_EVENT_SCAN_ABORT == uc_scan_complete_event)
        {
            GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device) = HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT;
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_COEX,
                                "{dmac_single_hal_device_scan_complete:: do scan abort right now!!!}");
        }
#endif
        dmac_scan_handle_switch_channel_back(pst_mac_device, pst_hal_device, pst_hal_scan_params);

        hal_device_handle_event(pst_hal_device, uc_scan_complete_event, 0, OAL_PTR_NULL);
    }
    else
    {
        hal_disable_machw_phy_and_pa(pst_hal_device);
        hal_clear_tx_hw_queue(pst_hal_device);
        hal_recover_machw_phy_and_pa(pst_hal_device);
        dmac_clear_tx_queue(pst_hal_device);

        hal_device_handle_event(pst_hal_device, uc_scan_complete_event, 0, OAL_PTR_NULL);

#ifdef _PRE_WLAN_FEATURE_BTCOEX
        if(HAL_DEVICE_EVENT_SCAN_ABORT == uc_scan_complete_event)
        {
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_COEX,
                             "{dmac_single_hal_device_scan_complete:: scan abort not need back home channel bt_sw_preempt_type[%d]!}",
                             GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device));

            GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device) = HAL_BTCOEX_SW_POWSAVE_SCAN_ABORT;
        }
#endif
    }

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    dmac_scan_check_2g_scan_results(pst_mac_device, pst_hal_device, &(pst_dmac_vap->st_vap_base_info), pst_hal_scan_params->uc_last_channel_band);
#endif
}

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
extern    oal_uint32        g_ul_frw_timer_start_stamp;     //维测信号，用来记录下一次软中断定时器的启动时间
extern    oal_uint32        g_ul_dispatch_event_time;       //维测信号，用来记录中断函数抛事件的时间
extern    oal_uint32        g_ul_proc_start_time;           //维测信号，用来记录事件处理的起始时间
extern    oal_uint32        g_ul_interrupt_start_time;
extern    oal_dlist_head_stru         g_st_timer_list;
OAL_STATIC oal_void dmac_scan_check_scan_timer(hal_to_dmac_device_stru  *pst_hal_device)
{
    oal_uint32             ul_timer_runtime;
    oal_uint32             ul_current_time;
    oal_dlist_head_stru   *pst_timeout_entry;
    frw_timeout_stru      *pst_timeout_element;
    frw_timeout_stru      *pst_scan_timer = &(pst_hal_device->st_hal_scan_params.st_scan_timer);

    /*timer create time = ul_time_stamp - ul_timeout */
    ul_current_time  = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    ul_timer_runtime = OAL_TIME_GET_RUNTIME((pst_scan_timer->ul_time_stamp - pst_scan_timer->ul_timeout), ul_current_time);
    if ((ul_timer_runtime > (pst_scan_timer->ul_timeout + DMAC_SCAN_TIMER_DEVIATION_TIME))&&(g_ul_interrupt_start_time > (g_ul_frw_timer_start_stamp + 2)))
    {
        OAM_WARNING_LOG3(0, OAM_SF_SCAN, "{dmac_scan_check_scan_timer::current time[%u]timer too long[%d]to run expert[%d]}", ul_current_time, ul_timer_runtime, pst_scan_timer->ul_timeout);
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "Device:frw_timer_start_stamp [%u],inter_start_time[%u],dispatch_event_time[%u],proc_start_time[%u]",g_ul_frw_timer_start_stamp,g_ul_interrupt_start_time,g_ul_dispatch_event_time,g_ul_proc_start_time);

        //排除因为timer事件抛出到等待处理的时间导致的timer延时
        OAM_WARNING_LOG3(0, OAM_SF_ANY, "current_timer enabled[%d], registerd[%d], period[%d]",
                     pst_scan_timer->en_is_enabled, pst_scan_timer->en_is_registerd, pst_scan_timer->en_is_periodic);
        OAM_WARNING_LOG3(0, OAM_SF_ANY, "current_timer stamp[%u], timeout[%u], curr timer stamp[%u]",
                     pst_scan_timer->ul_time_stamp, pst_scan_timer->ul_timeout, pst_scan_timer->ul_curr_time_stamp);
        pst_timeout_entry = g_st_timer_list.pst_next;
        if (OAL_PTR_NULL != pst_timeout_entry)
        {
            pst_timeout_element = OAL_DLIST_GET_ENTRY(pst_timeout_entry, frw_timeout_stru, st_entry);

            OAM_WARNING_LOG3(0, OAM_SF_ANY, "first_timer enabled[%d], registerd[%d], period[%d]",
                         pst_timeout_element->en_is_enabled, pst_timeout_element->en_is_registerd, pst_timeout_element->en_is_periodic);
            OAM_WARNING_LOG3(0, OAM_SF_ANY, "first_timer stamp[%u], timeout[%u], curr timer stamp[%u]",
                         pst_timeout_element->ul_time_stamp, pst_timeout_element->ul_timeout, pst_timeout_element->ul_curr_time_stamp);
        }
    }
}
#endif

OAL_STATIC oal_uint32  dmac_scan_curr_channel_scan_time_out(void *p_arg)
{
    mac_device_stru                 *pst_mac_device;
    hal_scan_params_stru            *pst_hal_scan_params;
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint8                        uc_do_meas;
    hal_to_dmac_device_stru         *pst_hal_device = (hal_to_dmac_device_stru *)p_arg;

    /* 获取扫描参数 */
    pst_hal_scan_params = &(pst_hal_device->st_hal_scan_params);

    pst_mac_device  = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_curr_channel_scan_time_out::id[%d]pst_mac_device == NULL}", pst_hal_device->uc_mac_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取发起扫描的dmac vap结构信息 */
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_device->st_scan_params.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_curr_channel_scan_time_out::mac_res_get_dmac_vap fail}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    dmac_scan_check_scan_timer(pst_hal_device);
#endif

    /* 更新本次信道的扫描次数 */
    pst_hal_scan_params->uc_curr_channel_scan_count++;

    /* 根据扫描参数中信道扫描最大次数，判断是否切换信道号 */
    if (pst_hal_scan_params->uc_curr_channel_scan_count >= pst_hal_scan_params->uc_max_scan_count_per_channel)
    {
        /* 本信道扫描结束，如果下发的扫描参数需要上报信道统计信息，则上报，默认关闭 */
        uc_do_meas = pst_mac_device->st_scan_params.uc_scan_func & (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS);
        if (uc_do_meas)
        {
            /* 上报信道测量结果 */
            dmac_scan_report_channel_statistics_result(pst_hal_device, (GET_HAL_DEV_CURRENT_SCAN_IDX(pst_hal_device)));

            /* 清空信道测量结果，准备下一次信道测量值的统计 */
            OAL_MEMZERO(&(pst_hal_device->st_chan_result), OAL_SIZEOF(wlan_scan_chan_stats_stru));
        }

        pst_hal_scan_params->uc_scan_chan_idx += 1;              /* 切换信道 */
    }
    else
    {
        /* 本信道扫描次数未完成，无需切换信道，直接发起扫描 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        /* 处于ps状态下，扫描等到bt的ps结束后来执行，置scan begin状态给btcoex */
        if(HAL_BTCOEX_SW_POWSAVE_WORK == GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device))
        {
            GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device) = HAL_BTCOEX_SW_POWSAVE_SCAN_BEGIN;
        }
        else
#endif
        {
            dmac_scan_begin(pst_mac_device, pst_hal_device);
        }

        return OAL_SUCC;
    }

    /* 此次扫描请求完成，做一些收尾工作 */
    if (pst_hal_scan_params->uc_scan_chan_idx >= pst_hal_scan_params->uc_channel_nums)
    {
        dmac_scan_prepare_end(pst_mac_device, pst_hal_device);
        return OAL_SUCC;
    }

    if (OAL_TRUE == dmac_scan_need_switch_home_channel(pst_hal_device))
    {
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        /* 需要回home channel时，如果处于ps状态，需要延迟，等到ps机制完成后回home channel */
        if (HAL_BTCOEX_SW_POWSAVE_WORK == GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device))
        {
            GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device) = HAL_BTCOEX_SW_POWSAVE_SCAN_WAIT;
        }
        else
#endif
        {
            dmac_scan_switch_home_channel_work(pst_mac_device, pst_hal_device);
        }
    }
    else
    {
        dmac_scan_one_channel_start(pst_hal_device, OAL_FALSE);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_pno_scan_send_probe_with_ssid(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_band)
{
    mac_pno_sched_scan_mgmt_stru *pst_pno_sched_scan_mgmt;
    dmac_vap_stru                *pst_dmac_vap;
    oal_uint8                     uc_band_tmp;
    oal_uint8                     uc_loop;
    oal_uint32                    ul_ret;

    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_pno_scan_send_probe_with_ssid::pst_mac_device is null.}");
        return;
    }

    pst_pno_sched_scan_mgmt = pst_mac_device->pst_pno_sched_scan_mgmt;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_pno_scan_send_probe_with_ssid::pst_dmac_vap null.}");
        return;
    }

    
    if(OAL_PTR_NULL == pst_dmac_vap->st_vap_base_info.pst_mib_info)
    {
        OAM_ERROR_LOG4(0, OAM_SF_SCAN, "{dmac_pno_scan_send_probe_with_ssid:: vap mib info is null,uc_vap_id[%d], p_fn_cb[%p], uc_scan_func[%d], uc_curr_channel_scan_count[%d].}",
                   pst_mac_device->st_scan_params.uc_vap_id,
                   pst_mac_device->st_scan_params.p_fn_cb,
                   pst_mac_device->st_scan_params.uc_scan_func,
                   pst_hal_device->st_hal_scan_params.uc_curr_channel_scan_count);
        return;
    }


    /* 发探测请求时，需要临时更新vap的band信息，防止5G发11b */
    uc_band_tmp = pst_dmac_vap->st_vap_base_info.st_channel.en_band;

    pst_dmac_vap->st_vap_base_info.st_channel.en_band = uc_band;

    for(uc_loop = 0; uc_loop < pst_pno_sched_scan_mgmt->st_pno_sched_scan_params.l_ssid_count; uc_loop++)
    {
        if(OAL_TRUE == pst_pno_sched_scan_mgmt->st_pno_sched_scan_params.ast_match_ssid_set[uc_loop].en_scan_ssid)
        {
            /* 如果是隐藏SSID,则指定SSID扫描 */
            ul_ret = dmac_scan_send_probe_req_frame(pst_dmac_vap, BROADCAST_MACADDR, (oal_int8 *)pst_pno_sched_scan_mgmt->st_pno_sched_scan_params.ast_match_ssid_set[uc_loop].auc_ssid);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{dmac_pno_scan_send_probe_with_ssid::dmac_scan_send_probe_req_frame failed[%u].}", ul_ret);
            }
        }
    }

    pst_dmac_vap->st_vap_base_info.st_channel.en_band = uc_band_tmp;
    return;
}


oal_void dmac_scan_begin(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device)
{
    hal_to_dmac_device_stru         *pst_original_hal_device;
    mac_scan_req_stru               *pst_scan_params;
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint32                       ul_ret;
    oal_uint8                        uc_band;
    oal_uint8                        uc_do_bss_scan;
    oal_uint8                        uc_do_meas;
    oal_uint8                        uc_loop;
    oal_uint8                        uc_scan_chan_idx;
    oal_uint8                        uc_do_p2p_listen = 0;
    oal_bool_enum_uint8              en_send_probe_req;
    oal_uint16                       us_scan_time;
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    oal_uint32                       aul_act_meas_start_time[2];
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    dmac_ftm_initiator_stru         *past_ftm_init;
#endif

    /* 此路hal device不存在,此次不扫描 */
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_begin::pst_hal_device NULL.}");
        return;
    }

    /* 此路优先扫描完成不再发pro req */
    if (MAC_SCAN_STATE_IDLE == pst_hal_device->st_hal_scan_params.en_curr_scan_state)
    {
        OAM_ERROR_LOG1(pst_hal_device->uc_device_id, OAM_SF_SCAN, "{dmac_scan_begin::now chan idx[%d] scan is completed.}", pst_hal_device->st_hal_scan_params.uc_scan_chan_idx);
        return;
    }

    /* 此路被pause不能扫描 */
    if (pst_hal_device->st_hal_scan_params.uc_scan_pause_bitmap)
    {
        OAM_WARNING_LOG2(pst_hal_device->uc_device_id, OAM_SF_SCAN, "{dmac_scan_begin::scan is paused[%x%x].scan chan idx[%d]}",
                                pst_hal_device->st_hal_scan_params.uc_scan_pause_bitmap, pst_hal_device->st_hal_scan_params.uc_scan_chan_idx);
        return;
    }

    /* 获取扫描参数 */
    pst_scan_params = &(pst_mac_device->st_scan_params);

    uc_scan_chan_idx = GET_HAL_DEV_CURRENT_SCAN_IDX(pst_hal_device);
    uc_band          = pst_scan_params->ast_channel_list[uc_scan_chan_idx].en_band;
    uc_do_bss_scan   = pst_scan_params->uc_scan_func & MAC_SCAN_FUNC_BSS;
    uc_do_p2p_listen = pst_scan_params->uc_scan_func & MAC_SCAN_FUNC_P2P_LISTEN;
    pst_dmac_vap     = mac_res_get_dmac_vap(pst_scan_params->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_scan_params->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_begin::pst_dmac_vap null.}");
        return;
    }

#ifdef _PRE_WLAN_FEATURE_FTM
    past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;
#endif

    /* 描过程中切换指针是否有问题 */
    pst_original_hal_device   = pst_dmac_vap->pst_hal_device;
    pst_dmac_vap->pst_hal_device = pst_hal_device;

    /* 如果当前扫描模式需要统计信道信息，则使能对应寄存器 */
    uc_do_meas = pst_scan_params->uc_scan_func & (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS);
    if (uc_do_meas)
    {
        /* 使能信道测量中断 */
        hal_set_ch_statics_period(pst_hal_device, DMAC_SCAN_CHANNEL_STATICS_PERIOD_US);
        hal_set_ch_measurement_period(pst_hal_device, DMAC_SCAN_CHANNEL_MEAS_PERIOD_MS);
        hal_enable_ch_statics(pst_hal_device, 1);
    }

#ifdef _PRE_WLAN_FEATURE_11K
    if ((OAL_TRUE == pst_dmac_vap->bit_11k_enable) &&
        (WLAN_SCAN_MODE_RRM_BEACON_REQ == pst_scan_params->en_scan_mode))
    {
        hal_vap_tsf_get_64bit(pst_dmac_vap->pst_hal_vap, &(pst_dmac_vap->pst_rrm_info->aul_act_meas_start_time[1]),
                                  &(pst_dmac_vap->pst_rrm_info->aul_act_meas_start_time[0]));
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{dmac_scan_begin::update start tsf ok, vap id[%d].}", pst_scan_params->uc_vap_id);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    if(mac_mib_get_dot11RadioMeasurementActivated(&(pst_dmac_vap->st_vap_base_info)))
    {
        hal_vap_tsf_get_64bit(pst_dmac_vap->pst_hal_vap, &aul_act_meas_start_time[1], &aul_act_meas_start_time[0]);
        //OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_begin::aul_act_meas_start_time[%u][%u].}", aul_act_meas_start_time[1], aul_act_meas_start_time[0]);
        dmac_send_sys_event(&(pst_dmac_vap->st_vap_base_info), WLAN_CFGID_GET_MEAS_START_TSF, OAL_SIZEOF(oal_uint32)*2, (oal_uint8*)aul_act_meas_start_time);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
    if(OAL_TRUE == past_ftm_init[0].en_iftmr)
    {
        OAM_WARNING_LOG0(pst_scan_params->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_begin::dmac_send_iftmr}");
        dmac_sta_send_ftm_req(pst_dmac_vap);
        past_ftm_init[0].en_iftmr = OAL_FALSE;
    }
    if(OAL_TRUE == past_ftm_init[0].en_ftm_trigger)
    {
        OAM_WARNING_LOG0(pst_scan_params->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_begin::dmac_ftm_send_trigger}");
        dmac_ftm_send_trigger(pst_dmac_vap);
        //dmac_sta_send_ftm_req(pst_dmac_vap);
        past_ftm_init[0].en_ftm_trigger = OAL_FALSE;
    }
#endif

    /* dfs信道判断，如果是雷达信道，执行被动扫描 */
    dmac_scan_update_dfs_channel_scan_param(pst_mac_device,
                                            pst_hal_device,
                                            &(pst_scan_params->ast_channel_list[uc_scan_chan_idx]),
                                            &us_scan_time,
                                            &en_send_probe_req);

    /* ACTIVE方式下发送广播RPOBE REQ帧 */
    if (uc_do_bss_scan && (WLAN_SCAN_TYPE_ACTIVE == pst_scan_params->en_scan_type)
        && (OAL_TRUE == en_send_probe_req))
    {
        /* PNO指定SSID扫描,最多指定16个SSID */
        if(WLAN_SCAN_MODE_BACKGROUND_PNO == pst_mac_device->st_scan_params.en_scan_mode)
        {
            dmac_pno_scan_send_probe_with_ssid(pst_mac_device, pst_hal_device, uc_band);
        }

        /* 每次信道发送的probe req帧的个数 */
        for (uc_loop = 0; uc_loop < pst_scan_params->uc_max_send_probe_req_count_per_channel; uc_loop++)
        {
            ul_ret = dmac_scan_send_bcast_probe(pst_mac_device, uc_band, uc_loop);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{dmac_scan_begin::dmac_scan_send_bcast_probe failed[%d].}", ul_ret);
            }
        }
    }

    /* 放在发送完成所有probe request报文再启动定时器,防止指定SSID扫描报文过多,定时器时间内都在发送,用于接收扫描结果时间过短 */
    /* 启动扫描定时器 */
    FRW_TIMER_CREATE_TIMER(&(pst_hal_device->st_hal_scan_params.st_scan_timer),
                           dmac_scan_curr_channel_scan_time_out,
                           us_scan_time,
                           pst_hal_device,
                           OAL_FALSE,
                           OAM_MODULE_ID_DMAC,
                           pst_mac_device->ul_core_id);

    pst_dmac_vap->pst_hal_device = pst_original_hal_device;

    /* p2p监听处理逻辑 */
    /* p2p listen时需要更改VAP的信道，组probe rsp帧(DSSS ie, ht ie)需要。listen结束后恢复 */
    if (uc_do_p2p_listen)
    {
        pst_mac_device->st_p2p_vap_channel = pst_dmac_vap->st_vap_base_info.st_channel;

        pst_dmac_vap->st_vap_base_info.st_channel = pst_scan_params->ast_channel_list[0];
    }

    return;
}


oal_void dmac_scan_end(mac_device_stru *pst_mac_device)
{
    dmac_vap_stru              *pst_dmac_vap;
    mac_vap_stru               *pst_mac_vap;
    wlan_scan_mode_enum_uint8   en_scan_mode = WLAN_SCAN_MODE_BUTT;
#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
    dmac_device_stru           *pst_dmac_device;
#endif
    /* 获取dmac vap */
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_end::pst_dmac_vap is null.}");

        /* 恢复device扫描状态为空闲状态 */
        pst_mac_device->en_curr_scan_state = MAC_SCAN_STATE_IDLE;
        return;
    }

    /* 获取扫描模式 */
    en_scan_mode = pst_mac_device->st_scan_params.en_scan_mode;
    pst_mac_vap  = &pst_dmac_vap->st_vap_base_info;

    if(WALN_LINKLOSS_SCAN_SWITCH_CHAN_EN == pst_mac_vap->st_ch_switch_info.en_linkloss_scan_switch_chan)
    {
        pst_mac_vap->st_ch_switch_info.en_linkloss_scan_switch_chan = WALN_LINKLOSS_SCAN_SWITCH_CHAN_DISABLE;
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_end::csa_fsm back to init}");
        dmac_sta_csa_fsm_post_event(&(pst_dmac_vap->st_vap_base_info), WLAN_STA_CSA_EVENT_TO_INIT, 0, OAL_PTR_NULL);

        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_scan_end::linkloss scan change channel from [%d] to [%d].}",
            pst_mac_vap->st_channel.uc_chan_number, pst_mac_vap->st_ch_switch_info.uc_linkloss_change_chanel);
        pst_mac_vap->st_ch_switch_info.uc_new_channel   = pst_mac_vap->st_ch_switch_info.uc_linkloss_change_chanel;
        pst_mac_vap->st_ch_switch_info.en_new_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
        dmac_chan_sta_switch_channel(pst_mac_vap);
    }

    /* 根据扫描模式进行对应扫描结果的处理 */
    switch (en_scan_mode)
    {
        case WLAN_SCAN_MODE_FOREGROUND:
        case WLAN_SCAN_MODE_BACKGROUND_STA:
        case WLAN_SCAN_MODE_BACKGROUND_AP:
#ifdef _PRE_WLAN_FEATURE_HILINK
        case WLAN_SCAN_MODE_BACKGROUND_HILINK:
#endif
        {
#ifdef _PRE_WLAN_FEATURE_STA_PM
            /* 平台的计数*/
            pfn_wlan_dumpsleepcnt();
#endif
            /* 上报扫描完成事件，扫描状态为成功 */
            (oal_void)dmac_scan_proc_scan_complete_event(pst_dmac_vap, MAC_SCAN_SUCCESS);
            break;
        }
        case WLAN_SCAN_MODE_BACKGROUND_OBSS:
        {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
            dmac_scan_proc_obss_scan_complete_event(pst_dmac_vap);
#endif
#endif
            break;
        }
        case WLAN_SCAN_MODE_BACKGROUND_PNO:
        {
            /* 是否扫描到了匹配的ssid，如果是，上报扫描结果; 否则进入睡眠 */
            if (OAL_TRUE == pst_mac_device->pst_pno_sched_scan_mgmt->en_is_found_match_ssid)
            {
                /* 上报扫描完成事件，扫描状态为成功 */
                (oal_void)dmac_scan_proc_scan_complete_event(pst_dmac_vap, MAC_SCAN_PNO);

                /* 释放PNO管理结构体内存 */
                OAL_MEM_FREE(pst_mac_device->pst_pno_sched_scan_mgmt, OAL_TRUE);
                pst_mac_device->pst_pno_sched_scan_mgmt = OAL_PTR_NULL;
            }

            break;
        }
        case WLAN_SCAN_MODE_BACKGROUND_CSA:
        {
            OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_scan_end::scan_mode BACKGROUND_CSA}");
            dmac_sta_csa_fsm_post_event(&(pst_dmac_vap->st_vap_base_info), WLAN_STA_CSA_EVENT_SCAN_END, 0, OAL_PTR_NULL);
            break;
        }

#ifdef _PRE_WLAN_FEATURE_11K
        case WLAN_SCAN_MODE_RRM_BEACON_REQ:
        {
            /* 扫描结束后组响应帧，填充并发出 */
            /* 申请管理帧内存并填充头部信息 */
            dmac_rrm_encap_and_send_bcn_rpt(pst_dmac_vap);
            break;
        }
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
        case WLAN_SCAN_MODE_FTM_REQ:
        {
            break;
        }
#endif

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
        case WLAN_SCAN_MODE_GNSS_SCAN:
        {
            pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
            pst_dmac_device->st_scan_for_gnss_info.ul_scan_end_timstamps = (oal_uint32)OAL_TIME_GET_STAMP_MS();
            OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_scan_end::scan_mode gscan}");
            break;
        }
#endif

        default:
        {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_end::scan_mode[%d] error.}", en_scan_mode);
            break;
        }
    }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_set_btcoex_soc_gpreg1(OAL_FALSE, BIT1, 1);   // 入网流程结束

    if (WLAN_P2P_DEV_MODE == pst_dmac_vap->st_vap_base_info.en_p2p_mode)
    {
        hal_set_btcoex_soc_gpreg0(OAL_FALSE, BIT14, 14);   // p2p 扫描流程结束
    }

    hal_coex_sw_irq_set(HAL_COEX_SW_IRQ_BT);
#endif

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
    dmac_alg_anti_intf_switch(pst_dmac_vap->pst_hal_device, OAL_TRUE);
#endif

    /* 恢复device扫描状态为空闲状态 */
    pst_mac_device->en_curr_scan_state = MAC_SCAN_STATE_IDLE;
    pst_mac_device->st_scan_params.en_scan_mode = WLAN_SCAN_MODE_BUTT;

    return;
}

oal_void dmac_prepare_fast_scan_end_in_dbdc(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device,
                                                    hal_to_dmac_device_stru   *pst_other_hal_device, dmac_vap_stru  *pst_dmac_vap)
{
    dmac_single_hal_device_scan_complete(pst_mac_device, pst_hal_device, pst_dmac_vap, HAL_DEVICE_EVENT_SCAN_END);
    if (MAC_SCAN_STATE_IDLE == pst_other_hal_device->st_hal_scan_params.en_curr_scan_state)
    {
        dmac_scan_end(pst_mac_device);
    }
}

oal_void dmac_prepare_fast_scan_end_in_normal(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device,
                                                    hal_to_dmac_device_stru   *pst_other_hal_device, dmac_vap_stru  *pst_dmac_vap)
{
    hal_to_dmac_device_stru    *pst_master_hal_device = OAL_PTR_NULL;
    hal_to_dmac_device_stru    *pst_slave_hal_device  = OAL_PTR_NULL;

    if (pst_other_hal_device->st_hal_scan_params.uc_scan_chan_idx >= pst_other_hal_device->st_hal_scan_params.uc_channel_nums)
    {
        if (HAL_DEVICE_ID_MASTER == pst_other_hal_device->uc_device_id)
        {
            pst_master_hal_device = pst_other_hal_device;
            pst_slave_hal_device  = pst_hal_device;
        }
        else
        {
            pst_master_hal_device = pst_hal_device;
            pst_slave_hal_device  = pst_other_hal_device;
        }

        dmac_single_hal_device_scan_complete(pst_mac_device, pst_slave_hal_device, pst_dmac_vap, HAL_DEVICE_EVENT_SCAN_END);//先辅路扫描完成切到idle
        dmac_single_hal_device_scan_complete(pst_mac_device, pst_master_hal_device, pst_dmac_vap, HAL_DEVICE_EVENT_SCAN_END);//主路扫描完成
        dmac_scan_end(pst_mac_device);
    }
}

OAL_STATIC oal_void dmac_scan_prepare_fscan_end(dmac_device_stru  *pst_dmac_device, hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap)
{
    hal_to_dmac_device_stru                *pst_other_hal_device  = OAL_PTR_NULL;
    mac_device_stru                        *pst_mac_device = pst_dmac_device->pst_device_base_info;

    pst_other_hal_device = dmac_device_get_another_h2d_dev(pst_dmac_device, pst_hal_device);
    if (OAL_PTR_NULL == pst_other_hal_device)
    {
        OAM_ERROR_LOG1(pst_mac_device->st_scan_params.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_prepare_fscan_end::dmac_device_get_another_h2d_dev ori hal[%d]}", pst_hal_device->uc_device_id);
        return;
    }

    /* 此路结束看另外一路是否被scan pause,true要恢复另外一路的扫描 */
    if (oal_bit_get_bit_one_byte(pst_other_hal_device->st_hal_scan_params.uc_scan_pause_bitmap, HAL_SCAN_PASUE_TYPE_CHAN_CONFLICT))
    {
        hal_device_handle_event(pst_other_hal_device, HAL_DEVICE_EVENT_SCAN_RESUME_FROM_CHAN_CONFLICT, 0, OAL_PTR_NULL);
    }

    if (OAL_TRUE == pst_mac_device->en_dbdc_running)
    {
        dmac_prepare_fast_scan_end_in_dbdc(pst_mac_device, pst_hal_device, pst_other_hal_device, pst_dmac_vap);
    }
    else
    {
        dmac_prepare_fast_scan_end_in_normal(pst_mac_device, pst_hal_device, pst_other_hal_device, pst_dmac_vap);
    }
}

oal_void  dmac_scan_prepare_end(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device)
{
    dmac_device_stru                       *pst_dmac_device;
    dmac_vap_stru                          *pst_dmac_vap;

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_device->uc_device_id, OAM_SF_SCAN, "{dmac_scan_prepare_end::pst_dmac_device null.}");
        return;
    }

    /* 获取发起扫描的dmac vap结构信息 */
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_device->st_scan_params.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_prepare_end::mac_res_get_dmac_vap fail}");
        return;
    }

    /* 并发扫描需要先辅路回去再主路结束,以便主路回mimo后继续工作 */
    if (OAL_TRUE == pst_dmac_device->en_is_fast_scan)
    {
        dmac_scan_prepare_fscan_end(pst_dmac_device, pst_hal_device, pst_dmac_vap);
    }
    else
    {
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        /* end事件要考虑btcoex的ps状态，ps使能时候，需要延迟来恢复end，其他状态不影响 */
        if (HAL_BTCOEX_SW_POWSAVE_WORK == GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device))
        {
            GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device) = HAL_BTCOEX_SW_POWSAVE_SCAN_END;
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_COEX, "{dmac_scan_prepare_end:: normal scan end delay by btcoex!}");
        }
        else
#endif
        {
            dmac_single_hal_device_scan_complete(pst_mac_device, pst_hal_device, pst_dmac_vap, HAL_DEVICE_EVENT_SCAN_END);
            dmac_scan_end(pst_mac_device);
        }
    }
}


oal_void  dmac_scan_abort(mac_device_stru *pst_mac_device)
{
    oal_uint8                               uc_device_id;
    oal_uint8                               uc_device_max;
    dmac_vap_stru                          *pst_dmac_vap;
    hal_to_dmac_device_stru                *pst_hal_device;
    dmac_device_stru                       *pst_dmac_device;

    /* 不处于扫描状态，或者不处于CCA扫描状态，直接返回。建议CCA扫描从普通扫描中剥离出去,此处CCA逻辑后续删除，TBD */
    if (MAC_SCAN_STATE_RUNNING != pst_mac_device->en_curr_scan_state)
    {
        return;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_device->st_scan_params.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_abort::pst_dmac_vap null.}");
        return;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_device->uc_device_id, OAM_SF_SCAN, "{dmac_scan_abort::pst_dmac_device null.}");
        return;
    }
    /* HAL接口获取支持device个数 */
    hal_chip_get_device_num(pst_mac_device->uc_chip_id, &uc_device_max);

    /* 并发非并发扫描结束方式不一样 */
    if (OAL_FALSE == pst_dmac_device->en_is_fast_scan)
    {
        for (uc_device_id = 0; uc_device_id < uc_device_max; uc_device_id++)
        {
            pst_hal_device = pst_dmac_device->past_hal_device[uc_device_id];
            if (OAL_PTR_NULL == pst_hal_device)
            {
                continue;
            }

            /* 删除扫描定时器 */
            if (OAL_TRUE == pst_hal_device->st_hal_scan_params.st_scan_timer.en_is_registerd)
            {
                FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hal_device->st_hal_scan_params.st_scan_timer));
            }

            if ((MAC_SCAN_STATE_RUNNING == pst_hal_device->st_hal_scan_params.en_curr_scan_state))
            {
                dmac_single_hal_device_scan_complete(pst_mac_device, pst_hal_device, pst_dmac_vap, HAL_DEVICE_EVENT_SCAN_ABORT);
            }
        }
    }
    else
    {
        pst_hal_device = DMAC_DEV_GET_SLA_HAL_DEV(pst_dmac_device);
        if (pst_hal_device != OAL_PTR_NULL)
        {
            dmac_single_hal_device_scan_complete(pst_mac_device, pst_hal_device, pst_dmac_vap, HAL_DEVICE_EVENT_SCAN_ABORT);//必须先辅路扫描完成切到idle
        }
        dmac_single_hal_device_scan_complete(pst_mac_device, DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device), pst_dmac_vap, HAL_DEVICE_EVENT_SCAN_ABORT);//主路扫描完成
    }

    pst_mac_device->st_scan_params.en_abort_scan_flag = OAL_TRUE;
    dmac_scan_end(pst_mac_device);
    pst_mac_device->st_scan_params.en_abort_scan_flag = OAL_FALSE;

    OAM_WARNING_LOG0(0, OAM_SF_SCAN, "dmac_scan_abort: scan has been aborted");
}


OAL_STATIC oal_uint32 dmac_scan_get_ssid_ie_info(mac_device_stru *pst_mac_device, oal_int8 *pc_ssid, oal_uint8  uc_index)
{
    dmac_vap_stru     *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_device->st_scan_params.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_get_ssid_ie_info::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (IS_LEGACY_VAP(&(pst_dmac_vap->st_vap_base_info)))
    {
        /* 根据索引号，准备probe req帧的ssid信息 */
        oal_memcopy(pc_ssid, pst_mac_device->st_scan_params.ast_mac_ssid_set[uc_index].auc_ssid, WLAN_SSID_MAX_LEN);
    }
    else
    {
        /* P2P 设备扫描，需要获取指定ssid 信息，对P2P 设备，扫描时只扫描一个指定ssid */
        oal_memcopy(pc_ssid, pst_mac_device->st_scan_params.ast_mac_ssid_set[0].auc_ssid, WLAN_SSID_MAX_LEN);
    }

    return OAL_SUCC;
}



OAL_STATIC oal_uint32 dmac_scan_send_bcast_probe(mac_device_stru *pst_mac_device, oal_uint8 uc_band, oal_uint8  uc_index)
{
    oal_int8           ac_ssid[WLAN_SSID_MAX_LEN] = {'\0'};
    dmac_vap_stru     *pst_dmac_vap;
    oal_uint32         ul_ret;
    oal_uint8          uc_band_tmp;

    if (0 == pst_mac_device->uc_vap_num)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_scan_send_bcast_probe::uc_vap_num=0.}");
        return OAL_FAIL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_send_bcast_probe::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    
    if(OAL_PTR_NULL == pst_dmac_vap->st_vap_base_info.pst_mib_info)
    {
        OAM_ERROR_LOG4(0, OAM_SF_SCAN, "{dmac_scan_send_bcast_probe:: vap mib info is null,uc_vap_id[%d], p_fn_cb[%p], uc_scan_func[%d], uc_curr_channel_scan_count[%d].}",
                   pst_mac_device->st_scan_params.uc_vap_id,
                   pst_mac_device->st_scan_params.p_fn_cb,
                   pst_mac_device->st_scan_params.uc_scan_func,
                   pst_dmac_vap->pst_hal_device->st_hal_scan_params.uc_curr_channel_scan_count);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 发探测请求时，需要临时更新vap的band信息，防止5G发11b */
    uc_band_tmp = pst_dmac_vap->st_vap_base_info.st_channel.en_band;

    pst_dmac_vap->st_vap_base_info.st_channel.en_band = uc_band;

    /* 获取本次扫描请求帧中需要携带的ssid ie信息 */
    ul_ret = dmac_scan_get_ssid_ie_info(pst_mac_device, ac_ssid, uc_index);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_send_bcast_probe::get ssid failed, error[%d].}", ul_ret);
        return ul_ret;
    }

    /* 发送probe req帧 */
    ul_ret = dmac_scan_send_probe_req_frame(pst_dmac_vap, BROADCAST_MACADDR, ac_ssid);

    pst_dmac_vap->st_vap_base_info.st_channel.en_band = uc_band_tmp;

    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_send_bcast_probe::dmac_mgmt_send_probe_request failed[%d].}", ul_ret);
    }

    return ul_ret;
}


oal_void  dmac_scan_switch_home_channel_work(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device)
{
    hal_scan_params_stru       *pst_scan_params;

    pst_scan_params = &(pst_hal_device->st_hal_scan_params);

    /* 切回工作信道工作时，根据是否为随机mac addr扫描，恢复vap原先的mac addr */
    dmac_scan_set_vap_mac_addr_by_scan_state(pst_mac_device, pst_hal_device, OAL_FALSE);

    /* 背景扫描 切回工作信道 */
    dmac_scan_switch_channel_back(pst_mac_device, pst_hal_device);

    hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_SCAN_SWITCH_CHANNEL_BACK, 0, OAL_PTR_NULL);

    /* 判零检查 */
    if (0 == pst_scan_params->us_work_time_on_home_channel)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{dmac_scan_switch_home_channel_work:work_time_on_home_channel is 0, set it to default [%d]ms!}",MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT);
        pst_scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT;
    }

    /* 启动间隔定时器，在工作信道工作一段时间后，切回扫描信道进行扫描 */
    FRW_TIMER_CREATE_TIMER(&(pst_scan_params->st_scan_timer),
                           dmac_scan_switch_home_channel_work_timeout,
                           pst_scan_params->us_work_time_on_home_channel,
                           pst_hal_device,
                           OAL_FALSE,
                           OAM_MODULE_ID_DMAC,
                           pst_mac_device->ul_core_id);

    return;
}


OAL_STATIC oal_uint32  dmac_scan_switch_home_channel_work_timeout(void *p_arg)
{
    mac_device_stru         *pst_mac_device;
    dmac_vap_stru           *pst_dmac_vap;
    hal_to_dmac_device_stru *pst_hal_device = (hal_to_dmac_device_stru *)p_arg;

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_switch_home_channel_work_timeout::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断是否还需要继续进行扫描，如果此时扫描状态为非运行状态，说明扫描已经停止，无需再继续扫描 */
    if (MAC_SCAN_STATE_RUNNING != pst_mac_device->en_curr_scan_state)
    {
        OAM_WARNING_LOG0(pst_mac_device->st_scan_params.uc_vap_id, OAM_SF_SCAN,
                         "{dmac_scan_switch_home_channel_work_timeout::scan has been aborted, no need to continue.}");
        return OAL_SUCC;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_device->st_scan_params.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_device->st_scan_params.uc_vap_id, OAM_SF_SCAN, "{dmac_scan_switch_home_channel_work_timeout::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    dmac_scan_check_scan_timer(pst_hal_device);
#endif

    dmac_scan_one_channel_start(pst_hal_device, OAL_TRUE);

    /* 清空信道测量结果 */
    OAL_MEMZERO(&(pst_hal_device->st_chan_result), OAL_SIZEOF(wlan_scan_chan_stats_stru));

    return OAL_SUCC;
}
#if 0

oal_void dmac_scan_radar_detected(mac_device_stru *pst_mac_device, hal_radar_det_event_stru *pst_radar_det_info)
{
    pst_mac_device->st_chan_result.uc_radar_detected = 1;
    pst_mac_device->st_chan_result.uc_radar_bw       = 0;
}
#endif


OAL_STATIC oal_uint32  dmac_scan_report_channel_statistics_result(hal_to_dmac_device_stru  *pst_hal_device, oal_uint8 uc_scan_idx)
{
    frw_event_mem_stru         *pst_event_mem;
    frw_event_stru             *pst_event;
    dmac_crx_chan_result_stru  *pst_chan_result_param;

    /* 抛信道扫描结果到HMAC, 申请事件内存 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_crx_chan_result_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_report_channel_statistics_result::alloc mem fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CRX,
                       DMAC_WLAN_CRX_EVENT_SUB_TYPE_CHAN_RESULT,
                       OAL_SIZEOF(dmac_crx_chan_result_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_hal_device->uc_chip_id,
                       pst_hal_device->uc_mac_device_id,
                       0);

    pst_chan_result_param = (dmac_crx_chan_result_stru *)(pst_event->auc_event_data);

    pst_chan_result_param->uc_scan_idx    = uc_scan_idx;
    pst_chan_result_param->st_chan_result = pst_hal_device->st_chan_result;

    frw_event_dispatch_event(pst_event_mem);
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}


oal_uint32 dmac_scan_init(mac_device_stru *pst_device)
{
    /* 初始化device扫描状态为空闲 */
    pst_device->en_curr_scan_state = MAC_SCAN_STATE_IDLE;

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
    dmac_scan_init_bss_info_list(pst_device);
#endif

    return OAL_SUCC;
}



oal_uint32 dmac_scan_exit(mac_device_stru *pst_device)
{
    return OAL_SUCC;
}

// TODO:算出来的数据不准:1、5g雷达信道时间不定2、回home channel的个数不是从这次开始计数
oal_uint32 dmac_scan_get_remaining_time(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 *pus_scan_remain_time)
{
    oal_uint8                uc_remain_scan_count;   //某信道剩余扫描次数
    oal_uint8                uc_remain_channel_nums; //已经扫描的信道个数
    oal_uint16               us_need_scan_channel_time;
    oal_uint16               us_need_work_on_home_channel_time;
    hal_scan_params_stru    *pst_hal_scan_params;
    mac_device_stru         *pst_mac_device;

    pst_mac_device =  mac_res_get_dev(pst_hal_device->uc_device_id);
    if (pst_mac_device->en_curr_scan_state != MAC_SCAN_STATE_RUNNING)
    {
        return OAL_FAIL;
    }

    /* 获取扫描参数 */
    pst_hal_scan_params = &(pst_hal_device->st_hal_scan_params);

    /* 剩余扫描信道数,总共的-已扫完的 */
    uc_remain_channel_nums = pst_hal_scan_params->uc_channel_nums - (pst_hal_scan_params->uc_scan_chan_idx - pst_hal_scan_params->uc_start_chan_idx);

    /* 某信道剩余扫描次数 */
    uc_remain_scan_count = pst_hal_scan_params->uc_max_scan_count_per_channel - pst_hal_scan_params->uc_curr_channel_scan_count;

    /*剩余单扫描信道需要的时间 = 一次扫描时间*(当前信道剩余次数+剩余信道扫描次数)*/
    us_need_scan_channel_time = (oal_uint16)(pst_hal_scan_params->us_scan_time * (uc_remain_scan_count + uc_remain_channel_nums * pst_hal_scan_params->uc_max_scan_count_per_channel));

    /* 需要在工作信道工作的剩余时间 = (剩余信道数/周期) * 工作时间 */
    us_need_work_on_home_channel_time = (oal_uint16)((uc_remain_channel_nums / pst_hal_scan_params->uc_scan_channel_interval) * pst_hal_scan_params->us_work_time_on_home_channel);

    *pus_scan_remain_time = us_need_scan_channel_time + us_need_work_on_home_channel_time;

    OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{dmac_scan_get_remaining_time::scan remain time[%d]ms}", *pus_scan_remain_time);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN

oal_void dmac_scan_update_gscan_vap_id(mac_vap_stru *pst_mac_vap, oal_uint8 en_is_add_vap)
{
    dmac_device_stru   *pst_dmac_device = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_update_gscan_vap_id::dmac_res_get_mac_dev[%d]fail}", pst_mac_vap->uc_device_id);
        return;
    }

    if (IS_LEGACY_STA(pst_mac_vap))
    {
        pst_dmac_device->uc_gscan_mac_vap_id = (OAL_TRUE == en_is_add_vap) ? pst_mac_vap->uc_vap_id : 0Xff;
    }
    else
    {
        OAM_ERROR_LOG3(0, OAM_SF_SCAN, "{dmac_scan_update_gscan_vap_id::vap[%d]mode[%d],not legacy sta[%d]}",
                        pst_mac_vap->uc_vap_id, pst_mac_vap->en_vap_mode, IS_LEGACY_VAP(pst_mac_vap));
    }
}


oal_uint32 dmac_trigger_gscan(oal_ipc_message_header_stru *pst_ipc_message)
{
    oal_uint8               uc_chan_idx;
    oal_uint8               uc_chan_num    = 0;
    oal_uint8               uc_channel_idx = 0;
    oal_uint32              ul_ret;
    mac_scan_req_stru       st_scan_req_params;
    dmac_gscan_params_stru *pst_ipc_scan_params;
    dmac_device_stru       *pst_dmac_device;
    mac_vap_stru           *pst_mac_vap;

    OAL_MEMZERO(&st_scan_req_params, OAL_SIZEOF(mac_scan_req_stru));

    /* 获取dmac device */
    pst_dmac_device = dmac_res_get_mac_dev(0);

    if (OAL_TRUE == dmac_device_check_is_vap_in_assoc(pst_dmac_device->pst_device_base_info))
    {
        OAM_WARNING_LOG0(pst_dmac_device->uc_gscan_mac_vap_id, OAM_SF_SCAN, "{dmac_trigger_gscan::vap is in assoc,not start gscan}");
        return OAL_FAIL;
    }

    /* 设置发起扫描的vap id,目前写死用第一个业务vap,这里注意要使用staut,不要使用aput */
    st_scan_req_params.uc_vap_id = pst_dmac_device->uc_gscan_mac_vap_id;
    pst_mac_vap = (mac_vap_stru*)mac_res_get_mac_vap(pst_dmac_device->uc_gscan_mac_vap_id);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG1(pst_dmac_device->uc_gscan_mac_vap_id, OAM_SF_SCAN, "{dmac_trigger_gscan::pst_mac_vap[%d] null.}", pst_dmac_device->uc_gscan_mac_vap_id);
        return OAL_FAIL;
    }

    /* 设置初始扫描请求的参数 */
    st_scan_req_params.en_bss_type     = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    st_scan_req_params.en_scan_type    = WLAN_SCAN_TYPE_ACTIVE;
    st_scan_req_params.en_scan_mode    = WLAN_SCAN_MODE_GNSS_SCAN;
    st_scan_req_params.us_scan_time    = WLAN_DEFAULT_ACTIVE_SCAN_TIME;
    st_scan_req_params.uc_probe_delay  = 0;
    st_scan_req_params.uc_scan_func    = MAC_SCAN_FUNC_BSS;   /* 默认扫描bss */
    st_scan_req_params.uc_max_scan_count_per_channel           = 1;  //gscan一个信道一次
    st_scan_req_params.uc_max_send_probe_req_count_per_channel = WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL;

    /* 设置扫描用的pro req的src mac地址*/
    oal_set_mac_addr(st_scan_req_params.auc_sour_mac_addr, mac_mib_get_StationID(pst_mac_vap));
    st_scan_req_params.en_is_random_mac_addr_scan = OAL_FALSE;

    /* 设置扫描请求的ssid信息 */
    st_scan_req_params.ast_mac_ssid_set[0].auc_ssid[0] = '\0';   /* 通配ssid */
    st_scan_req_params.uc_ssid_num = 1;

    /* 设置扫描请求只指定1个bssid，为广播地址 */
    oal_set_mac_addr(st_scan_req_params.auc_bssid[0], BROADCAST_MACADDR);
    st_scan_req_params.uc_bssid_num = 1;

    pst_ipc_scan_params = (dmac_gscan_params_stru *)((pst_ipc_message->ul_data_addr & 0x000fffff) + GNSS_TO_WIFI_MEM_OFFSET);
#if 0
    for (uc_chan_idx = 0; uc_chan_idx < pst_ipc_scan_params->uc_ch_valid_num; uc_chan_idx++)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_trigger_gscan::in[%d],ch num[%d]}",
                            pst_ipc_scan_params->ast_wlan_channel[uc_chan_idx].en_band, pst_ipc_scan_params->ast_wlan_channel[uc_chan_idx].uc_chan_number);
    }
#endif
    /* 目前gnss只要求2.4g扫描 */
    for (uc_chan_idx = 0; uc_chan_idx < pst_ipc_scan_params->uc_ch_valid_num; uc_chan_idx++)
    {
        ul_ret = mac_get_channel_idx_from_num(pst_ipc_scan_params->ast_wlan_channel[uc_chan_idx].en_band, pst_ipc_scan_params->ast_wlan_channel[uc_chan_idx].uc_chan_number, &uc_channel_idx);
        if (ul_ret != OAL_SUCC)
        {
            OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_trigger_gscan::in[%d]band,wrong ch num[%d]}",
                                    pst_ipc_scan_params->ast_wlan_channel[uc_chan_idx].en_band, pst_ipc_scan_params->ast_wlan_channel[uc_chan_idx].uc_chan_number);
            return OAL_FAIL;
        }
        st_scan_req_params.ast_channel_list[uc_chan_num].uc_chan_number = pst_ipc_scan_params->ast_wlan_channel[uc_chan_idx].uc_chan_number;
        st_scan_req_params.ast_channel_list[uc_chan_num].en_band        = pst_ipc_scan_params->ast_wlan_channel[uc_chan_idx].en_band;
        st_scan_req_params.ast_channel_list[uc_chan_num].uc_chan_idx    = uc_channel_idx;
        uc_chan_num++;
    }

    st_scan_req_params.uc_channel_nums = uc_chan_num;

    /* 调用扫描入口，执行扫描 */
    return dmac_scan_handle_scan_req_entry(pst_dmac_device->pst_device_base_info, MAC_GET_DMAC_VAP(pst_mac_vap), &st_scan_req_params);
}

oal_void dmac_scan_prepare_refuse_reponse_to_gnss(oal_ipc_message_header_stru *pst_req_ipc_message, oal_ipc_message_header_stru *pst_response_message)
{
    mac_vap_stru                *pst_mac_vap;
    oal_uint16                   us_scan_remain_time = 0;
    dmac_device_stru            *pst_dmac_device = dmac_res_get_mac_dev(0);

    pst_response_message->en_cmd = WLAN_REFUSE_REQUEST;

    pst_mac_vap = (mac_vap_stru*)mac_res_get_mac_vap(pst_dmac_device->uc_gscan_mac_vap_id);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        return;
    }

    /* 如果是正在扫描拒绝gnss的请求，告知他下次来获取结果的时间 */
    if (MAC_SCAN_STATE_RUNNING == pst_dmac_device->pst_device_base_info->en_curr_scan_state)
    {
        pst_response_message->en_cmd = (BFGX_REQUEST_SCAN == pst_req_ipc_message->en_cmd) ? WLAN_REFUSE_SCAN : WLAN_REFUSE_GET_DATA;
        dmac_scan_get_remaining_time(DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap), &us_scan_remain_time);
        pst_response_message->us_arg16 = us_scan_remain_time;
    }
}

oal_void dmac_handle_gscan_req_event(oal_ipc_message_header_stru *pst_req_ipc_message, oal_ipc_message_header_stru *pst_response_message)
{
    // TODO:需要加入wifi自己的判断不允许gnss扫描1、入网中2、跑流中
    if (OAL_SUCC == dmac_trigger_gscan(pst_req_ipc_message))
    {
        pst_response_message->en_cmd = WLAN_ACCEPT_SCAN;
    }
    else
    {
        dmac_scan_prepare_refuse_reponse_to_gnss(pst_req_ipc_message, pst_response_message);
    }
}

oal_void dmac_handle_gnss_get_result_event(oal_ipc_message_header_stru *pst_req_ipc_message, oal_ipc_message_header_stru *pst_response_message)
{
    oal_uint8                      uc_chan_num = 0;
    oal_uint32                     ul_current_time;
    mac_device_stru               *pst_mac_device;
    dmac_device_stru              *pst_dmac_device;
    dmac_scanned_bss_info_stru    *pst_scanned_bss_info;
    dmac_scan_for_gnss_stru       *pst_scan_for_gnss_info;
    oal_dlist_head_stru           *pst_scanned_bss_entry = OAL_PTR_NULL;
    oal_dlist_head_stru           *pst_temp_entry        = OAL_PTR_NULL;;
    dmac_gscan_report_info_stru   *pst_gscan_report_info;
    mac_vap_stru                  *pst_mac_vap;

    pst_response_message->en_cmd = WLAN_REFUSE_GET_DATA;

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(0);
    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_device->uc_device_id, OAM_SF_SCAN, "{dmac_handle_gnss_get_result_event::pst_dmac_device null.}");
        return;
    }

    pst_mac_vap = (mac_vap_stru*)mac_res_get_mac_vap(pst_dmac_device->uc_gscan_mac_vap_id);
    if ((MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state) || (OAL_PTR_NULL == pst_mac_vap))
    {
        dmac_scan_prepare_refuse_reponse_to_gnss(pst_req_ipc_message, pst_response_message);
        OAM_WARNING_LOG3(0, OAM_SF_SCAN, "{dmac_handle_gnss_get_result_event::scan mode[%d]scan state[%d],mac vap addr[0x%x]refuse gnss get data",
                                pst_mac_device->st_scan_params.en_scan_mode, pst_mac_device->en_curr_scan_state, pst_mac_vap);
        return;
    }

    pst_response_message->en_cmd = WLAN_SEND_SCAN_DATA;
    pst_gscan_report_info = (dmac_gscan_report_info_stru *)((pst_req_ipc_message->ul_data_addr & 0x000fffff) + GNSS_TO_WIFI_MEM_OFFSET);
    pst_scan_for_gnss_info = &(pst_dmac_device->st_scan_for_gnss_info);

    dmac_scan_dump_bss_list(&(pst_scan_for_gnss_info->st_dmac_scan_info_list));

    ul_current_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    pst_gscan_report_info->ul_interval_from_last_scan = OAL_TIME_GET_RUNTIME(ul_current_time, pst_scan_for_gnss_info->ul_scan_end_timstamps);
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_scanned_bss_entry, pst_temp_entry, &(pst_scan_for_gnss_info->st_dmac_scan_info_list))
    {
        /* 从链表头获得下一个需要处理的描述符 */
        pst_scanned_bss_info = OAL_DLIST_GET_ENTRY(pst_scanned_bss_entry, dmac_scanned_bss_info_stru, st_entry);
        oal_memcopy(&pst_gscan_report_info->ast_wlan_ap_measurement_info[uc_chan_num], &pst_scanned_bss_info->st_wlan_ap_measurement_info, OAL_SIZEOF(wlan_ap_measurement_info_stru));
        uc_chan_num++;
    }

    pst_gscan_report_info->uc_ap_valid_number = uc_chan_num;
}

oal_uint32 dmac_ipc_irq_event(frw_event_mem_stru *pst_event_mem)
{
    oal_ipc_message_header_stru *pst_ipc_message;
    oal_ipc_message_header_stru  st_response_ipc = {0};

    st_response_ipc.en_cmd = WLAN_REFUSE_REQUEST;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem)|| (OAL_FALSE == g_uc_wifi_support_gscan))
    {
        OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{dmac_ipc_irq_event::pst_event_mem[0x%x].,gscan_support[%d]}", pst_event_mem, g_uc_wifi_support_gscan);
        IPC_W2B_msg_tx((oal_uint8 *)&st_response_ipc, OAL_SIZEOF(oal_ipc_message_header_stru));
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ipc_message = (oal_ipc_message_header_stru *)frw_get_event_payload(pst_event_mem);
    if (BFGX_REQUEST_SCAN == pst_ipc_message->en_cmd)
    {
        dmac_handle_gscan_req_event(pst_ipc_message, &st_response_ipc);
    }
    else if (BFGX_GET_SCAN_DATA == pst_ipc_message->en_cmd)
    {
        dmac_handle_gnss_get_result_event(pst_ipc_message, &st_response_ipc);
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_ipc_irq_event::wrong cmd[0x%x]}", pst_ipc_message->en_cmd);
    }

    IPC_W2B_msg_tx((oal_uint8 *)&st_response_ipc, OAL_SIZEOF(oal_ipc_message_header_stru));

    return OAL_SUCC;
}

oal_void  dmac_scan_init_bss_info_list(mac_device_stru *pst_mac_dev)
{
    oal_uint8                            uc_scan_bss_info_ele_idx;
    dmac_device_stru                    *pst_dmac_device;
    dmac_scan_for_gnss_stru             *pst_scan_for_gnss_info;

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_dev->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_scan_init_bss_info_list::dmac_res_get_mac_dev[%d] null}", pst_mac_dev->uc_device_id);
        return;
    }

    pst_scan_for_gnss_info = &(pst_dmac_device->st_scan_for_gnss_info);
    OAL_MEMZERO(pst_scan_for_gnss_info, OAL_SIZEOF(pst_dmac_device->st_scan_for_gnss_info));

    oal_dlist_init_head(&(pst_scan_for_gnss_info->st_dmac_scan_info_list));
    oal_dlist_init_head(&(pst_scan_for_gnss_info->st_scan_info_res_list));
    for (uc_scan_bss_info_ele_idx = 0; uc_scan_bss_info_ele_idx < DMAC_SCAN_MAX_AP_NUM_TO_GNSS; uc_scan_bss_info_ele_idx++)
    {
        oal_dlist_add_tail(&(pst_scan_for_gnss_info->ast_scan_bss_info_member[uc_scan_bss_info_ele_idx].st_entry), &(pst_scan_for_gnss_info->st_scan_info_res_list));
    }
}
#if 0

oal_void  dmac_scan_free_bss_info_list(oal_dlist_head_stru  *pst_scan_info_list)
{
    dmac_scanned_bss_info_stru           *pst_scanned_bss_info;
    oal_dlist_head_stru                  *pst_dlist_entry;
    oal_dlist_head_stru                  *pst_temp               = OAL_PTR_NULL;

    /* 释放掉链表中未处理的中断信息成员 */
    oal_irq_disable();
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_dlist_entry, pst_temp, pst_scan_info_list)
    {
        pst_scanned_bss_info = OAL_DLIST_GET_ENTRY(pst_dlist_entry, dmac_scanned_bss_info_stru, st_entry);

        /* 从链表中删除 */
        oal_dlist_delete_entry(&pst_scanned_bss_info->st_entry);

        /* 释放本次的节点信息 */
        OAL_MEMZERO(pst_scanned_bss_info, OAL_SIZEOF(dmac_scanned_bss_info_stru));
    }
    oal_irq_enable();
}


OAL_STATIC oal_void dmac_scan_free_bss_info(oal_dlist_head_stru *pst_head, oal_dlist_head_stru *pst_free_bss)
{
    oal_irq_disable();
    oal_dlist_add_tail(pst_free_bss, pst_head);
    oal_irq_enable();
}
#endif

OAL_STATIC dmac_scanned_bss_info_stru *dmac_scan_alloc_bss_info(oal_dlist_head_stru *pst_head)
{
    oal_dlist_head_stru           *pst_scanned_bss_entry = OAL_PTR_NULL;
    dmac_scanned_bss_info_stru    *pst_scanned_bss_info  = OAL_PTR_NULL;

    if (!oal_dlist_is_empty(pst_head))
    {
        pst_scanned_bss_entry = oal_dlist_delete_head(pst_head);
        pst_scanned_bss_info = OAL_DLIST_GET_ENTRY(pst_scanned_bss_entry, dmac_scanned_bss_info_stru, st_entry);
        /* 存放内容区域清零 */
        OAL_MEMZERO(&(pst_scanned_bss_info->st_wlan_ap_measurement_info), OAL_SIZEOF(pst_scanned_bss_info->st_wlan_ap_measurement_info));
    }
    return pst_scanned_bss_info;
}

OAL_STATIC oal_void dmac_scan_info_add_in_order(oal_dlist_head_stru *pst_new, oal_dlist_head_stru *pst_head)
{
    dmac_scanned_bss_info_stru    *pst_scanned_bss_info;
    dmac_scanned_bss_info_stru    *pst_scanned_bss_info_new;
    oal_dlist_head_stru           *pst_scanned_bss_entry = OAL_PTR_NULL;
    oal_dlist_head_stru           *pst_temp_entry        = OAL_PTR_NULL;;

    pst_scanned_bss_info_new = OAL_DLIST_GET_ENTRY(pst_new, dmac_scanned_bss_info_stru, st_entry);
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_scanned_bss_entry, pst_temp_entry, pst_head)
    {
        /* 从链表头获得下一个需要处理的描述符 */
        pst_scanned_bss_info = OAL_DLIST_GET_ENTRY(pst_scanned_bss_entry, dmac_scanned_bss_info_stru, st_entry);
        /* 新来的相等rssi放前面 */
        if (pst_scanned_bss_info_new->st_wlan_ap_measurement_info.c_rssi >= pst_scanned_bss_info->st_wlan_ap_measurement_info.c_rssi)
        {
            break;
        }
    }

    /* 添加第一个节点和加在链表尾部 */
    if ((pst_scanned_bss_entry != OAL_PTR_NULL) && (pst_scanned_bss_entry->pst_prev != OAL_PTR_NULL))
    {
        oal_dlist_add(pst_new, pst_scanned_bss_entry->pst_prev, pst_scanned_bss_entry);
    }
    else
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_info_add_in_order::scan info list is broken !}");
    }
}

OAL_STATIC oal_void dmac_scan_dump_bss_list(oal_dlist_head_stru *pst_head)
{
    dmac_scanned_bss_info_stru    *pst_scanned_bss_info;
    wlan_ap_measurement_info_stru *pst_wlan_ap_measurement_info;
    oal_dlist_head_stru           *pst_scanned_bss_entry = OAL_PTR_NULL;
    oal_dlist_head_stru           *pst_temp_entry        = OAL_PTR_NULL;;

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_scanned_bss_entry, pst_temp_entry, pst_head)
    {
        /* 从链表头获得下一个需要处理的描述符 */
        pst_scanned_bss_info = OAL_DLIST_GET_ENTRY(pst_scanned_bss_entry, dmac_scanned_bss_info_stru, st_entry);
        pst_wlan_ap_measurement_info = &(pst_scanned_bss_info->st_wlan_ap_measurement_info);
        OAM_WARNING_LOG_ALTER(0, OAM_SF_SCAN, "{dmac_scan_dump_bss_info_list::mac addr[%x][%x][%x][%x][%x][%x]rssi[%d]channel[%d]", 8,
                      pst_wlan_ap_measurement_info->auc_bssid[0], pst_wlan_ap_measurement_info->auc_bssid[1], pst_wlan_ap_measurement_info->auc_bssid[2],
                      pst_wlan_ap_measurement_info->auc_bssid[3], pst_wlan_ap_measurement_info->auc_bssid[4], pst_wlan_ap_measurement_info->auc_bssid[5],
                      pst_wlan_ap_measurement_info->c_rssi,
                      pst_wlan_ap_measurement_info->uc_channel_num);
    }
}

OAL_STATIC oal_void dmac_scan_check_ap_bss_info(oal_dlist_head_stru *pst_head)
{
    dmac_scanned_bss_info_stru    *pst_scanned_bss_info;
    wlan_ap_measurement_info_stru *pst_wlan_ap_measurement_info;
    oal_dlist_head_stru           *pst_scanned_bss_entry = OAL_PTR_NULL;
    oal_dlist_head_stru           *pst_temp_entry        = OAL_PTR_NULL;
    oal_int8                       c_last_rssi = 0x7F;

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_scanned_bss_entry, pst_temp_entry, pst_head)
    {
        /* 从链表头获得下一个需要处理的描述符 */
        pst_scanned_bss_info = OAL_DLIST_GET_ENTRY(pst_scanned_bss_entry, dmac_scanned_bss_info_stru, st_entry);
        pst_wlan_ap_measurement_info = &(pst_scanned_bss_info->st_wlan_ap_measurement_info);
        if (c_last_rssi < pst_wlan_ap_measurement_info->c_rssi)
        {
            OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_check_ap_info::not max in head!!!}");
            dmac_scan_dump_bss_list(pst_head);
            break;
        }
        c_last_rssi = pst_wlan_ap_measurement_info->c_rssi;
    }
}


OAL_STATIC dmac_scanned_bss_info_stru *dmac_scan_find_scanned_bss_by_bssid(oal_dlist_head_stru *pst_head, oal_uint8  *puc_bssid)
{
    dmac_scanned_bss_info_stru    *pst_scanned_bss_info  = OAL_PTR_NULL;
    oal_dlist_head_stru           *pst_scanned_bss_entry = OAL_PTR_NULL;
    oal_dlist_head_stru           *pst_temp_entry        = OAL_PTR_NULL;

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_scanned_bss_entry, pst_temp_entry, pst_head)
    {
        pst_scanned_bss_info = OAL_DLIST_GET_ENTRY(pst_scanned_bss_entry, dmac_scanned_bss_info_stru, st_entry);

        /* 相同的bssid地址 */
        if (0 == oal_compare_mac_addr(pst_scanned_bss_info->st_wlan_ap_measurement_info.auc_bssid, puc_bssid))
        {
            return pst_scanned_bss_info;
        }
    }

    return OAL_PTR_NULL;
}


OAL_STATIC oal_void dmac_scan_proc_scanned_bss(mac_device_stru *pst_mac_device, oal_netbuf_stru *pst_netbuf)
{
    oal_uint8                                uc_frame_channel;
    dmac_rx_ctl_stru                        *pst_rx_ctrl;
    mac_ieee80211_frame_stru                *pst_frame_hdr;
    oal_uint8                               *puc_frame_body;
    dmac_scanned_bss_info_stru              *pst_scanned_bss_info;
    dmac_device_stru                        *pst_dmac_device;
    oal_dlist_head_stru                     *pst_scanned_bss_entry = OAL_PTR_NULL;
    oal_dlist_head_stru                     *pst_scan_info_list = OAL_PTR_NULL;
    oal_dlist_head_stru                     *pst_scan_info_res_list = OAL_PTR_NULL;
    oal_uint16                               us_frame_len;
    oal_uint16                               us_offset =  MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_device->uc_device_id, OAM_SF_SCAN, "{dmac_scan_proc_scanned_bss::pst_dmac_device null.}");
        return;
    }

    pst_rx_ctrl    = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_frame_hdr  = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctrl->st_rx_info));
    puc_frame_body = MAC_GET_RX_PAYLOAD_ADDR(pst_rx_ctrl, pst_netbuf);
    us_frame_len   = MAC_GET_RX_CB_PAYLOAD_LEN(&(pst_rx_ctrl->st_rx_info));  /*帧体长度*/

    pst_scan_info_list     = &(pst_dmac_device->st_scan_for_gnss_info.st_dmac_scan_info_list);
    pst_scan_info_res_list = &(pst_dmac_device->st_scan_for_gnss_info.st_scan_info_res_list);
    pst_scanned_bss_info = dmac_scan_find_scanned_bss_by_bssid(pst_scan_info_list, pst_frame_hdr->auc_address3);
    /* 相同bssid的已经在扫描链表里了,从链表中摘除,update并且重新add */
    if (pst_scanned_bss_info != OAL_PTR_NULL)
    {
        /* 扫到大的才更新 */
        if (pst_scanned_bss_info->st_wlan_ap_measurement_info.c_rssi >= pst_rx_ctrl->st_rx_statistic.c_rssi_dbm)
        {
            return;
        }
        oal_dlist_delete_entry(&pst_scanned_bss_info->st_entry);
    }
    else
    {
        if (OAL_TRUE == oal_dlist_is_empty(pst_scan_info_res_list))
        {
            /* 扫到大的才删去最小的 */
            pst_scanned_bss_info = OAL_DLIST_GET_ENTRY(pst_scan_info_list->pst_prev, dmac_scanned_bss_info_stru, st_entry);
            if (pst_scanned_bss_info->st_wlan_ap_measurement_info.c_rssi >= pst_rx_ctrl->st_rx_statistic.c_rssi_dbm)
            {
                return;
            }

            /* 无可用资源,从info list删除rssi最小的节点,再放回free list */
            pst_scanned_bss_entry = oal_dlist_delete_tail(pst_scan_info_list);
            oal_dlist_add_tail(pst_scanned_bss_entry, pst_scan_info_res_list);
        }
        pst_scanned_bss_info = dmac_scan_alloc_bss_info(pst_scan_info_res_list);
        if (OAL_PTR_NULL == pst_scanned_bss_info)
        {
            OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_scan_proc_scanned_bss::info res not empty but alloc fail}");
            dmac_scan_dump_bss_list(pst_scan_info_res_list);
            dmac_scan_dump_bss_list(pst_scan_info_list);
            return;
        }
    }
    /* 获取管理帧中的信道 */
    uc_frame_channel = mac_ie_get_chan_num(puc_frame_body, us_frame_len,
                       us_offset, pst_rx_ctrl->st_rx_info.uc_channel_number);

    oal_set_mac_addr(pst_scanned_bss_info->st_wlan_ap_measurement_info.auc_bssid, pst_frame_hdr->auc_address3);
    pst_scanned_bss_info->st_wlan_ap_measurement_info.c_rssi = pst_rx_ctrl->st_rx_statistic.c_rssi_dbm;
    pst_scanned_bss_info->st_wlan_ap_measurement_info.uc_channel_num = uc_frame_channel;
    dmac_scan_info_add_in_order(&(pst_scanned_bss_info->st_entry), pst_scan_info_list);
}

#endif


oal_void  dmac_scan_get_ch_statics_measurement_result_ram(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device)
{
    hal_ch_statics_irq_event_stru    st_stats_result;
    hal_ch_mac_statics_stru          st_mac_stats;
    oal_uint8                        uc_chan_idx;
    oal_uint32                       ul_trx_time_us = 0;
    hal_ch_mac_statics_stru         *pst_mac_stats;
    hal_to_dmac_device_rom_stru     *pst_hal_dev_rom;

    /* 读取结果 */
    OAL_MEMZERO(&st_stats_result, OAL_SIZEOF(st_stats_result));
    OAL_MEMZERO(&st_mac_stats, OAL_SIZEOF(st_mac_stats));

    /* MAC统计信息 */
    hal_get_txrx_frame_time(pst_hal_device, &st_mac_stats);
    hal_get_ch_statics_result(pst_hal_device, &st_stats_result);
    /* PHY测量信息 */
    hal_get_ch_measurement_result_ram(pst_hal_device, &st_stats_result);

#if defined(_PRE_WLAN_CHIP_TEST) && defined(_PRE_SUPPORT_ACS) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    dmac_acs_channel_meas_comp_handler(pst_hal_device, &st_stats_result);
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
    /* 当前的统计周期如果有PA关，则当前的统计周期不计算在内 */
    if (OAL_TRUE == pst_hal_device->en_intf_det_invalid)
    {
        if (OAL_TRUE == pst_hal_device->en_is_mac_pa_enabled)
        {
            pst_hal_device->en_intf_det_invalid = OAL_FALSE;
        }
        return;
    }
#endif

    uc_chan_idx  = GET_HAL_DEV_CURRENT_SCAN_IDX(pst_hal_device);

    /* PHY信道测量统计信息 */
    pst_hal_device->st_chan_result.uc_stats_valid = 1;
    pst_hal_device->st_chan_result.uc_channel_number = pst_mac_device->st_scan_params.ast_channel_list[uc_chan_idx].uc_chan_number;

    if (st_stats_result.c_pri20_idle_power < 0)
    {
        pst_hal_device->st_chan_result.s_free_power_stats_20M  += (oal_int8)st_stats_result.c_pri20_idle_power; /* 主20M信道空闲功率 */
        pst_hal_device->st_chan_result.s_free_power_stats_40M  += (oal_int8)st_stats_result.c_pri40_idle_power; /* 主40M信道空闲功率 */
        pst_hal_device->st_chan_result.s_free_power_stats_80M  += (oal_int8)st_stats_result.c_pri80_idle_power; /* 全部80M信道空闲功率 */
        pst_hal_device->st_chan_result.uc_free_power_cnt += 1;
    }

    /* MAC信道测量统计时间 */
    ul_trx_time_us = st_stats_result.ul_ch_rx_time_us + st_stats_result.ul_ch_tx_time_us;
    st_stats_result.ul_pri20_free_time_us = DMAC_SCAN_GET_VALID_FREE_TIME(ul_trx_time_us, st_stats_result.ul_ch_stats_time_us, st_stats_result.ul_pri20_free_time_us);
    st_stats_result.ul_pri40_free_time_us = DMAC_SCAN_GET_VALID_FREE_TIME(ul_trx_time_us, st_stats_result.ul_ch_stats_time_us, st_stats_result.ul_pri40_free_time_us);
    st_stats_result.ul_pri80_free_time_us = DMAC_SCAN_GET_VALID_FREE_TIME(ul_trx_time_us, st_stats_result.ul_ch_stats_time_us, st_stats_result.ul_pri80_free_time_us);

    pst_hal_device->st_chan_result.ul_total_free_time_20M_us += st_stats_result.ul_pri20_free_time_us;
    pst_hal_device->st_chan_result.ul_total_free_time_40M_us += st_stats_result.ul_pri40_free_time_us;
    pst_hal_device->st_chan_result.ul_total_free_time_80M_us += st_stats_result.ul_pri80_free_time_us;
    pst_hal_device->st_chan_result.ul_total_recv_time_us     += st_stats_result.ul_ch_rx_time_us;
    pst_hal_device->st_chan_result.ul_total_send_time_us     += st_stats_result.ul_ch_tx_time_us;
    pst_hal_device->st_chan_result.ul_total_stats_time_us    += st_stats_result.ul_ch_stats_time_us;

    /* 统计MAC FCS正确帧时间 */
    pst_hal_dev_rom = (hal_to_dmac_device_rom_stru*)pst_hal_device->_rom;
    pst_mac_stats = &pst_hal_dev_rom->st_mac_ch_stats;

    ul_trx_time_us = st_mac_stats.ul_rx_direct_time + st_mac_stats.ul_tx_time;
    st_mac_stats.ul_rx_nondir_time = DMAC_SCAN_GET_VALID_FREE_TIME(ul_trx_time_us, st_stats_result.ul_ch_stats_time_us, st_mac_stats.ul_rx_nondir_time);
    pst_mac_stats->ul_rx_direct_time    += st_mac_stats.ul_rx_direct_time;
    pst_mac_stats->ul_rx_nondir_time    += st_mac_stats.ul_rx_nondir_time;
    pst_mac_stats->ul_tx_time           += st_mac_stats.ul_tx_time;

}


oal_uint32 dmac_scan_channel_statistics_complete(frw_event_mem_stru *pst_event_mem)
{
    mac_device_stru                 *pst_mac_device;
    frw_event_stru                  *pst_event;
    oal_uint16                       us_total_scan_time_per_chan;
    oal_uint8                        uc_do_meas;        /* 本次扫描是否要获取信道测量的结果 */
    oal_uint8                        uc_chan_stats_cnt;
    hal_to_dmac_device_stru         *pst_hal_device;


    /* 寻找对应的DEVICE结构以及相应的ACS结构 */
    pst_event = frw_get_event_stru(pst_event_mem);

    hal_get_hal_to_dmac_device(pst_event->st_event_hdr.uc_chip_id, pst_event->st_event_hdr.uc_device_id, &pst_hal_device);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG2(0, OAM_SF_SCAN, "{dmac_scan_channel_statistics_complete::pst_hal_device null.chip id[%d],device id[%d]}",
                            pst_event->st_event_hdr.uc_chip_id, pst_event->st_event_hdr.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_scan_channel_statistics_complete::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 读取硬件统计 测量结果 */
    uc_do_meas = pst_mac_device->st_scan_params.uc_scan_func & (MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS);
    if (uc_do_meas)
    {
        dmac_scan_get_ch_statics_measurement_result_ram(pst_mac_device, pst_hal_device);
    }

    /* 每次硬件测量时间是10ms，根据本次扫描时长，确定要开启多少次硬件测量 */
    pst_hal_device->st_chan_result.uc_stats_cnt++;
    uc_chan_stats_cnt = pst_hal_device->st_chan_result.uc_stats_cnt;

    us_total_scan_time_per_chan = (oal_uint16)(pst_mac_device->st_scan_params.us_scan_time * pst_mac_device->st_scan_params.uc_max_scan_count_per_channel);
    if (uc_chan_stats_cnt * DMAC_SCAN_CHANNEL_MEAS_PERIOD_MS < us_total_scan_time_per_chan)
    {
        /* 使能信道测量前清空MAC统计信息,信道测量统计MAC/PHY自动清空 */
        hal_set_mac_clken(pst_hal_device, OAL_TRUE);
        hal_set_counter1_clear(pst_hal_device);
        hal_set_mac_clken(pst_hal_device, OAL_FALSE);

        /* 再次启动一次测量 */
        hal_set_ch_statics_period(pst_hal_device, DMAC_SCAN_CHANNEL_STATICS_PERIOD_US);
        hal_set_ch_measurement_period(pst_hal_device, DMAC_SCAN_CHANNEL_MEAS_PERIOD_MS);
        hal_enable_ch_statics(pst_hal_device, 1);
    }
    else
    {
        /* CCA检测使用信道测量中断内判断结束和正常扫描解耦 */
        if (WLAN_SCAN_MODE_BACKGROUND_CCA == pst_mac_device->st_scan_params.en_scan_mode)
        {
            if (OAL_PTR_NULL != pst_mac_device->st_scan_params.p_fn_cb)
            {
                pst_mac_device->st_scan_params.p_fn_cb(pst_mac_device);
                dmac_scan_calcu_channel_ratio(pst_hal_device);
                //信道测量功能需要整改，暂时在此处插入bsd特性的接口来处理信道测量的结果
        #ifdef _PRE_WLAN_FEATURE_BAND_STEERING
                dmac_bsd_device_load_scan_cb(pst_mac_device);
        #endif
            }
        }
    }

    return OAL_SUCC;
}

/*lint -e19 */

oal_module_symbol(dmac_scan_switch_channel_off);
oal_module_symbol(dmac_scan_begin);
oal_module_symbol(dmac_scan_abort);
oal_module_symbol(dmac_scan_switch_channel_back);
#ifdef _PRE_WLAN_FEATURE_CCA_OPT
oal_module_symbol(dmac_scan_handle_scan_req_entry);
#endif

/*lint +e19 */
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
