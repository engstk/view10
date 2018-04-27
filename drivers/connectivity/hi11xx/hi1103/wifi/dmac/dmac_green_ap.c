
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef    _PRE_WLAN_FEATURE_GREEN_AP

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include    "hal_ext_if.h"
#include    "mac_device.h"
#include    "mac_vap.h"
#include    "dmac_resource.h"

#include    "dmac_tx_bss_comm.h"
#include    "dmac_vap.h"
#include    "dmac_device.h"
#include    "dmac_scan.h"
#include    "dmac_fcs.h"
#include    "dmac_beacon.h"
#include    "dmac_mgmt_classifier.h"
#include    "dmac_config.h"
#include    "dmac_green_ap.h"
#include    "dmac_auto_adjust_freq.h"


#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/hrtimer.h>
#include <linux/time.h>           /* struct timespec    */
#endif


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_GREEN_AP_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
OAL_STATIC oal_uint32  dmac_green_ap_start(dmac_green_ap_mgr_stru *pst_gap_mgr);
OAL_STATIC oal_uint32  dmac_green_ap_stop(dmac_green_ap_mgr_stru *pst_gap_mgr);

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)     /* 1102 */
OAL_STATIC oal_void dmac_green_ap_timer_isr(oal_void);
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
OAL_STATIC enum hrtimer_restart dmac_green_ap_timer_isr(struct hrtimer *pst_hrtimer);
#endif

OAL_STATIC oal_void  dmac_green_ap_scan_req(mac_device_stru *pst_device, dmac_green_ap_mgr_stru *pst_gap_mgr, oal_uint8 uc_cur_slot);
OAL_STATIC oal_void dmac_green_ap_scan_chn_cb(oal_void *p_param);


/*****************************************************************************
  3 函数实现
*****************************************************************************/


OAL_STATIC OAL_INLINE dmac_green_ap_mgr_stru* dmac_get_green_ap_stru(oal_uint8 uc_device_id)
{
    dmac_device_stru    *pst_dmac_device;

    pst_dmac_device = dmac_res_get_mac_dev(uc_device_id);

    if (OAL_PTR_NULL == pst_dmac_device)
    {
        return OAL_PTR_NULL;
    }

    return &(pst_dmac_device->st_green_ap_mgr);
}


oal_uint32  dmac_green_ap_setup_timer(dmac_green_ap_mgr_stru* pst_gap_mgr)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)     /* 1102 */
    Timer_DBAC_exit(GREEN_AP_STIMER_INDEX);
    Timer_DBAC_setup(GREEN_AP_STIMER_INDEX, dmac_green_ap_timer_isr);

    return OAL_SUCC;
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    hrtimer_init(&pst_gap_mgr->st_gap_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    pst_gap_mgr->st_gap_timer.function = dmac_green_ap_timer_isr;

    return OAL_SUCC;
#endif
}


oal_void dmac_green_ap_start_timer(dmac_green_ap_mgr_stru *pst_gap_mgr, oal_uint32 ul_duration)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)     /* 1102 */
    Timer_DBAC_start(GREEN_AP_STIMER_INDEX, ul_duration<<10);
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    hrtimer_start(&pst_gap_mgr->st_gap_timer,
                    ktime_set(ul_duration / 1000, (ul_duration % 1000) * 1000000),
                    HRTIMER_MODE_REL);
#endif
}


oal_void dmac_green_ap_stop_timer(dmac_green_ap_mgr_stru *pst_gap_mgr)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)     /* 1102 */
    Timer_DBAC_stop(GREEN_AP_STIMER_INDEX);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    hrtimer_cancel(&pst_gap_mgr->st_gap_timer);
#endif
}


oal_void dmac_green_ap_release_timer(dmac_green_ap_mgr_stru *pst_gap_mgr)
{
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)     /* 1102 */
    Timer_DBAC_exit(GREEN_AP_STIMER_INDEX);
#endif
}


oal_uint8 dmac_green_ap_is_hw_queues_empty(hal_to_dmac_device_stru  *pst_hal_device)
{
    oal_uint8                        uc_queue_num;

    /* 硬件发送队列 */
    for (uc_queue_num = 0; uc_queue_num < HAL_TX_QUEUE_BUTT; uc_queue_num++)
    {
        /*对应的硬件队列检查 */
        if (OAL_FALSE == (oal_dlist_is_empty(&(pst_hal_device->ast_tx_dscr_queue[uc_queue_num].st_header))))
        {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}


oal_uint8  dmac_green_ap_is_tid_queues_empty(dmac_vap_stru  *pst_dmac_vap)
{
    dmac_user_stru                   *pst_user;

    /* TID队列 */
    pst_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_dmac_vap->st_vap_base_info.us_assoc_vap_id);

    if (OAL_PTR_NULL != pst_user)
    {
        if (OAL_FALSE == dmac_psm_is_tid_empty(pst_user))
        {
            return OAL_FALSE;
        }
        return OAL_TRUE;
    }
    else
    {
        return OAL_TRUE;
    }
}


oal_void  dmac_green_ap_pause_vap(mac_device_stru *pst_device, dmac_green_ap_mgr_stru *pst_gap_mgr)
{
    hal_one_packet_status_stru   st_status;
    mac_fcs_err_enum_uint8       en_fcs_req_ret;
    mac_fcs_mgr_stru            *pst_fcs_mgr;
    dmac_vap_stru               *pst_dmac_vap;

    /* 吞吐量大于门限，不切到pause */
    if (OAL_FALSE == pst_gap_mgr->en_green_ap_dyn_en)
    {
        OAM_INFO_LOG0(pst_gap_mgr->uc_vap_id, OAM_SF_GREEN_AP, "{dmac_green_ap_pause_vap:: green_ap_dyn_en FALSE!}");
        return;
    }

    /* 当前正在扫描，不切到pause */
    if (MAC_SCAN_STATE_RUNNING == pst_device->en_curr_scan_state)
    {
        OAM_WARNING_LOG0(pst_gap_mgr->uc_vap_id, OAM_SF_GREEN_AP, "{dmac_green_ap_pause_vap:: device is scanning!}");
        return;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_gap_mgr->uc_vap_id);
    if ((OAL_PTR_NULL == pst_dmac_vap)
        || (MAC_VAP_STATE_UP != pst_dmac_vap->st_vap_base_info.en_vap_state))
    {
        OAM_INFO_LOG2(0, OAM_SF_GREEN_AP, "{dmac_green_ap_pause_vap:: pst_dmac_vap[%d], vap state[%d]}",
                 pst_gap_mgr->uc_vap_id, pst_dmac_vap->st_vap_base_info.en_vap_state);
        return;
    }

    /*检查接收硬件发送队列和TID队列是否空*/
    if(OAL_FALSE == dmac_green_ap_is_hw_queues_empty(pst_dmac_vap->pst_hal_device))
    {
        OAM_INFO_LOG0(pst_gap_mgr->uc_vap_id, OAM_SF_GREEN_AP, "{dmac_green_ap_pause_vap:: hw queues not empty!}");
        return;
    }

    pst_fcs_mgr     = dmac_fcs_get_mgr_stru(pst_device);
    en_fcs_req_ret  = mac_fcs_request(pst_fcs_mgr, NULL, NULL);
    if (MAC_FCS_SUCCESS != en_fcs_req_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_GREEN_AP, "{dmac_green_ap_pause_vap::mac_fcs_request failed, ret=%d}", (oal_uint32)en_fcs_req_ret);
        return;
    }

    dmac_vap_pause_tx(&pst_dmac_vap->st_vap_base_info);

    /* 发送one packet */
    dmac_fcs_send_one_packet_start(pst_fcs_mgr, &pst_gap_mgr->st_one_packet_cfg, pst_dmac_vap->pst_hal_device, &st_status, OAL_TRUE);

    hal_disable_machw_phy_and_pa(pst_dmac_vap->pst_hal_device);
    hal_psm_rf_sleep(pst_dmac_vap->pst_hal_device, OAL_TRUE);

    /* 在one packet保护下，sleep过程中不会收发包 */
    hal_one_packet_stop(pst_dmac_vap->pst_hal_device);
    mac_fcs_release(pst_fcs_mgr);

    pst_gap_mgr->uc_state = DMAC_GREEN_AP_STATE_PAUSE;
    pst_gap_mgr->ul_pause_count++;
}


