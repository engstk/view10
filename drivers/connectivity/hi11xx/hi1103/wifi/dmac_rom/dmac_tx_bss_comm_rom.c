


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_util.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "oal_sdio.h"

#include "hal_ext_if.h"

#include "mac_frame.h"
#include "mac_data.h"

#include "dmac_tx_bss_comm.h"
#include "dmac_blockack.h"
#include "dmac_tx_complete.h"
#include "dmac_psm_ap.h"
#include "dmac_uapsd.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_11w.h"
#include "dmac_11i.h"
#include "dmac_dft.h"
#include "dmac_alg.h"
#include "dmac_fcs.h"

#ifdef _PRE_WLAN_FEATURE_DBAC
#include "mac_device.h"
#include "dmac_device.h"
#endif

#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_test_main.h"
#include "dmac_lpm_test.h"
#include "dmac_test_sch.h"
#include "dmac_config.h"
#endif

#ifdef _PRE_WIFI_DMT
#include "hal_witp_dmt_if.h"
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#include "pm_extern.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif

#include "oal_profiling.h"
#include "dmac_config.h"

#include "dmac_auto_adjust_freq.h"
#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
#include "dmac_vap.h"
#include "dmac_resource.h"
#endif

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
#include "oal_hcc_slave_if.h"
#endif
#ifdef _PRE_WLAN_FEATURE_AP_PM
#include "mac_pm.h"
#endif

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#include "dmac_user_extend.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TX_BSS_COMM_ROM_C

/*****************************************************************************
  2 函数原型声明
*****************************************************************************/

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
dmac_tx_bss_comm_rom_cb   g_st_dmac_tx_bss_comm_rom_cb = {dmac_tx_excp_free_netbuf,
                                                          dmac_tx_excp_free_dscr};


/*****************************************************************************
  4 函数实现
*****************************************************************************/


oal_uint32  dmac_tx_dump_get_switch(oam_user_track_frame_type_enum_uint8     en_frame_type,
                                    oal_uint8                               *pen_frame_switch,
                                    oal_uint8                               *pen_cb_switch,
                                    oal_uint8                               *pen_dscr_switch,
                                    mac_tx_ctl_stru                         *pst_tx_cb)
{
    oal_uint32                   ul_ret;
    mac_ieee80211_frame_stru    *pst_frame_hdr;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pen_frame_switch)
        || OAL_UNLIKELY(OAL_PTR_NULL == pen_cb_switch)
        || OAL_UNLIKELY(OAL_PTR_NULL == pen_dscr_switch)
        || OAL_UNLIKELY(OAL_PTR_NULL == pst_tx_cb))
    {
        OAM_ERROR_LOG4(0, OAM_SF_TX,
                        "{dmac_tx_dump_get_switch::param null, pen_frame_switch=%d pen_cb_switch=%d pen_dscr_switch=%d pst_tx_cb=%d.}",
                        pen_frame_switch, pen_cb_switch, pen_dscr_switch, pst_tx_cb);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 判断是否是probe request或者probe response，先获取一下开关 */
    pst_frame_hdr = MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_cb);
