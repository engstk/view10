


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "dmac_user.h"
#include "dmac_11i.h"
#include "dmac_wep.h"
#include "dmac_alg.h"
#include "dmac_psm_ap.h"
#include "dmac_uapsd.h"
#include "dmac_tx_complete.h"
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

#include "dmac_11w.h"
#endif
#include "dmac_beacon.h"
#include "dmac_psm_sta.h"
#include "dmac_device.h"
#include "dmac_resource.h"
#include "dmac_config.h"
#include "dmac_scan.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_blockack.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#include "hisi_customize_wifi.h"
//#include "hal_witp_rf.h"
#else
#include "hal_rf.h"
#endif
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_WLAN_FEATURE_TXOPPS
#include "dmac_txopps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_AP_PM
#include "dmac_ap_pm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#include "dmac_user_extend.h"
#endif

#ifdef _PRE_WLAN_11K_STAT
#include "dmac_stat.h"
#endif
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#include "dmac_bsd.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
#include "dmac_tx_qos_enhance.h"
#endif

#include "dmac_power.h"
#include "dmac_csa_sta.h"
#include "dmac_rx_data.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_USER_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
dmac_user_stru g_ast_dmac_user[MAC_RES_MAX_USER_LIMIT];
mac_res_user_idx_size_stru g_st_dmac_user_idx_size[MAC_RES_MAX_USER_LIMIT];
mac_res_user_cnt_size_stru g_st_dmac_user_cnt_size[MAC_RES_MAX_USER_LIMIT];
#endif

extern oal_void dmac_chan_select_real_channel(mac_device_stru  *pst_mac_device, mac_channel_stru *pst_channel, oal_uint8 uc_dst_chan_num);


/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_DEBUG_MODE_USER_TRACK

OAL_STATIC oal_uint32  dmac_user_track_init(dmac_user_stru  *pst_dmac_user)
{
    pst_dmac_user->st_txrx_protocol.en_rx_flg = OAL_TRUE;
    pst_dmac_user->st_txrx_protocol.en_tx_flg = OAL_TRUE;

    OAL_MEMZERO(&pst_dmac_user->st_user_track_ctx, OAL_SIZEOF(mac_user_track_ctx_stru));

    return OAL_SUCC;
}


oal_uint32  dmac_user_check_txrx_protocol_change(
                                  dmac_user_stru *pst_dmac_user,
                                  oal_uint8      uc_present_mode,
                                  oam_user_info_change_type_enum_uint8  en_type)
{
    mac_user_stru *pst_mac_user = (mac_user_stru *)pst_dmac_user;
    if (OAL_PTR_NULL == pst_mac_user)
    {
        MAC_ERR_LOG(0, "pst_mac_user is null!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    switch (en_type)
    {
        case OAM_USER_INFO_CHANGE_TYPE_TX_PROTOCOL:
            /* 如果是第一次记录，则直接赋值，不用比较上报 */
            if (OAL_TRUE == pst_dmac_user->st_txrx_protocol.en_tx_flg)
            {
                pst_dmac_user->st_txrx_protocol.uc_tx_protocol = uc_present_mode;
                pst_dmac_user->st_txrx_protocol.en_tx_flg = OAL_FALSE;
            }
            else if (uc_present_mode == pst_dmac_user->st_txrx_protocol.uc_tx_protocol)
            {
                ;
            }
            else
            {
                mac_user_change_info_event(pst_mac_user->auc_user_mac_addr,
                                           pst_mac_user->uc_vap_id,
                                           (oal_uint32)pst_dmac_user->st_txrx_protocol.uc_tx_protocol,
                                           (oal_uint32)uc_present_mode,
                                           OAM_MODULE_ID_DMAC,
                                           en_type);
                pst_dmac_user->st_txrx_protocol.uc_tx_protocol = uc_present_mode;
            }
        break;

        case OAM_USER_INFO_CHANGE_TYPE_RX_PROTOCOL:
            if (OAL_TRUE == pst_dmac_user->st_txrx_protocol.en_rx_flg)
            {
                pst_dmac_user->st_txrx_protocol.uc_rx_protocol = uc_present_mode;
                pst_dmac_user->st_txrx_protocol.en_rx_flg = OAL_FALSE;
            }
            else if (uc_present_mode == pst_dmac_user->st_txrx_protocol.uc_rx_protocol)
            {
                ;
            }
            else
            {
                mac_user_change_info_event(pst_mac_user->auc_user_mac_addr,
                                           pst_mac_user->uc_vap_id,
                                           (oal_uint32)pst_dmac_user->st_txrx_protocol.uc_rx_protocol,
                                           (oal_uint32)uc_present_mode,
                                           OAM_MODULE_ID_DMAC,
                                           en_type);
                pst_dmac_user->st_txrx_protocol.uc_rx_protocol = uc_present_mode;
            }
        break;

        default:
        break;
    }

    return OAL_SUCC;
}


#endif

oal_uint32  dmac_user_resume(dmac_user_stru *pst_dmac_user)
{
    oal_uint8                             uc_tid_idx;
    dmac_tid_stru                        *pst_tid;
    oal_uint32                            ul_ret;
    hal_to_dmac_device_stru              *pst_hal_device = OAL_PTR_NULL;

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_user_resume::pst_dmac_user null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = dmac_user_get_hal_device(&pst_dmac_user->st_user_base_info);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_COEX, "{dmac_user_resume::pst_hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (MAC_USER_STATE_BUTT == pst_dmac_user->st_user_base_info.en_user_asoc_state)
    {
        OAM_WARNING_LOG0(0, OAM_SF_COEX, "{dmac_user_resume::user in BUTT_mode, cannot resume tid!}");
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_DFT_EVENT
    dmac_user_status_change_to_sdt(pst_dmac_user, OAL_FALSE);
#endif

    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx++)
    {
        pst_tid = &(pst_dmac_user->ast_tx_tid_queue[uc_tid_idx]);

        ul_ret = dmac_tid_resume(pst_hal_device, pst_tid, DMAC_TID_PAUSE_RESUME_TYPE_PS);
        if (OAL_SUCC != ul_ret)
        {
            return ul_ret;
        }
    }

    return OAL_SUCC;

}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW

oal_void dmac_chip_pause_all_user(mac_vap_stru *pst_mac_vap)
{
    oal_uint8                           uc_vap_idx;
    oal_uint8                           uc_device;
    oal_uint8                           uc_device_max;
    mac_user_stru                      *pst_mac_user;
    dmac_user_stru                     *pst_dmac_user;
    mac_chip_stru                      *pst_mac_chip;
    mac_device_stru                    *pst_mac_device;
    oal_dlist_head_stru                *pst_entry;
    oal_dlist_head_stru                *pst_next_entry;

    pst_mac_chip = mac_res_get_mac_chip(pst_mac_vap->uc_chip_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_chip))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_chip_pause_all_user:: pst_mac_chip null!}");
        return;
    }

    uc_device_max = oal_chip_get_device_num(pst_mac_chip->ul_chip_ver);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        pst_mac_device = mac_res_get_dev(pst_mac_chip->auc_device_id[uc_device]);
        if (OAL_PTR_NULL == pst_mac_device)
        {
            continue;
        }

        for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
        {
            pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
            if (OAL_PTR_NULL == pst_mac_vap)
            {
                continue;
            }

            OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_next_entry, &(pst_mac_vap->st_mac_user_list_head))
            {
                pst_mac_user      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

                pst_dmac_user      = mac_res_get_dmac_user(pst_mac_user->us_assoc_id);
                if (OAL_PTR_NULL == pst_dmac_user)
                {
                    continue;
                }

                dmac_user_pause(pst_dmac_user);
            }
        }
    }
}



oal_void dmac_chip_resume_all_user(mac_vap_stru *pst_mac_vap)
{
    oal_uint8                           uc_vap_idx;
    oal_uint8                           uc_device;
    oal_uint8                           uc_device_max;
    mac_user_stru                      *pst_mac_user;
    dmac_user_stru                     *pst_dmac_user;
    mac_chip_stru                      *pst_mac_chip;
    mac_device_stru                    *pst_mac_device;
    oal_dlist_head_stru                *pst_entry;
    oal_dlist_head_stru                *pst_next_entry;

    pst_mac_chip = mac_res_get_mac_chip(pst_mac_vap->uc_chip_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_chip))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_chip_resume_all_user:: pst_mac_chip null!}");
        return;
    }

    uc_device_max = oal_chip_get_device_num(pst_mac_chip->ul_chip_ver);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        pst_mac_device = mac_res_get_dev(pst_mac_chip->auc_device_id[uc_device]);
        if (OAL_PTR_NULL == pst_mac_device)
        {
            continue;
        }

        for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
        {
            pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
            if (OAL_PTR_NULL == pst_mac_vap)
            {
                continue;
            }

            OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_next_entry, &(pst_mac_vap->st_mac_user_list_head))
            {
                pst_mac_user      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

                pst_dmac_user      = mac_res_get_dmac_user(pst_mac_user->us_assoc_id);
                if (OAL_PTR_NULL == pst_dmac_user)
                {
                    continue;
                }

                dmac_user_resume(pst_dmac_user);
            }
        }
    }
}
#endif


OAL_STATIC oal_uint32  dmac_user_init(dmac_user_stru *pst_dmac_user)
{
    mac_vap_stru            *pst_mac_vap;

    /* 清空dmac user结构体 */
    OAL_MEMZERO(((oal_uint8 *)pst_dmac_user) + OAL_SIZEOF(mac_user_stru), OAL_SIZEOF(dmac_user_stru) - OAL_SIZEOF(mac_user_stru));

    /* 设置dmac user的节能模式 */
    pst_dmac_user->bit_ps_mode     = OAL_FALSE;

    /* RSSI统计量初始化 */
    pst_dmac_user->s_rx_rssi        = OAL_RSSI_INIT_MARKER;
    pst_dmac_user->uc_max_key_index = 0;

    /* 初始化时间戳 */
    pst_dmac_user->ul_last_active_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* 初始化seq num缓存 */
    OAL_MEMZERO(pst_dmac_user->aus_txseqs, WLAN_TID_MAX_NUM * OAL_SIZEOF(pst_dmac_user->aus_txseqs[0]));
    OAL_MEMZERO(pst_dmac_user->aus_txseqs_frag, WLAN_TID_MAX_NUM * OAL_SIZEOF(pst_dmac_user->aus_txseqs_frag[0]));

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    /* 初始化非QOS帧的seq_num 12位全为1 */
    pst_dmac_user->us_non_qos_seq_frag_num = 65535;
#endif
    /* DMAC USER TID 初始化 */
    dmac_tid_tx_queue_init(pst_dmac_user->ast_tx_tid_queue, &(pst_dmac_user->st_user_base_info));

    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_PTR_NULL  == pst_mac_vap)
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_TX, "{dmac_user_init::pst_mac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置mac_user_stru中的gruopid和partial aid两个成员变量,beaforming和txop ps会用到 */
    dmac_user_set_groupid_partial_aid(pst_mac_vap,pst_dmac_user);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    /* 清除usr统计信息 */
    oam_stats_clear_user_stat_info(pst_dmac_user->st_user_base_info.us_assoc_id);
#endif
#ifdef _PRE_DEBUG_MODE_USER_TRACK
    /* 初始化维测用的信息 */
    dmac_user_track_init(pst_dmac_user);
#endif
    /* 初始化用户状态为新用户 */
    pst_dmac_user->bit_new_add_user = OAL_TRUE;

    /* 初始化默认不强制关闭RTS */
    pst_dmac_user->bit_forbid_rts = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_HILINK
    pst_dmac_user->ul_tx_minrate  = 0;
    pst_dmac_user->ul_tx_maxrate  = 0;
#endif

#ifdef _PRE_WLAN_11K_STAT
    dmac_stat_init_user(pst_dmac_user);
#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    pst_dmac_user->ul_sta_sleep_times = 0;
#endif

    pst_dmac_user->bit_ptk_need_install      = OAL_FALSE;
    pst_dmac_user->bit_is_rx_eapol_key_open  = OAL_TRUE;
    pst_dmac_user->bit_eapol_key_4_4_tx_succ = OAL_FALSE;

    /* 初始化user tx power */
    dmac_pow_init_user_info(pst_mac_vap,pst_dmac_user);

    return OAL_SUCC;
}


