


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
#define THIS_FILE_ID OAM_FILE_ID_DMAC_ALG_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
    /* 注册算法主结构体钩子, 钩子作为实际引用对象 */
dmac_alg_stru  *g_pst_alg_main = &g_st_alg_main;


#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
oal_uint32  gul_dmac_alg_pktno = 0;
#else
oal_uint8  guc_dmac_alg_pktno = 0;
#endif


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void dmac_alg_free_tid_priv_stru(dmac_user_stru *pst_user)
{
    oal_uint8   uc_index;

    for (uc_index = 0; uc_index < WLAN_TID_MAX_NUM; uc_index++)
    {
        if (OAL_PTR_NULL == pst_user->ast_tx_tid_queue[uc_index].p_alg_priv)
        {
            break;
        }

        OAL_MEM_FREE(pst_user->ast_tx_tid_queue[uc_index].p_alg_priv, OAL_TRUE);

        pst_user->ast_tx_tid_queue[uc_index].p_alg_priv = OAL_PTR_NULL;
    }

}


oal_uint32  dmac_alg_register_tx_mgmt_notify_func(dmac_alg_txmgmt_notify_enum_uint8  en_notify_sub_type,
                                                                p_alg_txmgmt_notify_func   p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_TX_MGMT_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);

    g_pst_alg_main->pa_tx_mgmt_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_tx_notify_func(dmac_alg_tx_notify_enum_uint8    en_notify_sub_type,
                                                      p_alg_tx_notify_func             p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_TX_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);

    g_pst_alg_main->pa_tx_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_tx_complete_notify_func(dmac_alg_tx_complete_notify_enum_uint8    en_notify_sub_type,
                                                                  p_alg_tx_complete_notify_func             p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_TX_COMPLETE_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_tx_complete_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_rx_notify_func(dmac_alg_rx_notify_enum_uint8    en_notify_sub_type,
                                                      p_alg_rx_notify_func             p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_RX_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_rx_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_tx_schedule_func(p_alg_tx_schedule_func  p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_tx_schedule_func = p_func;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC

oal_uint32  dmac_alg_register_tx_schedule_stat_event_notify_func(p_alg_tx_schedule_stat_event_notify_func  p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_tx_schedule_stat_event_notify_func = p_func;

    return OAL_SUCC;
}
#endif


oal_uint32  dmac_alg_register_tid_update_notify_func(p_alg_update_tid_notify_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_tid_update_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_downlink_flowctl_notify_func(p_alg_downlink_flowctl_notify_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_downlink_flowctl_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_mu_flowctl_netbuff_free_notify_func(p_alg_mu_flowctl_netbuff_free_notify_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_mu_flowctl_netbuff_free_notify_func = p_func;

    return OAL_SUCC;
}



oal_uint32  dmac_alg_register_add_vap_notify_func(dmac_alg_add_vap_notify_enum_uint8    en_notify_sub_type,
                                                          p_alg_create_vap_notify_func        p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_ADD_VAP_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_create_vap_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_del_vap_notify_func(dmac_alg_del_vap_notify_enum_uint8   en_notify_sub_type,
                                                          p_alg_delete_vap_notify_func       p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_DEL_VAP_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_delete_vap_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_cfg_channel_notify_func(dmac_alg_cfg_channel_notify_enum_uint8  en_notify_sub_type,
                                                            p_alg_cfg_channel_notify_func      p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_CHANNEL_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_cfg_channel_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_alg_register_nerbuff_usage_notify_func(dmac_alg_cfg_netbuff_info_notify_enum_uint8  en_notify_sub_type,
                                                            p_alg_cfg_netbuff_usage_info_notify_func p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_NETBUFF_INFO_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_cfg_netbuff_usage_info_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_cfg_start_connect_notify_func(dmac_alg_cfg_start_connect_notify_enum_uint8  en_notify_sub_type,
                                                                            p_alg_cfg_start_connect_notify_func     p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_START_CONNECT_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);

    g_pst_alg_main->pa_cfg_start_connect_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_BTCOEX

oal_uint32  dmac_alg_register_cfg_btcoex_state_notify_func(dmac_alg_cfg_btcoex_state_notify_enum_uint8  en_notify_sub_type,
                                                                            p_alg_cfg_btcoex_state_notify_func     p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_BTCOEX_STATE_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);

    g_pst_alg_main->pa_cfg_btcoex_state_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}
#endif
#endif /* _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE */
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_uint32 dmac_alg_register_cfg_dual_antenna_state_notify_func(p_alg_cfg_dual_antenna_state_notify_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_cfg_dual_antenna_state_notify_func = p_func;
    return OAL_SUCC;
}
#endif


oal_uint32  dmac_alg_register_cfg_user_bandwidth_notify_func(
                dmac_alg_cfg_user_bandwidth_notify_enum_uint8 en_notify_sub_type,
                p_alg_cfg_user_bandwidth_notify_func          p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_USER_BANDWIDTH_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_cfg_user_bandwidth_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_uint32  dmac_alg_register_cfg_get_ant_info_notify_func(
                p_alg_get_ant_info_notify_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_cfg_get_ant_info_notify_func = p_func;
    return OAL_SUCC;
}
oal_uint32  dmac_alg_register_cfg_double_ant_switch_notify_func(
                p_alg_double_ant_switch_notify_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_cfg_double_ant_switch_notify_func = p_func;
    return OAL_SUCC;
}
#endif


oal_uint32  dmac_alg_register_cfg_user_protocol_notify_func(dmac_alg_cfg_user_protocol_notify_enum_uint8 en_notify_sub_type,
                                                               p_alg_cfg_user_protocol_notify_func        p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_USER_PROTOCOL_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_cfg_user_protocol_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_cfg_user_spatial_stream_notify_func(
                dmac_alg_cfg_user_spatial_stream_notify_enum_uint8  en_notify_sub_type,
                p_alg_cfg_user_spatial_stream_notify_func          p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_USER_SPATIAL_STREAM_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_cfg_user_spatial_stream_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_add_user_notify_func(
                dmac_alg_add_user_notify_enum_uint8     en_notify_sub_type,
                p_alg_add_assoc_user_notify_func        p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_ADD_USER_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_add_assoc_user_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_del_user_notify_func(dmac_alg_del_user_notify_enum_uint8   en_notify_sub_type,
                                                                    p_alg_delete_assoc_user_notify_func       p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_DEL_USER_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_delete_assoc_user_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_rx_mgmt_notify_func(dmac_alg_rx_mgmt_notify_enum_uint8 en_notify_sub_type, p_alg_rx_mgmt_notify_func p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_RX_MGMT_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->p_rx_mgmt_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_config_notify_func(dmac_alg_config_id_enum_uint8 en_notify_sub_type, p_alg_config_func p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CONFIG_ID_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_alg_config_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_unregister_config_notify_func(dmac_alg_config_id_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CONFIG_ID_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_alg_config_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}



#ifdef _PRE_WLAN_FEATURE_DBAC

oal_uint32  dmac_alg_register_dbac_notify_func(dmac_alg_dbac_status_notify_enum_uint8    en_notify_sub_type,
                                                      p_alg_dbac_status_notify_func             p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_DBAC_STATUS_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);

    g_pst_alg_main->pa_dbac_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_dbac_status_notify(mac_device_stru *pst_dev)
{
    dmac_alg_stru  *pst_alg_stru;
    oal_uint8       uc_index;


    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY,
                       "{dmac_alg_dbac_status_notify:: ERROR INFO: pst_dev=null}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_DBAC_STATUS_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_dbac_notify_func[uc_index])
        {
            pst_alg_stru->pa_dbac_notify_func[uc_index](pst_dev);
        }
    }

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_vap_down_notify_func(dmac_alg_vap_down_notify_enum_uint8 en_notify_sub_type,
                                                      p_alg_vap_down_notify_func p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_VAP_DOWN_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_alg_vap_down_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_update_dbac_fcs_config_func(p_alg_update_dbac_config_func p_func)
{
    g_pst_alg_main->p_update_dbac_fcs_config_func = p_func;

    return OAL_SUCC;
}

#endif


oal_uint32  dmac_alg_register_vap_up_notify_func(dmac_alg_vap_up_notify_enum_uint8 en_notify_sub_type,
                                                      p_alg_vap_up_notify_func p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_VAP_UP_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_alg_vap_up_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_CHIP_TEST_ALG

oal_void  dmac_alg_register_rx_event_notify(p_alg_rx_event_notify_func p_func)
{
    g_pst_alg_main->p_rx_event_notify_func = p_func;
}


oal_void  dmac_alg_unregister_rx_event_notify(oal_void)
{
    g_pst_alg_main->p_rx_event_notify_func = OAL_PTR_NULL;
}

#endif

#ifdef _PRE_WLAN_FEATURE_FLOWCTL


oal_uint32  dmac_alg_register_flowctl_backp_notify_func(p_alg_flowctl_backp_notify_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_alg_flowctl_backp_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_flowctl_backp_notify_func()
{
    g_pst_alg_main->p_alg_flowctl_backp_func = OAL_PTR_NULL;

    return OAL_SUCC;
}



oal_uint32  dmac_alg_flowctl_backp(mac_vap_stru *pst_mac_vap, oal_uint16 us_assoc_id, oal_uint8 uc_tidno, oal_uint8 uc_is_stop)
{
    frw_event_mem_stru          *pst_event_mem;          /* 申请事件返回的内存指针 */
    frw_event_stru              *pst_dmac_to_hmac_event;  /* 指向申请事件的payload指针 */
    mac_ioctl_queue_backp_stru  *pst_flowctl_backp_event;
    oal_uint32                   ul_ret;

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(mac_ioctl_queue_backp_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_flowctl_backp::alloc event failed!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_dmac_to_hmac_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_dmac_to_hmac_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CRX,
                       DMAC_WLAN_CRX_EVENT_SUB_TYPE_FLOWCTL_BACKP,
                       OAL_SIZEOF(mac_ioctl_queue_backp_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_flowctl_backp_event = (mac_ioctl_queue_backp_stru *)(pst_dmac_to_hmac_event->auc_event_data);

    pst_flowctl_backp_event->uc_vap_id      = pst_mac_vap->uc_vap_id;
    pst_flowctl_backp_event->us_assoc_id    = us_assoc_id;
    pst_flowctl_backp_event->uc_tidno       = uc_tidno;
    pst_flowctl_backp_event->uc_is_stop     = uc_is_stop;

    /* 分发 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_SUCC != ul_ret)
    {
        FRW_EVENT_FREE(pst_event_mem);
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_flowctl_backp::frw_event_dispatch_event fail, ul_ret=%d}", ul_ret);
        return ul_ret;
    }

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}

#endif
#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC

oal_uint32  dmac_alg_unregister_tx_schedule_stat_event_notify_func(oal_void)
{

    g_pst_alg_main->p_tx_schedule_stat_event_notify_func = OAL_PTR_NULL;

    return OAL_SUCC;
}
#endif

oal_uint32  dmac_alg_register_user_active_change_notify_func(
                dmac_alg_user_active_change_notify_enum_uint8   en_notify_sub_type,
                p_alg_user_active_change_notify_func            p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_USER_ACTIVE_CHANGE_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_alg_user_active_change_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND

oal_uint32  dmac_alg_register_user_replace_notify_func(
                dmac_alg_user_replace_notify_enum_uint8         en_notify_sub_type,
                p_alg_user_replace_notify_func                  p_func)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_USER_REPLACE_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_alg_user_replace_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_user_replace_notify_func(dmac_alg_user_active_change_notify_enum_uint8 en_notify_sub_type)
{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_USER_REPLACE_NOTIFY_BUTT, OAL_FAIL);
    g_pst_alg_main->pa_alg_user_replace_notify_func[en_notify_sub_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}
#endif

oal_uint32  dmac_alg_register_timer(oal_uint32  ul_file_id,
                                   oal_uint32  ul_line_num,
                                   oam_module_id_enum_uint16 en_module_id,
                                   oal_uint32 ul_core_id,
                                   p_alg_timer_notify_func p_timer_notify_func,
                                   oal_void *p_arg,
                                   oal_bool_enum_uint8 en_is_periodic,
                                   dmac_alg_timer_stru **ppst_timer)
{
    dmac_alg_timer_stru *pst_timer;

    if (OAL_UNLIKELY(OAL_PTR_NULL == ppst_timer
        || OAL_PTR_NULL == p_timer_notify_func))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_alg_register_timer ERROR");
        return OAL_FAIL;
    }

    pst_timer = (dmac_alg_timer_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_alg_timer_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_timer)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_alg_register_timer OAL_MEM_ALLOC ERROR");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_timer->en_module_id   = en_module_id;
    pst_timer->p_func         = p_timer_notify_func;
    pst_timer->p_timeout_arg  = p_arg;
    pst_timer->en_is_periodic = en_is_periodic;
    pst_timer->ul_file_id     = ul_file_id;
    pst_timer->ul_line_num    = ul_line_num;
    pst_timer->en_is_enabled  = OAL_FALSE;
    pst_timer->en_is_registerd= OAL_FALSE;
    pst_timer->ul_core_id     = ul_core_id;
    pst_timer->ul_timeout     = DMAC_ALG_TIMER_DEFAULT_TIMEOUT;
    pst_timer->ul_time_stamp  = (oal_uint32)OAL_TIME_GET_STAMP_MS()+ DMAC_ALG_TIMER_DEFAULT_TIMEOUT;

    *ppst_timer = pst_timer;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_timer(dmac_alg_timer_stru *pst_timer)
{

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_timer))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_alg_unregister_timer ERROR");
        return OAL_FAIL;
    }

    /* 判断定时器是否停止 */
    if (pst_timer->en_is_enabled == OAL_TRUE)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_unregister_timer::pst_timer->en_is_enabled == OAL_TRUE}\r\n");
        return OAL_FAIL;
    }

    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(pst_timer);

    OAL_MEM_FREE(pst_timer, OAL_TRUE);


    return OAL_SUCC;
}


oal_uint32  dmac_alg_start_timer(dmac_alg_timer_stru *pst_timer, oal_uint16 us_timeout_ms)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_timer
        || 0 == us_timeout_ms))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_alg_start_timer ERROR");
        return OAL_FAIL;
    }

    /*重新启动定时器，插入链表*/
    pst_timer->ul_timeout         = us_timeout_ms;
    pst_timer->ul_time_stamp      = (oal_uint32)OAL_TIME_GET_STAMP_MS()+ us_timeout_ms;
    //pst_timer->en_is_periodic     = pst_timer->en_is_periodic;
    pst_timer->en_is_enabled      = OAL_TRUE;
    if (OAL_TRUE != pst_timer->en_is_registerd)
    {
        /* 未注册则注册，并加入链表 */
        pst_timer->en_is_registerd= OAL_TRUE;
        frw_timer_add_timer(pst_timer);
    }
    else
    {
        /* 注册则先删除后加入链表 */
        oal_dlist_delete_entry(&pst_timer->st_entry);
        frw_timer_add_timer(pst_timer);
    }

    return OAL_SUCC;
}


oal_uint32  dmac_alg_stop_timer(dmac_alg_timer_stru *pst_timer)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_timer))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_alg_stop_timer ERROR");
        return OAL_FAIL;
    }

    frw_timer_stop_timer(pst_timer);

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_chip_priv_stru(mac_chip_stru                 *pst_mac_chip,
                                                  dmac_alg_chip_stru_id_enum_uint8    en_chip_stru_type,
                                                  oal_void                           *p_chip_stru)
{
    dmac_alg_chip_stru  *pst_chip_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_mac_chip)
        || (OAL_PTR_NULL == p_chip_stru)
        || (en_chip_stru_type >= DMAC_ALG_CHIP_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{dmac_alg_register_chip_priv_stru:: ERROR INFO: pst_mac_chip=0x%x, p_chip_stru=0x%x, en_chip_stru_type=%d.}",
                       pst_mac_chip, p_chip_stru, en_chip_stru_type);
        return OAL_FAIL;
    }

    pst_chip_info = (dmac_alg_chip_stru *)MAC_CHIP_ALG_PRIV(pst_mac_chip);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_chip_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_register_chip_priv_stru:: pst_chip_info is NULL!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_chip_info->p_alg_info[en_chip_stru_type] = p_chip_stru;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_device_priv_stru(hal_to_dmac_device_stru    *pst_hal_dev,
                                                  hal_alg_device_stru_id_enum_uint8  en_dev_stru_type,
                                                  oal_void                           *p_dev_stru)
{
    hal_alg_device_stru  *pst_dev_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_hal_dev)
        || (OAL_PTR_NULL == p_dev_stru)
        || (en_dev_stru_type >= HAL_ALG_DEVICE_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{dmac_alg_register_device_priv_stru:: ERROR INFO: pst_hal_dev=0x%x, p_dev_stru=0x%x, en_dev_stru_type=%d.}",
                       pst_hal_dev, p_dev_stru, en_dev_stru_type);
        return OAL_FAIL;
    }

    pst_dev_info = (hal_alg_device_stru *)HAL_DEV_ALG_PRIV(pst_hal_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dev_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_register_device_priv_stru:: pst_dev_info is NULL!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dev_info->p_alg_info[en_dev_stru_type] = p_dev_stru;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_device_priv_stru(hal_to_dmac_device_stru  *pst_hal_dev,
                                                  hal_alg_device_stru_id_enum_uint8    en_dev_stru_type)
{
    hal_alg_device_stru  *pst_dev_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_hal_dev)
        || (en_dev_stru_type >= HAL_ALG_DEVICE_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_alg_unregister_device_priv_stru:: ERROR INFO: pst_hal_dev=0x%x, en_dev_stru_type=%d.}",
                       pst_hal_dev, en_dev_stru_type);
        return OAL_FAIL;
    }

    pst_dev_info = (hal_alg_device_stru *)HAL_DEV_ALG_PRIV(pst_hal_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dev_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_unregister_device_priv_stru:: pst_dev_info is NULL!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dev_info->p_alg_info[en_dev_stru_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_vap_priv_stru(mac_vap_stru                 *pst_vap,
                                              dmac_alg_vap_stru_id_enum_uint8    en_vap_stru_type,
                                              oal_void                     *p_vap_stru)
{
    dmac_alg_vap_stru  *pst_alg_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_vap)
        || (OAL_PTR_NULL == p_vap_stru)
        || (en_vap_stru_type >= DMAC_ALG_VAP_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{dmac_alg_register_vap_priv_stru:: ERROR INFO: pst_vap=0x%x, p_vap_stru=0x%x, en_vap_stru_type=%d.}",
                       pst_vap, p_vap_stru, en_vap_stru_type);
        return OAL_FAIL;
    }

    pst_alg_info = ((dmac_vap_stru *)pst_vap)->p_alg_priv;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_register_vap_priv_stru:: pst_alg_info is NULL!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_alg_info->p_alg_info[en_vap_stru_type] = p_vap_stru;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_vap_priv_stru(mac_vap_stru                 *pst_vap,
                                              dmac_alg_vap_stru_id_enum_uint8    en_vap_stru_type)
{
    dmac_alg_vap_stru  *pst_alg_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_vap)
        || (en_vap_stru_type >= DMAC_ALG_VAP_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_alg_unregister_vap_priv_stru:: ERROR INFO: pst_vap=0x%x, en_vap_stru_type=%d.}",
                       pst_vap, en_vap_stru_type);

        return OAL_FAIL;
    }

    pst_alg_info = ((dmac_vap_stru *)pst_vap)->p_alg_priv;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_unregister_vap_priv_stru:: pst_alg_info is NULL!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_alg_info->p_alg_info[en_vap_stru_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_user_priv_stru(mac_user_stru                     *pst_user,
                                                       dmac_alg_user_stru_id_enum_uint8   en_user_stru_type,
                                                       oal_void                          *p_user_stru)
{
    dmac_user_stru         *pst_dmac_user;
    dmac_alg_user_stru     *pst_alg_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_user)
                  || (OAL_PTR_NULL == p_user_stru)
                  || (en_user_stru_type >= DMAC_ALG_USER_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{dmac_alg_register_user_priv_stru:: ERROR INFO: pst_user=0x%x, p_user_stru=0x%x, en_user_stru_type=%d.}",
                       pst_user, p_user_stru, en_user_stru_type);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);
    pst_alg_info = (dmac_alg_user_stru *)pst_dmac_user->p_alg_priv;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_register_user_priv_stru:: pst_alg_info is NULL!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_alg_info->p_alg_info[en_user_stru_type] = p_user_stru;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_user_priv_stru(mac_user_stru                     *pst_user,
                                                       dmac_alg_user_stru_id_enum_uint8   en_user_stru_type)
{
    dmac_user_stru         *pst_dmac_user;
    dmac_alg_user_stru     *pst_alg_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_user)
                  || (en_user_stru_type >= DMAC_ALG_USER_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_alg_unregister_user_priv_stru:: ERROR INFO: pst_user=0x%x, en_user_stru_type=%d.}",
                       pst_user, en_user_stru_type);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);
    pst_alg_info = (dmac_alg_user_stru *)pst_dmac_user->p_alg_priv;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "ERROR INFO: p_alg_priv is NULL PTR");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_alg_info->p_alg_info[en_user_stru_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_tid_priv_stru(
                mac_user_stru                      *pst_user,
                oal_uint8                           uc_tid_no,
                dmac_alg_tid_stru_id_enum_uint8     en_tid_stru_type,
                oal_void                           *p_tid_stru)
{
    dmac_alg_tid_stru      *pst_alg_info;
    dmac_user_stru         *pst_dmac_user;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_user)
                  || (OAL_PTR_NULL == p_tid_stru)
                  || (uc_tid_no >= WLAN_TID_MAX_NUM)
                  || (en_tid_stru_type >= DMAC_ALG_TID_STRU_ID_BUTT)))
    {
         OAM_ERROR_LOG4(0, OAM_SF_ANY,
                        "{dmac_alg_register_tid_priv_stru:: ERROR INFO: pst_user=0x%x, p_tid_stru=0x%x, uc_tid_no=%d, en_tid_stru_type=%d.}",
                        pst_user, p_tid_stru, uc_tid_no, en_tid_stru_type);

         return OAL_FAIL;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    pst_alg_info = (dmac_alg_tid_stru *)pst_dmac_user->ast_tx_tid_queue[uc_tid_no].p_alg_priv;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
         OAM_ERROR_LOG0(0, OAM_SF_ANY, "ERROR INFO:pst_alg_info is NULL PTR.");

         return OAL_ERR_CODE_PTR_NULL;
    }

    pst_alg_info->p_alg_info[en_tid_stru_type] = p_tid_stru;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_tid_priv_stru(
                mac_user_stru                      *pst_user,
                oal_uint8                           uc_tid_no,
                dmac_alg_tid_stru_id_enum_uint8     en_tid_stru_type)
{
    dmac_alg_tid_stru      *pst_alg_info;
    dmac_user_stru         *pst_dmac_user;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_user)
                  || (uc_tid_no >= WLAN_TID_MAX_NUM)
                  || (en_tid_stru_type >= DMAC_ALG_TID_STRU_ID_BUTT)))
    {
         OAM_ERROR_LOG3(0, OAM_SF_ANY,
                        "{dmac_alg_unregister_tid_priv_stru:: ERROR INFO: pst_user=0x%x, uc_tid_no=%d, en_tid_stru_type=%d.}",
                        pst_user, uc_tid_no, en_tid_stru_type);

         return OAL_FAIL;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    pst_alg_info = (dmac_alg_tid_stru *)pst_dmac_user->ast_tx_tid_queue[uc_tid_no].p_alg_priv;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
         OAM_ERROR_LOG2(pst_user->uc_vap_id, OAM_SF_ANY, "ERROR INFO:pst_alg_info is NULL PTR, user idx = %d, tid no = %d.", pst_user->us_assoc_id, uc_tid_no);

         return OAL_ERR_CODE_PTR_NULL;
    }

    pst_alg_info->p_alg_info[en_tid_stru_type] = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_get_chip_priv_stru(
                mac_chip_stru                          *pst_chip_dev,
                dmac_alg_chip_stru_id_enum_uint8        en_chip_stru_type,
                oal_void                              **pp_chip_stru)
{
    dmac_alg_chip_stru  *pst_chip_info;

    pst_chip_info = (dmac_alg_chip_stru *)MAC_CHIP_ALG_PRIV(pst_chip_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_chip_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_chip_priv_stru::pst_chip_info is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pp_chip_stru = pst_chip_info->p_alg_info[en_chip_stru_type];

    if (OAL_UNLIKELY(*pp_chip_stru == OAL_PTR_NULL))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_alg_get_chip_priv_stru::pp_chip_stru is null, en_chip_stru_type=%d}", en_chip_stru_type);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}



oal_uint32  dmac_alg_get_device_priv_stru(hal_to_dmac_device_stru  *pst_hal_dev,
                                      hal_alg_device_stru_id_enum_uint8  en_dev_stru_type,
                                      oal_void                            **pp_dev_stru)
{
    hal_alg_device_stru  *pst_dev_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_hal_dev)
        || (OAL_PTR_NULL == pp_dev_stru)
        || (en_dev_stru_type >= HAL_ALG_DEVICE_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{dmac_alg_get_device_priv_stru:: ERROR INFO: pst_hal_dev=0x%x, pp_dev_stru=0x%x, en_dev_stru_type=%d.}",
                       pst_hal_dev, pp_dev_stru, en_dev_stru_type);


        return OAL_FAIL;
    }


    pst_dev_info = (hal_alg_device_stru *)HAL_DEV_ALG_PRIV(pst_hal_dev);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dev_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_device_priv_stru::pst_dev_info is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pp_dev_stru = pst_dev_info->p_alg_info[en_dev_stru_type];

    if (OAL_UNLIKELY(*pp_dev_stru == OAL_PTR_NULL))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_alg_get_device_priv_stru::pp_dev_stru is null, en_dev_stru_type=%d}", en_dev_stru_type);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}


