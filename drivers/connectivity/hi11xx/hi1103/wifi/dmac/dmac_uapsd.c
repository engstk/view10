


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_UAPSD

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_net.h"
#include "wlan_spec.h"
#include "hal_ext_if.h"
#include "mac_frame.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "dmac_vap.h"
#include "dmac_main.h"
#include "dmac_user.h"
#include "dmac_psm_ap.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_rx_data.h"
#include "dmac_uapsd.h"
#include "dmac_config.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_UAPSD_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC oal_uint32 dmac_uapsd_state_trans(dmac_vap_stru *pst_dmac_vap,dmac_user_stru *pst_dmac_user,mac_ieee80211_qos_frame_stru *pst_mac_header)
{
    dmac_user_uapsd_stru            *pst_uapsd_stru;
    oal_uint8                       uc_uapsd_flag;
    oal_int32                       ul_uapsd_qdepth;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_usr_uapsd_statis_stru      *pst_uapsd_statis;

    pst_uapsd_statis = pst_dmac_user->st_uapsd_stru.pst_uapsd_statis;
#endif


    pst_uapsd_stru = &pst_dmac_user ->st_uapsd_stru;

    uc_uapsd_flag = pst_dmac_user->uc_uapsd_flag;
    if(((1==(pst_mac_header->st_frame_control.bit_power_mgmt))&&(0==(uc_uapsd_flag & MAC_USR_UAPSD_TRIG)))
        ||((0==(pst_mac_header->st_frame_control.bit_power_mgmt))&&(0!=(uc_uapsd_flag & MAC_USR_UAPSD_TRIG))))
    {
       //OAM_INFO_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
       //               "dmac_uapsd_rx_trigger_check:state transfer,(uc_uapsd_flag & MAC_USR_UAPSD_TRIG)=%d",uc_uapsd_flag & MAC_USR_UAPSD_TRIG);

       DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_rx_trigger_state_trans);

       pst_dmac_user->uc_uapsd_flag &= ~MAC_USR_UAPSD_SP;
       if(pst_mac_header->st_frame_control.bit_power_mgmt)
       {
          /*从非节能态迁移到节能态*/
          oal_memset(&pst_uapsd_stru->us_uapsd_trigseq[0], 0xff, OAL_SIZEOF(pst_uapsd_stru->us_uapsd_trigseq));
          pst_dmac_user->uc_uapsd_flag |= MAC_USR_UAPSD_TRIG;
       }
       else
       {
         /*flush 节能队列*/
         ul_uapsd_qdepth = dmac_uapsd_flush_queue(pst_dmac_vap,pst_dmac_user);
         if((0 == ul_uapsd_qdepth)&& MAC_USR_UAPSD_USE_TIM(pst_dmac_user))
         {
            /* 调用PSM的TIM设置接口 */
            dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);
         }
        /*从节能态迁移到非节能态*/
         pst_dmac_user->uc_uapsd_flag &= ~MAC_USR_UAPSD_TRIG;
       }
       return OAL_TRUE;
    }

    return OAL_FALSE;
}


oal_void dmac_uapsd_rx_trigger_check (dmac_vap_stru *pst_dmac_vap,dmac_user_stru *pst_dmac_user,oal_netbuf_stru *pst_net_buf)
{
    mac_ieee80211_qos_frame_stru    *pst_mac_header;
    mac_rx_ctl_stru                 *pst_rx_ctrl;
    oal_uint8                       uc_uapsd_flag;
    oal_uint32                      ul_istrigger = OAL_FALSE;
    oal_uint8                       uc_tid;
    oal_uint8                       uc_ac = WLAN_WME_AC_BE;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_usr_uapsd_statis_stru      *pst_uapsd_statis;

    pst_uapsd_statis = pst_dmac_user->st_uapsd_stru.pst_uapsd_statis;
    if(OAL_PTR_NULL == pst_uapsd_statis)
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_uapsd_rx_trigger_check::pst_uapsd_statis null.}");
        return ;
    }
