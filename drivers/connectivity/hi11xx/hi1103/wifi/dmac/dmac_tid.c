


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_tid.h"
#include "dmac_main.h"
#include "dmac_ext_if.h"
#include "dmac_blockack.h"
#include "hal_ext_if.h"
#include "dmac_alg.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_tx_complete.h"
#include "wlan_types.h"
#include "oal_net.h"
#include "mac_data.h"



#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TID_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
oal_uint32  dmac_release_tid_buffs_by_user(dmac_user_stru    *pst_dmac_user,
                                        oal_uint32       ul_nums)
{
    oal_uint32                      ul_tid_idx;
    dmac_tid_stru                  *pst_tid_queue;
    oal_uint32                      ul_remain_nums = ul_nums;
    oal_uint32                      ul_free_nums = 0;
    oal_uint32                      ul_free_mpdu_nums = 0;

    for (ul_tid_idx = 0; ul_tid_idx < WLAN_TID_MAX_NUM; ul_tid_idx++)
    {
        pst_tid_queue = &pst_dmac_user->ast_tx_tid_queue[ul_tid_idx];

        if(0 == pst_tid_queue->us_mpdu_num)
            continue;

        if(pst_tid_queue->us_mpdu_num >= ul_remain_nums)
        {
            ul_free_mpdu_nums = ul_remain_nums;
        }
        else
        {
            ul_free_mpdu_nums = (oal_uint32)pst_tid_queue->us_mpdu_num;
        }

        if(OAL_SUCC == dmac_tid_delete_mpdu_tail(pst_tid_queue, (oal_uint16)ul_free_mpdu_nums))
        {
            ul_free_nums += ul_free_mpdu_nums;
        }

        if(ul_free_nums >= ul_nums)
        {
            break;
        }

        ul_remain_nums = ul_nums - ul_free_nums;
    }

    return ul_free_nums;
}

oal_uint32  dmac_release_tid_buffs_by_vap(dmac_vap_stru     *pst_dmac_vap,
                                        oal_uint32       ul_nums)
{
    dmac_user_stru          *pst_dmac_user;
    oal_dlist_head_stru     *pst_entry;
    mac_user_stru           *pst_user_tmp;
    oal_uint32          ul_remain_nums = ul_nums;
    oal_uint32          ul_free_nums = 0;

    if(NULL == pst_dmac_vap)
    {
        return ul_free_nums;
    }

    if(OAL_TRUE == oal_dlist_is_empty(&(pst_dmac_vap->st_vap_base_info.st_mac_user_list_head)))
    {
        return ul_free_nums;
    }

    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_dmac_vap->st_vap_base_info.st_mac_user_list_head))
    {
        pst_user_tmp = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
        /*lint -save -e774 */
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_user_tmp))
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_release_tid_buffs_by_vap::null pointer.}");
            continue;
        }
        /*lint -restore */

        pst_dmac_user = MAC_GET_DMAC_USER(pst_user_tmp);

        ul_free_nums += dmac_release_tid_buffs_by_user(pst_dmac_user, ul_remain_nums);
        if(ul_free_nums >= ul_nums)
        {
            break;
        }
        ul_remain_nums = ul_nums - ul_free_nums ;
    }

    return ul_free_nums;
}


oal_uint32  dmac_release_tid_buffs(dmac_vap_stru     *arg1,
                                        mac_device_stru  *arg2,
                                        oal_uint32       ul_nums)
{
    dmac_vap_stru    *pst_dmac_vap = (dmac_vap_stru*)arg1;
    mac_device_stru  *pst_device   = (mac_device_stru*)arg2;
    oal_uint32 ul_free_nums = 0;
    oal_uint32 ul_remain_nums = ul_nums;
    oal_uint8  uc_vap_idx;
    dmac_vap_stru  *pst_dmac_crr_vap = NULL;

    /*优先删除当前VAP*/
    if(NULL != pst_dmac_vap)
    {
        ul_free_nums += dmac_release_tid_buffs_by_vap(pst_dmac_vap, ul_remain_nums);
    }

    if(ul_free_nums >= ul_nums)
    {
        return ul_free_nums;
    }

    ul_remain_nums = ul_nums - ul_free_nums;

    /* 遍历device下所有vap， */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++)
    {
        pst_dmac_crr_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_dmac_crr_vap)
        {
            OAM_ERROR_LOG1(pst_device->auc_vap_id[uc_vap_idx], OAM_SF_ANY,
                           "{dmac_release_tid_buffs::pst_vap null,uc_vap_idx=%d.}",
                           uc_vap_idx);
            continue;
        }

        if(pst_dmac_crr_vap == pst_dmac_vap)
        {
            continue;
        }

        ul_free_nums += dmac_release_tid_buffs_by_vap(pst_dmac_crr_vap, ul_remain_nums);
        if(ul_free_nums >= ul_nums)
        {
            break;
        }

        ul_remain_nums = ul_nums - ul_free_nums;
    }

    return ul_free_nums;

}
#endif