oal_uint32  dmac_alg_get_vap_priv_stru(mac_vap_stru                 *pst_vap,
                                        dmac_alg_vap_stru_id_enum_uint8    en_vap_stru_type,
                                        oal_void                     **pp_vap_stru)
{
    dmac_alg_vap_stru  *pst_alg_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_vap)
        || (OAL_PTR_NULL == pp_vap_stru)
        || (en_vap_stru_type >= DMAC_ALG_VAP_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{dmac_alg_get_vap_priv_stru:: ERROR INFO: pst_vap=0x%x, pp_vap_stru=0x%x, en_vap_stru_type=%d.}",
                       pst_vap, pp_vap_stru, en_vap_stru_type);

        return OAL_FAIL;
    }

    pst_alg_info = ((dmac_vap_stru *)pst_vap)->p_alg_priv;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pp_vap_stru = pst_alg_info->p_alg_info[en_vap_stru_type];

    if (OAL_UNLIKELY(*pp_vap_stru == OAL_PTR_NULL))
    {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_get_vap_priv_stru::alg priv pp_vap_stru is null, en_vap_stru_type=%d}", en_vap_stru_type);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}


oal_uint32  dmac_alg_get_user_priv_stru(
                OAL_CONST mac_user_stru * OAL_CONST pst_user,
                dmac_alg_user_stru_id_enum_uint8    en_user_stru_type,
                oal_void                          **pp_user_stru)
{
    dmac_alg_user_stru     *pst_alg_info;
    dmac_user_stru         *pst_dmac_user;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pp_user_stru) || (en_user_stru_type >= DMAC_ALG_USER_STRU_ID_BUTT)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY,
                       "{dmac_alg_get_user_priv_stru:: ERROR INFO: pp_user_stru=0x%x, en_user_stru_type=%d.}",
                       pp_user_stru, en_user_stru_type);
        return OAL_FAIL;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    pst_alg_info = (dmac_alg_user_stru *)pst_dmac_user->p_alg_priv;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
        if (MAC_USER_STATE_ASSOC != pst_user->en_user_asoc_state)
        {
           OAM_WARNING_LOG4(pst_user->uc_vap_id, OAM_SF_ANY,
                            "{dmac_alg_get_user_priv_stru:: us_assoc_id=%d, en_user_asoc_state=%d, user mac: %2x:%2x}",
                            pst_user->us_assoc_id,
                            pst_user->en_user_asoc_state,
                            pst_user->auc_user_mac_addr[0],
                            pst_user->auc_user_mac_addr[1]);
           OAM_WARNING_LOG4(pst_user->uc_vap_id, OAM_SF_ANY,
                            "{dmac_alg_get_user_priv_stru:: user mac:%2x:%2x:%2x:%2x}",
                            pst_user->auc_user_mac_addr[2],
                            pst_user->auc_user_mac_addr[3],
                            pst_user->auc_user_mac_addr[4],
                            pst_user->auc_user_mac_addr[5]);
        }

        return OAL_ERR_CODE_PTR_NULL;
    }

    *pp_user_stru = pst_alg_info->p_alg_info[en_user_stru_type];

    if (OAL_UNLIKELY(*pp_user_stru == OAL_PTR_NULL))
    {
        OAM_WARNING_LOG2(pst_user->uc_vap_id, OAM_SF_ANY, "{dmac_alg_get_user_priv_stru::alg priv pp_user_stru is null, en_user_stru_type=%d, user idx = %d}",
                         en_user_stru_type, pst_user->us_assoc_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}


oal_uint32  dmac_alg_get_tid_priv_stru(
                OAL_CONST mac_user_stru * OAL_CONST pst_user,
                oal_uint8                           uc_tid_no,
                dmac_alg_tid_stru_id_enum_uint8     en_tid_stru_type,
                oal_void                          **pp_tid_stru)
{
    dmac_alg_tid_stru      *pst_alg_info;
    dmac_user_stru         *pst_dmac_user;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pp_tid_stru)
                  || (uc_tid_no >= WLAN_TID_MAX_NUM)
                  || (en_tid_stru_type >= DMAC_ALG_TID_STRU_ID_BUTT)))
    {
         OAM_ERROR_LOG3(0, OAM_SF_ANY,
                        "{dmac_alg_get_tid_priv_stru:: ERROR INFO: pp_tid_stru=0x%x, uc_tid_no=%d, en_tid_stru_type=%d.}",
                        pp_tid_stru, uc_tid_no, en_tid_stru_type);

         return OAL_FAIL;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    pst_alg_info = (dmac_alg_tid_stru *)pst_dmac_user->ast_tx_tid_queue[uc_tid_no].p_alg_priv;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
        if (MAC_USER_STATE_ASSOC != pst_user->en_user_asoc_state)
        {
            OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_alg_get_tid_priv_stru::p_alg_priv is null ptr. user state = %d, user_id=%d}",
                            pst_user->en_user_asoc_state,
                            pst_user->us_assoc_id);
        }

        return OAL_ERR_CODE_PTR_NULL;
    }

    *pp_tid_stru = pst_alg_info->p_alg_info[en_tid_stru_type];

    if (OAL_UNLIKELY(*pp_tid_stru == OAL_PTR_NULL))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_alg_get_tid_priv_stru::pp_tid_stru is null, en_tid_stru_type=%d}", en_tid_stru_type);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return OAL_SUCC;
}