oal_void  dmac_green_ap_resume_vap(mac_device_stru *pst_device, dmac_green_ap_mgr_stru *pst_gap_mgr)
{
    dmac_vap_stru *pst_dmac_vap;

    if (DMAC_GREEN_AP_STATE_WORK == pst_gap_mgr->uc_state)
    {
        return;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_gap_mgr->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_GREEN_AP, "{dmac_green_ap_resume_vap: pst_dmac_vap[%d] NULL!}", pst_gap_mgr->uc_vap_id);
        return;
    }

    hal_psm_rf_awake(pst_dmac_vap->pst_hal_device, OAL_TRUE);

    hal_enable_machw_phy_and_pa(pst_dmac_vap->pst_hal_device);

    if (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state)
    {
        mac_vap_resume_tx(&(pst_dmac_vap->st_vap_base_info));
        hal_vap_beacon_resume(pst_dmac_vap->pst_hal_vap);
    }

    pst_gap_mgr->uc_state = DMAC_GREEN_AP_STATE_WORK;
    pst_gap_mgr->ul_resume_count++;
}


oal_uint32  dmac_green_ap_timer_event_handler(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru              *pst_event;
    dmac_green_ap_mgr_stru      *pst_gap_mgr;
    mac_device_stru             *pst_device;
	dmac_vap_stru               *pst_dmac_vap;
    dmac_gap_fcs_event_stru     *pst_fcs_event;

    pst_event       = frw_get_event_stru(pst_event_mem);
    pst_fcs_event   = (dmac_gap_fcs_event_stru *)pst_event->auc_event_data;

    pst_device  = mac_res_get_dev(pst_fcs_event->uc_device_id);
    if(OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_timer_event_handler:: pst_device NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_gap_mgr  = dmac_get_green_ap_stru(pst_fcs_event->uc_device_id);
    if(OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_timer_event_handler:: pst_gap_mgr NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_gap_mgr->uc_state < DMAC_GREEN_AP_STATE_INITED)
    {
       return OAL_FAIL;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_gap_mgr->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_timer_event_handler:: pst_dmac_vap NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 为了防止事件处理延迟,造成不同步,不使用上半段的信息 */
    if (DMAC_GREEN_AP_STATE_PAUSE == pst_fcs_event->uc_state_to)
    {
         dmac_green_ap_pause_vap(pst_device, pst_gap_mgr);
    }
    else if (DMAC_GREEN_AP_STATE_WORK == pst_fcs_event->uc_state_to)
    {
         dmac_green_ap_resume_vap(pst_device, pst_gap_mgr);
         dmac_tx_complete_schedule(pst_dmac_vap->pst_hal_device, WLAN_WME_AC_BE);
         /* 发起扫描请求 */
         if (pst_gap_mgr->uc_tbtt_num <= DMAC_GAP_SCAN_MAX_TBTT_NUM)
         {
            dmac_green_ap_scan_req(pst_device, pst_gap_mgr, pst_fcs_event->uc_cur_slot);
         }
    }

    pst_gap_mgr->ul_total_count++;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_green_ap_post_pause_event(dmac_green_ap_mgr_stru *pst_gap_mgr)
{
    frw_event_mem_stru      *pst_event_mem;
    frw_event_stru          *pst_event;
    dmac_gap_fcs_event_stru *pst_fcs_event;

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_gap_fcs_event_stru));
    if (NULL == pst_event_mem)
    {
        OAM_WARNING_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_post_pause_event::alloc mem failed when post fcs event}");
        return  OAL_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_DMAC_MISC,
                       HAL_EVENT_DMAC_MISC_GREEN_AP,
                       OAL_SIZEOF(dmac_gap_fcs_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_0,
                       0,
                       0,
                       pst_gap_mgr->uc_vap_id); // no vap id

    pst_fcs_event = (dmac_gap_fcs_event_stru *)pst_event->auc_event_data;

    pst_fcs_event->uc_chip_id   =  pst_gap_mgr->uc_chip_id;
    pst_fcs_event->uc_device_id =  pst_gap_mgr->uc_device_id;
    pst_fcs_event->uc_cur_slot  =  pst_gap_mgr->uc_cur_slot;

    /* 奇数pause，偶数work */
    if (pst_gap_mgr->uc_cur_slot & 0x01)
    {
        pst_fcs_event->uc_state_to = DMAC_GREEN_AP_STATE_PAUSE;
    }
    else
    {
        pst_fcs_event->uc_state_to = DMAC_GREEN_AP_STATE_WORK;
    }

    if (OAL_SUCC != frw_event_dispatch_event(pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_post_pause_event::post fcs event error!}");
    }

#ifndef _PRE_WLAN_FEATURE_MEM_OPT
    FRW_EVENT_FREE(pst_event_mem);
#endif

    return  OAL_SUCC;
}



#if defined(_PRE_PRODUCT_ID_HI110X_DEV)     /* 1102 */
OAL_STATIC oal_void dmac_green_ap_timer_isr(void)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;
    oal_uint8                uc_time;
    oal_uint8                uc_offset;

    pst_gap_mgr = dmac_get_green_ap_stru(0);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_timer_isr:: pst_gap_mgr is NULL!}");
        return;
    }

    dmac_green_ap_stop_timer(pst_gap_mgr);

    if (pst_gap_mgr->uc_state <= DMAC_GREEN_AP_STATE_INITED)
    {
        OAM_WARNING_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_timer_isr:: waiting for tbtt isr!}");

        return;
    }

    pst_gap_mgr->uc_cur_slot++;

    dmac_green_ap_post_pause_event(pst_gap_mgr);

    /*最后一个时隙不启动timer，tbtt中断后启动*/
    if(pst_gap_mgr->uc_max_slot_cnt > pst_gap_mgr->uc_cur_slot)
    {
       uc_time   = (pst_gap_mgr->uc_cur_slot & 0x01) ? pst_gap_mgr->uc_pause_time : pst_gap_mgr->uc_work_time;
       uc_offset = (pst_gap_mgr->uc_cur_slot & 0x01) ? pst_gap_mgr->uc_resume_offset : 0;
       dmac_green_ap_start_timer(pst_gap_mgr, (uc_time - uc_offset));
    }
}
#endif

