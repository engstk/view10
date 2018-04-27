


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_profiling.h"

#include "oam_ext_if.h"

#include "mac_data.h"

#include "dmac_ext_if.h"
#include "dmac_tx_complete.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_blockack.h"
#include "dmac_psm_ap.h"
#include "dmac_uapsd.h"
#include "dmac_chan_mgmt.h"
#include "dmac_ext_if.h"
#include "dmac_dft.h"
#include "mac_data.h"
#include "dmac_p2p.h"
#include "dmac_beacon.h"
#include "dmac_config.h"
#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_test_main.h"
#include "dmac_test_sch.h"
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
#include "mac_board.h"
#include "dmac_device.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "dmac_ftm.h"
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
#include "dmac_smps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
#include "dmac_resource.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TX_COMPLETE_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
oal_uint8 g_uc_aggr_num_switch = 0;     /* 设置最大AMPDU聚合个数开关 */
oal_uint8 g_uc_max_aggr_num    = 0;     /* 设置最大AMPDU聚合个数 */


oal_uint32  dmac_tx_complete_dump_dscr(hal_to_dmac_device_stru *pst_hal_device,
                                          hal_tx_dscr_stru        *pst_base_dscr)
{
    oal_uint32             ul_ret;
    mac_tx_ctl_stru       *pst_tx_cb;
    oal_switch_enum_uint8  en_frame_switch;
    oal_switch_enum_uint8  en_cb_switch;
    oal_switch_enum_uint8  en_dscr_switch;
    oam_user_track_frame_type_enum_uint8 en_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
    oal_netbuf_stru *pst_netbuf;
    oal_uint8               uc_dscr_status;
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    mac_ieee80211_frame_stru *pst_frame;
#endif
#ifndef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    oal_uint32             ul_dscr_one_size;
    oal_uint32             ul_dscr_two_size;
    oal_uint8              auc_user_macaddr[WLAN_MAC_ADDR_LEN] = {0};
#endif

    pst_netbuf = pst_base_dscr->pst_skb_start_addr;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf))
    {
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_complete_dump_dscr::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取发送描述符大小 */
    pst_tx_cb = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf);
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_frame= ((mac_ieee80211_frame_stru *)((oal_uint8 *)pst_tx_cb + OAL_MAX_CB_LEN));
#endif

    if (WLAN_DATA_BASICTYPE == MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_cb))
    {
        if(MAC_GET_CB_IS_VIPFRAME(pst_tx_cb))
        {
            hal_tx_get_dscr_status(pst_hal_device, pst_base_dscr, &uc_dscr_status);
            /* 维测，输出一个关键帧打印 */
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
            OAM_WARNING_LOG4(0, OAM_SF_ANY, "{dmac_tx_complete_dump_dscr::tx datatype=%u to XX:XX:XX:XX:%x:%x  rst==%u}[0:dhcp 1:eapol 2:arp_rsp 3:arp_req]",
                         MAC_GET_CB_FRAME_SUBTYPE(pst_tx_cb),  pst_frame->auc_address1[4], pst_frame->auc_address1[5], uc_dscr_status);
#else
            OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_tx_complete_dump_dscr::tx datatype=%u rst==%u}[0:dhcp 1:eapol 2:arp_rsp 3:arp_req]",
                         MAC_GET_CB_FRAME_SUBTYPE(pst_tx_cb), uc_dscr_status);
