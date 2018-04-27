


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_PERFORM_STAT

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "mac_resource.h"
#include "dmac_alg_if.h"
#include "dmac_stat.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_STAT_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
OAL_STATIC dmac_stat_stru      g_ast_pfm_stat[MAC_STAT_TYPE_BUTT];
OAL_STATIC oal_uint32          g_ul_stat_node_limit[MAC_STAT_TYPE_BUTT];

/*****************************************************************************
  3 静态函数声明
*****************************************************************************/

OAL_STATIC dmac_stat_node_stru* dmac_stat_search_node(oal_dlist_head_stru        *pst_node_dlist_head,
                                                      mac_stat_module_enum_uint16 en_module_id,
                                                      oal_void                   *p_void);
OAL_STATIC oal_uint32  dmac_stat_format_title_string(oal_int8                  *pac_output_data,
                                                      oal_uint16                 ul_data_len,
                                                      dmac_stat_node_stru       *pst_stat_node);
OAL_STATIC oal_uint32  dmac_stat_format_data_string(oal_int8                  *pac_output_data,
                                                     oal_uint16                 ul_data_len,
                                                     mac_stat_type_enum_uint8   en_stat_type,
                                                     oal_uint32                 ul_index,
                                                     oal_uint32                *pul_data);
OAL_STATIC oal_uint32  dmac_stat_print(dmac_stat_node_stru     *pst_stat_node, oam_output_type_enum_uint8 en_output_type);
OAL_STATIC oal_uint32  dmac_stat_timer_handler(oal_void * p_void);
OAL_STATIC oal_uint32  dmac_stat_unregister_node(dmac_stat_node_stru   *pst_stat_node);
OAL_STATIC oal_uint32  dmac_stat_get_rx_tid(mac_vap_stru *pst_vap, dmac_rx_ctl_stru *pst_rx_ctl, oal_uint8 *puc_tidno);
OAL_STATIC oal_uint32  dmac_stat_update_thrpt(mac_vap_stru *pst_vap,
                                  mac_user_stru *pst_user,
                                  dmac_tid_stru *pst_tid,
                                  oal_uint32     ul_stat_bytes,
                                  dmac_stat_direct_enum_uint8 en_stat_direct);

#ifdef _PRE_WLAN_11K_STAT
OAL_STATIC oal_uint32 dmac_stat_get_node_info(dmac_stat_node_stru *pst_stat_node, mac_stat_type_enum_uint8 en_stat_type);
OAL_STATIC oal_uint32 dmac_stat_query_timer_handler(oal_void * p_void);
OAL_STATIC oal_void dmac_stat_calc_rcpi(dmac_stat_frm_rpt_stru *pst_stat_frm_rpt, hal_rx_statistic_stru *pst_rx_statistic);
OAL_STATIC oal_void dmac_stat_calc_rx_mpdu_num(dmac_stat_count_stru *pst_stat_count, dmac_rx_ctl_stru *pst_rx_ctrl);
OAL_STATIC oal_void dmac_stat_calc_rx_mpdu_tid_num(dmac_stat_count_tid_stru *pst_stat_count_tid, dmac_rx_ctl_stru *pst_rx_ctrl);

OAL_STATIC oal_void dmac_stat_calc_tx_delay(dmac_stat_tid_tx_delay_stru *pst_stat_tid_delay, oal_uint16 us_delay_ms);
OAL_STATIC oal_void dmac_stat_calc_tx_delay_hist(dmac_stat_tid_tx_delay_hist_stru *pst_stat_tid_delay_hist,
                        oal_uint16 us_delay_ms, oal_uint8 uc_tidno);
OAL_STATIC oal_void dmac_stat_calc_tx_mpdu_num( dmac_stat_count_stru *pst_stat_count, dmac_stat_count_common_stru *pst_stat_info, mac_tx_ctl_stru* pst_cb);
OAL_STATIC oal_void dmac_stat_calc_tx_mpdu_tid_num( dmac_stat_count_tid_stru *pst_stat_count_tid, dmac_stat_count_common_stru *pst_stat_info, oal_uint32 ul_sub_msdu_num);
OAL_STATIC oal_void dmac_stat_calc_tx_frm_num(dmac_stat_count_stru *pst_stat_count, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param, oal_uint8 bit_is_ampdu);
OAL_STATIC oal_void dmac_stat_calc_tx_mpdu_tid_bytes( dmac_stat_count_tid_stru *pst_stat_count_tid, dmac_stat_count_common_stru *pst_stat_info, oal_uint16 ul_len);

#endif
/*****************************************************************************
  4 函数实现
*****************************************************************************/

oal_uint32  dmac_stat_init(oal_void)
{
    oal_uint8   uc_loop = 0;

    /* 初始化所有性能统计节点链表 */
    for (uc_loop = 0; uc_loop < MAC_STAT_TYPE_BUTT; uc_loop++)
    {
        g_ast_pfm_stat[uc_loop].ul_node_num = 0;
        oal_dlist_init_head(&(g_ast_pfm_stat[uc_loop].st_stat_node_dlist));
    }

    /* 初始化各统计类型的统计节点数量限制 */
    g_ul_stat_node_limit[MAC_STAT_TYPE_TID_DELAY]   = DMAC_STAT_TID_DELAY_NODE_LIMIT;
    g_ul_stat_node_limit[MAC_STAT_TYPE_TID_PER]     = DMAC_STAT_TID_PER_NODE_LIMIT;
    g_ul_stat_node_limit[MAC_STAT_TYPE_TID_THRPT]   = DMAC_STAT_TID_THRPT_NODE_LIMIT;
    g_ul_stat_node_limit[MAC_STAT_TYPE_USER_THRPT]  = DMAC_STAT_USER_THRPT_NODE_LIMIT;
    g_ul_stat_node_limit[MAC_STAT_TYPE_VAP_THRPT]   = DMAC_STAT_VAP_THRPT_NODE_LIMIT;
    g_ul_stat_node_limit[MAC_STAT_TYPE_USER_BSD]    = DMAC_STAT_USER_THRPT_NODE_LIMIT;

    /* 注册钩子函数 */

    return OAL_SUCC;
}


oal_uint32  dmac_stat_exit(oal_void)
{
    mac_stat_type_enum_uint8     uc_stat_type       = 0;
    dmac_stat_stru              *pst_stat           = OAL_PTR_NULL;
    oal_dlist_head_stru         *pst_dlist_pos      = OAL_PTR_NULL;
    oal_dlist_head_stru         *pst_dlist_tmp      = OAL_PTR_NULL;
    dmac_stat_node_stru         *pst_stat_node      = OAL_PTR_NULL;
    oal_uint32                   ul_ret             = OAL_SUCC;

    for (uc_stat_type = 0; uc_stat_type < MAC_STAT_TYPE_BUTT; uc_stat_type++)
    {
        pst_stat = &(g_ast_pfm_stat[uc_stat_type]);

        /* 遍列该统计类型的所有统计节点 */
        OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_dlist_pos, pst_dlist_tmp, &(pst_stat->st_stat_node_dlist))
        {
            pst_stat_node = OAL_DLIST_GET_ENTRY(pst_dlist_pos, dmac_stat_node_stru, st_entry);

            /* 注销统计节点  */
            ul_ret = dmac_stat_unregister_node(pst_stat_node);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_scan_ch_statics_complete::pst_mac_device null.}");
                return ul_ret;
            }
        }
    }
    return OAL_SUCC;
}


oal_uint32    dmac_stat_register( mac_stat_module_enum_uint16     en_module_id,
                                mac_stat_type_enum_uint8        en_stat_type,
                                oal_void                       *p_void,
                                dmac_stat_param_stru           *pst_output_param,
                                dmac_stat_timeout_func          p_func,
                                oal_uint32                        ul_core_id)
{
    dmac_stat_node_stru     *pst_stat_node      = OAL_PTR_NULL;
    oal_uint8                uc_index           = 0;
    dmac_stat_stru          *pst_stat           = &(g_ast_pfm_stat[en_stat_type]);


#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    if ((OAL_PTR_NULL == p_void)
        || ((OAL_PTR_NULL == pst_output_param) && (MAC_STAT_MODULE_CMD != en_module_id && MAC_STAT_MODULE_QOS != en_module_id))
        || ((OAL_PTR_NULL == p_func) && (MAC_STAT_MODULE_CMD != en_module_id && MAC_STAT_MODULE_QOS != en_module_id)))
#else
    if ((OAL_PTR_NULL == p_void)
        || ((OAL_PTR_NULL == pst_output_param) && (MAC_STAT_MODULE_CMD != en_module_id))
        || ((OAL_PTR_NULL == p_func) && (MAC_STAT_MODULE_CMD != en_module_id)))
#endif
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_register::invalid param.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断统计节点是否超过限制 */
    if (pst_stat->ul_node_num >= g_ul_stat_node_limit[en_stat_type])
    {
        return OAL_SUCC;
    }

    /* 检查统计节点是否已存在 */
    pst_stat_node = dmac_stat_search_node(&(pst_stat->st_stat_node_dlist), en_module_id, p_void);
    if (OAL_PTR_NULL != pst_stat_node)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_stat_register::the stat_node has been existed.}");
        return OAL_FAIL;
    }

    /* 申请统计节点空间 */
    pst_stat_node = (dmac_stat_node_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_node_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_register::pst_stat_node null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    OAL_MEMZERO(pst_stat_node, sizeof(dmac_stat_node_stru));

    /* 初始化stat_node的参数 */
    pst_stat_node->p_inner_func                     = p_func;
    pst_stat_node->uc_stat_flag                     = OAL_FALSE;
    pst_stat_node->us_total_item                    = DMAC_STAT_ITEM_LIMIT;
    pst_stat_node->us_curr_item                     = 0;

    for (uc_index = 0; uc_index < DMAC_STAT_PER_BUTT; uc_index++)
    {
        pst_stat_node->aul_stat_cnt[uc_index]   = 0;
        pst_stat_node->aul_stat_sum[uc_index]   = 0;
    }

    /* 为MAC_STAT_MODULE_CMD模块申请存储空间 */
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    if ( MAC_STAT_MODULE_CMD == en_module_id || MAC_STAT_MODULE_QOS == en_module_id)
#else
    if ( MAC_STAT_MODULE_CMD == en_module_id)
#endif
    {
        pst_stat_node->pst_stat_param = (dmac_stat_param_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_param_stru), OAL_TRUE);
        if (OAL_PTR_NULL == pst_stat_node->pst_stat_param)
        {
            /* 释放空间 */
            OAL_MEM_FREE(pst_stat_node, OAL_TRUE);
            pst_stat_node = OAL_PTR_NULL;

            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        if(MAC_STAT_MODULE_CMD == en_module_id)
        {
            if ((MAC_STAT_TYPE_TID_THRPT == en_stat_type)
                ||(MAC_STAT_TYPE_USER_THRPT == en_stat_type)
                ||(MAC_STAT_TYPE_VAP_THRPT == en_stat_type)
                ||(MAC_STAT_TYPE_USER_BSD == en_stat_type))
            {
                pst_stat_node->pul_stat_avg = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,
                                                        (DMAC_STAT_BUTT) * (DMAC_STAT_ITEM_LIMIT) * OAL_SIZEOF(oal_uint32), OAL_TRUE);
            }
            else
            {
                pst_stat_node->pul_stat_avg = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,
                                                            (DMAC_STAT_ITEM_LIMIT) * OAL_SIZEOF(oal_uint32), OAL_TRUE);
            }
            if (OAL_PTR_NULL == pst_stat_node->pul_stat_avg)
            {
                /* 释放空间 */
                OAL_MEM_FREE(pst_stat_node->pst_stat_param, OAL_TRUE);
                pst_stat_node->pst_stat_param = OAL_PTR_NULL;
                OAL_MEM_FREE(pst_stat_node, OAL_TRUE);
                pst_stat_node = OAL_PTR_NULL;

                return OAL_ERR_CODE_ALLOC_MEM_FAIL;
            }
        }
    }
    else
    {
        pst_stat_node->pst_stat_param = pst_output_param;
    }

    pst_stat_node->pst_stat_param->en_module_id       = en_module_id;
    pst_stat_node->pst_stat_param->en_stat_type       = en_stat_type;
    pst_stat_node->pst_stat_param->p_void             = p_void;

    /* 定时器参数赋值, 统计周期默认值设为100ms */
    pst_stat_node->st_timer.ul_timeout      = DMAC_STAT_TIMER_CYCLE_MS;
    pst_stat_node->st_timer.en_module_id    = en_module_id;

    /* 注册定时器 */
#ifdef _PRE_WLAN_11K_STAT
    if (MAC_STAT_TYPE_USER_BSD == en_stat_type)
    {
        FRW_TIMER_CREATE_TIMER(&(pst_stat_node->st_timer),
                                dmac_stat_query_timer_handler ,
                                pst_stat_node->st_timer.ul_timeout,
                                (oal_void *)(pst_stat_node->pst_stat_param),
                                OAL_TRUE,
                                OAM_MODULE_ID_DMAC,
                                ul_core_id);
    }
    else
#endif
    {
        FRW_TIMER_CREATE_TIMER(&(pst_stat_node->st_timer),
                                dmac_stat_timer_handler,
                                pst_stat_node->st_timer.ul_timeout,
                                (oal_void *)(pst_stat_node->pst_stat_param),
                                OAL_TRUE,
                                OAM_MODULE_ID_DMAC,
                                ul_core_id);
    }
    /* 将统计节点插入相应的链表 */
    oal_dlist_add_tail(&(pst_stat_node->st_entry), &(pst_stat->st_stat_node_dlist));
    pst_stat->ul_node_num++;

    return OAL_SUCC;
}


oal_uint32    dmac_stat_unregister(mac_stat_module_enum_uint16    en_module_id,
                                 mac_stat_type_enum_uint8       en_stat_type,
                                 oal_void                      *p_void)
{
    oal_uint32               ul_ret             = OAL_SUCC;
    dmac_stat_node_stru     *pst_stat_node      = OAL_PTR_NULL;
    dmac_stat_stru          *pst_stat           = &(g_ast_pfm_stat[en_stat_type]);

    if (OAL_PTR_NULL == p_void)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_unregister::p_void null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 查找统计节点 */
    pst_stat_node = dmac_stat_search_node(&(pst_stat->st_stat_node_dlist), en_module_id, p_void);
    if (OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_unregister::pst_stat_node null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 注销节点，包括注销定时器 */
    ul_ret = dmac_stat_unregister_node(pst_stat_node);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_stat_unregister::dmac_stat_unregister_node failed[%d].}", ul_ret);

        return ul_ret;
    }

    /*更新统计节点数量 */
    pst_stat->ul_node_num--;

    return OAL_SUCC;
}



