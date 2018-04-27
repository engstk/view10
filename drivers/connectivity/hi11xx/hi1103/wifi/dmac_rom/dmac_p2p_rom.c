


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_P2P


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_util.h"
#include "mac_resource.h"
#include "mac_frame.h"
#include "dmac_vap.h"
#include "dmac_p2p.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_config.h"
#include "hal_ext_if.h"
#include "dmac_mgmt_ap.h"
#ifdef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
#include "dmac_psm_ap.h"
#endif
//#include "mac_pm.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "pm_extern.h"
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_sta_pm.h"
#include "dmac_psm_sta.h"
#include "hal_device_fsm.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_P2P_ROM_C

/*****************************************************************************
  2 静态函数声明
*****************************************************************************/


/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
dmac_p2p_cb g_st_dmac_p2p_rom_cb = {OAL_PTR_NULL,
                                    OAL_PTR_NULL};


/*****************************************************************************
  4 函数实现
*****************************************************************************/


/* 检查是否只包含11B 速率，11b速率集:0x82 = 1Mbps, 0x84 = 2Mbps, 0x8B = 5.5Mbps, 0x96 = 11Mbps */

oal_bool_enum_uint8 dmac_is_11b_rate(oal_uint8 uc_rate)
{
    uc_rate &= (oal_uint8)(~BIT7);
    if ((2 == uc_rate)
        || (4 == uc_rate)
        || (11 == uc_rate)
        || (22 == uc_rate))
    {
        return OAL_TRUE;
    }
    else
    {
        return OAL_FALSE;
    }
}


oal_bool_enum_uint8 mac_is_p2p_action_frame(oal_uint8 *puc_data)
{
    oal_bool_enum_uint8       ul_ret;

    /* 获取帧体指针 */

    /* Category */
    switch (puc_data[MAC_ACTION_OFFSET_CATEGORY])
    {

        case MAC_ACTION_CATEGORY_PUBLIC:
        {
            /* Action */
            switch (puc_data[MAC_ACTION_OFFSET_ACTION])
            {
                case MAC_PUB_VENDOR_SPECIFIC:
                {
                    ul_ret =  OAL_TRUE;
                }
                break;
                default:
                {
                    ul_ret =  OAL_FALSE;
                }
                break;
            }
        }
        break;

        case MAC_ACTION_CATEGORY_VENDOR:
        {
            ul_ret =  OAL_TRUE;
        }
        break;

        default:
        {
            ul_ret =  OAL_FALSE;
        }
        break;
    }
    return ul_ret;

}


oal_bool_enum_uint8 dmac_is_p2p_pd_disc_req_frame(OAL_CONST oal_uint8 *puc_data)
{
    if((MAC_ACTION_CATEGORY_PUBLIC == puc_data[MAC_ACTION_OFFSET_CATEGORY])&&
        (WFA_OUI_BYTE1== puc_data[P2P_PUB_ACT_OUI_OFF1])&&
        (WFA_OUI_BYTE2== puc_data[P2P_PUB_ACT_OUI_OFF2])&&
        (WFA_OUI_BYTE3== puc_data[P2P_PUB_ACT_OUI_OFF3])&&
        (WFA_P2P_v1_0== puc_data[P2P_PUB_ACT_OUI_TYPE_OFF])&&
        (P2P_PAF_PD_REQ== puc_data[P2P_PUB_ACT_OUI_SUBTYPE_OFF]))
        return OAL_TRUE;
    else
        return OAL_FALSE;

}