oal_uint32  dmac_alg_cfg_channel_notify(mac_vap_stru *pst_vap, dmac_alg_channel_bw_chg_type_uint8 en_type)
{
    dmac_alg_stru  *pst_alg_stru;
    oal_uint8       uc_index;

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_CFG_CHANNEL_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_channel_notify_func[uc_index])
        {
            pst_alg_stru->pa_cfg_channel_notify_func[uc_index](pst_vap, en_type);
        }
    }

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL

oal_uint32  dmac_alg_netbuff_usage_info_notify(mac_cfg_meminfo_stru *pst_meminfo_param)
{
    dmac_alg_stru  *pst_alg_stru;
    oal_uint8       uc_index;

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_CFG_NETBUFF_INFO_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_netbuff_usage_info_notify_func[uc_index])
        {
            pst_alg_stru->pa_cfg_netbuff_usage_info_notify_func[uc_index](pst_meminfo_param);
        }
    }

    return OAL_SUCC;
}
#endif

oal_uint32  dmac_alg_cfg_start_connect_notify(mac_vap_stru *pst_vap, oal_int8 c_rssi)
{
    dmac_alg_stru  *pst_alg_stru;
    oal_uint8       uc_index;
    oal_uint32      ul_ret;

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_CFG_START_CONNECT_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_start_connect_notify_func[uc_index])
        {
            ul_ret = pst_alg_stru->pa_cfg_start_connect_notify_func[uc_index](pst_vap, c_rssi);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG2(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_cfg_start_connect_notify::ul_ret=%d, uc_index=%d}", ul_ret, uc_index);
            }
        }
    }

    return OAL_SUCC;
}
#endif /* _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE */
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_uint32 dmac_alg_cfg_dual_antenna_state_notify(hal_to_dmac_device_stru *pst_hal_device, oal_uint32 ul_status)
{
    dmac_alg_stru  *pst_alg_stru;

    pst_alg_stru = g_pst_alg_main;
    if (OAL_PTR_NULL != pst_alg_stru->p_cfg_dual_antenna_state_notify_func)
    {
        pst_alg_stru->p_cfg_dual_antenna_state_notify_func(pst_hal_device, ul_status);
    }
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_INTF_DET

oal_uint32  dmac_alg_register_cfg_intf_det_mode_notify_func(dmac_alg_cfg_intf_det_mode_notify_enum_uint8 en_notify_sub_type,
                                                               p_alg_cfg_intf_det_mode_notify_func        p_func)

{
    ALG_ASSERT_RET(en_notify_sub_type < DMAC_ALG_CFG_INTF_DET_MODE_NOTIFY_BUTT && OAL_PTR_NULL != p_func, OAL_FAIL);
    g_pst_alg_main->pa_cfg_intf_det_mode_notify_func[en_notify_sub_type] = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_cfg_intf_det_mode_notify(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_old_intf_mode, oal_uint8 uc_cur_intf_mode)
{
    dmac_alg_stru            *pst_alg_stru;
    oal_uint8                 uc_index;

    pst_alg_stru = g_pst_alg_main;

    /* 调用相关回调函数 */
    for (uc_index = 0; uc_index < DMAC_ALG_CFG_INTF_DET_MODE_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_intf_det_mode_notify_func[uc_index])
        {
            pst_alg_stru->pa_cfg_intf_det_mode_notify_func[uc_index](pst_hal_device, uc_old_intf_mode, uc_cur_intf_mode);
        }
    }

    return OAL_SUCC;
}
#endif


oal_uint32  dmac_alg_cfg_user_bandwidth_notify(mac_vap_stru *pst_vap, mac_user_stru *pst_mac_user)
{
    dmac_alg_stru            *pst_alg_stru;
    oal_uint8                 uc_index;

    pst_alg_stru = g_pst_alg_main;

    /* 调用相关回调函数 */
    for (uc_index = 0; uc_index < DMAC_ALG_CFG_USER_BANDWIDTH_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_user_bandwidth_notify_func[uc_index])
        {
            pst_alg_stru->pa_cfg_user_bandwidth_notify_func[uc_index](pst_vap, pst_mac_user);
        }
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_uint32  dmac_alg_cfg_get_ant_info_notify(mac_vap_stru *pst_vap, oal_uint8 *puc_param,
    oal_uint32 *pul_param1, oal_uint32 *pul_param2, oal_uint32 *pul_param3, oal_uint32 *pul_param4, oal_uint32 *pul_param5)
{
    dmac_alg_stru  *pst_alg_stru;
    pst_alg_stru = g_pst_alg_main;
    if (OAL_PTR_NULL != pst_alg_stru->p_cfg_get_ant_info_notify_func)
    {
        pst_alg_stru->p_cfg_get_ant_info_notify_func(pst_vap, puc_param, pul_param1, pul_param2, pul_param3, pul_param4, pul_param5);
    }
    return OAL_SUCC;
}
oal_uint32  dmac_alg_cfg_double_ant_switch_notify(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_param)
{
    dmac_alg_stru  *pst_alg_stru;
    oal_uint32      ul_ret = OAL_FAIL;
    pst_alg_stru = g_pst_alg_main;
    if (OAL_PTR_NULL != pst_alg_stru->p_cfg_double_ant_switch_notify_func)
    {
        ul_ret = pst_alg_stru->p_cfg_double_ant_switch_notify_func(pst_hal_device, uc_param);
    }
    return ul_ret;
}
#endif


oal_uint32  dmac_alg_cfg_user_protocol_notify(mac_vap_stru *pst_vap, mac_user_stru *pst_mac_user)
{
    dmac_alg_stru            *pst_alg_stru;
    oal_uint8                 uc_index;

    pst_alg_stru = g_pst_alg_main;

    /* 调用相关回调函数 */
    for (uc_index = 0; uc_index < DMAC_ALG_CFG_USER_PROTOCOL_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_user_protocol_notify_func[uc_index])
        {
            pst_alg_stru->pa_cfg_user_protocol_notify_func[uc_index](pst_vap, pst_mac_user);
        }
    }

    return OAL_SUCC;
}


oal_uint32  dmac_alg_cfg_user_spatial_stream_notify(mac_user_stru *pst_mac_user)
{
    dmac_alg_stru            *pst_alg_stru;
    oal_uint8                 uc_index;

    pst_alg_stru = g_pst_alg_main;

    /* 调用相关回调函数 */
    for (uc_index = 0; uc_index < DMAC_ALG_CFG_USER_SPATIAL_STREAM_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_cfg_user_spatial_stream_notify_func[uc_index])
        {
            pst_alg_stru->pa_cfg_user_spatial_stream_notify_func[uc_index](pst_mac_user);
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_alg_add_assoc_user_notify(dmac_vap_stru *pst_vap, dmac_user_stru *pst_user)
{
    dmac_alg_user_stru  *pst_user_info;
    dmac_alg_tid_stru   *pst_tid_info;
    dmac_alg_stru       *pst_alg_stru;
    oal_uint8            uc_index;
    oal_uint8            uc_loop = 0;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_vap) || (OAL_PTR_NULL == pst_user)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "OAL_ERR_CODE_PTR_NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_PTR_NULL != pst_user->p_alg_priv)
    {
        dmac_alg_del_assoc_user_notify(pst_vap, pst_user);
    }

    /* 挂接用户级别的数据结构 */
    pst_user_info = (dmac_alg_user_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_alg_user_stru), OAL_TRUE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_user_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_alg_add_assoc_user_notify: alloc mem fail, dmac_alg_user_stru, pool id is OAL_MEM_POOL_ID_LOCAL");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    for (uc_index = 0; uc_index < DMAC_ALG_USER_STRU_ID_BUTT; uc_index++)
    {
        pst_user_info->p_alg_info[uc_index] = OAL_PTR_NULL;
    }

    pst_user->p_alg_priv = pst_user_info;

    /* 挂接TID级别的数据结构 */
    for (uc_index = 0; uc_index < WLAN_TID_MAX_NUM; uc_index++)
    {
        pst_tid_info = (dmac_alg_tid_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_alg_tid_stru), OAL_TRUE);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tid_info))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_alg_add_assoc_user_notify: alloc mem fail, dmac_alg_tid_stru, pool id is OAL_MEM_POOL_ID_LOCAL");
            OAL_MEM_FREE((oal_void *)pst_user_info, OAL_TRUE);
            pst_user->p_alg_priv = OAL_PTR_NULL;

            dmac_alg_free_tid_priv_stru(pst_user);

            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        for (uc_loop = 0; uc_loop < DMAC_ALG_TID_STRU_ID_BUTT; uc_loop++)
        {
            pst_tid_info->p_alg_info[uc_loop] = OAL_PTR_NULL;
        }

        pst_user->ast_tx_tid_queue[uc_index].p_alg_priv = pst_tid_info;
    }

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_ADD_USER_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_add_assoc_user_notify_func[uc_index])
        {
            pst_alg_stru->pa_add_assoc_user_notify_func[uc_index](&(pst_vap->st_vap_base_info), &(pst_user->st_user_base_info));
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_alg_del_assoc_user_notify(dmac_vap_stru *pst_vap, dmac_user_stru *pst_user)
{
    dmac_alg_user_stru     *pst_user_info;
    dmac_alg_stru          *pst_alg_stru;
    oal_uint8               uc_index;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_vap) || (OAL_PTR_NULL == pst_user)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "OAL_ERR_CODE_PTR_NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_PTR_NULL == pst_user->p_alg_priv)
    {
        return OAL_SUCC;
    }

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_DEL_USER_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_delete_assoc_user_notify_func[uc_index])
        {
            pst_alg_stru->pa_delete_assoc_user_notify_func[uc_index](&(pst_vap->st_vap_base_info), &(pst_user->st_user_base_info));
        }
    }

    /* 释放TID级别的数据结构 */
    dmac_alg_free_tid_priv_stru(pst_user);

    /* 释放用户级别的数据结构 */
    pst_user_info = pst_user->p_alg_priv;

    if (OAL_PTR_NULL != pst_user_info)
    {
        OAL_MEM_FREE(pst_user_info, OAL_TRUE);
        pst_user->p_alg_priv = OAL_PTR_NULL;
    }

    return OAL_SUCC;
}


