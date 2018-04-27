    


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_net.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "dmac_vap.h"
#include "dmac_main.h"
#include "dmac_user.h"
#include "dmac_psm_ap.h"
#include "dmac_uapsd.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_rx_data.h"
#include "hal_ext_if.h"
#include "dmac_blockack.h"
#include "dmac_tx_complete.h"
#include "dmac_beacon.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "dmac_p2p.h"
#endif
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_PSM_AP_C
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/


OAL_STATIC oal_uint32 dmac_psm_queue_send(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{
    dmac_user_ps_stru       *pst_ps_structure;
    oal_netbuf_stru         *pst_net_buf;
    oal_int32                l_ps_mpdu_num;
    mac_tx_ctl_stru         *pst_tx_ctrl;
    oal_uint32               ul_ret;
    mac_device_stru         *pst_mac_device;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_user_psm_stats_stru *pst_psm_stats;
#endif

    pst_ps_structure = &pst_dmac_user->st_ps_structure;

#ifdef _PRE_WLAN_DFT_STAT
    pst_psm_stats = pst_ps_structure->pst_psm_stats;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_psm_stats))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_queue_send::psm_stats is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    /* 对节能队列进行操作，需要加锁保护 */
    oal_spin_lock(&pst_ps_structure->st_lock_ps);

    /* 从节能队列中取出一个mpdu,并将队列中mpdu数目减1 */
    pst_net_buf = dmac_psm_dequeue_first_mpdu(pst_ps_structure);
    if (OAL_PTR_NULL == pst_net_buf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_psm_queue_send::pst_net_buf null.}");
        DMAC_PSM_STATS_INCR(pst_psm_stats->ul_psm_dequeue_fail_cnt);
        oal_spin_unlock(&pst_ps_structure->st_lock_ps);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_spin_unlock(&pst_ps_structure->st_lock_ps);

    DMAC_PSM_STATS_INCR(pst_psm_stats->ul_psm_dequeue_succ_cnt);

    /* 判断节能队列中是否还有缓存帧，如果有，设置帧头里的MORE DATA位为1，如果队列
       已空，并且此次发送的是单播缓存帧，则关闭tim_bitmap的相应位 */
    l_ps_mpdu_num = oal_atomic_read(&pst_ps_structure->uc_mpdu_num);
    if(IS_AP(&pst_dmac_vap->st_vap_base_info))
    {
        if (0 != l_ps_mpdu_num)
        {
            dmac_psm_set_more_data(pst_net_buf);
        }
#ifndef _PRE_WLAN_MAC_BUGFIX_MCAST_HW_Q
        /*广播在dtim count为0的beacon组帧完成后置0*/
        else if (OAL_TRUE != pst_dmac_user->st_user_base_info.en_is_multi_user)
#else
        else
#endif
        {
            dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);
        }
    }

    pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);

    if(OAL_TRUE == MAC_GET_CB_IS_MCAST(pst_tx_ctrl))
    {
#ifndef _PRE_WLAN_MAC_BUGFIX_MCAST_HW_Q
        /* 将节能缓存帧设为节能AC，从组播队列发送 */
        MAC_GET_CB_WME_AC_TYPE(pst_tx_ctrl) = WLAN_WME_AC_PSM;
#else
        MAC_GET_CB_WME_AC_TYPE(pst_tx_ctrl) = WLAN_WME_AC_VO;
        /*32用户时转发的ARP太多，原来进入管理帧队列会导致管理帧发不出来，所以修改为VO队列*/
#endif
    }

    /* 置这一位的原因是:接下来要调用dmac_tx_process_data，而进入这个函数之后会判断
       是否需要将mpdu入队，如果没有下面的这个判断，会导致循环入队，这个mpdu在pspoll
       节能机制下将永远发不出去 */
    MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_ctrl) = OAL_TRUE;

    /* 根据帧的类型，调用相应发送接口 */
    if (WLAN_DATA_BASICTYPE == MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_ctrl))
    {
        if (OAL_FALSE == pst_dmac_user->st_user_base_info.en_is_multi_user && OAL_TRUE == dmac_user_get_ps_mode(&pst_dmac_user->st_user_base_info))
        {
            dmac_tid_resume(pst_dmac_vap->pst_hal_device, &pst_dmac_user->ast_tx_tid_queue[MAC_GET_CB_WME_TID_TYPE(pst_tx_ctrl)], DMAC_TID_PAUSE_RESUME_TYPE_PS);
        }

        if (MAC_GET_CB_WLAN_FRAME_SUBTYPE(pst_tx_ctrl) == WLAN_NULL_FRAME)
        {
            ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_net_buf, MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl));
            if (OAL_SUCC != ul_ret)
            {
                oal_netbuf_free(pst_net_buf);
                OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                                 "{dmac_psm_queue_send::null data from ps queue. failed[%d].}", ul_ret);
                DMAC_PSM_STATS_INCR(pst_psm_stats->ul_psm_send_mgmt_fail_cnt);
            }
            else
            {
                OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                                "{dmac_psm_queue_send::null data from ps queue.succ}");
            }
            return ul_ret;
        }

        ul_ret = dmac_tx_process_data(pst_dmac_vap->pst_hal_device, pst_dmac_vap, pst_net_buf);
        if (OAL_SUCC != ul_ret)
        {
            pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
            if (OAL_PTR_NULL == pst_mac_device)
            {
                oal_netbuf_free(pst_net_buf);
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_device_id, OAM_SF_PWR, "{dmac_psm_queue_send::pst_mac_device null.}");
                return OAL_ERR_CODE_PTR_NULL;
            }
            OAM_INFO_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                            "{dmac_psm_queue_send::dmac_tx_process_data failed[%d],dev_mpdu_num = %d}", ul_ret, pst_mac_device->us_total_mpdu_num);
            OAM_INFO_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_psm_queue_send:: be = %d, bk = %d, vi = %d, vo = %d}",
                             pst_mac_device->aus_ac_mpdu_num[WLAN_WME_AC_BE],
                             pst_mac_device->aus_ac_mpdu_num[WLAN_WME_AC_BK],
                             pst_mac_device->aus_ac_mpdu_num[WLAN_WME_AC_VI],
                             pst_mac_device->aus_ac_mpdu_num[WLAN_WME_AC_VO]);
            dmac_tx_excp_free_netbuf(pst_net_buf);
            DMAC_PSM_STATS_INCR(pst_psm_stats->ul_psm_send_data_fail_cnt);

            return ul_ret;
        }
    }
    else
    {
        ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_net_buf, MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl));
        if (OAL_SUCC != ul_ret)
        {
            oal_netbuf_free(pst_net_buf);
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                             "{dmac_psm_queue_send::dmac_tx_mgmt failed[%d].}", ul_ret);
            DMAC_PSM_STATS_INCR(pst_psm_stats->ul_psm_send_mgmt_fail_cnt);
            return ul_ret;
        }
    }

    return OAL_SUCC;
}