oal_uint32  dmac_tid_get_min_max_mpdu_length(mac_user_stru *pst_mac_user,
                                            oal_uint8 uc_tid_num,
                                        oal_uint16      us_head_mpdu_num,
                                        oal_uint16     *pus_min_mpdu_len,
                                        oal_uint16     *pus_max_mpdu_len,
                                        oal_uint8      *puc_msdu_num)
{
    oal_uint16              us_mpdu_idx;
    oal_uint16              us_mpdu_num;
    hal_tx_dscr_stru       *pst_dscr;
    oal_dlist_head_stru    *pst_dscr_entry;
    oal_netbuf_stru        *pst_netbuf;
    mac_tx_ctl_stru        *pst_tx_ctrl;
    oal_uint16              us_min_mpdu_len = 0xffff;
    oal_uint16              us_max_mpdu_len = 0;
    oal_uint8               uc_msdu_num = 0;
    dmac_user_stru         *pst_dmac_user;
    dmac_tid_stru          *pst_tid_queue;
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_uint16              us_tid_netbuf_num;
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    oal_uint8               uc_is_amsdu = OAL_FALSE;
#endif
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user || OAL_PTR_NULL == pus_min_mpdu_len ||
        OAL_PTR_NULL == pus_max_mpdu_len))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_tid_get_min_max_mpdu_length::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_mac_user);
    pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[uc_tid_num]);
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    //us_min_mpdu_len = 0;
    //us_max_mpdu_len = 0;

    /* 先遍历重传队列 */
    if (OAL_TRUE != oal_dlist_is_empty(&pst_tid_queue->st_retry_q))
    {
        pst_dscr_entry  = pst_tid_queue->st_retry_q.pst_next;
        us_mpdu_num     = OAL_MIN(pst_tid_queue->uc_retry_num, us_head_mpdu_num);

        for (us_mpdu_idx = 0; us_mpdu_idx < us_mpdu_num; us_mpdu_idx++)
        {
            pst_dscr    = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
            pst_netbuf  = pst_dscr->pst_skb_start_addr;
            pst_tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

            if (MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) < us_min_mpdu_len)
            {
                us_min_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctrl);
            }

            if (MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) > us_max_mpdu_len)
            {
                us_max_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctrl);

            #ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
                /* 1103适配tx amsdu+ampdu,避免聚合个数计算有误 */
                if (OAL_TRUE == MAC_GET_CB_HAS_EHTER_HEAD(pst_tx_ctrl))
                {
                    uc_is_amsdu = OAL_TRUE;
                }
            #endif
            }

            if (MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl) >  uc_msdu_num)
            {
                uc_msdu_num = MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl);
            }

            pst_dscr_entry = pst_dscr_entry->pst_next;
        }
    }

    if ((us_head_mpdu_num > pst_tid_queue->uc_retry_num) && (OAL_TRUE != oal_netbuf_list_empty(&pst_tid_queue->st_buff_head)))
    {
        //us_mpdu_num = us_head_mpdu_num - pst_tid_queue->uc_retry_num;
        us_tid_netbuf_num  = pst_tid_queue->us_mpdu_num - pst_tid_queue->uc_retry_num;

        us_mpdu_num = OAL_MIN(us_tid_netbuf_num, us_head_mpdu_num - pst_tid_queue->uc_retry_num);

        /* 再遍历缓存队列 */
        pst_netbuf   = pst_tid_queue->st_buff_head.pst_next;
        for (us_mpdu_idx = 0; us_mpdu_idx < us_mpdu_num; us_mpdu_idx++)
        {
            pst_tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
            if (MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) < us_min_mpdu_len)
            {
                us_min_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctrl);
            }

            if (MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) > us_max_mpdu_len)
            {
                us_max_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctrl);

            #ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
                /* 1103适配tx amsdu+ampdu，避免聚合个数计算有误  */
                if (OAL_TRUE == MAC_GET_CB_HAS_EHTER_HEAD(pst_tx_ctrl))
                {
                    uc_is_amsdu = OAL_TRUE;
                }
            #endif
            }

            if (MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl) >  uc_msdu_num)
            {
                uc_msdu_num = MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl);
            }

            pst_netbuf = pst_netbuf->next;

            if (pst_netbuf == (oal_netbuf_stru *)&pst_tid_queue->st_buff_head)
            {
                break;
            }

        }

    }
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    if(OAL_TRUE == uc_is_amsdu)
    {
        us_max_mpdu_len = (oal_uint16)((us_max_mpdu_len + ETHER_HDR_LEN) << 1);
    }
