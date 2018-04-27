


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_sdio.h"
#include "oal_mem.h"
#include "oal_types.h"
#include "oal_workqueue.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "oal_hcc_slave_if.h"
#endif
#ifdef _PRE_WLAN_PROFLING_MIPS
#include "oal_profiling.h"
#endif
#if (defined(_PRE_PLAT_FEATURE_CUSTOMIZE) && defined(_PRE_PRODUCT_ID_HI110X_DEV))
#include "hal_mac.h"
#endif

#ifdef _PRE_WLAN_ALG_ENABLE
#include "alg_ext_if.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
//#include "alg_tpc.h"
#endif
#endif
#include "mac_data.h"
#include "mac_device.h"
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_vap.h"
#include "dmac_ext_if.h"
#include "dmac_main.h"
#include "dmac_vap.h"
#include "dmac_psm_ap.h"
#include "dmac_rx_data.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_mgmt_sta.h"
#include "dmac_mgmt_ap.h"
#include "dmac_tx_complete.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_uapsd.h"
#include "dmac_blockack.h"
#include "dmac_beacon.h"
#include "dmac_user.h"
#include "dmac_11i.h"
#include "dmac_wep.h"
#include "dmac_uapsd.h"
#include "dmac_reset.h"
#include "dmac_config.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_txopps.h"
#include "dmac_dft.h"
#include "dmac_beacon.h"
#include "dmac_scan.h"
#include "dmac_psm_ap.h"
#include "dmac_device.h"
#include "dmac_resource.h"

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
#include "cali_data.h"
#include "pm_extern.h"
#endif
#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_test_main.h"
#include "dmac_lpm_test.h"
#include "dmac_frame_filter_test.h"
#include "dmac_wmm_test.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
//#ifdef _PRE_WLAN_PRODUCT_1151V200
//#include "hal_witp_phy_reg_1151v2.h"
//#else
//#include "hal_witp_phy_reg.h"
//#endif
//#include "hal_witp_pa_reg.h"
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "hal_phy_reg.h"
#include "hal_mac_reg.h"
#endif
#endif

#ifdef _PRE_WIFI_DMT
#include "hal_witp_dmt_if.h"
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#include "pm_extern.h"
#include "dmac_sta_pm.h"
#endif

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

#include "dmac_11w.h"
#endif
#include "dmac_chan_mgmt.h"

#include "dmac_reset.h"
#include "oal_net.h"
#include "dmac_config.h"
#include "dmac_main.h"
#include "dmac_rx_filter.h"

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "core_cr4.h"
#endif
#include "oal_profiling.h"

#ifdef _PRE_SUPPORT_ACS
#include "dmac_acs.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
#include "mac_board.h"
#endif

#include "dmac_arp_offload.h"

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
#include "dmac_auto_adjust_freq.h"
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "dmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "dmac_smps.h"
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#include "dmac_bsd.h"
#endif

#if defined(_PRE_DEBUG_MODE) && defined(_PRE_WLAN_FEATURE_11V)
#include "dmac_11v.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CONFIG_DEBUG_ROM_C

#if defined(_PRE_WLAN_FEATURE_MEMORY_USAGE_TRACE) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifndef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
#error "mu-traffic-ctl should be defined when using netbuff memory trace."
#endif
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
oal_uint32 g_ul_al_mpdu_len                = 1510; /*指示常发mpdu长度， 可根据实际情况修改*/

extern mac_phy_debug_switch_stru  g_st_mac_phy_debug_switch;
mac_phy_debug_switch_stru  *g_pst_mac_phy_debug_switch = &g_st_mac_phy_debug_switch;


oal_uint32  dmac_config_pause_tid(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_pause_tid_param_stru   *pst_pause_tid_param;
    dmac_user_stru                 *pst_dmac_user;
    oal_uint8                       uc_tid;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_pause_tid_param = (mac_cfg_pause_tid_param_stru *)puc_param;
    uc_tid = pst_pause_tid_param->uc_tid;

    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_pause_tid_param->auc_mac_addr);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pause_tid::pst_dmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pause_tid::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if  (OAL_TRUE == pst_pause_tid_param->uc_is_paused)
    {
        dmac_tid_pause(&(pst_dmac_user->ast_tx_tid_queue[uc_tid]), DMAC_TID_PAUSE_RESUME_TYPE_BA);
    }
    else
    {
        g_st_dmac_tid_rom_cb.dmac_tid_resume_cb(pst_hal_device, &(pst_dmac_user->ast_tx_tid_queue[uc_tid]), DMAC_TID_PAUSE_RESUME_TYPE_BA);
    }

    return OAL_SUCC;
}

oal_uint32  dmac_config_send_bar(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_pause_tid_param_stru   *pst_pause_tid_param;
    dmac_user_stru                 *pst_dmac_user;
    oal_uint8                       uc_tid;
    dmac_tid_stru                  *pst_tid;

    pst_pause_tid_param = (mac_cfg_pause_tid_param_stru *)puc_param;
    uc_tid = pst_pause_tid_param->uc_tid;

    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_pause_tid_param->auc_mac_addr);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_bar::pst_dmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_tid = &pst_dmac_user->ast_tx_tid_queue[uc_tid];

    if (OAL_PTR_NULL == pst_tid->pst_ba_tx_hdl || DMAC_BA_COMPLETE != pst_tid->pst_ba_tx_hdl->en_ba_conn_status)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_bar::ba not established.}");
        return OAL_FAIL;
    }

    return g_st_dmac_ba_rom_cb.dmac_ba_send_bar_cb(pst_tid->pst_ba_tx_hdl, pst_dmac_user, pst_tid);
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_config_dump_rx_dscr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifndef _PRE_WLAN_PROFLING_MIPS
    hal_to_dmac_device_stru         *pst_hal_device;
    oal_uint32                       ul_value;
    oal_uint32                       ul_rx_dscr_len = 0;
#ifdef _PRE_DEBUG_MODE
    oal_uint32                       ul_loop;
    oal_uint32                      *pul_original_dscr;
    oal_uint32                       ul_dscr_num;
    oal_uint32                      *pul_curr_dscr;
#endif
    oal_dlist_head_stru  *pst_dlist_entry;
    hal_rx_dscr_stru     *pst_dscr;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_rx_dscr::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_value = *((oal_uint32 *)puc_param);

    if (ul_value >= HAL_RX_DSCR_QUEUE_ID_BUTT)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{dmac_config_dump_rx_dscr::invalid rx dscr queue[%d].}", ul_value);
        return OAL_FAIL;
    }

    hal_rx_get_size_dscr(pst_hal_device, &ul_rx_dscr_len);

#ifdef _PRE_DEBUG_MODE
    /* 将描述符原始地址打出来 */
    OAM_INFO_LOG0(0, OAM_SF_CFG, "the origin dscr phy addr and virtual addr:\n");

    if (HAL_RX_DSCR_NORMAL_PRI_QUEUE == ul_value)
    {
        pul_original_dscr = pst_hal_device->aul_nor_rx_dscr;
        ul_dscr_num       = HAL_NORMAL_RX_MAX_BUFFS;
    }
    else
    {
        pul_original_dscr = pst_hal_device->aul_hi_rx_dscr;
        ul_dscr_num       = HAL_HIGH_RX_MAX_BUFFS;
    }

    for (ul_loop = 0; ul_loop < ul_dscr_num; ul_loop++)
    {
        pul_curr_dscr = (oal_uint32 *)OAL_DSCR_PHY_TO_VIRT(pul_original_dscr[ul_loop]);
        OAM_INFO_LOG3(0, OAM_SF_CFG, "%2d 0x%08x, 0x%08x\n",
                     ul_loop,
                     pul_original_dscr[ul_loop],
                     (oal_uint32)pul_curr_dscr);
        oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pul_curr_dscr, (oal_uint16)ul_rx_dscr_len, OAM_OTA_RX_DSCR_TYPE);
    }

    OAM_INFO_LOG1(0, OAM_SF_CFG, "dscr exception free cnt is %d\n", pst_hal_device->ul_exception_free);
#endif


    OAL_DLIST_SEARCH_FOR_EACH(pst_dlist_entry, &pst_hal_device->ast_rx_dscr_queue[ul_value].st_header)
    {
        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dlist_entry, hal_rx_dscr_stru, st_entry);

        OAM_INFO_LOG2(0, OAM_SF_CFG, "virtual addr:0x%08x, phy addr:0x%08x\n", (oal_uint32)pst_dscr, (oal_uint32)HAL_RX_DSCR_GET_REAL_ADDR((oal_uint32*)pst_dscr));
        oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_dscr, (oal_uint16)ul_rx_dscr_len, OAM_OTA_RX_DSCR_TYPE);
    }

    hal_rx_show_dscr_queue_info(pst_hal_device, (oal_uint8)ul_value);
#endif
    return OAL_SUCC;
}

#else
oal_uint32  dmac_config_dump_rx_dscr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifndef _PRE_WLAN_PROFLING_MIPS

    hal_rx_dscr_queue_header_stru   *pst_rx_dscr_queue;
    hal_to_dmac_device_stru         *pst_hal_device;
    oal_uint32                      *pul_curr_dscr;
    hal_rx_dscr_stru               *pst_hal_to_dmac_dscr;
    oal_uint32                       ul_value;
    oal_uint32                       ul_rx_dscr_len = 0;
#ifdef _PRE_DEBUG_MODE
    oal_uint32                       ul_loop;
    oal_uint32                      *pul_original_dscr;
    oal_uint32                       ul_dscr_num;
#endif

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_rx_dscr::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_value = *((oal_uint32 *)puc_param);

    if (ul_value >= HAL_RX_DSCR_QUEUE_ID_BUTT)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{dmac_config_dump_rx_dscr::invalid rx dscr queue[%d].}", ul_value);
        return OAL_FAIL;
    }

    hal_rx_get_size_dscr(pst_hal_device, &ul_rx_dscr_len);

