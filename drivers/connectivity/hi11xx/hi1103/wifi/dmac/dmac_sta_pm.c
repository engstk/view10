


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
#include "dmac_pm_sta.h"
#include "frw_timer.h"
#include "hal_device_fsm.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_STA_PM_C

OAL_STATIC oal_void sta_power_state_active_entry(oal_void *p_ctx);
OAL_STATIC oal_void sta_power_state_active_exit(oal_void *p_ctx);
OAL_STATIC oal_uint32 sta_power_state_active_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_void sta_power_state_doze_entry(oal_void *p_ctx);

OAL_STATIC oal_void sta_power_state_doze_exit(oal_void *p_ctx);

OAL_STATIC oal_uint32 sta_power_state_doze_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data);

OAL_STATIC oal_void sta_power_state_awake_entry(oal_void *p_ctx);

OAL_STATIC oal_void sta_power_state_awake_exit(oal_void *p_ctx);

OAL_STATIC oal_uint32 sta_power_state_awake_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oal_uint32                 g_lightsleep_fe_awake_cnt = 0; //浅睡恢复前端计数
oal_uint32                 g_deepsleep_fe_awake_cnt  = 0; //深睡恢复前端计数

/* 全局状态机函数表 */
oal_fsm_state_info  g_sta_power_fsm_info[] = {

    {
        STA_PWR_SAVE_STATE_ACTIVE,
        "ACTIVE",
        sta_power_state_active_entry,
        sta_power_state_active_exit,
        sta_power_state_active_event,
    },

    {
        STA_PWR_SAVE_STATE_DOZE,
        "DOZE",
        sta_power_state_doze_entry,
        sta_power_state_doze_exit,
        sta_power_state_doze_event,
    },
    {
        STA_PWR_SAVE_STATE_AWAKE,
        "AWAKE",
        sta_power_state_awake_entry,
        sta_power_state_awake_exit,
        sta_power_state_awake_event,
    },
};
/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void dmac_pm_process_deassociate(dmac_vap_stru *pst_dmac_vap)
{
    mac_cfg_ps_open_stru         st_ps_open;
    mac_sta_pm_handler_stru     *pst_sta_pm_handler = &pst_dmac_vap->st_sta_pm_handler;
    mac_device_stru             *pst_mac_device;

    /*去关联后需要关闭协议低功耗否则会在下次重新关联时还会获取dhcp前进入低功耗模式--发睡眠null帧 */
    st_ps_open.uc_pm_enable      = MAC_STA_PM_SWITCH_OFF;
    st_ps_open.uc_pm_ctrl_type   = MAC_STA_PM_CTRL_TYPE_HOST;

    dmac_config_set_sta_pm_on(&(pst_dmac_vap->st_vap_base_info), OAL_SIZEOF(mac_cfg_ps_open_stru), (oal_uint8 *)&st_ps_open);

    /* 去关联后低功耗关闭标志清0,防止DBAC,ROAM不对称的开关低功耗,导致无法再打开低功耗 */
    pst_dmac_vap->uc_sta_pm_close_status = 0;

    /* 由于去关联删除定时器 */
    if(OAL_TRUE == pst_sta_pm_handler->st_inactive_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_sta_pm_handler->st_inactive_timer));
    }

    if (OAL_TRUE == pst_sta_pm_handler->st_mcast_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_sta_pm_handler->st_mcast_timer));
    }

    /* 去关联后输出本次关联过程中的协议低功耗关键统计信息(临时) */
    dmac_pm_key_info_dump(pst_dmac_vap);

    /* 去关联后协议低功耗统计计数清零 */
    OAL_MEMZERO(&(pst_sta_pm_handler->aul_pmDebugCount),OAL_SIZEOF(pst_sta_pm_handler->aul_pmDebugCount));

    /* p2p cl 去关联时恢复深睡,防止noa浅睡最后未收到beacon没有恢复深睡 */
    if (IS_P2P_CL(&(pst_dmac_vap->st_vap_base_info)))
    {
#ifdef _PRE_WLAN_FEATURE_P2P_NOA_DSLEEP
   /*TODO:mpw2上03 p2p noa深睡暂不支持，待pilot上硬件更改触发睡眠中断的流程后再打开*/
        /* 去使能p2p noa唤醒中断 */
        hal_vap_set_ext_noa_disable(pst_dmac_vap->pst_hal_vap);
#else
        PM_WLAN_EnableDeepSleep();
#endif
        pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
        if (pst_mac_device == OAL_PTR_NULL)
        {
            OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_process_deassociate::mac_res_get_dev fail,dev id[%d]}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        }
        else
        {
            if(OAL_TRUE == pst_mac_device->st_p2p_info.en_p2p_ps_pause)
            {
                OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_pm_process_deassociate::cl is ps pause need vote active]}");
                hal_device_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), HAL_DEVICE_EVENT_VAP_CHANGE_TO_ACTIVE, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
            }
        }
    }

    /* 去关联时vap下的dtim周期清零保证重新关联ap后能再次更新keepalive值 */
    pst_dmac_vap->uc_psm_dtim_period = 0;
}

