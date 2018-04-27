


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_ext_if.h"
#include "mac_ie.h"
#include "dmac_main.h"
#include "dmac_alg.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_chan_mgmt.h"
#include "dmac_scan.h"
#include "oam_ext_if.h"
#include "dmac_beacon.h"
#include "dmac_mgmt_sta.h"
#include "dmac_mgmt_ap.h"
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#endif
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#include "dmac_bsd.h"
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE
#include "dmac_radar.h"
#endif
#include "dmac_csa_sta.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CHAN_MGMT_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_chan_mgmt_rom_cb  g_st_dmac_chan_mgmt_rom_cb  = {OAL_PTR_NULL,
                                                      OAL_PTR_NULL};


/*****************************************************************************
  3 函数实现
*****************************************************************************/
oal_uint32 dmac_dump_chan(mac_vap_stru *pst_mac_vap, oal_uint8* puc_param)
{
    dmac_set_chan_stru *pst_chan;
    oal_uint8 uc_vap_id;

    if ( (OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == puc_param) )
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_vap_id = pst_mac_vap->uc_vap_id;
    pst_chan = (dmac_set_chan_stru*)puc_param;
    OAM_INFO_LOG4(uc_vap_id, OAM_SF_2040,
                "chan_number=%d band=%d bandwidth=%d idx=%d\r\n",
                pst_chan->st_channel.uc_chan_number, pst_chan->st_channel.en_band,
                pst_chan->st_channel.en_bandwidth, pst_chan->st_channel.uc_chan_idx);

    OAM_INFO_LOG4(uc_vap_id, OAM_SF_2040,
                "announced_channel=%d announced_bandwidth=%d ch_switch_cnt=%d ch_switch_status=%d\r\n",
                pst_chan->st_ch_switch_info.uc_announced_channel, pst_chan->st_ch_switch_info.en_announced_bandwidth,
                pst_chan->st_ch_switch_info.uc_ch_switch_cnt, pst_chan->st_ch_switch_info.en_ch_switch_status);

    OAM_INFO_LOG4(uc_vap_id, OAM_SF_2040,
                "bw_switch_status=%d csa_present_in_bcn=%d start_chan_idx=%d end_chan_idx=%d\r\n",
                pst_chan->st_ch_switch_info.en_bw_switch_status, pst_chan->st_ch_switch_info.en_csa_present_in_bcn,
                pst_chan->st_ch_switch_info.uc_start_chan_idx, pst_chan->st_ch_switch_info.uc_end_chan_idx);

   OAM_INFO_LOG3(uc_vap_id, OAM_SF_2040,
               "user_pref_bandwidth=%d new_channel=%d new_bandwidth=%d\r\n",
               pst_chan->st_ch_switch_info.en_user_pref_bandwidth,
               pst_chan->st_ch_switch_info.uc_new_channel, pst_chan->st_ch_switch_info.en_new_bandwidth);

   /* OAM_INFO_LOG4(uc_vap_id, OAM_SF_2040,
                "en_user_pref_bandwidth=%d en_bw_change=%d uc_new_channel=%d en_new_bandwidth=%d\r\n",
                pst_chan->st_ch_switch_info.en_user_pref_bandwidth, pst_chan->st_ch_switch_info.en_bw_change,
                pst_chan->st_ch_switch_info.uc_new_channel, pst_chan->st_ch_switch_info.en_new_bandwidth);*/

   OAM_INFO_LOG2(uc_vap_id, OAM_SF_2040,
                "sta_csa_last_cnt=%d current_status =%d\r\n",
                pst_mac_vap->st_sta_csa_fsm_info.uc_sta_csa_last_cnt, dmac_sta_csa_fsm_get_current_state(pst_mac_vap));


    OAM_INFO_LOG4(uc_vap_id, OAM_SF_2040,
                "te_b=%d chan_report_for_te_a=%d switch_immediately=%d check_cac=%d\r\n",
                pst_chan->st_ch_switch_info.en_te_b, pst_chan->st_ch_switch_info.ul_chan_report_for_te_a,
                pst_chan->en_switch_immediately, pst_chan->en_check_cac);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_DFS

oal_uint32  dmac_mgmt_scan_dfs_timeout(void *p_arg)
{
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = (hal_to_dmac_device_stru *)p_arg;

    /* 使能雷达检测 */
    hal_enable_radar_det(pst_hal_device, 1);

    return OAL_SUCC;
}


oal_void  dmac_dfs_radar_detect_check(hal_to_dmac_device_stru *pst_hal_device, mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap)
{
    oal_bool_enum_uint8             en_enable_dfs;

    /* 使能雷达检测 */
    hal_enable_radar_det(pst_hal_device, 1);

    en_enable_dfs = mac_is_ch_in_radar_band(pst_mac_device->en_max_band, pst_mac_vap->st_channel.uc_chan_idx);
    if (0 != pst_mac_device->us_dfs_timeout && OAL_TRUE ==en_enable_dfs)
    {
        hal_enable_radar_det(pst_hal_device, 0);
        /* 启动定时器 */
        FRW_TIMER_CREATE_TIMER(&pst_mac_device->st_dfs.st_dfs_radar_timer,
                               dmac_mgmt_scan_dfs_timeout,
                               pst_mac_device->us_dfs_timeout,
                               pst_hal_device,
                               OAL_FALSE,
                               OAM_MODULE_ID_DMAC,
                               pst_mac_device->ul_core_id);
    }
    else
    {
        hal_enable_radar_det(pst_hal_device, en_enable_dfs);
    }
}
#endif


oal_void dmac_chan_select_real_channel(mac_device_stru  *pst_mac_device, mac_channel_stru *pst_channel, oal_uint8 uc_dst_chan_num)
{
    oal_uint8                uc_vap_idx;
    mac_vap_stru            *pst_vap;
    oal_bool_enum_uint8      en_all_same_channel = OAL_TRUE;
    oal_bool_enum_uint8      en_found_channel = OAL_FALSE;
    mac_channel_stru         st_real_channel;
    wlan_channel_bandwidth_enum_uint8 max_en_bandwidth = WLAN_BAND_WIDTH_20M;

    if((OAL_PTR_NULL == pst_channel) || (OAL_PTR_NULL == pst_mac_device))
    {
        OAM_WARNING_LOG2(0, OAM_SF_FRAME_FILTER, "{dmac_chan_select_real_channel has null point:pst_channel=%p pst_mac_device=%p}",pst_channel,pst_mac_device);
        return;
    }

    oal_memset(&st_real_channel, 0, sizeof(mac_channel_stru));

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "vap is null, vap id is %d", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if ((MAC_VAP_STATE_UP == pst_vap->en_vap_state)
#ifdef _PRE_WLAN_FEATURE_DBAC
            || ((mac_is_dbac_running(pst_mac_device)
                  && WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode))
#endif
            || (MAC_VAP_STATE_PAUSE == pst_vap->en_vap_state)
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (MAC_VAP_STATE_ROAMING == pst_vap->en_vap_state)
#endif //_PRE_WLAN_FEATURE_ROAM
            || (MAC_VAP_STATE_STA_LISTEN == pst_vap->en_vap_state && pst_vap->us_user_nums > 0)
            || (MAC_VAP_STATE_AP_WAIT_START == pst_vap->en_vap_state))
        {
            if (pst_vap->st_channel.uc_chan_number != uc_dst_chan_num)
            {
               en_all_same_channel = OAL_FALSE;
               break;
            }

            if ((pst_vap->st_channel.en_bandwidth >= max_en_bandwidth) && (pst_vap->st_channel.en_bandwidth != WLAN_BAND_WIDTH_BUTT))
            {
                st_real_channel = pst_vap->st_channel;
                en_found_channel = OAL_TRUE;
                max_en_bandwidth = pst_vap->st_channel.en_bandwidth;
            }
        }
    }

    if ((OAL_TRUE == en_all_same_channel) && (OAL_TRUE == en_found_channel))
    {
        *pst_channel = st_real_channel;
    }
}