/* 1151 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
OAL_STATIC enum hrtimer_restart dmac_green_ap_timer_isr(struct hrtimer *pst_hrtimer)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;
    oal_uint8                uc_time;
    oal_uint8                uc_offset;

    pst_gap_mgr = dmac_get_green_ap_stru((OAL_CONTAINER_OF(pst_hrtimer, dmac_green_ap_mgr_stru, st_gap_timer))->uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_timer_isr:: pst_gap_mgr NULL!}");
        return HRTIMER_NORESTART;
    }

    if (pst_gap_mgr->uc_state < DMAC_GREEN_AP_STATE_INITED)
    {
        OAM_WARNING_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_timer_isr:: waiting for tbtt isr!}");

        return HRTIMER_NORESTART;
    }

    pst_gap_mgr->uc_cur_slot++;

    dmac_green_ap_post_pause_event(pst_gap_mgr);

    /*最后一个时隙不启动timer，tbtt中断后启动 */
    if(pst_gap_mgr->uc_max_slot_cnt > pst_gap_mgr->uc_cur_slot)
    {
       uc_time   = (pst_gap_mgr->uc_cur_slot & 0x01) ? pst_gap_mgr->uc_pause_time : pst_gap_mgr->uc_work_time;
       uc_offset = (pst_gap_mgr->uc_cur_slot & 0x01) ? pst_gap_mgr->uc_resume_offset : 0;
       dmac_green_ap_start_timer(pst_gap_mgr, (uc_time - uc_offset));
    }

    return HRTIMER_NORESTART;
}
#endif


mac_vap_stru*  dmac_green_ap_get_vap(mac_device_stru  *pst_device)
{
    mac_vap_stru            *pst_vap = OAL_PTR_NULL;
    oal_uint8                uc_vap_idx;
    if (!pst_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_get_vap:: pst_device NULL!}");
        return OAL_PTR_NULL;
    }

    /* 只在单个aput下开启 */
    if(pst_device->uc_vap_num != 1)
    {
        return OAL_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_DBAC
    /*与DBAC功能互斥*/
    if(mac_is_dbac_running(pst_device))
    {
        return OAL_PTR_NULL;
    }
#endif

    for(uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if(OAL_PTR_NULL == pst_vap)
        {
            OAM_WARNING_LOG1(0, OAM_SF_DFS, "{dmac_green_ap_get_vap::pst_mac_vap null, vap id=%d.", pst_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
    #ifdef _PRE_WLAN_FEATURE_DFS
        /* dfs使能,且在dfs工作信道,不开启green ap */
        if ((OAL_TRUE == mac_vap_get_dfs_enable(pst_vap))
            && (OAL_TRUE == mac_is_cover_dfs_channel(pst_device->en_max_band, pst_device->en_max_bandwidth, pst_device->uc_max_channel))
            )
        {
            OAM_INFO_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_get_vap:: current mode cover dfs channel.}");
            return OAL_PTR_NULL;
        }
    #endif

        if ((WLAN_VAP_MODE_BSS_AP == pst_vap->en_vap_mode)
            && ((MAC_VAP_STATE_AP_WAIT_START == pst_vap->en_vap_state) ||
                (MAC_VAP_STATE_UP == pst_vap->en_vap_state) ||
                (MAC_VAP_STATE_PAUSE == pst_vap->en_vap_state)))
        {
            return pst_vap;
        }
    }
    return  OAL_PTR_NULL;
}


oal_void dmac_green_ap_tbtt_isr(oal_uint8 uc_hal_vap_id, oal_void  *p_arg)
{
    dmac_green_ap_mgr_stru     *pst_gap_mgr;
    hal_to_dmac_device_stru    *pst_hal_to_dmac_device = (hal_to_dmac_device_stru *)p_arg;
    mac_device_stru            *pst_mac_device;
    mac_vap_stru               *pst_vap;

    pst_mac_device  = mac_res_get_dev(pst_hal_to_dmac_device->uc_mac_device_id);
    if (OAL_UNLIKELY(NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_tbtt_isr::pst_mac_device == NULL}");
        return;
    }

    pst_gap_mgr = dmac_get_green_ap_stru(pst_mac_device->uc_device_id);
    if ((OAL_PTR_NULL == pst_gap_mgr)
       || (OAL_FALSE == pst_gap_mgr->uc_green_ap_enable))
    {
        return;
    }

    /* 防止未start就进入 */
    if (pst_gap_mgr->uc_state < DMAC_GREEN_AP_STATE_INITED)
    {
        return;
    }

    if (DMAC_GREEN_AP_STATE_PAUSE == pst_gap_mgr->uc_state)
    {
        dmac_green_ap_resume_vap(pst_mac_device, pst_gap_mgr);
        OAM_WARNING_LOG1(pst_gap_mgr->uc_vap_id, OAM_SF_GREEN_AP, "{dmac_green_ap_tbtt_isr::in pause state,cur_slot = %d}", pst_gap_mgr->uc_cur_slot);
    }

    /* 实时检测 */
    pst_vap = dmac_green_ap_get_vap(pst_mac_device);
    if (OAL_PTR_NULL == pst_vap)
    {
        return;
    }

    /* 空闲率低,不开启green ap */
    if (OAL_TRUE == pst_gap_mgr->uc_cca_scan_enble)
    {
        OAM_INFO_LOG0(pst_gap_mgr->uc_vap_id, OAM_SF_GREEN_AP, "{dmac_green_ap_tbtt_isr:: cca_scan_enble!}");
        return;
    }
    pst_gap_mgr->uc_tbtt_num ++;

    /* 最后一个时隙不启动timer，tbtt中断后启动 */
    pst_gap_mgr->uc_cur_slot = 0;
    dmac_green_ap_start_timer(pst_gap_mgr, (pst_gap_mgr->uc_work_time - DMAC_DEFAULT_GAP_BCN_GUARD_TIME));
}


oal_void  dmac_green_ap_fcs_isr(oal_uint8 uc_vap_id, oal_void  *p_arg)
{
    mac_device_stru            *pst_mac_device;
    hal_to_dmac_device_stru    *pst_hal_to_dmac_device = (hal_to_dmac_device_stru *)p_arg;

    mac_fcs_verify_timestamp(MAC_FCS_STAGE_ONE_PKT_INTR);

    if (OAL_UNLIKELY(NULL == pst_hal_to_dmac_device))
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_GREEN_AP, "{dmac_green_ap_fcs_isr::in mac_fcs_isr p_arg=NULL}");
        return;
    }

    pst_mac_device  = mac_res_get_dev(pst_hal_to_dmac_device->uc_mac_device_id);
    if (OAL_UNLIKELY(NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_GREEN_AP, "{dmac_green_ap_fcs_isr::pst_mac_device == NULL}");
        return;
    }

    pst_mac_device->st_fcs_mgr.en_fcs_done = OAL_TRUE;
}