oal_void dmac_pm_change_to_active_state(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_event)
{
    mac_sta_pm_handler_stru   *pst_pm_handler = &pst_dmac_vap->st_sta_pm_handler;

    /* No need to track this flag in ACTIVE state */
    pst_pm_handler->en_more_data_expected = OAL_FALSE;

    /* active data/null帧发成功重启activity 定时器 */
    dmac_psm_start_activity_timer(pst_dmac_vap,pst_pm_handler);

    if (STA_PWR_SAVE_STATE_ACTIVE != GET_PM_STATE(pst_pm_handler))
    {
        dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_ACTIVE, uc_event);

        /* 睡眠到唤醒的次数统计 */
        pst_pm_handler->aul_pmDebugCount[PM_MSG_WAKE_TO_ACTIVE]++;
    }
}

oal_void dmac_process_send_null_succ_event(dmac_vap_stru *pst_dmac_vap, mac_ieee80211_frame_stru  *pst_mac_hdr)
{
    mac_sta_pm_handler_stru     *pst_pm_handler = &pst_dmac_vap->st_sta_pm_handler;

    if (OAL_PTR_NULL == pst_mac_hdr)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_ps_process_send_null_succ_event::pst_mac_hdr NULL.}");
        return;
    }
    /* 收beacon从awake状态切换到active状态发送null帧成功 */
    if (STA_PWR_SAVE_STATE_ACTIVE == pst_mac_hdr->st_frame_control.bit_power_mgmt)
    {
        /* fast_ps才会再次进入active状态 */
        if (OAL_TRUE == pst_pm_handler->st_null_wait.en_active_null_wait)
        {
            pst_pm_handler->st_null_wait.en_active_null_wait  = OAL_FALSE;
            dmac_pm_change_to_active_state(pst_dmac_vap, STA_PWR_EVENT_SEND_NULL_SUCCESS);
        }
    }
    /* 超时函数进入,此时切换到doze状态 */
    else
    {
        if(OAL_TRUE == (pst_pm_handler->st_null_wait.en_doze_null_wait))
        {
            pst_pm_handler->st_null_wait.en_doze_null_wait  = OAL_FALSE;

            if (STA_PWR_SAVE_STATE_AWAKE == GET_PM_STATE(pst_pm_handler))
            {
                /* 唤醒到睡眠的次数统计 */
                pst_pm_handler->aul_pmDebugCount[PM_MSG_WAKE_TO_DOZE]++;
            }
            else if (STA_PWR_SAVE_STATE_ACTIVE == GET_PM_STATE(pst_pm_handler))
            {
                /* 唤醒到睡眠的次数统计 */
                pst_pm_handler->aul_pmDebugCount[PM_MSG_ACTIVE_TO_DOZE]++;
            }
            else
            {
                OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_null_frame_complete_sta::event[%d] change state doze}", pst_pm_handler->uc_doze_event);
            }

            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_DOZE, STA_PWR_EVENT_SEND_NULL_SUCCESS);
        }
    }

}


