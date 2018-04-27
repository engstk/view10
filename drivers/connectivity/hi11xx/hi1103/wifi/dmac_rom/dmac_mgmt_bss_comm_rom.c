


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "wlan_spec.h"
#include "wlan_mib.h"

/* TBD，待整改 保留hal_ext_if.h*/
#include "frw_ext_if.h"

#include "hal_ext_if.h"

//#include "hal_spec.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#include "mac_frame.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_blockack.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_mgmt_sta.h"
#include "dmac_psm_ap.h"
#include "dmac_scan.h"

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#include "pm_extern.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_MGMT_BSS_COMM_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

dmac_mgmt_bss_cb g_st_mgmt_bss_rom_cb = {OAL_PTR_NULL};


/*****************************************************************************
  3 函数实现
*****************************************************************************/


oal_uint16  dmac_ba_encap_blockack_req(
                dmac_vap_stru     *pst_dmac_vap,
                oal_netbuf_stru   *pst_netbuf,
                dmac_ba_tx_stru   *pst_ba_tx_hdl,
                oal_uint8          uc_tid)
{
    oal_uint8    *puc_mac_hdr;
    oal_uint8    *puc_payload;
    oal_uint16    us_bar_ctl =0;

    if ((OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_netbuf) || (OAL_PTR_NULL == pst_ba_tx_hdl))
    {
        OAM_ERROR_LOG3(0, OAM_SF_BA,
                       "{dmac_ba_encap_blockack_req::param null, pst_dmac_vap=%d puc_data=%d pst_ba_tx_hdl=%d.}",
                       pst_dmac_vap, pst_netbuf, pst_ba_tx_hdl);
        return 0;
    }

    puc_mac_hdr = (oal_uint8 *)OAL_NETBUF_HEADER(pst_netbuf);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    puc_payload = OAL_NETBUF_PAYLOAD(pst_netbuf);
#else
    puc_payload = puc_mac_hdr + MAC_80211_CTL_HEADER_LEN;
#endif

    /*************************************************************************/
    /*                     BlockAck Request Frame Format                     */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BAR Control|BlockAck Starting    |FCS|  */
    /* |             |        |  |  |           |Sequence number      |   |  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |2          |2                    |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/

    /* All the fields of the Frame Control Field are set to zero. Only the   */
    /* Type/Subtype field is set.                                            */
    mac_hdr_set_frame_control(puc_mac_hdr, (oal_uint16)WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_CTL | WLAN_FC0_SUBTYPE_BAR);

    /* Set DA to the address of the STA requesting authentication */
    oal_set_mac_addr(puc_mac_hdr + 4, pst_ba_tx_hdl->puc_dst_addr);

    /* Set SA to the dot11MacAddress */
    oal_set_mac_addr(puc_mac_hdr + 10, mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info));

    /*************************************************************************/
    /*                Set the fields in the frame payload                     */
    /*************************************************************************/
    /* BAR Control field */
    us_bar_ctl = (oal_uint16)(uc_tid << 12);

    /* BAR-Ack Policy is set to Normal Ack */
    us_bar_ctl &= ~BIT0;

    /* Multi-TID set to 0 */
    us_bar_ctl |= BIT2;

    puc_payload[0] = us_bar_ctl & 0xFF;
    puc_payload[1] = (us_bar_ctl >> 8) & 0xFF;

    /* Sequence number */
    puc_payload[2] = (oal_uint8)(pst_ba_tx_hdl->us_baw_start<< 4);
    puc_payload[3] = (oal_uint8)((pst_ba_tx_hdl->us_baw_start >> 4) & 0xFF);

    return WLAN_MAX_BAR_DATA_LEN;
}