oal_void  dmac_chan_restart_network_after_switch(mac_device_stru *pst_mac_device, dmac_vap_stru *pst_dmac_vap)
{
    /* 在新信道上恢复Beacon帧的发送 */
    hal_vap_beacon_resume(pst_dmac_vap->pst_hal_vap);

    /* 在新信道上恢复硬件的发送 */
    hal_set_machw_tx_resume(pst_dmac_vap->pst_hal_device);

   // OAM_INFO_LOG0(0, OAM_SF_ANY, "Transmitter is enabled!");
}


oal_uint32  dmac_chan_restart_network_after_switch_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru       *pst_event;
    frw_event_hdr_stru   *pst_event_hdr;
    mac_device_stru      *pst_mac_device;
    dmac_vap_stru        *pst_dmac_vap;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_chan_restart_network_after_switch_event::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event     = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);

    /* 获取dmac vap结构的信息 */
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_SCAN, "{dmac_chan_restart_network_after_switch_event::pst_dmac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取device结构的信息 */
    pst_mac_device   = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_chan_restart_network_after_switch_event::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    dmac_chan_restart_network_after_switch(pst_mac_device, pst_dmac_vap);

    /* mayuan TBD 这里看要不要调用此函数 */
    //dmac_vap_resume_tx_by_chl(pst_mac_device, &(pst_dmac_vap->st_vap_base_info.st_channel));

    return OAL_SUCC;
}


