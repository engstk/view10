


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
#include "mac_frame.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "mac_regdomain.h"
#include "dmac_ext_if.h"
#include "mac_user.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FRAME_C


/*****************************************************************************
  2 函数原型声明
*****************************************************************************/


/*****************************************************************************
  3 全局变量定义
*****************************************************************************/



/*****************************************************************************
  4 函数实现
*****************************************************************************/


oal_uint8 *mac_find_ie_ext_ie(oal_uint8 uc_eid,oal_uint8 uc_ext_ie, oal_uint8 *puc_ies, oal_int32 l_len)
{
    if (OAL_PTR_NULL == puc_ies)
    {
        return OAL_PTR_NULL;
    }

    while ((l_len > MAC_IE_EXT_HDR_LEN)
            && (puc_ies[0] != uc_eid )
            && (puc_ies[2] != uc_ext_ie))
    {
        l_len   -= puc_ies[1] + MAC_IE_HDR_LEN;
        puc_ies += puc_ies[1] + MAC_IE_HDR_LEN;
    }

    if ((l_len < MAC_IE_EXT_HDR_LEN) || (l_len < (MAC_IE_HDR_LEN + puc_ies[1]))
        || ((l_len == MAC_IE_EXT_HDR_LEN) && (puc_ies[0] != uc_eid)))
    {
        return OAL_PTR_NULL;
    }

    return puc_ies;
}

#ifdef _PRE_WLAN_FEATURE_11AX

OAL_STATIC oal_void mac_set_he_mac_capinfo_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru              *pst_mac_vap        = (mac_vap_stru *)pst_vap;

    mac_frame_he_mac_cap_stru *pst_he_mac_capinfo = (mac_frame_he_mac_cap_stru *)puc_buffer;

    /*********************** HE MAC 能力信息域 ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------|
     | +HTC    | TWT         | TWT         | Fragmentation | Max Num     | Min         |
     | HE        | Requester | Responder  | support          | Fragmented | Fragment |
     | Support | Support     |   Support   |                     | MSDUs        |     Size     |
     |-----------------------------------------------------------------------|
     | B0        | B1            | B2            |   B3  B4          |  B5 B6 B7   |  B8 B9      |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | Trigger Frame | Muti-TID     | HE           | ALL        | UMRS        | BSR       |
     | MAC Padding  | Aggregation | Link         | ACK        | Support     | Support  |
     | Duration        | Support       | Adaptation| Support   |                |              |
     |-----------------------------------------------------------------------|
     |    B10   B11   | B12      B14 | B15    B16  |   B17     |    B18       | B19        |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | Broadcast | 32-bit BA | MU       | Ack-Enable             | Group Addressed   Multi-STA  |
     | TWT        | bitmap     | Cascade| Multi-TID               | BlockAck In DL MU                |
     | Support    | Support    | Support | Aggregation Support| Support                               |
     |-------------------------------------------------------------------------|
     | B20         | B21         | B22       |   B23                     |   B24                                  |
     |-------------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | OM        | OFDMA  | Max A-MPDU  | A-MSDU              | Flexible  TWT|
     | Control  | RA         | Length          | Fragmentation     |  schedule                |
     | Support  | Support  | Exponent      |  Support              | Support |         Support                     |
     |-------------------------------------------------------------------------|
     | B25       | B26       | B27  B28       |   B29                  |   B30                                 |
     |-------------------------------------------------------------------------|
    ***************************************************************************/

    pst_he_mac_capinfo->bit_htc_he_support                 = mac_mib_get_he_HTControlFieldSupported(pst_mac_vap);
    pst_he_mac_capinfo->bit_twt_requester_support          = mac_mib_get_he_TWTOptionActivated(pst_mac_vap);
    pst_he_mac_capinfo->bit_max_ampdu_length_exponent      = mac_mib_get_he_MaxAMPDULength(pst_mac_vap);
    pst_he_mac_capinfo->bit_trigger_mac_padding_duration   = mac_mib_get_he_TriggerMacPaddingDuration(pst_mac_vap);
    pst_he_mac_capinfo->bit_multi_tid_aggregation_support  = 0;

    if(OAL_TRUE == mac_mib_get_he_HTControlFieldSupported(pst_mac_vap))
    {
        pst_he_mac_capinfo->bit_om_control_support         = mac_mib_get_he_OperatingModeIndication(pst_mac_vap);
        pst_he_mac_capinfo->bit_bsr_support                = mac_mib_get_he_BSRSupport(pst_mac_vap);
        pst_he_mac_capinfo->bit_om_control_support         = mac_mib_get_he_OperatingModeIndication(pst_mac_vap);
    }

    pst_he_mac_capinfo->bit_rx_control_frame_to_multibss   = mac_mib_get_he_MultiBSSIDImplemented(pst_mac_vap);
    pst_he_mac_capinfo->bit_sr_responder                   = mac_mib_get_he_SRPBaseSR(pst_mac_vap);

}



OAL_STATIC oal_void mac_set_he_phy_capinfo_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru              *pst_mac_vap        = (mac_vap_stru *)pst_vap;

    mac_frame_he_phy_cap_stru *pst_he_phy_capinfo = (mac_frame_he_phy_cap_stru *)puc_buffer;

    pst_he_phy_capinfo->bit_dual_band_support     = mac_mib_get_he_DualBandSupport(pst_mac_vap);

    pst_he_phy_capinfo->bit_channel_width_set     = mac_mib_get_he_HEChannelWidthSet(pst_mac_vap);
    pst_he_phy_capinfo->bit_ldpc_coding_in_paylod = mac_mib_get_he_LDPCCodingInPayload(pst_mac_vap);

    pst_he_phy_capinfo->bit_su_beamformer         = mac_mib_get_he_SUBeamformer(pst_mac_vap);
    pst_he_phy_capinfo->bit_su_beamformee         = mac_mib_get_he_SUBeamformee(pst_mac_vap);
    pst_he_phy_capinfo->bit_mu_beamformer         = mac_mib_get_he_MUBeamformer(pst_mac_vap);
    pst_he_phy_capinfo->bit_srp_based_sr_support  = mac_mib_get_he_SRPBaseSR(pst_mac_vap);
}



OAL_STATIC  oal_uint8 mac_set_he_tx_rx_mcs_nss_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap                                  = (mac_vap_stru *)pst_vap;
    oal_uint8      uc_len                                      = 0;
    oal_uint8      uc_mcs_map_len                              = 0;
    //mac_frame_he_mcs_nss_stru *pst_he_mcs_nss;
    mac_frame_he_tx_rx_mcs_nss_stru *pst_he_tx_rx_mcs_nss_info = (mac_frame_he_tx_rx_mcs_nss_stru *)puc_buffer;

    pst_he_tx_rx_mcs_nss_info->bit_highest_nss_supported_m1    = mac_mib_get_he_HighestNSS(pst_mac_vap);
    pst_he_tx_rx_mcs_nss_info->bit_highest_mcs_supported       = mac_mib_get_he_HighestMCS(pst_mac_vap);

    switch(pst_he_tx_rx_mcs_nss_info->bit_highest_mcs_supported)
    {
        case MAC_MAX_SUP_MCS7_11AX_EACH_NSS:
            uc_mcs_map_len = 7;
            break;
        case MAC_MAX_SUP_MCS8_11AX_EACH_NSS:
            uc_mcs_map_len = 8;
            break;
        case MAC_MAX_SUP_MCS9_11AX_EACH_NSS:
            uc_mcs_map_len = 9;
            break;
        case MAC_MAX_SUP_MCS10_11AX_EACH_NSS:
            uc_mcs_map_len = 10;
            break;
        case MAC_MAX_SUP_MCS11_11AX_EACH_NSS:
            uc_mcs_map_len = 11;
            break;

        default:
            uc_len = 0;
            return uc_len;
    }

    uc_len = OAL_SIZEOF(mac_frame_he_tx_rx_mcs_nss_stru) +  (oal_uint8)(uc_mcs_map_len * 2);


    return uc_len;
}