oal_uint32  dmac_user_add_multi_user(mac_vap_stru *pst_mac_vap, oal_uint16 us_multi_user_idx)
{
    dmac_user_stru  *pst_dmac_multi_user;
    oal_uint16       us_user_idx;
    dmac_vap_stru   *pst_dmac_vap;
    oal_uint32       ul_ret;

    us_user_idx = us_multi_user_idx;

    /* 申请dmac user */
    ul_ret = dmac_user_alloc(us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_user_add_multi_user::mac_res_alloc_dmac_user failed[%d], userindex[%d].", ul_ret, us_user_idx);
        return ul_ret;
    }

    pst_dmac_multi_user = mac_res_get_dmac_user(us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_multi_user)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_user_init(&pst_dmac_multi_user->st_user_base_info, us_user_idx, OAL_PTR_NULL, pst_mac_vap->uc_chip_id,  pst_mac_vap->uc_device_id, pst_mac_vap->uc_vap_id);
#endif

    dmac_user_init(pst_dmac_multi_user);

    /* 组播用户都是活跃的 */
    pst_dmac_multi_user->bit_active_user = OAL_TRUE;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);

    dmac_alg_add_assoc_user_notify(pst_dmac_vap, pst_dmac_multi_user);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode || IS_P2P_CL(pst_mac_vap))
#else
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
#endif
    {
        ul_ret = dmac_psm_user_ps_structure_init(pst_dmac_multi_user);
        if (OAL_SUCC != ul_ret)
        {
            return ul_ret;
        }
    }
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_user_add_multi_user, user index[%d].}", us_user_idx);

    return OAL_SUCC;
}


oal_uint32  dmac_config_del_multi_user(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                *pst_mac_device;
    dmac_user_stru                 *pst_dmac_user;
    dmac_vap_stru                  *pst_dmac_vap;
    oal_uint16                      us_user_idx = *(oal_uint16*)puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_del_user::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_del_user::pst_dmac_user[%d] null.}", us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* dmac user相关操作去注册 */
    dmac_alg_del_assoc_user_notify(pst_dmac_vap, pst_dmac_user);

    /* 删除tid队列中的所有信息 */
    dmac_tid_clear(&(pst_dmac_user->st_user_base_info), pst_mac_device);
    dmac_tid_tx_queue_exit(pst_dmac_user);

    /* 删除用户节能结构 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode || IS_P2P_CL(&pst_dmac_vap->st_vap_base_info))
#else
    if (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode)
#endif
    {
        dmac_psm_user_ps_structure_destroy(pst_dmac_user);
    }

    dmac_user_free(us_user_idx);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP

oal_bool_enum_uint8 dmac_user_check_rsp_soft_ctl(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    mac_device_stru                 *pst_mac_device;
    mac_vap_stru                    *pst_tmp_vap;
    oal_uint8                        uc_vap_idx;

    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == pst_mac_user))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_user_check_rsp_soft_ctl::vap or user is null.}");
        return OAL_FALSE;
    }

    if (WLAN_BAND_5G != pst_mac_vap->st_channel.en_band)
    {
        return OAL_FALSE;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == pst_mac_user))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_user_check_rsp_soft_ctl::pst_mac_device is null, dev id[%d].}", pst_mac_vap->uc_device_id);
        return OAL_FALSE;
    }

    /* 仅支持单wlan场景 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_tmp_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_tmp_vap)
        {
            continue;
        }

        if ((WLAN_P2P_GO_MODE == pst_tmp_vap->en_p2p_mode) || (WLAN_P2P_CL_MODE == pst_tmp_vap->en_p2p_mode))
        {
            return OAL_FALSE;
        }
    }

    if ((WLAN_VHT_MODE == pst_mac_user->en_cur_protocol_mode) || (WLAN_VHT_ONLY_MODE == pst_mac_user->en_cur_protocol_mode))
    {
        if (pst_mac_user->en_cur_bandwidth > WLAN_BW_CAP_20M)
        {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


oal_uint32 dmac_user_update_sw_ctrl_rsp(mac_vap_stru *pst_mac_vap, mac_user_stru  *pst_mac_user)
{
    dmac_device_stru                *pst_dmac_dev = OAL_PTR_NULL;
    oal_bool_enum_uint8              en_check_rslt;

    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == pst_mac_user)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_user_update_sw_ctrl_rsp_state::input null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_dev = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_dev)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_user_update_sw_ctrl_rsp_state::pst_dmac_dev null.dev id [%d]}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_check_rslt = dmac_user_check_rsp_soft_ctl(pst_mac_vap, pst_mac_user);
    if (OAL_TRUE == en_check_rslt)
    {
        switch(pst_mac_user->en_cur_bandwidth)
        {
            case WLAN_BW_CAP_20M:
                pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_20M;
                break;
            case WLAN_BW_CAP_40M:
                pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_40M_DUP;
                break;
            case WLAN_BW_CAP_80M:
                pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_80M_DUP;
                break;
            case WLAN_BW_CAP_160M:
                pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_160M_DUP;
                break;
            default:
                pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_20M;
                break;
        }
        hal_cfg_rsp_dyn_bw(OAL_TRUE, pst_dmac_dev->en_usr_bw_mode);
        /* 默认使用6M响应帧速率，接收到数据后再行调整 */
        hal_set_rsp_rate(WLAN_PHY_RATE_6M);
        pst_dmac_dev->en_state_in_sw_ctrl_mode = OAL_TRUE;
    }
    else
    {
        hal_cfg_rsp_dyn_bw(OAL_FALSE, WLAN_BAND_ASSEMBLE_20M);
        pst_dmac_dev->en_state_in_sw_ctrl_mode = OAL_FALSE;
    }
    return OAL_SUCC;
}
#endif


oal_uint32  dmac_user_inactive(dmac_user_stru *pst_dmac_user)
{
    mac_vap_stru       *pst_mac_vap;
    mac_chip_stru      *pst_mac_chip;
    oal_uint32          ul_ret;

    /* 已经是非活跃用户，直接返回 */
    if (OAL_FALSE == pst_dmac_user->bit_active_user)
    {
        return OAL_SUCC;
    }

    pst_mac_vap = mac_res_get_mac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG4(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "dmac_user_inactive::pst_mac_vap ptr null, uid=%d, user_mac_addr[%02x-xx-xx-xx-%02x-%02x].",
            pst_dmac_user->st_user_base_info.us_assoc_id,
            pst_dmac_user->st_user_base_info.auc_user_mac_addr[0],
            pst_dmac_user->st_user_base_info.auc_user_mac_addr[4],
            pst_dmac_user->st_user_base_info.auc_user_mac_addr[5]);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "dmac_user_inactive user_idx=%d, return lut index=%d.",
        pst_dmac_user->st_user_base_info.us_assoc_id, pst_dmac_user->uc_lut_index);

    /* delete key & peer addr */
    ul_ret = dmac_11i_remove_key_from_user(pst_mac_vap, pst_dmac_user);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "dmac_user_inactive::delete machw key failed(rslt=%u).",ul_ret);
        return ul_ret;
    }
    ul_ret = dmac_11i_del_peer_macaddr(pst_mac_vap, pst_dmac_user->uc_lut_index);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "dmac_user_inactive::delete machw peer addr failed(rslt=%u).",ul_ret);
        return ul_ret;
    }

    /* 将gtk的乒乓指示位清0 注意:此位ap无作用，sta使用*/
    dmac_reset_gtk_token(pst_mac_vap);

    /* 归还lut index */
    pst_mac_chip = dmac_res_get_mac_chip(pst_dmac_user->st_user_base_info.uc_chip_id);
    if(OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "dmac_user_inactive::pst_mac_chip null!\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_del_ra_lut_index(pst_mac_chip->st_lut_table.auc_ra_lut_index_table, pst_dmac_user->uc_lut_index);


    /* set inactive user lut index invalid */
    pst_dmac_user->uc_lut_index    = DMAC_INVALID_USER_LUT_INDEX;

    pst_dmac_user->bit_active_user = OAL_FALSE;

    /* 通知算法用户闲置 */
    dmac_alg_user_active_change_notify(pst_dmac_user);

    /* active用户数减1 */
    mac_chip_dec_active_user(pst_mac_chip);

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
    /* delete node from chip active dlist */
    oal_dlist_delete_entry(&((mac_user_stru *)pst_dmac_user)->st_active_user_dlist_entry);
#endif

    return OAL_SUCC;
}


oal_uint32  dmac_user_active(dmac_user_stru *pst_dmac_user)
{
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    mac_device_stru    *pst_mac_device;
#endif
    mac_chip_stru      *pst_mac_chip;
    mac_vap_stru       *pst_mac_vap;
    mac_user_stru      *pst_mac_user;
    dmac_vap_stru      *pst_dmac_vap;
    oal_uint8           uc_lut_idx;
    oal_uint16          us_start = 0, us_stop = WLAN_ACTIVE_USER_MAX_NUM;
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
    oal_dlist_head_stru    *pst_dlist_head;
#endif

    pst_mac_user = &(pst_dmac_user->st_user_base_info);

    /* 已经是活跃用户，直接返回 */
    if (OAL_TRUE == pst_dmac_user->bit_active_user)
    {
        return OAL_SUCC;
    }

    pst_mac_vap = mac_res_get_mac_vap(pst_mac_user->uc_vap_id);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{dmac_user_active::null mac vap }");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_mac_chip = dmac_res_get_mac_chip(pst_mac_user->uc_chip_id);
    if(OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_ERROR_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{dmac_user_active::null pst_mac_chip!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_chip->uc_active_user_cnt >= mac_chip_get_max_active_user())
    {
        OAM_WARNING_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{dmac_user_active::uc_active_user_cnt >= WLAN_ACTIVE_USER_MAX_NUM.}");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    pst_mac_device = mac_res_get_dev(pst_mac_user->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{dmac_user_active::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dmac_psta_update_lut_range(pst_mac_device, pst_dmac_vap,  &us_start, &us_stop);
#endif

    /* 申请lut index */
    uc_lut_idx = mac_user_get_ra_lut_index(pst_mac_chip->st_lut_table.auc_ra_lut_index_table, us_start, us_stop);
    if (uc_lut_idx >= mac_chip_get_max_active_user())
    {
        OAM_WARNING_LOG0(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{dmac_user_active::uc_lut_idx >= WLAN_ACTIVE_USER_MAX_NUM.}");
        return OAL_FAIL;
    }
    pst_dmac_user->uc_lut_index      = uc_lut_idx; /* 置为合法lut idx，表示用户是active有效用户 */
    pst_dmac_user->bit_active_user   = OAL_TRUE;

    if(OAL_FALSE == pst_dmac_user->bit_new_add_user)
    {
        /* 非新创建用户状态通知算法用户激活(目前替换用户状态时才需要) */
        dmac_alg_user_active_change_notify(pst_dmac_user);
    }

    OAM_INFO_LOG2(0, 0, "dmac_user_active user_idx=%d, apply lut index=%d.",
        pst_dmac_user->st_user_base_info.us_assoc_id, pst_dmac_user->uc_lut_index);

    /* active用户数加1 */
    mac_chip_inc_active_user(pst_mac_chip);

    /* 设置hal lut index */
    hal_machw_seq_num_index_update_per_tid(pst_dmac_vap->pst_hal_device, uc_lut_idx, OAL_TRUE);

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
    pst_mac_chip = mac_res_get_mac_chip(pst_mac_vap->uc_chip_id);
    /* 加入活跃用户链表表头 */
    pst_dlist_head = &pst_mac_chip->st_user_extend.st_active_user_list_head;
    oal_dlist_add_head(&pst_mac_user->st_active_user_dlist_entry, pst_dlist_head);

    /* 如果活跃用户达到规格-1，启动转非活跃机制 */
    if (pst_mac_chip->uc_active_user_cnt + 1 >= mac_chip_get_max_active_user())
    {
        if (mac_chip_get_max_asoc_user(pst_mac_chip->uc_chip_id) < mac_chip_get_max_active_user())
        {
            /* 最大关联用户小于最大活跃用户数，不开启转非活跃 */
            /* do nothing */
        }
        else if (OAL_FALSE == pst_mac_chip->st_active_user_timer.en_is_registerd)
        {
            FRW_TIMER_CREATE_TIMER(&pst_mac_chip->st_active_user_timer,
                                   dmac_user_active_timer,
                                   WLAN_USER_ACTIVE_TRIGGER_TIME,               /* 1000ms触发一次 */
                                   pst_mac_chip,
                                   OAL_TRUE,
                                   OAM_MODULE_ID_DMAC,
                                   pst_dmac_vap->pst_hal_device->ul_core_id);
        }
    }
#endif

    /* 设置peer地址 */
    hal_ce_add_peer_macaddr(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index, pst_mac_user->auc_user_mac_addr);

    /* 设置密钥 */
    return dmac_11i_add_key_from_user(pst_mac_vap, pst_dmac_user);

}

oal_void  dmac_user_key_search_fail_handler(mac_vap_stru *pst_mac_vap,dmac_user_stru *pst_dmac_user,mac_ieee80211_frame_stru *pst_frame_hdr)
{
    mac_chip_stru    *pst_mac_chip;

    /* 如果用户不存在，什么都不做 */
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_user_key_search_fail_handler::pst_dmac_user null,Deauth it...}");
        dmac_rx_data_user_is_null(pst_mac_vap, pst_frame_hdr);
        return;
    }

    pst_mac_chip = dmac_res_get_mac_chip(pst_dmac_user->st_user_base_info.uc_chip_id);
    if(OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_user_key_search_fail_handler::null pst_mac_chip!}");
        return;
    }

    if (pst_mac_chip->uc_active_user_cnt >= mac_chip_get_max_active_user())
    {
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
        OAM_INFO_LOG4(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND, "dmac_user_key_search_fail_handler RX KEY SEARCH FAIL(id=%d,lut=%d,addr=[xx-xx-xx-xx-%02x-%02x]) REPLACE!",
            pst_dmac_user->st_user_base_info.us_assoc_id,
            pst_dmac_user->uc_lut_index,
            pst_dmac_user->st_user_base_info.auc_user_mac_addr[4],
            pst_dmac_user->st_user_base_info.auc_user_mac_addr[5]);
        dmac_user_max_active_user_handle(pst_dmac_user);
#endif
        return;
    }

    /* active if not up to maximum */
    dmac_user_active(pst_dmac_user);
}

