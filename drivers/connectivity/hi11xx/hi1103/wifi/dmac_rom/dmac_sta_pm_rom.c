


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_sta_pm.h"
#include "dmac_psm_sta.h"
#include "dmac_ext_if.h"
#include "mac_resource.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_psm_ap.h"
#include "pm_extern.h"
#include "dmac_p2p.h"
#include "dmac_config.h"
#include "dmac_mgmt_classifier.h"

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif
#include "frw_timer.h"
#include "hal_device_fsm.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_STA_PM_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void dmac_pm_key_info_dump(dmac_vap_stru  *pst_dmac_vap)
{
    mac_sta_pm_handler_stru     *pst_mac_sta_pm_handle;

    if ((WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode) || (IS_P2P_DEV(&pst_dmac_vap->st_vap_base_info)))
    {
        return;
    }

    pst_mac_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;
    if (OAL_FALSE == pst_mac_sta_pm_handle->en_is_fsm_attached)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_key_info_dump::pm fsm not attached.}");
        return;
    }

    OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{tbtt_cnt[%d],bcn_cnt[%d],bcn_tout_cnt[%d],b_tout_forbid_sleep_cnt[%d],deep_sleep_cnt[%d],single_rx_cnt[%d],b_tout_set_chain[%d].}",
                            7,pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_TBTT_CNT],pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_PSM_BEACON_CNT],
                            pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_BEACON_TIMEOUT_CNT],pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_BCNTIMOUT_DIS_ALLOW_SLEEP],
                            pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_DEEP_DOZE_CNT],pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_SINGLE_BCN_RX_CNT],
                            pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_BCN_TIMEOUT_SET_RX_CHAIN]);
}

oal_uint32 dmac_pm_sta_wait_for_mcast_callback(void *p_arg)
{
    dmac_vap_stru                   *pst_dmac_vap = (dmac_vap_stru *)(p_arg);
    mac_sta_pm_handler_stru         *pst_sta_pm_handle;

    pst_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;
    pst_sta_pm_handle->aul_pmDebugCount[PM_MSG_DTIM_TMOUT_SLEEP]++;

    dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_LAST_MCAST, 0, OAL_PTR_NULL);
    return OAL_SUCC;
}

oal_void dmac_pm_sta_wait_for_mcast(dmac_vap_stru *pst_dmac_vap, mac_sta_pm_handler_stru *pst_mac_sta_pm_handle)
{
    if (pst_mac_sta_pm_handle->st_mcast_timer.en_is_registerd != OAL_TRUE)
    {
        FRW_TIMER_CREATE_TIMER(&(pst_mac_sta_pm_handle->st_mcast_timer),
                        dmac_pm_sta_wait_for_mcast_callback,
                        pst_mac_sta_pm_handle->us_mcast_timeout ,
                        pst_dmac_vap,
                        OAL_FALSE,
                        OAM_MODULE_ID_DMAC,
                        pst_dmac_vap->st_vap_base_info.ul_core_id);

    }
    else
    {
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_sta_wait_for_mcast::timer is registerd,doze event[%d],now time[%d],timer[%d],curr[%d]}",
                           pst_mac_sta_pm_handle->uc_doze_event,(oal_uint32)OAL_TIME_GET_STAMP_MS(),pst_mac_sta_pm_handle->st_mcast_timer.ul_time_stamp,
                                            pst_mac_sta_pm_handle->st_mcast_timer.ul_curr_time_stamp);
    }
}

oal_void dmac_pm_sta_doze_state_trans(dmac_vap_stru *pst_dmac_vap, oal_uint16 us_event)
{
    oal_uint8                uc_doze_trans_flag;
    mac_sta_pm_handler_stru *pst_handler = &pst_dmac_vap->st_sta_pm_handler;

    uc_doze_trans_flag = (pst_handler->en_beacon_frame_wait) | (pst_handler->st_null_wait.en_doze_null_wait << 1) | (pst_handler->en_more_data_expected << 2)
                | (pst_handler->st_null_wait.en_active_null_wait << 3) | (pst_handler->en_direct_change_to_active << 4);

    if (us_event != STA_PWR_EVENT_BEACON_TIMEOUT)
    {
        if ((OAL_FALSE == uc_doze_trans_flag) && (OAL_TRUE == dmac_can_sta_doze_prot(pst_dmac_vap)))
        {
            pst_handler->uc_doze_event = (oal_uint8)us_event;
            pst_handler->uc_state_fail_doze_trans_cnt = 0;                      //失败统计清零
            pst_handler->uc_can_sta_sleep = OAL_TRUE;                           //允许深睡
            oal_fsm_trans_to_state(&pst_handler->st_oal_fsm, STA_PWR_SAVE_STATE_DOZE);
        }
        else if (OAL_FALSE != uc_doze_trans_flag)
        {
            /* 睡眠null发送完成允许协议上切到doze状态,但不向平台投票睡眠,避免重复的睡眠唤醒null帧发送 */
            if (STA_PWR_EVENT_SEND_NULL_SUCCESS == us_event)
            {
                pst_handler->uc_can_sta_sleep = OAL_FALSE;                    //只允许切状态不允许睡
                pst_handler->uc_doze_event = (oal_uint8)us_event;
                oal_fsm_trans_to_state(&pst_handler->st_oal_fsm, STA_PWR_SAVE_STATE_DOZE);
            }

            pst_handler->uc_state_fail_doze_trans_cnt++;

            if (DMAC_STATE_DOZE_TRANS_FAIL_NUM == pst_handler->uc_state_fail_doze_trans_cnt)
            {
                pst_handler->uc_state_fail_doze_trans_cnt = 0;

                OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_sta_doze_state_trans::now event:[%d],but wait flag:[%d] can not vote sleep.}",us_event,uc_doze_trans_flag);
            }
        }
    }
    else
    {
        /* beacon连续收不到uc_bcn_tout_max_cnt次不睡眠低功耗状态停在awake状态 or 正在发送数据,唤醒的null帧,有more data */
        if ((pst_dmac_vap->bit_beacon_timeout_times > pst_dmac_vap->uc_bcn_tout_max_cnt) || (uc_doze_trans_flag & (BIT2 | BIT3 | BIT4)))
        {
            pst_handler->aul_pmDebugCount[PM_MSG_BCNTIMOUT_DIS_ALLOW_SLEEP]++;
        #ifndef HI110x_EDA
            return;
        #endif
        }

        pst_handler->uc_can_sta_sleep = OAL_TRUE;                             //beacon超时允许深睡
        pst_handler->en_beacon_frame_wait = OAL_FALSE;
        pst_handler->st_null_wait.en_doze_null_wait = OAL_FALSE;
        pst_handler->en_more_data_expected = OAL_FALSE;
        pst_handler->st_null_wait.en_active_null_wait = OAL_FALSE;
        pst_handler->en_direct_change_to_active  = OAL_FALSE;

        pst_handler->uc_doze_event = (oal_uint8)us_event;

        oal_fsm_trans_to_state(&pst_handler->st_oal_fsm, STA_PWR_SAVE_STATE_DOZE);
    }
}


oal_void dmac_pm_sta_state_trans(mac_sta_pm_handler_stru* pst_handler,oal_uint8 uc_state, oal_uint16 us_event)
{
    oal_fsm_stru    *pst_oal_fsm = &pst_handler->st_oal_fsm;
    dmac_vap_stru   *pst_dmac_vap;

    pst_dmac_vap = GET_PM_FSM_OWNER(pst_handler);

    if(uc_state >= STA_PWR_SAVE_STATE_BUTT)
    {
        /* OAM日志中不能使用%s*/
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_sta_state_trans:invalid state %d}",uc_state);
        return;
    }

    /* 1102新增切状态时记录抛事件的类型 */
    switch (uc_state)
    {
        case STA_PWR_SAVE_STATE_ACTIVE:
            pst_handler->uc_active_event = (oal_uint8)us_event;
            if (STA_PWR_SAVE_STATE_ACTIVE != GET_PM_STATE(pst_handler))
            {
                oal_fsm_trans_to_state(pst_oal_fsm, uc_state);
            }
            else
            {
                OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_sta_state_trans::dmac event:[%d]trans state to active in[%d]state}", us_event, GET_PM_STATE(pst_handler));
            }
        break;

        case STA_PWR_SAVE_STATE_AWAKE:
            pst_handler->uc_awake_event = (oal_uint8)us_event;
            if (STA_PWR_SAVE_STATE_AWAKE != GET_PM_STATE(pst_handler))
            {
                oal_fsm_trans_to_state(pst_oal_fsm, uc_state);
            }
            else
            {
                OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_sta_state_trans::dmac event:[%d]trans state to awake in awake}", us_event);
            }
        break;

        /* 必须满足条件才能切换到doze状态,否则会造成sta没有真正睡下去,状态却已经切成doze,tbtt重复唤醒 */
        case STA_PWR_SAVE_STATE_DOZE:
        if (STA_PWR_SAVE_STATE_DOZE != GET_PM_STATE(pst_handler))
        {
            dmac_pm_sta_doze_state_trans(pst_dmac_vap, us_event);
        }
        else
        {
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_sta_state_trans::dmac event:[%d]trans state to doze in doze}", us_event);
        }
        break;

        default:
        break;
    }
    return;

}

oal_uint32 dmac_pm_sta_post_event(dmac_vap_stru* pst_dmac_vap, oal_uint16 us_type, oal_uint16 us_datalen, oal_uint8* pst_data)
{
    mac_sta_pm_handler_stru     *pst_handler;
    oal_uint32                  ul_ret;
    oal_uint8                   uc_pm_state;
    oal_uint8                   uc_event = 0;

    OAL_REFERENCE(uc_event);

    if(pst_dmac_vap == OAL_PTR_NULL)
    {
        OAM_WARNING_LOG1(0, OAM_SF_PWR, "{dmac_pm_sta_post_event::pst_dmac_vap null.event:[%d]}", us_type);
        return OAL_FAIL;
    }

    pst_handler = &pst_dmac_vap->st_sta_pm_handler;

    uc_pm_state = GET_PM_STATE(pst_handler);

    /* 上次切换到xx状态的事件 */
    switch (uc_pm_state)
    {
        case STA_PWR_SAVE_STATE_DOZE:
            uc_event = pst_handler->uc_doze_event;
        break;

        case STA_PWR_SAVE_STATE_AWAKE:
            uc_event = pst_handler->uc_awake_event;
        break;

        case STA_PWR_SAVE_STATE_ACTIVE:
            uc_event = pst_handler->uc_active_event;
        break;

        default:
        break;

    }

    /* 期望所在的状态抛出的事件和现实所在状态不一致的错误维测打印 */
    switch(us_type)
    {
        case STA_PWR_EVENT_TX_DATA:
        case STA_PWR_EVENT_TBTT:
        case STA_PWR_EVENT_FORCE_AWAKE:
        case STA_PWR_EVENT_RX_BEACON:
        case STA_PWR_EVENT_RX_DATA:
            if (STA_PWR_SAVE_STATE_DOZE != uc_pm_state)
            {
                OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_sta_post_event::now event:[%d],but event:[%d]change state to[%d]not doze}",us_type, uc_event, uc_pm_state);
            }
        break;

        case STA_PWR_EVENT_RX_UCAST:
        case STA_PWR_EVENT_LAST_MCAST:
        case STA_PWR_EVENT_TIM:
        case STA_PWR_EVENT_DTIM:
        case STA_PWR_EVENT_NORMAL_SLEEP:
            if (STA_PWR_SAVE_STATE_AWAKE != uc_pm_state)
            {
                OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_sta_post_event::now event:[%d],but event:[%d]change state to[%d]not awake}",us_type, uc_event, uc_pm_state);
            }
        break;

        default:
        break;

    }
    ul_ret = oal_fsm_event_dispatch(&pst_handler->st_oal_fsm, us_type, us_datalen, pst_data);

    return ul_ret;

}

oal_void dmac_pm_sta_detach(dmac_vap_stru *pst_dmac_vap)
{
    mac_sta_pm_handler_stru *pst_handler = OAL_PTR_NULL;

    pst_handler = &pst_dmac_vap->st_sta_pm_handler;
    if (OAL_FALSE == pst_handler->en_is_fsm_attached)
    {
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "dmac_pm_sta_detach::vap id[%d]pm fsm not attatched", pst_dmac_vap->st_vap_base_info.uc_vap_id);
        return;
    }

    /* 不是active状态切换到active状态 */
    if (GET_PM_STATE(pst_handler) != STA_PWR_SAVE_STATE_ACTIVE)
    {
        dmac_pm_sta_state_trans(pst_handler, STA_PWR_SAVE_STATE_AWAKE, STA_PWR_EVENT_DETATCH);
        dmac_pm_sta_state_trans(pst_handler, STA_PWR_SAVE_STATE_ACTIVE, STA_PWR_EVENT_DETATCH);
    }

    if (OAL_TRUE == pst_handler->st_inactive_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_handler->st_inactive_timer));
    }

    if (OAL_TRUE == pst_handler->st_mcast_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_handler->st_mcast_timer));
    }
    pst_handler->en_is_fsm_attached = OAL_FALSE;

    return;
}

#endif /* _PRE_WLAN_FEATURE_STA_PM */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


