


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "mac_ie.h"
#include "mac_vap.h"
#include "dmac_vap.h"
#include "dmac_main.h"
#include "dmac_blockack.h"
#include "dmac_alg.h"
#include "dmac_beacon.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_tx_complete.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_mgmt_sta.h"

#ifdef _PRE_WIFI_DMT
#include "dmt_stub.h"
#endif
#include "dmac_device.h"
#include "mac_device.h"
#include "dmac_config.h"
#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
#include "dmac_resource.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11K
#include "dmac_11k.h"
#endif
#ifdef _PRE_WLAN_PERFORM_STAT
#include "dmac_stat.h"
#endif

#include "dmac_power.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_VAP_ROM_C
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_VOWIFI

void dmac_vap_vowifi_exit(dmac_vap_stru *pst_dmac_vap)
{
    mac_vowifi_status_stru    *pst_vowifi_status;

    if (OAL_PTR_NULL == pst_dmac_vap->pst_vowifi_status)
    {
        return;
    }

    pst_vowifi_status = pst_dmac_vap->pst_vowifi_status;

    /*先置空再释放*/
    pst_dmac_vap->pst_vowifi_status = OAL_PTR_NULL;
    OAL_MEM_FREE(pst_vowifi_status, OAL_TRUE);

}
#endif /* _PRE_WLAN_FEATURE_VOWIFI */

oal_uint32  mac_vap_pause_tx(mac_vap_stru *pst_vap)
{
    pst_vap->en_vap_state = MAC_VAP_STATE_PAUSE;

    return  OAL_SUCC;
}

oal_uint32  dmac_vap_fake_queue_empty_assert(mac_vap_stru *pst_mac_vap, oal_uint32 ul_file_id)
{
    oal_uint8                       uc_q_idx;
    oal_dlist_head_stru            *pst_dlist_entry;
    hal_tx_dscr_stru               *pst_tx_dscr;
    oal_netbuf_stru                *pst_netbuf;
    mac_tx_ctl_stru                *pst_tx_ctl;
    oal_uint32                      ul_ret = OAL_TRUE;
    hal_tx_dscr_queue_header_stru  *pst_fake_queue;

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_DBAC, "dmac_vap_fake_queue_empty_assert::pst_mac_vap null");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_vap);

    for (uc_q_idx = 0; uc_q_idx < HAL_TX_QUEUE_BUTT; uc_q_idx++)
    {
        if (!oal_dlist_is_empty(&pst_fake_queue[uc_q_idx].st_header))
        {
            /* 虚假队列不为空，将第一个帧通过OTA上报上来 */
            pst_dlist_entry = pst_fake_queue[uc_q_idx].st_header.pst_next;
            pst_tx_dscr     = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_entry, hal_tx_dscr_stru, st_entry);
            oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_tx_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
            pst_netbuf = pst_tx_dscr->pst_skb_start_addr;

            pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

            oam_report_80211_frame(BROADCAST_MACADDR,
                                    (oal_uint8*)MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl),
                                    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl),
                                    (oal_uint8*)oal_netbuf_payload(pst_netbuf),
                                    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) + MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl),
                                    OAM_OTA_FRAME_DIRECTION_TYPE_TX);

            OAM_ERROR_LOG4(0, OAM_SF_DBAC, "fake q is not empty.vap id[%d],ac:%d, file_id:%d, ppdu_cnt:%d",
                                    pst_mac_vap->uc_vap_id, uc_q_idx, ul_file_id, pst_fake_queue[uc_q_idx].uc_ppdu_cnt);
            ul_ret = OAL_FALSE;
        }
    }

    return ul_ret;
}

oal_uint32  dmac_vap_clear_fake_queue(mac_vap_stru  *pst_mac_vap)
{
    oal_int8                        c_q_id;
    hal_tx_dscr_queue_header_stru  *pst_fake_queue;
    oal_dlist_head_stru            *pst_dlist_entry;
    hal_tx_dscr_stru               *pst_tx_dscr;

    pst_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_vap);

    /* 遍历6个发送队列 一定要先处理高优先级队列防止普通优先级队列发送完成产生管理帧入错队列 */
    for (c_q_id = HAL_TX_QUEUE_BUTT - 1; c_q_id >= 0; c_q_id--)
    {
        while(!oal_dlist_is_empty(&pst_fake_queue[(oal_uint8)c_q_id].st_header))
        {
            pst_dlist_entry = pst_fake_queue[(oal_uint8)c_q_id].st_header.pst_next;

            pst_tx_dscr     = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_entry, hal_tx_dscr_stru, st_entry);

            dmac_tx_complete_free_dscr(pst_tx_dscr);
        }

        pst_fake_queue[(oal_uint8)c_q_id].uc_ppdu_cnt = 0;
    }

    return OAL_SUCC;
}

oal_uint32  mac_vap_resume_tx(mac_vap_stru *pst_vap)
{
    pst_vap->en_vap_state = MAC_VAP_STATE_UP;

    return  OAL_SUCC;
}



oal_void  dmac_vap_pause_tx(mac_vap_stru *pst_mac_vap)
{
    dmac_vap_stru  *pst_dmac_vap;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_TX, "dmac_vap_pause_tx: dmac vap is null. vap mode:%d, vap state:%d",
                        pst_mac_vap->en_vap_mode, pst_mac_vap->en_vap_state);
        return;
    }

    if (MAC_VAP_STATE_UP != pst_mac_vap->en_vap_state)
    {
        return;
    }

    mac_vap_pause_tx(pst_mac_vap);
    hal_vap_beacon_suspend(pst_dmac_vap->pst_hal_vap);
}



oal_void  dmac_vap_pause_tx_by_chl(mac_device_stru *pst_device, mac_channel_stru *pst_src_chl)
{
    oal_uint8               uc_vap_idx;
    mac_vap_stru           *pst_vap;

    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "{dmac_vap_pause_tx_by_chl::pst_vap null, vap id is %d.}",
                           pst_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (OAL_TRUE == mac_fcs_is_same_channel(&pst_vap->st_channel, pst_src_chl) && (MAC_VAP_STATE_UP == pst_vap->en_vap_state))
        {
            dmac_vap_pause_tx(pst_vap);
        }
    }
}

oal_bool_enum_uint8  dmac_vap_is_fakeq_empty(mac_vap_stru *pst_mac_vap)
{
    oal_uint8                       uc_q_idx;
    hal_tx_dscr_queue_header_stru  *pst_fake_queue;

    pst_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_vap);
    for (uc_q_idx = 0; uc_q_idx < HAL_TX_QUEUE_BUTT; uc_q_idx++)
    {
        if (!oal_dlist_is_empty(&pst_fake_queue[uc_q_idx].st_header))
        {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}

oal_uint32  dmac_vap_is_in_p2p_listen(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_vap_is_in_p2p_listen::pst_mac_device[%d] null!}", pst_mac_vap->uc_device_id);
        return OAL_FALSE;
    }

    if ((MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state) &&
         (MAC_SCAN_FUNC_P2P_LISTEN == pst_mac_device->st_scan_params.uc_scan_func))
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT


oal_void dmac_vap_init_sensing_bssid_list(dmac_vap_stru *pst_dmac_vap)
{
    OAL_MEMZERO(&(pst_dmac_vap->st_sensing_bssid_list), OAL_SIZEOF(dmac_sensing_bssid_list_stru));
    oal_spin_lock_init(&(pst_dmac_vap->st_sensing_bssid_list.st_lock));
    oal_dlist_init_head(&(pst_dmac_vap->st_sensing_bssid_list.st_list_head));
}


oal_void dmac_vap_clear_sensing_bssid_list(dmac_vap_stru *pst_dmac_vap)
{
    dmac_sensing_bssid_list_stru     *pst_sensing_bssid_list;
    dmac_sensing_bssid_list_member_stru    *pst_sensing_bssid_member;
    oal_dlist_head_stru             *pst_entry = OAL_PTR_NULL;
    oal_dlist_head_stru             *pst_dlist_tmp = OAL_PTR_NULL;

    pst_sensing_bssid_list = &(pst_dmac_vap->st_sensing_bssid_list);
    oal_spin_lock(&(pst_sensing_bssid_list->st_lock));
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_sensing_bssid_list->st_list_head))
    {
        pst_sensing_bssid_member = OAL_DLIST_GET_ENTRY(pst_entry, dmac_sensing_bssid_list_member_stru, st_dlist);
        oal_dlist_delete_entry(&pst_sensing_bssid_member->st_dlist);
        pst_sensing_bssid_list->uc_member_nums--;
        OAL_MEM_FREE(pst_sensing_bssid_member, OAL_TRUE);
    }
    oal_spin_unlock(&(pst_sensing_bssid_list->st_lock));
}


