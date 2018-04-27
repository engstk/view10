


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_net.h"
#include "oam_ext_if.h"
#include "hal_ext_if.h"
#include "oal_mem.h"
#include "mac_resource.h"
#include "dmac_dft.h"
#include "dmac_ext_if.h"
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "hal_phy_reg.h"
#include "hal_mac_reg.h"
#include "hal_mac_reg_field.h"
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "oal_mem.h"
#include "frw_event_main.h"
#include "pm_extern.h"
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_sta_pm.h"
#endif
#endif
#include "dmac_config.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_DFT_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/*****************************************************************************
  3 函数实现
*****************************************************************************/


oal_uint32  dmac_dft_set_phy_stat_node(hal_to_dmac_device_stru     *pst_hal_device,
                                                 oam_stats_phy_node_idx_stru *pst_phy_node_idx)
{
    hal_dft_set_phy_stat_node(pst_hal_device,pst_phy_node_idx);

    return OAL_SUCC;
}

oal_uint32  dmac_dft_get_chan_stat_result(hal_to_dmac_device_stru       *pst_hal_device,
                                                               oam_stats_dbb_env_param_stru  *pst_dbb_env_param)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
    oal_uint32       ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_CH_LOAD_STAT_PERIOD_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_ch_stats_period_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_PRIMARY_20M_IDLE_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_pri20_free_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_PRIMARY_40M_IDLE_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_pri40_free_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_PRIMARY_80M_IDLE_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_pri80_free_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_RX_PROGRESS_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_ch_rx_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_TX_PROGRESS_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_ch_tx_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_PHY_FREE_ACCUM_WIN_REG, &ul_reg_value);
    pst_dbb_env_param->uc_phy_ch_estimate_time_ms = (oal_uint8)(ul_reg_value & 0xF);

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_PHY_PRI20_IDLE_PWR_REG, &ul_reg_value);
    pst_dbb_env_param->c_phy_pri20_idle_power_dBm = (oal_int8)(ul_reg_value & 0xFF);

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_PHY_PRI40_IDLE_PWR_REG, &ul_reg_value);
    pst_dbb_env_param->c_phy_pri40_idle_power_dBm = (oal_int8)(ul_reg_value & 0xFF);

    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_PHY_PRI80_IDLE_PWR_REG, &ul_reg_value);
    pst_dbb_env_param->c_phy_pri80_idle_power_dBm = (oal_int8)(ul_reg_value & 0xFF);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    oal_uint32       ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_CH_LOAD_STAT_PERIOD_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_ch_stats_period_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_PRIMARY_20M_IDLE_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_pri20_free_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_PRIMARY_40M_IDLE_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_pri40_free_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_PRIMARY_80M_IDLE_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_pri80_free_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_RX_PROGRESS_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_ch_rx_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_TX_PROGRESS_COUNT_REG, &ul_reg_value);
    pst_dbb_env_param->us_mac_ch_tx_time_us = (oal_uint16)ul_reg_value;

    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_PHY_FREE_ACCUM_WIN_REG, &ul_reg_value);
    pst_dbb_env_param->uc_phy_ch_estimate_time_ms = (oal_uint8)(ul_reg_value & 0xF);

    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_PHY_PRI20_IDLE_PWR_REG, &ul_reg_value);
    pst_dbb_env_param->c_phy_pri20_idle_power_dBm = (oal_int8)(ul_reg_value & 0xFF);

    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_PHY_PRI40_IDLE_PWR_REG, &ul_reg_value);
    pst_dbb_env_param->c_phy_pri40_idle_power_dBm = (oal_int8)(ul_reg_value & 0xFF);
#ifdef _PRE_WLAN_1103_PILOT
    //TODO
#else
    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_PHY_PRI80_IDLE_PWR_REG, &ul_reg_value);
    pst_dbb_env_param->c_phy_pri80_idle_power_dBm = (oal_int8)(ul_reg_value & 0xFF);
#endif
#endif
    return OAL_SUCC;
}


oal_uint32  dmac_dft_report_dbb_env_param(mac_device_stru *pst_macdev, hal_to_dmac_device_stru *pst_hal_device)
{
    oam_stats_dbb_env_param_stru        st_dbb_env_param;
    oal_uint32                          ul_beacon_miss_num;
    oal_uint32                          ul_loop;
    oal_uint32                          ul_clear_reg_val = 0;

    OAL_MEMZERO(&st_dbb_env_param, OAL_SIZEOF(oam_stats_dbb_env_param_stru));

    /* 获取接收到非本机地址的帧个数，单位是: 个/s */
    st_dbb_env_param.ul_non_directed_frame_num = (pst_macdev->st_dbb_env_param_ctx.ul_non_directed_frame_num
                                               * DMAC_DFT_REPORT_TO_COLLECT_TIMES) >> 1;
    /* 清零超时次数和非本机地址帧个数 */
    pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt = 0;
    pst_macdev->st_dbb_env_param_ctx.ul_non_directed_frame_num = 0;

    /* 获取每一个ap beacon miss最大次数 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_BEACON_MISS_NUM_REG, &ul_beacon_miss_num);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_BEACON_MISS_MAX_NUM_REG, &ul_beacon_miss_num);
#endif
    for (ul_loop = 0; ul_loop < WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE; ul_loop++)
    {
        DMAC_DFT_AP_BEACON_MISS_MAX_NUM(ul_loop, ul_beacon_miss_num, &st_dbb_env_param.aul_beacon_miss_max_num[ul_loop]);
    }

    /* 清零mac统计ap beacon miss最大次数 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_COUNTER_CLEAR_REG, &ul_clear_reg_val);
    hal_reg_write(pst_hal_device, (oal_uint32)HI1102_MAC_COUNTER_CLEAR_REG, ul_clear_reg_val | BIT23);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_COUNTER_CLEAR_REG, &ul_clear_reg_val);
    hal_reg_write(pst_hal_device, (oal_uint32)HI1103_MAC_COUNTER_CLEAR_REG, ul_clear_reg_val | BIT23);
#endif
    /* 获取MAC PHY测量信道结果 */
    dmac_dft_get_chan_stat_result(pst_hal_device, &st_dbb_env_param);

    /* 上报参数 */
    return oam_report_dft_params(BROADCAST_MACADDR, (oal_uint8 *)&st_dbb_env_param,(oal_uint16)OAL_SIZEOF(oam_stats_dbb_env_param_stru), OAM_OTA_TYPE_DBB_ENV_PARAM);

}



oal_uint32  dmac_dft_collect_dbb_env_param_timeout(oal_void *p_arg)
{
    mac_vap_stru            *pst_mac_vap;
    mac_device_stru         *pst_macdev;
    hal_to_dmac_device_stru *pst_hal_device;
    oal_uint32               ul_reg_tmp_val;

    pst_mac_vap = (mac_vap_stru *)p_arg;

    pst_macdev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_macdev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_collect_dbb_env_param_timeout::device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_collect_dbb_env_param_timeout::pst_hal_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 增加超时次数的计数 */
    pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt++;

    /* 如果本次是第98次超时，通知DBB MAC开始统计CCA空闲率，DBB PHY开始统计信道空闲功率 */
    if (DMAC_DFT_REPORT_TO_COLLECT_TIMES - 2 == pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt)
    {
        /* 配置DBB MAC统计周期,20ms */
        hal_set_ch_statics_period(pst_hal_device, DMAC_DFT_MAC_CHAN_STAT_PERIOD);

        /* 配置DBB PHY统计周期 */
        hal_set_ch_measurement_period(pst_hal_device, DMAC_DFT_PHY_CHAN_MEASUREMENT_PERIOD);

        /* 使能DBB MAC和DBB PHY开始统计 */
        hal_enable_ch_statics(pst_hal_device, 1);

        return OAL_SUCC;
    }
    /* 如果本次是第99次超时，开始收集一个周期(20ms)数据(接收到非本机地址的帧个数) */
    else if (DMAC_DFT_REPORT_TO_COLLECT_TIMES - 1 == pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt)
    {
        /* 配置mac不过滤，持续20ms */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
        hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_RX_FRAMEFILT_REG, &ul_reg_tmp_val);
        hal_reg_write(pst_hal_device, (oal_uint32)HI1102_MAC_RX_FRAMEFILT_REG, ul_reg_tmp_val & (~(BIT11 | BIT13)));
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_RX_FRAMEFILT_REG, &ul_reg_tmp_val);
        hal_reg_write(pst_hal_device, (oal_uint32)HI1103_MAC_RX_FRAMEFILT_REG, ul_reg_tmp_val & (~(HI1103_MAC_CFG_NON_DIRECT_DATA_OTHER_BSS_FLT_EN_MASK | HI1103_MAC_CFG_NON_DIRECT_MGMT_OTHER_BSS_FLT_EN_MASK)));
#endif

        return OAL_SUCC;
    }
    else if (DMAC_DFT_REPORT_TO_COLLECT_TIMES == pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt)
    {
        /* 配置mac过滤 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
        hal_reg_info(pst_hal_device, (oal_uint32)HI1102_MAC_RX_FRAMEFILT_REG, &ul_reg_tmp_val);
        hal_reg_write(pst_hal_device, (oal_uint32)HI1102_MAC_RX_FRAMEFILT_REG, ul_reg_tmp_val | (BIT11 | BIT13));
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        hal_reg_info(pst_hal_device, (oal_uint32)HI1103_MAC_RX_FRAMEFILT_REG, &ul_reg_tmp_val);
        hal_reg_write(pst_hal_device, (oal_uint32)HI1103_MAC_RX_FRAMEFILT_REG, ul_reg_tmp_val | (HI1103_MAC_CFG_NON_DIRECT_DATA_OTHER_BSS_FLT_EN_MASK | HI1103_MAC_CFG_NON_DIRECT_MGMT_OTHER_BSS_FLT_EN_MASK));
#endif

        return dmac_dft_report_dbb_env_param(pst_macdev, pst_hal_device);
    }
    else
    {
        return OAL_SUCC;
    }
}



oal_uint32  dmac_dft_start_report_dbb_env(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_start_report_param::vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_start_report_param::device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 启动采集维测参数定时器，定时器超时100次的时候上报维测参数 */
    FRW_TIMER_CREATE_TIMER(&pst_device->st_dbb_env_param_ctx.st_collect_period_timer,
                           dmac_dft_collect_dbb_env_param_timeout,
                           DMAC_DFT_COLLECT_DBB_ENV_PARAM_TIMEOUT,
                           (oal_void *)pst_mac_vap,
                           OAL_TRUE,
                           OAM_MODULE_ID_DMAC,
                           pst_device->ul_core_id);

    pst_device->st_dbb_env_param_ctx.en_non_directed_frame_stat_flg = OAL_TRUE;

    return OAL_SUCC;
}


oal_uint32  dmac_dft_stop_report_dbb_env(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_stop_report_param::vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_stop_report_param::device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 删除定时器 */
    if (OAL_TRUE == pst_device->st_dbb_env_param_ctx.st_collect_period_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_device->st_dbb_env_param_ctx.st_collect_period_timer);
    }

    /* 清零计数器 */
    pst_device->st_dbb_env_param_ctx.uc_collect_period_cnt = 0;
    pst_device->st_dbb_env_param_ctx.s_ant_power           = 0;
    pst_device->st_dbb_env_param_ctx.en_non_directed_frame_stat_flg = 0;
    pst_device->st_dbb_env_param_ctx.ul_non_directed_frame_num = 0;

    return OAL_SUCC;
}



oal_uint32  dmac_dft_mgmt_stat_incr(
                                    mac_device_stru   *pst_mac_dev,
                                    oal_uint8   *puc_mac_hdr_addr,
                                    mac_dev_mgmt_stat_type_enum_uint8 en_type)
{
    oal_uint8           uc_subtype;

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_mac_hdr_addr || OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_dft_mgmt_stat_incr::param is null. puc_mac_hdr_addr = [%d],\
                       mac_dev = [%d]}", puc_mac_hdr_addr, pst_mac_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (MAC_DEV_MGMT_STAT_TYPE_BUTT <= en_type)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_dft_mgmt_stat_incr::en_type exceeds! en_type = [%d].}", en_type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (WLAN_MANAGEMENT == mac_frame_get_type_value(puc_mac_hdr_addr))
    {
        uc_subtype = mac_frame_get_subtype_value(puc_mac_hdr_addr);
        if (uc_subtype >= WLAN_MGMT_SUBTYPE_BUTT)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_dft_mgmt_stat_incr::uc_subtype exceeds! uc_subtype = [%d].}",
                           uc_subtype);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        switch (en_type)
        {
            case MAC_DEV_MGMT_STAT_TYPE_TX:
                DMAC_DFT_MGMT_STAT_INCR(pst_mac_dev->st_mgmt_stat.aul_tx_mgmt_soft[uc_subtype]);
            break;

            case MAC_DEV_MGMT_STAT_TYPE_RX:
                DMAC_DFT_MGMT_STAT_INCR(pst_mac_dev->st_mgmt_stat.aul_rx_mgmt[uc_subtype]);
            break;

            case MAC_DEV_MGMT_STAT_TYPE_TX_COMPLETE:
                DMAC_DFT_MGMT_STAT_INCR(pst_mac_dev->st_mgmt_stat.aul_tx_mgmt_hardware[uc_subtype]);
            break;

            default:
            break;
        }
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_STA_PM

oal_void dmac_mac_key_statis_info(dmac_vap_stru *pst_dmac_vap, mac_device_stru  *pst_device)
{
    hal_mac_key_statis_info_stru      st_mac_key_statis_info;

    if ((WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode) || (IS_P2P_DEV(&pst_dmac_vap->st_vap_base_info)))
    {
        return;
    }

    hal_get_mac_statistics_data(pst_dmac_vap->pst_hal_device, &st_mac_key_statis_info);
    st_mac_key_statis_info.ul_tkipccmp_rep_fail_cnt += pst_device->st_mac_key_statis_info.ul_tkipccmp_rep_fail_cnt;
    st_mac_key_statis_info.ul_tx_mpdu_cnt += pst_device->st_mac_key_statis_info.ul_tx_mpdu_cnt;
    st_mac_key_statis_info.ul_rx_passed_mpdu_cnt += pst_device->st_mac_key_statis_info.ul_rx_passed_mpdu_cnt;
    st_mac_key_statis_info.ul_rx_failed_mpdu_cnt += pst_device->st_mac_key_statis_info.ul_rx_failed_mpdu_cnt;
    st_mac_key_statis_info.ul_rx_tkipccmp_mic_fail_cnt += pst_device->st_mac_key_statis_info.ul_rx_tkipccmp_mic_fail_cnt;
    st_mac_key_statis_info.ul_key_search_fail_cnt += pst_device->st_mac_key_statis_info.ul_key_search_fail_cnt;
    st_mac_key_statis_info.ul_phy_rx_dotb_ok_frm_cnt += pst_device->st_mac_key_statis_info.ul_phy_rx_dotb_ok_frm_cnt;
    st_mac_key_statis_info.ul_phy_rx_htvht_ok_frm_cnt += pst_device->st_mac_key_statis_info.ul_phy_rx_htvht_ok_frm_cnt;
    st_mac_key_statis_info.ul_phy_rx_lega_ok_frm_cnt += pst_device->st_mac_key_statis_info.ul_phy_rx_lega_ok_frm_cnt;
    st_mac_key_statis_info.ul_phy_rx_dotb_err_frm_cnt += pst_device->st_mac_key_statis_info.ul_phy_rx_dotb_err_frm_cnt;
    st_mac_key_statis_info.ul_phy_rx_htvht_err_frm_cnt += pst_device->st_mac_key_statis_info.ul_phy_rx_htvht_err_frm_cnt;
    st_mac_key_statis_info.ul_phy_rx_lega_err_frm_cnt += pst_device->st_mac_key_statis_info.ul_phy_rx_lega_err_frm_cnt;

    OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_mac_key_statis_info::tx_mpdu_cnt:[%d],rx_passed_mpdu_cnt[%d],rx_failed_mpdu_cnt[%d],tkipccmp_rep_fail_cnt:[%d],rx_mic_fail_cnt[%d],key_search_fail_cnt[%d].}",6,
                st_mac_key_statis_info.ul_tx_mpdu_cnt,st_mac_key_statis_info.ul_rx_passed_mpdu_cnt,st_mac_key_statis_info.ul_rx_failed_mpdu_cnt,st_mac_key_statis_info.ul_tkipccmp_rep_fail_cnt,st_mac_key_statis_info.ul_rx_tkipccmp_mic_fail_cnt,st_mac_key_statis_info.ul_key_search_fail_cnt);

    OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_mac_key_statis_info::phy rx status, rx_dotb_ok_frm_cnt:[%d],rx_htvht_ok_frm_cnt[%d],rx_lega_ok_frm_cnt:[%d],rx_dotb_err_frm_cnt[%d],rx_htvht_err_frm_cnt[%d],rx_lega_err_frm_cnt[%d].}",6,
                st_mac_key_statis_info.ul_phy_rx_dotb_ok_frm_cnt,st_mac_key_statis_info.ul_phy_rx_htvht_ok_frm_cnt,
                st_mac_key_statis_info.ul_phy_rx_lega_ok_frm_cnt,st_mac_key_statis_info.ul_phy_rx_dotb_err_frm_cnt,
                st_mac_key_statis_info.ul_phy_rx_htvht_err_frm_cnt,st_mac_key_statis_info.ul_phy_rx_lega_err_frm_cnt);
}
#endif


