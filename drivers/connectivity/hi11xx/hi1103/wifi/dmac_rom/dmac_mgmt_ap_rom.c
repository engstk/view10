


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "mac_resource.h"
#include "dmac_main.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_mgmt_ap.h"
#include "dmac_mgmt_sta.h"
#include "oal_net.h"
#include "mac_ie.h"
#include "dmac_beacon.h"

#ifdef _PRE_WLAN_FEATURE_P2P
#include "dmac_p2p.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#include "dmac_bsd.h"
#endif



#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_MGMT_AP_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_mgmt_ap_cb g_st_dmac_mgmt_ap_rom_cb = {OAL_PTR_NULL};


/*****************************************************************************
  3 函数实现
*****************************************************************************/


oal_uint16  dmac_mgmt_encap_probe_response(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf, oal_uint8 *puc_ra, oal_bool_enum_uint8 en_is_p2p_req)
{
    oal_uint8        uc_ie_len = 0;
    mac_vap_stru    *pst_mac_vap    = &(pst_dmac_vap->st_vap_base_info);
    oal_uint8       *puc_mac_header = oal_netbuf_header(pst_netbuf);
    oal_uint8       *puc_payload_addr        = mac_netbuf_get_payload(pst_netbuf);
    oal_uint8       *puc_payload_addr_origin = puc_payload_addr;
    oal_uint16       us_app_ie_len;
    oal_uint8       uc_dsss_channel_num;

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
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_PROBE_RSP);

    /* 设置地址1为发送probe request帧的STA */
    if(OAL_PTR_NULL != puc_ra)
    {
        oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, puc_ra);
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    if (dmac_vap_is_in_p2p_listen(&(pst_dmac_vap->st_vap_base_info)))
    {
        /* 设置地址2为自己的MAC地址 */
        oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_p2p0_dot11StationID(&pst_dmac_vap->st_vap_base_info));

        /* 设置地址3为bssid */
        oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, mac_mib_get_p2p0_dot11StationID(&pst_dmac_vap->st_vap_base_info));
    }
    else
#endif
    {
        /* 设置地址2为自己的MAC地址 */
        oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info));

        /* 设置地址3为bssid */
        oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_dmac_vap->st_vap_base_info.auc_bssid);
    }

    /* 设置分片序号, 管理帧为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /*************************************************************************/
    /*                  Probe Response Frame - Frame Body                    */
    /* ----------------------------------------------------------------------*/
    /* |Timestamp|BcnInt|CapInfo|SSID|SupRates|DSParamSet      |CountryElem |*/
    /* ----------------------------------------------------------------------*/
    /* |8        |2     |2      |2-34|3-10    |3               |8-256       |*/
    /* ----------------------------------------------------------------------*/
    /* |PowerConstraint |Quiet|TPC Report|ERP |RSN  |WMM |Extended Sup Rates|*/
    /* ----------------------------------------------------------------------*/
    /* |3               |8    |4         |3   |4-255|26  | 3-257            |*/
    /* ----------------------------------------------------------------------*/
    /* |BSS Load |HT Capabilities |HT Operation |Overlapping BSS Scan       |*/
    /* ----------------------------------------------------------------------*/
    /* |7        |28              |24           |16                         |*/
    /* ----------------------------------------------------------------------*/
    /* |Extended Capabilities |                                              */
    /* ----------------------------------------------------------------------*/
    /* |3-8                   |                                              */
    /*************************************************************************/
    /* Initialize index */
    //puc_buffer += MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN;
    puc_payload_addr += MAC_TIME_STAMP_LEN;

    /* 设置beacon interval */
    mac_set_beacon_interval_field(pst_mac_vap, puc_payload_addr);
    puc_payload_addr += MAC_BEACON_INTERVAL_LEN;

    /* 设置capability information */
    dmac_set_cap_info_field(pst_mac_vap, puc_payload_addr);
    puc_payload_addr += MAC_CAP_INFO_LEN;

    /* 设置ssid element */
#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        mac_set_p2p0_ssid_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len, WLAN_FC0_SUBTYPE_PROBE_RSP);
        puc_payload_addr += uc_ie_len;
    }
    else
#endif
    {
        mac_set_ssid_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len, WLAN_FC0_SUBTYPE_PROBE_RSP);
        puc_payload_addr += uc_ie_len;
    }

    /* 设置支持的速率集 */
    mac_set_supported_rates_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 获取dsss ie内的channel num */
    uc_dsss_channel_num = dmac_get_dsss_ie_channel_num(pst_mac_vap);/* 设置dsss参数集 */

    mac_set_dsss_params(pst_mac_vap, puc_payload_addr, &uc_ie_len, uc_dsss_channel_num);
    puc_payload_addr += uc_ie_len;

#ifdef _PRE_WLAN_FEATURE_11D
    /* 填充country信息 */
    mac_set_country_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;
#endif

    /* 填充power constraint信息 */
    mac_set_pwrconstraint_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充quiet信息 */
    mac_set_quiet_ie(pst_mac_vap, puc_payload_addr, MAC_QUIET_COUNT, MAC_QUIET_PERIOD,
                     MAC_QUIET_DURATION, MAC_QUIET_OFFSET, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充TPC Report信息 */
    mac_set_tpc_report_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

#if defined(_PRE_WLAN_FEATURE_11K_EXTERN) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)
    mac_set_rrm_enabled_cap_field((oal_void *)pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;
#endif

    /* 填充erp信息 */
    mac_set_erp_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充RSN 安全相关信息 */
    mac_set_rsn_ie(pst_mac_vap, OAL_PTR_NULL, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充WPA 安全相关信息 */
    mac_set_wpa_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充wmm信息 */
    //mac_set_wmm_params_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    mac_set_wmm_params_ie(pst_mac_vap, puc_payload_addr, mac_mib_get_dot11QosOptionImplemented(pst_mac_vap), &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充extended supported rates信息 */
    mac_set_exsup_rates_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充bss load信息 */
    mac_set_bssload_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充HT Capabilities信息 */
    mac_set_ht_capabilities_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充HT Operation信息 */
    mac_set_ht_opern_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充Overlapping BSS Scan信息 */
    mac_set_obss_scan_params(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充Extended Capabilities信息 */
    mac_set_ext_capabilities_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充vht cap信息 */
    mac_set_vht_capabilities_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

    /* 填充vht opern信息 */
    mac_set_vht_opern_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    mac_set_opmode_notify_ie((oal_void *)pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;
#endif

#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_set_11ntxbf_vendor_ie(pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;
#endif

    if(OAL_TRUE == en_is_p2p_req)
    {
        /* 填充WPS P2P信息 */
        mac_add_app_ie((oal_void *)pst_mac_vap, puc_payload_addr, &us_app_ie_len, OAL_APP_PROBE_RSP_IE);
    }
    else
    {
        /* 对于非p2p 设备发起的扫描，回复probe resp 帧不能携带p2p ie 信息，回复WPS 信息 */
        mac_add_wps_ie((oal_void *)pst_mac_vap, puc_payload_addr, &us_app_ie_len, OAL_APP_PROBE_RSP_IE);
    }
    puc_payload_addr += us_app_ie_len;
#ifdef _PRE_WLAN_FEATURE_HILINK
    /* 填充okc ie信息 */
    mac_add_app_ie((oal_void *)pst_mac_vap, puc_payload_addr, &us_app_ie_len, OAL_APP_OKC_PROBE_IE);
    puc_payload_addr += us_app_ie_len;
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_add_app_ie((oal_void *)pst_mac_vap, puc_payload_addr, &us_app_ie_len, OAL_APP_VENDOR_IE);
    puc_payload_addr += us_app_ie_len;
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
    /* 填充p2p noa Attribute*/
    if((OAL_TRUE == en_is_p2p_req)&&
        (IS_P2P_GO(&pst_dmac_vap->st_vap_base_info))&&
        (IS_P2P_NOA_ENABLED(pst_dmac_vap) || IS_P2P_OPPPS_ENABLED(pst_dmac_vap)))
    {
        mac_set_p2p_noa(pst_mac_vap, puc_payload_addr, &uc_ie_len);
        puc_payload_addr += uc_ie_len;
    }
#endif

#ifdef _PRE_WLAN_NARROW_BAND
    if (pst_mac_vap->st_nb.en_open)
    {
        mac_set_nb_ie(puc_payload_addr, &uc_ie_len);
        puc_payload_addr += uc_ie_len;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_11R_AP
        mac_set_md_ie((oal_void *)pst_mac_vap, puc_payload_addr, &uc_ie_len);
        puc_payload_addr += uc_ie_len;
#endif
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
    /* multi-sta特性下新增4地址ie */
    mac_set_vender_4addr_ie((oal_void *)pst_mac_vap, puc_payload_addr, &uc_ie_len);
    puc_payload_addr += uc_ie_len;
#endif

    if (OAL_PTR_NULL != g_st_dmac_mgmt_ap_rom_cb.encap_probe_rsp_cb)
    {
        g_st_dmac_mgmt_ap_rom_cb.encap_probe_rsp_cb(pst_dmac_vap, pst_netbuf, puc_payload_addr);
    }

    return (oal_uint16)((puc_payload_addr - puc_payload_addr_origin) + MAC_80211_FRAME_LEN);
}


oal_uint32 dmac_ap_check_ssid(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_probe_req_ssid, oal_uint8 *puc_sta_mac)
{
    oal_uint8           uc_prb_req_ssid_len;
    oal_uint8           uc_ssid_len;
    oal_uint8          *puc_ssid;
#ifdef _PRE_WLAN_FEATURE_P2P
    oal_uint8          *puc_prb_req_ssid;
#endif
    oal_uint8           uc_hide_ssid;
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    oal_uint16                              us_user_idx;
    oal_uint32                              ul_ret;
    dmac_user_stru                         *pst_dmac_user;
    oal_bool_enum_uint8                     en_user_asso = OAL_FALSE;
#endif
#ifdef _PRE_WLAN_FEATURE_HILINK
    oal_dlist_head_stru                    *pst_entry;
    oal_dlist_head_stru                    *pst_entry_temp;
    mac_okc_ssid_hidden_white_list_stru    *pst_okc_ssid_hidden_white_list;
    mac_okc_white_list_member_stru         *pst_hmac_okc_ssid_hidden_white;
    oal_uint8                               auc_hilink_target_mac[WLAN_MAC_ADDR_LEN];

    pst_okc_ssid_hidden_white_list = &(pst_dmac_vap->st_vap_base_info.st_okc_ssid_hidden_white_list);
    oal_memcopy(auc_hilink_target_mac, puc_sta_mac, WLAN_MAC_ADDR_LEN);
#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    /* 判断该mac对应的user的关联状态 */
    ul_ret = mac_vap_find_user_by_macaddr(&(pst_dmac_vap->st_vap_base_info), auc_hilink_target_mac, &us_user_idx);
    if (OAL_SUCC == ul_ret)
    {
        pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
        if (OAL_PTR_NULL != pst_dmac_user)
        {
            if (MAC_USER_STATE_ASSOC == pst_dmac_user->st_user_base_info.en_user_asoc_state)
            {
                en_user_asso = OAL_TRUE;
            }
        }
    }
#endif

    uc_prb_req_ssid_len = puc_probe_req_ssid[1];
#ifdef _PRE_WLAN_FEATURE_P2P
    puc_prb_req_ssid    = &puc_probe_req_ssid[MAC_IE_HDR_LEN];
    /* P2P device 检查ssid 是否为"DIRECT-"
        P2P GO 需要和AP 一样，检查probe req ssid 内容 */
    if (IS_P2P_CL(&pst_dmac_vap->st_vap_base_info)|| IS_P2P_DEV(&pst_dmac_vap->st_vap_base_info))
    {
        if (IS_P2P_WILDCARD_SSID(puc_prb_req_ssid, uc_prb_req_ssid_len))
        {
            return OAL_SUCC;
        }
        else
        {
            return OAL_FAIL;
        }
    }

    if (IS_P2P_GO(&pst_dmac_vap->st_vap_base_info) && IS_P2P_WILDCARD_SSID(puc_prb_req_ssid, uc_prb_req_ssid_len))
    {
        return OAL_SUCC;
    }
#endif

    puc_ssid = mac_mib_get_DesiredSSID(&pst_dmac_vap->st_vap_base_info);
    uc_hide_ssid = pst_dmac_vap->st_vap_base_info.st_cap_flag.bit_hide_ssid;


    if(0 == uc_prb_req_ssid_len)
    {
        /* 隐藏ssid配置下，不接受ssid len为0的情况 */
        if(uc_hide_ssid)
        {
#ifdef _PRE_WLAN_FEATURE_HILINK
            /* 加锁保护 */
            oal_spin_lock(&(pst_okc_ssid_hidden_white_list->st_lock));
            OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_entry_temp, &(pst_okc_ssid_hidden_white_list->st_list_head))
            {
                pst_hmac_okc_ssid_hidden_white = OAL_DLIST_GET_ENTRY(pst_entry, mac_okc_white_list_member_stru, st_dlist);
                if (0 == oal_memcmp(pst_hmac_okc_ssid_hidden_white->auc_mac_addr, auc_hilink_target_mac, WLAN_MAC_ADDR_LEN))
                {
#ifndef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
                    oal_spin_unlock(&(pst_okc_ssid_hidden_white_list->st_lock));
                    return OAL_SUCC;
#else
                    /*hilink白名单直接回复probe req(优先级高于已关联过的STA列表)*/
                    if (pst_hmac_okc_ssid_hidden_white->uc_flag & (oal_uint8)BIT0)
                    {
                        oal_spin_unlock(&(pst_okc_ssid_hidden_white_list->st_lock));
                        return OAL_SUCC;
                    }
                    else
                    {
                        /* STA已关联上AP，回复probe resp次数不统计累加 */
                        if (en_user_asso)
                        {
                            oal_spin_unlock(&(pst_okc_ssid_hidden_white_list->st_lock));
                            return OAL_SUCC;
                        }
                        else
                        {
                            /*回复probe resp次数超过50次不在继续回复*/
                            if (pst_hmac_okc_ssid_hidden_white->uc_resp_nums > MAC_SENT_PROBE_RESPONSE_CNT)
                            {
                                oal_spin_unlock(&(pst_okc_ssid_hidden_white_list->st_lock));
                                return OAL_FAIL;
                            }
                            else
                            {
                                /*统计回复probe resp的次数*/
                                pst_hmac_okc_ssid_hidden_white->uc_resp_nums++;
                                oal_spin_unlock(&(pst_okc_ssid_hidden_white_list->st_lock));
                                return OAL_SUCC;
                            }
                        }
                    }
#endif
                }
            }
            oal_spin_unlock(&(pst_okc_ssid_hidden_white_list->st_lock));
            return OAL_FAIL;
#else
            //OAM_INFO_LOG0(0, OAM_SF_SCAN, "{dmac_ap_check_ssid::hide_ssid && ssid len==0 .}");
            return OAL_FAIL;
#endif
        }
        return OAL_SUCC;
    }

    uc_ssid_len = (oal_uint8)OAL_STRLEN((oal_int8*)puc_ssid);
    if (uc_prb_req_ssid_len != uc_ssid_len)
    {
        return OAL_FAIL;
    }

    return ((0 == oal_memcmp(&puc_probe_req_ssid[MAC_IE_HDR_LEN], puc_ssid, uc_prb_req_ssid_len)) ? OAL_SUCC : OAL_FAIL);
    /* return OAL_SUCC; */

}


oal_bool_enum_uint8  dmac_ap_check_probe_req(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_probe_req, mac_ieee80211_frame_stru *pst_frame_hdr)
{
    oal_uint8 *puc_bssid;

    puc_bssid = pst_frame_hdr->auc_address3;
    if(OAL_SUCC != dmac_ap_check_ssid(pst_dmac_vap, puc_probe_req, pst_frame_hdr->auc_address2))
    {
        //OAM_INFO_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_ap_check_probe_req::probe req check ssid error.}\r\n");
        return OAL_FALSE;
    }

    /* 检查probe request帧中的bssid是不是本AP的bssid或者广播地址 */
    if ((0 != oal_compare_mac_addr(BROADCAST_MACADDR, puc_bssid)) &&
        (0 != oal_compare_mac_addr(pst_dmac_vap->st_vap_base_info.auc_bssid, puc_bssid)))
    {
        //OAM_INFO_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_ap_check_probe_req::probe req check ssid err.}\r\n");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT


oal_bool_enum_uint8 dmac_ap_check_probe_rej_sta(dmac_vap_stru *pst_dmac_vap, mac_ieee80211_frame_stru *pst_frame_hdr)
{
    mac_fbt_mgmt_stru              *pst_fbt_mgmt       = OAL_PTR_NULL;
    oal_uint8                       uc_tmp_idx;
    oal_bool_enum_uint8             en_need_resp = OAL_TRUE;
    mac_fbt_disable_user_info_stru *pst_dis_user = OAL_PTR_NULL;

    pst_fbt_mgmt = &(pst_dmac_vap->st_vap_base_info.st_fbt_mgmt);

    for (uc_tmp_idx = 0; uc_tmp_idx < pst_fbt_mgmt->uc_disabled_user_cnt; uc_tmp_idx++)
    {
        pst_dis_user = &(pst_fbt_mgmt->ast_fbt_disable_connect_user_list[uc_tmp_idx]);
        if (0 == oal_memcmp(pst_dis_user->auc_user_mac_addr, pst_frame_hdr->auc_address2, WLAN_MAC_ADDR_LEN))
        {
            if (pst_dis_user->uc_mlme_phase_mask & IEEE80211_MLME_PHASE_PROB)
            {
                en_need_resp = OAL_FALSE;
            }
            break;
        }
    }
    return en_need_resp;
}

#endif


oal_bool_enum_uint8  dmac_chan_is_40MHz_scb_allowed(
                mac_vap_stru                 *pst_mac_vap,
                dmac_eval_scan_report_stru   *pst_chan_scan_report,
                oal_uint8                     uc_pri_chan_idx,
                mac_sec_ch_off_enum_uint8     en_user_chan_offset)
{
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    wlan_channel_band_enum_uint8    en_band = pst_mac_vap->st_channel.en_band;
    oal_uint8                       uc_sec_ch_idx_offset = mac_get_sec_ch_idx_offset(en_band);
    oal_uint8                       uc_sec_chan_idx = 0;
    oal_uint32                      ul_ret;

    OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
            "{dmac_chan_is_40MHz_sca_allowed::uc_pri_chan_idx=%d, en_user_chan_offset=%d}",
            uc_pri_chan_idx, en_user_chan_offset);

    if (MAC_SCA == en_user_chan_offset)
    {
        return OAL_FALSE;
    }

    if ((OAL_TRUE == mac_mib_get_2040BSSCoexistenceManagementSupport(pst_mac_vap)) &&
        (WLAN_BAND_2G == en_band))
    {
        if (!(pst_chan_scan_report[uc_pri_chan_idx].en_chan_op & DMAC_SCB_ALLOWED))
        {
            return OAL_FALSE;
        }
    }

    if (uc_pri_chan_idx >= uc_sec_ch_idx_offset)
    {
        uc_sec_chan_idx = uc_pri_chan_idx - uc_sec_ch_idx_offset;
    }
    else
    {
        return OAL_FALSE;
    }

    ul_ret = mac_is_channel_idx_valid(en_band, uc_sec_chan_idx);
    if (OAL_SUCC != ul_ret)
    {
        return OAL_FALSE;
    }

    return OAL_TRUE;
#else
    return OAL_FALSE;
#endif
}


oal_void  dmac_chan_init_chan_scan_report(
                mac_vap_stru                 *pst_mac_vap,
                dmac_eval_scan_report_stru   *pst_chan_scan_report,
                oal_uint8                     uc_num_supp_chan)
{
    oal_uint8           uc_idx;
    mac_device_stru    *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_chan_init_chan_scan_report::mac_res_get_dev[%d] Error.}",
        pst_mac_vap->uc_device_id);
        return;
    }

    OAL_MEMZERO(pst_chan_scan_report, uc_num_supp_chan * OAL_SIZEOF(*pst_chan_scan_report));

    for (uc_idx = 0; uc_idx < uc_num_supp_chan; uc_idx++)
    {
    #ifdef _PRE_WLAN_FEATURE_DFS
        if (OAL_TRUE == mac_vap_get_dfs_enable(pst_mac_vap))
        {
            if ((MAC_CHAN_NOT_SUPPORT        != pst_mac_device->st_ap_channel_list[uc_idx].en_ch_status) &&
                (MAC_CHAN_BLOCK_DUE_TO_RADAR != pst_mac_device->st_ap_channel_list[uc_idx].en_ch_status))
            {
                pst_chan_scan_report[uc_idx].en_chan_op |= DMAC_OP_ALLOWED;
            }
        }
        else
    #endif
        {
            pst_chan_scan_report[uc_idx].en_chan_op |= DMAC_OP_ALLOWED;
        }
    }
}


oal_bool_enum_uint8  dmac_chan_is_40MHz_sca_allowed(
                mac_vap_stru                 *pst_mac_vap,
                dmac_eval_scan_report_stru   *pst_chan_scan_report,
                oal_uint8                     uc_pri_chan_idx,
                mac_sec_ch_off_enum_uint8     en_user_chan_offset)
{
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    wlan_channel_band_enum_uint8    en_band = pst_mac_vap->st_channel.en_band;
    oal_uint8                       uc_num_supp_chan = mac_get_num_supp_channel(en_band);
    oal_uint8                       uc_sec_chan_idx = 0;
    oal_uint32                      ul_ret;

    OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_2040,
            "{dmac_chan_is_40MHz_sca_allowed::uc_pri_chan_idx=%d, en_user_chan_offset=%d}",
            uc_pri_chan_idx, en_user_chan_offset);

    if (MAC_SCB == en_user_chan_offset)
    {
        return OAL_FALSE;
    }

    if ((OAL_TRUE == mac_mib_get_2040BSSCoexistenceManagementSupport(pst_mac_vap)) &&
        (WLAN_BAND_2G == en_band))
    {
        if (!(pst_chan_scan_report[uc_pri_chan_idx].en_chan_op & DMAC_SCA_ALLOWED))
        {
            return OAL_FALSE;
        }
    }

    uc_sec_chan_idx = uc_pri_chan_idx + mac_get_sec_ch_idx_offset(en_band);
    if (uc_sec_chan_idx >= uc_num_supp_chan)
    {
        return OAL_FALSE;
    }

    ul_ret = mac_is_channel_idx_valid(en_band, uc_sec_chan_idx);
    if (OAL_SUCC != ul_ret)
    {
        return OAL_FALSE;
    }

    return OAL_TRUE;
#else
    return OAL_FALSE;
#endif
}


oal_uint16  dmac_chan_get_cumulative_networks(
                mac_device_stru                     *pst_mac_device,
                wlan_channel_bandwidth_enum_uint8    en_band,
                oal_uint8                            uc_pri_chan_idx)
{
    oal_uint16   us_cumulative_networks = 0;
    oal_uint8    uc_num_supp_chan = mac_get_num_supp_channel(en_band);
    oal_uint8    uc_affected_ch_idx_offset = mac_get_affected_ch_idx_offset(en_band);
    oal_uint8    uc_affected_chan_lo, uc_affected_chan_hi;
    oal_uint8    uc_chan_idx;
    oal_uint32   ul_ret;

    uc_affected_chan_lo = (uc_pri_chan_idx >= uc_affected_ch_idx_offset) ?
                    (uc_pri_chan_idx - uc_affected_ch_idx_offset) : 0;

    uc_affected_chan_hi = (uc_num_supp_chan > uc_pri_chan_idx + uc_affected_ch_idx_offset) ?
                    (uc_pri_chan_idx + uc_affected_ch_idx_offset) : (uc_num_supp_chan - 1);

    for (uc_chan_idx = uc_affected_chan_lo; uc_chan_idx <= uc_affected_chan_hi; uc_chan_idx++)
    {
        ul_ret = mac_is_channel_idx_valid(en_band, uc_chan_idx);
        if (OAL_SUCC == ul_ret)
        {
            us_cumulative_networks += pst_mac_device->st_ap_channel_list[uc_pri_chan_idx].us_num_networks;
        }
    }

    return us_cumulative_networks;
}



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