#endif
        }

        if (OAL_SWITCH_ON != oam_report_data_get_global_switch(OAM_OTA_FRAME_DIRECTION_TYPE_TX))
        {
            return OAL_SUCC;
        }

        en_frame_type = OAM_USER_TRACK_FRAME_TYPE_DATA;
    }
    else if (WLAN_MANAGEMENT == MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_cb))
    {
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        if (WLAN_MANAGEMENT == pst_frame->st_frame_control.bit_type
            && (WLAN_DISASOC == pst_frame->st_frame_control.bit_sub_type || WLAN_DEAUTH == pst_frame->st_frame_control.bit_sub_type))
        {
            hal_tx_get_dscr_status(pst_hal_device, pst_base_dscr, &uc_dscr_status);

            OAM_WARNING_LOG4(0, OAM_SF_ANY, "{dmac_tx_complete_dump_dscr::tx disassoc or deauth=%u[10:DISASOC, 12:DEAUTH] to XX:XX:XX:XX:%x:%x. status=%u.}",
                                             pst_frame->st_frame_control.bit_sub_type, pst_frame->auc_address1[4], pst_frame->auc_address1[5], uc_dscr_status);
        }
#endif
    }

    ul_ret = dmac_tx_dump_get_switch(en_frame_type, &en_frame_switch, &en_cb_switch, &en_dscr_switch, pst_tx_cb);
    if (OAL_ERR_CODE_PTR_NULL == ul_ret)
    {
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_dump_dscr::dmac_tx_dump_get_switch return null, user_idx=%d.", pst_tx_cb->bit_tx_user_idx);
#else
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_dump_dscr::dmac_tx_dump_get_switch return null, user_idx=%d.", pst_tx_cb->us_tx_user_idx);
#endif
        return ul_ret;
    }
    else if (OAL_FAIL == ul_ret)
    {
        return OAL_SUCC;
    }

    if (OAL_SWITCH_ON == en_dscr_switch)
    {
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
        dmac_tx_dump_dscr_list(pst_hal_device, pst_base_dscr);
#else
        /* 获取用户mac地址 */
        ul_ret = dmac_tx_dump_get_user_macaddr(pst_tx_cb, auc_user_macaddr);
        hal_tx_get_size_dscr(pst_hal_device, MAC_GET_CB_NETBUF_NUM(pst_tx_cb), &ul_dscr_one_size, &ul_dscr_two_size);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_dump_dscr::dmac_tx_get_user_macaddr failed[%d].", ul_ret);
            return ul_ret;
        }
        /* report聚合首帧 */

        oam_report_dscr(auc_user_macaddr,
                        (oal_uint8 *)pst_base_dscr,
                        (oal_uint16)(ul_dscr_one_size + ul_dscr_two_size),
                        OAM_OTA_TX_DSCR_TYPE);
#endif
    }

#ifdef _PRE_DEBUG_MODE
    if ((pst_base_dscr->uc_q_num < HAL_TX_QUEUE_HI))
    {
        g_ast_tx_complete_stat[pst_hal_device->uc_mac_device_id].ul_tx_complete_bh2_num++;
        OAM_INFO_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_dump_dscr::g_ul_tx_complete_bh2_num = %d.", g_ast_tx_complete_stat[pst_hal_device->uc_mac_device_id].ul_tx_complete_bh2_num);
    }
#endif

    return OAL_SUCC;
}




oal_void  dmac_tx_get_vap_id(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_vap_id)
{
    oal_uint8                        uc_hal_vap_id;
    hal_to_dmac_vap_stru            *pst_hal_vap = OAL_PTR_NULL;

    hal_tx_get_vap_id(pst_hal_device, pst_tx_dscr, &uc_hal_vap_id);

    hal_get_hal_vap(pst_hal_device, uc_hal_vap_id, &pst_hal_vap);

    if (OAL_PTR_NULL == pst_hal_vap)
    {
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_get_vap_id::pst_hal_vap null uc_hal_vap_id = %d.}",uc_hal_vap_id);

        *puc_vap_id = 0;
    }
    else
    {
        *puc_vap_id = pst_hal_vap->uc_mac_vap_id;
    }

    return;
}



oal_void  dmac_tx_complete_free_dscr(hal_tx_dscr_stru *pst_dscr)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dscr))
    {
         OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_complete_free_dscr::dscr is NULL.}");
         return ;
    }

    /*将描述符从链表中删除*/
    oal_dlist_delete_entry(&pst_dscr->st_entry);

    /*释放描述符中指向的netbuf*/
    if (OAL_PTR_NULL != pst_dscr->pst_skb_start_addr)
    {
        if (OAL_SUCC != g_st_dmac_tx_bss_comm_rom_cb.p_dmac_tx_excp_free_netbuf(pst_dscr->pst_skb_start_addr))
        {
            OAM_ERROR_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_free_dscr::netbuff free fail, addr=0x%x.}", pst_dscr->pst_skb_start_addr);
        }
        pst_dscr->pst_skb_start_addr = OAL_PTR_NULL;
    }

    //OAL_MEM_TRACE(pst_dscr, OAL_FALSE);

    /*释放描述符自身*/
    if (OAL_SUCC != OAL_MEM_FREE(pst_dscr, OAL_TRUE))
    {
        OAM_ERROR_LOG1(0, OAM_SF_TX, "{dmac_tx_complete_free_dscr::tx dscr free fail, addr=0x%x.}", pst_dscr);
    }
}


