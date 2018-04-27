


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "dmac_ext_if.h"
#include "mac_vap.h"
#include "dmac_user.h"
#include "dmac_main.h"
#include "oal_net.h"
#include "dmac_tx_qos_enhance.h"
#include "dmac_rx_data.h"
#include "dmac_stat.h"



#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TX_QOS_ENHANCE_C

/*****************************************************************************
  2 函数原型声明
*****************************************************************************/

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/

/*****************************************************************************
  4 函数实现
*****************************************************************************/

oal_uint32 dmac_tx_add_qos_enhance_list(mac_vap_stru *pst_mac_vap ,oal_uint8 *puc_sta_member_addr)
{
    mac_qos_enhance_sta_stru         *pst_qos_enhance_sta = OAL_PTR_NULL;
    mac_qos_enhance_stru             *pst_qos_enhance      = (mac_qos_enhance_stru *)(&pst_mac_vap->st_qos_enhance);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_qos_enhance))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{dmac_tx_add_qos_enhance_list::pst_qos_enhance null.}");
        return OAL_FAIL;
    }

    /* 对链表操作前加锁 */
    oal_spin_lock(&(pst_qos_enhance->st_lock));

    /* 判断STA是否在qos_enhance链表中 */
    pst_qos_enhance_sta = mac_tx_find_qos_enhance_list(pst_mac_vap, puc_sta_member_addr);

    /* STA不在qos_enhance链表中 */
    if (OAL_PTR_NULL == pst_qos_enhance_sta)
    {
        if (pst_qos_enhance->uc_qos_enhance_sta_count >= WLAN_ASSOC_USER_MAX_NUM)
        {
            /* 解锁 */
            oal_spin_unlock(&(pst_qos_enhance->st_lock));
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{dmac_tx_add_qos_enhance_list::pst_qos_enhance->uc_qos_enhance_sta_count is [%d].}",pst_qos_enhance->uc_qos_enhance_sta_count);
            return OAL_FAIL;
        }
        /* 创建qos_enhance_sta节点 */
        pst_qos_enhance_sta = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_qos_enhance_sta_stru), OAL_TRUE);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_qos_enhance_sta))
        {
            /* 解锁 */
            oal_spin_unlock(&(pst_qos_enhance->st_lock));
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{dmac_tx_add_qos_enhance_list::pst_qos_enhance_hash null.}");
            return OAL_FAIL;
        }

        oal_memset(pst_qos_enhance_sta, 0x0, OAL_SIZEOF(mac_qos_enhance_sta_stru));
        oal_set_mac_addr(pst_qos_enhance_sta->auc_qos_enhance_mac, puc_sta_member_addr);
        oal_dlist_add_tail(&(pst_qos_enhance_sta->st_qos_enhance_entry), &(pst_qos_enhance->st_list_head));
        pst_qos_enhance->uc_qos_enhance_sta_count++;

    }
    else /* STA在qos_enhance链表中 */
    {
        /* 次数小于qos_enhance链表加入门限值 */
        if (pst_qos_enhance_sta->uc_add_num < MAC_QOS_ENHANCE_ADD_NUM)
        {
            pst_qos_enhance_sta->uc_add_num++;
            pst_qos_enhance_sta->uc_delete_num = 0;
        }
    }
    /* 解锁 */
    oal_spin_unlock(&(pst_qos_enhance->st_lock));

    return OAL_SUCC;
}