oal_uint32  dmac_mgmt_delba(
                dmac_vap_stru          *pst_dmac_vap,
                dmac_user_stru         *pst_dmac_user,
                oal_uint8               uc_tid,
                oal_uint8               uc_initiator,
                oal_uint8               uc_reason)
{
    frw_event_mem_stru             *pst_event_mem;      /* 申请事件返回的内存指针 */
    frw_event_stru                 *pst_hmac_to_dmac_ctx_event;
    dmac_ctx_action_event_stru     *pst_wlan_ctx_action;

    if ((OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BA, "{dmac_mgmt_delba::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 抛事件到HMAC生成DELBA帧 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_ctx_action_event_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA, "{dmac_mgmt_delba::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_hmac_to_dmac_ctx_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_hmac_to_dmac_ctx_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CRX,
                       DMAC_WLAN_CRX_EVENT_SUB_TYPE_DELBA,
                       OAL_SIZEOF(dmac_ctx_action_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_dmac_vap->st_vap_base_info.uc_chip_id,
                       pst_dmac_vap->st_vap_base_info.uc_device_id,
                       pst_dmac_vap->st_vap_base_info.uc_vap_id);

    /*填写事件payload */
    pst_wlan_ctx_action = (dmac_ctx_action_event_stru *)(pst_hmac_to_dmac_ctx_event->auc_event_data);
    pst_wlan_ctx_action->en_action_category  = MAC_ACTION_CATEGORY_BA;
    pst_wlan_ctx_action->uc_action           = MAC_BA_ACTION_DELBA;
    pst_wlan_ctx_action->uc_tidno            = uc_tid;
    pst_wlan_ctx_action->uc_status           = uc_reason;
    pst_wlan_ctx_action->uc_initiator        = uc_initiator;
    oal_memcopy(pst_wlan_ctx_action->auc_mac_addr, pst_dmac_user->st_user_base_info.auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 分发 */
    frw_event_dispatch_event(pst_event_mem);

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}


oal_uint32  dmac_mgmt_rx_ampdu_end(
                mac_device_stru                *pst_device,
                dmac_vap_stru                  *pst_dmac_vap,
                mac_priv_req_args_stru         *pst_crx_req_args)
{
    oal_uint8               uc_tidno;
    dmac_user_stru         *pst_dmac_user;

    if ((OAL_PTR_NULL == pst_device) || (OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_crx_req_args))
    {
        OAM_ERROR_LOG3(0, OAM_SF_AMPDU,
                       "{dmac_mgmt_rx_ampdu_end::param null, pst_device=%d pst_dmac_vap=%d pst_crx_req_args=%d.}",
                       pst_device, pst_dmac_vap, pst_crx_req_args);

        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_tidno        = pst_crx_req_args->uc_arg1;                    /* AMPDU_START时，uc_arg1代表对应的tid队列号 */
    pst_dmac_user   = (dmac_user_stru *)mac_res_get_dmac_user(pst_crx_req_args->us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_AMPDU, "{dmac_mgmt_rx_ampdu_end::pst_dmac_user[%d] null.}",
            pst_crx_req_args->us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }
  /* 获取对应的TID队列 */
    pst_dmac_user->ast_tx_tid_queue[uc_tidno].en_tx_mode  = DMAC_TX_MODE_NORMAL;
    dmac_reset_tx_ampdu_session(pst_dmac_vap->pst_hal_device, pst_dmac_user, uc_tidno);

    return OAL_SUCC;
}




oal_uint16  dmac_mgmt_encap_csa_action(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buffer, oal_uint8 uc_channel, oal_uint8 uc_csa_cnt, wlan_channel_bandwidth_enum_uint8 en_bw)
{
    oal_uint8        uc_len           = 0;
    oal_uint8       *puc_mac_header   = oal_netbuf_header(pst_buffer);
    oal_uint8       *puc_payload_addr = mac_netbuf_get_payload(pst_buffer);
    oal_uint8       *puc_payload_addr_origin = puc_payload_addr;
    oal_uint16       us_frame_length  = 0;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/

    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* 设置地址1，广播地址 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, BROADCAST_MACADDR);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, pst_dmac_vap->st_vap_base_info.auc_bssid);

    /* 地址3，为VAP自己的MAC地址 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_dmac_vap->st_vap_base_info.auc_bssid);

    /*************************************************************************/
    /*                Set the fields in the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*                       Channel Switch Announcement Frame - Frame Body  */
    /* --------------------------------------------------------------------- */
    /* |Category |Action |Ch switch IE| 2nd Ch offset|Wide bw IE (11ac only) */
    /* --------------------------------------------------------------------- */
    /* |1        |1      | 5          |  3           |5                      */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /* 设置Action的Category   */
    /* 0: Spectrum Management */
    puc_payload_addr[0] = 0;

    /* 设置Spectrum Management Action Field */
    /* 4: Channel Switch Announcement Frame */
    puc_payload_addr[1] = 4;

    puc_payload_addr += 2;

    /* 开始封装信道切换所需的各种IE */

    /* 封装CSA IE */
    if (OAL_SUCC != mac_set_csa_ie(pst_dmac_vap->st_vap_base_info.st_ch_switch_info.en_csa_mode,uc_channel, uc_csa_cnt, puc_payload_addr, &uc_len))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "dmac_mgmt_encap_csa_action: build csa ie failed!");
        return 0;
    }
    puc_payload_addr += uc_len;

    /* 封装Second channel offset IE */
    if (OAL_SUCC != mac_set_second_channel_offset_ie(en_bw, puc_payload_addr, &uc_len))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_mgmt_encap_csa_action::mac_set_second_channel_offset_ie failed}");
        return 0;
    }
    puc_payload_addr += uc_len;

    if (OAL_TRUE != mac_mib_get_VHTOptionImplemented(&pst_dmac_vap->st_vap_base_info))
    {
        return (oal_uint16)((puc_payload_addr - puc_payload_addr_origin) + MAC_80211_FRAME_LEN);
    }

    /* 11AC Wide Bandwidth Channel Switch IE */
    if (OAL_SUCC != mac_set_11ac_wideband_ie(uc_channel, en_bw, puc_payload_addr, &uc_len))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_mgmt_encap_csa_action::mac_set_11ac_wideband_ie failed}");
        return 0;
    }
    puc_payload_addr += uc_len;

    us_frame_length = ((oal_uint16)(puc_payload_addr - puc_payload_addr_origin) + MAC_80211_FRAME_LEN);

    if (OAL_PTR_NULL != g_st_mgmt_bss_rom_cb.encap_csa_action_cb)
    {
        g_st_mgmt_bss_rom_cb.encap_csa_action_cb(pst_dmac_vap, pst_buffer, puc_payload_addr, &us_frame_length);
    }


    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_DFS, "{dmac_mgmt_encap_csa_action::LEN = %d.}", us_frame_length);

    return us_frame_length;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

