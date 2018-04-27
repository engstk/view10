


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
#include "dmac_psm_ap.h"
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
#define THIS_FILE_ID OAM_FILE_ID_DMAC_P2P_C

/*****************************************************************************
  2 静态函数声明
*****************************************************************************/

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/

/*****************************************************************************
  4 函数实现
*****************************************************************************/

oal_void dmac_p2p_go_absence_period_start_sta_prot(dmac_vap_stru * pst_dmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    mac_sta_pm_handler_stru          *pst_mac_sta_pm_handle;

    oal_uint8   uc_tid = WLAN_TID_MAX_NUM;
    pst_mac_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;
    if (OAL_FALSE == pst_mac_sta_pm_handle->en_is_fsm_attached)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "{dmac_p2p_go_absence_period_start_sta_prot::pm fsm not attached.}");
        return;
    }

    /* If WMM-PS USP is in process, queue a QoS NULL trigger frame in the    */
    /* highest priority trigger enabled queue                                */
    if(dmac_is_uapsd_sp_not_in_progress(pst_mac_sta_pm_handle) == OAL_FALSE)
    {
        uc_tid = dmac_get_highest_trigger_enabled_priority(pst_dmac_vap);

        if(uc_tid != WLAN_TID_MAX_NUM)
        {
            dmac_send_null_frame_to_ap(pst_dmac_vap, STA_PWR_SAVE_STATE_DOZE, OAL_TRUE);
        }
    }
#endif
}

oal_uint32 dmac_p2p_noa_absent_start_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    dmac_vap_stru              *pst_dmac_vap = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{dmac_p2p_noa_absent_start_event::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_dmac_vap = mac_res_get_dmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_event->st_event_hdr.uc_vap_id, OAM_SF_P2P, "{dmac_p2p_noa_absent_start_event::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
    #ifdef _PRE_WLAN_FEATURE_STA_PM
        if(STA_PWR_SAVE_STATE_DOZE != dmac_psm_get_state(pst_dmac_vap))
        {
            /* Protocol dependent processing */
            dmac_p2p_go_absence_period_start_sta_prot(pst_dmac_vap);

            if(OAL_TRUE == dmac_is_ps_poll_rsp_pending(pst_dmac_vap))
            {
                if (OAL_SUCC != dmac_send_pspoll_to_ap(pst_dmac_vap))
                {
                    OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_p2p_noa_absent_start_event::dmac_send_pspoll_to_ap fail}");
                }

            }
        }
        if(IS_P2P_NOA_ENABLED(pst_dmac_vap))
        {
            dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_P2P_SLEEP, 0, OAL_PTR_NULL);
        }
    #endif
    }

    else
    {
        if(IS_P2P_NOA_ENABLED(pst_dmac_vap))
        {
            /* 暂停发送 */

            dmac_p2p_handle_ps(pst_dmac_vap, OAL_TRUE);
        }
    }
    return OAL_SUCC;
}