oal_uint32  dmac_green_ap_init(mac_device_stru *pst_mac_device)
{
    dmac_green_ap_mgr_stru   *pst_gap_mgr;
    dmac_device_stru         *pst_dmac_device;

    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_init:: pst_mac_device is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_init:: pst_dmac_device is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_gap_mgr = dmac_get_green_ap_stru(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Green AP功能E5上默认打开 */
#if (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5)
    pst_gap_mgr->uc_green_ap_enable  = OAL_TRUE;
#else
    pst_gap_mgr->uc_green_ap_enable  = OAL_FALSE;
#endif
    pst_gap_mgr->en_green_ap_dyn_en      = OAL_FALSE;
    pst_gap_mgr->en_green_ap_dyn_en_old  = OAL_FALSE;
    pst_gap_mgr->uc_green_ap_suspend     = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_INTF_DET
    pst_gap_mgr->uc_cca_scan_enble       = OAL_TRUE;
#else
    pst_gap_mgr->uc_cca_scan_enble       = OAL_FALSE;
#endif
    pst_gap_mgr->ul_free_time_th_cca     = DMAC_GAP_DEFAULT_FREE_RATIO_CCA;
    pst_gap_mgr->ul_free_time_th_gap     = DMAC_GAP_DEFAULT_FREE_RATIO_GAP;
    pst_gap_mgr->uc_tbtt_num             = 0;
    pst_gap_mgr->uc_scan_req_time        = 0;
    pst_gap_mgr->uc_debug_mode           = OAL_FALSE;

    pst_gap_mgr->uc_state                = DMAC_GREEN_AP_STATE_NOT_INITED;
    pst_gap_mgr->uc_device_id            = pst_mac_device->uc_device_id;
    pst_gap_mgr->uc_chip_id              = pst_mac_device->uc_chip_id;

    pst_gap_mgr->uc_vap_id               = 0xff;
    pst_gap_mgr->uc_hal_vap_id           = 0xff;

    pst_gap_mgr->uc_work_time       = DMAC_DEFAULT_GAP_WORK_TIME;
    pst_gap_mgr->uc_pause_time      = DMAC_DEFAULT_GAP_PAUSE_TIME;
    pst_gap_mgr->uc_max_slot_cnt    = DMAC_DEFAULT_GAP_SLOT_CNT;
    pst_gap_mgr->uc_cur_slot        = 0;
    pst_gap_mgr->uc_resume_offset   = DMAC_DEFAULT_GAP_RESUME_OFFSET;

    pst_gap_mgr->ul_pause_count     = 0;
    pst_gap_mgr->ul_resume_count    = 0;
    pst_gap_mgr->ul_total_count     = 0;

    hal_register_gap_isr_hook(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device), HAL_ISR_TYPE_TBTT, dmac_green_ap_tbtt_isr);
    hal_register_gap_isr_hook(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device), HAL_ISR_TYPE_ONE_PKT, dmac_green_ap_fcs_isr);
    dmac_green_ap_setup_timer(pst_gap_mgr);

    /* 注册并启动PPS统计 */
    dmac_set_auto_freq_pps_start();

    return  OAL_SUCC;
}


oal_uint32  dmac_green_ap_exit(mac_device_stru *pst_mac_device)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;
    dmac_device_stru        *pst_dmac_device;

    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_exit:: pst_mac_device is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_exit:: pst_dmac_device is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_gap_mgr = dmac_get_green_ap_stru(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_unregister_gap_isr_hook(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device), HAL_ISR_TYPE_TBTT);
    hal_unregister_gap_isr_hook(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device), HAL_ISR_TYPE_ONE_PKT);

    dmac_green_ap_release_timer(pst_gap_mgr);

    /* 去注册PPS统计 */
    dmac_set_auto_freq_pps_stop();

    return OAL_SUCC;
}


