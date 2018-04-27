


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
#include "dmac_power.h"
#include "dmac_csa_sta.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CSA_STA_C

OAL_STATIC oal_uint32 dmac_sta_csa_fsm_init_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data);
OAL_STATIC oal_uint32 dmac_sta_csa_fsm_start_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data);
OAL_STATIC oal_uint32 dmac_sta_csa_fsm_switch_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data);
OAL_STATIC oal_uint32 dmac_sta_csa_fsm_wait_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data);
OAL_STATIC oal_uint32 dmac_sta_csa_fsm_invalid_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data);
OAL_STATIC oal_void dmac_sta_csa_fsm_init_entry(oal_void *p_ctx);
OAL_STATIC oal_void dmac_sta_csa_fsm_invalid_entry(oal_void *p_ctx);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oal_fsm_state_info g_sta_csa_fsm_info[] = {
    {
        WLAN_STA_CSA_FSM_INIT,
        "INIT",
        dmac_sta_csa_fsm_init_entry,
        OAL_PTR_NULL,
        dmac_sta_csa_fsm_init_event,
    },
    {
        WLAN_STA_CSA_FSM_START,
        "START",
        OAL_PTR_NULL,
        OAL_PTR_NULL,
        dmac_sta_csa_fsm_start_event,
    },
    {
        WLAN_STA_CSA_FSM_SWITCH,
        "SWITCH",
        OAL_PTR_NULL,
        OAL_PTR_NULL,
        dmac_sta_csa_fsm_switch_event,
    },
    {
        WLAN_STA_CSA_FSM_WAIT,
        "WAIT",
        OAL_PTR_NULL,
        OAL_PTR_NULL,
        dmac_sta_csa_fsm_wait_event,
    },
    {
        WLAN_STA_CSA_FSM_INVALID,
        "INVALID",
        dmac_sta_csa_fsm_invalid_entry,
        OAL_PTR_NULL,
        dmac_sta_csa_fsm_invalid_event,
    }
};


/*****************************************************************************
  3 函数实现
*****************************************************************************/

OAL_STATIC oal_void dmac_sta_csa_fsm_init_entry(oal_void *p_ctx)
{
    mac_sta_csa_fsm_info_stru       *pst_csa_fsm_info = OAL_PTR_NULL;
    mac_ch_switch_info_stru         *pst_ch_switch_info = OAL_PTR_NULL;
    mac_vap_stru                    *pst_mac_vap = (mac_vap_stru *)p_ctx;

    if(WLAN_STA_CSA_FSM_INIT == dmac_sta_csa_fsm_get_current_state(pst_mac_vap))
    {
        return;
    }

    pst_csa_fsm_info = &(pst_mac_vap->st_sta_csa_fsm_info);
    pst_ch_switch_info  = &(pst_mac_vap->st_ch_switch_info);

    if(OAL_TRUE == pst_csa_fsm_info->st_csa_handle_timer.en_is_registerd)
    {
        FRW_TIMER_DESTROY_TIMER(&(pst_csa_fsm_info->st_csa_handle_timer));
    }

    if(OAL_TRUE == dmac_sta_csa_is_in_waiting(pst_mac_vap))
    {
        dmac_chan_enable_machw_tx(pst_mac_vap); //恢复发送，防止问题ap发的beacon让停止发送
    }

    pst_csa_fsm_info->uc_next_expect_cnt              = 0;
    pst_csa_fsm_info->en_expect_cnt_status            = OAL_FALSE;
    pst_csa_fsm_info->uc_csa_scan_after_tbtt          = 0;
    pst_csa_fsm_info->uc_sta_csa_last_cnt             = 0;
    pst_ch_switch_info->st_old_channel.en_band        = WLAN_BAND_BUTT;
    pst_ch_switch_info->st_old_channel.uc_chan_number = 0;
    pst_ch_switch_info->st_old_channel.en_bandwidth   = WLAN_BAND_WIDTH_BUTT;
    pst_ch_switch_info->en_new_bandwidth              = WLAN_BAND_WIDTH_BUTT;

    return;
}