#ifndef _PRE_PRODUCT_ID_HI110X_DEV
    if (OAL_PTR_NULL == pst_frame_hdr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_dump_get_switch::pst_frame_hdr null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
    if (WLAN_MANAGEMENT == pst_frame_hdr->st_frame_control.bit_type)
    {
        if(WLAN_PROBE_REQ == pst_frame_hdr->st_frame_control.bit_sub_type
        || WLAN_PROBE_RSP == pst_frame_hdr->st_frame_control.bit_sub_type)
        {
            ul_ret = oam_report_80211_probe_get_switch(OAM_OTA_FRAME_DIRECTION_TYPE_TX,
                                                       pen_frame_switch,
                                                       pen_cb_switch,
                                                       pen_dscr_switch);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_dump_get_switch::oam_report_80211_probe_get_switch failed[%d].}", ul_ret);
                return ul_ret;
            }

            return OAL_SUCC;
        }
        else if(WLAN_ACTION == pst_frame_hdr->st_frame_control.bit_sub_type
                || WLAN_ACTION_NO_ACK == pst_frame_hdr->st_frame_control.bit_sub_type)
        {
                *pen_cb_switch    = 1;
                *pen_dscr_switch  = 1;
                *pen_frame_switch = 1;
                return OAL_SUCC;
        }
    }
    /* 获取非probe request和probe response的开关 */
    if (OAL_TRUE == MAC_GET_CB_IS_MCAST(pst_tx_cb))
    {
        ul_ret = oam_report_80211_mcast_get_switch(OAM_OTA_FRAME_DIRECTION_TYPE_TX,
                                                   en_frame_type,
                                                   pen_frame_switch,
                                                   pen_cb_switch,
                                                   pen_dscr_switch);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG2(0, OAM_SF_TX,
                             "{dmac_tx_dump_get_switch::dmac_tx_dump_get_switch failed! ul_ret=[%d],frame_type=[%d]}",
                             ul_ret, en_frame_type);
            return ul_ret;
        }
    }
    else
    {
        /* 如果是0xffff，说明没有此用户 */
        if (MAC_INVALID_USER_ID == MAC_GET_CB_TX_USER_IDX(pst_tx_cb))
        {
            if (WLAN_MANAGEMENT == pst_frame_hdr->st_frame_control.bit_type
                && (WLAN_DISASOC == pst_frame_hdr->st_frame_control.bit_sub_type
                || WLAN_DEAUTH == pst_frame_hdr->st_frame_control.bit_sub_type))
            {
                *pen_cb_switch    = 1;
                *pen_dscr_switch  = 1;
                *pen_frame_switch = 1;
            }
            else
            {
                *pen_cb_switch    = 0;
                *pen_dscr_switch  = 0;
                *pen_frame_switch = 0;
            }

            return OAL_FAIL;
        }

        /* 获取非probe response 的开关 */
        ul_ret = oam_report_80211_ucast_get_switch(OAM_OTA_FRAME_DIRECTION_TYPE_TX,
                                                   en_frame_type,
                                                   pen_frame_switch,
                                                   pen_cb_switch,
                                                   pen_dscr_switch,
                                                   MAC_GET_CB_TX_USER_IDX(pst_tx_cb));
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG3(0, OAM_SF_TX,
                             "{dmac_tx_dump_get_switch::oam_report_80211_ucast_get_switch failed! ul_ret=[%d],frame_type=[%d], user_idx=[%d]}",
                             ul_ret, en_frame_type, MAC_GET_CB_TX_USER_IDX(pst_tx_cb));
            OAM_WARNING_LOG3(0, OAM_SF_TX, "{dmac_tx_dump_get_switch::frame_switch=[%d], cb_switch=[%d], dscr_switch=[%d]",
                             pen_frame_switch, pen_cb_switch, pen_dscr_switch);
            return ul_ret;
        }
    }

    return OAL_SUCC;
}


oal_uint32  dmac_tx_get_user_macaddr(mac_tx_ctl_stru *pst_tx_cb,
                                                        oal_uint8 auc_user_macaddr[])
{
    mac_user_stru      *pst_mac_user;

    if (OAL_TRUE == MAC_GET_CB_IS_MCAST(pst_tx_cb))
    {
        oal_set_mac_addr(auc_user_macaddr, BROADCAST_MACADDR);
    }
    else
    {
        pst_mac_user = mac_res_get_mac_user(MAC_GET_CB_TX_USER_IDX(pst_tx_cb));
        if (OAL_PTR_NULL == pst_mac_user)
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_get_user_macaddr::pst_mac_user[%d] null.",
                MAC_GET_CB_TX_USER_IDX(pst_tx_cb));
            return OAL_ERR_CODE_PTR_NULL;
        }
        oal_set_mac_addr(auc_user_macaddr, pst_mac_user->auc_user_mac_addr);
    }

    return OAL_SUCC;
}


oal_uint32  dmac_tx_dump_get_user_macaddr(mac_tx_ctl_stru *pst_tx_cb,
                                                     oal_uint8 auc_user_macaddr[])
{
    mac_ieee80211_frame_stru    *pst_frame_hdr;

    pst_frame_hdr = MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_cb);
#ifndef _PRE_PRODUCT_ID_HI110X_DEV
    if (OAL_PTR_NULL == pst_frame_hdr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tx_dump_get_user_macaddr::pst_frame_hdr null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif
    /* 判断是否是probe request或者probe response，GO Neg, GAS 不用去找用户，因为找不到 */
    if (WLAN_MANAGEMENT == pst_frame_hdr->st_frame_control.bit_type
        && ((WLAN_PROBE_REQ == pst_frame_hdr->st_frame_control.bit_sub_type)
        || (WLAN_PROBE_RSP == pst_frame_hdr->st_frame_control.bit_sub_type)
        || ((WLAN_ACTION == pst_frame_hdr->st_frame_control.bit_sub_type) && (MAC_GET_CB_TX_USER_IDX(pst_tx_cb) == MAC_INVALID_USER_ID))))
    {
        oal_set_mac_addr(auc_user_macaddr, BROADCAST_MACADDR);
        return OAL_SUCC;
    }

    return dmac_tx_get_user_macaddr(pst_tx_cb, auc_user_macaddr);
}