oal_void  dmac_switch_complete_notify(mac_vap_stru *pst_mac_vap,
                                                    oal_bool_enum_uint8 en_check_cac)
{
    frw_event_mem_stru   *pst_event_mem;
    frw_event_stru       *pst_event;
    oal_uint32            ul_ret;
    dmac_set_chan_stru   *pst_set_chan;

    #ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    dmac_vap_stru        *pst_dmac_vap;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{dmac_switch_complete_notify::pst_dmac_vap null.}");
        return;
    }

    /* 带宽已成功切换到40M, 停止40M恢复定时器 */
    if ((pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M) &&
         (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode))
    {
        dmac_chan_stop_40M_recovery_timer(pst_dmac_vap);
    }
    #endif

    /* 申请事件内存 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_set_chan_stru));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{dmac_switch_complete_notify::pst_event_mem null.}");

        return;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                    FRW_EVENT_TYPE_WLAN_CRX,
                    DMAC_WLAN_CRX_EVENT_SUB_TYPR_CH_SWITCH_COMPLETE,
                    OAL_SIZEOF(dmac_set_chan_stru),
                    FRW_EVENT_PIPELINE_STAGE_1,
                    pst_mac_vap->uc_chip_id,
                    pst_mac_vap->uc_device_id,
                    pst_mac_vap->uc_vap_id);

    pst_set_chan = (dmac_set_chan_stru *)pst_event->auc_event_data;
    oal_memcopy(&pst_set_chan->st_channel, &pst_mac_vap->st_channel, OAL_SIZEOF(mac_channel_stru));
    oal_memcopy(&pst_set_chan->st_ch_switch_info, &pst_mac_vap->st_ch_switch_info,
                OAL_SIZEOF(mac_ch_switch_info_stru));
    pst_set_chan->en_check_cac = en_check_cac;

    //OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{dmac_switch_complete_notify}");
    dmac_dump_chan(pst_mac_vap, (oal_uint8*)pst_set_chan);

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                       "{dmac_switch_complete_notify::frw_event_dispatch_event failed[%d].}", ul_ret);
        FRW_EVENT_FREE(pst_event_mem);

        return;
    }

    /* 释放事件 */
    FRW_EVENT_FREE(pst_event_mem);
}

#ifdef _PRE_WLAN_FEATURE_DFS
oal_uint32  dmac_dfs_test(frw_event_mem_stru* pst_dmac_event_mem)
{
    frw_event_mem_stru              *pst_event_mem;
    frw_event_stru                  *pst_event_desc;
    hal_radar_det_event_stru        *pst_radar_det_info;
    oal_uint8 uc_chip_id;
    oal_uint8 uc_device_id;
    oal_uint8 uc_vap_id;

    pst_event_desc = frw_get_event_stru(pst_dmac_event_mem);
    uc_chip_id   = pst_event_desc->st_event_hdr.uc_chip_id;
    uc_device_id = pst_event_desc->st_event_hdr.uc_device_id;
    uc_vap_id    = pst_event_desc->st_event_hdr.uc_vap_id;

    /* 注意: 雷达事件的具体内容在中断下半部读取 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(hal_radar_irq_reg_list_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
       // HAL_ERR_LOG(0, "hi1102_irq_radar_isr: alloc pst_event_mem failed!");
        OAM_ERROR_LOG0(0, OAM_SF_IRQ, "{dmac_dfs_test::alloc pst_event_mem failed.}\r\n");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event_desc = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event_desc->st_event_hdr),
                       FRW_EVENT_TYPE_DMAC_MISC,
                       HAL_EVENT_DMAC_MISC_RADAR_DETECTED,
                       OAL_SIZEOF(hal_radar_irq_reg_list_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       uc_chip_id,
                       uc_device_id,
                       uc_vap_id );


    /* 读取雷达控制寄存器 */
    pst_radar_det_info = (hal_radar_det_event_stru *)(pst_event_desc->auc_event_data);
    pst_radar_det_info->uc_radar_type = 1;

    /* 事件分发 */
    frw_event_dispatch_event(pst_event_mem);

#ifndef _PRE_WLAN_FEATURE_MEM_OPT
    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);
#endif

    return OAL_SUCC;
}
#endif

oal_void  dmac_chan_disable_machw_tx(mac_vap_stru *pst_mac_vap)
{
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_chan_disable_machw_tx::pst_hal_device null}");
        return;
    }

    /* 禁止硬件发送 */
    hal_set_machw_tx_suspend(pst_hal_device);

    /* 禁止硬件回ack */
    hal_disable_machw_ack_trans(pst_hal_device);

    /* 禁止硬件回cts */
    hal_disable_machw_cts_trans(pst_hal_device);

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_chan_disable_machw_tx::tx disabled.}");
}


oal_void  dmac_chan_enable_machw_tx(mac_vap_stru *pst_mac_vap)
{
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_chan_enable_machw_tx::pst_hal_device null}");
        return;
    }

    /* 恢复硬件发送 */
    hal_set_machw_tx_resume(pst_hal_device);

    /* 恢复硬件回ack */
    hal_enable_machw_ack_trans(pst_hal_device);

    /* 恢复硬件回cts */
    hal_enable_machw_cts_trans(pst_hal_device);

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_chan_enable_machw_tx::tx enabled.}");
}