oal_uint32  dmac_green_ap_start(dmac_green_ap_mgr_stru  *pst_gap_mgr)
{
    dmac_vap_stru   *pst_dmac_vap;
    oal_uint32       ul_beacon_period;
    oal_uint8        uc_sched_period_num;

    /* 未使能，返回 */
    if (OAL_FALSE == pst_gap_mgr->uc_green_ap_enable)
    {
        OAM_WARNING_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_start:: green ap is not enable!}");
        return OAL_FAIL;
    }

    /* green ap已经在running */
    if (pst_gap_mgr->uc_state > DMAC_GREEN_AP_STATE_INITED)
    {
        OAM_WARNING_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_start:: green ap is already running!}");
        return OAL_SUCC;
    }

    /* 初始化配置参数 */
    pst_dmac_vap = mac_res_get_dmac_vap(pst_gap_mgr->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_GREEN_AP, "{dmac_green_ap_start:: pst_dmac_vap[id=%d] NULL!}", pst_gap_mgr->uc_vap_id);
        return OAL_FAIL;
    }

    ul_beacon_period = pst_dmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_sta_config.ul_dot11BeaconPeriod;
    if (0 != ul_beacon_period)
    {
        /* 除不尽部分归为work时隙 */
        uc_sched_period_num = ul_beacon_period / (pst_gap_mgr->uc_work_time + pst_gap_mgr->uc_pause_time);

        /* beacon周期<50，划分为两个slot */
        if (0 == uc_sched_period_num)
        {
            uc_sched_period_num = 1;
            pst_gap_mgr->uc_work_time = (ul_beacon_period  >> 1);
            pst_gap_mgr->uc_pause_time = pst_gap_mgr->uc_work_time;
        }
        pst_gap_mgr->uc_max_slot_cnt = (uc_sched_period_num << 1);
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_start:: beacon=0}");
        return OAL_FAIL;
    }
    dmac_fcs_prepare_one_packet_cfg(&pst_dmac_vap->st_vap_base_info, &pst_gap_mgr->st_one_packet_cfg, pst_gap_mgr->uc_pause_time);

    /* 等待tbtt中断到来 */
    pst_gap_mgr->uc_state = DMAC_GREEN_AP_STATE_INITED;

    return OAL_SUCC;
}


oal_uint32  dmac_green_ap_stop(dmac_green_ap_mgr_stru *pst_gap_mgr)
{
    if(OAL_FALSE == pst_gap_mgr->uc_green_ap_enable)
    {
        OAM_WARNING_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_stop:: green ap not enabled!}");
        return OAL_SUCC;
    }

    dmac_green_ap_stop_timer(pst_gap_mgr);
    dmac_green_ap_release_timer(pst_gap_mgr);

    if (DMAC_GREEN_AP_STATE_PAUSE == pst_gap_mgr->uc_state)
    {
        OAM_WARNING_LOG1(pst_gap_mgr->uc_vap_id, OAM_SF_GREEN_AP, "{dmac_green_ap_stop::in pause state,cur_slot = %d}",pst_gap_mgr->uc_cur_slot);

        dmac_green_ap_resume_vap(mac_res_get_dev(pst_gap_mgr->uc_device_id), pst_gap_mgr);
    }

    pst_gap_mgr->uc_state   = DMAC_GREEN_AP_STATE_NOT_INITED;

    return OAL_SUCC;
}


oal_uint32  dmac_green_ap_suspend(mac_device_stru *pst_device)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;

    pst_gap_mgr = dmac_get_green_ap_stru(pst_device->uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_resume:: pst_gap_mgr NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(OAL_FALSE == pst_gap_mgr->uc_green_ap_enable)
    {
        return OAL_SUCC;
    }

    dmac_green_ap_stop(pst_gap_mgr);

    pst_gap_mgr->uc_green_ap_suspend = OAL_TRUE;

    return OAL_SUCC;
}


oal_uint32  dmac_green_ap_resume(mac_device_stru *pst_device)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;

    pst_gap_mgr = dmac_get_green_ap_stru(pst_device->uc_device_id);

    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_resume:: pst_gap_mgr NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 防止wow唤醒,直接开启green ap */
    if (OAL_FALSE == pst_gap_mgr->en_green_ap_dyn_en)
    {
        return OAL_SUCC;
    }

    if(OAL_TRUE == pst_gap_mgr->uc_green_ap_suspend)
    {
        dmac_green_ap_start(pst_gap_mgr);
    }

    pst_gap_mgr->uc_green_ap_suspend = OAL_FALSE;

    return OAL_SUCC;
}


oal_uint32  dmac_green_ap_dump_info(oal_uint8 uc_device_id)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;

    pst_gap_mgr = dmac_get_green_ap_stru(uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_dump_info:: pst_gap_mgr NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_IO_PRINT("Enable[%d]:Pause[%d],resume[%d],total[%d]\r\n", pst_gap_mgr->uc_green_ap_enable,
           pst_gap_mgr->ul_pause_count, pst_gap_mgr->ul_resume_count, pst_gap_mgr->ul_total_count);

    return OAL_SUCC;
}


oal_void  dmac_green_ap_pps_process(oal_uint32 ul_pps_rate)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;
    oal_uint32               ul_thrpt_stat;

    pst_gap_mgr = dmac_get_green_ap_stru(0);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_pps_process:: pst_gap_mgr NULL!}");
        return;
    }

    /* pps转换吞吐量(Mbps) */
    ul_thrpt_stat = DMAC_GAP_PPS_TO_THRPT_MBPS(ul_pps_rate);

    pst_gap_mgr->en_green_ap_dyn_en_old = pst_gap_mgr->en_green_ap_dyn_en;

    /* 吞吐量大于门限，关闭green ap */
    if (ul_thrpt_stat > DMAC_GAP_EN_THRPT_HIGH_TH)
    {
        pst_gap_mgr->en_green_ap_dyn_en = OAL_FALSE;
    }
    else
    {
        pst_gap_mgr->en_green_ap_dyn_en = OAL_TRUE;
    }

    /* 避免临界条件下，打印过于频繁 */
    if ((pst_gap_mgr->en_green_ap_dyn_en_old != pst_gap_mgr->en_green_ap_dyn_en)
        && ((DMAC_GAP_PPS_MAX_COUNT - 1) == pst_gap_mgr->uc_pps_count)
        && (DMAC_GREEN_AP_STATE_WORK == pst_gap_mgr->uc_state))
    {
        OAM_WARNING_LOG3(0, OAM_SF_GREEN_AP, "{dmac_green_ap_pps_process:: en_green_ap_dyn_en change from[%d] to [%d], throughput[%d]}",
                    pst_gap_mgr->en_green_ap_dyn_en_old, pst_gap_mgr->en_green_ap_dyn_en, ul_thrpt_stat);
    }

    pst_gap_mgr->uc_pps_count ++;
    pst_gap_mgr->uc_pps_count %= DMAC_GAP_PPS_MAX_COUNT;
}