#endif
#else
    /* 如果tid缓存队列为空，程序直接退出 */
    if (OAL_TRUE == oal_dlist_is_empty(&pst_tid_queue->st_hdr))
    {
        *pus_min_mpdu_len = 0;
        *pus_max_mpdu_len = 0;
        *puc_msdu_num     = 0;
        return OAL_SUCC;
    }

    pst_dscr_entry  = pst_tid_queue->st_hdr.pst_next;
    us_mpdu_num     = OAL_MIN(pst_tid_queue->us_mpdu_num, us_head_mpdu_num);

    for (us_mpdu_idx = 0; us_mpdu_idx < us_mpdu_num; us_mpdu_idx++)
    {
        pst_dscr    = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        pst_netbuf  = pst_dscr->pst_skb_start_addr;
        pst_tx_ctrl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

        if (MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) < us_min_mpdu_len)
        {
            us_min_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctrl);
        }

        if (MAC_GET_CB_MPDU_LEN(pst_tx_ctrl) > us_max_mpdu_len)
        {
            us_max_mpdu_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctrl);
        }

        if (MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl) >  uc_msdu_num)
        {
            uc_msdu_num = MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl);
        }

        pst_dscr_entry = pst_dscr_entry->pst_next;
    }
#endif /* */

    *pus_min_mpdu_len = us_min_mpdu_len;
    *pus_max_mpdu_len = us_max_mpdu_len;
    *puc_msdu_num     = uc_msdu_num;
    return OAL_SUCC;
}


oal_uint32  dmac_tid_clear(mac_user_stru *pst_mac_user, mac_device_stru *pst_mac_device)
{
    oal_uint32                      ul_tid_idx;
    dmac_tid_stru                  *pst_tid_queue;
    oal_dlist_head_stru            *pst_entry;
    hal_tx_dscr_stru               *pst_tx_dscr;
    dmac_user_stru                 *pst_dmac_user;
#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    mac_vap_stru                   *pst_vap;
#endif
#if defined(_PRE_WLAN_FEATURE_TX_DSCR_OPT) || defined(_PRE_DEBUG_MODE)
    oal_netbuf_stru                *pst_netbuf;
#endif
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8       en_trace_pkt_type;
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    mac_tx_ctl_stru            *pst_cb;
#endif

    if (OAL_PTR_NULL == pst_mac_user || OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_tid_clear::param null, pst_mac_user=%d, pst_mac_device=%d.}",
                       pst_mac_user, pst_mac_device);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_user = MAC_GET_DMAC_USER(pst_mac_user);

    for (ul_tid_idx = 0; ul_tid_idx < WLAN_TID_MAX_NUM; ul_tid_idx++)
    {
        pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[ul_tid_idx]);

    #ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        /* 释放重传包 */
        pst_entry = pst_tid_queue->st_retry_q.pst_next;
        while (pst_entry != &pst_tid_queue->st_retry_q)
        {
            pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_entry, hal_tx_dscr_stru, st_entry);
            oal_dlist_delete_entry(&pst_tx_dscr->st_entry);
            /* 重传队列中可能存在AMSDU帧 */
        #ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
            pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_tx_dscr->pst_skb_start_addr);
            if (OAL_TRUE == MAC_GET_CB_IS_LARGE_SKB_AMSDU(pst_cb))
            {
                dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 2);
            }
            else
        #endif
            {
                dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 1);
            }
            dmac_free_tx_dscr(pst_tx_dscr);

            pst_entry = pst_tid_queue->st_retry_q.pst_next;
        }

        pst_netbuf  = pst_tid_queue->st_buff_head.pst_next;
        while (pst_netbuf != (oal_netbuf_stru *)&pst_tid_queue->st_buff_head)
        {
            /* 再释放netbuf缓存队列 */
            pst_netbuf  = dmac_tx_dequeue_first_mpdu(&pst_tid_queue->st_buff_head);
        #ifdef _PRE_DEBUG_MODE
            en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_clear::type%d ampdu free[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
        #endif
            /* TID的只有单帧 */
            dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 1);

            dmac_tx_excp_free_netbuf(pst_netbuf);

            pst_netbuf  = pst_tid_queue->st_buff_head.pst_next;
        }
    #else
        /* 释放TID缓存中的包 */
        pst_entry = pst_tid_queue->st_hdr.pst_next;
        if (OAL_PTR_NULL == pst_entry)
        {
            continue;
        }

        while (pst_entry != &pst_tid_queue->st_hdr)
        {
            pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_entry, hal_tx_dscr_stru, st_entry);

#ifdef _PRE_DEBUG_MODE
            pst_netbuf = pst_tx_dscr->pst_skb_start_addr;
            en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_clear::type%d ampdu free[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
#endif

            pst_entry = pst_entry->pst_next;

            oal_dlist_delete_entry(&pst_tx_dscr->st_entry);
            dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 1);
            dmac_free_tx_dscr(pst_tx_dscr);
        }
    #endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
        pst_tid_queue->us_mpdu_num = 0;
        pst_tid_queue->uc_retry_num = 0;

        /* 释放BA相关的内容 */
        if (DMAC_BA_COMPLETE == pst_tid_queue->st_ba_rx_hdl.en_ba_conn_status)
        {
            dmac_ba_reset_rx_handle(pst_mac_device, pst_dmac_user, &pst_tid_queue->st_ba_rx_hdl);
        }

        if (OAL_PTR_NULL != pst_tid_queue->pst_ba_tx_hdl)
        {
            dmac_ba_reset_tx_handle(pst_mac_device, &(pst_tid_queue->pst_ba_tx_hdl), (oal_uint8)ul_tid_idx);
        }

    }