oal_void  dmac_tx_dump(dmac_tx_dump_param_stru *pst_tx_dump_param)
{
    oal_uint32               ul_dscr_one_size = 0;
    oal_uint32               ul_dscr_two_size = 0;
    oal_uint32               ul_ret;

    /* 上报帧内容 */
    if (OAL_SWITCH_ON == pst_tx_dump_param->en_frame_switch)
    {
       ul_ret =  oam_report_80211_frame(pst_tx_dump_param->auc_user_macaddr,
                               pst_tx_dump_param->puc_mac_hdr_addr,
                               pst_tx_dump_param->uc_mac_hdr_len,
                               pst_tx_dump_param->puc_mac_payload_addr,
                               pst_tx_dump_param->us_mac_frame_len,
                               OAM_OTA_FRAME_DIRECTION_TYPE_TX);
       if (OAL_SUCC != ul_ret)
       {
           OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_dump::oam_report_80211_frame return err: 0x%x.}\r\n", ul_ret);
       }
    }

    /* 上报帧对应的发送描述符 */
    if (OAL_SWITCH_ON == pst_tx_dump_param->en_dscr_switch)
    {
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        hal_tx_get_size_dscr(pst_tx_dump_param->pst_hal_device,
                             pst_tx_dump_param->pst_tx_cb->bit_netbuf_num,
                             &ul_dscr_one_size,
                             &ul_dscr_two_size);
#else
        hal_tx_get_size_dscr(pst_tx_dump_param->pst_hal_device,
                             pst_tx_dump_param->pst_tx_cb->uc_netbuf_num,
                             &ul_dscr_one_size,
                             &ul_dscr_two_size);
#endif
        ul_ret = oam_report_dscr(pst_tx_dump_param->auc_user_macaddr,
                        (oal_uint8 *)pst_tx_dump_param->pst_tx_dscr,
                        (oal_uint16)(ul_dscr_one_size + ul_dscr_two_size),
                        OAM_OTA_TX_DSCR_TYPE);

        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_dump::oam_report_dscr return err: 0x%x.}\r\n", ul_ret);
        }
    }

    /* 上报帧对应的CB */
    if (OAL_SWITCH_ON == pst_tx_dump_param->en_cb_switch)
    {
        ul_ret = oam_report_netbuf_cb(pst_tx_dump_param->auc_user_macaddr,
                             (oal_uint8 *)OAL_NETBUF_CB(pst_tx_dump_param->pst_netbuf),
                             OAM_OTA_TYPE_TX_CB);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_dump::oam_report_netbuf_cb return err: 0x%x.}\r\n", ul_ret);
        }
    }
}