oal_uint32  dmac_user_tx_inactive_user_handler(dmac_user_stru *pst_dmac_user)
{
    mac_chip_stru    *pst_mac_chip;

    pst_mac_chip = dmac_res_get_mac_chip(pst_dmac_user->st_user_base_info.uc_chip_id);
    if(OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY,
            "{dmac_user_tx_inactive_user_handler::null pst_mac_chip!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_mac_chip->uc_active_user_cnt >= mac_chip_get_max_active_user())
    {
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
        OAM_INFO_LOG4(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND, "dmac_user_tx_inactive_user_handler TX(id=%d,lut=%d,addr=[xx-xx-xx-xx-%02x-%02x]) REPLACE!",
            pst_dmac_user->st_user_base_info.us_assoc_id,
            pst_dmac_user->uc_lut_index,
            pst_dmac_user->st_user_base_info.auc_user_mac_addr[4],
            pst_dmac_user->st_user_base_info.auc_user_mac_addr[5]);
        return dmac_user_max_active_user_handle(pst_dmac_user);
#else
        OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG,
                         "{dmac_user_tx_inactive_user_handler::active user >= WLAN_ACTIVE_USER_MAX_NUMl.}");
        return OAL_FAIL;
#endif
    }

    /* active if not up to maximum */
    return dmac_user_active(pst_dmac_user);

}


oal_uint32  dmac_send_null_frame_to_sta(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    dmac_vap_stru             *pst_dmac_vap;
    dmac_user_stru            *pst_dmac_user;
#ifdef _PRE_WLAN_FEATURE_UAPSD
    oal_uint8                  uc_ac          =  WLAN_WME_AC_VO;
    oal_uint8                  uc_uapsd_flag  =  OAL_FALSE;
#endif

#ifdef _PRE_WLAN_FEATURE_UAPSD
	oal_int8                   c_i;
#endif

    /* 获取vap结构信息 */
    pst_dmac_vap  = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "{dmac_send_null_frame_to_sta::pst_dmac_vap null.}");
        return OAL_ERR_CODE_KEEPALIVE_PTR_NULL;
    }

    /* 获取user结构信息 */
    pst_dmac_user = MAC_GET_DMAC_USER(pst_mac_user);

    if (WLAN_VAP_MODE_BSS_AP ==  pst_mac_vap->en_vap_mode)
    {
    #ifdef _PRE_WLAN_FEATURE_UAPSD
        /* AP侧根据user节能状态下选择发null帧还是Qos null帧 */
        uc_uapsd_flag = pst_dmac_user->uc_uapsd_flag;

        if (OAL_FALSE != (uc_uapsd_flag & MAC_USR_UAPSD_EN))
        {
            for(c_i = MAC_AC_PARAM_LEN - 1; c_i >= 0; c_i--)
            {
                if (OAL_TRUE == pst_dmac_user->st_uapsd_status.uc_ac_trigger_ena[(oal_uint8)c_i])
                {
                    uc_ac = (oal_uint8)c_i;
                    break;
                }
            }
            return dmac_send_qosnull(pst_dmac_vap, pst_dmac_user, uc_ac, OAL_FALSE);
        }
    #endif
        /* 用户处于非节能状态 */
        return dmac_psm_send_null_data(pst_dmac_vap, pst_dmac_user, OAL_FALSE);
    }
    else
    {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        /*1151 sta keepalive 临时实现，02实现后废弃*/
        return dmac_psm_send_null_data(pst_dmac_vap, pst_dmac_user, OAL_FALSE);
    }
#else
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "{dmac_send_null_frame_to_sta:: vap mode %d is wrong.}", pst_dmac_vap->st_vap_base_info.en_vap_mode);
    }

    return OAL_FAIL;
#endif
}


oal_uint32 dmac_user_set_bandwith_handler(mac_vap_stru *pst_mac_vap, dmac_user_stru *pst_dmac_user,
                                                       wlan_bw_cap_enum_uint8 en_bw_cap)
{
    mac_user_stru   *pst_mac_user;

    pst_mac_user = &pst_dmac_user->st_user_base_info;
    mac_user_set_bandwidth_info(pst_mac_user, en_bw_cap, en_bw_cap);

    /* user级别调用算法改变带宽通知链 */
    dmac_alg_cfg_user_bandwidth_notify(pst_mac_vap, pst_mac_user);

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                   "{dmac_user_set_bandwith_handler:usr id[%d] bw cap[%d]avil[%d]cur[%d].}",
                   pst_mac_user->us_assoc_id,
                   pst_mac_user->en_bandwidth_cap,
                   pst_mac_user->en_avail_bandwidth,
                   pst_mac_user->en_cur_bandwidth);

    return OAL_SUCC;
}


oal_uint32  dmac_user_keepalive_timer(void *p_arg)
{
    mac_device_stru     *pst_mac_device;
    oal_uint8            uc_vap_idx;
    mac_vap_stru        *pst_mac_vap;
    oal_dlist_head_stru *pst_entry;
    oal_dlist_head_stru *pst_dlist_tmp = OAL_PTR_NULL;
    mac_user_stru       *pst_user_tmp;
    dmac_user_stru      *pst_dmac_user_tmp;
    oal_uint32           ul_runtime;
    oal_uint32           ul_present_time;
#ifndef _PRE_WLAN_FEATURE_USER_EXTEND
    oal_uint32           ul_list_count = 0;
#endif
    oal_uint32           ul_aging_time;
    oal_uint32           ul_send_null_frame_time;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    hal_btcoex_btble_status_stru *pst_btcoex_btble_status;
    hal_to_dmac_chip_stru        *pst_hal_chip = OAL_PTR_NULL;
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    dmac_vap_stru       *pst_dmac_vap;
#endif

    if (OAL_PTR_NULL == p_arg)
    {
        OAM_ERROR_LOG0(0, OAM_SF_KEEPALIVE, "{dmac_user_keepalive_timer::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = (mac_device_stru *)p_arg;

    ul_present_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();


    /* 遍历device下的所有用户，将到期老化的删除掉 */
    /* 业务vap从1开始 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
        {
            OAM_WARNING_LOG0(pst_mac_device->auc_vap_id[uc_vap_idx], OAM_SF_KEEPALIVE, "{dmac_user_keepalive_timer::pst_mac_vap null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        /* 获取dmac vap结构体 */
        pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
        if (OAL_PTR_NULL == pst_dmac_vap)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "{dmac_user_keepalive_timer::pst_dmac_vap null!}");
            return OAL_ERR_CODE_PTR_NULL;
        }
#endif
        /* 用户老化只针对AP模式，非AP模式则跳出 或没有keepalive能力则跳出 */
        if (WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)
        {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#else
            continue;
#endif
        }

        /* 如果keepalive 开关已关, 此时定时器还是开启的话就关闭定时器 */
        if (OAL_FALSE ==  pst_mac_vap->st_cap_flag.bit_keepalive)
        {
            if (OAL_TRUE == pst_mac_device->st_keepalive_timer.en_is_registerd)
            {
               FRW_TIMER_DESTROY_TIMER(&(pst_mac_device->st_keepalive_timer));
            }
            continue;
        }


        ul_aging_time           = WLAN_AP_USER_AGING_TIME;
        ul_send_null_frame_time = WLAN_AP_KEEPALIVE_INTERVAL;

#ifdef _PRE_WLAN_FEATURE_P2P
        if (IS_P2P_GO(pst_mac_vap))
        {
            ul_aging_time           = WLAN_P2PGO_USER_AGING_TIME;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
            pst_hal_chip = DMAC_VAP_GET_HAL_CHIP(pst_mac_vap);
            if (OAL_PTR_NULL == pst_hal_chip)
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_COEX, "{dmac_user_keepalive_timer:: DMAC_VAP_GET_HAL_CHIP null}");
                continue;
            }

            pst_btcoex_btble_status = &(pst_hal_chip->st_btcoex_btble_status);
            if(pst_btcoex_btble_status->un_ble_status.st_ble_status.bit_bt_ba)
            {
                ul_aging_time           = 2*WLAN_P2PGO_USER_AGING_TIME;
            }
#endif
            ul_send_null_frame_time = WLAN_GO_KEEPALIVE_INTERVAL;
        }
#endif

        OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
        {
            pst_user_tmp      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
            /*lint -save -e774 */
            if (OAL_PTR_NULL == pst_user_tmp)
            {
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "pst_user_tmp is null.");
                return OAL_ERR_CODE_PTR_NULL;
            }
            /*lint -restore */
            pst_dmac_user_tmp = MAC_GET_DMAC_USER(pst_user_tmp);

            ul_runtime = (oal_uint32)OAL_TIME_GET_RUNTIME(pst_dmac_user_tmp->ul_last_active_timestamp, ul_present_time);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
            /*sta keepalive功能临时实现，02实现后废弃*/
            if(MAC_SCAN_STATE_IDLE == pst_mac_device->en_curr_scan_state
            && (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
            && (MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state))
            {
                pst_dmac_vap->uc_sta_keepalive_cnt++;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
                if(pst_dmac_vap->uc_sta_keepalive_cnt >= hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_STA_KEEPALIVE_CNT_TH))
#else
                if(pst_dmac_vap->uc_sta_keepalive_cnt >= WLAN_STA_KEEALIVE_CNT_TH)
#endif
                {
                    dmac_send_null_frame_to_sta(pst_mac_vap, pst_user_tmp);
                    pst_dmac_vap->uc_sta_keepalive_cnt = 0;
                }
            }
            else
            {
#endif
                if (ul_runtime > ul_aging_time) //5mins
                {
                    dmac_send_disasoc_misc_event(pst_mac_vap,pst_user_tmp->us_assoc_id, DMAC_DISASOC_MISC_KEEPALIVE);
                }
                else if(ul_runtime > ul_send_null_frame_time) //15s
                {
                    /* 发送队列与节能队列无数据缓冲，发送null 帧触发keepalive ; 否则不发送null 帧 */
                    if ((OAL_TRUE == dmac_psm_is_psm_empty(pst_dmac_user_tmp))
                        && (OAL_TRUE == dmac_psm_is_tid_empty(pst_dmac_user_tmp))
                        && (OAL_TRUE == dmac_psm_is_uapsd_empty(pst_dmac_user_tmp)))
                    {
                        dmac_send_null_frame_to_sta(pst_mac_vap, pst_user_tmp);
                        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "{dmac_user_keepalive_timer::send null frame to %02X:XX:XX:%02X:%02X:%02X.}",
                                pst_user_tmp->auc_user_mac_addr[0],
                                pst_user_tmp->auc_user_mac_addr[3],
                                pst_user_tmp->auc_user_mac_addr[4],
                                pst_user_tmp->auc_user_mac_addr[5]);
                    }
                    else
                    {
                        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "{dmac_user_keepalive_timer::tx queue have buffered data.do not send null frame to %02X:XX:XX:%02X:%02X:%02X.}",
                                pst_user_tmp->auc_user_mac_addr[0],
                                pst_user_tmp->auc_user_mac_addr[3],
                                pst_user_tmp->auc_user_mac_addr[4],
                                pst_user_tmp->auc_user_mac_addr[5]);
                    }

                }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
            }
#endif
#ifndef _PRE_WLAN_FEATURE_USER_EXTEND
            if(ul_list_count++ > mac_chip_get_max_active_user())
            {
#ifdef _PRE_WLAN_DFT_STAT
                OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "{dmac_user_keepalive_timer:st_mac_user_list_head ul_hash_cnt = %d ul_dlist_cnt = %d.}",pst_mac_vap->ul_hash_cnt,pst_mac_vap->ul_dlist_cnt);