oal_uint32  dmac_mgmt_tx_complete(mac_vap_stru *pst_mac_vap,
                                  oal_uint8     mgmt_frame_id,
                                  oal_uint32    uc_dscr_status,
                                  oal_uint16    us_user_idx)
{
    frw_event_mem_stru         *pst_event_mem;
    frw_event_stru             *pst_event;
    dmac_crx_mgmt_tx_status_stru *pst_mgmt_tx_status_param;

    /* 抛mgmt tx 结果到HMAC, 申请事件内存 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_crx_mgmt_tx_status_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "dmac_mgmt_tx_complete::alloc mem fail");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    pst_event = frw_get_event_stru(pst_event_mem);

    pst_mgmt_tx_status_param = (dmac_crx_mgmt_tx_status_stru *)(pst_event->auc_event_data);
    pst_mgmt_tx_status_param->uc_dscr_status = uc_dscr_status;
    pst_mgmt_tx_status_param->mgmt_frame_id = mgmt_frame_id;
    pst_mgmt_tx_status_param->us_user_idx   = us_user_idx;

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CFG80211_MGMT_TX_STATUS, OAL_SIZEOF(dmac_crx_mgmt_tx_status_stru), (oal_uint8 *)pst_mgmt_tx_status_param);
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW

oal_void dmac_tx_dump_dscr_list(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_dscr)
{
    hal_tx_dscr_stru *pst_dscr_next;
    oal_uint8         uc_dscr_index;
    oal_uint8         uc_dscr_num;
    oal_netbuf_stru  *pst_buf;
    mac_tx_ctl_stru  *pst_cb;

    pst_buf = pst_dscr->pst_skb_start_addr;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_buf))
    {
        return ;
    }

    pst_cb          = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_buf);
    uc_dscr_num     = MAC_GET_CB_MPDU_NUM(pst_cb);

    pst_dscr_next = pst_dscr;

    for (uc_dscr_index = 0; uc_dscr_index < uc_dscr_num; uc_dscr_index++)
    {
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dscr_next))
        {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw:: pst_dscr_next NULL .}");
            break;
        }

        pst_dscr = pst_dscr_next;
        OAL_MEM_TRACE(pst_dscr, OAL_FALSE);
        pst_dscr_next = OAL_DLIST_GET_ENTRY(pst_dscr->st_entry.pst_next, hal_tx_dscr_stru, st_entry);
        oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_dscr, WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
    }

}


oal_void dmac_tx_complete_hw_seq_num_err(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru* pst_dscr, oal_uint8 uc_dscr_num, oal_uint8 uc_q_num)
{
    oal_uint16                   us_seq_num_pre = 0;
    oal_uint16                   us_seq_num_new = 0;
    hal_tx_dscr_stru            *pst_dscr_head;
    hal_tx_dscr_stru            *pst_dscr_next;
    oal_uint8                    uc_dscr_index;
    oal_uint8                    uc_seq_vld;

    pst_dscr_next = pst_dscr;
    pst_dscr_head = pst_dscr;

    /* 维测用 */
    for (uc_dscr_index = 0; uc_dscr_index < uc_dscr_num; uc_dscr_index++)
    {
        if (OAL_UNLIKELY(oal_dlist_is_empty(&pst_hal_device->ast_tx_dscr_queue[uc_q_num].st_header)))
        {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw::q empty.}");
            break;
        }

        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dscr_next))
        {
            OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw:: pst_dscr_next NULL .}");
            break;
        }

        pst_dscr = pst_dscr_next;
        OAL_MEM_TRACE(pst_dscr, OAL_FALSE);
        pst_dscr_next = OAL_DLIST_GET_ENTRY(pst_dscr->st_entry.pst_next, hal_tx_dscr_stru, st_entry);

        hal_tx_get_dscr_seqnum(pst_dscr, &us_seq_num_new, &uc_seq_vld);

        /* 聚合中的首帧 */
        if (0 == us_seq_num_pre)
        {
            us_seq_num_pre = us_seq_num_new;
            continue;
        }

        /* 如果溢出 */
        if (DMAC_BA_SEQNO_MASK == us_seq_num_pre)
        {
            us_seq_num_pre = 0;
            continue;
        }

        /* 上报的描述符seq倒序/非递增异常 */
        if ((us_seq_num_pre + 1) != us_seq_num_new)
        {
            OAM_ERROR_LOG2(0, OAM_SF_TX, "{dmac_tx_complete_ampdu_buffer_hw:: pre seq[%d]+1 != new seq[%d].}", us_seq_num_pre, us_seq_num_new);
            dmac_tx_dump_dscr_list(pst_hal_device, pst_dscr_head);
            break;
        }

        us_seq_num_pre++;
    }
}
#endif

/*lint -e19*/

/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