oal_uint32 dmac_vap_find_and_del_sensing_bssid(dmac_vap_stru *pst_dmac_vap, dmac_sensing_bssid_cfg_stru *pst_sensing_bssid)
{
    dmac_sensing_bssid_list_stru          *pst_sensing_bssid_list;
    dmac_sensing_bssid_list_member_stru   *pst_sensing_bssid_member;
    oal_dlist_head_stru                   *pst_entry;
    oal_dlist_head_stru                   *pst_entry_temp;
    oal_uint32                             ul_broadcast_ether_addr;

    pst_sensing_bssid_list = &(pst_dmac_vap->st_sensing_bssid_list);
    ul_broadcast_ether_addr = oal_is_broadcast_ether_addr(pst_sensing_bssid->auc_mac_addr);

    /* 遍历链表，看目标mac是否在sta列表内 */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_entry_temp, &(pst_sensing_bssid_list->st_list_head))
    {
        pst_sensing_bssid_member = OAL_DLIST_GET_ENTRY(pst_entry, dmac_sensing_bssid_list_member_stru, st_dlist);
        if (ul_broadcast_ether_addr)
        {
            if (pst_sensing_bssid->en_operation == OAL_SENSING_BSSID_OPERATE_DEL)
            {
                oal_dlist_delete_entry(&pst_sensing_bssid_member->st_dlist);
                pst_sensing_bssid_list->uc_member_nums--;
                OAL_MEM_FREE(pst_sensing_bssid_member, OAL_TRUE);
                OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{dmac_vap_find_and_del_sensing_bssid, broadcast delete::[xx:xx:%02x:%02x:%02x:%02x] del seccess!}",
                             pst_sensing_bssid->auc_mac_addr[2], pst_sensing_bssid->auc_mac_addr[3], pst_sensing_bssid->auc_mac_addr[4], pst_sensing_bssid->auc_mac_addr[5]);
            }
            else
            {
                OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{dmac_vap_find_and_del_sensing_bssid, broadcast add !}");
                break;   /** 广播地址不能添加 */
            }
        }
        else if (0 == oal_memcmp(pst_sensing_bssid_member->auc_mac_addr, pst_sensing_bssid->auc_mac_addr, WLAN_MAC_ADDR_LEN))
        {
            if (pst_sensing_bssid->en_operation == OAL_SENSING_BSSID_OPERATE_DEL)
            {
                oal_dlist_delete_entry(&pst_sensing_bssid_member->st_dlist);
                pst_sensing_bssid_list->uc_member_nums--;
                OAL_MEM_FREE(pst_sensing_bssid_member, OAL_TRUE);
                OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{dmac_vap_find_and_del_sensing_bssid::[xx:xx:%02x:%02x:%02x:%02x] del seccess!}",
                             pst_sensing_bssid->auc_mac_addr[2], pst_sensing_bssid->auc_mac_addr[3], pst_sensing_bssid->auc_mac_addr[4], pst_sensing_bssid->auc_mac_addr[5]);
            }
            else
            {
                OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{dmac_vap_find_and_del_sensing_bssid::[xx:xx:%02x:%02x:%02x:%02x]already in sensing bssid list!}",
                            pst_sensing_bssid->auc_mac_addr[2], pst_sensing_bssid->auc_mac_addr[3], pst_sensing_bssid->auc_mac_addr[4], pst_sensing_bssid->auc_mac_addr[5]);
            }
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}


oal_uint32 dmac_vap_add_sensing_bssid_list(dmac_vap_stru *pst_dmac_vap, dmac_sensing_bssid_cfg_stru *pst_sensing_bssid)
{
    dmac_sensing_bssid_list_stru           *pst_sensing_bssid_list;
    dmac_sensing_bssid_list_member_stru    *pst_sensing_bssid_member;

    pst_sensing_bssid_list =  &(pst_dmac_vap->st_sensing_bssid_list);
    if (DMAC_SENSING_BSSID_LIST_MAX_MEMBER_CNT <= pst_sensing_bssid_list->uc_member_nums)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{dmac_vap_add_sensing_bssid_list::add mem failed!sensing bssid list member out of limit.}");
        return OAL_FAIL;
    }

    pst_sensing_bssid_member = (dmac_sensing_bssid_list_member_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,OAL_SIZEOF(dmac_sensing_bssid_list_member_stru),OAL_TRUE);
    if(OAL_PTR_NULL == pst_sensing_bssid_member)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{dmac_vap_add_sensing_bssid_list::oal_memalloc fail !}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    OAL_MEMZERO(pst_sensing_bssid_member, OAL_SIZEOF(dmac_sensing_bssid_list_member_stru));
    oal_memcopy(pst_sensing_bssid_member->auc_mac_addr, pst_sensing_bssid->auc_mac_addr, WLAN_MAC_ADDR_LEN);
    oal_dlist_add_tail(&(pst_sensing_bssid_member->st_dlist), &(pst_sensing_bssid_list->st_list_head));
    pst_sensing_bssid_list->uc_member_nums++;

    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_HILINK, "{dmac_vap_add_sensing_bssid_list::add[xx:xx:%02x:%02x:%02x:%02x]to sensing bssid list success!}",
                     pst_sensing_bssid->auc_mac_addr[2], pst_sensing_bssid->auc_mac_addr[3], pst_sensing_bssid->auc_mac_addr[4], pst_sensing_bssid->auc_mac_addr[5]);
    return OAL_SUCC;
}


oal_uint32 dmac_vap_update_sensing_bssid_list(mac_vap_stru *pst_mac_vap, dmac_sensing_bssid_cfg_stru *pst_sensing_bssid)
{
    oal_uint32                                ul_ret = OAL_SUCC;
    dmac_sensing_bssid_list_stru             *pst_sensing_bssid_list;
    dmac_vap_stru                            *pst_dmac_vap;

    /* 入参检查 */
    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == pst_sensing_bssid))
    {
        OAM_WARNING_LOG0(0, OAM_SF_HILINK, "{dmac_vap_update_sensing_bssid_list::null pst_hmac_vap or pst_sensing_bssid}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "dmac_vap_update_sensing_bssid_list: dmac vap is null. vap mode:%d, vap state:%d",
                        pst_mac_vap->en_vap_mode, pst_mac_vap->en_vap_state);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_sensing_bssid_list = &(pst_dmac_vap->st_sensing_bssid_list);
    oal_spin_lock(&(pst_sensing_bssid_list->st_lock));

    ul_ret = dmac_vap_find_and_del_sensing_bssid(pst_dmac_vap, pst_sensing_bssid);
    if (ul_ret == OAL_FAIL)
    {
        if (pst_sensing_bssid->en_operation == OAL_SENSING_BSSID_OPERATE_ADD)
        {
            if (!oal_is_broadcast_ether_addr(pst_sensing_bssid->auc_mac_addr))
            {
                ul_ret = dmac_vap_add_sensing_bssid_list(pst_dmac_vap, pst_sensing_bssid);
            }
        }
        else
        {
            ul_ret = OAL_SUCC; /** 删除操作默认成功 */
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{dmac_vap_update_sensing_bssid_list::[xx:xx:%02x:%02x:%02x:%02x] not in sensing bssid list while del!}",
                            pst_sensing_bssid->auc_mac_addr[2], pst_sensing_bssid->auc_mac_addr[3], pst_sensing_bssid->auc_mac_addr[4], pst_sensing_bssid->auc_mac_addr[5]);
        }
    }

    /* 返回之前，释放内存，释放锁 */
    oal_spin_unlock(&(pst_sensing_bssid_list->st_lock));

    return ul_ret;
}

#endif


oal_uint32 dmac_sta_bw_switch_fsm_post_event(dmac_vap_stru* pst_dmac_vap, oal_uint16 us_type, oal_uint16 us_datalen, oal_uint8* pst_data)
{
    dmac_sta_bw_switch_fsm_info_stru *pst_handler      = OAL_PTR_NULL;
    oal_uint32                        ul_ret;

    pst_handler = (dmac_sta_bw_switch_fsm_info_stru *)(pst_dmac_vap->pst_sta_bw_switch_fsm);

    if (OAL_UNLIKELY((us_type >= DMAC_STA_BW_SWITCH_EVENT_BUTT) || (OAL_PTR_NULL == pst_handler)))
    {
        OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                       "dmac_sta_bw_switch_fsm_post_event:: event type[%d] pst_handler[%p] error, NULL!", us_type, pst_handler);
        return OAL_FAIL;
    }

    if (OAL_FALSE == pst_handler->en_is_fsm_attached)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ASSOC,
                         "dmac_sta_bw_switch_fsm_post_event::bw_fsm_attached = %d!",pst_handler->en_is_fsm_attached);
        return OAL_FAIL;
    }

    ul_ret = oal_fsm_event_dispatch(&(pst_handler->st_oal_fsm), us_type, us_datalen, pst_data);

    return ul_ret;
}


oal_uint8 dmac_sta_bw_fsm_get_current_state(mac_vap_stru *pst_mac_vap)
{
    dmac_vap_stru                    *pst_dmac_vap        = (dmac_vap_stru *)pst_mac_vap;
    dmac_sta_bw_switch_fsm_info_stru *pst_bw_fsm_handler  = (dmac_sta_bw_switch_fsm_info_stru *)pst_dmac_vap->pst_sta_bw_switch_fsm;

    if (OAL_PTR_NULL == pst_bw_fsm_handler)
    {
        return DMAC_STA_BW_SWITCH_FSM_BUTT;
    }

    return pst_bw_fsm_handler->st_oal_fsm.uc_cur_state;
}


/*lint -e578*//*lint -e19*/
oal_module_symbol(mac_vap_pause_tx);
oal_module_symbol(mac_vap_resume_tx);
oal_module_symbol(dmac_vap_pause_tx_by_chl);
oal_module_symbol(dmac_vap_clear_fake_queue);
oal_module_symbol(dmac_vap_fake_queue_empty_assert);
/*lint +e578*//*lint +e19*/









#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