oal_uint32  dmac_chan_disable_machw_tx_event_process(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru           *pst_event;
    frw_event_hdr_stru       *pst_event_hdr;
    mac_vap_stru             *pst_mac_vap;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_chan_disable_machw_tx_event_process::pst_event_mem null.}");


        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event     = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);

    pst_mac_vap  = (mac_vap_stru *)mac_res_get_mac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_chan_disable_machw_tx_event_process::pst_mac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 禁止硬件的全部发送 */
    dmac_chan_disable_machw_tx(pst_mac_vap);

    return OAL_SUCC;
}


oal_uint32  dmac_chan_enable_machw_tx_event_process(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru           *pst_event;
    frw_event_hdr_stru       *pst_event_hdr;
    mac_vap_stru             *pst_mac_vap;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_chan_enable_machw_tx_event_process::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event     = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);

    pst_mac_vap   = (mac_vap_stru *)mac_res_get_mac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_TX, "{dmac_chan_enable_machw_tx_event_process::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 恢复硬件发送 */
    dmac_chan_enable_machw_tx(pst_mac_vap);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_DFS
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_void  dmac_chan_tx_complete_suspend_tx(mac_device_stru           *pst_mac_device,
                                           mac_vap_stru              *pst_mac_vap,
                                           hal_to_dmac_device_stru   *pst_hal_device,
                                           oal_netbuf_stru           *pst_netbuf)
{
    oal_uint8     *puc_payload;

    /* 当Channel Switch Announcement帧发送后，需要禁止硬件发送 */
    if (mac_ieeee80211_is_action(oal_netbuf_header(pst_netbuf)))
    {
        puc_payload = (oal_uint8 *)oal_netbuf_data(pst_netbuf);

        if ((MAC_ACTION_CATEGORY_SPECMGMT == puc_payload[0]) && (MAC_SPEC_CH_SWITCH_ANNOUNCE == puc_payload[1]))
        {
            pst_mac_device->uc_csa_vap_cnt--;

            /* 当device下所有running AP的CSA帧都发送完成后，挂起硬件发送 */
            if (0 == pst_mac_device->uc_csa_vap_cnt)
            {
                //OAM_INFO_LOG0(0, OAM_SF_DFS, "{dmac_chan_tx_complete_suspend_tx::machw tx suspend.}\r\n");

                /* 挂起硬件发送 */
                hal_set_machw_tx_suspend(pst_hal_device);

                if (mac_dfs_get_debug_level(pst_mac_device) & 0x1)
                {

                    //OAM_INFO_LOG1(0, OAM_SF_DFS, "{dmac_chan_tx_complete_suspend_tx::chan shutdown time(ms): %d.}", ul_delta_time_for_chan_shutdown);
                }
            }
        }
    }
}
#else

oal_void  dmac_chan_tx_complete_suspend_tx(mac_device_stru           *pst_mac_device,
                                           mac_vap_stru              *pst_mac_vap,
                                           hal_to_dmac_device_stru   *pst_hal_device,
                                           oal_netbuf_stru           *pst_netbuf)
{
    oal_uint8     *puc_payload;
    oal_uint32     ul_chan_shutdown_time;
    oal_uint32     ul_delta_time_for_chan_shutdown;

    /* 当Channel Switch Announcement帧发送后，需要禁止硬件发送 */
    if (mac_ieeee80211_is_action(oal_netbuf_data(pst_netbuf)))
    {
        puc_payload = (oal_uint8 *)oal_netbuf_data(pst_netbuf) + MAC_80211_FRAME_LEN;

        if ((MAC_ACTION_CATEGORY_SPECMGMT == puc_payload[0]) && (MAC_SPEC_CH_SWITCH_ANNOUNCE == puc_payload[1]))
        {
            pst_mac_device->uc_csa_vap_cnt--;

            /* 当device下所有running AP的CSA帧都发送完成后，挂起硬件发送 */
            if (0 == pst_mac_device->uc_csa_vap_cnt)
            {
                //OAM_INFO_LOG0(0, OAM_SF_DFS, "{dmac_chan_tx_complete_suspend_tx::machw tx suspend.}\r\n");

                /* 挂起硬件发送 */
                hal_set_machw_tx_suspend(pst_hal_device);

                if (mac_dfs_get_debug_level(pst_mac_device) & 0x1)
                {
                    ul_chan_shutdown_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();
                    ul_delta_time_for_chan_shutdown = (oal_uint32)OAL_TIME_GET_RUNTIME(pst_mac_device->st_dfs.st_dfs_info.ul_radar_detected_timestamp, ul_chan_shutdown_time);

                    //OAM_INFO_LOG1(0, OAM_SF_DFS, "{dmac_chan_tx_complete_suspend_tx::chan shutdown time(ms): %d.}", ul_delta_time_for_chan_shutdown);
                }
            }
        }
    }
}
#endif
oal_void dmac_dfs_radar_detect_log(mac_device_stru *pst_mac_device, hal_radar_det_event_stru *pst_radar_info, oal_uint8 uc_vap_id)
{
    dmac_device_stru               *pst_dmac_device;

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_dfs_radar_detect_log: pst dmac device null pointer! mac dev id:%d}", pst_mac_device->uc_device_id);
        return;
    }

    OAM_WARNING_LOG2(uc_vap_id, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::[DFS]radar detected %u, radar type = %d!", pst_dmac_device->ul_dfs_cnt, pst_radar_info->uc_radar_type);
    OAM_WARNING_LOG4(uc_vap_id, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::[DFS]radar_freq_offset=%d,radar_bw=%d,band=%d,channel_num=%d,working_bw=%d!",
                       pst_radar_info->uc_radar_freq_offset, pst_radar_info->uc_radar_bw,pst_radar_info->uc_band,pst_radar_info->uc_working_bw);
    #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    // forgive me, 51 use this to check radar
    OAL_IO_PRINT("{dmac_dfs_radar_detect_event::[DFS]radar detected %u, radar type = %d! \n",
                   pst_dmac_device->ul_dfs_cnt, pst_dmac_device->past_hal_device[0]->uc_radar_type);
    #endif

    pst_dmac_device->ul_dfs_cnt++;
    return;
}


