


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "dmac_config.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_smps.h"
#include "mac_ie.h"
#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_SMPS_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/


oal_uint8 dmac_smps_get_device_optimal_mode(mac_device_stru *pst_mac_device)
{
    mac_vap_stru                         *pst_mac_vap = OAL_PTR_NULL;
    oal_uint8                             uc_vap_idx;
    oal_uint8                             uc_static_smps_cnt = 0;

    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMPS, "{dmac_smps_get_device_optimal_mode::pst_mac_device null.}");
        return WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_smps_check_all_vap_status::pst_mac_vap null.}");
            continue;
        }

        /* 不支持HT则MIMO */
        if (OAL_FALSE == mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap))
        {
            return WLAN_MIB_MIMO_POWER_SAVE_MIMO;
        }

        /* 存在一个vap MIB不支持SMPS则MIMO */
        if (WLAN_MIB_MIMO_POWER_SAVE_MIMO == mac_vap_get_smps_mode(pst_mac_vap))
        {
            return WLAN_MIB_MIMO_POWER_SAVE_MIMO;
        }

        /* 关联的VAP存在静态SMPS */
        if (WLAN_MIB_MIMO_POWER_SAVE_STATIC == mac_vap_get_smps_mode(pst_mac_vap))
        {
            uc_static_smps_cnt++;
        }
    }

    /* 存在静态SMPS则MAC能力为静态 */
    if (uc_static_smps_cnt != 0)
    {
        return WLAN_MIB_MIMO_POWER_SAVE_STATIC;
    }

    return WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC;
}


oal_uint8 dmac_smps_get_hal_mode(oal_uint8 uc_dev_smps_mode)
{
    hal_smps_mode_enum_uint8 uc_hal_smps_mode = HAL_SMPS_MODE_DYNAMIC;

    if (WLAN_MIB_MIMO_POWER_SAVE_MIMO == uc_dev_smps_mode)
    {
        uc_hal_smps_mode = HAL_SMPS_MODE_DISABLE;
    }
    else if (WLAN_MIB_MIMO_POWER_SAVE_STATIC == uc_dev_smps_mode)
    {
        uc_hal_smps_mode = HAL_SMPS_MODE_STATIC;
    }

    return uc_hal_smps_mode;
}