oal_uint32  dmac_green_ap_switch_auto(oal_uint8 uc_device_id, dmac_green_ap_switch_type_uint8 uc_switch_type)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;
    mac_device_stru         *pst_device;
    mac_vap_stru            *pst_mac_vap;

    pst_gap_mgr = dmac_get_green_ap_stru(uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_switch_auto:: dmac_get_green_ap_stru NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 未使能，返回 */
    if (OAL_FALSE == pst_gap_mgr->uc_green_ap_enable)
    {
        OAM_INFO_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_switch_auto:: green ap is not enable!}");
        return OAL_SUCC;
    }

    pst_device = mac_res_get_dev(uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_switch_auto:: pst_device NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }


    /* 发起扫描时，立即切到work状态 */
    if (DMAC_GREEN_AP_SCAN_START == uc_switch_type)
    {
        if ((MAC_SCAN_STATE_RUNNING == pst_device->en_curr_scan_state)
           && (DMAC_GREEN_AP_STATE_PAUSE == pst_gap_mgr->uc_state))
        {
            dmac_green_ap_resume_vap(pst_device, pst_gap_mgr);
            OAM_WARNING_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_switch_auto:: scan start and change to work!}");
        }
        return OAL_SUCC;
    }

    pst_mac_vap = dmac_green_ap_get_vap(pst_device);

    /* vap up或enable事件开启green ap */
    if ((OAL_PTR_NULL != pst_mac_vap) &&
        ((DMAC_GREEN_AP_ENABLE == uc_switch_type) || (DMAC_GREEN_AP_VAP_UP == uc_switch_type)))
    {
        pst_gap_mgr->uc_vap_id = pst_device->auc_vap_id[0];
        dmac_green_ap_start(pst_gap_mgr);
        OAM_WARNING_LOG4(0, OAM_SF_GREEN_AP,"{dmac_green_ap_switch_auto:: start! event_type[%d], vap_id[%d], vap_state[%d], gap_state[%d]}",
                    uc_switch_type, pst_gap_mgr->uc_vap_id, pst_mac_vap->en_vap_state, pst_gap_mgr->uc_state);
    }
    else
    {
        /* 多个vap停止green ap */
        dmac_green_ap_stop(pst_gap_mgr);
        pst_gap_mgr->uc_vap_id = 0xff;
        OAM_WARNING_LOG2(0, OAM_SF_GREEN_AP, "{dmac_green_ap_switch_auto:: stop! event_type[%d], vap_num[%d]}",
                    uc_switch_type, pst_device->uc_vap_num);
        /* 维测 */
        if ((OAL_PTR_NULL != pst_mac_vap) && (uc_switch_type != DMAC_GREEN_AP_VAP_DOWN))
        {
            OAM_ERROR_LOG1(0, OAM_SF_GREEN_AP, "{dmac_green_ap_switch_auto:: stop unexpectedly! vap state[%d].}",
                    pst_mac_vap->en_vap_state);
            oam_report_backtrace();
        }
    }

    return OAL_SUCC;
}


oal_void  dmac_green_ap_enable(oal_uint8 uc_device_id)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;

    pst_gap_mgr = dmac_get_green_ap_stru(uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_enable:: dmac_get_green_ap_stru NULL!}");
        return;
    }

    /* 已经使能,不重复配置 */
    if (OAL_TRUE == pst_gap_mgr->uc_green_ap_enable)
    {
        return;
    }

    pst_gap_mgr->uc_green_ap_enable  = OAL_TRUE;

    dmac_green_ap_switch_auto(uc_device_id, DMAC_GREEN_AP_ENABLE);
}


oal_void  dmac_green_ap_disable(oal_uint8 uc_device_id)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;

    pst_gap_mgr = dmac_get_green_ap_stru(uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_disable:: dmac_get_green_ap_stru NULL!}");
        return;
    }

    /* 已经关闭,直接返回 */
    if (OAL_FALSE == pst_gap_mgr->uc_green_ap_enable)
    {
        return;
    }

    dmac_green_ap_stop(pst_gap_mgr);
    pst_gap_mgr->uc_green_ap_enable  = OAL_FALSE;
}



oal_void  dmac_green_ap_optimization(mac_device_stru *pst_device, wlan_scan_chan_stats_stru *pst_chan_result, oal_bool_enum_uint8 en_from_cca)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;
    oal_uint32               ul_free_time_ratio_20;
    oal_uint32               ul_free_time_ratio_40;
    oal_uint32               ul_free_time_ratio_80;
    oal_uint32               ul_green_ap_on_th;

    pst_gap_mgr = dmac_get_green_ap_stru(pst_device->uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_optimaztion:: dmac_get_green_ap_stru NULL!}");
        return;
    }

    if (OAL_FALSE == pst_gap_mgr->uc_green_ap_enable)
    {
        OAM_INFO_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_optimaztion:: green ap is disable!}");
        return;
    }

    /* 计算空闲时间比率 */
    if (OAL_TRUE == en_from_cca)
    {
        ul_green_ap_on_th = pst_gap_mgr->ul_free_time_th_cca;

        ul_free_time_ratio_20 =
            DMAC_GAP_FREE_RATIO_CALC(pst_chan_result->ul_total_free_time_20M_us, pst_chan_result->ul_total_stats_time_us);
        ul_free_time_ratio_40 =
            DMAC_GAP_FREE_RATIO_CALC(pst_chan_result->ul_total_free_time_40M_us, pst_chan_result->ul_total_stats_time_us);
        ul_free_time_ratio_80 =
            DMAC_GAP_FREE_RATIO_CALC(pst_chan_result->ul_total_free_time_80M_us, pst_chan_result->ul_total_stats_time_us);

        /* 维测log */
        if (OAL_TRUE == pst_gap_mgr->uc_debug_mode)
        {
            OAM_WARNING_LOG4(0, OAM_SF_GREEN_AP, "CCA SCAN- free20[%d], free40[%d], free80[%d], total_time[%d]",
                    pst_chan_result->ul_total_free_time_20M_us, \
                    pst_chan_result->ul_total_free_time_40M_us, \
                    pst_chan_result->ul_total_free_time_80M_us, \
                    pst_chan_result->ul_total_stats_time_us);
        }
    }
    else
    {
         ul_green_ap_on_th = pst_gap_mgr->ul_free_time_th_gap;

        /* 如果发起扫描次数偏少,测量不准,使能cca */
        if (pst_gap_mgr->uc_scan_req_time < ((pst_gap_mgr->uc_tbtt_num * pst_gap_mgr->uc_max_slot_cnt) >> 3))
        {
            ul_free_time_ratio_20 = 0;
            ul_free_time_ratio_40 = 0;
            ul_free_time_ratio_80 = 0;
        }
        else
        {
            ul_free_time_ratio_20 =
                DMAC_GAP_FREE_RATIO_CALC(pst_gap_mgr->st_scan_info.ul_total_free_time_20M, pst_gap_mgr->st_scan_info.ul_total_stat_time);
            ul_free_time_ratio_40 =
                DMAC_GAP_FREE_RATIO_CALC(pst_gap_mgr->st_scan_info.ul_total_free_time_40M, pst_gap_mgr->st_scan_info.ul_total_stat_time);
            ul_free_time_ratio_80 =
                DMAC_GAP_FREE_RATIO_CALC(pst_gap_mgr->st_scan_info.ul_total_free_time_80M, pst_gap_mgr->st_scan_info.ul_total_stat_time);
        }

        /* 维测log */
        if (OAL_TRUE == pst_gap_mgr->uc_debug_mode)
        {
            OAM_WARNING_LOG4(0, OAM_SF_GREEN_AP, "GreenAP SCAN- free20[%d], free40[%d], free80[%d], total_time[%d]",
                    pst_gap_mgr->st_scan_info.ul_total_free_time_20M, \
                    pst_gap_mgr->st_scan_info.ul_total_free_time_40M, \
                    pst_gap_mgr->st_scan_info.ul_total_free_time_80M, \
                    pst_gap_mgr->st_scan_info.ul_total_stat_time);
        }
    }

    /* 信道空闲率低于门限, 关闭green ap, 使能cca扫描 */
    if ((ul_free_time_ratio_20 < ul_green_ap_on_th)
        || (ul_free_time_ratio_40 < ul_green_ap_on_th)
        || (ul_free_time_ratio_80 < ul_green_ap_on_th))
    {
        pst_gap_mgr->uc_cca_scan_enble = OAL_TRUE;

        /* 维测log */
        if (OAL_TRUE == pst_gap_mgr->uc_debug_mode)
        {
            OAM_ERROR_LOG3(0, OAM_SF_GREEN_AP, "{dmac_green_ap_optimization: en_from_cca[%d], free ratio20[%d] < th[%d]}",
                   en_from_cca, ul_free_time_ratio_20, ul_green_ap_on_th);
        }
    }
    else
    {
        pst_gap_mgr->uc_cca_scan_enble = OAL_FALSE;
    }

    pst_gap_mgr->uc_tbtt_num      = 0;
    pst_gap_mgr->uc_scan_req_time = 0;
    OAL_MEMZERO(&pst_gap_mgr->st_scan_info, OAL_SIZEOF(dmac_gap_scan_info_stru));
}



OAL_STATIC oal_void  dmac_green_ap_scan_req(mac_device_stru *pst_device, dmac_green_ap_mgr_stru *pst_gap_mgr, oal_uint8 uc_cur_slot)
{
    mac_scan_req_stru  st_scan_params;
    dmac_vap_stru      *pst_dmac_vap;

    OAL_MEMZERO(&st_scan_params, OAL_SIZEOF(mac_scan_req_stru));

    pst_dmac_vap = mac_res_get_dmac_vap(pst_gap_mgr->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_GREEN_AP, "{dmac_green_ap_scan_req: pst_dmac_vap[%d] NULL!}", pst_gap_mgr->uc_vap_id);
        return;
    }

    /* beacon周期小于50ms,不开启扫描 */
    if (pst_gap_mgr->uc_work_time < DMAC_DEFAULT_GAP_WORK_TIME)
    {
        OAM_INFO_LOG1(0, OAM_SF_GREEN_AP, "{dmac_green_ap_scan_req: work time[%d] < 25ms.}", pst_gap_mgr->uc_work_time);
        return;
    }

    st_scan_params.uc_vap_id                      = pst_dmac_vap->st_vap_base_info.uc_vap_id;
    st_scan_params.en_bss_type                    = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    st_scan_params.en_scan_type                   = WLAN_SCAN_TYPE_PASSIVE;
    st_scan_params.uc_probe_delay                 = 0;
    st_scan_params.uc_scan_func                   = MAC_SCAN_FUNC_MEAS | MAC_SCAN_FUNC_STATS;
    st_scan_params.uc_max_scan_count_per_channel  = WLAN_DEFAULT_BG_SCAN_COUNT_PER_CHANNEL;
    st_scan_params.en_scan_mode                   = WLAN_SCAN_MODE_BACKGROUND_CCA;
    st_scan_params.uc_channel_nums                = 1;
    st_scan_params.ast_channel_list[0]            = pst_dmac_vap->st_vap_base_info.st_channel;
    st_scan_params.p_fn_cb                        = dmac_green_ap_scan_chn_cb;
    if (uc_cur_slot == pst_gap_mgr->uc_max_slot_cnt)
    {
        st_scan_params.us_scan_time = (DMAC_GAP_SCAN_TIME_MS + DMAC_GAP_SCAN_RESV_TIME_MS); //ms
    }
    else
    {
        st_scan_params.us_scan_time = (DMAC_GAP_SCAN_TIME_MS << 1) + DMAC_GAP_SCAN_RESV_TIME_MS; //ms
    }

     /* 竞争到扫描权限后，将扫描参数拷贝到mac deivce结构体下，此时拷贝，也是为了防止扫描参数被覆盖情况 */
    oal_memcopy(&(pst_device->st_scan_params), &st_scan_params, OAL_SIZEOF(mac_scan_req_stru));

    /* 清空信道测量结果 */
    OAL_MEMZERO(&(pst_dmac_vap->pst_hal_device->st_chan_result), OAL_SIZEOF(wlan_scan_chan_stats_stru));

    /* 使能信道测量中断 */
    hal_set_ch_statics_period(pst_dmac_vap->pst_hal_device, DMAC_SCAN_CHANNEL_STATICS_PERIOD_US);
    hal_set_ch_measurement_period(pst_dmac_vap->pst_hal_device, DMAC_SCAN_CHANNEL_MEAS_PERIOD_MS);
    hal_enable_ch_statics(pst_dmac_vap->pst_hal_device, 1);

    pst_gap_mgr->uc_scan_req_time ++;
}



OAL_STATIC oal_void dmac_green_ap_scan_chn_cb(oal_void *p_param)
{
    mac_device_stru             *pst_device;
    dmac_green_ap_mgr_stru      *pst_gap_mgr;
    wlan_scan_chan_stats_stru    *pst_chan_result;
    dmac_device_stru            *pst_dmac_device;

    if (OAL_PTR_NULL == p_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CCA_OPT, "{dmac_green_ap_scan_chn_cb: input pointer is null!}");
        return;
    }

    pst_device      = (mac_device_stru*)p_param;
    pst_dmac_device = dmac_res_get_mac_dev(pst_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_scan_chn_cb:: pst_dmac_device is NULL!}");
        return;
    }

    pst_chan_result = &(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device)->st_chan_result);

    pst_gap_mgr = dmac_get_green_ap_stru(pst_device->uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_scan_chn_cb:: dmac_get_green_ap_stru NULL!}");
        return;
    }

    pst_gap_mgr->st_scan_info.ul_total_stat_time     += pst_chan_result->ul_total_stats_time_us;
    pst_gap_mgr->st_scan_info.ul_total_free_time_20M += pst_chan_result->ul_total_free_time_20M_us;
    pst_gap_mgr->st_scan_info.ul_total_free_time_40M += pst_chan_result->ul_total_free_time_40M_us;
    pst_gap_mgr->st_scan_info.ul_total_free_time_80M += pst_chan_result->ul_total_free_time_80M_us;

    /* debug info */
    if (OAL_TRUE == pst_gap_mgr->uc_debug_mode)
    {
        OAM_INFO_LOG4(0, OAM_SF_GREEN_AP, "{dmac_green_ap_scan_chn_cb:: stats_time[%d], free_time20[%d], green_total[%d], green_free20[%d].}",
                pst_chan_result->ul_total_stats_time_us, pst_chan_result->ul_total_free_time_20M_us, pst_gap_mgr->st_scan_info.ul_total_stat_time, pst_gap_mgr->st_scan_info.ul_total_free_time_20M);
    }
}



oal_uint32  dmac_green_ap_get_scan_enable(mac_device_stru *pst_device)
{
    dmac_green_ap_mgr_stru      *pst_gap_mgr;

    pst_gap_mgr = dmac_get_green_ap_stru(pst_device->uc_device_id);
    if ((OAL_PTR_NULL == pst_gap_mgr)
        || (OAL_FALSE == pst_gap_mgr->uc_green_ap_enable))
    {
        OAM_INFO_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_get_scan_enable:: green ap null or disable!}");
        return OAL_TRUE;
    }

    return pst_gap_mgr->uc_cca_scan_enble;
}


oal_void  dmac_green_ap_set_scan_enable(oal_uint8 uc_device_id, oal_uint8 uc_cca_en)
{
    dmac_green_ap_mgr_stru      *pst_gap_mgr;

    pst_gap_mgr = dmac_get_green_ap_stru(uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_INFO_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_set_scan_enable:: green ap stru null!}");
        return;
    }

    /* cca未使能,不开启空闲率检测 */
    if (OAL_FALSE == uc_cca_en)
    {
        pst_gap_mgr->uc_cca_scan_enble = OAL_FALSE;
    }
}


 oal_void dmac_green_ap_set_free_ratio(oal_uint8 uc_device_id, oal_uint8 uc_green_ap_mode, oal_uint8 uc_free_ratio_th)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;

    pst_gap_mgr = dmac_get_green_ap_stru(uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_set_free_ratio:: dmac_get_green_ap_stru NULL!}");
        return;
    }

    if (1 == uc_green_ap_mode)
    {
        pst_gap_mgr->ul_free_time_th_gap = (10 * uc_free_ratio_th);
        OAM_WARNING_LOG1(0, OAM_SF_GREEN_AP, "{dmac_green_ap_set_free_ratio: GreenAP active th of thousand =[%d].}", pst_gap_mgr->ul_free_time_th_gap);
    }
    else
    {
        pst_gap_mgr->ul_free_time_th_cca = (10 * uc_free_ratio_th);
        OAM_WARNING_LOG1(0, OAM_SF_GREEN_AP, "{dmac_green_ap_set_free_ratio: GreenAP inactive th of thousand =[%d].}", pst_gap_mgr->ul_free_time_th_cca);
    }
}



oal_void dmac_green_ap_set_debug_mode(oal_uint8 uc_device_id, oal_uint8 uc_debug_en)
{
    dmac_green_ap_mgr_stru  *pst_gap_mgr;

    pst_gap_mgr = dmac_get_green_ap_stru(uc_device_id);
    if (OAL_PTR_NULL == pst_gap_mgr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_GREEN_AP, "{dmac_green_ap_set_free_ratio:: dmac_get_green_ap_stru NULL!}");
        return;
    }

    pst_gap_mgr->uc_debug_mode = uc_debug_en;
    OAM_WARNING_LOG1(0, OAM_SF_GREEN_AP, "{dmac_green_ap_set_debug_mode: debug en[%d].}", pst_gap_mgr->uc_debug_mode);
}


oal_module_symbol(dmac_green_ap_optimization);
oal_module_symbol(dmac_green_ap_get_scan_enable);
oal_module_symbol(dmac_green_ap_set_scan_enable);

#endif /* _PRE_WLAN_FEATURE_GREEN_AP */

#ifdef  __cplusplus
#if     __cplusplus
    }
#endif
#endif