oal_uint32    dmac_stat_start( mac_stat_module_enum_uint16    en_module_id,
                             mac_stat_type_enum_uint8       en_stat_type,
                             oal_uint16                     us_stat_period,
                             oal_uint16                     us_stat_num,
                             oal_void                      *p_void)
{
    dmac_stat_node_stru     *pst_stat_node      = OAL_PTR_NULL;
    dmac_stat_stru          *pst_stat           = &(g_ast_pfm_stat[en_stat_type]);
    oal_uint8                uc_cnt             = 0;

    if (OAL_PTR_NULL == p_void)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_start::p_void null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 查找相应的统计节点 */
    pst_stat_node = dmac_stat_search_node(&(pst_stat->st_stat_node_dlist), en_module_id, p_void);
    if (OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_start::pst_stat_node null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 初始化相关统计量 */
    pst_stat_node->uc_stat_flag                 = OAL_TRUE;
    pst_stat_node->us_total_item                = OAL_MIN(us_stat_num, DMAC_STAT_ITEM_LIMIT);
    pst_stat_node->us_curr_item                 = 0;

    for (uc_cnt = 0; uc_cnt < DMAC_STAT_PER_BUTT; uc_cnt++)
    {
        pst_stat_node->aul_stat_cnt[uc_cnt]   = 0;
        pst_stat_node->aul_stat_sum[uc_cnt]   = 0;
    }

#ifdef _PRE_WLAN_11K_STAT
    dmac_stat_get_node_info(pst_stat_node, en_stat_type);
#endif

    /* 给MAC_STAT_MODULE_CMD模块初始化内存，以便重新统计 */
    if ( MAC_STAT_MODULE_CMD == en_module_id)
    {
        if ((MAC_STAT_TYPE_TID_THRPT == en_stat_type)
            ||(MAC_STAT_TYPE_USER_THRPT == en_stat_type)
            ||(MAC_STAT_TYPE_VAP_THRPT == en_stat_type)
            ||(MAC_STAT_TYPE_USER_BSD == en_stat_type))
        {
            OAL_MEMZERO(pst_stat_node->pul_stat_avg, ((DMAC_STAT_BUTT) * (DMAC_STAT_ITEM_LIMIT) * OAL_SIZEOF(oal_uint32)));
        }
        else
        {
            OAL_MEMZERO(pst_stat_node->pul_stat_avg, ((DMAC_STAT_ITEM_LIMIT) * OAL_SIZEOF(oal_uint32)));
        }
    }

    pst_stat_node->st_timer.ul_timeout  = us_stat_period;

    frw_timer_restart_timer(&(pst_stat_node->st_timer),
                              pst_stat_node->st_timer.ul_timeout,
                              OAL_TRUE);

    return OAL_SUCC;
}


oal_uint32    dmac_stat_stop(mac_stat_module_enum_uint16    en_module_id,
                             mac_stat_type_enum_uint8     en_stat_type,
                             oal_void                    *p_void)
{
    dmac_stat_node_stru     *pst_stat_node      = OAL_PTR_NULL;
    dmac_stat_stru          *pst_stat           = &(g_ast_pfm_stat[en_stat_type]);

    if (OAL_PTR_NULL == p_void)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_stop::p_void null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_stat_node = dmac_stat_search_node(&(pst_stat->st_stat_node_dlist), en_module_id, p_void);
    if (OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_stop::pst_stat_node null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    frw_timer_stop_timer(&(pst_stat_node->st_timer));

    /* 该节点停止统计 */
    pst_stat_node->uc_stat_flag = OAL_FALSE;

    return OAL_SUCC;
}


oal_uint32  dmac_stat_tid_delay(dmac_tid_stru *pst_dmac_tid)
{
    dmac_stat_stru          *pst_stat           = &(g_ast_pfm_stat[MAC_STAT_TYPE_TID_DELAY]);
    oal_dlist_head_stru     *pst_dlist_pos      = OAL_PTR_NULL;
    dmac_stat_node_stru     *pst_stat_node      = OAL_PTR_NULL;
    oal_netbuf_stru         *pst_netbuf         = OAL_PTR_NULL;
    oal_uint32               ul_diff_time_us    = 0;
    oal_uint16               us_delay_ms        = 0;
    mac_tx_ctl_stru         *pst_tx_ctl         = OAL_PTR_NULL;
    hal_tx_dscr_stru        *pst_dscr           = OAL_PTR_NULL;
    oal_dlist_head_stru     *pst_dscr_entry     = OAL_PTR_NULL;
    oal_time_us_stru         st_time;

    if (OAL_PTR_NULL == pst_dmac_tid)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_tid_delay::pst_dmac_tid null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
    if (OAL_FALSE == oal_dlist_is_empty(&pst_dmac_tid->st_retry_q))
    {
        pst_dscr_entry = pst_dmac_tid->st_retry_q.pst_next;

        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        pst_netbuf = pst_dscr->pst_skb_start_addr;
    }
    else if (OAL_FALSE == oal_netbuf_list_empty(&pst_dmac_tid->st_buff_head))
    {
        pst_netbuf = pst_dmac_tid->st_buff_head.pst_next;
    }
    else
    {
        return OAL_FAIL;
    }
#else
    if (oal_dlist_is_empty(&pst_dmac_tid->st_hdr))
    {
        return OAL_FAIL;
    }

    pst_dscr_entry = pst_dmac_tid->st_hdr.pst_next;

    pst_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
    pst_netbuf = pst_dscr->pst_skb_start_addr;
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

    pst_tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);

    /* 获取系统时间 */
    oal_time_get_stamp_us(&st_time);

    /* 计算报文的等待时延 */
    ul_diff_time_us = (oal_uint32)OAL_TIME_GET_RUNTIME(MAC_GET_CB_TIMESTAMP(pst_tx_ctl), (oal_uint32)DMAC_TIME_USEC_INT64(&st_time));
    us_delay_ms = (oal_uint16)(ul_diff_time_us >> 10);

    /* 遍列该统计类型的所有统计节点 */
    OAL_DLIST_SEARCH_FOR_EACH(pst_dlist_pos, &(pst_stat->st_stat_node_dlist))
    {
        pst_stat_node = OAL_DLIST_GET_ENTRY(pst_dlist_pos, dmac_stat_node_stru, st_entry);

        if (((oal_void *)pst_dmac_tid == pst_stat_node->pst_stat_param->p_void)
            && (OAL_TRUE == pst_stat_node->uc_stat_flag))
        {
            pst_stat_node->aul_stat_cnt[DMAC_STAT_TX]++;
            pst_stat_node->aul_stat_sum[DMAC_STAT_TX] += us_delay_ms;

            return OAL_SUCC;
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_stat_tid_per(mac_user_stru *pst_user,
                             oal_uint8 uc_tidno,
                             oal_uint16 us_mpdu_num,
                             oal_uint16 us_err_mpdu_num,
                             dmac_stat_per_reason_enum_uint8 en_per_reason)
{
    dmac_stat_stru          *pst_stat           = &(g_ast_pfm_stat[MAC_STAT_TYPE_TID_PER]);
    oal_dlist_head_stru     *pst_dlist_pos      = OAL_PTR_NULL;
    dmac_stat_node_stru     *pst_stat_node      = OAL_PTR_NULL;
    dmac_tid_stru           *pst_tid            = OAL_PTR_NULL;

    if (OAL_PTR_NULL == pst_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_tid_per::pst_dmac_user null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    dmac_user_get_tid_by_num(pst_user, uc_tidno, &pst_tid);

    /* video报文丢弃，增加日志打印 */
    if ((WLAN_TIDNO_VIDEO == pst_tid->uc_tid)
            && (en_per_reason != DMAC_STAT_PER_MAC_TOTAL)
            && (0 != us_err_mpdu_num))
    {
        switch(en_per_reason)
        {
            case DMAC_STAT_PER_BUFF_OVERFLOW:
                OAM_INFO_LOG2(0, OAM_SF_ANY, "{video drop!!!, reason = %d(buff_overflow), drop_num = %d}", en_per_reason, us_err_mpdu_num);
                break;
            case DMAC_STAT_PER_BUFF_BE_SEIZED:
                OAM_INFO_LOG2(0, OAM_SF_ANY, "{video drop!!!, reason = %d(vi_be_seized), drop_num = %d}", en_per_reason, us_err_mpdu_num);
                break;
            case DMAC_STAT_PER_DELAY_OVERTIME:
                OAM_INFO_LOG2(0, OAM_SF_ANY, "{video drop!!!, reason = %d(vi_overtime), drop_num = %d}", en_per_reason, us_err_mpdu_num);
                break;
            case DMAC_STAT_PER_SW_RETRY_AMPDU:
                OAM_INFO_LOG2(0, OAM_SF_ANY, "{video pkt drop!!!, reason = %d(whole ampdu failed), drop_num = %d}", en_per_reason, us_err_mpdu_num);
                break;
            case DMAC_STAT_PER_SW_RETRY_SUB_AMPDU:
                OAM_INFO_LOG2(0, OAM_SF_ANY, "{video pkt drop!!!, reason = %d(ampdu's sub_mpdu failed), drop_num = %d}", en_per_reason, us_err_mpdu_num);
                break;
            case DMAC_STAT_PER_SW_RETRY_MPDU:
                OAM_INFO_LOG2(0, OAM_SF_ANY, "{video pkt drop!!!, reason = %d(non_ampdu mpdu failed), drop_num = %d}", en_per_reason, us_err_mpdu_num);
                break;
            case DMAC_STAT_PER_SW_RETRY_OVERFLOW:
                OAM_INFO_LOG2(0, OAM_SF_ANY, "{video pkt drop!!!, reason = %d(retry mpdu re_insert failed), drop_num = %d}", en_per_reason, us_err_mpdu_num);
                break;
            case DMAC_STAT_PER_RTS_FAIL:
                OAM_INFO_LOG2(0, OAM_SF_ANY, "{video pkt drop!!!, reason = %d(rts_fail), drop_num = %d}", en_per_reason, us_err_mpdu_num);
                break;
            case DMAC_STAT_PER_HW_SW_FAIL:
                OAM_INFO_LOG2(0, OAM_SF_ANY, "{video pkt drop!!!, reason = %d(hw_sw_fail), drop_num = %d}", en_per_reason, us_err_mpdu_num);
                break;
            default:
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_stat_tid_per::unknown stat_per_reaion type.}");
        }
    }

    /* 遍列该统计类型的所有统计节点 */
    OAL_DLIST_SEARCH_FOR_EACH(pst_dlist_pos, &(pst_stat->st_stat_node_dlist))
    {
        pst_stat_node = OAL_DLIST_GET_ENTRY(pst_dlist_pos, dmac_stat_node_stru, st_entry);

        if ((pst_tid == pst_stat_node->pst_stat_param->p_void)
            && (OAL_TRUE == pst_stat_node->uc_stat_flag))
        {
            switch (en_per_reason)
            {
                case DMAC_STAT_PER_MAC_TOTAL:
                case DMAC_STAT_PER_BUFF_OVERFLOW:
                case DMAC_STAT_PER_RTS_FAIL:
                case DMAC_STAT_PER_HW_SW_FAIL:
                    pst_stat_node->aul_stat_cnt[en_per_reason] += us_mpdu_num;
                    pst_stat_node->aul_stat_sum[en_per_reason] += us_err_mpdu_num;
                    break;

                case DMAC_STAT_PER_BUFF_BE_SEIZED:
                case DMAC_STAT_PER_DELAY_OVERTIME:
                case DMAC_STAT_PER_SW_RETRY_AMPDU:
                case DMAC_STAT_PER_SW_RETRY_SUB_AMPDU:
                case DMAC_STAT_PER_SW_RETRY_MPDU:
                case DMAC_STAT_PER_SW_RETRY_OVERFLOW:
                    pst_stat_node->aul_stat_sum[en_per_reason] += us_err_mpdu_num;
                    break;

                default:
                    OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_tid_per::wrong stat_per_reason type.}");
                    return OAL_FAIL;
            }
        }
    }

    return OAL_SUCC;
}



oal_uint32 dmac_stat_tx_thrpt(dmac_user_stru *pst_dmac_user, oal_uint8 uc_tidno, hal_tx_dscr_ctrl_one_param st_tx_dscr_param)
{
    dmac_tid_stru               *pst_tid            = OAL_PTR_NULL;
    mac_vap_stru                *pst_vap            = OAL_PTR_NULL;
    mac_user_stru               *pst_user           = OAL_PTR_NULL;
    oal_uint32                   ul_ret             = 0;
    oal_uint32                   ul_stat_bytes      = 0;

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_tx_thrpt::pst_dmac_user null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_tid = &(pst_dmac_user->ast_tx_tid_queue[uc_tidno]);

    pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_tid->uc_vap_id);

    pst_user = (mac_user_stru *)(&(pst_dmac_user->st_user_base_info));

    ul_stat_bytes = st_tx_dscr_param.us_mpdu_len
                        * (st_tx_dscr_param.uc_mpdu_num - st_tx_dscr_param.uc_error_mpdu_num);

    /* 更新统计量 */
    ul_ret = dmac_stat_update_thrpt(pst_vap, pst_user, pst_tid,
                                            ul_stat_bytes, DMAC_STAT_TX);

    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_RX, "{dmac_stat_tx_thrpt::dmac_stat_update_thrpt failed[%d].}", ul_ret);

        return ul_ret;

    }

    return OAL_SUCC;
}


oal_uint32 dmac_stat_rx_thrpt(frw_event_hdr_stru *pst_event_hdr, mac_vap_stru *pst_vap, dmac_rx_ctl_stru *pst_rx_ctl)
{
    dmac_user_stru             *pst_dmac_user   = OAL_PTR_NULL;
    mac_user_stru               *pst_mac_user    = OAL_PTR_NULL;
    dmac_tid_stru              *pst_tid         = OAL_PTR_NULL;
    oal_uint32                  ul_ret          = OAL_SUCC;
    oal_uint8                   uc_tidno;

    if ((OAL_PTR_NULL == pst_event_hdr) || (OAL_PTR_NULL == pst_vap) || (OAL_PTR_NULL == pst_rx_ctl))
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{dmac_stat_rx_thrpt::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 管理帧不需要统计 */
    if (FRW_EVENT_TYPE_WLAN_DRX != pst_event_hdr->en_type)
    {
        return OAL_SUCC;
    }

    /* 获取用户、帧信息 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_RX_CB_TA_USER_IDX(&pst_rx_ctl->st_rx_info));
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_RX,
            "{dmac_stat_rx_thrpt::pst_dmac_user[%d] null.}", MAC_GET_RX_CB_TA_USER_IDX(&pst_rx_ctl->st_rx_info));
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 广播用户不需要统计 */
    pst_mac_user = (mac_user_stru *)(&(pst_dmac_user->st_user_base_info));
    if (OAL_TRUE == pst_mac_user->en_is_multi_user)
    {
        return OAL_SUCC;
    }

    /* 获取接收报文的tidno */
    ul_ret = dmac_stat_get_rx_tid(pst_vap, pst_rx_ctl, &uc_tidno);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac_stat_rx_thrpt::dmac_stat_get_rx_tid failed[%d].}", ul_ret);

        return ul_ret;
    }

    pst_tid = &(pst_dmac_user->ast_tx_tid_queue[uc_tidno]);

    /* 更新统计量 */
    ul_ret = dmac_stat_update_thrpt(pst_vap, pst_mac_user, pst_tid,
                                (oal_uint32)pst_rx_ctl->st_rx_info.us_frame_len, DMAC_STAT_RX);

    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac_stat_rx_thrpt::dmac_stat_update_thrpt failed[%d].}", ul_ret);

        return ul_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_stat_update_thrpt(mac_vap_stru *pst_vap,
                                              mac_user_stru *pst_user,
                                              dmac_tid_stru *pst_tid,
                                              oal_uint32     ul_stat_bytes,
                                              dmac_stat_direct_enum_uint8 en_stat_direct)
{
    oal_uint8                uc_stat_type    = 0;
    oal_void                *p_void          = OAL_PTR_NULL;
    dmac_stat_stru          *pst_stat        = OAL_PTR_NULL;
    oal_dlist_head_stru     *pst_dlist_pos   = OAL_PTR_NULL;
    dmac_stat_node_stru     *pst_stat_node   = OAL_PTR_NULL;

    for (uc_stat_type = MAC_STAT_TYPE_TID_THRPT; uc_stat_type <= MAC_STAT_TYPE_VAP_THRPT; uc_stat_type++)
    {
        /* 根据统计类型的不同，对p_void进行赋值 */
        if (MAC_STAT_TYPE_TID_THRPT == uc_stat_type)
        {
            p_void = (oal_void *)pst_tid;
        }
        else if (MAC_STAT_TYPE_USER_THRPT == uc_stat_type)
        {
            p_void = (oal_void *)pst_user;
        }
        else if (MAC_STAT_TYPE_VAP_THRPT == uc_stat_type)
        {
            p_void = (oal_void *)pst_vap;
        }

        pst_stat = &(g_ast_pfm_stat[uc_stat_type]);

        /* 遍列该统计类型的所有统计节点 */
        OAL_DLIST_SEARCH_FOR_EACH(pst_dlist_pos, &(pst_stat->st_stat_node_dlist))
        {
            pst_stat_node = OAL_DLIST_GET_ENTRY(pst_dlist_pos, dmac_stat_node_stru, st_entry);

            if ((p_void == pst_stat_node->pst_stat_param->p_void))
            {
                pst_stat_node->aul_stat_cnt[en_stat_direct]++;
                pst_stat_node->aul_stat_sum[en_stat_direct] += ul_stat_bytes;

                pst_stat_node->aul_stat_cnt[DMAC_STAT_BOTH]++;
                pst_stat_node->aul_stat_sum[DMAC_STAT_BOTH] += ul_stat_bytes;

                return OAL_SUCC;
            }
        }
    }

    return OAL_SUCC;
}



oal_uint32  dmac_stat_get_rx_tid(mac_vap_stru *pst_vap, dmac_rx_ctl_stru *pst_rx_ctl, oal_uint8 *puc_tidno)
{
    mac_ieee80211_frame_stru   *pst_frame_hdr   = OAL_PTR_NULL;
    oal_bool_enum_uint8         en_is_4addr;
    oal_uint8                   uc_is_tods;
    oal_uint8                   uc_is_from_ds;

    if ((OAL_PTR_NULL == pst_vap) || (OAL_PTR_NULL == pst_rx_ctl) || (OAL_PTR_NULL == puc_tidno))
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{dmac_stat_get_rx_tid::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&pst_rx_ctl->st_rx_info);

    if (OAL_PTR_NULL == pst_frame_hdr)
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_RX, "{dmac_stat_get_rx_tid::pst_frame_hdr null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 如果 该vap不是ht，或者该帧不是qos帧， 或者该帧是组播，则tid直接标记为be
       否则，则根据qos获取tid号 */
    if ((OAL_FALSE == mac_mib_get_HighThroughputOptionImplemented(pst_vap))
        ||((WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA) != ((oal_uint8 *)pst_frame_hdr)[0])
        ||(mac_is_grp_addr((oal_uint8 *)pst_frame_hdr)))
    {
        *puc_tidno = WLAN_TIDNO_BEST_EFFORT;
    }
    else
    {
        /* 考虑四地址情况获取报文的tid */
        uc_is_tods    = mac_hdr_get_to_ds((oal_uint8 *)pst_frame_hdr);
        uc_is_from_ds = mac_hdr_get_from_ds((oal_uint8 *)pst_frame_hdr);
        en_is_4addr   = uc_is_tods && uc_is_from_ds;

        *puc_tidno = mac_get_tid_value((oal_uint8 *)pst_frame_hdr, en_is_4addr);
    }

    return OAL_SUCC;
}


OAL_STATIC dmac_stat_node_stru* dmac_stat_search_node(oal_dlist_head_stru        *pst_node_dlist_head,
                                                      mac_stat_module_enum_uint16 en_module_id,
                                                      oal_void                   *p_void)
{
    oal_dlist_head_stru     *pst_dlist_pos      = OAL_PTR_NULL;
    dmac_stat_node_stru     *pst_stat_node      = OAL_PTR_NULL;

    if ((OAL_PTR_NULL == pst_node_dlist_head) || (OAL_PTR_NULL == p_void))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_search_node::param null.}");

        return OAL_PTR_NULL;
    }

    if (OAL_TRUE == oal_dlist_is_empty(pst_node_dlist_head))
    {
        return OAL_PTR_NULL;
    }

    /* 查找对应的统计节点 */
    OAL_DLIST_SEARCH_FOR_EACH(pst_dlist_pos, pst_node_dlist_head)
    {
        pst_stat_node = OAL_DLIST_GET_ENTRY(pst_dlist_pos, dmac_stat_node_stru, st_entry);

        if ((p_void == pst_stat_node->pst_stat_param->p_void) && (en_module_id == pst_stat_node->pst_stat_param->en_module_id))
        {
            return pst_stat_node;
        }
    }

    /* 没有找到，则返回空指针 */
    return OAL_PTR_NULL;
}


oal_uint32    dmac_stat_display(mac_stat_module_enum_uint16       en_module_id,
                                 mac_stat_type_enum_uint8       en_stat_type,
                                 oal_void                      *p_void)
{
    dmac_stat_node_stru         *pst_stat_node      = OAL_PTR_NULL;
    dmac_stat_stru              *pst_stat           = &(g_ast_pfm_stat[en_stat_type]);
    oal_uint32                   ul_ret             = OAL_SUCC;
    oam_output_type_enum_uint8   en_output_type     = OAM_OUTPUT_TYPE_BUTT;

    /* 查找统计节点 */
    pst_stat_node = dmac_stat_search_node(&(pst_stat->st_stat_node_dlist), en_module_id, p_void);
    if (OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_display::pst_stat_nodenull.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 停止定时器 */
    frw_timer_stop_timer(&(pst_stat_node->st_timer));

    /* 从oam模块获取输出方式 */
    ul_ret = oam_get_output_type(&en_output_type);
    if (OAL_SUCC != ul_ret)
    {
        return ul_ret;
    }

    ul_ret = dmac_stat_print(pst_stat_node, en_output_type);

    return ul_ret;
}


OAL_STATIC oal_uint32    dmac_stat_unregister_node(dmac_stat_node_stru   *pst_stat_node)
{
    if (OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_unregister_node::ps_stat_node null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 注销定时器 */
    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_stat_node->st_timer));

    /* 从链表中删除该节点 */
    oal_dlist_delete_entry(&(pst_stat_node->st_entry));

    /* 释放存储统计值的内存空间 */
    if (MAC_STAT_MODULE_CMD == pst_stat_node->pst_stat_param->en_module_id)
    {
        OAL_MEM_FREE(pst_stat_node->pul_stat_avg, OAL_TRUE);

        OAL_MEM_FREE(pst_stat_node->pst_stat_param, OAL_TRUE);
    }

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    if (MAC_STAT_MODULE_QOS == pst_stat_node->pst_stat_param->en_module_id)
    {
        OAL_MEM_FREE(pst_stat_node->pst_stat_param, OAL_TRUE);
    }
#endif

    /* 注销统计节点 */
    OAL_MEM_FREE(pst_stat_node, OAL_TRUE);

    return OAL_SUCC;
}



OAL_STATIC oal_uint32    dmac_stat_timer_handler(oal_void * p_void)
{
    dmac_stat_param_stru    *pst_stat_param     = OAL_PTR_NULL;
    dmac_stat_node_stru     *pst_stat_node      = OAL_PTR_NULL;
    dmac_stat_stru          *pst_stat           = OAL_PTR_NULL;
    oal_uint32               ul_index           = 0;
    oal_uint8                uc_cnt             = 0;

    if (OAL_PTR_NULL == p_void)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_timer_handler::p_void null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_stat_param = (dmac_stat_param_stru *)(p_void);
    pst_stat       = &(g_ast_pfm_stat[pst_stat_param->en_stat_type]);

    /* 查找统计节点 */
    pst_stat_node = dmac_stat_search_node(&(pst_stat->st_stat_node_dlist), pst_stat_param->en_module_id, pst_stat_param->p_void);
    if(OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_timer_handler::pst_stat_node null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新统计量 */
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    if (MAC_STAT_MODULE_QOS != pst_stat_param->en_module_id)
    {
        pst_stat_node->us_curr_item++;
    }
#else
    pst_stat_node->us_curr_item++;
#endif

    /* 根据统计类型更新统计量 */
    switch (pst_stat_param->en_stat_type)
    {
        case MAC_STAT_TYPE_TID_DELAY:
            pst_stat_param->aul_stat_avg[DMAC_STAT_TX]= pst_stat_node->aul_stat_cnt[DMAC_STAT_TX] ?
                (10 * pst_stat_node->aul_stat_sum[DMAC_STAT_TX] / pst_stat_node->aul_stat_cnt[DMAC_STAT_TX]) : 0;
            break;

        case MAC_STAT_TYPE_TID_PER:

            /* 统计mac层per */
            pst_stat_param->aul_stat_avg[DMAC_STAT_PER_MAC_TOTAL] = pst_stat_node->aul_stat_cnt[DMAC_STAT_PER_MAC_TOTAL] ?
                (10000 * pst_stat_node->aul_stat_sum[DMAC_STAT_PER_MAC_TOTAL] / pst_stat_node->aul_stat_cnt[DMAC_STAT_PER_MAC_TOTAL]) : 0;

            /* 分类统计各种原因导致的per */
            for (uc_cnt = DMAC_STAT_PER_BUFF_OVERFLOW; uc_cnt < DMAC_STAT_PER_RTS_FAIL; uc_cnt++)
            {
                if (0 == pst_stat_node->aul_stat_cnt[DMAC_STAT_PER_BUFF_OVERFLOW])
                {
                    pst_stat_param->aul_stat_avg[uc_cnt] = 0;
                }
                else
                {
                    pst_stat_param->aul_stat_avg[uc_cnt] = (10000 * pst_stat_node->aul_stat_sum[uc_cnt]) / pst_stat_node->aul_stat_cnt[DMAC_STAT_PER_BUFF_OVERFLOW];
                }
            }

            /* 统计硬件RTS发送失败情况，统计硬件重传和软件重传对应的数据帧失败情况 */
            for (uc_cnt = DMAC_STAT_PER_RTS_FAIL; uc_cnt < DMAC_STAT_PER_BUTT; uc_cnt++)
            {
                pst_stat_param->aul_stat_avg[uc_cnt] = pst_stat_node->aul_stat_cnt[uc_cnt] ?
                    (10000 * pst_stat_node->aul_stat_sum[uc_cnt] / pst_stat_node->aul_stat_cnt[uc_cnt]) : 0;
            }

            OAM_ERROR_LOG4(0, OAM_SF_ANY, "IP_layer stat1: ip_total = %d, ip_err_of = %d, ip_err_sz = %d, ip_err_ot = %d",
                                pst_stat_node->aul_stat_cnt[DMAC_STAT_PER_BUFF_OVERFLOW],
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_BUFF_OVERFLOW],
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_BUFF_BE_SEIZED],
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_DELAY_OVERTIME]);

            OAM_ERROR_LOG4(0, OAM_SF_ANY, "IP_layer stat2: ip_err_sw_ampdu = %d, ip_err_sw_sub_ampdu = %d, ip_err_sw_mpdu = %d, ip_err_sw_rt_of = %d",
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_SW_RETRY_AMPDU],
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_SW_RETRY_SUB_AMPDU],
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_SW_RETRY_MPDU],
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_SW_RETRY_OVERFLOW]);

            OAM_ERROR_LOG4(0, OAM_SF_ANY, "MAC_layer stat: mac_total(sw) = %d, mac_err_total(sw) = %d, mac_total(hw&sw) = %d, mac_err_total(hw&sw) = %d",
                                pst_stat_node->aul_stat_cnt[DMAC_STAT_PER_MAC_TOTAL],
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_MAC_TOTAL],
                                pst_stat_node->aul_stat_cnt[DMAC_STAT_PER_HW_SW_FAIL],
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_HW_SW_FAIL]);

            OAM_ERROR_LOG2(0, OAM_SF_ANY, "MAC_layer stat: rts_total = %d, rts_fail = %d",
                                pst_stat_node->aul_stat_cnt[DMAC_STAT_PER_RTS_FAIL],
                                pst_stat_node->aul_stat_sum[DMAC_STAT_PER_RTS_FAIL]);
            break;

        case MAC_STAT_TYPE_TID_THRPT:
        case MAC_STAT_TYPE_USER_THRPT:
        case MAC_STAT_TYPE_VAP_THRPT:
            if(0 != pst_stat_node->st_timer.ul_timeout)
            {
                pst_stat_param->aul_stat_avg[DMAC_STAT_TX]     = 8 * pst_stat_node->aul_stat_sum[DMAC_STAT_TX] / pst_stat_node->st_timer.ul_timeout;
                pst_stat_param->aul_stat_avg[DMAC_STAT_RX]     = 8 * pst_stat_node->aul_stat_sum[DMAC_STAT_RX] / pst_stat_node->st_timer.ul_timeout;
                pst_stat_param->aul_stat_avg[DMAC_STAT_BOTH]   = 8 * pst_stat_node->aul_stat_sum[DMAC_STAT_BOTH] / pst_stat_node->st_timer.ul_timeout;
            }
            else
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_stat_timer_handler: Error! 0 == timeout!");
                pst_stat_param->aul_stat_avg[DMAC_STAT_TX] = 0;
                pst_stat_param->aul_stat_avg[DMAC_STAT_RX] = 0;
                pst_stat_param->aul_stat_avg[DMAC_STAT_BOTH] = 0;
            }
            break;

        default:
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_timer_handler::invalid en_stat_type.}");

            return OAL_FAIL;
    }

    /* 如果为命令配置模块，则更新至内部存储空间 */
    if (MAC_STAT_MODULE_CMD == pst_stat_param->en_module_id)
    {
        ul_index = pst_stat_node->us_curr_item - 1;

        if ((MAC_STAT_TYPE_TID_THRPT == pst_stat_param->en_stat_type)
            ||(MAC_STAT_TYPE_USER_THRPT == pst_stat_param->en_stat_type)
            ||(MAC_STAT_TYPE_VAP_THRPT == pst_stat_param->en_stat_type))
        {
            for (uc_cnt = 0; uc_cnt <= DMAC_STAT_BOTH; uc_cnt++)
            {
                *(pst_stat_node->pul_stat_avg + ul_index + uc_cnt * DMAC_STAT_ITEM_LIMIT) = pst_stat_param->aul_stat_avg[uc_cnt];
            }
        }
        else if (MAC_STAT_TYPE_TID_PER == pst_stat_param->en_stat_type)
        {
            for (uc_cnt = 0; uc_cnt < DMAC_STAT_PER_BUTT; uc_cnt++)
            {
                *(pst_stat_node->pul_stat_avg + ul_index + uc_cnt * DMAC_STAT_ITEM_LIMIT) = pst_stat_param->aul_stat_avg[uc_cnt];
            }
        }
        else
        {
            *(pst_stat_node->pul_stat_avg + ul_index) = pst_stat_param->aul_stat_avg[0];
        }
    }
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    else if(MAC_STAT_MODULE_QOS == pst_stat_param->en_module_id)
    {
    }