#ifdef _PRE_DEBUG_MODE
    /* 将描述符原始地址打出来 */
    OAM_INFO_LOG0(0, OAM_SF_CFG, "the origin dscr phy addr and virtual addr:\n");

    if (HAL_RX_DSCR_NORMAL_PRI_QUEUE == ul_value)
    {
        pul_original_dscr = pst_hal_device->aul_nor_rx_dscr;
        ul_dscr_num       = HAL_NORMAL_RX_MAX_BUFFS;
    }
    else
    {
        pul_original_dscr = pst_hal_device->aul_hi_rx_dscr;
        ul_dscr_num       = HAL_HIGH_RX_MAX_BUFFS;
    }

    for (ul_loop = 0; ul_loop < ul_dscr_num; ul_loop++)
    {
        pul_curr_dscr = (oal_uint32 *)OAL_DSCR_PHY_TO_VIRT(pul_original_dscr[ul_loop]);
        OAM_INFO_LOG3(0, OAM_SF_CFG, "%2d 0x%08x, 0x%08x\n",
                     ul_loop,
                     pul_original_dscr[ul_loop],
                     (oal_uint32)pul_curr_dscr);
        oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pul_curr_dscr, (oal_uint16)ul_rx_dscr_len, OAM_OTA_RX_DSCR_TYPE);
    }

    OAM_INFO_LOG1(0, OAM_SF_CFG, "dscr exception free cnt is %d\n", pst_hal_device->ul_exception_free);
#endif

    pst_rx_dscr_queue = &(pst_hal_device->ast_rx_dscr_queue[ul_value]);

    pul_curr_dscr = pst_rx_dscr_queue->pul_element_head;

    //OAM_INFO_LOG1(0, OAM_SF_CFG, "the current dscr cnt is: %d\n", pst_rx_dscr_queue->us_element_cnt);
    //OAM_INFO_LOG1(0, OAM_SF_CFG, "head ptr virtual addr is: 0x%08x\n", (oal_uint32)pst_rx_dscr_queue->pul_element_head);
    //OAM_INFO_LOG1(0, OAM_SF_CFG, "tail ptr virtual addr is: 0x%08x\n", (oal_uint32)pst_rx_dscr_queue->pul_element_tail);

    OAM_INFO_LOG0(0, OAM_SF_CFG, "the current dscr addr and dscr's content:\n");
    while(OAL_PTR_NULL != pul_curr_dscr)
    {
        OAM_INFO_LOG2(0, OAM_SF_CFG, "virtual addr:0x%08x, phy addr:0x%08x\n", (oal_uint32)pul_curr_dscr, (oal_uint32)OAL_DSCR_VIRT_TO_PHY(HAL_RX_DSCR_GET_REAL_ADDR(pul_curr_dscr)));
        oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pul_curr_dscr, (oal_uint16)ul_rx_dscr_len, OAM_OTA_RX_DSCR_TYPE);
        pst_hal_to_dmac_dscr = (hal_rx_dscr_stru *)pul_curr_dscr;
        if (NULL != pst_hal_to_dmac_dscr->pul_next_rx_dscr)
        {
            pul_curr_dscr = HAL_RX_DSCR_GET_SW_ADDR((oal_uint32 *)OAL_DSCR_PHY_TO_VIRT((oal_uint)(pst_hal_to_dmac_dscr->pul_next_rx_dscr)));
        }
        else
        {
            pul_curr_dscr = HAL_RX_DSCR_GET_SW_ADDR(pst_hal_to_dmac_dscr->pul_next_rx_dscr);
        }
    }

    hal_rx_show_dscr_queue_info(pst_hal_device, (oal_uint8)ul_value);
#endif
    return OAL_SUCC;
}
#endif

oal_uint32  dmac_config_rssi_limit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_device_stru                    *pst_dmac_device;
    mac_cfg_rssi_limit_stru             *pst_rssi_limit;

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
       OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_rssi_limit::pst_dmac_device null.}");
       return OAL_ERR_CODE_PTR_NULL;
    }

    pst_rssi_limit = (mac_cfg_rssi_limit_stru *)puc_param;

    switch(pst_rssi_limit->en_rssi_limit_type)
    {
        case MAC_RSSI_LIMIT_SHOW_INFO:
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_rssi_limit::[show_info] device_id[%d], rssi_limit_flag[%d], rssi_limit[%d], rssi_limit_delta[%d].}",
                        pst_mac_vap->uc_device_id, pst_dmac_device->st_rssi_limit.en_rssi_limit_enable_flag,
                        (oal_int32)pst_dmac_device->st_rssi_limit.c_rssi, (oal_int32)pst_dmac_device->st_rssi_limit.c_rssi_delta);
            break;
        case MAC_RSSI_LIMIT_ENABLE:
            pst_dmac_device->st_rssi_limit.en_rssi_limit_enable_flag = pst_rssi_limit->en_rssi_limit_enable_flag;
            break;
        case MAC_RSSI_LIMIT_DELTA:
            pst_dmac_device->st_rssi_limit.c_rssi_delta = pst_rssi_limit->c_rssi_delta;
            break;
        case MAC_RSSI_LIMIT_THRESHOLD:
            pst_dmac_device->st_rssi_limit.c_rssi = pst_rssi_limit->c_rssi;
            break;
        default:
            break;
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_DFT_STAT

oal_uint32  dmac_config_report_irq_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_user_keepalive_timer::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_show_irq_info(pst_hal_device, *puc_param);
#endif
    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 dmac_config_set_stbc_cap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    oal_bool_enum_uint8           uc_value;
    mac_device_stru              *pst_mac_device = OAL_PTR_NULL;

    uc_value = (oal_bool_enum_uint8)*puc_param;
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_stbc_cap::mac_device NULL pointer.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_value > 1)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_stbc_cap::stbc_value is limit! value = [%d].}", uc_value);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if (pst_mac_vap->en_vap_rx_nss >= WLAN_DOUBLE_NSS)
    {
        mac_mib_set_TxSTBCOptionImplemented(pst_mac_vap, uc_value);
        mac_mib_set_TxSTBCOptionActivated(pst_mac_vap, uc_value);
        mac_mib_set_VHTTxSTBCOptionImplemented(pst_mac_vap, uc_value);
    }
    else
    {
        mac_mib_set_TxSTBCOptionImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_TxSTBCOptionActivated(pst_mac_vap, OAL_FALSE);
        mac_mib_set_VHTTxSTBCOptionImplemented(pst_mac_vap, OAL_FALSE);
    }

    mac_mib_set_RxSTBCOptionImplemented(pst_mac_vap, uc_value);
    mac_mib_set_VHTRxSTBCOptionImplemented(pst_mac_vap, uc_value);

    return OAL_SUCC;
}

oal_uint32 dmac_config_set_ldpc_cap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_bool_enum_uint8   uc_value;

    uc_value = (oal_bool_enum_uint8)(*puc_param);

    if (uc_value > 1)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_ldpc_cap::ldpc_value is limit! value = [%d].}\r\n", uc_value);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    mac_mib_set_LDPCCodingOptionImplemented(pst_mac_vap, uc_value);
    mac_mib_set_LDPCCodingOptionActivated(pst_mac_vap, uc_value);
    mac_mib_set_VHTLDPCCodingOptionImplemented(pst_mac_vap, uc_value);

    return OAL_SUCC;
}

oal_uint32  dmac_config_dump_timer(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    frw_timer_dump_timer(pst_mac_vap->ul_core_id);
#if 0
    oal_dlist_head_stru *pst_entry;
    oal_dlist_head_stru *pst_user_list_head;
    mac_user_stru       *pst_user_tmp;
    dmac_user_stru      *pst_dmac_user_tmp;

    pst_user_list_head = &(pst_mac_vap->st_mac_user_list_head);

    /* 遍历VAP下面的用户，dump用户last active timestamp  */
    for (pst_entry = pst_user_list_head->pst_next; pst_entry != pst_user_list_head;)
    {
        pst_user_tmp      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        pst_dmac_user_tmp = mac_res_get_dmac_user(pst_user_tmp->us_assoc_id);

        /* 指向双向链表下一个节点 */
        pst_entry = pst_entry->pst_next;

        if (OAL_PTR_NULL == pst_dmac_user_tmp)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "dmac_config_dump_timer::user is null. us_assoc_id is %d", pst_user_tmp->us_assoc_id);
            continue;
        }

        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "dmac_config_dump_timer::us_assoc_id is %d, last_active_timestamp[%u]",
                         pst_user_tmp->us_assoc_id, pst_dmac_user_tmp->ul_last_active_timestamp);

    }
#endif
    return OAL_SUCC;
}

#else

oal_uint32  dmac_config_phy_stat_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oam_stats_phy_stat_stru         st_phy_stat;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_phy_stat_info::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(&st_phy_stat, OAL_SIZEOF(oam_stats_phy_stat_stru));

    hal_get_phy_stat_info(pst_hal_device, &st_phy_stat);

    /* 将获取到的统计值上报 */
    return oam_report_dft_params(BROADCAST_MACADDR, (oal_uint8 *)&st_phy_stat,(oal_uint16)OAL_SIZEOF(oam_stats_phy_stat_stru), OAM_OTA_TYPE_PHY_STAT);
}



oal_uint32  dmac_config_machw_stat_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oam_stats_machw_stat_stru        st_machw_stat;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_machw_stat_info::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == *puc_param)
    {
        /* 清零mac统计,mac统计清零寄存器的bit2~bit22,bit4是发送beacon帧数目，bit9是高优先级队列发送数目，不清零，
           这两个统计值放在管理帧统计功能控制
        */
        hal_set_counter_clear_value(pst_hal_device, 0x7FFDEC);

        return OAL_SUCC;
    }
    else
    {
        OAL_MEMZERO(&st_machw_stat, OAL_SIZEOF(oam_stats_machw_stat_stru));

        /* 从MAC寄存器获取统计值 */
        dmac_dft_get_machw_stat_info(pst_hal_device, &st_machw_stat);
        return oam_report_dft_params(BROADCAST_MACADDR, (oal_uint8 *)&st_machw_stat,(oal_uint16)OAL_SIZEOF(oam_stats_machw_stat_stru), OAM_OTA_TYPE_MACHW_STAT);
    }
}