#endif

    uc_uapsd_flag = pst_dmac_user->uc_uapsd_flag;

    /*如果当前Usr不是uapsd使能的或者是处于一个Service Period中，返回*/
    if(!(uc_uapsd_flag & MAC_USR_UAPSD_EN)||
        (uc_uapsd_flag & MAC_USR_UAPSD_SP))
    {
        DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_rx_trigger_in_sp);
        return ;
    }

    pst_rx_ctrl = (mac_rx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
    pst_mac_header = (mac_ieee80211_qos_frame_stru*)(mac_get_rx_cb_mac_hdr(pst_rx_ctrl));
    if(OAL_TRUE == dmac_uapsd_state_trans(pst_dmac_vap,pst_dmac_user,pst_mac_header))
    {
        return;
    }

    /* 检查是否是一个trigger帧:QOS DATA or Qos NULL DATA，非trigger帧也要继续处理是否需要状态切换 */
    if((WLAN_DATA_BASICTYPE == pst_mac_header->st_frame_control.bit_type)&&
        ((WLAN_QOS_DATA == pst_mac_header->st_frame_control.bit_sub_type)||
        (WLAN_QOS_NULL_FRAME == pst_mac_header->st_frame_control.bit_sub_type)))
    {
        ul_istrigger = OAL_TRUE;
        uc_tid = pst_mac_header->bit_qc_tid;
        uc_ac = WLAN_WME_TID_TO_AC(uc_tid);
    }

    if((OAL_TRUE == ul_istrigger) && (MAC_USR_UAPSD_AC_CAN_TIGGER(uc_ac,pst_dmac_user)))
    {
        dmac_uapsd_process_trigger(pst_dmac_vap,pst_dmac_user,uc_ac,pst_net_buf);
    }

    return ;
}


oal_void dmac_uapsd_process_trigger (dmac_vap_stru *pst_dmac_vap,dmac_user_stru *pst_dmac_user,oal_uint8 uc_ac,oal_netbuf_stru *pst_net_buf)
{

    mac_ieee80211_qos_frame_stru    *pst_mac_header;
    mac_rx_ctl_stru                 *pst_rx_ctrl;
    oal_int32                       al_uapsd_qdepth;

#ifdef _PRE_WLAN_DFT_STAT
    dmac_usr_uapsd_statis_stru      *pst_uapsd_statis;

    pst_uapsd_statis = pst_dmac_user->st_uapsd_stru.pst_uapsd_statis;
#endif

    pst_rx_ctrl = (mac_rx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
    pst_mac_header = (mac_ieee80211_qos_frame_stru*)(mac_get_rx_cb_mac_hdr(pst_rx_ctrl));

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (OAL_FALSE == pst_dmac_user->st_user_base_info.en_is_multi_user)
    {
        dmac_tid_resume(pst_dmac_vap->pst_hal_device, &pst_dmac_user->ast_tx_tid_queue[uc_ac], DMAC_TID_PAUSE_RESUME_TYPE_PS);
    }
#ifndef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
    hal_tx_disable_peer_sta_ps_ctrl(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif
#endif

    if((OAL_TRUE == pst_mac_header->st_frame_control.bit_retry)&&
        (pst_mac_header->bit_seq_num == pst_dmac_user->st_uapsd_stru.us_uapsd_trigseq[uc_ac]))
    {
        //OAM_INFO_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR, "dmac_uapsd_process_trigger:sequnce duplicated");

        DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_rx_trigger_dup_seq);
        return ;
    }
    pst_dmac_user->st_uapsd_stru.us_uapsd_trigseq[uc_ac] = pst_mac_header->bit_seq_num;

    al_uapsd_qdepth = dmac_uapsd_process_queue(pst_dmac_vap,pst_dmac_user,uc_ac);
    /*在设置DMAC_USR_UAPSD_SP前，需判断是否有发送报文，-1表示没有报文发送，不应该置上DMAC_USR_UAPSD_SP*/
    if( DMAC_UAPSD_NOT_SEND_FRAME == al_uapsd_qdepth)
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_uapsd_process_trigger::DMAC_UAPSD_NOT_SEND_FRAME == al_uapsd_qdepth.}");
        return ;
    }

    pst_dmac_user->uc_uapsd_flag |= MAC_USR_UAPSD_SP;

    if((0 == al_uapsd_qdepth)&& MAC_USR_UAPSD_USE_TIM(pst_dmac_user))
     {
        /* 调用PSM的TIM设置接口 */
        OAM_INFO_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
              "dmac_uapsd_process_trigger:set PVB to 0,usr id = %d",pst_dmac_user->st_user_base_info.us_assoc_id);

        dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);
     }
}