oal_void dmac_tx_dump_data(hal_to_dmac_device_stru     *pst_hal_device,
                            oal_dlist_head_stru         *pst_tx_dscr_list_hdr)
{
    oal_netbuf_stru         *pst_netbuf = OAL_PTR_NULL;
    hal_tx_dscr_stru        *pst_dscr   = OAL_PTR_NULL;
    oal_dlist_head_stru     *pst_dlist_node;
    oal_switch_enum_uint8    en_frame_switch = 0;
    oal_switch_enum_uint8    en_cb_switch = 0;
    oal_switch_enum_uint8    en_dscr_switch = 0;
    oal_uint32               ul_ret = 0;
    oal_uint8                auc_user_macaddr[WLAN_MAC_ADDR_LEN] = {0};
    dmac_tx_dump_param_stru  st_tx_dump_param;
    mac_tx_ctl_stru             *pst_tx_ctl;

    pst_dlist_node = pst_tx_dscr_list_hdr->pst_next;
    while (pst_dlist_node != pst_tx_dscr_list_hdr)
    {
        pst_dscr = OAL_DLIST_GET_ENTRY(pst_dlist_node, hal_tx_dscr_stru, st_entry);
        pst_netbuf = pst_dscr->pst_skb_start_addr;

        if(OAL_PTR_NULL != pst_netbuf)
        {
            pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

            /* 获取打印开关 */
            ul_ret = dmac_tx_dump_get_switch(OAM_USER_TRACK_FRAME_TYPE_DATA,
                                             &en_frame_switch,
                                             &en_cb_switch,
                                             &en_dscr_switch,
                                             pst_tx_ctl);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_dump_data::dmac_tx_dump_get_switch failed[%d].}", ul_ret);
                pst_dlist_node = pst_dlist_node->pst_next;
                continue;
            }

            /* 获取用户mac地址，用于SDT过滤 */
            ul_ret = dmac_tx_dump_get_user_macaddr(pst_tx_ctl, auc_user_macaddr);
            if (OAL_SUCC != ul_ret)
            {
                OAM_ERROR_LOG1(0, OAM_SF_TX, "{dmac_tx_dump_get_switch::dmac_tx_dump_get_switch failed[%d].}", ul_ret);

                pst_dlist_node = pst_dlist_node->pst_next;
                continue;
            }

            /* 填写打印参数 */
            st_tx_dump_param.pst_hal_device       = pst_hal_device;
            st_tx_dump_param.pst_netbuf           = pst_netbuf;
            st_tx_dump_param.pst_tx_cb            = pst_tx_ctl;
            st_tx_dump_param.pst_tx_dscr          = pst_dscr;

            st_tx_dump_param.puc_mac_hdr_addr     = (oal_uint8 *)(MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl));
            st_tx_dump_param.puc_mac_payload_addr = oal_netbuf_payload(pst_netbuf);
            st_tx_dump_param.uc_mac_hdr_len       = MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl);
            st_tx_dump_param.us_mac_frame_len     = (oal_uint16)MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl)
                                                  + (oal_uint16)oal_netbuf_get_len(pst_netbuf);

            st_tx_dump_param.en_frame_switch      = en_frame_switch;
            st_tx_dump_param.en_cb_switch         = en_cb_switch;
            st_tx_dump_param.en_dscr_switch       = en_dscr_switch;
            oal_set_mac_addr(st_tx_dump_param.auc_user_macaddr, auc_user_macaddr);

            dmac_tx_dump(&st_tx_dump_param);

        }
        else
        {
            OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_dump_data::pst_netbuf null.}");
        }

        pst_dlist_node = pst_dlist_node->pst_next;
    }
}


oal_void  dmac_tx_dump_mgmt(hal_to_dmac_device_stru *pst_hal_device,
                                oal_netbuf_stru *pst_netbuf_mgmt,
                                hal_tx_mpdu_stru *pst_mpdu,
                                hal_tx_dscr_stru *pst_mgmt_dscr)
{
    mac_tx_ctl_stru       *pst_tx_cb;
    oal_switch_enum_uint8  en_frame_switch = 0;
    oal_switch_enum_uint8  en_cb_switch = 0;
    oal_switch_enum_uint8  en_dscr_switch = 0;
    oal_uint32             ul_ret;
    oal_uint8              auc_user_macaddr[WLAN_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    dmac_tx_dump_param_stru  st_tx_dump_param;
    mac_ieee80211_frame_stru  *pst_mac_header;
    oal_uint8                 uc_subtype;

    pst_mac_header = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf_mgmt);

    uc_subtype = mac_get_frame_type_and_subtype((oal_uint8 *)pst_mac_header);

    if (((WLAN_FC0_SUBTYPE_NODATA | WLAN_FC0_TYPE_DATA) == uc_subtype)
        || ((WLAN_FC0_SUBTYPE_QOS_NULL | WLAN_FC0_TYPE_DATA) == uc_subtype))
    {
        /*null data帧不需要打印*/
        return;
    }

    pst_tx_cb = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_netbuf_mgmt);

    /* 获取开关，要区分是组播帧还是单播帧的开关 */
    ul_ret = dmac_tx_dump_get_switch(OAM_USER_TRACK_FRAME_TYPE_MGMT,
                                     &en_frame_switch,
                                     &en_cb_switch,
                                     &en_dscr_switch,
                                     pst_tx_cb);
    if (OAL_ERR_CODE_PTR_NULL == ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_tx_dump_mgmt::dmac_tx_dump_get_switch failed[%d].}", ul_ret);
        return;
    }

    if (OAL_SUCC == ul_ret)
    {
        /* 获取用户mac地址，用于SDT过滤 */
        ul_ret = dmac_tx_dump_get_user_macaddr(pst_tx_cb, auc_user_macaddr);
        if (OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG1(0, OAM_SF_TX, "{dmac_tx_dump_mgmt::dmac_tx_get_user_macaddr failed[%d].", ul_ret);

            return;
        }
    }
    /* 填写打印参数 */
    st_tx_dump_param.en_cb_switch = en_cb_switch;
    st_tx_dump_param.en_dscr_switch = en_dscr_switch;
    st_tx_dump_param.en_frame_switch = en_frame_switch;
    st_tx_dump_param.pst_hal_device = pst_hal_device;
    st_tx_dump_param.pst_netbuf = pst_netbuf_mgmt;
    st_tx_dump_param.pst_tx_cb = pst_tx_cb;
    st_tx_dump_param.pst_tx_dscr = pst_mgmt_dscr;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    st_tx_dump_param.puc_mac_hdr_addr =  (oal_uint8 *)(MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_cb));
    st_tx_dump_param.uc_mac_hdr_len = MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_cb);
    st_tx_dump_param.puc_mac_payload_addr =  (oal_uint8 *)(MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_cb))+st_tx_dump_param.uc_mac_hdr_len;
    st_tx_dump_param.us_mac_frame_len =  (oal_uint16)MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_cb)
                                                  + (oal_uint16)MAC_GET_CB_MPDU_LEN(pst_tx_cb);
