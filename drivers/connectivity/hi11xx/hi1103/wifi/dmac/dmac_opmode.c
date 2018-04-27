


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_opmode.h"
#include "dmac_vap.h"
#include "oal_types.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_config.h"
#include "dmac_psm_ap.h"
#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_OPMODE_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint32 dmac_ie_proc_opmode_notify(mac_user_stru *pst_mac_user, mac_vap_stru *pst_mac_vap, mac_opmode_notify_stru *pst_opmode_notify)
{
    oal_uint32              ul_relt;
    wlan_bw_cap_enum_uint8  en_bwcap_user = 0;                      /* user之前的带宽信息 */
    wlan_nss_enum_uint8     en_avail_bf_num_spatial_stream;         /* 用户支持的Beamforming空间流个数 */
    wlan_nss_enum_uint8     en_avail_num_spatial_stream;            /* Tx和Rx支持Nss的交集,供算法调用 */

    en_avail_bf_num_spatial_stream = pst_mac_user->en_avail_bf_num_spatial_stream;
    en_avail_num_spatial_stream = pst_mac_user->en_avail_num_spatial_stream;

    en_bwcap_user = pst_mac_user->en_avail_bandwidth;

    ul_relt = mac_ie_proc_opmode_field(pst_mac_vap, pst_mac_user, pst_opmode_notify);
    if (OAL_UNLIKELY(OAL_SUCC != ul_relt))
    {
        OAM_WARNING_LOG1(pst_mac_user->uc_vap_id, OAM_SF_OPMODE, "{dmac_ie_proc_opmode_notify::mac_ie_proc_opmode_field failed[%d].}", ul_relt);
        return ul_relt;
    }

    /*若空间流能力发送变化，则调用算法钩子函数,如果带宽和空间流同时改变，要先调用空间流的算法函数*/
    if ((pst_mac_user->en_avail_bf_num_spatial_stream != en_avail_bf_num_spatial_stream) ||
          (pst_mac_user->en_avail_num_spatial_stream != en_avail_num_spatial_stream))
    {
        dmac_alg_cfg_user_spatial_stream_notify(pst_mac_user);
    }

    /* opmode带宽改变通知算法,并同步带宽信息到HOST */
    if (pst_mac_user->en_avail_bandwidth != en_bwcap_user)
    {
        /* 调用算法改变带宽通知链 */
        dmac_alg_cfg_user_bandwidth_notify(pst_mac_vap, pst_mac_user);
    }

    return OAL_SUCC;
}


oal_uint32 dmac_check_opmode_notify(
                mac_vap_stru                   *pst_mac_vap,
                oal_uint8                       *puc_payload,
                oal_uint32                       ul_msg_len,
                mac_user_stru                   *pst_mac_user)
{
    mac_opmode_notify_stru *pst_opmode_notify = OAL_PTR_NULL;
    oal_uint8              *puc_opmode_notify_ie;
    wlan_bw_cap_enum_uint8  en_bwcap_user = 0;                      /* user之前的带宽信息 */
    oal_uint32              ul_change = MAC_NO_CHANGE;
    oal_uint32              ul_ret = 0;

    en_bwcap_user = pst_mac_user->en_avail_bandwidth;

    if ((OAL_FALSE == mac_mib_get_VHTOptionImplemented(pst_mac_vap))
        || (OAL_FALSE == mac_mib_get_OperatingModeNotificationImplemented(pst_mac_vap)))
    {
        return ul_change;
    }

    puc_opmode_notify_ie = mac_find_ie(MAC_EID_OPMODE_NOTIFY, puc_payload, (oal_int32)(ul_msg_len));
    if (OAL_PTR_NULL == puc_opmode_notify_ie)
    {
        return ul_change;
    }

    if (puc_opmode_notify_ie[1] < MAC_OPMODE_NOTIFY_LEN)
    {
        OAM_WARNING_LOG1(0, OAM_SF_OPMODE, "{dmac_check_opmode_notify::invalid opmode notify ie len[%d]!}", puc_opmode_notify_ie[1]);
        return ul_change;
    }

    pst_opmode_notify = (mac_opmode_notify_stru *)(puc_opmode_notify_ie + MAC_IE_HDR_LEN);

    /* SMPS已经解析并更新空间流，因此若空间流不等则SMPS和OPMODE的空间流信息不同 */
    if(pst_mac_user->en_avail_num_spatial_stream > pst_opmode_notify->bit_rx_nss ||
        (pst_mac_user->en_avail_num_spatial_stream == WLAN_SINGLE_NSS && pst_opmode_notify->bit_rx_nss != WLAN_SINGLE_NSS))
    {
        OAM_WARNING_LOG0(0, OAM_SF_OPMODE, "{dmac_check_opmode_notify::SMPS and OPMODE show different nss!}");
        if(WLAN_HT_MODE == pst_mac_user->en_cur_protocol_mode || WLAN_HT_ONLY_MODE == pst_mac_user->en_cur_protocol_mode)
        {
            return ul_change;
        }
    }

    ul_ret = mac_ie_proc_opmode_field(pst_mac_vap, pst_mac_user, pst_opmode_notify);
    if(OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_user->uc_vap_id,OAM_SF_OPMODE, "{dmac_check_opmode_notify:: mac_ie_proc_opmode_field failed [%d].}", ul_ret);
    }
    if(pst_mac_user->en_avail_bandwidth != en_bwcap_user)
    {
        ul_change = MAC_BW_OPMODE_CHANGE;
    }
    return ul_change;

}


oal_uint32  dmac_mgmt_send_opmode_notify_action(mac_vap_stru *pst_mac_vap, wlan_nss_enum_uint8 en_nss, oal_bool_enum_uint8 en_bool)
{
    oal_netbuf_stru        *pst_mgmt_buf;
    oal_uint16              us_mgmt_len;
    mac_tx_ctl_stru        *pst_tx_ctl;
    oal_uint32              ul_ret;
    dmac_vap_stru          *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_OPMODE, "{dmac_mgmt_send_opmode_notify_action::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请管理帧内存 */
    pst_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_mgmt_buf)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{dmac_mgmt_send_opmode_notify_action::pst_mgmt_buf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(pst_mgmt_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_mgmt_buf,OAL_PTR_NULL);
    OAL_MEM_NETBUF_TRACE(pst_mgmt_buf, OAL_TRUE);

    /* 封装 operating mode notify 帧 */
    dmac_mgmt_encap_opmode_notify_action(pst_mac_vap, pst_mgmt_buf, &us_mgmt_len, en_bool, en_nss);
    if (0 == us_mgmt_len)
    {
        oal_netbuf_free(pst_mgmt_buf);
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{dmac_mgmt_send_opmode_notify_action::encap opmode notify action failed.}");
        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_mgmt_buf);

    OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl)      = pst_mac_vap->us_assoc_vap_id;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl)      = WLAN_WME_AC_MGMT;
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl)       = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl)    = WLAN_ACTION_OPMODE_FRAME_SUBTYPE;

    /* 调用发送管理帧接口 */
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, us_mgmt_len);
    if (OAL_SUCC != ul_ret)
    {
        oal_netbuf_free(pst_mgmt_buf);
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{dmac_mgmt_send_opmode_notify_action::tx opmode notify action failed.}");
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32  dmac_mgmt_rx_opmode_notify_frame(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_netbuf)
{
    mac_opmode_notify_stru     *pst_opmode_notify = OAL_PTR_NULL;
    mac_vap_stru               *pst_mac_vap;
    mac_user_stru              *pst_mac_user;
    dmac_rx_ctl_stru           *pst_rx_ctl;
    mac_ieee80211_frame_stru   *pst_frame_hdr;
    oal_uint8                  *puc_data;
    oal_uint8                   uc_power_save;
#ifdef _PRE_WLAN_FEATURE_M2S
    wlan_bw_cap_enum_uint8      en_bwcap_user = WLAN_BW_CAP_20M;        /* user之前的带宽信息 */
    wlan_nss_enum_uint8         en_avail_bf_num_spatial_stream;         /* 用户支持的Beamforming空间流个数 */
    wlan_nss_enum_uint8         en_avail_num_spatial_stream;            /* Tx和Rx支持Nss的交集,供算法调用 */
    oal_bool_enum_uint8         en_nss_change = OAL_FALSE;
    oal_bool_enum_uint8         en_bw_change  = OAL_FALSE;
    oal_uint32                  ul_relt;
#endif

    if ((OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_dmac_user) || (OAL_PTR_NULL == pst_netbuf))
    {
        OAM_ERROR_LOG3(0, OAM_SF_OPMODE, "{dmac_mgmt_rx_opmode_notify_frame::pst_dmac_vap = [%p], pst_dmac_user = [%p],pst_netbuf = [%p]!}\r\n",
                        pst_dmac_vap, pst_dmac_user, pst_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap  = &(pst_dmac_vap->st_vap_base_info);
    pst_mac_user = &(pst_dmac_user->st_user_base_info);

    /* vap是单空间流时不调整空间流 */
    if(IS_VAP_SINGLE_NSS(pst_mac_vap) || IS_USER_SINGLE_NSS(pst_mac_user))
    {
        return OAL_SUCC;
    }

    if ((OAL_FALSE == mac_mib_get_VHTOptionImplemented(pst_mac_vap))
        || (OAL_FALSE == mac_mib_get_OperatingModeNotificationImplemented(pst_mac_vap)))
    {
        OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{dmac_mgmt_rx_opmode_notify_frame::the vap is not support OperatingModeNotification!}");
        return OAL_SUCC;
    }

    pst_rx_ctl    = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctl->st_rx_info));
    /* 获取帧体指针 */
    puc_data = MAC_GET_RX_PAYLOAD_ADDR(&(pst_rx_ctl->st_rx_info), pst_netbuf);

    /* 是否需要处理Power Management bit位 */
    uc_power_save = (oal_uint8)pst_frame_hdr->st_frame_control.bit_power_mgmt;

    /* 如果节能位开启(bit_power_mgmt == 1),抛事件到DMAC，处理用户节能信息 */
    if ((OAL_TRUE == uc_power_save) && (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode))
    {
        if (OAL_FALSE == pst_dmac_user->bit_ps_mode)
        {
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_OPMODE, "{dmac_mgmt_rx_opmode_notify_frame::user changes ps mode to powersave!}\r\n");
            dmac_psm_doze(pst_dmac_vap, pst_dmac_user);
        }
    }

    /****************************************************/
    /*   OperatingModeNotification Frame - Frame Body   */
    /* -------------------------------------------------*/
    /* |   Category   |   Action   |   OperatingMode   |*/
    /* -------------------------------------------------*/
    /* |   1          |   1        |   1               |*/
    /* -------------------------------------------------*/
    /*                                                  */
    /****************************************************/

    /* 获取pst_opmode_notify的指针 */
    pst_opmode_notify = (mac_opmode_notify_stru *)(puc_data + MAC_ACTION_OFFSET_ACTION + 1);

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{dmac_mgmt_rx_opmode_notify_frame::user[%d] nss type[%d],rx nss[%d],bandwidch[%d]!}",
        pst_mac_user->us_assoc_id, pst_opmode_notify->bit_rx_nss_type,
        pst_opmode_notify->bit_rx_nss, pst_opmode_notify->bit_channel_width);

#ifdef _PRE_WLAN_FEATURE_M2S
    en_avail_bf_num_spatial_stream = pst_mac_user->en_avail_bf_num_spatial_stream;
    en_avail_num_spatial_stream    = pst_mac_user->en_avail_num_spatial_stream;
    en_bwcap_user                  = pst_mac_user->en_avail_bandwidth;

    ul_relt = mac_ie_proc_opmode_field(pst_mac_vap, pst_mac_user, pst_opmode_notify);
    if (OAL_UNLIKELY(OAL_SUCC != ul_relt))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE, "{dmac_mgmt_rx_opmode_notify_frame::mac_ie_proc_opmode_field failed[%d].}", ul_relt);
        return ul_relt;
    }

    /*1.若空间流能力发送变化，则调用算法钩子函数,如果带宽和空间流同时改变，要先调用空间流的算法函数*/
    if ((pst_mac_user->en_avail_bf_num_spatial_stream != en_avail_bf_num_spatial_stream) ||
          (pst_mac_user->en_avail_num_spatial_stream != en_avail_num_spatial_stream))
    {
        en_nss_change = OAL_TRUE;
    }

    /* 2.opmode带宽改变通知算法,并同步带宽信息到HOST */
    if (pst_mac_user->en_avail_bandwidth != en_bwcap_user)
    {
        en_bw_change = OAL_TRUE;
    }

    if(OAL_TRUE == en_nss_change || OAL_TRUE == en_bw_change)
    {
        /* 根据带宽或者nss变化，刷新硬件队列的包，并通知算法，同步到host侧 */
        dmac_m2s_nss_and_bw_alg_notify(pst_mac_vap, pst_mac_user, en_nss_change, en_bw_change);
    }
#else

    dmac_ie_proc_opmode_notify(pst_mac_user, pst_mac_vap, pst_opmode_notify);

    /* user能力同步到hmac */
    if (OAL_SUCC != dmac_config_d2h_user_m2s_info_syn(pst_mac_vap, pst_mac_user))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_OPMODE,
                  "{dmac_mgmt_rx_opmode_notify_frame::dmac_config_d2h_user_m2s_info_syn failed.}");
    }
#endif

    return OAL_SUCC;
}
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