oal_void dmac_psm_queue_flush(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{
    oal_int32       l_ps_mpdu_num;
    oal_uint32      ul_tid_mpdu_num;
    oal_int32       l_ps_mpdu_send_succ = 0;
    oal_int32       l_ps_mpdu_send_fail = 0;
    oal_uint32      ul_ret;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_user_psm_stats_stru *pst_psm_stats;
#endif

    l_ps_mpdu_num = oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num);
    if (l_ps_mpdu_num == 0)
    {
        if(IS_AP(&pst_dmac_vap->st_vap_base_info))
        {
            ul_tid_mpdu_num = dmac_psm_tid_mpdu_num(pst_dmac_user);
            if (0 == ul_tid_mpdu_num)
            {
                dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);
            }
            else
            {
                OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_PWR,"dmac_psm_queue_flush:: user[%d] has ps[%d] & Total mpdus[%d] in TID",
                           pst_dmac_user->st_user_base_info.us_assoc_id, l_ps_mpdu_num, ul_tid_mpdu_num);
            }
        }
        return;
    }

    /*  背景扫描或者DBAC切信道后，VAP处于PAUSE状态，节能队列里的数据帧
        如果是广播帧，会进高优先级硬件队列发错信道
        如果是单播数据帧，也会被重新进TID队列
        需要在VAP PAUSE的时候挡掉节能队列里数据帧的发送 */
    if (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                        "dmac_psm_queue_flush: vap status is pause, do not send psm queue frame! user id[%d]",
                        pst_dmac_user->st_user_base_info.us_assoc_id);
        return;
    }

    /* 循环将节能队列中的所有帧发给用户 */
    while (l_ps_mpdu_num-- > 0)
    {
        ul_ret = dmac_psm_queue_send(pst_dmac_vap, pst_dmac_user);
        if (OAL_SUCC != ul_ret)
        {
            OAM_INFO_LOG1(0, OAM_SF_PWR, "{dmac_psm_queue_flush::dmac_psm_queue_send fail[%d].}", ul_ret);
            l_ps_mpdu_send_fail++;
        }
        else
        {
            l_ps_mpdu_send_succ++;
        }
    }

#ifdef _PRE_WLAN_DFT_STAT
    pst_psm_stats = pst_dmac_user->st_ps_structure.pst_psm_stats;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_psm_stats))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_queue_flush::psm_stats is null.}");
        return;
    }
    pst_psm_stats->ul_psm_dequeue_succ_cnt = (oal_uint32)l_ps_mpdu_send_succ;
    pst_psm_stats->ul_psm_dequeue_fail_cnt = (oal_uint32)l_ps_mpdu_send_fail;
#endif
    if (l_ps_mpdu_send_fail)
    {
        OAM_INFO_LOG3(0, OAM_SF_PWR, "{dmac_psm_queue_flush::user[%d] send %d & fail %d}",
                       pst_dmac_user->st_user_base_info.us_assoc_id, l_ps_mpdu_send_succ, l_ps_mpdu_send_fail);
    }

    OAM_INFO_LOG3(0, OAM_SF_PWR, "{dmac_psm_queue_flush::user[%d] send %d & fail %d}",
                  pst_dmac_user->st_user_base_info.us_assoc_id, l_ps_mpdu_send_succ, l_ps_mpdu_send_fail);

}
#ifdef _PRE_WLAN_MAC_BUGFIX_PSM_BACK