#endif
    else    /* 调用相应的处理函数 */
    {
        pst_stat_node->p_inner_func(pst_stat_param);
    }

    /* 该周期内的统计参数清零 */
    for (uc_cnt = 0; uc_cnt < DMAC_STAT_PER_BUTT; uc_cnt++)
    {
        pst_stat_node->aul_stat_cnt[uc_cnt]   = 0;
        pst_stat_node->aul_stat_sum[uc_cnt]   = 0;
    }

    /* 统计结束 */
    if (pst_stat_node->us_curr_item >= pst_stat_node->us_total_item)
    {
        frw_timer_stop_timer(&(pst_stat_node->st_timer));
        pst_stat_node->uc_stat_flag = OAL_FALSE;
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_stat_format_title_string(oal_int8                  *pac_output_data,
                                                      oal_uint16                 ul_data_len,
                                                      dmac_stat_node_stru       *pst_stat_node)
{
    switch (pst_stat_node->pst_stat_param->en_stat_type)
    {
        case MAC_STAT_TYPE_TID_PER:

            /* 格式化输出tid per统计信息 */
            OAL_SPRINTF(pac_output_data,
                        ul_data_len,
                        "stat_num=%u,stat_period(ms)=%u,start_time =%u\r\n%15s%15s%15s%15s%15s%15s%15s%15s%15s%15s%15s\r\n",
                        pst_stat_node->us_total_item,
                        pst_stat_node->st_timer.ul_timeout,
                        pst_stat_node->st_timer.ul_time_stamp,
                        "idx",
                        "mac_per(10^-4)",
                        "overflow_per",
                        "seize_per",
                        "overtime_per",
                        "sw_per_ampdu",
                        "sw_per_sub",
                        "sw_per_mpdu",
                        "sw_overf_per",
                        "rts_fail",
                        "mac_per(sw&hw)");
            break;

        case MAC_STAT_TYPE_TID_DELAY:

            /* 格式化输出tid delay统计信息 */
            OAL_SPRINTF(pac_output_data,
                        ul_data_len,
                        "stat_num=%u,stat_period(ms)=%u,start_time=%u\r\n%15s%15s\r\n",
                        pst_stat_node->us_total_item,
                        pst_stat_node->st_timer.ul_timeout,
                        pst_stat_node->st_timer.ul_time_stamp,
                        "index",
                        "delay(0.1ms)");

            break;

        case MAC_STAT_TYPE_TID_THRPT:
        case MAC_STAT_TYPE_USER_THRPT:
        case MAC_STAT_TYPE_VAP_THRPT:

            /* 格式化输出吞吐量统计信息 */
            OAL_SPRINTF(pac_output_data,
                        ul_data_len,
                        "stat_num=%u,stat_period=%u,start_time=%15u\r\n%15s%15s%15s%15s\r\n",
                        pst_stat_node->us_total_item,
                        pst_stat_node->st_timer.ul_timeout,
                        pst_stat_node->st_timer.ul_time_stamp,
                        "index",
                        "tx(kbps)",
                        "rx(kbps)",
                        "total(kbps)");

            break;

        default:
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_format_title_string::invalid en_stat_type.}");

            return OAL_FAIL;
    }
    return OAL_SUCC;

}



OAL_STATIC oal_uint32  dmac_stat_format_data_string(oal_int8                  *pac_output_data,
                                                     oal_uint16                 ul_data_len,
                                                     mac_stat_type_enum_uint8   en_stat_type,
                                                     oal_uint32                 ul_index,
                                                     oal_uint32                *pul_data)
{
    switch (en_stat_type)
    {
        case MAC_STAT_TYPE_TID_THRPT:
        case MAC_STAT_TYPE_USER_THRPT:
        case MAC_STAT_TYPE_VAP_THRPT:
            /* 格式化输出吞吐量信息 */
            OAL_SPRINTF(pac_output_data,
                        ul_data_len,
                        "%15u%15u%15u%15u\r\n",
                        ul_index,
                        *(pul_data),
                        *(pul_data + DMAC_STAT_ITEM_LIMIT),
                        *(pul_data + 2 * DMAC_STAT_ITEM_LIMIT));
            break;
        case MAC_STAT_TYPE_TID_DELAY:
            /* 格式化输tid delay,per信息 */
            OAL_SPRINTF(pac_output_data,
                        ul_data_len,
                        "%15u%15u\r\n",
                        ul_index,
                        *(pul_data));
            break;
        case MAC_STAT_TYPE_TID_PER:
            /* 格式化输出吞吐量信息 */
            OAL_SPRINTF(pac_output_data,
                        ul_data_len,
                        "%15u%15u%15u%15u%15u%15u%15u%15u%15u%15u%15u\r\n",
                        ul_index,
                        *(pul_data),
                        *(pul_data + 1 * DMAC_STAT_ITEM_LIMIT),
                        *(pul_data + 2 * DMAC_STAT_ITEM_LIMIT),
                        *(pul_data + 3 * DMAC_STAT_ITEM_LIMIT),
                        *(pul_data + 4 * DMAC_STAT_ITEM_LIMIT),
                        *(pul_data + 5 * DMAC_STAT_ITEM_LIMIT),
                        *(pul_data + 6 * DMAC_STAT_ITEM_LIMIT),
                        *(pul_data + 7 * DMAC_STAT_ITEM_LIMIT),
                        *(pul_data + 8 * DMAC_STAT_ITEM_LIMIT),
                        *(pul_data + 9 * DMAC_STAT_ITEM_LIMIT));
            break;
        default:
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_format_data_string::invalid en_stat_type.}");
            return OAL_FAIL;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_uint32  dmac_stat_print(dmac_stat_node_stru     *pst_stat_node,
                                       oam_output_type_enum_uint8 en_output_type)
{
    oal_uint32              ul_ret              = OAL_SUCC;
    oal_uint16              us_index            = 0;

    oal_int8                ac_output_data[OAM_PRINT_FORMAT_LENGTH];

    if (OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_print_to_std::pst_stat_node null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 格式化抬头信息 */
    ul_ret = dmac_stat_format_title_string(ac_output_data,
                                           OAM_PRINT_FORMAT_LENGTH,
                                           pst_stat_node);
    if (OAL_SUCC != ul_ret)
    {
        return ul_ret;
    }

    /* 根据输出方式进行输出 */
    switch (en_output_type)
    {
        /* 输出至控制台 */
        case OAM_OUTPUT_TYPE_CONSOLE:
            OAL_IO_PRINT("%s\r\n", ac_output_data);
            break;

        /* 输出至PC侧调测工具平台 */
        case OAM_OUTPUT_TYPE_SDT:
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
            oam_print(ac_output_data);
#endif
            break;

        /* 无效配置 */
        default:
            ul_ret = OAL_ERR_CODE_INVALID_CONFIG;
            return ul_ret;
    }

    /* 打印所有的统计数据 */
    for (us_index = 0; us_index < pst_stat_node->us_total_item; us_index++)
    {
        ul_ret = dmac_stat_format_data_string(ac_output_data,
                                               OAM_PRINT_FORMAT_LENGTH,
                                               pst_stat_node->pst_stat_param->en_stat_type,
                                               us_index,
                                               pst_stat_node->pul_stat_avg + us_index);
        if (OAL_SUCC != ul_ret)
        {
            return ul_ret;
        }

        /* 根据输出方式进行输出 */
        switch (en_output_type)
        {
            /* 输出至控制台 */
            case OAM_OUTPUT_TYPE_CONSOLE:
                OAL_IO_PRINT("%s\r\n", ac_output_data);
                break;

            /* 输出至PC侧调测工具平台 */
            case OAM_OUTPUT_TYPE_SDT:
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
                oam_print(ac_output_data);
#endif
            /* 无效配置 */
            default:
                ul_ret = OAL_ERR_CODE_INVALID_CONFIG;
                return ul_ret;
        }
    }

    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE

oal_uint32  dmac_stat_qos_init_user(dmac_user_stru *pst_dmac_user)
{
    mac_vap_stru                   *pst_mac_vap     = OAL_PTR_NULL;
    oal_void                       *p_void          = OAL_PTR_NULL;
    oal_uint32                      ul_ret          = OAL_SUCC;

    /* 检查输入参数是否为空 */
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_qos_init_user:: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    p_void = (oal_void *)(&(pst_dmac_user->st_user_base_info));
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);

    /* 注册统计节点 */
    ul_ret = dmac_stat_register(MAC_STAT_MODULE_QOS, MAC_STAT_TYPE_USER_THRPT, p_void, OAL_PTR_NULL, OAL_PTR_NULL,pst_mac_vap->ul_core_id);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_qos_init_user:: register stat node failed!}");
        return OAL_SUCC;
    }

    /* 同时开始启动统计 */
    ul_ret = dmac_stat_start(MAC_STAT_MODULE_QOS,
                             MAC_STAT_TYPE_USER_THRPT,
                             DMAC_QOS_TIMER_CYCLE_MS,
                             1,
                             p_void);

    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_qos_init_user:: stat_start failed!}");
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


oal_uint32  dmac_stat_qos_exit_user(dmac_user_stru *pst_dmac_user)
{
    oal_void                       *p_void              = OAL_PTR_NULL;
    oal_uint32                      ul_ret              = OAL_SUCC;

    /* 检查输入参数是否为空 */
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_qos_exit_user:: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    p_void = (oal_void *)(&(pst_dmac_user->st_user_base_info));

    /* 注销统计节点 */
    ul_ret = dmac_stat_unregister(MAC_STAT_MODULE_QOS, MAC_STAT_TYPE_USER_THRPT, p_void);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_qos_init_user:: unregister stat node failed!}");
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


oal_uint32 dmac_stat_tx_sta_thrpt(mac_user_stru *pst_user)
{
    oal_void                *p_void          = OAL_PTR_NULL;
    dmac_stat_stru          *pst_stat        = OAL_PTR_NULL;
    dmac_stat_node_stru     *pst_stat_node   = OAL_PTR_NULL;
    oal_uint32               ul_tx_rate = 0 ;

    /* 检查输入参数是否为空 */
    if (OAL_PTR_NULL == pst_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_tx_sta_thrpt:: input pointer is null!}");
        return ul_tx_rate;
    }

    p_void = (oal_void *)pst_user;
    pst_stat = &(g_ast_pfm_stat[MAC_STAT_TYPE_USER_THRPT]);

    /* 寻找匹配的统计user节点 */
    pst_stat_node = dmac_stat_search_node(&(pst_stat->st_stat_node_dlist), MAC_STAT_MODULE_QOS, p_void);
    if (OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_tx_sta_thrpt:: can not search pst_stat_node!}");
        return ul_tx_rate;
    }

    /* 获取匹配的统计user节点的tx吞吐值 */
    if (pst_stat_node->pst_stat_param->aul_stat_avg[DMAC_STAT_TX] >= 0)
    {
        ul_tx_rate = pst_stat_node->pst_stat_param->aul_stat_avg[DMAC_STAT_TX];
    }

    return ul_tx_rate;
}
#endif

#ifdef _PRE_WLAN_11K_STAT

#define DMAC_STAT_EXTERNAL_FUNC

oal_int32  dmac_stat_init_device(oal_void)
{
    oal_uint32                     ul_ret           = OAL_SUCC;
    hal_to_dmac_device_stru       *pst_hal_device;
    oal_uint8                      uc_device_num;
    oal_uint8                      uc_chip_num;
    dmac_device_stru               *pst_dmac_device;

    for (uc_chip_num = 0; uc_chip_num < WLAN_CHIP_MAX_NUM_PER_BOARD; uc_chip_num++)
    {
        for (uc_device_num = 0; uc_device_num < WLAN_DEVICE_MAX_NUM_PER_CHIP; uc_device_num++)
        {
            ul_ret = hal_chip_get_hal_device(uc_chip_num, uc_device_num, &pst_hal_device);
            if ((OAL_SUCC != ul_ret) || (OAL_PTR_NULL == pst_hal_device))
            {
                OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_stat_init_device::hal_chip_get_hal_device fail!! ret:%d!hal device[%p]}", ul_ret, pst_hal_device);
                continue;
            }

            pst_dmac_device = dmac_res_get_mac_dev(pst_hal_device->uc_mac_device_id);
            if (OAL_PTR_NULL == pst_dmac_device)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_stat_init_device: pst dmac device null pointer! mac dev id:%d}", pst_hal_device->uc_mac_device_id);
                continue;
            }

            pst_dmac_device->pst_stat_count = (dmac_stat_count_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,
                    OAL_SIZEOF(dmac_stat_count_stru), OAL_TRUE);

            if ( OAL_PTR_NULL == pst_dmac_device->pst_stat_count )
            {
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_stat_init_device::mem alloc fail!!.");
                continue;
            }
            OAL_MEMZERO(pst_dmac_device->pst_stat_count, OAL_SIZEOF(dmac_stat_count_stru));

        }
    }

    return OAL_SUCC;
}


oal_int32  dmac_stat_exit_device(oal_void)
{
    oal_uint32                     ul_ret           = OAL_SUCC;
    hal_to_dmac_device_stru       *pst_hal_device;
    oal_uint8                      uc_device_num;
    oal_uint8                      uc_chip_num;
    dmac_device_stru               *pst_dmac_device;

    for (uc_chip_num = 0; uc_chip_num < WLAN_CHIP_MAX_NUM_PER_BOARD; uc_chip_num++)
    {
        for (uc_device_num = 0; uc_device_num < WLAN_DEVICE_MAX_NUM_PER_CHIP; uc_device_num++)
        {
            ul_ret = hal_chip_get_hal_device(uc_chip_num, uc_device_num, &pst_hal_device);
            if ((OAL_SUCC != ul_ret) || (OAL_PTR_NULL == pst_hal_device))
            {
                OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_stat_exit_device::hal_chip_get_hal_device fail!! ret:%d!hal device[%p]}", ul_ret, pst_hal_device);
                continue;
            }

            pst_dmac_device = dmac_res_get_mac_dev(pst_hal_device->uc_mac_device_id);
            if (OAL_PTR_NULL == pst_dmac_device)
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_stat_exit_device: pst dmac device null pointer! mac dev id:%d}", pst_hal_device->uc_mac_device_id);
                continue;
            }

            if ( OAL_PTR_NULL == pst_dmac_device->pst_stat_count )
            {
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_stat_exit_device::pst_stat_count is null!!.");
                continue;
            }
            OAL_MEM_FREE(pst_dmac_device->pst_stat_count, OAL_TRUE);

        }
    }

    return OAL_SUCC;
}


