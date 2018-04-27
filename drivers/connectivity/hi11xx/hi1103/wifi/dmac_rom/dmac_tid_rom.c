


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
#include "oal_types.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TID_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_tid_cb g_st_dmac_tid_rom_cb = {OAL_PTR_NULL, //del_mpdu_head_cb
                                    OAL_PTR_NULL, //del_mpdu_tail_cb
                                    OAL_PTR_NULL, //queue_enq_head_cb
                                    OAL_PTR_NULL, //tx_q_init_cb
                                    OAL_PTR_NULL, //tx_q_exit_cb
                                    dmac_tid_resume,
                                    dmac_tid_tx_enqueue_update};


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint32  dmac_tid_tx_queue_init(dmac_tid_stru *past_tx_tid_queue, mac_user_stru *pst_user)
{
    oal_uint8       uc_tid;
    dmac_tid_stru  *pst_tid_queue   = OAL_PTR_NULL;
    oal_uint32      ul_rst          = OAL_SUCC;

    if (OAL_UNLIKELY(OAL_PTR_NULL == past_tx_tid_queue))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tid_tx_queue_init::past_tx_tid_queue null.");

        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_tid = 0; uc_tid < WLAN_TID_MAX_NUM; uc_tid++)
    {
        pst_tid_queue = &past_tx_tid_queue[uc_tid];

        pst_tid_queue->uc_retry_num     = 0;

        pst_tid_queue->uc_tid           = uc_tid;
        pst_tid_queue->us_mpdu_num      = 0;
        pst_tid_queue->ul_mpdu_avg_len  = 0;
        pst_tid_queue->us_user_idx      = pst_user->us_assoc_id;
        pst_tid_queue->uc_vap_id        = pst_user->uc_vap_id;

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        oal_dlist_init_head(&pst_tid_queue->st_retry_q);
        oal_netbuf_list_head_init(&pst_tid_queue->st_buff_head);
#else
        oal_dlist_init_head(&pst_tid_queue->st_hdr);
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
        /* 初始化HT相关的内容 */
        pst_tid_queue->st_ht_tx_hdl.uc_ampdu_max_num    = 0;
        pst_tid_queue->st_ht_tx_hdl.us_ampdu_max_size   = 0;
        pst_tid_queue->st_ht_tx_hdl.ul_ampdu_max_size_vht   = 0;
        pst_tid_queue->en_tx_mode                       = DMAC_TX_MODE_NORMAL;

        /* 初始化RX BA相关的内容 */
        pst_tid_queue->st_ba_rx_hdl.en_ba_conn_status = DMAC_BA_INIT;
        pst_tid_queue->st_ba_rx_hdl.uc_lut_index = 0;
        /* 初始化TX BA相关的内容 */
        pst_tid_queue->pst_ba_tx_hdl = OAL_PTR_NULL;

        pst_tid_queue->en_is_delba_ing = OAL_FALSE;
        pst_tid_queue->uc_rx_wrong_ampdu_num = 0;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        /* 初始化seq_num 12位全为1 */
        pst_tid_queue->us_last_seq_frag_num   = 65535;

    #ifdef _PRE_WLAN_DFT_STAT
        pst_tid_queue->pst_tid_stats = (dmac_tid_stats_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,
                                                                            OAL_SIZEOF(dmac_tid_stats_stru),
                                                                            OAL_TRUE);
        if (OAL_PTR_NULL == pst_tid_queue->pst_tid_stats)
        {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tid_tx_queue_init::tid_stats is null.");
            ul_rst = OAL_ERR_CODE_PTR_NULL;
            break;
        }

        OAL_MEMZERO(pst_tid_queue->pst_tid_stats, OAL_SIZEOF(dmac_tid_stats_stru));
    #endif
#endif
    #ifdef _PRE_DEBUG_MODE
        pst_tid_queue->pst_tid_ampdu_stat = (dmac_tid_ampdu_stat_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,
                                                                                      OAL_SIZEOF(dmac_tid_ampdu_stat_stru),
                                                                                      OAL_TRUE);
        if (OAL_PTR_NULL == pst_tid_queue->pst_tid_ampdu_stat)
        {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tid_tx_queue_init::tid_ampdu_stat is null.");
            ul_rst = OAL_ERR_CODE_PTR_NULL;
            break;
        }

        OAL_MEMZERO(pst_tid_queue->pst_tid_ampdu_stat, OAL_SIZEOF(dmac_tid_ampdu_stat_stru));
    #endif

        OAL_MEMZERO(&(pst_tid_queue->st_rate_stats), OAL_SIZEOF(dmac_tx_normal_rate_stats_stru));
    }

    /*lint -save -e774 */
    if(OAL_SUCC != ul_rst)
    {
        for (uc_tid = 0; uc_tid < WLAN_TID_MAX_NUM; uc_tid++)
        {
             pst_tid_queue = &past_tx_tid_queue[uc_tid];

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_DFT_STAT
             if (OAL_PTR_NULL != pst_tid_queue->pst_tid_stats)
             {
                OAL_MEM_FREE(pst_tid_queue->pst_tid_stats, OAL_TRUE);
                pst_tid_queue->pst_tid_stats = OAL_PTR_NULL;
             }
#endif
#endif

#ifdef _PRE_DEBUG_MODE

             if (OAL_PTR_NULL != pst_tid_queue->pst_tid_ampdu_stat)
             {
                OAL_MEM_FREE(pst_tid_queue->pst_tid_ampdu_stat, OAL_TRUE);
                pst_tid_queue->pst_tid_ampdu_stat = OAL_PTR_NULL;
             }
#endif
        }
    }
    /*lint -restore */

    if (OAL_PTR_NULL != g_st_dmac_tid_rom_cb.tx_q_init_cb)
    {
        g_st_dmac_tid_rom_cb.tx_q_init_cb(pst_tid_queue, pst_user, &ul_rst);
    }

    return ul_rst;
}