oal_uint8 dmac_p2p_listen_rx_mgmt(dmac_vap_stru   *pst_dmac_vap,
                                  oal_netbuf_stru *pst_netbuf)
{
    mac_ieee80211_frame_stru   *pst_frame_hdr;
    mac_device_stru            *pst_mac_device;
    oal_uint8                  *puc_frame_body;
    oal_uint16                  us_frame_len;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_netbuf)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{dmac_p2p_listen_rx_mgmt::param null.}");
        return OAL_FALSE;
    }

    pst_mac_device  = (mac_device_stru *)mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "{dmac_p2p_listen_rx_mgmt::pst_mac_device null.}");
        return OAL_FALSE;
    }

    /* 获取帧信息 */
    pst_frame_hdr  = (mac_ieee80211_frame_stru *)OAL_NETBUF_HEADER(pst_netbuf);
    puc_frame_body = OAL_NETBUF_PAYLOAD(pst_netbuf);
    us_frame_len   = (oal_uint16)oal_netbuf_get_len(pst_netbuf);

    if ((WLAN_ACTION == pst_frame_hdr->st_frame_control.bit_sub_type) &&(OAL_TRUE == mac_is_p2p_action_frame(puc_frame_body)))
    {
        /*判断是否是presence request action frame*/
        if(OAL_TRUE == dmac_is_p2p_presence_req_frame(puc_frame_body))
        {
            dmac_process_p2p_presence_req(pst_dmac_vap,pst_netbuf);
            return OAL_FALSE;
        }
        else
        {
            if(OAL_TRUE == dmac_is_p2p_go_neg_req_frame(puc_frame_body) || OAL_TRUE == dmac_is_p2p_pd_disc_req_frame(puc_frame_body))
            {
                /* 延长监听时间，由于监听共用扫描接口，故延长扫描定时器 */
                if (OAL_TRUE == pst_dmac_vap->pst_hal_device->st_hal_scan_params.st_scan_timer.en_is_enabled)
                {
                    FRW_TIMER_STOP_TIMER(&(pst_dmac_vap->pst_hal_device->st_hal_scan_params.st_scan_timer));
                    FRW_TIMER_RESTART_TIMER(&(pst_dmac_vap->pst_hal_device->st_hal_scan_params.st_scan_timer),
                                              (pst_dmac_vap->pst_hal_device->st_hal_scan_params.us_scan_time), OAL_FALSE);
                }
            }

            /* 如果是ACTION 帧，则上报 */
            return OAL_TRUE;
        }
    }
    else if (WLAN_PROBE_REQ== pst_frame_hdr->st_frame_control.bit_sub_type)
    {
        if (OAL_SUCC != dmac_p2p_listen_filter_vap(pst_dmac_vap))
        {
            return OAL_FALSE;
        }
        if (OAL_SUCC != dmac_p2p_listen_filter_frame(pst_dmac_vap, puc_frame_body, us_frame_len))
        {
            return OAL_FALSE;
        }

        /* 接收到probe req 帧，返回probe response 帧 */
        dmac_ap_up_rx_probe_req(pst_dmac_vap, pst_netbuf,(oal_uint8*)OAL_PTR_NULL,0);
    }

    return OAL_TRUE;
}


oal_uint32  dmac_process_p2p_presence_req(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf)
{
    oal_netbuf_stru            *pst_mgmt_buf;
    oal_uint16                  us_mgmt_len;
    mac_tx_ctl_stru            *pst_tx_ctl;
    dmac_rx_ctl_stru           *pst_rx_ctl;
    mac_ieee80211_frame_stru   *pst_frame_hdr;
    oal_uint8                  *puc_frame_body;
    oal_uint32                  ul_ret;
    oal_uint16                  us_user_idx = 0;

    /* 获取帧头信息 */
    pst_rx_ctl    = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_frame_hdr = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(&(pst_rx_ctl->st_rx_info)));
    puc_frame_body = MAC_GET_RX_PAYLOAD_ADDR(&(pst_rx_ctl->st_rx_info), pst_netbuf);

    ul_ret = mac_vap_find_user_by_macaddr((&pst_dmac_vap->st_vap_base_info), pst_frame_hdr->auc_address2, &us_user_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "{dmac_process_p2p_presence_req::no user.}", ul_ret);
        return ul_ret;
    }

    /* 申请管理帧内存 */
    pst_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_mgmt_buf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "{dmac_process_p2p_presence_req::pst_mgmt_buf null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(pst_mgmt_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_mgmt_buf, OAL_PTR_NULL);

    OAL_MEM_NETBUF_TRACE(pst_mgmt_buf, OAL_TRUE);

    /* 封装presence request帧 */
    us_mgmt_len = dmac_mgmt_encap_p2p_presence_rsp(pst_dmac_vap, pst_mgmt_buf, pst_frame_hdr->auc_address2, puc_frame_body);
    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "{dmac_process_p2p_presence_req::dmac_mgmt_encap_p2p_presence_rsp. length=%d}\r\n",us_mgmt_len);

    /* 调用发送管理帧接口 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_mgmt_buf);


    OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl)   = us_user_idx;
    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "{dmac_process_p2p_presence_req::us_user_idx=%d}\r\n",us_user_idx);
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_MGMT;
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, us_mgmt_len);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "{dmac_process_p2p_presence_req::dmac_tx_mgmt failed[%d].", ul_ret);
        oal_netbuf_free(pst_mgmt_buf);
        return ul_ret;
    }

    return OAL_SUCC;
}

oal_void dmac_process_p2p_noa(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru            *pst_rx_ctrl;
    mac_rx_ctl_stru             *pst_rx_info;
    oal_uint16                   us_frame_len;
    oal_uint8                   *puc_noa_attr = OAL_PTR_NULL;
    oal_uint8                   *puc_payload;
    mac_cfg_p2p_ops_param_stru   st_p2p_ops;
    mac_cfg_p2p_noa_param_stru   st_p2p_noa;
    oal_uint16                   us_attr_index = 0;
    hal_to_dmac_vap_stru        *pst_hal_vap;
    oal_uint16                   us_attr_len = 0;
    oal_uint32                   ul_current_tsf_lo;

    if (OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_netbuf)
    {
        //OAM_INFO_LOG0(0, OAM_SF_P2P, "{dmac_process_p2p_noa::param null.}");
        return;
    }
    pst_rx_ctrl         = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_rx_info         = (mac_rx_ctl_stru *)(&(pst_rx_ctrl->st_rx_info));
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    us_frame_len     = pst_rx_info->us_frame_len - pst_rx_info->uc_mac_header_len; /*帧体长度*/
#else
    us_frame_len     = pst_rx_info->us_frame_len - pst_rx_info->uc_mac_header_len; /*帧体长度*/
#endif
    puc_payload         = OAL_NETBUF_PAYLOAD(pst_netbuf);

    OAL_MEMZERO(&st_p2p_ops, OAL_SIZEOF(st_p2p_ops));
    OAL_MEMZERO(&st_p2p_noa, OAL_SIZEOF(st_p2p_noa));
    /* 取得NoA attr*/
    puc_noa_attr = dmac_get_p2p_noa_attr(puc_payload, us_frame_len, MAC_DEVICE_BEACON_OFFSET, &us_attr_len);
    if (OAL_PTR_NULL == puc_noa_attr)
    {
        if(IS_P2P_NOA_ENABLED(pst_dmac_vap) || IS_P2P_OPPPS_ENABLED(pst_dmac_vap))
        {
            /* 停止节能 */
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_FALSE);
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P,
                            "{dmac_process_p2p_noa::puc_noa_attr null. stop p2p ps}");

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    #ifdef _PRE_WLAN_FEATURE_P2P_NOA_DSLEEP
    /*TODO:mpw2上03 p2p noa深睡暂不支持，待pilot上硬件更改触发睡眠中断的流程后再打开*/
            /* 去使能p2p noa唤醒中断 */
            hal_vap_set_ext_noa_disable(pst_dmac_vap->pst_hal_vap);
    #else
            /*停止p2p noa oppps 时恢复深睡 */
            PM_WLAN_EnableDeepSleep();
    #endif

#endif
        }
        else
        {
            return;
        }
    }
    else
    {
        /* 解析ops参数*/
        if((puc_noa_attr[us_attr_index] & BIT7) != 0)
        {
            st_p2p_ops.en_ops_ctrl = 1;
            st_p2p_ops.uc_ct_window = (puc_noa_attr[us_attr_index] & 0x7F);
        }

        if(us_attr_len > 2)
        {
            /* 解析NoA参数*/
            us_attr_index++;
            st_p2p_noa.uc_count = puc_noa_attr[us_attr_index++];
            st_p2p_noa.ul_duration = OAL_MAKE_WORD32(OAL_MAKE_WORD16(puc_noa_attr[us_attr_index],
                                             puc_noa_attr[us_attr_index + 1]),
                                             OAL_MAKE_WORD16(puc_noa_attr[us_attr_index + 2],
                                             puc_noa_attr[us_attr_index + 3]));
            us_attr_index += 4;
            st_p2p_noa.ul_interval = OAL_MAKE_WORD32(OAL_MAKE_WORD16(puc_noa_attr[us_attr_index],
                                             puc_noa_attr[us_attr_index + 1]),
                                             OAL_MAKE_WORD16(puc_noa_attr[us_attr_index + 2],
                                             puc_noa_attr[us_attr_index + 3]));
            us_attr_index += 4;
            st_p2p_noa.ul_start_time = OAL_MAKE_WORD32(OAL_MAKE_WORD16(puc_noa_attr[us_attr_index],
                                             puc_noa_attr[us_attr_index + 1]),
                                             OAL_MAKE_WORD16(puc_noa_attr[us_attr_index + 2],
                                             puc_noa_attr[us_attr_index + 3]));
        }
    }

    pst_hal_vap  = pst_dmac_vap->pst_hal_vap;
    /* 保存GO节能参数，设置P2P ops 寄存器 */
    if((pst_dmac_vap->st_p2p_ops_param.en_ops_ctrl != st_p2p_ops.en_ops_ctrl)||
        (pst_dmac_vap->st_p2p_ops_param.uc_ct_window != st_p2p_ops.uc_ct_window))
    {
        pst_dmac_vap->st_p2p_ops_param.en_ops_ctrl = st_p2p_ops.en_ops_ctrl;
        pst_dmac_vap->st_p2p_ops_param.uc_ct_window = st_p2p_ops.uc_ct_window;
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "dmac_process_p2p_noa:ctrl:%d, ct_window:%d\r\n",
                    pst_dmac_vap->st_p2p_ops_param.en_ops_ctrl,
                    pst_dmac_vap->st_p2p_ops_param.uc_ct_window);

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        /* 开启p2p oppps只能浅睡 */
        if (st_p2p_ops.en_ops_ctrl)
        {
            PM_WLAN_DisbaleDeepSleep();
        }