OAL_STATIC oal_int32 dmac_uapsd_tx_pkt (oal_netbuf_stru *pst_net_buf, dmac_vap_stru* pst_dmac_vap, dmac_user_stru *pst_dmac_user,
                                                oal_uint8 uc_ac,oal_uint8 uc_sp_last,oal_uint8* pst_extra_qosnull)
{
    dmac_user_uapsd_stru *pst_dmac_uapsd;
    mac_tx_ctl_stru             *pst_tx_ctrl;
    mac_ieee80211_qos_frame_stru    *pst_mac_header;
    mac_ieee80211_frame_stru        *pst_frame_hdr;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_usr_uapsd_statis_stru  *pst_uapsd_statis;

    pst_uapsd_statis = pst_dmac_user->st_uapsd_stru.pst_uapsd_statis;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_uapsd_statis))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_uapsd_tx_pkt::uapsd_statis is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    pst_dmac_uapsd = &pst_dmac_user->st_uapsd_stru;

    pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
    MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_ctrl) = OAL_TRUE;

    /*非flush操作，tid设置为uapsd专用tid，ac设置为trigger的AC*/
    MAC_GET_CB_WME_TID_TYPE(pst_tx_ctrl)= WLAN_TIDNO_UAPSD;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctrl) = uc_ac;

    pst_mac_header = (mac_ieee80211_qos_frame_stru*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl);

    /*管理帧没有EOSP位，因此额外发送一个qos null结束USP*/
    if (WLAN_MANAGEMENT == pst_mac_header->st_frame_control.bit_type) {
        pst_frame_hdr = MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl);
        pst_frame_hdr->st_frame_control.bit_more_data = 0;
        *pst_extra_qosnull = OAL_TRUE;
    } else {
    /*
            每次发送时如果后面还有帧，则将EOSP置0，More Data置1。
            如果到max sp length，后面还有缓存帧，则将EOSP置1，More Data置1。
            如果后面没有帧发送，则将EOSP置1，More Data置0。
            当delivery-enable的AC队列中没有缓存帧，发送一个QoSNull帧，More Data置0。
    */
        if (OAL_FALSE == uc_sp_last) {
            pst_mac_header->st_frame_control.bit_more_data = 1;
            pst_mac_header->bit_qc_eosp = 0;
        } else if (1 == oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num)) {
            pst_mac_header->st_frame_control.bit_more_data = 0;
            pst_mac_header->bit_qc_eosp = 1;
        } else {
            pst_mac_header->st_frame_control.bit_more_data = 1;
            pst_mac_header->bit_qc_eosp = 1;
        }

    }

    oal_atomic_dec(&pst_dmac_uapsd->uc_mpdu_num);

    if(WLAN_DATA_BASICTYPE == pst_mac_header->st_frame_control.bit_type)
    {
        /*发送不成功，则终止本次USP*/
        if(OAL_SUCC != dmac_tx_process_data(pst_dmac_vap->pst_hal_device, pst_dmac_vap, pst_net_buf))
        {
            OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_uapsd_tx_pkt::dmac_tx_process_data failed.}");

            DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_process_queue_error);
            *pst_extra_qosnull = OAL_TRUE;

            dmac_tx_excp_free_netbuf(pst_net_buf);
            return OAL_FAIL;
        }
    }
    else
    {
        /*发送不成功，则终止本次USP*/
        if(OAL_SUCC != dmac_tx_mgmt(pst_dmac_vap, pst_net_buf, MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl)))
        {
            oal_netbuf_free(pst_net_buf);
            OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_uapsd_tx_pkt::dmac_tx_mgmt failed.}");

            DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_process_queue_error);
            *pst_extra_qosnull = OAL_TRUE;
            return OAL_FAIL;
        }
    }

    return  OAL_SUCC;

}