oal_void dmac_tx_delete_qos_enhance_sta(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_sta_member_addr, oal_uint8 uc_flag)
{
    mac_qos_enhance_stru             *pst_qos_enhance = (mac_qos_enhance_stru *)(&pst_mac_vap->st_qos_enhance);
    mac_qos_enhance_sta_stru         *pst_qos_enhance_sta = OAL_PTR_NULL;
    oal_dlist_head_stru              *pst_sta_list_entry;
    oal_dlist_head_stru              *pst_sta_list_entry_temp;

    if (OAL_PTR_NULL == pst_qos_enhance)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{dmac_tx_delete_qos_enhance_sta::pst_qos_enhance null.}");
        return;
    }

    if (0 == pst_qos_enhance->uc_qos_enhance_sta_count)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{dmac_tx_delete_qos_enhance_sta::no sta in qos list while delete.}");
        return ;
    }

    /* 对链表操作前加锁 */
    oal_spin_lock(&(pst_qos_enhance->st_lock));

    /* 遍历qos_enhance表，找到地址匹配的STA */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_sta_list_entry, pst_sta_list_entry_temp, &(pst_qos_enhance->st_list_head))
    {
        pst_qos_enhance_sta = OAL_DLIST_GET_ENTRY(pst_sta_list_entry, mac_qos_enhance_sta_stru, st_qos_enhance_entry);

        if (!oal_compare_mac_addr(puc_sta_member_addr, pst_qos_enhance_sta->auc_qos_enhance_mac))
        {
            if (uc_flag)
            {
                /* 次数>=qos_enhance链表删除门限值，将STA从表中删除 */
                if (pst_qos_enhance_sta->uc_delete_num >= MAC_QOS_ENHANCE_QUIT_NUM)
                {
                    oal_dlist_delete_entry(&(pst_qos_enhance_sta->st_qos_enhance_entry));
                    OAL_MEM_FREE(pst_qos_enhance_sta, OAL_TRUE);
                    pst_qos_enhance->uc_qos_enhance_sta_count--;
                }
                else /* 次数小于qos_enhance链表删除门限值，次数累加 */
                {
                    pst_qos_enhance_sta->uc_add_num = 0;
                    pst_qos_enhance_sta->uc_delete_num++;
                }
            }
            else /* 删除STA后，清除qos list */
            {
                oal_dlist_delete_entry(&(pst_qos_enhance_sta->st_qos_enhance_entry));
                OAL_MEM_FREE(pst_qos_enhance_sta, OAL_TRUE);
                pst_qos_enhance->uc_qos_enhance_sta_count--;
            }
        }
    }

    /* 释放锁 */
    oal_spin_unlock(&(pst_qos_enhance->st_lock));
}


oal_uint32 dmac_tx_qos_enhance_add_or_delete(oal_int8 c_rx_rssi, oal_uint32 ul_tx_rate)
{
    oal_uint32                        ul_mid_interval = 0;

    ul_mid_interval = ((DMAC_QOS_ENHANCE_TP_HIGH_THD - DMAC_QOS_ENHANCE_TP_MID_THD)/(DMAC_QOS_ENHANCE_RSSI_HIGH_THD - DMAC_QOS_ENHANCE_RSSI_LOW_THD));

    /* STA的rssi强度为强等级 */
    if (c_rx_rssi > DMAC_QOS_ENHANCE_RSSI_HIGH_THD)
    {
        /* 低速率 */
        if (ul_tx_rate < DMAC_QOS_ENHANCE_TP_HIGH_THD)
        {
            return OAL_TRUE;
        }
        /* 高速率 */
        else
        {
            return OAL_FALSE;
        }
    }
    /* STA的rssi强度为弱等级 */
    else if (c_rx_rssi < DMAC_QOS_ENHANCE_RSSI_LOW_THD)
    {
        //STA的rssi强度大于边界rssi
        if(c_rx_rssi > DMAC_QOS_ENHANCE_RSSI_EDGE_THD)
        {
            /* 低速率 */
            if (ul_tx_rate < DMAC_QOS_ENHANCE_TP_LOW_THD)
            {
                return OAL_TRUE;
            }
            /* 高速率 */
            else
            {
                return OAL_FALSE;
            }
        }
        /* 强度小于边界rssi */
        else
        {
            return OAL_FALSE;
        }
    }
    /* STA的rssi强度为中等级 */
    else
    {
        /* 低速率 */
        if (ul_tx_rate < ul_mid_interval*(c_rx_rssi - DMAC_QOS_ENHANCE_RSSI_LOW_THD) + DMAC_QOS_ENHANCE_TP_MID_THD)
        {
            return OAL_TRUE;
        }
        /* 高速率 */
        else
        {
            return OAL_FALSE;
        }
    }


}