#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    pst_vap = mac_res_get_mac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    dmac_alg_flowctl_backp_notify(pst_vap, pst_mac_device->us_total_mpdu_num, pst_mac_device->aus_ac_mpdu_num);
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    if (pst_mac_device->us_total_mpdu_num > WLAN_TID_MPDU_NUM_LIMIT)
    {
        OAM_ERROR_LOG2(0, OAM_SF_TX, "{dmac_tid_clear::WLAN_TID_MPDU_NUM_LIMIT exceed, us_total_mpdu_num = %d.func_call[0x%x]}", pst_mac_device->us_total_mpdu_num, __return_address());
    }
#endif

    return OAL_SUCC;
}


oal_uint32  dmac_tid_resume(hal_to_dmac_device_stru *pst_hal_device, dmac_tid_stru *pst_tid, oal_uint8 uc_type)
{
    if ((OAL_PTR_NULL == pst_tid) || (OAL_PTR_NULL == pst_hal_device))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (DMAC_TID_PAUSE_RESUME_TYPE_BA == uc_type)
    {
        pst_tid->uc_is_paused &= ~BIT0;
    }
    else if (DMAC_TID_PAUSE_RESUME_TYPE_PS == uc_type)
    {
        pst_tid->uc_is_paused &= ~BIT1;
    }

    if (0 == pst_tid->uc_is_paused)
    {
#ifdef _PRE_WLAN_DFT_EVENT
        dmac_tid_status_change_event_to_sdt(pst_tid, pst_tid->uc_is_paused);
#endif
        /* 通知算法 */
        dmac_alg_tid_update_notify(pst_tid);
        dmac_tx_complete_schedule(pst_hal_device, WLAN_WME_TID_TO_AC(pst_tid->uc_tid));
    }

    return OAL_SUCC;
}


oal_uint32  dmac_tid_get_util_ratio(oal_uint8 uc_chip_id, oal_uint8 uc_device_id, oal_uint8 *puc_ratio)
{
    mac_device_stru *pst_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_ratio))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_tid_get_util_ratio::puc_ratio null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((uc_chip_id >= WLAN_CHIP_MAX_NUM_PER_BOARD) || (uc_device_id >= MAC_RES_MAX_DEV_NUM))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_tid_get_util_ratio::invalid param,uc_chip_id=%d, uc_device_id=%d.}", uc_chip_id, uc_device_id);

        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    pst_device = mac_res_get_dev(uc_device_id);
    if(OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_tid_get_util_ratio::pst_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 使用率(%) = (当前总数 * 100 )/最大数 = (当前总数 * 100 )/256 = (当前总数 * 100 ) >> 8 */
    *puc_ratio = (oal_uint8)((pst_device->us_total_mpdu_num * 100 ) >> WLAN_TID_MPDU_NUM_BIT);

    return OAL_SUCC;
}


oal_uint32  dmac_tid_delete_mpdu_head(
                dmac_tid_stru              *pst_tid_queue,
                oal_uint16                  us_mpdu_num)
{
    mac_device_stru            *pst_mac_device;
    oal_netbuf_stru            *pst_netbuf = OAL_PTR_NULL;
    hal_tx_dscr_stru           *pst_dscr;
    oal_dlist_head_stru        *pst_dscr_entry;
    dmac_user_stru             *pst_user;
    oal_uint16                  us_mpdu_idx;
    mac_tx_ctl_stru            *pst_cb;
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_uint16                  us_num_tmp;
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    mac_vap_stru               *pst_vap;
#endif
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8   en_trace_pkt_type;
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tid_queue))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_tid_delete_mpdu_head::pst_tid_queue null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_tid_queue->us_user_idx);

    if (OAL_PTR_NULL == pst_user)
    {
        OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_ANY,
            "{dmac_tid_delete_mpdu_head::pst_user[%d] null.}", pst_tid_queue->us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_user->st_user_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_tid_queue->uc_vap_id, OAM_SF_ANY, "{dmac_tid_delete_mpdu_head::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    /* 先遍历重传队列 */
    us_num_tmp      = OAL_MIN(pst_tid_queue->uc_retry_num, us_mpdu_num);
    for (us_mpdu_idx = 0; us_mpdu_idx < us_num_tmp; us_mpdu_idx++)
    {
        if (OAL_TRUE == oal_dlist_is_empty(&pst_tid_queue->st_retry_q))
        {
            break;
        }
        pst_dscr_entry  = oal_dlist_delete_head(&(pst_tid_queue->st_retry_q));
        pst_dscr        = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        pst_netbuf      = pst_dscr->pst_skb_start_addr;
        pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_delete_mpdu_head::type%d delete[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif
        if ((0 != pst_tid_queue->uc_retry_num) && (OAL_TRUE == pst_dscr->bit_is_retried))
        {
            pst_tid_queue->uc_retry_num--;
            if (OAL_PTR_NULL != pst_tid_queue->pst_ba_tx_hdl)
            {
                dmac_ba_update_baw(pst_tid_queue->pst_ba_tx_hdl, MAC_GET_CB_SEQ_NUM(pst_cb));
            }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    #ifdef _PRE_WLAN_DFT_STAT
            if (OAL_PTR_NULL != pst_tid_queue->pst_tid_stats)
            {
                DMAC_TID_STATS_INCR(pst_tid_queue->pst_tid_stats->ul_tid_retry_dequeue_cnt, 1);
            }
    #endif
#endif
        }

        /* 重传队列存在AMSDU帧 */
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
        if (OAL_TRUE == MAC_GET_CB_IS_LARGE_SKB_AMSDU(pst_cb))
        {
            dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 2);
        }
        else
#endif
        {
            dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 1);
        }

        OAL_MEM_FREE(pst_dscr, OAL_TRUE);
        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_netbuf);
    }

    if (us_mpdu_num > us_mpdu_idx)
    {
        us_num_tmp = us_mpdu_num - us_mpdu_idx;
        for (us_mpdu_idx = 0; us_mpdu_idx < us_num_tmp; us_mpdu_idx++)
        {
            if (OAL_TRUE == oal_netbuf_list_empty(&pst_tid_queue->st_buff_head))
            {
                break;
            }

            pst_netbuf  = dmac_tx_dequeue_first_mpdu(&pst_tid_queue->st_buff_head);
#ifdef _PRE_DEBUG_MODE
            //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
            en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_delete_mpdu_head::type%d delete[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
#endif
            g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_netbuf);

            dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 1);
        }
    }
#else
    for (us_mpdu_idx = 0; us_mpdu_idx < us_mpdu_num; us_mpdu_idx++)
    {
        /* 如果tid缓存队列为空，程序直接退出 */
        if (OAL_TRUE == oal_dlist_is_empty(&pst_tid_queue->st_hdr))
        {
            break;
        }

        pst_dscr_entry  = oal_dlist_delete_head(&(pst_tid_queue->st_hdr));
        pst_dscr        = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);

        pst_netbuf = pst_dscr->pst_skb_start_addr;
#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_delete_mpdu_head::type%d delete[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif

        if (0 != pst_tid_queue->uc_retry_num)
        {
            pst_tid_queue->uc_retry_num--;
            if (OAL_PTR_NULL != pst_tid_queue->pst_ba_tx_hdl)
            {
                pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
                dmac_ba_update_baw(pst_tid_queue->pst_ba_tx_hdl, MAC_GET_CB_SEQ_NUM(pst_cb));
            }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

    #ifdef _PRE_WLAN_DFT_STAT
            if (OAL_PTR_NULL != pst_tid_queue->pst_tid_stats)
            {
                DMAC_TID_STATS_INCR(pst_tid_queue->pst_tid_stats->ul_tid_retry_dequeue_cnt, 1);
            }
    #endif
#endif
        }
        OAL_MEM_FREE(pst_dscr, OAL_TRUE);
        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_netbuf);

        dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 1);
    }
#endif /* _PRE_TX_DSCR */

    if (OAL_PTR_NULL != g_st_dmac_tid_rom_cb.del_mpdu_head_cb)
    {
        if (OAL_RETURN == g_st_dmac_tid_rom_cb.del_mpdu_head_cb(pst_mac_device, pst_tid_queue, us_mpdu_num))
        {
            return OAL_SUCC;
        }
    }

    /* 更新tid_dlist链表 */
    dmac_alg_tid_update_notify(pst_tid_queue);

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    pst_vap = mac_res_get_mac_vap(pst_tid_queue->uc_vap_id);
    dmac_alg_flowctl_backp_notify(pst_vap, pst_mac_device->us_total_mpdu_num, pst_mac_device->aus_ac_mpdu_num);
#endif

    return OAL_SUCC;
}