OAL_STATIC oal_void sta_power_state_active_entry(oal_void *p_ctx)
{
    mac_sta_pm_handler_stru    *pst_sta_pm_handler = (mac_sta_pm_handler_stru*)p_ctx;
    dmac_vap_stru              *pst_dmac_vap = GET_PM_FSM_OWNER(pst_sta_pm_handler);
    hal_device_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), HAL_DEVICE_EVENT_VAP_CHANGE_TO_ACTIVE, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));

    /* 状态机attach时不起定时器,开低功耗时起 */
    if (STA_PWR_SAVE_STATE_ACTIVE != GET_PM_STATE(pst_sta_pm_handler))
    {
        dmac_psm_start_activity_timer(pst_dmac_vap, pst_sta_pm_handler);
    }
    return;
}


OAL_STATIC oal_void sta_power_state_active_exit(oal_void *p_ctx)
{
    return;
}


OAL_STATIC oal_uint32 sta_power_state_active_event(oal_void   *p_ctx,
                                                        oal_uint16    us_event,
                                                        oal_uint16    us_event_data_len,
                                                        oal_void      *p_event_data)
{
    oal_uint32                       ul_ret;
    mac_ieee80211_frame_stru        *pst_mac_hdr;
    dmac_vap_stru                   *pst_dmac_vap = OAL_PTR_NULL;
    mac_device_stru                 *pst_mac_device;

    mac_sta_pm_handler_stru*  pst_pm_handler = (mac_sta_pm_handler_stru*)p_ctx;
    if(OAL_PTR_NULL == pst_pm_handler)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{sta_power_state_active_event::pst_pm_handler null.}");
        return OAL_FAIL;
    }

    pst_dmac_vap = GET_PM_FSM_OWNER(pst_pm_handler);
    if(OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{sta_power_state_active_event::pst_dmac_vap null.}");
        return OAL_FAIL;
    }
    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{sta_power_state_active_event::pst_mac_device null.}");
        return OAL_FAIL;
    }

    switch(us_event)
    {
        case STA_PWR_EVENT_TIMEOUT:
            //OAM_INFO_LOG0(0, OAM_SF_PWR, "{sta_power_state_active_event::dmac_send_null_frame_to_ap doze}");
            ul_ret = dmac_send_null_frame_to_ap(pst_dmac_vap, STA_PWR_SAVE_STATE_DOZE, OAL_FALSE);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{sta_power_state_active_event:dmac_send_null_frame_to_ap:[%d].}", ul_ret);

                /* pspoll模式第一个睡眠的null帧发送失败重启睡眠定时器重新发送 */
                dmac_psm_start_activity_timer(pst_dmac_vap,pst_pm_handler);
            }
        break;

        /* active 状态下null帧发送成功重新进入节能模式 */
        case STA_PWR_EVENT_SEND_NULL_SUCCESS:
        	pst_mac_hdr = (mac_ieee80211_frame_stru *)(p_event_data);
            dmac_process_send_null_succ_event(pst_dmac_vap, pst_mac_hdr);
        break;

        /* active 下启动keepalive定时器 */
        case STA_PWR_EVENT_KEEPALIVE:
            /* 此时开启keepalive */
            pst_dmac_vap->st_vap_base_info.st_cap_flag.bit_keepalive   =  OAL_TRUE;

            if (OAL_TRUE != pst_pm_handler->st_inactive_timer.en_is_registerd)
            {
                /* 节能下超时时间和非节能下的超时时间不一样，启动定时器 */
                FRW_TIMER_CREATE_TIMER(&(pst_pm_handler->st_inactive_timer),
                                    dmac_psm_alarm_callback,
                                    pst_pm_handler->ul_activity_timeout ,
                                    pst_dmac_vap,
                                    OAL_FALSE,
                                    OAM_MODULE_ID_DMAC,
                                    pst_mac_device->ul_core_id);
            }
        break;

        /* p2p 此时应RESTART active->doze的定时器 */
        case STA_PWR_EVENT_P2P_SLEEP:
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_TRUE);

            if ((WLAN_MIB_PWR_MGMT_MODE_PWRSAVE == mac_mib_get_powermanagementmode(&(pst_dmac_vap->st_vap_base_info))) &&
              (OAL_TRUE == dmac_is_sta_fast_ps_enabled(pst_pm_handler)))
            {
                dmac_psm_start_activity_timer(pst_dmac_vap,pst_pm_handler);
            }
        break;

        case STA_PWR_EVENT_P2P_AWAKE:
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_FALSE);
        break;

        case STA_PWR_EVENT_DEASSOCIATE:
            dmac_pm_process_deassociate(pst_dmac_vap);
        break;

        default:
        break;
    }
    return OAL_SUCC;
}