oal_bool_enum_uint8 dmac_p2p_is_only_11b_rates(oal_uint8 *puc_frame_body, oal_uint16 us_frame_len)
{
    oal_uint8      *puc_ie;
    oal_uint8       uc_ie_len;
    oal_uint8       uc_rate;
    oal_uint32      ul_loop;
    oal_uint32      ul_ie_index;
    oal_uint8      *puc_ie_array[2];

    /* 检查是否只包含11B 速率集，11b速率集:0x82 = 1Mbps, 0x84 = 2Mbps, 0x8B = 5.5Mbps, 0x96 = 11Mbps */

    puc_ie_array[0] = mac_find_ie(MAC_EID_RATES, puc_frame_body, us_frame_len);
    puc_ie_array[1] = mac_find_ie(MAC_EID_XRATES, puc_frame_body, us_frame_len);
    for (ul_ie_index = 0; ul_ie_index < OAL_ARRAY_SIZE(puc_ie_array); ul_ie_index++)
    {
        if (OAL_PTR_NULL == puc_ie_array[ul_ie_index])
        {
            continue;
        }

        puc_ie = puc_ie_array[ul_ie_index];
        uc_ie_len = *(puc_ie + 1);

        for (ul_loop = 0; ul_loop < uc_ie_len; ul_loop++)
        {
            uc_rate = *(puc_ie + MAC_IE_HDR_LEN + ul_loop);
            if (OAL_FALSE == dmac_is_11b_rate(uc_rate))
            {
                return OAL_FALSE;
            }
        }
    }
    return OAL_TRUE;
}


oal_bool_enum_uint8 dmac_is_p2p_presence_req_frame(oal_uint8 *puc_data)
{
    if((MAC_ACTION_CATEGORY_VENDOR == puc_data[MAC_ACTION_OFFSET_CATEGORY])&&
        (P2P_PRESENCE_REQ == puc_data[P2P_GEN_ACT_OUI_SUBTYPE_OFF]))
        return OAL_TRUE;
    else
        return OAL_FALSE;
}


oal_uint32  dmac_p2p_listen_filter_frame(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_frame_body, oal_uint16 us_frame_len)
{
    oal_uint8                   *puc_p2p_ie     = OAL_PTR_NULL;

    /* 如果接收到的probe req 帧只包含11b 速率， 则返回 */
    if (OAL_TRUE == dmac_p2p_is_only_11b_rates(puc_frame_body, us_frame_len))
    {
        return OAL_FAIL;
    }

    /* 如果接收到的probe req 帧不包含SSID "DIRECT-" ， 则返回 */
    if (!IS_P2P_WILDCARD_SSID(&puc_frame_body[MAC_IE_HDR_LEN], puc_frame_body[1]))
    {
        return OAL_FAIL;
    }

    /* 如果接收到的probe req 帧不包含P2P_IE， 则返回 */
    //puc_p2p_ie = mac_get_p2p_ie(puc_frame_body, us_frame_len, 0);
    puc_p2p_ie = mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, puc_frame_body, us_frame_len);
    if (puc_p2p_ie == OAL_PTR_NULL)
    {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


oal_uint32  dmac_p2p_listen_filter_vap(dmac_vap_stru *pst_dmac_vap)
{

    /* 只有P2P CL和P2P DEV处于Listen状态且当前VAP信道与P2P0 Listen Channel一致的情况下才可以回复Probe resp帧 */
    if ((IS_P2P_CL((&pst_dmac_vap->st_vap_base_info)) || IS_P2P_DEV((&pst_dmac_vap->st_vap_base_info)))
         && dmac_vap_is_in_p2p_listen(&(pst_dmac_vap->st_vap_base_info))
         && (pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number == pst_dmac_vap->st_vap_base_info.uc_p2p_listen_channel))
    {
        return OAL_SUCC;
    }

    return OAL_FAIL;
}


oal_bool_enum_uint8 dmac_is_p2p_go_neg_req_frame(OAL_CONST oal_uint8* puc_data)
{
    if((MAC_ACTION_CATEGORY_PUBLIC == puc_data[MAC_ACTION_OFFSET_CATEGORY])&&
        (WFA_OUI_BYTE1== puc_data[P2P_PUB_ACT_OUI_OFF1])&&
        (WFA_OUI_BYTE2== puc_data[P2P_PUB_ACT_OUI_OFF2])&&
        (WFA_OUI_BYTE3== puc_data[P2P_PUB_ACT_OUI_OFF3])&&
        (WFA_P2P_v1_0== puc_data[P2P_PUB_ACT_OUI_TYPE_OFF])&&
        (P2P_PAF_GON_REQ== puc_data[P2P_PUB_ACT_OUI_SUBTYPE_OFF]))
        return OAL_TRUE;
    else
        return OAL_FALSE;
}

oal_void  mac_set_p2p0_ssid_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len, oal_uint16 us_frm_type)
{
    oal_uint8     uc_ssid_len;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    if (IS_LEGACY_VAP(pst_mac_vap))
    {
        return;
    }

    /***************************************************************************
                    ----------------------------
                    |Element ID | Length | SSID|
                    ----------------------------
           Octets:  |1          | 1      | 0~32|
                    ----------------------------
    ***************************************************************************/
    /***************************************************************************
      A SSID  field  of length 0 is  used  within Probe
      Request management frames to indicate the wildcard SSID.
    ***************************************************************************/
    /* 只有beacon会隐藏ssid */
    if((pst_mac_vap->st_cap_flag.bit_hide_ssid) && (WLAN_FC0_SUBTYPE_BEACON == us_frm_type))
    {
        /* ssid ie */
        *puc_buffer = MAC_EID_SSID;
        /* ssid len */
        *(puc_buffer + 1) = 0;
        *puc_ie_len = MAC_IE_HDR_LEN;
        return;
    }

    *puc_buffer = MAC_EID_SSID;
    uc_ssid_len = (oal_uint8)OAL_STRLEN((oal_int8 *)DMAC_P2P_WILDCARD_SSID );   /* 不包含'\0'*/

    *(puc_buffer + 1) = uc_ssid_len;

    oal_memcopy(puc_buffer + MAC_IE_HDR_LEN, DMAC_P2P_WILDCARD_SSID , uc_ssid_len);

    *puc_ie_len = uc_ssid_len + MAC_IE_HDR_LEN;

}

