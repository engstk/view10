


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_vap.h"
#include "dmac_11i.h"
#include "dmac_user_extend.h"
#include "dmac_alg_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_USER_EXTEND_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint32  dmac_user_active_timer(void *p_arg)
{
    mac_chip_stru       *pst_mac_chip = (mac_chip_stru *)p_arg;
    mac_device_stru     *pst_mac_device;
    oal_uint8            uc_device_idx;
    oal_uint8            uc_vap_idx;
    mac_vap_stru        *pst_mac_vap;
    oal_dlist_head_stru *pst_entry;
    mac_user_stru       *pst_user_tmp;
    dmac_user_stru      *pst_dmac_user_tmp;
    oal_uint32           ul_present_time;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_chip))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_user_active_timer::pst_mac_chip null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_present_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    for (uc_device_idx = 0; uc_device_idx < pst_mac_chip->uc_device_nums; uc_device_idx++)
    {
        pst_mac_device = mac_res_get_dev(pst_mac_chip->auc_device_id[uc_device_idx]);
        if (OAL_PTR_NULL == pst_mac_device)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_user_active_timer::mac_res_get_dev id[%d] NULL}",pst_mac_chip->auc_device_id[uc_device_idx]);
            continue;
        }

        /* 遍历device下所有用户，对超过活跃时间的用户作非活跃处理 */
        /* 业务vap从1开始 */
        for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
        {
            pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
            if (OAL_PTR_NULL == pst_mac_vap)
            {
                OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_user_active_timer::pst_mac_vap null.");
                return OAL_ERR_CODE_PTR_NULL;
            }

            /* 活跃用户管理只针对AP模式，非AP模式则跳出 */
            if (WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)
            {
                continue;
            }

            OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
            {
                pst_user_tmp      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
                if (OAL_PTR_NULL == pst_user_tmp)
                {
                    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_user_active_timer::pst_user_tmp null.");
                    return OAL_ERR_CODE_PTR_NULL;
                }

                pst_dmac_user_tmp = MAC_GET_DMAC_USER(pst_user_tmp);

                if ((oal_uint32)OAL_TIME_GET_RUNTIME(pst_dmac_user_tmp->ul_last_active_timestamp, ul_present_time) > WLAN_USER_ACTIVE_TO_INACTIVE_TIME)
                {
                    dmac_user_inactive(pst_dmac_user_tmp);
                }
            }
        }
    }

    /* 如果活跃用户小于规格-1，关闭转非活跃定时器 */
    if (pst_mac_chip->uc_active_user_cnt + 1 < mac_chip_get_max_active_user())
    {
        FRW_TIMER_DESTROY_TIMER(&(pst_mac_chip->st_active_user_timer));
    }

    return OAL_SUCC;
}


oal_uint32 dmac_user_update_active_user_dlist(dmac_user_stru *pst_dmac_user)
{
    mac_chip_stru          *pst_mac_chip        = OAL_PTR_NULL;
    oal_dlist_head_stru    *pst_dlist_head;

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG0(0, OAM_SF_USER_EXTEND, "dmac_user_update_active_user_dlist::pst_dmac_user null.");
        return OAL_ERR_CODE_PTR_NULL;
    }
    OAM_INFO_LOG4(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND,
        "dmac_user_update_active_user_dlist::entry. user[%02x-xx-xx-xx-%02x-%02x], active=%d.",
        pst_dmac_user->st_user_base_info.auc_user_mac_addr[0],
        pst_dmac_user->st_user_base_info.auc_user_mac_addr[4],
        pst_dmac_user->st_user_base_info.auc_user_mac_addr[5],
        pst_dmac_user->bit_active_user);

    pst_mac_chip = mac_res_get_mac_chip(pst_dmac_user->st_user_base_info.uc_chip_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_chip))
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND, "dmac_user_update_active_user_dlist::pst_mac_chip ptr null.");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 未使能 */
    if (!pst_mac_chip->st_user_extend.en_flag)
    {
        return OAL_SUCC;
    }

    /* dlist nodes must be active ucast users. */
    if (!pst_dmac_user->bit_active_user
        || pst_dmac_user->st_user_base_info.en_is_multi_user)
    {
        OAM_INFO_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND, "dmac_user_update_active_user_dlist::user not active or is multi user, return.");
        return OAL_SUCC;
    }

    /* add spinlock if nessary:lock */

    /* move to dlist head */
    pst_dlist_head = &pst_mac_chip->st_user_extend.st_active_user_list_head;
    oal_dlist_delete_entry(&pst_dmac_user->st_user_base_info.st_active_user_dlist_entry);
    oal_dlist_add_head(&pst_dmac_user->st_user_base_info.st_active_user_dlist_entry, pst_dlist_head);

    /* add spinlock if nessary:unlock */

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_PRODUCT_1151V200

OAL_STATIC oal_uint32 dmac_user_add_machw_peer_resp_dis(dmac_user_stru *pst_dmac_user)
{
    hal_peer_resp_dis_cfg_stru      st_cfg;
    hal_to_dmac_device_stru        *pst_hal_device;

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取hal device */
    pst_hal_device = dmac_user_get_hal_device((mac_user_stru *)pst_dmac_user);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND, "dmac_user_add_machw_peer_resp_dis::pst_hal_device ptr null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_cfg.uc_lut_index = pst_dmac_user->uc_lut_index;
    st_cfg.uc_peer_resp_dis = pst_dmac_user->bit_peer_resp_dis;

    /* 获取peer resp dis */
    hal_get_peer_resp_dis(pst_hal_device, &st_cfg);

    return OAL_SUCC;
}
#endif


OAL_STATIC dmac_user_stru * dmac_user_get_candidate_user(mac_chip_stru *pst_mac_chip)
{
    dmac_user_stru         *pst_dmac_user_candi = OAL_PTR_NULL;

    if (OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_WARNING_LOG0(0, OAM_SF_USER_EXTEND, "dmac_user_get_candidate_user::pst_mac_chip null.");
        return OAL_PTR_NULL;
    }

    /* 直接获取chip下活跃用户链表的尾节点 */
    if (oal_dlist_is_empty(&pst_mac_chip->st_user_extend.st_active_user_list_head))
    {
        OAM_ERROR_LOG0(0, OAM_SF_USER_EXTEND, "dmac_user_get_candidate_user::chip active_user_dlist is empty, should not be here!!");
        return OAL_PTR_NULL;
    }

    pst_dmac_user_candi = (dmac_user_stru *)OAL_DLIST_GET_ENTRY(pst_mac_chip->st_user_extend.st_active_user_list_head.pst_prev, mac_user_stru, st_active_user_dlist_entry);
    if (OAL_PTR_NULL == pst_dmac_user_candi)
    {
        OAM_WARNING_LOG0(0, OAM_SF_USER_EXTEND, "dmac_user_get_candidate_user::pst_user_tmp null.");
        return OAL_PTR_NULL;
    }

    OAM_INFO_LOG3(0, OAM_SF_USER_EXTEND, "dmac_user_get_candidate_user::candi user_idx=%d, lut_idx=%d, belong_vap_id=%d.",
        pst_dmac_user_candi->st_user_base_info.us_assoc_id,
        pst_dmac_user_candi->uc_lut_index,
        pst_dmac_user_candi->st_user_base_info.uc_vap_id);
    OAM_INFO_LOG4(0, OAM_SF_USER_EXTEND, "dmac_user_get_candidate_user::candi user mac addr[%02x-xx-xx-%02x-%02x-%02x].",
        pst_dmac_user_candi->st_user_base_info.auc_user_mac_addr[0],
        pst_dmac_user_candi->st_user_base_info.auc_user_mac_addr[3],
        pst_dmac_user_candi->st_user_base_info.auc_user_mac_addr[4],
        pst_dmac_user_candi->st_user_base_info.auc_user_mac_addr[5]);

    return pst_dmac_user_candi;
}


OAL_STATIC oal_void dmac_user_save_candidate_pn(dmac_user_stru *pst_dmac_user_candi)
{
    hal_pn_lut_cfg_stru         st_pn_lut_cfg;
    hal_to_dmac_device_stru    *pst_hal_device;
    oal_uint8                   uc_tid_loop;

    if (OAL_PTR_NULL == pst_dmac_user_candi)
    {
        OAM_ERROR_LOG0(0, OAM_SF_USER_EXTEND, "dmac_user_save_candidate_latest_pn::pst_dmac_user_candi null.");
        return;
    }

    /* 获取user隶属vap所在的hal device */
    pst_hal_device = dmac_user_get_hal_device((mac_user_stru *)pst_dmac_user_candi);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_dmac_user_candi->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND, "dmac_user_save_candidate_pn::pst_hal_device ptr null.");
        return;
    }

    /* 单播tx pn */
    {
        /* 读取寄存器中的值 */
        st_pn_lut_cfg.uc_pn_key_type = 1;
        st_pn_lut_cfg.uc_pn_peer_idx = pst_dmac_user_candi->uc_lut_index;
        hal_get_tx_pn(pst_hal_device, &st_pn_lut_cfg);

        /* 保存读取到的值 */
        pst_dmac_user_candi->st_pn.st_ucast_tx_pn.ul_pn_msb = st_pn_lut_cfg.ul_pn_msb;
        pst_dmac_user_candi->st_pn.st_ucast_tx_pn.ul_pn_lsb = st_pn_lut_cfg.ul_pn_lsb;
    }

    /* 单播rx pn */
    for(uc_tid_loop = 0; uc_tid_loop < WLAN_TID_MAX_NUM; uc_tid_loop++)
    {
        /* 读取寄存器中的值 */
        st_pn_lut_cfg.uc_pn_key_type = 1;
        st_pn_lut_cfg.uc_pn_peer_idx = pst_dmac_user_candi->uc_lut_index;
        st_pn_lut_cfg.uc_pn_tid      = uc_tid_loop;
        hal_get_rx_pn(pst_hal_device, &st_pn_lut_cfg);

        /* 保存读取到的值 */
        pst_dmac_user_candi->st_pn.ast_tid_ucast_rx_pn[uc_tid_loop].ul_pn_msb = st_pn_lut_cfg.ul_pn_msb;
        pst_dmac_user_candi->st_pn.ast_tid_ucast_rx_pn[uc_tid_loop].ul_pn_lsb = st_pn_lut_cfg.ul_pn_lsb;
    }
}

#ifdef _PRE_WLAN_PRODUCT_1151V200

OAL_STATIC oal_void dmac_user_save_candidate_peer_resp_dis(dmac_user_stru *pst_dmac_user_candi)
{
    hal_peer_resp_dis_cfg_stru      st_cfg;
    hal_to_dmac_device_stru        *pst_hal_device;

    if (OAL_PTR_NULL == pst_dmac_user_candi)
    {
        return;
    }

    /* 获取hal device */
    pst_hal_device = dmac_user_get_hal_device((mac_user_stru *)pst_dmac_user_candi);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_dmac_user_candi->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND, "dmac_user_save_candidate_peer_resp_dis::pst_hal_device ptr null.");
        return;
    }

    st_cfg.uc_lut_index = pst_dmac_user_candi->uc_lut_index;

    /* 获取peer resp dis */
    hal_get_peer_resp_dis(pst_hal_device, &st_cfg);

    pst_dmac_user_candi->bit_peer_resp_dis = st_cfg.uc_peer_resp_dis;
}
#endif