oal_uint32 dmac_dfs_radar_detect_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    mac_device_stru            *pst_mac_device;
    hal_to_dmac_device_stru    *pst_hal_device;
    mac_vap_stru               *pst_mac_vap = OAL_PTR_NULL;
    hal_radar_det_event_stru   *pst_radar_det_info;
    oal_uint8                   uc_vap_idx;
    oal_bool_enum_uint8         en_found_running_ap  = OAL_FALSE;
    oal_bool_enum_uint8         en_found_starting_ap = OAL_FALSE;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::pst_mac_device is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_radar_det_info = (hal_radar_det_event_stru *)pst_event->auc_event_data;

    if (g_st_dmac_chan_mgmt_rom_cb.p_dmac_dfs_radar_detect_event)
    {
        if (OAL_RETURN == g_st_dmac_chan_mgmt_rom_cb.p_dmac_dfs_radar_detect_event(pst_event_mem))
        {
            return OAL_SUCC;
        }
    }

#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE
    if (OAL_TRUE == dmac_radar_filter(pst_mac_device, pst_radar_det_info))
    {
        return OAL_SUCC;
    }
#endif

    /* 如果雷达检测没使能，则直接返回 */
    if (OAL_FALSE == mac_dfs_get_dfs_enable(pst_mac_device))
    {
        OAM_WARNING_LOG0(0, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::[DFS]dfs is disabled.");
        return OAL_SUCC;
    }

    /* 找一个running AP */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
       pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
       if ((OAL_PTR_NULL != pst_mac_vap)                      &&
           (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode) &&
           (MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state))
       {
           en_found_running_ap = OAL_TRUE;
           break;
       }
    }

    /* 没找到 running AP，寻找是否有正在 start 的AP */
    if (OAL_FALSE == en_found_running_ap)
    {
       for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
       {
           pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
           if ((OAL_PTR_NULL != pst_mac_vap)                      &&
               (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode) &&
               (MAC_VAP_STATE_AP_WAIT_START == pst_mac_vap->en_vap_state))
           {
               en_found_starting_ap = OAL_TRUE;
               break;
           }
       }
    }

    /* 既没有正在 running 的 AP，也没有正在 start 的 AP，因此无需对雷达信道做出响应 */
    if (((OAL_FALSE == en_found_running_ap) && (OAL_FALSE == en_found_starting_ap)) ||
       (OAL_PTR_NULL == pst_mac_vap))
    {
       OAM_WARNING_LOG0(0, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::no running ap,radar event ignor.");
       return OAL_SUCC;
    }

    if(OAL_TRUE != mac_vap_get_dfs_enable(pst_mac_vap))
    {
       OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::vap [DFS] DISABLE.");
       return OAL_SUCC;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
       OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_dfs_radar_detect_event::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
       return OAL_ERR_CODE_PTR_NULL;
    }

    /* 冗余检测，防止硬件误报，非雷达信道不应该产生雷达中断 且产生雷达中断的信道号与当前信道号不一致时也不上报*/
    if(pst_radar_det_info->uc_flag != RADAR_INFO_FLAG_DUMMY)
    {
        if((pst_radar_det_info->uc_band < MAC_RC_START_FREQ_5) ||
            (OAL_FALSE == mac_is_dfs_channel(pst_radar_det_info->uc_band, pst_radar_det_info->uc_channel_num)))
        {
            OAM_WARNING_LOG1(0, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::[DFS]non radar channel[%d] should not generate radar event.",pst_radar_det_info->uc_channel_num);
            return OAL_SUCC;
        }

        if(pst_radar_det_info->uc_channel_num != pst_hal_device->uc_current_chan_number)
        {
            OAM_WARNING_LOG2(0, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::radar irq generate by channel[%d] which is not current channel[%d].",pst_radar_det_info->uc_channel_num,pst_mac_device->uc_max_channel);
            OAL_IO_PRINT("{dmac_dfs_radar_detect_event::radar irq generate by channel[%d] which is not current channel[%d].",pst_radar_det_info->uc_channel_num,pst_mac_device->uc_max_channel);
            return OAL_SUCC;
        }

        /*产生雷达中断的信道号与处于up或者wait start状态 vap的home信道号不一致时也不上报*/
        if (pst_radar_det_info->uc_channel_num != pst_mac_vap->st_channel.uc_chan_number)
        {
            OAM_WARNING_LOG2(0, OAM_SF_DFS, "{dmac_dfs_radar_detect_event::radar irq generate by channel[%d] which is not home channel[%d].",pst_radar_det_info->uc_channel_num, pst_mac_vap->st_channel.uc_chan_number);
            return OAL_SUCC;
        }
    }

    if (mac_dfs_get_debug_level(pst_mac_device) & 0x1)
    {
        dmac_dfs_radar_detect_log(pst_mac_device, pst_radar_det_info, pst_event->st_event_hdr.uc_vap_id);
        /* 记录雷达检测时间戳 */
        pst_mac_device->st_dfs.st_dfs_info.ul_radar_detected_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    }

    if (mac_dfs_get_debug_level(pst_mac_device) & 0x2)
    {
        dmac_dfs_radar_detect_log(pst_mac_device, pst_radar_det_info, pst_event->st_event_hdr.uc_vap_id);
        /*使能雷达检测，不使能切换信道*/
#if (_PRE_TEST_MODE_UT != _PRE_TEST_MODE)
        return OAL_SUCC;
#endif
    }


    /* 关闭硬件雷达检测 */
    hal_enable_radar_det(pst_hal_device, OAL_FALSE);

    /* 更新事件头中的VAP ID */
    pst_event->st_event_hdr.uc_vap_id = pst_mac_vap->uc_vap_id;

    FRW_EVENT_HDR_MODIFY_PIPELINE_AND_SUBTYPE(&(pst_event->st_event_hdr), DMAC_MISC_SUB_TYPE_RADAR_DETECT);

    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);

    return OAL_SUCC;
}

#endif

oal_uint32  dmac_ie_proc_wide_bandwidth_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload)
{
    oal_uint8     uc_new_channel;
    oal_uint8     uc_channel_width;
    oal_uint8     uc_channel_center_freq_seg0;
    oal_uint32    ul_check;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == puc_payload)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_ie_proc_wide_bandwidth_ie::param null,pst_mac_vap[%p], puc_payload[%p].}",pst_mac_vap, puc_payload);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 非切换信道过程返回 */
    if (OAL_FALSE == dmac_sta_csa_is_in_waiting(pst_mac_vap))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_wide_bandwidth_ie::not waiting shit channel,csa_status=%d}",
            dmac_sta_csa_fsm_get_current_state(pst_mac_vap));

        return OAL_SUCC;
    }

    uc_new_channel = pst_mac_vap->st_ch_switch_info.uc_new_channel;

    /* 检查当前管制域是否支持该信道，如果不支持，则直接返回 */
    ul_check = mac_is_channel_num_valid(pst_mac_vap->st_channel.en_band, uc_new_channel);
    if (OAL_SUCC != ul_check)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_wide_bandwidth_ie::mac_is_channel_num_valid failed[%d], uc_new_chan=%d.}",
                         ul_check, uc_new_channel);
        return ul_check;
    }

    /* IE长度检查 */
    if (puc_payload[1] < MAC_WIDE_BW_CH_SWITCH_IE_LEN)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_wide_bandwidth_ie::invalid wide bw ch switch ie len[%d]}", puc_payload[1]);
        return OAL_FAIL;
    }

    uc_channel_width            = puc_payload[MAC_IE_HDR_LEN];
    uc_channel_center_freq_seg0 = puc_payload[MAC_IE_HDR_LEN + 1];

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_wide_bandwidth_ie:: channel_width=[%d] seg0=[%d]}",
        uc_channel_width,uc_channel_center_freq_seg0);
    // 2. 对于80M切换要区分fpga/asic
    if (WLAN_MIB_VHT_OP_WIDTH_20_40 < uc_channel_width)
    {
        pst_mac_vap->st_ch_switch_info.en_new_bandwidth = mac_get_bandwith_from_center_freq_seg0(uc_channel_width, uc_new_channel, uc_channel_center_freq_seg0);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_wide_bandwidth_ie::csa get new_bw=[%d] from MAC_EID_WIDE_BW_CH_SWITCH(IE194)}",
            pst_mac_vap->st_ch_switch_info.en_new_bandwidth);
    }
    else
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_wide_bandwidth_ie::unused vht channel with[%d]}",
                         uc_channel_width);
    }
    return OAL_SUCC;
}