oal_void  dmac_tid_tx_queue_exit(dmac_user_stru *pst_dmac_user)
{
    oal_uint8           uc_tid_idx;
    dmac_tid_stru      *pst_tid_queue;

    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx++)
    {
        pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[uc_tid_idx]);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    #ifdef _PRE_WLAN_DFT_STAT
        if (OAL_PTR_NULL != pst_tid_queue->pst_tid_stats)
        {
            OAL_MEM_FREE(pst_tid_queue->pst_tid_stats, OAL_TRUE);
            pst_tid_queue->pst_tid_stats = OAL_PTR_NULL;
        }
    #endif
#else
     pst_tid_queue = pst_tid_queue;
#endif

    #ifdef _PRE_DEBUG_MODE
        if (OAL_PTR_NULL != pst_tid_queue->pst_tid_ampdu_stat)
        {
            OAL_MEM_FREE(pst_tid_queue->pst_tid_ampdu_stat, OAL_TRUE);
            pst_tid_queue->pst_tid_ampdu_stat = OAL_PTR_NULL;
        }
    #endif
    }

    if (OAL_PTR_NULL != g_st_dmac_tid_rom_cb.tx_q_exit_cb)
    {
        g_st_dmac_tid_rom_cb.tx_q_exit_cb(&pst_dmac_user->st_user_base_info);
    }
}



oal_uint32  dmac_tid_get_mpdu_by_index(dmac_tid_stru *pst_tid_queue,
                                        oal_uint16 us_mpdu_index,
                                        oal_netbuf_stru **ppst_netbuf_stru)
{
    oal_uint8              uc_mpdu_idx = 0;
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    oal_netbuf_stru       *pst_netbuf_tmp;
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
    hal_tx_dscr_stru      *pst_dscr;
    oal_dlist_head_stru   *pst_dscr_entry = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tid_queue || OAL_PTR_NULL == ppst_netbuf_stru))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_tid_get_mpdu_by_index::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断us_mpdu_index是否超出当前的mpdu数量 */
    if (OAL_UNLIKELY(us_mpdu_index > pst_tid_queue->us_mpdu_num))
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_tid_get_mpdu_by_index::us_mpdu_index[%d] > us_mpdu_num[%d].}",
                         us_mpdu_index, pst_tid_queue->us_mpdu_num);

        *ppst_netbuf_stru = OAL_PTR_NULL;
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    /* 先遍历重传队列 */
    uc_mpdu_idx     = 0;
    pst_dscr_entry  = OAL_PTR_NULL;
    if (OAL_TRUE != oal_dlist_is_empty(&pst_tid_queue->st_retry_q))
    {
        pst_dscr_entry = pst_tid_queue->st_retry_q.pst_next;
        for (uc_mpdu_idx = 0; uc_mpdu_idx < us_mpdu_index; uc_mpdu_idx++)
        {
            /* 入参us_mpdu_index=0表示第一个缓存包 */
            pst_dscr_entry = pst_dscr_entry->pst_next;
            if (pst_dscr_entry == &pst_tid_queue->st_retry_q)
            {
                pst_dscr_entry = OAL_PTR_NULL;
                break;
            }
        }

    }

    /* 判断在重传队列是否找到 */
    if ((uc_mpdu_idx == us_mpdu_index) && (pst_dscr_entry != OAL_PTR_NULL))
    {
        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        *ppst_netbuf_stru = pst_dscr->pst_skb_start_addr;

        return OAL_SUCC;
    }

    /* 如果在重传队列没有找到 */
    us_mpdu_index -= uc_mpdu_idx;
    uc_mpdu_idx    = 0;
    pst_netbuf_tmp = OAL_PTR_NULL;
    /* 再遍历netbuf队列 */
    if (OAL_TRUE == oal_netbuf_list_empty(&pst_tid_queue->st_buff_head))
    {
        *ppst_netbuf_stru = OAL_PTR_NULL;

        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_tid_get_mpdu_by_index::queue empty.}");
        return OAL_FAIL;
    }

    pst_netbuf_tmp   = pst_tid_queue->st_buff_head.pst_next;
    for (uc_mpdu_idx = 0; uc_mpdu_idx < us_mpdu_index; uc_mpdu_idx++)
    {
        pst_netbuf_tmp = pst_netbuf_tmp->next;
        if (pst_netbuf_tmp == (oal_netbuf_stru *)&pst_tid_queue->st_buff_head)
        {
            pst_netbuf_tmp = OAL_PTR_NULL;
            break;
        }
    }

    /* 在netbuf队列找到 */
    if (pst_netbuf_tmp != OAL_PTR_NULL)
    {
        *ppst_netbuf_stru = pst_netbuf_tmp;
        return OAL_SUCC;
    }

    /* 没有找到 */
    *ppst_netbuf_stru = OAL_PTR_NULL;
    return OAL_FAIL;
