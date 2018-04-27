


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oam_ext_if.h"
#include "hal_ext_if.h"
#include "mac_resource.h"
#include "mac_device.h"
#include "dmac_txopps.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TXOPPS_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint32  dmac_txopps_set_machw_en_sta(
                                    mac_vap_stru *pst_mac_vap,
                                    mac_txopps_machw_param_stru *pst_txopps_machw_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == pst_txopps_machw_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_TXOP,
                       "{dmac_txopps_set_machw_en_sta::param is null,vap=[%d],machw_param=[%d]}.",
                       pst_mac_vap, pst_txopps_machw_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_COEX, "{dmac_txopps_set_machw_en_sta::pst_hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_set_txop_ps_enable(pst_hal_device, pst_txopps_machw_param->en_machw_txopps_en);
    hal_set_txop_ps_condition1(pst_hal_device, pst_txopps_machw_param->en_machw_txopps_condition1);
    hal_set_txop_ps_condition2(pst_hal_device, pst_txopps_machw_param->en_machw_txopps_condition2);

    return OAL_SUCC;
}


oal_uint32  dmac_txopps_set_machw(mac_vap_stru *pst_mac_vap)
{
    mac_txopps_machw_param_stru       st_txopps_machw_param;

    if (!IS_LEGACY_STA(pst_mac_vap))
    {
        return OAL_SUCC;
    }

    if (OAL_FALSE == mac_vap_get_txopps(pst_mac_vap))
    {
        st_txopps_machw_param.en_machw_txopps_en = 0;
        st_txopps_machw_param.en_machw_txopps_condition1 = 0;
        st_txopps_machw_param.en_machw_txopps_condition2 = 0;
    }
    else
    {
        st_txopps_machw_param.en_machw_txopps_en = 1;
        st_txopps_machw_param.en_machw_txopps_condition1 = 1;
        st_txopps_machw_param.en_machw_txopps_condition2 = 1;
    }

    dmac_txopps_set_machw_en_sta(pst_mac_vap, &st_txopps_machw_param);

    return OAL_SUCC;
}



oal_uint32 dmac_config_set_txop_ps_machw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_txopps_machw_param_stru *pst_txopps;

    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG1(0, OAM_SF_TXOP, "{dmac_config_set_txop_ps_machw::null poiter[%p]}.", pst_mac_vap);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果不是11ac的，不涉及txop ps，直接返回 */
    if ((WLAN_VHT_MODE != pst_mac_vap->en_protocol) && (WLAN_VHT_ONLY_MODE != pst_mac_vap->en_protocol))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TXOP,
                         "{dmac_config_set_txop_ps_machw::vap is not 11ac, mode[%d].}", pst_mac_vap->en_protocol);
        return OAL_SUCC;
    }

    pst_txopps = (mac_txopps_machw_param_stru *)puc_param;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TXOP,"{dmac_config_set_txop_ps_machw::txopps enable[%d]}.", pst_txopps->en_machw_txopps_en);

    mac_mib_set_txopps(pst_mac_vap, pst_txopps->en_machw_txopps_en);

    if (WLAN_VAP_MODE_BSS_STA != pst_mac_vap->en_vap_mode)
    {
        /* aput 不需要填写寄存器 */
        return OAL_SUCC;
    }

    return dmac_txopps_set_machw_en_sta(pst_mac_vap, pst_txopps);

}



oal_uint32  dmac_txopps_set_machw_partialaid(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                           ul_partial_aid;
    mac_cfg_txop_sta_stru               *pst_txop_info;
    dmac_vap_stru                       *pst_dmac_vap;

    if (OAL_PTR_NULL == puc_param || OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_TXOP, "{dmac_txopps_set_machw_partialaid_sta::INPUT NULL PTR[%p].}", pst_mac_vap);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 只有STA需要设置TXOPPS */
    if (!IS_LEGACY_STA(pst_mac_vap))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TXOP,"{dmac_txopps_set_machw_partialaid_sta::NOT STA[%d].", pst_mac_vap->en_vap_mode);
        return OAL_SUCC;
    }

    pst_txop_info = (mac_cfg_txop_sta_stru *)puc_param;
    ul_partial_aid = pst_txop_info->us_partial_aid;

    /* 转化为DMAC VAP */
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG1(0, OAM_SF_TXOP, "{dmac_txopps_set_machw_partialaid_sta::dmac_vap[0x%p]}",pst_dmac_vap);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TXOP,"{dmac_txopps_set_machw_partialaid_sta::partial aid[%d]}.", ul_partial_aid);

    hal_set_txop_ps_partial_aid(pst_dmac_vap->pst_hal_vap, ul_partial_aid);

    return OAL_SUCC;
}


#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

