


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_net.h"
#include "oam_ext_if.h"
#include "oam_main.h"

#include "wlan_spec.h"
#include "wlan_mib.h"

#include "mac_frame.h"
#include "mac_data.h"
#include "hal_ext_if.h"
#include "dmac_device.h"
#include "dmac_resource.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_blockack.h"
#include "dmac_rx_data.h"
#include "dmac_ext_if.h"
#include "dmac_beacon.h"
#include "dmac_reset.h"
#include "oal_profiling.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_RX_DATA_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/


 
oal_bool_enum_uint8  dmac_rx_check_mgmt_replay_failure(dmac_rx_ctl_stru  *pst_cb_ctrl)
{
    mac_ieee80211_frame_stru  *pst_frame_hdr;

    /* 获取802.11头指针 */
    pst_frame_hdr  = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_cb_ctrl->st_rx_info));

    if (MAC_IEEE80211_FTYPE_MGMT != pst_frame_hdr->st_frame_control.bit_type)
    {
        return OAL_TRUE;
    }

    switch (pst_cb_ctrl->st_rx_status.bit_dscr_status)
    {
        case HAL_RX_CCMP_REPLAY_FAILURE:
        case HAL_RX_TKIP_REPLAY_FAILURE:
        case HAL_RX_BIP_REPLAY_FAILURE:
        {
            return OAL_TRUE;
        }
        default:
        {
            return OAL_FALSE;
        }
    }
}



oal_uint32  dmac_rx_send_event(
                dmac_vap_stru         *pst_dmac_vap,
                dmac_wlan_crx_event_sub_type_enum_uint8 en_event,
                oal_uint8                 *puc_param,
                oal_uint32             ul_len)
{
    frw_event_mem_stru             *pst_event_mem;      /* 申请事件返回的内存指针 */
    frw_event_stru                 *pst_hmac_to_dmac_ctx_event;

    pst_event_mem = FRW_EVENT_ALLOC((oal_uint16)ul_len);
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{dmac_rx_send_event::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_hmac_to_dmac_ctx_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CRX,
                       en_event,
                       (oal_uint16)ul_len,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_dmac_vap->st_vap_base_info.uc_chip_id,
                       pst_dmac_vap->st_vap_base_info.uc_device_id,
                       pst_dmac_vap->st_vap_base_info.uc_vap_id);

    oal_memcopy(pst_hmac_to_dmac_ctx_event->auc_event_data,
                puc_param, ul_len);
    /* 分发 */
    frw_event_dispatch_event(pst_event_mem);

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}