#endif
                break;
            }
#endif
        }
    }

    return OAL_SUCC;
}

oal_uint32 dmac_alg_distance_notify_hook(mac_user_stru *pst_user, dmac_alg_distance_notify_info_stru *pst_distance_info)
{
    hal_to_dmac_device_stru *pst_hal_device;
    hal_alg_stat_info_stru  *pst_hal_alg_stat = OAL_PTR_NULL;

    pst_hal_device = dmac_user_get_hal_device(pst_user);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_alg_distance_notify_hook::pst_hal_device null, device_id: %d.}", pst_user->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_alg_stat = &pst_hal_device->st_hal_alg_stat;

    if(pst_hal_alg_stat->en_alg_distance_stat != pst_distance_info->en_old_distance)
    {
        OAM_INFO_LOG2(pst_user->uc_vap_id, OAM_SF_ANY, "{dmac_alg_distance_notify_hook::alg distance status is out of the way1.old:%d, new:%d}",
                        pst_distance_info->en_old_distance, pst_distance_info->en_new_distance);
    }

    if(pst_distance_info->en_new_distance > HAL_ALG_USER_DISTANCE_BUTT)
    {
        OAM_WARNING_LOG2(pst_user->uc_vap_id, OAM_SF_ANY, "{dmac_alg_distance_notify_hook::alg distance status is out of the way2.old:%d, new:%d}",
                        pst_distance_info->en_old_distance, pst_distance_info->en_new_distance);
    }

    pst_hal_alg_stat->en_alg_distance_stat = pst_distance_info->en_new_distance;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#ifdef _PRE_WLAN_PHY_BUGFIX_IMPROVE_CE_TH
    /* PHY算法频偏问题规避,近场时提升1*1improve门限至22,远场恢复门限至16 */
    if(HAL_ALG_USER_DISTANCE_NEAR == pst_hal_alg_stat->en_alg_distance_stat)
    {
        hal_set_improve_ce_threshold(pst_hal_device, 22);
        hal_set_acc_symb_num(pst_hal_device, 1);
    }
    else
    {
        hal_set_improve_ce_threshold(pst_hal_device, 16);
        hal_set_acc_symb_num(pst_hal_device, 3);
    }
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    dmac_config_update_dsss_scaling_reg(pst_hal_device, pst_hal_alg_stat->en_alg_distance_stat);
#endif  /* _PRE_PLAT_FEATURE_CUSTOMIZE */
#endif
    return OAL_SUCC;
}

oal_uint32 dmac_alg_co_intf_notify_hook(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_intf_state_cur, oal_bool_enum_uint8 en_intf_state_old)
{
    hal_alg_stat_info_stru  *pst_hal_alg_stat = OAL_PTR_NULL;

    pst_hal_alg_stat = &pst_hal_device->st_hal_alg_stat;

    if(pst_hal_alg_stat->en_co_intf_state != en_intf_state_old)
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_alg_co_intf_notify_hook::alg co_intf status is out of the way.old:%d, new:%d}",
                        en_intf_state_old, en_intf_state_cur);
    }

    pst_hal_alg_stat->en_co_intf_state = en_intf_state_cur;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_INTF_DET
oal_uint32 dmac_alg_intf_det_notify_hook(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_old_intf_mode, oal_uint8 uc_cur_intf_mode)
{
    hal_alg_stat_info_stru  *pst_hal_alg_stat = OAL_PTR_NULL;

    pst_hal_alg_stat = &pst_hal_device->st_hal_alg_stat;

    if(pst_hal_alg_stat->en_adj_intf_state != uc_old_intf_mode)
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_alg_intf_det_notify_hook::intf status is out of the way.old:%d, new:%d}",
                        uc_old_intf_mode, uc_cur_intf_mode);
    }

    pst_hal_alg_stat->en_adj_intf_state = uc_cur_intf_mode;

    return OAL_SUCC;
}

#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#ifdef _PRE_WLAN_PHY_BUGFIX_IMPROVE_CE_TH
oal_void dmac_rx_compatibility_identify(dmac_user_stru *pst_dmac_user, oal_uint32 ul_rate_kbps)
{
    oal_uint32                          ul_rate_kbps_average;
    hal_to_dmac_device_stru            *pst_hal_device   = OAL_PTR_NULL;
    hal_compatibility_stat_stru        *pst_hal_compatibility_stat = OAL_PTR_NULL;
    hal_alg_stat_info_stru             *pst_hal_alg_stat = OAL_PTR_NULL;
    wlan_protocol_enum_uint8            en_protocol_mode;
    wlan_bw_cap_enum_uint8              en_bandwidth;

    pst_hal_device = dmac_user_get_hal_device(&pst_dmac_user->st_user_base_info);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_RX, "{dmac_rx_compatibility_identify:: pst_hal_device null.");
        return;
    }

    pst_hal_compatibility_stat = &pst_hal_device->st_hal_compatibility_stat;

    if(OAL_FALSE == pst_hal_compatibility_stat->en_compatibility_enable)
    {
        return;
    }
    if(OAL_TRUE == pst_hal_compatibility_stat->en_compatibility_stat)
    {
        return;
    }

    pst_hal_alg_stat = &pst_hal_device->st_hal_alg_stat;

    if((HAL_ALG_USER_DISTANCE_NEAR != pst_hal_alg_stat->en_alg_distance_stat)
       || (HAL_ALG_INTF_DET_ADJINTF_NO != pst_hal_alg_stat->en_adj_intf_state)
       || (OAL_FALSE != pst_hal_alg_stat->en_co_intf_state))
    {
        pst_hal_compatibility_stat->ul_rx_rate = 0;
        pst_hal_compatibility_stat->us_rx_rate_stat_count = 0;
        return;
    }

    if (0 != ul_rate_kbps)
    {
        pst_hal_compatibility_stat->ul_rx_rate += ul_rate_kbps;
        pst_hal_compatibility_stat->us_rx_rate_stat_count++;
    }

    if(DMAC_COMPATIBILITY_PKT_NUM_LIMIT != pst_hal_compatibility_stat->us_rx_rate_stat_count)
    {
        return;
    }

    ul_rate_kbps_average = pst_hal_compatibility_stat->ul_rx_rate/pst_hal_compatibility_stat->us_rx_rate_stat_count;

    en_protocol_mode = pst_dmac_user->st_user_base_info.en_cur_protocol_mode;
    en_bandwidth     = pst_dmac_user->st_user_base_info.en_cur_bandwidth;

    if(ul_rate_kbps_average < pst_hal_compatibility_stat->aul_compatibility_rate_limit[en_bandwidth][en_protocol_mode])
    {
        hal_set_acc_symb_num(pst_hal_device, 1);
        pst_hal_compatibility_stat->en_compatibility_stat = OAL_TRUE;
        OAM_WARNING_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_RX, "{dmac_rx_compatibility_identify::compability status change to %d. (0:normal;1:compatibity mode)}",
                    pst_hal_compatibility_stat->en_compatibility_stat);
        OAM_WARNING_LOG3(0, OAM_SF_ANY, "{dmac_rx_compatibility_identify:average rx rate:%ukbp/s, protocol:%u, bandwidth:%u.}",
                  ul_rate_kbps_average, en_protocol_mode, en_bandwidth);
    }

    pst_hal_compatibility_stat->ul_rx_rate = 0;
    pst_hal_compatibility_stat->us_rx_rate_stat_count = 0;

    return;
}
#endif


OAL_STATIC oal_void dmac_compatibility_handler(dmac_vap_stru *pst_dmac_vap, mac_ap_type_enum_uint8 en_ap_type, oal_bool_enum_uint8 en_is_add_user)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
    hal_to_dmac_device_stru    *pst_hal_device = OAL_PTR_NULL;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);

    if(en_is_add_user)
    {
        switch (en_ap_type)
        {
            case MAC_AP_TYPE_GOLDENAP:
#ifdef _PRE_WLAN_PHY_BUGFIX_VHT_SIG_B
                hal_enable_sigB(pst_dmac_vap->pst_hal_device, OAL_TRUE);
                hal_enable_improve_ce(pst_dmac_vap->pst_hal_device, OAL_FALSE);
#endif
                break;

            case MAC_AP_TYPE_NORMAL:
#if _PRE_WLAN_PHY_BUGFIX_IMPROVE_CE_TH
                pst_hal_device->st_hal_compatibility_stat.en_compatibility_enable = OAL_TRUE;
#endif
                break;
            default:
                break;
        }
    }
    else
    {
        switch (en_ap_type)
        {
            case MAC_AP_TYPE_GOLDENAP:
#ifdef _PRE_WLAN_PHY_BUGFIX_VHT_SIG_B
                hal_enable_sigB(pst_dmac_vap->pst_hal_device, OAL_FALSE);
                hal_enable_improve_ce(pst_dmac_vap->pst_hal_device, OAL_TRUE);
#endif
                break;
            case MAC_AP_TYPE_NORMAL:
#if _PRE_WLAN_PHY_BUGFIX_IMPROVE_CE_TH
                hal_set_acc_symb_num(pst_dmac_vap->pst_hal_device, 3);
                hal_set_improve_ce_threshold(pst_dmac_vap->pst_hal_device, 16);
                pst_hal_device->st_hal_compatibility_stat.en_compatibility_enable = OAL_FALSE;
                pst_hal_device->st_hal_compatibility_stat.en_compatibility_stat   = OAL_FALSE;
#endif
                break;
            default:
                break;
        }
    }

    return;
#endif
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE) && defined(_PRE_FEATURE_WAVEAPP_CLASSIFY)
typedef  struct
{
    wlan_protocol_enum_uint8                en_mode;
    wlan_bw_cap_enum_uint8                  en_bw;
    wlan_nss_enum_uint8                     en_nss;
    oal_uint8                               uc_best_mcs;
}waveapp_best_mcs;

//waveapp仪器用户进行峰值测试时使用的速率一定是vap当前配置支持的最高速率，如果有其它仪器场景的峰值需要测试
//扩展此表格即可
OAL_STATIC OAL_CONST waveapp_best_mcs  ast_waveapp_best_mcs_tbl[]=
{
    {WLAN_VHT_MODE,WLAN_BW_CAP_80M,WLAN_DOUBLE_NSS,9},    //11ac80 双天线,最高速率MCS9
    {WLAN_VHT_MODE,WLAN_BW_CAP_40M,WLAN_DOUBLE_NSS,9},    //11ac40 双天线,最高速率MCS9
    {WLAN_VHT_MODE,WLAN_BW_CAP_20M,WLAN_DOUBLE_NSS,8},    //11ac20 双天线,最高速率MCS8
    {WLAN_HT_MODE,WLAN_BW_CAP_40M,WLAN_DOUBLE_NSS,15},    //11n40 双天线,最高速率MCS15
    {WLAN_HT_MODE,WLAN_BW_CAP_20M,WLAN_DOUBLE_NSS,15},    //11n20 双天线,最高速率MCS15
};


oal_uint8   dmac_get_waveapp_user_expect_mcs(mac_vap_stru *pst_mac_vap)
{
    oal_uint8   i;
    wlan_bw_cap_enum_uint8 en_bw;

    mac_vap_get_bandwidth_cap(pst_mac_vap, &en_bw);
    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_get_waveapp_user_expect_mcs:: protocol=%d,bw=%d,nss=%d.}",
    pst_mac_vap->en_protocol,en_bw,pst_mac_vap->en_vap_rx_nss);
    for(i = 0; i < OAL_ARRAY_SIZE(ast_waveapp_best_mcs_tbl);i++)
    {
        if((ast_waveapp_best_mcs_tbl[i].en_bw == en_bw)
        &&(ast_waveapp_best_mcs_tbl[i].en_mode == pst_mac_vap->en_protocol)
        &&(ast_waveapp_best_mcs_tbl[i].en_nss == pst_mac_vap->en_vap_rx_nss))
        {
            return ast_waveapp_best_mcs_tbl[i].uc_best_mcs;
        }
    }

    return 0xff;
}

#endif


oal_uint32  dmac_user_add(frw_event_mem_stru *pst_event_mem)
{
    dmac_user_stru                 *pst_dmac_user;
    frw_event_stru                 *pst_event;
    dmac_ctx_add_user_stru         *pst_add_user_payload;
    oal_uint32                      ul_ret;
    mac_chip_stru                  *pst_mac_chip;
    mac_device_stru                *pst_mac_device;
    mac_vap_stru                   *pst_mac_vap;
    dmac_vap_stru                  *pst_dmac_vap;
    oal_uint16                      us_user_idx;

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    oal_uint16                       us_start = 0, us_stop = WLAN_ACTIVE_USER_MAX_NUM;
    oal_uint8                       *puc_mac_addr = OAL_PTR_NULL;
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE) && defined(_PRE_FEATURE_WAVEAPP_CLASSIFY)
    hal_to_dmac_device_stru   *pst_hal_device;
#endif

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_event_mem)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_user_add::pst_event_mem null.}");
        return OAL_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_add_user_payload = (dmac_ctx_add_user_stru *)pst_event->auc_event_data;
    us_user_idx          = pst_add_user_payload->us_user_idx;
    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_CFG, "{dmac_user_add::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_chip = dmac_res_get_mac_chip(pst_mac_device->uc_chip_id);
    if (OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_user_add::pst_mac_chip null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请dmac user */
    ul_ret = dmac_user_alloc(us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG2(pst_event->st_event_hdr.uc_vap_id, OAM_SF_CFG, "{dmac_user_add::mac_res_alloc_dmac_user failed[%d], userindx[%d].", ul_ret, us_user_idx);
        return ul_ret;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_user_add::null pointer,pst_dmac_vap[%d].}",
                    pst_event->st_event_hdr.uc_vap_id);
        dmac_user_free(us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &pst_dmac_vap->st_vap_base_info;
    pst_dmac_user = mac_res_get_dmac_user(us_user_idx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_user_add:: pst_dmac_user[%d] null pointer.}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_CCA_OPT
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    {
        hal_cfg_custom_cca_stru *pst_cfg_cus_cca;
        oal_int8 c_delta_cca_ed_high_20th_default;
        oal_int8 c_delta_cca_ed_high_40th_default;

        hal_config_get_cus_cca_param(&pst_cfg_cus_cca);
        c_delta_cca_ed_high_20th_default = HAL_CCA_OPT_GET_DEFAULT_ED_20TH(pst_dmac_vap->st_vap_base_info.st_channel.en_band, pst_cfg_cus_cca);
        c_delta_cca_ed_high_40th_default = HAL_CCA_OPT_GET_DEFAULT_ED_40TH(pst_dmac_vap->st_vap_base_info.st_channel.en_band, pst_cfg_cus_cca);
        hal_set_ed_high_th(pst_dmac_vap->pst_hal_device, c_delta_cca_ed_high_20th_default, c_delta_cca_ed_high_40th_default, OAL_TRUE);
    }
#else
    hal_set_ed_high_th(pst_dmac_vap->pst_hal_device, HAL_CCA_OPT_ED_HIGH_20TH_DEF, HAL_CCA_OPT_ED_HIGH_40TH_DEF, OAL_TRUE);
#endif
#endif


    /* mac user初始化 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_user_init(&(pst_dmac_user->st_user_base_info), us_user_idx, pst_add_user_payload->auc_user_mac_addr,
                  pst_event->st_event_hdr.uc_chip_id,
                  pst_event->st_event_hdr.uc_device_id,
                  pst_event->st_event_hdr.uc_vap_id);

    ul_ret = mac_vap_add_assoc_user(pst_mac_vap, us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        dmac_user_free(us_user_idx);
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_user_add::mac_vap_add_assoc_user fail.}");
        return OAL_FAIL;
    }

    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        mac_vap_set_assoc_id(pst_mac_vap, us_user_idx);

        /* 关联时初始化信道切换结构体 */
        OAL_MEMZERO(&(pst_mac_vap->st_ch_switch_info), OAL_SIZEOF(mac_ch_switch_info_stru));
        pst_mac_vap->st_ch_switch_info.en_new_bandwidth = WLAN_BAND_WIDTH_BUTT;
    }

    /* 重新关联用户的时候，重置乒乓位 */
    dmac_reset_gtk_token(pst_mac_vap);

    /* MAC统计信息初始化 */
    OAL_MEMZERO(&(pst_mac_device->st_mac_key_statis_info),OAL_SIZEOF(hal_mac_key_statis_info_stru));
#endif

    /* dmac user初始化 */
    dmac_user_init(pst_dmac_user);

    /* add user时，初始化user保存的rssi值 */
    oal_rssi_smooth(&(pst_dmac_user->s_rx_rssi), pst_add_user_payload->c_rssi);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /*p2p noa也需要节能队列*/
    if(WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode || IS_P2P_CL(pst_mac_vap))
#else
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
#endif
    {
        ul_ret = dmac_psm_user_ps_structure_init(pst_dmac_user);
        if (OAL_SUCC != ul_ret)
        {
            dmac_user_free(us_user_idx);
            return ul_ret;
        }
    }
    /* 初始化linkloss的状态 */
    dmac_vap_linkloss_clean(pst_dmac_vap);

    /* 启用keepalive定时器, 若定时器已开启, 则不用再开启 */
    if ((OAL_FALSE == pst_mac_device->st_keepalive_timer.en_is_registerd) &&
        (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_keepalive))
    {
        FRW_TIMER_CREATE_TIMER(&(pst_mac_device->st_keepalive_timer),
                               dmac_user_keepalive_timer,
                               WLAN_AP_KEEPALIVE_TRIGGER_TIME,                /* 30s触发一次(1101中60s触发一次) TBD:参数待定 */
                               pst_mac_device,
                               OAL_TRUE,
                               OAM_MODULE_ID_DMAC,
                               pst_mac_device->ul_core_id);
    }

#ifdef _PRE_WLAN_FEATURE_UAPSD
    ul_ret = dmac_uapsd_user_init(pst_dmac_user);
    if(OAL_SUCC!= ul_ret)
    {
        OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_user_add::dmac_uapsd_user_init failed.}");
        dmac_user_free(us_user_idx);
        return ul_ret;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_AP_PM
    if ((WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
        || (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
        )
    {
        dmac_pm_post_event(pst_dmac_vap,AP_PWR_EVENT_USR_ASSOC,0,OAL_PTR_NULL);
    }

#if(_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT)
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        dmac_pcie_ps_switch(&pst_dmac_vap->st_vap_base_info, OAL_FALSE);
    }
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
    dmac_psm_init_null_frm_cnt(pst_dmac_vap);
#endif  /* _PRE_WLAN_FEATURE_STA_PM */

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    dmac_psta_update_lut_range(pst_mac_device, pst_dmac_vap, &us_start, &us_stop);
#endif

    /* 芯片活跃用户超规格处理(和lut资源对应，超过了lut资源限制) */
    if (pst_mac_chip->uc_active_user_cnt >= WLAN_ACTIVE_USER_MAX_NUM)
    {
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
        OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG, "dmac_user_add::active user up to maximum, call max active user handle.");
        ul_ret = dmac_user_max_active_user_handle(pst_dmac_user);
        if(OAL_SUCC!= ul_ret)
        {
            OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_user_add::dmac_user_max_active_user_handle failed, free user.}");
            dmac_user_free(us_user_idx);
            return ul_ret;
        }
#else
        OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG, "dmac_user_add::active user up to maximum, user extend feature not support, free the user!");
        dmac_user_free(us_user_idx);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
#endif
    }
    else
    {
        /* active用户，申请lut资源 */
        ul_ret = dmac_user_active(pst_dmac_user);
        if(OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_user_add::dmac_user_active failed, free user.}");
            dmac_user_free(us_user_idx);
            return ul_ret;
        }
    }

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (mac_is_proxysta_enabled(pst_mac_device))
    {
        if (mac_vap_is_vsta(pst_mac_vap))
        {
            puc_mac_addr = mac_mib_get_StationID(pst_mac_vap);
        }
    }

    hal_ce_add_peer_macaddr(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index, puc_mac_addr);
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* offload下chip下用户数++ */
    mac_chip_inc_assoc_user(pst_mac_chip);
#endif

    /* 本hal device上用户数+1 */
    hal_device_inc_assoc_user_nums(pst_dmac_vap->pst_hal_device);

    /* 清零芯片维护的序列号 */
    dmac_clear_tx_qos_seq_num(pst_dmac_vap, pst_dmac_user);

#ifdef _PRE_WLAN_FEATURE_ROAM
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        dmac_sta_roam_trigger_init(MAC_GET_DMAC_VAP(pst_mac_vap));
    }
#endif //_PRE_WLAN_FEATURE_ROAM

#ifdef _PRE_WLAN_FEATURE_11V
    /* 申请11V动态内存 并初始化 */
    pst_dmac_user->pst_11v_ctrl_info = (dmac_user_11v_ctrl_stru*) OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_user_11v_ctrl_stru), OAL_TRUE);
    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user->pst_11v_ctrl_info))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_user_add:: pst_dmac_user->pst_11v_ctrl_info null pointer.}");
    }
    else
    {
        pst_dmac_user->pst_11v_ctrl_info->dmac_11v_callback_fn = OAL_PTR_NULL;
        pst_dmac_user->pst_11v_ctrl_info->uc_user_bsst_token = 1;
        pst_dmac_user->pst_11v_ctrl_info->uc_user_status = DMAC_11V_AP_STATUS_INIT;
    }
    #ifdef _PRE_DEBUG_MODE
    OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_user_add:: 11v info: ctrl[0x%p].}",pst_dmac_user->pst_11v_ctrl_info);
    #endif
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        dmac_bsd_user_add_handle(pst_dmac_vap,pst_dmac_user);
    }
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE) && defined(_PRE_FEATURE_WAVEAPP_CLASSIFY)
    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_user_add::pst_hal_device null.}");
    }
    else
    {

       if((WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)&&(0 == pst_mac_vap->us_user_nums))
       {
            //第一个用户关联进来时将仪器识别初始化为非仪器场景
            pst_hal_device->ul_rx_mcs_cnt = 0;
            pst_hal_device->us_rx_assoc_id = 0xffff;
            pst_hal_device->uc_rx_expect_mcs= dmac_get_waveapp_user_expect_mcs(pst_mac_vap);
            //TBD:以免重新定义一个枚举数据结构，此处用OAL_BUTT表示仪器未识别的状态
            //pst_hal_device->en_test_is_on_waveapp_flag = OAL_FALSE;  //新用户关联初始化为未识别状态
       }
    }
#endif

    if (IS_LEGACY_STA(pst_mac_vap))
    {
        /* 带宽切换状态机 */
        dmac_sta_bw_switch_fsm_post_event((dmac_vap_stru *)pst_mac_vap, DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC, 0, OAL_PTR_NULL);
    }

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    dmac_compatibility_handler(pst_dmac_vap, pst_add_user_payload->en_ap_type, OAL_TRUE);
    OAM_WARNING_LOG_ALTER(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG,
                 "{dmac_user_add::add user[%d] SUCC! vap user nums[%d],chip user nums[%d],hal device user nums[%d], user mac_addr:%02x:xx:xx:xx:%02x:%02x, devID[%d]bw[%d]lut index[%d].}",
                 10 ,us_user_idx, pst_mac_vap->us_user_nums, pst_mac_chip->uc_assoc_user_cnt, pst_dmac_vap->pst_hal_device->uc_assoc_user_nums,
                 pst_add_user_payload->auc_user_mac_addr[0],
                 pst_add_user_payload->auc_user_mac_addr[4],
                 pst_add_user_payload->auc_user_mac_addr[5],
                 pst_mac_device->uc_device_id,
                 pst_dmac_user->st_user_base_info.en_bandwidth_cap,
                 pst_dmac_user->uc_lut_index);
#else
    OAM_WARNING_LOG4(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG,
                 "{dmac_user_add::add user[%d] SUCC! vap user nums[%d],chip user nums[%d], user mac_addr:xx:xx:xx:xx:xx:%02x.}",
                 us_user_idx, pst_mac_vap->us_user_nums, pst_mac_chip->uc_assoc_user_cnt, pst_add_user_payload->auc_user_mac_addr[5]);

#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        hal_set_btcoex_soc_gpreg1(OAL_TRUE, BIT2, 2);   // 入网流程开始,发起auth
    }
#endif

#ifdef _PRE_WLAN_PRODUCT_1151V200
    if ((WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
        && (pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)
        && (OAL_FALSE == pst_mac_vap->uc_1st_asoc_done)
        && (OAL_TRUE == pst_mac_vap->uc_force_resp_mode_80M))
    {
        hal_set_80m_resp_mode(pst_dmac_vap->pst_hal_device, OAL_TRUE);

        OAM_WARNING_LOG0(0, OAM_SF_CFG, "dmac_user_add:  1st assoc process start!");
    }
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        /* 注册统计节点 */
        dmac_stat_qos_init_user(pst_dmac_user);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_NO_FRM_INT
    if (IS_LEGACY_STA(pst_mac_vap))
    {
        pst_dmac_vap->st_sta_pm_handler.uc_max_skip_bcn_cnt = DMAC_PSM_MAX_SKIP_BCN_CNT;
    }
    else
    {
        pst_dmac_vap->st_sta_pm_handler.uc_max_skip_bcn_cnt = 0;
    }
#endif

    return OAL_SUCC;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_void dmac_ap_resume_all_user(mac_vap_stru *pst_mac_vap)
{
    oal_dlist_head_stru                  *pst_entry;
    mac_user_stru                        *pst_user_tmp;
    dmac_user_stru                       *pst_dmac_user_tmp;
    dmac_vap_stru                        *pst_dmac_vap;

#ifndef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
    hal_to_dmac_device_stru              *pst_hal_device = OAL_PTR_NULL;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ap_resume_all_user::pst_hal_device null}");
        return;
    }
#endif

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id,OAM_SF_ANY,"{dmac_ap_resume_all_user::mac_res_get_dmac_vap fail or pst_dmac_vap->pst_hal_vap NULL}");
        return;
    }

    /* 遍历vap下所有用户,pause tid 队列 */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        /*lint -save -e774 */
        if (OAL_PTR_NULL == pst_user_tmp)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY,"{dmac_ap_resume_all_user::pst_user_tmp null.");
            continue;
        }
        /*lint -restore */

        pst_dmac_user_tmp = MAC_GET_DMAC_USER(pst_user_tmp);

        /* 恢复tid，并将节能队列的包发出去。*/
        dmac_user_resume(pst_dmac_user_tmp);

        /*如果用户此时不在doze状态，才发包*/
        if(OAL_TRUE != pst_dmac_user_tmp->bit_ps_mode)
        {
        #ifndef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
            /* 恢复该vap下的所有用户的硬件队列的发送 */
            hal_tx_disable_peer_sta_ps_ctrl(pst_hal_device, pst_dmac_user_tmp->uc_lut_index);
        #endif
            /* 将所有的缓存帧发送出去 */
            dmac_psm_queue_flush(pst_dmac_vap, pst_dmac_user_tmp);
        }
    }
}

oal_void dmac_full_phy_freq_user_add(mac_vap_stru *pst_mac_vap, dmac_user_stru *pst_dmac_user)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_full_phy_freq_user_add::vap[%d] get hal device fail!}", pst_mac_vap->uc_vap_id);
        return;
    }

    /* STAUT支持vht ht uc_full_phy_freq_user_cnt++ */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        if (OAL_TRUE == DMAC_GET_USER_SUPPORT_VHT(&(pst_dmac_user->st_user_base_info)) ||
                (OAL_TRUE == DMAC_GET_USER_SUPPORT_HT(&(pst_dmac_user->st_user_base_info))))
        {
            pst_hal_device->uc_full_phy_freq_user_cnt++;
        }
    }
    /* APUT不能降频 */
    else if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        pst_hal_device->uc_full_phy_freq_user_cnt++;
    }

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "dmac_full_phy_freq_user_add::now vap mode[%d]hal device[%d]remain[%d]full phy user",
                        pst_mac_vap->en_vap_mode, pst_hal_device->uc_device_id, pst_hal_device->uc_full_phy_freq_user_cnt);

    /*phy 降频处理 */
    hal_process_phy_freq(pst_hal_device);
}

oal_uint32  dmac_user_del_offload(mac_vap_stru* pst_vap, oal_uint16 us_user_idx)
{
    mac_device_stru   *pst_mac_device;
    oal_uint32         ul_ret;
    mac_chip_stru     *pst_mac_chip;
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    mac_cfg_txop_sta_stru    st_txop_info = {0};
#endif

    if (OAL_PTR_NULL == pst_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_user_del_offload::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 从vap中删除用户 */
    ul_ret = mac_vap_del_user(pst_vap, us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_user_del::vap del failed.}");
    }

    pst_mac_device = mac_res_get_dev(pst_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_user_del_offload::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_chip = dmac_res_get_mac_chip(pst_mac_device->uc_chip_id);
    if (OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_user_del::pst_mac_chip null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 释放用户内存 */
    ul_ret = dmac_user_free(us_user_idx);
    if (OAL_SUCC == ul_ret)
    {
        /* offload模式下chip下已关联user个数-- */
        mac_chip_dec_assoc_user(pst_mac_chip);
    }
    else
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{dmac_user_del_offload::mac_res_free_mac_user failed[%d].", ul_ret);
    }

    /* STA模式下将关联的VAP的id置为非法值 */
    if (WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode)
    {
        mac_vap_set_assoc_id(pst_vap, MAC_INVALID_USER_ID);

#ifdef _PRE_WLAN_FEATURE_TXOPPS
        dmac_txopps_set_machw(pst_vap);
        /* 去关联时MAC partial aid配置为0 */
        dmac_txopps_set_machw_partialaid(pst_vap, 0, (oal_uint8*)(&st_txop_info));

        //dmac_txopps_clear_machw_partialaid_sta(pst_vap);
#endif
    }

    return OAL_SUCC;
}

oal_void dmac_full_phy_freq_user_del(mac_vap_stru *pst_mac_vap, dmac_user_stru *pst_dmac_user)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_bool_enum_uint8             en_need_del_full_phy_user = OAL_FALSE;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_full_phy_freq_user_del::DMAC_VAP_GET_HAL_DEVICE null");
        return;
    }

    /* STAUT支持vht ht uc_full_phy_freq_user_cnt-- */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        if (OAL_TRUE == DMAC_GET_USER_SUPPORT_VHT(&(pst_dmac_user->st_user_base_info)) ||
        (OAL_TRUE == DMAC_GET_USER_SUPPORT_HT(&(pst_dmac_user->st_user_base_info))))
        {
            en_need_del_full_phy_user = OAL_TRUE;
        }
    }
    else if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        en_need_del_full_phy_user = OAL_TRUE;
    }

    if (OAL_TRUE == en_need_del_full_phy_user)
    {
        if (0 == pst_hal_device->uc_full_phy_freq_user_cnt)
        {
            OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG, "dmac_full_phy_freq_user_del::vht support user cnt should not be 0");
        }
        else
        {
            pst_hal_device->uc_full_phy_freq_user_cnt--;
        }
    }
    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "dmac_full_phy_freq_user_del::now vap mode[%d],hal device[%d]remain[%d]full phy user",
                        pst_mac_vap->en_vap_mode, pst_hal_device->uc_device_id, pst_hal_device->uc_full_phy_freq_user_cnt);

    /*phy 降频处理 */
    hal_process_phy_freq(pst_hal_device);
}

oal_uint32 dmac_psm_overrun_throw_half(dmac_user_stru  *pst_dmac_user,oal_uint32 ul_mpdu_delete_num)
{
    oal_uint32     ul_tid_mpdu_num       = 0; /*tid队列中包的数目，包括重传队列的*/
    oal_uint32     ul_psm_mpdu_num       = 0;  /*节能队列中包的数目*/
    oal_uint32     ul_psm_delete_num     = 0;  /*节能队列需要删除的mpdu数目*/
    oal_uint8      uc_tid_idx            = 0;
    oal_uint32     ul_ret                = 0;

    dmac_tid_stru  *pst_tid_queue;

    /*得到用户当前tid队列中的包*/
    ul_tid_mpdu_num = dmac_psm_tid_mpdu_num(pst_dmac_user);
    ul_psm_mpdu_num = (oal_uint32)oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num);


    /*先删tid队列的*/
    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx++)
    {
        pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[uc_tid_idx]);
        ul_ret = dmac_tid_delete_mpdu_head(pst_tid_queue, pst_dmac_user->ast_tx_tid_queue[uc_tid_idx].us_mpdu_num);
        if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_psm_overrun_throw_half::tid[%d] throw mpdu failed !}",uc_tid_idx);
            return ul_ret;
        }

    }
    /*如果tid队列中的mpdu数目小于需要删除的mpdu数目，再删节能队列的*/
    if(ul_tid_mpdu_num < ul_mpdu_delete_num)
    {
         ul_psm_delete_num = ul_mpdu_delete_num - ul_tid_mpdu_num;
         if(ul_psm_delete_num <=  ul_psm_mpdu_num)
         {
             dmac_psm_delete_ps_queue_head(pst_dmac_user,ul_psm_delete_num);
         }
    }
    return OAL_SUCC;
}


oal_void dmac_user_ps_queue_overrun_notify(mac_vap_stru *pst_mac_vap)
{
    mac_user_stru                *pst_mac_user;
    dmac_user_stru               *pst_dmac_user;
    oal_dlist_head_stru          *pst_entry;
    oal_uint32                    ul_ps_mpdu_num = 0;
    oal_uint32                    ul_mpdu_num_sum = 0;
    dmac_vap_stru                *pst_dmac_vap;
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id,OAM_SF_ANY,"{dmac_user_ps_queue_overrun_notify::mac_res_get_dmac_vap fail or pst_dmac_vap->pst_hal_vap NULL}");
        return;
    }
    /* 遍历VAP下所有USER */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_dmac_vap->st_vap_base_info.st_mac_user_list_head))
    {
        pst_mac_user = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        /*lint -save -e774 */
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
        {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_multi_user_ps_queue_overrun_notify::null pointer.}");
            continue;
        }
        /*lint -restore */

        pst_dmac_user = MAC_GET_DMAC_USER(pst_mac_user);

        /*用户处于节能状态，且节能队列中有包时，节能计数器加1；如果节能队列为空，则计数器清零。*/
        if(OAL_TRUE == pst_dmac_user->bit_ps_mode)
        {
            if(OAL_FALSE == dmac_psm_is_psm_empty(pst_dmac_user))
            {
                pst_dmac_user->st_ps_structure.uc_ps_time_count++;
            }
            else
            {
                pst_dmac_user->st_ps_structure.uc_ps_time_count = 0;
                continue;
            }
            /*如果连续5次检查到节能队列中有包，且此时节能队列中包的数目大于128，则认为用户异常，避免内存耗尽，进行丢包,返回true*/
            ul_ps_mpdu_num = (oal_uint32)oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num);
            ul_mpdu_num_sum = ul_ps_mpdu_num + dmac_psm_tid_mpdu_num(pst_dmac_user);
            if(5 <= pst_dmac_user->st_ps_structure.uc_ps_time_count && ul_ps_mpdu_num > MAX_MPDU_NUM_IN_PS_QUEUE)
            {
                /*丢包时先丢tid队列的，再丢节能队列的*/
                OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_user_ps_queue_overrun_notify::PS mpdu num[%d], TID mpdu num[%d]!}",ul_ps_mpdu_num,dmac_psm_tid_mpdu_num(pst_dmac_user));
                dmac_psm_overrun_throw_half(pst_dmac_user,ul_mpdu_num_sum/2);
                pst_dmac_user->st_ps_structure.uc_ps_time_count = 0;
            }
         }
    }
}
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) && defined(_PRE_WLAN_PRODUCT_1151V200)

OAL_STATIC void  dmac_ap_triple_nss_check(dmac_vap_stru  *pst_dmac_vap)
{
    mac_device_stru               *pst_device;
    mac_chip_stru                 *pst_mac_chip;
    oal_uint8                      uc_device_num;
    oal_uint8                      uc_vap_num;

    mac_user_stru                 *pst_mac_user;
    mac_vap_stru                  *pst_mac_vap;
    oal_dlist_head_stru           *pst_list_pos;

    pst_mac_chip = mac_res_get_mac_chip(pst_dmac_vap->st_vap_base_info.uc_chip_id);

    if (OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ASSOC,
                       "{dmac_ap_triple_nss_check::pst_mac_chip[id=%d] is null.}", pst_dmac_vap->st_vap_base_info.uc_chip_id);
        return;
    }

    for (uc_device_num = 0; uc_device_num < WLAN_DEVICE_MAX_NUM_PER_CHIP; uc_device_num++)
    {
        pst_device = mac_res_get_dev(pst_mac_chip->auc_device_id[uc_device_num]);

        if (OAL_PTR_NULL == pst_device)
        {
           OAM_ERROR_LOG1(0, OAM_SF_ASSOC,
                       "{dmac_ap_triple_nss_check::pst_device[id=%d] is null.}", pst_mac_chip->auc_device_id[uc_device_num]);
           return;
        }

        if(pst_device->en_band_cap !=  WLAN_BAND_CAP_5G)
        {
            /* 非5G频段 20038c45改成1b81b8*/
            hal_config_adc_target(pst_dmac_vap->pst_hal_device, 0x1b81b8);
            return;
        }

        /* vap遍历 */
        for (uc_vap_num = 0; uc_vap_num < pst_device->uc_vap_num; uc_vap_num++)
        {
            pst_mac_vap = (mac_vap_stru*)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_num]);
            if (OAL_PTR_NULL == pst_mac_vap)
            {
                continue;
            }
            //遍历vap下面的所有user
            MAC_VAP_FOREACH_USER(pst_mac_user, pst_mac_vap, pst_list_pos)
            {
                /* 找到非三天线用户20038c45改成1b81b8*/
                if(pst_mac_user->en_user_num_spatial_stream != WLAN_TRIPLE_NSS)
                {
                    hal_config_adc_target(((dmac_vap_stru*)pst_mac_vap)->pst_hal_device, 0x1b81b8);
                    return;
                }
            }
        }
    }

    /* 都是三天线用户20038c45改成1c41c4 */
    hal_config_adc_target(pst_dmac_vap->pst_hal_device, 0x1c41c4);
    return;
}
#endif


oal_uint32  dmac_user_add_notify_alg(frw_event_mem_stru *pst_event_mem)
{
    oal_uint32               ul_ret;
    dmac_vap_stru           *pst_dmac_vap;
    mac_vap_stru            *pst_mac_vap;
    frw_event_stru          *pst_event;
    dmac_user_stru          *pst_dmac_user;
    mac_user_stru           *pst_mac_user;
    dmac_ctx_add_user_stru  *pst_add_user_payload;
    mac_device_stru         *pst_mac_device;
    hal_to_dmac_device_stru *pst_hal_device;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_channel_stru         st_channel;
#endif

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 通知算法 */
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_UM,
                         "{dmac_user_add_notify_alg::mac_res_get_dmac_vap failed.vap_idx:[%u].");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &pst_dmac_vap->st_vap_base_info;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{dmac_user_add_notify_alg::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{dmac_user_add_notify_alg::DMAC_VAP_GET_HAL_DEVICE null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_add_user_payload = (dmac_ctx_add_user_stru *)pst_event->auc_event_data;
    //oal_memcmp(pst_dmac_vap->st_vap_base_info.auc_bssid, pst_add_user_payload->auc_bssid, WLAN_MAC_ADDR_LEN);
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_add_user_payload->us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_UM, "{dmac_user_add_notify_alg::pst_mac_user[%d] null.",
            pst_add_user_payload->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_user = &pst_dmac_user->st_user_base_info;

    mac_vap_set_aid(pst_mac_vap, pst_add_user_payload->us_sta_aid);
    mac_user_set_assoc_id(pst_mac_user, pst_add_user_payload->us_user_idx);

    mac_user_set_ht_hdl(pst_mac_user, &pst_add_user_payload->st_ht_hdl);
    mac_user_set_vht_hdl(pst_mac_user, &pst_add_user_payload->st_vht_hdl);

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    mac_vap_update_txopps(pst_mac_vap, pst_mac_user);
#endif

    /* 先通知dbac判断是否需要迁移,后续各个算法再注册 */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        dmac_full_phy_freq_user_add(pst_mac_vap, pst_dmac_user);
#endif
        dmac_alg_vap_up_notify(pst_mac_vap);
    }

    ul_ret = dmac_alg_add_assoc_user_notify(pst_dmac_vap, pst_dmac_user);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_event->st_event_hdr.uc_vap_id, OAM_SF_CFG,
                 "{dmac_user_add_notify_alg::dmac_alg_add_assoc_user_notify failed[%d].", ul_ret);
    }

    /* 调用算法钩子函数 */
    dmac_alg_cfg_user_spatial_stream_notify(pst_mac_user);

    /* 调用算法改变带宽通知链 */
    dmac_alg_cfg_user_bandwidth_notify(pst_mac_vap, pst_mac_user);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 变量初始化为当前vap信息 */
    st_channel = pst_mac_vap->st_channel;

    /* 选择需要设置的信道信息 */
    dmac_chan_select_real_channel(pst_mac_device, &st_channel, st_channel.uc_chan_number);

    OAM_WARNING_LOG4(pst_event->st_event_hdr.uc_vap_id, OAM_SF_UM,
                 "{dmac_user_add_notify_alg:: vap bw[%d], new set bandwith[%d], hal_device bandwidth=[%d], en_dbac_running=[%d].",
                 pst_mac_vap->st_channel.en_bandwidth, st_channel.en_bandwidth,
                 pst_hal_device->st_wifi_channel_status.en_bandwidth,
                 pst_mac_device->en_dbac_running);

    if ((OAL_FALSE == mac_is_dbac_running(pst_mac_device))
        && (st_channel.en_bandwidth != pst_hal_device->st_wifi_channel_status.en_bandwidth))
    {
        /* 切换信道不需要清fifo，传入FALSE */
        dmac_mgmt_switch_channel(pst_hal_device, &st_channel, OAL_FALSE);
    }
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) && defined(_PRE_WLAN_PRODUCT_1151V200)
    /*
    遍历当前chip下所有device/vap均工作在5g，则进行下列操作:
    关联的所有用户全部为三天线用户时20038c45改成1c41c4，其他场景全部回退成原始值1b81b8
    */
    dmac_ap_triple_nss_check(pst_dmac_vap);
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP

oal_void dmac_user_del_p2p_in_dyn_bw(dmac_device_stru *pst_dmac_dev, dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8                       uc_vap_idx;
    mac_vap_stru                   *pst_tmp_vap;
    mac_user_stru                  *pst_mac_user;

    if (OAL_PTR_NULL == pst_dmac_dev || OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(0,OAM_SF_CFG, "dmac_user_del_p2p_in_dyn_bw::input null ptr");
        return;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_dmac_dev->pst_device_base_info->uc_vap_num; uc_vap_idx++)
    {
        pst_tmp_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_dmac_dev->pst_device_base_info->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_tmp_vap)
        {
            continue;
        }

        /* 搜索除本vap外其他vap */
        if (pst_tmp_vap->uc_vap_id == pst_dmac_vap->st_vap_base_info.uc_vap_id)
        {
            continue;
        }

        /* wlan为STA模式下配置 */
        if ((WLAN_LEGACY_VAP_MODE != pst_tmp_vap->en_p2p_mode) || (WLAN_VAP_MODE_BSS_STA != pst_tmp_vap->en_vap_mode))
        {
            continue;
        }

        /* 5G下有用户才开启 */
        if ((WLAN_BAND_5G != pst_tmp_vap->st_channel.en_band) || (0 == pst_tmp_vap->us_user_nums))
        {
            continue;
        }

        /* VHT 40/80 */
        if ((WLAN_VHT_MODE == pst_tmp_vap->en_protocol || WLAN_VHT_ONLY_MODE == pst_tmp_vap->en_protocol)
        && (pst_tmp_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M))
        {

            pst_mac_user = mac_res_get_mac_user(pst_tmp_vap->us_assoc_vap_id);
            if (OAL_PTR_NULL == pst_mac_user)
            {
                OAM_WARNING_LOG1(pst_tmp_vap->uc_vap_id, OAM_SF_ANY, "{dmac_user_del::pst_mac_user[%d] null.}",
                    pst_tmp_vap->us_assoc_vap_id);
                continue;
            }

            switch(pst_mac_user->en_cur_bandwidth)
            {
                case WLAN_BW_CAP_20M:
                    pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_20M;
                    break;
                case WLAN_BW_CAP_40M:
                    pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_40M_DUP;
                    break;
                case WLAN_BW_CAP_80M:
                    pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_80M_DUP;
                    break;
                case WLAN_BW_CAP_160M:
                    pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_160M_DUP;
                    break;
                default:
                    pst_dmac_dev->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_20M;
                    break;
            }
            hal_cfg_rsp_dyn_bw(OAL_TRUE, pst_dmac_dev->en_usr_bw_mode);
            /* 默认使用6M响应帧速率，接收到数据后再行调整 */
            hal_set_rsp_rate(WLAN_PHY_RATE_6M);
            pst_dmac_dev->en_state_in_sw_ctrl_mode = OAL_TRUE;
        }
    }
}

 OAL_STATIC oal_void dmac_update_dyn_bw_info(dmac_vap_stru *pst_dmac_vap)
{
    dmac_device_stru *pst_dmac_dev;

    pst_dmac_dev = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_dev)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_update_dyn_bw_info::pst_dmac_dev null.dev id [%d]}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    if (OAL_TRUE == pst_dmac_dev->en_state_in_sw_ctrl_mode)
    {
        hal_cfg_rsp_dyn_bw(OAL_FALSE, WLAN_BAND_ASSEMBLE_20M);
        pst_dmac_dev->en_state_in_sw_ctrl_mode = OAL_FALSE;
    }
    else
    {
        /* p2p删用户时，如果之前有wlan业务，在只剩wlan时，再开启动态带宽 */
        if ((WLAN_P2P_GO_MODE == pst_dmac_vap->st_vap_base_info.en_p2p_mode)
            || (WLAN_P2P_CL_MODE == pst_dmac_vap->st_vap_base_info.en_p2p_mode))
        {
            dmac_user_del_p2p_in_dyn_bw(pst_dmac_dev, pst_dmac_vap);
        }
    }
}

#endif


OAL_STATIC oal_void dmac_alg_stat_info_reset(dmac_vap_stru *pst_dmac_vap)
{
    hal_to_dmac_device_stru     *pst_hal_device = OAL_PTR_NULL;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);

    pst_hal_device->st_hal_alg_stat.en_adj_intf_state    = HAL_ALG_INTF_DET_ADJINTF_NO;
    pst_hal_device->st_hal_alg_stat.en_co_intf_state     = OAL_FALSE;
    pst_hal_device->st_hal_alg_stat.en_alg_distance_stat = HAL_ALG_USER_DISTANCE_NORMAL;
}


oal_uint32  dmac_user_del(frw_event_mem_stru *pst_event_mem)
{
    mac_chip_stru                  *pst_mac_chip;
    mac_device_stru                *pst_mac_device;
    dmac_vap_stru                  *pst_dmac_vap;
    dmac_user_stru                 *pst_dmac_user;
    frw_event_stru                 *pst_event;
    dmac_ctx_del_user_stru         *pst_del_user_payload;
    oal_uint16                      us_user_idx;
    oal_uint32                      ul_rslt = OAL_FAIL;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_event_mem)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_user_del::pst_event_mem null.}");
        return OAL_FAIL;
    }
    pst_event = frw_get_event_stru(pst_event_mem);

    pst_del_user_payload = (dmac_ctx_del_user_stru *)pst_event->auc_event_data;
    us_user_idx          = pst_del_user_payload->us_user_idx;

    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_UM, "{dmac_user_del::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_chip = dmac_res_get_mac_chip(pst_mac_device->uc_chip_id);
    if (OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_UM, "{dmac_user_del::pst_mac_chip null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap   = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_UM, "{dmac_user_del::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 下发无效user idx，用mac地址查找user */
    if (MAC_INVALID_USER_ID == us_user_idx)
    {
        ul_rslt = mac_vap_find_user_by_macaddr(&(pst_dmac_vap->st_vap_base_info), pst_del_user_payload->auc_user_mac_addr, &us_user_idx);
        if (OAL_SUCC != ul_rslt)
        {
            OAM_ERROR_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_UM,"{dmac_user_del::Cannot find user by idx[%d],and mac_addr[%02x XX XX XX %02x %02x]!.}",
                        us_user_idx,
                        pst_del_user_payload->auc_user_mac_addr[0],
                        pst_del_user_payload->auc_user_mac_addr[4],
                        pst_del_user_payload->auc_user_mac_addr[5]);
            return OAL_ERR_CODE_PTR_NULL;
        }
    }

    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
            "{dmac_user_del::pst_dmac_user[%d] null.}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

    /* AP 侧硬件PMF控制开关填写 */
    dmac_11w_update_users_status(pst_dmac_vap, &pst_dmac_user->st_user_base_info, OAL_FALSE);
#endif /* #if(_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT) */

    /* 删除tid队列中的所有信息 */
    dmac_tid_clear(&(pst_dmac_user->st_user_base_info), pst_mac_device);
    dmac_tid_tx_queue_exit(pst_dmac_user);

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    /* 清空ip过滤的黑名单，目前仅支持staut模式(只有一个用户) */
    if (OAL_TRUE == pst_dmac_vap->st_vap_base_info.st_cap_flag.bit_ip_filter)
    {
        dmac_clear_ip_filter_btable(&(pst_dmac_vap->st_vap_base_info));
    }
#endif //_PRE_WLAN_FEATURE_IP_FILTER

    dmac_alg_stat_info_reset(pst_dmac_vap);
#ifdef _PRE_WLAN_FEATURE_CCA_OPT
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    {
        hal_cfg_custom_cca_stru *pst_cfg_cus_cca;
        oal_int8 c_delta_cca_ed_high_20th_default;
        oal_int8 c_delta_cca_ed_high_40th_default;

        hal_config_get_cus_cca_param(&pst_cfg_cus_cca);
        c_delta_cca_ed_high_20th_default = HAL_CCA_OPT_GET_DEFAULT_ED_20TH(pst_dmac_vap->st_vap_base_info.st_channel.en_band, pst_cfg_cus_cca);
        c_delta_cca_ed_high_40th_default = HAL_CCA_OPT_GET_DEFAULT_ED_40TH(pst_dmac_vap->st_vap_base_info.st_channel.en_band, pst_cfg_cus_cca);
        hal_set_ed_high_th(pst_dmac_vap->pst_hal_device, c_delta_cca_ed_high_20th_default, c_delta_cca_ed_high_40th_default, OAL_TRUE);
    }
#else
    hal_set_ed_high_th(pst_dmac_vap->pst_hal_device, HAL_CCA_OPT_ED_HIGH_20TH_DEF, HAL_CCA_OPT_ED_HIGH_40TH_DEF, OAL_TRUE);
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
    hal_pow_del_machw_resp_power_lut_entry(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif

    if (IS_LEGACY_STA(&(pst_dmac_vap->st_vap_base_info)))
    {
        /* 带宽切换状态机 */
        dmac_sta_bw_switch_fsm_post_event(pst_dmac_vap, DMAC_STA_BW_SWITCH_EVENT_USER_DEL, 0, OAL_PTR_NULL);
    }

    /* 如果是STA删除用户，表示此STA去关联了，调用vap down通知链 */
    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
        dmac_scan_destroy_obss_timer(pst_dmac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_VOWIFI
        /* 更新vowifi模式，同时初始化相关统计值 */
        if (WLAN_LEGACY_VAP_MODE == pst_dmac_vap->st_vap_base_info.en_p2p_mode)
        {
            dmac_vap_vowifi_init(pst_dmac_vap);
        }
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
        mac_vap_set_aid(&(pst_dmac_vap->st_vap_base_info), 0);
        dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_DEASSOCIATE, 0, OAL_PTR_NULL);
#endif

#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
        /* STA去关联时清空aid寄存器 */
        hal_set_mac_aid(pst_dmac_vap->pst_hal_vap, 0);
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        dmac_full_phy_freq_user_del(&(pst_dmac_vap->st_vap_base_info), pst_dmac_user);
#endif

        /*恢复STA为无保护状态*/
        dmac_set_protection_mode(&(pst_dmac_vap->st_vap_base_info), WLAN_PROT_NO);

#ifdef _PRE_WLAN_FEATURE_P2P
        
        if (IS_P2P_CL(&(pst_dmac_vap->st_vap_base_info)))
        {
            hal_vap_set_noa(pst_dmac_vap->pst_hal_vap, 0, 0, 0, 0);
            hal_vap_set_ops(pst_dmac_vap->pst_hal_vap, 0, 0);
            OAL_MEMZERO(&(pst_dmac_vap->st_p2p_ops_param), OAL_SIZEOF(mac_cfg_p2p_ops_param_stru));
            OAL_MEMZERO(&(pst_dmac_vap->st_p2p_noa_param), OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            dmac_psm_user_ps_structure_destroy(pst_dmac_user);
#endif
            pst_mac_device->st_p2p_info.en_p2p_ps_pause = OAL_FALSE;
        }
        
#endif
    }

    #ifdef _PRE_WLAN_FEATURE_UAPSD
    dmac_uapsd_user_destroy(pst_dmac_user);
    #endif

    /* 删除用户节能结构,清除vap保存的该用户的tim_bitmap信息 */
    if (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        dmac_psm_user_ps_structure_destroy(pst_dmac_user);
        dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);

        if (OAL_TRUE == pst_dmac_user->bit_ps_mode)
        {
            pst_dmac_vap->uc_ps_user_num--;
        }

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
        dmac_bsd_user_del_handle(pst_dmac_vap,pst_dmac_user);
#endif

#if defined(_PRE_WLAN_FEATURE_AP_PM) && (_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT)
        dmac_pcie_ps_switch(&pst_dmac_vap->st_vap_base_info, OAL_TRUE);
#endif
    }

#ifndef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
    hal_tx_disable_peer_sta_ps_ctrl(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif

#ifdef _PRE_WLAN_1103_PILOT
    hal_tx_disable_resp_ps_bit_ctrl(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif

    /* inactive，lut资源置为非法 */
    dmac_user_inactive(pst_dmac_user);

    /* dmac user相关操作去注册 */
    dmac_alg_del_assoc_user_notify(pst_dmac_vap, pst_dmac_user);

    /* 如果VAP模式是STA，则需要恢复STA寄存器到初始状态 */
    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
        if (mac_is_proxysta_enabled(pst_mac_device))
        {
            if (mac_vap_is_msta(&pst_dmac_vap->st_vap_base_info))
            {
                dmac_vap_sta_reset(pst_dmac_vap);
            }
            else
            {
                /* do nothing，不删除user */
            }
        }
        else
        {
            dmac_vap_sta_reset(pst_dmac_vap);
        }
#else
        dmac_vap_sta_reset(pst_dmac_vap);

        
        if (OAL_TRUE == dmac_sta_csa_is_in_waiting(&(pst_dmac_vap->st_vap_base_info)))
        {
            hal_set_machw_tx_resume(pst_dmac_vap->pst_hal_device);
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_UM,
                            "{dmac_user_del::csa_fsm back to init, resume MAC TX.}");
            dmac_sta_csa_fsm_post_event(&(pst_dmac_vap->st_vap_base_info), WLAN_STA_CSA_EVENT_TO_INIT, 0, OAL_PTR_NULL);
        }
        
#endif
    }

#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
    dmac_update_dyn_bw_info(pst_dmac_vap);
#endif

#ifdef _PRE_DEBUG_MODE_USER_TRACK
    /* 单用户跟踪删除定时器 */
    if (OAL_TRUE == pst_dmac_user->st_user_track_ctx.st_txrx_param_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_dmac_user->st_user_track_ctx.st_txrx_param_timer);
    }
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* 删除用户，更新SMPS能力 */
    mac_user_set_sm_power_save(&pst_dmac_user->st_user_base_info, 0);
#endif

    dmac_user_del_offload(&pst_dmac_vap->st_vap_base_info, us_user_idx);
#else
    /* 非offload模式下，device下用户数已经在hmac侧--，这里不需要再判断返回值做用户数--动作 */
    dmac_user_free(us_user_idx);
#endif

    /* 删除用户，更新hal device下用户数统计，做-- */
    hal_device_dec_assoc_user_nums(pst_dmac_vap->pst_hal_device);

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    dmac_config_btcoex_disassoc_state_syn(&(pst_dmac_vap->st_vap_base_info));
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    dmac_m2s_disassoc_state_syn(&(pst_dmac_vap->st_vap_base_info));
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    pst_mac_device->st_dataflow_brk_bypass.en_brk_limit_aggr_enable = OAL_FALSE;
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    dmac_compatibility_handler(pst_dmac_vap, pst_del_user_payload->en_ap_type, OAL_FALSE);
    OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_UM, "{dmac_user_del::Del user[%d] SUCC! vap user nums:%d,chip user nums:%d,hal device user nums:%d, user mac_addr:%02x:xx:xx:xx:%02x:%02x.}",
                    7,
                    us_user_idx,
                    pst_dmac_vap->st_vap_base_info.us_user_nums,
                    pst_mac_chip->uc_assoc_user_cnt,
                    pst_dmac_vap->pst_hal_device->uc_assoc_user_nums,
                    pst_del_user_payload->auc_user_mac_addr[0],
                    pst_del_user_payload->auc_user_mac_addr[4],
                    pst_del_user_payload->auc_user_mac_addr[5]);
#else
    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_UM, "{dmac_user_del::Del user[%d] SUCC! vap user nums:%d,chip user nums:%d, user mac_addr:xx:xx:xx:xx:xx:%02x.}",
                    us_user_idx,
                    pst_dmac_vap->st_vap_base_info.us_user_nums,
                    pst_mac_chip->uc_assoc_user_cnt,
                    pst_del_user_payload->auc_user_mac_addr[5]);
#endif

#ifdef _PRE_WLAN_PRODUCT_1151V200
    if ((WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode)
        && (pst_dmac_vap->st_vap_base_info.st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)
        && (OAL_FALSE == pst_dmac_vap->st_vap_base_info.uc_1st_asoc_done)
        && (OAL_TRUE == pst_dmac_vap->st_vap_base_info.uc_force_resp_mode_80M))
    {
        hal_set_80m_resp_mode(pst_dmac_vap->pst_hal_device, OAL_FALSE);
        pst_dmac_vap->st_vap_base_info.uc_1st_asoc_done = OAL_TRUE;

        OAM_WARNING_LOG0(0, OAM_SF_CFG, "dmac_user_del:  1st assoc end!");
    }
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    if (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        /* 注销统计节点 */
        dmac_stat_qos_exit_user(pst_dmac_user);
        /* 将STA从qos链表中删除 */
        dmac_tx_delete_qos_enhance_sta(&pst_dmac_vap->st_vap_base_info, pst_del_user_payload->auc_user_mac_addr, OAL_FALSE);
    }
#endif

#ifdef _PRE_WLAN_11K_STAT
    dmac_stat_exit_user(pst_dmac_user);
#endif
/* 释放11v申请的内存 */
#ifdef _PRE_WLAN_FEATURE_11V
    if(OAL_PTR_NULL != pst_dmac_user->pst_11v_ctrl_info)
    {
        OAL_MEM_FREE(pst_dmac_user->pst_11v_ctrl_info, OAL_TRUE);
        pst_dmac_user->pst_11v_ctrl_info = OAL_PTR_NULL;
        #ifdef _PRE_DEBUG_MODE
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_user_del:: pst_11v_ctrl_info free done.}");
        #endif
    }

#endif

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_HILINK

oal_void dmac_user_notify_best_rate(dmac_user_stru *pst_dmac_user, oal_uint32 ul_best_rate_kbps)
{
    /* 更新最小发送速率 */
    if (pst_dmac_user->ul_tx_minrate > 0)
    {
        pst_dmac_user->ul_tx_minrate = OAL_MIN(pst_dmac_user->ul_tx_minrate, ul_best_rate_kbps);
    }
    else
    {
        pst_dmac_user->ul_tx_minrate = ul_best_rate_kbps;
    }

    /* 更新最大发送速率 */
    if (pst_dmac_user->ul_tx_maxrate > 0)
    {
        pst_dmac_user->ul_tx_maxrate = OAL_MAX(pst_dmac_user->ul_tx_maxrate, ul_best_rate_kbps);
    }
    else
    {
        pst_dmac_user->ul_tx_maxrate = ul_best_rate_kbps;
    }
}
#endif

oal_uint32 dmac_user_get_smartant_training_state(
                mac_user_stru                          *pst_user,
                dmac_user_smartant_training_enum_uint8 *pen_training_state)
{
    dmac_user_stru     *pst_dmac_user;

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    *pen_training_state = pst_dmac_user->en_smartant_training;

    return OAL_SUCC;
}

oal_uint32  dmac_user_set_smartant_training_state(
                mac_user_stru                              *pst_user,
                dmac_user_smartant_training_enum_uint8      en_training_state)
{
    dmac_user_stru     *pst_dmac_user;

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    pst_dmac_user->en_smartant_training = en_training_state;

    return OAL_SUCC;
}

oal_uint32  dmac_user_get_smartant_normal_rate_stats(
                mac_user_stru                      *pst_mac_user,
                dmac_tx_normal_rate_stats_stru    **ppst_rate_stats_info)
{

    dmac_user_stru     *pst_dmac_user;

    pst_dmac_user = MAC_GET_DMAC_USER(pst_mac_user);

    *ppst_rate_stats_info = &(pst_dmac_user->st_smartant_rate_stats);

    return OAL_SUCC;
}

oal_bool_enum_uint8  dmac_user_get_vip_flag(OAL_CONST mac_user_stru * OAL_CONST pst_user)
{
    dmac_user_stru                 *pst_dmac_user;

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    return pst_dmac_user->en_vip_flag;
}

oal_uint32  dmac_user_set_vip_flag(
                mac_user_stru               *pst_user,
                oal_bool_enum_uint8          en_vip_flag)
{
    dmac_user_stru                 *pst_dmac_user;

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    pst_dmac_user->en_vip_flag = en_vip_flag;

    return OAL_SUCC;
}


/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_alg_distance_notify_hook);
oal_module_symbol(dmac_alg_co_intf_notify_hook);
oal_module_symbol(dmac_alg_intf_det_notify_hook);
#ifdef _PRE_WLAN_FEATURE_HILINK
oal_module_symbol(dmac_user_notify_best_rate);
#endif
oal_module_symbol(dmac_user_get_smartant_training_state);
oal_module_symbol(dmac_user_set_smartant_training_state);
oal_module_symbol(dmac_user_get_smartant_normal_rate_stats);
oal_module_symbol(dmac_user_get_vip_flag);
oal_module_symbol(dmac_user_set_vip_flag);


/*lint +e578*//*lint +e19*/



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