oal_uint32  dmac_alg_rx_mgmt_notify(mac_vap_stru *pst_vap,  mac_user_stru *pst_user, oal_netbuf_stru *pst_buf)
{
    dmac_alg_stru  *pst_alg_stru;
    oal_uint8       uc_index;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_vap) || (OAL_PTR_NULL == pst_buf)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY,
                       "{dmac_alg_rx_mgmt_notify:: ERROR INFO: pst_vap=0x%x, pst_buf=0x%x.}",
                       pst_vap, pst_buf);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_RX_MGMT_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->p_rx_mgmt_func[uc_index])
        {
            pst_alg_stru->p_rx_mgmt_func[uc_index](pst_vap, pst_user, pst_buf);
        }
    }

    return OAL_SUCC;
}


oal_uint32  dmac_alg_rx_cntl_notify(mac_vap_stru *pst_vap,  mac_user_stru *pst_user, oal_netbuf_stru *pst_buf)
{
    dmac_alg_stru  *pst_alg_stru;
    oal_uint8       uc_index;


    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_vap) || (OAL_PTR_NULL == pst_user) || (OAL_PTR_NULL == pst_buf)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{dmac_alg_rx_cntl_notify:: ERROR INFO: pst_vap=0x%x, pst_user=0x%x, pst_buf=0x%x.}",
                       pst_vap, pst_user, pst_buf);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_RX_CNTL_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->p_rx_cntl_func[uc_index])
        {
            pst_alg_stru->p_rx_cntl_func[uc_index](pst_vap, pst_user, pst_buf);
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_alg_create_vap_notify(dmac_vap_stru *pst_vap)
{
    dmac_alg_vap_stru  *pst_alg_info;
    dmac_alg_stru  *pst_alg_stru;
    oal_uint8       uc_index;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_create_vap_notify::pst_vap is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_alg_info = (dmac_alg_vap_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_alg_vap_stru), OAL_TRUE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_alg_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_create_vap_notify::pst_alg_infois null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_index = 0; uc_index < DMAC_ALG_VAP_STRU_ID_BUTT; uc_index++)
    {
        pst_alg_info->p_alg_info[uc_index] = OAL_PTR_NULL;
    }


    /* 挂接VAP级别的数据结构 */
    pst_vap->p_alg_priv = pst_alg_info;

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_ADD_VAP_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_create_vap_notify_func[uc_index])
        {
            pst_alg_stru->pa_create_vap_notify_func[uc_index]((mac_vap_stru *)pst_vap);
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_alg_delete_vap_notify(dmac_vap_stru *pst_vap)
{
    dmac_alg_stru  *pst_alg_stru;
    oal_uint8       uc_index;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    for (uc_index = 0; uc_index < DMAC_ALG_DEL_VAP_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_delete_vap_notify_func[uc_index])
        {
            pst_alg_stru->pa_delete_vap_notify_func[uc_index]((mac_vap_stru *)pst_vap);
        }
    }

    /* 删除VAP级别的数据结构 */
    if (OAL_PTR_NULL != pst_vap->p_alg_priv)
    {
        OAL_MEM_FREE(pst_vap->p_alg_priv, OAL_TRUE);
        pst_vap->p_alg_priv = OAL_PTR_NULL;
    }

    return OAL_SUCC;
}