oal_uint32  dmac_rx_get_vap(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_hal_vap_id, oal_uint8 uc_rx_queue_id, mac_vap_stru **pst_vap)
{
    hal_to_dmac_vap_stru       *pst_hal_vap  = OAL_PTR_NULL;
    oal_uint8                   uc_mac_vap_id;

#if (defined _PRE_WLAN_FEATURE_DBAC) || (defined _PRE_WLAN_FEATURE_PROXYSTA) || (defined _PRE_WLAN_DFT_STAT)
    mac_device_stru            *pst_device;

    pst_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{dmac_rx_get_vap::pst_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

#ifdef _PRE_WLAN_DFT_STAT
    if ((WLAN_HAL_OTHER_BSS_UCAST_ID == uc_hal_vap_id)
        && (OAL_TRUE == pst_device->st_dbb_env_param_ctx.en_non_directed_frame_stat_flg))
    {
        pst_device->st_dbb_env_param_ctx.ul_non_directed_frame_num++;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (mac_is_proxysta_enabled(pst_device))
    {
        /* 如果开启Proxy STA特性，则hal_vap_id的合法范围为0~4,16~31 */
        if (uc_hal_vap_id > WLAN_PROXY_STA_END_ID)
        {
            OAM_ERROR_LOG2(0, OAM_SF_RX, "{dmac_rx_get_vap:invalid vap id=%d max=%d(proxysta enabled)}", uc_hal_vap_id, WLAN_PROXY_STA_END_ID);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }
    else
#endif
    {
        /*lint -e774*/
        if ((uc_hal_vap_id > WLAN_HAL_OHTER_BSS_ID) && (!HAL_VAP_ID_IS_VALID(uc_hal_vap_id)))
        {
            //OAM_WARNING_LOG2(0, OAM_SF_RX, "{dmac_rx_get_vap:invalid vap id=%d max=%d}", uc_hal_vap_id, WLAN_HAL_OHTER_BSS_ID);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
        /*lint +e774*/
    }

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if ((mac_is_proxysta_enabled(pst_device) && ((HAL_VAP_ID_IS_VALID(uc_hal_vap_id))
          ||((uc_hal_vap_id >= WLAN_PROXY_STA_START_ID) && (uc_hal_vap_id <= WLAN_PROXY_STA_END_ID))))
        || (!mac_is_proxysta_enabled(pst_device) && (HAL_VAP_ID_IS_VALID(uc_hal_vap_id))))
#else
    if (HAL_VAP_ID_IS_VALID(uc_hal_vap_id))
#endif
     /* 通过hal接口获取mac层vap id */
    {
        hal_get_hal_vap(pst_hal_device, uc_hal_vap_id, &pst_hal_vap);
        if (OAL_PTR_NULL == pst_hal_vap)
        {
            OAM_INFO_LOG1(0, OAM_SF_RX, "{dmac_rx_get_vap:get vap faild, hal vap id=%d}", uc_hal_vap_id);
            *pst_vap = OAL_PTR_NULL;
            return OAL_ERR_CODE_PTR_NULL;
        }

        uc_mac_vap_id = pst_hal_vap->uc_mac_vap_id;
    }
    else    /* 广播帧使用配置vap进行操作 */
    {
        uc_mac_vap_id = 0;

        /* 其它BSS的广播数据帧直接过滤掉 */
        if (HAL_RX_DSCR_NORMAL_PRI_QUEUE == uc_rx_queue_id)
        {
            OAM_WARNING_LOG0(0, OAM_SF_PROXYSTA, "{dmac_rx_get_vap::broadcast packets from other bss, should be filtered}");
            return OAL_FAIL;
        }
    }

    *pst_vap = (mac_vap_stru *)mac_res_get_dmac_vap(uc_mac_vap_id);
    if (OAL_PTR_NULL == *pst_vap)
    {
        OAM_ERROR_LOG0(uc_mac_vap_id, OAM_SF_RX, "{dmac_rx_get_vap::pst_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_HW_TEST
    if (HAL_ALWAYS_RX_DISABLE == pst_hal_device->bit_al_rx_flag)
#endif
    {
        /* 数据帧情况下，判断VAP是否已经UP */
        if (((*pst_vap)->en_vap_state != MAC_VAP_STATE_UP) && (HAL_RX_DSCR_NORMAL_PRI_QUEUE == uc_rx_queue_id))
        {
            if ((*pst_vap)->en_vap_state == MAC_VAP_STATE_PAUSE
            #ifdef _PRE_WLAN_FEATURE_ROAM
                || (*pst_vap)->en_vap_state == MAC_VAP_STATE_ROAMING
            #endif //_PRE_WLAN_FEATURE_ROAM
                )
            {
                return OAL_SUCC;
            }

            OAM_WARNING_LOG2(uc_mac_vap_id, OAM_SF_RX, "{dmac_rx_get_vap::the vap is not up! en_dscr_queue_id=%d en_vap_state=%d.",
                             uc_rx_queue_id, (*pst_vap)->en_vap_state);
#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

            /* 丢包统计 */
            (*pst_vap)->st_vap_stats.ul_rx_vap_non_up_dropped++;
#endif
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}




#ifdef _PRE_WLAN_DFT_DUMP_FRAME
oal_uint32  dmac_rx_compare_frametype_and_rxq(
                oal_uint8                           uc_frametype,
                hal_rx_dscr_queue_id_enum_uint8     en_q,
                dmac_rx_ctl_stru                   *pst_cb_ctrl,
                oal_netbuf_stru                    *pst_curr_netbuf)
#else
oal_uint32  dmac_rx_compare_frametype_and_rxq(
                oal_uint8                           uc_frametype,
                hal_rx_dscr_queue_id_enum_uint8     en_q)
#endif
{
#ifdef _PRE_WLAN_DFT_DUMP_FRAME
    oal_uint8                    *puc_payload;
    mac_ieee80211_frame_stru     *pst_frame_hdr;
#endif

    /* 数据帧 上报的队列是高优先级队列，异常 */
    if (WLAN_FC0_TYPE_DATA == uc_frametype && HAL_RX_DSCR_HIGH_PRI_QUEUE == en_q)
    {
        OAM_WARNING_LOG0(0, OAM_SF_RX, "{dmac_rx_compare_frametype_and_rxq::data frame, but hi rx q.}");
    }
    else if (WLAN_FC0_TYPE_DATA != uc_frametype && HAL_RX_DSCR_NORMAL_PRI_QUEUE == en_q)
    {
        OAM_WARNING_LOG0(0, OAM_SF_RX, "{dmac_rx_compare_frametype_and_rxq::mgmt frame, but normal rx q.}");
    }
    else
    {
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_DFT_DUMP_FRAME
    puc_payload = MAC_GET_RX_PAYLOAD_ADDR(&(pst_cb_ctrl->st_rx_info), pst_curr_netbuf);
    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_cb_ctrl->st_rx_info));

    if (OAL_SWITCH_ON == oam_report_data_get_global_switch(OAM_OTA_FRAME_DIRECTION_TYPE_TX))
    {
        oam_report_80211_frame(BROADCAST_MACADDR,
                               (oal_uint8 *)(pst_frame_hdr),
                               MAC_GET_RX_CB_MAC_HEADER_LEN(&(pst_cb_ctrl->st_rx_info)),
                               puc_payload,
                               pst_cb_ctrl->st_rx_info.us_frame_len,
                               OAM_OTA_FRAME_DIRECTION_TYPE_TX);

        if (OAL_SUCC != oam_report_netbuf_cb(BROADCAST_MACADDR, (oal_uint8 *)pst_cb_ctrl, OAM_OTA_TYPE_RX_DMAC_CB))
        {
            OAM_WARNING_LOG0(0, OAM_SF_RX, "{dmac_rx_compare_frametype_and_rxq::oam_report_netbuf_cb return err: 0x%x.}\r\n");
            return OAL_FAIL;
        }
    }
#endif

    return OAL_FAIL;
}


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_bool_enum_uint8 dmac_check_arp_req_owner(oal_netbuf_stru *pst_netbuf, oal_uint8 uc_vap_id, oal_uint8 uc_data_type)
{
#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    dmac_vap_stru         *pst_dmac_vap;
    mac_llc_snap_stru     *pst_rx_snap;
    oal_eth_arphdr_stru   *pst_rx_arp_hdr;
#endif

    if (uc_data_type != MAC_DATA_ARP_REQ)
    {
        return OAL_FALSE;
    }

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    pst_rx_snap = (mac_llc_snap_stru *)oal_netbuf_payload(pst_netbuf);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_rx_snap))
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_RX, "{dmac_check_arp_req_owner::Get netbuf snap payload failed.}");
        return OAL_FALSE;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG1(uc_vap_id, OAM_SF_RX, "{dmac_check_arp_req_owner::pst_dmac_vap[%d] null.}", uc_vap_id);
        return OAL_FALSE;
    }

    pst_rx_arp_hdr = (oal_eth_arphdr_stru *)(pst_rx_snap + 1);
    return dmac_ao_is_ipv4_addr_owner(pst_dmac_vap, pst_rx_arp_hdr->auc_ar_tip);
#else
    /* dmac侧未保存IP地址，不能判断arp帧是否属于自己 */
    return OAL_FALSE;
#endif
}
#endif /*(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