oal_uint32  dmac_stat_init_vap(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8                           uc_bin_idx;

    /*1. 检查输入参数是否为空*/
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_init_vap:: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*2. 统计模块使能*/
    OAL_MEMZERO(&(pst_dmac_vap->st_stat_cap_flag), OAL_SIZEOF(dmac_stat_cap_flag_stru));
    pst_dmac_vap->st_stat_cap_flag.bit_enable           = OAL_TRUE;
    pst_dmac_vap->st_stat_cap_flag.bit_count            = OAL_TRUE;
    pst_dmac_vap->st_stat_cap_flag.bit_frm_rpt          = OAL_FALSE;
    pst_dmac_vap->st_stat_cap_flag.bit_tid_count        = OAL_TRUE;
    pst_dmac_vap->st_stat_cap_flag.bit_tid_tx_delay     = OAL_TRUE;
    pst_dmac_vap->st_stat_cap_flag.bit_tsc_rpt          = OAL_FALSE;
    pst_dmac_vap->st_stat_cap_flag.bit_user_tid_count   = OAL_FALSE;

    /*3. 判断是否使能统计*/
    if(OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable)
    {
        return OAL_SUCC;
    }

    /*4. 申请空间和初始化*/
    if(pst_dmac_vap->st_stat_cap_flag.bit_count)
    {
        pst_dmac_vap->pst_stat_count = (dmac_stat_count_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_count_stru), OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_vap->pst_stat_count)
        {
            OAL_MEMZERO(pst_dmac_vap->pst_stat_count, OAL_SIZEOF(dmac_stat_count_stru));
        }
    }
    if(pst_dmac_vap->st_stat_cap_flag.bit_frm_rpt)
    {
        pst_dmac_vap->pst_stat_frm_rpt = (dmac_stat_frm_rpt_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_frm_rpt_stru), OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_vap->pst_stat_frm_rpt)
        {
            OAL_MEMZERO(pst_dmac_vap->pst_stat_frm_rpt, OAL_SIZEOF(dmac_stat_frm_rpt_stru));
        }
    }
    if(pst_dmac_vap->st_stat_cap_flag.bit_tid_count)
    {
        pst_dmac_vap->pst_stat_count_tid = (dmac_stat_count_tid_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_count_tid_stru) * WLAN_TIDNO_BUTT, OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_vap->pst_stat_count_tid )
        {
            OAL_MEMZERO(pst_dmac_vap->pst_stat_count_tid , OAL_SIZEOF(dmac_stat_count_tid_stru) * WLAN_TIDNO_BUTT);
        }
    }
    if(pst_dmac_vap->st_stat_cap_flag.bit_tid_tx_delay)
    {
        pst_dmac_vap->pst_stat_tid_tx_delay = (dmac_stat_tid_tx_delay_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_tid_tx_delay_stru) * WLAN_TIDNO_BUTT, OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_vap->pst_stat_tid_tx_delay )
        {
            OAL_MEMZERO(pst_dmac_vap->pst_stat_tid_tx_delay , OAL_SIZEOF(dmac_stat_tid_tx_delay_stru) * WLAN_TIDNO_BUTT);
        }
    }
    if(pst_dmac_vap->st_stat_cap_flag.bit_tsc_rpt)
    {
        pst_dmac_vap->pst_stat_tsc_rpt = (dmac_stat_tsc_rpt_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_tsc_rpt_stru), OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_vap->pst_stat_tsc_rpt)
        {
            OAL_MEMZERO(pst_dmac_vap->pst_stat_tsc_rpt, OAL_SIZEOF(dmac_stat_tsc_rpt_stru));
        }
    }

    /*设置TXDELAY统计直方图*/
    if(OAL_PTR_NULL !=pst_dmac_vap->pst_stat_tsc_rpt)
    {
        for ( uc_bin_idx = 0 ; uc_bin_idx < DMAC_STAT_TX_DELAY_HIST_BIN_NUM; uc_bin_idx++ )
        {
            pst_dmac_vap->pst_stat_tsc_rpt->st_tid_tx_delay_hist.aus_hist_range[uc_bin_idx] = DMAC_STAT_TX_DELAY_HIST_BIN0_RANGE * uc_bin_idx;
        }
    }

    return OAL_SUCC;

}


oal_uint32  dmac_stat_exit_vap(dmac_vap_stru *pst_dmac_vap)
{
    /*1. 检查输入参数是否为空*/
    if( OAL_PTR_NULL == pst_dmac_vap )
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_exit_vap:: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*2. 释放*/
    if(OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable)
    {
        return OAL_SUCC;
    }

    if(OAL_PTR_NULL != pst_dmac_vap->pst_stat_count)
    {
        OAL_MEM_FREE(pst_dmac_vap->pst_stat_count, OAL_TRUE);
    }
    if (OAL_PTR_NULL != pst_dmac_vap->pst_stat_frm_rpt)
    {
        OAL_MEM_FREE(pst_dmac_vap->pst_stat_frm_rpt, OAL_TRUE);
    }
    if (OAL_PTR_NULL != pst_dmac_vap->pst_stat_count_tid)
    {
        OAL_MEM_FREE(pst_dmac_vap->pst_stat_count_tid, OAL_TRUE);
    }
    if (OAL_PTR_NULL != pst_dmac_vap->pst_stat_tid_tx_delay)
    {
        OAL_MEM_FREE(pst_dmac_vap->pst_stat_tid_tx_delay, OAL_TRUE);
    }
    if (OAL_PTR_NULL != pst_dmac_vap->pst_stat_tsc_rpt)
    {
        OAL_MEM_FREE(pst_dmac_vap->pst_stat_tid_tx_delay, OAL_TRUE);
    }

    return OAL_SUCC;
}


oal_uint32  dmac_stat_init_user(dmac_user_stru *pst_dmac_user)
{
    dmac_vap_stru                       *pst_dmac_vap;
    oal_uint8                           uc_bin_idx;

    /*1. 检查输入参数是否为空*/
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_init_user:: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*2. 获取vap 信息*/
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_stat_init_user::pst_dmac_vap[vap_id=%d] null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*3.判断是否使能统计*/
    if(OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable)
    {
        return OAL_SUCC;
    }

    /*4. 组播用户不统计*/
    if (OAL_TRUE == pst_dmac_user->st_user_base_info.en_is_multi_user)
    {
        return OAL_SUCC;
    }

    /*5. 申请空间和初始化*/
    OAL_MEMZERO(&(pst_dmac_user->st_dmac_thrpt_stat_info), OAL_SIZEOF(dmac_thrpt_stat_info_stru));
    OAL_MEMZERO(&(pst_dmac_user->st_user_rate_info), OAL_SIZEOF(dmac_user_rate_info_stru));

    if(pst_dmac_vap->st_stat_cap_flag.bit_count)
    {
        pst_dmac_user->pst_stat_count = (dmac_stat_count_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_count_stru), OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_user->pst_stat_count)
        {
            OAL_MEMZERO(pst_dmac_user->pst_stat_count, OAL_SIZEOF(dmac_stat_count_stru));
        }
    }
    if(pst_dmac_vap->st_stat_cap_flag.bit_frm_rpt)
    {
        pst_dmac_user->pst_stat_frm_rpt = (dmac_stat_frm_rpt_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_frm_rpt_stru), OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_user->pst_stat_frm_rpt)
        {
            OAL_MEMZERO(pst_dmac_user->pst_stat_frm_rpt, OAL_SIZEOF(dmac_stat_frm_rpt_stru));
        }
    }
    if(pst_dmac_vap->st_stat_cap_flag.bit_user_tid_count)
    {
        pst_dmac_user->pst_stat_count_tid = (dmac_stat_count_tid_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_count_tid_stru) * WLAN_TIDNO_BUTT, OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_user->pst_stat_count_tid)
        {
            OAL_MEMZERO(pst_dmac_user->pst_stat_count_tid, OAL_SIZEOF(dmac_stat_count_tid_stru) * WLAN_TIDNO_BUTT);
        }
    }
    if(pst_dmac_vap->st_stat_cap_flag.bit_tid_tx_delay)
    {
        pst_dmac_user->pst_stat_tid_tx_delay = (dmac_stat_tid_tx_delay_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_tid_tx_delay_stru) * WLAN_TIDNO_BUTT, OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_user->pst_stat_tid_tx_delay)
        {
            OAL_MEMZERO(pst_dmac_user->pst_stat_tid_tx_delay, OAL_SIZEOF(dmac_stat_tid_tx_delay_stru) * WLAN_TIDNO_BUTT);
        }
    }
    if(pst_dmac_vap->st_stat_cap_flag.bit_tsc_rpt)
    {
        pst_dmac_user->pst_stat_tsc_rpt = (dmac_stat_tsc_rpt_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_stat_tsc_rpt_stru), OAL_TRUE);
        if (OAL_PTR_NULL != pst_dmac_user->pst_stat_tsc_rpt)
        {
            OAL_MEMZERO(pst_dmac_user->pst_stat_tsc_rpt, OAL_SIZEOF(dmac_stat_tsc_rpt_stru));
        }
    }

    /*设置TXDELAY统计直方图*/
    if(OAL_PTR_NULL !=pst_dmac_user->pst_stat_tsc_rpt)
    {
        for ( uc_bin_idx = 0 ; uc_bin_idx < DMAC_STAT_TX_DELAY_HIST_BIN_NUM; uc_bin_idx++ )
        {
            pst_dmac_user->pst_stat_tsc_rpt->st_tid_tx_delay_hist.aus_hist_range[uc_bin_idx] = DMAC_STAT_TX_DELAY_HIST_BIN0_RANGE * uc_bin_idx;
        }
    }

    return OAL_SUCC;

}