OAL_STATIC oal_uint32 dmac_user_save_candidate_info(dmac_user_stru *pst_dmac_user_candi)
{
    if (OAL_PTR_NULL == pst_dmac_user_candi)
    {
        OAM_ERROR_LOG0(0, OAM_SF_USER_EXTEND, "dmac_user_save_candidate_info::pst_dmac_user_candi null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 回读更新PN/TSC */
    dmac_user_save_candidate_pn(pst_dmac_user_candi);

#ifdef _PRE_WLAN_PRODUCT_1151V200
    /* 回读更新回复响应帧禁能bit */
    dmac_user_save_candidate_peer_resp_dis(pst_dmac_user_candi);
#endif

    /* 通知算法用户被替换，保存用户算法参数 */
    dmac_alg_user_replace_notify(pst_dmac_user_candi, DMAC_ALG_USER_OPER_TYPE_REPLACED);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_user_add_demand_machw_info(dmac_user_stru *pst_dmac_user_demand)
{
    oal_uint32          ul_ret;

    if (OAL_PTR_NULL == pst_dmac_user_demand)
    {
        OAM_ERROR_LOG0(0, OAM_SF_USER_EXTEND, "dmac_user_save_candidate_info::pst_dmac_user_demand null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 添加PN/TSC */
    ul_ret = dmac_11i_add_machw_pn(pst_dmac_user_demand);

#ifdef _PRE_WLAN_PRODUCT_1151V200
    /* 设置回复peer ack/ba 禁能bit */
    ul_ret = dmac_user_add_machw_peer_resp_dis(pst_dmac_user_demand);
#endif

    /* 通知算法用户替换，向硬件添加用户算法参数 */
    dmac_alg_user_replace_notify(pst_dmac_user_demand, DMAC_ALG_USER_OPER_TYPE_REPLACE);

    return ul_ret;
}


oal_uint32 dmac_user_max_active_user_handle(dmac_user_stru *pst_user_demand)
{
    mac_chip_stru      *pst_mac_chip;
    dmac_user_stru     *pst_dmac_user_candi;
    oal_uint32          ul_ret;

    if (OAL_PTR_NULL == pst_user_demand)
    {
        OAM_WARNING_LOG0(0, OAM_SF_USER_EXTEND, "dmac_user_max_active_user_handle::pst_user_demand null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取隶属的chip */
    pst_mac_chip = mac_res_get_mac_chip(pst_user_demand->st_user_base_info.uc_chip_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_chip))
    {
        OAM_ERROR_LOG0(pst_user_demand->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND, "dmac_user_max_active_user_handle::pst_mac_chip ptr null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 未使能 */
    if (!pst_mac_chip->st_user_extend.en_flag)
    {
        OAM_ERROR_LOG0(pst_user_demand->st_user_base_info.uc_vap_id, OAM_SF_USER_EXTEND,
            "dmac_user_max_active_user_handle::should not be here, feature not enabled!");
        return OAL_ERR_CODE_USER_EXTEND_DISABLED;
    }

    /* 刷新为非新用户状态 */
    pst_user_demand->bit_new_add_user = OAL_FALSE;

    /* 获取候选用户 */
    pst_dmac_user_candi = dmac_user_get_candidate_user(pst_mac_chip);

    /* inactive前保存候选用户的最近一次信息 */
    dmac_user_save_candidate_info(pst_dmac_user_candi);

    /* inactive候选用户 */
    ul_ret = dmac_user_inactive(pst_dmac_user_candi);

    /* active需求用户 */
    ul_ret = dmac_user_active(pst_user_demand);

    /* 向硬件添加新用户的信息 */
    ul_ret = dmac_user_add_demand_machw_info(pst_user_demand);

    return ul_ret;
}

oal_uint32 dmac_user_replace(dmac_user_stru *pst_user_demand, dmac_user_stru *pst_user_candidate)
{
    oal_uint32          ul_ret;
    /* inactive前保存候选用户的最近一次信息 */
    dmac_user_save_candidate_info(pst_user_candidate);

    /* inactive候选用户 */
    ul_ret = dmac_user_inactive(pst_user_candidate);

    /* active需求用户 */
    ul_ret = dmac_user_active(pst_user_demand);

    /* 向硬件添加新用户的信息 */
    ul_ret = dmac_user_add_demand_machw_info(pst_user_demand);
    return ul_ret;
}

#endif /* _PRE_WLAN_FEATURE_USER_EXTEND */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