OAL_STATIC oal_uint8 mac_set_he_ppe_thresholds_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    oal_uint8 uc_len = 0;

    /*TODO*/

    return uc_len;

}





 /*lint -save -e438 */
 oal_void mac_set_he_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
 {
     mac_vap_stru *pst_mac_vap        = (mac_vap_stru *)pst_vap;
     oal_uint8    *puc_ie_length;
     oal_uint8     uc_info_length;

    *puc_ie_len = 0;
     if (OAL_TRUE != mac_mib_get_HEOptionImplemented(pst_mac_vap))
     {
         return;
     }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |EID Extension|HE MAC Capa. Info |HE PHY Capa. Info|
    -------------------------------------------------------------------------
    |1   |1          |1                  |5                         |9                       |
    -------------------------------------------------------------------------
    |Tx Rx HE MCS NSS Support |PPE Thresholds(Optional)|
    -------------------------------------------------------------------------
    |2 or More                         |Variable                       |
    -------------------------------------------------------------------------
    ***************************************************************************/
    *puc_buffer    = MAC_EID_HE;
    puc_ie_length  = puc_buffer + 1;

    puc_buffer    += MAC_IE_HDR_LEN;
    *puc_ie_len   += MAC_IE_HDR_LEN;

    *puc_buffer    = MAC_EID_EXT_HE_CAP;
    puc_buffer    += 1;

    *puc_ie_len   += 1;

    /* 填充HE mac capabilities information域信息 */
    mac_set_he_mac_capinfo_field(pst_vap, puc_buffer);
    puc_buffer    += MAC_HE_MAC_CAP_LEN;
    *puc_ie_len   += MAC_HE_MAC_CAP_LEN;

    /* 填充HE PHY Capabilities Information 域信息 */
    mac_set_he_phy_capinfo_field(pst_vap, puc_buffer);
    puc_buffer    += MAC_HE_PHY_CAP_LEN;
    *puc_ie_len   += MAC_HE_PHY_CAP_LEN;

    /*填充 HE tx rx he mcs nss support*/
    uc_info_length = mac_set_he_tx_rx_mcs_nss_field(pst_vap,puc_buffer);
    puc_buffer    += uc_info_length;
    *puc_ie_len   += uc_info_length;

    /*填充 PPE Thresholds field*/
    uc_info_length = mac_set_he_ppe_thresholds_field(pst_vap,puc_buffer);
    puc_buffer    += uc_info_length;
    *puc_ie_len   += uc_info_length;

    *puc_ie_length = *puc_ie_len - MAC_IE_HDR_LEN;

}
/*lint -restore */
#endif

oal_uint32  mac_vap_set_cb_tx_user_idx(mac_vap_stru *pst_mac_vap, mac_tx_ctl_stru *pst_tx_ctl, oal_uint8 *puc_data)
{

    oal_uint16  us_user_idx = MAC_INVALID_USER_ID;
    oal_uint32  ul_ret;

    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_data, &us_user_idx);
    if (OAL_SUCC != ul_ret)
    {

        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_set_cb_tx_user_idx:: cannot find user_idx from xx:xx:xx:%x:%x:%x, set TX_USER_IDX %d.}",
        puc_data[3],
        puc_data[4],
        puc_data[5],
        MAC_INVALID_USER_ID);
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = MAC_INVALID_USER_ID;
        return ul_ret;
    }

    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_user_idx;

    return OAL_SUCC;
}
/*lint -e578*//*lint -e19*/
oal_module_symbol(mac_vap_set_cb_tx_user_idx);
/*lint -e578*//*lint -e19*/

#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE

oal_void mac_set_ie_field(oal_void *pst_data, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8                   uc_index = 0;
    oal_uint8                   *puc_ie_buff;
    mac_vap_ie_set_stru         *pst_set_ie_info;

    /* vap下对应的ie设置指针不为空则需要设置 */
    if (OAL_PTR_NULL == pst_data)
    {
        *puc_ie_len = 0;
        return;
    }
    pst_set_ie_info = (mac_vap_ie_set_stru *)pst_data;
    puc_ie_buff = pst_set_ie_info->auc_ie_content;
    /* 根据ie设置类型进行IE设置 */
    switch (pst_set_ie_info->en_set_type)
    {
        case OAL_IE_SET_TYPE_AND:    /* 与操作 */
        {
            if (0 == *puc_ie_len)       /* 长度为0 说明帧不包含该IE 无法与操作 */
            {
                return;
            }
            /* 与操作 */
            for (uc_index=0; uc_index<pst_set_ie_info->us_ie_content_len; uc_index++)
            {
                /* 从IE的content 开始进行与操作 */
                puc_buffer[MAC_IE_HDR_LEN+uc_index] = puc_buffer[MAC_IE_HDR_LEN+uc_index] & puc_ie_buff[uc_index];
            }
            *puc_ie_len = 0;                            /* 没有新增BUFF DATA 将长度置0 */
            return;
        }
        case OAL_IE_SET_TYPE_OR:
        {
            if (0 == *puc_ie_len)       /* 长度为0 说明帧不包含该IE 则添加IE到BUFF */
            {
                puc_buffer[0] = pst_set_ie_info->en_eid;
                puc_buffer[1] = (oal_uint8)pst_set_ie_info->us_ie_content_len;
                oal_memcopy(puc_buffer+MAC_IE_HDR_LEN, puc_ie_buff, pst_set_ie_info->us_ie_content_len);
                *puc_ie_len = (oal_uint8)(pst_set_ie_info->us_ie_content_len) + MAC_IE_HDR_LEN;   /* 新增IE 修改长度 */
            }
            else
            {
                /* 或操作 */
                for (uc_index=0; uc_index<pst_set_ie_info->us_ie_content_len; uc_index++)
                {
                    /* 从IE的content 开始进行或操作 */
                    puc_buffer[MAC_IE_HDR_LEN+uc_index] = puc_buffer[MAC_IE_HDR_LEN+uc_index] | puc_ie_buff[uc_index];
                }
                *puc_ie_len = 0;                            /* 没有新增BUFF DATA 将长度置0 */
            }
            return;
        }
        case OAL_IE_SET_TYPE_ADD:
        {
            if (0 == *puc_ie_len)       /* 长度为0 说明帧不包含该IE 则添加IE到BUFF */
            {
                puc_buffer[0] = pst_set_ie_info->en_eid;
                puc_buffer[1] = (oal_uint8)pst_set_ie_info->us_ie_content_len;
                oal_memcopy(puc_buffer+MAC_IE_HDR_LEN, puc_ie_buff, pst_set_ie_info->us_ie_content_len);
                *puc_ie_len = (oal_uint8)(pst_set_ie_info->us_ie_content_len) + MAC_IE_HDR_LEN;   /* 新增IE 修改长度 */
            }
            else    /* 存在该IE 则替换现有IE的内容 */
            {
                oal_memcopy(puc_buffer+MAC_IE_HDR_LEN, puc_ie_buff, pst_set_ie_info->us_ie_content_len);
                *puc_ie_len = 0;                            /* 没有新增BUFF DATA 将长度置0 */
            }
            return;
        }
        default:    /* 其他设置类型不支持 直接返回 */
            *puc_ie_len = 0;
            return;
    }
}

#endif // end of _PRE_WLAN_FEATURE_11KV_INTERFACE

#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA

 oal_void mac_set_vender_4addr_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
 {
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *) pst_vap;

    *puc_ie_len = 0;

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID | Length |HUAWEI OUI |WIFI OUT FOUR ADDR |HE PHY Capa. Info| Version |
    -------------------------------------------------------------------------
    |221 |variable|     3     |         4         |        待定     |         |
    -------------------------------------------------------------------------
    ***************************************************************************/
    /* 直接调用11KV接口的设置IE接口 宏未定义编译失败 */
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
    mac_set_ie_field(pst_mac_vap->pst_msta_ie_info, puc_buffer, puc_ie_len);
#else
#error "the virtual multi-sta feature dependent _PRE_WLAN_FEATURE_11KV_INTERFACE, please define it!"
#endif

}
#endif


oal_void mac_ftm_add_to_ext_capabilities_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
#ifdef _PRE_WLAN_FEATURE_FTM
    mac_vap_stru                *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_ext_cap_ftm_ie_stru     *pst_ext_cap_ftm;
    mac_ftm_mode_enum_uint8      en_ftm_mode = mac_check_ftm_enable(pst_mac_vap);

    if(MAC_FTM_DISABLE_MODE == en_ftm_mode)
    {
        return;
    }

    puc_buffer[1] = MAC_XCAPS_EX_FTM_LEN;

    pst_ext_cap_ftm = (mac_ext_cap_ftm_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    switch (en_ftm_mode)
    {
        case MAC_FTM_RESPONDER_MODE:
            pst_ext_cap_ftm->bit_ftm_resp = OAL_TRUE;
            pst_ext_cap_ftm->bit_ftm_int = OAL_FALSE;
            break;

        case MAC_FTM_INITIATOR_MODE:
            pst_ext_cap_ftm->bit_ftm_resp = OAL_FALSE;
            pst_ext_cap_ftm->bit_ftm_int = OAL_TRUE;
            break;

        case MAC_FTM_MIX_MODE:
            pst_ext_cap_ftm->bit_ftm_resp = OAL_TRUE;
            pst_ext_cap_ftm->bit_ftm_int = OAL_TRUE;
            break;

        default:
            break;
    }

    (*puc_ie_len)++;
#endif
}

#ifdef _PRE_WLAN_FEATURE_1024QAM

oal_void mac_set_1024qam_vendor_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru                        *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_ieee80211_vendor_ie_stru          *pst_vendor_ie;

    if (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_1024qam)
    {
        *puc_ie_len = 0;
        return;
    }

    pst_vendor_ie = (mac_ieee80211_vendor_ie_stru *)puc_buffer;
    pst_vendor_ie->uc_element_id = MAC_EID_VENDOR;
    pst_vendor_ie->uc_len = sizeof(mac_ieee80211_vendor_ie_stru) - MAC_IE_HDR_LEN;

    pst_vendor_ie->uc_oui_type = MAC_HISI_1024QAM_IE;

    pst_vendor_ie->auc_oui[0] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 16) & 0xff);
    pst_vendor_ie->auc_oui[1] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 8) & 0xff);
    pst_vendor_ie->auc_oui[2] = (oal_uint8)((MAC_HUAWEI_VENDER_IE) & 0xff);

    *puc_ie_len = OAL_SIZEOF(mac_ieee80211_vendor_ie_stru);

}
#endif

oal_void mac_set_ext_capabilities_ie_rom_cb(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
#ifdef _PRE_WLAN_FEATURE_11V
    mac_ext_cap_ie_stru     *pst_ext_cap;

    pst_ext_cap = (mac_ext_cap_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);

     /* 首先需先使能wirelessmanagerment标志 */
     /* 然后如果是站点本地能力位和扩展控制变量均支持BSS TRANSITION 设置扩展能力bit位 */
    if ( (OAL_TRUE == mac_mib_get_WirelessManagementImplemented(pst_mac_vap)) &&
         (OAL_TRUE == mac_mib_get_MgmtOptionBSSTransitionImplemented(pst_mac_vap)) &&
         (OAL_TRUE == mac_mib_get_MgmtOptionBSSTransitionActivated(pst_mac_vap)))
    {
        pst_ext_cap->bit_bss_transition = 1;
    }
#endif
    mac_ftm_add_to_ext_capabilities_ie(pst_mac_vap, puc_buffer, puc_ie_len);
}


oal_void mac_set_vht_capinfo_field_cb(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
#ifdef _PRE_WLAN_FEATURE_M2S
    mac_vap_stru           *pst_mac_vap     = (mac_vap_stru *)pst_vap;
    mac_vht_cap_info_stru  *pst_vht_capinfo = (mac_vht_cap_info_stru *)puc_buffer;
    mac_user_stru          *pst_mac_user;

    pst_vht_capinfo->bit_num_bf_ant_supported    = mac_mib_get_VHTBeamformeeNTxSupport(pst_mac_vap) - 1;

    /* 参考标杆,该字段根据对端空间流能力和自己的能力取交集*/
    pst_mac_user = mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id);
    if(WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode && OAL_PTR_NULL != pst_mac_user)
    {
        pst_vht_capinfo->bit_num_bf_ant_supported = OAL_MIN(pst_vht_capinfo->bit_num_bf_ant_supported,
                                                               pst_mac_user->st_vht_hdl.bit_num_sounding_dim);
    }
#endif
}

#ifdef _PRE_WLAN_NARROW_BAND

oal_void  mac_get_nb_ie( oal_void *pst_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len)
{
    oal_uint8       *puc_nb;
    oal_uint8        uc_nb_len;
    oal_uint8        uc_loop;
    oal_uint8       *puc_bw;

    mac_vap_stru    *pst_mac_vap = (mac_vap_stru *)pst_vap;

    puc_nb = mac_find_vendor_ie(MAC_EID_VENDOR, MAC_HISI_NB_IE, puc_payload, us_frame_len);
    if (OAL_PTR_NULL == puc_nb)
    {
        return ;
    }

    uc_nb_len = puc_nb[1];

    if (uc_nb_len <=  MAC_OUI_LEN)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "mac_get_nb_ie::nb_len[%u] err!", uc_nb_len);
        return;
    }

    puc_bw = puc_nb + MAC_IE_HDR_LEN + MAC_OUI_LEN + MAC_OUITYPE_LEN;
    for (uc_loop = 0; uc_loop < (uc_nb_len - (MAC_OUI_LEN + MAC_OUITYPE_LEN)); uc_loop++)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "mac_get_nb_ie::bw[%x]!", puc_bw[uc_loop]);
    }

}
#endif

/*lint -e19*/
#ifdef _PRE_WLAN_FEATURE_VIRTUAL_MULTI_STA
oal_module_symbol(mac_set_vender_4addr_ie);
#endif
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