oal_uint32  dmac_mgmt_encap_ext_csa_action(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_buffer, oal_uint8 uc_opert_class, oal_uint8 uc_channel, oal_uint8 uc_csa_cnt, wlan_channel_bandwidth_enum_uint8 en_bw)
{
    oal_uint8       *puc_origin = puc_buffer;
    oal_uint8        uc_len;
    oal_uint32       ul_ret;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/
    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(puc_buffer, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_buffer, 0);

    /* 设置地址1，广播地址 */
    oal_set_mac_addr(puc_buffer + WLAN_HDR_ADDR1_OFFSET, BROADCAST_MACADDR);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(puc_buffer + WLAN_HDR_ADDR2_OFFSET, pst_dmac_vap->st_vap_base_info.auc_bssid);

    /* 地址3，为VAP自己的MAC地址 */
    oal_set_mac_addr(puc_buffer + WLAN_HDR_ADDR3_OFFSET, pst_dmac_vap->st_vap_base_info.auc_bssid);

    /*************************************************************************/
    /*                Set the fields in the frame body                       */
    /*************************************************************************/

    /****************************************************************************************************/
    /*                             Channel Switch Announcement Frame - Frame Body                       */
    /* ------------------------------------------------------------------------------------------------ */
    /* |Category |Action |Chan Swt Mode|New Opert Class|New Channel|Chan Swt Cnt|Wide bw IE (11ac only) */
    /* ------------------------------------------------------------------------------------------------ */
    /* |1        |1      |1            |1              |1          |1           |5                    | */
    /* ------------------------------------------------------------------------------------------------ */
    /*                                                                                                  */
    /****************************************************************************************************/
    puc_buffer += MAC_80211_FRAME_LEN;

    /* Category Field */
    puc_buffer[0] = MAC_ACTION_CATEGORY_PUBLIC;

    /* Action Field */
    puc_buffer[1] = MAC_PUB_EX_CH_SWITCH_ANNOUNCE;

    /* Channel Switch Mode */
    puc_buffer[2] = 1;

    /* New Operating Class */
    puc_buffer[3] = uc_opert_class;

    /* New Channel Number */
    puc_buffer[4] = uc_channel;

    /* Channel Switch Count */
    puc_buffer[5] = uc_csa_cnt;

    puc_buffer += 6;

    /* 开始封装信道切换所需的各种IE */
    if (OAL_TRUE != mac_mib_get_VHTOptionImplemented(&pst_dmac_vap->st_vap_base_info))
    {
        return (oal_uint16)(puc_buffer - puc_origin);
    }

    /* 11AC Wide Bandwidth Channel Switch IE */
    ul_ret = mac_set_11ac_wideband_ie(uc_channel, en_bw, puc_buffer, &uc_len);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_mgmt_encap_ext_csa_action::mac_set_11ac_wideband_ie failed[%d].}", ul_ret);

        return 0;
    }
    puc_buffer += uc_len;

    return (oal_uint32)(puc_buffer - puc_origin);
}
#endif



oal_void  dmac_set_cap_info_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
#ifdef   _PRE_WLAN_FEATURE_P2P
    mac_cap_info_stru  *pst_cap_info = (mac_cap_info_stru *)puc_buffer;
    mac_vap_stru       *pst_mac_vap  = (mac_vap_stru *)pst_vap;
#endif

    mac_set_cap_info_ap(pst_vap, puc_buffer);

#ifdef   _PRE_WLAN_FEATURE_P2P
    if (dmac_vap_is_in_p2p_listen(pst_mac_vap))
    {
        pst_cap_info->bit_ess = 0;
    }
#endif
}

oal_uint8  dmac_get_dsss_ie_channel_num(mac_vap_stru *pst_mac_vap)
{
    dmac_vap_stru           *pst_dmac_vap;
    mac_device_stru         *pst_mac_device;
    oal_uint8               uc_chan_num = 0;

#if IS_DEVICE
    oal_uint8                uc_scan_chan_idx;
#endif

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "mac_set_dsss_params::device_id[%d] ERROR in vap_stru!", pst_mac_vap->uc_device_id);
        return uc_chan_num;
    }
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_get_dsss_ie_channel_num::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_chan_num = pst_mac_vap->st_channel.uc_chan_number;

#if IS_DEVICE
    uc_scan_chan_idx = GET_HAL_DEV_CURRENT_SCAN_IDX(pst_dmac_vap->pst_hal_device);

    if ((IS_STA(pst_mac_vap)) && (MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state))
    {
        uc_chan_num = pst_mac_device->st_scan_params.ast_channel_list[uc_scan_chan_idx].uc_chan_number;
    }
#endif

    return uc_chan_num;
}


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

