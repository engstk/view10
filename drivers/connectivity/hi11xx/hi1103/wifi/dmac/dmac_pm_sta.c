


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_hcc_slave_if.h"
#include "hal_reset.h"
#include "wlan_spec.h"
#include "wlan_mib.h"
#include "hal_chip.h"
#include "hal_device.h"
#include "hal_phy_reg.h"

#include "oal_mem.h"
#include "mac_resource.h"
#include "dma.h"
#include "dmac_pm_sta.h"
#include "dmac_rx_data.h"
#include "frw_timer.h"
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif
#include "pm_extern.h"

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
#include "dmac_auto_adjust_freq.h"
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
#include "hal_rf_dev.h"
#endif
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_PM_STA_C



/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oal_uint32  g_ul_wait_netbuf = 0;
oal_uint32  g_ul_coex_cnt  = 0;
oal_uint32  g_ul_wait_scanning = 0;
oal_uint32  g_ul_wait_dbac = 0;

/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void dmac_psm_sync_tsf_to_sta(oal_void)
{
#ifdef _PRE_WLAN_MAC_BUGFIX_TSF_SYNC
    hal_to_dmac_vap_stru                 st_sta_hal_vap;

    /* 关闭pa */
    dmac_psm_cbb_stopwork();

    /* 将ap0的tsf和tbtt同步到sta0 */
    st_sta_hal_vap.uc_vap_id = 4;

    hal_sta_tsf_restore(&st_sta_hal_vap);

    /* 将ap1的tsf和tbtt同步到sta1 */
    st_sta_hal_vap.uc_vap_id = 5;

    hal_sta_tsf_restore(&st_sta_hal_vap);
#endif
    return;
}

oal_void dmac_psm_sync_tsf_to_ap(oal_void)
{
#ifdef _PRE_WLAN_MAC_BUGFIX_TSF_SYNC
    hal_to_dmac_vap_stru    st_sta_hal_vap;

    /* 将sta0的tsf和tbtt同步到ap0 */
    st_sta_hal_vap.uc_vap_id = 4;

    //低功耗唤醒后sta->ap的同步过程中不能进行ap——>sta的同步
    hal_sta_tsf_save(&st_sta_hal_vap, OAL_FALSE);

    /* 将sta1的tsf和tbtt同步到ap1 */
    st_sta_hal_vap.uc_vap_id = 5;

    hal_sta_tsf_save(&st_sta_hal_vap, OAL_FALSE);
#endif
    return;

}

oal_uint8 dmac_psm_check_hw_txq_state(oal_void)
{
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;
    oal_uint32                   ul_ret;
    hal_to_dmac_device_stru     *pst_hal_device = OAL_PTR_NULL;

    /* HAL接口获取CHIP0支持device个数 */
    hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "{dmac_psm_check_hw_txq_state::get_hal_device fail[%d].}", uc_device);
            continue;
        }

        /*检查接收硬件发送队列和TID队列是否空*/
        if(OAL_FALSE == hal_is_hw_tx_queue_empty(pst_hal_device))
        {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}


oal_uint8 dmac_psm_check_txrx_state(oal_void)
{
    oal_uint32                   uc_device_max;
    oal_uint8                    uc_device;
    dmac_device_stru            *pst_dmac_device;
    mac_device_stru             *pst_mac_device;
    mac_chip_stru               *pst_mac_chip;

    /* OAL接口获取支持device个数 */
    pst_mac_chip = dmac_res_get_mac_chip(0);
    uc_device_max = oal_chip_get_device_num(pst_mac_chip->ul_chip_ver);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        pst_dmac_device = dmac_res_get_mac_dev(pst_mac_chip->auc_device_id[uc_device]);
        if (OAL_PTR_NULL == pst_dmac_device)
        {
            OAM_ERROR_LOG2(0,OAM_SF_PWR,"{dmac_psm_check_txrx_state::uc_device[%d], dev id[%d].}", uc_device, pst_mac_chip->auc_device_id[uc_device]);
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
        if(pst_dmac_device->pst_hal_chip->st_btcoex_btble_status.un_bt_status.st_bt_status.bit_bt_on)
        {
            g_ul_coex_cnt++;
        }
#endif
        pst_mac_device = pst_dmac_device->pst_device_base_info;

        /* 正在扫描不睡眠 */
        if (MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state)
        {
            /* 仅在host允许睡的情况下计数++,此计数是为了验证低功耗在扫描过程中睡眠了 */
            if (OAL_TRUE == PM_WLAN_IsHostSleep())
            {
                g_ul_wait_scanning++;
            }
            return OAL_FALSE;
        }

        /* DBAC running 不睡眠 */
        if(OAL_TRUE == mac_is_dbac_running(pst_mac_device))
        {
            g_ul_wait_dbac++;
            return OAL_FALSE;
        }
    }

    /*代替最初的检查TID,hw队列,rx irq list，归一到检查netbuf池是否有使用*/
    if(OAL_FALSE == oal_mem_netbuf_psm_check())
    {
        g_ul_wait_netbuf++;
        return OAL_FALSE;
    }

    return OAL_TRUE;

}