//oal_module_symbol();


oal_void mac_set_p2p_noa(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8       uc_index;
    mac_vap_stru    *pst_mac_vap = (mac_vap_stru *)pst_vap;
    dmac_vap_stru   *pst_dmac_vap;
    oal_uint8       CTWindow = 0;
    oal_uint32      ul_start_time;
    oal_uint32      ul_duration;
    oal_uint32      ul_interval;
    oal_uint16      us_attr_len;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{mac_set_p2p_noa::pst_dmac_vap null.}");
        return;
    }

    /* p2p noa Element                                                       */
    /* --------------------------------------------------------------------- */
    /* |1          |1          |3     | 1        | 1            | 2        | */
    /* --------------------------------------------------------------------- */
    /* |Tag number |Tag length |OUI   | OUI Type | Attribute ID | Length   | */
    /* --------------------------------------------------------------------- */
    /* |  1    |       1        | 13                                       | */
    /* --------------------------------------------------------------------- */
    /* | Index | CTWindow/OppPS |Notice of Absence Descriptor              | */
    /* --------------------------------------------------------------------- */

    /* Tag number */
    puc_buffer[0] = MAC_EID_P2P;

    /* Tag length */
    puc_buffer[1] = P2P_OUI_LEN+ P2P_ATTR_HDR_LEN + (P2P_NOA_DESC_NUM*13) + 2;
    uc_index = MAC_IE_HDR_LEN;

    /* OUI */
    oal_memcopy(&puc_buffer[uc_index], g_auc_p2p_oui, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_buffer[uc_index++] = MAC_OUITYPE_P2P;

    /* Notice of Absence attribute*/
    /* Attribute ID*/
    puc_buffer[uc_index++] = NOTICE_OF_ABSENCE;

    /* Length*/
    us_attr_len = (P2P_NOA_DESC_NUM*13) + 2;
    puc_buffer[uc_index++] = us_attr_len & 0x00FF;
    puc_buffer[uc_index++] = us_attr_len >> 8;

    /* Index*/
    puc_buffer[uc_index++] = 0;

    /* CTWindow and Opportunity parameters field */
    if(IS_P2P_OPPPS_ENABLED(pst_dmac_vap))
    {
        CTWindow = pst_dmac_vap->st_p2p_ops_param.uc_ct_window;
        CTWindow |= BIT7;
    }
    puc_buffer[uc_index++] = CTWindow;

    if(!IS_P2P_NOA_ENABLED(pst_dmac_vap))
    {
        *puc_ie_len = MAC_IE_HDR_LEN + P2P_OUI_LEN+ P2P_ATTR_HDR_LEN + 2;
        return;
    }
    /* Notice of Absence Descriptor*/
    puc_buffer[uc_index++] = pst_dmac_vap->st_p2p_noa_param.uc_count;

    ul_duration = pst_dmac_vap->st_p2p_noa_param.ul_duration;
    puc_buffer[uc_index++] = ul_duration & 0x000000FF;
    puc_buffer[uc_index++] = (ul_duration >> 8) & 0x000000FF;
    puc_buffer[uc_index++] = (ul_duration >> 16) & 0x000000FF;
    puc_buffer[uc_index++] = (ul_duration >> 24);

    ul_interval = pst_dmac_vap->st_p2p_noa_param.ul_interval;
    puc_buffer[uc_index++] = ul_interval & 0x000000FF;
    puc_buffer[uc_index++] = (ul_interval >> 8) & 0x000000FF;
    puc_buffer[uc_index++] = (ul_interval >> 16) & 0x000000FF;
    puc_buffer[uc_index++] = (ul_interval >> 24);

    ul_start_time = pst_dmac_vap->st_p2p_noa_param.ul_start_time;
    puc_buffer[uc_index++] = ul_start_time & 0x000000FF;
    puc_buffer[uc_index++] = (ul_start_time >> 8) & 0x000000FF;
    puc_buffer[uc_index++] = (ul_start_time >> 16) & 0x000000FF;
    puc_buffer[uc_index++] = (ul_start_time >> 24);

    *puc_ie_len = MAC_IE_HDR_LEN + P2P_OUI_LEN+ P2P_ATTR_HDR_LEN + (P2P_NOA_DESC_NUM*13) + 2;

    if (OAL_PTR_NULL != g_st_dmac_p2p_rom_cb.set_p2p_noa_cb)
    {
        g_st_dmac_p2p_rom_cb.set_p2p_noa_cb(pst_dmac_vap, puc_buffer, puc_ie_len);
    }


}


oal_void mac_set_p2p_none_noa(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8       uc_index;

    /* p2p noa Element                                                       */
    /* --------------------------------------------------------------------- */
    /* |1          |1          |3     | 1        | 1            | 2        | */
    /* --------------------------------------------------------------------- */
    /* |Tag number |Tag length |OUI   | OUI Type | Attribute ID | Length   | */
    /* --------------------------------------------------------------------- */
    /* |  1    |       1        | 13                                       | */
    /* --------------------------------------------------------------------- */
    /* | Index | CTWindow/OppPS |Notice of Absence Descriptor              | */
    /* --------------------------------------------------------------------- */

    /* Tag number */
    puc_buffer[0] = MAC_EID_P2P;

    /* Tag length */
    puc_buffer[1] = P2P_OUI_LEN+ P2P_ATTR_HDR_LEN + 2;
    uc_index = MAC_IE_HDR_LEN;

    /* OUI */
    oal_memcopy(&puc_buffer[uc_index], g_auc_p2p_oui, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_buffer[uc_index++] = MAC_OUITYPE_P2P;

    /* Notice of Absence attribute*/
    /* Attribute ID*/
    puc_buffer[uc_index++] = NOTICE_OF_ABSENCE;

    /* Length*/
    puc_buffer[uc_index++] = 2;
    puc_buffer[uc_index++] = 0;

    /* Index*/
    puc_buffer[uc_index++] = 0;
    /* Set CTWindow and Opportunity parameters field to 0*/
    puc_buffer[uc_index++] = 0;

    *puc_ie_len = MAC_IE_HDR_LEN + P2P_OUI_LEN+ P2P_ATTR_HDR_LEN + 2;
}



oal_void mac_set_p2p_status(oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len, P2P_STATUS_CODE_T status)
{
    oal_uint8       uc_index = 0;
    oal_uint16      us_attr_len;

    /* -------------------------------------------*/
    /* p2p Status Attribute                       */
    /* -------------------------------------------*/
    /* | 1            | 2        | 1             |*/
    /* -------------------------------------------*/
    /* |Attribute ID  | Length   | Status Code   |*/
    /* -------------------------------------------*/

    /* Tag number */
    puc_buffer[0] = MAC_EID_P2P;

    /* Tag length */
    puc_buffer[1] = P2P_OUI_LEN+ P2P_ATTR_HDR_LEN + 1;
    uc_index = MAC_IE_HDR_LEN;

    /* OUI */
    oal_memcopy(&puc_buffer[uc_index], g_auc_p2p_oui, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_buffer[uc_index++] = MAC_OUITYPE_P2P;

    /* Attribute ID*/
    puc_buffer[uc_index++] = P2P_STATUS;

    /* Length*/
    us_attr_len = 1;
    puc_buffer[uc_index++] = us_attr_len & 0x00FF;
    puc_buffer[uc_index++] = us_attr_len >> 8;

    puc_buffer[uc_index++] = (oal_uint8)status;

    *puc_ie_len = uc_index;
}

oal_bool_enum_uint8 dmac_is_p2p_ie(oal_uint8 *puc_data)
{
    if((MAC_EID_P2P   == puc_data[0]) &&
       (WFA_OUI_BYTE1 == puc_data[2]) &&
       (WFA_OUI_BYTE2 == puc_data[3]) &&
       (WFA_OUI_BYTE3 == puc_data[4]) &&
       (WFA_P2P_v1_0  == puc_data[5]))
        return OAL_TRUE;
    else
        return OAL_FALSE;
}


oal_uint16  dmac_mgmt_encap_p2p_presence_rsp(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf, oal_uint8 *puc_ra, oal_uint8 *puc_data)
{
    oal_uint8        uc_ie_len;
    mac_vap_stru    *pst_mac_vap    = &(pst_dmac_vap->st_vap_base_info);
    oal_uint8       *puc_mac_header = oal_netbuf_header(pst_netbuf);
    oal_uint8       *puc_payload_addr        = mac_netbuf_get_payload(pst_netbuf);
    oal_uint8       *puc_payload_addr_origin = puc_payload_addr;
    oal_uint8        uc_index = 0;

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
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置地址1为发送presence request帧的STA */
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
    /*                  presence response Frame - Frame Body                 */
    /* ----------------------------------------------------------------------*/
    /* |Category |OUI   |OUI type|OUI subtype  | Dialog Token   |Element    |*/
    /* ----------------------------------------------------------------------*/
    /* |1        |3     |1       |1            |1               |variable   |*/
    /* ----------------------------------------------------------------------*/
    /*************************************************************************/

    puc_payload_addr[0] = MAC_ACTION_CATEGORY_VENDOR;
    uc_index = P2P_GEN_ACT_OUI_OFF1;

    /* OUI */
    oal_memcopy(&puc_payload_addr[uc_index], g_auc_p2p_oui, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_payload_addr[uc_index++] = MAC_OUITYPE_P2P;

    /* OUI Subtype */
    puc_payload_addr[uc_index++] = P2P_PRESENCE_RESP;

    /* Dialog Token */
    puc_payload_addr[uc_index++] = puc_data[P2P_GEN_ACT_DIALOG_TOKEN_OFF];
    puc_payload_addr += P2P_GEN_ACT_TAG_PARAM_OFF;

    /* Element */
    /* 填充p2p Status Attribute*/
    mac_set_p2p_status(puc_payload_addr, &uc_ie_len, P2P_STAT_SUCCESS);
    puc_payload_addr += uc_ie_len;

    /* 填充p2p noa Attribute*/
    if(IS_P2P_PS_ENABLED(pst_dmac_vap))
    {
        mac_set_p2p_noa(pst_mac_vap, puc_payload_addr, &uc_ie_len);
        puc_payload_addr += uc_ie_len;
    }
    else
    {
        mac_set_p2p_none_noa(pst_mac_vap, puc_payload_addr, &uc_ie_len);
        puc_payload_addr += uc_ie_len;
    }

    if (OAL_PTR_NULL != g_st_dmac_p2p_rom_cb.encap_p2p_presence_rsp_cb)
    {
        g_st_dmac_p2p_rom_cb.encap_p2p_presence_rsp_cb(pst_dmac_vap, pst_netbuf, puc_payload_addr);
    }


    return (oal_uint16)((puc_payload_addr - puc_payload_addr_origin) + MAC_80211_FRAME_LEN);
}


oal_uint8  *dmac_get_p2p_noa_attr(oal_uint8 *puc_frame_body, oal_uint16 us_rx_len, oal_uint16 uc_tag_param_offset, oal_uint16 *pus_attr_len)
{
    oal_uint16  us_index         = 0;
    oal_uint16  us_num_bytes     = 0;
	oal_uint32  ulIndex          = 0;
	oal_uint8  *pucData         = NULL;
    //oal_uint8  *puc_noa_attr    = NULL;
    //oal_uint16  us_loops;
    us_index = uc_tag_param_offset;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* --------------------------------------------------------------------- */
    /* |Timestamp |BeaconInt |CapInfo |SSID |SupRates |DSParSet |TAG       | */
    /* --------------------------------------------------------------------- */
    /* |8         |2         |2       |2-34 |3-10     |3        |???       | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
#if 0
        OAL_IO_PRINT("dmac_get_p2p_noa_attr:: us_rx_len=%d\r\n",us_rx_len);
        for (us_loops = 0; us_loops < us_rx_len; us_loops++)
        {
            OAL_IO_PRINT("%02x ", puc_frame_body[us_loops]);
            if ((us_loops+1)%16 == 0)
                OAL_IO_PRINT("\r\n");
        }
        OAL_IO_PRINT("\r\n");
#endif

    while (us_index < (us_rx_len - WLAN_HDR_FCS_LENGTH))
    {
        if(OAL_TRUE == dmac_is_p2p_ie(puc_frame_body + us_index)) /* 找到P2P IE */
        {
            ulIndex = 0;
            /* get the P2P IE len */
			us_num_bytes = puc_frame_body[us_index + 1] - P2P_OUI_LEN;
            pucData    = puc_frame_body + us_index + MAC_IE_HDR_LEN + P2P_OUI_LEN;

            while(ulIndex < us_num_bytes)
            {
                if(NOTICE_OF_ABSENCE == pucData[ulIndex])/* 找到NoA属性 */
                {
                    *pus_attr_len = GET_ATTR_LEN(pucData + ulIndex);
                    return (oal_uint8*)(puc_frame_body + us_index + ulIndex + MAC_IE_HDR_LEN + P2P_OUI_LEN + P2P_ATTR_HDR_LEN + 1);
                }
                ulIndex += GET_ATTR_LEN(pucData + ulIndex) + P2P_ATTR_HDR_LEN;
            }
        }
        /* Move to the next IE. */
        us_index += (MAC_IE_HDR_LEN + puc_frame_body[us_index + 1]);
    }
    return OAL_PTR_NULL;
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif



