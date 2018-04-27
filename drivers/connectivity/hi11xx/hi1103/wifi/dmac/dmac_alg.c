


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_alg.h"
#include "dmac_config.h"
#include "frw_main.h"
#include "frw_timer.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_ALG_C

extern frw_event_sub_table_item_stru g_ast_dmac_misc_event_sub_table[];

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 算法配置命令 */
dmac_alg_config_table_stru g_ast_dmac_alg_config_table[] =
{
#ifdef _PRE_WLAN_FEATURE_DBAC
    {"dbac",        DMAC_ALG_CONFIG_ID_DBAC},
#endif

#ifdef _PRE_WLAN_FEATURE_SCHEDULE
    {"sch",         DMAC_ALG_CONFIG_ID_SCH},
#endif

#ifdef _PRE_WLAN_FEATURE_TRAFFIC_CTL
    {"tfctl",      DMAC_ALG_CONFIG_ID_TRAFF_CTL},
#endif

#ifdef _PRE_WLAN_CHIP_TEST_ALG
    {"txbf",        DMAC_ALG_CONFIG_ID_TXBF},
    {"test",        DMAC_ALG_CONFIG_ID_TEST_MAIN},
    {"dbac_ct",     DMAC_ALG_CONFIG_ID_DBAC_TEST},
    {"ar_test",     DMAC_ALG_CONFIG_ID_AUTORATE_TEST},
    {"ant_test",    DMAC_ALG_CONFIG_ID_SMARTANT_TEST},
    {"rssi_test",   DMAC_ALG_CONFIG_ID_RSSI_TEST},
    {"tpc_test",    DMAC_ALG_CONFIG_ID_TPC_TEST},
#endif

    {"cmn",         DMAC_ALG_CONFIG_ID_COMMON},
    {"err_name",    DMAC_ALG_CONFIG_ID_ERROR}
};



/* 算法框架主结构体 */
dmac_alg_stru  g_st_alg_main;


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint32  dmac_alg_register_dmac_misc_event(hal_dmac_misc_sub_type_enum en_event_type, oal_uint32 (*p_func)(frw_event_mem_stru *))
{
    ALG_ASSERT_RET(en_event_type < HAL_EVENT_DMAC_MISC_SUB_TYPE_BUTT && NULL != p_func, OAL_FAIL);
    g_ast_dmac_misc_event_sub_table[en_event_type].p_func = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_dmac_misc_event(hal_dmac_misc_sub_type_enum en_event_type)
{
    ALG_ASSERT_RET(en_event_type < HAL_EVENT_DMAC_MISC_SUB_TYPE_BUTT, OAL_FAIL);
    g_ast_dmac_misc_event_sub_table[en_event_type].p_func = NULL;
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_INTF_DET
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS

oal_uint32  dmac_alg_register_intf_det_psd_notify_func(p_alg_intf_det_psd_notify_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_cfg_intf_det_psd_notify_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_intf_det_psd_notify_func(oal_void)
{
    g_pst_alg_main->p_cfg_intf_det_psd_notify_func = OAL_PTR_NULL;
    return OAL_SUCC;
}


oal_uint32  dmac_alg_cfg_intf_det_psd_notify(hal_to_dmac_device_stru *pst_hal_device)
{
    dmac_alg_stru            *pst_alg_stru;

    pst_alg_stru = g_pst_alg_main;
    /* 调用相关回调函数 */
    if (OAL_PTR_NULL != pst_alg_stru->p_cfg_intf_det_psd_notify_func)
    {
        pst_alg_stru->p_cfg_intf_det_psd_notify_func(pst_hal_device);
    }

    return OAL_SUCC;
}
#endif//_PRE_WLAN_FEATURE_PSD_ANALYSIS
#endif//_PRE_WLAN_FEATURE_INTF_DET

oal_uint32  dmac_alg_unregister_get_rate_kbps_func(oal_void)
{
    g_pst_alg_main->p_get_rate_kbps_func = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_get_rate_kbps(hal_to_dmac_device_stru *pst_hal_device, hal_statistic_stru *pst_per_rate, oal_uint32 *pul_rate_kbps)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_get_rate_kbps_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
    g_pst_alg_main->p_get_rate_kbps_func(pst_hal_device, pst_per_rate, pul_rate_kbps);
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF

oal_uint32  dmac_alg_unregister_anti_intf_switch_notify(oal_void)
{
    g_pst_alg_main->p_anti_intf_switch_func = OAL_PTR_NULL;

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP

oal_uint32  dmac_alg_unregister_edca_stat_event_notify(oal_void)
{
    g_pst_alg_main->p_edca_stat_event_notify_func = OAL_PTR_NULL;

    return OAL_SUCC;
}
#endif


oal_uint32  dmac_alg_cfg_bandwidth_notify(mac_vap_stru *pst_vap, dmac_alg_channel_bw_chg_type_uint8 en_type)
{
    dmac_alg_stru            *pst_alg_stru;
    oal_uint8                 uc_index;
    oal_dlist_head_stru      *pst_entry;
    mac_user_stru            *pst_mac_user;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "OAL_ERR_CODE_PTR_NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_CFG_BANDWIDTH_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_bandwidth_notify_func[uc_index])
        {
            pst_alg_stru->pa_cfg_bandwidth_notify_func[uc_index](pst_vap, en_type);
        }
    }

    if (CH_BW_CHG_TYPE_SCAN == en_type)
    {
        return OAL_SUCC;
    }

    /* 遍历VAP下所有USER */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_vap->st_mac_user_list_head))
    {
        pst_mac_user = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

        /* 通知算法 */
        dmac_alg_cfg_user_bandwidth_notify(pst_vap, pst_mac_user);
    }

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_delete_ba_fail_notify(oal_void)
{
    g_pst_alg_main->p_delete_ba_fail_notify_func = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_get_tx_best_rate(mac_user_stru *pst_mac_user,
                                  wlan_wme_ac_type_enum_uint8 en_traffic_type,
                                  oal_uint32 *pul_rate_kbps)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user
       ||  OAL_PTR_NULL == pul_rate_kbps))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY,
                       "{dmac_alg_get_tx_best_rate:: ERROR INFO: pst_mac_user=0x%x, pul_rate_kbps=0x%x.}",
                       pst_mac_user, pul_rate_kbps);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_get_tx_best_rate_func))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_tx_best_rate::p_get_rate_by_table_func is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_get_tx_best_rate_func(pst_mac_user, en_traffic_type, pul_rate_kbps);
}