#else
    st_tx_dump_param.puc_mac_hdr_addr = oal_netbuf_header(pst_netbuf_mgmt);
    st_tx_dump_param.puc_mac_payload_addr = oal_netbuf_payload(pst_netbuf_mgmt);
    st_tx_dump_param.uc_mac_hdr_len = pst_mpdu->st_mpdu_mac_hdr.uc_mac_hdr_len;
    st_tx_dump_param.us_mac_frame_len = pst_mpdu->us_mpdu_len;
#endif

    oal_set_mac_addr(st_tx_dump_param.auc_user_macaddr, auc_user_macaddr);

    dmac_tx_dump(&st_tx_dump_param);
}



oal_uint32  dmac_tx_switch_tx_queue(hal_tx_dscr_queue_header_stru  *pst_fake_queue1, hal_tx_dscr_queue_header_stru  *pst_fake_queue2)
{
    oal_uint8                       uc_q_idx;
    oal_uint8                       uc_ppdu_num;
    oal_dlist_head_stru             st_head_tmp;

    for (uc_q_idx = 0; uc_q_idx < HAL_TX_QUEUE_BUTT; uc_q_idx++)
    {
        oal_dlist_init_head(&st_head_tmp);
        oal_dlist_join_head(&st_head_tmp, &pst_fake_queue1[uc_q_idx].st_header);

        oal_dlist_init_head(&pst_fake_queue1[uc_q_idx].st_header);
        oal_dlist_join_head(&pst_fake_queue1[uc_q_idx].st_header, &pst_fake_queue2[uc_q_idx].st_header);

        oal_dlist_init_head(&pst_fake_queue2[uc_q_idx].st_header);
        oal_dlist_join_head(&pst_fake_queue2[uc_q_idx].st_header, &st_head_tmp);

        uc_ppdu_num                           = pst_fake_queue1[uc_q_idx].uc_ppdu_cnt;
        pst_fake_queue1[uc_q_idx].uc_ppdu_cnt = pst_fake_queue2[uc_q_idx].uc_ppdu_cnt;
        pst_fake_queue2[uc_q_idx].uc_ppdu_cnt = uc_ppdu_num;
    }

    return OAL_SUCC;
}

oal_void dmac_tx_check_need_response_to_hmac(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru  *pst_tx_dscr)
{
    oal_uint8            uc_vap_id;
    oal_uint8            uc_dscr_status;
    mac_tx_ctl_stru     *pst_cb;
    mac_vap_stru        *pst_mac_vap;
    dmac_user_stru      *pst_dmac_user;

    if (OAL_PTR_NULL == pst_tx_dscr->pst_skb_start_addr)
    {
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_tx_check_need_response_to_hmac::pst_buf null.}");
        return;
    }

    pst_cb   = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_tx_dscr->pst_skb_start_addr);
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_cb));

    /* 检查cb字段 bit_need_rsp, 上报hmac 传送状态*/
    if ((pst_dmac_user != OAL_PTR_NULL) || (MAC_GET_CB_IS_NEED_RESP(pst_cb) != OAL_TRUE))
    {
        return;
    }

    /* mgmt tx 结束上报 */
    dmac_tx_get_vap_id(pst_hal_device, pst_tx_dscr, &uc_vap_id);
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_TX, "{dmac_tx_check_need_response_to_hmac::pst_mac_vap null!!!.}");
        return;
    }

    /* 获取发送状态位 */
    hal_tx_get_dscr_status(pst_hal_device, pst_tx_dscr, &uc_dscr_status);

    OAM_WARNING_LOG2(uc_vap_id, OAM_SF_TX, "{dmac_tx_check_need_response_to_hmac::frame id[%d],tx status[%d]send to host!!!}",MAC_GET_CB_MGMT_FRAME_ID(pst_cb),uc_dscr_status);

    /*此时获取发送状态可能还存在非INVALID的,这里是最后清直接传入INVALID,告知host结束即可 */
    dmac_mgmt_tx_complete(pst_mac_vap, MAC_GET_CB_MGMT_FRAME_ID(pst_cb), DMAC_TX_INVALID, MAC_GET_CB_TX_USER_IDX(pst_cb));
}


oal_void  dmac_clear_tx_queue(hal_to_dmac_device_stru *pst_hal_device)
{
    oal_int8             c_q_id             = 0;
    oal_dlist_head_stru *pst_dlist_entry    = OAL_PTR_NULL;
    hal_tx_dscr_stru    *pst_tx_dscr        = OAL_PTR_NULL;

    /* 遍历6个发送队列 一定要先处理高优先级队列防止普通优先级队列发送完成产生管理帧入错队列 */
    for (c_q_id = HAL_TX_QUEUE_BUTT - 1; c_q_id >= 0; c_q_id--)
    {
        while(!oal_dlist_is_empty(&pst_hal_device->ast_tx_dscr_queue[(oal_uint8)c_q_id].st_header))
        {
            pst_dlist_entry = pst_hal_device->ast_tx_dscr_queue[(oal_uint8)c_q_id].st_header.pst_next;

            pst_tx_dscr     = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_entry, hal_tx_dscr_stru, st_entry);

            dmac_tx_check_need_response_to_hmac(pst_hal_device, pst_tx_dscr);
            /* 注意流控算法异常 */
            dmac_tx_complete_free_dscr(pst_tx_dscr);
        }

        pst_hal_device->ast_tx_dscr_queue[(oal_uint8)c_q_id].uc_ppdu_cnt = 0;
    }
}


oal_uint32 dmac_alg_downlink_flowctl_notify(mac_device_stru *pst_mac_device,
                                                       dmac_vap_stru *pst_dmac_vap,
                                                       OAL_CONST mac_user_stru * OAL_CONST pst_user,
                                                       oal_netbuf_stru *pst_buf)
{
    oal_uint32                         ul_ret;
    dmac_txrx_output_type_enum_uint8   en_output;
#ifdef _PRE_DEBUG_MODE
    pkt_trace_type_enum_uint8          en_trace_pkt_type;
#endif

    /* 不用拥塞控制算法 进行判断(当tx buffer使用完以后，后来的报文直接丢弃) */
    if (OAL_PTR_NULL == g_pst_alg_main->p_downlink_flowctl_func)
    {
        if( WLAN_TID_MPDU_NUM_LIMIT <= pst_mac_device->us_total_mpdu_num)
        {
#ifdef _PRE_DEBUG_MODE
            //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
            en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_buf), MAC_NETBUFF_PAYLOAD_SNAP);
            if( PKT_TRACE_BUTT != en_trace_pkt_type)
            {
                OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_alg_downlink_flowctl_notify::type%d flowctl drop[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
            }
#endif
            OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ACS, "tx buffer is full[%d],the coming buffer drop", pst_mac_device->us_total_mpdu_num);
            return OAL_FAIL;
        }

        return OAL_SUCC;
    }
    else
    {
        /* 多用户多优先级跑流时不使能拥塞控制 */
        if (OAL_FALSE == pst_dmac_vap->en_multi_user_multi_ac_flag)
        {
            /* 根据拥塞控制算法 判断报文是否丢弃 */
            ul_ret = g_pst_alg_main->p_downlink_flowctl_func(&pst_dmac_vap->st_vap_base_info, pst_user, pst_buf, &en_output);
            if ((OAL_SUCC != ul_ret)||(DMAC_TXRX_DROP == en_output))
            {
#ifdef _PRE_DEBUG_MODE
                //增加关键帧打印，首先判断是否是需要打印的帧类型，然后打印出帧类型
                en_trace_pkt_type = mac_pkt_should_trace( OAL_NETBUF_DATA(pst_buf), MAC_NETBUFF_PAYLOAD_SNAP);
                if( PKT_TRACE_BUTT != en_trace_pkt_type)
                {
                    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_alg_downlink_flowctl_notify::type%d flowctl drop[0:dhcp 1:arp_req 2:arp_rsp 3:eapol 4:icmp 5:assoc_req 6:assoc_rsp 9:dis_assoc 10:auth 11:deauth]}\r\n", en_trace_pkt_type);
                }
#endif
                OAM_INFO_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_alg_downlink_flowctl_notify::ul_ret=%d, en_output=%d}",
                              ul_ret, en_output);
                return OAL_FAIL;
            }
        }
        return OAL_SUCC;
    }

}


oal_void  dmac_free_tx_dscr_queue(dmac_vap_stru *pst_dmac_vap, oal_dlist_head_stru *pst_dscr_head)
{
    oal_dlist_head_stru        *pst_dscr_entry      = OAL_PTR_NULL;
    hal_tx_dscr_stru           *pst_tx_dscr         = OAL_PTR_NULL;

    OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, pst_dscr_head)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_free_tx_dscr_queue::tx pkt dropped .}");
        pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);
        oal_dlist_delete_entry(pst_dscr_entry);
        dmac_free_tx_dscr(pst_tx_dscr);
    }

    return;
}


oal_void dmac_tx_mgmt_get_txop_para(dmac_vap_stru *pst_dmac_vap,
                                                          hal_tx_txop_alg_stru **ppst_txop_alg,
                                                          mac_tx_ctl_stru *pst_tx_ctl)
{
    if (OAL_TRUE == MAC_GET_CB_IS_MCAST(pst_tx_ctl))
    {
        /* 获取组播、广播管理帧 发送参数 */
        *ppst_txop_alg = &(pst_dmac_vap->ast_tx_mgmt_bmcast[pst_dmac_vap->st_vap_base_info.st_channel.en_band]);
    }
    else
    {
        /* 获取单播 管理帧 发送参数 */
        *ppst_txop_alg = &(pst_dmac_vap->ast_tx_mgmt_ucast[pst_dmac_vap->st_vap_base_info.st_channel.en_band]);
    }
}



oal_uint32  dmac_flush_txq_to_tid_of_vo(hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint8            uc_q_idx           = 0;
    oal_dlist_head_stru *pst_dlist_entry    = OAL_PTR_NULL;
    hal_tx_dscr_stru    *pst_tx_dscr        = OAL_PTR_NULL;
    mac_tx_ctl_stru     *pst_cb             = OAL_PTR_NULL;
    dmac_tid_stru       *pst_tid            = OAL_PTR_NULL;
    dmac_user_stru      *pst_dmac_user      = OAL_PTR_NULL;
    mac_device_stru                     *pst_mac_device;
    mac_ieee80211_qos_htc_frame_stru *pst_frame_head = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    mac_vap_stru        *pst_vap            = OAL_PTR_NULL;
#endif
    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        MAC_ERR_LOG(0, "pst_hal_device is null");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_flush_txq_to_tid_of_vo::pst_hal_device null.}");

        return OAL_FAIL;
    }

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        MAC_ERR_LOG(0, "pst_mac_device is null");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_flush_txq_to_tid_of_vo::pst_mac_device null.}");

        return OAL_FAIL;
    }

    /* 遍历其余4个发送队列 */
    for (uc_q_idx = 0; uc_q_idx < HAL_TX_QUEUE_HI; uc_q_idx++)
    {
        /* 将发送队列下的报文塞回对应的tid缓存队列 */
        while(!oal_dlist_is_empty(&pst_hal_device->ast_tx_dscr_queue[uc_q_idx].st_header))
        {
            pst_dlist_entry = oal_dlist_delete_tail(&pst_hal_device->ast_tx_dscr_queue[uc_q_idx].st_header);
            pst_tx_dscr     = (hal_tx_dscr_stru *)OAL_DLIST_GET_ENTRY(pst_dlist_entry, hal_tx_dscr_stru, st_entry);

            pst_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_tx_dscr->pst_skb_start_addr);

            pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(MAC_GET_CB_TX_USER_IDX(pst_cb));
            if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
            {
                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_flush_txq_to_tid_of_vo::mac_res_get_dmac_user fail.user_idx = %u}",MAC_GET_CB_TX_USER_IDX(pst_cb));
                continue;
            }
            /* 修改描述符和cb里edca相关的字段 */
            pst_tx_dscr->uc_q_num = HAL_TX_QUEUE_VO;
            MAC_GET_CB_WME_TID_TYPE(pst_cb)= MAC_WMM_SWITCH_TID;
            MAC_GET_CB_WME_AC_TYPE(pst_cb) = WLAN_WME_AC_VO;
            /* 修改帧头的tid */
            pst_frame_head = (mac_ieee80211_qos_htc_frame_stru *)MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb);
            pst_frame_head->bit_qc_tid = MAC_WMM_SWITCH_TID;

            /* 关闭WMM前，将所有Q回收到TID为6的队列 */
            pst_tid       = &pst_dmac_user->ast_tx_tid_queue[MAC_WMM_SWITCH_TID];
        #ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
            pst_tx_dscr->bit_is_retried = OAL_TRUE;
            oal_dlist_add_head(pst_dlist_entry, &pst_tid->st_retry_q);
        #else
            oal_dlist_add_head(pst_dlist_entry, &pst_tid->st_hdr);
        #endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

            g_st_dmac_tid_rom_cb.dmac_tid_tx_enqueue_update_cb(pst_mac_device, pst_tid, 1);

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
            pst_vap = mac_res_get_mac_vap(pst_tid->uc_vap_id);
            dmac_alg_flowctl_backp_notify(pst_vap, pst_mac_device->us_total_mpdu_num, pst_mac_device->aus_ac_mpdu_num);
#endif
        }
        pst_hal_device->ast_tx_dscr_queue[uc_q_idx].uc_ppdu_cnt = 0;
    }


    return OAL_SUCC;
}

oal_void dmac_post_soft_tx_complete_event(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_base_dscr, oal_uint8 uc_dscr_num)
{
    frw_event_mem_stru         *pst_event_mem = OAL_PTR_NULL;          /* 申请事件返回的内存指针 */
    frw_event_stru             *pst_hal_to_dmac_event = OAL_PTR_NULL;  /* 指向申请事件的payload指针 */
    hal_tx_complete_event_stru *pst_tx_comp_event;

    /* 申请事件内存 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(hal_tx_complete_event_stru));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        return;
    }

    /* 获得事件指针 */
    pst_hal_to_dmac_event = frw_get_event_stru(pst_event_mem);

    FRW_EVENT_HDR_INIT(&pst_hal_to_dmac_event->st_event_hdr,
                        FRW_EVENT_TYPE_WLAN_TX_COMP,
                        HAL_TX_COMP_SUB_TYPE_TX,
                        OAL_SIZEOF(hal_tx_complete_event_stru),
                        FRW_EVENT_PIPELINE_STAGE_0,
                        pst_hal_device->uc_chip_id,
                        pst_hal_device->uc_mac_device_id,
                        0);

    /*填写tx complete事件 */
    pst_tx_comp_event = (hal_tx_complete_event_stru *)(pst_hal_to_dmac_event->auc_event_data);
    pst_tx_comp_event->pst_base_dscr    = pst_base_dscr;        /* 设置描述符基地址 */
    pst_tx_comp_event->uc_dscr_num      = uc_dscr_num;          /* 发送的描述符的个数 */
    pst_tx_comp_event->pst_hal_device   = pst_hal_device;

    /* 分发 */
    frw_event_dispatch_event(pst_event_mem);

#ifndef _PRE_WLAN_FEATURE_MEM_OPT
    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);
#endif

}


#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX

oal_void  dmac_tx_data_always_tx(hal_to_dmac_device_stru *pst_hal_device,
                                                             dmac_vap_stru *pst_dmac_vap,
                                                             hal_tx_dscr_stru *pst_tx_dscr)
{
    mac_tx_ctl_stru             *pst_cb = OAL_PTR_NULL;

    /* 常发模式根据标志位决定mac头的内容，随机情况下不改变mac头 */
    if (OAL_SWITCH_ON == pst_dmac_vap->st_vap_base_info.bit_al_tx_flag)
    {
        pst_cb = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_tx_dscr->pst_skb_start_addr);
        if(RF_PAYLOAD_ALL_ZERO == pst_dmac_vap->st_vap_base_info.bit_payload_flag)
        {
            OAL_MEMZERO(MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb), OAL_SIZEOF(mac_ieee80211_frame_stru));
        }
        else if(RF_PAYLOAD_ALL_ONE == pst_dmac_vap->st_vap_base_info.bit_payload_flag)
        {
            oal_memset(MAC_GET_CB_FRAME_HEADER_ADDR(pst_cb), 0xFF, OAL_SIZEOF(mac_ieee80211_frame_stru));
        }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
        hal_tx_ctrl_dscr_link(pst_hal_device, pst_tx_dscr, pst_tx_dscr);
#endif
        hal_rf_test_enable_al_tx(pst_hal_device, pst_tx_dscr);
    }
}
#endif


/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_clear_tx_queue);
/*lint +e578*//*lint +e19*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