oal_uint8  dmac_psm_is_fake_queues_empty(oal_void)
{
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;
    mac_chip_stru               *pst_mac_chip;

    pst_mac_chip = dmac_res_get_mac_chip(0);

    /* OAL接口获取支持device个数 */
    uc_device_max = oal_chip_get_device_num(pst_mac_chip->ul_chip_ver);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        if (OAL_FALSE == dmac_device_check_fake_queues_empty(uc_device))
        {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}


oal_void dmac_psm_cpu_save (hal_to_dmac_device_stru  *pst_hal_device)
{
    hal_reset_reg_save(pst_hal_device, HAL_RESET_HW_TYPE_TCM);

}

oal_void dmac_psm_save_start_dma (oal_uint8* puc_ch0,oal_uint8* puc_ch1,oal_uint8* puc_ch2)
{
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;
    oal_uint32                   ul_ret;
    hal_to_dmac_device_stru     *pst_hal_device;

    /* HAL接口获取CHIP0支持device个数 */
    hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "{dmac_psm_save_start_dma::get_hal_device fail[%d].}", uc_device);
            continue;
        }

        hal_reset_reg_dma_save(pst_hal_device, puc_ch0, puc_ch1, puc_ch2);

    }

}

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT

oal_void dmac_psm_save_dutycycle (hal_to_dmac_device_stru *pst_hal_device)
{
    hal_ch_mac_statics_stru        st_stats_result;;
    hal_to_dmac_device_rom_stru    *pst_hal_dev_rom = OAL_PTR_NULL;

    pst_hal_dev_rom = (hal_to_dmac_device_rom_stru*)pst_hal_device->_rom;

    /* 读取mac统计寄存器 */
    hal_get_txrx_frame_time(pst_hal_device, &st_stats_result);
   /*保存进入低功耗前的tx rx时间*/
    pst_hal_dev_rom->ul_rx_dir_duty_lp    += st_stats_result.ul_rx_direct_time;
    pst_hal_dev_rom->ul_rx_nondir_duty_lp += st_stats_result.ul_rx_nondir_time;
    pst_hal_dev_rom->ul_tx_ratio_lp       += st_stats_result.ul_tx_time;
}
#endif

oal_void dmac_psm_save_mac_statistics_data(mac_device_stru *pst_device, hal_to_dmac_device_stru *pst_hal_device)
{
    hal_mac_key_statis_info_stru   st_mac_key_statis_info;

    hal_get_mac_statistics_data(pst_hal_device, &st_mac_key_statis_info);

    pst_device->st_mac_key_statis_info.ul_tkipccmp_rep_fail_cnt += st_mac_key_statis_info.ul_tkipccmp_rep_fail_cnt;
    pst_device->st_mac_key_statis_info.ul_tx_mpdu_cnt  += st_mac_key_statis_info.ul_tx_mpdu_cnt;
    pst_device->st_mac_key_statis_info.ul_rx_passed_mpdu_cnt  += st_mac_key_statis_info.ul_rx_passed_mpdu_cnt;
    pst_device->st_mac_key_statis_info.ul_rx_failed_mpdu_cnt  += st_mac_key_statis_info.ul_rx_failed_mpdu_cnt;
    pst_device->st_mac_key_statis_info.ul_rx_tkipccmp_mic_fail_cnt  += st_mac_key_statis_info.ul_rx_tkipccmp_mic_fail_cnt;
    pst_device->st_mac_key_statis_info.ul_key_search_fail_cnt += st_mac_key_statis_info.ul_key_search_fail_cnt;
    pst_device->st_mac_key_statis_info.ul_phy_rx_dotb_ok_frm_cnt += st_mac_key_statis_info.ul_phy_rx_dotb_ok_frm_cnt;
    pst_device->st_mac_key_statis_info.ul_phy_rx_htvht_ok_frm_cnt  += st_mac_key_statis_info.ul_phy_rx_htvht_ok_frm_cnt;
    pst_device->st_mac_key_statis_info.ul_phy_rx_lega_ok_frm_cnt  += st_mac_key_statis_info.ul_phy_rx_lega_ok_frm_cnt;
    pst_device->st_mac_key_statis_info.ul_phy_rx_dotb_err_frm_cnt += st_mac_key_statis_info.ul_phy_rx_dotb_err_frm_cnt;
    pst_device->st_mac_key_statis_info.ul_phy_rx_htvht_err_frm_cnt  += st_mac_key_statis_info.ul_phy_rx_htvht_err_frm_cnt;
    pst_device->st_mac_key_statis_info.ul_phy_rx_lega_err_frm_cnt  += st_mac_key_statis_info.ul_phy_rx_lega_err_frm_cnt;
}


