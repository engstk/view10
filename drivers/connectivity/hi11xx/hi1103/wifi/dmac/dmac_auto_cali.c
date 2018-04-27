


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_auto_cali.h"
#include "hal_ext_if.h"
#include "dmac_main.h"
#include "dmac_config.h"
#include "dmac_device.h"
#include "hisi_customize_wifi.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_AUTO_CALI_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void dmac_rf_auto_cali(oal_work_stru *pst_work)
{
    mac_device_stru     *pst_mac_device;
    dmac_device_stru    *pst_dmac_device;
    hal_to_dmac_device_stru  *pst_hal_device;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hal_cfg_custom_nvram_params_stru    *past_src;
    oal_uint16           aus_dbb_scale[NUM_OF_NV_MAX_TXPOWER] = {0};
    oal_uint8            uc_scale_idx = 0;
#endif

    pst_mac_device = OAL_CONTAINER_OF(pst_work, mac_device_stru, auto_cali_work);
    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if(OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "dmac_rf_auto_cali::pst_dmac_device null.\n");
        return;
    }

    pst_hal_device = DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device);

    //hal_rf_auto_cali(pst_mac_device->pst_device_stru);
    /* 停止PA和PHY的工作 */
    hal_disable_machw_phy_and_pa(pst_hal_device);

    /* 初始化MAC硬件 */
    hal_initialize_machw(pst_hal_device);

    /* 初始化PHY */
    hal_initialize_phy(pst_hal_device);

    /* 单芯片修改chip_id导致pcie定时获取id错误，需要关掉 */
#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT)
    #ifdef _PRE_DEBUG_MODE
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hal_device->st_exception_report_timer));
    #endif
#endif

    pst_hal_device->uc_cali_type = pst_mac_device->uc_cali_type;
    /* 初始化RF系统 */
    hal_rf_auto_cali(pst_hal_device);

    /* 使能pa */
    hal_enable_machw_phy_and_pa(pst_hal_device);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* 更新dbb scaling */
    hal_config_get_cus_nvram_params(&past_src);
    for (uc_scale_idx = 0 ; uc_scale_idx < NUM_OF_NV_MAX_TXPOWER ; uc_scale_idx++ )
    {
        aus_dbb_scale[uc_scale_idx] = past_src[uc_scale_idx].us_dbb_scale;
    }
    hal_config_update_scaling_reg(pst_hal_device, aus_dbb_scale);
#endif

    /* 打开定时器 */
#if (_PRE_TARGET_PRODUCT_TYPE_E5 == _PRE_CONFIG_TARGET_PRODUCT)
    #ifdef _PRE_DEBUG_MODE
        FRW_TIMER_CREATE_TIMER(&(pst_hal_device->st_exception_report_timer),
                        dmac_device_exception_report_timeout_fn,
                        MAC_EXCEPTION_TIME_OUT,
                        pst_hal_device,
                        OAL_TRUE,
                        OAM_MODULE_ID_DMAC,
                        pst_mac_device->ul_core_id);
    #endif
#endif
    pst_mac_device->en_cali_rdy = OAL_TRUE;
}


oal_uint32 dmac_config_auto_cali(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru     *pst_device = OAL_PTR_NULL;

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "dmac_config_auto_cali::pst_dmac_vap null.\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_auto_cali::pst_device null.\n");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device->uc_cali_type = *puc_param;
    //OAL_IO_PRINT("dmac cali type:%x\n", pst_device->uc_cali_type);
    pst_device->en_cali_rdy = OAL_FALSE;
    oal_workqueue_schedule(&(pst_device->auto_cali_work));

    return OAL_SUCC;
}


oal_uint32  dmac_auto_cali_init(mac_device_stru  *pst_device)
{
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "dmac_auto_cali_init::pst_device null.\n");

        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_INIT_WORK(&pst_device->auto_cali_work, dmac_rf_auto_cali);
    return OAL_SUCC;
}


oal_uint32  dmac_auto_cali_exit(mac_device_stru  *pst_device)
{
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "dmac_auto_cali_exit::pst_device null.\n");

        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_cancel_work_sync(&pst_device->auto_cali_work);
    return OAL_SUCC;
}

/*lint -e578*//*lint -e19*/
oal_module_license("GPL");
/*lint +e578*//*lint +e19*/

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