oal_uint32  dmac_psm_flush_txq_to_tid(mac_device_stru *pst_mac_device,
                                                        dmac_vap_stru  *pst_dmac_vap,
                                                        dmac_user_stru *pst_dmac_user)
{
    oal_uint8                     uc_q_idx           = 0;
    hal_tx_dscr_stru             *pst_tx_dscr        = OAL_PTR_NULL;
    mac_tx_ctl_stru              *pst_cb             = OAL_PTR_NULL;
    dmac_tid_stru                *pst_tid_queue      = OAL_PTR_NULL;
    hal_to_dmac_device_stru      *pst_hal_device     = OAL_PTR_NULL;
    oal_dlist_head_stru          *pst_dlist_pos      = OAL_PTR_NULL;
    oal_dlist_head_stru          *pst_dlist_n        = OAL_PTR_NULL;
    oal_netbuf_stru              *pst_mgmt_buf       = OAL_PTR_NULL;
    dmac_user_stru               *pst_mcast_user     = OAL_PTR_NULL;
    oal_dlist_head_stru           ast_pending_q[WLAN_TID_MAX_NUM];
    oal_uint8                     auc_mpdu_num[WLAN_TID_MAX_NUM];
    oal_uint8                     uc_tid             = 0;
    oal_uint32                    ul_ret             = 0;
    oal_uint16                    us_seq_num         = 0;
    dmac_ba_tx_stru              *pst_ba_hdl;
    oal_uint8                     uc_dscr_status     = DMAC_TX_INVALID;
    hal_tx_dscr_stru             *pst_tx_dscr_next   = OAL_PTR_NULL;

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_flush_txq_to_tid::pst_mac_device is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_flush_txq_to_tid::pst_hal_device is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 遍历高优先级发送队列，将属于节能用户的管理帧的描述符释放，并把帧放到节能队列 */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_dlist_pos, pst_dlist_n, &pst_hal_device->ast_tx_dscr_queue[HAL_TX_QUEUE_HI].st_header)
    {
        pst_tx_dscr  = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_pos, hal_tx_dscr_stru, st_entry);
        pst_mgmt_buf = pst_tx_dscr->pst_skb_start_addr;
        pst_cb       = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_mgmt_buf);
        /* 如果不是本用户的帧，不需要挪到节能队列 */
        if (MAC_GET_CB_TX_USER_IDX(pst_cb) != pst_dmac_user->st_user_base_info.us_assoc_id)
        {
            continue;
        }
        hal_tx_get_dscr_status(pst_hal_device, pst_tx_dscr, &uc_dscr_status);
        /* 如果该帧已经发送过了，不需要挪到节能队列 */
        if (DMAC_TX_INVALID != uc_dscr_status)
        {
            continue;
        }
        /* 释放描述符 */
        oal_dlist_delete_entry(&pst_tx_dscr->st_entry);
        OAL_MEM_FREE(pst_tx_dscr, OAL_TRUE);

        /* 放到节能队列，此时节能队列为空，enqueue操作后，数据排序同硬件队列的先后顺序 */
        dmac_psm_ps_enqueue(pst_dmac_vap, pst_dmac_user, pst_mgmt_buf);

        /* 从硬件队列删除ppdu后，需要将ppducnt减1 */
        pst_hal_device->ast_tx_dscr_queue[HAL_TX_QUEUE_HI].uc_ppdu_cnt
                = OAL_SUB(pst_hal_device->ast_tx_dscr_queue[HAL_TX_QUEUE_HI].uc_ppdu_cnt, 1);

    }
    /* 初始化临时队列 */
    for (uc_tid = 0; uc_tid < WLAN_TID_MAX_NUM; uc_tid++)
    {
        oal_dlist_init_head(&ast_pending_q[uc_tid]);
        auc_mpdu_num[uc_tid] = 0;
    }

    /* 遍历其余4个发送队列，将属于该节能用户的mpud从硬件队列删除并添加到临时队列中 */
    for (uc_q_idx = 0; uc_q_idx < HAL_TX_QUEUE_HI; uc_q_idx++)
    {
        oal_bool_enum en_dscr_list_is_ampdu       = OAL_FALSE;  /*当前硬件队列的描述符链类型是否是ampdu类型*/
        oal_bool_enum en_dscr_list_first_flag     = OAL_TRUE;   /*是否是描述符链的第一个描述符*/
        oal_bool_enum en_dscr_list_back_flag      = OAL_FALSE;   /*描述符链是否需要放回tid队列*/
        oal_bool_enum en_curr_dscr_back_flag      = OAL_FALSE;   /*当前描述符是否需要放回tid队列*/

        OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_dlist_pos, pst_dlist_n, &pst_hal_device->ast_tx_dscr_queue[uc_q_idx].st_header)
        {
            pst_tx_dscr  = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_pos, hal_tx_dscr_stru, st_entry);
            pst_cb       = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_tx_dscr->pst_skb_start_addr);

            /* 如果不是本用户的帧，不需要挪到TID队列 */
            if (MAC_GET_CB_TX_USER_IDX(pst_cb) != pst_dmac_user->st_user_base_info.us_assoc_id)
            {
                continue;
            }

            /*如果是描述符链的第一个描述符，则需要确定是amdpu描述链还是mpdu描述符链*/
            if (OAL_TRUE == en_dscr_list_first_flag)
            {
                en_dscr_list_is_ampdu = (0 == pst_tx_dscr->bit_is_ampdu)? OAL_FALSE : OAL_TRUE;
            }

            /* 如果是ampdu链的第一个描述符，或者是mpdu链的第一个未处理描述符 */
            if (((OAL_TRUE == en_dscr_list_is_ampdu) &&  (OAL_TRUE == en_dscr_list_first_flag))
                 || (OAL_FALSE == en_dscr_list_is_ampdu))
            {
                /* 获取发送状态位 */
                hal_tx_get_dscr_status(pst_hal_device, pst_tx_dscr, &uc_dscr_status);

                /* 如果硬件没有发送，或者发生状态为9(软件需要再次挂到硬件，硬件才能发送)，软件需要放回TID队列 */
                if ((DMAC_TX_INVALID == uc_dscr_status)||(DMAC_TX_PENDING == uc_dscr_status))
                {
                    /* 从硬件队列删除ppdu后，需要将ppducnt减1 */
                    pst_hal_device->ast_tx_dscr_queue[uc_q_idx].uc_ppdu_cnt
                                = OAL_SUB(pst_hal_device->ast_tx_dscr_queue[uc_q_idx].uc_ppdu_cnt, 1);

                    en_dscr_list_back_flag = OAL_TRUE;  /*指示AMPDU链需要放回TID队列， MPDU链不需要使用*/
                    en_curr_dscr_back_flag = OAL_TRUE;  /*指示本描述符需要放回TID队列， AMPDU链/MPDU链都要使用*/
                }
                else /*已经发送过的AMPDU链， 或者MPDU链中的元素， 不需要放回tid队列*/
                {
                    en_dscr_list_back_flag = OAL_FALSE; /*指示AMPDU链不需要放回TID队列， MPDU链不需要使用*/
                    en_curr_dscr_back_flag = OAL_FALSE; /*指示本描述符不需要放回TID队列， AMPDU链/MPDU链都要使用*/

                    /*
                        MPDU描述符链，发送过的MPDU的next指针要指向空，防止后续硬件未发送的帧重新挂到硬件时，
                        已发送的最后一个帧的next指向了新挂进来的第一帧，造成无法正确找到新挂到硬件的第一帧。
                        AMPDU没有此问题。
                    */
                    if(OAL_FALSE == en_dscr_list_is_ampdu)
                    {
                        hal_tx_ctrl_dscr_unlink(pst_hal_device, pst_tx_dscr);
                    }
                }

            }
            else /*AMPDU的后续帧处理与AMPDU的第一帧保持一致*/
            {
                en_curr_dscr_back_flag = en_dscr_list_back_flag;
            }

            /*根据flag指示， 将描述从dev队列中删除，放入临时队列中*/
            if (OAL_TRUE == en_curr_dscr_back_flag)
            {
                /* 将device队列中的描述符删除 */
                oal_dlist_delete_entry(&pst_tx_dscr->st_entry);

                /* 设置软件重发标记 */
                pst_tx_dscr->bit_is_retried = OAL_TRUE;
                MAC_GET_CB_RETRIED_NUM(pst_cb)++;

                /* 将描述符放入临时队列，待统一增加到tid头部 */
                oal_dlist_add_tail(&pst_tx_dscr->st_entry, &ast_pending_q[MAC_GET_CB_WME_TID_TYPE(pst_cb)]);
                /* 统计每一个临时变量的mpdu个数 */
                auc_mpdu_num[MAC_GET_CB_WME_TID_TYPE(pst_cb)]++;
            }

            /* 获取新的dscr list开始 */
            hal_get_tx_dscr_next(pst_hal_device, pst_tx_dscr, &pst_tx_dscr_next);
            en_dscr_list_first_flag = (pst_tx_dscr_next == OAL_PTR_NULL) ? OAL_TRUE : OAL_FALSE;
        }
    }

    /* 将临时队列中的mpdu插入tid的头部 */
    for (uc_tid = 0; uc_tid < WLAN_TID_MAX_NUM; uc_tid++)
    {
        if (oal_dlist_is_empty(&ast_pending_q[uc_tid]))
        {
            continue;
        }
        pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[uc_tid]);

        ul_ret = dmac_tid_tx_queue_enqueue_head(&pst_dmac_user->st_user_base_info, pst_tid_queue, &ast_pending_q[uc_tid], auc_mpdu_num[uc_tid]);
        if (OAL_SUCC == ul_ret)
        {
            pst_tid_queue->uc_retry_num += auc_mpdu_num[uc_tid];
            /* 入队成功时需要通知算法 */
            dmac_alg_tid_update_notify(pst_tid_queue);
            OAM_INFO_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                          "{dmac_psm_flush_txq_to_tid:: uc_tid[%d] mpdu_num[%d]} OK!", uc_tid, auc_mpdu_num[uc_tid]);
        #if (defined(_PRE_WLAN_DFT_STAT) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE))
            if (OAL_PTR_NULL != pst_tid_queue->pst_tid_stats)
            {
                DMAC_TID_STATS_INCR(pst_tid_queue->pst_tid_stats->ul_tid_retry_enqueue_cnt, auc_mpdu_num[uc_tid]);
            }
        #endif
        }
        else
        {
            /* 入队失败需要设置ba会话标志位 */
            pst_ba_hdl = pst_tid_queue->pst_ba_tx_hdl;
            if (OAL_PTR_NULL != pst_ba_hdl)
            {
                OAL_DLIST_SEARCH_FOR_EACH(pst_dlist_pos, &ast_pending_q[uc_tid])
                {
                    pst_tx_dscr  = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_pos, hal_tx_dscr_stru, st_entry);
                    hal_tx_get_dscr_seq_num(pst_hal_device, pst_tx_dscr, &us_seq_num);
                    dmac_ba_update_baw(pst_ba_hdl, us_seq_num);
                }
            }
        #if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
            pst_dmac_vap->st_vap_base_info.st_vap_stats.ul_tx_dropped_packets += auc_mpdu_num[uc_tid];
        #endif
            OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                             "{dmac_psm_flush_txq_to_tid::uc_tid[%d] mpdu_num[%d]} failed[%d].}", uc_tid, auc_mpdu_num[uc_tid], ul_ret);
            dmac_tx_excp_free_dscr(&ast_pending_q[uc_tid], pst_hal_device);
        }
    }

    /* 将VO队列中的组播\广播帧也需要回收到当前vap下的组播用户的对应tid里面 */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_dlist_pos, pst_dlist_n, &pst_hal_device->ast_tx_dscr_queue[HAL_TX_QUEUE_VO].st_header)
    {
        pst_tx_dscr  = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_pos, hal_tx_dscr_stru, st_entry);
        pst_cb       = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_tx_dscr->pst_skb_start_addr);

        /* 如果是组播\广播帧，并且属于当前vap(入参vap)，则回收到vap的组播用户的tid里面 */
        if ((OAL_TRUE != MAC_GET_CB_IS_MCAST(pst_cb)) || (MAC_GET_CB_TX_VAP_INDEX(pst_cb) == pst_dmac_vap->st_vap_base_info.uc_vap_id))
        {
            continue;
        }

        /* 从硬件队列拿下来的mcast和bcast放回相应的vap的组播用户的相应tid队列 */
        pst_mcast_user = mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_cb));
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mcast_user))
        {
            OAM_WARNING_LOG1(0, OAM_SF_PWR,
                "{dmac_psm_flush_txq_to_tid::pst_mcast_user[%d] is null.}", MAC_GET_CB_TX_USER_IDX(pst_cb));
            continue;
        }
        /* 将该mpdu从硬件队列拿下来，并将ppducnt减1 */
        oal_dlist_delete_entry(&pst_tx_dscr->st_entry);
        pst_hal_device->ast_tx_dscr_queue[HAL_TX_QUEUE_VO].uc_ppdu_cnt
                = OAL_SUB(pst_hal_device->ast_tx_dscr_queue[HAL_TX_QUEUE_VO].uc_ppdu_cnt, 1);

        pst_tid_queue = &pst_mcast_user->ast_tx_tid_queue[MAC_GET_CB_WME_TID_TYPE(pst_cb)];
    #ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        pst_tx_dscr->bit_is_retried = OAL_TRUE;
        /* 入重传队列 */
        oal_dlist_add_head(&pst_tx_dscr->st_entry, &pst_tid_queue->st_retry_q);
    #else
        oal_dlist_add_head(&pst_tx_dscr->st_entry, &pst_tid_queue->st_hdr);
    #endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

        /* 放入重传队列 */
        dmac_tid_tx_enqueue_update(pst_mac_device, pst_tid_queue, 1);
        if (OAL_TRUE == pst_tx_dscr->bit_is_retried)
        {
            pst_tid_queue->uc_retry_num++;
        }

    #ifdef _PRE_WLAN_FEATURE_FLOWCTL
        dmac_alg_flowctl_backp_notify(&pst_dmac_vap->st_vap_base_info,
                                      pst_mac_device->us_total_mpdu_num,
                                      pst_mac_device->aus_ac_mpdu_num);
    #endif

    }
    /* 清空硬件发送队列，并重新将各队列通知mac */
    hal_clear_hw_fifo(pst_hal_device);

    /*将还没有来得及处理的描述符挂回给硬件， 软件ppdu num不需要修改*/
    for (uc_q_idx = 0; uc_q_idx < HAL_TX_QUEUE_BUTT; uc_q_idx++)
    {
        oal_bool_enum en_dscr_list_is_ampdu   = OAL_FALSE;  /*当前挂给硬件的描述符链类型是否是ampdu类型*/
        oal_bool_enum en_dscr_list_first_flag = OAL_TRUE;   /*是否是描述符链的第一个描述符*/
        oal_bool_enum en_mpdu_list_put_flag   = OAL_FALSE;  /*表示mpdu描述符链是否已经挂给硬件*/

        OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_dlist_pos, pst_dlist_n, &pst_hal_device->ast_tx_dscr_queue[uc_q_idx].st_header)
        {
            pst_tx_dscr  = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_pos, hal_tx_dscr_stru, st_entry);
            if (OAL_TRUE == en_dscr_list_first_flag)
            {
                en_dscr_list_is_ampdu = (0 == pst_tx_dscr->bit_is_ampdu)? OAL_FALSE : OAL_TRUE;
                en_mpdu_list_put_flag = OAL_FALSE;
            }

            /*如果是ampdu链的第一个描述符， 或者是mpdu链的第一个未处理描述符，则需要处理*/
            if (((OAL_TRUE == en_dscr_list_is_ampdu) && (OAL_TRUE == en_dscr_list_first_flag))
                || ((OAL_FALSE == en_dscr_list_is_ampdu) && (OAL_FALSE == en_mpdu_list_put_flag)))
            {
                /* 获取发送状态位 */
                hal_tx_get_dscr_status(pst_hal_device, pst_tx_dscr, &uc_dscr_status);

                /* 如果硬件没有发送，软件需要重新通知到硬件FIFO */
                /* 如果硬件已经发送，软件不需要通知到硬件FIFO，等着中断上来直接处理 */
                if ((DMAC_TX_INVALID == uc_dscr_status)||(DMAC_TX_PENDING == uc_dscr_status))
                {
                    hal_tx_put_dscr(pst_hal_device, uc_q_idx, pst_tx_dscr);
                    if (OAL_FALSE == en_dscr_list_is_ampdu)
                    {
                        en_mpdu_list_put_flag = OAL_TRUE;
                    }
                }
            }

            /* 获取新的dscr list开始 */
            hal_get_tx_dscr_next(pst_hal_device, pst_tx_dscr, &pst_tx_dscr_next);
            en_dscr_list_first_flag = (pst_tx_dscr_next == OAL_PTR_NULL) ? OAL_TRUE : OAL_FALSE;
        }
    }

    return OAL_SUCC;
}

#endif


oal_uint32  dmac_psm_reset(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru          *pst_event;
    frw_event_hdr_stru      *pst_event_hdr;
    oal_uint16              *pus_user_id;
    dmac_user_stru          *pst_dmac_user;
    dmac_vap_stru           *pst_dmac_vap;
    oal_int32                ul_uapsd_qdepth = 0;

    if ((OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_reset::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event       = frw_get_event_stru(pst_event_mem);
    pst_event_hdr   = &(pst_event->st_event_hdr);
    pus_user_id     = (oal_uint16 *)pst_event->auc_event_data;

    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(*pus_user_id);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_PWR, "{dmac_psm_reset::pst_dmac_user[%d] null.", *pus_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == pst_dmac_user->bit_ps_mode)
    {
        /* 用户是非节能的，直接返回成功 */

        return OAL_SUCC;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_PWR, "{dmac_psm_reset::pst_dmac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

#ifndef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
    hal_tx_disable_peer_sta_ps_ctrl(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif

    OAM_WARNING_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_PWR, "{dmac_psm_reset::user changes ps mode to active.}");

    DMAC_PSM_CHANGE_USER_PS_STATE(pst_dmac_user->bit_ps_mode, OAL_FALSE);


    pst_dmac_vap->uc_ps_user_num = OAL_SUB(pst_dmac_vap->uc_ps_user_num, 1);    /* 节能用户个数清除 */
    dmac_user_resume(pst_dmac_user);                                            /* 恢复user，恢复该user的每一个tid */
    dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);                  /* 该修改体现在beacon中的信息 */


    dmac_psm_queue_flush(pst_dmac_vap, pst_dmac_user);
#ifdef _PRE_WLAN_FEATURE_UAPSD
    ul_uapsd_qdepth = dmac_uapsd_flush_queue(pst_dmac_vap, pst_dmac_user);
    if(ul_uapsd_qdepth < 0)
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_uapsd_flush_queue:: return value is -1.}");
        return OAL_FAIL;
    }
    /*uc_uapsd_send = dmac_uapsd_flush_queue(pst_dmac_vap, pst_dmac_user, &uc_uapsd_left);
    OAM_INFO_LOG2(pst_event_hdr->uc_vap_id, OAM_SF_PWR,
                  "{dmac_psm_reset::dmac_uapsd_flush_queue %d send & %d left}", uc_uapsd_send, uc_uapsd_left);*/
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_psm_awake(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{

#ifndef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
    hal_tx_disable_peer_sta_ps_ctrl(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif

    DMAC_PSM_CHANGE_USER_PS_STATE(pst_dmac_user->bit_ps_mode, OAL_FALSE);
    pst_dmac_user->st_ps_structure.uc_ps_time_count = 0;
    pst_dmac_vap->uc_ps_user_num--;

    /* 恢复user，恢复该user的每一个tid */
    dmac_user_resume(pst_dmac_user);

    /* 将所有的缓存帧发送出去 */
    dmac_psm_queue_flush(pst_dmac_vap, pst_dmac_user);

    /* 发送完节能队列里的缓存帧后，设置了本地bitmap，及时更新beacon的tim信息元素 */
    dmac_encap_beacon(pst_dmac_vap, pst_dmac_vap->pauc_beacon_buffer[pst_dmac_vap->uc_beacon_idx], &(pst_dmac_vap->us_beacon_len));

    OAM_INFO_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_PWR,
                  "{dmac_psm_awake::user[%d] is active.}\r\n", pst_dmac_user->st_user_base_info.us_assoc_id);

    return OAL_SUCC;
}


oal_uint32  dmac_psm_doze(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{
    mac_device_stru     *pst_macdev;

    pst_macdev = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_macdev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_doze:: mac_dev is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    DMAC_PSM_CHANGE_USER_PS_STATE(pst_dmac_user->bit_ps_mode, OAL_TRUE);
    pst_dmac_user->st_ps_structure.uc_ps_time_count = 0;
    pst_dmac_vap->uc_ps_user_num++;

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    pst_dmac_user->ul_sta_sleep_times++;                    /** STA休眠次数加1 */
#endif

    /* pause该用户,但是不能pause uapsd专用tid */
    dmac_user_pause(pst_dmac_user);
    dmac_tid_resume(pst_dmac_vap->pst_hal_device, &pst_dmac_user->ast_tx_tid_queue[WLAN_TIDNO_UAPSD], DMAC_TID_PAUSE_RESUME_TYPE_PS);

#ifdef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
    /* suspend硬件队列 */
    hal_set_machw_tx_suspend(pst_dmac_vap->pst_hal_device);

    /* 遍历硬件队列，将属于该用户的帧都放回tid */
    dmac_psm_flush_txq_to_tid(pst_macdev, pst_dmac_vap, pst_dmac_user);

    hal_set_machw_tx_resume(pst_dmac_vap->pst_hal_device);
#else
    /* 暂停该vap下的所有用户的硬件队列的发送, 硬件上报psm_back,软件再回收 */
    hal_tx_enable_peer_sta_ps_ctrl(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif

    /* 如果psm/tid不为空，则设置PVB */
    if ((OAL_FALSE == dmac_psm_is_psm_empty(pst_dmac_user)) ||
        (OAL_FALSE == dmac_psm_is_tid_empty(pst_dmac_user)))
    {
        dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 1);
        OAM_INFO_LOG2(0, OAM_SF_PWR, "{dmac_psm_doze::user[%d].%d mpdu in tid.}",
                       pst_dmac_user->st_user_base_info.us_assoc_id, oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num));
    }

    return OAL_SUCC;
}


oal_void dmac_psm_rx_process(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_net_buf)
{
    mac_ieee80211_frame_stru        *pst_mac_header;
    mac_rx_ctl_stru                 *pst_rx_ctrl;
    oal_int32                        l_ps_mpdu_num = 0;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_user || OAL_PTR_NULL == pst_net_buf))
    {
        OAM_ERROR_LOG3(0, OAM_SF_PWR, "{dmac_psm_rx_process_data::param is null.vap=[%d], user=[%d],buf=[%d]}.",
                       (oal_uint32)pst_dmac_vap, (oal_uint32)pst_dmac_user, (oal_uint32)pst_net_buf);
        return;
    }

    pst_rx_ctrl = (mac_rx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
    pst_mac_header = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(pst_rx_ctrl);
    /*
       如果节能位开启(bit_power_mgmt == 1),同时之前处于非节能模式，则修改节能
       模式为节能,否则什么都不做，因为用户仍在节能，ap继续为其缓存帧就可以了;
       如果节能位关闭(bit_power_mgmt == 0),同时之前处于节能模式，则修改节能模
       式为非节能，并且将相应节能队列中的所有缓存帧都发给该用户，否则，什么都
       不做，用户一直都是非节能的，ap与用户之间正常收发数据
    */
    if ((OAL_TRUE == pst_mac_header->st_frame_control.bit_power_mgmt)
         && (OAL_FALSE == pst_dmac_user->bit_ps_mode))
    {
        dmac_psm_doze(pst_dmac_vap, pst_dmac_user);
    }
    if ((OAL_FALSE == pst_mac_header->st_frame_control.bit_power_mgmt)
         && (OAL_TRUE == pst_dmac_user->bit_ps_mode))
    {
        dmac_psm_awake(pst_dmac_vap, pst_dmac_user);
    }
    else if ((OAL_FALSE == pst_mac_header->st_frame_control.bit_power_mgmt)
        && (OAL_FALSE == pst_dmac_user->bit_ps_mode))
    {
        /* 用户之前是非节能的，但是可能由于VAP PAUSE导致节能队列帧未及时发送出去 */
        l_ps_mpdu_num = oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num);
        if (l_ps_mpdu_num > 0)
        {
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                        "dmac_psm_queue_flush: force flush psm queue if it is not empty");
            /* 将所有的缓存帧发送出去 */
            dmac_psm_queue_flush(pst_dmac_vap, pst_dmac_user);
             /* 发送完节能队列里的缓存帧后，设置了本地bitmap，及时更新beacon的tim信息元素 */
            dmac_encap_beacon(pst_dmac_vap, pst_dmac_vap->pauc_beacon_buffer[pst_dmac_vap->uc_beacon_idx], &(pst_dmac_vap->us_beacon_len));
        }
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P GO关连设备停止节能，P2P OppPS暂停*/
    if ((OAL_FALSE == pst_mac_header->st_frame_control.bit_power_mgmt)&&
        (IS_P2P_GO(&pst_dmac_vap->st_vap_base_info))&&
        (IS_P2P_OPPPS_ENABLED(pst_dmac_vap)))
    {
        pst_dmac_vap->st_p2p_ops_param.en_pause_ops = OAL_TRUE;
    }
    /* P2P GO关连设备使能节能，允许P2P OppPS*/
    if ((OAL_TRUE == pst_mac_header->st_frame_control.bit_power_mgmt)&&
        (IS_P2P_GO(&pst_dmac_vap->st_vap_base_info))&&
        (IS_P2P_OPPPS_ENABLED(pst_dmac_vap)))
    {
        pst_dmac_vap->st_p2p_ops_param.en_pause_ops = OAL_FALSE;

    }

#endif
}

oal_uint32  dmac_psm_send_null_data(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_bool_enum_uint8 en_ps)
{
    oal_netbuf_stru                 *pst_net_buf;
    mac_tx_ctl_stru                 *pst_tx_ctrl;
    oal_uint32                       ul_ret;
    oal_uint16                       us_tx_direction = WLAN_FRAME_FROM_AP;
    mac_ieee80211_frame_stru        *pst_mac_header;
    oal_uint8                        uc_legacy_rate;
    wlan_phy_protocol_enum_uint8     en_protocol_mode;
    oal_uint8                        uc_null_protocol_mode;
    oal_uint8                        uc_null_legacy_rate;
    mac_device_stru                 *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                        "dmac_psm_send_null_data: mac_device is null.device_id[%d]",
                        pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state) &&
        (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                        "dmac_psm_send_null_data: device is scaning and vap status is pause, do not send null!vap_mode[%d]",
                        pst_dmac_vap->st_vap_base_info.en_vap_mode);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_P2P_DEV_MODE == mac_get_p2p_mode(&pst_dmac_vap->st_vap_base_info))
    {
        return OAL_FAIL;
    }
#endif

    /* 申请net_buff */
    pst_net_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_net_buf)
    {
        
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_psm_send_null_data::alloc netbuff failed.}");
        OAL_MEM_INFO_PRINT(OAL_MEM_POOL_ID_NETBUF);
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    OAL_MEM_NETBUF_TRACE(pst_net_buf, OAL_TRUE);

    oal_set_netbuf_prev(pst_net_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_net_buf, OAL_PTR_NULL);

    /* null帧发送方向From AP || To AP */
    us_tx_direction = (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode) ? WLAN_FRAME_FROM_AP : WLAN_FRAME_TO_AP;
    /* 填写帧头,其中from ds为1，to ds为0，因此frame control的第二个字节为02 */
    mac_null_data_encap(OAL_NETBUF_HEADER(pst_net_buf),
                        ((oal_uint16)(WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_NODATA) | us_tx_direction),
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                        mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info));
    pst_mac_header = (mac_ieee80211_frame_stru*)OAL_NETBUF_HEADER(pst_net_buf);

    /*  NB: power management bit is never sent by an AP */
    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        pst_mac_header->st_frame_control.bit_power_mgmt = en_ps;
    }

    /* 填写cb字段 */
    pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
    OAL_MEMZERO(pst_tx_ctrl, OAL_SIZEOF(mac_tx_ctl_stru));

    /* 填写tx部分 */
    MAC_GET_CB_ACK_POLACY(pst_tx_ctrl)            = WLAN_TX_NORMAL_ACK;
    MAC_GET_CB_EVENT_TYPE(pst_tx_ctrl)         = FRW_EVENT_TYPE_WLAN_DTX;
    MAC_GET_CB_RETRIED_NUM(pst_tx_ctrl)           = 0;
    MAC_GET_CB_WME_TID_TYPE(pst_tx_ctrl)   =  WLAN_TID_FOR_DATA;
    MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctrl)          = pst_dmac_vap->st_vap_base_info.uc_vap_id;
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctrl)           = pst_dmac_user->st_user_base_info.us_assoc_id;

    if (IS_AP(&(pst_dmac_vap->st_vap_base_info)))
    {
        MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_ctrl)   = OAL_FALSE;/* AP 发送null 帧放入节能队列 */
    }
    else
    {
        MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_ctrl)   = OAL_TRUE;
    }
    MAC_SET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl, (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_net_buf));
    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl)    = OAL_SIZEOF(mac_ieee80211_frame_stru);
    MAC_GET_CB_MPDU_NUM(pst_tx_ctrl)               = 1;
    MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl)             = 1;
    MAC_GET_CB_MPDU_LEN(pst_tx_ctrl)               = 0;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctrl) = WLAN_WME_AC_MGMT;


    /* 2.4G初始化为11b 1M, long preable, 发送null帧时修改为11g/a的速率 */
    uc_legacy_rate   = pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate;
    en_protocol_mode = pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
    dmac_change_null_data_rate(pst_dmac_vap,pst_dmac_user,&uc_null_protocol_mode, &uc_null_legacy_rate);

    dmac_psm_set_ucast_mgmt_tx_rate(pst_dmac_vap, WLAN_BAND_2G, uc_null_legacy_rate, uc_null_protocol_mode);

    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_net_buf, OAL_SIZEOF(mac_ieee80211_frame_stru));

    /* 发送完成后恢复默认值: 2.4G初始化为11b 1M, long preable */
    dmac_psm_set_ucast_mgmt_tx_rate(pst_dmac_vap, WLAN_BAND_2G, uc_legacy_rate, en_protocol_mode);

    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                         "{dmac_psm_send_null_data::dmac_tx_mgmt failed[%d].}", ul_ret);
        if (OAL_LIKELY(OAL_PTR_NULL != pst_dmac_user->st_ps_structure.pst_psm_stats))
        {
            DMAC_PSM_STATS_INCR(pst_dmac_user->st_ps_structure.pst_psm_stats->ul_psm_send_null_fail_cnt);
        }

        dmac_tx_excp_free_netbuf(pst_net_buf);
        return ul_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_psm_sch_psm_queue(dmac_vap_stru *pst_dmac_vap, dmac_user_stru  *pst_dmac_user)
{
    oal_int32              l_ps_mpdu_num;
    oal_uint32             ul_ret;

    l_ps_mpdu_num       = oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num);

    if (l_ps_mpdu_num > 0)
    {
        ul_ret = dmac_psm_queue_send(pst_dmac_vap, pst_dmac_user);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG3(0, OAM_SF_PWR, "{dmac_psm_sch_psm_queue::user[%d] send fail[%d] & %d left.}",
                           pst_dmac_user->st_user_base_info.us_assoc_id, ul_ret, l_ps_mpdu_num - 1);
            return ul_ret;
        }
        OAM_INFO_LOG2(0, OAM_SF_PWR, "{dmac_psm_sch_psm_queue::user[%d] send 1 & %d left}",
                       pst_dmac_user->st_user_base_info.us_assoc_id, l_ps_mpdu_num - 1);
    }

    return OAL_SUCC;
}

oal_uint32 dmac_psm_sch_tid_queue(dmac_user_stru  *pst_dmac_user, oal_uint32 ul_ps_mpdu_num)
{
    hal_to_dmac_device_stru      *pst_hal_device;
    oal_dlist_head_stru          *pst_dlist_pos      = OAL_PTR_NULL;
    hal_tx_dscr_stru             *pst_tx_dscr        = OAL_PTR_NULL;
    oal_netbuf_stru              *pst_mgmt_buf       = OAL_PTR_NULL;
    oal_uint8                     uc_tid_idx         = 0;
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    dmac_tid_stru                *pst_txtid;
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
    pst_hal_device = dmac_user_get_hal_device(&(pst_dmac_user->st_user_base_info));
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_PWR, "{dmac_psm_sch_tid_queue::pst_hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }


#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    /* 先遍历重传队列 */
    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx ++)
    {
        pst_txtid = &pst_dmac_user->ast_tx_tid_queue[uc_tid_idx];
        if (OAL_TRUE == oal_dlist_is_empty(&pst_txtid->st_retry_q))
        {
            continue;
        }

        // dmac_tid_resume中会调度并发送，可能删除当前tid的一个或者多个帧，即便使用SAFE版本也不能解决
        // 必须在删除之后立刻退出该循环
        OAL_DLIST_SEARCH_FOR_EACH(pst_dlist_pos, &pst_txtid->st_retry_q)
        {
            /* 节能队列非空或该用户TID队列还有其他mpdu，需要设置moredata */
            /* 先设置再resume，虽然调度出的帧可能并不是首帧 */
            if (ul_ps_mpdu_num > 1)
            {
                pst_tx_dscr  = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_pos, hal_tx_dscr_stru, st_entry);
                pst_mgmt_buf = pst_tx_dscr->pst_skb_start_addr;
                dmac_psm_set_more_data(pst_mgmt_buf);
            }

            /* 发送的是该用户任意非空TID队列中的第一个 */
            dmac_tid_resume(pst_hal_device, pst_txtid, DMAC_TID_PAUSE_RESUME_TYPE_PS);

            return OAL_SUCC;
        }
    }


    /* 再遍历缓存队列 */
    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx ++)
    {
        pst_txtid = &pst_dmac_user->ast_tx_tid_queue[uc_tid_idx];
        if (OAL_TRUE == oal_netbuf_list_empty(&pst_txtid->st_buff_head))
        {
            continue;
        }

        OAL_NETBUF_SEARCH_FOR_EACH(pst_mgmt_buf, &pst_txtid->st_buff_head)
        {
            /* 节能队列非空或该用户TID队列还有其他mpdu，需要设置moredata */
            if (ul_ps_mpdu_num > 1)
            {
                dmac_psm_set_more_data(pst_mgmt_buf);
            }

            /* 发送的是该用户任意非空TID队列中的第一个 */
            dmac_tid_resume(pst_hal_device, pst_txtid, DMAC_TID_PAUSE_RESUME_TYPE_PS);

            return OAL_SUCC;
        }
    }
#else
    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx ++)
    {
        if (OAL_TRUE == oal_dlist_is_empty(&pst_dmac_user->ast_tx_tid_queue[uc_tid_idx].st_hdr))
        {
            continue;
        }
        OAL_DLIST_SEARCH_FOR_EACH(pst_dlist_pos, &pst_dmac_user->ast_tx_tid_queue[uc_tid_idx].st_hdr)
        {
            /* 节能队列非空或该用户TID队列还有其他mpdu，需要设置moredata */
            if (ul_ps_mpdu_num > 1)
            {
                pst_tx_dscr  = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_pos, hal_tx_dscr_stru, st_entry);
                pst_mgmt_buf = pst_tx_dscr->pst_skb_start_addr;
                dmac_psm_set_more_data(pst_mgmt_buf);
            }

            /* 发送的是该用户任意非空TID队列中的第一个 */
            dmac_tid_resume(pst_hal_device, &pst_dmac_user->ast_tx_tid_queue[uc_tid_idx], DMAC_TID_PAUSE_RESUME_TYPE_PS);

            return OAL_SUCC;
        }
    }
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_psm_handle_pspoll(dmac_vap_stru *pst_dmac_vap, dmac_user_stru  *pst_dmac_user, oal_uint8 *puc_extra_qosnull)
{
    oal_uint32                    ul_ret            = OAL_SUCC;
    oal_uint32                    ul_tid_mpud_num   = 0;

#ifndef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
    hal_tx_disable_peer_sta_ps_ctrl(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif

    /* 如果tid不为空 */
    ul_tid_mpud_num  = dmac_psm_tid_mpdu_num(pst_dmac_user);

    if (ul_tid_mpud_num)
    {
        ul_tid_mpud_num += (oal_uint32)oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num);
        ul_ret = dmac_psm_sch_tid_queue(pst_dmac_user, ul_tid_mpud_num);
        if (OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG1(0, OAM_SF_PWR, "{dmac_psm_handle_pspoll::dmac_psm_sch_tid_queue fail[%d].}", ul_ret);
            return ul_ret;
        }
        if (ul_tid_mpud_num == 1)
        {
            dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);
        }
    }
    /* 如果psm节能不为空 */
    else if(oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num))
    {
        ul_ret = dmac_psm_sch_psm_queue(pst_dmac_vap, pst_dmac_user);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_PWR, "{dmac_psm_handle_pspoll::dmac_psm_sch_psm_queue fail[%d].}", ul_ret);
            return ul_ret;
        }
        if (0 == oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num))
        {
            dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);
        }
    }
    else
    {
        *puc_extra_qosnull = 1;
        return OAL_SUCC;
    }
    pst_dmac_user->st_ps_structure.en_is_pspoll_rsp_processing = OAL_TRUE;

    return ul_ret;
}


oal_uint32 dmac_psm_resv_ps_poll(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{
    oal_uint32          ul_ret;
    oal_uint8           uc_extra_qosnull = 0;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_user_psm_stats_stru *pst_psm_stats;

    pst_psm_stats = pst_dmac_user->st_ps_structure.pst_psm_stats;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_psm_stats))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_resv_ps_poll::psm_stats is null!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    /* 检查vap模式 */
    if (WLAN_VAP_MODE_BSS_AP != pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                         "{dmac_psm_resv_ps_poll::ap is not in ap mode.}");
        return OAL_SUCC;
    }

    /* 检查是否有ps-poll没处理完，如果有，则忽略当前的ps-poll，直接返回 */
    if (OAL_TRUE == pst_dmac_user->st_ps_structure.en_is_pspoll_rsp_processing)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                         "{dmac_psm_resv_ps_poll::ignor excess ps-poll.}");
        return OAL_SUCC;
    }

    ul_ret = dmac_psm_handle_pspoll(pst_dmac_vap, pst_dmac_user, &uc_extra_qosnull);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                         "{dmac_psm_resv_ps_poll::hand_pspoll return [%d].}", ul_ret);
        DMAC_PSM_STATS_INCR(pst_psm_stats->ul_psm_rsp_pspoll_fail_cnt);
        return ul_ret;
    }

    if (uc_extra_qosnull)
    {
        dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 0);
        DMAC_PSM_STATS_INCR(pst_psm_stats->ul_psm_resv_pspoll_send_null);
        ul_ret = dmac_psm_send_null_data(pst_dmac_vap, pst_dmac_user, OAL_FALSE);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,
                             "{dmac_psm_resv_ps_poll::user[%d] send_null fail[%d].}", ul_ret);
        }
        return ul_ret;
    }

    DMAC_PSM_STATS_INCR(pst_psm_stats->ul_psm_rsp_pspoll_succ_cnt);
    return OAL_SUCC;
}