oal_uint8  dmac_is_sta_allow_to_sleep(mac_device_stru *pst_device, dmac_vap_stru *pst_dmac_vap, mac_sta_pm_handler_stru* pst_sta_pm_handler)
{
    /* 正在扫描不睡眠 */
    if (MAC_SCAN_STATE_RUNNING == pst_device->en_curr_scan_state)
    {
        pst_sta_pm_handler->aul_pmDebugCount[PM_MSG_SCAN_DIS_ALLOW_SLEEP]++;
        return OAL_FALSE;
    }

    /* 睡眠null帧发送成功,低功耗状态机切到doze,但如果不满足深睡条件不投票睡眠 */
    if (OAL_FALSE == pst_sta_pm_handler->uc_can_sta_sleep)
    {
        pst_sta_pm_handler->aul_pmDebugCount[PM_MSG_NULL_NOT_SLEEP]++;
        return OAL_FALSE;
    }

    /* p2p 开启了noa oppps时收beacon时不投睡眠票,仅在noa oppps期间浅睡 */
    if (OAL_TRUE == (oal_uint8)IS_P2P_PS_ENABLED(pst_dmac_vap))
    {
        pst_sta_pm_handler->aul_pmDebugCount[PM_MSG_PSM_P2P_PS]++;
        return OAL_FALSE;
    }

    /* DBAC running 不睡眠 */
    if(OAL_TRUE == mac_is_dbac_running(pst_device))
    {
        pst_sta_pm_handler->aul_pmDebugCount[PM_MSG_DBAC_DIS_ALLOW_SLEEP]++;
        return OAL_FALSE;
    }

    return OAL_TRUE;

}



oal_void dmac_power_state_process_doze(mac_sta_pm_handler_stru* pst_sta_pm_handler)
{

    mac_device_stru             *pst_device;
    dmac_vap_stru               *pst_dmac_vap;

    pst_dmac_vap    = GET_PM_FSM_OWNER(pst_sta_pm_handler);

    pst_device      = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_psm_check_txrx_state::pst_device[id:0](%p) is NULL!}",
                    pst_dmac_vap->st_vap_base_info.uc_device_id,
                    pst_device);
        return ;
    }


    pst_sta_pm_handler->aul_pmDebugCount[PM_MSG_PROCESS_DOZE_CNT]++;

    if(OAL_FALSE == dmac_is_sta_allow_to_sleep(pst_device,pst_dmac_vap,pst_sta_pm_handler))
    {
        return ;
    }
    /* 超时函数内不能自己删自己的定时器,否则frw timer那有刷屏打印,这里是为了保证协议投票前此定时器一定是destroy的 */
    if ((OAL_TRUE == pst_sta_pm_handler->st_mcast_timer.en_is_registerd) && (pst_sta_pm_handler->uc_doze_event != STA_PWR_EVENT_LAST_MCAST))
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_sta_pm_handler->st_mcast_timer));
        OAM_WARNING_LOG1(0, OAM_SF_ANY,"dmac_power_state_process_doze::event[%d] vote sleep,but mcast timer is registerd",
                                pst_sta_pm_handler->uc_doze_event);
    }

    /* 进入doze状态时保证inactive定时器被destroy */
    if (OAL_TRUE == pst_sta_pm_handler->st_inactive_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_sta_pm_handler->st_inactive_timer));
    }

    pst_sta_pm_handler->aul_pmDebugCount[PM_MSG_DEEP_DOZE_CNT]++;

    hal_device_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), HAL_DEVICE_EVENT_VAP_CHANGE_TO_DOZE, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));