oal_uint32  dmac_ie_proc_csa_bw_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len)
{
    oal_uint8          *puc_ie                = OAL_PTR_NULL;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == puc_payload)))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CSA, "{dmac_ie_proc_csa_bw_ie::param null,pst_mac_vap[%p], puc_payload[%p].}",pst_mac_vap, puc_payload);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 非切换信道过程返回 */
    if (OAL_FALSE == dmac_sta_csa_is_in_waiting(pst_mac_vap))
    {
        return OAL_SUCC;
    }

    puc_ie = mac_find_ie(MAC_EID_SEC_CH_OFFSET, puc_payload, us_frame_len);
    if ((OAL_PTR_NULL != puc_ie) && (puc_ie[1] >= MAC_SEC_CH_OFFSET_IE_LEN))
    {
        pst_mac_vap->st_ch_switch_info.en_new_bandwidth = mac_get_bandwidth_from_sco(puc_ie[MAC_IE_HDR_LEN]);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_csa_bw_ie::csa get new_bw=[%d] from MAC_EID_SEC_CH_OFFSET(IE62)}",
        pst_mac_vap->st_ch_switch_info.en_new_bandwidth);
    }

    puc_ie = mac_find_ie(MAC_EID_WIDE_BW_CH_SWITCH, puc_payload, us_frame_len);
    if ((OAL_PTR_NULL != puc_ie) && (OAL_TRUE == mac_mib_get_VHTOptionImplemented(pst_mac_vap)))
    {
        dmac_ie_proc_wide_bandwidth_ie(pst_mac_vap, puc_ie);
    }

    return OAL_SUCC;
}