oal_uint32  dmac_config_report_mgmt_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                 *pst_mac_dev;
    oam_stats_mgmt_stat_stru         st_mgmt_stat;
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_report_mgmt_stat::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_report_mgmt_stat::dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == *puc_param)
    {
        /* 清零mac统计的发送beacon帧数目、高优先级队列发送数目:MAC统计寄存器的bit4和bit9 */
        hal_set_counter_clear_value(pst_hal_device, 0x210);

        /* 清零软件统计的管理帧收发数目 */
        OAL_MEMZERO(&pst_mac_dev->st_mgmt_stat, OAL_SIZEOF(mac_device_mgmt_statistic_stru));

        return OAL_SUCC;
    }
    else
    {
        /* 获取mac统计的发送beacon帧数目和高优先级队列发送数目 */
        hal_get_tx_bcn_count(pst_hal_device, &st_mgmt_stat.ul_machw_stat_tx_bcn_cnt);
        hal_get_tx_hi_pri_mpdu_cnt(pst_hal_device, &st_mgmt_stat.ul_machw_stat_tx_hi_cnt);

        /* 获取软件的管理帧收发统计 */
        oal_memcopy(st_mgmt_stat.aul_sw_rx_mgmt_cnt,
                    &pst_mac_dev->st_mgmt_stat,
                    OAL_SIZEOF(mac_device_mgmt_statistic_stru));

        /* 将统计值上报 */
        return oam_report_dft_params(BROADCAST_MACADDR, (oal_uint8 *)&st_mgmt_stat,(oal_uint16)OAL_SIZEOF(oam_stats_mgmt_stat_stru), OAM_OTA_TYPE_MGMT_STAT);
    }
}

oal_uint32  dmac_config_usr_queue_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_usr_queue_param_stru        *pst_usr_queue_param;
    dmac_user_stru                      *pst_dmac_user;

    pst_usr_queue_param = (mac_cfg_usr_queue_param_stru *)puc_param;

    /* 获取用户 */
    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_usr_queue_param->auc_user_macaddr);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_usr_queue_stat::dmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == pst_usr_queue_param->uc_param)
    {
        /* 清空用户队列统计信息 */
        return dmac_dft_clear_usr_queue_stat(pst_dmac_user);
    }
    else
    {
        return dmac_dft_report_usr_queue_stat(pst_dmac_user);
    }
}

#endif

oal_uint32  dmac_config_report_all_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int8             *pc_token_type;
    oal_int8             *pc_token;
    oal_int8             *pc_end;
    oal_int8             *pc_ctx;
    oal_int8             *pc_sep = " ";
    oal_uint8             uc_val;

    /* 获取要读取的寄存器类型 */
    pc_token_type = oal_strtok((oal_int8 *)puc_param, pc_sep, &pc_ctx);
    if (NULL == pc_token_type)
    {
        return OAL_FAIL;
    }
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (NULL == pc_token)
    {
        return OAL_FAIL;
    }

    uc_val = (oal_uint8)oal_strtol(pc_token, &pc_end, 16);

    if (0 == oal_strcmp(pc_token, "soc"))
    {
        if (OAL_TRUE == uc_val)
        {
            dmac_config_phy_stat_info(pst_mac_vap, uc_len, &uc_val);
        }
    }
    else if (0 == oal_strcmp(pc_token, "machw"))
    {
        dmac_config_machw_stat_info(pst_mac_vap, uc_len, &uc_val);
    }
    else if (0 == oal_strcmp(pc_token, "mgmt"))
    {
        dmac_config_report_mgmt_stat(pst_mac_vap, uc_len, &uc_val);
    }
    else if (0 == oal_strcmp(pc_token, "irq"))
    {
        dmac_config_report_irq_info(pst_mac_vap, uc_len, &uc_val);
    }
    else
    {
        dmac_config_report_irq_info(pst_mac_vap, uc_len, &uc_val);
        dmac_config_report_mgmt_stat(pst_mac_vap, uc_len, &uc_val);
        dmac_config_machw_stat_info(pst_mac_vap, uc_len, &uc_val);
        if (OAL_TRUE == uc_val)
        {
            dmac_config_phy_stat_info(pst_mac_vap, uc_len, &uc_val);
        }
    }
    return OAL_SUCC;
}
#endif

oal_uint32  dmac_config_set_nss(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_tx_comp_stru            *pst_event_set_nss;
    dmac_vap_stru                   *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_nss::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }


    /* 设置数据 */
    pst_event_set_nss = (mac_cfg_tx_comp_stru *)puc_param;
    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_nss_mode = pst_event_set_nss->uc_param;
    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = pst_event_set_nss->en_protocol_mode;

    /* 更新协议速率 */
    pst_dmac_vap->uc_protocol_rate_dscr = (oal_uint8)((pst_event_set_nss->en_protocol_mode << 6) | pst_event_set_nss->uc_param);

    if (OAL_SWITCH_ON == pst_dmac_vap->pst_hal_device->bit_al_tx_flag)
    {
        hal_set_tx_dscr_field(pst_dmac_vap->pst_hal_device, pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].ul_value, HAL_RF_TEST_DATA_RATE_ZERO);
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_mcs::tx dscr nss=%d.", pst_event_set_nss->uc_param);

    return OAL_SUCC;
}
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

oal_uint32 dmac_config_enable_pmf(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_bool_enum_uint8        en_pmf_active;
    wlan_pmf_cap_status_uint8 *puc_pmf_cap;
    oal_dlist_head_stru       *pst_entry;
    mac_user_stru             *pst_user_tmp;
    dmac_vap_stru             *pst_dmac_vap;
    mac_device_stru           *pst_device;

    puc_pmf_cap = (wlan_pmf_cap_status_uint8 *)puc_param;
    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG2(0, OAM_SF_PMF, "{dmac_config_enable_pmf:: pointer is null,pst_mac_vap[%d], puc_param[%d] .}", pst_mac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_PMF, "{dmac_config_enable_pmf::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    switch(*puc_pmf_cap)
    {
        case MAC_PMF_DISABLED:
        {
            mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_FALSE);
            mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_FALSE);
            mac_mib_set_rsnaactivated(pst_mac_vap, OAL_FALSE);
            en_pmf_active = OAL_FALSE;

             /* 配置pmf的加解密总开关 */
            hal_set_pmf_crypto(pst_dmac_vap->pst_hal_vap, OAL_FALSE);
        }
        break;
        /* enable状态，不改变现有user和硬件的 */
        case MAC_PMF_ENABLED:
        {
            mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_TRUE);
            mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_FALSE);
            mac_mib_set_rsnaactivated(pst_mac_vap, OAL_TRUE);
            return OAL_SUCC;
        }
        case MAC_PMF_REQUIRED:
        {
            mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_TRUE);
            mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_TRUE);
            mac_mib_set_rsnaactivated(pst_mac_vap, OAL_TRUE);
            en_pmf_active = OAL_TRUE;

             /* 配置pmf的加解密总开关 */
            hal_set_pmf_crypto(pst_dmac_vap->pst_hal_vap, OAL_TRUE);

        }
        break;
        default:
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_PMF, "{dmac_config_enable_pmf: commend error!}");
            return OAL_FALSE;
        }
    }

    if (MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state)
    {
        OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
        {
            pst_user_tmp      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
            /*lint -save -e774 */
            if (OAL_PTR_NULL == pst_user_tmp)
            {
                OAM_ERROR_LOG0(0, OAM_SF_PMF, "dmac_config_enable_pmf:: pst_user_tmp is null");
                return OAL_ERR_CODE_PTR_NULL;
            }
            /*lint -restore */

            mac_user_set_pmf_active(pst_user_tmp, en_pmf_active);
        }
    }

    return OAL_SUCC;
}
#endif

oal_uint32  dmac_config_set_user_vip(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_user_vip_param_stru    *pst_user_vip_param;
    dmac_user_stru                 *pst_dmac_user;

    pst_user_vip_param = (mac_cfg_user_vip_param_stru *)puc_param;

    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_user_vip_param->auc_mac_addr);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_user_vip::pst_dmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_user->en_vip_flag = pst_user_vip_param->uc_vip_flag;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_user_vip, vip_flag = %d}", pst_dmac_user->en_vip_flag);

    return OAL_SUCC;
}


oal_uint32  dmac_config_set_vap_host(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                  *pst_dmac_vap;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_vap_host::param null.}");
         return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap->en_is_host_vap = *puc_param;

    OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_vap_host, host_flag = %d}", pst_dmac_vap->en_is_host_vap);

    return OAL_SUCC;
}

