
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
#if defined(_PRE_WLAN_CHIP_TEST) && defined(_PRE_SUPPORT_ACS) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "dmac_acs.h"
#endif
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

#include "dmac_power.h"

#include "dmac_mgmt_bss_comm.h"
#include "dmac_csa_sta.h"
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_SCAN_ROM_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_scan_rom_cb g_st_dmac_scan_rom_cb = {OAL_PTR_NULL,
                                          OAL_PTR_NULL};

/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void  dmac_scan_get_ch_statics_measurement_result(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device)
{
    hal_ch_statics_irq_event_stru    st_stats_result;
    oal_uint8                        uc_chan_idx;
    oal_uint32                       ul_trx_time_us = 0;

    /* 读取结果 */
    OAL_MEMZERO(&st_stats_result, OAL_SIZEOF(st_stats_result));
    hal_get_ch_statics_result(pst_hal_device, &st_stats_result);
    hal_get_ch_measurement_result(pst_hal_device, &st_stats_result);

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

    /* 复制信道统计/测量结果信息 */
    pst_hal_device->st_chan_result.uc_stats_valid = 1;
    pst_hal_device->st_chan_result.uc_channel_number = pst_mac_device->st_scan_params.ast_channel_list[uc_chan_idx].uc_chan_number;

    if (st_stats_result.c_pri20_idle_power < 0)
    {
        pst_hal_device->st_chan_result.s_free_power_stats_20M  += (oal_int8)st_stats_result.c_pri20_idle_power; /* 主20M信道空闲功率 */
        pst_hal_device->st_chan_result.s_free_power_stats_40M  += (oal_int8)st_stats_result.c_pri40_idle_power; /* 主40M信道空闲功率 */
        pst_hal_device->st_chan_result.s_free_power_stats_80M  += (oal_int8)st_stats_result.c_pri80_idle_power; /* 全部80M信道空闲功率 */
        pst_hal_device->st_chan_result.uc_free_power_cnt += 1;
    }
    //else
    //{
    //    OAM_WARNING_LOG1(0, OAM_SF_SCAN, "{dmac_scan_get_ch_statics_measurement_result::c_pri20_idle_power=%u}", (oal_uint8)st_stats_result.c_pri20_idle_power);
    //}

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

    if (g_st_dmac_scan_rom_cb.p_dmac_scan_get_ch_statics_measurement_result != OAL_PTR_NULL)
    {
        g_st_dmac_scan_rom_cb.p_dmac_scan_get_ch_statics_measurement_result(pst_hal_device, &st_stats_result);
    }
}

oal_void dmac_scan_calcu_channel_ratio(hal_to_dmac_device_stru   *pst_hal_device)
{
    wlan_scan_chan_stats_stru      *pst_chan_result;
    wlan_chan_ratio_stru           *pst_chan_ratio;

#if !(defined(_PRE_PRODUCT_ID_HI110X_DEV))
    mac_device_stru                *pst_mac_device;
#endif

    /* 判断入参合法性 */
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_calcu_channel_ratio: input pointer is null!}");
        return;
    }

    pst_chan_result = &(pst_hal_device->st_chan_result);
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_chan_ratio  = &(pst_hal_device->st_chan_ratio);
#else
    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_calcu_channel_ratio: pst_mac_device pointer is null!}");
        return;
    }
    pst_chan_ratio  = &(pst_mac_device->st_chan_ratio);
#endif
    /*获取统计的信道结果:繁忙度/千分之X */
    if(0 == pst_chan_result->ul_total_stats_time_us)
    {
        pst_chan_ratio->us_chan_ratio_20M = 0;
        pst_chan_ratio->us_chan_ratio_40M = 0;
        pst_chan_ratio->us_chan_ratio_80M = 0;
    }
    else
    {
        pst_chan_ratio->us_chan_ratio_20M = (oal_uint16)DMAC_SCAN_GET_DUTY_CYC_RATIO(pst_chan_result, pst_chan_result->ul_total_free_time_20M_us);
        pst_chan_ratio->us_chan_ratio_40M = (oal_uint16)DMAC_SCAN_GET_DUTY_CYC_RATIO(pst_chan_result, pst_chan_result->ul_total_free_time_40M_us);
        pst_chan_ratio->us_chan_ratio_80M = (oal_uint16)DMAC_SCAN_GET_DUTY_CYC_RATIO(pst_chan_result, pst_chan_result->ul_total_free_time_80M_us);
    }

    /* 获取空闲功率 */
    if (0 == pst_chan_result->uc_free_power_cnt)
    {
        pst_chan_ratio->c_free_power_20M = 0;
        pst_chan_ratio->c_free_power_40M = 0;
        pst_chan_ratio->c_free_power_80M = 0;
    }
    else
    {
        pst_chan_ratio->c_free_power_20M = (oal_int8)(pst_chan_result->s_free_power_stats_20M / (oal_int16)pst_chan_result->uc_free_power_cnt);
        pst_chan_ratio->c_free_power_40M = (oal_int8)(pst_chan_result->s_free_power_stats_40M / (oal_int16)pst_chan_result->uc_free_power_cnt);
        pst_chan_ratio->c_free_power_80M = (oal_int8)(pst_chan_result->s_free_power_stats_80M / (oal_int16)pst_chan_result->uc_free_power_cnt);
    }

    if (g_st_dmac_scan_rom_cb.p_dmac_scan_calcu_channel_ratio != OAL_PTR_NULL)
    {
        g_st_dmac_scan_rom_cb.p_dmac_scan_calcu_channel_ratio(pst_hal_device);
    }

}

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif

