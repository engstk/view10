

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_STA_UAPSD

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_uapsd_sta.h"
#include "dmac_psm_sta.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_UAPSD_STA_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint8 dmac_get_highest_trigger_enabled_priority(dmac_vap_stru *pst_dmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    oal_uint8 uc_tid = WLAN_TID_MAX_NUM;

    /* Check for highest priority trigger enabled AC */
    if(dmac_is_trigger_enabled(pst_dmac_vap, WLAN_TIDNO_VOICE) == OAL_TRUE)
    {
        uc_tid = WLAN_TIDNO_VOICE;
    }
    else if(dmac_is_trigger_enabled(pst_dmac_vap, WLAN_TIDNO_VIDEO) == OAL_TRUE)
    {
        uc_tid = WLAN_TIDNO_VIDEO;
    }
    else if(dmac_is_trigger_enabled(pst_dmac_vap, WLAN_TIDNO_BEST_EFFORT) == OAL_TRUE)
    {
        uc_tid = WLAN_TIDNO_BEST_EFFORT;
    }
    else if (dmac_is_trigger_enabled(pst_dmac_vap, WLAN_TIDNO_BACKGROUND) == OAL_TRUE)
    {
        uc_tid = WLAN_TIDNO_BACKGROUND;
    }

    return uc_tid;
#else
    return WLAN_TIDNO_VOICE;
#endif /* _PRE_FEATURE_WLAN_STA_UAPSD */
}


oal_uint8 dmac_uapsd_tx_process_data_sta(dmac_vap_stru *pst_dmac_vap, mac_tx_ctl_stru *pst_tx_ctl)
{
    oal_uint8                 uc_tid;
    mac_sta_pm_handler_stru  *pst_mac_sta_pm_handle;

    pst_mac_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;

    uc_tid = MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl);

    /* If the AP is UAPSD capable and the AC for given priority is trigger   */
    /* enabled the UAPSD service period is started. Note that the frame      */
    /* transmission status needs to be checked before actually starting the  */
    /* service period. This is done during transmit complete processing.     */
    /* The power save state is STA_DOZE so that the AP continues to buffer   */
    /* packets for this STA                                                  */
    if(OAL_TRUE == dmac_is_uapsd_capable(pst_dmac_vap))
    {
        if(OAL_TRUE == dmac_is_trigger_enabled(pst_dmac_vap, uc_tid))
        {
            dmac_wait_uapsd_sp_start(pst_mac_sta_pm_handle);
            return STA_PWR_SAVE_STATE_DOZE;
        }

        else if(OAL_TRUE == dmac_is_delivery_enabled(pst_dmac_vap, uc_tid))
        {
            /* Exception case. Should not occur. If it is an only delivery   */
            /* enabled AC there should be no uplink traffic.                 */
    		DMAC_STA_UAPSD_STATS_INCR(pst_mac_sta_pm_handle->st_wmmps_info.ul_wmmpssta_dacul);
        }
    }

    return STA_PWR_SAVE_STATE_ACTIVE;
}


oal_uint8 dmac_is_uapsd_capable(dmac_vap_stru *pst_dmac_vap)
{
    /* ap支持并且sta支持才进uapsd的处理逻辑 */
    if ((pst_dmac_vap->st_vap_base_info.uc_uapsd_cap) && (pst_dmac_vap->st_vap_base_info.st_sta_uapsd_cfg.uc_max_sp_len != 0))
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

oal_uint8 dmac_is_trigger_enabled(dmac_vap_stru *pst_dmac_vap, oal_uint8  uc_tid)
{
    oal_uint8                uc_ac;
    mac_cfg_uapsd_sta_stru   st_uapsd_cfg_sta;

    uc_ac = WLAN_WME_TID_TO_AC(uc_tid);
    st_uapsd_cfg_sta = pst_dmac_vap->st_vap_base_info.st_sta_uapsd_cfg;
    return st_uapsd_cfg_sta.uc_trigger_enabled[uc_ac];
}

#endif /* _PRE_WLAN_FEATURE_STA_UAPSD */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