#ifdef PM_WLAN_FPGA_DEBUG
    /*睡眠流程时间观测点P12,拉低*/
    //WRITEW(0x50002174,READW(0x50002174)&(~(1<<2)));
#endif

    return;
}


OAL_STATIC oal_void sta_power_state_doze_entry(oal_void *p_ctx)
{
    mac_sta_pm_handler_stru*    pst_sta_pm_handler = (mac_sta_pm_handler_stru*)p_ctx;

    if(OAL_PTR_NULL == pst_sta_pm_handler)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{sta_power_state_doze_entry::pst_pm_handler null.}");
        return;
    }

    /* 将所有缓存帧发送到host,todo */

    /* 处理doze状态下的深睡和浅睡，调用平台的接口 */
    dmac_power_state_process_doze(pst_sta_pm_handler);

    /* Increment the number of STA sleeps */
    //DMAC_STA_PSM_STATS_INCR(pst_sta_pm_handler->st_psm_stat_info.ul_sta_doze_times);
    return;
}


OAL_STATIC oal_void sta_power_state_doze_exit(oal_void *p_ctx)
{
    return;
}


OAL_STATIC oal_uint32 sta_power_state_doze_event(oal_void   *p_ctx,
                                                        oal_uint16    us_event,
                                                        oal_uint16    us_event_data_len,
                                                        oal_void      *p_event_data)
{
    oal_uint32                  ul_ret;
    mac_sta_pm_handler_stru*    pst_pm_handler = (mac_sta_pm_handler_stru*)p_ctx;
    dmac_vap_stru              *pst_dmac_vap;

    if(OAL_PTR_NULL == pst_pm_handler)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{sta_power_state_doze_event::pst_pm_handler null.}");
        return OAL_FAIL;
    }

    pst_dmac_vap = GET_PM_FSM_OWNER(pst_pm_handler);
    if(OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{sta_power_state_doze_event::pst_dmac_vap null.}");
        return OAL_FAIL;
    }

    switch(us_event)
    {
        /* null帧发送成功,发送完成,此时却是与doze状态的异常处理 */
        case STA_PWR_EVENT_SEND_NULL_SUCCESS:
        case STA_PWR_EVENT_TX_DATA:
        case STA_PWR_EVENT_TX_COMPLETE:
            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_AWAKE, us_event);
            pst_pm_handler->aul_pmDebugCount[PM_MSG_HOST_AWAKE]++;
        break;

        /* DOZE状态下的TBTT事件 */
        case STA_PWR_EVENT_TBTT:
        case STA_PWR_EVENT_RX_BEACON:
        case STA_PWR_EVENT_RX_DATA:
            /* TBTT事件切换到awake状态 */
            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_AWAKE, us_event);
        break;

        /* 异常处理,fast ps 模式下，切到doze后tbtt中断还未上来唤醒devie，手动唤醒 */
        case STA_PWR_EVENT_FORCE_AWAKE:
            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_AWAKE, us_event);
        break;

        case STA_PWR_EVENT_P2P_SLEEP:
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_TRUE);
        break;

        case STA_PWR_EVENT_P2P_AWAKE:
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_FALSE);
        break;

        /*非节能模式下，处于doze状态的异常处理 */
        case STA_PWR_EVENT_NO_POWERSAVE:
            if (MAC_VAP_STATE_UP == pst_dmac_vap->st_vap_base_info.en_vap_state)
            {
                ul_ret = dmac_send_null_frame_to_ap(pst_dmac_vap, STA_PWR_SAVE_STATE_ACTIVE, OAL_FALSE);
                if (OAL_SUCC != ul_ret)
                {
                    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{sta_power_state_awake_event:dmac_send_null_frame_to_ap:[%d].}", ul_ret);
                }
            }
            //dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_AWAKE, us_event);/* 先唤醒 */
            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_ACTIVE, us_event);/* 再设状态 */
        break;
        case STA_PWR_EVENT_DEASSOCIATE:
            dmac_pm_process_deassociate(pst_dmac_vap);
        break;

        case STA_PWR_EVENT_TX_MGMT:
        case STA_PWR_EVENT_ENABLE_FRONT_END:
            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_ACTIVE, us_event);
        break;
        default:
        break;
    }
    return OAL_SUCC;
}


OAL_STATIC oal_void sta_power_state_awake_entry(oal_void *p_ctx)
{
    mac_sta_pm_handler_stru     *pst_sta_pm_handler = (mac_sta_pm_handler_stru*)p_ctx;
    dmac_vap_stru               *pst_dmac_vap;
    mac_device_stru             *pst_mac_device;
    oal_uint8                    uc_event;
    if(OAL_PTR_NULL == pst_sta_pm_handler)
    {
        return;
    }

    pst_dmac_vap    = GET_PM_FSM_OWNER(pst_sta_pm_handler);

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{sta_power_state_awake_entry::pst_mac_device null.}");
        return;
    }

    //PM_WLAN_PRINT("PSM awake entry vote wakeup"NEWLINE);
    #ifdef HI1102_FPGA
        /*睡眠流程时间观测点P12*/
        //WRITEW(0x50002174,READW(0x50002174)|((1<<2)));
    #endif

    /* 动态配置offset需要hal dev状态机事件区分是否为tbtt唤醒 */
    if (STA_PWR_EVENT_TBTT == pst_sta_pm_handler->uc_awake_event)
    {
        uc_event = HAL_DEVICE_EVENT_TBTT_WAKE_UP;
    }
    else
    {
        uc_event = HAL_DEVICE_EVENT_VAP_CHANGE_TO_AWAKE;
    }
    hal_device_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), uc_event, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
}

OAL_STATIC oal_void sta_power_state_awake_exit(oal_void *p_ctx)
{
    return;
}