#else
    /* 如果tid缓存队列为空，程序直接退出 */
    if (OAL_TRUE == oal_dlist_is_empty(&pst_tid_queue->st_hdr))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_tid_get_mpdu_by_index::queue empty.}");
        *ppst_netbuf_stru = OAL_PTR_NULL;
        return OAL_FAIL;
    }

    pst_dscr_entry = pst_tid_queue->st_hdr.pst_next;
    for (uc_mpdu_idx = 0; uc_mpdu_idx < us_mpdu_index; uc_mpdu_idx++)
    {
        /* 获取CB */
        pst_dscr_entry = pst_dscr_entry->pst_next;
    }
    pst_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
    *ppst_netbuf_stru = pst_dscr->pst_skb_start_addr;

    return OAL_SUCC;
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */
}


oal_uint32  dmac_tid_get_normal_rate_stats(
                OAL_CONST mac_user_stru * OAL_CONST pst_mac_user,
                oal_uint8                           uc_tid_id,
                dmac_tx_normal_rate_stats_stru    **ppst_rate_stats_info)
{
    dmac_tid_stru      *pst_tid_queue = OAL_PTR_NULL;

    if ((OAL_PTR_NULL == pst_mac_user) || (OAL_PTR_NULL == ppst_rate_stats_info))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_tid_get_normal_rate_stats::param null, pst_mac_user=%d, ppst_rate_stats_info=%d.}",
                       pst_mac_user, ppst_rate_stats_info);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_tid_id >= WLAN_TID_MAX_NUM)
    {
        OAM_ERROR_LOG1(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{dmac_tid_get_normal_rate_stats::invalid uc_tid_id[%d].", uc_tid_id);

        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    dmac_user_get_tid_by_num(pst_mac_user, uc_tid_id, &pst_tid_queue);

    *ppst_rate_stats_info = &pst_tid_queue->st_rate_stats;

    return OAL_SUCC;
}


oal_uint32  dmac_tid_set_normal_rate_stats(
                mac_user_stru                      *pst_mac_user,
                oal_uint8                           uc_tid_id,
                dmac_tx_normal_rate_stats_stru     *pst_rate_stats_info)
{
    dmac_tid_stru      *pst_tid_queue = OAL_PTR_NULL;
    dmac_tx_normal_rate_stats_stru *pst_rate_stats;

    if ((OAL_PTR_NULL == pst_mac_user)
     || (OAL_PTR_NULL == pst_rate_stats_info))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_tid_set_normal_rate_stats::param null, pst_mac_user=%d, pst_rate_stats_info=%d.}",
                       pst_mac_user, pst_rate_stats_info);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_tid_id >= WLAN_TID_MAX_NUM)
    {
        OAM_ERROR_LOG1(pst_mac_user->uc_vap_id, OAM_SF_ANY, "{dmac_tid_set_normal_rate_stats::invalid uc_tid_id[%d].", uc_tid_id);

        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    dmac_user_get_tid_by_num(pst_mac_user, uc_tid_id, &pst_tid_queue);
    pst_rate_stats = &pst_tid_queue->st_rate_stats;

	pst_rate_stats->ul_best_rate_goodput_kbps = pst_rate_stats_info->ul_best_rate_goodput_kbps;
    pst_rate_stats->ul_rate_kbps        = pst_rate_stats_info->ul_rate_kbps;
    pst_rate_stats->uc_aggr_subfrm_size = pst_rate_stats_info->uc_aggr_subfrm_size;
    pst_rate_stats->uc_per              = pst_rate_stats_info->uc_per;

    return OAL_SUCC;
}
#if 0