oal_void  dmac_dft_report_linkloss_info(dmac_vap_stru *pst_dmac_sta, mac_device_stru *pst_mac_dev)
{

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    OAM_WARNING_LOG3(pst_dmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
        "bt_linkloss_info: bt_link_loss_cnt =%d, bt_linkloss_threshold = %d, bt state is bt_on/off =%d",
        GET_LINKLOSS_CNT(pst_dmac_sta, WLAN_LINKLOSS_MODE_BT),
        GET_LINKLOSS_THRESHOLD(pst_dmac_sta, WLAN_LINKLOSS_MODE_BT),
        pst_dmac_sta->pst_hal_chip->st_btcoex_btble_status.un_bt_status.st_bt_status.bit_bt_on);
#endif
    OAM_WARNING_LOG3(pst_dmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
        "dbac_linkloss_info: dbac_link_loss_cnt =%d, dbac_linkloss_threshold = %d, dbac_runing = %d",
        GET_LINKLOSS_CNT(pst_dmac_sta, WLAN_LINKLOSS_MODE_DBAC),
        GET_LINKLOSS_THRESHOLD(pst_dmac_sta, WLAN_LINKLOSS_MODE_DBAC),
        pst_mac_dev->en_dbac_running);

    OAM_WARNING_LOG2(pst_dmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
        "normal_linkloss_info: normal_link_loss_cnt =%d, normal_linkloss_threshold = %d",
        GET_LINKLOSS_CNT(pst_dmac_sta, WLAN_LINKLOSS_MODE_NORMAL),
        GET_LINKLOSS_THRESHOLD(pst_dmac_sta, WLAN_LINKLOSS_MODE_NORMAL));

}

oal_void dmac_dft_set_simple_vap_info(dmac_vap_dft_stru *pst_dmac_vap_dft, mac_vap_stru  *pst_mac_vap)
{
    dmac_vap_stru       *pst_dmac_vap;

    if ((OAL_PTR_NULL == pst_dmac_vap_dft) || (OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_dft_set_simple_vap_info:: input params is invalid, [%p] [%p]!}",
                       pst_dmac_vap_dft, pst_mac_vap);
        return;
    }

    /* 获取dmac vap */
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_dft_set_simple_vap_info:: dmac vap[%d] is null!}",
                       pst_mac_vap->uc_vap_id);
        return;
    }

    OAL_MEMZERO(pst_dmac_vap_dft, OAL_SIZEOF(dmac_vap_dft_stru));

    /* dmac_vap_dft_stru结构体赋值 */
    /* 以下赋值从mac_vap_stru结构体中取值 */
    pst_dmac_vap_dft->uc_vap_id    = pst_mac_vap->uc_vap_id;
    pst_dmac_vap_dft->uc_device_id = pst_mac_vap->uc_device_id;
    pst_dmac_vap_dft->uc_chip_id   = pst_mac_vap->uc_vap_id;
    pst_dmac_vap_dft->en_vap_mode  = pst_mac_vap->en_vap_mode;
    pst_dmac_vap_dft->en_vap_state = pst_mac_vap->en_vap_state;
    pst_dmac_vap_dft->en_protocol  = pst_mac_vap->en_protocol;
    pst_dmac_vap_dft->bit_has_user_bw_limit  = pst_mac_vap->bit_has_user_bw_limit;
    pst_dmac_vap_dft->bit_vap_bw_limit       = pst_mac_vap->bit_vap_bw_limit;
    pst_dmac_vap_dft->uc_tx_power            = pst_mac_vap->uc_tx_power;
    pst_dmac_vap_dft->en_p2p_mode            = pst_mac_vap->en_p2p_mode;
    pst_dmac_vap_dft->uc_p2p_gocl_hal_vap_id = pst_mac_vap->uc_p2p_gocl_hal_vap_id;
    pst_dmac_vap_dft->us_user_nums           = pst_mac_vap->us_user_nums;
    pst_dmac_vap_dft->us_multi_user_idx      = pst_mac_vap->us_multi_user_idx;

    oal_memcopy(pst_dmac_vap_dft->auc_bssid, pst_mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN);
    oal_memcopy((oal_uint8 *)&pst_dmac_vap_dft->st_channel,
                (oal_uint8 *)&pst_mac_vap->st_channel, OAL_SIZEOF(mac_channel_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_vap_dft->st_cap_flag,
                (oal_uint8 *)&pst_mac_vap->st_cap_flag, OAL_SIZEOF(mac_cap_flag_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_vap_dft->st_protection,
                (oal_uint8 *)&pst_mac_vap->st_protection, OAL_SIZEOF(mac_protection_stru));

    /* 以下赋值从dmac_vap_stru结构体中取值 */
    pst_dmac_vap_dft->uc_ps_user_num     = pst_dmac_vap->uc_ps_user_num;
    pst_dmac_vap_dft->uc_dtim_count      = pst_dmac_vap->uc_dtim_count;
    pst_dmac_vap_dft->uc_uapsd_max_depth = pst_dmac_vap->uc_uapsd_max_depth;

    oal_memcopy((oal_uint8 *)&pst_dmac_vap_dft->st_linkloss_info,
                (oal_uint8 *)&pst_dmac_vap->st_linkloss_info, OAL_SIZEOF(dmac_vap_linkloss_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_vap_dft->st_tx_alg,
                (oal_uint8 *)&pst_dmac_vap->st_tx_alg, OAL_SIZEOF(hal_tx_txop_alg_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_vap_dft->st_tx_data_mcast,
                (oal_uint8 *)&pst_dmac_vap->st_tx_data_mcast, OAL_SIZEOF(hal_tx_txop_alg_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_vap_dft->st_tx_data_bcast,
                (oal_uint8 *)&pst_dmac_vap->st_tx_data_bcast, OAL_SIZEOF(hal_tx_txop_alg_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_vap_dft->ast_tx_mgmt_ucast,
                (oal_uint8 *)&pst_dmac_vap->ast_tx_mgmt_ucast, OAL_SIZEOF(hal_tx_txop_alg_stru) * WLAN_BAND_BUTT);
    oal_memcopy((oal_uint8 *)&pst_dmac_vap_dft->ast_tx_mgmt_bmcast,
                (oal_uint8 *)&pst_dmac_vap->ast_tx_mgmt_bmcast, OAL_SIZEOF(hal_tx_txop_alg_stru) * WLAN_BAND_BUTT);

    return;
}


oal_void dmac_dft_report_dmac_vap_info(mac_vap_stru  *pst_mac_vap)
{
    dmac_vap_dft_stru   st_dmac_vap_dft;

    /* ota上报的dmac_vap_dft_stru结构体赋值 */
    dmac_dft_set_simple_vap_info(&st_dmac_vap_dft, pst_mac_vap);

    /* 上报dmac vap信息 */
    oam_report_dft_params(BROADCAST_MACADDR,
                          (oal_uint8 *)&st_dmac_vap_dft,
                          (oal_uint16)OAL_SIZEOF(dmac_vap_dft_stru),
                          OAM_OTA_TYPE_DMAC_VAP);

    return;
}


oal_void dmac_dft_set_simple_user_info(dmac_user_dft_stru *pst_dmac_user_dft, dmac_user_stru  *pst_dmac_user)
{
    mac_user_stru       *pst_mac_user;

    if ((OAL_PTR_NULL == pst_dmac_user_dft) || (OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_dft_set_simple_user_info:: input params is invalid, [%p] [%p]!}",
                       pst_dmac_user_dft, pst_dmac_user);
        return;
    }

    OAL_MEMZERO(pst_dmac_user_dft, OAL_SIZEOF(dmac_user_dft_stru));

    /* dmac_user_dft_stru结构体赋值 */
    /* 以下赋值从mac_user_stru结构体下取值 */
    pst_mac_user = &pst_dmac_user->st_user_base_info;
    pst_dmac_user_dft->us_assoc_id      = pst_mac_user->us_assoc_id;
    pst_dmac_user_dft->uc_device_id     = pst_mac_user->uc_device_id;
    pst_dmac_user_dft->en_is_multi_user = pst_mac_user->en_is_multi_user;
    pst_dmac_user_dft->uc_vap_id        = pst_mac_user->uc_vap_id;
    pst_dmac_user_dft->uc_chip_id       = pst_mac_user->uc_chip_id;
    pst_dmac_user_dft->en_protocol_mode            = pst_mac_user->en_protocol_mode;
    pst_dmac_user_dft->en_avail_protocol_mode      = pst_mac_user->en_avail_protocol_mode;
    pst_dmac_user_dft->en_cur_protocol_mode        = pst_mac_user->en_cur_protocol_mode;
    pst_dmac_user_dft->en_avail_num_spatial_stream = pst_mac_user->en_avail_num_spatial_stream;
    pst_dmac_user_dft->en_user_num_spatial_stream  = pst_mac_user->en_user_num_spatial_stream;
    pst_dmac_user_dft->en_avail_bf_num_spatial_stream = pst_mac_user->en_avail_bf_num_spatial_stream;
    pst_dmac_user_dft->en_port_valid      = pst_mac_user->en_port_valid;
    pst_dmac_user_dft->en_bandwidth_cap   = pst_mac_user->en_bandwidth_cap;
    pst_dmac_user_dft->en_avail_bandwidth = pst_mac_user->en_avail_bandwidth;
    pst_dmac_user_dft->en_cur_bandwidth   = pst_mac_user->en_cur_bandwidth;
    pst_dmac_user_dft->en_user_asoc_state = pst_mac_user->en_user_asoc_state;

    oal_memcopy(pst_dmac_user_dft->auc_user_mac_addr, pst_mac_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);
    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_avail_op_rates,
                (oal_uint8 *)&pst_mac_user->st_avail_op_rates, OAL_SIZEOF(mac_rate_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_user_tx_info,
                (oal_uint8 *)&pst_mac_user->st_user_tx_info, OAL_SIZEOF(mac_user_tx_param_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_cap_info,
                (oal_uint8 *)&pst_mac_user->st_cap_info, OAL_SIZEOF(mac_user_cap_info_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_ht_hdl,
                (oal_uint8 *)&pst_mac_user->st_ht_hdl, OAL_SIZEOF(mac_user_ht_hdl_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_vht_hdl,
                (oal_uint8 *)&pst_mac_user->st_vht_hdl, OAL_SIZEOF(mac_vht_hdl_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_key_info,
                (oal_uint8 *)&pst_mac_user->st_key_info, OAL_SIZEOF(mac_key_mgmt_stru));

    /* 以下赋值从dmac_user_stru结构体下取值 */
    pst_dmac_user_dft->uc_lut_index     = pst_dmac_user->uc_lut_index;
    pst_dmac_user_dft->uc_uapsd_flag    = pst_dmac_user->uc_uapsd_flag;
    pst_dmac_user_dft->uc_max_key_index = pst_dmac_user->uc_max_key_index;

    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_tx_data_mcast,
                (oal_uint8 *)&pst_dmac_user->st_tx_data_mcast, OAL_SIZEOF(hal_tx_txop_alg_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_tx_data_bcast,
                (oal_uint8 *)&pst_dmac_user->st_tx_data_bcast, OAL_SIZEOF(hal_tx_txop_alg_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_uapsd_status,
                (oal_uint8 *)&pst_dmac_user->st_uapsd_status, OAL_SIZEOF(mac_user_uapsd_status_stru));
    oal_memcopy((oal_uint8 *)&pst_dmac_user_dft->st_user_rate_info,
                (oal_uint8 *)&pst_dmac_user->st_user_rate_info, OAL_SIZEOF(dmac_user_rate_info_stru));

    return;
}


oal_void dmac_dft_report_dmac_user_info(mac_vap_stru  *pst_mac_vap)
{
    oal_dlist_head_stru     *pst_entry;
    dmac_user_stru          *pst_dmac_user;
    dmac_user_dft_stru       st_dmac_user_dft;

    /* 遍历vap下所有用户 */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_dmac_user = (dmac_user_stru *)pst_entry;

        /* ota上报的dmac_user_dft_stru结构体赋值 */
        dmac_dft_set_simple_user_info(&st_dmac_user_dft, pst_dmac_user);

        oam_report_dft_params(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                              (oal_uint8 *)&st_dmac_user_dft,
                              (oal_uint16)OAL_SIZEOF(dmac_user_dft_stru),
                              OAM_OTA_TYPE_DMAC_USER);
    }

    return;
}


oal_void dmac_dft_report_dmac_event_info(mac_vap_stru  *pst_mac_vap)
{
    frw_event_mgmt_stru    *pst_event_mgmt;
    frw_event_queue_stru    ast_event_queue[WLAN_FRW_MAX_NUM_CORES * FRW_EVENT_MAX_NUM_QUEUES];
    oal_uint32              ul_core_id;

    /* 遍历每个核的每个vap对应的事件队列 */
    for(ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++)
    {
        /* 根据核找到相应的核上的事件管理结构体 */
        pst_event_mgmt = &g_ast_event_manager[ul_core_id];

        oal_memcopy((oal_uint8 *)(&ast_event_queue[ul_core_id * FRW_EVENT_MAX_NUM_QUEUES]),
                    (oal_uint8 *)pst_event_mgmt->st_event_queue, OAL_SIZEOF(pst_event_mgmt->st_event_queue));
    }

    /* 上报当前存在的所有事件队列信息 */
    oam_report_dft_params(BROADCAST_MACADDR,
                          (oal_uint8 *)ast_event_queue,
                          (oal_uint16)OAL_SIZEOF(ast_event_queue),
                          OAM_OTA_TYPE_EVENT_QUEUE);

    return;
}


oal_void dmac_dft_report_dmac_memory_info(mac_vap_stru  *pst_mac_vap)
{
    mem_pool_dft_stru          ast_mem_poop_dft_info[OAL_MEM_POOL_ID_BUTT];
    mem_pool_dft_stru         *pst_mem_poop_dft;
    mem_subpool_dft_stru      *pst_mem_subpool_dft;
    oal_mem_pool_stru         *pst_mem_pool;
    oal_mem_subpool_stru      *pst_mem_subpool;
    oal_netbuf_pool_stru      *pst_netbuf_pool;
    oal_netbuf_subpool_stru   *pst_netbuf_subpool;
    oal_uint32                 ul_pool_id = 0;
    oal_uint32                 ul_sub_pool_id = 0;
    oal_uint32                 ul_sub_pool_cnt = 0;
    oal_uint                   ul_flag = 0;

    /* OTA上报内存使用情况的信息清零 */
    OAL_MEMZERO(ast_mem_poop_dft_info, OAL_SIZEOF(ast_mem_poop_dft_info));

    oal_irq_save(&ul_flag, OAL_5115IRQ_DMSC);

    for(ul_pool_id = 0; ul_pool_id < OAL_MEM_POOL_ID_NETBUF; ul_pool_id++)
    {
        /* 获取内存池 */
        pst_mem_pool = oal_mem_get_pool((oal_uint8)ul_pool_id);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mem_pool))
        {
            oal_irq_enable();
            return;
        }

        /* 将内存池的使用情况进行赋值，用于上报 */
        pst_mem_poop_dft = &(ast_mem_poop_dft_info[ul_pool_id]);
        pst_mem_poop_dft->uc_pool_id = (oal_uint8)ul_pool_id;
        pst_mem_poop_dft->uc_subpool_cnt   = pst_mem_pool->uc_subpool_cnt;
        pst_mem_poop_dft->us_max_byte_len  = pst_mem_pool->us_max_byte_len;
        pst_mem_poop_dft->us_mem_total_cnt = pst_mem_pool->us_mem_total_cnt;
        pst_mem_poop_dft->us_mem_used_cnt  = pst_mem_pool->us_mem_used_cnt;

        /* 获取当前内存池的子池个数 */
        ul_sub_pool_cnt = (oal_uint32)pst_mem_poop_dft->uc_subpool_cnt;
        for (ul_sub_pool_id = 0; ul_sub_pool_id < ul_sub_pool_cnt; ul_sub_pool_id++)
        {
            pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[ul_sub_pool_id]);

            /* 将本内存池的子内存池的使用情况进行赋值，用于上报 */
            pst_mem_subpool_dft = &(pst_mem_poop_dft->ast_subpool_table[ul_sub_pool_id]);
            pst_mem_subpool_dft->uc_sub_pool_id = (oal_uint8)ul_sub_pool_id;
            pst_mem_subpool_dft->us_total_cnt   = pst_mem_subpool->us_total_cnt;
            pst_mem_subpool_dft->us_free_cnt    = pst_mem_subpool->us_free_cnt;
        }

    }

    /* nebuf内存池使用情况赋值，用于上报 */
    pst_netbuf_pool = &g_st_netbuf_pool;

    pst_mem_poop_dft = &(ast_mem_poop_dft_info[OAL_MEM_POOL_ID_NETBUF]);
    pst_mem_poop_dft->uc_pool_id       = OAL_MEM_POOL_ID_NETBUF;
    pst_mem_poop_dft->us_max_byte_len  = pst_netbuf_pool->us_max_byte_len;
    pst_mem_poop_dft->us_mem_total_cnt = pst_netbuf_pool->us_mem_total_cnt;
    pst_mem_poop_dft->us_mem_used_cnt  = pst_netbuf_pool->us_mem_used_cnt;

    ul_sub_pool_cnt = pst_netbuf_pool->uc_subpool_cnt;
    for (ul_sub_pool_id = 0; ul_sub_pool_id < ul_sub_pool_cnt; ul_sub_pool_id++)
    {
        pst_netbuf_subpool = &(pst_netbuf_pool->ast_subpool_table[ul_sub_pool_id]);

        /* netbuf内存子池的使用情况进行赋值，用于上报 */
        pst_mem_subpool_dft = &(pst_mem_poop_dft->ast_subpool_table[ul_sub_pool_id]);
        pst_mem_subpool_dft->uc_sub_pool_id = (oal_uint8)ul_sub_pool_id;
        pst_mem_subpool_dft->us_total_cnt   = pst_netbuf_subpool->us_total_cnt;
        pst_mem_subpool_dft->us_free_cnt    = pst_netbuf_subpool->us_free_cnt;
    }

    oal_irq_restore(&ul_flag, OAL_5115IRQ_DMSC);

    /* 上报当前内存池的使用情况 */
    oam_report_dft_params(BROADCAST_MACADDR,
                          (oal_uint8 *)ast_mem_poop_dft_info,
                          (oal_uint16)OAL_SIZEOF(ast_mem_poop_dft_info),
                          OAM_OTA_TYPE_MEMPOOL);

    return;
}


oal_void dmac_dft_report_mac_hardware_info(mac_vap_stru  *pst_mac_vap)
{
    hal_device_dft_stru          st_hal_device_info;
    mac_device_stru             *pst_mac_device;
    hal_to_dmac_device_stru     *pst_hal_device;

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY,
                         "{dmac_dft_report_mac_hardware_info:: mac device[%d] is null!}", pst_mac_vap->uc_device_id);
        return;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_dft_report_mac_hardware_info::vap id [%d],dmac_user_get_hal_device null}",pst_mac_vap->uc_vap_id);
        return;
    }

    /* OTA上报hal device信息清零 */
    OAL_MEMZERO(&st_hal_device_info, OAL_SIZEOF(hal_device_dft_stru));

    /* 上报硬件信息赋值 */
    st_hal_device_info.uc_chip_id       = pst_hal_device->uc_chip_id;
    st_hal_device_info.uc_device_id     = pst_hal_device->uc_device_id;
    st_hal_device_info.uc_mac_device_id = pst_hal_device->uc_mac_device_id;
    st_hal_device_info.uc_curr_state    = pst_hal_device->uc_curr_state;
    st_hal_device_info.ul_core_id       = pst_hal_device->ul_core_id;

    /* 拷贝发送、接收、虚假队列中的信息，用于ota上报 */
    oal_memcopy((oal_uint8 *)(st_hal_device_info.ast_rx_dscr_queue),
                (oal_uint8 *)pst_hal_device->ast_rx_dscr_queue, OAL_SIZEOF(st_hal_device_info.ast_rx_dscr_queue));
    oal_memcopy((oal_uint8 *)(st_hal_device_info.ast_tx_dscr_queue),
                (oal_uint8 *)pst_hal_device->ast_tx_dscr_queue, OAL_SIZEOF(st_hal_device_info.ast_tx_dscr_queue));


    /* 上报当前mac硬件相关信息，主要包括硬件发送、接收队列 */
    oam_report_dft_params(BROADCAST_MACADDR,
                          (oal_uint8 *)(&st_hal_device_info),
                          (oal_uint16)OAL_SIZEOF(st_hal_device_info),
                          OAM_OTA_TYPE_HARDWARE_INFO);

    return;
}


oal_void dmac_dft_report_dmac_queue_info(mac_vap_stru  *pst_mac_vap)
{
    oal_dlist_head_stru                    *pst_entry;
    dmac_user_stru                         *pst_dmac_user;
    dmac_user_queue_info_dft_stru           st_queue_info;

    /* 遍历vap下所有用户 */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_dmac_user = (dmac_user_stru *)pst_entry;

        oal_memcopy((oal_uint8 *)st_queue_info.ast_tx_tid_queue,
                    (oal_uint8 *)pst_dmac_user->ast_tx_tid_queue, OAL_SIZEOF(st_queue_info.ast_tx_tid_queue));

        #ifdef _PRE_WLAN_FEATURE_UAPSD
        oal_memcopy((oal_uint8 *)&st_queue_info.st_uapsd_stru,
                    (oal_uint8 *)&pst_dmac_user->st_uapsd_stru, OAL_SIZEOF(st_queue_info.st_uapsd_stru));
        #endif

        oam_report_dft_params(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                              (oal_uint8 *)&st_queue_info,
                              (oal_uint16)OAL_SIZEOF(st_queue_info),
                              OAM_OTA_TYPE_USER_QUEUE_INFO);
    }

    return;
}

oal_void  dmac_dft_print_mac_phy_rf(hal_to_dmac_device_stru *pst_hal_device)
{
    /* 打印mac维测信息，上报寄存器可增加*/
    hal_dft_print_machw_stat(pst_hal_device);

    /* 打印phy维测信息，上报寄存器可增加*/
    hal_dft_print_phyhw_stat(pst_hal_device);

    /* 打印rf维测信息，上报寄存器可增加*/
    hal_dft_print_rfhw_stat(pst_hal_device);
}


oal_void  dmac_dft_report_all_ota_state(mac_vap_stru *pst_mac_sta)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_sta);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_dft_report_all_ota_state::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_sta->uc_vap_id);
        return;
    }

    hal_dft_report_all_reg_state(pst_hal_device);
}

oal_void  dmac_dft_print_all_para(dmac_vap_stru *pst_dmac_sta, oal_bool_enum_uint8 en_all_info)
{
    mac_device_stru                 *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(pst_dmac_sta->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG0(pst_dmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_dft_report_all_ota_state::dev is null!}");
        return;
    }

    /*打印当前的信号状况和link_loss计数*/
    OAM_WARNING_LOG1(pst_dmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
        "signal = %d",
        oal_get_real_rssi(pst_dmac_sta->st_query_stats.s_signal));

    /*打印link_loss计数*/
    dmac_dft_report_linkloss_info(pst_dmac_sta, pst_mac_dev);

    /*打印当前ac队列信息*/
    OAM_WARNING_LOG_ALTER(pst_dmac_sta->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{mpdu num:total = %d, be = %d, bk = %d, vi = %d, vo = %d}",
                     5,
                     pst_mac_dev->us_total_mpdu_num,
                     pst_mac_dev->aus_ac_mpdu_num[WLAN_WME_AC_BE],
                     pst_mac_dev->aus_ac_mpdu_num[WLAN_WME_AC_BK],
                     pst_mac_dev->aus_ac_mpdu_num[WLAN_WME_AC_VI],
                     pst_mac_dev->aus_ac_mpdu_num[WLAN_WME_AC_VO]);


#ifdef _PRE_WLAN_FEATURE_STA_PM
    /* mac发送接收关键信息打印 */
    dmac_mac_key_statis_info(pst_dmac_sta,pst_mac_dev);
#endif


    if (OAL_FALSE == en_all_info)
    {
        return;
    }

#ifdef _PRE_WLAN_FEATURE_STA_PM

    /* 协议的关键计数 */
    dmac_pm_key_info_dump(pst_dmac_sta);

    /* 平台的睡眠计数*/
    pfn_wlan_dumpsleepcnt();
#endif


    /*linkloss打印mac维测信息，上报寄存器可增加*/
    hal_dft_print_machw_stat(pst_dmac_sta->pst_hal_device);

    /*linkloss打印phy维测信息，上报寄存器可增加*/
    hal_dft_print_phyhw_stat(pst_dmac_sta->pst_hal_device);

    /*linkloss打印rf维测信息，上报寄存器可增加*/
    hal_dft_print_rfhw_stat(pst_dmac_sta->pst_hal_device);
}

oal_void dmac_dft_report_all_para(dmac_vap_stru *pst_dmac_sta,oal_uint8 uc_ota_switch)
{

    /*打印维测信息*/
    dmac_dft_print_all_para(pst_dmac_sta, (oal_bool_enum_uint8 )uc_ota_switch);
    /*OTA上报维测信息*/
    if(OAL_TRUE == uc_ota_switch)
    {
        dmac_dft_report_all_ota_state(&pst_dmac_sta->st_vap_base_info);
    }
}

#endif
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