OAL_STATIC oal_bool_enum dmac_sta_csa_have_vap_is_in_assoc(mac_vap_stru *pst_mac_vap)
{
    oal_uint8          uc_vap_idx;
    mac_device_stru   *pst_mac_device    = OAL_PTR_NULL;
    mac_vap_stru      *pst_index_mac_vap = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CSA,"{dmac_sta_csa_have_vap_is_in_assoc::get mac_device(%d) is null. Return}",pst_mac_vap->uc_device_id);
        return OAL_TRUE;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_index_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if(((pst_index_mac_vap->en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) && (pst_index_mac_vap->en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC))
#ifdef _PRE_WLAN_FEATURE_ROAM
        || (MAC_VAP_STATE_ROAMING == pst_index_mac_vap->en_vap_state)
#endif
            )
        {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CSA,"{dmac_sta_csa_have_vap_is_in_assoc::vap_%d_state=[%d], Return OAL_TRUE}",
                pst_index_mac_vap->uc_vap_id, pst_index_mac_vap->en_vap_state);
            return OAL_TRUE;
        }

    }

    return OAL_FALSE;
}



static void dmac_sta_csa_fsm_trans_to_state(mac_vap_stru *pst_mac_vap,oal_uint8 uc_state)
{
    oal_bool_enum                   en_now_wait_status;
    oal_bool_enum                   en_now_next_status;
#ifdef _PRE_WLAN_FEATURE_NO_FRM_INT
    dmac_vap_stru                  *pst_dmac_vap;
#endif

    if(uc_state == dmac_sta_csa_fsm_get_current_state(pst_mac_vap))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{dmac_sta_csa_fsm_trans_to_state::now status = %d not need to change}",uc_state);
        return;
    }

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{dmac_sta_csa_fsm_trans_to_state::csa status change from [%d] to [%d]}",
        pst_mac_vap->st_sta_csa_fsm_info.st_oal_fsm.uc_cur_state,uc_state);

    en_now_wait_status = dmac_sta_csa_is_in_waiting(pst_mac_vap);
    oal_fsm_trans_to_state(&(pst_mac_vap->st_sta_csa_fsm_info.st_oal_fsm),uc_state);

    en_now_next_status = dmac_sta_csa_is_in_waiting(pst_mac_vap);
    if(en_now_wait_status != en_now_next_status)
    {
        #ifdef _PRE_WLAN_FEATURE_STA_PM
        /* 准备切信道,更新dtim,listen interval周期 按dtim 1唤醒产生tbtt中断 */
        dmac_psm_update_dtime_period(pst_mac_vap,(oal_uint8)mac_mib_get_dot11dtimperiod(pst_mac_vap),mac_mib_get_BeaconPeriod(pst_mac_vap));
        #ifdef _PRE_WLAN_FEATURE_NO_FRM_INT
        /* 最大允许跳过的beaocn数设置为0 */
        pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
        pst_dmac_vap->st_sta_pm_handler.uc_max_skip_bcn_cnt = 0;
        #endif
        #endif
    }
    return;
}