#endif
        /* 设置P2P ops 寄存器 */
        hal_vap_set_ops(pst_hal_vap, pst_dmac_vap->st_p2p_ops_param.en_ops_ctrl, pst_dmac_vap->st_p2p_ops_param.uc_ct_window);
    }

    /* 保存GO节能参数，设置P2P NoA 寄存器 */
    if((pst_dmac_vap->st_p2p_noa_param.uc_count != st_p2p_noa.uc_count)||
        (pst_dmac_vap->st_p2p_noa_param.ul_duration != st_p2p_noa.ul_duration)||
        (pst_dmac_vap->st_p2p_noa_param.ul_interval != st_p2p_noa.ul_interval) ||
        (pst_dmac_vap->st_p2p_noa_param.ul_start_time != st_p2p_noa.ul_start_time))
    {
        pst_dmac_vap->st_p2p_noa_param.uc_count = st_p2p_noa.uc_count;
        pst_dmac_vap->st_p2p_noa_param.ul_duration = st_p2p_noa.ul_duration;
        pst_dmac_vap->st_p2p_noa_param.ul_interval = st_p2p_noa.ul_interval;
        pst_dmac_vap->st_p2p_noa_param.ul_start_time = st_p2p_noa.ul_start_time;
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "dmac_process_p2p_noa:start_time:%u, duration:%d, interval:%d, count:%d\r\n",
                    pst_dmac_vap->st_p2p_noa_param.ul_start_time,
                    pst_dmac_vap->st_p2p_noa_param.ul_duration,
                    pst_dmac_vap->st_p2p_noa_param.ul_interval,
                    pst_dmac_vap->st_p2p_noa_param.uc_count);

        hal_vap_tsf_get_32bit(pst_hal_vap, &ul_current_tsf_lo);
        if((st_p2p_noa.ul_start_time < ul_current_tsf_lo)&&
          (st_p2p_noa.ul_start_time != 0))
        {
            OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "dmac_process_p2p_noa:start time %u is less than tsf time %u\r\n",
                    st_p2p_noa.ul_start_time,
                    ul_current_tsf_lo);
            st_p2p_noa.ul_start_time = ul_current_tsf_lo + (st_p2p_noa.ul_interval - (ul_current_tsf_lo - st_p2p_noa.ul_start_time)% st_p2p_noa.ul_interval);
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_P2P, "dmac_process_p2p_noa:start time update to %u\r\n",
                            st_p2p_noa.ul_start_time);

        }

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        if (st_p2p_noa.uc_count)
        {
    #ifdef _PRE_WLAN_FEATURE_P2P_NOA_DSLEEP
    /*TODO:mpw2上03 p2p noa深睡暂不支持，待pilot上硬件更改触发睡眠中断的流程后再打开*/
            /* 设置ext p2p noa参数*/
            hal_vap_set_ext_noa_para(pst_dmac_vap->pst_hal_vap, st_p2p_noa.ul_duration, st_p2p_noa.ul_interval);

            /* 设置ext p2p noa offset参数*/
            hal_vap_set_ext_noa_offset(pst_dmac_vap->pst_hal_vap,PM_DEFAULT_EXT_TBTT_OFFSET);

            /* 使能ext p2p noa唤醒中断 */
            hal_vap_set_ext_noa_enable(pst_dmac_vap->pst_hal_vap);

            /* 设置 p2p noa inner offset参数*/
            hal_vap_set_noa_offset(pst_dmac_vap->pst_hal_vap,PM_DEFAULT_STA_INTER_TBTT_OFFSET);

            /* 设置ext p2p noa bcn out参数*/
            hal_vap_set_noa_timeout_val(pst_dmac_vap->pst_hal_vap,pst_dmac_vap->us_beacon_timeout);
    #else
        /* 开启p2p NOA只能浅睡 */
            PM_WLAN_DisbaleDeepSleep();
    #endif

        }