oal_uint32  dmac_stat_exit_user(dmac_user_stru *pst_dmac_user)
{
    dmac_vap_stru               *pst_dmac_vap;

    /*1. 检查输入参数是否为空*/
    if(NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_exit_user:: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*2. 获取vap 信息*/
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_stat_init_user::pst_dmac_vap[vap_id=%d] null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*3.判断是否使能统计*/
    if(OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable)
    {
        return OAL_SUCC;
    }

    /*4. 组播用户不统计*/
    if (OAL_TRUE == pst_dmac_user->st_user_base_info.en_is_multi_user)
    {
        return OAL_SUCC;
    }

    /*5. 释放*/
    if(OAL_PTR_NULL != pst_dmac_user->pst_stat_count)
    {
        OAL_MEM_FREE(pst_dmac_user->pst_stat_count, OAL_TRUE);
    }
    if (OAL_PTR_NULL != pst_dmac_user->pst_stat_frm_rpt)
    {
        OAL_MEM_FREE(pst_dmac_user->pst_stat_frm_rpt, OAL_TRUE);
    }
    if (OAL_PTR_NULL != pst_dmac_user->pst_stat_count_tid)
    {
        OAL_MEM_FREE(pst_dmac_user->pst_stat_count_tid, OAL_TRUE);
    }
    if (OAL_PTR_NULL != pst_dmac_user->pst_stat_tid_tx_delay)
    {
        OAL_MEM_FREE(pst_dmac_user->pst_stat_tid_tx_delay, OAL_TRUE);
    }
    if (OAL_PTR_NULL != pst_dmac_user->pst_stat_tsc_rpt)
    {
        OAL_MEM_FREE(pst_dmac_user->pst_stat_tid_tx_delay, OAL_TRUE);
    }

    return OAL_SUCC;
}


oal_uint32 dmac_user_stat_tx_frm_info(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
        oal_uint8 uc_tidno, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param, oal_uint8 bit_is_ampdu)
{
    oal_uint32                   ul_stat_bytes;
    dmac_stat_count_tid_stru     *pst_count_stat_tid_info;
    dmac_device_stru             *pst_dmac_device;

    /*判断是否使能统计*/
    if (OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable)
    {
        return OAL_SUCC;
    }

    /*用户TX吞吐量统计*/
    ul_stat_bytes = pst_tx_dscr_param->us_mpdu_len *
        (pst_tx_dscr_param->uc_mpdu_num - pst_tx_dscr_param->uc_error_mpdu_num);
    DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_dmac_thrpt_stat_info.ull_tx_bytes, ul_stat_bytes);
    DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_dmac_thrpt_stat_info.ul_tx_thrpt_stat_count, 1 );

    /* 获取Device结构体指针 */
    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CCA_OPT, "{dmac_user_stat_tx_frm_info: pst dmac device null pointer! mac dev id:%d}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_count)
    {
        dmac_stat_calc_tx_frm_num(pst_dmac_device->pst_stat_count, pst_tx_dscr_param, bit_is_ampdu);
        dmac_stat_calc_tx_frm_num(pst_dmac_vap->pst_stat_count, pst_tx_dscr_param, bit_is_ampdu);
        dmac_stat_calc_tx_frm_num(pst_dmac_user->pst_stat_count, pst_tx_dscr_param, bit_is_ampdu);
    }

    if (OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_tid_count)
    {
        /*判断count tid统计结构体是否为空*/
        if ( OAL_PTR_NULL == pst_dmac_vap->pst_stat_count_tid
             || OAL_PTR_NULL == pst_dmac_user->pst_stat_count_tid)
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_count_stat_tid_info = (dmac_stat_count_tid_stru*)(&pst_dmac_vap->pst_stat_count_tid[uc_tidno]);
        DMAC_VAP_STATS_PKT_INCR(pst_count_stat_tid_info->st_stat_mac_stat.ul_rts_suc_num, pst_tx_dscr_param->uc_rts_succ);
        DMAC_VAP_STATS_PKT_INCR(pst_count_stat_tid_info->st_stat_mac_stat.ul_rts_fail_num, pst_tx_dscr_param->uc_cts_failure);

        pst_count_stat_tid_info = (dmac_stat_count_tid_stru*)(&pst_dmac_user->pst_stat_count_tid[uc_tidno]);
        DMAC_USER_STATS_PKT_INCR(pst_count_stat_tid_info->st_stat_mac_stat.ul_rts_suc_num, pst_tx_dscr_param->uc_rts_succ);
        DMAC_USER_STATS_PKT_INCR(pst_count_stat_tid_info->st_stat_mac_stat.ul_rts_fail_num, pst_tx_dscr_param->uc_cts_failure);
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT

oal_uint32 dmac_user_stat_tx_mcs_count(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
    mac_tx_ctl_stru* pst_cb, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param)
{
    oal_uint8                    uc_rate_rank;
    hal_tx_txop_per_rate_params_union   *pst_per_rate_params = &pst_tx_dscr_param->ast_per_rate[HAL_TX_RATE_MAX_NUM-1];

    if (OAL_TRUE != pst_dmac_vap->st_stat_cap_flag.bit_count)
    {
        return OAL_SUCC;
    }

    uc_rate_rank = HAL_TX_RATE_MAX_NUM;
    while(uc_rate_rank > 0)
    {
        uc_rate_rank--;
        if (pst_per_rate_params->rate_bit_stru.bit_tx_count != 0)
        {
            if (pst_per_rate_params->rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE)
            {
                oal_uint8 bit_ht_mcs = pst_per_rate_params->rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs;
                if (bit_ht_mcs <= 15)
                {
                    pst_dmac_user->pst_stat_count->st_count_mpdu.aul_sta_tx_mcs_cnt[bit_ht_mcs]++;
                }
            }
            else if (pst_per_rate_params->rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE)
            {
                oal_uint8 bit_vht_mcs = pst_per_rate_params->rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs;
                pst_dmac_user->pst_stat_count->st_count_mpdu.aul_sta_tx_mcs_cnt[bit_vht_mcs]++;
            }
            break;
        }
        pst_per_rate_params--;
    }
    return OAL_SUCC;
}

#endif


oal_uint32 dmac_user_stat_tx_mpdu_info(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
    mac_tx_ctl_stru* pst_cb, oal_uint8 uc_dscr_status, oal_bool_enum_uint8 en_is_discarded)
{
    oal_uint32                   ul_sub_msdu_num;
    oal_uint8                    uc_tidno;

    dmac_stat_count_common_stru  st_stat_info;
    dmac_stat_count_tid_stru     *pst_count_stat_tid_info;
    dmac_device_stru             *pst_dmac_device;

    /*判断是否使能统计*/
    if (OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable)
    {
        return OAL_SUCC;
    }

    /*获取MSDU个数*/
    ul_sub_msdu_num   = MAC_GET_CB_NETBUF_NUM(pst_cb);

    uc_tidno = MAC_GET_CB_WME_TID_TYPE(pst_cb);

    OAL_MEMZERO(&st_stat_info, OAL_SIZEOF(dmac_stat_count_common_stru));

    if(DMAC_TX_SUCC == uc_dscr_status)
    {
        /* dot11TransmittedFragmentCount 成功发送的MPDU个数，分片 */
        st_stat_info.ul_tx_frag_mpdu_num = (OAL_TRUE == MAC_GET_CB_IS_MORE_FRAGMENTS(pst_cb))?1:0;

        /* dot11MulticastTransmitted FrameCount 成功发送的MSDU个数，目的地址为组播*/
        st_stat_info.ul_tx_multicast_mpdu_num = (OAL_TRUE == pst_cb->bit_ismcast)?1:0;

        /* dot11TransmittedFrameCount, 成功发送的MSDU个数*/
        st_stat_info.ul_tx_succ_mpdu_num = 1;

        /* dot11RetryCount, 在一次或者多次重传后发送成功的MSDU个数*/
        st_stat_info.ul_tx_retry_succ_mpdu_num = ( MAC_GET_CB_RETRIED_NUM(pst_cb) >= 2 )? 1: 0;

        /* dot11MultipleRetryCount, 在多次重传后发送成功的MSDU个数*/
        st_stat_info.ul_tx_multi_retry_succ_mpdu_num = ( MAC_GET_CB_RETRIED_NUM(pst_cb) > 2 )? 1: 0;
    }
    else
    {
        /* dot11ACKFailureCount, 未收到ACK的MPDU个数*/
        st_stat_info.ul_ack_fail_mpdu_num = ( ( OAL_FALSE == en_is_discarded ) && (DMAC_TX_FAIL == uc_dscr_status))? 1:0;

        /* dot11FailedCount, 失败MSDU个数, 发送次数超出RetryLimit */
        st_stat_info.ul_tx_fail_mpdu_num = (( OAL_TRUE == en_is_discarded ) && (MAC_GET_CB_RETRIED_NUM(pst_cb) >= pst_dmac_vap->uc_sw_retry_limit))? 1 : 0;

        /* dot11QosDiscardedFrameCount, 失败MSDU个数, 发送次数超出RetryLimit 或者　超出lifetime*/
        st_stat_info.ul_tx_discard_mpdu_num = ( OAL_TRUE == en_is_discarded )? 1: 0;
    }

    /* 获取Device结构体指针 */
    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CCA_OPT, "{dmac_user_stat_tx_mpdu_info: pst dmac device null pointer! mac dev id:%d}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_count)
    {
        dmac_stat_calc_tx_mpdu_num(pst_dmac_device->pst_stat_count, &st_stat_info, pst_cb);
        dmac_stat_calc_tx_mpdu_num(pst_dmac_vap->pst_stat_count, &st_stat_info, pst_cb);
        dmac_stat_calc_tx_mpdu_num(pst_dmac_user->pst_stat_count, &st_stat_info, pst_cb);
    }

    if (OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_tid_count)
    {
        pst_count_stat_tid_info = (dmac_stat_count_tid_stru*)(&pst_dmac_vap->pst_stat_count_tid[uc_tidno]);
        dmac_stat_calc_tx_mpdu_tid_num(pst_count_stat_tid_info, &st_stat_info, ul_sub_msdu_num);
        dmac_stat_calc_tx_mpdu_tid_bytes(pst_count_stat_tid_info, &st_stat_info, MAC_GET_CB_MPDU_LEN(pst_cb));
        if (FRW_EVENT_TYPE_HOST_DRX != MAC_GET_CB_EVENT_TYPE(pst_cb)&& OAL_PTR_NULL != pst_count_stat_tid_info)
        {
            pst_count_stat_tid_info->ul_forward_num++;
            pst_count_stat_tid_info->ul_forward_bytes += MAC_GET_CB_MPDU_NUM(pst_cb) * MAC_GET_CB_MPDU_LEN(pst_cb);
        }

    }
    if (OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_user_tid_count)
    {
        pst_count_stat_tid_info = (dmac_stat_count_tid_stru*)(&pst_dmac_user->pst_stat_count_tid[uc_tidno]);
        dmac_stat_calc_tx_mpdu_tid_num(pst_count_stat_tid_info, &st_stat_info, ul_sub_msdu_num);
    }

    return OAL_SUCC;
}


oal_uint32 dmac_user_stat_rx_info(frw_event_hdr_stru *pst_event_hdr, mac_vap_stru *pst_vap,
        dmac_rx_ctl_stru *pst_rx_ctrl, oal_uint8 uc_stat_lvl)
{
    dmac_user_stru              *pst_dmac_user      = OAL_PTR_NULL;
    oal_uint32                   ul_ret             = OAL_SUCC;
    oal_uint8                    uc_tidno           = 0;
    dmac_vap_stru               *pst_dmac_vap;
    dmac_device_stru            *pst_dmac_device;
    dmac_stat_count_tid_stru    *pst_count_stat_tid_info;

    /*获取vap 信息*/
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_RX, "{dmac_user_stat_rx_info::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*判断是否使能统计*/
    if (OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable)
    {
        return OAL_SUCC;
    }

    /* 获取Device结构体指针 */
    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_RX, "{dmac_user_stat_rx_info: pst dmac device null pointer! mac dev id:%d}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户、帧信息 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_RX_CB_TA_USER_IDX(&pst_rx_ctrl->st_rx_info));
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        //OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac_user_stat_rx_info::pst_dmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取接收报文的ti dno */
    ul_ret = dmac_stat_get_rx_tid(pst_vap, pst_rx_ctrl, &uc_tidno);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_RX, "{dmac_user_stat_rx_info::dmac_stat_get_rx_tid failed[%d].}", ul_ret);
        return ul_ret;
    }

    /*统计帧级别信息*/
    if (DMAC_STAT_FRM_LVL == uc_stat_lvl)
    {
        /*Frame Report*/
        if(OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_frm_rpt)
        {
            dmac_stat_calc_rcpi(pst_dmac_vap->pst_stat_frm_rpt, &(pst_rx_ctrl->st_rx_statistic));
            dmac_stat_calc_rcpi(pst_dmac_user->pst_stat_frm_rpt, &(pst_rx_ctrl->st_rx_statistic));
        }

        /*RSSI*/
        if (0 != pst_rx_ctrl->st_rx_statistic.c_rssi_dbm)
        {
            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_user_rate_info.st_dmac_rssi_stat_info.l_rx_rssi ,
                         pst_rx_ctrl->st_rx_statistic.c_rssi_dbm);
            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_user_rate_info.st_dmac_rssi_stat_info.us_rx_rssi_stat_count, 1);
        }
    }

    /*统计MPDU级别信息*/
    if (DMAC_STAT_MPDU_LVL == uc_stat_lvl)
    {
        /*吞吐量*/
        if(FRW_EVENT_TYPE_WLAN_DRX == pst_event_hdr->en_type)
        {
            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_dmac_thrpt_stat_info.ull_rx_bytes,
                         (oal_uint64)pst_rx_ctrl->st_rx_info.us_frame_len);
            DMAC_USER_STATS_PKT_INCR(pst_dmac_user->st_dmac_thrpt_stat_info.ul_rx_thrpt_stat_count, 1);
        }

        /*count统计*/
        if(OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_count)
        {
            dmac_stat_calc_rx_mpdu_num(pst_dmac_vap->pst_stat_count, pst_rx_ctrl);
            dmac_stat_calc_rx_mpdu_num(pst_dmac_device->pst_stat_count, pst_rx_ctrl);
            dmac_stat_calc_rx_mpdu_num(pst_dmac_user->pst_stat_count, pst_rx_ctrl);
        }

        /*count tid 统计*/
        if(OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_tid_count)
        {
            pst_count_stat_tid_info = (dmac_stat_count_tid_stru*)(&pst_dmac_vap->pst_stat_count_tid[uc_tidno]);
            dmac_stat_calc_rx_mpdu_tid_num(pst_count_stat_tid_info, pst_rx_ctrl);
        }
        if (OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_user_tid_count)
        {
            pst_count_stat_tid_info = (dmac_stat_count_tid_stru*)(&pst_dmac_user->pst_stat_count_tid[uc_tidno]);
            dmac_stat_calc_rx_mpdu_tid_num(pst_count_stat_tid_info, pst_rx_ctrl);
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_stat_rx_tid_fcs_error(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_hal_vap_id, dmac_rx_ctl_stru *pst_rx_ctrl)
{
    oal_uint32                   ul_ret             = OAL_SUCC;
    oal_uint8                    uc_tidno           = 0;
    dmac_vap_stru               *pst_dmac_vap;
    dmac_stat_count_tid_stru *pst_stat_count_tid;
    dmac_device_stru            *pst_dmac_device;
    mac_vap_stru                *pst_vap = OAL_PTR_NULL;
    hal_to_dmac_vap_stru       *pst_hal_vap  = OAL_PTR_NULL;

    if (OAL_PTR_NULL == pst_hal_device || OAL_PTR_NULL == pst_rx_ctrl)
    {
        OAM_ERROR_LOG0(uc_hal_vap_id, OAM_SF_RX, "{dmac_stat_tid_fcs_error::pst pointer null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((uc_hal_vap_id > WLAN_HAL_OHTER_BSS_ID) || (!HAL_VAP_ID_IS_VALID(uc_hal_vap_id)))
    {
        //OAM_WARNING_LOG2(0, OAM_SF_RX, "{dmac_rx_get_vap:invalid vap id=%d max=%d}", uc_hal_vap_id, WLAN_HAL_OHTER_BSS_ID);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }
    hal_get_hal_vap(pst_hal_device, uc_hal_vap_id, &pst_hal_vap);
    if (OAL_PTR_NULL == pst_hal_vap)
    {
        OAM_INFO_LOG1(0, OAM_SF_RX, "{dmac_stat_rx_tid_fcs_error:get vap faild, hal vap id=%d}", uc_hal_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_vap = (mac_vap_stru *)mac_res_get_dmac_vap(pst_hal_vap->uc_mac_vap_id);
    if (OAL_PTR_NULL == pst_vap)
    {
        OAM_ERROR_LOG0(uc_hal_vap_id, OAM_SF_RX, "{dmac_stat_rx_tid_fcs_error::pst_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /*获取vap 信息*/
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_RX, "{dmac_stat_tid_fcs_error::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }


    /*判断是否使能统计*/
    if (OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable || OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_count)
    {
        return OAL_SUCC;
    }

    /* 获取Device结构体指针 */
    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_RX, "{dmac_user_stat_rx_fcs_error_num: pst dmac device null pointer! mac dev id:%d}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(OAL_PTR_NULL == pst_dmac_vap->pst_stat_count
        || OAL_PTR_NULL == pst_dmac_device->pst_stat_count)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap->pst_stat_count->st_count_mpdu.ul_fcs_err_mpdu_num++;
    pst_dmac_device->pst_stat_count->st_count_mpdu.ul_fcs_err_mpdu_num++;

    /* 获取接收报文的ti dno */
    ul_ret = dmac_stat_get_rx_tid(pst_vap, pst_rx_ctrl, &uc_tidno);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_RX, "{dmac_stat_tid_fcs_error::dmac_stat_get_rx_tid failed[%d].}", ul_ret);
        return ul_ret;
    }


    /*count tid 统计*/
    if(OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_tid_count)
    {
        pst_stat_count_tid = (dmac_stat_count_tid_stru*)(&pst_dmac_vap->pst_stat_count_tid[uc_tidno]);
        if (OAL_PTR_NULL != pst_stat_count_tid)
        {
            pst_stat_count_tid->ul_rx_fail_num++;
            DMAC_VAP_STATS_PKT_INCR(pst_stat_count_tid->ul_rx_fail_bytes,
                   pst_rx_ctrl->st_rx_info.us_frame_len - pst_rx_ctrl->st_rx_info.uc_mac_header_len);
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_stat_rx_duplicate_num(dmac_user_stru *pst_dmac_user)
{
    dmac_vap_stru               *pst_dmac_vap;
    dmac_device_stru            *pst_dmac_device;

    /*获取vap 信息*/
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_stat_rx_duplicate_num::pst_dmac_vap[vap_id=%d] null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*判断是否使能统计*/
    if (OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable || OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_count)
    {
        return OAL_SUCC;
    }

    /* 获取Device结构体指针 */
    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_RX, "{dmac_stat_rx_duplicate_num: pst dmac device null pointer! mac dev id:%d}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if( OAL_PTR_NULL == pst_dmac_user->pst_stat_count
        ||OAL_PTR_NULL == pst_dmac_vap->pst_stat_count
        || OAL_PTR_NULL == pst_dmac_device->pst_stat_count)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_user->pst_stat_count->st_mac_stat.ul_rx_dup_frm_num++;
    pst_dmac_vap->pst_stat_count->st_mac_stat.ul_rx_dup_frm_num++;
    pst_dmac_device->pst_stat_count->st_mac_stat.ul_rx_dup_frm_num++;

    return OAL_SUCC;
}


oal_uint32  dmac_user_stat_tx_dropped_mpdu_num(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
    mac_device_stru *pst_mac_device, oal_uint8 uc_tidno)
{
    dmac_device_stru *pst_dmac_device;

    if ((OAL_PTR_NULL == pst_dmac_user) || (OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_mac_device))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
    /*判断是否使能统计*/
    if (OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable || OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_count)
    {
        return OAL_SUCC;
    }

    /*组播用户不统计*/
    if (OAL_TRUE == pst_dmac_user->st_user_base_info.en_is_multi_user)
    {
        return OAL_SUCC;
    }

    /*判断count统计结构体是否为空*/
    if ( OAL_PTR_NULL == pst_dmac_vap->pst_stat_count
         || OAL_PTR_NULL == pst_dmac_user->pst_stat_count
         || OAL_PTR_NULL == pst_dmac_device->pst_stat_count)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_user->pst_stat_count->aul_tx_dropped[uc_tidno]++;
    pst_dmac_vap->pst_stat_count->aul_tx_dropped[uc_tidno]++;
    pst_dmac_device->pst_stat_count->aul_tx_dropped[uc_tidno]++;

    return OAL_SUCC;
}


oal_uint32  dmac_user_stat_tid_delay(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
    mac_tx_ctl_stru *pst_tx_ctl,  oal_time_us_stru *pst_time)
{
    oal_uint32                      ul_diff_time_us;
    oal_uint16                      us_delay_ms;
    dmac_stat_tid_queue_delay_stru  *pst_stat_tid_queue_delay;
    oal_uint8                       uc_tidno = pst_tx_ctl->uc_tid;

    /*判断是否使能统计*/
    if ( OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable || OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_tsc_rpt)
    {
        return OAL_SUCC;
    }

    /*组播用户不统计*/
    if (OAL_TRUE == pst_dmac_user->st_user_base_info.en_is_multi_user)
    {
        return OAL_SUCC;
    }

    /*判断tsc统计结构体是否为空*/
    if ( OAL_PTR_NULL == pst_dmac_vap->pst_stat_tsc_rpt
         || OAL_PTR_NULL == pst_dmac_user->pst_stat_tsc_rpt)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 计算报文的等待时延 */
    ul_diff_time_us = (oal_uint32)OAL_TIME_GET_RUNTIME(MAC_GET_CB_TIMESTAMP(pst_tx_ctl), (oal_uint32)DMAC_TIME_USEC_INT64(pst_time));
    us_delay_ms = (oal_uint16)(ul_diff_time_us >> 10);

    /*更新USER时延统计*/
    pst_stat_tid_queue_delay  = &(pst_dmac_user->pst_stat_tsc_rpt->ast_tid_queue_delay[uc_tidno]);
    pst_stat_tid_queue_delay->ull_queue_delay_sum+= us_delay_ms;
    pst_stat_tid_queue_delay->ul_queue_delay_cnt++;

    /*更新VAP时延统计*/
    pst_stat_tid_queue_delay  = &(pst_dmac_vap->pst_stat_tsc_rpt->ast_tid_queue_delay[uc_tidno]);
    pst_stat_tid_queue_delay->ull_queue_delay_sum+= us_delay_ms;
    pst_stat_tid_queue_delay->ul_queue_delay_cnt++;

    return OAL_SUCC;
}


oal_uint32  dmac_user_stat_tx_delay(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_tidno,
        mac_tx_ctl_stru *pst_tx_ctl, oal_time_us_stru *pst_time)
{
    oal_uint32                              ul_diff_time_us;
    oal_uint16                              us_delay_ms;
    dmac_stat_tid_tx_delay_stru             *pst_stat_tid_tx_delay;

    /*判断是否使能统计*/
    if ( OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable || OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_tid_tx_delay)
    {
        return OAL_SUCC;
    }

    /*组播用户不统计*/
    if (OAL_TRUE == pst_dmac_user->st_user_base_info.en_is_multi_user)
    {
        return OAL_SUCC;
    }

    /*判断tsc统计结构体是否为空*/
    if ( OAL_PTR_NULL == pst_dmac_vap->pst_stat_tid_tx_delay
         || OAL_PTR_NULL == pst_dmac_user->pst_stat_tid_tx_delay)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 计算报文从入TID到发送成功的时延*/
    /* 获取系统时间 */
    ul_diff_time_us = (oal_uint32)OAL_TIME_GET_RUNTIME(MAC_GET_CB_TIMESTAMP(pst_tx_ctl), (oal_uint32)DMAC_TIME_USEC_INT64(pst_time));
    us_delay_ms = (oal_uint16)(ul_diff_time_us >> 10);

    /*入TID到发送成功时延*/
    pst_stat_tid_tx_delay = (dmac_stat_tid_tx_delay_stru*)&(pst_dmac_user->pst_stat_tid_tx_delay[uc_tidno]);
    dmac_stat_calc_tx_delay(pst_stat_tid_tx_delay, us_delay_ms);
    pst_stat_tid_tx_delay = (dmac_stat_tid_tx_delay_stru*)&(pst_dmac_vap->pst_stat_tid_tx_delay[uc_tidno]);
    dmac_stat_calc_tx_delay(pst_stat_tid_tx_delay, us_delay_ms);

    /*判断是否使能统计*/
    if ( !(pst_dmac_vap->st_stat_cap_flag.bit_tsc_rpt)
        || OAL_PTR_NULL == pst_dmac_vap->pst_stat_tsc_rpt
        || OAL_PTR_NULL == pst_dmac_user->pst_stat_tsc_rpt)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*统计时延直方图*/
    dmac_stat_calc_tx_delay_hist(&(pst_dmac_user->pst_stat_tsc_rpt->st_tid_tx_delay_hist), us_delay_ms, uc_tidno);
    dmac_stat_calc_tx_delay_hist(&(pst_dmac_vap->pst_stat_tsc_rpt->st_tid_tx_delay_hist), us_delay_ms, uc_tidno);

    return OAL_SUCC;

}

#define DMAC_STAT_INTERNAL_FUNC

OAL_STATIC oal_void dmac_stat_calc_tx_delay(dmac_stat_tid_tx_delay_stru *pst_stat_tid_delay, oal_uint16 us_delay_ms)
{
    /*时延累计值*/
    if(0 == pst_stat_tid_delay->ul_tx_delay_cnt)
    {
        pst_stat_tid_delay->ul_min_tx_delay = 0xFFFFFFFF;
    }

    pst_stat_tid_delay->ull_tx_delay_sum += us_delay_ms;
    pst_stat_tid_delay->ul_tx_delay_cnt++;

    /*时延极限值*/
    if(us_delay_ms > pst_stat_tid_delay->ul_max_tx_delay)
    {
        pst_stat_tid_delay->ul_max_tx_delay = us_delay_ms;
    }

    if ((0 != us_delay_ms) && (us_delay_ms < pst_stat_tid_delay->ul_min_tx_delay))
    {
        pst_stat_tid_delay->ul_min_tx_delay = us_delay_ms;
    }

}


OAL_STATIC oal_void dmac_stat_calc_tx_delay_hist(dmac_stat_tid_tx_delay_hist_stru *pst_stat_tid_delay_hist,
                        oal_uint16 us_delay_ms, oal_uint8 uc_tidno)
{
    oal_uint8                   uc_bin_idx;

    /*使能目标TID的TXDELAY直方图统计*/
    if (uc_tidno == pst_stat_tid_delay_hist->uc_tidno)
    {
        if(us_delay_ms >= pst_stat_tid_delay_hist->aus_hist_range[DMAC_STAT_TX_DELAY_HIST_BIN_NUM-1])
        {
            pst_stat_tid_delay_hist->auc_tx_delay_hist_bin[DMAC_STAT_TX_DELAY_HIST_BIN_NUM-1]++;
        }

        for ( uc_bin_idx = 0 ; uc_bin_idx < DMAC_STAT_TX_DELAY_HIST_BIN_NUM-1 ; uc_bin_idx++ )
        {
            if((us_delay_ms >= pst_stat_tid_delay_hist->aus_hist_range[uc_bin_idx])
               && (us_delay_ms < pst_stat_tid_delay_hist->aus_hist_range[uc_bin_idx+1]))
            {
                pst_stat_tid_delay_hist->auc_tx_delay_hist_bin[uc_bin_idx]++;
            }
        }
    }
}


OAL_STATIC oal_void dmac_stat_calc_rcpi(dmac_stat_frm_rpt_stru *pst_stat_frm_rpt, hal_rx_statistic_stru *pst_rx_statistic)
{
    if(OAL_PTR_NULL == pst_stat_frm_rpt)
    {
        return;
    }

    /*snr, rssi上报值为0时不统计*/
    if((0 == pst_rx_statistic->c_snr_ant0) || (0 == pst_rx_statistic->c_snr_ant1) || (0 == pst_rx_statistic->c_rssi_dbm))
    {
        return;
    }

    /*last rsni*/
    pst_stat_frm_rpt->uc_last_rsni= (oal_uint8)((pst_rx_statistic->c_snr_ant0 + pst_rx_statistic->c_snr_ant1)) >> 1;

    /*last rcpi*/
    pst_stat_frm_rpt->uc_last_rcpi = (oal_uint8)((oal_uint8)((pst_rx_statistic->c_rssi_dbm + 110)) << 1);

    pst_stat_frm_rpt->ul_sum_rcpi += pst_stat_frm_rpt->uc_last_rcpi ;
    pst_stat_frm_rpt->ul_rx_mag_data_frm_num++;

    /*用户avrg rcpi*/
    if(pst_stat_frm_rpt->ul_rx_mag_data_frm_num <= 32)
    {
        pst_stat_frm_rpt->uc_avrg_rcpi = (oal_uint8)(pst_stat_frm_rpt->ul_sum_rcpi / pst_stat_frm_rpt->ul_rx_mag_data_frm_num);
    }
    else
    {
        pst_stat_frm_rpt->ul_sum_rcpi = 0;
        pst_stat_frm_rpt->uc_avrg_rcpi =
            (pst_stat_frm_rpt->uc_avrg_rcpi*31+ pst_stat_frm_rpt->uc_last_rcpi)>>5;
    }
}


OAL_STATIC oal_void dmac_stat_calc_rx_mpdu_num(dmac_stat_count_stru *pst_stat_count, dmac_rx_ctl_stru *pst_rx_ctrl)
{
    mac_ieee80211_frame_stru *pst_frame_hdr;

    if ( OAL_PTR_NULL == pst_stat_count )
    {
        return;
    }

    /*dot11ReceivedFragmentCount, 分片的MPDU数目*/
    pst_stat_count->st_count_mpdu.ul_rx_frag_mpdu_num += (OAL_TRUE == pst_rx_ctrl->st_rx_info.bit_is_fragmented)? 1:0;

    pst_stat_count->st_count_mpdu.ul_rx_msdu_num += pst_rx_ctrl->st_rx_info.uc_msdu_in_buffer;

    /*dot11GroupReceivedFrameCount, 组播MSDU个数*/
    pst_frame_hdr     = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(&(pst_rx_ctrl->st_rx_info)));
    if (OAL_TRUE == ETHER_IS_MULTICAST(pst_frame_hdr->auc_address1))
    {
        pst_stat_count->st_count_mpdu.ul_rx_multicast_msdu_num += pst_rx_ctrl->st_rx_info.uc_msdu_in_buffer;
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
        pst_stat_count->st_count_mpdu.ul_rx_mcast_bytes += pst_rx_ctrl->st_rx_info.us_frame_len;
#endif
    }
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    if (pst_rx_ctrl->st_rx_statistic.un_nss_rate.st_ht_rate.bit_protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE)
    {
        oal_uint8 bit_ht_mcs = pst_rx_ctrl->st_rx_statistic.un_nss_rate.st_ht_rate.bit_ht_mcs;
        if (bit_ht_mcs <= 15)
        {
            pst_stat_count->st_count_mpdu.aul_sta_rx_mcs_cnt[bit_ht_mcs]++;
        }
    }
    else if (pst_rx_ctrl->st_rx_statistic.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE)
    {
        oal_uint8 bit_vht_mcs = pst_rx_ctrl->st_rx_statistic.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs;
        pst_stat_count->st_count_mpdu.aul_sta_rx_mcs_cnt[bit_vht_mcs]++;
    }
#endif
    if(pst_rx_ctrl->st_rx_info.bit_amsdu_enable)
    {
        /*dot11ReceivedAMSDUCount, 接收的AMSDU个数*/
        pst_stat_count->st_count_amsdu.ul_rx_num++;

        /*dot11ReceivedOctetsInAMSDUCount， 接收的AMSDU帧体字节数*/
        pst_stat_count->st_count_amsdu.ull_rx_octets_num +=
            pst_rx_ctrl->st_rx_info.us_frame_len - pst_rx_ctrl->st_rx_info.uc_mac_header_len;
    }

    if(pst_rx_ctrl->st_rx_status.bit_AMPDU)
    {
        /*dot11AMPDUReceivedCount */
        pst_stat_count->st_count_ampdu.ul_rx_num += (OAL_TRUE == pst_rx_ctrl->st_rx_info.bit_is_first_buffer)? 1:0;

        /*dot11MPDUInReceivedAMPDUCount */
        pst_stat_count->st_count_ampdu.ul_rx_mpdu_num++;

        /*dot11ReceivedOctetsInAMPDUCount*/
        pst_stat_count->st_count_ampdu.ull_rx_octets_num += pst_rx_ctrl->st_rx_info.us_frame_len;
    }
}


OAL_STATIC oal_void dmac_stat_calc_rx_mpdu_tid_num(dmac_stat_count_tid_stru *pst_stat_count_tid, dmac_rx_ctl_stru *pst_rx_ctrl)
{
    mac_ieee80211_frame_stru *pst_frame_hdr;

    if ( OAL_PTR_NULL == pst_stat_count_tid )
    {
        return;
    }

    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&pst_rx_ctrl->st_rx_info);
    /*dot11ReceivedFragmentCount, 分片的MPDU数目*/
    pst_stat_count_tid->ul_rx_frag_mpdu_num += (OAL_TRUE == pst_rx_ctrl->st_rx_info.bit_is_fragmented)? 1:0;

    /*dot11QosMPDUsReceivedCount,总接收到的MPDU数目*/
    pst_stat_count_tid->ul_rx_mpdu_num++;

    DMAC_VAP_STATS_PKT_INCR(pst_stat_count_tid->ul_rx_succ_bytes,
                   pst_rx_ctrl->st_rx_info.us_frame_len - pst_rx_ctrl->st_rx_info.uc_mac_header_len);
    /*dot11QosRetriesReceivedCount, 重传的MPDU数目*/
    pst_stat_count_tid->ul_rx_retry_mpdu_num += (OAL_TRUE == pst_frame_hdr->st_frame_control.bit_retry)?1:0;
}


OAL_STATIC oal_void dmac_stat_calc_tx_frm_num(dmac_stat_count_stru *pst_stat_count, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param, oal_uint8 bit_is_ampdu)
{
    if (OAL_PTR_NULL == pst_stat_count)
    {
        return;
    }

    /*dot11RTSSuccessCount*/
    DMAC_DEV_STATS_PKT_INCR(pst_stat_count->st_mac_stat.ul_rts_suc_num, pst_tx_dscr_param->uc_rts_succ);
    /*dot11RTSFailureCount*/
    DMAC_DEV_STATS_PKT_INCR(pst_stat_count->st_mac_stat.ul_rts_fail_num, pst_tx_dscr_param->uc_cts_failure);

    if(bit_is_ampdu)
    {
        /*dot11TransmittedAMPDUCount */
        pst_stat_count->st_count_ampdu.ul_tx_num            += 1;
        /*dot11TransmittedMPDUsInAMPDUCount */
        pst_stat_count->st_count_ampdu.ul_tx_mpdu_num       += pst_tx_dscr_param->uc_mpdu_num;
        /*dot11TransmittedOctetsInAMPDUCount */
        pst_stat_count->st_count_ampdu.ull_tx_octets_num    += pst_tx_dscr_param->uc_mpdu_num * pst_tx_dscr_param->us_mpdu_len;
    }
}


OAL_STATIC oal_void dmac_stat_calc_tx_mpdu_num( dmac_stat_count_stru *pst_stat_count, dmac_stat_count_common_stru *pst_stat_info, mac_tx_ctl_stru* pst_cb)
{
    oal_uint32 ul_sub_msdu_num;

   /*判断count统计结构体是否为空*/
    if ( OAL_PTR_NULL == pst_stat_count)
    {
        return;
    }

    ul_sub_msdu_num = (oal_uint32)MAC_GET_CB_NETBUF_NUM(pst_cb);

    pst_stat_count->st_count_mpdu.ul_tx_frag_mpdu_num               += pst_stat_info->ul_tx_frag_mpdu_num;
    pst_stat_count->st_count_mpdu.ul_tx_multicast_msdu_num          += pst_stat_info->ul_tx_multicast_mpdu_num * ul_sub_msdu_num;
    pst_stat_count->st_count_mpdu.ul_tx_succ_msdu_num               += pst_stat_info->ul_tx_succ_mpdu_num * ul_sub_msdu_num;
    pst_stat_count->st_count_mpdu.ul_tx_fail_msdu_num               += pst_stat_info->ul_tx_fail_mpdu_num;

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    if (pst_stat_info->ul_tx_multicast_mpdu_num > 0)
    {
        pst_stat_count->st_count_mpdu.ul_tx_mcast_bytes             += pst_cb->us_mpdu_payload_len + pst_cb->uc_frame_header_length;
    }
#endif
    pst_stat_count->st_mac_stat.ul_tx_retry_succ_msdu_num           += pst_stat_info->ul_tx_retry_succ_mpdu_num * ul_sub_msdu_num;
    pst_stat_count->st_mac_stat.ul_tx_multi_retry_succ_msud_num     += pst_stat_info->ul_tx_multi_retry_succ_mpdu_num * ul_sub_msdu_num;
    pst_stat_count->st_mac_stat.ul_ack_fail_mpdu_num                += pst_stat_info->ul_ack_fail_mpdu_num;

    if(pst_cb->bit_is_amsdu)
    {
        pst_stat_count->st_count_amsdu.ul_tx_succ_num             += pst_stat_info->ul_tx_succ_mpdu_num;
        pst_stat_count->st_count_amsdu.ul_tx_fail_num             += pst_stat_info->ul_tx_fail_mpdu_num;
        pst_stat_count->st_count_amsdu.ul_tx_retry_succ_num       += pst_stat_info->ul_tx_retry_succ_mpdu_num;
        pst_stat_count->st_count_amsdu.ul_tx_multi_retry_succ_num += pst_stat_info->ul_tx_multi_retry_succ_mpdu_num;
        pst_stat_count->st_count_amsdu.ull_tx_succ_octets_num     += pst_cb->us_mpdu_payload_len;
        pst_stat_count->st_count_amsdu.ul_ack_fail_num            += pst_stat_info->ul_ack_fail_mpdu_num;

    }
}



OAL_STATIC oal_void dmac_stat_calc_tx_mpdu_tid_num( dmac_stat_count_tid_stru *pst_stat_count_tid, dmac_stat_count_common_stru *pst_stat_info, oal_uint32 ul_sub_msdu_num)
{
    /*判断count tid统计结构体是否为空*/
    if ( OAL_PTR_NULL == pst_stat_count_tid)
    {
        return;
    }

    pst_stat_count_tid->ul_tx_frag_mpdu_num                += pst_stat_info->ul_tx_frag_mpdu_num;
    pst_stat_count_tid->ul_tx_fail_msdu_num                += pst_stat_info->ul_tx_fail_mpdu_num * ul_sub_msdu_num;
    pst_stat_count_tid->st_stat_mac_stat.ul_tx_retry_succ_msdu_num          += pst_stat_info->ul_tx_retry_succ_mpdu_num * ul_sub_msdu_num;
    pst_stat_count_tid->st_stat_mac_stat.ul_tx_multi_retry_succ_msud_num    += pst_stat_info->ul_tx_multi_retry_succ_mpdu_num * ul_sub_msdu_num;
    pst_stat_count_tid->st_stat_mac_stat.ul_ack_fail_mpdu_num               += pst_stat_info->ul_ack_fail_mpdu_num;
    pst_stat_count_tid->ul_tx_succ_msdu_num                += pst_stat_info->ul_tx_succ_mpdu_num * ul_sub_msdu_num;
    pst_stat_count_tid->ul_tx_discard_msdu_num             += pst_stat_info->ul_tx_discard_mpdu_num * ul_sub_msdu_num;
}



OAL_STATIC oal_void dmac_stat_calc_tx_mpdu_tid_bytes( dmac_stat_count_tid_stru *pst_stat_count_tid, dmac_stat_count_common_stru *pst_stat_info, oal_uint16 ul_len)
{
    /*判断count tid统计结构体是否为空*/
    if ( OAL_PTR_NULL == pst_stat_count_tid)
    {
        return;
    }

    pst_stat_count_tid->ul_tx_succ_bytes += (pst_stat_info->ul_tx_succ_mpdu_num + pst_stat_info->ul_tx_retry_succ_mpdu_num
                                            + pst_stat_info->ul_tx_multi_retry_succ_mpdu_num) * ul_len;
    pst_stat_count_tid->ul_tx_fail_bytes += pst_stat_info->ul_tx_fail_mpdu_num * ul_len;
}


OAL_STATIC oal_uint32 dmac_stat_get_node_info(dmac_stat_node_stru *pst_stat_node, mac_stat_type_enum_uint8 en_stat_type)
{
    mac_user_stru                   *pst_user;
    dmac_user_stru                  *pst_dmac_user;

    /* 根据统计类型更新统计量 */
    switch (en_stat_type)
    {
        case MAC_STAT_TYPE_USER_BSD:
                pst_user = (mac_user_stru*)pst_stat_node->pst_stat_param->p_void;

                /* 获取用户、帧信息 */
                pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_user->us_assoc_id);
                if (OAL_PTR_NULL == pst_dmac_user)
                {
                    OAM_WARNING_LOG1(0, OAM_SF_ANY,
                        "{dmac_stat_get_node_info::pst_dmac_user[%d] null.}", pst_user->us_assoc_id);
                    return OAL_ERR_CODE_PTR_NULL;
                }

                pst_stat_node->aul_stat_sum[DMAC_STAT_BSD_TX_THRPT] = (oal_uint32)pst_dmac_user->st_dmac_thrpt_stat_info.ull_tx_bytes;
                pst_stat_node->aul_stat_sum[DMAC_STAT_BSD_RX_THRPT] = (oal_uint32)pst_dmac_user->st_dmac_thrpt_stat_info.ull_rx_bytes;
                pst_stat_node->aul_stat_sum[DMAC_STAT_BSD_RX_RSSI]  = (oal_uint32)(-pst_dmac_user->st_user_rate_info.st_dmac_rssi_stat_info.l_rx_rssi);

                pst_stat_node->aul_stat_cnt[DMAC_STAT_BSD_TX_THRPT] = pst_dmac_user->st_dmac_thrpt_stat_info.ul_tx_thrpt_stat_count;
                pst_stat_node->aul_stat_cnt[DMAC_STAT_BSD_RX_THRPT] = pst_dmac_user->st_dmac_thrpt_stat_info.ul_rx_thrpt_stat_count;
                pst_stat_node->aul_stat_cnt[DMAC_STAT_BSD_RX_RSSI]  = pst_dmac_user->st_user_rate_info.st_dmac_rssi_stat_info.us_rx_rssi_stat_count;

            break;
        default:
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_init_node_info::invalid en_stat_type.}");

            return OAL_FAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_stat_query_timer_handler(oal_void * p_void)
{
    dmac_stat_param_stru    *pst_stat_param     = OAL_PTR_NULL;
    dmac_stat_node_stru     *pst_stat_node      = OAL_PTR_NULL;
    dmac_stat_stru          *pst_stat           = OAL_PTR_NULL;
    oal_uint32               ul_index           = 0;
    oal_uint8                uc_cnt             = 0;
    dmac_stat_node_stru      st_stat_node_begin;

    if (OAL_PTR_NULL == p_void)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_query_timer_handler::p_void null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_stat_param = (dmac_stat_param_stru *)(p_void);
    pst_stat       = &(g_ast_pfm_stat[pst_stat_param->en_stat_type]);

    /* 查找统计节点 */
    pst_stat_node = dmac_stat_search_node(&(pst_stat->st_stat_node_dlist), pst_stat_param->en_module_id, pst_stat_param->p_void);
    if (OAL_PTR_NULL == pst_stat_node)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_query_timer_handler::pst_stat_node null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新统计量 */
    pst_stat_node->us_curr_item++;

    oal_memcopy(&(st_stat_node_begin), pst_stat_node, sizeof(dmac_stat_node_stru));

    dmac_stat_get_node_info(pst_stat_node, pst_stat_param->en_stat_type);

    /* 根据统计类型更新统计量 */
    switch (pst_stat_param->en_stat_type)
    {
        case MAC_STAT_TYPE_USER_BSD:
            if(0 != pst_stat_node->st_timer.ul_timeout)
            {
                pst_stat_param->aul_stat_avg[DMAC_STAT_BSD_TX_THRPT]     = DMAC_STAT_CALC_THRPT(&(st_stat_node_begin), pst_stat_node, DMAC_STAT_BSD_TX_THRPT);
                pst_stat_param->aul_stat_avg[DMAC_STAT_BSD_RX_THRPT]     = DMAC_STAT_CALC_THRPT(&(st_stat_node_begin), pst_stat_node, DMAC_STAT_BSD_RX_THRPT);
                pst_stat_param->aul_stat_avg[DMAC_STAT_BSD_RX_RSSI]      = DMAC_STAT_CALC_RSSI(&(st_stat_node_begin), pst_stat_node, DMAC_STAT_BSD_RX_RSSI);

                OAM_WARNING_LOG3(0, OAM_SF_ANY, "dmac_stat_query_timer_handler: TX_THRPT=%u,RX_THRPT=%u,RX_RSSI=%d",
                pst_stat_param->aul_stat_avg[DMAC_STAT_BSD_TX_THRPT],
                pst_stat_param->aul_stat_avg[DMAC_STAT_BSD_RX_THRPT],
                -(oal_int32)(pst_stat_param->aul_stat_avg[DMAC_STAT_BSD_RX_RSSI]));
            }
            else
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_stat_query_timer_handler: Error! 0 == timeout!");
                pst_stat_param->aul_stat_avg[DMAC_STAT_BSD_TX_THRPT] = 0;
                pst_stat_param->aul_stat_avg[DMAC_STAT_BSD_RX_THRPT] = 0;
                pst_stat_param->aul_stat_avg[DMAC_STAT_BSD_RX_RSSI] = 0;
            }
            break;

        default:
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_stat_query_timer_handler::invalid en_stat_type.}");

            return OAL_FAIL;
    }

    /* 如果为命令配置模块，则更新至内部存储空间 */
    if (MAC_STAT_MODULE_CMD == pst_stat_param->en_module_id)
    {
        ul_index = pst_stat_node->us_curr_item - 1;

        if ((MAC_STAT_TYPE_TID_THRPT == pst_stat_param->en_stat_type)
            ||(MAC_STAT_TYPE_USER_THRPT == pst_stat_param->en_stat_type)
            ||(MAC_STAT_TYPE_VAP_THRPT == pst_stat_param->en_stat_type)
            ||(MAC_STAT_TYPE_USER_BSD == pst_stat_param->en_stat_type))
        {
            for (uc_cnt = 0; uc_cnt <= DMAC_STAT_BOTH; uc_cnt++)
            {
                *(pst_stat_node->pul_stat_avg + ul_index + uc_cnt * DMAC_STAT_ITEM_LIMIT) = pst_stat_param->aul_stat_avg[uc_cnt];
            }
        }
        else if (MAC_STAT_TYPE_TID_PER == pst_stat_param->en_stat_type)
        {
            for (uc_cnt = 0; uc_cnt < DMAC_STAT_PER_BUTT; uc_cnt++)
            {
                *(pst_stat_node->pul_stat_avg + ul_index + uc_cnt * DMAC_STAT_ITEM_LIMIT) = pst_stat_param->aul_stat_avg[uc_cnt];
            }
        }
        else
        {
            *(pst_stat_node->pul_stat_avg + ul_index) = pst_stat_param->aul_stat_avg[0];
        }
    }
    else    /* 调用相应的处理函数 */
    {
        pst_stat_node->p_inner_func(pst_stat_param);
    }

    /* 统计结束 */
    if( 0 != pst_stat_node->us_total_item)
    {
        if (pst_stat_node->us_curr_item >= pst_stat_node->us_total_item)
        {
            frw_timer_stop_timer(&(pst_stat_node->st_timer));
            pst_stat_node->uc_stat_flag = OAL_FALSE;
            return OAL_SUCC;
        }
    }

    return OAL_SUCC;
}

#endif

#if defined _PRE_WLAN_PRODUCT_1151V200 && defined _PRE_WLAN_RX_DSCR_TRAILER

oal_void dmac_stat_update_ant_rssi(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_hal_vap_id, hal_rx_statistic_stru st_rx_stat)
{
    oal_bool_enum_uint8 en_on = OAL_FALSE;
    mac_device_stru         *pst_mac_device = OAL_PTR_NULL;
    oal_int16           s_rssi_ant0 = 0;
    oal_int16           s_rssi_ant1 = 0;

    if (OAL_PTR_NULL == pst_hal_device )
    {
        OAM_ERROR_LOG0(uc_hal_vap_id, OAM_SF_RX, "{dmac_stat_update_ant_rssi::pst pointer null.}");
        return;
    }

    hal_get_ant_rssi_rep_sw(pst_hal_device, &en_on);
    if (OAL_FALSE == en_on)
    {
        OAM_INFO_LOG2(0, OAM_SF_RX, "{dmac_stat_update_ant_rssi:vap: %d update switch not enable =%d}", uc_hal_vap_id, en_on);
        return;
    }
    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);

    /* 设备未初始化 */
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{dmac_stat_update_ant_rssi::device is null}");
        return;
    }

    s_rssi_ant0 = pst_hal_device->s_rssi_ant0;
    s_rssi_ant1 = pst_hal_device->s_rssi_ant1;
    OAM_INFO_LOG2(0, OAM_SF_RX, "dmac_stat_update_ant_rssi::ant0_old: %d, ant1_old: %d", s_rssi_ant0, s_rssi_ant1);/*lint !e571*/
    if (WLAN_BAND_5G == pst_mac_device->en_band_cap)
    {
        oal_rssi_smooth(&s_rssi_ant0, (oal_int8)((st_rx_stat.s_rssi_ant0 + 2) / 4 + 3));
        oal_rssi_smooth(&s_rssi_ant1, (oal_int8)((st_rx_stat.s_rssi_ant1 + 2) / 4 + 3));
    }
    else
    {
        oal_rssi_smooth(&s_rssi_ant0, (oal_int8)((st_rx_stat.s_rssi_ant0 + 2) / 4 + 2));
        oal_rssi_smooth(&s_rssi_ant1, (oal_int8)((st_rx_stat.s_rssi_ant1 + 2) / 4 + 2));
    }
    hal_update_ant_rssi_value(pst_hal_device, s_rssi_ant0, s_rssi_ant1);
    OAM_INFO_LOG4(uc_hal_vap_id, OAM_SF_RX, "{dmac_stat_update_ant_rssi:ant0 raw: %d, ant1 raw: %d, ant0_smth: %d, ant1_smth: %d}",
        st_rx_stat.s_rssi_ant0, st_rx_stat.s_rssi_ant1, s_rssi_ant0, s_rssi_ant1);/*lint !e571*/
}
#endif

/*lint -e19*/
oal_module_symbol(dmac_stat_register);
oal_module_symbol(dmac_stat_unregister);
oal_module_symbol(dmac_stat_start);
oal_module_symbol(dmac_stat_stop);
oal_module_symbol(dmac_stat_tid_per);
#ifdef _PRE_WLAN_11K_STAT
oal_module_symbol(dmac_user_stat_tx_dropped_mpdu_num);
#endif
/*lint +e19*/

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