OAL_STATIC oal_uint32 sta_power_state_awake_event(oal_void   *p_ctx,
                                                        oal_uint16    us_event,
                                                        oal_uint16    us_event_data_len,
                                                        oal_void      *p_event_data)
{
    oal_uint32                           ul_ret;
    mac_ieee80211_frame_stru            *pst_mac_hdr;
    dmac_vap_stru                       *pst_dmac_vap;

    mac_sta_pm_handler_stru*    pst_pm_handler = (mac_sta_pm_handler_stru*)p_ctx;
    if(OAL_PTR_NULL == pst_pm_handler)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{sta_power_state_awake_event::pst_pm_handler null}");
        return OAL_FAIL;
    }

    pst_dmac_vap = GET_PM_FSM_OWNER(pst_pm_handler);
    if(OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{sta_power_state_awake_event::pst_dmac_vap null}");
        return OAL_FAIL;
    }


    switch(us_event)
    {
        case STA_PWR_EVENT_RX_UCAST:
            ul_ret = dmac_send_pspoll_to_ap(pst_dmac_vap);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{sta_power_state_awake_event::rx ucast event dmac_send_pspoll_to_ap fail [%d].}", ul_ret);
            }

        break;

        /* TIM is set */
        case STA_PWR_EVENT_TIM:
            if (OAL_TRUE == pst_pm_handler->en_more_data_expected)
            {
                ul_ret = dmac_send_pspoll_to_ap(pst_dmac_vap);
                if (OAL_SUCC != ul_ret)
                {
                    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{sta_power_state_awake_event::rx beacon event dmac_send_pspoll_to_ap fail [%d].}", ul_ret);
                }
            }
        break;

        /* DTIM set, stay in AWAKE mode to recieve all broadcast frames */
        case STA_PWR_EVENT_DTIM:
            pst_pm_handler->en_more_data_expected = OAL_TRUE;
            dmac_pm_sta_wait_for_mcast(pst_dmac_vap,pst_pm_handler);
            pst_pm_handler->aul_pmDebugCount[PM_MSG_DTIM_AWAKE]++;
        break;

        /* AWAKE状态下接收null帧发送成功的处理 */
        case STA_PWR_EVENT_SEND_NULL_SUCCESS:
            pst_mac_hdr = (mac_ieee80211_frame_stru *)(p_event_data);
            dmac_process_send_null_succ_event(pst_dmac_vap, pst_mac_hdr);
        break;

       /* 睡眠事件 */
        case STA_PWR_EVENT_BEACON_TIMEOUT:
        case STA_PWR_EVENT_NORMAL_SLEEP:
#ifdef _PRE_WLAN_DOWNLOAD_PM
        case STA_PWR_EVENT_NOT_EXCEED_MAX_SLP_TIME:
#endif
        case STA_PWR_EVENT_NO_PS_FRM:
            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_DOZE, us_event);
        break;

        /* Awake 状态下收到最后一个组播/广播 */
        case STA_PWR_EVENT_LAST_MCAST:
            pst_pm_handler->en_more_data_expected   = OAL_FALSE;
            pst_pm_handler->aul_pmDebugCount[PM_MSG_LAST_DTIM_SLEEP]++;
            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_DOZE, us_event);
        break;

        /* 超时函数处理事件 pspoll模式下,不需要超时进doze,直接看beacon tim 元素 */
        case STA_PWR_EVENT_TIMEOUT:
            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_DOZE, us_event);
        break;

        case STA_PWR_EVENT_P2P_SLEEP:
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_TRUE);
        break;

        case STA_PWR_EVENT_P2P_AWAKE:
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_FALSE);
        break;

        /* 非节能模式事件 */
         case STA_PWR_EVENT_NO_POWERSAVE:
            if (MAC_VAP_STATE_UP == pst_dmac_vap->st_vap_base_info.en_vap_state)
            {
                ul_ret = dmac_send_null_frame_to_ap(pst_dmac_vap, STA_PWR_SAVE_STATE_ACTIVE, OAL_FALSE);
                if (OAL_SUCC != ul_ret)
                {
                    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{sta_power_state_awake_event::dmac_send_null_frame_to_ap:[%d]}", ul_ret);
                }
            }

            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_ACTIVE, us_event);/* 设状态 */
        break;
        case STA_PWR_EVENT_DEASSOCIATE:
            dmac_pm_process_deassociate(pst_dmac_vap);
        break;

        case STA_PWR_EVENT_TX_MGMT:
        case STA_PWR_EVENT_ENABLE_FRONT_END:
            dmac_pm_sta_state_trans(pst_pm_handler, STA_PWR_SAVE_STATE_ACTIVE, us_event);
        break;
        default:
        break;
    }
    return OAL_SUCC;
}

oal_void dmac_sta_initialize_psm_globals(mac_sta_pm_handler_stru *p_handler)
{
        p_handler->en_beacon_frame_wait             = OAL_TRUE;
        p_handler->st_null_wait.en_active_null_wait = OAL_FALSE;
        p_handler->st_null_wait.en_doze_null_wait   = OAL_FALSE;
        p_handler->en_more_data_expected            = OAL_FALSE;
        p_handler->en_send_null_delay               = OAL_FALSE;
        p_handler->ul_tx_rx_activity_cnt            = 0;
        p_handler->en_direct_change_to_active       = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
        p_handler->uc_uaspd_sp_status               = DMAC_SP_NOT_IN_PROGRESS;
        p_handler->uc_eosp_timeout_cnt              = 0;
#endif
        p_handler->ul_activity_timeout              = MIN_ACTIVITY_TIME_OUT;
        p_handler->us_mcast_timeout                 = DMAC_RECE_MCAST_TIMEOUT;
        p_handler->ul_ps_keepalive_cnt              = 0;
        p_handler->ul_ps_keepalive_max_num          = WLAN_PS_KEEPALIVE_MAX_NUM;
        p_handler->uc_timer_fail_doze_trans_cnt     = 0;
        p_handler->uc_state_fail_doze_trans_cnt     = 0;
        p_handler->en_ps_back_active_pause          = OAL_FALSE;
        p_handler->en_ps_back_doze_pause            = OAL_FALSE;
        p_handler->uc_psm_timer_restart_cnt         = 0;

        p_handler->uc_tbtt_cnt_since_full_bcn       = 0;
}


void dmac_pm_sta_attach(dmac_vap_stru *pst_dmac_vap)
{
    mac_sta_pm_handler_stru *pst_handler  = OAL_PTR_NULL;
    oal_uint8                auc_fsm_name[6] = {0};
    oal_uint32               ul_ret;

    pst_handler = &pst_dmac_vap->st_sta_pm_handler;
    if (pst_handler->en_is_fsm_attached)
    {
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "{dmac_pm_sta_attach::vap id[%d]pm fsm aready attached.}", pst_dmac_vap->st_vap_base_info.uc_vap_id);
        return;
    }

    OAL_MEMZERO(pst_handler, OAL_SIZEOF(mac_sta_pm_handler_stru));

    dmac_sta_initialize_psm_globals(pst_handler);
    pst_handler->uc_vap_ps_mode                   = NO_POWERSAVE;
    pst_handler->en_hw_ps_enable                  = OAL_TRUE;    /* 是否仅是协议栈低功耗,配置命令可配 */

    /* 准备一个唯一的fsmname */
    auc_fsm_name[0] = (oal_uint8)(pst_dmac_vap->st_vap_base_info.ul_core_id);
    auc_fsm_name[1] = pst_dmac_vap->st_vap_base_info.uc_chip_id;
    auc_fsm_name[2] = pst_dmac_vap->st_vap_base_info.uc_device_id;
    auc_fsm_name[3] = pst_dmac_vap->st_vap_base_info.uc_vap_id;
    auc_fsm_name[4] = pst_dmac_vap->st_vap_base_info.en_vap_mode;
    auc_fsm_name[5] = 0;

    ul_ret = oal_fsm_create(pst_dmac_vap,
                            auc_fsm_name,
                            pst_handler,
                            &pst_handler->st_oal_fsm,
                            STA_PWR_SAVE_STATE_ACTIVE,
                            g_sta_power_fsm_info,
                            OAL_SIZEOF(g_sta_power_fsm_info)/OAL_SIZEOF(oal_fsm_state_info));
    if (OAL_SUCC != ul_ret)
    {
        pst_handler->en_is_fsm_attached = OAL_FALSE;
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "dmac_pm_sta_attach::vap id[%d]pm fsm attach failed.", pst_dmac_vap->st_vap_base_info.uc_vap_id);
        return;
    }
    pst_handler->en_is_fsm_attached = OAL_TRUE;
}
#endif /* _PRE_WLAN_FEATURE_STA_PM */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