oal_uint32 dmac_tx_qos_enhance_proc(mac_vap_stru *pst_mac_vap)
{
    mac_user_stru                    *pst_mac_user = OAL_PTR_NULL;
    dmac_user_stru                   *pst_dmac_user = OAL_PTR_NULL;
    oal_dlist_head_stru              *pst_entry;
    oal_uint32                        ul_tx_rate;
    oal_uint32                        ul_ret = OAL_SUCC;

    /* 入参判断 */
    if ((OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG1(0, OAM_SF_QOS, "{dmac_tx_qos_enhance_proc::param null, pst_mac_vap=%p}", pst_mac_vap);
        return OAL_FAIL;
    }

    /* vap下挂STA个数<2,不进行qos enhance判断 */
    if (pst_mac_vap->us_user_nums < 2)
    {
        OAM_WARNING_LOG1(0, OAM_SF_QOS, "{dmac_tx_qos_enhance_proc:associate user num is %d}",pst_mac_vap->us_user_nums);
        return OAL_FAIL;
    }


    /* 遍历vap下挂STA列表 */
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
    {
        pst_mac_user = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

        /*lint -save -e774 */
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{dmac_tx_qos_enhance_proc:pst_user null pointer.}");
            continue;
        }
        /*lint -restore */

        pst_dmac_user = MAC_GET_DMAC_USER(pst_mac_user);

        /* 获取AP到STA间的TX吞吐量 */
        ul_tx_rate = dmac_stat_tx_sta_thrpt(pst_mac_user);

        if (dmac_tx_qos_enhance_add_or_delete(oal_get_real_rssi(pst_dmac_user->s_rx_rssi), ul_tx_rate))
        {
            /* 加入qos enhance链表 */
            ul_ret = dmac_tx_add_qos_enhance_list(pst_mac_vap, pst_mac_user->auc_user_mac_addr);
            if (OAL_UNLIKELY(OAL_FAIL == ul_ret))
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{dmac_tx_qos_enhance_proc::add_qos_enhance_list fail.}");
                return OAL_FAIL;
            }
        }
        else
        {
            /* 从qos enhance链表中删除 */
            dmac_tx_delete_qos_enhance_sta(pst_mac_vap, pst_mac_user->auc_user_mac_addr, OAL_TRUE);
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{dmac_tx_qos_enhance_proc::delete qos enhance sta.}");
        }

        if (oal_get_real_rssi(pst_dmac_user->s_rx_rssi) > DMAC_QOS_ENHANCE_RSSI_HIGH_THD)
        {
            pst_mac_user->en_qos_enhance_sta_state = MAC_USER_QOS_ENHANCE_NEAR;
        }
        else if (oal_get_real_rssi(pst_dmac_user->s_rx_rssi) < DMAC_QOS_ENHANCE_RSSI_LOW_THD)
        {
            pst_mac_user->en_qos_enhance_sta_state = MAC_USER_QOS_ENHANCE_FAR;
        }
    }
    return OAL_SUCC;

}


oal_uint32 dmac_tx_qos_enhance_time_fn(oal_void *p_arg)
{
    mac_vap_stru      *pst_mac_vap     = (mac_vap_stru *)p_arg;
    oal_uint32         ul_ret           =  OAL_SUCC;

    ul_ret = dmac_tx_qos_enhance_proc(pst_mac_vap);
    return ul_ret;
}


oal_void dmac_tx_qos_enhance_attach(dmac_vap_stru *pst_dmac_vap)
{
    /* 启动定时器 */
    FRW_TIMER_CREATE_TIMER(&(pst_dmac_vap->st_qos_enhance_timer),
                           dmac_tx_qos_enhance_time_fn,
                           DMAC_DEF_QOS_ENHANCE_TIMER,
                           (oal_void *)(&pst_dmac_vap->st_vap_base_info),
                           OAL_TRUE,
                           OAM_MODULE_ID_DMAC,
                           pst_dmac_vap->st_vap_base_info.ul_core_id);
}


oal_void dmac_tx_qos_enhance_detach(dmac_vap_stru *pst_dmac_vap)
{
    /* 关闭定时器 */
    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_dmac_vap->st_qos_enhance_timer));
}

/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_tx_qos_enhance_attach);
oal_module_symbol(dmac_tx_qos_enhance_detach);
oal_module_symbol(dmac_tx_delete_qos_enhance_sta);
/*lint +e578*//*lint +e19*/
#endif /* _PRE_WLAN_FEATURE_QOS_ENHANCE */


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