oal_uint32  dmac_config_dump_ba_bitmap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifdef _PRE_DEBUG_MODE
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
//#if ((_PRE_TARGET_PRODUCT_TYPE_5610DMB != _PRE_CONFIG_TARGET_PRODUCT) )
#if 0
        hal_to_dmac_device_stru          *pst_hal_device;
        dmac_vap_stru                    *pst_dmac_vap;
	 dmac_user_stru			 *pst_dmac_user;
        oal_uint16                        us_track_index;
        oal_uint16                        us_seq_index;
        oal_uint8                         uc_num = 0;
        oal_uint8                         uc_lut_idx;
        dmac_tx_ba_track_stru            *pst_tx_ba_track;
        mac_cfg_mpdu_ampdu_tx_param_stru *pst_aggr_tx_on_param;
        oal_file                         *pst_fs = OAL_PTR_NULL;
        oal_mm_segment_t                  old_fs;
        dmac_seq_track_stru               *pst_seq_track = OAL_PTR_NULL;

        pst_aggr_tx_on_param = (mac_cfg_mpdu_ampdu_tx_param_stru *)puc_param;

        pst_dmac_vap    = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
        pst_hal_device  = pst_dmac_vap->pst_hal_device;
        pst_dmac_user = (dmac_user_stru *)mac_vap_get_user_by_addr(pst_mac_vap, pst_aggr_tx_on_param->auc_ra_mac);

	 if (OAL_PTR_NULL == pst_dmac_user)
	 {
	     OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_ba_bitmap::dmac_user_null!\n.}");
	     return OAL_FAIL;
	 }

	 if ( (pst_aggr_tx_on_param->uc_tid >= WLAN_TIDNO_BUTT) || (pst_aggr_tx_on_param->uc_tid < WLAN_TIDNO_BEST_EFFORT) )
	 {
	     OAM_ERROR_LOG1(0, OAM_SF_BA, "{dmac_config_dump_ba_bitmap::invalid input tid number[%d]\n.}", pst_aggr_tx_on_param->uc_tid);
	     return OAL_FAIL;
	 }

	 if ( OAL_PTR_NULL == (pst_dmac_user->ast_tx_tid_queue[pst_aggr_tx_on_param->uc_tid].pst_ba_tx_hdl) )
	 {
	     OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_ba_bitmap::pst_tx_ba_handle null\n.}");
	     return OAL_FAIL;
	 }

	 uc_lut_idx = pst_dmac_user->ast_tx_tid_queue[pst_aggr_tx_on_param->uc_tid].pst_ba_tx_hdl->uc_tx_ba_lut;

	 if (uc_lut_idx >= HAL_MAX_AMPDU_LUT_SIZE)
	 {
	     OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_ba_bitmap::invalid uc_lut_idx[%d].}", uc_lut_idx);
	     return OAL_FAIL;
	 }

        pst_tx_ba_track = &g_ast_tx_ba_track[pst_mac_vap->uc_device_id][uc_lut_idx];
        us_track_index  = pst_tx_ba_track->us_track_head;

        pst_fs = oal_kernel_file_open(DMAC_DUMP_BA_BITMAP_PATH, OAL_O_RDWR|OAL_O_CREAT|OAL_O_APPEND);
        if(!pst_fs)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "witp_reset_dump_mac_reg,fail to open file!\n");
            return OAL_FAIL;
        }

        old_fs = oal_get_fs();
        oal_set_fs(OAL_KERNEL_DS);                  //扩展内核空间到用户空间

        if (pst_tx_ba_track->us_track_head > OAM_SEQ_TRACK_NUM - 1 || pst_tx_ba_track->us_track_tail > OAM_SEQ_TRACK_NUM - 1)
        {
            oal_kernel_file_print(pst_fs, "head: %d  tail:%d \n", pst_tx_ba_track->us_track_head, pst_tx_ba_track->us_track_tail);
            return OAL_FAIL;
        }
        oal_kernel_file_print(pst_fs, "tid %d \n", pst_tx_ba_track->ast_seq_bitmap_log[us_track_index].uc_tid);

        while (pst_tx_ba_track->us_track_tail != us_track_index)
        {
            pst_seq_track = &(pst_tx_ba_track->ast_seq_bitmap_log[us_track_index]);
            oal_kernel_file_print(pst_fs, "********************************************************************\n");
            oal_kernel_file_print(pst_fs, "No.%d \n", uc_num);
            uc_num++;

            /*发送完成， 上报描述符链打印*/
            if (pst_seq_track->en_is_before_tx_track == OAL_FALSE)
            {
                oal_kernel_file_print(pst_fs, "ba track AFTER tx \n");
                oal_kernel_file_print(pst_fs, "tx status =  %d, report_dscr_num = %d \n", pst_seq_track->un_mix_dscr.st_report.uc_tx_status, pst_seq_track->un_mix_dscr.st_report.uc_report_mpdu_num);
                oal_kernel_file_print(pst_fs, "report seqnum by hw : ");

                for (us_seq_index = 0; us_seq_index < pst_seq_track->un_mix_dscr.st_report.uc_report_mpdu_num; us_seq_index++)
                {
                    oal_kernel_file_print(pst_fs, "%d ", pst_seq_track->un_mix_dscr.st_report.aus_report_seqnum[us_seq_index]);
                }

                oal_kernel_file_print(pst_fs, "\n");
            }
            else /*发送前， 调度描述符链打印*/
            {
                oal_kernel_file_print(pst_fs, "ba track BEFORE tx \n");
                oal_kernel_file_print(pst_fs, "schedule_mpdu_num = %d \n", pst_seq_track->un_mix_dscr.st_schedule.uc_schedule_mpdu_num);
                oal_kernel_file_print(pst_fs, "schedule seqnum : ");
                for (us_seq_index = 0; us_seq_index < pst_seq_track->un_mix_dscr.st_schedule.uc_schedule_mpdu_num; us_seq_index++)
                {
                     oal_kernel_file_print(pst_fs, "%d ", pst_seq_track->un_mix_dscr.st_schedule.aus_schedule_seqnum[us_seq_index]);
                }

                oal_kernel_file_print(pst_fs, "\n");
            }

            /*tid 队列信息打印*/
            oal_kernel_file_print(pst_fs, "tid_total_mpdu_num = %d, tid_retry_mpdu_num = %d)\n", pst_seq_track->us_tid_total_mpdu_num, pst_seq_track->uc_tid_retry_mpdu_num);
            oal_kernel_file_print(pst_fs, "tid_queue_seqnum : ");
            for (us_seq_index = 0; us_seq_index < pst_seq_track->uc_tid_record_mpdu_num; us_seq_index++)
            {
                oal_kernel_file_print(pst_fs, "%d ", pst_tx_ba_track->ast_seq_bitmap_log[us_track_index].aus_tid_seqnum[us_seq_index]);
            }
            oal_kernel_file_print(pst_fs, "\n");

            /*ba handle信息打印*/
            oal_kernel_file_print(pst_fs, "ba bitmap : %08x %08x %08x %08x \n",
                         pst_seq_track->aul_bitmap[0],
                         pst_seq_track->aul_bitmap[1],
                         pst_seq_track->aul_bitmap[2],
                         pst_seq_track->aul_bitmap[3]);

            oal_kernel_file_print(pst_fs, "baw_start: %d , baw_lsn: %d, baw_head: %d, baw_tail: %d \n",
                         pst_seq_track->us_seq_start,
                         pst_seq_track->us_lsn,
                         pst_seq_track->us_baw_head,
                         pst_seq_track->us_baw_tail);

            us_track_index = (us_track_index + 1) & (OAM_SEQ_TRACK_NUM - 1);
        }
//#endif
#endif
#endif
#endif
    return OAL_SUCC;
}


oal_uint32  dmac_config_get_mpdu_num(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_get_mpdu_num_stru   *pst_param;
    mac_device_stru             *pst_mac_dev;
    dmac_user_stru              *pst_dmac_user;
    oam_report_mpdu_num_stru     st_mpdu_num;

    pst_param = (mac_cfg_get_mpdu_num_stru *)puc_param;

    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_param->auc_user_macaddr);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_get_mpdu_num:: pst_dmac_user is Null");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_get_mpdu_num:: pst_mac_dev is Null");
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_mpdu_num.us_total_mpdu_num_in_device = pst_mac_dev->us_total_mpdu_num;
    st_mpdu_num.us_mpdu_num_in_tid0         = pst_dmac_user->ast_tx_tid_queue[0].us_mpdu_num;
    st_mpdu_num.us_mpdu_num_in_tid1         = pst_dmac_user->ast_tx_tid_queue[1].us_mpdu_num;
    st_mpdu_num.us_mpdu_num_in_tid2         = pst_dmac_user->ast_tx_tid_queue[2].us_mpdu_num;
    st_mpdu_num.us_mpdu_num_in_tid3         = pst_dmac_user->ast_tx_tid_queue[3].us_mpdu_num;
    st_mpdu_num.us_mpdu_num_in_tid4         = pst_dmac_user->ast_tx_tid_queue[4].us_mpdu_num;
    st_mpdu_num.us_mpdu_num_in_tid5         = pst_dmac_user->ast_tx_tid_queue[5].us_mpdu_num;
    st_mpdu_num.us_mpdu_num_in_tid6         = pst_dmac_user->ast_tx_tid_queue[6].us_mpdu_num;
    st_mpdu_num.us_mpdu_num_in_tid7         = pst_dmac_user->ast_tx_tid_queue[7].us_mpdu_num;

    return oam_report_mpdu_num(pst_dmac_user->st_user_base_info.auc_user_mac_addr, &st_mpdu_num);
}


oal_uint32 dmac_config_ota_beacon_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8                                uc_vap_id_loop;
    oal_uint32                               ul_ret;
    oal_int32                                l_value;

    l_value = *((oal_int32 *)puc_param);

    for (uc_vap_id_loop = 0; uc_vap_id_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id_loop++)
    {
        ul_ret = oam_ota_set_beacon_switch(uc_vap_id_loop,
                                          (oam_sdt_print_beacon_rxdscr_type_enum_uint8)l_value);

        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG0(uc_vap_id_loop, OAM_SF_CFG, "{dmac_config_ota_beacon_switch::ota beacon switch set failed!}\r\n");
            return ul_ret;
        }
    }
    return OAL_SUCC;
}


oal_uint32 dmac_config_ota_rx_dscr_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8                                uc_vap_id_loop;
    oal_uint32                               ul_ret;
    oal_int32                                l_value;

    l_value = *((oal_int32 *)puc_param);

    for (uc_vap_id_loop = 0; uc_vap_id_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_id_loop++)
    {
          ul_ret = oam_ota_set_rx_dscr_switch(uc_vap_id_loop,
                                             (oal_switch_enum_uint8)l_value);

        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG0(uc_vap_id_loop, OAM_SF_CFG, "{dmac_config_ota_rx_dscr_switch::ota rx_dscr switch set failed!}\r\n");
            return ul_ret;
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_config_set_all_ota(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_switch_enum_uint8      en_switch;

    en_switch = *((oal_switch_enum_uint8 *)puc_param);
    return oam_report_set_all_switch(en_switch);
}


oal_uint32 dmac_config_oam_output(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int32                   l_value;
    oal_uint32                  ul_ret;

    l_value = *((oal_int32 *)puc_param);

    /* 设置OAM log模块的开关 */
    ul_ret = oam_set_output_type((oam_output_type_enum_uint8)l_value);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_oam_output::oam_set_output_type failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32 dmac_config_probe_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifndef _PRE_WLAN_PROFLING_MIPS

    mac_cfg_probe_switch_stru       *pst_probe_switch;
    oal_uint32                       ul_ret = 0;

    pst_probe_switch = (mac_cfg_probe_switch_stru *)puc_param;

    ul_ret = oam_report_80211_probe_set_switch(pst_probe_switch->en_frame_direction,
                                               pst_probe_switch->en_frame_switch,
                                               pst_probe_switch->en_cb_switch,
                                               pst_probe_switch->en_dscr_switch);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_probe_switch::oam_report_80211_probe_set_switch failed[%d].}", ul_ret);
        return ul_ret;
    }
#endif

    return OAL_SUCC;
}

oal_uint32 dmac_config_80211_mcast_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_80211_mcast_switch_stru *pst_80211_switch_param;
    oal_uint32                       ul_ret = 0;

    pst_80211_switch_param = (mac_cfg_80211_mcast_switch_stru *)puc_param;

    ul_ret = oam_report_80211_mcast_set_switch(pst_80211_switch_param->en_frame_direction,
                                               pst_80211_switch_param->en_frame_type,
                                               pst_80211_switch_param->en_frame_switch,
                                               pst_80211_switch_param->en_cb_switch,
                                               pst_80211_switch_param->en_dscr_switch);

    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_80211_mcast_switch::oam_report_80211_mcast_set_switch failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_WMMAC

oal_uint32 dmac_config_wmmac_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    g_en_wmmac_switch = !!((oal_uint8)*puc_param);

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_config_wmmac_switch::g_en_wmmac_switch = %d}", g_en_wmmac_switch);
    return OAL_SUCC;
}
#endif


oal_uint32  dmac_config_80211_ucast_switch(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param)
{
    mac_cfg_80211_ucast_switch_stru *pst_80211_switch_param;
    oal_uint16                       us_user_idx = 0;
    oal_uint16                       us_max_user_idx = 0;
    oal_uint32                       ul_ret;

    pst_80211_switch_param = (mac_cfg_80211_ucast_switch_stru *)puc_param;

    us_max_user_idx = mac_board_get_max_user();

    /* 广播地址，操作所有用户的单播帧开关 */
    if (ETHER_IS_BROADCAST(pst_80211_switch_param->auc_user_macaddr))
    {
        for (us_user_idx = 0; us_user_idx < us_max_user_idx; us_user_idx++)
        {
            oam_report_80211_ucast_set_switch(pst_80211_switch_param->en_frame_direction,
                                              pst_80211_switch_param->en_frame_type,
                                              pst_80211_switch_param->en_frame_switch,
                                              pst_80211_switch_param->en_cb_switch,
                                              pst_80211_switch_param->en_dscr_switch,
                                              us_user_idx);
        }
        return OAL_SUCC;
    }

    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap,
                                          pst_80211_switch_param->auc_user_macaddr,
                                          &us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_80211_ucast_switch::mac_vap_find_user_by_macaddr[%02X:XX:XX:%02X:%02X:%02X]failed !!}",
                        pst_80211_switch_param->auc_user_macaddr[0],
                        pst_80211_switch_param->auc_user_macaddr[3],
                        pst_80211_switch_param->auc_user_macaddr[4],
                        pst_80211_switch_param->auc_user_macaddr[5]);
        return ul_ret;
    }

    ul_ret = oam_report_80211_ucast_set_switch(pst_80211_switch_param->en_frame_direction,
                                               pst_80211_switch_param->en_frame_type,
                                               pst_80211_switch_param->en_frame_switch,
                                               pst_80211_switch_param->en_cb_switch,
                                               pst_80211_switch_param->en_dscr_switch,
                                               us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_80211_ucast_switch::Set switch of report_ucast failed[%d]!!frame_switch[%d], cb_switch[%d], dscr_switch[%d].}",
                ul_ret,
                pst_80211_switch_param->en_frame_switch,
                pst_80211_switch_param->en_cb_switch,
                pst_80211_switch_param->en_dscr_switch);
        return ul_ret;
    }
    return OAL_SUCC;
}
#endif

oal_uint32  dmac_config_dump_tx_dscr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru         *pst_hal_device;
    oal_uint32                       ul_value;
    hal_tx_dscr_stru                *pst_dscr;
    oal_dlist_head_stru             *pst_pos;
    oal_netbuf_stru                 *pst_netbuf;
    mac_tx_ctl_stru                 *pst_tx_cb;
    oal_uint32                       ul_dscr_one_size = 0;
    oal_uint32                       ul_dscr_two_size = 0;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_tx_dscr::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_value = *((oal_uint32 *)puc_param);

    if (ul_value >= HAL_TX_QUEUE_NUM)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_config_dump_rx_dscr::invalid tx dscr queue[%d].}", ul_value);
        return OAL_FAIL;
    }

    OAL_DLIST_SEARCH_FOR_EACH(pst_pos, &(pst_hal_device->ast_tx_dscr_queue[ul_value].st_header))
    {
        pst_dscr   = OAL_DLIST_GET_ENTRY(pst_pos, hal_tx_dscr_stru, st_entry);
        pst_netbuf = pst_dscr->pst_skb_start_addr;
        pst_tx_cb  = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);

        hal_tx_get_size_dscr(pst_hal_device, MAC_GET_CB_NETBUF_NUM(pst_tx_cb), &ul_dscr_one_size, &ul_dscr_two_size);

        oam_report_dscr(BROADCAST_MACADDR,
                        (oal_uint8 *)pst_dscr,
                        (oal_uint16)(ul_dscr_one_size + ul_dscr_two_size),
                        OAM_OTA_TX_DSCR_TYPE);
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_DFT_STAT
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_config_phy_stat_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if 0
    oam_stats_phy_stat_stru         st_phy_stat;
    mac_device_stru                *pst_macdev;
    oal_uint8                       auc_user_macaddr[WLAN_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    pst_macdev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_macdev))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_phy_stat_info::dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(&st_phy_stat, OAL_SIZEOF(oam_stats_phy_stat_stru));

    hal_dft_get_phyhw_stat_info(pst_macdev->pst_device_stru,&st_phy_stat);

    /* 将获取到的统计值上报 */
    return oam_report_dft_params(auc_user_macaddr, (oal_uint8 *)&st_phy_stat,(oal_uint16)OAL_SIZEOF(oam_stats_phy_stat_stru), OAM_OTA_TYPE_PHY_STAT);
#endif
    return OAL_SUCC;
}



oal_uint32  dmac_config_machw_stat_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if 0
    mac_device_stru                 *pst_mac_dev;
#if 0
    oal_uint8                        uc_loop;
    oal_uint32                       ul_reg_addr;
    oal_uint32                       ul_reg_val = 0;
#endif
    oam_stats_machw_stat_stru        st_machw_stat;
    oal_uint8                        auc_user_macaddr[WLAN_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_machw_stat_info::dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == *puc_param)
    {
        /* 清零mac统计,mac统计清零寄存器的bit2~bit22,bit4是发送beacon帧数目，bit9是高优先级队列发送数目，不清零，
           这两个统计值放在管理帧统计功能控制
        */
        hal_dft_clear_machw_stat_info(pst_mac_dev->pst_device_stru);
        return OAL_SUCC;
    }
    else
    {
        OAL_MEMZERO(&st_machw_stat, OAL_SIZEOF(oam_stats_machw_stat_stru));

        /* 从MAC寄存器获取统计值 */
        dmac_dft_get_machw_stat_info(pst_mac_dev->pst_device_stru, &st_machw_stat);
    #if 0
        /* 获取mac rx统计信息，直接读mac寄存器 */
        for (ul_reg_addr = (oal_uint32)WITP_PA_RX_AMPDU_COUNT_REG, uc_loop = 0;
             ul_reg_addr <= (oal_uint32)WITP_PA_RX_FILTERED_CNT_REG && uc_loop < OAM_MACHW_STAT_RX_MEMBER_CNT;
             ul_reg_addr += 4, uc_loop++)
        {
            hal_reg_info(pst_mac_dev->pst_device_stru, ul_reg_addr, &ul_reg_val);
            st_machw_stat.aul_machw_stat_rx_cnt[uc_loop] = ul_reg_val;
        }

        /* 获取mac tx统计信息，直接读mac寄存器 */
        for (ul_reg_addr = (oal_uint32)WITP_PA_TX_HI_PRI_MPDU_CNT_REG, uc_loop = 0;
             ul_reg_addr <= (oal_uint32)WITP_PA_HI_PRI_RETRY_CNT_REG && uc_loop < OAM_MACHW_STAT_TX_MEMBER_CNT;
             ul_reg_addr += 4, uc_loop++)
        {
            hal_reg_info(pst_mac_dev->pst_device_stru, ul_reg_addr, &ul_reg_val);
            st_machw_stat.aul_machw_stat_tx_cnt[uc_loop] = ul_reg_val;
        }
    #endif
        return oam_report_dft_params(auc_user_macaddr, (oal_uint8 *)&st_machw_stat,(oal_uint16)OAL_SIZEOF(oam_stats_machw_stat_stru), OAM_OTA_TYPE_MACHW_STAT);
    }
#endif
    return OAL_SUCC;
}


oal_uint32  dmac_config_report_mgmt_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                 *pst_mac_dev;
    oam_stats_mgmt_stat_stru         st_mgmt_stat;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_report_mgmt_stat::dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_report_mgmt_stat::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == *puc_param)
    {
        /* 清零mac统计的发送beacon帧数目、高优先级队列发送数目:MAC统计寄存器的bit4和bit9 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
        hal_reg_write(pst_hal_device, (oal_uint32)HI1102_MAC_COUNTER_CLEAR_REG, 0x210);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        hal_reg_write(pst_hal_device, (oal_uint32)HI1103_MAC_COUNTER_CLEAR_REG, 0x210);
#endif
        /* 清零软件统计的管理帧收发数目 */
        OAL_MEMZERO(&pst_mac_dev->st_mgmt_stat, OAL_SIZEOF(mac_device_mgmt_statistic_stru));

        return OAL_SUCC;
    }
    else
    {
        /* 获取mac统计的发送beacon帧数目和高优先级队列发送数目 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
        hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_TX_BCN_COUNT_REG, &st_mgmt_stat.ul_machw_stat_tx_bcn_cnt);
        hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_TX_HI_PRI_MPDU_CNT_REG, &st_mgmt_stat.ul_machw_stat_tx_hi_cnt);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_TX_BCN_COUNT_REG, &st_mgmt_stat.ul_machw_stat_tx_bcn_cnt);
        hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_TX_HI_PRI_MPDU_CNT_REG, &st_mgmt_stat.ul_machw_stat_tx_hi_cnt);
#endif

        /* 获取软件的管理帧收发统计 */
        oal_memcopy(st_mgmt_stat.aul_sw_rx_mgmt_cnt,
                    &pst_mac_dev->st_mgmt_stat,
                    OAL_SIZEOF(mac_device_mgmt_statistic_stru));

        /* 将统计值上报 */
        return oam_report_dft_params(BROADCAST_MACADDR, (oal_uint8 *)&st_mgmt_stat, (oal_uint16)OAL_SIZEOF(oam_stats_mgmt_stat_stru),OAM_OTA_TYPE_MGMT_STAT);
    }
}
#endif