#endif
        hal_vap_set_noa(pst_hal_vap,
                        st_p2p_noa.ul_start_time,
                        st_p2p_noa.ul_duration,
                        st_p2p_noa.ul_interval,
                        st_p2p_noa.uc_count);


    }
}

oal_void dmac_p2p_handle_ps(dmac_vap_stru * pst_dmac_vap, oal_bool_enum_uint8 en_pause)
{

#ifdef _PRE_WLAN_FEATURE_STA_PM

    mac_vap_stru               *pst_mac_vap = OAL_PTR_NULL;
    dmac_user_stru             *pst_dmac_user = OAL_PTR_NULL;
    mac_device_stru            *pst_mac_device;
    mac_sta_pm_handler_stru    *pst_mac_sta_pm_handle;
    oal_uint8                  uc_noa_not_sleep_flag;

    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{dmac_p2p_handle_ps::pst_dmac_vap null.}");
        return;
    }
    pst_mac_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;

    pst_mac_vap = &pst_dmac_vap->st_vap_base_info;

    pst_mac_device  = (mac_device_stru *)mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{dmac_p2p_handle_ps::pst_mac_device is null.}");
        return;
    }

     /* dbac在运行,直接return */
    if ((OAL_TRUE == mac_is_dbac_running(pst_mac_device)))
    {
        return;
    }

    if(pst_mac_device->st_p2p_info.en_p2p_ps_pause == en_pause)
    {
        return;
    }

    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_mac_vap->us_assoc_vap_id);
        if (OAL_PTR_NULL == pst_dmac_user)
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_P2P,
                "{dmac_p2p_handle_ps::pst_dmac_user[%d] null.}", pst_mac_vap->us_assoc_vap_id);
            return;
        }

        /* P2P CLIENT 芯片节能针对P2P CLIENT 注册，而不能对P2P DEVICE 注册 */
        if(OAL_TRUE == en_pause)
        {
            dmac_user_pause(pst_dmac_user);

#ifdef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
            /*suspend硬件队列*/
            hal_set_machw_tx_suspend(pst_dmac_vap->pst_hal_device);
            /* 遍历硬件队列，将属于该用户的帧都放回tid */
            dmac_psm_flush_txq_to_tid(pst_mac_device, pst_dmac_vap, pst_dmac_user);
            /* 恢复硬件队列 */
            hal_set_machw_tx_resume(pst_dmac_vap->pst_hal_device);
#else
            /* 告知MAC用户进入节能模式 */
            hal_tx_enable_peer_sta_ps_ctrl(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif
            pst_mac_device->uc_mac_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;  /* 记录睡眠时vap id */

            /* 不满足睡眠条件,此次noa不浅睡,不关闭前端 */
            uc_noa_not_sleep_flag = (pst_mac_sta_pm_handle->en_beacon_frame_wait) | (pst_mac_sta_pm_handle->st_null_wait.en_doze_null_wait << 1) | (pst_mac_sta_pm_handle->en_more_data_expected << 2)
                    | (pst_mac_sta_pm_handle->st_null_wait.en_active_null_wait << 3) | (pst_mac_sta_pm_handle->en_direct_change_to_active << 4);

            if (uc_noa_not_sleep_flag == 0)
            {
                hal_device_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), HAL_DEVICE_EVENT_VAP_CHANGE_TO_DOZE, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
                pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_PSM_P2P_SLEEP]++;
            }
        }
        else
        {
            hal_device_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), HAL_DEVICE_EVENT_VAP_CHANGE_TO_ACTIVE, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
            pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_PSM_P2P_AWAKE]++;