oal_void  dmac_tid_flush_retry_frame(mac_device_stru *pst_device, dmac_tid_stru *pst_tid)
{
    oal_uint8            uc_dscr_index;
    hal_tx_dscr_stru    *pst_dscr;
    oal_dlist_head_stru *pst_dscr_entry;
    oal_netbuf_stru     *pst_netbuf = OAL_PTR_NULL;


    //维测，计算下tid队列的长度，和retry_num做比较
    oal_dlist_head_stru *pst_dlist_pos;
    oal_uint16           us_num = 0 ;

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    mac_vap_stru        *pst_vap    = OAL_PTR_NULL;
#endif
    OAL_DLIST_SEARCH_FOR_EACH(pst_dlist_pos,&pst_tid->st_hdr)
    {
        us_num++;
    }

    for (uc_dscr_index = 0; uc_dscr_index < pst_tid->uc_retry_num; uc_dscr_index++)
    {
        if (OAL_TRUE == oal_dlist_is_empty(&pst_tid->st_hdr))
        {
            OAM_ERROR_LOG3(0, OAM_SF_ANY, "{dmac_tid_flush_retry_frame::retry num wrong, dscr in tid queue = %d,uc_dscr_index = %d,uc_retry_num = %d.}",us_num,uc_dscr_index,pst_tid->uc_retry_num);
            #if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            OAL_IO_PRINT(KERN_CRIT"retry num wrong, dscr in tid queue = %d,uc_dscr_index = %d,uc_retry_num = %d\n",us_num,uc_dscr_index,pst_tid->uc_retry_num);
            OAL_BUG_ON(1);
            #endif
            break;
        }

        pst_dscr_entry = oal_dlist_delete_head(&pst_tid->st_hdr);
        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);

        pst_netbuf = pst_dscr->pst_skb_start_addr;
        OAL_MEM_FREE(pst_dscr, OAL_TRUE);
        dmac_tx_excp_free_netbuf(pst_netbuf);

        pst_tid->us_mpdu_num--;
        pst_device->us_total_mpdu_num--;
        pst_device->aus_vap_mpdu_num[pst_tid->uc_vap_id]--;
        pst_device->aus_ac_mpdu_num[WLAN_WME_TID_TO_AC(pst_tid->uc_tid)]--;
    }
#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    pst_vap = mac_res_get_mac_vap(pst_tid->uc_vap_id);
    dmac_alg_flowctl_backp_notify(pst_vap, pst_device->us_total_mpdu_num, pst_device->aus_ac_mpdu_num);
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_DFT_STAT
    if (OAL_PTR_NULL != pst_tid->pst_tid_stats)
    {
        DMAC_TID_STATS_INCR(pst_tid->pst_tid_stats->ul_tid_retry_dequeue_cnt, pst_tid->uc_retry_num);
    }
#endif
#endif
    pst_tid->uc_retry_num = 0;
}
#endif



oal_uint32  dmac_tid_pause(dmac_tid_stru *pst_tid, oal_uint8 uc_type)
{
    oal_uint8   uc_is_paused;

    if (OAL_PTR_NULL == pst_tid)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_is_paused = pst_tid->uc_is_paused;

    if (DMAC_TID_PAUSE_RESUME_TYPE_BA == uc_type)
    {
        pst_tid->uc_is_paused |= BIT0;
    }
    else if (DMAC_TID_PAUSE_RESUME_TYPE_PS == uc_type)
    {
        pst_tid->uc_is_paused |= BIT1;
    }

    if (0 == uc_is_paused)
    {
        /* 通知算法 */
        dmac_alg_tid_update_notify(pst_tid);
    }

#ifdef _PRE_WLAN_DFT_EVENT
    dmac_tid_status_change_event_to_sdt(pst_tid, pst_tid->uc_is_paused);
#endif

    return OAL_SUCC;
}


/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_tid_get_util_ratio);
oal_module_symbol(dmac_tid_get_mpdu_by_index);
oal_module_symbol(dmac_tid_get_normal_rate_stats);
oal_module_symbol(dmac_tid_set_normal_rate_stats);
oal_module_symbol(dmac_tid_tx_queue_init);
oal_module_symbol(dmac_tid_pause);
/*lint +e578*//*lint +e19*/



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