oal_void dmac_psm_reset_rx_dscr_default(hal_to_dmac_device_stru *pst_hal_device)
{
    pst_hal_device->us_rx_normal_dscr_cnt = HAL_NORMAL_RX_MAX_BUFFS;
    pst_hal_device->us_rx_small_dscr_cnt  = HAL_SMALL_RX_MAX_BUFFS;
}


oal_uint8 dmac_psm_clean_state (oal_void)
{
    oal_uint32                   ul_is_netbuf_empty;
    oal_uint32                   ul_ret;
    extern frw_event_mem_stru   *g_pst_first_rx_event;
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;
    hal_to_dmac_device_stru     *pst_hal_device;

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    dmac_freq_control_stru      *pst_dmac_freq;
    pst_dmac_freq = dmac_get_auto_freq_handle();
    pst_dmac_freq->uc_pm_enable = OAL_FALSE;
#endif

    if(g_pst_first_rx_event)
    {
        FRW_EVENT_FREE(g_pst_first_rx_event);
        g_pst_first_rx_event = OAL_PTR_NULL;
    }

    hcc_slave_suspend();

    /* HAL接口获取支持device个数 */
    ul_ret = hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_DESTROY_RX_DSCR, 0, OAL_PTR_NULL);
        dmac_psm_reset_rx_dscr_default(pst_hal_device);
    }

    /* 检查除oam子池外的三个netbuf子池,深睡前应该这三个子池的netbuf内存都已经释放 */
    ul_is_netbuf_empty = oal_mem_is_netbuf_empty();

    if (ul_is_netbuf_empty != OAL_TRUE)
    {
        OAM_ERROR_LOG0(0,OAM_SF_PWR,"{dmac_psm_clean_state::before deep sleep netbuf is not empty.}");
        OAL_MEM_INFO_PRINT(OAL_MEM_POOL_ID_NETBUF);
    }
#ifndef HI110x_EDA
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    /* 检查rf dev状态 */
    hal_rf_psm_check_state();
#endif
#endif
    return OAL_SUCC;
}


oal_uint8 dmac_psm_save_ps_state (oal_void)
{
    hal_to_dmac_device_stru     *pst_hal_device;
    mac_device_stru             *pst_mac_device;
    oal_uint32                   ul_ret;
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;

    /* HAL接口获取支持device个数 */
    ul_ret = hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        pst_mac_device  = mac_res_get_dev(0);
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        dmac_psm_cpu_save(pst_hal_device);

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT
        dmac_psm_save_dutycycle(pst_hal_device);
#endif
        dmac_psm_save_mac_statistics_data(pst_mac_device, pst_hal_device);
    }

    return OAL_TRUE;
}


oal_void dmac_psm_init_netbuf_pool(oal_void)
{
    oal_uint32 ul_atcm_save = 0;

    ul_atcm_save = oal_mem_is_atcm_need_save();

    if(OAL_TRUE == ul_atcm_save)
    {
        oal_mem_do_create_netbuf_subpool(OAL_MEM_NETBUF_POOL_ID_LARGE_PKT);

        oal_mem_clear_atcm_need_save_flag();
    }
}

oal_void dmac_psm_init_dscr_pool(oal_void)
{
    oal_mem_init_dscr_pool();
}

oal_void  dmac_psm_recover_mac_reg(hal_to_dmac_device_stru  *pst_hal_device)
{
    hal_reset_reg_restore(pst_hal_device, HAL_RESET_HW_TYPE_MAC);
}


oal_void dmac_psm_recover_tcm (hal_to_dmac_device_stru  *pst_hal_device)
{
    hal_reset_reg_restore(pst_hal_device, HAL_RESET_HW_TYPE_TCM);
}