oal_uint32  dmac_tid_delete_mpdu_tail(dmac_tid_stru *pst_tid_queue, oal_uint16 us_mpdu_num)
{
    mac_device_stru            *pst_mac_device;
    oal_netbuf_stru            *pst_netbuf = OAL_PTR_NULL;
    hal_tx_dscr_stru           *pst_dscr;
    oal_dlist_head_stru        *pst_dscr_entry;
    dmac_user_stru             *pst_user;
    oal_uint16                  us_mpdu_idx;
    mac_tx_ctl_stru            *pst_cb;

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    mac_vap_stru               *pst_vap;
#endif
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_uint16                  us_num_tmp;
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8   en_trace_pkt_type;
#endif


    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tid_queue))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_tid_delete_mpdu_tail::pst_tid_queue null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_UNLIKELY(pst_tid_queue->us_mpdu_num < us_mpdu_num))
    {
        OAM_WARNING_LOG2(pst_tid_queue->uc_vap_id, OAM_SF_ANY, "{dmac_tid_delete_mpdu_tail::uc_mpdu_num[%d] > tid_queue.us_mpdu_num[%d].}",
                         us_mpdu_num, pst_tid_queue->us_mpdu_num);
        return OAL_FAIL;
    }

    pst_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_tid_queue->us_user_idx);

    if (OAL_PTR_NULL == pst_user)
    {
        OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_ANY,
            "{dmac_tid_delete_mpdu_tail::pst_user[%d] null.}", pst_tid_queue->us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_user->st_user_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_tid_queue->uc_vap_id, OAM_SF_ANY, "{dmac_tid_delete_mpdu_tail::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    /* 先删除netbuf队列 */
    us_num_tmp = us_mpdu_num;
    for (us_mpdu_idx = 0; us_mpdu_idx < us_num_tmp; us_mpdu_idx++)
    {
        if (OAL_TRUE == oal_netbuf_list_empty(&pst_tid_queue->st_buff_head))
        {
            break;
        }

        pst_netbuf  = dmac_tx_dequeue_first_mpdu(&pst_tid_queue->st_buff_head);
        if (OAL_PTR_NULL == pst_netbuf)
        {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tid_delete_mpdu_tail::pst_netbuf null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_delete_mpdu_tail::type%d delete[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif

        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_netbuf);

        dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 1);
    }

    /* 再删除重传队列 */
    if (us_mpdu_num > us_mpdu_idx)
    {
        us_num_tmp = us_mpdu_num - us_mpdu_idx;
        for (us_mpdu_idx = 0; us_mpdu_idx < us_num_tmp; us_mpdu_idx++)
        {
            if (OAL_TRUE == oal_dlist_is_empty(&pst_tid_queue->st_retry_q))
            {
                break;
            }
            pst_dscr_entry  = oal_dlist_delete_tail(&(pst_tid_queue->st_retry_q));
            pst_dscr        = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
            pst_netbuf = pst_dscr->pst_skb_start_addr;
            pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    #ifdef _PRE_DEBUG_MODE
            //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
            en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_delete_mpdu_tail::type%d delete[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
    #endif
            if ((0 != pst_tid_queue->uc_retry_num) && (OAL_TRUE == pst_dscr->bit_is_retried))
            {
                pst_tid_queue->uc_retry_num--;
                if (OAL_PTR_NULL != pst_tid_queue->pst_ba_tx_hdl)
                {
                    dmac_ba_update_baw(pst_tid_queue->pst_ba_tx_hdl, MAC_GET_CB_SEQ_NUM(pst_cb));
                }
        #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        #ifdef _PRE_WLAN_DFT_STAT
                if (OAL_PTR_NULL != pst_tid_queue->pst_tid_stats)
                {
                    DMAC_TID_STATS_INCR(pst_tid_queue->pst_tid_stats->ul_tid_retry_dequeue_cnt, 1);
                }
        #endif
        #endif
            }

            /* 重传队列中可能存在AMSDU帧 */
    #ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
            if (OAL_TRUE == MAC_GET_CB_IS_LARGE_SKB_AMSDU(pst_cb))
            {
                dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 2);
            }
            else
    #endif
            {
                dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 1);
            }

            OAL_MEM_FREE(pst_dscr, OAL_TRUE);
            g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_netbuf);

        }
    }
#else
    /* 删除从pst_netbuf开始的N个MPDU */
    for (us_mpdu_idx = 0; us_mpdu_idx < us_mpdu_num; us_mpdu_idx++)
    {
        pst_dscr_entry = oal_dlist_delete_tail(&pst_tid_queue->st_hdr);
        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        pst_netbuf = pst_dscr->pst_skb_start_addr;
#ifdef _PRE_DEBUG_MODE
        //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
        en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_netbuf), MAC_NETBUFF_PAYLOAD_SNAP);
        if( PKT_TRACE_BUTT != en_trace_pkt_type)
        {
            OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_delete_mpdu_tail::type%d delete[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
        }
#endif

        if (pst_tid_queue->us_mpdu_num == pst_tid_queue->uc_retry_num)
        {
            pst_tid_queue->uc_retry_num--;

            if (OAL_PTR_NULL != pst_tid_queue->pst_ba_tx_hdl)
            {
                pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
                dmac_ba_update_baw(pst_tid_queue->pst_ba_tx_hdl, MAC_GET_CB_SEQ_NUM(pst_cb));
            }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    #ifdef _PRE_WLAN_DFT_STAT
            if (OAL_PTR_NULL != pst_tid_queue->pst_tid_stats)
            {
                DMAC_TID_STATS_INCR(pst_tid_queue->pst_tid_stats->ul_tid_retry_dequeue_cnt, 1);
            }
    #endif
#endif
        }

        OAL_MEM_FREE(pst_dscr, OAL_TRUE);
        g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_netbuf);

        dmac_tid_tx_dequeue_update(pst_mac_device, pst_tid_queue, 1);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        if(pst_tid_queue->us_mpdu_num < pst_tid_queue->uc_retry_num)
        {
            OAL_IO_PRINT(KERN_CRIT "pst_tid_queue->us_mpdu_num(%d) < pst_tid_queue->uc_retry_num(%d)\n",
                        pst_tid_queue->us_mpdu_num,pst_tid_queue->uc_retry_num);
        }
#endif
    }
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

    if (OAL_PTR_NULL != g_st_dmac_tid_rom_cb.del_mpdu_tail_cb)
    {
        if (OAL_RETURN== g_st_dmac_tid_rom_cb.del_mpdu_tail_cb(pst_mac_device, pst_tid_queue, us_mpdu_num))
        {
            return OAL_SUCC;
        }
    }

    /* 更新tid_dlist链表 */
    dmac_alg_tid_update_notify(pst_tid_queue);

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    pst_vap = mac_res_get_mac_vap(pst_tid_queue->uc_vap_id);
    dmac_alg_flowctl_backp_notify(pst_vap, pst_mac_device->us_total_mpdu_num, pst_mac_device->aus_ac_mpdu_num);
#endif

    return OAL_SUCC;
}


oal_void  dmac_tid_tx_enqueue_update(mac_device_stru *pst_device, dmac_tid_stru *pst_tid_queue, oal_uint8 uc_mpdu_num)
{
    /* 更新TID包计数 */
    pst_tid_queue->us_mpdu_num += uc_mpdu_num;

    /* 更新DEV包计数 */
    pst_device->us_total_mpdu_num += uc_mpdu_num;
    pst_device->aus_vap_mpdu_num[pst_tid_queue->uc_vap_id] += uc_mpdu_num;
    pst_device->aus_ac_mpdu_num[WLAN_WME_TID_TO_AC(pst_tid_queue->uc_tid)] += (oal_uint16)uc_mpdu_num;

}


oal_void  dmac_tid_tx_dequeue_update(mac_device_stru *pst_device, dmac_tid_stru *pst_tid_queue, oal_uint8 uc_mpdu_num)
{
    /* 更新TID包计数 */
    pst_tid_queue->us_mpdu_num -= uc_mpdu_num;

    /* 更新DEV包计数 */
    pst_device->us_total_mpdu_num -= uc_mpdu_num;
    pst_device->aus_vap_mpdu_num[pst_tid_queue->uc_vap_id] -= uc_mpdu_num;
    pst_device->aus_ac_mpdu_num[WLAN_WME_TID_TO_AC(pst_tid_queue->uc_tid)] -= (oal_uint16)uc_mpdu_num;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    if ((pst_device->us_total_mpdu_num > WLAN_TID_MPDU_NUM_LIMIT)
        || (pst_device->aus_ac_mpdu_num[WLAN_WME_TID_TO_AC(pst_tid_queue->uc_tid)] > WLAN_TID_MPDU_NUM_LIMIT))
    {
        OAM_ERROR_LOG2(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_dequeue_update::WLAN_TID_MPDU_NUM_LIMIT exceed, us_total_mpdu_num = %d.func_call[0x%x]}", pst_device->us_total_mpdu_num, __return_address());
    }
#endif
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_tid_tx_enqueue_tid_head(mac_user_stru *pst_user, dmac_tid_stru *pst_tid_queue, oal_netbuf_stru *pst_netbuf)
{
    mac_device_stru     *pst_device;
    oal_uint8            uc_mpdu_num = 0;
    oal_netbuf_stru     *pst_buf_next;

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    mac_vap_stru       *pst_vap;
#endif

    /* 更新device结构体下的统计信息 */
    pst_device = mac_res_get_dev(pst_user->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue_head::pst_device[%d] null.}", pst_user->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 进入TID队列,需要每个netbuf处理 */
    while (OAL_PTR_NULL != pst_netbuf)
    {
        pst_buf_next = oal_get_netbuf_next(pst_netbuf);

        dmac_tx_queue_mpdu_head(pst_netbuf, &pst_tid_queue->st_buff_head);

        dmac_tid_tx_enqueue_update(pst_device, pst_tid_queue, 1);

        uc_mpdu_num++;

        pst_netbuf = pst_buf_next;
    }

    if (pst_device->us_total_mpdu_num + uc_mpdu_num > WLAN_TID_MPDU_NUM_LIMIT)
    {
        OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue_head::WLAN_TID_MPDU_NUM_LIMIT exceed, dev_mpdu_num = %d}", pst_device->us_total_mpdu_num);
        OAM_WARNING_LOG4(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue_head::be = %d, bk = %d, vi = %d, vo = %d}",
                            pst_device->aus_ac_mpdu_num[WLAN_WME_AC_BE],
                            pst_device->aus_ac_mpdu_num[WLAN_WME_AC_BK],
                            pst_device->aus_ac_mpdu_num[WLAN_WME_AC_VI],
                            pst_device->aus_ac_mpdu_num[WLAN_WME_AC_VO]);
#ifdef _PRE_WLAN_PERFORM_STAT
        /* 性能统计日志 */
        dmac_stat_tid_per(pst_user, pst_tid_queue->uc_tid, 0, uc_mpdu_num, DMAC_STAT_PER_SW_RETRY_OVERFLOW);
#endif
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    pst_vap = mac_res_get_mac_vap(pst_tid_queue->uc_vap_id);
    dmac_alg_flowctl_backp_notify(pst_vap, pst_device->us_total_mpdu_num, pst_device->aus_ac_mpdu_num);
#endif

    return OAL_SUCC;
}
#endif


oal_uint32  dmac_tid_tx_queue_enqueue_head(mac_user_stru *pst_user, dmac_tid_stru *pst_tid_queue, oal_dlist_head_stru *pst_tx_dscr_list_hdr, oal_uint8 uc_mpdu_num)
{
    mac_device_stru     *pst_device;

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    mac_vap_stru       *pst_vap;
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE) && defined(_PRE_WLAN_DFT_STAT)
    dmac_tid_stats_stru *pst_tid_stats;

    pst_tid_stats = pst_tid_queue->pst_tid_stats;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tid_stats))
    {
        OAM_WARNING_LOG0(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue_head::tid_stats is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    /* 更新device结构体下的统计信息 */
    pst_device = mac_res_get_dev(pst_user->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue_head::pst_device[%d] null.}", pst_user->uc_device_id);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE) && defined(_PRE_WLAN_DFT_STAT)
        DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_enqueue_head_ptr_null_cnt, uc_mpdu_num);
#endif
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((pst_device->us_total_mpdu_num + uc_mpdu_num) > WLAN_TID_MPDU_NUM_LIMIT)
    {
        OAM_WARNING_LOG1(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue_head::WLAN_TID_MPDU_NUM_LIMIT exceed, dev_mpdu_num = %d}", pst_device->us_total_mpdu_num);
        OAM_WARNING_LOG4(pst_tid_queue->uc_vap_id, OAM_SF_TX, "{dmac_tid_tx_queue_enqueue_head::be = %d, bk = %d, vi = %d, vo = %d}",
                            pst_device->aus_ac_mpdu_num[WLAN_WME_AC_BE],
                            pst_device->aus_ac_mpdu_num[WLAN_WME_AC_BK],
                            pst_device->aus_ac_mpdu_num[WLAN_WME_AC_VI],
                            pst_device->aus_ac_mpdu_num[WLAN_WME_AC_VO]);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE) && defined(_PRE_WLAN_DFT_STAT)
        DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_enqueue_head_full_cnt, uc_mpdu_num);
#endif
#ifdef _PRE_WLAN_PERFORM_STAT
        /* 性能统计日志 */
        dmac_stat_tid_per(pst_user, pst_tid_queue->uc_tid, 0, uc_mpdu_num, DMAC_STAT_PER_SW_RETRY_OVERFLOW);
#endif
        return OAL_FAIL;
    }

    /* 全部进入重传队列 */
    dmac_tid_tx_enqueue_update(pst_device, pst_tid_queue, uc_mpdu_num);

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_dlist_join_tail(&pst_tid_queue->st_retry_q, pst_tx_dscr_list_hdr);
#else
    oal_dlist_join_head(&pst_tid_queue->st_hdr, pst_tx_dscr_list_hdr);
#endif

    if (OAL_PTR_NULL != g_st_dmac_tid_rom_cb.queue_enq_head_cb)
    {
        if (OAL_RETURN == g_st_dmac_tid_rom_cb.queue_enq_head_cb(pst_user, pst_tid_queue, uc_mpdu_num))
        {
            return OAL_SUCC;
        }
    }


#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    pst_vap = mac_res_get_mac_vap(pst_tid_queue->uc_vap_id);
    dmac_alg_flowctl_backp_notify(pst_vap, pst_device->us_total_mpdu_num, pst_device->aus_ac_mpdu_num);
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE) && defined(_PRE_WLAN_DFT_STAT)
    DMAC_TID_STATS_INCR(pst_tid_stats->ul_tid_enqueue_total_cnt, uc_mpdu_num);
#endif

    return OAL_SUCC;
}

/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_tid_get_min_max_mpdu_length);
oal_module_symbol(dmac_tid_resume);
oal_module_symbol(dmac_tid_delete_mpdu_tail);
oal_module_symbol(dmac_tid_delete_mpdu_head);
/*lint +e578*//*lint +e19*/



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