oal_uint32  dmac_alg_user_active_change_notify(dmac_user_stru *pst_dmac_user)
{
    dmac_alg_stru            *pst_alg_stru;
    oal_uint8                 uc_index;

    pst_alg_stru = g_pst_alg_main;

    /* 调用相关回调函数 */
    for (uc_index = 0; uc_index < DMAC_ALG_USER_ACTIVE_CHANGE_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_alg_user_active_change_notify_func[uc_index])
        {
            pst_alg_stru->pa_alg_user_active_change_notify_func[uc_index](pst_dmac_user);
        }
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND

oal_uint32  dmac_alg_user_replace_notify(dmac_user_stru *pst_dmac_user, dmac_alg_user_oper_type_enum_uint8 en_oper_type)
{
    dmac_alg_stru            *pst_alg_stru;
    oal_uint8                 uc_index;

    pst_alg_stru = g_pst_alg_main;

    /* 调用相关回调函数 */
    for (uc_index = 0; uc_index < DMAC_ALG_USER_REPLACE_NOTIFY_BUTT; uc_index++)
    {
        if (OAL_PTR_NULL != pst_alg_stru->pa_alg_user_replace_notify_func[uc_index])
        {
            pst_alg_stru->pa_alg_user_replace_notify_func[uc_index](pst_dmac_user, en_oper_type);
        }
    }

    return OAL_SUCC;
}
#endif

oal_uint32  dmac_alg_del_ba(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_uint8 uc_tid)
{
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "user is null ptr!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    MAC_GET_DMAC_USER(pst_mac_user)->en_delete_ba_flag = OAL_TRUE;

    return OAL_SUCC;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_alg_syn_info(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    frw_event_mem_stru                      *pst_event_mem;          /* 申请事件返回的内存指针 */
    frw_event_stru                          *pst_dmac_to_hmac_event;  /* 指向申请事件的payload指针 */
    dmac_to_hmac_syn_info_event_stru        *pst_syn_info_event;
    oal_uint32                               ul_ret;

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_to_hmac_syn_info_event_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_syn_info: alloc event failed! size=%d}", OAL_SIZEOF(dmac_to_hmac_syn_info_event_stru));

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_dmac_to_hmac_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_dmac_to_hmac_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_SDT_REG,
                       DMAC_TO_HMAC_ALG_INFO_SYN,
                       OAL_SIZEOF(dmac_to_hmac_syn_info_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_syn_info_event = (dmac_to_hmac_syn_info_event_stru *)(pst_dmac_to_hmac_event->auc_event_data);

    pst_syn_info_event->us_user_index  = pst_mac_user->us_assoc_id;
    pst_syn_info_event->uc_cur_protocol = pst_mac_user->en_cur_protocol_mode;
    pst_syn_info_event->uc_cur_bandwidth = pst_mac_user->en_cur_bandwidth;

    /* 分发 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_syn_info::frw_event_dispatch_event fail, ul_ret=%d}", ul_ret);
    }

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return ul_ret;
}


oal_uint32  dmac_alg_voice_aggr(mac_vap_stru *pst_mac_vap)
{
    frw_event_mem_stru                      *pst_event_mem;          /* 申请事件返回的内存指针 */
    frw_event_stru                          *pst_dmac_to_hmac_event;  /* 指向申请事件的payload指针 */
    dmac_to_hmac_voice_aggr_event_stru      *pst_voice_aggr_event;
    oal_uint32                               ul_ret;

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_to_hmac_voice_aggr_event_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_voice_aggr: alloc event failed! size=%d}", OAL_SIZEOF(dmac_to_hmac_voice_aggr_event_stru));

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_dmac_to_hmac_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_dmac_to_hmac_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_SDT_REG,
                       DMAC_TO_HMAC_VOICE_AGGR,
                       OAL_SIZEOF(dmac_to_hmac_voice_aggr_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    pst_voice_aggr_event = (dmac_to_hmac_voice_aggr_event_stru *)(pst_dmac_to_hmac_event->auc_event_data);
    pst_voice_aggr_event->uc_vap_id     = pst_mac_vap->uc_vap_id;
    pst_voice_aggr_event->en_voice_aggr = pst_mac_vap->bit_voice_aggr;

    /* 分发 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_voice_aggr: frw_event_dispatch_event fail, ul_ret=%d}", ul_ret);
    }

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return ul_ret;
}

#endif


oal_uint32 dmac_alg_get_user_index(mac_user_stru *pst_mac_user, oal_uint32 *pul_user_index)
{
    oal_uint32            ul_user_index  = 0;

    /* user私有算法结构体空间, 获取user index */
    ul_user_index = DMAC_ALG_GET_USER_INFO_INDEX(pst_mac_user);
    if (ul_user_index >= WLAN_ALG_ASOC_USER_NUM_LIMIT)
    {
        OAM_ERROR_LOG3(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{dmac_alg_get_user_index::user index: %d; assoc id: %d; chip_id: %d;",
                                            ul_user_index, pst_mac_user->us_assoc_id, pst_mac_user->uc_chip_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    *pul_user_index = ul_user_index;

    return OAL_SUCC;
}


wlan_bw_cap_enum_uint8 dmac_alg_get_device_bw_mode(hal_to_dmac_device_stru *pst_hal_device)
{
    mac_vap_stru                      *pst_vap       = OAL_PTR_NULL;
    wlan_channel_bandwidth_enum_uint8  en_max_vap_bw = WLAN_BAND_WIDTH_20M;
    oal_uint8                          uc_vap_index  = 0;
    oal_uint8                          uc_up_vap_num = 0;
    oal_uint8                          auc_mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};

    /* 遍历所有vap，并记录最大工作带宽 */
    uc_up_vap_num = hal_device_find_all_up_vap(pst_hal_device, auc_mac_vap_id);
    for (uc_vap_index = 0; uc_vap_index < uc_up_vap_num; uc_vap_index++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(auc_mac_vap_id[uc_vap_index]);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_alg_get_device_bw_mode::pst_vap[%d] is NULL.", auc_mac_vap_id[uc_vap_index]);
            continue;
        }

        if (pst_vap->st_channel.en_bandwidth < WLAN_BAND_WIDTH_BUTT)
        {
            en_max_vap_bw = OAL_MAX(en_max_vap_bw, pst_vap->st_channel.en_bandwidth);
        }
    }

    if (WLAN_BAND_WIDTH_20M == en_max_vap_bw)
    {
        return WLAN_BW_CAP_20M;
    }
    else if ((WLAN_BAND_WIDTH_40MINUS == en_max_vap_bw) || (WLAN_BAND_WIDTH_40PLUS == en_max_vap_bw))
    {
        return WLAN_BW_CAP_40M;
    }
    else if ((en_max_vap_bw >= WLAN_BAND_WIDTH_80PLUSPLUS) && (en_max_vap_bw <= WLAN_BAND_WIDTH_80MINUSMINUS))
    {
        return WLAN_BW_CAP_80M;
    }
#ifdef _PRE_WLAN_FEATURE_160M
    else if ((en_max_vap_bw >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) && (en_max_vap_bw <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS))
    {
        return WLAN_BW_CAP_160M;
    }
#endif
    else
    {
        return WLAN_BW_CAP_BUTT;
    }
}


oal_uint32  dmac_alg_get_qap_wme_info(
                mac_vap_stru               *pst_vap,
                wlan_wme_ac_type_enum_uint8 en_wme_type,
                mac_wme_param_stru         *pst_wme_info)
{
    if ((OAL_PTR_NULL == pst_vap)||(OAL_PTR_NULL == pst_wme_info))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "OAL_PTR_NULL == pst_vap");
        return OAL_FAIL;
    }

    /* config vap没有mib库，不能查询wme参数 */
    if (WLAN_VAP_MODE_CONFIG == pst_vap->en_vap_mode)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_qap_wme_info::config vap has no mib}\r\n");

        return OAL_FAIL;
    }

    if (en_wme_type >= WLAN_WME_AC_BUTT)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_get_qap_wme_info::en_wme_type=%d}", en_wme_type);
        return OAL_FAIL;
    }

    pst_wme_info->ul_aifsn      = mac_mib_get_QAPEDCATableAIFSN(pst_vap, en_wme_type);
    pst_wme_info->ul_logcwmax   = mac_mib_get_QAPEDCATableCWmax(pst_vap, en_wme_type);
    pst_wme_info->ul_logcwmin   = mac_mib_get_QAPEDCATableCWmin(pst_vap, en_wme_type);
    pst_wme_info->ul_txop_limit = mac_mib_get_QAPEDCATableTXOPLimit(pst_vap, en_wme_type);

    return OAL_SUCC;
}


oal_uint32  dmac_alg_set_qap_msdu_lifetime(
                                    mac_vap_stru               *pst_vap,
                                    wlan_wme_ac_type_enum_uint8 en_wme_type,
                                    oal_uint32                  ul_msdu_lifetime)
{
    oal_uint32   aul_param[3];
    oal_uint32   ul_ret = OAL_SUCC;

    if (OAL_PTR_NULL == pst_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "OAL_PTR_NULL == pst_vap");
        return OAL_FAIL;
    }

    if (en_wme_type >= WLAN_WME_AC_BUTT)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_alg_set_qap_msdu_lifetime::en_wme_type=%d}", en_wme_type);
        return OAL_FAIL;
    }

    /* 调用配置接口 */
    aul_param[1] = (oal_uint32)en_wme_type;
    aul_param[2] = ul_msdu_lifetime;

    ul_ret = dmac_config_set_qap_msdu_lifetime(pst_vap, OAL_SIZEOF(aul_param), (oal_uint8 *)aul_param);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_ANY, "dmac_config_set_msdu_lifetime failed");
        return ul_ret;
    }

    return OAL_SUCC;
}



oal_uint32  dmac_alg_register(dmac_alg_id_enum_uint32 en_alg_id)
{
    ALG_ASSERT_RET(en_alg_id < DMAC_ALG_ID_BUTT, OAL_ERR_CODE_ARRAY_OVERFLOW);
    g_pst_alg_main->ul_alg_bitmap |= en_alg_id;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister(dmac_alg_id_enum_uint32 en_alg_id)
{
    ALG_ASSERT_RET(en_alg_id < DMAC_ALG_ID_BUTT, OAL_ERR_CODE_ARRAY_OVERFLOW);
    g_pst_alg_main->ul_alg_bitmap &= (~en_alg_id);

    return OAL_SUCC;
}


oal_uint32  dmac_alg_is_registered(dmac_alg_id_enum_uint32 en_alg_id, oal_bool_enum_uint8 *pen_is_registered)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != pen_is_registered && en_alg_id < DMAC_ALG_ID_BUTT, OAL_ERR_CODE_PTR_NULL);
    if((g_pst_alg_main->ul_alg_bitmap & en_alg_id) == en_alg_id)
    {
        *pen_is_registered = OAL_TRUE;
    }
    else
    {
        *pen_is_registered = OAL_FALSE;
    }

    return OAL_SUCC;
}


oal_void dmac_chip_alg_init(mac_chip_stru *pst_chip)
{
    dmac_alg_chip_stru *p_alg_chip_stru = OAL_PTR_NULL;

    p_alg_chip_stru = (dmac_alg_chip_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_alg_chip_stru), OAL_TRUE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == p_alg_chip_stru))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_chip_alg_init: OAL_PTR_NULL == p_alg_chip_stru\n");
        return;
    }

    OAL_MEMZERO(p_alg_chip_stru, OAL_SIZEOF(dmac_alg_chip_stru));

    /* 挂接算法主结构体 */
    MAC_CHIP_ALG_PRIV(pst_chip) = p_alg_chip_stru;

    return;
}
#if 0
// never user this interface to force ppdu_cnt of device. gaolin
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

oal_uint32   dmac_alg_set_hal_queue_ppdu_num(oal_uint16 us_assoc_id, oal_uint8 uc_ac_num, oal_uint8 puc_ppdu_num)
{
    dmac_vap_stru             *pst_dmac_vap;
    hal_to_dmac_device_stru   *pst_hal_device;
    mac_user_stru             *pst_user;
    pst_user = (mac_user_stru *)mac_res_get_mac_user(us_assoc_id);
    if(OAL_PTR_NULL == pst_user)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "dmac_alg_set_hal_queue_ppdu_num: pst_user[%d] null.}", us_assoc_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap    = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_user->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_user->uc_vap_id, OAM_SF_ANY, "{dmac_alg_set_hal_queue_ppdu_num::mac_res_get_dmac_vap fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device  = pst_dmac_vap->pst_hal_device;
    pst_hal_device->ast_tx_dscr_queue[HAL_AC_TO_Q_NUM(uc_ac_num)].uc_ppdu_cnt = puc_ppdu_num;

    return OAL_SUCC;
}
#endif
#endif

hal_to_dmac_vap_stru *dmac_alg_get_hal_to_dmac_vap(oal_uint8 uc_mac_vap_id)
{
    dmac_vap_stru   *pst_dmac_vap;

    pst_dmac_vap = mac_res_get_dmac_vap(uc_mac_vap_id);
    if (NULL == pst_dmac_vap)
    {
        return OAL_PTR_NULL;
    }

    return pst_dmac_vap->pst_hal_vap;
}

/* 单用户跟踪获取参数相关接口 */
#ifdef _PRE_DEBUG_MODE_USER_TRACK