oal_uint32  dmac_alg_unregister_get_tx_best_rate_notify(oal_void)
{
    g_pst_alg_main->p_get_tx_best_rate_func = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_get_rate_by_table_notify(oal_void)
{
    g_pst_alg_main->p_get_rate_by_table_func = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_get_vap_rate_idx_for_tx_power(mac_user_stru *pst_mac_user,
                                                      hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param,
                                                      oal_uint8 *puc_rate_index,
                                                      oal_uint8 *puc_rate_to_pow_code_idx)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_vap_rate_idx_for_tx_power:: pst_mac_user is NULL.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_get_vap_rate_idx_for_tx_power_func))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_vap_rate_idx_for_tx_power::p_get_vap_rate_idx_for_tx_power_func is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_get_vap_rate_idx_for_tx_power_func(pst_mac_user, pst_tx_dscr_param, puc_rate_index, puc_rate_to_pow_code_idx);
}

oal_uint32  dmac_alg_register_cfg_bandwidth_notify_func(dmac_alg_cfg_bandwidth_notify_enum_uint8    en_notify_sub_type,
                                                               p_alg_cfg_bandwidth_notify_func        p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_BANDWIDTH_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_cfg_bandwidth_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_cfg_user_ant_changed_notify_func(
                dmac_alg_cfg_user_ant_changed_notify_enum_uint8 en_notify_sub_type,
                p_alg_cfg_user_ant_changed_notify_func          p_func)
{
    if (OAL_UNLIKELY((en_notify_sub_type >= DMAC_ALG_CFG_USER_ANT_CHANGED_NOTIFY_BUTT) || (OAL_PTR_NULL == p_func)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "en_notify_sub_type >= DMAC_ALG_CFG_ANT_CHANGED_NOTIFY_BUTT or p_func == OAL_PTR_NULL");
        return OAL_FAIL;
    }

    g_pst_alg_main->pa_cfg_user_ant_changed_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_DBAC

oal_uint32  dmac_alg_unregister_dbac_notify_func(dmac_alg_dbac_status_notify_enum_uint8    en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_DBAC_STATUS_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_dbac_notify_func[en_notify_sub_type] = NULL;
    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_enqueue_tid_notify_func(dmac_alg_enqueue_tid_notify_enum_uint8 en_notify_sub_type,
                                                      p_alg_enqueue_tid_notify_func p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_ENQUEUE_TID_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_alg_enqueue_tid_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_enqueue_tid_notify_func(dmac_alg_enqueue_tid_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_ENQUEUE_TID_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_alg_enqueue_tid_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_vap_down_notify_func(dmac_alg_vap_down_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_VAP_DOWN_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_alg_vap_down_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_probe_req_rx_notify_func(dmac_alg_probe_req_rx_notify_enum_uint8 en_notify_sub_type,
                                                      p_alg_probe_req_rx_notify_func p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_PROBE_REQ_RX_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_alg_probe_req_rx_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_probe_req_rx_notify_func(dmac_alg_vap_down_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_PROBE_REQ_RX_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_alg_probe_req_rx_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}

#endif

oal_uint32  dmac_alg_unregister_vap_up_notify_func(dmac_alg_vap_up_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_VAP_UP_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_alg_vap_up_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_rx_cntl_notify_func(dmac_alg_rx_cntl_notify_enum_uint8 en_notify_sub_type, p_alg_rx_cntl_notify_func p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_RX_CNTL_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->p_rx_cntl_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_tx_notify_func(dmac_alg_tx_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_TX_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_tx_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_tx_mgmt_notify_func(dmac_alg_txmgmt_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_TX_MGMT_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_tx_mgmt_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_tx_complete_notify_func(dmac_alg_tx_complete_notify_enum_uint8  en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_TX_COMPLETE_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_tx_complete_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_rx_notify_func(dmac_alg_rx_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_RX_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_rx_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_tx_schedule_func(oal_void)
{
    g_pst_alg_main->p_tx_schedule_func = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_tid_update_notify_func(oal_void)
{
    g_pst_alg_main->p_tid_update_func = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_downlink_flowctl_notify_func(oal_void)
{
    g_pst_alg_main->p_downlink_flowctl_func = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_flowctl_netbuff_free_notify_func(oal_void)
{
    g_pst_alg_main->p_mu_flowctl_netbuff_free_notify_func = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_add_vap_notify_func(dmac_alg_add_vap_notify_enum_uint8  en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_ADD_VAP_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_create_vap_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_cfg_channel_notify_func(dmac_alg_cfg_channel_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_CHANNEL_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_cfg_channel_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_cfg_bandwidth_notify_func(dmac_alg_cfg_bandwidth_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_BANDWIDTH_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_cfg_bandwidth_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_cfg_user_bandwidth_notify_func(dmac_alg_cfg_user_bandwidth_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_USER_BANDWIDTH_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_cfg_user_bandwidth_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_cfg_user_ant_changed_notify_func(dmac_alg_cfg_user_ant_changed_notify_enum_uint8 en_notify_sub_type)
{
    if (OAL_UNLIKELY(en_notify_sub_type >= DMAC_ALG_CFG_USER_ANT_CHANGED_NOTIFY_BUTT))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "en_notify_sub_type >= DMAC_ALG_CFG_ANT_CHANGED_NOTIFY_BUTT");
        return OAL_FAIL;
    }

    g_pst_alg_main->pa_cfg_user_ant_changed_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_uint32  dmac_alg_unregister_cfg_get_ant_info_notify_func(oal_void)
{
    g_pst_alg_main->p_cfg_get_ant_info_notify_func = OAL_PTR_NULL;

    return OAL_SUCC;
}
oal_uint32  dmac_alg_unregister_cfg_double_ant_switch_notify_func(oal_void)
{
    g_pst_alg_main->p_cfg_double_ant_switch_notify_func = OAL_PTR_NULL;
    return OAL_SUCC;
}
#endif

oal_uint32  dmac_alg_unregister_cfg_user_protocol_notify_func(dmac_alg_cfg_user_protocol_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_USER_PROTOCOL_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_cfg_user_protocol_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_cfg_user_spatial_stream_notify_func(dmac_alg_cfg_user_spatial_stream_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_USER_SPATIAL_STREAM_NOTIFY_BUTT, OAL_FAIL);

    g_pst_alg_main->pa_cfg_user_spatial_stream_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_del_vap_notify_func(dmac_alg_del_vap_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_DEL_VAP_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_delete_vap_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_del_user_notify_func(dmac_alg_del_user_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_DEL_USER_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_delete_assoc_user_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_add_user_notify_func(dmac_alg_add_user_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_ADD_USER_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_add_assoc_user_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}



oal_uint32  dmac_alg_unregister_rx_mgmt_notify_func(dmac_alg_rx_mgmt_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_RX_MGMT_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->p_rx_mgmt_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_rx_cntl_notify_func(dmac_alg_rx_cntl_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_RX_CNTL_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->p_rx_cntl_func[en_notify_sub_type] = OAL_PTR_NULL;
	return OAL_SUCC;

}


oal_uint32  dmac_alg_unregister_user_active_change_notify_func(dmac_alg_user_active_change_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_USER_ACTIVE_CHANGE_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_alg_user_active_change_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_chip_priv_stru(mac_chip_stru                 *pst_mac_chip,
                                                  dmac_alg_chip_stru_id_enum_uint8    en_chip_stru_type)
{
    dmac_alg_chip_stru  *pst_chip_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_mac_chip)
        || (en_chip_stru_type >= DMAC_ALG_CHIP_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_alg_unregister_chip_priv_stru:: ERROR INFO: pst_mac_chip=0x%x, en_dev_chip_type=%d.}",
                       pst_mac_chip, en_chip_stru_type);
        return OAL_FAIL;
    }

    pst_chip_info = (dmac_alg_chip_stru *)MAC_DEV_ALG_PRIV(pst_mac_chip);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_chip_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_unregister_device_priv_stru:: pst_chip_info is NULL!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_chip_info->p_alg_info[en_chip_stru_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_BTCOEX

oal_uint32  dmac_alg_cfg_btcoex_state_notify(hal_to_dmac_device_stru *pst_hal_device, dmac_alg_bt_state_type_uint8 en_type)
{
    dmac_alg_stru  *pst_alg_stru;

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_btcoex_state_notify_func[DMAC_ALG_CFG_START_CONNECT_NOTIFY_TPC])
    {
        pst_alg_stru->pa_cfg_btcoex_state_notify_func[DMAC_ALG_CFG_START_CONNECT_NOTIFY_TPC](pst_hal_device, en_type);
    }

    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_BTCOEX */
#ifdef _PRE_WLAN_FEATURE_INTF_DET

oal_uint32  dmac_alg_unregister_cfg_intf_det_mode_notify_func(dmac_alg_cfg_intf_det_mode_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_INTF_DET_MODE_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_cfg_intf_det_mode_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET

oal_uint32  dmac_alg_unregister_intf_det_pk_mode_notify_func(oal_void)
{
    g_pst_alg_main->p_cfg_intf_det_pk_mode_notify_func = OAL_PTR_NULL;
    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_intf_det_pk_mode_notify_func(p_alg_intf_det_pk_mode_notify_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_cfg_intf_det_pk_mode_notify_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_cfg_intf_det_pk_mode_notify(dmac_vap_stru *pst_dmac_vap, oal_bool_enum_uint8 en_is_pk_mode)
{
    if (g_pst_alg_main->p_cfg_intf_det_pk_mode_notify_func)
    {
        g_pst_alg_main->p_cfg_intf_det_pk_mode_notify_func(pst_dmac_vap, en_is_pk_mode);
    }

    return OAL_SUCC;
}
#endif
#endif //_PRE_WLAN_FEATURE_INTF_DET

oal_uint32  dmac_alg_cfg_user_ant_changed_notify(mac_vap_stru *pst_vap, mac_user_stru *pst_mac_user)
{
    dmac_alg_stru            *pst_alg_stru;
    oal_uint8                 uc_index;
    oal_uint32                ul_ret;

    pst_alg_stru = g_pst_alg_main;

    /* 调用相关回调函数 目前只需要通知txbf，后续可能需要通知其他模块，预留接口*/
    for (uc_index = 0; uc_index < DMAC_ALG_CFG_USER_ANT_CHANGED_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_user_ant_changed_notify_func[uc_index])
        {
            ul_ret = pst_alg_stru->pa_cfg_user_ant_changed_notify_func[uc_index](pst_vap, pst_mac_user);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG2(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{dmac_alg_cfg_user_ant_changed_notify::ul_ret=%d, uc_index=%d}", ul_ret, uc_index);
            }
        }
    }

    return OAL_SUCC;
}

oal_uint32  dmac_alg_create_ba(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_uint8 uc_tid)
{
    frw_event_mem_stru          *pst_event_mem;          /* 申请事件返回的内存指针 */
    frw_event_stru              *pst_dmac_to_hmac_event;  /* 指向申请事件的payload指针 */
    dmac_to_hmac_ctx_event_stru *pst_create_ba_event;
    oal_uint32                   ul_ret;
    dmac_tid_stru               *pst_tid;

    dmac_user_get_tid_by_num(pst_mac_user, uc_tid, &pst_tid);

    if (OAL_PTR_NULL != pst_tid->pst_ba_tx_hdl)
    {
        return OAL_SUCC;
    }

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_to_hmac_ctx_event_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "alloc event failed!");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_dmac_to_hmac_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_dmac_to_hmac_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_SDT_REG,
                       DMAC_TO_HMAC_CREATE_BA,
                       OAL_SIZEOF(dmac_to_hmac_ctx_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_create_ba_event = (dmac_to_hmac_ctx_event_stru *)(pst_dmac_to_hmac_event->auc_event_data);

    pst_create_ba_event->us_user_index  = pst_mac_user->us_assoc_id;
    pst_create_ba_event->uc_tid         = uc_tid;
    pst_create_ba_event->uc_vap_id      = pst_mac_vap->uc_vap_id;

    /* 分发 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_SUCC != ul_ret)
    {
        FRW_EVENT_FREE(pst_event_mem);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_create_ba::frw_event_dispatch_event fail, ul_ret=%d}\r\n", ul_ret);
        return ul_ret;
    }

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}
//#if ALG_SCHEUDLE_EDCA_FEATURE

oal_uint32  dmac_alg_set_qap_wme_info(
                mac_vap_stru               *pst_vap,
                wlan_wme_ac_type_enum_uint8 en_wme_type,
                mac_wme_param_stru         *pst_wme_info)
{
    oal_uint32      aul_param[3];
    oal_uint32      ul_ret;

    if ((OAL_PTR_NULL == pst_vap)||(OAL_PTR_NULL == pst_wme_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "OAL_PTR_NULL == pst_vap");
        return OAL_FAIL;
    }

    if (en_wme_type >= WLAN_WME_AC_BUTT)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_set_qap_wme_info::en_wme_type=%d}", en_wme_type);
        return OAL_FAIL;
    }

    /* 调用配置接口 */
    aul_param[0] = WLAN_CFGID_EDCA_TABLE_CWMIN;
    aul_param[1] = en_wme_type;
    aul_param[2] = pst_wme_info->ul_logcwmin;
    ul_ret = dmac_config_set_qap_cwmin(pst_vap, OAL_SIZEOF(aul_param), (oal_uint8 *)aul_param);
    if(ul_ret != OAL_SUCC)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_set_qap_wme_info::dmac_config_set_qap_cwmin fail,ul_ret=%d}", ul_ret);
        return ul_ret;
    }

    aul_param[0] = WLAN_CFGID_EDCA_TABLE_CWMAX;
    aul_param[1] = en_wme_type;
    aul_param[2] = pst_wme_info->ul_logcwmax;
    ul_ret = dmac_config_set_qap_cwmax(pst_vap, OAL_SIZEOF(aul_param), (oal_uint8 *)aul_param);
    if(ul_ret != OAL_SUCC)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_set_qap_wme_info::dmac_config_set_qap_cwmax fail,ul_ret=%d}", ul_ret);
        return ul_ret;
    }

    aul_param[0] = WLAN_CFGID_EDCA_TABLE_AIFSN;
    aul_param[1] = en_wme_type;
    aul_param[2] = pst_wme_info->ul_aifsn;
    ul_ret = dmac_config_set_qap_aifsn(pst_vap, OAL_SIZEOF(aul_param), (oal_uint8 *)aul_param);
    if(ul_ret != OAL_SUCC)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_set_qap_wme_info::dmac_config_set_qap_aifsn fail,ul_ret=%d}", ul_ret);
        return ul_ret;
    }

    aul_param[0] = WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT;
    aul_param[1] = en_wme_type;
    aul_param[2] = pst_wme_info->ul_txop_limit;
    dmac_config_set_qap_txop_limit(pst_vap, OAL_SIZEOF(aul_param), (oal_uint8 *)aul_param);

    return OAL_SUCC;
}
//#endif

oal_void  dmac_chip_alg_exit(mac_chip_stru  *pst_chip)
{
    dmac_alg_chip_stru *p_alg_chip_stru;     /* CHIP级别算法的私有数据结构 */

    if (OAL_PTR_NULL == pst_chip)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_alg_exit: OAL_PTR_NULL == pst_device");
        return;
    }

    p_alg_chip_stru = MAC_CHIP_ALG_PRIV(pst_chip);
    if (OAL_UNLIKELY(OAL_PTR_NULL == p_alg_chip_stru))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_chip_exit::OAL_PTR_NULL == p_alg_chip_stru}\r\n");
        return;
    }

    OAL_MEM_FREE(p_alg_chip_stru, OAL_TRUE);

    return;
}

oal_uint32   dmac_alg_get_hal_queue_ppdu_num(oal_uint16 us_assoc_id, oal_uint8 uc_ac_num, oal_uint8 *puc_ppdu_num)
{
    dmac_vap_stru             *pst_dmac_vap;
    mac_user_stru             *pst_user;
    pst_user = (mac_user_stru *)mac_res_get_mac_user(us_assoc_id);
    if(OAL_PTR_NULL == pst_user)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_alg_get_hal_queue_ppdu_num: pst_user[%d] null.}", us_assoc_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_dmac_vap    = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_user->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_alg_get_hal_queue_ppdu_num::mac_res_get_dmac_vap fail. vap_id[%u]}",pst_user->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    *puc_ppdu_num   = pst_dmac_vap->pst_hal_device->ast_tx_dscr_queue[HAL_AC_TO_Q_NUM(uc_ac_num)].uc_ppdu_cnt;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_SMPS

oal_uint32  dmac_alg_register_cfg_smps_notify_func(dmac_alg_cfg_smps_notify_enum_uint8    en_notify_sub_type,
                                                   p_alg_cfg_smps_notify_func        p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_SMPS_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_cfg_smps_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_cfg_smps_notify_func(dmac_alg_cfg_smps_notify_enum_uint8    en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_SMPS_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_cfg_smps_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_cfg_smps_notify(mac_vap_stru *pst_vap, mac_user_stru *pst_user)
{
    dmac_alg_stru  *pst_alg_stru;
    oal_uint8       uc_index;

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_CFG_SMPS_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_smps_notify_func[uc_index])
        {
            pst_alg_stru->pa_cfg_smps_notify_func[uc_index](pst_vap, pst_user);
        }
    }

    return OAL_SUCC;
}

#endif

oal_uint32  dmac_alg_unregister_get_user_rate_idx_for_tx_power_notify(oal_void)
{
    g_pst_alg_main->p_get_user_rate_idx_for_tx_power_func = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_get_vap_rate_idx_for_tx_power_notify(oal_void)
{
    g_pst_alg_main->p_get_vap_rate_idx_for_tx_power_func = OAL_PTR_NULL;

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_alg_unregister_cfg_start_connect_notify_func(dmac_alg_cfg_channel_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_START_CONNECT_NOTIFY_BUTT, OAL_FAIL);

    g_pst_alg_main->pa_cfg_start_connect_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_BTCOEX

oal_uint32  dmac_alg_unregister_btcoex_state_notify_func(dmac_alg_cfg_btcoex_state_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_BTCOEX_STATE_NOTIFY_BUTT, OAL_FAIL);

    g_pst_alg_main->pa_cfg_btcoex_state_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_uint32 dmac_alg_unregister_cfg_dual_antenna_state_notify_func()
{
    g_pst_alg_main->p_cfg_dual_antenna_state_notify_func = OAL_PTR_NULL;
    return OAL_SUCC;
}
#endif

oal_uint32  dmac_alg_unregister_nerbuff_usage_notify_func(dmac_alg_cfg_netbuff_info_notify_enum_uint8  en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_NETBUFF_INFO_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_cfg_netbuff_usage_info_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}
#endif /*_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE */

/*lint -e19*/
oal_module_symbol(dmac_alg_register_dmac_misc_event);
oal_module_symbol(dmac_alg_unregister_dmac_misc_event);
oal_module_symbol(dmac_alg_unregister_get_rate_kbps_func);
oal_module_symbol(dmac_alg_get_rate_kbps);
oal_module_symbol(dmac_alg_unregister_delete_ba_fail_notify);
oal_module_symbol(dmac_alg_unregister_get_tx_best_rate_notify);
oal_module_symbol(dmac_alg_unregister_get_rate_by_table_notify);
oal_module_symbol(dmac_alg_unregister_vap_up_notify_func);
oal_module_symbol(dmac_alg_unregister_tx_notify_func);
oal_module_symbol(dmac_alg_unregister_tx_schedule_func);
oal_module_symbol(dmac_alg_unregister_tx_mgmt_notify_func);
oal_module_symbol(dmac_alg_unregister_tx_complete_notify_func);
oal_module_symbol(dmac_alg_unregister_rx_notify_func);
oal_module_symbol(dmac_alg_unregister_flowctl_netbuff_free_notify_func);
oal_module_symbol(dmac_alg_unregister_add_vap_notify_func);
oal_module_symbol(dmac_alg_unregister_downlink_flowctl_notify_func);
oal_module_symbol(dmac_alg_unregister_tid_update_notify_func);
oal_module_symbol(dmac_alg_unregister_cfg_channel_notify_func);
oal_module_symbol(dmac_alg_unregister_cfg_user_bandwidth_notify_func);
oal_module_symbol(dmac_alg_unregister_cfg_user_ant_changed_notify_func);
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_module_symbol(dmac_alg_unregister_cfg_get_ant_info_notify_func);
oal_module_symbol(dmac_alg_unregister_cfg_double_ant_switch_notify_func);
#endif
oal_module_symbol(dmac_alg_unregister_cfg_user_protocol_notify_func);
oal_module_symbol(dmac_alg_unregister_cfg_user_spatial_stream_notify_func);
oal_module_symbol(dmac_alg_unregister_del_vap_notify_func);
oal_module_symbol(dmac_alg_unregister_del_user_notify_func);
oal_module_symbol(dmac_alg_unregister_add_user_notify_func);
oal_module_symbol(dmac_alg_unregister_rx_mgmt_notify_func);
oal_module_symbol(dmac_alg_register_user_active_change_notify_func);
oal_module_symbol(dmac_alg_cfg_user_ant_changed_notify);

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
oal_module_symbol(dmac_alg_unregister_anti_intf_switch_notify);
#endif
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
oal_module_symbol(dmac_alg_unregister_edca_stat_event_notify);
#endif
oal_module_symbol(dmac_alg_register_cfg_user_ant_changed_notify_func);
#ifdef _PRE_WLAN_FEATURE_DBAC
oal_module_symbol(dmac_alg_unregister_dbac_notify_func);
oal_module_symbol(dmac_alg_register_enqueue_tid_notify_func);
oal_module_symbol(dmac_alg_unregister_enqueue_tid_notify_func);
oal_module_symbol(dmac_alg_unregister_vap_down_notify_func);
oal_module_symbol(dmac_alg_register_probe_req_rx_notify_func);
oal_module_symbol(dmac_alg_unregister_probe_req_rx_notify_func);
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
oal_module_symbol(dmac_alg_unregister_cfg_start_connect_notify_func);
#endif
#ifdef _PRE_WLAN_FEATURE_INTF_DET
oal_module_symbol(dmac_alg_unregister_cfg_intf_det_mode_notify_func);
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
oal_module_symbol(dmac_alg_unregister_intf_det_pk_mode_notify_func);
oal_module_symbol(dmac_alg_register_intf_det_pk_mode_notify_func);
oal_module_symbol(dmac_alg_cfg_intf_det_pk_mode_notify);
#endif
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
oal_module_symbol(dmac_alg_register_intf_det_psd_notify_func);
oal_module_symbol(dmac_alg_unregister_intf_det_psd_notify_func);
oal_module_symbol(dmac_alg_cfg_intf_det_psd_notify);
#endif
#endif
//#if ALG_SCHEUDLE_EDCA_FEATURE
oal_module_symbol(dmac_alg_set_qap_wme_info);
//#endif
oal_module_symbol(dmac_alg_get_hal_queue_ppdu_num);
oal_module_symbol(dmac_alg_unregister_get_user_rate_idx_for_tx_power_notify);
oal_module_symbol(dmac_alg_unregister_get_vap_rate_idx_for_tx_power_notify);
oal_module_symbol(dmac_alg_register_rx_cntl_notify_func);
oal_module_symbol(dmac_alg_unregister_rx_cntl_notify_func);

/*lint +e19*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