oal_uint8  dmac_psm_recover_powerdown(oal_uint8 uc_dmach0,oal_uint8 uc_dmach1,oal_uint8 uc_dmach2)
{
    hal_to_dmac_device_stru     *pst_hal_device;
    hal_chip_stru               *pst_hal_chip;
    hal_error_state_stru         st_error_state;
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    dmac_freq_control_stru      *pst_dmac_freq;
#endif
    oal_uint8                    uc_device;

    pst_hal_chip = HAL_GET_CHIP_POINTER(0);


    for (uc_device = 0; uc_device < WLAN_DEVICE_MAX_NUM_PER_CHIP; uc_device++)
    {
        /*和TSF同步相关的BANK寄存器恢复*/
        dmac_psm_recover_mac_reg((hal_to_dmac_device_stru*)HAL_GET_DEVICE_POINTER(pst_hal_chip, uc_device));
    }


    /* TSF开始同步 */
    PM_WLAN_Tsf_Aon_to_Inner_start();

    /* 1102 TCM 恢复/1103 mac其它寄存器恢复*/
    for (uc_device = 0; uc_device < WLAN_DEVICE_MAX_NUM_PER_CHIP; uc_device++)
    {
        dmac_psm_recover_tcm((hal_to_dmac_device_stru*)HAL_GET_DEVICE_POINTER(pst_hal_chip, uc_device));
    }

    //restart移动至paldo 开启处
    frw_timer_time_fix();

    frw_timer_restart();

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 低功耗唤醒时，需要判断ps状态，防止中断下半部没有及时到来，此处刷新下ps状态下软件基本配置，暂时只处理主路 TBD */
    pst_hal_device = (hal_to_dmac_device_stru*)HAL_GET_DEVICE_POINTER(pst_hal_chip, HAL_DEVICE_ID_MASTER);

    dmac_btcoex_ps_powerdown_recover_handle(pst_hal_device);
#endif
#endif


#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST))
    dmac_psm_init_netbuf_pool();
#endif

    /* 低功耗上电重新初始化dscr内存池 */
    dmac_psm_init_dscr_pool();

    /*避免复位过程中接收描述符队列异常，重新初始化接收描述符队列*/
    //hal_rx_destroy_dscr_queue(pst_hal_device,OAL_FALSE);

#ifdef _PRE_WLAN_MAC_BUGFIX_TSF_SYNC
    dmac_psm_sync_tsf_to_ap();
#endif

    if(0xff!=uc_dmach0)
    {
      DMAWaitChannelDone(uc_dmach0);
    }

    if(0xff!=uc_dmach1)
    {
      DMAWaitChannelDone(uc_dmach1);
    }
    /* 注册chip级别中断 */
    hal_chip_irq_init(pst_hal_chip);

    /* 清中断状态 */
    st_error_state.ul_error1_val = 0xFFFFFFFF;
    st_error_state.ul_error2_val = 0xFFFFFFFF;

    for (uc_device = 0; uc_device < WLAN_DEVICE_MAX_NUM_PER_CHIP; uc_device++)
    {
        hal_irq_init(&pst_hal_chip->ast_device[uc_device]);

        pst_hal_device = (hal_to_dmac_device_stru*)HAL_GET_DEVICE_POINTER(pst_hal_chip, uc_device);

        hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_INIT_RX_DSCR, 0, OAL_PTR_NULL);

        hal_clear_mac_error_int_status(pst_hal_device, &st_error_state);
        hal_clear_mac_int_status(pst_hal_device, 0xffffffff);
    }


    /* SDIO 重新初始化 */
    hcc_slave_resume();

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
        pst_dmac_freq = dmac_get_auto_freq_handle();
        pst_dmac_freq->uc_pm_enable = OAL_TRUE;
#endif

     /*等待tsf同步完成*/
    PM_WLAN_Aon_Tsf_Sync();

    return OAL_TRUE;

}



oal_void  dmac_psm_recover_start_dma(oal_uint8 *puc_ch0,oal_uint8 *puc_ch1,oal_uint8 *puc_ch2)
{
    hal_to_dmac_device_stru     *pst_hal_device;
    oal_uint32                   ul_ret;
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;

    /* HAL接口获取支持device个数 */
    hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG1(0,OAM_SF_PWR,"{dmac_psm_recover_start_dma::pst_hal_device NULL [%d].}", uc_device);
            continue;
        }

        hal_reset_reg_dma_restore(pst_hal_device,puc_ch0,puc_ch1,puc_ch2);
    }

}


oal_uint8 dmac_psm_recover_no_powerdown (oal_void)
{
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;
    hal_to_dmac_device_stru     *pst_hal_device;
    hal_error_state_stru         st_error_state;
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    dmac_freq_control_stru      *pst_dmac_freq;
#endif
    oal_uint32                   ul_ret;

    PM_WLAN_Tsf_Aon_to_Inner_start();

    //restart移动至paldo 开启处
    frw_timer_time_fix();

    frw_timer_restart();

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 低功耗唤醒时，需要判断ps状态，防止中断下半部没有及时到来，此处刷新下ps状态下软件基本配置，暂时只处理主路 TBD */
    pst_hal_device = (hal_to_dmac_device_stru*)HAL_GET_DEVICE_POINTER(HAL_GET_CHIP_POINTER(0), HAL_DEVICE_ID_MASTER);

    dmac_btcoex_ps_powerdown_recover_handle(pst_hal_device);
#endif
#endif

    /* g_st_netbuf_pool不下电, 重新初始化内存池时需清零 */
    //OAL_MEMZERO(&g_st_netbuf_pool, OAL_SIZEOF(g_st_netbuf_pool));

    //dmac_psm_init_netbuf_pool();

    /*避免复位过程中接收描述符队列异常，重新初始化接收描述符队列*/
    //hal_rx_destroy_dscr_queue(pst_hal_device,OAL_FALSE);


     /* 清中断状态 */
    st_error_state.ul_error1_val = 0xFFFFFFFF;
    st_error_state.ul_error2_val = 0xFFFFFFFF;

    /* HAL接口获取CHIP0支持device个数 */
    hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG1(0,OAM_SF_PWR,"{dmac_psm_recover_powerdown::pst_hal_device NULL [%d].}", uc_device);
            continue;
        }

        /* 低功耗不下电唤醒后清除rx中断,防止rx中断里保留上一次睡前的值造成rx new */
        hal_psm_clear_mac_rx_isr(pst_hal_device);

        hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_INIT_RX_DSCR, 0, OAL_PTR_NULL);

        hal_clear_mac_error_int_status(pst_hal_device, &st_error_state);
        hal_clear_mac_int_status(pst_hal_device, 0xffffffff);
    }

    /* SDIO 重新初始化 */
    hcc_slave_resume();

#ifdef _PRE_WLAN_MAC_BUGFIX_TSF_SYNC
    dmac_psm_sync_tsf_to_ap();
#endif

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    pst_dmac_freq = dmac_get_auto_freq_handle();
    pst_dmac_freq->uc_pm_enable = OAL_TRUE;
#endif

    /*等待tsf同步完成*/
    PM_WLAN_Aon_Tsf_Sync();

    return OAL_TRUE;

}
#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)

oal_uint8 dmac_psm_cbb_stopwork (oal_void)
{
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;
    oal_uint32                   ul_ret;
    hal_to_dmac_device_stru     *pst_hal_device;

    /* HAL接口获取CHIP0支持device个数 */
    hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "{dmac_psm_cbb_stopwork::get_hal_device fail[%d].}", uc_device);
            continue;
        }

        /* 关闭pa */
        hal_disable_machw_phy_and_pa(pst_hal_device);
    }

    return OAL_TRUE;
}


oal_void dmac_psm_rf_sleep(oal_uint8 uc_restore_reg)
{
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;
    oal_uint32                   ul_ret;
    hal_to_dmac_device_stru     *pst_hal_device;

    /* HAL接口获取CHIP0支持device个数 */
    hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "{dmac_psm_rf_sleep::get_hal_device fail[%d].}", uc_device);
            continue;
        }

        hal_psm_rf_sleep(pst_hal_device, uc_restore_reg);
    }

    return;
}

oal_void dmac_psm_rf_awake (oal_uint8 uc_restore_reg)
{
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;
    oal_uint32                   ul_ret;
    hal_to_dmac_device_stru     *pst_hal_device;

    /* HAL接口获取CHIP0支持device个数 */
    hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "{dmac_psm_rf_awake::get_hal_device fail[%d].}", uc_device);
            continue;
        }

        hal_psm_rf_awake(pst_hal_device,uc_restore_reg);
        hal_psm_recover_primary_channel(pst_hal_device);
    }

    return;
}
#endif


oal_bool_enum_uint8 dmac_check_all_sleep_time(oal_void)
{
    oal_uint8                    uc_device_max;
    oal_uint8                    uc_device;
    oal_uint32                   ul_ret;
    hal_to_dmac_device_stru     *pst_hal_device;

    /* HAL接口获取CHIP0支持device个数 */
    hal_chip_get_device_num(0, &uc_device_max);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
        if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "{dmac_check_all_sleep_time::get_hal_device fail[%d].}", uc_device);
            continue;
        }

        if (OAL_FALSE == hal_device_check_all_sleep_time(pst_hal_device))
        {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}



#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