oal_netbuf_stru* dmac_psm_dequeue_first_mpdu(dmac_user_ps_stru  *pst_ps_structure)
{
    oal_netbuf_stru        *pst_first_net_buf;
    oal_netbuf_stru        *pst_tmp_net_buf;
    oal_netbuf_stru        *pst_net_buf;
    mac_tx_ctl_stru        *pst_tx_ctrl;
    oal_uint8               uc_netbuf_num_per_mpdu;
    oal_netbuf_head_stru   *pst_ps_queue_head;

    pst_ps_queue_head = &pst_ps_structure->st_ps_queue_head;

    pst_first_net_buf = OAL_NETBUF_HEAD_NEXT(pst_ps_queue_head);
    pst_tx_ctrl       = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_first_net_buf);

    /* mpdu不是a-msdu，返回第一个net_buff即可 */
    if (OAL_FALSE == MAC_GET_CB_IS_AMSDU(pst_tx_ctrl))
    {
        pst_first_net_buf = oal_netbuf_delist(pst_ps_queue_head);
        oal_atomic_dec(&pst_ps_structure->uc_mpdu_num);
        return pst_first_net_buf;
    }

    if (OAL_FALSE == MAC_GET_CB_IS_FIRST_MSDU(pst_tx_ctrl))
    {
        /* 这个错误只有踩内存才会出现，如果出现就无法恢复，以太网来的包无法释放，
           软件复位也没用，内存也会泄漏
        */
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_dequeue_first_mpdu::not the first msdu.}");
        return OAL_PTR_NULL;
    }

    /* 节能队列中的第一个mpdu是a-msdu，获取skb个数 */
    uc_netbuf_num_per_mpdu = MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl);

    /* 将第一个mpdu中的所有skb从节能队列中取出，然后组成一个net_buff链 */
    pst_first_net_buf = oal_netbuf_delist(pst_ps_queue_head);
    uc_netbuf_num_per_mpdu--;

    pst_tmp_net_buf = pst_first_net_buf;
    while (0 != uc_netbuf_num_per_mpdu)
    {
        pst_net_buf = oal_netbuf_delist(pst_ps_queue_head);
        oal_set_netbuf_prev(pst_net_buf, pst_tmp_net_buf);
        oal_set_netbuf_next(pst_net_buf, OAL_PTR_NULL);

        oal_set_netbuf_next(pst_tmp_net_buf, pst_net_buf);

        pst_tmp_net_buf = pst_net_buf;

        uc_netbuf_num_per_mpdu--;
    }

    oal_atomic_dec(&pst_ps_structure->uc_mpdu_num);

    return pst_first_net_buf;

}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_void dmac_psm_delete_ps_queue_head(dmac_user_stru *pst_dmac_user,oal_uint32 ul_psm_delete_num)
{
    dmac_user_ps_stru              *pst_ps_structure;
    oal_netbuf_stru                *pst_net_buf;

    pst_ps_structure = &(pst_dmac_user->st_ps_structure);
    oal_spin_lock(&pst_ps_structure->st_lock_ps);
    /* 节能队列不为空的情况下，释放节能队列中的mpdu */
    while (oal_atomic_read(&pst_ps_structure->uc_mpdu_num) && (ul_psm_delete_num--))
    {
        /* 由于节能队列中的mpdu可能来自wlan也可能来自lan，在释放的时候需要区分，
           因此不能将队列中的mpdu一次性释放，而是应该以mpdu为单位进行释放
        */
        pst_net_buf = dmac_psm_dequeue_first_mpdu(pst_ps_structure);

        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_net_buf);

        oal_spin_unlock(&pst_ps_structure->st_lock_ps);
        oal_spin_lock(&pst_ps_structure->st_lock_ps);
    }
    oal_spin_unlock(&pst_ps_structure->st_lock_ps);

}
#endif


oal_void dmac_psm_clear_ps_queue(dmac_user_stru *pst_dmac_user)
{
    dmac_user_ps_stru   *pst_ps_structure;
    oal_netbuf_stru     *pst_net_buf;

    pst_ps_structure = &(pst_dmac_user->st_ps_structure);

    /* 节能队列不为空的情况下，释放节能队列中的mpdu */
    while (oal_atomic_read(&pst_ps_structure->uc_mpdu_num))
    {
        oal_spin_lock(&pst_ps_structure->st_lock_ps);

        /* 由于节能队列中的mpdu可能来自wlan也可能来自lan，在释放的时候需要区分，
           因此不能将队列中的mpdu一次性释放，而是应该以mpdu为单位进行释放
        */
        pst_net_buf = dmac_psm_dequeue_first_mpdu(pst_ps_structure);

        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_net_buf);

        oal_spin_unlock(&pst_ps_structure->st_lock_ps);
    }

}


oal_uint32  dmac_psm_user_ps_structure_destroy(dmac_user_stru *pst_dmac_user)
{
    dmac_user_ps_stru   *pst_ps_structure;
    oal_netbuf_stru     *pst_net_buf;

    pst_ps_structure = &(pst_dmac_user->st_ps_structure);

#ifdef _PRE_WLAN_DFT_STAT
    if (OAL_PTR_NULL != pst_ps_structure->pst_psm_stats)
    {
        OAL_MEM_FREE(pst_ps_structure->pst_psm_stats, OAL_TRUE);
        pst_ps_structure->pst_psm_stats = OAL_PTR_NULL;
    }
#endif

    pst_ps_structure->en_is_pspoll_rsp_processing = OAL_FALSE;

    /* 节能队列不为空的情况下，释放节能队列中的mpdu */
    while (oal_atomic_read(&pst_ps_structure->uc_mpdu_num))
    {
        oal_spin_lock(&pst_ps_structure->st_lock_ps);

        /* 由于节能队列中的mpdu可能来自wlan也可能来自lan，在释放的时候需要区分，
           因此不能将队列中的mpdu一次性释放，而是应该以mpdu为单位进行释放
        */
        pst_net_buf = dmac_psm_dequeue_first_mpdu(pst_ps_structure);

        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_net_buf);

        oal_spin_unlock(&pst_ps_structure->st_lock_ps);
    }

    return OAL_SUCC;
}


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