oal_uint32  dmac_sta_csa_timeout_fn(oal_void *p_arg)
{
    mac_vap_stru                    *pst_mac_vap = (mac_vap_stru *)(p_arg);

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CSA,"{dmac_sta_csa_timeout_fn:: To wait tbtt event}");

    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_SWITCH);
    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_sta_csa_start_timer(mac_vap_stru *pst_mac_vap,oal_ieee80211_channel_sw_ie *pst_csa_info)
{
    oal_uint32                      ul_timeout;
    mac_sta_csa_fsm_info_stru       *pst_sta_csa_fsm_info = &(pst_mac_vap->st_sta_csa_fsm_info);

    if(1 == pst_csa_info->mode)
    {
        dmac_chan_disable_machw_tx(pst_mac_vap);
    }

    pst_sta_csa_fsm_info->uc_sta_csa_last_cnt     = pst_csa_info->count;
    pst_sta_csa_fsm_info->uc_next_expect_cnt      = pst_csa_info->count;
    pst_mac_vap->st_ch_switch_info.uc_new_channel = pst_csa_info->new_ch_num;
    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_START);
    ul_timeout = mac_mib_get_BeaconPeriod(pst_mac_vap) * pst_csa_info->count ;
    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{dmac_sta_csa_start_timer::create timer,timeout = [%d](%d)}",
        ul_timeout,pst_csa_info->count);
    FRW_TIMER_CREATE_TIMER(&(pst_sta_csa_fsm_info->st_csa_handle_timer),
        dmac_sta_csa_timeout_fn,
        ul_timeout,
        (void *)pst_mac_vap,
        OAL_FALSE,
        OAM_MODULE_ID_DMAC,
        pst_mac_vap->ul_core_id);

    return;
}



OAL_STATIC oal_void dmac_sta_csa_fsm_start_get_csa_ie(mac_vap_stru *pst_mac_vap,oal_ieee80211_channel_sw_ie *pst_csa_info)
{
    oal_uint32                      ul_timeout;

    ul_timeout = mac_mib_get_BeaconPeriod(pst_mac_vap) * pst_csa_info->count;
    if(pst_mac_vap->st_sta_csa_fsm_info.uc_next_expect_cnt == pst_csa_info->count)
    {
        pst_mac_vap->st_sta_csa_fsm_info.en_expect_cnt_status = OAL_TRUE;
        FRW_TIMER_RESTART_TIMER(&(pst_mac_vap->st_sta_csa_fsm_info.st_csa_handle_timer),ul_timeout, OAL_FALSE);
    }
    /*count ==0 时立即切换,经讨论,考虑action帧cnt 非0，紧接着时候到beacon帧cnt为0的场景*/
    else if(0 == pst_csa_info->count)
    {
        pst_mac_vap->st_sta_csa_fsm_info.en_expect_cnt_status = OAL_FALSE;
        FRW_TIMER_RESTART_TIMER(&(pst_mac_vap->st_sta_csa_fsm_info.st_csa_handle_timer),ul_timeout, OAL_FALSE);
    }
    else
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{dmac_sta_csa_fsm_start_get_csa_ie::next_expect_cnt=[%d], new_cnt = [%d],fsm to WLAN_STA_CSA_FSM_INVALID}",
            pst_mac_vap->st_sta_csa_fsm_info.uc_next_expect_cnt,pst_csa_info->count);
        pst_mac_vap->st_sta_csa_fsm_info.uc_next_expect_cnt = pst_csa_info->count;
        pst_mac_vap->st_sta_csa_fsm_info.en_expect_cnt_status = OAL_FALSE;
        dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INVALID);
    }

    pst_mac_vap->st_sta_csa_fsm_info.uc_sta_csa_last_cnt = pst_csa_info->count;

    return;
}


OAL_STATIC oal_uint32 dmac_sta_csa_fsm_init_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data)
{
    oal_ieee80211_channel_sw_ie     *pst_csa_info;
    mac_vap_stru                    *pst_mac_vap = (mac_vap_stru *)p_ctx;

    switch(event)
    {
        case WLAN_STA_CSA_EVENT_GET_IE:
            if((OAL_PTR_NULL != event_data) && (event_data_len == OAL_SIZEOF(oal_ieee80211_channel_sw_ie)))
            {
                pst_csa_info = (oal_ieee80211_channel_sw_ie *)event_data;
                dmac_sta_csa_start_timer(pst_mac_vap,pst_csa_info);
            }
            break;
        default:
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_sta_csa_fsm_start_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data)
{
    oal_ieee80211_channel_sw_ie     *pst_csa_info;
    mac_vap_stru                    *pst_mac_vap = (mac_vap_stru *)p_ctx;

    switch(event)
    {
        case WLAN_STA_CSA_EVENT_GET_IE:
            if((OAL_PTR_NULL != event_data) && (event_data_len == OAL_SIZEOF(oal_ieee80211_channel_sw_ie)))
            {
                pst_csa_info = (oal_ieee80211_channel_sw_ie *)event_data;
                dmac_sta_csa_fsm_start_get_csa_ie(pst_mac_vap,pst_csa_info);
            }
            break;
        case WLAN_STA_CSA_EVENT_TBTT:
            if(pst_mac_vap->st_sta_csa_fsm_info.uc_next_expect_cnt > 0)
            {
                pst_mac_vap->st_sta_csa_fsm_info.uc_next_expect_cnt --;
                OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{dmac_sta_csa_fsm_start_event::uc_next_expect_cnt = %d}",
                    pst_mac_vap->st_sta_csa_fsm_info.uc_next_expect_cnt);
            }
            break;
        case WLAN_STA_CSA_EVENT_TO_INIT:
            dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INIT);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_sta_csa_fsm_switch_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data)
{
    mac_vap_stru                    *pst_mac_vap = (mac_vap_stru *)p_ctx;

    switch(event)
    {
        case WLAN_STA_CSA_EVENT_TBTT:
            if(MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state)
            {
                if(OAL_FALSE == dmac_sta_csa_have_vap_is_in_assoc(pst_mac_vap))
                {
                    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CSA,"{dmac_sta_csa_fsm_switch_event::switch channel from [%d] to [%d]}",
                    pst_mac_vap->st_channel.uc_chan_number,pst_mac_vap->st_ch_switch_info.uc_new_channel);
                    dmac_chan_sta_switch_channel(pst_mac_vap);
                    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_WAIT);
                }
                else
                {
                    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CSA,"{dmac_sta_csa_fsm_switch_event::have vap in assoc, csa return INIT}");
                    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INIT);
                }
            }
            break;
        case WLAN_STA_CSA_EVENT_TO_INIT:
            dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INIT);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_sta_csa_fsm_wait_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data)
{
    mac_vap_stru                    *pst_mac_vap;
    mac_ch_switch_info_stru         *pst_ch_switch_info;
    mac_sta_csa_fsm_info_stru       *pst_sta_csa_fsm_info;
    oal_uint8                        uc_ap_channel;

    pst_mac_vap = (mac_vap_stru *)p_ctx;

    pst_ch_switch_info = &(pst_mac_vap->st_ch_switch_info);
    pst_sta_csa_fsm_info = &(pst_mac_vap->st_sta_csa_fsm_info);
    switch(event)
    {
        case WLAN_STA_CSA_EVENT_TBTT:
            pst_sta_csa_fsm_info->uc_csa_scan_after_tbtt++;
            if(pst_sta_csa_fsm_info->uc_csa_scan_after_tbtt>= 2)
            {/*csa 扫描*/
                if(OAL_FALSE == dmac_sta_csa_have_vap_is_in_assoc(pst_mac_vap))
                {
                    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{dmac_sta_csa_fsm_wait_event::dmac_trigger_csa_scan}");
                    pst_sta_csa_fsm_info->uc_associate_channel = 0;
                    dmac_trigger_csa_scan(pst_mac_vap);
                }
                else
                {
                    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CSA,"{dmac_sta_csa_fsm_wait_event::have vap in assoc, csa return INIT}");
                    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INIT);
                }
                pst_sta_csa_fsm_info->uc_csa_scan_after_tbtt = 0;
            }
            break;
        case WLAN_STA_CSA_EVENT_RCV_BEACON:
        case WLAN_STA_CSA_EVENT_RCV_PROBE_RSP:
            if(OAL_PTR_NULL != event_data)
            {
                uc_ap_channel = *(oal_uint8 *)event_data;
                pst_mac_vap->st_sta_csa_fsm_info.uc_associate_channel = uc_ap_channel;
                OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{dmac_sta_csa_fsm_wait_event::get associate_channel=[%d]}",
                    pst_mac_vap->st_sta_csa_fsm_info.uc_associate_channel);
            }
            break;
        case WLAN_STA_CSA_EVENT_SCAN_END:
            if(0 == pst_mac_vap->st_sta_csa_fsm_info.uc_associate_channel)
            {/*未收到csa扫描 response*/
                if(OAL_TRUE == pst_sta_csa_fsm_info->en_expect_cnt_status)
                {
                    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INIT);
                }
                else
                {
                    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INVALID);
                }
            }
            else
            {/*收到csa扫描response*/
                if(pst_mac_vap->st_channel.uc_chan_number == pst_mac_vap->st_sta_csa_fsm_info.uc_associate_channel)
                {/*信道相等*/
                    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INIT);
                }
                else
                {
                    pst_ch_switch_info->uc_new_channel = pst_mac_vap->st_sta_csa_fsm_info.uc_associate_channel;
                    if(pst_mac_vap->st_sta_csa_fsm_info.uc_associate_channel == pst_mac_vap->st_ch_switch_info.st_old_channel.uc_chan_number)
                    {
                        pst_ch_switch_info->en_new_bandwidth = pst_mac_vap->st_ch_switch_info.st_old_channel.en_bandwidth;
                    }
                    else
                    {
                        pst_ch_switch_info->en_new_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
                    }
                    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{dmac_sta_csa_fsm_wait_event:: csa check fail, channel from [%d] back to [%d].}",
                        pst_mac_vap->st_channel.uc_chan_number, pst_ch_switch_info->uc_new_channel);
                    dmac_chan_sta_switch_channel(pst_mac_vap);
                    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INVALID);
                }
                pst_mac_vap->st_sta_csa_fsm_info.uc_associate_channel = 0;
            }
            break;
        case WLAN_STA_CSA_EVENT_TO_INIT:
            dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INIT);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_sta_csa_fsm_invalid_entry(oal_void *p_ctx)
{
    mac_sta_csa_fsm_info_stru       *pst_csa_fsm_info = OAL_PTR_NULL;
    mac_ch_switch_info_stru         *pst_ch_switch_info = OAL_PTR_NULL;
    mac_vap_stru                    *pst_mac_vap = (mac_vap_stru *)p_ctx;

    if(WLAN_STA_CSA_FSM_INVALID == dmac_sta_csa_fsm_get_current_state(pst_mac_vap))
    {
        return;
    }

    pst_csa_fsm_info = &(pst_mac_vap->st_sta_csa_fsm_info);
    pst_ch_switch_info  = &(pst_mac_vap->st_ch_switch_info);

    if(OAL_TRUE == pst_csa_fsm_info->st_csa_handle_timer.en_is_registerd)
    {
        FRW_TIMER_DESTROY_TIMER(&(pst_csa_fsm_info->st_csa_handle_timer));
    }

    if(OAL_TRUE == dmac_sta_csa_is_in_waiting(pst_mac_vap))
    {
        dmac_chan_enable_machw_tx(pst_mac_vap); //恢复发送，防止问题ap发的beacon让停止发送
    }

    pst_ch_switch_info->en_new_bandwidth              = WLAN_BAND_WIDTH_BUTT;

    return;
}



OAL_STATIC oal_uint32 dmac_sta_csa_fsm_invalid_event(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data)
{
    mac_vap_stru                    *pst_mac_vap;
    oal_ieee80211_channel_sw_ie     *pst_csa_info;

    pst_mac_vap = (mac_vap_stru *)p_ctx;
    switch(event)
    {
        case WLAN_STA_CSA_EVENT_GET_IE:
            if((OAL_PTR_NULL != event_data) && (OAL_SIZEOF(oal_ieee80211_channel_sw_ie) == event_data_len))
            {
                pst_csa_info = (oal_ieee80211_channel_sw_ie*)event_data;
                if(pst_mac_vap->st_sta_csa_fsm_info.uc_sta_csa_last_cnt != pst_csa_info->count)
                {
                    dmac_sta_csa_start_timer(pst_mac_vap,pst_csa_info);
                }
            }
            break;
        case WLAN_STA_CSA_EVENT_TO_INIT:
            dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INIT);
            break;
        default:
            break;
    }

    return OAL_SUCC;
}



oal_void dmac_sta_csa_fsm_attach(dmac_vap_stru *pst_dmac_vap)
{
    mac_sta_csa_fsm_info_stru       *pst_handler  = OAL_PTR_NULL;
    oal_uint8                       auc_fsm_name[8] = {0};
    oal_uint32                      ul_ret;

    pst_handler = &(pst_dmac_vap->st_vap_base_info.st_sta_csa_fsm_info);

    if (OAL_TRUE == pst_handler->st_csa_handle_timer.en_is_registerd)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CSA, "dmac_sta_csa_fsm_attach::st_csa_handle_timer registerd.");
        FRW_TIMER_DESTROY_TIMER(&(pst_handler->st_csa_handle_timer));
    }

    OAL_MEMZERO(pst_handler,OAL_SIZEOF(mac_sta_csa_fsm_info_stru));
    /* 准备一个唯一的fsmname,只能到6,最后给'\0' */
    auc_fsm_name[0] = '0' + (oal_uint8)(pst_dmac_vap->st_vap_base_info.ul_core_id);
    auc_fsm_name[1] = '0' + pst_dmac_vap->st_vap_base_info.uc_chip_id;
    auc_fsm_name[2] = '0' + pst_dmac_vap->st_vap_base_info.uc_device_id;
    auc_fsm_name[3] = '0' + pst_dmac_vap->st_vap_base_info.uc_vap_id;
    auc_fsm_name[4] = 'C';
    auc_fsm_name[5] = 'S';
    auc_fsm_name[6] = 'A';

    ul_ret = oal_fsm_create(pst_dmac_vap,
                            auc_fsm_name,
                            &(pst_dmac_vap->st_vap_base_info),
                            &(pst_handler->st_oal_fsm),
                            WLAN_STA_CSA_FSM_INIT,
                            g_sta_csa_fsm_info,
                            OAL_SIZEOF(g_sta_csa_fsm_info)/OAL_SIZEOF(oal_fsm_state_info));

    if (OAL_SUCC != ul_ret)
    {
        pst_handler->en_is_fsm_attached = OAL_FALSE;
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CSA, "dmac_sta_csa_fsm_attach::oal_fsm_create fail.");
        return;
    }

    pst_handler->en_is_fsm_attached = OAL_TRUE;
    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CSA, "dmac_sta_csa_fsm_attach::vap_id = %d vap_mode=%d p2p_mode=%d attached succ.",
        pst_dmac_vap->st_vap_base_info.uc_vap_id,pst_dmac_vap->st_vap_base_info.en_vap_mode,pst_dmac_vap->st_vap_base_info.en_p2p_mode);

    return;
}


oal_void dmac_sta_csa_fsm_detach(dmac_vap_stru *pst_dmac_vap)
{
    mac_vap_stru                    *pst_mac_vap = OAL_PTR_NULL;

    pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);
    if (OAL_FALSE == pst_mac_vap->st_sta_csa_fsm_info.en_is_fsm_attached)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "dmac_sta_csa_fsm_detach::vap id[%d]pm fsm not attatched", pst_mac_vap->uc_vap_id);
        return;
    }

    dmac_sta_csa_fsm_trans_to_state(pst_mac_vap,WLAN_STA_CSA_FSM_INIT);
    pst_mac_vap->st_sta_csa_fsm_info.en_is_fsm_attached = OAL_FALSE;

    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CSA, "dmac_sta_csa_fsm_detach::vap_id = %d vap_mode=%d p2p_mode=%d detached succ.",
        pst_dmac_vap->st_vap_base_info.uc_vap_id,pst_dmac_vap->st_vap_base_info.en_vap_mode,pst_dmac_vap->st_vap_base_info.en_p2p_mode);
    return;
}




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