oal_void dmac_smps_encap_action(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_netbuf,
          oal_uint16 *puc_len, wlan_mib_mimo_power_save_enum_uint8  en_smps_mode, oal_bool_enum_uint8 en_bool)
{
    oal_uint16                     us_index;
    mac_ieee80211_frame_stru      *puc_mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf);
    mac_smps_action_mgt_stru      *puc_payload_addr = (mac_smps_action_mgt_stru *)oal_netbuf_data(pst_netbuf);

    *puc_len = 0;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /* 设置 Frame Control field */
    mac_hdr_set_frame_control((oal_uint8 *)puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /*  power management bit is never sent by an AP */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        puc_mac_header->st_frame_control.bit_power_mgmt = en_bool;
    }

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number((oal_uint8 *)puc_mac_header, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID)*/
    oal_set_mac_addr((oal_uint8 *)puc_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_mac_vap->auc_bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr((oal_uint8 *)puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr((oal_uint8 *)puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    /*************************************************************************/
    /*                  SMPS Management frame - Frame Body                   */
    /* ----------------------------------------------------------------------*/
    /* |Category |HT Action |SMPS Control field|                             */
    /* ----------------------------------------------------------------------*/
    /* |1        |1         |1                 |                             */
    /* ----------------------------------------------------------------------*/
    /*                                                                       */
    /*************************************************************************/
    us_index = MAC_80211_FRAME_LEN;

    puc_payload_addr->category = MAC_ACTION_CATEGORY_HT;            /* HT Category */
    puc_payload_addr->action = MAC_HT_ACTION_SMPS;               /* HT Action */

    us_index += MAC_IE_HDR_LEN;

    /* SMPS Control field */
    /* Bit0 --- enable/disable smps   Bit1 --- SMPS MODE */
    puc_payload_addr->sm_ctl = 0;                                /* Element ID */

    /* Bit0始终为1表示enable SMPS; Bit1取MAC smps MODE */
    if (WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC == en_smps_mode)
    {
        puc_payload_addr->sm_ctl = BIT1 | BIT0;
    }
    else if (WLAN_MIB_MIMO_POWER_SAVE_STATIC == en_smps_mode)
    {
        puc_payload_addr->sm_ctl = BIT0;
    }

    us_index++;

    *puc_len = us_index;

    return ;
}


oal_void dmac_smps_send_action(mac_vap_stru *pst_mac_vap,
       wlan_mib_mimo_power_save_enum_uint8  en_smps_mode, oal_bool_enum_uint8 en_bool)
{
    dmac_vap_stru                        *pst_dmac_vap = OAL_PTR_NULL;
    oal_netbuf_stru                      *pst_mgmt_buf = OAL_PTR_NULL;
    oal_uint16                            us_mgmt_len;
    mac_tx_ctl_stru                      *pst_tx_ctl;
    oal_uint32                            ul_ret;

    pst_dmac_vap = (dmac_vap_stru*)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMPS, "{dmac_smps_send_action::pst_dmac_vap null.}");
        return;
    }

    /* 申请管理帧内存 */
    pst_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_mgmt_buf)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_smps_send_action::pst_mgmt_buf null.}");
        OAL_MEM_INFO_PRINT(OAL_MEM_POOL_ID_NETBUF);
        return;
    }

    oal_set_netbuf_prev(pst_mgmt_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_mgmt_buf, OAL_PTR_NULL);
    OAL_MEM_NETBUF_TRACE(pst_mgmt_buf, OAL_TRUE);

    dmac_smps_encap_action(pst_mac_vap, pst_mgmt_buf, &us_mgmt_len, en_smps_mode, en_bool);
    if (0 == us_mgmt_len)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_smps_send_action::us_mgmt_len is zero.}");
        oal_netbuf_free(pst_mgmt_buf);
        return;
    }

    /* 调用发送管理帧接口 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_mgmt_buf);

    OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl)    = pst_mac_vap->us_assoc_vap_id;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl)    = WLAN_WME_AC_MGMT;
    MAC_GET_CB_FRAME_TYPE(pst_tx_ctl)     = WLAN_CB_FRAME_TYPE_ACTION;
    MAC_GET_CB_FRAME_SUBTYPE(pst_tx_ctl)  = WLAN_ACTION_SMPS_FRAME_SUBTYPE;

    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, us_mgmt_len);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_smps_send_action::dmac_tx_mgmt failed[%d].", ul_ret);
        oal_netbuf_free(pst_mgmt_buf);
        return ;
    }
}


oal_void dmac_smps_update_user_capbility(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    wlan_nss_enum_uint8                   en_avail_num_spatial_stream;

    switch(pst_mac_user->st_ht_hdl.bit_sm_power_save)
    {
        case WLAN_MIB_MIMO_POWER_SAVE_STATIC:
            en_avail_num_spatial_stream = WLAN_SINGLE_NSS;
            break;
        case WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC:
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_smps_update_user_capbility: user smps mode is DYNAMIC!}");
            en_avail_num_spatial_stream = WLAN_DOUBLE_NSS;
            break;
        case WLAN_MIB_MIMO_POWER_SAVE_MIMO:
            en_avail_num_spatial_stream = WLAN_DOUBLE_NSS;
            break;
        default:
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_smps_update_user_capbility: en_user_smps_mode mode[%d] fail!}",
            pst_mac_user->st_ht_hdl.bit_sm_power_save);
            return;
    }

    /*SMPS能力超过user nss则不处理*/
    if(en_avail_num_spatial_stream > pst_mac_user->en_user_num_spatial_stream)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS,
        "{dmac_smps_update_user_capbility::smps exceed user_nss}");
        return;
    }

    /* 其他能力都已经同步，只是需要同步空间流个数即可 */
    pst_mac_user->en_avail_num_spatial_stream = OAL_MIN(en_avail_num_spatial_stream, pst_mac_vap->en_vap_rx_nss);

    /* 调用算法钩子函数 */
    dmac_alg_cfg_user_spatial_stream_notify(pst_mac_user);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* user能力同步到hmac */
    if (OAL_SUCC != dmac_config_d2h_user_m2s_info_syn(pst_mac_vap, pst_mac_user))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS,
                  "{dmac_smps_update_user_capbility::dmac_config_d2h_user_m2s_info_syn failed.}");
    }
#endif
}


oal_void dmac_smps_update_device_capbility(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru                        *pst_mac_device;
    wlan_mib_mimo_power_save_enum_uint8     en_smps_mode;

    /* 获取device结构的信息 */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_SMPS, "{dmac_smps_update_device_capbility::pst_mac_device null[%d].}", pst_mac_vap->uc_device_id);
        return ;
    }

    /* VAP更新能力后,更新device SMPS能力 */
    en_smps_mode = dmac_smps_get_device_optimal_mode(pst_mac_device);

    if (pst_mac_device->en_mac_smps_mode == en_smps_mode)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_smps_update_device_capbility:: no need to change smps mode[%d][1.static 2.dynamic 3.mimo].}",en_smps_mode);
        return ;
    }

    pst_mac_device->en_mac_smps_mode = en_smps_mode;

    /* MAC的能力变更，STA发送action控制帧切换状态 */
    dmac_smps_send_action(pst_mac_vap, en_smps_mode, OAL_FALSE);

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_smps_update_device_capbility:: set DEVICE smps mode[%d][1.static 2.dynamic 3.mimo].}",en_smps_mode);
}


oal_void dmac_smps_set_mac_mode(hal_to_dmac_device_stru *pst_hal_device)
{
    mac_device_stru             *pst_mac_device;
    hal_smps_mode_enum_uint8     uc_hal_smps_mode;

    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMPS, "{dmac_smps_set_mac_mode::pst_hal_device null.}");
        return ;
    }

    /* 获取device结构的信息 */
    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMPS, "{dmac_smps_set_mac_mode::pst_dmac_device null.}");
        return ;
    }

    /* 获取需要切换的SMPS mode */
    uc_hal_smps_mode = dmac_smps_get_hal_mode(pst_mac_device->en_mac_smps_mode);

    /* 写SMPS控制寄存器 */
    hal_set_smps_mode(pst_hal_device, uc_hal_smps_mode);

    OAM_WARNING_LOG2(0, OAM_SF_SMPS, "{dmac_smps_set_mac_mode::uc_mac_smps_mode[%d],uc_hal_smps_mode[%d].}", pst_mac_device->en_mac_smps_mode, uc_hal_smps_mode);
}


oal_void dmac_smps_check_rx_action(hal_to_dmac_device_stru *pst_hal_device, oal_netbuf_stru* pst_buf)
{
    mac_ieee80211_frame_stru           *puc_mac_header;
    oal_uint8                           uc_mgmt_subtype;
    mac_smps_action_mgt_stru           *puc_payload_addr;


    puc_mac_header = (mac_ieee80211_frame_stru*)oal_netbuf_header(pst_buf);
    uc_mgmt_subtype = mac_frame_get_subtype_value((oal_uint8*)puc_mac_header);
    puc_payload_addr = (mac_smps_action_mgt_stru*)oal_netbuf_data(pst_buf);

    /* action发送成功 设置MAC smps能力 */
    if(WLAN_ACTION == uc_mgmt_subtype)
    {
        if(MAC_ACTION_CATEGORY_HT == puc_payload_addr->category && MAC_HT_ACTION_SMPS == puc_payload_addr->action)
        {
            dmac_smps_set_mac_mode(pst_hal_device);
        }
    }
}


oal_uint32 dmac_check_smps_field(
                mac_vap_stru                   *pst_mac_vap,
                oal_uint8                       *puc_payload,
                oal_uint32                       ul_msg_len,
                mac_user_stru                   *pst_mac_user)
{
    oal_uint8 *puc_ie = OAL_PTR_NULL;
    wlan_mib_mimo_power_save_enum_uint8 uc_smps;
    puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_payload, (oal_int32)ul_msg_len);
    if(OAL_PTR_NULL == puc_ie)
    {
        return OAL_FAIL;
    }
    uc_smps = mac_ie_proc_sm_power_save_field(pst_mac_user, (puc_ie[MAC_IE_HDR_LEN] & (BIT3 | BIT2)) >> 2);
    if(uc_smps != pst_mac_user->st_ht_hdl.bit_sm_power_save)
    {
        pst_mac_user->st_ht_hdl.bit_sm_power_save = uc_smps;

        mac_smps_update_user_status(pst_mac_vap, pst_mac_user);
    }
    return OAL_SUCC;
}


oal_uint32  dmac_mgmt_rx_smps_frame(mac_vap_stru *pst_mac_vap, dmac_user_stru *pst_dmac_user, oal_uint8 *puc_frame_payload)
{
    mac_user_stru                          *pst_mac_user;
    wlan_mib_mimo_power_save_enum_uint8     en_user_smps_mode;

    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == pst_dmac_user) || (OAL_PTR_NULL == puc_frame_payload))
    {
        OAM_ERROR_LOG3(0, OAM_SF_OPMODE, "{dmac_mgmt_rx_smps_frame::pst_mac_vap = [%p], pst_mac_user = [%p],puc_frame_payload = [%p]!}\r\n",
                        pst_mac_vap, pst_dmac_user, puc_frame_payload);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_user   = &(pst_dmac_user->st_user_base_info);

    if(IS_VAP_SINGLE_NSS(pst_mac_vap) || IS_USER_SINGLE_NSS(pst_mac_user))
    {
        return OAL_SUCC;
    }

    /* 更新STA的sm_power_save field, 获取enable bit */
    if (0 == (puc_frame_payload[MAC_ACTION_OFFSET_ACTION + 1] & BIT0))
    {

        en_user_smps_mode = WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    }
    else
    {
        /* 如果SMPS enable,则配置为相应模式(不考虑动态状态更新，动态只支持配置命令配置) */
        if (0 == (puc_frame_payload[MAC_ACTION_OFFSET_ACTION + 1] & BIT1))
        {
            /* 静态SMPS */
            en_user_smps_mode = WLAN_MIB_MIMO_POWER_SAVE_STATIC;
        }
        else/* 动态SMPS */
        {
            en_user_smps_mode = WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC;
        }
    }

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_mgmt_rx_smps_frame::user[%d] smps mode[%d] change to[%d]!}",
        pst_mac_user->us_assoc_id, pst_mac_user->st_ht_hdl.bit_sm_power_save, en_user_smps_mode);

    /* 用户更新的smps能力不能超过本vap的能力 */
    if(en_user_smps_mode > mac_mib_get_smps(pst_mac_vap))
    {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_mgmt_rx_smps_frame::user[%d] new smps mode[%d] > vap smps mode[%d]!}",
            pst_mac_user->us_assoc_id, en_user_smps_mode, mac_mib_get_smps(pst_mac_vap));
        return OAL_FAIL;
    }

    /* 如果user的SMPS状态发生改变，需要做user和vap状态更新 */
    if (en_user_smps_mode != pst_mac_user->st_ht_hdl.bit_sm_power_save)
    {
        /* 更新user的smps能力 */
        pst_mac_user->st_ht_hdl.bit_sm_power_save = en_user_smps_mode;

        mac_smps_update_user_status(pst_mac_vap, pst_mac_user);

#ifdef _PRE_WLAN_FEATURE_M2S
        /* 根据nss变化，刷新硬件队列的包，并通知算法，同步到host侧 */
        dmac_m2s_nss_and_bw_alg_notify(pst_mac_vap, pst_mac_user, OAL_TRUE, OAL_FALSE);
#else

        /* 调用算法钩子函数 */
        dmac_alg_cfg_user_spatial_stream_notify(&(pst_dmac_user->st_user_base_info));

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* user能力同步到hmac */
        if (OAL_SUCC != dmac_config_d2h_user_m2s_info_syn(pst_mac_vap, &(pst_dmac_user->st_user_base_info)))
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS,
                      "{dmac_mgmt_rx_smps_frame::dmac_config_d2h_user_m2s_info_syn failed.}");
        }
#endif
#endif
    }

    return OAL_SUCC;
}
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