oal_int32 dmac_uapsd_process_queue (dmac_vap_stru* pst_dmac_vap, dmac_user_stru *pst_dmac_user,oal_uint8 uc_ac)
{
    dmac_user_uapsd_stru        *pst_dmac_uapsd;
    mac_user_uapsd_status_stru  *pst_mac_uapsd_status;
    oal_uint8                   uc_send_num;
    oal_uint8                   uc_loop;
    oal_netbuf_stru             *pst_net_buf;
    oal_uint8                   uc_extra_qosnull = OAL_FALSE;
    oal_uint8                   uc_sp_last = OAL_FALSE;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_usr_uapsd_statis_stru  *pst_uapsd_statis;
    pst_uapsd_statis = pst_dmac_user->st_uapsd_stru.pst_uapsd_statis;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_uapsd_statis))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_uapsd_process_queue::uapsd_statis is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    pst_dmac_uapsd = &pst_dmac_user->st_uapsd_stru;

    if(0 == oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num))
    {
         DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_send_qosnull);
         //OAM_INFO_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
         //         "dmac_uapsd_process_queue:queue is empty，send a qosnull");

         if(OAL_SUCC != dmac_uapsd_send_qosnull(pst_dmac_vap,pst_dmac_user,uc_ac))
         {
            return DMAC_UAPSD_NOT_SEND_FRAME;
         }
         else
         {
            return 0;
         }

    }

    pst_mac_uapsd_status = &pst_dmac_user->st_uapsd_status;

    uc_send_num = (pst_mac_uapsd_status->uc_max_sp_len < (oal_uint8)oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num))?
                   pst_mac_uapsd_status->uc_max_sp_len : (oal_uint8)oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num);

    for(uc_loop = 0;uc_loop < uc_send_num;uc_loop++)
    {
        oal_spin_lock(&pst_dmac_uapsd->st_lock_uapsd);
        pst_net_buf = dmac_tx_dequeue_first_mpdu(&(pst_dmac_uapsd->st_uapsd_queue_head));
        if(OAL_PTR_NULL == pst_net_buf)
        {
           oal_spin_unlock(&pst_dmac_uapsd->st_lock_uapsd);

           DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_process_queue_error);

           OAM_ERROR_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
                          "{dmac_uapsd_process_queue::pst_net_buf null, uc_mpdu_num=%d.}",
                          oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num));
           return oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num);
        }

        DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_tx_dequeue_count);
        oal_spin_unlock(&pst_dmac_uapsd->st_lock_uapsd);

        if(uc_loop == uc_send_num-1)
        {
            uc_sp_last = OAL_TRUE;
        }

        if(OAL_SUCC!= dmac_uapsd_tx_pkt(pst_net_buf,pst_dmac_vap,pst_dmac_user,uc_ac,uc_sp_last,&uc_extra_qosnull))
        {
            OAM_ERROR_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
                          "{dmac_uapsd_process_queue::dmac_uapsd_tx_pkt failed:us_assoc_id = %d.}",pst_dmac_user->st_user_base_info.us_assoc_id);
            break;
        }
    }

    if(OAL_TRUE == uc_extra_qosnull)
    {
        DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_send_extra_qosnull);
        //OAM_INFO_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
        //              "dmac_uapsd_process_queue:send a extra qosnull");
        dmac_uapsd_send_qosnull(pst_dmac_vap,pst_dmac_user,uc_ac);
    }

    return oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num);

}



oal_int32 dmac_uapsd_flush_queue (dmac_vap_stru* pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{
    dmac_user_uapsd_stru        *pst_dmac_uapsd;
    oal_netbuf_stru             *pst_net_buf;
    mac_tx_ctl_stru             *pst_tx_ctrl;
    mac_ieee80211_frame_stru    *pst_frame_hdr;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_usr_uapsd_statis_stru  *pst_uapsd_statis;

    pst_uapsd_statis = pst_dmac_user->st_uapsd_stru.pst_uapsd_statis;
    if(OAL_PTR_NULL == pst_uapsd_statis )
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
                       "{dmac_uapsd_flush_queue::pst_uapsd_statis null.}");
        return DMAC_UAPSD_NOT_SEND_FRAME;
    }
#endif

    pst_dmac_uapsd = &pst_dmac_user->st_uapsd_stru;

    while(0 != oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num))
    {
        oal_spin_lock(&pst_dmac_uapsd->st_lock_uapsd);
        pst_net_buf = dmac_tx_dequeue_first_mpdu(&(pst_dmac_uapsd->st_uapsd_queue_head));
        if(OAL_PTR_NULL == pst_net_buf)
        {
           oal_spin_unlock(&pst_dmac_uapsd->st_lock_uapsd);

           DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_flush_queue_error);

           OAM_ERROR_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
                          "{dmac_uapsd_flush_queue::pst_net_buf null, uc_mpdu_num=%d.}",
                          oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num));
           return oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num);
        }

        DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_tx_dequeue_count);
        oal_spin_unlock(&pst_dmac_uapsd->st_lock_uapsd);

        pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
        MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_ctrl) = OAL_TRUE;

        /*flush流程，视为正常流程的继续，清掉more data bit*/
        if (1 == oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num))
        {
            pst_frame_hdr = MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl);
            pst_frame_hdr->st_frame_control.bit_more_data = 0;
        }

        oal_atomic_dec(&pst_dmac_uapsd->uc_mpdu_num);

        pst_frame_hdr = MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl);

        if(WLAN_DATA_BASICTYPE == pst_frame_hdr->st_frame_control.bit_type)
        {
            /*发送不成功，则终止本次USP*/
            if(OAL_SUCC != dmac_tx_process_data(pst_dmac_vap->pst_hal_device, pst_dmac_vap, pst_net_buf))
            {
                OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
                               "{dmac_uapsd_flush_queue::dmac_tx_process_data failed.}");

                DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_flush_queue_error);

                dmac_tx_excp_free_netbuf(pst_net_buf);
                continue;
            }
        }
        else
        {
            /*发送不成功，则终止本次USP*/
            if(OAL_SUCC != dmac_tx_mgmt(pst_dmac_vap, pst_net_buf, MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl)))
            {
                OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
                               "{dmac_uapsd_flush_queue::dmac_tx_mgmt failed.}");

                DMAC_UAPSD_STATS_INCR(pst_uapsd_statis->ul_uapsd_flush_queue_error);

                dmac_tx_excp_free_netbuf(pst_net_buf);
                continue;
            }
        }

    }

    return oal_atomic_read(&pst_dmac_uapsd->uc_mpdu_num);

}



oal_uint32 dmac_uapsd_send_qosnull(dmac_vap_stru *pst_dmac_vap,dmac_user_stru *pst_dmac_user,oal_uint8 uc_ac)
{
    oal_netbuf_stru                 *pst_net_buf;
    mac_tx_ctl_stru                 *pst_tx_ctrl;
    oal_uint32                       ul_ret;
    mac_ieee80211_qos_frame_stru    *pst_mac_header;

    /* 申请net_buff */
    pst_net_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_net_buf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_uapsd_send_qosnull::pst_net_buf failed.}");

        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    OAL_MEM_NETBUF_TRACE(pst_net_buf, OAL_TRUE);

    oal_set_netbuf_prev(pst_net_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_net_buf, OAL_PTR_NULL);

    /* 填写帧头,其中from ds为1，to ds为0，因此frame control的第二个字节为02 */
    OAL_MEMZERO(oal_netbuf_header(pst_net_buf), OAL_SIZEOF(mac_ieee80211_qos_frame_stru));
    mac_null_data_encap(oal_netbuf_header(pst_net_buf),
                        (oal_uint16)(WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS_NULL) | 0x0200,
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                        pst_dmac_vap->st_vap_base_info.auc_bssid);

    pst_mac_header = (mac_ieee80211_qos_frame_stru*)oal_netbuf_header(pst_net_buf);
    pst_mac_header->bit_qc_tid = WLAN_WME_AC_TO_TID(uc_ac);
    pst_mac_header->bit_qc_eosp = 1;
    /*协议规定单播的QOS NULL DATA只允许normal ack*/
    pst_mac_header->bit_qc_ack_polocy = WLAN_TX_NORMAL_ACK;

    /* 填写cb字段 */
    pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
    OAL_MEMZERO(pst_tx_ctrl, OAL_SIZEOF(mac_tx_ctl_stru));

    /* 填写tx部分 */
    MAC_GET_CB_ACK_POLACY(pst_tx_ctrl)     = WLAN_TX_NORMAL_ACK;
    MAC_GET_CB_EVENT_TYPE(pst_tx_ctrl)       = FRW_EVENT_TYPE_WLAN_DTX;
    //MAC_GET_CB_EN_IS_BAR(pst_tx_ctrl)           = OAL_FALSE;
    //MAC_GET_CB_AC(pst_tx_ctrl)                  = uc_ac;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctrl) = uc_ac;
    MAC_GET_CB_RETRIED_NUM(pst_tx_ctrl)         = 0;
    //MAC_GET_CB_TID(pst_tx_ctrl)                 = WLAN_TIDNO_UAPSD;
    MAC_GET_CB_WME_TID_TYPE(pst_tx_ctrl)= WLAN_TIDNO_UAPSD;
    MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctrl)        = pst_dmac_vap->st_vap_base_info.uc_vap_id;
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctrl)         = pst_dmac_user->st_user_base_info.us_assoc_id;
    //MAC_GET_CB_EN_IS_QOSDATA(pst_tx_ctrl)       = OAL_TRUE;
    MAC_SET_CB_IS_QOS_DATA(pst_tx_ctrl, OAL_TRUE);
    /* 填写tx rx公共部分 */
    MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_ctrl)   = OAL_TRUE;
    //MAC_GET_CB_FRAME_HEADER(pst_tx_ctrl)           = (mac_ieee80211_frame_stru *)oal_netbuf_data(pst_net_buf);
    MAC_SET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl, (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_net_buf));
    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl)    = OAL_SIZEOF(mac_ieee80211_qos_frame_stru);
    MAC_GET_CB_MPDU_NUM(pst_tx_ctrl)               = 1;
    MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl)             = 1;
    MAC_GET_CB_MPDU_LEN(pst_tx_ctrl)               = 0;
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctrl)            = pst_dmac_user->st_user_base_info.us_assoc_id;

    ul_ret = dmac_tx_process_data(pst_dmac_vap->pst_hal_device, pst_dmac_vap, pst_net_buf);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                       "{dmac_uapsd_send_qosnull::dmac_tx_process_data failed[%d].}", ul_ret);

        dmac_tx_excp_free_netbuf(pst_net_buf);

        if (OAL_LIKELY(OAL_PTR_NULL != pst_dmac_user->st_uapsd_stru.pst_uapsd_statis))
        {
            DMAC_UAPSD_STATS_INCR(pst_dmac_user->st_uapsd_stru.pst_uapsd_statis->ul_uapsd_send_qosnull_fail);
        }

        return ul_ret;
    }

    return ul_ret;

}


oal_uint32  dmac_config_set_uapsden(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8               uc_value;
    dmac_vap_stru           *pst_dmac_vap;
    dmac_user_stru          *pst_dmac_user;
    oal_dlist_head_stru     *pst_entry;
    mac_user_stru           *pst_user_tmp;
    oal_int32               al_uapsd_qdepth;

    uc_value = *puc_param;

    /* 设置mib值 */
    mac_vap_set_uapsd_en(pst_mac_vap, uc_value);
    g_uc_uapsd_cap = *puc_param;

    /*如果是非使能，清除每个User*/
    if(0 == uc_value)
    {
        if(WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
        {
            pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

            OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
            {
                pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
                /*lint -save -e774 */
                if (OAL_UNLIKELY(OAL_PTR_NULL == pst_user_tmp))
                {
                    OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_uapsden::null pointer.}");
                    continue;
                }
                /*lint -restore */

                pst_dmac_user = MAC_GET_DMAC_USER(pst_user_tmp);

                al_uapsd_qdepth = dmac_uapsd_flush_queue(pst_dmac_vap, pst_dmac_user);
                if((0 == al_uapsd_qdepth)&& MAC_USR_UAPSD_USE_TIM(pst_dmac_user))
                {
                    /* 调用PSM的TIM设置接口 */
                    dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);
                }
                else
                {
                    if(0 != al_uapsd_qdepth)
                    {
                        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_uapsden::al_uapsd_qdepth null.}");
                    }
                }
            }
        }
    }

    return OAL_SUCC;
}



oal_void  dmac_uapsd_user_destroy(dmac_user_stru *pst_dmac_usr)
{
    dmac_user_uapsd_stru       *pst_uapsd_stru;

    oal_netbuf_stru             *pst_net_buf;

    /*调用者保证入参指针非空*/
    pst_uapsd_stru = &(pst_dmac_usr->st_uapsd_stru);

#ifdef _PRE_WLAN_DFT_STAT
    /* 首先，释放统计计数器占用的内存 */
    if(OAL_PTR_NULL != pst_uapsd_stru->pst_uapsd_statis)
    {
        OAL_MEM_FREE(pst_uapsd_stru->pst_uapsd_statis, OAL_TRUE);
        pst_uapsd_stru->pst_uapsd_statis = OAL_PTR_NULL;
    }
#endif

    /* 释放节能队列中的mpdu */
    oal_spin_lock(&pst_uapsd_stru->st_lock_uapsd);
    while (0 != oal_atomic_read(&pst_uapsd_stru->uc_mpdu_num))
    {
        pst_net_buf = dmac_tx_dequeue_first_mpdu(&pst_uapsd_stru->st_uapsd_queue_head);

        if(OAL_PTR_NULL == pst_net_buf)
        {
            break;
        }
        oal_atomic_dec(&pst_uapsd_stru->uc_mpdu_num);
        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_net_buf);
    }
    oal_spin_unlock(&pst_uapsd_stru->st_lock_uapsd);

    return ;

}

#endif /* _PRE_WLAN_FEATURE_UAPSD */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