oal_uint32  dmac_config_report_vap_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru               *pst_dmac_vap;
    dmac_vap_query_stats_stru    st_vap_dft_stats;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{dmac_config_report_vap_stat::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_OS_VERSION == _PRE_OS_VERSION_RAW)
    OAM_WARNING_LOG_ALTER(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{dmac_config_report_vap_stat::drv_tx_pkts[%lu], hw_tx_pkts_succ[%lu], tx_fail[%lu], rx_mpdus[%lu], rx_drops[%lu], rx_null[%lu], rx_mgmt[%lu]}", 7,
                        pst_dmac_vap->st_query_stats.ul_drv_tx_pkts, pst_dmac_vap->st_query_stats.ul_hw_tx_pkts, pst_dmac_vap->st_query_stats.ul_tx_failed,
                        pst_dmac_vap->st_query_stats.ul_rx_mpdu_total_num,
                        (pst_dmac_vap->st_query_stats.ul_rx_dropped_misc + pst_dmac_vap->st_query_stats.ul_rx_alg_process_dropped + pst_dmac_vap->st_query_stats.ul_rx_security_check_fail_dropped),
                        pst_dmac_vap->st_query_stats.ul_rx_null_frame_dropped,
                        pst_dmac_vap->st_query_stats.ul_rx_mgmt_mpdu_num_cnt);
#endif
    oal_memcopy(&st_vap_dft_stats,
                &(pst_dmac_vap->st_query_stats),
                OAL_SIZEOF(dmac_vap_query_stats_stru));

    return oam_report_dft_params(BROADCAST_MACADDR, (oal_uint8 *)&st_vap_dft_stats,(oal_uint16)OAL_SIZEOF(dmac_vap_query_stats_stru),OAM_OTA_TYPE_VAP_STAT);
}


oal_uint32  dmac_config_set_phy_stat_en(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oam_stats_phy_node_idx_stru     *pst_phy_node_idx;
    hal_to_dmac_device_stru         *pst_hal_device;

    pst_phy_node_idx = (oam_stats_phy_node_idx_stru *)puc_param;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_phy_stat_en::dev is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return dmac_dft_set_phy_stat_node(pst_hal_device, pst_phy_node_idx);
}


oal_uint32  dmac_config_dbb_env_param(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    if (OAL_FALSE == *puc_param)
    {
        /* 停止上报，并清除资源 */
        return dmac_dft_stop_report_dbb_env(pst_mac_vap);
    }
    else
    {
        /* 开始周期(20ms)采集,周期(2s)上报 */
        return dmac_dft_start_report_dbb_env(pst_mac_vap);
    }
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 dmac_config_report_vap_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifdef _PRE_WLAN_DFT_STAT
    oal_uint32               ul_flags_value;

    /* 参数合法性判断 */
    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == puc_param))
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_config_report_vap_info::input params is invalid, %p, %p.}",
                         pst_mac_vap, puc_param);
        return OAL_FAIL;
    }

    ul_flags_value = *(oal_uint32 *)puc_param;

    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_config_report_vap_info::vap_id[%d], flags_value[0x%08x].}",
                     pst_mac_vap->uc_vap_id, ul_flags_value);

    /* 上报硬件信息 */
    if (ul_flags_value & MAC_REPORT_INFO_FLAGS_HARDWARE_INFO)
    {
        dmac_dft_report_mac_hardware_info(pst_mac_vap);
    }

    /* 上报队列信息 */
    if (ul_flags_value & MAC_REPORT_INFO_FLAGS_QUEUE_INFO)
    {
        dmac_dft_report_dmac_queue_info(pst_mac_vap);
    }

    /* 上报内存信息 */
    if (ul_flags_value & MAC_REPORT_INFO_FLAGS_MEMORY_INFO)
    {
        dmac_dft_report_dmac_memory_info(pst_mac_vap);
    }

    /* 上报事件信息 */
    if (ul_flags_value & MAC_REPORT_INFO_FLAGS_EVENT_INFO)
    {
        dmac_dft_report_dmac_event_info(pst_mac_vap);
    }

    /* 上报vap信息 */
    if (ul_flags_value & MAC_REPORT_INFO_FLAGS_VAP_INFO)
    {
        dmac_dft_report_dmac_vap_info(pst_mac_vap);
    }

    /* 上报用户信息 */
    if (ul_flags_value & MAC_REPORT_INFO_FLAGS_USER_INFO)
    {
        dmac_dft_report_dmac_user_info(pst_mac_vap);
    }

    /* 上报收发包统计信息 */
    if (ul_flags_value & MAC_REPORT_INFO_FLAGS_TXRX_PACKET_STATISTICS)
    {
        dmac_config_report_vap_stat(pst_mac_vap, uc_len, puc_param);
    }
#else
    OAM_WARNING_LOG0(0, OAM_SF_ANY,
                     "{dmac_config_report_vap_info::DFT macro switch is not open, do nothing.}");
#endif

    return OAL_SUCC;
}
#endif


oal_uint32 dmac_config_set_feature_log(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    // 设置device log level，业务添加处理逻辑
    oal_uint16      us_param;
    us_param = *(oal_uint16 *)puc_param;
    return oam_log_set_feature_level(pst_mac_vap->uc_vap_id, (oal_uint8)(us_param>>8), (oal_uint8)us_param) ;
}

oal_uint32  dmac_config_get_thruput(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint32                       ul_rx_octects_in_ampdu;
    oal_uint32                       ul_tx_octects_in_ampdu;
    oal_uint32                       ul_rx_normal_mdpu_succ_num;
    oal_uint32                       ul_rx_ampdu_succ_num;
    oal_uint32                       ul_tx_ppdu_succ_num;
    oal_uint32                       ul_rx_ppdu_fail_num;
    oal_uint32                       ul_tx_ppdu_fail_num;
    oal_uint32                       ul_rx_ampdu_fcs_num;
    oal_uint32                       ul_rx_delimiter_fail_num;
    oal_uint32                       ul_rx_mpdu_fcs_num;
    oal_uint32                       ul_rx_phy_err_mac_passed_num;
    oal_uint32                       ul_rx_phy_longer_err_num;
    oal_uint32                       ul_rx_phy_shorter_err_num;
    oal_uint32                       ul_timestamp;
    oal_uint8                        uc_stage;       /*0为开始统计阶段， 1为结束统计阶段*/
    oal_uint32                       ul_rx_rate;     /*单位mpbs*/
    oal_uint32                       ul_tx_rate;     /*单位mpbs*/
    oal_uint32                       ul_time_offest; /*统计时间差, 单位ms*/
#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    oal_cpu_usage_stat_stru          st_cpu_stat;
    oal_uint64                       ull_alltime;
#endif
    mac_device_stru                 *pst_mac_device;
    oal_uint32                       ul_cycles = 0;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    frw_event_mem_stru             *pst_event_mem;
    frw_event_stru                 *pst_event_up;
    dmac_thruput_info_sync_stru     *pst_thruput_info_sync;
#endif

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_thruput::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_thruput::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }


    uc_stage = *(oal_bool_enum_uint8 *)puc_param;

    if(0 == uc_stage)
    {
        pst_mac_device->ul_first_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    #if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    #if (!defined(_PRE_PC_LINT) && !defined(WIN32))
        //enable_cycle_counter();
    #endif
    #endif

        /*清零*/
        hal_set_counter_clear(pst_dmac_vap->pst_hal_device);
        pst_dmac_vap->pst_hal_device->ul_rx_normal_mdpu_succ_num = 0;
        pst_dmac_vap->pst_hal_device->ul_rx_ampdu_succ_num = 0;
        pst_dmac_vap->pst_hal_device->ul_tx_ppdu_succ_num = 0;
        pst_dmac_vap->pst_hal_device->ul_rx_ppdu_fail_num = 0;
        pst_dmac_vap->pst_hal_device->ul_tx_ppdu_fail_num = 0;
    }
    else
    {
        ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
        ul_time_offest = ul_timestamp - pst_mac_device->ul_first_timestamp;

        #if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        #if (!defined(_PRE_PC_LINT) && !defined(WIN32))
            //disable_cycle_counter();

            //ul_cycles = get_cycle_count();
        #endif
        #endif

        ul_rx_normal_mdpu_succ_num = pst_dmac_vap->pst_hal_device->ul_rx_normal_mdpu_succ_num;
        ul_rx_ampdu_succ_num = pst_dmac_vap->pst_hal_device->ul_rx_ampdu_succ_num;
        ul_tx_ppdu_succ_num = pst_dmac_vap->pst_hal_device->ul_tx_ppdu_succ_num;
        ul_rx_ppdu_fail_num = pst_dmac_vap->pst_hal_device->ul_rx_ppdu_fail_num;
        ul_tx_ppdu_fail_num = pst_dmac_vap->pst_hal_device->ul_tx_ppdu_fail_num;

        if (HAL_ALWAYS_TX_MPDU == pst_dmac_vap->pst_hal_device->bit_al_tx_flag)
        {
            ul_tx_rate = ((ul_tx_ppdu_succ_num + ul_tx_ppdu_fail_num)*g_ul_al_mpdu_len/(ul_time_offest))*8;
            ul_rx_rate = ((ul_rx_normal_mdpu_succ_num + ul_rx_ppdu_fail_num)*g_ul_al_mpdu_len/(ul_time_offest))*8;
        }
        else
        {
            hal_get_ampdu_bytes(pst_dmac_vap->pst_hal_device, &ul_tx_octects_in_ampdu, &ul_rx_octects_in_ampdu);

            ul_rx_rate = (ul_rx_octects_in_ampdu/(ul_time_offest))*8;
            ul_tx_rate = (ul_tx_octects_in_ampdu/(ul_time_offest))*8;

            OAM_INFO_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_all_rx_dscr::tx octects=%u, rx octects=%u, cycles=%u.}",
                           ul_tx_octects_in_ampdu, ul_rx_octects_in_ampdu, ul_cycles);

        }

        /*错误检查*/
        hal_get_rx_err_count(pst_dmac_vap->pst_hal_device,
                        &ul_rx_ampdu_fcs_num,
                        &ul_rx_delimiter_fail_num,
                        &ul_rx_mpdu_fcs_num,
                        &ul_rx_phy_err_mac_passed_num,
                        &ul_rx_phy_longer_err_num,
                        &ul_rx_phy_shorter_err_num);

        OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{tx succ num: %u, tx fail num: %u}", ul_tx_ppdu_succ_num, ul_tx_ppdu_fail_num);
        OAM_INFO_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{rx normal succ num: %u, rx ampdu succ num: %u, rx fail num: %u}", ul_rx_normal_mdpu_succ_num, ul_rx_ampdu_succ_num, ul_rx_ppdu_fail_num);
        OAM_INFO_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{tx rate(Mbps): %u, rx rate(Mbps): %u, ul_cycles: %u,}", ul_tx_rate, ul_rx_rate, ul_cycles);
        OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{rx ampdu fcs num: %u, rx delimiter fail num: %u,}", ul_rx_ampdu_fcs_num, ul_rx_delimiter_fail_num);
        OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{rx mpdu fcs num: %u, rx phy err mac passed num: %u,}", ul_rx_mpdu_fcs_num, ul_rx_phy_err_mac_passed_num);
        OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{rx phy longer err num: %u, rx phy shorter err num: %u,}", ul_rx_phy_longer_err_num, ul_rx_phy_shorter_err_num);