oal_uint32  dmac_alg_register_start_stat_rssi_notify(
                                        p_alg_start_stat_rssi_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_start_stat_rssi_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_start_stat_rssi_notify(oal_void)
{
    g_pst_alg_main->p_start_stat_rssi_func = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_get_rssi_notify(p_alg_get_rssi_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_get_rssi_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_get_rssi_notify(oal_void)
{
    g_pst_alg_main->p_get_rssi_func = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_stop_stat_rssi_notify(
                                        p_alg_stop_stat_rssi_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_stop_stat_rssi_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_stop_stat_rssi_notify(oal_void)
{
    g_pst_alg_main->p_stop_stat_rssi_func = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_start_stat_rate_notify(
                                        p_alg_start_stat_rate_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_start_stat_rate_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_start_stat_rate_notify(oal_void)
{
    g_pst_alg_main->p_start_stat_rate_func = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_get_rate_notify(p_alg_get_rate_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_get_rate_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_get_rate_notify(oal_void)
{
    g_pst_alg_main->p_get_rate_func = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_stop_stat_rate_notify(
                                        p_alg_start_stat_rate_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_stop_stat_rate_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_stop_stat_rate_notify(oal_void)
{
    g_pst_alg_main->p_stop_stat_rate_func = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_start_stat_best_rate_notify(
                                        p_alg_start_stat_best_rate_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_start_stat_best_rate_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_start_stat_best_rate_notify(oal_void)
{
    g_pst_alg_main->p_start_stat_best_rate_func = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_get_best_rate_notify(p_alg_get_best_rate_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_get_best_rate_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_get_best_rate_notify(oal_void)
{
    g_pst_alg_main->p_get_best_rate_func = OAL_PTR_NULL;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_stop_stat_best_rate_notify(
                                        p_alg_stop_stat_best_rate_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_stop_stat_best_rate_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_unregister_stop_stat_best_rate_notify(oal_void)
{
    g_pst_alg_main->p_stop_stat_best_rate_func = OAL_PTR_NULL;

    return OAL_SUCC;
}



oal_uint32  dmac_alg_start_stat_rssi(mac_user_stru *pst_mac_user)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "param null!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_start_stat_rssi_func))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "p_start_stat_rssi_func null!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_start_stat_rssi_func(pst_mac_user);
}


oal_uint32  dmac_alg_get_rssi(mac_user_stru *pst_mac_user,
                                  oal_int8      *pc_tx_rssi,
                                  oal_int8      *pc_rx_rssi)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user
       || OAL_PTR_NULL == pc_rx_rssi || OAL_PTR_NULL == pc_tx_rssi))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{dmac_alg_get_rssi:: ERROR INFO: pst_mac_user=0x%x, pc_tx_rssi=0x%x, pc_rx_rssi=0x%x.}",
                       pst_mac_user, pc_tx_rssi, pc_rx_rssi);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_get_rssi_func))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_rssi::p_get_rssi_func is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_get_rssi_func(pst_mac_user, pc_tx_rssi, pc_rx_rssi);
}



oal_uint32  dmac_alg_stop_stat_rssi(mac_user_stru *pst_mac_user)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_stop_stat_rssi::pst_mac_user is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_stop_stat_rssi_func))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_stop_stat_rssi::p_stop_stat_rssi_func is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_stop_stat_rssi_func(pst_mac_user);
}


oal_uint32  dmac_alg_start_stat_rate(mac_user_stru *pst_mac_user)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_start_stat_rate::pst_mac_user is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_start_stat_rate_func))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "p_start_stat_rate_func null!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_start_stat_rate_func(pst_mac_user);
}



oal_uint32  dmac_alg_get_rate(mac_user_stru *pst_mac_user,
                                  oal_uint32     *pul_tx_rate,
                                  oal_uint32     *pul_rx_rate)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user
       || OAL_PTR_NULL == pul_rx_rate || OAL_PTR_NULL == pul_tx_rate))
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY,
                       "{dmac_alg_get_rate:: ERROR INFO: pst_mac_user=0x%x, pul_tx_rate=0x%x, pul_rx_rate=0x%x.}",
                       pst_mac_user, pul_tx_rate, pul_rx_rate);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_get_rate_func))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_rate::p_get_rate_func is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_get_rate_func(pst_mac_user, pul_tx_rate, pul_rx_rate);
}


oal_uint32  dmac_alg_stop_stat_rate(mac_user_stru *pst_mac_user)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_stop_stat_rate::pst_mac_user is null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_stop_stat_rate_func))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_stop_stat_rate::p_stop_stat_rate_func is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_stop_stat_rate_func(pst_mac_user);
}


oal_uint32  dmac_alg_start_stat_best_rate(mac_user_stru *pst_mac_user)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_start_stat_best_rate::user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_start_stat_best_rate_func))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_start_stat_best_rate::p_start_stat_best_rate_func is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_start_stat_best_rate_func(pst_mac_user);
}



oal_uint32  dmac_alg_get_best_rate(
                                  mac_user_stru   *pst_mac_user,
                                  dmac_best_rate_stat_info_stru *pst_best_rate_info)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user || OAL_PTR_NULL == pst_best_rate_info))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_alg_get_best_rate::pst_mac_user = [%d], \
                       pst_best_rate_info = [%d].}", pst_mac_user, pst_best_rate_info);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_get_best_rate_func))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_best_rate::p_get_best_rate_func is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_get_best_rate_func(pst_mac_user, pst_best_rate_info);
}


oal_uint32  dmac_alg_stop_stat_best_rate(mac_user_stru *pst_mac_user)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_stop_stat_best_rate::usr is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_stop_stat_best_rate_func))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_stop_stat_best_rate::p_stop_stat_best_rate_func is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_stop_stat_best_rate_func(pst_mac_user);
}


#endif


oal_uint32  dmac_alg_register_get_user_rate_idx_for_tx_power_notify(p_alg_get_user_rate_idx_for_tx_power_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_get_user_rate_idx_for_tx_power_func = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_get_user_rate_idx_for_tx_power(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 *puc_ar_enable, oal_uint8 *puc_rate_to_pow_code_idx)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_user_rate_idx_for_tx_power:: pst_mac_user is NULL.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_get_user_rate_idx_for_tx_power_func))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_user_rate_idx_for_tx_power::p_get_user_rate_idx_for_tx_power_func is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_get_user_rate_idx_for_tx_power_func(pst_mac_user, puc_ar_enable, puc_rate_to_pow_code_idx);
}


oal_uint32  dmac_alg_register_get_vap_rate_idx_for_tx_power_notify(p_alg_get_vap_rate_idx_for_tx_power_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_get_vap_rate_idx_for_tx_power_func = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_get_rate_by_table_notify(p_alg_get_rate_by_table_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_get_rate_by_table_func = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_get_rate_by_table(mac_user_stru *pst_mac_user,
                                  dmac_rx_ctl_stru  *pst_cb_ctrl,
                                  oal_uint32 *pul_rate_kbps)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user || OAL_PTR_NULL == pul_rate_kbps))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY,
                       "{dmac_alg_get_rate_by_table:: ERROR INFO: pst_mac_user=0x%x, pul_rate_kbps=0x%x.}",
                       pst_mac_user, pul_rate_kbps);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_get_rate_by_table_func))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_get_rate_by_table::p_get_rate_by_table_func is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_get_rate_by_table_func(pst_mac_user, pst_cb_ctrl, pul_rate_kbps);
}


oal_uint32  dmac_alg_register_get_tx_best_rate_notify(p_alg_get_tx_best_rate_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_get_tx_best_rate_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_delete_ba_fail_notify(p_alg_delete_ba_fail_notify_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_delete_ba_fail_notify_func = p_func;

    return OAL_SUCC;
}


oal_uint32  dmac_alg_delete_ba_fail_notify(mac_user_stru *pst_mac_user)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
                       "{dmac_alg_delete_ba_fail_notify:: ERROR INFO: pst_mac_user=0x%x.}",
                       pst_mac_user);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_delete_ba_fail_notify_func))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_alg_delete_ba_fail_notify::p_delete_ba_fail_notify_func is null}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_delete_ba_fail_notify_func(pst_mac_user);
}

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP

oal_uint32 dmac_edca_opt_stat_event_process(frw_event_mem_stru *pst_event_mem)
{

    frw_event_stru          *pst_event;
    frw_event_hdr_stru      *pst_event_hdr;
    dmac_alg_stru           *pst_alg_stru;
    oal_uint8                uc_vap_id;
    oal_uint32               ul_ret;
    oal_uint8                aast_uc_traffic_num[WLAN_WME_AC_BUTT][ALG_TXRX_DATA_BUTT];

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANTI_INTF, "{dmac_join_set_reg_event_process::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event       = frw_get_event_stru(pst_event_mem);
    pst_event_hdr   = &(pst_event->st_event_hdr);
    uc_vap_id       = pst_event_hdr->uc_vap_id;

    /*lint -save -e420 */
    oal_memcopy(aast_uc_traffic_num, pst_event->auc_event_data, OAL_SIZEOF(aast_uc_traffic_num));
    /*lint -restore */

    /* 调用相关回调函数 */
    pst_alg_stru = g_pst_alg_main;

    /* EDCA 算法钩子函数，内部识别多用户多优先级和32用户单优先级业务 */
    if (OAL_PTR_NULL != pst_alg_stru->p_edca_stat_event_notify_func)
    {
        ul_ret = pst_alg_stru->p_edca_stat_event_notify_func(uc_vap_id, aast_uc_traffic_num);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANTI_INTF, "{dmac_edca_opt_stat_event_process::ul_ret=%d", ul_ret);
            return ul_ret;
        }
    }

#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC
    /*调度更新参数算法钩子函数*/
    if (OAL_PTR_NULL != pst_alg_stru->p_tx_schedule_stat_event_notify_func)
    {
        ul_ret = pst_alg_stru->p_tx_schedule_stat_event_notify_func(uc_vap_id, aast_uc_traffic_num);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANTI_INTF, "{dmac_edca_opt_stat_event_process::ul_ret=%d", ul_ret);
            return ul_ret;
        }
    }
#endif

    return OAL_SUCC;
}


oal_uint32  dmac_alg_register_edca_stat_event_notify(p_alg_edca_stat_event_notify_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_edca_stat_event_notify_func = p_func;

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_DBAC

oal_uint32  dmac_alg_register_dbac_pause_notify(
                p_alg_dbac_pause_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_dbac_pause_func = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_dbac_resume_notify(
                p_alg_dbac_resume_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_dbac_resume_func = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_register_dbac_is_pause(
                p_alg_dbac_is_pause   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_dbac_is_pause_func = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_dbac_pause(hal_to_dmac_device_stru *pst_hal_device)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_dbac_pause_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_dbac_pause_func(pst_hal_device);
}


oal_uint32  dmac_alg_dbac_resume(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_need_resume_channel)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_dbac_resume_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_dbac_resume_func(pst_hal_device, en_need_resume_channel);
}
#endif
#ifdef _PRE_WLAN_FEATURE_DBDC

oal_uint32  dmac_alg_register_dbdc_shift_alg(
                p_alg_dbdc_alg_shift_func   p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_dbdc_alg_shift_func = p_func;

    return OAL_SUCC;
}


oal_bool_enum_uint8  dmac_alg_dbdc_shift_alg(mac_vap_stru *pst_shift_vap, dmac_alg_dbdc_switch_enum_uint8 en_dbdc_state)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_dbdc_alg_shift_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_dbdc_alg_shift_func(pst_shift_vap, en_dbdc_state);
}
#endif
#ifdef _PRE_WLAN_FEATURE_DBAC

oal_bool_enum_uint8  dmac_alg_dbac_is_pause(hal_to_dmac_device_stru *pst_hal_device)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_dbac_is_pause_func))
    {
        return OAL_FALSE;
    }

    return g_pst_alg_main->p_dbac_is_pause_func(pst_hal_device);
}
#endif

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF

oal_uint32  dmac_alg_register_anti_intf_switch_notify(
                p_alg_anti_intf_switch_func p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_anti_intf_switch_func = p_func;

    return OAL_SUCC;
}

oal_uint32  dmac_alg_anti_intf_switch(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_alg_enable)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == g_pst_alg_main->p_anti_intf_switch_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return g_pst_alg_main->p_anti_intf_switch_func(pst_hal_device, uc_alg_enable);
}
#endif


oal_uint32  dmac_alg_register_get_rate_kbps_func(p_alg_get_rate_kbps_func  p_func)
{
    ALG_ASSERT_RET(OAL_PTR_NULL != p_func, OAL_ERR_CODE_PTR_NULL);
    g_pst_alg_main->p_get_rate_kbps_func = p_func;

    return OAL_SUCC;
}
/*lint -e19*/
oal_module_symbol(dmac_alg_register_add_user_notify_func);
oal_module_symbol(dmac_alg_register_device_priv_stru);
oal_module_symbol(dmac_alg_get_device_priv_stru);
oal_module_symbol(dmac_alg_register_tx_schedule_func);
#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC
oal_module_symbol(dmac_alg_register_tx_schedule_stat_event_notify_func);
oal_module_symbol(dmac_alg_unregister_tx_schedule_stat_event_notify_func);
#endif

oal_module_symbol(dmac_alg_unregister_device_priv_stru);
oal_module_symbol(dmac_alg_create_vap_notify);
oal_module_symbol(dmac_alg_add_assoc_user_notify);
oal_module_symbol(dmac_alg_register_del_user_notify_func);
oal_module_symbol(dmac_alg_unregister_user_priv_stru);
oal_module_symbol(dmac_alg_unregister);
oal_module_symbol(dmac_alg_register_tx_complete_notify_func);
oal_module_symbol(dmac_alg_register_user_priv_stru);
oal_module_symbol(dmac_alg_register);
oal_module_symbol(dmac_alg_stop_timer);
oal_module_symbol(dmac_alg_register_timer);
oal_module_symbol(dmac_alg_register_tx_notify_func);
oal_module_symbol(dmac_alg_register_tx_mgmt_notify_func);
oal_module_symbol(dmac_alg_start_timer);
oal_module_symbol(dmac_alg_is_registered);
oal_module_symbol(dmac_alg_unregister_timer);
oal_module_symbol(dmac_alg_register_tid_update_notify_func);
oal_module_symbol(dmac_alg_get_tid_priv_stru);
oal_module_symbol(dmac_alg_register_tid_priv_stru);
oal_module_symbol(dmac_alg_unregister_vap_priv_stru);
oal_module_symbol(dmac_alg_register_vap_priv_stru);
oal_module_symbol(dmac_alg_register_cfg_channel_notify_func);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
oal_module_symbol(dmac_alg_register_nerbuff_usage_notify_func);
oal_module_symbol(dmac_alg_unregister_nerbuff_usage_notify_func);
oal_module_symbol(dmac_alg_register_cfg_start_connect_notify_func);
#endif /* _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE */
oal_module_symbol(dmac_alg_register_del_vap_notify_func);
oal_module_symbol(dmac_alg_unregister_tid_priv_stru);
oal_module_symbol(dmac_alg_get_vap_priv_stru);
oal_module_symbol(dmac_alg_register_add_vap_notify_func);
oal_module_symbol(dmac_alg_get_user_priv_stru);
oal_module_symbol(dmac_alg_register_rx_notify_func);
oal_module_symbol(dmac_alg_del_ba);
#if 0
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
oal_module_symbol(dmac_alg_set_hal_queue_ppdu_num);
#endif
#endif

oal_module_symbol(dmac_alg_get_qap_wme_info);
oal_module_symbol(dmac_alg_set_qap_msdu_lifetime);
oal_module_symbol(dmac_alg_get_hal_to_dmac_vap);
oal_module_symbol(dmac_alg_register_config_notify_func);
oal_module_symbol(dmac_alg_unregister_config_notify_func);
oal_module_symbol(dmac_alg_register_downlink_flowctl_notify_func);
oal_module_symbol(dmac_alg_register_mu_flowctl_netbuff_free_notify_func);

oal_module_symbol(dmac_alg_register_cfg_user_bandwidth_notify_func);
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_module_symbol(dmac_alg_register_cfg_get_ant_info_notify_func);
oal_module_symbol(dmac_alg_register_cfg_double_ant_switch_notify_func);
#endif
oal_module_symbol(dmac_alg_register_cfg_user_protocol_notify_func);
oal_module_symbol(dmac_alg_cfg_user_protocol_notify);
oal_module_symbol(dmac_alg_register_rx_mgmt_notify_func);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
oal_module_symbol(dmac_alg_syn_info);
oal_module_symbol(dmac_alg_voice_aggr);
#endif
oal_module_symbol(dmac_alg_get_user_index);
oal_module_symbol(dmac_alg_get_device_bw_mode);


#ifdef _PRE_WLAN_FEATURE_DBAC
oal_module_symbol(dmac_alg_register_vap_down_notify_func);
oal_module_symbol(dmac_alg_register_update_dbac_fcs_config_func);
oal_module_symbol(dmac_alg_dbac_status_notify);
oal_module_symbol(dmac_alg_register_dbac_notify_func);
#endif

oal_module_symbol(dmac_alg_register_vap_up_notify_func);

#ifdef _PRE_WLAN_CHIP_TEST_ALG
oal_module_symbol(dmac_alg_register_rx_event_notify);
oal_module_symbol(dmac_alg_unregister_rx_event_notify);
#endif

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
oal_module_symbol(dmac_alg_register_flowctl_backp_notify_func);
oal_module_symbol(dmac_alg_unregister_flowctl_backp_notify_func);
oal_module_symbol(dmac_alg_flowctl_backp);
#endif

#ifdef _PRE_DEBUG_MODE_USER_TRACK
oal_module_symbol(dmac_alg_register_start_stat_rssi_notify);
oal_module_symbol(dmac_alg_unregister_start_stat_rssi_notify);
oal_module_symbol(dmac_alg_register_start_stat_rate_notify);
oal_module_symbol(dmac_alg_unregister_start_stat_rate_notify);
oal_module_symbol(dmac_alg_register_start_stat_best_rate_notify);
oal_module_symbol(dmac_alg_unregister_start_stat_best_rate_notify);
oal_module_symbol(dmac_alg_register_get_rssi_notify);
oal_module_symbol(dmac_alg_unregister_get_rssi_notify);
oal_module_symbol(dmac_alg_register_get_best_rate_notify);
oal_module_symbol(dmac_alg_unregister_get_best_rate_notify);
oal_module_symbol(dmac_alg_register_get_rate_notify);
oal_module_symbol(dmac_alg_unregister_get_rate_notify);
oal_module_symbol(dmac_alg_register_stop_stat_rssi_notify);
oal_module_symbol(dmac_alg_unregister_stop_stat_rssi_notify);
oal_module_symbol(dmac_alg_register_stop_stat_best_rate_notify);
oal_module_symbol(dmac_alg_unregister_stop_stat_best_rate_notify);
oal_module_symbol(dmac_alg_register_stop_stat_rate_notify);
oal_module_symbol(dmac_alg_unregister_stop_stat_rate_notify);

#endif

oal_module_symbol(dmac_alg_register_get_user_rate_idx_for_tx_power_notify);
oal_module_symbol(dmac_alg_register_get_vap_rate_idx_for_tx_power_notify);
oal_module_symbol(dmac_alg_register_get_rate_by_table_notify);

oal_module_symbol(dmac_alg_register_cfg_user_spatial_stream_notify_func);
#ifdef _PRE_WLAN_FEATURE_DBAC
oal_module_symbol(dmac_alg_register_dbac_pause_notify);
oal_module_symbol(dmac_alg_register_dbac_resume_notify);
oal_module_symbol(dmac_alg_register_dbac_is_pause);
#endif
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
oal_module_symbol(dmac_alg_register_edca_stat_event_notify);
#endif

oal_module_symbol(dmac_alg_cfg_user_bandwidth_notify);
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_module_symbol(dmac_alg_cfg_get_ant_info_notify);
oal_module_symbol(dmac_alg_cfg_double_ant_switch_notify);
#endif
oal_module_symbol(dmac_alg_register_get_tx_best_rate_notify);

#ifdef _PRE_WLAN_FEATURE_INTF_DET
oal_module_symbol(dmac_alg_register_cfg_intf_det_mode_notify_func);
oal_module_symbol(dmac_alg_cfg_intf_det_mode_notify);
#endif

oal_module_symbol(dmac_alg_register_delete_ba_fail_notify);

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
oal_module_symbol(dmac_alg_register_anti_intf_switch_notify);
#endif

oal_module_symbol(dmac_alg_unregister_user_active_change_notify_func);
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
oal_module_symbol(dmac_alg_register_user_replace_notify_func);
oal_module_symbol(dmac_alg_unregister_user_replace_notify_func);
#endif

oal_module_symbol(dmac_alg_register_get_rate_kbps_func);

/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif



