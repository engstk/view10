


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

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#include "hisi_customize_wifi.h"
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

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_USER_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/

void*  mac_res_get_dmac_user(oal_uint16 us_idx)
{
    return  mac_res_get_mac_user(us_idx);
}


void*  mac_res_get_dmac_user_alloc(oal_uint16 us_idx)
{
    mac_user_stru*  pst_mac_user;

    pst_mac_user = (mac_user_stru*)_mac_res_get_mac_user(us_idx);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{mac_res_get_dmac_user_alloc::pst_mac_user null,user_idx=%d.}", us_idx);
        return OAL_PTR_NULL;
    }

    /* 非offload模式，在hmac侧已经ALLOCED判断，此处不涉及 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 重复申请异常,避免影响业务，暂时打印error但正常申请 */
    if (MAC_USER_ALLOCED == pst_mac_user->uc_is_user_alloced)
    {
        OAM_ERROR_LOG1(0, OAM_SF_UM, "{mac_res_get_dmac_user_alloc::[E]user has been alloced,user_idx=%d.}", us_idx);
    }
#endif

    /* mac_user_stru是dmac_user_stru首元素，可强转 */
    return  (void*)pst_mac_user;
}


oal_uint32 dmac_user_alloc(oal_uint16 us_user_idx)
{
    oal_uint32        ul_ret = 0;
    dmac_user_stru *  pst_dmac_user;

    /* 申请dmac user */
    ul_ret = mac_res_alloc_dmac_user(us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_user_alloc::dmac_user_alloc failed[%d] userindx[%d].", ul_ret, us_user_idx);
        return ul_ret;
    }

    /* 获取dmac user */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user_alloc(us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_user_alloc::pst_dmac_user null, userindx[%d].}", us_user_idx);
        mac_res_free_mac_user(us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 非offload模式，mac user标记都在hmac侧执行 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 初始清0 */
    OAL_MEMZERO(&(pst_dmac_user->st_user_base_info), OAL_SIZEOF(mac_user_stru));

    /* 设置alloc标志 */
    pst_dmac_user->st_user_base_info.uc_is_user_alloced = MAC_USER_ALLOCED;
#endif

    return OAL_SUCC;
}


oal_uint32 dmac_user_free(oal_uint16 us_user_idx)
{
    oal_uint32       ul_ret = OAL_SUCC;
    dmac_user_stru*  pst_dmac_user;

    pst_dmac_user = (dmac_user_stru*)mac_res_get_dmac_user(us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_user_free::pst_dmac_user[%d] null.}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

#if 0 //重复释放属于问题，会在前面直接打印ERROR，然后返回空指针，需要进行定位。
    /* 重复释放异常, 继续释放不返回 */
    if (MAC_USER_FREED == pst_dmac_user->st_user_base_info.uc_is_user_alloced)
    {
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
        /*lint -e718*//*lint -e746*/
        OAM_WARNING_LOG2(0, OAM_SF_UM, "{dmac_user_free::[E]user has been freed,user_idx=%d, func[%x].}", us_user_idx, (oal_uint32)__return_address());
        /*lint +e718*//*lint +e746*/
#else
        OAM_WARNING_LOG1(0, OAM_SF_UM, "{dmac_user_free::[E]user has been freed,user_idx=%d.}", us_user_idx);
#endif
    }
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    ul_ret = mac_res_free_mac_user(us_user_idx);
    if(OAL_SUCC == ul_ret)
    {
        /* 清除alloc标志 */
        pst_dmac_user->st_user_base_info.uc_is_user_alloced = MAC_USER_FREED;
    }
#endif

    return ul_ret;
}


oal_uint32  dmac_user_get_tid_by_num(OAL_CONST mac_user_stru * OAL_CONST pst_mac_user, oal_uint8 uc_tid_num, dmac_tid_stru **ppst_tid_queue)
{
    *ppst_tid_queue = &(MAC_GET_DMAC_USER(pst_mac_user)->ast_tx_tid_queue[uc_tid_num]);

    return OAL_SUCC;
}
#if 0

oal_uint32  dmac_user_get_txchain_mask(
                mac_user_stru          *pst_user,
                wlan_nss_enum_uint8     en_nss,
                oal_uint8              *puc_chainmask)
{
    dmac_user_stru                 *pst_dmac_user;

    if (en_nss >= WLAN_NSS_LIMIT)
    {
        OAM_ERROR_LOG1(pst_user->uc_vap_id, OAM_SF_TX_CHAIN, "{dmac_user_get_txchain_mask::invalid en_nss[%d].", en_nss);

        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(pst_user->uc_vap_id, OAM_SF_TX_CHAIN, "{dmac_user_get_txchain_mask::pst_dmac_user null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    *puc_chainmask = pst_dmac_user->auc_txchain_mask[en_nss];

    return OAL_SUCC;
}


oal_uint32  dmac_user_set_txchain_mask(
                mac_user_stru          *pst_user,
                wlan_nss_enum_uint8     en_nss,
                oal_uint8               uc_chainmask)
{
    dmac_user_stru                 *pst_dmac_user;

    if (en_nss >= WLAN_NSS_LIMIT)
    {
        OAM_ERROR_LOG1(pst_user->uc_vap_id, OAM_SF_TX_CHAIN, "{dmac_user_set_txchain_mask::invalid en_nss[%d].", en_nss);

        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(pst_user->uc_vap_id, OAM_SF_TX_CHAIN, "{dmac_user_set_txchain_mask::pst_dmac_user null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_user->auc_txchain_mask[en_nss] = uc_chainmask;

    return OAL_SUCC;
}
#endif


oal_bool_enum_uint8  dmac_user_get_ps_mode(mac_user_stru  *pst_user)
{
    dmac_user_stru                 *pst_dmac_user;

    pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

    return (oal_bool_enum_uint8)(pst_dmac_user->bit_ps_mode);
}

oal_void dmac_user_init_slottime(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    hal_to_dmac_device_stru    *pst_hal_device;

    pst_hal_device = dmac_user_get_hal_device(pst_mac_user);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{dmac_user_init_slottime::hal dev null, fail to update slot time, chip[%d],mac_dev[%d].}",
                    pst_mac_vap->uc_chip_id,
                    pst_mac_vap->uc_device_id);
        return;
    }

    if (WLAN_LEGACY_11B_MODE == pst_mac_user->en_cur_protocol_mode)
    {
        hal_cfg_slottime_type(pst_hal_device, 1);
    }
    else
    {
        hal_cfg_slottime_type(pst_hal_device, 0);
    }
}

oal_uint32 dmac_psm_tid_mpdu_num(dmac_user_stru  *pst_dmac_user)
{
    oal_uint8                     uc_tid_idx         = 0;
    oal_uint32                    ul_tid_mpdu_num    = 0;

    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx ++)
    {
        ul_tid_mpdu_num += pst_dmac_user->ast_tx_tid_queue[uc_tid_idx].us_mpdu_num;
    }

    return ul_tid_mpdu_num;
}

oal_bool_enum_uint8 dmac_psm_is_psm_empty(dmac_user_stru *pst_dmac_user)
{
    return (0 == oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num));
}


oal_bool_enum_uint8 dmac_psm_is_uapsd_empty(dmac_user_stru  *pst_dmac_user)
{
    return (0 == oal_atomic_read(&pst_dmac_user->st_uapsd_stru.uc_mpdu_num));
}


oal_bool_enum_uint8 dmac_psm_is_tid_empty(dmac_user_stru  *pst_dmac_user)
{
    return (0 == dmac_psm_tid_mpdu_num(pst_dmac_user));
}
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#ifdef _PRE_WLAN_PHY_BUGFIX_IMPROVE_CE_TH
oal_void dmac_rx_compatibility_show_stat(dmac_user_stru *pst_dmac_user)
{
    dmac_device_stru        *pst_dmac_device;
    hal_to_dmac_device_stru   *pst_hal_device = OAL_PTR_NULL;

    pst_hal_device = dmac_user_get_hal_device(&pst_dmac_user->st_user_base_info);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_RX, "dmac_rx_compatibility_show_stat::pst_hal_device ptr null.");
        return;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_user->st_user_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_device))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_rx_compatibility_show_stat:: pst_dmac_device null, user maybe deleted. device_id[%d]}",
                                 pst_dmac_user->st_user_base_info.uc_device_id);
        return;
    }
    OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_rx_compatibility_show_stat::%d(0:normal,1:compatibity),txopenb:%d,tx_ba_num:%d.}",
                    pst_hal_device->st_hal_compatibility_stat.en_compatibility_stat,
                    pst_dmac_device->pst_device_base_info->en_txop_enable,
                    pst_dmac_device->pst_device_base_info->uc_tx_ba_num);
    OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_rx_compatibility_show_stat::distance:%d,cca_intr:%d,co_intr:%d.}",
                    pst_hal_device->st_hal_alg_stat.en_alg_distance_stat,
                    pst_hal_device->st_hal_alg_stat.en_adj_intf_state,
                    pst_hal_device->st_hal_alg_stat.en_co_intf_state);

    return;
}
#endif
#endif
#ifdef _PRE_WLAN_DFT_EVENT

oal_void  dmac_user_status_change_to_sdt(
                                       dmac_user_stru       *pst_dmac_user,
                                       oal_bool_enum_uint8   en_is_user_paused )
{
    oal_uint8       auc_event[50] = {0};
    auc_event[0] = en_is_user_paused;

    DMAC_EVENT_PAUSE_OR_RESUME_USER(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                                    pst_dmac_user->st_user_base_info.uc_vap_id,
                                    OAM_EVENT_PAUSE_OR_RESUME_USER,
                                    auc_event);
}
#endif


oal_uint32  dmac_user_pause(dmac_user_stru *pst_dmac_user)
{
    oal_uint8       uc_tid_idx;
    dmac_tid_stru  *pst_tid;
    oal_uint32      ul_ret;

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_user_pause::pst_dmac_user null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    if (MAC_USER_STATE_ASSOC != pst_dmac_user->st_user_base_info.en_user_asoc_state)
    {
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_DFT_EVENT
    dmac_user_status_change_to_sdt(pst_dmac_user, OAL_TRUE);
#endif

    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx++)
    {
        pst_tid = &(pst_dmac_user->ast_tx_tid_queue[uc_tid_idx]);

        ul_ret = dmac_tid_pause(pst_tid, DMAC_TID_PAUSE_RESUME_TYPE_PS);
        if (OAL_SUCC != ul_ret)
        {
            return ul_ret;
        }
    }

    return OAL_SUCC;
}

oal_uint32  dmac_user_set_groupid_partial_aid(mac_vap_stru  *pst_mac_vap,
                                                      dmac_user_stru *pst_dmac_user)
{
    oal_uint16     us_temp_partial_aid;
    oal_uint16     us_temp_aid;
    oal_uint8      uc_temp_bssid;


    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        pst_dmac_user->uc_groupid     = 0;

        /* 取BSSID[39:47] */
        us_temp_partial_aid = OAL_MAKE_WORD16(pst_mac_vap->auc_bssid[4], pst_mac_vap->auc_bssid[5]);

        /* 把bssid中48个bit的高9bit对应的10进制值作为paid,STA时给txbf ndp使用 */
        pst_dmac_user->us_partial_aid = (us_temp_partial_aid & 0xFF80) >> 7;

    }
    else if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        /* AP保存对端user的属性，发送txop使用 */
        pst_dmac_user->uc_groupid     = 63;

        us_temp_aid   = pst_dmac_user->st_user_base_info.us_assoc_id & 0x1FF;
        uc_temp_bssid = (pst_mac_vap->auc_bssid[5] & 0x0F) ^ ((pst_mac_vap->auc_bssid[5] & 0xF0) >> 4);
        pst_dmac_user->us_partial_aid = (us_temp_aid + (uc_temp_bssid << 5) ) & ((1 << 9) -1);
    }
    else
    {
        pst_dmac_user->uc_groupid     = 63;
        pst_dmac_user->us_partial_aid = 0;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_SMPS

oal_uint8 dmac_user_get_smps_mode(mac_vap_stru  *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    wlan_mib_mimo_power_save_enum_uint8 en_vap_smps;
    wlan_mib_mimo_power_save_enum_uint8 en_user_smps;

    if (OAL_TRUE != pst_mac_user->st_ht_hdl.en_ht_capable)
    {
       return WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    }

    en_vap_smps  = mac_mib_get_smps(pst_mac_vap);
    en_user_smps = (wlan_mib_mimo_power_save_enum_uint8)pst_mac_user->st_ht_hdl.bit_sm_power_save;

    return (oal_uint8)((en_vap_smps >= en_user_smps)? en_user_smps : en_vap_smps);
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_void dmac_ap_pause_all_user(mac_vap_stru *pst_mac_vap)
{
    oal_dlist_head_stru                  *pst_entry;
    mac_user_stru                        *pst_user_tmp;
    dmac_user_stru                       *pst_dmac_user_tmp;
	mac_device_stru                      *pst_mac_device;
    hal_to_dmac_device_stru              *pst_hal_device;
#ifdef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
    dmac_vap_stru                        *pst_dmac_vap;
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id,OAM_SF_ANY,"{dmac_ap_pause_all_user::mac_res_get_dmac_vap fail or pst_dmac_vap->pst_hal_vap NULL}");
        return;
    }
#endif

    pst_mac_device  = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_ap_pause_all_user::pst_device_base_info[%d] null!}", pst_mac_vap->uc_device_id);
        return;
    }

    pst_hal_device  = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ap_pause_all_user::pst_device_base_info[%d] null!}");
        return;
    }

    /* 遍历vap下所有用户,pause tid 队列 */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_user_tmp      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        /*lint -save -e774 */
        if (OAL_PTR_NULL == pst_user_tmp)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY,"{dmac_ap_pause_all_user::pst_user_tmp null.");
            continue;
        }
        /*lint -restore */

        pst_dmac_user_tmp = MAC_GET_DMAC_USER(pst_user_tmp);

        /* pause tid */
        dmac_user_pause(pst_dmac_user_tmp);

#ifdef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
        /*suspend硬件队列*/
        hal_set_machw_tx_suspend(pst_hal_device);
        /* 遍历硬件队列，将属于该用户的帧都放回tid */
        dmac_psm_flush_txq_to_tid(pst_mac_device, pst_dmac_vap, pst_dmac_user_tmp);
        /* 恢复硬件队列 */
        hal_set_machw_tx_resume(pst_hal_device);
#else
        /* 暂停该vap下的所有用户的硬件队列的发送, 硬件上报psm_back,软件再回收 */
        hal_tx_enable_peer_sta_ps_ctrl(pst_hal_device, pst_dmac_user_tmp->uc_lut_index);
#endif
    }
}
#endif


dmac_user_stru  *mac_vap_get_dmac_user_by_addr(mac_vap_stru *pst_mac_vap, oal_uint8  *puc_mac_addr)
{
    oal_uint32              ul_ret;
    oal_uint16              us_user_idx   = 0xffff;
    dmac_user_stru         *pst_dmac_user = OAL_PTR_NULL;

    /*根据mac addr找sta索引*/
    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_mac_addr, &us_user_idx);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_vap_get_dmac_user_by_addr::find_user_by_macaddr failed[%d].}", ul_ret);
        if (OAL_PTR_NULL != puc_mac_addr)
        {
            OAM_WARNING_LOG3(0, OAM_SF_ANY,"{mac_vap_get_dmac_user_by_addr:: mac_addr[%02x XX XX XX %02x %02x]!.}",
                                puc_mac_addr[0], puc_mac_addr[4], puc_mac_addr[5]);
        }
        return OAL_PTR_NULL;
    }

    /*根据sta索引找到user内存区域*/
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_vap_get_dmac_user_by_addr::user[%d] ptr null.}", us_user_idx);
    }
    return pst_dmac_user;
}

#ifdef _PRE_WLAN_FEATURE_HILINK_TEMP_PROTECT


oal_uint32 dmac_user_notify_all_sta_rssi(oal_void *ptr_null)
{
    oal_uint8           uc_device_num;
    oal_uint8           uc_chip_num;
    oal_int16           us_user_idx;
    oal_int8            c_rssi[MAC_RES_MAX_USER_LIMIT];
    dmac_user_stru     *pst_dmac_user;

    for (us_user_idx = 0; us_user_idx < MAC_RES_MAX_USER_LIMIT; us_user_idx++)
    {
        pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
        if (OAL_PTR_NULL == pst_dmac_user)
        {
            c_rssi[us_user_idx] = -127;   /** 使用-127设置当默认值 */
        }
        else
        {
            c_rssi[us_user_idx] = oal_get_real_rssi(pst_dmac_user->s_rx_rssi);
        }
    }

    for ((uc_chip_num) = 0; (uc_chip_num) < WLAN_CHIP_MAX_NUM_PER_BOARD; (uc_chip_num)++)
    {
        for ((uc_device_num) = 0; (uc_device_num) < WLAN_DEVICE_MAX_NUM_PER_CHIP; (uc_device_num)++)
        {
            mac_device_stru          *pst_mac_device;
            hal_to_dmac_device_stru  *pst_hal_device;
            oal_uint32                ul_ret;
            mac_vap_stru             *pst_mac_vap;

            ul_ret = hal_chip_get_hal_device(uc_chip_num, uc_device_num, &pst_hal_device);
            if ((OAL_SUCC != ul_ret) || (OAL_PTR_NULL == pst_hal_device))
            {
                continue;
            }
            pst_mac_device  = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
            if (NULL == pst_mac_device)
            {
                continue;
            }

            ul_ret = mac_device_find_up_ap(pst_mac_device, &pst_mac_vap);
            if (ul_ret == OAL_SUCC)
            {
                dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_NOTIFY_STA_RSSI, sizeof(c_rssi), (oal_uint8 *)c_rssi);
                return OAL_SUCC;
            }
        }
    }
    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT

oal_uint32 dmac_user_update_all_sta_tx_bytes(oal_void *ptr_null)
{
#define TID_STAT_TO_USER(_stat) (((_stat)[0])+((_stat)[1])+((_stat)[2])+((_stat)[3])+((_stat)[4])+((_stat)[5])+((_stat)[6])+((_stat)[7]))

    oal_uint32                           ul_loop;
    oam_stat_info_stru                  *pst_oam_stat;
    oam_user_stat_info_stru             *pst_oam_user_stat;

    pst_oam_stat = OAM_STAT_GET_STAT_ALL();
    for (ul_loop = 0; ul_loop < WLAN_USER_MAX_USER_LIMIT; ul_loop++)
    {
        pst_oam_user_stat = &pst_oam_stat->ast_user_stat_info[ul_loop];
        pst_oam_user_stat->ul_last2_tx_bytes = pst_oam_user_stat->ul_last_tx_bytes;
        pst_oam_user_stat->ul_last_tx_bytes = pst_oam_user_stat->ul_curr_tx_bytes;
        pst_oam_user_stat->ul_curr_tx_bytes = TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_mpdu_bytes)+TID_STAT_TO_USER(pst_oam_user_stat->aul_tx_ampdu_bytes);
    }
#undef TID_STAT_TO_USER
    return OAL_SUCC;
}

#endif

#if defined(_PRE_WLAN_FEATURE_HILINK_TEMP_PROTECT) || defined(_PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT)

oal_uint32 dmac_user_pub_timer_callback_func(oal_void *ptr_null)
{
#ifdef _PRE_WLAN_FEATURE_HILINK_TEMP_PROTECT
    dmac_user_notify_all_sta_rssi(ptr_null);
#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    dmac_user_update_all_sta_tx_bytes(ptr_null);
#endif
    return OAL_SUCC;
}
#endif

/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_user_get_tid_by_num);
oal_module_symbol(dmac_user_get_ps_mode);
oal_module_symbol(mac_res_get_dmac_user);
oal_module_symbol(mac_vap_get_dmac_user_by_addr);

#ifdef _PRE_WLAN_FEATURE_SMPS
oal_module_symbol(dmac_user_get_smps_mode);
#endif

/*lint +e578*//*lint +e19*/



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