#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
        OAL_MEMZERO(&st_cpu_stat, OAL_SIZEOF(oal_cpu_usage_stat_stru));
        oal_get_cpu_stat(&st_cpu_stat);
        ull_alltime = st_cpu_stat.ull_user + st_cpu_stat.ull_nice + st_cpu_stat.ull_system + st_cpu_stat.ull_idle + st_cpu_stat.ull_iowait +
                      st_cpu_stat.ull_irq + st_cpu_stat.ull_softirq + st_cpu_stat.ull_steal + st_cpu_stat.ull_guest;
        OAL_IO_PRINT("user=%llu, nice=%llu, system=%llu, idle=%llu, iowait=%llu, irq=%llu, softirq=%llu, steal=%llu, guest=%llu, alltime=%llu\r\n",
                    st_cpu_stat.ull_user, st_cpu_stat.ull_nice, st_cpu_stat.ull_system, st_cpu_stat.ull_idle, st_cpu_stat.ull_iowait,
                    st_cpu_stat.ull_irq,  st_cpu_stat.ull_softirq, st_cpu_stat.ull_steal, st_cpu_stat.ull_guest, ull_alltime);
#endif

        OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_all_rx_dscr::TX succ num=%d,fail num=%d;RX succ num=%d, fail num=%d}",
                       ul_tx_ppdu_succ_num, ul_tx_ppdu_fail_num, ul_rx_normal_mdpu_succ_num, ul_rx_ppdu_fail_num);
        OAM_INFO_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_all_rx_dscr::tx rate(Mbps)=%d, rx rate(Mbps)=%d, ul_cycles=%d}",
                       ul_tx_rate, ul_rx_rate, ul_cycles);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* 将需要同步的数据抛事件同步到hmac */
        pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_thruput_info_sync_stru));
        if (OAL_PTR_NULL == pst_event_mem)
        {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_query_event_response::pst_event_memory null.}");

            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_event_up          = frw_get_event_stru(pst_event_mem);
        pst_thruput_info_sync = (dmac_thruput_info_sync_stru *)pst_event_up->auc_event_data;
        pst_thruput_info_sync->ul_cycles        = ul_cycles;
        pst_thruput_info_sync->ul_sw_tx_succ_num = ul_tx_ppdu_succ_num;
        pst_thruput_info_sync->ul_sw_tx_fail_num = ul_tx_ppdu_fail_num;
        pst_thruput_info_sync->ul_sw_rx_ampdu_succ_num = ul_rx_ampdu_succ_num;
        pst_thruput_info_sync->ul_sw_rx_ppdu_fail_num = ul_rx_ppdu_fail_num;
        pst_thruput_info_sync->ul_sw_rx_mpdu_succ_num  = ul_rx_normal_mdpu_succ_num;
        pst_thruput_info_sync->ul_hw_rx_ampdu_fcs_fail_num = ul_rx_ampdu_fcs_num;
        pst_thruput_info_sync->ul_hw_rx_mpdu_fcs_fail_num = ul_rx_mpdu_fcs_num;
        dmac_send_sys_event((mac_vap_stru *)&(pst_dmac_vap->st_vap_base_info), WLAN_CFGID_THRUPUT_INFO, OAL_SIZEOF(dmac_thruput_info_sync_stru), (oal_uint8 *)pst_thruput_info_sync);
        FRW_EVENT_FREE(pst_event_mem);
#endif
    }

    return OAL_SUCC;
}



oal_uint32  dmac_config_beacon_chain_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8         uc_value;
    dmac_vap_stru    *pst_dmac_vap;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_beacon_chain_switch::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_value = *((oal_uint8 *)puc_param);

    /* 配置beacon帧发送通道，0为普通单通道发送，1为开启双通道轮流发送能力,2为开启双通道发送能力 */
    pst_dmac_vap->en_beacon_tx_policy = (dmac_beacon_tx_policy_enum_uint8)uc_value;

    return OAL_SUCC;
}


oal_uint32  dmac_config_hide_ssid(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8         uc_value;

    uc_value = *((oal_uint8 *)puc_param);
    mac_vap_set_hide_ssid(pst_mac_vap, uc_value);
    OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_config_hide_ssid::mac_vap_set_hide_ssid [%d].}", uc_value);

    //OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "dmac_config_hide_ssid[%d] succ!", uc_value);
#endif
    return OAL_SUCC;
}



oal_uint32  dmac_config_set_bw_fixed(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8         uc_value;

    uc_value = *((oal_uint8 *)puc_param);

    pst_mac_vap->bit_bw_fixed = uc_value;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_set_bw_fixed[%d] succ!", uc_value);
#endif
    return OAL_SUCC;
}


#ifdef _PRE_WLAN_DFT_STAT
oal_uint32  dmac_config_set_performance_log_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_set_performance_log_switch_stru *pst_set_performance_log_switch = (mac_cfg_set_performance_log_switch_stru *)puc_param;
    oal_uint8                    uc_loop_index;
    dmac_vap_stru *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{dmac_config_set_performance_log_switch::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if(pst_set_performance_log_switch->uc_performance_log_switch_type >= DFX_PERFORMANCE_LOG_BUTT)
    {
        for(uc_loop_index = 0;uc_loop_index < DFX_PERFORMANCE_LOG_BUTT;uc_loop_index++)
        {
            DFX_SET_PERFORMANCE_LOG_SWITCH_ENABLE(uc_loop_index,pst_set_performance_log_switch->uc_value);
        }
    }
    else if(pst_set_performance_log_switch->uc_performance_log_switch_type == DFX_PERFORMANCE_DUMP)
    {
        if(0 == pst_set_performance_log_switch->uc_value )
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{ampdu length (1-14)%d  (15-17)%d  (18-30)%d  (31-32)%d\n.}",
                pst_dmac_vap->st_query_stats.aul_tx_count_per_apmpdu_length[DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_0],
                pst_dmac_vap->st_query_stats.aul_tx_count_per_apmpdu_length[DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_1],
                pst_dmac_vap->st_query_stats.aul_tx_count_per_apmpdu_length[DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_2],
                pst_dmac_vap->st_query_stats.aul_tx_count_per_apmpdu_length[DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_3]
            );
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{ul_tx_hardretry_count = %d,ul_tx_cts_fail = %d\n.}",
                pst_dmac_vap->st_query_stats.ul_tx_retries,
                pst_dmac_vap->st_query_stats.ul_tx_cts_fail);
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{ul_tx_mpdu_succ_num = %d, ul_tx_mpdu_fail_num = %d\n.}",
                pst_dmac_vap->st_query_stats.ul_tx_mpdu_succ_num,
                pst_dmac_vap->st_query_stats.ul_tx_mpdu_fail_num);
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{ul_tx_ampdu_succ_num = %d, ul_tx_mpdu_in_ampdu = %d,ul_tx_ampdu_fail_num = %d, ul_tx_mpdu_fail_in_ampdu = %d\n.}",
                pst_dmac_vap->st_query_stats.ul_tx_ampdu_succ_num,
                pst_dmac_vap->st_query_stats.ul_tx_mpdu_in_ampdu,
                pst_dmac_vap->st_query_stats.ul_tx_ampdu_fail_num,
                pst_dmac_vap->st_query_stats.ul_tx_mpdu_fail_in_ampdu);
        }
        else
        {
            for(uc_loop_index = 0;uc_loop_index < DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_BUTT;uc_loop_index++)
            {
                pst_dmac_vap->st_query_stats.aul_tx_count_per_apmpdu_length[uc_loop_index] = 0;
            }
        }
    }
    else
    {
        DFX_SET_PERFORMANCE_LOG_SWITCH_ENABLE(pst_set_performance_log_switch->uc_performance_log_switch_type,pst_set_performance_log_switch->uc_value);
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

oal_uint32  dmac_config_set_opmode_notify(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8    uc_value;

    uc_value = *puc_param;

    if (OAL_TRUE == mac_mib_get_VHTOptionImplemented(pst_mac_vap))
    {
        mac_mib_set_OperatingModeNotificationImplemented(pst_mac_vap, (oal_bool_enum_uint8)uc_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{dmac_config_set_opmode_notify::pst_mac_vap is not 11ac. en_protocol = [%d].}\r\n", pst_mac_vap->en_protocol);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

oal_uint32  dmac_config_get_user_rssbw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_add_user_param_stru    *pst_user;
    dmac_vap_stru                  *pst_dmac_vap;
    dmac_user_stru                 *pst_dmac_user;

    pst_user = (mac_cfg_add_user_param_stru *)puc_param;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_OPMODE, "{dmac_config_get_user_rssbw::pst_dmac_vap null.}");
        return OAL_FAIL;
    }

    pst_dmac_user = (dmac_user_stru *)mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_user->auc_mac_addr);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{dmac_config_get_user_rssbw::pst_dmac_user null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{dmac_config_get_user_rssbw::nss_cap[%d]avail_nss[%d]user bw_cap[%d]avail_bw[%d].}",
                pst_dmac_user->st_user_base_info.en_user_num_spatial_stream, pst_dmac_user->st_user_base_info.en_avail_num_spatial_stream,
                pst_dmac_user->st_user_base_info.en_bandwidth_cap, pst_dmac_user->st_user_base_info.en_avail_bandwidth);

    return OAL_SUCC;
}
#endif

oal_uint32  dmac_config_set_vap_nss(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8            uc_value;

    uc_value = *puc_param;

    mac_vap_init_rx_nss_by_protocol(pst_mac_vap);
    mac_vap_set_rx_nss(pst_mac_vap, OAL_MIN(pst_mac_vap->en_vap_rx_nss, (uc_value - 1)));

    return OAL_SUCC;
}


oal_uint32  dmac_config_set_ampdu_aggr_num(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_aggr_num_stru   *pst_aggr_num_ctrl;

    pst_aggr_num_ctrl = (mac_cfg_aggr_num_stru *)puc_param;

    g_uc_aggr_num_switch = pst_aggr_num_ctrl->uc_aggr_num_switch;

    if (0 != pst_aggr_num_ctrl->uc_aggr_num_switch)
    {
        g_uc_max_aggr_num = pst_aggr_num_ctrl->uc_aggr_num;
    }
    else
    {
        g_uc_max_aggr_num = 0;
    }

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_ampdu_aggr_num::aggr num [%d],enable[%d].}", g_uc_max_aggr_num, g_uc_aggr_num_switch);

    return OAL_SUCC;
}

oal_uint32 dmac_config_show_device_memleak(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_device_pool_id_stru          *pst_pool_id_param;
    oal_uint8                         uc_pool_id;

    pst_pool_id_param = (mac_device_pool_id_stru *)puc_param;
    uc_pool_id  = pst_pool_id_param->uc_pool_id;

    if (uc_pool_id >=  OAL_MEM_POOL_ID_BUTT)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_show_device_memleak::uc_pool_id %d >= OAL_MEM_POOL_ID_BUTT.}", uc_pool_id);
        return OAL_FAIL;
    }

    if (uc_pool_id < OAL_MEM_POOL_ID_NETBUF)
    {
    #ifndef _PRE_WLAN_FEATURE_MEM_OPT
        oal_mem_leak(uc_pool_id);
    #endif
    }
    else if (OAL_MEM_POOL_ID_NETBUF == uc_pool_id)
    {
        oal_mem_netbuf_leak();
    }
#endif
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_PROFLING_MIPS

oal_uint32 dmac_config_set_mips(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_mips_type_param_stru      *pst_mips_type_param;

    pst_mips_type_param = (oal_mips_type_param_stru *)puc_param;

    switch (pst_mips_type_param->l_mips_type)
    {
        case OAL_MIPS_TX:
        {
            if (OAL_SWITCH_ON == pst_mips_type_param->l_switch)
            {
                if (OAL_SWITCH_OFF == g_mips_tx_statistic.en_switch)
                {
                    oal_profiling_mips_tx_init();
                    oal_profiling_enable_cycles();

                    g_mips_tx_statistic.en_switch = OAL_SWITCH_ON;
                    g_mips_tx_statistic.uc_flag |= BIT0;
                    OAL_MIPS_STATISTIC_REGISTER_IRQ_SAVE_NOTIFY_FUNC(oal_profiling_irq_save);
                    OAL_MIPS_STATISTIC_REGISTER_IRQ_RESTORE_NOTIFY_FUNC(oal_profiling_irq_restore);
                }
            }
            else if (OAL_SWITCH_OFF == pst_mips_type_param->l_switch)
            {
                if (OAL_SWITCH_ON == g_mips_tx_statistic.en_switch)
                {
                    oal_profiling_disable_cycles();

                    g_mips_tx_statistic.en_switch = OAL_SWITCH_OFF;
                    OAL_MIPS_STATISTIC_REGISTER_IRQ_SAVE_NOTIFY_FUNC(oal_profiling_irq_null_func);
                    OAL_MIPS_STATISTIC_REGISTER_IRQ_RESTORE_NOTIFY_FUNC(oal_profiling_irq_null_func);
                }
            }
        }
        break;

        case OAL_MIPS_RX:
        {
            if (OAL_SWITCH_ON == pst_mips_type_param->l_switch)
            {
                if (OAL_SWITCH_OFF == g_mips_rx_statistic.en_switch)
                {
                    oal_profiling_mips_rx_init();
                    oal_profiling_enable_cycles();

                    g_mips_rx_statistic.en_switch = OAL_SWITCH_ON;
                    OAL_MIPS_STATISTIC_REGISTER_IRQ_SAVE_NOTIFY_FUNC(oal_profiling_irq_save);
                    OAL_MIPS_STATISTIC_REGISTER_IRQ_RESTORE_NOTIFY_FUNC(oal_profiling_irq_restore);
                }
            }
            else if (OAL_SWITCH_OFF == pst_mips_type_param->l_switch)
            {
                if (OAL_SWITCH_ON == g_mips_rx_statistic.en_switch)
                {
                    oal_profiling_disable_cycles();

                    g_mips_rx_statistic.en_switch = OAL_SWITCH_OFF;
                    OAL_MIPS_STATISTIC_REGISTER_IRQ_SAVE_NOTIFY_FUNC(oal_profiling_irq_null_func);
                    OAL_MIPS_STATISTIC_REGISTER_IRQ_RESTORE_NOTIFY_FUNC(oal_profiling_irq_null_func);
                }
            }
        }
        break;

        default:
        {
            OAL_IO_PRINT("dmac_config_set_mips: mips type is wrong!\r\n");
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_config_show_mips(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int32 l_mips_type;

    l_mips_type = *((oal_int32 *)puc_param);

    switch (l_mips_type)
    {
        case OAL_MIPS_TX:
        {
            oal_profiling_tx_mips_show();
        }
        break;

        case OAL_MIPS_RX:
        {
            oal_profiling_rx_mips_show();
        }
        break;

        default:
        {
            OAL_IO_PRINT("dmac_config_show_mips: mips type is wrong!\r\n");
        }
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD

oal_uint32 dmac_config_enable_arp_offload(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru *pst_dmac_vap  = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_dmac_vap->en_arp_offload_switch = *(oal_switch_enum_uint8 *)puc_param;
    return OAL_SUCC;
}


oal_uint32 dmac_config_show_arpoffload_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_arpoffload_info_stru    *pst_ps_info;
    mac_device_stru                 *pst_mac_dev;
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint8                        uc_show_ip_addr;
    oal_uint8                        uc_show_arpoffload_info;
    oal_uint32                       ul_loop;

    pst_mac_dev  = mac_res_get_dev(pst_mac_vap->uc_device_id);
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);

    pst_ps_info                 = (mac_cfg_arpoffload_info_stru *)puc_param;
    uc_show_ip_addr             = pst_ps_info->uc_show_ip_addr;
    uc_show_arpoffload_info     = pst_ps_info->uc_show_arpoffload_info;
    if ((OAL_PTR_NULL == pst_mac_dev) || (OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_ps_info))
    {
        OAM_ERROR_LOG3(0, OAM_SF_CFG, "{dmac_config_show_arpoffload_info:: pointer is null,pst_mac_de[0x%x],vpst_mac_vap[0x%x],puc_param[0x%x]",pst_mac_dev,pst_dmac_vap,pst_ps_info);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (1 == uc_show_ip_addr)
    {
        for (ul_loop = 0; ul_loop < DMAC_MAX_IPV4_ENTRIES; ul_loop++)
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_show_ip_addr::IPv4 index[%d]: %d.X.X.%d. MASK[0x%08X]}",
                             ul_loop,
                             pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_local_ip.auc_value[0],
                             pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_local_ip.auc_value[3],
                             pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_mask.ul_value);
        }

        for (ul_loop = 0; ul_loop < DMAC_MAX_IPV6_ENTRIES; ul_loop++)
        {
            OAM_WARNING_LOG_ALTER(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_show_ip_addr::IPv6 index[%d]: %04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x.}",
                                  5,
                                  ul_loop,
                                  OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr))[0]),
                                  OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr))[1]),
                                  OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr))[6]),
                                  OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr))[7]));
        }
    }

    if (1 == uc_show_arpoffload_info)
    {
        OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,"suspend state:[%d],arpofflad drop frame:[%d],send arp rsp:[%d]",pst_mac_dev->uc_in_suspend,g_ul_arpoffload_drop_frame,g_ul_arpoffload_send_arprsp);
    }
    /* 统计清零 */
    else if (0 == uc_show_arpoffload_info)
    {
        g_ul_arpoffload_drop_frame  = 0;
        g_ul_arpoffload_send_arprsp = 0;
    }
    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST

oal_uint32 dmac_config_enable_2040bss(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru           *pst_mac_device;
    oal_bool_enum_uint8        en_2040bss_switch;
    oal_uint8                  uc_vap_idx;
    mac_vap_stru              *pst_vap;

    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{dmac_config_enable_2040bss:: pointer is null,pst_mac_vap[%d], puc_param[%d] .}", pst_mac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_enable_2040bss::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_2040bss_switch = (*puc_param == 0) ? OAL_FALSE : OAL_TRUE;
    //同步device下所有vap的mib 2040特性的配置开关
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_config_enable_2040bss::pst_mac_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
        mac_mib_set_2040SwitchProhibited(pst_vap,((en_2040bss_switch == OAL_TRUE)? OAL_FALSE : OAL_TRUE));
    }
    mac_set_2040bss_switch(pst_mac_device, en_2040bss_switch);

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_enable_2040bss::set 2040bss switch[%d].}", en_2040bss_switch);

    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_20_40_80_COEXIST */

#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