#ifndef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
            /* 恢复该用户的硬件队列的发送 */
            hal_tx_disable_peer_sta_ps_ctrl(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif
            dmac_user_resume(pst_dmac_user);   /* 恢复user */
            /* 将所有的缓存帧发送出去 */
            dmac_psm_queue_flush(pst_dmac_vap, pst_dmac_user);

            dmac_p2p_resume_send_null_to_ap(pst_dmac_vap,pst_mac_sta_pm_handle);
        }

    }
    else if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {   //P2P GO 芯片节能需要通过命令配置
        if (OAL_TRUE == en_pause)
        {
            dmac_ap_pause_all_user(pst_mac_vap);

            pst_mac_device->uc_mac_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;  /* 记录睡眠时vap id */

            //PM_WLAN_PsmHandle(pst_dmac_vap->pst_hal_vap->uc_service_id, PM_WLAN_LIGHTSLEEP_PROCESS);/* 投票休眠 */
        }
        else
        {
            //PM_WLAN_PsmHandle(pst_dmac_vap->pst_hal_vap->uc_service_id, PM_WLAN_WORK_PROCESS);/* 投票唤醒 */

            dmac_ap_resume_all_user(pst_mac_vap);
        }
    }

    /*记录目前p2p节能状态*/
    pst_mac_device->st_p2p_info.en_p2p_ps_pause = en_pause;
#endif
}


oal_uint32 dmac_p2p_noa_absent_end_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    mac_device_stru            *pst_mac_device;
    mac_vap_stru               *pst_mac_vap = OAL_PTR_NULL;
    dmac_vap_stru              *pst_dmac_vap;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{dmac_p2p_noa_absent_end_event::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{dmac_p2p_noa_absent_end_event::pst_mac_device is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = mac_res_get_mac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{dmac_p2p_noa_absent_end_event::mac_res_get_mac_vap fail.vap_id = %u}",pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{dmac_p2p_noa_absent_end_event::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    //判断寄存器状态noa count是否结束，0:结束 1: 工作中; 更新P2P IE
    if(((hal_p2p_pm_event_stru *)(pst_event->auc_event_data))->p2p_noa_status == OAL_FALSE)
    {
        OAM_WARNING_LOG0(0, OAM_SF_P2P, "{dmac_p2p_noa_absent_end_event::p2p NoA count expired}");
        OAL_MEMZERO(&(pst_dmac_vap->st_p2p_noa_param), OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));
    }

    //OAL_IO_PRINT("dmac_p2p_handle_ps_cl:resume\r\n");
    /* 恢复发送 */
    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
    #ifdef _PRE_WLAN_FEATURE_STA_PM
        dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_P2P_AWAKE, 0, OAL_PTR_NULL);
    #endif
    }
    else
    {
        dmac_p2p_handle_ps(pst_dmac_vap, OAL_FALSE);
    }

    return OAL_SUCC;
}

oal_uint32 dmac_p2p_oppps_ctwindow_end_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    mac_device_stru            *pst_mac_device;
    mac_vap_stru               *pst_mac_vap = OAL_PTR_NULL;
    dmac_vap_stru              *pst_dmac_vap;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{dmac_p2p_ctwindow_end_event::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{dmac_p2p_ctwindow_end_event::pst_mac_device is null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = mac_res_get_mac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG1(0, OAM_SF_P2P, "{dmac_p2p_ctwindow_end_event::mac_res_get_mac_vap fail.vap_id = %u}",pst_event->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_P2P, "{dmac_p2p_ctwindow_end_event::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        /* 记录暂停的信道，tbtt中断后切回 */
        pst_dmac_vap->pst_hal_device->st_hal_scan_params.st_home_channel = pst_mac_vap->st_channel;
    #ifdef _PRE_WLAN_FEATURE_STA_PM
        dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_P2P_SLEEP, 0, OAL_PTR_NULL);
    #endif
    }
    else
    {
        if(OAL_FALSE == pst_dmac_vap->st_p2p_ops_param.en_pause_ops)
        {
            /* 记录暂停的信道，tbtt中断后切回 */
            pst_dmac_vap->pst_hal_device->st_hal_scan_params.st_home_channel = pst_mac_vap->st_channel;
#ifdef _PRE_WLAN_FEATURE_STA_PM
            /* 暂停发送 */
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_TRUE);
#endif

        }
    }
    return OAL_SUCC;
}

oal_void dmac_p2p_oppps_ctwindow_start_event(dmac_vap_stru * pst_dmac_vap)
{
#ifdef _PRE_WLAN_FEATURE_STA_PM
    oal_uint32 ui_retval = 0;
#endif

    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
#ifdef _PRE_WLAN_FEATURE_STA_PM
        ui_retval = dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_P2P_AWAKE, 0, OAL_PTR_NULL);
        if(OAL_SUCC != ui_retval)
        {
            OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_p2p_oppps_ctwindow_start_event::[%d]}", ui_retval);
        }
#endif
    }
    else
    {
        /* 恢复发送 */
        dmac_p2p_handle_ps(pst_dmac_vap, OAL_FALSE);
    }
}


oal_void dmac_p2p_reset_ps_status_for_dbac(
                                mac_device_stru  *pst_device,
                                mac_vap_stru     *pst_led_vap,
                                mac_vap_stru     *pst_flw_vap)
{
    dmac_vap_stru *pst_dmac_p2p_vap;
    mac_vap_stru  *pst_p2p_vap;

#ifdef _PRE_WLAN_FEATURE_STA_PM
    mac_cfg_ps_open_stru  st_ps_open;

    st_ps_open.uc_pm_enable      = MAC_STA_PM_SWITCH_OFF;
    st_ps_open.uc_pm_ctrl_type   = MAC_STA_PM_CTRL_TYPE_DBAC;

	/* 开启dbac前关闭低功耗 */
    if (WLAN_VAP_MODE_BSS_STA == pst_led_vap->en_vap_mode)
    {
        dmac_config_set_sta_pm_on(pst_led_vap, OAL_SIZEOF(mac_cfg_ps_open_stru), (oal_uint8 *)&st_ps_open);
    }

    if(WLAN_VAP_MODE_BSS_STA == pst_flw_vap->en_vap_mode)
    {
        dmac_config_set_sta_pm_on(pst_flw_vap, OAL_SIZEOF(mac_cfg_ps_open_stru), (oal_uint8 *)&st_ps_open);
    }
#endif

    /* 开dbac前p2p已经pause过tid了,需要恢复 */
    if (OAL_TRUE == pst_device->st_p2p_info.en_p2p_ps_pause)
    {
        if (WLAN_LEGACY_VAP_MODE != pst_led_vap->en_p2p_mode)
        {
            pst_p2p_vap = pst_led_vap;
        }
        else
        {
            pst_p2p_vap = pst_flw_vap;
        }

        pst_dmac_p2p_vap = mac_res_get_dmac_vap(pst_p2p_vap->uc_vap_id);
        dmac_p2p_handle_ps(pst_dmac_p2p_vap, OAL_FALSE);
    }
}
#ifdef _PRE_WLAN_FEATURE_STA_PM

oal_void dmac_p2p_resume_send_null_to_ap(dmac_vap_stru *pst_dmac_vap,mac_sta_pm_handler_stru *pst_mac_sta_pm_handle)
{
    oal_uint8       uc_power_mgmt = 0xff;

    /* 不需要重传 */
    if ((0 == pst_mac_sta_pm_handle->en_ps_back_active_pause) && (0 == pst_mac_sta_pm_handle->en_ps_back_doze_pause))
    {
        return;
    }

    /* 根据状态机状态发送相应null帧 */
    if (STA_PWR_SAVE_STATE_ACTIVE == GET_PM_STATE(pst_mac_sta_pm_handle))
    {
        if (OAL_TRUE == pst_mac_sta_pm_handle->en_ps_back_doze_pause)
        {
            uc_power_mgmt = 1;
        }
    }
    else
    {
        if (OAL_TRUE == pst_mac_sta_pm_handle->en_ps_back_active_pause)
        {
            if (STA_PWR_SAVE_STATE_DOZE == GET_PM_STATE(pst_mac_sta_pm_handle))
            {
                dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_TX_DATA, 0, OAL_PTR_NULL);
            }
            uc_power_mgmt = 0;
        }
    }

    if ((0 == uc_power_mgmt) || (1 == uc_power_mgmt))
    {
        if (OAL_SUCC != dmac_psm_process_fast_ps_state_change(pst_dmac_vap, uc_power_mgmt))
        {
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,"dmac_p2p_resume_send_null_to_ap::send[%d]null fail",uc_power_mgmt);

            /* 睡眠的null帧发送失败需要重启定时器等待超时发送,唤醒的null帧发送失败,等待下次发送或者beacon再告知缓存 */
            if (uc_power_mgmt)
            {
                dmac_psm_start_activity_timer(pst_dmac_vap,pst_mac_sta_pm_handle);
            }
        }
    }
}
#endif

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif



