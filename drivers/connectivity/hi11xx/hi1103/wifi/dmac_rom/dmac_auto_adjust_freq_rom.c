


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_auto_adjust_freq.h"
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
#include "dmac_blockack.h"
#include "pm_extern.h"
#include "dmac_psm_sta.h"
#endif
#ifdef _PRE_WLAN_FEATURE_GREEN_AP
#include "dmac_green_ap.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_AUTO_ADJUST_FREQ_ROM_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_pps_statistics_stru *pst_dev_pps_stat = &g_device_pps_statistics;

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
device_pps_freq_level_stru *pst_dev_ba_pps_freq = g_device_ba_pps_freq_level;
device_pps_freq_level_stru *pst_dev_no_ba_pps_freq = g_device_no_ba_pps_freq_level;

dmac_freq_control_stru *pst_dev_freq_type = &g_device_freq_type;

/*****************************************************************************
  3 函数实现
*****************************************************************************/

dmac_freq_control_stru* dmac_get_auto_freq_handle(oal_void)
{
    return pst_dev_freq_type;
}

device_pps_freq_level_stru* dmac_get_ba_pps_freq_level(oal_void)
{
    return pst_dev_ba_pps_freq;
}

device_pps_freq_level_stru* dmac_get_no_ba_pps_freq_level(oal_void)
{
    return pst_dev_no_ba_pps_freq;
}



oal_uint8 dmac_get_device_nss_type(void)
{

    wlan_nss_enum_uint8           uc_nss_num = WLAN_SINGLE_NSS; //空间流个数SISO/MIMO
    wlan_nss_enum_uint8           uc_max_nss_num = WLAN_SINGLE_NSS;

    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint8                       uc_device_id;
    oal_uint8                       uc_hal_dev_num_per_chip;


    /*通过mac_vap中的chip_id找到hal_device,从而获得nss空间流个数*/
    hal_chip_get_device_num(0, &uc_hal_dev_num_per_chip);

    for (uc_device_id = 0; uc_device_id < uc_hal_dev_num_per_chip; uc_device_id++)
    {
        hal_get_hal_to_dmac_device(0, uc_device_id, &pst_hal_device);
        if (OAL_PTR_NULL == pst_hal_device)
        {
          continue;
        }

        /*获取所有VAP设备中最大的nss_num*/
        uc_nss_num = pst_hal_device->st_cfg_cap_info.en_nss_num;

        uc_max_nss_num = OAL_MAX(uc_nss_num, uc_max_nss_num);
    }


   return uc_max_nss_num;

}


oal_uint8 dmac_get_device_bw_nss_type(wlan_auto_freq_bw_enum_uint8 uc_auto_freq_bw_type, wlan_nss_enum_uint8  uc_max_nss_num)
{

    wlan_auto_freq_bw_enum_uint8 uc_auto_freq_bw_nss_type = WLAN_BW_20;

    if(WLAN_SINGLE_NSS == uc_max_nss_num)
    {

        uc_auto_freq_bw_nss_type = uc_auto_freq_bw_type;
    }

    else
    {
        uc_auto_freq_bw_nss_type = uc_auto_freq_bw_type + WLAN_BW_20_MIMO; //uc_auto_freq_bw_type + 5
    }

    return uc_auto_freq_bw_nss_type;

}



oal_void dmac_auto_freq_set_pps_level(oal_uint32 ul_pps_rate)
{
    dmac_freq_control_stru   *pst_freq_handle;
    device_pps_freq_level_stru *pst_pps_freq;
    device_pps_freq_level_stru *pst_no_ba_pps_freq;
    oal_uint8 level_idx = 0;

    pst_freq_handle = dmac_get_auto_freq_handle();
    pst_pps_freq = dmac_get_ba_pps_freq_level();
    pst_no_ba_pps_freq = dmac_get_no_ba_pps_freq_level();

    if(dmac_is_ba_setup())/* 已经建立BA */
    {
        if (ul_pps_rate <= pst_pps_freq[1].ul_speed_level)
        {
            level_idx = 0;
        }
        else if ((ul_pps_rate > pst_pps_freq[1].ul_speed_level)
            && (ul_pps_rate <= pst_pps_freq[2].ul_speed_level))
        {
            level_idx = 1;
        }
        else if ((ul_pps_rate > pst_pps_freq[2].ul_speed_level)
            && (ul_pps_rate <= pst_pps_freq[3].ul_speed_level))
        {
            level_idx = 2;
        }
        else
        {
            level_idx = 3;
        }

        pst_freq_handle->uc_req_freq_level = (oal_uint8)pst_pps_freq[level_idx].ul_cpu_freq_level;
    }
    else
    {
        if (ul_pps_rate <= pst_no_ba_pps_freq[1].ul_speed_level)
        {
            level_idx = 0;
        }
        else if ((ul_pps_rate > pst_no_ba_pps_freq[1].ul_speed_level)
            && (ul_pps_rate <= pst_no_ba_pps_freq[2].ul_speed_level))
        {
            level_idx = 1;
        }
        else if ((ul_pps_rate > pst_no_ba_pps_freq[2].ul_speed_level)
            && (ul_pps_rate <= pst_no_ba_pps_freq[3].ul_speed_level))
        {
            level_idx = 2;
        }
        else
        {
            level_idx = 3;
        }

        pst_freq_handle->uc_req_freq_level = (oal_uint8)pst_no_ba_pps_freq[level_idx].ul_cpu_freq_level;
    }

}




oal_void dmac_set_auto_freq_deinit(oal_void)
{
    frw_timeout_stru  *pst_timer;
    dmac_freq_control_stru   *pst_freq_handle;
    dmac_pps_statistics_stru *pst_pps_handle;

    pst_pps_handle = dmac_get_pps_statistics_handle();
    pst_timer = &pst_pps_handle->timer;

    pst_freq_handle = dmac_get_auto_freq_handle();
    pst_freq_handle->uc_auto_freq_enable = OAL_FALSE;

    if(OAL_TRUE == pst_timer->en_is_registerd)
    {
        pst_pps_handle->uc_timer_reuse_count --;

        /* 由最后退出的模块删除定时器 */
        if (0 == pst_pps_handle->uc_timer_reuse_count)
        {
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(pst_timer);
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_set_auto_freq_init: timer exit.}");
        }
    }

}

#endif /* end of _PRE_WLAN_FEATURE_AUTO_FREQ */
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif










