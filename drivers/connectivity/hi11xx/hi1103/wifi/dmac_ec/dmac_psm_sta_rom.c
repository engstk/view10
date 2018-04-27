


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "hal_chip.h"
#include "hal_device.h"
#include "hal_device_fsm.h"
#include "frw_timer.h"
#include "mac_data.h"
#include "dmac_config.h"
#include "dmac_psm_sta.h"
#include "dmac_psm_ap.h"
#include "dmac_sta_pm.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "dmac_p2p.h"
#endif
#include "pm_extern.h"
#include "dmac_csa_sta.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_PSM_STA_ROM_C

oal_uint8  g_uc_max_powersave = OAL_FALSE;
oal_uint8  g_uc_max_powersave_limit = 100;      /*beacon*dtim<=100ms*/

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_psm_sta_rom_cb g_st_dmac_psm_sta_rom_cb = {dmac_psm_update_dtime_period,
                                                dmac_psm_update_keepalive};


//OAL_STATIC oal_uint8  g_uc_bmap[8] = {1, 2, 4, 8, 16, 32, 64, 128}; /* Bit map */
/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint32  dmac_psm_get_max_sleep_time(dmac_vap_stru  *pst_dmac_vap)
{
    oal_uint32    ul_dtim_period;
    oal_uint32    ul_bank3_bcn_period;
    oal_uint32    ul_max_allow_sleep_time;
    //oal_uint32    aul_addr[]              = {0x2010020c,0x20100234,0x2010025c};

    /* 关联上后才计算最大睡眠时间 */
    if ((MAC_VAP_STATE_UP == pst_dmac_vap->st_vap_base_info.en_vap_state) || (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state))
    {
        //ul_dtim_period = mac_mib_get_dot11dtimperiod(&(pst_dmac_vap->st_vap_base_info));
        ul_dtim_period = pst_dmac_vap->uc_psm_dtim_period;
        hal_vap_get_beacon_period(pst_dmac_vap->pst_hal_vap, &ul_bank3_bcn_period);

        if (0 == ul_bank3_bcn_period)
        {
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_PWR,"{dmac_psm_get_max_sleep_time::bank3 beacon period is 0.}");
#else
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_PWR,"{dmac_psm_get_max_sleep_time::bank3 beacon period is 0.}");
#endif
#ifdef _PRE_WLAN_MAC_BUGFIX_TSF_SYNC
            hal_sta_tsf_restore(pst_dmac_vap->pst_hal_vap);
#endif

        }
        /* 此维测用于判断是否睡的过多，由于动态dtim周期可能会从500ms调整到100ms，因此需要放宽 */
        ul_max_allow_sleep_time = (ul_bank3_bcn_period * ul_dtim_period + 500);
    }
    /* 否则不计算睡眠时间*/
    else
    {
        ul_max_allow_sleep_time = 0xFFFFFFFF;
    }

    return ul_max_allow_sleep_time;
}


oal_void dmac_psm_process_no_powersave(dmac_vap_stru *pst_dmac_vap)
{

    if (IS_P2P_DEV(&(pst_dmac_vap->st_vap_base_info)) || (WLAN_VAP_MODE_BSS_STA != pst_dmac_vap->st_vap_base_info.en_vap_mode))
    {
        return;
    }

    /* 异常此时应该处于active状态 */
    if (GET_PM_STATE(&pst_dmac_vap->st_sta_pm_handler) != STA_PWR_SAVE_STATE_ACTIVE)
    {
        dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_NO_POWERSAVE, 0, OAL_PTR_NULL);
    }

}

oal_void dmac_psm_sync_dtim_count(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_dtim_count)
{
    oal_uint16               us_listen_interval_count;

    /* 写STA dtim_count寄存器, dtim update 寄存器不允许写0 */
    if (0 == uc_dtim_count)
    {
        hal_set_psm_dtim_count(pst_dmac_vap->pst_hal_vap, pst_dmac_vap->uc_psm_dtim_period);
        hal_set_psm_listen_interval_count(pst_dmac_vap->pst_hal_vap, pst_dmac_vap->us_psm_listen_interval);
    }
    else
    {
        hal_set_psm_dtim_count(pst_dmac_vap->pst_hal_vap, uc_dtim_count);

        if(uc_dtim_count <= pst_dmac_vap->us_psm_listen_interval)
        {
            us_listen_interval_count = uc_dtim_count;
        }
        else
        {
            us_listen_interval_count = pst_dmac_vap->us_psm_listen_interval;
        }

        hal_set_psm_listen_interval_count(pst_dmac_vap->pst_hal_vap, us_listen_interval_count);
    }

}


oal_void dmac_psm_max_powersave_enable(mac_device_stru *pst_mac_device)
{
    oal_uint8       uc_vap_idx;
    dmac_vap_stru   *pst_dmac_vap;

    if(OAL_PTR_NULL == pst_mac_device)
    {
        return ;
    }

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "{dmac_psm_max_powersave_enable:enable[%d]}",pst_mac_device->uc_in_suspend);

    g_uc_max_powersave = (0== pst_mac_device->uc_in_suspend)?0:1;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_dmac_vap)
        {
            OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_max_powersave_enable::pst_dmac_vap null.}");
            return;
        }

        /* 关联的STAUT才需要更新 */
        if ((WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
            &&(0 != mac_mib_get_dot11dtimperiod(&pst_dmac_vap->st_vap_base_info))
            &&((MAC_VAP_STATE_UP == pst_dmac_vap->st_vap_base_info.en_vap_state) || (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state)))
        {
            g_st_dmac_psm_sta_rom_cb.p_dmac_psm_update_dtime_period(&(pst_dmac_vap->st_vap_base_info),
                                        (oal_uint8)mac_mib_get_dot11dtimperiod(&pst_dmac_vap->st_vap_base_info),
                                        mac_mib_get_BeaconPeriod(&pst_dmac_vap->st_vap_base_info));

            g_st_dmac_psm_sta_rom_cb.p_dmac_psm_update_keepalive(pst_dmac_vap);
        }
    }

}

oal_void  dmac_psm_update_bcn_tout_max_cnt(dmac_vap_stru  *pst_dmac_vap)
{
    oal_uint32  ul_div_value;
    oal_uint32  ul_bcn_tout_max_time = DMAC_BEACON_TIMEOUT_MAX_TIME;
    oal_uint32  ul_mib_bcn_period;

    ul_mib_bcn_period = mac_mib_get_BeaconPeriod(&pst_dmac_vap->st_vap_base_info);
    ul_div_value = ul_mib_bcn_period * (pst_dmac_vap->uc_psm_dtim_period);
    if (ul_div_value != 0)
    {
        /* 不能整除+1 1000 / (100 * 3) 取4 */
        if (ul_bcn_tout_max_time % ul_div_value)
        {
            pst_dmac_vap->uc_bcn_tout_max_cnt = (oal_uint8)((ul_bcn_tout_max_time / ul_div_value) + 1);
        }
        else
        {
            pst_dmac_vap->uc_bcn_tout_max_cnt = (oal_uint8)(ul_bcn_tout_max_time / ul_div_value);
        }
    }
    else
    {
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_psm_update_bcn_tout_max_cnt::bcn period[%d],dtim period[%d]",ul_mib_bcn_period,
                        pst_dmac_vap->uc_psm_dtim_period);
    }
}
#if 0

oal_uint8 dmac_psm_is_tim_dtim_set(dmac_vap_stru *pst_dmac_vap, oal_uint8* puc_tim_elm)
{
    oal_uint8                   uc_len  = 0;
    oal_uint8                   uc_bmap_ctrl = 0;
    oal_uint8                   uc_bmap_offset  = 0;
    oal_uint8                   uc_byte_offset = 0;
    oal_uint8                   uc_bit_offset  = 0;
    oal_uint8                   uc_status      = 0 ;
    oal_uint32                  ul_aid;
    mac_sta_pm_handler_stru     *pst_mac_sta_pm_handle;

    ul_aid = pst_dmac_vap->st_vap_base_info.us_sta_aid;
    pst_mac_sta_pm_handle = (mac_sta_pm_handler_stru *)(pst_dmac_vap->pst_pm_handler);

    /*检查是否是TIM*/
    if(puc_tim_elm[0]!= MAC_EID_TIM)
    {
      return uc_status;
    }

    uc_len = puc_tim_elm[1];
    uc_bmap_ctrl = puc_tim_elm[4];

    /* Check if AP's DTIM period has changed */
    if(puc_tim_elm[3] != mac_mib_get_dot11dtimperiod(&(pst_dmac_vap->st_vap_base_info)))
    {
      /* 重新设置DTIM寄存器的值 */
      hal_set_psm_dtim_period(pst_dmac_vap->pst_hal_vap, puc_tim_elm[3], pst_mac_sta_pm_handle->uc_listen_intervl_to_dtim_times,
                                  pst_mac_sta_pm_handle->en_receive_dtim);
    }

    /* Compute the bit map offset, which is given by the MSB 7 bits in the   */
    /* bit map control field of the TIM element.  */
    uc_bmap_offset = (uc_bmap_ctrl & 0xFE);

    /* A DTIM count of zero indicates that the current TIM is a DTIM. The    */
    /* BIT0 of the bit map control field is set (for TIMs with a value of 0  */
    /* in the DTIM count field) when one or more broadcast/multicast frames  */
    /* are buffered at the AP.                                               */
    if(OAL_TRUE == (uc_bmap_ctrl & BIT0))
    {
        uc_status |= DMAC_DTIM_IS_SET;
    }

    /* Traffic Indication Virtual Bit Map within the AP, generates the TIM   */
    /* such that if a station has buffered packets, then the corresponding   */
    /* bit (which can be found from the association ID) is set. The byte     */
    /* offset is obtained by dividing the association ID by '8' and the bit  */
    /* offset is the remainder of the association ID when divided by '8'.    */
    uc_byte_offset = ul_aid >> 3;
    uc_bit_offset  = ul_aid & 0x07;

    /* Bit map offset should always be greater than the computed byte offset */
    /* and the byte offset should always be lesser than the 'maximum' number */
    /* of bytes in the Virtual Bitmap. If either of the above two conditions */
    /* are not satisfied, then the 'status' is returned as is.               */
    if(uc_byte_offset < uc_bmap_offset || uc_byte_offset > uc_bmap_offset + uc_len - 4)
    {
        return uc_status;
    }

    /* The station has buffered packets only if the corresponding bit is set */
    /* in the Virtual Bit Map field. Note: Virtual Bit Map field starts      */
    /* 5 bytes from the start of the TIM element.                            */
    if((puc_tim_elm[5 + uc_byte_offset - uc_bmap_offset] & g_uc_bmap[uc_bit_offset]) != 0)
    {
        uc_status |= DMAC_TIM_IS_SET;
    }
    return uc_status;
}
#endif


oal_void dmac_psm_init_null_frm_cnt(dmac_vap_stru *pst_dmac_vap)
{
    pst_dmac_vap->uc_null_frm_ofdm_succ_cnt = DMAC_ALG_NULL_FRM_INIT_CNT;
    pst_dmac_vap->uc_null_frm_cnt = DMAC_ALG_NULL_FRM_INIT_CNT;
}



oal_void dmac_psm_inc_null_frm_ofdm_succ(dmac_vap_stru *pst_dmac_vap)
{
    if (pst_dmac_vap->uc_null_frm_ofdm_succ_cnt < DMAC_ALG_NULL_FRM_INIT_CNT)
    {
        pst_dmac_vap->uc_null_frm_ofdm_succ_cnt++;
    }

    dmac_psm_inc_null_frm(pst_dmac_vap);
}


oal_void dmac_psm_dec_null_frm_ofdm_succ(dmac_vap_stru *pst_dmac_vap)
{
    if (pst_dmac_vap->uc_null_frm_ofdm_succ_cnt > 0)
    {
         pst_dmac_vap->uc_null_frm_ofdm_succ_cnt--;
    }

    dmac_psm_inc_null_frm(pst_dmac_vap);
}


oal_void dmac_psm_inc_null_frm(dmac_vap_stru *pst_dmac_vap)
{
    if (pst_dmac_vap->uc_null_frm_cnt < DMAC_ALG_NULL_FRM_INIT_CNT)
    {
        pst_dmac_vap->uc_null_frm_cnt += DMAC_ALG_NULL_FRM_INC_CNT;
    }
}


oal_void dmac_psm_dec_null_frm(dmac_vap_stru *pst_dmac_vap)
{
    if (pst_dmac_vap->uc_null_frm_cnt > 0)
    {
        pst_dmac_vap->uc_null_frm_cnt--;
    }
}




oal_uint8  dmac_psm_is_tid_queues_empty(dmac_vap_stru  *pst_dmac_vap)
{
    dmac_user_stru                   *pst_user;
    oal_uint8                        uc_tid_idx;

    /* TID队列 */
    pst_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_dmac_vap->st_vap_base_info.us_assoc_vap_id);

    if (OAL_PTR_NULL != pst_user)
    {
        if (OAL_FALSE == dmac_psm_is_tid_empty(pst_user))
        {
            //PM_WLAN_PRINT("{tid queue not empty,psm state[%d]}\r\n",dmac_psm_get_state(pst_dmac_vap));

            for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx ++)
            {
                if(pst_user->ast_tx_tid_queue[uc_tid_idx].us_mpdu_num)
                {
                    PM_WLAN_PRINT("TID[%d],mpdu_num[%d],retry[%d],paused[%d]\r\n",uc_tid_idx,
                          pst_user->ast_tx_tid_queue[uc_tid_idx].us_mpdu_num,
                          pst_user->ast_tx_tid_queue[uc_tid_idx].uc_retry_num,
                          pst_user->ast_tx_tid_queue[uc_tid_idx].uc_is_paused);
                }
            }
            //OAM_INFO_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_KEEPALIVE, "psm call back::tid queue not empty");
            return OAL_FALSE;
        }
        return OAL_TRUE;
    }
    else
    {
        return OAL_TRUE;
    }

}


oal_void dmac_send_null_frame_to_ap_opt(dmac_vap_stru *pst_dmac_vap, oal_uint8  uc_psm, oal_bool_enum_uint8 en_qos)
{
    mac_sta_pm_handler_stru  *pst_sta_pm_handle;

    pst_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;
    if (OAL_FALSE == pst_sta_pm_handle->en_is_fsm_attached)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                            "{dmac_send_null_frame_to_ap_opt::pm fsm not attached.");
        return;
    }

    if(OAL_FALSE == en_qos)
    {
        if(STA_PWR_SAVE_STATE_ACTIVE == uc_psm)
        {
            pst_sta_pm_handle->st_null_wait.en_active_null_wait = OAL_TRUE;
            pst_sta_pm_handle->st_null_wait.en_doze_null_wait   = OAL_FALSE;
        }
        else
        {
            pst_sta_pm_handle->st_null_wait.en_active_null_wait = OAL_FALSE;
            pst_sta_pm_handle->st_null_wait.en_doze_null_wait   = OAL_TRUE;
        }
    }
}

oal_uint8  dmac_psm_get_state(dmac_vap_stru* pst_dmac_vap)
{
    mac_sta_pm_handler_stru     *pst_mac_sta_pm_handle;

    pst_mac_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;
    if (OAL_FALSE == pst_mac_sta_pm_handle->en_is_fsm_attached)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_psm_get_state::pm fsm not attached.}");
        return STA_PWR_SAVE_STATE_ACTIVE;
    }
    return GET_PM_STATE(pst_mac_sta_pm_handle);
}


oal_uint8 dmac_can_sta_doze_prot(dmac_vap_stru *pst_dmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    mac_sta_pm_handler_stru  *pst_mac_sta_pm_handle;

    pst_mac_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;

    /* If the AP is UAPSD capable and UAPSD service period is in progress    */
    /* or STA is waiting for UAPSD service period to start the STA cannot go */
    /* to doze mode.                                                         */
    if(OAL_TRUE == dmac_is_uapsd_capable(pst_dmac_vap))
    {
        if (OAL_FALSE == dmac_is_uapsd_sp_not_in_progress(pst_mac_sta_pm_handle))
        {
            return OAL_FALSE;
        }
    }
#endif /* _PRE_WLAN_FEATURE_STA_UAPSD */
    return OAL_TRUE;
}

#endif /*_PRE_WLAN_FEATURE_STA_PM*/
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