oal_uint32  dmac_ie_proc_ch_switch_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, mac_eid_enum_uint8 en_eid_type)
{
    oal_uint8    uc_ch_sw_mode = 0;
    oal_uint8    uc_new_chan   = 0;
    oal_uint8    uc_sw_cnt     = 0;
    oal_uint32   ul_check      = OAL_FAIL;
    oal_ieee80211_channel_sw_ie st_csa_info;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == puc_payload)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_ie_proc_ch_switch_ie::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (g_st_dmac_chan_mgmt_rom_cb.p_dmac_ie_proc_ch_switch_ie)
    {
        if (OAL_RETURN == (g_st_dmac_chan_mgmt_rom_cb.p_dmac_ie_proc_ch_switch_ie)(pst_mac_vap, puc_payload, en_eid_type))
        {
            return OAL_SUCC;
        }
    }

    /*************************************************************************/
    /*                    Channel Switch Announcement element                */
    /* --------------------------------------------------------------------- */
    /* |Element ID|Length |Channel switch Mode|New Channel| Ch switch count| */
    /* --------------------------------------------------------------------- */
    /* |1         |1      |1                  |1          |1               | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*                Extended Channel Switch Announcement element           */
    /* --------------------------------------------------------------------- */
    /* |Elem ID|Length|Ch Switch Mode|New Reg Class|New Ch| Ch switch count| */
    /* --------------------------------------------------------------------- */
    /* |1      |1     |1             |1            |1     |1               | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    if (MAC_EID_CHANSWITCHANN == en_eid_type)
    {
        if (puc_payload[1] < MAC_CHANSWITCHANN_IE_LEN)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_ie_proc_ch_switch_ie::invalid chan switch ann ie len[%d]}", puc_payload[1]);
            return OAL_FAIL;
        }
        /* Channel Switch Announcement element */
        uc_ch_sw_mode = puc_payload[MAC_IE_HDR_LEN];
        uc_new_chan   = puc_payload[MAC_IE_HDR_LEN + 1];
        uc_sw_cnt     = puc_payload[MAC_IE_HDR_LEN + 2];
    }
    else if (MAC_EID_EXTCHANSWITCHANN == en_eid_type)
    {
        if (puc_payload[1] < MAC_EXT_CHANSWITCHANN_IE_LEN)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_ie_proc_ch_switch_ie::invalid ext chan switch ann ie len[%d]}", puc_payload[1]);
            return OAL_FAIL;
        }
        /* Extended Channel Switch Announcement element */
        uc_ch_sw_mode = puc_payload[MAC_IE_HDR_LEN];
        /* Skip New Operating Class = puc_payload[MAC_IE_HDR_LEN + 1]; */
        uc_new_chan   = puc_payload[MAC_IE_HDR_LEN + 2];
        uc_sw_cnt     = puc_payload[MAC_IE_HDR_LEN + 3];
    }
    else
    {
        return OAL_FAIL;
    }

    if((WLAN_STA_CSA_FSM_INVALID != dmac_sta_csa_fsm_get_current_state(pst_mac_vap)) || (uc_sw_cnt != pst_mac_vap->st_sta_csa_fsm_info.uc_sta_csa_last_cnt))
    {
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_ch_switch_ie::csa info  chan=%d mode=%d cnt=%d currt_channel=%d}",
                  uc_new_chan, uc_ch_sw_mode,uc_sw_cnt, pst_mac_vap->st_channel.uc_chan_number);
    }

    /* 检查当前管制域是否支持该信道，如果不支持，则直接返回 */
    ul_check = mac_is_channel_num_valid(pst_mac_vap->st_channel.en_band, uc_new_chan);
    if (OAL_SUCC != ul_check)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_ch_switch_ie::mac_is_channel_num_valid failed[%d], uc_new_chan=%d.}",
                      ul_check, uc_sw_cnt);
        return ul_check;
    }
    if(pst_mac_vap->st_channel.uc_chan_number != uc_new_chan)
    {
        st_csa_info.new_ch_num     = uc_new_chan;
        st_csa_info.mode           = uc_ch_sw_mode;
        st_csa_info.count          = uc_sw_cnt;
        dmac_sta_csa_fsm_post_event(pst_mac_vap, WLAN_STA_CSA_EVENT_GET_IE, sizeof(st_csa_info), (oal_uint8*)&st_csa_info);
        /* 重新设置信道，vap下的带宽切到normal状态 */
        if ((IS_LEGACY_STA(pst_mac_vap)) && (OAL_TRUE == dmac_sta_csa_is_in_waiting(pst_mac_vap))
              && (DMAC_STA_BW_SWITCH_FSM_NORMAL != MAC_VAP_GET_CURREN_BW_STATE(pst_mac_vap)))
        {
            dmac_sta_bw_switch_fsm_post_event((dmac_vap_stru *)pst_mac_vap, DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC, 0, OAL_PTR_NULL);
            /* 带宽切换状态机 */
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ie_proc_ch_switch_ie::VAP CURREN BW STATE[%d].}",
                             MAC_VAP_GET_CURREN_BW_STATE(pst_mac_vap));
        }

    }
    return OAL_SUCC;
}


oal_bool_enum_uint8  dmac_ie_check_csa_ie(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len)
{
    oal_uint16   us_index = 0;
    oal_uint16   us_ch_index = 0xFFFF;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_ie_check_csa_ie::pst_mac_vap null.}");

        return OAL_FALSE;
    }

    /*************************************************************************/
    /*                    Channel Switch Announcement element                */
    /* --------------------------------------------------------------------- */
    /* |Element ID|Length |Channel switch Mode|New Channel| Ch switch count| */
    /* --------------------------------------------------------------------- */
    /* |1         |1      |1                  |1          |1               | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*                Extended Channel Switch Announcement element           */
    /* --------------------------------------------------------------------- */
    /* |Elem ID|Length|Ch Switch Mode|New Reg Class|New Ch| Ch switch count| */
    /* --------------------------------------------------------------------- */
    /* |1      |1     |1             |1            |1     |1               | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    while(us_index < us_frame_len)
    {
        if (MAC_EID_CHANSWITCHANN == puc_payload[us_index])
        {
            us_ch_index = us_index;
        }
        else if (MAC_EID_EXTCHANSWITCHANN == puc_payload[us_index])
        {
            us_ch_index = us_index;
            break;
        }

        us_index += (MAC_IE_HDR_LEN + puc_payload[us_index + 1]);

    }

    if (0xFFFF != us_ch_index)
    {
        dmac_ie_proc_ch_switch_ie(pst_mac_vap, &puc_payload[us_ch_index], (mac_eid_enum_uint8)puc_payload[us_ch_index]);

        return OAL_TRUE;
    }

    return OAL_FALSE;
}


oal_void  dmac_chan_update_csw_info(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len)
{
    if (OAL_FALSE == mac_mib_get_SpectrumManagementImplemented(pst_mac_vap))
    {
        return;
    }

    dmac_ie_check_csa_ie(pst_mac_vap, puc_payload, us_frame_len);
}


oal_uint16 dmac_encap_notify_chan_width(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_data, oal_uint8 *puc_da)
{
    oal_uint16 us_len = 0;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(puc_data, WLAN_FC0_SUBTYPE_ACTION);
    /*  Set DA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_data)->auc_address1, puc_da);
    /*  Set SA  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_data)->auc_address2, mac_mib_get_StationID(pst_mac_vap));
    /*  Set SSID  */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_data)->auc_address3, pst_mac_vap->auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*     Notify Channel Width frame Action field format                    */
    /* --------------------------------------------------------------------- */
    /* |   Category   |HT Action       |  Channel Width           | */
    /* --------------------------------------------------------------------- */
    /* |1             |1               | 1Byte                             | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
     puc_data[MAC_80211_FRAME_LEN] = MAC_ACTION_CATEGORY_HT;
     puc_data[MAC_80211_FRAME_LEN+1] = MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH;
     puc_data[MAC_80211_FRAME_LEN+2] = (pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M) ? 1 : 0;

     us_len = MAC_80211_FRAME_LEN + MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN;
     return us_len;

}
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST

oal_void dmac_chan_stop_40M_recovery_timer(dmac_vap_stru *pst_dmac_vap)
{
    if(OAL_FALSE == pst_dmac_vap->st_40M_recovery_timer.en_is_registerd)
    {
        return;
    }
    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_dmac_vap->st_40M_recovery_timer));
    OAM_WARNING_LOG0(0, OAM_SF_2040, "{dmac_chan_stop_40M_recovery_timer}");
}
#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


