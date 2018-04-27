


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_sdio.h"
#include "oal_mem.h"
#include "oal_types.h"
#include "oal_workqueue.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "oal_hcc_slave_if.h"
#endif
#ifdef _PRE_WLAN_PROFLING_MIPS
#include "oal_profiling.h"
#endif
#if (defined(_PRE_PLAT_FEATURE_CUSTOMIZE) && defined(_PRE_PRODUCT_ID_HI110X_DEV))
#include "hal_mac.h"
#endif
#ifdef _PRE_PRODUCT_ID_HI110X_DEV
#include "hal_device.h"
#endif
#ifdef _PRE_WLAN_ALG_ENABLE
#include "alg_ext_if.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
//#include "alg_tpc.h"
#endif
#endif

#include "mac_data.h"
#include "mac_device.h"
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_vap.h"
#include "dmac_ext_if.h"
#include "dmac_main.h"
#include "dmac_vap.h"
#include "dmac_psm_ap.h"
#include "dmac_rx_data.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_mgmt_sta.h"
#include "dmac_mgmt_ap.h"
#include "dmac_tx_complete.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_uapsd.h"
#include "dmac_blockack.h"
#include "dmac_beacon.h"
#include "dmac_user.h"
#include "dmac_11i.h"
#include "dmac_wep.h"
#include "dmac_uapsd.h"
#ifdef _PRE_WLAN_FEATURE_AP_PM
#include "dmac_ap_pm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "dmac_data_acq.h"
#endif
#ifdef _PRE_DEBUG_MODE_USER_TRACK
#include "dmac_user_track.h"
#endif
#include "dmac_mgmt_bss_comm.h"
#include "dmac_txopps.h"
#include "dmac_dft.h"
#include "dmac_reset.h"
#include "dmac_config.h"
#include "dmac_beacon.h"
#include "dmac_scan.h"
#include "dmac_psm_ap.h"
#include "dmac_device.h"
#include "dmac_resource.h"

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
#include "cali_data.h"
#include "pm_extern.h"
#endif
#include "dmac_test_main.h"
#include "dmac_frame_filter_test.h"

#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_lpm_test.h"
#include "dmac_wmm_test.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
//#ifdef _PRE_WLAN_PRODUCT_1151V200
//#include "hal_witp_phy_reg_1151v2.h"
//#else
//#include "hal_witp_phy_reg.h"
//#endif
//#include "hal_witp_pa_reg.h"
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "hal_phy_reg.h"
#endif
#endif

#ifdef _PRE_WIFI_DMT
#include "hal_witp_dmt_if.h"
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#include "pm_extern.h"
#include "dmac_sta_pm.h"
#endif

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

#include "dmac_11w.h"
#endif
#include "dmac_chan_mgmt.h"

#include "dmac_reset.h"
#include "oal_net.h"
#include "dmac_config.h"
#include "dmac_main.h"
#include "dmac_rx_filter.h"

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "core_cr4.h"
#endif
#include "oal_profiling.h"

#ifdef _PRE_SUPPORT_ACS
#include "dmac_acs.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
#include "mac_board.h"
#endif

#include "dmac_arp_offload.h"

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
#include "dmac_auto_adjust_freq.h"
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "dmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "dmac_smps.h"
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#include "dmac_bsd.h"
#endif

#ifdef _PRE_WLAN_FEATURE_11V
#include "dmac_11v.h"
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
#include "dmac_ftm.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CONFIG_DEBUG_C

#if defined(_PRE_WLAN_FEATURE_MEMORY_USAGE_TRACE) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifndef _PRE_WLAN_FEATURE_MU_TRAFFIC_CTL
#error "mu-traffic-ctl should be defined when using netbuff memory trace."
#endif
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
OAL_STATIC dmac_reset_para_stru                g_st_reset_param;
#endif
#ifdef _PRE_FEATURE_FAST_AGING
extern oal_uint32 dmac_fast_aging_timeout(oal_void *pst_void_dmac_dev);
#endif

#ifdef _PRE_WLAN_CFGID_DEBUG
mac_phy_debug_switch_stru  g_st_mac_phy_debug_switch = {0};
#ifdef _PRE_WLAN_DFT_STAT
#ifdef _PRE_WLAN_FEATURE_DFR
#ifdef _PRE_DEBUG_MODE

OAL_STATIC oal_uint32  dmac_config_dfr_enable(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dfr_enable::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device->en_dfr_enable = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_config_dfr_enable enter, dfr_enable = %d\r\n", pst_hal_device->en_dfr_enable);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_trig_loss_tx_comp(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru             *pst_dev;
    hal_to_dmac_device_stru        *pst_hal_device;


    pst_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dev)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "dmac_config_trig_loss_tx_comp: mac_res_get_dev null: uc_device_id = %d", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_trig_loss_tx_comp::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device->ul_cfg_loss_tx_comp_cnt = *((oal_uint32 *)puc_param);

    OAL_IO_PRINT("dmac_config_trig_loss_tx_comp enter, cnt = %d\r\n", pst_hal_device->ul_cfg_loss_tx_comp_cnt);
    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_config_trig_loss_tx_comp enter, cnt = %d\r\n", pst_hal_device->ul_cfg_loss_tx_comp_cnt);

    return OAL_SUCC;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC oal_uint32  dmac_config_trig_pcie_reset(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    OAM_WARNING_LOG0(0, OAM_SF_CFG, "dmac_config_trig_pcie_reset succ");
    OAL_IO_PRINT("dmac_config_trig_pcie_reset succ\r\n");
    return  oal_pci_hand_reset(pst_mac_vap->uc_chip_id);
}
#endif
#endif
#endif
#endif

oal_uint32  dmac_config_set_thruput_bypass(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_set_thruput_bypass_stru *pst_set_thruput_bypass = (mac_cfg_set_thruput_bypass_stru *)puc_param;

    OAL_SET_THRUPUT_BYPASS_ENABLE(pst_set_thruput_bypass->uc_bypass_type, pst_set_thruput_bypass->uc_value);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_config_send_frame_timer(void *p_arg)
{
    dmac_vap_stru                   *pst_dmac_vap;
    oal_netbuf_stru                 *pst_netbuf;
    mac_tx_ctl_stru                 *pst_tx_ctl;
    oal_uint32                       ul_ret;
    mac_vap_stru                    *pst_mac_vap;
    mac_device_stru                 *pst_mac_device;
    dmac_test_encap_frame            pst_test_encap_frame_fun;
    dmac_user_stru                  *pst_dmac_user     = OAL_PTR_NULL;
    mac_cfg_send_frame_param_stru   *pst_param         = (mac_cfg_send_frame_param_stru  *)p_arg;
    oal_uint32                       ul_len            = 0;
    oal_uint16                       us_assoc_id       = 0;
    oal_uint32                       ul_frame_len      = 0;

    pst_mac_vap   = (mac_vap_stru *)mac_res_get_mac_vap(pst_param->uc_vap_idx);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_config_send_frame_timer::mac_res_get_mac_vap fail}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(OAL_FALSE == oal_is_broadcast_ether_addr(pst_param->auc_mac_ra))
    {
        pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_param->auc_mac_ra);
        if (OAL_PTR_NULL == pst_dmac_user)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                           "{dmac_config_send_frame_timer::cannot find user by addr!.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
        if (OAL_PTR_NULL == pst_dmac_vap)
        {
            OAM_ERROR_LOG1(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_send_frame_timer::cannot find vap by user[user_id is %d]!}",
                                    pst_dmac_user->st_user_base_info.uc_vap_id);

            return OAL_ERR_CODE_PTR_NULL;
        }
        us_assoc_id = pst_dmac_user->st_user_base_info.us_assoc_id;
    }
    else
    {
        pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
        if (OAL_PTR_NULL == pst_dmac_vap)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_frame_timer::cannot find dmac vap }");
            return OAL_ERR_CODE_PTR_NULL;
        }
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_config_send_frame_timer::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 发送帧计数 */
    pst_param->uc_frame_cnt++;

    /* 申请帧内存 */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_frame_timer::cannot alloc netbuff in size[%d].}", WLAN_LARGE_NETBUF_SIZE);

        return OAL_ERR_CODE_PTR_NULL;
    }
     /* 初始化前后指针为NULL */
    oal_set_netbuf_prev(pst_netbuf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_netbuf, OAL_PTR_NULL);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    OAL_MEMZERO(pst_tx_ctl, OAL_NETBUF_CB_SIZE());
    MAC_GET_CB_EVENT_TYPE(pst_tx_ctl) = FRW_EVENT_TYPE_HOST_CRX;
    if(OAL_FALSE == oal_is_broadcast_ether_addr(pst_param->auc_mac_ra))
    {
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = us_assoc_id;
    }
    else
    {
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) = pst_dmac_vap->st_vap_base_info.us_multi_user_idx; /* channel switch帧是广播帧 */
        MAC_GET_CB_IS_MCAST(pst_tx_ctl)    = OAL_TRUE;
    }
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl)  = WLAN_WME_AC_MGMT;
    MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_ctl) = WLAN_CONTROL;

    /* 组帧 */
    pst_test_encap_frame_fun = dmac_test_get_encap_func(pst_param->en_frame_type);
    if(OAL_PTR_NULL == pst_test_encap_frame_fun)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_frame_timer::pst_test_encap_frame_fun null.}");
        oal_netbuf_free(pst_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(MAC_TEST_MGMT_ACTION == pst_param->en_frame_type)
    {
        ul_frame_len = pst_test_encap_frame_fun(pst_mac_vap,(oal_uint8 *)(pst_netbuf),p_arg, ul_len);
    }
    else
    {
        ul_frame_len = pst_test_encap_frame_fun(pst_mac_vap,
                                            (oal_uint8 *)OAL_NETBUF_HEADER(pst_netbuf),
                                            p_arg, ul_len);
    }
    if (0 == ul_frame_len)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_frame_timer::ul_frame_len=0.}");
        oal_netbuf_free(pst_netbuf);

        return OAL_FAIL;
    }

    /* 调用发送管理帧接口 */
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_netbuf, (oal_uint16)ul_frame_len);
    if (OAL_SUCC != ul_ret)
    {
        oal_netbuf_free(pst_netbuf);
        return ul_ret;
    }

    if(pst_param->uc_frame_cnt < pst_param->uc_pkt_num)
    {
        FRW_TIMER_CREATE_TIMER(&(pst_mac_device->st_send_frame),
                               dmac_config_send_frame_timer,
                               DMAC_TEST_SEND_FRAME_TIME ,  /* 10ms 触发一次 */
                               pst_param,
                               OAL_FALSE,
                               OAM_MODULE_ID_DMAC,
                               pst_mac_device->ul_core_id);
    }
    else
    {
        OAL_MEM_FREE(p_arg, OAL_TRUE);
    }

    return OAL_SUCC;
}


oal_uint32 dmac_config_send_frame(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_send_frame_param_stru   *pst_param;
    mac_device_stru                 *pst_mac_device;

    pst_param = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_cfg_send_frame_param_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_config_send_frame::melloc err!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_memcopy(pst_param , puc_param, OAL_SIZEOF(mac_cfg_send_frame_param_stru));
    pst_param->uc_vap_idx = pst_mac_vap->uc_vap_id;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_config_send_frame::frame_types = %d send_times = %d body_len=%d.}",
        pst_param->en_frame_type,pst_param->uc_pkt_num,pst_param->uc_frame_body_length);

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)  || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV))
    if(MAC_TEST_MGMT_BEACON_INCLUDE_IE == pst_param->en_frame_type)
    {
        if((pst_param->uc_frame_body_length > 0) && (pst_param->uc_frame_body_length < MAC_TEST_INCLUDE_FRAME_BODY_LEN) && (pst_param->uc_pkt_num > 0))
        {
            pst_mac_vap->st_ap_beacon_test_ie.en_include_test_ie = OAL_TRUE;
            pst_mac_vap->st_ap_beacon_test_ie.uc_include_times = pst_param->uc_pkt_num;
            pst_mac_vap->st_ap_beacon_test_ie.uc_test_ie_len = pst_param->uc_frame_body_length;
            oal_memcopy(pst_mac_vap->st_ap_beacon_test_ie.uc_test_ie_info, pst_param->uc_frame_body,pst_param->uc_frame_body_length);
        }
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_config_send_frame::en_include_test_ie=%d length = %d times = %d.}",
            pst_mac_vap->st_ap_beacon_test_ie.en_include_test_ie,pst_mac_vap->st_ap_beacon_test_ie.uc_test_ie_len,pst_mac_vap->st_ap_beacon_test_ie.uc_include_times);
        OAL_MEM_FREE(pst_param, OAL_TRUE);
        return OAL_SUCC;
    }
#endif
    if(pst_param->uc_pkt_num > 0)
    {
        FRW_TIMER_CREATE_TIMER(&(pst_mac_device->st_send_frame),
                               dmac_config_send_frame_timer,
                               DMAC_TEST_SEND_FRAME_TIME ,   /* 10ms 触发一次 */
                               pst_param,
                               OAL_FALSE,
                               OAM_MODULE_ID_DMAC,
                               pst_mac_device->ul_core_id);
    }
    else
    {
        OAL_MEM_FREE(pst_param, OAL_TRUE);
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_CHIP_TEST

OAL_STATIC oal_uint32 dmac_config_beacon_offload_test(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_sta_beacon_offload_test(pst_mac_vap, puc_param);
    return OAL_SUCC;
}
#endif

OAL_STATIC oal_uint32 dmac_config_protocol_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                            ul_ret;
    mac_protocol_debug_switch_stru       *pst_protocol_debug;
    dmac_vap_stru                        *pst_dmac_vap;
    dmac_set_chan_stru                    st_set_chan = {{0}};
    wlan_channel_bandwidth_enum_uint8     en_new_40M_bandwidth;
    oal_bool_enum_uint8                   en_bandwidth_change = OAL_FALSE;

    pst_protocol_debug = (mac_protocol_debug_switch_stru *)puc_param;
    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);


    /* 改变带宽命令*/
    if(pst_protocol_debug->ul_cmd_bit_map & BIT0)
    {
        mac_mib_set_2040SwitchProhibited(pst_mac_vap, OAL_FALSE);

        en_new_40M_bandwidth = pst_protocol_debug->en_band_force_switch_bit0;

        if((WLAN_BAND_WIDTH_40PLUS == en_new_40M_bandwidth)
            ||(WLAN_BAND_WIDTH_40MINUS == en_new_40M_bandwidth))
        {
            dmac_chan_prepare_for_40M_recovery(pst_dmac_vap, en_new_40M_bandwidth);
            dmac_chan_multi_select_channel_mac(pst_mac_vap, pst_mac_vap->st_channel.uc_chan_number, en_new_40M_bandwidth);

            mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_TRUE);
            en_bandwidth_change = OAL_TRUE;
        }
        else if(WLAN_BAND_WIDTH_20M == en_new_40M_bandwidth)
        {
            if((WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth)
                ||(WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth))
            {
                dmac_chan_multi_switch_to_20MHz_ap(pst_dmac_vap);
                dmac_chan_start_40M_recovery_timer(pst_dmac_vap);

                mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_FALSE);
                en_bandwidth_change = OAL_TRUE;
            }
            else
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                "{dmac_config_protocol_debug_switch::en_bandwidth is changing to 40M, but it is not 20M now.so do nothing!}");
                en_bandwidth_change = OAL_FALSE;
            }
        }
        else
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040,
            "{dmac_config_protocol_debug_switch::en_bandwidth is changing to %d.so do nothing!}", en_new_40M_bandwidth);
            en_bandwidth_change = OAL_FALSE;
        }

        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_2040,
                        "{dmac_config_protocol_debug_switch::uc_chan_number [%d], en_bandwidth [%d], en_new_40M_bandwidth [%d].}",
                        pst_mac_vap->st_channel.uc_chan_number, pst_mac_vap->st_channel.en_bandwidth, en_new_40M_bandwidth);

        if(OAL_TRUE == en_bandwidth_change)
        {
            ul_ret = dmac_send_notify_chan_width(pst_mac_vap, BROADCAST_MACADDR);
            if (OAL_SUCC != ul_ret)
            {
                OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{dmac_config_protocol_debug_switch::dmac_send_notify_chan_width return %d.}", ul_ret);
                return ul_ret;
            }

            ul_ret = dmac_chan_sync_event(pst_mac_vap, &st_set_chan);
            if (OAL_SUCC != ul_ret)
            {
                OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{dmac_config_protocol_debug_switch::dmac_chan_sync_event return %d.}", ul_ret);
                return ul_ret;
            }

            /* 同步mib项到host */
            dmac_config_d2h_vap_mib_syn(pst_mac_vap);

            /*改变带宽禁止开关*/
            //mac_mib_set_2040SwitchProhibited(pst_mac_vap, OAL_TRUE);
        }
    }
    /* 不允许20/40带宽切换开关*/
    if(pst_protocol_debug->ul_cmd_bit_map & BIT1)
    {
        if(OAL_TRUE == pst_protocol_debug->en_2040_ch_swt_prohi_bit1)
        {
            mac_mib_set_2040SwitchProhibited(pst_mac_vap, OAL_TRUE);
        }
        else
        {
            mac_mib_set_2040SwitchProhibited(pst_mac_vap, OAL_FALSE);
        }
    }
    /* 不容忍40M带宽开关*/
    if(pst_protocol_debug->ul_cmd_bit_map & BIT2)
    {
        if(WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)
        {
            if(OAL_TRUE == pst_protocol_debug->en_40_intolerant_bit2)
            {
                mac_mib_set_FortyMHzIntolerant(pst_mac_vap, OAL_TRUE);

                /*切20M*/
                if((OAL_FALSE == mac_mib_get_2040SwitchProhibited(pst_mac_vap)))
                {
                    if((WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth)
                            ||(WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth))
                    {
                        dmac_chan_multi_switch_to_20MHz_ap(pst_dmac_vap);
                        dmac_chan_start_40M_recovery_timer(pst_dmac_vap);
                    }
                }
            }
            else
            {
                mac_mib_set_FortyMHzIntolerant(pst_mac_vap, OAL_FALSE);
            }
        }
        else
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_config_protocol_debug_switch::band is not 2G,but [%d].", pst_dmac_vap->st_vap_base_info.st_channel.en_band);
        }
    }
    /*csa cmd */
    if(pst_protocol_debug->ul_cmd_bit_map & BIT3)
    {
        OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_config_protocol_debug_switch::csa_mode=[%d],csa_channel=[%d],csa_cnt=[%d],debuf_flag=[%d].",
            pst_protocol_debug->st_csa_debug_bit3.en_mode,pst_protocol_debug->st_csa_debug_bit3.uc_channel,
            pst_protocol_debug->st_csa_debug_bit3.uc_cnt,pst_protocol_debug->st_csa_debug_bit3.en_debug_flag);
        if(MAC_CSA_FLAG_CANCLE_DEBUG ==  pst_protocol_debug->st_csa_debug_bit3.en_debug_flag)
        {
            pst_mac_vap->st_ch_switch_info.en_csa_present_in_bcn       = OAL_FALSE;
            pst_mac_vap->st_ch_switch_info.en_ch_switch_status         = WLAN_CH_SWITCH_DONE;
        }
        else if(MAC_CSA_FLAG_START_DEBUG ==  pst_protocol_debug->st_csa_debug_bit3.en_debug_flag)
        {
            pst_mac_vap->st_ch_switch_info.uc_announced_channel        = pst_protocol_debug->st_csa_debug_bit3.uc_channel;
            pst_mac_vap->st_ch_switch_info.en_csa_mode                 = pst_protocol_debug->st_csa_debug_bit3.en_mode;
            pst_mac_vap->st_ch_switch_info.uc_ch_switch_cnt            = pst_protocol_debug->st_csa_debug_bit3.uc_cnt;
            pst_mac_vap->st_ch_switch_info.en_csa_present_in_bcn       = OAL_TRUE;
            pst_mac_vap->st_ch_switch_info.en_ch_switch_status         = WLAN_CH_SWITCH_DONE;
        }
        else
        {
            return OAL_SUCC;
        }
        dmac_encap_beacon(pst_dmac_vap, pst_dmac_vap->pauc_beacon_buffer[pst_dmac_vap->uc_beacon_idx], &(pst_dmac_vap->us_beacon_len));
    }
#ifdef _PRE_WLAN_FEATURE_HWBW_20_40
    if(pst_protocol_debug->ul_cmd_bit_map & BIT4)
    {
        if(OAL_TRUE == pst_protocol_debug->en_2040_user_switch_bit4)
        {
            dmac_acs_2040_user_set_switch(OAL_TRUE, pst_mac_vap);
        }
        else
        {
            dmac_acs_2040_user_set_switch(OAL_FALSE, pst_mac_vap);
        }
    }
#endif
    /*lsigtxop使能*/
    if(pst_protocol_debug->ul_cmd_bit_map & BIT5)
    {
        mac_mib_set_LsigTxopFullProtectionActivated(pst_mac_vap, pst_protocol_debug->en_lsigtxop_bit5);
    }

    return OAL_SUCC;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

oal_void dmac_config_reset_debug_all_work(oal_work_stru *pst_work)
{
    mac_vap_stru        *pst_mac_vap;
    mac_device_stru     *pst_mac_device;
    dmac_device_stru    *pst_dmac_device;

    pst_mac_vap = OAL_CONTAINER_OF(pst_work, mac_vap_stru, debug_reg_work);
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_reset_debug_all_work::pst_mac_device is null}");
        return;
    }
    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if(OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_reset_debug_all_work::pst_dmac_device is null}");
        return;
    }
    dmac_reset_debug_all(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device),&g_st_reset_param);

}
#endif


/*lint -save -e438 */
OAL_STATIC oal_uint32  dmac_config_reset_hw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifndef _PRE_WLAN_PROFLING_MIPS

    mac_device_stru                *pst_device;
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint32                      ul_off_set = 0;
    oal_int8                        ac_name[DMAC_HIPRIV_CMD_NAME_MAX_LEN];
    dmac_reset_para_stru            st_reset_param;

    /* 复位硬件phy&mac: hipriv "Hisilicon0 reset_hw 0|1|2|3(all|phy|mac|debug) 0|1(reset phy reg) 0|1(reset mac reg)"*/
    /* 命令复用来debug显示lut表和寄存器:hipriv "Hisilicon0 reset_hw 3(debug) 0|1|2(all|mac reg|phy reg|lut) "*/

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if(OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_config_reset_hw::pst_device is null,id=%d}", pst_mac_vap->uc_device_id);
        return OAL_FAIL;
    }
    OAL_MEMZERO(&st_reset_param, OAL_SIZEOF(dmac_reset_para_stru));
    OAL_MEMZERO(ac_name, DMAC_HIPRIV_CMD_NAME_MAX_LEN);
    /* 获取复位类型*/
    dmac_get_cmd_one_arg((oal_int8*)puc_param, ac_name, &ul_off_set);
    st_reset_param.uc_reset_type = (oal_uint8)oal_atoi(ac_name);
    /* 偏移，取下一个参数 */
    puc_param = puc_param + ul_off_set;

    //if (st_reset_param.uc_reset_type <= HAL_RESET_HW_TYPE_MAC_PHY)
    if (st_reset_param.uc_reset_type < HAL_RESET_HW_TYPE_DUMP_MAC)
    {
        /* 获取是否复位phy reg */
        dmac_get_cmd_one_arg((oal_int8*)puc_param, ac_name, &ul_off_set);
        st_reset_param.uc_reset_phy_reg = (oal_uint8)oal_atoi(ac_name);
        /* 偏移，取下一个参数 */
        puc_param = puc_param + ul_off_set;

        /* 获取是否复位mac reg */
        dmac_get_cmd_one_arg((oal_int8*)puc_param, ac_name, &ul_off_set);
        st_reset_param.uc_reset_mac_reg = (oal_uint8)oal_atoi(ac_name);
        /* 偏移，取下一个参数 */
        puc_param = puc_param + ul_off_set;

        st_reset_param.uc_reset_mac_mod = HAL_RESET_MAC_ALL;    /* HAL_RESET_MAC_ALL*/
        st_reset_param.en_reason = DMAC_RESET_REASON_HW_ERR;           /*DMAC_RESET_REASON_CONFIG*/
        dmac_reset_hw(pst_device, pst_hal_device, (oal_uint8*)&st_reset_param);
    }
    else
    {

        /* 获取debug类型:0|1|2(all|mac reg|phy reg|lut) */
        dmac_get_cmd_one_arg((oal_int8*)puc_param, ac_name, &ul_off_set);
        st_reset_param.uc_debug_type= (oal_uint8)oal_atoi(ac_name);
        /* 偏移，取下一个参数 */
        puc_param = puc_param + ul_off_set;

    #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
        /* hi1151读取所有寄存器时有进行文件操作，需要放到工作队列中进行 */
        g_st_reset_param = st_reset_param;
        OAL_INIT_WORK(&pst_mac_vap->debug_reg_work, dmac_config_reset_debug_all_work);
        oal_workqueue_schedule(&(pst_mac_vap->debug_reg_work));
    #else
        dmac_reset_debug_all(pst_hal_device,&st_reset_param);
    #endif
    }
#endif

    return OAL_SUCC;
}
/*lint -restore */

#ifdef _PRE_WLAN_FEATUER_PCIE_TEST

OAL_STATIC oal_uint32  dmac_config_pcie_test(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                    *pst_mac_device;
    hal_to_dmac_device_stru            *pst_hal_device;
    mac_cfg_pcie_test_stru             *pst_pcie_test_param;
    oal_uint8                          uc_rw_flag;
    oal_uint32                         ul_addr;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_pcie_test::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pcie_test::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_pcie_test_param = (mac_cfg_pcie_test_stru *)puc_param;
    uc_rw_flag = (pst_pcie_test_param->uc_wdata || pst_pcie_test_param->uc_rdata);

    /*写burst大小*/
    hal_pcie_test_write_burst(pst_hal_device, pst_pcie_test_param->us_burst);

    if (pst_mac_device->uc_pcie_test_flag && uc_rw_flag)
    {
        hal_pcie_test_rdata_bit(pst_hal_device, pst_pcie_test_param->uc_rdata);
        hal_pcie_test_wdata_bit(pst_hal_device, pst_pcie_test_param->uc_wdata);
    }
    else if ((!pst_mac_device->uc_pcie_test_flag) && uc_rw_flag)
    {
        pst_mac_device->pst_buff_start = (oal_uint32 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, 128, OAL_TRUE);
        if (OAL_PTR_NULL == pst_mac_device->pst_buff_start)
        {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_pcie_test::pcie_test_buf_start null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        pst_mac_device->uc_pcie_test_flag = 1;
        ul_addr = (oal_uint32)OAL_VIRT_TO_PHY_ADDR(pst_mac_device->pst_buff_start);
        hal_pcie_test_wdata_addr( pst_hal_device, ul_addr);
        hal_pcie_test_rdata_addr( pst_hal_device, ul_addr);
        hal_pcie_test_wdata_bit(pst_hal_device, pst_pcie_test_param->uc_wdata);
        hal_pcie_test_rdata_bit(pst_hal_device, pst_pcie_test_param->uc_rdata);
    }
    else if (!uc_rw_flag)
    {
        pst_mac_device->uc_pcie_test_flag = 0;
        /*填读写不使能bit*/
        hal_pcie_test_rdata_bit(pst_hal_device, pst_pcie_test_param->uc_rdata);
        hal_pcie_test_wdata_bit(pst_hal_device, pst_pcie_test_param->uc_wdata);
        /* 释放对应内存 */
        if (OAL_PTR_NULL != pst_mac_device->pst_buff_start)
        {
            OAL_MEM_FREE(pst_mac_device->pst_buff_start, OAL_TRUE);
            pst_mac_device->pst_buff_start = OAL_PTR_NULL;
        }
    }

    return OAL_SUCC;
}
#endif
#ifdef _PRE_DEBUG_MODE_USER_TRACK

OAL_STATIC oal_uint32  dmac_config_report_thrput_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_usr_thrput_stru             *pst_usr_thrput_param;
    dmac_user_stru                      *pst_dmac_user;

    pst_usr_thrput_param = (mac_cfg_usr_thrput_stru *)puc_param;

    /* 获取用户 */
    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_usr_thrput_param->auc_user_macaddr);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_report_thrput_stat::dmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == pst_usr_thrput_param->uc_param)
    {
        /* 停止上报 */
        return dmac_user_track_clear_usr_thrput_stat(&pst_dmac_user->st_user_base_info);
    }
    else
    {
        return dmac_user_track_report_usr_thrput_stat(&pst_dmac_user->st_user_base_info);
    }
}
#endif

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST

OAL_STATIC oal_uint32 dmac_config_chip_check(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;
    hal_to_dmac_device_stru         *pst_hal_device;
    oal_uint32                       ul_result;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_chip_check::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 写测试寄存器与读测试结果寄存器，检查寄存器配置是否生效*/
    hal_check_test_value_reg(pst_hal_device, 0xf000, &ul_result);

    if (0xf != ul_result)
    {
        return OAL_FAIL;
    }

    return OAL_SUCC;

}


OAL_STATIC oal_uint32 dmac_config_get_cali_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint8                        uc_hipriv_ack = OAL_FALSE;
    hal_to_dmac_device_stru         *pst_hal_device;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_cali_info::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_cali_info::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#ifndef _PRE_WLAN_PRODUCT_1151V200  /* 待51V200把校准合入后，再把该宏去掉 */
    hal_get_cali_info(pst_hal_device, puc_param);
#endif
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);

    return OAL_SUCC;

}

#endif
#ifdef _PRE_DEBUG_MODE

OAL_STATIC oal_uint32  dmac_config_set_rxch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    dmac_vap_stru                    *pst_dmac_vap;
    oal_uint8                         uc_rxch             = 0;
    hal_to_dmac_device_stru          *pst_hal_device      = OAL_PTR_NULL;



    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);


    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_rxch::pst_hal_to_dmac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_rxch = *puc_param;

    /* 配置device的rx通道 */
    pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain = uc_rxch;
    pst_hal_device->st_cfg_cap_info.uc_phy_chain = uc_rxch;

#ifdef _PRE_WLAN_PRODUCT_1151V200
    /* 51V200接收通路修改，需要关开pa才能生效 */
    hal_disable_machw_phy_and_pa(pst_dmac_vap->pst_hal_device);
    hal_clear_tx_hw_queue(pst_dmac_vap->pst_hal_device);

    /* 清fifo之后，也要删除tx队列中所有帧 */
    dmac_clear_tx_queue(pst_dmac_vap->pst_hal_device);
#endif
    hal_set_rx_multi_ant(pst_dmac_vap->pst_hal_device, pst_hal_device->st_cfg_cap_info.uc_phy_chain);
#ifdef _PRE_WLAN_PRODUCT_1151V200
    hal_enable_machw_phy_and_pa(pst_dmac_vap->pst_hal_device);
#endif

    return OAL_SUCC;
}



OAL_STATIC oal_uint32  dmac_config_dync_txpower(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru         *pst_hal_device_base;
    mac_device_stru                 *pst_device;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device_base = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device_base)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dync_txpower::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (*puc_param == pst_hal_device_base->en_dync_txpower_flag)
    {
        return OAL_SUCC;
    }

    pst_hal_device_base->en_dync_txpower_flag = *puc_param;

    #ifdef _PRE_WLAN_REALTIME_CALI
#ifdef _PRE_WLAN_FEATURE_DOUBLE_CHIP
    if (g_uc_wlan_double_chip_5g_id == pst_hal_device_base->uc_chip_id)
#else
    if (g_uc_wlan_single_chip_5g_id == pst_hal_device_base->uc_chip_id)
#endif
    {
        OAL_IO_PRINT("dmac_config_dync_txpower:5G dync txpower can not use!\n");
        return OAL_SUCC;
    }

    /* 创建定时器 */
    if (OAL_TRUE == pst_hal_device_base->en_dync_txpower_flag)
    {
        FRW_TIMER_CREATE_TIMER(&(pst_device->st_realtime_cali_timer),
                        dmac_rf_realtime_cali_timeout,
                        WLAN_REALTIME_CALI_INTERVAL_INIT,
                        pst_device,
                        OAL_TRUE,
                        OAM_MODULE_ID_DMAC,
                        pst_device->ul_core_id);
    }
    /* 删除定时器 */
    else if (OAL_FALSE == pst_hal_device_base->en_dync_txpower_flag)
    {
        FRW_TIMER_STOP_TIMER(&pst_device->st_realtime_cali_timer);
    }
    #endif

    OAM_INFO_LOG1(0, OAM_SF_CFG, "dmac_config_dync_txpower:uc_dync_power_flag = %d\n", pst_hal_device_base->en_dync_txpower_flag);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_dync_pow_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru         *pst_hal_device_base;
    pst_hal_device_base = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device_base)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dync_pow_debug_switch::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device_base->en_dync_pow_debug_switch = *puc_param;

    return OAL_SUCC;
}

#endif


#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX

OAL_STATIC oal_uint32  dmac_config_set_rfch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_tx_comp_stru            *pst_event_set_rfch;
    dmac_vap_stru                   *pst_dmac_vap;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_rfch::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置数据 */
    pst_event_set_rfch = (mac_cfg_tx_comp_stru *)puc_param;
    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.bit_tx_chain_selection = pst_event_set_rfch->uc_param;

    /* 常发模式下在线配置 */
    if (OAL_SWITCH_ON == pst_dmac_vap->pst_hal_device->bit_al_tx_flag)
    {
        hal_set_tx_dscr_field(pst_dmac_vap->pst_hal_device, pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].ul_value, HAL_RF_TEST_DATA_RATE_ZERO);
    }

    /* 动态校准需要在接收通道检测pdet，所以rxch需要配置为与txch一致 */
    pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain    = pst_event_set_rfch->uc_param;
    pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy_chain         = pst_event_set_rfch->uc_param;

    hal_set_rx_multi_ant(pst_dmac_vap->pst_hal_device, pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy_chain);

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_mcs::tx dscr tx chain selection=%d.", pst_event_set_rfch->uc_param);

    return OAL_SUCC;
}
#endif




OAL_STATIC oal_uint32  dmac_config_set_freq_skew(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifdef _PRE_WLAN_PHY_PLL_DIV
    mac_cfg_freq_skew_stru          *pst_freq_skew;
    hal_to_dmac_device_stru         *pst_hal_device_base;

    pst_hal_device_base = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device_base)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_freq_skew::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 参数判断 */
    pst_freq_skew = (mac_cfg_freq_skew_stru*)puc_param;
    if (pst_freq_skew->us_idx >= WITP_RF_SUPP_NUMS)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_freq_skew::us_idx=%d.", pst_freq_skew->us_idx);
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    hal_rf_set_freq_skew(pst_hal_device_base, pst_freq_skew->us_idx, pst_freq_skew->us_chn, pst_freq_skew->as_corr_data);
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_adjust_ppm(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_cfg_adjust_ppm_stru         *pst_adjust_ppm;
    mac_device_stru                 *pst_device;
	dmac_vap_stru                   *pst_dmac_vap;

    pst_device= mac_res_get_dev(pst_mac_vap->uc_device_id);


    /* 获取device下的配置vap */
    pst_dmac_vap = mac_res_get_dmac_vap(pst_device->auc_vap_id[0]);


    /* 参数判断 */
    pst_adjust_ppm = (mac_cfg_adjust_ppm_stru*)puc_param;

    hal_rf_adjust_ppm(pst_dmac_vap->pst_hal_device, pst_adjust_ppm->c_ppm_val, pst_dmac_vap->st_vap_base_info.st_channel.en_bandwidth, pst_adjust_ppm->uc_clock_freq);
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HILINK

OAL_STATIC oal_uint32  dmac_config_clear_fbt_scan_list(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru    *pst_mac_device;
    oal_uint32          ul_ret = OAL_SUCC;

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{dmac_config_clear_fbt_scan_list::pst_mac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{dmac_config_clear_fbt_scan_list::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = mac_device_clear_fbt_scan_list(pst_mac_device, puc_param);

    return ul_ret;
}



OAL_STATIC oal_uint32  dmac_config_fbt_scan_specified_sta(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru            *pst_mac_device;
    mac_fbt_scan_sta_addr_stru *pst_fbt_scan_sta;
    oal_uint32                  ul_ret = OAL_SUCC;

    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{dmac_config_fbt_scan_specified_sta::pst_mac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{dmac_config_fbt_scan_specified_sta::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_fbt_scan_sta = (mac_fbt_scan_sta_addr_stru *)puc_param;

    ul_ret = mac_device_set_fbt_scan_sta(pst_mac_device, pst_fbt_scan_sta);


    return ul_ret;
}



oal_uint32  dmac_config_fbt_scan_interval(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                  ul_fbt_scan_interval;
    mac_device_stru            *pst_mac_dev;
    oal_uint32                  ul_ret = OAL_SUCC;

    /* 入参检查 */
    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{dmac_config_fbt_scan_interval::pst_mac_vap or puc_param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{dmac_config_fbt_scan_interval::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_fbt_scan_interval = *((oal_uint32 *)puc_param);
    ul_ret = mac_device_set_fbt_scan_interval(pst_mac_dev, ul_fbt_scan_interval);

    return ul_ret;
}


oal_uint32  dmac_config_fbt_scan_channel(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                  ul_ret = OAL_SUCC;
    oal_uint8                   uc_fbt_scan_channel;
    mac_device_stru            *pst_mac_dev;

    /* 入参检查 */
    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{dmac_config_fbt_scan_channel::pst_mac_vap or puc_param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{hmac_config_fbt_scan_channel::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_fbt_scan_channel = *((oal_uint8 *)puc_param);

    /* 判断信道是否有效 */
    ul_ret = mac_is_channel_num_valid(pst_mac_vap->st_channel.en_band, uc_fbt_scan_channel);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{hmac_config_fbt_scan_channel::mac_is_channel_num_valid[%d] failed[%d].}", uc_fbt_scan_channel, ul_ret);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

#ifdef _PRE_WLAN_FEATURE_11D
    /* 信道14特殊处理，只在11b协议模式下有效 */
    if ((14 == uc_fbt_scan_channel) && (WLAN_LEGACY_11B_MODE != pst_mac_vap->en_protocol))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_HILINK,
                         "{hmac_config_fbt_scan_channel::channel-14 only available in 11b, curr protocol=%d.}", pst_mac_vap->en_protocol);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
#endif

    ul_ret = mac_device_set_fbt_scan_channel(pst_mac_dev, uc_fbt_scan_channel);

    return ul_ret;

}



oal_uint32  dmac_config_fbt_scan_report_period(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                  l_value;
    mac_device_stru            *pst_mac_dev;
    oal_uint32                  ul_ret = OAL_SUCC;

    /* 入参检查 */
    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG2(0, OAM_SF_HILINK, "{dmac_config_fbt_scan_report_period::null param,pst_mac_vap=%d puc_param=%d.}",pst_mac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{hmac_config_fbt_scan_report_period::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    l_value = *((oal_uint32 *)puc_param);

    ul_ret = mac_device_set_fbt_scan_report_period(pst_mac_dev, l_value);

    return ul_ret;

}


oal_uint32  dmac_config_fbt_scan_enable(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    oal_uint8                   uc_cfg_fbt_scan_enable = 0;
    mac_device_stru            *pst_mac_dev = OAL_PTR_NULL;
    oal_uint32                  ul_ret = OAL_SUCC;

    /* 入参检查 */
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{dmac_config_fbt_scan_enable::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{dmac_config_fbt_scan_enable::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取用户下发的要配置到fbt scan管理实体中的参数 */
    uc_cfg_fbt_scan_enable = *puc_param;

    /* 记录配置的模式到fbt scan管理实体中，当前只支持侦听一个用户 */
    ul_ret = mac_device_set_fbt_scan_enable(pst_mac_dev, uc_cfg_fbt_scan_enable);

    return ul_ret;
}


oal_uint32  dmac_config_set_white_lst_ssidhiden(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_hilink_white_node_stru      *pst_white_node;
    oal_uint32                       ul_ret = OAL_SUCC;

    /* 入参检查 */
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{dmac_config_set_white_lst_ssidhiden::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_white_node = (oal_hilink_white_node_stru *)puc_param;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_white_node))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{dmac_config_set_white_lst_ssidhiden::pst_white_node null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    ul_ret = mac_vap_update_hilink_white_list(pst_mac_vap, pst_white_node);

    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT


oal_uint32  dmac_config_set_mgmt_frame_filters(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                       ul_mgmt_frame_filters;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG2(0, OAM_SF_HILINK, "{dmac_config_set_mgmt_frame_filters::null param,pst_mac_vap=%d puc_param=%d.}",
                       pst_mac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_mgmt_frame_filters = *(oal_uint32 *)puc_param;
    pst_mac_vap->us_mgmt_frame_filters = (oal_uint16)ul_mgmt_frame_filters;
    return OAL_SUCC;
}


oal_uint32  dmac_config_get_sta_diag_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_param_char_stru             st_param;
    mac_sta_status_diag_info_stru      *pst_in_sta_diag_info;
    mac_sta_status_diag_info_stru      *psta_diag_info;
    dmac_user_stru                     *pst_dmac_user;
    dmac_tid_stru                      *pst_tid;
    oal_uint32                          ul_tid;

    pst_in_sta_diag_info = (mac_sta_status_diag_info_stru *)puc_param;
    pst_dmac_user = MAC_GET_DMAC_USER(pst_in_sta_diag_info->pst_mac_usr);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{dmac_config_get_sta_diag_info::pst_dmac_user null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(&st_param, sizeof(st_param));
    psta_diag_info = (mac_sta_status_diag_info_stru *)st_param.auc_buff;
    psta_diag_info->c_rssi = oal_get_real_rssi(pst_dmac_user->s_rx_rssi);
    for (ul_tid = 0; ul_tid < WLAN_TID_MAX_NUM; ul_tid++)
    {
        pst_tid = &(pst_dmac_user->ast_tx_tid_queue[ul_tid]);
        if (pst_tid->pst_ba_tx_hdl != NULL)
        {
            if (pst_tid->pst_ba_tx_hdl->en_ba_conn_status == DMAC_BA_COMPLETE)
            {
                psta_diag_info->uc_agg_mode |= (oal_uint8)(1 << ul_tid);
            }
        }
    }

    pst_tid = &(pst_dmac_user->ast_tx_tid_queue[0]);
    if (pst_tid->pst_ba_tx_hdl != NULL)
    {
        psta_diag_info->uc_max_agg_num = pst_tid->pst_ba_tx_hdl->uc_ampdu_max_num;
    }

    //hal_cfg_rts_tx_param_stru  sta_diag_info.rts_rate
    //dmac_tx_update_alg_param  sta_diag_info.rts_retry_cnt
    psta_diag_info->uc_rts_rate = 0;
    psta_diag_info->uc_rts_retry_cnt = 3;//ALG_AUTORATE_TOLERANT_RTS_ALL_FAIL_CNT;
    psta_diag_info->ul_tx_unicast_bytes = (oal_uint32)pst_dmac_user->st_dmac_thrpt_stat_info.ull_tx_bytes;
    psta_diag_info->ul_rx_unicast_bytes = (oal_uint32)pst_dmac_user->st_dmac_thrpt_stat_info.ull_rx_bytes;
    psta_diag_info->ul_tx_mcast_bytes = pst_dmac_user->pst_stat_count->st_count_mpdu.ul_tx_mcast_bytes;
    psta_diag_info->ul_rx_mcast_bytes = pst_dmac_user->pst_stat_count->st_count_mpdu.ul_rx_mcast_bytes;
    oal_memcopy(psta_diag_info->aul_sta_tx_mcs_cnt, pst_dmac_user->pst_stat_count->st_count_mpdu.aul_sta_tx_mcs_cnt,
                  sizeof(psta_diag_info->aul_sta_tx_mcs_cnt));
    oal_memcopy(psta_diag_info->aul_sta_rx_mcs_cnt, pst_dmac_user->pst_stat_count->st_count_mpdu.aul_sta_rx_mcs_cnt,
                  sizeof(psta_diag_info->aul_sta_rx_mcs_cnt));
    psta_diag_info->ul_sleep_times = pst_dmac_user->ul_sta_sleep_times;

    st_param.l_buff_len = sizeof(mac_sta_status_diag_info_stru);
    return dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_STA_DIAG_INFO_RSP, sizeof(mac_cfg_param_char_stru), (oal_uint8 *)&st_param);
}


oal_uint32  dmac_config_get_vap_diag_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_param_char_stru         st_param;
    mac_vap_status_diag_info_stru  *pst_vap_diag_info;
    mac_device_stru                *pst_mac_device;
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
    dmac_device_stru               *pst_dmac_device;
#endif
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint32                      ul_rx_ampdu_fcs_num;
    oal_uint32                      ul_rx_delimiter_fail_num;
    oal_uint32                      ul_rx_mpdu_fcs_num;
    oal_uint32                      ul_rx_phy_err_mac_passed_num;
    oal_uint32                      ul_rx_phy_longer_err_num;
    oal_uint32                      ul_rx_phy_shorter_err_num;
    wlan_scan_chan_stats_stru       *pst_chan_result;
    oal_uint32                      ul_noninterference_time = 0;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    /* 设置配置命令参数 */
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{dmac_config_get_vap_diag_info::pst_device null.");
        return OAL_FAIL;
    }
    OAL_MEMZERO(&st_param, sizeof(st_param));
    pst_vap_diag_info = (mac_vap_status_diag_info_stru *)st_param.auc_buff;

    pst_hal_device  = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "{dmac_config_get_vap_diag_info::pst_hal_device null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    hi1151_get_vap_diag_info(pst_hal_device, (oal_uint32 *)pst_vap_diag_info);
#endif
    pst_vap_diag_info->uc_offset_second_channel = 0;
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if(OAL_PTR_NULL != pst_dmac_device)
    {
        oal_uint32  ul_offset;
        oal_uint32  n;
        oal_uint32  sum_sample = 0;
        dmac_device_bsd_ringbuf     *pst_sample_buf = &pst_dmac_device->st_bsd.st_sample_result_buf;
        //vap_diag_info.busy;
        //dmac_bsd_ringbuf_put(&(pst_dmac_device->st_bsd.st_sample_result_buf),us_ratio);
        for( n = 0; n < pst_sample_buf->ul_size; n++)
        {
            if (pst_sample_buf->ul_in > n)
            {
                ul_offset = pst_sample_buf->ul_in - n - 1;
            }
            else
            {
                ul_offset = BSD_VAP_LOAD_SAMPLE_MAX_CNT + pst_sample_buf->ul_in - n - 1;
            }
            sum_sample = sum_sample + pst_sample_buf->aus_buf[ul_offset];
        }
        if (pst_sample_buf->ul_size > 0)
        {
            pst_vap_diag_info->uc_channel_usage = (oal_uint8)(sum_sample / pst_sample_buf->ul_size);
        }
        else
        {
            pst_vap_diag_info->uc_channel_usage = 0;
        }
    }
#else
    pst_vap_diag_info->uc_channel_usage = 0;
#endif
   hal_get_rx_err_count(pst_hal_device,
                       &ul_rx_ampdu_fcs_num,
                       &ul_rx_delimiter_fail_num,
                       &ul_rx_mpdu_fcs_num,
                       &ul_rx_phy_err_mac_passed_num,
                       &ul_rx_phy_longer_err_num,
                       &ul_rx_phy_shorter_err_num);
    pst_vap_diag_info->us_fa = (oal_uint16)(ul_rx_ampdu_fcs_num + ul_rx_mpdu_fcs_num);
    pst_vap_diag_info->us_cca = (oal_uint16)pst_hal_device->ul_rx_normal_mdpu_succ_num;

    pst_chan_result = &pst_hal_device->st_chan_result;
    if(WLAN_BAND_WIDTH_20M == pst_mac_vap->st_channel.en_bandwidth)
    {
        ul_noninterference_time = pst_chan_result->ul_total_free_time_20M_us;
    }
    else if((WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth) || (WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth))
    {
        ul_noninterference_time = pst_chan_result->ul_total_free_time_40M_us;
    }
    else
    {
        ul_noninterference_time = pst_chan_result->ul_total_free_time_80M_us;
    }

    ul_noninterference_time += pst_chan_result->ul_total_send_time_us + pst_chan_result->ul_total_recv_time_us;

    if (pst_chan_result->ul_total_stats_time_us <= ul_noninterference_time)
    {
        pst_vap_diag_info->ul_channel_busy = 1000;
    }
    else
    {
        pst_vap_diag_info->ul_channel_busy = (pst_chan_result->ul_total_stats_time_us - ul_noninterference_time) * 1000/
                                              pst_chan_result->ul_total_stats_time_us;
    }

    pst_vap_diag_info->ul_wait_agg_time = 10; //g_aus_aggr_time_list
    pst_vap_diag_info->ul_skb_remain_buffer = WLAN_TID_MPDU_NUM_LIMIT - pst_mac_device->us_total_mpdu_num;
    pst_vap_diag_info->ul_vo_remain_count = WLAN_TID_MPDU_NUM_LIMIT - pst_mac_device->us_total_mpdu_num;
    pst_vap_diag_info->ul_vi_remain_count = WLAN_TID_MPDU_NUM_LIMIT - pst_mac_device->us_total_mpdu_num;
    pst_vap_diag_info->ul_be_remain_count = WLAN_TID_MPDU_NUM_LIMIT - pst_mac_device->us_total_mpdu_num;
    pst_vap_diag_info->ul_bk_remain_count = WLAN_TID_MPDU_NUM_LIMIT - pst_mac_device->us_total_mpdu_num;

    st_param.l_buff_len = sizeof(mac_vap_status_diag_info_stru);
    return dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_VAP_DIAG_INFO_RSP, sizeof(mac_cfg_param_char_stru), (oal_uint8 *)&st_param);
}


oal_uint32  dmac_config_set_sensing_bssid(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                       ul_ret;
    dmac_sensing_bssid_cfg_stru          *pst_sensing_bssid;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_HILINK, "{dmac_config_set_sensing_bssid::null param,pst_mac_vap=%d puc_param=%d.}",
                       pst_mac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_sensing_bssid = (dmac_sensing_bssid_cfg_stru *)puc_param;
    ul_ret = dmac_vap_update_sensing_bssid_list(pst_mac_vap, pst_sensing_bssid);
    return ul_ret;
}


oal_uint32  dmac_config_get_sensing_bssid_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                          *pst_dmac_vap;
    dmac_sensing_bssid_list_stru           *pst_sensing_bssid_list;
    dmac_sensing_bssid_list_member_stru    *pst_sensing_bssid_member;
    mac_cfg_param_char_stru                 st_param;
    oal_dlist_head_stru                    *pst_entry;
    oal_dlist_head_stru                    *pst_dlist_tmp;
    dmac_query_sensing_bssid_stru          *pst_ret_sensing_info;
    oal_uint32                              ul_sensing_bssid_num = 0;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "dmac_config_get_sensing_bssid_info: dmac vap is null. vap mode:%d, vap state:%d",
                        pst_mac_vap->en_vap_mode, pst_mac_vap->en_vap_state);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_sensing_bssid_list = &(pst_dmac_vap->st_sensing_bssid_list);
    oal_spin_lock(&(pst_sensing_bssid_list->st_lock));

    pst_ret_sensing_info = (dmac_query_sensing_bssid_stru *)st_param.auc_buff;
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_sensing_bssid_list->st_list_head))
    {
        /** 防止st_param.auc_buff缓冲区溢出 */
        if ((ul_sensing_bssid_num + 1) * sizeof(dmac_query_sensing_bssid_stru) > sizeof(st_param.auc_buff))
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_HILINK, "dmac_config_get_sensing_bssid_info:too much sensing_bssid_num %d.",
                            ul_sensing_bssid_num);

            break;
        }

        pst_sensing_bssid_member = OAL_DLIST_GET_ENTRY(pst_entry, dmac_sensing_bssid_list_member_stru, st_dlist);
        pst_ret_sensing_info->ul_timestamp = pst_sensing_bssid_member->ul_timestamp;
        pst_ret_sensing_info->c_rssi = pst_sensing_bssid_member->c_rssi;
        oal_memcopy(pst_ret_sensing_info->auc_mac_addr, pst_sensing_bssid_member->auc_mac_addr, WLAN_MAC_ADDR_LEN);
        pst_ret_sensing_info++;
        ul_sensing_bssid_num++;
    }
    oal_spin_unlock(&(pst_sensing_bssid_list->st_lock));

    st_param.l_buff_len = (oal_int32)(sizeof(dmac_query_sensing_bssid_stru) * ul_sensing_bssid_num);
    return dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_SENSING_BSSID_INFO, sizeof(mac_cfg_param_char_stru), (oal_uint8 *)&st_param);
}

#endif

#ifdef _PRE_WLAN_FEATURE_11V

OAL_STATIC oal_int32 dmac_hilink_tx_11v_request_cb(oal_void *p,
                                     oal_uint8 uc_event_type,
                                     oal_void* param)
{
    dmac_user_stru  *pst_dmac_user = (dmac_user_stru*)p;

    switch(uc_event_type)
    {
        case DMAC_11V_CALLBACK_RETURN_REICEVE_RSP:
        {
            if(((dmac_bsst_rsp_info_stru*)param)->uc_status_code)
            {
                OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_HILINK,
                    "{dmac_hilink_tx_11v_request_cb::user[xx:xx:xx:%02x:%02x:%02x]* reject change to dest vap!",
                    pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                    pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                    pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);
            }
            else
            {
                OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_HILINK,
                    "{dmac_hilink_tx_11v_request_cb::user[xx:xx:xx:%02x:%02x:%02x]* will change to dest vap triged by 11v request!",
                    pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                    pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                    pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);
            }
            break;
        }
        case DMAC_11V_CALLBACK_RETURN_WAIT_RSP_TIMEOUT:
        {
            OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_HILINK,
                "{dmac_bsd_tx_11v_request_cb::user[xx:xx:xx:%02x:%02x:%02x]* not response to 11v request!",
                pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);
            break;
        }
        default:
            OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_HILINK,
                                "{dmac_hilink_tx_11v_request_cb::en_event_type error!");
            break;
    }

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE oal_void dmac_hilink_set_dest_vap_info(oal_hilink_change_sta_to_target_ap *pst_change_ap_param,
                                               dmac_neighbor_bss_info_stru *pst_neighbor_bss_list)
{
    oal_memcopy(pst_neighbor_bss_list->auc_mac_addr, pst_change_ap_param->auc_target_ap_mac_addr, WLAN_MAC_ADDR_LEN);
    oal_memcopy(&(pst_neighbor_bss_list->st_bssid_info),&(pst_change_ap_param->st_target_ap_info),OAL_SIZEOF(oal_bssid_infomation_stru));
    pst_neighbor_bss_list->uc_candidate_perf = pst_change_ap_param->uc_candidate_perf;
    pst_neighbor_bss_list->uc_opt_class      = pst_change_ap_param->uc_opt_class;
    pst_neighbor_bss_list->uc_phy_type       = pst_change_ap_param->uc_phy_type;
    pst_neighbor_bss_list->uc_chl_num        = pst_change_ap_param->uc_chl_num;

    OAL_MEMZERO(&pst_neighbor_bss_list->st_term_duration.auc_termination_tsf,DMAC_11V_TERMINATION_TSF_LENGTH);
    pst_neighbor_bss_list->st_term_duration.us_duration_min = 0xffff;
    pst_neighbor_bss_list->st_term_duration.uc_sub_ie_id = 4;
}



oal_uint32  dmac_config_fbt_change_to_other_ap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                     *pst_mac_dev = OAL_PTR_NULL;
    oal_uint32                           ul_ret = OAL_SUCC;
    dmac_vap_stru                       *pst_dmac_vap;
    dmac_user_stru                      *pst_dmac_user;
    dmac_neighbor_bss_info_stru          st_neighbor_bss_list;
    oal_hilink_change_sta_to_target_ap  *pst_change_ap_param;

    /* 入参检查 */


    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);

    pst_change_ap_param = (oal_hilink_change_sta_to_target_ap*)puc_param;
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    /* 获取用户 */
    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_change_ap_param->auc_sta_mac_addr);
    dmac_hilink_set_dest_vap_info(pst_change_ap_param,&st_neighbor_bss_list);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_HILINK, "{dmac_config_fbt_change_to_other_ap::dmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    dmac_tx_bsst_req_action_one_bss(pst_dmac_vap,pst_dmac_user,&st_neighbor_bss_list,(dmac_user_callback_func_11v)dmac_hilink_tx_11v_request_cb);

    return ul_ret;
}
#endif
#endif

oal_uint32  dmac_config_set_dyn_cali_param(mac_vap_stru * OAL_CONST pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                      ul_ret = OAL_SUCC;
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    hal_to_dmac_device_stru        *pst_hal_device;
    mac_ioctl_dyn_cali_param_stru  *pst_dyn_cali_param;
    oal_uint16                      us_param_val;
    hal_dyn_cali_val_stru          *pst_dyn_cali_val;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "{dmac_config_set_dyn_cali_param::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_dyn_cali_val = &pst_hal_device->st_dyn_cali_val;
    pst_dyn_cali_param = (mac_ioctl_dyn_cali_param_stru *)puc_param;
    us_param_val = (pst_dyn_cali_param->us_value) ;
    switch (pst_dyn_cali_param->en_dyn_cali_cfg)
    {
        case MAC_DYN_CALI_CFG_SET_EN_REALTIME_CALI_ADJUST:
            pst_dyn_cali_val->en_realtime_cali_adjust = !!us_param_val;;
            OAM_WARNING_LOG1(0, OAM_SF_CALIBRATE, "{dmac_config_set_dyn_cali_param:: set en_realtime_cali_adjust flag[%d]}", pst_dyn_cali_val->en_realtime_cali_adjust);
            break;
        case MAC_DYN_CALI_CFG_SET_2G_DSCR_INT:
            hal_config_set_dyn_cali_dscr_interval(pst_hal_device, WLAN_BAND_2G, us_param_val);
            OAM_WARNING_LOG1(0, OAM_SF_CALIBRATE, "{dmac_config_set_dyn_cali_param:: config success: pst_hal_device->st_dyn_cali_val.aus_cali_en_interval[WLAN_BAND_2G]= %d}\n", pst_dyn_cali_val->aus_cali_en_interval[WLAN_BAND_2G]);
            break;
        case MAC_DYN_CALI_CFG_SET_5G_DSCR_INT:
            hal_config_set_dyn_cali_dscr_interval(pst_hal_device, WLAN_BAND_5G, us_param_val);
            OAM_WARNING_LOG1(0, OAM_SF_CALIBRATE, "{dmac_config_set_dyn_cali_param:: config success: pst_hal_device->st_dyn_cali_val.aus_cali_en_interval[WLAN_BAND_5G]= %d}\n", pst_dyn_cali_val->aus_cali_en_interval[WLAN_BAND_5G]);
            break;
        case MAC_DYN_CALI_CFG_SET_CHAIN_INT:
            pst_dyn_cali_val->us_cali_ch_sw_interval = us_param_val;
            OAM_WARNING_LOG1(0, OAM_SF_CALIBRATE, "{dmac_config_set_dyn_cali_param:: config success: pst_hal_device->st_dyn_cali_val.us_cali_ch_sw_interval = %d}\n", pst_dyn_cali_val->us_cali_ch_sw_interval);
            break;
        case MAC_DYN_CALI_CFG_SET_PDET_MIN_TH:
            pst_dyn_cali_val->uc_cali_pdet_min_th = (oal_uint8)us_param_val;
            OAM_WARNING_LOG1(0, OAM_SF_CALIBRATE, "{dmac_config_set_dyn_cali_param:: config success: pst_hal_device->st_dyn_cali_val.uc_cali_pdet_min_th = %d}\n", pst_dyn_cali_val->uc_cali_pdet_min_th);
            break;
        case MAC_DYN_CALI_CFG_SET_PDET_MAX_TH:
            pst_dyn_cali_val->uc_cali_pdet_max_th = (oal_uint8)us_param_val;
            OAM_WARNING_LOG1(0, OAM_SF_CALIBRATE, "{dmac_config_set_dyn_cali_param:: config success: pst_hal_device->st_dyn_cali_val.uc_cali_pdet_max_th = %d}\n", pst_dyn_cali_val->uc_cali_pdet_max_th);
            break;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_CALIBRATE, "{dmac_config_set_dyn_cali_param:: pst_dyn_cali_param->en_dyn_cali_cfg = %d}\n", pst_dyn_cali_param->en_dyn_cali_cfg);
            ul_ret = OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
#endif
    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_AP_PM
OAL_STATIC oal_uint32  dmac_config_wifi_en(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    oal_int32                    l_value;
    dmac_vap_stru               *pst_dmac_vap;
    oal_uint16                   us_type;
    mac_device_stru             *pst_mac_dev;
    oal_uint8                    uc_state_to;
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                    uc_hipriv_ack = OAL_FALSE;
#endif

    l_value = *((oal_int32 *)puc_param);
    if(1 == l_value)
    {
        us_type = AP_PWR_EVENT_WIFI_ENABLE;
    }
    else
    {
        us_type = AP_PWR_EVENT_WIFI_DISABLE;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_config_wifi_en::pst_mac_dev[%d] is NULL!}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(pst_mac_dev->uc_cfg_vap_id == pst_mac_vap->uc_vap_id)
    {
        /*cfg vap,直接配置device*/
        if (AP_PWR_EVENT_WIFI_ENABLE == us_type)
        {
            uc_state_to = DEV_PWR_STATE_WORK;
        }
        else
        {
            uc_state_to = DEV_PWR_STATE_DEEP_SLEEP;
        }
        mac_pm_set_hal_state(pst_mac_dev, pst_mac_vap, uc_state_to);
    }
    else
    {
        if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
        {
        /*业务VAP，给PM推送事件*/
            pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);

            dmac_pm_post_event(pst_dmac_vap,us_type,0,OAL_PTR_NULL);
        }
    }

    /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif

    return OAL_SUCC;

}



OAL_STATIC oal_uint32  dmac_config_pm_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    mac_device_stru             *pst_device;
    dmac_vap_stru               *pst_dmac_vap;
    mac_pm_arbiter_stru         *pst_pm_arbiter;
    mac_pm_handler_stru         *pst_pm_handler;
    oal_fsm_stru                *pst_pm_fsm;
    oal_uint32                   ul_loop;
    oal_uint8                    uc_vap_idx;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pm_info::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*device下arbiter的信息*/
    pst_pm_arbiter = pst_device->pst_pm_arbiter;
    if(OAL_PTR_NULL == pst_pm_arbiter)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
    OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "Requestor num = %d,id_bitmap = 0x%x\n",pst_pm_arbiter->uc_requestor_num,pst_pm_arbiter->ul_id_bitmap);
    for(ul_loop=0;ul_loop<pst_pm_arbiter->uc_requestor_num;ul_loop++)
    {
        if(0 == ul_loop%4)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "\n");
        }
        OAL_IO_PRINT("Requestor id %d:%s ,",ul_loop,pst_pm_arbiter->requestor[ul_loop].auc_id_name);

    }
    /*state bitmap*/
    for(ul_loop=0;ul_loop<DEV_PWR_STATE_BUTT;ul_loop++)
    {
        if(0 == ul_loop%4)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "\n");
        }
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "state %d bitmap:0x%x ,",
                        ul_loop,pst_pm_arbiter->ul_state_bitmap[ul_loop]);
    }

    OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "\n Curr State:(%d), Prev State:(%d)\n",
                  pst_pm_arbiter->uc_cur_state, pst_pm_arbiter->uc_prev_state);
   /*每VAP状态机FSM*/
   /* 遍历device下所有vap */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++)
    {
        pst_dmac_vap = mac_res_get_dmac_vap(pst_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_dmac_vap)
        {
            OAM_WARNING_LOG1(pst_device->uc_cfg_vap_id, OAM_SF_CFG, "{hmac_config_wifi_enable::pst_mac_vap null, vap id=%d.", pst_device->auc_vap_id[uc_vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }
        pst_pm_handler = &pst_dmac_vap->st_pm_handler;
        if(OAL_FALSE == pst_pm_handler->en_is_fsm_attached)
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{hmac_config_wifi_enable::pm fsm not attached.}");
            return OAL_FAIL;
        }
        pst_pm_fsm = &pst_pm_handler->st_oal_fsm;

        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,"Arbiter id = %d,max_inactive_time = %d\n",
                    pst_pm_handler->ul_pwr_arbiter_id,pst_pm_handler->ul_max_inactive_time);
        OAM_ERROR_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,"inactive_time = %d,user_check_count = %d,user_num = %d\n",
                    pst_pm_handler->ul_inactive_time,pst_pm_handler->ul_user_check_count,pst_dmac_vap->st_vap_base_info.us_user_nums);
        OAM_ERROR_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,"auto_sleep_en = %d, wow_en = %d, siso_en = %d\n",
                    pst_pm_handler->bit_pwr_sleep_en,pst_pm_handler->bit_pwr_wow_en,
                    pst_pm_handler->bit_pwr_siso_en);
        OAM_ERROR_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,"Cur State:(%d), Prev state:(%d), Last event:(%d)\n",
                    pst_pm_fsm->uc_cur_state, pst_pm_fsm->uc_prev_state, pst_pm_fsm->us_last_event);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_pm_en(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru             *pst_device;
    dmac_vap_stru               *pst_dmac_vap;
    mac_pm_arbiter_stru         *pst_pm_arbiter;
    oal_uint8                    uc_vap_idx;
    oal_int32                    l_value;
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                    uc_hipriv_ack = OAL_FALSE;
#endif

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pm_en::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*device下arbiter的信息*/
    pst_pm_arbiter = pst_device->pst_pm_arbiter;
    if(OAL_PTR_NULL == pst_pm_arbiter)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pm_en::pst_pm_arbiter null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    l_value = *((oal_int32 *)puc_param);
    pst_device->en_pm_enable = (1==l_value)?OAL_TRUE:OAL_FALSE;


    /*使能,为每个VAP attach pm handler*/
    /*去使能,为每个VAP deattach pm handler*/
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++)
    {
      pst_dmac_vap = mac_res_get_dmac_vap(pst_device->auc_vap_id[uc_vap_idx]);
      if (OAL_PTR_NULL == pst_dmac_vap)
      {
          OAM_WARNING_LOG1(pst_device->uc_cfg_vap_id, OAM_SF_CFG, "{dmac_config_pm_en::pst_mac_vap null, vap id=%d.", pst_device->auc_vap_id[uc_vap_idx]);
          return OAL_ERR_CODE_PTR_NULL;
      }

      /*STA模式待开发*/
      if(WLAN_VAP_MODE_BSS_STA ==  pst_dmac_vap->st_vap_base_info.en_vap_mode)
      {
        continue;
      }

      if(OAL_TRUE == pst_device->en_pm_enable)
      {
        dmac_pm_ap_attach(pst_dmac_vap);
      }
      else
      {
        dmac_pm_ap_deattach(pst_dmac_vap);
      }
    }

    /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_PERFORM_STAT

OAL_STATIC oal_uint32  dmac_config_pfm_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_stat_param_stru        *pst_stat_param  = OAL_PTR_NULL;
    mac_user_stru                  *pst_user        = OAL_PTR_NULL;
    dmac_tid_stru                  *pst_tid         = OAL_PTR_NULL;
    oal_void                       *p_void          = OAL_PTR_NULL;
    oal_uint32                      ul_ret          = OAL_SUCC;

    pst_stat_param = (mac_cfg_stat_param_stru *)puc_param;

    switch (pst_stat_param->en_stat_type)
    {
        case MAC_STAT_TYPE_TID_DELAY:
        case MAC_STAT_TYPE_TID_PER:
        case MAC_STAT_TYPE_TID_THRPT:
            pst_user = mac_vap_get_user_by_addr(pst_mac_vap, pst_stat_param->auc_mac_addr);
            if (OAL_PTR_NULL == pst_user)
            {
                return OAL_ERR_CODE_PTR_NULL;
            }

            dmac_user_get_tid_by_num(pst_user, pst_stat_param->uc_tidno, &pst_tid);

            p_void = (oal_void *)pst_tid;

            break;

        case MAC_STAT_TYPE_USER_THRPT:
        case MAC_STAT_TYPE_USER_BSD:
            pst_user = mac_vap_get_user_by_addr(pst_mac_vap, pst_stat_param->auc_mac_addr);

            p_void = (oal_void *)pst_user;

            break;

        case MAC_STAT_TYPE_VAP_THRPT:
            p_void = (oal_void *)pst_mac_vap;

            break;

        default:
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pfm_stat::mac_vap_find_user_by_macaddr failed[%d].}", ul_ret);
            return OAL_FAIL;
    }

    /* 注册统计节点 */
    ul_ret = dmac_stat_register(MAC_STAT_MODULE_CMD, pst_stat_param->en_stat_type, p_void, OAL_PTR_NULL, OAL_PTR_NULL,pst_mac_vap->ul_core_id);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pfm_stat::invalid en_stat_type[%d].}", pst_stat_param->en_stat_type);
        return ul_ret;
    }

    /* 同时开始启动统计 */
    ul_ret = dmac_stat_start(MAC_STAT_MODULE_CMD,
                             pst_stat_param->en_stat_type,
                             pst_stat_param->us_stat_period,
                             pst_stat_param->us_stat_num,
                             p_void);

    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pfm_stat::dmac_stat_start failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}



OAL_STATIC oal_uint32  dmac_config_pfm_display(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_display_param_stru     *pst_display_param  = OAL_PTR_NULL;
    mac_user_stru                  *pst_user            = OAL_PTR_NULL;
    dmac_tid_stru                  *pst_tid             = OAL_PTR_NULL;
    oal_void                       *p_void              = OAL_PTR_NULL;
    oal_uint32                      ul_ret              = OAL_SUCC;

    pst_display_param = (mac_cfg_display_param_stru *)puc_param;

    switch (pst_display_param->en_stat_type)
    {
        case MAC_STAT_TYPE_TID_DELAY:
        case MAC_STAT_TYPE_TID_PER:
        case MAC_STAT_TYPE_TID_THRPT:
            pst_user = mac_vap_get_user_by_addr(pst_mac_vap, pst_display_param->auc_mac_addr);
            if (OAL_PTR_NULL == pst_user)
            {
                return OAL_ERR_CODE_PTR_NULL;
            }

            dmac_user_get_tid_by_num(pst_user, pst_display_param->uc_tidno, &pst_tid);

            p_void = (oal_void *)pst_tid;

            break;

        case MAC_STAT_TYPE_USER_THRPT:
            pst_user = mac_vap_get_user_by_addr(pst_mac_vap, pst_display_param->auc_mac_addr);

            p_void = (oal_void *)pst_user;

            break;

        case MAC_STAT_TYPE_VAP_THRPT:
            p_void = (oal_void *)pst_mac_vap;

            break;

        default:
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pfm_display::invalid en_stat_type[%d].}", pst_display_param->en_stat_type);
            return OAL_FAIL;
    }

    /* 显示统计信息 */
    ul_ret = dmac_stat_display(MAC_STAT_MODULE_CMD, pst_display_param->en_stat_type, p_void);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pfm_display::dmac_stat_display failed[%d].}", ul_ret);
        return ul_ret;
    }

    /* 注销统计节点 */
    ul_ret = dmac_stat_unregister(MAC_STAT_MODULE_CMD, pst_display_param->en_stat_type, p_void);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pfm_display::dmac_stat_unregister failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  dmac_config_set_lpm_soc_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    mac_device_stru                  *pst_device;
    mac_cfg_lpm_soc_set_stru         *pst_soc_para;
    hal_to_dmac_device_stru        *pst_hal_device;
    wlan_channel_bandwidth_enum_uint8 en_cur_bandwidth;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_user_keepalive_timer::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_soc_para = (mac_cfg_lpm_soc_set_stru*)puc_param;

    en_cur_bandwidth = pst_device->en_max_bandwidth; //此变量是不是要移入hal device

    switch(pst_soc_para->en_mode)
    {
        case MAC_LPM_SOC_BUS_GATING:
            hal_set_soc_lpm(pst_hal_device,HAL_LPM_SOC_BUS_GATING,pst_soc_para->uc_on_off,pst_soc_para->uc_pcie_idle);
            break;
        case MAC_LPM_SOC_PCIE_RD_BYPASS:
            hal_set_soc_lpm(pst_hal_device,HAL_LPM_SOC_PCIE_RD_BYPASS,pst_soc_para->uc_on_off,pst_soc_para->uc_pcie_idle);
            break;
        case MAC_LPM_SOC_MEM_PRECHARGE:
            hal_set_soc_lpm(pst_hal_device,HAL_LPM_SOC_MEM_PRECHARGE,pst_soc_para->uc_on_off,pst_soc_para->uc_pcie_idle);
            break;
        case MAC_LPM_SOC_PCIE_L0_S:
            pst_hal_device->bit_l0s_on = pst_soc_para->uc_on_off;
            hal_set_soc_lpm(pst_hal_device,HAL_LPM_SOC_PCIE_L0,pst_soc_para->uc_on_off,pst_soc_para->uc_pcie_idle);
            break;
        case MAC_LPM_SOC_PCIE_L1_0:
            pst_hal_device->bit_l1pm_on = pst_soc_para->uc_on_off;
            hal_set_soc_lpm(pst_hal_device,HAL_LPM_SOC_PCIE_L1_PM,pst_soc_para->uc_on_off,pst_soc_para->uc_pcie_idle);
            break;
        case MAC_LPM_SOC_AUTOCG_ALL:
            pst_hal_device->bit_autCG_on = pst_soc_para->uc_on_off;
            hal_set_soc_lpm(pst_hal_device,HAL_LPM_SOC_AUTOCG_ALL,pst_soc_para->uc_on_off,pst_soc_para->uc_pcie_idle);
            break;
        case MAC_LPM_SOC_ADC_FREQ:
            if (mac_is_dbac_enabled(pst_device))
            {
                hal_set_soc_lpm(pst_hal_device, HAL_LPM_SOC_ADC_FREQ,0,pst_soc_para->uc_pcie_idle);
            }
            else
            {
                hal_set_soc_lpm(pst_hal_device, HAL_LPM_SOC_ADC_FREQ,pst_soc_para->uc_on_off,en_cur_bandwidth);
            }
            break;
        case MAC_LPM_SOC_PCIE_L1S:
            hal_set_soc_lpm(pst_hal_device, HAL_LPM_SOC_PCIE_L1S,pst_soc_para->uc_on_off,pst_soc_para->uc_pcie_idle);
            break;
        default:
            break;
    }

    return OAL_SUCC ;

}

#ifdef _PRE_WLAN_CHIP_TEST

OAL_STATIC oal_uint32  dmac_config_set_lpm_chip_state(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_lpm_state_param_stru         st_para;
    mac_cfg_lpm_sleep_para_stru     *pst_set_para;
    hal_to_dmac_device_stru         *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_set_lpm_chip_state::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(&st_para, OAL_SIZEOF(hal_lpm_state_param_stru));

    pst_set_para = (mac_cfg_lpm_sleep_para_stru*)puc_param;
    if (MAC_LPM_STATE_SOFT_SLEEP == pst_set_para->uc_pm_switch)
    {
        st_para.bit_soft_sleep_en = 1;
        st_para.ul_sleep_time     = 1000*(pst_set_para->us_sleep_ms);   /*15ms*/

        //dmac_test_lpm_create_sleep_timer(pst_device,st_para.us_sleep_time);

        hal_set_lpm_state(pst_hal_device,HAL_LPM_STATE_NORMAL_WORK,HAL_LPM_STATE_DEEP_SLEEP,&st_para, OAL_PTR_NULL);
    }

    if (MAC_LPM_STATE_GPIO_SLEEP == pst_set_para->uc_pm_switch)
    {
        st_para.bit_gpio_sleep_en = 1;
        hal_set_lpm_state(pst_hal_device,HAL_LPM_STATE_NORMAL_WORK,HAL_LPM_STATE_DEEP_SLEEP, &st_para, OAL_PTR_NULL);
    }

    if (MAC_LPM_STATE_WORK == pst_set_para->uc_pm_switch)
    {
        /*唤醒*/
        hal_set_lpm_state(pst_hal_device,HAL_LPM_STATE_DEEP_SLEEP,HAL_LPM_STATE_NORMAL_WORK,OAL_PTR_NULL, OAL_PTR_NULL);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_lpm_psm_param(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    mac_device_stru                  *pst_device;
    mac_cfg_lpm_psm_param_stru       *pst_psm_para;
    dmac_vap_stru                    *pst_dmac_vap;
    hal_to_dmac_vap_stru             *pst_hal_vap;

    /* 转化为DMAC VAP */
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    pst_hal_vap = pst_dmac_vap->pst_hal_vap;

	pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if(OAL_PTR_NULL == pst_hal_vap || OAL_PTR_NULL == pst_device)
    {
        return OAL_FAIL;
    }
    pst_psm_para = (mac_cfg_lpm_psm_param_stru*)puc_param;
    if(0 == pst_psm_para->uc_psm_on||1 == pst_psm_para->uc_psm_on)
    {
        hal_set_psm_listen_interval(pst_hal_vap, pst_psm_para->us_psm_listen_interval);
        hal_set_psm_tbtt_offset(pst_hal_vap, pst_psm_para->us_psm_tbtt_offset);
        hal_set_psm_wakeup_mode(pst_dmac_vap->pst_hal_device, pst_psm_para->uc_psm_wakeup_mode);
        hal_set_psm_status(pst_dmac_vap->pst_hal_device, pst_psm_para->uc_psm_on);

        hal_test_lpm_set_psm_en(pst_psm_para->uc_psm_on);
    }
    else
    {
        /*debug显示结果*/
        hal_test_lpm_psm_dump_record();
    }

    return OAL_SUCC;

}



OAL_STATIC oal_uint32  dmac_config_set_lpm_smps_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    oal_uint8                       uc_mode;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_set_lpm_smps_mode::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_mode = *puc_param;
    hal_set_smps_mode(pst_hal_device, uc_mode);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_lpm_smps_stub(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    mac_device_stru						 *pst_device;
    mac_cfg_lpm_smps_stub_stru           *pst_smps_stub;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_device)
    {
        return OAL_FAIL;
    }

    pst_smps_stub = (mac_cfg_lpm_smps_stub_stru*)puc_param;
    g_st_dmac_test_mng.st_lpm_smps_stub.uc_stub_type= pst_smps_stub->uc_stub_type;
    g_st_dmac_test_mng.st_lpm_smps_stub.uc_rts_en = pst_smps_stub->uc_rts_en;

    return OAL_SUCC;

}


OAL_STATIC oal_uint32  dmac_config_set_lpm_txop_ps(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    mac_cfg_lpm_txopps_set_stru* pst_txopps_set;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_set_lpm_txop_ps::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_txopps_set = (mac_cfg_lpm_txopps_set_stru*)puc_param;
    if((0 == pst_txopps_set->uc_txop_ps_on)||(1 == pst_txopps_set->uc_txop_ps_on))
    {
        hal_set_txop_ps_condition1(pst_hal_device, pst_txopps_set->uc_conditon1);
        hal_set_txop_ps_condition2(pst_hal_device, pst_txopps_set->uc_conditon2);
        hal_set_txop_ps_enable(pst_hal_device, pst_txopps_set->uc_txop_ps_on);

        dmac_test_lpm_txopps_en(pst_txopps_set->uc_txop_ps_on);
    }
    else
    {
        dmac_test_lpm_txopps_debug();
    }

    return OAL_SUCC;

}


OAL_STATIC oal_uint32  dmac_config_set_lpm_txop_ps_tx_stub(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    mac_device_stru						 *pst_device;
    mac_cfg_lpm_txopps_tx_stub_stru      *pst_txopps_tx_stub;
    dmac_lpm_txopps_tx_stub_stru         *pst_txop_stub;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_device)
    {
        return OAL_FAIL;
    }
    pst_txopps_tx_stub = (mac_cfg_lpm_txopps_tx_stub_stru*)puc_param;
    pst_txop_stub = &g_st_dmac_test_mng.st_lpm_txop_stub;
    pst_txop_stub->uc_stub_on = pst_txopps_tx_stub->uc_stub_on;
    pst_txop_stub->us_begin_num = pst_txopps_tx_stub->us_begin_num;
    pst_txop_stub->us_curr_num = 0;
    pst_txop_stub->us_partial_aid_real = 0;

    return OAL_SUCC;

}


OAL_STATIC oal_uint32  dmac_config_lpm_tx_probe_request(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                 *pst_mac_device ;
    dmac_vap_stru                   *pst_dmac_vap ;
    mac_cfg_lpm_tx_data_stru        *pst_lpm_tx_data ;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        return OAL_FAIL ;
    }

    if (0 == pst_mac_device->uc_vap_num)
    {
       OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_lpm_tx_probe_request::can't find existed vap to send probe_req.}");
       return OAL_FAIL ;
    }

   pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_device->auc_vap_id[0]);
   if (OAL_PTR_NULL == pst_dmac_vap)
   {
       OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_lpm_tx_probe_request::pst_dmac_vap null.}");

       return OAL_ERR_CODE_PTR_NULL;
   }

   pst_lpm_tx_data = (mac_cfg_lpm_tx_data_stru*)puc_param;

   dmac_test_lpm_send_probe_requst(pst_dmac_vap,pst_lpm_tx_data->uc_positive,pst_lpm_tx_data->auc_da);

   return OAL_SUCC ;
}


OAL_STATIC oal_uint32  dmac_config_remove_user_lut(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_remove_lut_stru *pst_param;    /* 复用删除用户结构体 */
    dmac_user_stru          *pst_dmac_user;


    pst_param = (mac_cfg_remove_lut_stru *)puc_param;

    pst_dmac_user = mac_res_get_dmac_user(pst_param->us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_remove_user_lut::pst_dmac_user[%d] null.}",
            pst_param->us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (1 == pst_param->uc_is_remove)
    {
        /* remove lut */
        dmac_user_inactive(pst_dmac_user);


    }
    else
    {
        /* resume lut */
        dmac_user_active(pst_dmac_user);
    }

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_GREEN_AP

OAL_STATIC oal_uint32 dmac_config_set_gap_free_ratio(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_free_ratio_set_stru      *pst_set_para;

    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_GREEN_AP, "{dmac_config_set_gap_free_ratio: pst_mac_vap NULL.}");
        return OAL_FAIL ;
    }

    pst_set_para = (mac_cfg_free_ratio_set_stru*)puc_param;

    dmac_green_ap_set_free_ratio(pst_mac_vap->uc_device_id, pst_set_para->en_mode, pst_set_para->uc_th_value);

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_CHIP_TEST

OAL_STATIC oal_uint32  dmac_config_set_rx_pn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    hal_to_dmac_device_stru        *pst_hal_device;
    mac_cfg_set_rx_pn_stru          *pst_rx_pn;
    dmac_user_stru                  *pst_dmac_usr;
    hal_pn_lut_cfg_stru             st_pn_lut_cfg;

    pst_rx_pn = (mac_cfg_set_rx_pn_stru *)puc_param;

    pst_dmac_usr = mac_res_get_dmac_user(pst_rx_pn->us_user_idx);
    if(OAL_PTR_NULL == pst_dmac_usr)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_rx_pn::pst_dmac_usr[%d] null.}", pst_rx_pn->us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_set_rx_pn::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_pn_lut_cfg.uc_pn_key_type = 1;
    st_pn_lut_cfg.uc_pn_peer_idx = pst_dmac_usr->uc_lut_index;
    st_pn_lut_cfg.uc_pn_tid      = 0;
    st_pn_lut_cfg.ul_pn_msb      = 0;
    st_pn_lut_cfg.ul_pn_lsb      = pst_rx_pn->us_rx_pn;
    hal_set_rx_pn(pst_hal_device,&st_pn_lut_cfg);
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_soft_retry(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_test_set_software_retry(puc_param);
	return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_open_addr4(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_test_open_addr4(puc_param);
	return OAL_SUCC;
}

OAL_STATIC oal_uint32  dmac_config_open_wmm_test(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_test_open_wmm_test(pst_mac_vap, *((oal_uint8 *)puc_param));
	return OAL_SUCC;
}



OAL_STATIC oal_uint32  dmac_config_chip_test_open(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_test_set_chip_test(*(oal_uint8*)puc_param);
	return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_coex(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_coex_ctrl_param_stru    *pst_coex_ctrl;
    hal_to_dmac_device_stru         *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_set_coex::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_coex_ctrl = (mac_cfg_coex_ctrl_param_stru *)puc_param;

    OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_coex::ul_mac_ctrl=%d ul_rf_ctrl=%d.}",
                  pst_coex_ctrl->ul_mac_ctrl, pst_coex_ctrl->ul_rf_ctrl);

    hal_set_coex_ctrl(pst_hal_device, pst_coex_ctrl->ul_mac_ctrl, pst_coex_ctrl->ul_rf_ctrl);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_dfx(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int32       l_value;

    l_value = *((oal_int32 *)puc_param);

    g_st_dmac_test_mng.en_cfg_tx_cnt = (oal_switch_enum_uint8)l_value;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_send_pspoll(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_netbuf_stru         *pst_netbuf;
    oal_uint8               *puc_data;
    mac_tx_ctl_stru         *pst_tx_cb;
    oal_uint32               ul_ret;

    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf))
    {
       OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_pspoll::alloc netbuff failed.");
       return OAL_PTR_NULL;
    }

    oal_netbuf_put(pst_netbuf, 16);

    puc_data = OAL_NETBUF_HEADER(pst_netbuf);

    /* frame control,发往DS的ps-poll */
    puc_data[0] = 0xA4;
    puc_data[1] = 0x01;

    /* AID */
    puc_data[2] = (oal_uint8)pst_mac_vap->us_sta_aid;
    puc_data[3] = 0xC0;

    /* BSSID */
    oal_set_mac_addr(&puc_data[4], pst_mac_vap->auc_bssid);

    /* Transmitter address */
    oal_set_mac_addr(&puc_data[10], mac_mib_get_StationID(pst_mac_vap));

    oal_set_netbuf_next(pst_netbuf, OAL_PTR_NULL);
    oal_set_netbuf_prev(pst_netbuf, OAL_PTR_NULL);

    pst_tx_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    OAL_MEMZERO(pst_tx_cb, OAL_NETBUF_CB_SIZE());
    MAC_GET_CB_TX_USER_IDX(pst_tx_cb)   = pst_mac_vap->us_assoc_vap_id;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_cb) = WLAN_WME_AC_MGMT;
    MAC_GET_CB_TX_VAP_INDEX(pst_tx_cb)           = pst_mac_vap->uc_vap_id;
    MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_cb)          = WLAN_CONTROL;
    MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_cb)   = OAL_TRUE;
    MAC_SET_CB_FRAME_HEADER_ADDR(pst_tx_cb, (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf));
    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_cb)    = 16;
    MAC_GET_CB_MPDU_NUM(pst_tx_cb)               = 1;
    MAC_GET_CB_NETBUF_NUM(pst_tx_cb)             = 1;
    MAC_GET_CB_MPDU_LEN(pst_tx_cb)               = 0;

    ul_ret = dmac_tx_mgmt(MAC_GET_DMAC_VAP(pst_mac_vap), pst_netbuf, 16);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_pspoll::dmac_tx_mgmt failed[%d].", ul_ret);
        oal_netbuf_free(pst_netbuf);
        return ul_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_send_nulldata(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_netbuf_stru                 *pst_net_buf;
    mac_tx_ctl_stru                 *pst_tx_ctrl;
    oal_uint32                       ul_ret;
    oal_uint16                       us_frame_ctl_second_byte = 0;
    mac_user_stru                   *pst_user;
    oal_uint8                       *puc_frame;
    mac_ieee80211_qos_frame_stru    *pst_qos_header;
    mac_cfg_tx_nulldata_stru        *pst_tx_nulldata;

    /* 申请net_buff */
    pst_net_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_net_buf)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_nulldata::pst_net_buf null.}");

        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_tx_nulldata = (mac_cfg_tx_nulldata_stru *)puc_param;

    oal_set_netbuf_prev(pst_net_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_net_buf, OAL_PTR_NULL);

    if (pst_tx_nulldata->l_is_qos)
    {
        oal_netbuf_put(pst_net_buf, OAL_SIZEOF(mac_ieee80211_qos_frame_stru));
    }
    else
    {
        oal_netbuf_put(pst_net_buf, 24);
    }

    puc_frame = OAL_NETBUF_HEADER(pst_net_buf);

    if (0 == pst_tx_nulldata->l_is_psm)
    {
        us_frame_ctl_second_byte = 0x0100;
    }
    else
    {
        us_frame_ctl_second_byte = 0x1100;
    }

    pst_user = mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id);
    if (OAL_PTR_NULL == pst_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_nulldata::pst_user[%d] null.}", pst_mac_vap->us_assoc_vap_id);
        dmac_tx_excp_free_netbuf(pst_net_buf);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写帧头,其中from ds为1，to ds为0，因此frame control的第二个字节为02 */
    if (pst_tx_nulldata->l_is_qos)
    {
        mac_hdr_set_frame_control(puc_frame, (oal_uint16)(WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_QOS_NULL) | us_frame_ctl_second_byte);
    }
    else
    {
        mac_hdr_set_frame_control(puc_frame, (oal_uint16)(WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_NODATA) | us_frame_ctl_second_byte);
    }

    if (pst_tx_nulldata->l_is_qos)
    {
        pst_qos_header = (mac_ieee80211_qos_frame_stru *)puc_frame;
        pst_qos_header->bit_qc_tid = (oal_uint8)pst_tx_nulldata->l_tidno;
        pst_qos_header->bit_qc_eosp = 1;
        pst_qos_header->bit_qc_ack_polocy = WLAN_TX_NORMAL_ACK;
    }

    /* BSSID */
    oal_set_mac_addr(&puc_frame[4], pst_user->auc_user_mac_addr);

    /* SA */
    oal_set_mac_addr(&puc_frame[10], mac_mib_get_StationID(pst_mac_vap));

    /* DA */
    oal_set_mac_addr(&puc_frame[16], pst_user->auc_user_mac_addr);

    /* 填写cb字段 */
    pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
    OAL_MEMZERO(pst_tx_ctrl, OAL_SIZEOF(mac_tx_ctl_stru));

    /* 填写tx部分 */
    MAC_GET_CB_ACK_POLACY(pst_tx_ctrl)            = WLAN_TX_NORMAL_ACK;
    MAC_GET_CB_EVENT_TYPE(pst_tx_ctrl)            = FRW_EVENT_TYPE_WLAN_DTX;

    if (pst_tx_nulldata->l_is_qos)
    {
        MAC_GET_CB_WME_AC_TYPE(pst_tx_ctrl) = WLAN_WME_TID_TO_AC(pst_tx_nulldata->l_tidno);
    }
    else
    {
        MAC_GET_CB_WME_AC_TYPE(pst_tx_ctrl) = WLAN_WME_AC_BE;
    }

    if (pst_tx_nulldata->l_is_qos)
    {
        MAC_GET_CB_WME_TID_TYPE(pst_tx_ctrl)= (oal_uint8)pst_tx_nulldata->l_tidno;
    }
    else
    {
        MAC_GET_CB_WME_TID_TYPE(pst_tx_ctrl)= WLAN_TID_FOR_DATA;
    }

    MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctrl)          = pst_mac_vap->uc_vap_id;
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctrl)           = pst_mac_vap->us_assoc_vap_id;
    MAC_GET_CB_RETRIED_NUM(pst_tx_ctrl)          = 0;

    /* 填写tx rx公共部分 */
    MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_ctrl)         = OAL_TRUE;
    MAC_GET_CB_IS_4ADDRESS(pst_tx_ctrl) = OAL_FALSE;
    MAC_SET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl, (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_net_buf));
    if (pst_tx_nulldata->l_is_qos)
    {
        MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl)     = OAL_SIZEOF(mac_ieee80211_qos_frame_stru);
    }
    else
    {
        MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctrl)     = OAL_SIZEOF(mac_ieee80211_frame_stru);
    }

    MAC_GET_CB_MPDU_NUM(pst_tx_ctrl)                = 1;
    MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl)              = 1;
    MAC_GET_CB_MPDU_LEN(pst_tx_ctrl)                = 0;

    ul_ret = dmac_tx_process_data(MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_device, (dmac_vap_stru *)pst_mac_vap, pst_net_buf);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_nulldata::dmac_tx_process_data failed[%d].}", ul_ret);

        dmac_tx_excp_free_netbuf(pst_net_buf);
        return ul_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_clear_all_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_clear_all_stat::pst_hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 清除中断统计信息 */
    hal_clear_irq_stat(pst_hal_device);

    return OAL_SUCC;
}
#endif /* CHIP_TEST */

OAL_STATIC oal_uint32  dmac_config_get_fem_pa_status(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_atcmdsrv_atcmd_response_event     st_atcmdsrv_fem_pa_info;
    oal_uint32                             ul_cali_check_fem_pa_status = 0;
//#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    hal_to_dmac_device_stru               *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_get_fem_pa_status::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hal_get_hw_status(pst_hal_device,&ul_cali_check_fem_pa_status);
//#endif
    st_atcmdsrv_fem_pa_info.uc_event_id = OAL_ATCMDSRV_FEM_PA_INFO_EVENT;
    st_atcmdsrv_fem_pa_info.ul_event_para = ul_cali_check_fem_pa_status;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHECK_FEM_PA, OAL_SIZEOF(dmac_atcmdsrv_atcmd_response_event), (oal_uint8 *)&st_atcmdsrv_fem_pa_info);
    return OAL_SUCC;
}

#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) && defined(_PRE_WLAN_FEATURE_SMPS)&& defined(_PRE_WLAN_FEATURE_M2S)

OAL_STATIC oal_uint32  dmac_config_set_m2s_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_m2s_mgr_stru           *pst_m2s_mgr;
    hal_to_dmac_device_stru    *pst_hal_device;
    oal_uint16                  us_switch_type = HAL_M2S_EVENT_BUTT;
    wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode;

    pst_m2s_mgr = (mac_m2s_mgr_stru*)puc_param;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_config_set_m2s_switch::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    switch(pst_m2s_mgr->en_cfg_m2s_mode)
    {
        /* mimo-siso切换参数查询 */
        case MAC_M2S_MODE_QUERY:
            dmac_m2s_mgr_param_info(pst_hal_device);
            dmac_m2s_show_blacklist_in_list(pst_hal_device);
            break;

        /* 软切换和硬切换测试模式(前面已经提前抛事件做了参数更新)，这里开始运用切换管理模块统一来做 */
        case MAC_M2S_MODE_SW_TEST:
        case MAC_M2S_MODE_HW_TEST:
        case MAC_M2S_MODE_DELAY_SWITCH:
            /* 标识业务类型 */
            if(MAC_M2S_MODE_DELAY_SWITCH == pst_m2s_mgr->en_cfg_m2s_mode)
            {
                uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_COMMAND;
            }
            else
            {
                uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_TEST;
            }

            /* 0.刷新软硬件切换能力 */
            pst_hal_device->st_hal_m2s_fsm.en_m2s_type = pst_m2s_mgr->pri_data.test_mode.en_m2s_type;

            /* 1.期望切换到siso */
            if(WLAN_SINGLE_NSS == pst_m2s_mgr->pri_data.test_mode.en_m2s_nss)
            {
                /* 期望切换到c0 siso */
                if(WLAN_PHY_CHAIN_ZERO == pst_m2s_mgr->pri_data.test_mode.uc_chain)
                {
                    /* 当前是c1 siso  切换到c0 siso */
                    if((WLAN_SINGLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                        (WLAN_PHY_CHAIN_ONE == pst_hal_device->st_cfg_cap_info.uc_phy_chain)&&
                        (WLAN_PHY_CHAIN_ONE == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain))
                    {
                        /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
                        if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
                        {
                            us_switch_type = HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0;
                        }
                        else
                        {
                            us_switch_type = HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0;
                        }
                    }
                    /* 当前是mimo 切换c0 siso */
                    else if((WLAN_DOUBLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                        (WLAN_PHY_CHAIN_DOUBLE == pst_hal_device->st_cfg_cap_info.uc_phy_chain))
                    {
                        /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
                        if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
                        {
                            us_switch_type = HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0;
                        }
                        else
                        {
                            us_switch_type = HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0;
                        }
                    }
                    /* 当前是c1 miso 切换c0 siso 主要是指延迟切换 */
                    else if((WLAN_SINGLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                        (WLAN_PHY_CHAIN_DOUBLE == pst_hal_device->st_cfg_cap_info.uc_phy_chain)&&
                        (WLAN_PHY_CHAIN_ONE == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain))
                    {
                        /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
                        if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
                        {
                            /* 1.说明此时miso是稳态，切换也只能稳定切换，关闭了切换保护
                               2.miso过渡态，还没完成10帧探测，又来了切换操作，此时切换保护继续执行即可 */
                            us_switch_type = HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0;
                        }
                    }
                    else
                    {
                        OAM_WARNING_LOG_ALTER(0, OAM_SF_M2S, "{dmac_config_set_m2s_switch: state the same expert nss[%d] uc_chain[%d], real cur_state[%d]nss[%d]phy2dscr_chain[%d]phy_chain[%d]!}",
                                   6, pst_m2s_mgr->pri_data.test_mode.en_m2s_nss, pst_m2s_mgr->pri_data.test_mode.uc_chain,
                                   GET_HAL_M2S_CUR_STATE(pst_hal_device), pst_hal_device->st_cfg_cap_info.en_nss_num,
                                   pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain, pst_hal_device->st_cfg_cap_info.uc_phy_chain);

                        /* 状态不变的话，仍然可以做业务，直接抛状态给m2s，只是状态不变，不做处理，只是置业务标记 */
                        if(WLAN_M2S_TRIGGER_MODE_BUTT != uc_trigger_mode)
                        {
                            GET_HAL_M2S_MODE_TPYE(pst_hal_device) |= uc_trigger_mode;
                        }

                        return OAL_FAIL;
                    }
                }
                /* 期望切换到c1 siso */
                else if(WLAN_PHY_CHAIN_ONE == pst_m2s_mgr->pri_data.test_mode.uc_chain)
                {
                    /* 当前是c0 siso  切换到c1 siso */
                    if((WLAN_SINGLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                        (WLAN_PHY_CHAIN_ZERO == pst_hal_device->st_cfg_cap_info.uc_phy_chain)&&
                        (WLAN_PHY_CHAIN_ZERO == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain))
                    {
                        /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
                        if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
                        {
                            us_switch_type = HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1;
                        }
                        else
                        {
                            us_switch_type = HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1;
                        }
                    }
                    /* 当前是mimo 切换到c1 siso */
                    else if((WLAN_DOUBLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                        (WLAN_PHY_CHAIN_DOUBLE == pst_hal_device->st_cfg_cap_info.uc_phy_chain))
                    {
                        if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
                        {
                            us_switch_type = HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1;
                        }
                        else
                        {
                            us_switch_type = HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1;
                        }
                    }
                    /* 当前是c0 miso 切换c1 siso 主要是指延迟切换 */
                    else if((WLAN_SINGLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                        (WLAN_PHY_CHAIN_DOUBLE == pst_hal_device->st_cfg_cap_info.uc_phy_chain)&&
                        (WLAN_PHY_CHAIN_ZERO == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain))
                    {
                        /* 1.说明此时miso是稳态，切换也只能稳定切换，关闭了切换保护
                           2.miso过渡态，还没完成10帧探测，又来了切换操作，此时切换保护继续执行即可 */
                        us_switch_type = HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1;
                    }
                    else
                    {
                        OAM_WARNING_LOG_ALTER(0, OAM_SF_M2S, "{dmac_config_set_m2s_switch: state the same expert nss[%d] uc_chain[%d], real cur_state[%d]nss[%d]phy2dscr_chain[%d]phy_chain[%d]!}",
                                   6, pst_m2s_mgr->pri_data.test_mode.en_m2s_nss, pst_m2s_mgr->pri_data.test_mode.uc_chain,
                                   GET_HAL_M2S_CUR_STATE(pst_hal_device), pst_hal_device->st_cfg_cap_info.en_nss_num,
                                   pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain, pst_hal_device->st_cfg_cap_info.uc_phy_chain);

                        /* 状态不变的话，仍然可以做业务，直接抛状态给m2s，只是状态不变，不做处理，只是置业务标记 */
                        if(WLAN_M2S_TRIGGER_MODE_BUTT != uc_trigger_mode)
                        {
                            GET_HAL_M2S_MODE_TPYE(pst_hal_device) |= uc_trigger_mode;
                        }

                        return OAL_FAIL;
                    }
                }
            }
            /* 期望切换到mimo */
            else
            {
                /* 当前是c0 siso 切换到mimo */
                if((WLAN_SINGLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                    (WLAN_PHY_CHAIN_ZERO == pst_hal_device->st_cfg_cap_info.uc_phy_chain)&&
                    (WLAN_PHY_CHAIN_ZERO == pst_hal_device->st_cfg_cap_info.uc_phy_chain))
                {
                    /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
                    if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
                    {
                        us_switch_type = HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO;
                    }
                    else
                    {
                        us_switch_type = HAL_M2S_EVENT_TEST_SISO_C0_TO_MIMO;
                    }
                }
                /* 当前是c1 siso 切换到mimo */
                else if((WLAN_SINGLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                    (WLAN_PHY_CHAIN_ONE == pst_hal_device->st_cfg_cap_info.uc_phy_chain)&&
                    (WLAN_PHY_CHAIN_ONE == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain))
                {
                    /* command方式切换，如果存在黑名单用户，切换方案直接修订为切到siso */
                    if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
                    {
                        us_switch_type = HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO;
                    }
                    else
                    {
                        us_switch_type = HAL_M2S_EVENT_TEST_SISO_C1_TO_MIMO;
                    }
                }
                /* 当前是c1 miso 切换到mimo */
                else if((WLAN_SINGLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                    (WLAN_PHY_CHAIN_DOUBLE == pst_hal_device->st_cfg_cap_info.uc_phy_chain)&&
                    (WLAN_PHY_CHAIN_ONE == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain))
                {
                    us_switch_type = HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO;
                }
                /* 当前是c0 miso 切换到mimo */
                else if((WLAN_SINGLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num)&&
                 (WLAN_PHY_CHAIN_DOUBLE == pst_hal_device->st_cfg_cap_info.uc_phy_chain)&&
                 (WLAN_PHY_CHAIN_ZERO == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain))
                {
                    us_switch_type = HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO;
                }
                else
                {
                    OAM_WARNING_LOG_ALTER(0, OAM_SF_M2S, "{dmac_config_set_m2s_switch: state the same expert nss[%d] uc_chain[%d], real cur_state[%d]nss[%d]phy2dscr_chain[%d]phy_chain[%d]!}",
                               6, pst_m2s_mgr->pri_data.test_mode.en_m2s_nss, pst_m2s_mgr->pri_data.test_mode.uc_chain,
                               GET_HAL_M2S_CUR_STATE(pst_hal_device), pst_hal_device->st_cfg_cap_info.en_nss_num,
                               pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain, pst_hal_device->st_cfg_cap_info.uc_phy_chain);

                    /* 状态不变的话，仍然可以做业务，直接抛状态给m2s，只是状态不变，不做处理，只是置业务标记 */
                    if(WLAN_M2S_TRIGGER_MODE_BUTT != uc_trigger_mode)
                    {
                        GET_HAL_M2S_MODE_TPYE(pst_hal_device) &= (~uc_trigger_mode);
                    }

                    return OAL_FAIL;
                }
            }

            /* 满足切换条件，进行切换跳转 */
            if(OAL_TRUE == dmac_m2s_switch_apply_and_confirm(pst_hal_device, us_switch_type, uc_trigger_mode))
            {
                dmac_m2s_handle_event(pst_hal_device, us_switch_type, 0, OAL_PTR_NULL);
            }

            break;

         case MAC_M2S_MODE_RSSI:
             OAM_WARNING_LOG3(0, OAM_SF_M2S, "{dmac_config_set_m2s_switch: cfg_m2s_mode[%d] item[%d] val[%d]!}",
                        pst_m2s_mgr->en_cfg_m2s_mode, pst_m2s_mgr->pri_data.rssi_snr_mode.uc_ch, pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value);
             switch(pst_m2s_mgr->pri_data.rssi_snr_mode.uc_ch)
             {
                 case HAL_DEV_RSSI_SWITCH_EN:
                     pst_hal_device->st_rssi.st_rx_rssi.en_ant_rssi_sw = (oal_uint8)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_RSSI_HIGH_TH:
                     pst_hal_device->st_rssi.st_rx_rssi.uc_rssi_high = (oal_uint8)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_RSSI_LOW_TH:
                     pst_hal_device->st_rssi.st_rx_rssi.uc_rssi_low = (oal_uint8)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_RSSI_HIGH_TH_CNT:
                     pst_hal_device->st_rssi.st_rx_rssi.uc_rssi_high_cnt_th = (oal_uint16)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_RSSI_LOW_TH_CNT:
                     pst_hal_device->st_rssi.st_rx_rssi.uc_rssi_low_cnt_th = (oal_uint16)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_RSSI_LOG_PRINT:
                     pst_hal_device->st_rssi.st_rx_rssi.en_log_print = (oal_uint8)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;

                 default:
                 break;
             }
             break;
         case MAC_M2S_MODE_SNR:
             OAM_WARNING_LOG3(0, OAM_SF_M2S, "{dmac_config_set_m2s_switch: cfg_m2s_mode[%d] item[%d] val[%d]!}",
                        pst_m2s_mgr->en_cfg_m2s_mode, pst_m2s_mgr->pri_data.rssi_snr_mode.uc_ch, pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value);
             switch(pst_m2s_mgr->pri_data.rssi_snr_mode.uc_ch)
             {
                 case HAL_DEV_SNR_SWITCH_EN:
                     pst_hal_device->st_rssi.st_rx_snr.en_ant_snr_sw = (oal_uint8)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_SNR_HIGH_TH:
                     pst_hal_device->st_rssi.st_rx_snr.uc_snr_high = (oal_uint8)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_SNR_LOW_TH:
                     pst_hal_device->st_rssi.st_rx_snr.uc_snr_low = (oal_uint8)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_SNR_HIGH_TH_CNT:
                     pst_hal_device->st_rssi.st_rx_snr.uc_snr_high_cnt_th = (oal_uint16)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_SNR_LOW_TH_CNT:
                     pst_hal_device->st_rssi.st_rx_snr.uc_snr_low_cnt_th = (oal_uint16)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;
                 case HAL_DEV_SNR_LOG_PRINT:
                     pst_hal_device->st_rssi.st_rx_snr.en_log_print = (oal_uint8)pst_m2s_mgr->pri_data.rssi_snr_mode.ul_value;
                 break;

                 default:
                 break;
             }
             break;

         default:
            OAM_WARNING_LOG1(0, OAM_SF_M2S, "{dmac_config_set_m2s_switch: en_cfg_m2s_mode[%d] error!}", pst_m2s_mgr->en_cfg_m2s_mode);
            return OAL_FAIL;
    }

    return OAL_SUCC;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_uint32   dmac_config_resume_rx_intr_fifo(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_resume_rx_intr_fifo_stru   *pst_param;
    hal_to_dmac_device_stru            *pst_hal_device;

    pst_param = (mac_cfg_resume_rx_intr_fifo_stru *)puc_param;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_resume_rx_intr_fifo::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device->en_rx_intr_fifo_resume_flag = pst_param->uc_is_on;

    return OAL_SUCC;
}
#endif

#ifdef  _PRE_DEBUG_MODE
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

oal_void dmac_config_get_all_reg_value_work(oal_work_stru *pst_work)
{
    mac_vap_stru        *pst_mac_vap;
    mac_device_stru     *pst_mac_device;
    dmac_device_stru    *pst_dmac_device;
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                   uc_hipriv_ack = OAL_FALSE;
#endif
    pst_mac_vap = OAL_CONTAINER_OF(pst_work, mac_vap_stru, reg_file_work);
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_all_reg_value_work:pst_mac_device null.}");
        return;
    }
    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if(OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_all_reg_value_work:null dev ptr, id=%d}", pst_mac_device->uc_device_id);
        return;
    }

    /* 读取寄存器的值 */
    hal_get_all_reg_value(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device));

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif
}
#endif

OAL_STATIC oal_uint32  dmac_config_get_all_reg_value(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_device_stru                *pst_device;
#else
    hal_to_dmac_device_stru        *pst_hal_device;
#endif

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1151)
    oal_uint8                   uc_hipriv_ack = OAL_FALSE;
#endif
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_all_reg_value:pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* hi1151读取所有寄存器时有进行文件操作，需要放到工作队列中进行 */
    OAL_INIT_WORK(&pst_mac_vap->reg_file_work, dmac_config_get_all_reg_value_work);
    oal_workqueue_schedule(&(pst_mac_vap->reg_file_work));
#else
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_all_reg_value::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 读取寄存器的值 */
    hal_get_all_reg_value(pst_hal_device);

    /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_get_cali_data(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_get_cali_data::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取校准数据 */
    hal_get_cali_data(pst_hal_device);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_report_ampdu_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_ampdu_stat_stru     *pst_ampdu_stat;
    dmac_user_stru              *pst_dmac_user;
    dmac_tid_stru               *pst_tid;

    pst_ampdu_stat = (mac_cfg_ampdu_stat_stru *)puc_param;

    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_ampdu_stat->auc_user_macaddr);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_report_ampdu_stat:: user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取tid，如果tid_no为8，代表对所有tid进行操作 */
    if (pst_ampdu_stat->uc_tid_no > WLAN_TID_MAX_NUM)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_config_report_ampdu_stat::tid_no invalid, tid_no = [%d]}", pst_ampdu_stat->uc_tid_no);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (WLAN_TID_MAX_NUM == pst_ampdu_stat->uc_tid_no)
    {
        return dmac_dft_report_all_ampdu_stat(pst_dmac_user, pst_ampdu_stat->uc_param);
    }
    else
    {
        pst_tid = &pst_dmac_user->ast_tx_tid_queue[pst_ampdu_stat->uc_tid_no];

        return dmac_dft_report_ampdu_stat(pst_tid, pst_ampdu_stat->auc_user_macaddr, pst_ampdu_stat->uc_param);
    }
}
#endif
#ifdef _PRE_DEBUG_MODE

OAL_STATIC oal_uint32  dmac_config_freq_adjust(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_freq_adjust_stru       *pst_freq_adjust;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_freq_adjust::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_freq_adjust = (mac_cfg_freq_adjust_stru *)puc_param;

    hal_freq_adjust(pst_hal_device, pst_freq_adjust->us_pll_int, pst_freq_adjust->us_pll_frac);

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_P2P

OAL_STATIC oal_uint32  dmac_config_set_p2p_ps_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_p2p_stat_param_stru *pst_p2p_stat;
    hal_to_dmac_device_stru     *pst_hal_device;
    pst_p2p_stat = (mac_cfg_p2p_stat_param_stru *)puc_param;
    //OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_set_p2p_ps_stat::ctrl:%d\r\n",
    //                pst_p2p_stat->uc_p2p_statistics_ctrl);
    /* 获取hal_device 结构体 */
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_p2p_ps_stat::hal dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_p2p_stat->uc_p2p_statistics_ctrl == 0)
    {
        /* 清除统计值 */
        hal_clear_irq_stat(pst_hal_device);
    }
    else if (pst_p2p_stat->uc_p2p_statistics_ctrl == 1)
    {
        /* 打印统计值 */
        hal_show_irq_info(pst_hal_device, 0);
    }
    else
    {
        /* 错误控制命令 */
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_set_p2p_ps_stat:: wrong p2p ps ctrl vale \r\n");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST

OAL_STATIC oal_uint32 dmac_config_send_cw_signal(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8
*puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;
    hal_to_dmac_device_stru         *pst_hal_device;
    wlan_channel_band_enum_uint8     en_band;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_cw_signal::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
                {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_send_cw_signal::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 写测试寄存器与读测试结果寄存器，检查寄存器配置是否生效*/
    en_band = pst_mac_vap->st_channel.en_band;

    /*0: 关闭单音*/
    if ( 0 == *puc_param )
    {
      hal_revert_cw_signal_reg(pst_hal_device, en_band);
    }
    /*1: 通道0单音*/
    else if ( 1 == *puc_param )
    {
      hal_cfg_cw_signal_reg(pst_hal_device, 0, en_band);
    }
    /*2: 通道1单音*/
    else if ( 2 == *puc_param )
    {
      hal_cfg_cw_signal_reg(pst_hal_device, 1, en_band);
    }
    /*3: 读相关寄存器*/
    else if ( 3 == *puc_param )
    {
      hal_get_cw_signal_reg(pst_hal_device, 0, en_band);
      hal_get_cw_signal_reg(pst_hal_device, 1, en_band);
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS

OAL_STATIC oal_uint32  dmac_config_set_psd(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru     *pst_hal_device = OAL_PTR_NULL;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_psd::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (0 == *puc_param)
    {
        hal_set_psd_en(pst_hal_device, OAL_FALSE);
    }
    else if (1 == *puc_param)
    {
        hal_set_psd_en(pst_hal_device, OAL_TRUE);
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32  dmac_config_cfg_psd(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru     *pst_hal_device = OAL_PTR_NULL;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_psd::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_set_psd_nb_det_en(pst_hal_device, 1);
    hal_set_psd_11b_det_en(pst_hal_device, 1);
    hal_set_psd_ofdm_det_en(pst_hal_device, 1);
    /* 1:业务场景，0: 非业务场景 默认为业务场景 */
    hal_set_psd_wifi_work_en(pst_hal_device, 1);
    hal_set_force_reg_clk_on(pst_hal_device, 1);
    hal_set_sync_data_path_div_num(pst_hal_device, 0);
    hal_set_psd_the_num_nb(pst_hal_device, 7);
    hal_set_psd_the_power_nb(pst_hal_device, 6);
    hal_set_psd_the_rssi_nb(pst_hal_device, -70);
    hal_set_psd_the_bottom_noise(pst_hal_device, -80);
    hal_set_up_fft_psd_en(pst_hal_device, 1);

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_CSI

OAL_STATIC oal_uint32  dmac_config_set_csi(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru     *pst_hal_device = OAL_PTR_NULL;
    mac_cfg_csi_param_stru      *pst_cfg_csi_param;
    oal_bool_enum_uint8          en_ta_check;
    oal_bool_enum_uint8          en_csi;
    //OAL_STATIC oal_uint32       *pul_start_addr = OAL_PTR_NULL;
    oal_uint32                   ul_reg_num = 0;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_csi::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_cfg_csi_param = (mac_cfg_csi_param_stru *)puc_param;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_cfg_csi_param))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_csi::pst_cfg_csi_param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    en_ta_check       = pst_cfg_csi_param->en_ta_check;
    en_csi            = pst_cfg_csi_param->en_csi;


    pst_hal_device->uc_csi_status = OAL_FALSE;

    hal_set_csi_en(pst_hal_device, OAL_FALSE);

    /* 使能固定TA上报CSI */
    if((!mac_addr_is_zero(pst_cfg_csi_param->auc_mac_addr)) && (OAL_TRUE == en_ta_check))
    {
       /* 设置上报csi帧的TA mac地址 */
       hal_set_csi_ta(pst_hal_device, pst_cfg_csi_param->auc_mac_addr);
       /* 设置mac使能CSI TA检查 */
       hal_set_csi_ta_check(pst_hal_device, en_ta_check);
       /* 使能CSI，每次写1，使能一次CSI的采集 */
       OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_csi::enable csi with check TA.}");
    }
    if(OAL_TRUE == en_csi)
    {
       /* 设置CSI数采长度，申请16K字节。默认4096，单位:32bit */
        ul_reg_num = 4096;

        /* 配置CSI数采信息 */
        hal_set_csi_memory(pst_hal_device, &ul_reg_num);

        /* 使能CSI，每次写1，使能一次CSI的采集 */
        hal_set_csi_en(pst_hal_device, OAL_TRUE);
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_csi::enable csi!.}");
    }
    else
    {
        hal_set_csi_en(pst_hal_device, OAL_FALSE);
        hal_disable_csi_sample(pst_hal_device);
        hal_free_csi_sample_mem(pst_hal_device);
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_csi::disable csi!.}");
    }


    return OAL_SUCC;
}
#endif

oal_uint32 dmac_config_set_auto_protection(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param)
{
    oal_uint8 uc_auto_protection_flag;

    uc_auto_protection_flag = (oal_uint8)*((oal_uint32 *)puc_param);

    return dmac_protection_set_autoprot(pst_mac_vap, uc_auto_protection_flag);
}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW

oal_uint32 dmac_config_set_ampdu_tx_hw_on(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8                       uc_snd_type;
    oal_uint8                       uc_enable_tx_hw = 0;
    mac_cfg_ampdu_tx_on_param_stru *pst_ampdu_tx_on;



    pst_ampdu_tx_on = (mac_cfg_ampdu_tx_on_param_stru*)puc_param;

    uc_enable_tx_hw = pst_ampdu_tx_on->uc_aggr_tx_on;
    uc_snd_type     = pst_ampdu_tx_on->uc_snd_type;

    /* 删建聚合切换方式 */
    if (AMPDU_SWITCH_BY_DEL_BA == pst_ampdu_tx_on->en_aggr_switch_mode)
    {
        dmac_ba_set_ampdu_ctrl(pst_mac_vap, uc_enable_tx_hw, uc_snd_type);
    }
    else
    {
        dmac_ba_tx_ampdu_switch(pst_mac_vap, uc_enable_tx_hw, uc_snd_type);
    }

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_ampdu_tx_hw_on::ba type[%d],enable[%d],snd partial[%d].}",
                          pst_ampdu_tx_on->en_aggr_switch_mode, uc_enable_tx_hw, uc_snd_type);

    return OAL_SUCC;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 dmac_config_set_pkts_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_set_tlv_stru          *pst_config_para;
    dmac_tx_pkts_stat_stru        *pst_pkts_stat;
    oal_uint32                     ul_durance;
    oal_uint32                     ul_rcv_mbits;
    oal_uint32                     ul_snd_mbits;
    oal_uint8                      uc_tidno;
    dmac_user_stru                *pst_dmac_user;
    oal_dlist_head_stru           *pst_entry;
    oal_dlist_head_stru           *pst_next_entry;
    mac_user_stru                 *pst_mac_user;
    oal_uint32                     ul_tid_mpdu = 0;
    dmac_tid_stru                 *pst_tid;

    pst_pkts_stat = &g_tx_pkts_stat;

    pst_config_para = (mac_cfg_set_tlv_stru*)puc_param;

    if (PKT_STAT_SET_ENABLE == pst_config_para->uc_cmd_type)
    {
        pst_pkts_stat->en_stat_en = (0 != pst_config_para->ul_value) ? OAL_TRUE : OAL_FALSE;
        if (OAL_TRUE == pst_pkts_stat->en_stat_en)
        {
            pst_pkts_stat->ul_rcv_pkts = 0;
            pst_pkts_stat->ul_snd_pkts = 0;
        }
        else
        {
            OAM_ERROR_LOG2(0, OAM_SF_CFG, "{dmac_config_set_pkts_stat::rcv pkts[%d],snd pkts[%d].}",
                 pst_pkts_stat->ul_rcv_pkts,pst_pkts_stat->ul_snd_pkts);
        }
    }
    else if (PKT_STAT_SET_START_STAT == pst_config_para->uc_cmd_type)
    {
        pst_pkts_stat->en_stat_rate_start = (0 != pst_config_para->ul_value) ? OAL_TRUE : OAL_FALSE;
        if (OAL_TRUE == pst_pkts_stat->en_stat_rate_start)
        {
            pst_pkts_stat->ul_rcv_pkts = 0;
            pst_pkts_stat->ul_snd_pkts = 0;
            pst_pkts_stat->ul_start_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();
        }
        else
        {
            ul_durance = (oal_uint32)OAL_TIME_GET_STAMP_MS();
            ul_durance -= pst_pkts_stat->ul_start_time;
            if (0 == ul_durance)
            {
                OAM_ERROR_LOG2(0, OAM_SF_CFG, "{dmac_config_set_pkts_stat::START TIME[%d],NOW TINE[%d].}",
                     pst_pkts_stat->ul_start_time,OAL_TIME_GET_STAMP_MS());
                return OAL_SUCC;
            }
            ul_rcv_mbits = ((pst_pkts_stat->ul_rcv_pkts * pst_pkts_stat->us_pkt_len) / ul_durance) >> 7;
            ul_snd_mbits = ((pst_pkts_stat->ul_snd_pkts * pst_pkts_stat->us_pkt_len) / ul_durance) >> 7;

            OAM_ERROR_LOG2(0, OAM_SF_CFG, "{dmac_config_set_pkts_stat::PKT LEN[%d],TIME[%d].}",
                 pst_pkts_stat->us_pkt_len,ul_durance);
            OAM_ERROR_LOG4(0, OAM_SF_CFG, "{dmac_config_set_pkts_stat::rcv rate[%d]Mbits,snd rate[%d]Mbits,rcv pkts[%d],snd pkts[%d].}",
                 ul_rcv_mbits,ul_snd_mbits, pst_pkts_stat->ul_rcv_pkts, pst_pkts_stat->ul_snd_pkts);

            OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_next_entry, &(pst_mac_vap->st_mac_user_list_head))
            {
                pst_mac_user      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

                pst_dmac_user = mac_res_get_dmac_user(pst_mac_user->us_assoc_id);
                if (OAL_PTR_NULL == pst_dmac_user)
                {
                    continue;
                }

                for (uc_tidno = 0; uc_tidno < WLAN_TID_MAX_NUM; uc_tidno++)
                {
                    pst_tid = &pst_dmac_user->ast_tx_tid_queue[uc_tidno];
                    ul_tid_mpdu += (pst_tid->us_mpdu_num - pst_tid->uc_retry_num);
                }
            }

            OAM_ERROR_LOG3(0, OAM_SF_CFG, "{dmac_config_set_pkts_stat::rcv pkts[%d],snd pkts[%d],tid pkts[%d].}",
               pst_pkts_stat->ul_rcv_pkts, pst_pkts_stat->ul_snd_pkts, ul_tid_mpdu);
        }
    }
    else if (PKT_STAT_SET_FRAME_LEN == pst_config_para->uc_cmd_type)
    {
        pst_pkts_stat->us_pkt_len = (oal_uint16)pst_config_para->ul_value;
    }

    return OAL_SUCC;
}


oal_uint32 dmac_config_set_mem_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if 0
    mac_cfg_set_tlv_stru    *pst_config_para;
    oal_mem_free_debug_stat *pst_mem_stat;

    pst_config_para = (mac_cfg_set_tlv_stru*)puc_param;

    pst_mem_stat = oal_mem_buf_free_get_para();

    switch (pst_config_para->uc_cmd_type)
    {
        case OAL_SET_ENABLE:
            pst_mem_stat->en_enable = (pst_config_para->ul_value > 0) ? OAL_TRUE : OAL_FALSE;
            break;
        case OAL_SET_MAX_BUFF:
            pst_mem_stat->ul_max_buff_num = pst_config_para->ul_value;
            break;
        case OAL_SET_THRESHOLD:
            pst_mem_stat->uc_threshold = pst_config_para->ul_value;
            break;
        case OAL_SET_LOG_PRINT:
            OAM_ERROR_LOG3(0, OAM_SF_CFG, "{dmac_config_set_mem_stat::short avg free mem[%d],long avg free mem[%d],thr[%d].}",
                      pst_mem_stat->ul_short_avg_buf, pst_mem_stat->ul_long_avg_buf, pst_mem_stat->uc_threshold);
            OAM_ERROR_LOG4(0, OAM_SF_CFG, "{dmac_config_set_mem_stat::shortcnt[%d],shortstat[%d],longcnt[%d], longstat[%d].}",
                      pst_mem_stat->uc_short_suspend_cnt, pst_mem_stat->ul_cur_buf_free_stat, pst_mem_stat->uc_long_suspend_cnt, pst_mem_stat->ul_long_buf_free_stat);
            break;
        case OAL_SET_CLEAR:
            OAL_MEMZERO(pst_mem_stat, OAL_SIZEOF(oal_mem_free_debug_stat));
            break;


        default:
            break;
    }

    OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_config_set_mem_stat::SET CMD[%d],SET VAL[%d].}",
              pst_config_para->uc_cmd_type, pst_config_para->ul_value);
#endif
    return OAL_SUCC;
}

oal_uint32 dmac_config_set_txbf_cap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru           *pst_mac_device = OAL_PTR_NULL;
    oal_bool_enum_uint8        uc_value       = 0;
    oal_bool_enum_uint8        en_rx_switch   = OAL_FALSE;
    oal_bool_enum_uint8        en_tx_switch   = OAL_FALSE;
    oal_uint8                  uc_rx_sts_num  = 0;

    uc_value = *puc_param;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap->pst_mib_info))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_txbf_cap::pst_mac_vap->pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    en_rx_switch = uc_value & 0x1;             //rx cap
    en_tx_switch = (uc_value & 0x2) >> 1;      //tx cap
    uc_rx_sts_num = (en_rx_switch & OAL_TRUE) ? VHT_BFEE_NTX_SUPP_STS_CAP : 1;   //support max Nsts

    /* siso能力时，配置命令不打开txbf的TX能力 */
    if (pst_mac_vap->en_vap_rx_nss >= WLAN_DOUBLE_NSS)
    {
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
        mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, en_tx_switch);
        pst_mac_vap->st_txbf_add_cap.bit_exp_comp_txbf_cap = en_tx_switch;
#endif
        mac_mib_set_VHTSUBeamformerOptionImplemented(pst_mac_vap, en_tx_switch);
        mac_mib_set_VHTNumberSoundingDimensions(pst_mac_vap, MAC_DEVICE_GET_NSS_NUM(pst_mac_device));
    }
    else
    {
        mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_VHTSUBeamformerOptionImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_VHTNumberSoundingDimensions(pst_mac_vap, WLAN_SINGLE_NSS);
    }

#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, en_rx_switch);
    mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, uc_rx_sts_num);
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, en_rx_switch & WLAN_MIB_HT_ECBF_DELAYED);
    pst_mac_vap->st_txbf_add_cap.bit_channel_est_cap = en_rx_switch;
#endif

    mac_mib_set_VHTSUBeamformeeOptionImplemented(pst_mac_vap, en_rx_switch);
    mac_mib_set_VHTBeamformeeNTxSupport(pst_mac_vap, uc_rx_sts_num);
#if (WLAN_MU_BFEE_ACTIVED == WLAN_MU_BFEE_ENABLE)
    mac_mib_set_VHTMUBeamformeeOptionImplemented(pst_mac_vap, en_rx_switch);
#endif

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_set_txbf_cap::rx_cap[%d], tx_cap[%d], rx_sts_nums[%d].}",
                                en_rx_switch, en_tx_switch, uc_rx_sts_num);

    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND

OAL_STATIC oal_uint32  dmac_config_user_extend_enable(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_chip_stru      *pst_mac_chip;

    pst_mac_chip = dmac_res_get_mac_chip(pst_mac_vap->uc_chip_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_chip))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_user_extend_enable::pst_mac_chip null, expect chip_id=%d.}",
            pst_mac_vap->uc_chip_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置DMAC侧开关 */
    pst_mac_chip->st_user_extend.en_flag = !!(*puc_param);

    return OAL_SUCC;
}
#endif /* #ifdef _PRE_WLAN_FEATURE_USER_EXTEND */

#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE

OAL_STATIC oal_uint32 dmac_config_packet_capture_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru         *pst_hal_device;
    dmac_device_stru                *pst_dmac_device;
    hal_to_dmac_vap_stru            *pst_hal_vap = OAL_PTR_NULL;

    pst_dmac_device = (dmac_device_stru *)dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_packet_capture_switch::pst_dmac_device is null, but device_id is [%d].}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 进行配置参数的检测, 如果重复配置了则直接返回 */
    if (*puc_param == (pst_dmac_device->st_pkt_capture_stat.uc_capture_switch + 2 * pst_dmac_device->st_pkt_capture_stat.en_report_sdt_switch))
    {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_packet_capture_switch:: unused cfg! *puc_param[%d], uc_capture_switch[%d], en_report_sdt_switch[%d].}",
            *puc_param, pst_dmac_device->st_pkt_capture_stat.uc_capture_switch, pst_dmac_device->st_pkt_capture_stat.en_report_sdt_switch);
        return OAL_SUCC;
    }
    /* 混合抓包和纯抓包模式之间不能直接转换 */
    if (((DMAC_PKT_CAP_MIX == *puc_param) && (DMAC_PKT_CAP_MONITOR == pst_dmac_device->st_pkt_capture_stat.uc_capture_switch)) || ((DMAC_PKT_CAP_MONITOR == *puc_param) && (DMAC_PKT_CAP_MIX == pst_dmac_device->st_pkt_capture_stat.uc_capture_switch)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_packet_capture_switch::can not switch between mix and monitor mode.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_hal_device = pst_dmac_device->past_hal_device[0];
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_packet_capture_switch::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 如果EXT AP11被使用了, 则开启抓包失败 */
    hal_get_hal_vap(pst_hal_device, WLAN_HAL_EXT_AP11_VAP_ID, &pst_hal_vap);
    if (OAL_PTR_NULL != pst_hal_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_packet_capture_switch::provide tsf's ext ap11 is being used.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (0 == *puc_param)
    {
        /* 配置硬件寄存器关闭 */
        hal_packet_capture_switch_reg(pst_hal_device, DMAC_PKT_CAP_NORMAL);

        /* 抓包标识位置为0, 即正常模式; 上报标识位置为OAL_FALSE, 即不上报到sdt */
        pst_dmac_device->st_pkt_capture_stat.uc_capture_switch = DMAC_PKT_CAP_NORMAL;
        pst_dmac_device->st_pkt_capture_stat.en_report_sdt_switch = OAL_FALSE;

        /* 总抓包个数统计输出 */
        OAM_WARNING_LOG1(0, OAM_SF_PKT_CAP, "{dmac_config_packet_capture_switch::Total packet is [%d].}", pst_dmac_device->st_pkt_capture_stat.ul_total_report_pkt_num);

        /* 释放对应内存 */
        if (OAL_PTR_NULL != pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start)
        {
            OAL_MEM_FREE(pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start, OAL_TRUE);
            pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start = OAL_PTR_NULL;
        }
    }
    else
    {
        /* 填写抓包模式标识和上报方式标识 */
        if (1 == *puc_param)
        {
            pst_dmac_device->st_pkt_capture_stat.uc_capture_switch = DMAC_PKT_CAP_MIX;
            pst_dmac_device->st_pkt_capture_stat.en_report_sdt_switch = OAL_FALSE;
        }
        else if (2 == *puc_param)
        {
            pst_dmac_device->st_pkt_capture_stat.uc_capture_switch = DMAC_PKT_CAP_MONITOR;
            pst_dmac_device->st_pkt_capture_stat.en_report_sdt_switch = OAL_FALSE;
        }
        else if (3 == *puc_param)
        {
            pst_dmac_device->st_pkt_capture_stat.uc_capture_switch = DMAC_PKT_CAP_MIX;
            pst_dmac_device->st_pkt_capture_stat.en_report_sdt_switch = OAL_TRUE;
        }
        else
        {
            pst_dmac_device->st_pkt_capture_stat.uc_capture_switch = DMAC_PKT_CAP_MONITOR;
            pst_dmac_device->st_pkt_capture_stat.en_report_sdt_switch = OAL_TRUE;
        }

        /* 混合抓包模式下需增加对tx包上报的配置 */
        if (DMAC_PKT_CAP_MIX == pst_dmac_device->st_pkt_capture_stat.uc_capture_switch)
        {
            if (OAL_PTR_NULL != pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start)
            {
                OAL_MEM_FREE(pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start, OAL_TRUE);
                pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start = OAL_PTR_NULL;
            }

            /* 申请对应内存 */
            pst_dmac_device->st_pkt_capture_stat.us_circle_buf_depth = WLAN_PACKET_CAPTURE_CIRCLE_BUF_DEPTH;
            pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start = (oal_uint32 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, (oal_uint16)(pst_dmac_device->st_pkt_capture_stat.us_circle_buf_depth*OAL_SIZEOF(dmac_mem_circle_buf_stru)), OAL_TRUE);
            if (OAL_PTR_NULL == pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start)
            {
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_packet_capture_switch::pul_circle_buf_start null.}");
                return OAL_ERR_CODE_PTR_NULL;
            }
            OAL_MEMZERO(pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start, pst_dmac_device->st_pkt_capture_stat.us_circle_buf_depth*OAL_SIZEOF(dmac_mem_circle_buf_stru));

            /* 混合抓包循环buffer的index赋值为0 */
            pst_dmac_device->st_pkt_capture_stat.us_circle_buf_index = 0;

            /* 配置循环buffer起始地址和深度 */
            hal_packet_capture_write_reg(pst_hal_device, pst_dmac_device->st_pkt_capture_stat.pul_circle_buf_start, pst_dmac_device->st_pkt_capture_stat.us_circle_buf_depth);
        }

        /* 抓包统计位初始化为0 */
        pst_dmac_device->st_pkt_capture_stat.ul_total_report_pkt_num = 0;

        /* 配置硬件寄存器打开 */
        hal_packet_capture_switch_reg(pst_hal_device, pst_dmac_device->st_pkt_capture_stat.uc_capture_switch);
    }
    return OAL_SUCC;
}
#endif

#if defined(_PRE_DEBUG_MODE) && defined(_PRE_WLAN_FEATURE_11V)

oal_uint32  dmac_trigger_tx_bsst_query(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param)
{
    dmac_user_stru                  *pst_dmac_user      = OAL_PTR_NULL;
    dmac_vap_stru                   *pst_dmac_vap       = OAL_PTR_NULL;
    oal_uint8                       auc_mac_addr[WLAN_MAC_ADDR_LEN];
    dmac_bsst_query_info_stru       *pst_bsst_query_info = OAL_PTR_NULL;
    dmac_neighbor_bss_info_stru     *pst_neigbor_bss_list = OAL_PTR_NULL;
    oal_uint8                       uc_neighbor_num = 2;
    oal_uint8                       uc_neighbor_index = 0;

    oal_memcopy(auc_mac_addr, puc_param, WLAN_MAC_ADDR_LEN);
    /* 获取用户 */
    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, auc_mac_addr);

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSSTRANSITION, "{dmac_trigger_tx_bsst_query::dmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if ( OAL_PTR_NULL == pst_dmac_vap )
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSSTRANSITION, "{dmac_trigger_tx_bsst_query::pst_dmac_vap null, uc_vap_id=%d.}", pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 调用接口发送query帧 */
    pst_bsst_query_info = (dmac_bsst_query_info_stru *) OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_bsst_query_info_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_bsst_query_info)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSSTRANSITION, "{dmac_trigger_tx_bsst_query::pst_bsst_query_info NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_bsst_query_info->uc_reason = 6;                         // 发送原因设置为6: Better AP found
    /* 发送2个邻居AP列表 */
    pst_neigbor_bss_list = (dmac_neighbor_bss_info_stru *) OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, uc_neighbor_num*OAL_SIZEOF(dmac_neighbor_bss_info_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_neigbor_bss_list)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSSTRANSITION, "{dmac_trigger_tx_bsst_query::pst_neigbor_bss_list NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    for( ; uc_neighbor_index<uc_neighbor_num; uc_neighbor_index++)
    {
        /* MAC地址假定为11:11:11:11:11:11 */
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[0] = 0x33;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[1] = 0x33;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[2] = 0x33;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[3] = 0x33;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[4] = 0x33;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[5] = 0x33+uc_neighbor_index;;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_ap_reachability = 2;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_security = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_key_scope = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_spectrum_mgmt = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_qos = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_apsd = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_radio_meas = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_delay_block_ack = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_immediate_block_ack = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_mobility_domain = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_high_throughput = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_resv1 = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_resv2 = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_resv3 = 0;
        pst_neigbor_bss_list[uc_neighbor_index].uc_chl_num = 0;
        pst_neigbor_bss_list[uc_neighbor_index].uc_opt_class = 0;
        pst_neigbor_bss_list[uc_neighbor_index].uc_phy_type = 0;
        pst_neigbor_bss_list[uc_neighbor_index].uc_candidate_perf = uc_neighbor_index+100;
    }
    pst_bsst_query_info->pst_neighbor_bss_list = pst_neigbor_bss_list;
    pst_bsst_query_info->uc_bss_list_num = uc_neighbor_num;
    dmac_tx_bsst_query_action(pst_dmac_vap, pst_dmac_user, pst_bsst_query_info);

    /* 使用完成后释放内存 */
    if(OAL_PTR_NULL != pst_neigbor_bss_list)
    {
        OAL_MEM_FREE(pst_neigbor_bss_list, OAL_TRUE);
        pst_neigbor_bss_list = OAL_PTR_NULL;
        pst_bsst_query_info->pst_neighbor_bss_list = OAL_PTR_NULL;
    }
    if(OAL_PTR_NULL != pst_bsst_query_info)
    {
        OAL_MEM_FREE(pst_bsst_query_info, OAL_TRUE);
        pst_bsst_query_info = OAL_PTR_NULL;
    }
    return OAL_SUCC;
}

oal_uint32  dmac_trigger_tx_bsst_request(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param)
{
    dmac_user_stru                  *pst_dmac_user      = OAL_PTR_NULL;
    dmac_vap_stru                   *pst_dmac_vap       = OAL_PTR_NULL;
    oal_uint8                       auc_mac_addr[WLAN_MAC_ADDR_LEN];
    dmac_bsst_req_info_stru         *pst_bsst_req_info  = OAL_PTR_NULL;
    dmac_neighbor_bss_info_stru     *pst_neigbor_bss_list = OAL_PTR_NULL;
    oal_uint8                       uc_neighbor_num = 1;
    oal_uint8                       uc_neighbor_index = 0;

    oal_memcopy(auc_mac_addr, puc_param, WLAN_MAC_ADDR_LEN);
    /* 获取用户 */
    pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, auc_mac_addr);

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSSTRANSITION, "{dmac_trigger_tx_bsst_request::dmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if ( OAL_PTR_NULL == pst_dmac_vap )
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSSTRANSITION, "{dmac_trigger_tx_bsst_request::pst_dmac_vap null, uc_vap_id=%d.}", pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 调用接口发送query帧 */
    pst_bsst_req_info = (dmac_bsst_req_info_stru *) OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(dmac_bsst_req_info_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_bsst_req_info)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSSTRANSITION, "{dmac_trigger_tx_bsst_request::pst_bsst_query_info NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 发送2个邻居AP列表 */
    pst_neigbor_bss_list = (dmac_neighbor_bss_info_stru *) OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, uc_neighbor_num*OAL_SIZEOF(dmac_neighbor_bss_info_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_neigbor_bss_list)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSSTRANSITION, "{dmac_trigger_tx_bsst_request::pst_neigbor_bss_list NULL}");
        if(OAL_PTR_NULL != pst_bsst_req_info)
        {
            OAL_MEM_FREE(pst_bsst_req_info, OAL_TRUE);
            pst_bsst_req_info = OAL_PTR_NULL;
        }
        return OAL_ERR_CODE_PTR_NULL;
    }
    for( ; uc_neighbor_index<uc_neighbor_num; uc_neighbor_index++)
    {
        /* MAC地址假定为11:11:11:11:11:11 */
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[0] = 0xaa;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[1] = 0xbb;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[2] = 0xcc;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[3] = 0x17;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[4] = 0x58;
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[5] = 0x43;
        // 如果自己的mac地址是43 就让STA切换到44上去
        if(0x43 == pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[5])
        {
        pst_neigbor_bss_list[uc_neighbor_index].auc_mac_addr[5] = 0x44;
        }
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_ap_reachability = 2;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_security = 1;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_key_scope = 1;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_spectrum_mgmt = 1;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_qos = 1;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_apsd = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_radio_meas = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_delay_block_ack = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_immediate_block_ack = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_mobility_domain = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_high_throughput = 1;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_resv1 = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_resv2 = 0;
        pst_neigbor_bss_list[uc_neighbor_index].st_bssid_info.bit_resv3 = 0;
        pst_neigbor_bss_list[uc_neighbor_index].uc_chl_num = 0;
        pst_neigbor_bss_list[uc_neighbor_index].uc_opt_class = 0;
        pst_neigbor_bss_list[uc_neighbor_index].uc_phy_type = 0;
        pst_neigbor_bss_list[uc_neighbor_index].uc_candidate_perf = uc_neighbor_index+100;
    }
    pst_bsst_req_info->pst_neighbor_bss_list = pst_neigbor_bss_list;
    pst_bsst_req_info->uc_bss_list_num = uc_neighbor_num;
    pst_bsst_req_info->puc_session_url = OAL_PTR_NULL;            /* URL置空 无URL信息 */
    pst_bsst_req_info->st_request_mode.bit_candidate_list_include = OAL_TRUE;                     /* 包含候选邻居列表 */
    pst_bsst_req_info->st_request_mode.bit_abridged   = OAL_FALSE;                                /* 无隔离AP信息 */
    pst_bsst_req_info->st_request_mode.bit_bss_disassoc_imminent = OAL_TRUE;                      /* 含即将解关联信息 解关联时间域为1:立即断开 */
    pst_bsst_req_info->st_request_mode.bit_ess_disassoc_imminent = OAL_FALSE;                     /* 无ESS终止时间 即不含URL信息 */
    pst_bsst_req_info->st_request_mode.bit_termination_include = OAL_FALSE;                       /* 无BSS终止时间 */
    pst_bsst_req_info->st_term_duration.uc_sub_ie_id = DMAC_11V_SUBELEMENT_ID_RESV;               /* ID置0 帧中不包含该子元素 */
    pst_bsst_req_info->uc_validity_interval = 0;                                                  /* 零值为预留 */
    pst_bsst_req_info->us_disassoc_time = 1;                                                      /* 0值为预留 故写为1表示即将踢掉STA */
    dmac_tx_bsst_req_action(pst_dmac_vap, pst_dmac_user, pst_bsst_req_info,OAL_PTR_NULL);

    /* 使用完成后释放内存 */
    if(OAL_PTR_NULL != pst_neigbor_bss_list)
    {
        OAL_MEM_FREE(pst_neigbor_bss_list, OAL_TRUE);
        pst_neigbor_bss_list = OAL_PTR_NULL;
        pst_bsst_req_info->pst_neighbor_bss_list = OAL_PTR_NULL;
    }
    if(OAL_PTR_NULL != pst_bsst_req_info)
    {
        OAL_MEM_FREE(pst_bsst_req_info, OAL_TRUE);
        pst_bsst_req_info = OAL_PTR_NULL;
    }
    return OAL_SUCC;
}
#endif //defined(_PRE_DEBUG_MODE) && defined(_PRE_WLAN_FEATURE_11V)

#ifdef _PRE_WLAN_11K_STAT

OAL_STATIC oal_void dmac_config_query_stat_count(dmac_vap_stru *pst_dmac_vap, dmac_stat_count_stru *pst_stat_count)
{
    oal_uint8               uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;

    if(OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_count
        || OAL_PTR_NULL == pst_stat_count)
    {
        return;
    }

    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11TransmittedFragmentCount[%u]}",     pst_stat_count->st_count_mpdu.ul_tx_frag_mpdu_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11GroupTransmittedFrameCount::%u}",   pst_stat_count->st_count_mpdu.ul_tx_multicast_msdu_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11FailedCount[%u].}",                 pst_stat_count->st_count_mpdu.ul_tx_fail_msdu_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11ReceivedFragmentCount[%u].}",        pst_stat_count->st_count_mpdu.ul_rx_frag_mpdu_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11GroupReceivedFrameCount[%u].}",      pst_stat_count->st_count_mpdu.ul_rx_multicast_msdu_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11FCSErrorCount[%u].}",                pst_stat_count->st_count_mpdu.ul_fcs_err_mpdu_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11TransmittedFrameCount[%u].}",        pst_stat_count->st_count_mpdu.ul_tx_succ_msdu_num);

    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11RetryCount[%u].}",                   pst_stat_count->st_mac_stat.ul_tx_retry_succ_msdu_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11MultipleRetryCount[%u].}",           pst_stat_count->st_mac_stat.ul_tx_multi_retry_succ_msud_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11FrameDuplicateCount[%u].}",          pst_stat_count->st_mac_stat.ul_rx_dup_frm_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11RTSSuccessCount[%u].}",              pst_stat_count->st_mac_stat.ul_rts_suc_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11RTSFailureCount[%u].}",              pst_stat_count->st_mac_stat.ul_rts_fail_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11ACKFailureCount[%u].}",              pst_stat_count->st_mac_stat.ul_ack_fail_mpdu_num);

    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11TransmittedAMSDUCount[%u].}",        pst_stat_count->st_count_amsdu.ul_tx_succ_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11FailedAMSDUCount[%u].}",             pst_stat_count->st_count_amsdu.ul_tx_fail_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11RetryAMSDUCount[%u].}",              pst_stat_count->st_count_amsdu.ul_tx_retry_succ_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11MultipleRetryAMSDUCount[%u].}",      pst_stat_count->st_count_amsdu.ul_tx_multi_retry_succ_num);
    OAM_WARNING_LOG2(uc_vap_id, OAM_SF_CFG, "{dot11TransmittedOctetsInAMSDUCount[%u][%u].}",   (pst_stat_count->st_count_amsdu.ull_tx_succ_octets_num>>32), pst_stat_count->st_count_amsdu.ull_tx_succ_octets_num & 0xFFFFFFFF);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11AMSDUAckFailureCount[%u].}",                 pst_stat_count->st_count_amsdu.ul_ack_fail_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11ReceivedAMSDUCount[%u].}",                   pst_stat_count->st_count_amsdu.ul_rx_num);
    OAM_WARNING_LOG2(uc_vap_id, OAM_SF_CFG, "{dot11ReceivedOctetsInAMSDUCount[%u][%u].}",      (pst_stat_count->st_count_amsdu.ull_rx_octets_num>>32), pst_stat_count->st_count_amsdu.ull_rx_octets_num & 0xFFFFFFFF);

    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11TransmittedAMPDUCount[%u].}",                pst_stat_count->st_count_ampdu.ul_tx_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11TransmittedMPDUsInAMPDUCount[%u].}",         pst_stat_count->st_count_ampdu.ul_tx_mpdu_num);
    OAM_WARNING_LOG2(uc_vap_id, OAM_SF_CFG, "{dot11TransmittedOctetsInAMPDUCount[%u][%u].}",   (pst_stat_count->st_count_ampdu.ull_tx_octets_num>>32), pst_stat_count->st_count_ampdu.ull_tx_octets_num & 0xFFFFFFFF);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11AMPDUReceivedCount[%u].}",                   pst_stat_count->st_count_ampdu.ul_rx_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11MPDUInReceivedAMPDUCount[%u].}",             pst_stat_count->st_count_ampdu.ul_rx_mpdu_num);
    OAM_WARNING_LOG2(uc_vap_id, OAM_SF_CFG, "{dot11ReceivedOctetsInAMPDUCount[%u][%u].}",       (pst_stat_count->st_count_ampdu.ull_rx_octets_num>>32),  pst_stat_count->st_count_ampdu.ull_rx_octets_num & 0xFFFFFFFF);

    OAM_WARNING_LOG4(uc_vap_id, OAM_SF_CFG, "{tx_dropped[%u][%u][%u][%u].}",                    pst_stat_count->aul_tx_dropped[0], pst_stat_count->aul_tx_dropped[1], pst_stat_count->aul_tx_dropped[2], pst_stat_count->aul_tx_dropped[3]);

}



OAL_STATIC oal_void dmac_config_query_stat_count_tid(dmac_vap_stru *pst_dmac_vap, dmac_stat_count_tid_stru *pst_stat_count_tid)
{
    oal_uint8               uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;

    if(OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_tid_count || OAL_PTR_NULL == pst_stat_count_tid)
    {
        return;
    }

    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosTransmittedFragmentCount[%u].}",      pst_stat_count_tid->ul_tx_frag_mpdu_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosFailedCount[%u].}",                   pst_stat_count_tid->ul_tx_fail_msdu_num);
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosRetryCount[%u].}",                    pst_stat_count_tid->st_stat_mac_stat.ul_tx_retry_succ_msdu_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosMultipleRetryCount[%u].}",            pst_stat_count_tid->st_stat_mac_stat.ul_tx_multi_retry_succ_msud_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosFrameDuplicateCount[%u].}",           pst_stat_count_tid->st_stat_mac_stat.ul_rx_dup_frm_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosRTSSuccessCount[%u].}",               pst_stat_count_tid->st_stat_mac_stat.ul_rts_suc_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosRTSFailureCount[%u].}",               pst_stat_count_tid->st_stat_mac_stat.ul_rts_fail_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosACKFailureCount[%u].}",               pst_stat_count_tid->st_stat_mac_stat.ul_ack_fail_mpdu_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosReceivedFragmentCount[%u].}",         pst_stat_count_tid->ul_rx_frag_mpdu_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosTransmittedFrameCount[%u].}",         pst_stat_count_tid->ul_tx_succ_msdu_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosDiscardedFrameCount[%u].}",           pst_stat_count_tid->ul_tx_discard_msdu_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosMPDUsReceivedCount[%u].}",            pst_stat_count_tid->ul_rx_mpdu_num );
    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dot11QosRetriesReceivedCount[%u].}",          pst_stat_count_tid->ul_rx_retry_mpdu_num );
}



OAL_STATIC oal_void dmac_config_query_stat_frm_rpt(dmac_vap_stru *pst_dmac_vap, dmac_stat_frm_rpt_stru *pst_stat_frm_rpt)
{
    oal_uint8               uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;

    if(OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_frm_rpt || OAL_PTR_NULL == pst_stat_frm_rpt)
    {
        return;
    }

    /*3. last rcpi, avg rcpi, mag+data cnt*/
    OAM_WARNING_LOG4(uc_vap_id, OAM_SF_CFG,
        "{dmac_config_query_stat_info::frm report::last_rsni=%u, last_rcpi=%u, avrg_rcpi=%u, rx_mag_data_frm_num=%u}",
        pst_stat_frm_rpt->uc_last_rsni,
        pst_stat_frm_rpt->uc_last_rcpi,
        pst_stat_frm_rpt->uc_avrg_rcpi,
        pst_stat_frm_rpt->ul_rx_mag_data_frm_num
        );
}



OAL_STATIC oal_void dmac_config_query_stat_tsc_rpt(dmac_vap_stru *pst_dmac_vap, dmac_stat_tid_queue_delay_stru *pst_tid_queue_delay)
{
    oal_uint8               uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;

    if(OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_tsc_rpt || OAL_PTR_NULL == pst_tid_queue_delay)
    {
        return;
    }

    OAM_WARNING_LOG1(uc_vap_id, OAM_SF_CFG, "{dmac_config_query_stat_info::queue_delay_average[%u]}",
        (0 == pst_tid_queue_delay->ul_queue_delay_cnt)?0:oal_div_u64(pst_tid_queue_delay->ull_queue_delay_sum, pst_tid_queue_delay->ul_queue_delay_cnt));
}



OAL_STATIC oal_void dmac_config_query_stat_tx_delay(dmac_vap_stru *pst_dmac_vap, dmac_stat_tid_tx_delay_stru *pst_stat_tid_tx_delay)
{
    oal_uint8               uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;

    if(OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_tid_tx_delay || OAL_PTR_NULL == pst_stat_tid_tx_delay)
    {
        return;
    }

    OAM_WARNING_LOG3(uc_vap_id, OAM_SF_CFG, "{dmac_config_query_stat_info::max_tx_delay[%u], min_tx_delay[%u], average_tx_delay[%u]}",
        pst_stat_tid_tx_delay->ul_max_tx_delay, pst_stat_tid_tx_delay->ul_min_tx_delay,
        (0 == pst_stat_tid_tx_delay->ul_tx_delay_cnt)? 0: oal_div_u64(pst_stat_tid_tx_delay->ull_tx_delay_sum, pst_stat_tid_tx_delay->ul_tx_delay_cnt));
}



OAL_STATIC oal_uint32 dmac_config_query_stat_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_user_stru                  *pst_dmac_user      = OAL_PTR_NULL;
    dmac_vap_stru                   *pst_dmac_vap       = OAL_PTR_NULL;
    oal_uint8                        uc_tid = 0;
    dmac_device_stru                *pst_dmac_device;
    oal_int8                        *pc_token;
    oal_int8                        *pc_end;
    oal_int8                        *pc_ctx;
    oal_int8                        *pc_sep = " ";
    oal_uint8                        auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* MAC地址 */

    /* 获取命令类型 */
    pc_token = oal_strtok((oal_int8 *)puc_param, pc_sep, &pc_ctx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取dmac vap */
    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);

    /* 获取Device结构体指针 */
    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);

    if (0 == oal_strcmp(pc_token, "device"))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_query_stat_info::get device info.}");

        /*1.count*/
        dmac_config_query_stat_count(pst_dmac_vap, pst_dmac_device->pst_stat_count);
    }
    else if (0 == oal_strcmp(pc_token, "vap"))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_query_stat_info::get vap info.}");

        /*1. count*/
        dmac_config_query_stat_count(pst_dmac_vap, pst_dmac_vap->pst_stat_count);
        /*2. frm rpt*/
        dmac_config_query_stat_frm_rpt(pst_dmac_vap, pst_dmac_vap->pst_stat_frm_rpt);
    }
    else if (0 == oal_strcmp(pc_token, "vap_tid"))
    {
        /* 获取tid*/
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        uc_tid = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_query_stat_info::get vap_tid[%d] info.}",
            uc_tid);

        /*1. count_tid*/
        dmac_config_query_stat_count_tid(pst_dmac_vap, &(pst_dmac_vap->pst_stat_count_tid[uc_tid]));
        /*2. tsc report*/
        dmac_config_query_stat_tsc_rpt(pst_dmac_vap, &(pst_dmac_vap->pst_stat_tsc_rpt->ast_tid_queue_delay[uc_tid]));
        /*3. tx delay*/
        dmac_config_query_stat_tx_delay(pst_dmac_vap, &(pst_dmac_vap->pst_stat_tid_tx_delay[uc_tid]));
    }
    else if (0 == oal_strcmp(pc_token, "user"))
    {
        /* 获取用户 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }
        oal_strtoaddr(pc_token, auc_mac_addr);

        pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, auc_mac_addr);
        if (OAL_PTR_NULL == pst_dmac_user)
        {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_query_stat_info::dmac_user is null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{dmac_config_query_stat_info::get user[%x:%x:%x] info}",
            auc_mac_addr[3], auc_mac_addr[4], auc_mac_addr[5]);

        /*1. count*/
        dmac_config_query_stat_count(pst_dmac_vap, pst_dmac_user->pst_stat_count);

        /*2. frm rpt*/
        dmac_config_query_stat_frm_rpt(pst_dmac_vap, pst_dmac_user->pst_stat_frm_rpt);

        /*2. tx_rx thrpt*/
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{dmac_config_query_stat_info::tx_bytes[%u][%u], count[%u].}",
            pst_dmac_user->st_dmac_thrpt_stat_info.ull_tx_bytes>>32,
            pst_dmac_user->st_dmac_thrpt_stat_info.ull_tx_bytes & 0xFFFFFFFF,
            pst_dmac_user->st_dmac_thrpt_stat_info.ul_tx_thrpt_stat_count);
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{dmac_config_query_stat_info::rx_bytes[%u][%u], coun[%u].}",
            pst_dmac_user->st_dmac_thrpt_stat_info.ull_rx_bytes>>32,
            pst_dmac_user->st_dmac_thrpt_stat_info.ull_rx_bytes & 0xFFFFFFFF,
            pst_dmac_user->st_dmac_thrpt_stat_info.ul_rx_thrpt_stat_count);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{dmac_config_query_stat_info::rx_rssi_average[%d].}",
        (0 == pst_dmac_user->st_user_rate_info.st_dmac_rssi_stat_info.us_rx_rssi_stat_count)? 0:
        pst_dmac_user->st_user_rate_info.st_dmac_rssi_stat_info.l_rx_rssi/pst_dmac_user->st_user_rate_info.st_dmac_rssi_stat_info.us_rx_rssi_stat_count);

    }
    else if (0 == oal_strcmp(pc_token, "user_tid"))
    {
        /* 获取用户 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }
        oal_strtoaddr(pc_token, auc_mac_addr);

        pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, auc_mac_addr);

        if (OAL_PTR_NULL == pst_dmac_user)
        {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_query_stat_info::dmac_user is null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        /* 获取tid*/
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        uc_tid = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{dmac_config_query_stat_info::get user[%x:%x:%x]_tid[%d] info.}",
            auc_mac_addr[3], auc_mac_addr[4], auc_mac_addr[5],uc_tid);

        /*1. count_tid*/
        if(OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_user_tid_count)
        {
            dmac_config_query_stat_count_tid(pst_dmac_vap, &(pst_dmac_user->pst_stat_count_tid[uc_tid]));
        }
        /*2. tsc report*/
        dmac_config_query_stat_tsc_rpt(pst_dmac_vap, &(pst_dmac_user->pst_stat_tsc_rpt->ast_tid_queue_delay[uc_tid]));
        /*3. tx delay*/
        dmac_config_query_stat_tx_delay(pst_dmac_vap, &(pst_dmac_user->pst_stat_tid_tx_delay[uc_tid]));
    }
    else if (0 == oal_strcmp(pc_token, "reset"))
    {
        if(OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_count)
        {
            OAL_MEMZERO(pst_dmac_vap->pst_stat_count, sizeof(dmac_stat_count_stru));
            OAL_MEMZERO(pst_dmac_device->pst_stat_count, sizeof(dmac_stat_count_stru));
        }
        if(OAL_TRUE == pst_dmac_vap->st_stat_cap_flag.bit_tid_count)
        {
            OAL_MEMZERO(pst_dmac_vap->pst_stat_count_tid, sizeof(dmac_stat_count_tid_stru)*WLAN_TIDNO_BUTT);
        }
    }

    return OAL_SUCC;

}
#endif

#ifdef _PRE_FEATURE_FAST_AGING

oal_uint32  dmac_config_fast_aging(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_device_stru               *pst_dmac_dev;
    mac_cfg_fast_aging_stru        *pst_aging_cfg_param = (mac_cfg_fast_aging_stru*)puc_param;
    frw_timeout_stru               *pst_fast_aging_timer;

    pst_dmac_dev = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_dev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CAR, "{dmac_config_fast_aging::pst_dmac_dev null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_fast_aging_timer = &pst_dmac_dev->st_fast_aging.st_timer;

    switch (pst_aging_cfg_param->en_cmd)
    {
        case MAC_FAST_AGING_ENABLE:
            pst_dmac_dev->st_fast_aging.en_enable = pst_aging_cfg_param->en_enable;
            if (OAL_TRUE == pst_dmac_dev->st_fast_aging.en_enable)
            {
                /* 打开定时器 */
                if(OAL_FALSE == pst_fast_aging_timer->en_is_enabled)
                {
                    frw_timer_restart_timer(pst_fast_aging_timer, pst_dmac_dev->st_fast_aging.us_timeout_ms, OAL_TRUE);
                    OAM_WARNING_LOG1(0, OAM_SF_CAR, "{dmac_config_fast_aging:: en_fast_aging[%d].}", pst_dmac_dev->st_fast_aging.en_enable);
                }
            }
            else
            {
                FRW_TIMER_STOP_TIMER(pst_fast_aging_timer);
            }
            break;
        case MAC_FAST_AGING_TIMEOUT:
            pst_dmac_dev->st_fast_aging.us_timeout_ms = pst_aging_cfg_param->us_timeout_ms;
            OAM_WARNING_LOG1(0, OAM_SF_CAR, "{dmac_config_fast_aging:: us_fast_aging_timeout_ms[%d].}", pst_aging_cfg_param->us_timeout_ms);

            //先销毁原来的定时器
            if (pst_fast_aging_timer->en_is_registerd)
            {
                FRW_TIMER_IMMEDIATE_DESTROY_TIMER(pst_fast_aging_timer);
            }
            //重新启动定时器
            if (pst_dmac_dev->pst_device_base_info)
            {
                FRW_TIMER_CREATE_TIMER(pst_fast_aging_timer,
                                       dmac_fast_aging_timeout,
                                       pst_dmac_dev->st_fast_aging.us_timeout_ms,
                                       (oal_void *)pst_dmac_dev,
                                       OAL_TRUE,
                                       OAM_MODULE_ID_DMAC,
                                       pst_dmac_dev->pst_device_base_info->ul_core_id);
                //若未使能，则关闭定时器
                if (OAL_FALSE == pst_dmac_dev->st_fast_aging.en_enable)
                {
                    FRW_TIMER_STOP_TIMER(pst_fast_aging_timer);
                }
            }
            break;
       case MAC_FAST_AGING_COUNT:
            pst_dmac_dev->st_fast_aging.uc_count_limit = pst_aging_cfg_param->uc_count_limit;
            OAM_WARNING_LOG1(0, OAM_SF_CAR, "{dmac_config_fast_aging:: uc_count_limit[%d].}", pst_aging_cfg_param->uc_count_limit);
            break;
        default:
            break;
        }

    return OAL_SUCC;


}


oal_uint32  dmac_config_get_fast_aging(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_device_stru               *pst_dmac_dev;
    mac_cfg_param_char_stru         st_param;
    mac_cfg_fast_aging_stru        *pst_aging_cfg_param;

    pst_dmac_dev = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_dev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CAR, "{dmac_config_get_fast_aging::pst_dmac_dev null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_param.l_buff_len = OAL_SIZEOF(mac_cfg_fast_aging_stru);

    pst_aging_cfg_param = (mac_cfg_fast_aging_stru*)st_param.auc_buff;

    pst_aging_cfg_param->en_enable = pst_dmac_dev->st_fast_aging.en_enable;
    pst_aging_cfg_param->us_timeout_ms = pst_dmac_dev->st_fast_aging.us_timeout_ms;
    pst_aging_cfg_param->uc_count_limit = pst_dmac_dev->st_fast_aging.uc_count_limit;

    OAM_WARNING_LOG3(0, OAM_SF_CAR, "{dmac_config_get_fast_aging:: en_fast_aging=%d, us_timeout_ms=%d, uc_count_limit=%d.}",
        pst_aging_cfg_param->en_enable,
        pst_aging_cfg_param->us_timeout_ms,
        pst_aging_cfg_param->uc_count_limit);

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_FAST_AGING, OAL_SIZEOF(mac_cfg_param_char_stru), (oal_uint8 *)&st_param);

    return OAL_SUCC;


}

#endif


oal_uint32  dmac_config_alg(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32        ul_ret = OAL_FAIL;
    oal_uint8         uc_idx;

    oal_int8         *pac_argv[DMAC_ALG_CONFIG_MAX_ARG + 1];

    mac_ioctl_alg_config_stru *pst_alg_config = (mac_ioctl_alg_config_stru *)puc_param;

    for (uc_idx = OAL_SIZEOF(mac_ioctl_alg_config_stru); uc_idx < uc_len; uc_idx++)
    {
        if(puc_param[uc_idx] == ' ')
        {
            puc_param[uc_idx] = 0;
        }
    }

    for(uc_idx = 0; uc_idx < pst_alg_config->uc_argc; uc_idx++)
    {
        pac_argv[uc_idx] = (oal_int8 *)puc_param + OAL_SIZEOF(mac_ioctl_alg_config_stru) + pst_alg_config->auc_argv_offset[uc_idx];
    }

    pac_argv[uc_idx] = NULL;

    for(uc_idx = 0; uc_idx < DMAC_ALG_CONFIG_ID_BUTT; uc_idx++)
    {
        if(0 == oal_strcmp(pac_argv[0], g_ast_dmac_alg_config_table[uc_idx].puc_alg_name))
        {
            break;
        }
    }

    if (uc_idx == DMAC_ALG_CONFIG_ID_BUTT)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_alg: uc_idx error: %d}", uc_idx);

        return OAL_FAIL;
    }


    if(g_pst_alg_main->pa_alg_config_notify_func[uc_idx])
    {
        ul_ret = g_pst_alg_main->pa_alg_config_notify_func[uc_idx](pst_mac_vap, pst_alg_config->uc_argc - 1, pac_argv + 1);
    }
    else
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_alg::p_func null.}");
    }

    return ul_ret;
}

oal_uint8 *g_puc_sample_addr = OAL_PTR_NULL;
oal_uint32 dmac_config_show_device_meminfo(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

    oal_uint8                         uc_pool_id;
    oal_uint8                         uc_sample_alloc_banks;
    oal_uint8                         uc_idx;
    oal_uint8                         uc_pool_id_start = 0;
    oal_uint8                         uc_pool_id_end = 0;
    oal_uint8                         uc_pm_off;

    mac_cfg_meminfo_stru             *pst_meminfo_param;

    pst_meminfo_param = (mac_cfg_meminfo_stru *)puc_param;
    uc_pool_id  = pst_meminfo_param->uc_object_index;

    if (MAC_MEMINFO_BUTT <= pst_meminfo_param->uc_meminfo_type)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_show_device_meminfo::uc_meminfo_type %d >= MAC_MEMINFO_BUTT.}", pst_meminfo_param->uc_meminfo_type);
        return OAL_FAIL;
    }

    if (MAC_MEMINFO_SDIO_TRX == pst_meminfo_param->uc_meminfo_type)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{memoryinfo::number of netbuffs pending in sdio_sch_txq[%d],sdio_sch_rxq[%d].}", hcc_tx_queue_len(), hcc_rx_queue_len());
        return OAL_SUCC;
    }

    if ((MAC_MEMINFO_USER == pst_meminfo_param->uc_meminfo_type)
        || (MAC_MEMINFO_VAP == pst_meminfo_param->uc_meminfo_type))
    {
    #ifdef _PRE_WLAN_FEATURE_MEMORY_USAGE_TRACE
        dmac_alg_netbuff_usage_info_notify(pst_meminfo_param);
    #endif
        return OAL_SUCC;
    }

    if (MAC_MEMINFO_POOL_DBG == pst_meminfo_param->uc_meminfo_type)
    {
        if (uc_pool_id >= OAL_MEM_POOL_ID_BUTT)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_show_device_meminfo::uc_pool_id %d >= OAL_MEM_POOL_ID_BUTT.}", uc_pool_id);
            return OAL_FAIL;
        }
        else
        {
            OAL_MEM_INFO_PRINT(uc_pool_id);
        }

        return OAL_SUCC;
    }

    uc_sample_alloc_banks = pst_meminfo_param->uc_object_index;

    if (MAC_MEMINFO_SAMPLE_ALLOC == pst_meminfo_param->uc_meminfo_type)
    {
        if (g_puc_sample_addr != OAL_PTR_NULL)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_alloc_sample_meminfo::mem already alloced[%x].}",g_puc_sample_addr);
        }
        else
        {
            g_puc_sample_addr = OAL_MEM_SAMPLE_NETBUF_ALLOC(uc_sample_alloc_banks);

            if(g_puc_sample_addr != OAL_PTR_NULL)
            {
                uc_pm_off = 0;
                dmac_config_set_pm_switch(pst_mac_vap, 0, &uc_pm_off);

                OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_alloc_sample_meminfo::sample mem alloc success.}");
            }
            else
            {
                OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_alloc_sample_meminfo:: sample mem alloc failed.}");
            }
        }
        return OAL_SUCC;

    }

    if (MAC_MEMINFO_SAMPLE_FREE == pst_meminfo_param->uc_meminfo_type)
    {
        if (g_puc_sample_addr == OAL_PTR_NULL)
        {
            OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_alloc_sample_meminfo::sample mem already freed.}");
        }
        else
        {
            uc_pm_off = 1;
            dmac_config_set_pm_switch(pst_mac_vap, 0, &uc_pm_off);
            OAL_MEM_SAMPLE_NETBUF_FREE(g_puc_sample_addr);
            g_puc_sample_addr = OAL_PTR_NULL;
            OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_alloc_sample_meminfo::sample mem freed success.}");
        }
        return OAL_SUCC;
    }

    do{
        if (MAC_MEMINFO_POOL_INFO == pst_meminfo_param->uc_meminfo_type)
        {
            if (uc_pool_id == 0xff)
            {
                uc_pool_id_start = 0;
                uc_pool_id_end   = OAL_MEM_POOL_ID_NETBUF;
            }
            else
            {
                uc_pool_id_start = uc_pool_id;
                uc_pool_id_end   = uc_pool_id;
            }
        }

        if (MAC_MEMINFO_ALL == pst_meminfo_param->uc_meminfo_type)
        {
            uc_pool_id_start = 0;
            uc_pool_id_end   = OAL_MEM_POOL_ID_NETBUF;
        }

        if (MAC_MEMINFO_NETBUFF == pst_meminfo_param->uc_meminfo_type)
        {
            uc_pool_id_start = OAL_MEM_POOL_ID_NETBUF;
            uc_pool_id_end   = OAL_MEM_POOL_ID_NETBUF;
        }

        if (MAC_MEMINFO_DSCR == pst_meminfo_param->uc_meminfo_type)
        {
            uc_pool_id_start = OAL_MEM_POOL_ID_RX_DSCR;
            uc_pool_id_end   = OAL_MEM_POOL_ID_TX_DSCR_2;
        }

        for (uc_idx = uc_pool_id_start; uc_idx <= uc_pool_id_end; uc_idx++)
        {
            OAL_MEM_USAGE_INFO_PRINT(uc_idx);
        }
    } while(0);

    if (MAC_MEMINFO_ALL == pst_meminfo_param->uc_meminfo_type)
    {
    #ifdef _PRE_WLAN_FEATURE_MEMORY_USAGE_TRACE
        dmac_alg_netbuff_usage_info_notify(pst_meminfo_param);
    #endif

    }
#endif
    return OAL_SUCC;

}

#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND

OAL_STATIC oal_uint32  dmac_config_waveapp_32plus_user_enable(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru           *pst_mac_device = OAL_PTR_NULL;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_waveapp_40user_enable::vap id [%d],pst_mac_device null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_waveapp_40user_enable::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_set_is_waveapp_test(pst_hal_device, *puc_param);

#ifndef _PRE_WLAN_PRODUCT_1151V200
    /* V100 特殊配置 */
    if(OAL_TRUE == *puc_param)
    {
        hal_set_agc_track_ant_sel(pst_hal_device, (oal_uint32)0x00);
        pst_mac_device->uc_lock_channel = 0x00;
    }
    else
    {
        hal_set_agc_track_ant_sel(pst_hal_device, (oal_uint32)0x02);
        pst_mac_device->uc_lock_channel = 0x02;
    }
#endif
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_WEB_CMD_COMM

OAL_STATIC oal_uint32  dmac_config_get_hw_flow_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_machw_flow_stat_stru    *pst_machw_stat;
    hal_to_dmac_device_stru     *pst_hal_device;
    dmac_vap_stru               *pst_dmac_vap;
    oam_stats_machw_stat_stru st_machw_stat_q;

    pst_dmac_vap = (dmac_vap_stru *)MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_hw_flow_stat::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);

    pst_machw_stat = (oal_machw_flow_stat_stru*)puc_param;
    OAL_MEMZERO(&st_machw_stat_q, OAL_SIZEOF(oam_stats_machw_stat_stru));
    hal_dft_get_machw_stat_info_ext(pst_hal_device, &st_machw_stat_q);
    pst_machw_stat->ul_rx_bytes = st_machw_stat_q.ul_machw_rx_octects_in_ampdu;
    pst_machw_stat->ul_rx_discard = st_machw_stat_q.ul_machw_rx_filtered_cnt;
    pst_machw_stat->ul_rx_err = st_machw_stat_q.ul_machw_rx_failed_mpdu_cnt;
    pst_machw_stat->ul_rx_pkts = st_machw_stat_q.ul_machw_rx_ampdu_count;
    pst_machw_stat->ul_tx_bytes = st_machw_stat_q.ul_machw_tx_octects_in_ampdu;
    pst_machw_stat->ul_tx_pkts = st_machw_stat_q.ul_machw_tx_ampdu_count;
    pst_machw_stat->ul_tx_discard = 0;
    pst_machw_stat->ul_tx_err = st_machw_stat_q.ul_machw_tx_normal_pri_retry_cnt;
    pst_machw_stat->ul_tx_bcn_cnt = st_machw_stat_q.ul_machw_tx_bcn_count;

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_HW_FLOW_STAT, OAL_SIZEOF(oal_machw_flow_stat_stru), (oal_uint8 *)pst_machw_stat);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_get_wme_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifdef _PRE_WLAN_11K_STAT
    oal_wme_stat_stru    *pst_wme_stat;
    dmac_vap_stru               *pst_dmac_vap;
    dmac_stat_count_tid_stru     *pst_count_stat_tid_info;
    oal_uint8                   uc_idx = 0;

    pst_dmac_vap = (dmac_vap_stru *)MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_wme_stat::PTR null: pst_dmac_vap %d, puc_param %d.}",
                pst_dmac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /*判断是否使能统计*/
    if (OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable || OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_tid_count
         || OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_tid_tx_delay)
    {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_wme_stat::wme stat not enabled. enable[%d], tid[%d], delay[%d]}",
                pst_dmac_vap->st_stat_cap_flag.bit_enable, pst_dmac_vap->st_stat_cap_flag.bit_tid_count, pst_dmac_vap->st_stat_cap_flag.bit_tid_tx_delay);
        return OAL_FAIL;
    }

    pst_wme_stat = (oal_wme_stat_stru*)puc_param;
    for (uc_idx = 0; uc_idx < WLAN_WME_AC_BUTT; uc_idx++)
    {
        pst_count_stat_tid_info = (dmac_stat_count_tid_stru*)(&pst_dmac_vap->pst_stat_count_tid[WLAN_WME_AC_TO_TID(uc_idx)]);
        pst_wme_stat->ul_tx_expire[uc_idx] = (oal_uint32)pst_dmac_vap->pst_stat_tid_tx_delay[WLAN_WME_AC_TO_TID(uc_idx)].ull_tx_delay_sum;

        pst_wme_stat->ul_forward_bytes[uc_idx] = pst_count_stat_tid_info->ul_forward_bytes;
        pst_wme_stat->ul_forward_num[uc_idx] = pst_count_stat_tid_info->ul_forward_num;
        pst_wme_stat->ul_rx_fail_bytes[uc_idx] = pst_count_stat_tid_info->ul_rx_fail_bytes;
        pst_wme_stat->ul_rx_fail_num[uc_idx] = pst_count_stat_tid_info->ul_rx_fail_num;
        pst_wme_stat->ul_rx_mpdu_num[uc_idx] = pst_count_stat_tid_info->ul_rx_mpdu_num;
        pst_wme_stat->ul_rx_succ_bytes[uc_idx] = pst_count_stat_tid_info->ul_rx_succ_bytes;
        pst_wme_stat->ul_tx_fail_bytes[uc_idx] = pst_count_stat_tid_info->ul_tx_fail_bytes;
        pst_wme_stat->ul_tx_fail_num[uc_idx] = pst_count_stat_tid_info->ul_tx_fail_msdu_num;
        pst_wme_stat->ul_tx_succ_bytes[uc_idx] = pst_count_stat_tid_info->ul_tx_succ_bytes;
        pst_wme_stat->ul_tx_succ_num[uc_idx] = pst_count_stat_tid_info->ul_tx_succ_msdu_num;
    }

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_WME_STAT, OAL_SIZEOF(oal_wme_stat_stru), (oal_uint8 *)pst_wme_stat);

    return OAL_SUCC;
#else
    return OAL_FAIL;
#endif
}


OAL_STATIC oal_uint32  dmac_config_get_ant_rssi(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if defined _PRE_WLAN_PRODUCT_1151V200 && defined _PRE_WLAN_RX_DSCR_TRAILER
    mac_cfg_param_char_stru             st_param;
    oal_bool_enum_uint8         en_on = OAL_FALSE;
    hal_to_dmac_device_stru     *pst_hal_device;
    dmac_vap_stru               *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_ant_rssi::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_ant_rssi::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_param.l_buff_len = OAL_SIZEOF(oal_int32);
    hal_get_ant_rssi_rep_sw(pst_hal_device, &en_on);
    if (OAL_TRUE == en_on)
    {
        hal_get_ant_rssi_value(pst_hal_device, (oal_int16*)st_param.auc_buff, ((oal_int16*)st_param.auc_buff + 1));
        *((oal_int16*)st_param.auc_buff) = oal_get_real_rssi(*((oal_int16*)st_param.auc_buff));
        *((oal_int16*)st_param.auc_buff + 1) = oal_get_real_rssi(*((oal_int16*)st_param.auc_buff + 1));
    }
    else
    {
        *((oal_int16*)st_param.auc_buff) = 0;
        *((oal_int16*)st_param.auc_buff + 1) = 0;
    }

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_ANT_RSSI, OAL_SIZEOF(mac_cfg_param_char_stru), (oal_uint8 *)&st_param);

    return OAL_SUCC;
#else
    return OAL_SUCC;
#endif
}


OAL_STATIC oal_uint32  dmac_config_ant_rssi_report(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if defined _PRE_WLAN_PRODUCT_1151V200 && defined _PRE_WLAN_RX_DSCR_TRAILER
    hal_to_dmac_device_stru     *pst_hal_device;
    dmac_vap_stru               *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_ant_rssi_report::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_ant_rssi_report::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (0 == *puc_param)
    {
        hal_set_ant_rssi_report(pst_hal_device, OAL_FALSE);
    }
    else if (1 == *puc_param)
    {
        hal_set_ant_rssi_report(pst_hal_device, OAL_TRUE);
    }

#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_11K_STAT

OAL_STATIC oal_uint32  dmac_config_get_tx_delay_ac(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_tx_delay_ac_stru           *pst_tx_delay_ac;
    dmac_vap_stru                  *pst_dmac_vap;
    oal_uint8                       uc_idx = 0;
    dmac_stat_tid_tx_delay_stru    *pst_stat_tx_delay;
    oal_uint64                      aull_tx_delay_sum[WLAN_WME_AC_BUTT]    = {0};                    /*发送时延:TID入队到上报发送完成中断的时间累加值*/
    oal_uint32                      aul_tx_delay_cnt[WLAN_WME_AC_BUTT]     = {0};                    /*tx delay的统计个数*/

    pst_dmac_vap = (dmac_vap_stru *)MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_tx_delay_ac::PTR null: pst_dmac_vap %d, puc_param %d.}",
                pst_dmac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }
    /*判断是否使能统计*/
    if (OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_enable || OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_tid_count
         || OAL_FALSE == pst_dmac_vap->st_stat_cap_flag.bit_tid_tx_delay)
    {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_tx_delay_ac::tx delay not enabled. enable[%d], tid[%d], delay[%d]}",
                pst_dmac_vap->st_stat_cap_flag.bit_enable, pst_dmac_vap->st_stat_cap_flag.bit_tid_count, pst_dmac_vap->st_stat_cap_flag.bit_tid_tx_delay);
        return OAL_FAIL;
    }

    pst_tx_delay_ac = (oal_tx_delay_ac_stru*)puc_param;
    OAL_MEMZERO(pst_tx_delay_ac, OAL_SIZEOF(oal_tx_delay_ac_stru));

    for (uc_idx = 0; uc_idx < WLAN_TID_MAX_NUM; uc_idx++)//把TID对应到AC上
    {
        pst_stat_tx_delay = pst_dmac_vap->pst_stat_tid_tx_delay + uc_idx;
        /*时延累计值*/
        aull_tx_delay_sum[WLAN_WME_TID_TO_AC(uc_idx)] += pst_stat_tx_delay->ull_tx_delay_sum;
        aul_tx_delay_cnt[WLAN_WME_TID_TO_AC(uc_idx)] += pst_stat_tx_delay->ul_tx_delay_cnt;
        /*时延极限值*/
        if(pst_tx_delay_ac->ul_max_tx_delay[WLAN_WME_TID_TO_AC(uc_idx)] < pst_stat_tx_delay->ul_max_tx_delay)
        {
            pst_tx_delay_ac->ul_max_tx_delay[WLAN_WME_TID_TO_AC(uc_idx)] = pst_stat_tx_delay->ul_max_tx_delay;
        }

        if (((0!= pst_stat_tx_delay->ul_min_tx_delay)
                && ( pst_tx_delay_ac->ul_min_tx_delay[WLAN_WME_TID_TO_AC(uc_idx)] > pst_stat_tx_delay->ul_min_tx_delay ))
                || (0 == pst_tx_delay_ac->ul_min_tx_delay[WLAN_WME_TID_TO_AC(uc_idx)]))
        {
            pst_tx_delay_ac->ul_min_tx_delay[WLAN_WME_TID_TO_AC(uc_idx)] = pst_stat_tx_delay->ul_min_tx_delay;
        }

    }
    for (uc_idx = 0; uc_idx < WLAN_WME_AC_BUTT; uc_idx++)//计算平均时延
    {
        if(0 != aul_tx_delay_cnt[uc_idx])
        {
            //pst_tx_delay_ac->ul_ave_tx_delay[uc_idx]       = aull_tx_delay_sum[uc_idx] / aul_tx_delay_cnt[uc_idx];
            pst_tx_delay_ac->ul_ave_tx_delay[uc_idx]       = (oal_uint32)oal_div_u64(aull_tx_delay_sum[uc_idx], aul_tx_delay_cnt[uc_idx]);
        }
    }

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_TX_DELAY_AC, OAL_SIZEOF(oal_tx_delay_ac_stru), (oal_uint8 *)pst_tx_delay_ac);

    return OAL_SUCC;
}
#endif
#endif

#ifdef _PRE_WLAN_PRODUCT_1151V200
OAL_STATIC oal_uint32  dmac_config_80m_rts_debug(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);

    /* 参数判断 */
    pst_mac_vap->uc_force_resp_mode_80M = *puc_param;

    if (0 == pst_mac_vap->uc_force_resp_mode_80M)
    {
        hal_set_80m_resp_mode(pst_hal_device, OAL_FALSE);
    }
    else
    {
        pst_mac_vap->uc_1st_asoc_done = OAL_FALSE;
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_80m_rts_debug: uc_force_resp_mode_80M = %d.", pst_mac_vap->uc_force_resp_mode_80M);

    return OAL_SUCC;

}
#endif

#ifdef _PRE_WLAN_FEATURE_DBDC

OAL_STATIC oal_uint32  dmac_config_dbdc_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_device_stru                *pst_dmac_device;
    mac_dbdc_debug_switch_stru      *pst_dbdc_debug_switch;

    pst_dbdc_debug_switch = (mac_dbdc_debug_switch_stru *)puc_param;

    /* vap change hal device before up */
    if (pst_dbdc_debug_switch->ul_cmd_bit_map & BIT(MAC_DBDC_CHANGE_HAL_DEV))
    {
        dmac_vap_change_hal_dev(pst_mac_vap, pst_dbdc_debug_switch);
    }

    /* DBDC软件开关 */
    if (pst_dbdc_debug_switch->ul_cmd_bit_map & BIT(MAC_DBDC_SWITCH))
    {
        pst_dmac_device = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
        if (OAL_PTR_NULL == pst_dmac_device)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_device_id, OAM_SF_CFG, "{dmac_config_dbdc_debug_switch::pst_dmac_device null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        pst_dmac_device->en_dbdc_enable  = pst_dbdc_debug_switch->uc_dbdc_enable;
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dbdc_debug_switch::mac device[%d],dbdc enable[%d]}",
                pst_dmac_device->pst_device_base_info->uc_device_id, pst_dmac_device->en_dbdc_enable);
    }

    /* 并发扫描开关 */
    if (pst_dbdc_debug_switch->ul_cmd_bit_map & BIT(MAC_FAST_SCAN_SWITCH))
    {
        pst_dmac_device = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
        if (OAL_PTR_NULL == pst_dmac_device)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_device_id, OAM_SF_CFG, "{dmac_config_dbdc_debug_switch::pst_dmac_device null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        pst_dmac_device->en_fast_scan_enable  = pst_dbdc_debug_switch->en_fast_scan_enable;
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dbdc_debug_switch::mac device[%d],fast scan enable[%d]}",
                pst_dmac_device->pst_device_base_info->uc_device_id, pst_dmac_device->en_fast_scan_enable);
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_FTM

OAL_STATIC oal_uint32 dmac_config_ftm_dbg(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                        *pst_dmac_vap;
    mac_ftm_debug_switch_stru            *pst_ftm_debug;
    dmac_ftm_initiator_stru              *past_ftm_init;
    dmac_ftm_responder_stru              *past_ftm_rsp;
    oal_int8                              c_session_id = -1;
    oal_uint8                             uc_session_id = 0;
    oal_bool_enum_uint8                   en_ftm_status;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;
    past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;

    pst_ftm_debug = (mac_ftm_debug_switch_stru *)puc_param;

    /* ftm_initiator命令*/
    if (pst_ftm_debug->ul_cmd_bit_map & BIT0)
    {
        mac_mib_set_FineTimingMsmtInitActivated(pst_mac_vap, pst_ftm_debug->en_ftm_initiator_bit0);
        //mac_mib_set_WirelessManagementImplemented(pst_mac_vap, pst_ftm_debug->en_ftm_initiator_bit0);

        hal_set_ftm_initiator(pst_dmac_vap->pst_hal_device, pst_ftm_debug->en_ftm_initiator_bit0);
    }
    /* 发送iftmr命令*/
    if (pst_ftm_debug->ul_cmd_bit_map & BIT1)
    {
        if (pst_ftm_debug->st_send_iftmr_bit1.uc_channel_num == 0)
        {
            past_ftm_init[0].st_channel_ftm.uc_chan_number = pst_dmac_vap->st_vap_base_info.st_channel.uc_chan_number;
            past_ftm_init[0].st_channel_ftm.en_band = pst_dmac_vap->st_vap_base_info.st_channel.en_band;
            past_ftm_init[0].st_channel_ftm.en_bandwidth = pst_dmac_vap->st_vap_base_info.st_channel.en_bandwidth;
        }
        else if ((pst_ftm_debug->st_send_iftmr_bit1.uc_channel_num <= 14)
                &&(pst_ftm_debug->st_send_iftmr_bit1.uc_channel_num >= 1))
        {
            past_ftm_init[0].st_channel_ftm.uc_chan_number = pst_ftm_debug->st_send_iftmr_bit1.uc_channel_num;
            past_ftm_init[0].st_channel_ftm.en_band = WLAN_BAND_2G;
            past_ftm_init[0].st_channel_ftm.en_bandwidth = WLAN_BAND_WIDTH_20M;
        }
        else
        {
            past_ftm_init[0].st_channel_ftm.uc_chan_number = pst_ftm_debug->st_send_iftmr_bit1.uc_channel_num;
            past_ftm_init[0].st_channel_ftm.en_band = WLAN_BAND_5G;
            past_ftm_init[0].st_channel_ftm.en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
        }

        mac_get_channel_idx_from_num(past_ftm_init[0].st_channel_ftm.en_band,
                                     past_ftm_init[0].st_channel_ftm.uc_chan_number,
                                     &past_ftm_init[0].st_channel_ftm.uc_chan_idx);

        /*如果命令没有指定BSSID，取关联ap的BSSID*/
        if(mac_addr_is_zero(past_ftm_init[0].auc_bssid))
        {
            oal_memcopy(past_ftm_init[0].auc_bssid, pst_dmac_vap->st_vap_base_info.auc_bssid, OAL_MAC_ADDR_LEN);
        }
        else
        {
            oal_memcopy(past_ftm_init[0].auc_bssid, pst_ftm_debug->st_send_iftmr_bit1.auc_bssid, OAL_MAC_ADDR_LEN);
        }

        past_ftm_init[0].en_lci_ie = pst_ftm_debug->st_send_iftmr_bit1.measure_req;
        past_ftm_init[0].en_location_civic_ie = pst_ftm_debug->st_send_iftmr_bit1.measure_req;

        past_ftm_init[0].us_burst_cnt = pst_ftm_debug->st_send_iftmr_bit1.uc_burst_num;
        past_ftm_init[0].uc_ftms_per_burst_cmd = pst_ftm_debug->st_send_iftmr_bit1.uc_ftms_per_burst;
        past_ftm_init[0].en_asap = pst_ftm_debug->st_send_iftmr_bit1.en_asap;

        if(past_ftm_init[0].en_asap == OAL_TRUE)
        {
            OAL_MEMZERO(&past_ftm_init[0].st_ftm_range_report, OAL_SIZEOF(past_ftm_init[0].st_ftm_range_report));
            past_ftm_init[0].bit_range = 0;

            if(past_ftm_init[0].uc_ftms_per_burst_cmd > 2)
            {
                past_ftm_init[0].uc_ftms_per_burst = past_ftm_init[0].uc_ftms_per_burst_cmd;
                past_ftm_init[0].us_burst_period = past_ftm_init[0].uc_ftms_per_burst_cmd;
            }
            else
            {
                past_ftm_init[0].uc_ftms_per_burst = 2;
                past_ftm_init[0].us_burst_period = 1;
            }
            past_ftm_init[0].en_iftmr = OAL_TRUE;
            dmac_sta_ftm_start_bust(pst_dmac_vap);
        }
        else
        {
            past_ftm_init[0].en_iftmr = OAL_TRUE;
            dmac_sta_start_scan_for_ftm(pst_dmac_vap, FTM_WAIT_TIMEOUT);
        }
    }
    /* ftm enable命令*/
    if (pst_ftm_debug->ul_cmd_bit_map & BIT2)
    {
        hal_set_ftm_enable(pst_dmac_vap->pst_hal_device, pst_ftm_debug->en_enable_bit2);
    }
    /* ftm cali命令*/
    if (pst_ftm_debug->ul_cmd_bit_map & BIT3)
    {
        past_ftm_init[0].en_cali = pst_ftm_debug->en_cali_bit3;

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
        if (OAL_SWITCH_ON == pst_dmac_vap->st_vap_base_info.bit_al_tx_flag)
        {
            hal_set_ftm_cali(pst_dmac_vap->pst_hal_device, NULL, past_ftm_init[0].en_cali);
        }
#endif
    }
    /* 发送ftm命令*/
    if (pst_ftm_debug->ul_cmd_bit_map & BIT4)
    {
        /*查找session id*/
        c_session_id = dmac_ftm_find_session_index(pst_dmac_vap, MAC_FTM_RESPONDER_MODE, pst_ftm_debug->st_send_ftm_bit4.auc_mac);
        if (c_session_id < 0)
        {
            return OAL_FAIL;
        }

        uc_session_id = (oal_uint8)c_session_id;
        en_ftm_status = past_ftm_rsp[uc_session_id].en_ftm_responder;
        dmac_ftm_enable_session_index(pst_dmac_vap, MAC_FTM_RESPONDER_MODE, pst_ftm_debug->st_send_ftm_bit4.auc_mac, uc_session_id);

        past_ftm_rsp[uc_session_id].uc_dialog_token = 2;
        past_ftm_rsp[uc_session_id].uc_follow_up_dialog_token = 1;

        /*non_asap*/
        past_ftm_rsp[uc_session_id].en_asap = OAL_FALSE;
        /* 不携带 ie*/
        past_ftm_rsp[uc_session_id].en_lci_ie = OAL_FALSE;
        past_ftm_rsp[uc_session_id].en_location_civic_ie = OAL_FALSE;

        past_ftm_rsp[uc_session_id].en_ftm_parameters = OAL_TRUE;
        past_ftm_rsp[uc_session_id].en_ftm_synchronization_information = OAL_TRUE;

        past_ftm_rsp[uc_session_id].ull_tod = 0;
        past_ftm_rsp[uc_session_id].ull_toa = 0;

        /* 获取TSF*/
        hal_vap_tsf_get_32bit(pst_dmac_vap->pst_hal_vap, &past_ftm_rsp[uc_session_id].ul_tsf);

        dmac_ftm_rsp_send_ftm(pst_dmac_vap, uc_session_id);
        /*恢复session状态*/
        past_ftm_rsp[uc_session_id].en_ftm_responder = en_ftm_status;
    }
    /* ftm_resp命令*/
    if (pst_ftm_debug->ul_cmd_bit_map & BIT5)
    {
        mac_mib_set_FineTimingMsmtRespActivated(pst_mac_vap, pst_ftm_debug->en_ftm_resp_bit5);
    }
    /*  设置校准时间*/
    if (pst_ftm_debug->ul_cmd_bit_map & BIT6)
    {
        dmac_set_ftm_correct_time(pst_dmac_vap ,pst_ftm_debug->st_ftm_time_bit6);
    }

    return OAL_SUCC;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
OAL_STATIC oal_uint32 dmac_config_pm_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if defined(_PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN) || defined(_PRE_PM_DYN_SET_TBTT_OFFSET)
    mac_pm_debug_cfg_stru    *pst_pm_debug;
    hal_to_dmac_device_stru  *pst_hal_device;
    oal_uint8                 uc_up_vap_num;

    pst_pm_debug = (mac_pm_debug_cfg_stru *)puc_param;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
    /* siso收beacon开关 */
    if (pst_pm_debug->ul_cmd_bit_map & BIT(MAC_PM_DEBUG_SISO_RECV_BCN))
    {
        pst_hal_device->bit_srb_switch = pst_pm_debug->uc_srb_switch;
        /* 关闭siso收beacon功能时需要reset */
        if (0 == pst_hal_device->bit_srb_switch)
        {
            uc_up_vap_num = hal_device_calc_up_vap_num(pst_hal_device);
            hal_device_reset_bcn_rf_chain(pst_hal_device, uc_up_vap_num);
        }
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "dmac_config_pm_debug_switch:g_uc_srb_switch set to %d", pst_hal_device->bit_srb_switch);
    }
#endif

#ifdef _PRE_PM_DYN_SET_TBTT_OFFSET
    /* 动态tbtt offset开关 */
    if (pst_pm_debug->ul_cmd_bit_map & BIT(MAC_PM_DEBUG_DYN_TBTT_OFFSET))
    {
        hal_dyn_tbtt_offset_switch(pst_hal_device, pst_pm_debug->uc_dto_switch);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_NO_FRM_INT
    /* mac解析无缓存帧中断开关 */
    if (pst_pm_debug->ul_cmd_bit_map & BIT(MAC_PM_DEBUG_NO_PS_FRM_INT))
    {
        pst_hal_device->bit_no_ps_frm_int_switch = pst_pm_debug->uc_nfi_switch;
    }
#endif

    return OAL_SUCC;
}
#endif


oal_uint32 dmac_config_phy_debug_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_phy_debug_switch_stru      *pst_phy_debug_switch;
    hal_to_dmac_device_stru        *pst_hal_device = OAL_PTR_NULL;
    dmac_vap_stru                  *pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_phy_debug_switch:: hal_device null.!!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_phy_debug_switch = (mac_phy_debug_switch_stru *)puc_param;

    if(pst_phy_debug_switch->st_tone_tran.uc_tone_tran_switch == 1)
    {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_phy_debug_switch:: tone_tran_switch[%d] data_len[%d] chain[%d].!!}", pst_phy_debug_switch->st_tone_tran.uc_tone_tran_switch, pst_phy_debug_switch->st_tone_tran.us_data_len, pst_phy_debug_switch->st_tone_tran.uc_chain_idx);
        hal_rf_tone_transmit_entrance(pst_hal_device, pst_phy_debug_switch->st_tone_tran.us_data_len, pst_phy_debug_switch->st_tone_tran.uc_chain_idx);
    }
    else if(pst_phy_debug_switch->st_tone_tran.uc_tone_tran_switch == 0)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_phy_debug_switch:: tone_tran_switch[%d].!!}", pst_phy_debug_switch->st_tone_tran.uc_tone_tran_switch);
        hal_rf_tone_transmit_exit(pst_hal_device);
    }

    /* 设置rssi开关信息 */
    oal_memcopy(g_pst_mac_phy_debug_switch, pst_phy_debug_switch, OAL_SIZEOF(mac_phy_debug_switch_stru));

    if (1 == g_pst_mac_phy_debug_switch->uc_force_work_switch)
    {
        hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_JOIN_COMP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
    }
    else if (0 == g_pst_mac_phy_debug_switch->uc_force_work_switch)
    {
        hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_VAP_DOWN, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
    }

    OAM_WARNING_LOG4(0, OAM_SF_ANY, "{dmac_config_phy_debug_switch::rssi[%d]snr[%d]trlr[%d],interval[%d].}",
                          g_pst_mac_phy_debug_switch->en_rssi_debug_switch, g_pst_mac_phy_debug_switch->en_snr_debug_switch,
                          g_pst_mac_phy_debug_switch->en_trlr_debug_switch, g_pst_mac_phy_debug_switch->ul_rx_comp_isr_interval);

    hal_rx_set_trlr_report_info(pst_hal_device, pst_phy_debug_switch->auc_trlr_sel_info, pst_phy_debug_switch->en_trlr_debug_switch);

    return OAL_SUCC;
}

dmac_config_syn_stru g_ast_dmac_config_syn_debug[] =
{
    {WLAN_CFGID_CHECK_FEM_PA,        {0, 0},            dmac_config_get_fem_pa_status},
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    {WLAN_CFGID_CHIP_CHECK_SWITCH,          {0, 0},       dmac_config_chip_check},
    {WLAN_CFGID_SEND_CW_SIGNAL,             {0, 0},       dmac_config_send_cw_signal},
    {WLAN_CFGID_GET_CALI_INFO,              {0, 0},       dmac_config_get_cali_info},
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_80211_UCAST_SWITCH, {0, 0},         dmac_config_80211_ucast_switch},
	{WLAN_CFGID_REPORT_VAP_INFO,         {0, 0},    dmac_config_report_vap_info},
#endif
#ifdef _PRE_DEBUG_MODE_USER_TRACK
    {WLAN_CFGID_USR_THRPUT_STAT,            {0, 0},         dmac_config_report_thrput_stat},
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    {WLAN_CFGID_TXOP_PS_MACHW,              {0, 0},         dmac_config_set_txop_ps_machw},
#endif
    {WLAN_CFGID_80211_MCAST_SWITCH, {0, 0},         dmac_config_80211_mcast_switch},
    {WLAN_CFGID_PROBE_SWITCH,       {0, 0},         dmac_config_probe_switch},
    {WLAN_CFGID_PROTOCOL_DBG,       {0, 0},         dmac_config_protocol_debug_switch},
    {WLAN_CFGID_PHY_DEBUG_SWITCH,   {0, 0},         dmac_config_phy_debug_switch},
#ifdef _PRE_WLAN_FEATURE_WMMAC
    {WLAN_CFGID_WMMAC_SWITCH,       {0, 0},         dmac_config_wmmac_switch},
#endif
    {WLAN_CFGID_GET_MPDU_NUM,       {0, 0},         dmac_config_get_mpdu_num},
#ifdef _PRE_WLAN_CHIP_TEST
    {WLAN_CFGID_SET_BEACON_OFFLOAD_TEST, {0, 0},    dmac_config_beacon_offload_test},
#endif
    {WLAN_CFGID_OTA_BEACON_SWITCH,  {0, 0},         dmac_config_ota_beacon_switch},
    {WLAN_CFGID_OTA_RX_DSCR_SWITCH, {0, 0},         dmac_config_ota_rx_dscr_switch},
    {WLAN_CFGID_SET_ALL_OTA,        {0, 0},         dmac_config_set_all_ota},
    {WLAN_CFGID_OAM_OUTPUT_TYPE,    {0, 0},         dmac_config_oam_output},
    {WLAN_CFGID_SET_FEATURE_LOG,    {0, 0},         dmac_config_set_feature_log},
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_DUMP_TIEMR,         {0, 0},         dmac_config_dump_timer},

    {WLAN_CFGID_SET_STBC_CAP,       {0, 0},         dmac_config_set_stbc_cap},
    {WLAN_CFGID_SET_LDPC_CAP,       {0, 0},         dmac_config_set_ldpc_cap},
    {WLAN_CFGID_TXBF_SWITCH,        {0, 0},         dmac_config_set_txbf_cap},
#endif
    {WLAN_CFGID_PAUSE_TID,          {0, 0},         dmac_config_pause_tid},
    {WLAN_CFGID_SET_USER_VIP,       {0, 0},         dmac_config_set_user_vip},
    {WLAN_CFGID_SET_VAP_HOST,       {0, 0},         dmac_config_set_vap_host},
    {WLAN_CFGID_SEND_BAR,                   {0, 0},         dmac_config_send_bar},
    {WLAN_CFGID_DUMP_BA_BITMAP,    {0, 0},              dmac_config_dump_ba_bitmap},
    {WLAN_CFGID_RESET_HW,                   {0, 0},         dmac_config_reset_hw},
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_RESET_HW_OPERATE,           {0, 0},         dmac_reset_sys_event},
#endif
#ifdef _PRE_WLAN_FEATURE_UAPSD
    {WLAN_CFGID_UAPSD_DEBUG,        {0, 0},         dmac_config_uapsd_debug},
#endif

#ifdef _PRE_WLAN_DFT_STAT
    {WLAN_CFGID_PHY_STAT_EN,                {0, 0},         dmac_config_set_phy_stat_en},
    {WLAN_CFGID_DBB_ENV_PARAM,              {0, 0},         dmac_config_dbb_env_param},
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    {WLAN_CFGID_USR_QUEUE_STAT,             {0, 0},         dmac_config_usr_queue_stat},
#endif
    {WLAN_CFGID_VAP_STAT,                   {0, 0},         dmac_config_report_vap_stat},
    {WLAN_CFGID_ALL_STAT,                   {0, 0},         dmac_config_report_all_stat},
#endif
    {WLAN_CFGID_DUMP_RX_DSCR,               {0, 0},         dmac_config_dump_rx_dscr},
    {WLAN_CFGID_DUMP_TX_DSCR,               {0, 0},         dmac_config_dump_tx_dscr},
    {WLAN_CFGID_ALG,                        {0, 0},         dmac_config_alg},
#ifdef _PRE_FEATURE_FAST_AGING
    {WLAN_CFGID_FAST_AGING,                 {0, 0},         dmac_config_fast_aging},
    {WLAN_CFGID_GET_FAST_AGING,             {0, 0},         dmac_config_get_fast_aging},
#endif
    {WLAN_CFGID_BEACON_CHAIN_SWITCH,        {0, 0},      dmac_config_beacon_chain_switch},
#ifdef _PRE_WLAN_FEATUER_PCIE_TEST
    {WLAN_CFGID_PCIE_TEST,                  {0, 0},         dmac_config_pcie_test},
#endif
    {WLAN_CFGID_RSSI_LIMIT_CFG,             {0, 0},         dmac_config_rssi_limit},

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_RESUME_RX_INTR_FIFO, {0, 0},            dmac_config_resume_rx_intr_fifo},
#endif
#ifdef _PRE_SUPPORT_ACS
    {WLAN_CFGID_ACS_CONFIG,         {0, 0},             dmac_config_acs},
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
#ifdef _PRE_DEBUG_MODE
    {WLAN_CFGIG_DFR_ENABLE,                 {0, 0},         dmac_config_dfr_enable},
    {WLAN_CFGID_TRIG_LOSS_TX_COMP,          {0, 0},         dmac_config_trig_loss_tx_comp},
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    {WLAN_CFGID_TRIG_PCIE_RESET,            {0, 0},         dmac_config_trig_pcie_reset},
#endif
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    {WLAN_CFGID_SET_RFCH,          {0, 0},              dmac_config_set_rfch},
#endif
    {WLAN_CFGID_SET_NSS,           {0, 0},              dmac_config_set_nss},
#ifdef _PRE_DEBUG_MODE
    {WLAN_CFGID_SET_RXCH,          {0, 0},              dmac_config_set_rxch},
    {WLAN_CFGID_DYNC_TXPOWER,      {0, 0},              dmac_config_dync_txpower},
    {WLAN_CFGID_DYNC_POW_DEBUG,    {0, 0},              dmac_config_dync_pow_debug_switch},
#endif
    {WLAN_CFGID_GET_THRUPUT,       {0, 0},              dmac_config_get_thruput},
    {WLAN_CFGID_SET_FREQ_SKEW,     {0, 0},              dmac_config_set_freq_skew},
    {WLAN_CFGID_ADJUST_PPM,        {0, 0},              dmac_config_adjust_ppm},
#ifdef _PRE_WLAN_PERFORM_STAT
    {WLAN_CFGID_PFM_STAT,                  {0, 0},      dmac_config_pfm_stat},
    {WLAN_CFGID_PFM_DISPLAY,               {0, 0},      dmac_config_pfm_display},
#endif
    {WLAN_CFGID_LPM_SOC_MODE,      {0, 0},              dmac_config_set_lpm_soc_mode},
#ifdef _PRE_WLAN_CHIP_TEST
    {WLAN_CFGID_LPM_TX_PROBE_REQUEST,     {0, 0},       dmac_config_lpm_tx_probe_request},
    {WLAN_CFGID_LPM_CHIP_STATE,     {0, 0},             dmac_config_set_lpm_chip_state},
    {WLAN_CFGID_LPM_PSM_PARAM,      {0, 0},             dmac_config_set_lpm_psm_param},
    {WLAN_CFGID_LPM_SMPS_MODE,      {0, 0},             dmac_config_set_lpm_smps_mode},
    {WLAN_CFGID_LPM_SMPS_STUB,      {0, 0},             dmac_config_set_lpm_smps_stub},
    {WLAN_CFGID_LPM_TXOP_PS_SET,    {0, 0},             dmac_config_set_lpm_txop_ps},
    {WLAN_CFGID_LPM_TXOP_TX_STUB,   {0, 0},             dmac_config_set_lpm_txop_ps_tx_stub},
#endif
    {WLAN_CFGID_SEND_FRAME,         {0, 0},             dmac_config_send_frame},
#ifdef _PRE_WLAN_CHIP_TEST
    {WLAN_CFGID_SET_RX_PN_REG,      {0, 0},             dmac_config_set_rx_pn},
    {WLAN_CFGID_SET_SOFT_RETRY,     {0, 0},             dmac_config_set_soft_retry},
    {WLAN_CFGID_OPEN_ADDR4,         {0, 0},             dmac_config_open_addr4},
    {WLAN_CFGID_OPEN_WMM_TEST,      {0, 0},             dmac_config_open_wmm_test},
    {WLAN_CFGID_CHIP_TEST_OPEN,     {0, 0},             dmac_config_chip_test_open},
    {WLAN_CFGID_SET_COEX,           {0, 0},             dmac_config_set_coex},
    {WLAN_CFGID_DFX_SWITCH,         {0, 0},             dmac_config_set_dfx},
    {WLAN_CFGID_REMOVE_LUT,         {0, 0},             dmac_config_remove_user_lut},
    {WLAN_CFGID_SEND_PSPOLL,        {0, 0},             dmac_config_send_pspoll},
    {WLAN_CFGID_SEND_NULLDATA,      {0, 0},             dmac_config_send_nulldata},
    {WLAN_CFGID_CLEAR_ALL_STAT,    {0, 0},              dmac_config_clear_all_stat},
#endif

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    {WLAN_CFGID_PMF_ENABLE,         {0, 0},             dmac_config_enable_pmf},
#endif
#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    {WLAN_CFGID_FREE_RATIO_SET,     {0, 0},             dmac_config_set_gap_free_ratio},
#endif
    {WLAN_CFGID_HIDE_SSID,                 {0, 0},      dmac_config_hide_ssid},
    {WLAN_CFGID_SET_THRUPUT_BYPASS,  {0, 0},            dmac_config_set_thruput_bypass},
#ifdef _PRE_DEBUG_MODE
    {WLAN_CFGID_GET_ALL_REG_VALUE,   {0, 0},            dmac_config_get_all_reg_value},
    {WLAN_CFGID_GET_CALI_DATA,       {0, 0},            dmac_config_get_cali_data},
#endif
#ifdef _PRE_WLAN_FEATURE_DAQ
    {WLAN_CFGID_DATA_ACQ,            {0, 0},            dmac_config_data_acq},
#endif
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
    {WLAN_CFGID_SET_PSD,            {0, 0},             dmac_config_set_psd},
    {WLAN_CFGID_CFG_PSD,            {0, 0},             dmac_config_cfg_psd},
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    {WLAN_CFGID_SET_CSI,            {0, 0},             dmac_config_set_csi},
#endif
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    {WLAN_CFGID_SET_OPMODE_NOTIFY,  {0, 0},             dmac_config_set_opmode_notify},
    {WLAN_CFGID_GET_USER_RSSBW,     {0, 0},             dmac_config_get_user_rssbw},
#endif
#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) && defined(_PRE_WLAN_FEATURE_SMPS)&& defined(_PRE_WLAN_FEATURE_M2S)
    {WLAN_CFGID_SET_M2S_SWITCH,     {0, 0},             dmac_config_set_m2s_switch},
#endif

    {WLAN_CFGID_SET_VAP_NSS,         {0, 0},            dmac_config_set_vap_nss},
#ifdef _PRE_DEBUG_MODE
    {WLAN_CFGID_REPORT_AMPDU_STAT,   {0, 0},            dmac_config_report_ampdu_stat},
#endif
    {WLAN_CFGID_SET_AGGR_NUM,        {0, 0},            dmac_config_set_ampdu_aggr_num},
#ifdef _PRE_DEBUG_MODE
    {WLAN_CFGID_FREQ_ADJUST,         {0, 0},            dmac_config_freq_adjust},
#endif

#ifdef _PRE_WLAN_FEATURE_AP_PM
    {WLAN_CFGID_WIFI_EN,          {0, 0},               dmac_config_wifi_en},
    {WLAN_CFGID_PM_INFO,          {0, 0},               dmac_config_pm_info},
    {WLAN_CFGID_PM_EN,          {0, 0},                 dmac_config_pm_en},
#endif
    {WLAN_CFGID_DEVICE_MEM_LEAK,   {0, 0},              dmac_config_show_device_memleak},
    {WLAN_CFGID_DEVICE_MEM_INFO,   {0, 0},              dmac_config_show_device_meminfo},
#ifdef _PRE_WLAN_FEATURE_P2P
    {WLAN_CFGID_SET_P2P_PS_STAT,    {0, 0},                 dmac_config_set_p2p_ps_stat},
#endif
#ifdef _PRE_WLAN_PROFLING_MIPS
    {WLAN_CFGID_SET_MIPS,           {0, 0},             dmac_config_set_mips},
    {WLAN_CFGID_SHOW_MIPS,          {0, 0},             dmac_config_show_mips},
#endif
#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    {WLAN_CFGID_ENABLE_ARP_OFFLOAD,         {0, 0},     dmac_config_enable_arp_offload},
    {WLAN_CFGID_SHOW_ARPOFFLOAD_INFO,       {0, 0},     dmac_config_show_arpoffload_info},
#endif
#ifdef _PRE_WLAN_DFT_STAT
    {WLAN_CFGID_SET_PERFORMANCE_LOG_SWITCH,  {0, 0},            dmac_config_set_performance_log_switch},
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    {WLAN_CFGID_2040BSS_ENABLE,     {0, 0},             dmac_config_enable_2040bss},
#endif
#ifdef _PRE_WLAN_FEATURE_HILINK
    {WLAN_CFGID_FBT_SCAN_LIST_CLEAR,        {0, 0},     dmac_config_clear_fbt_scan_list},
    {WLAN_CFGID_FBT_SCAN_SPECIFIED_STA,     {0, 0},     dmac_config_fbt_scan_specified_sta},
    {WLAN_CFGID_FBT_SCAN_INTERVAL,          {0, 0},     dmac_config_fbt_scan_interval},
    {WLAN_CFGID_FBT_SCAN_CHANNEL,           {0, 0},     dmac_config_fbt_scan_channel},
    {WLAN_CFGID_FBT_SCAN_REPORT_PERIOD,     {0, 0},     dmac_config_fbt_scan_report_period},
    {WLAN_CFGID_FBT_SCAN_ENABLE,            {0, 0},     dmac_config_fbt_scan_enable},
    {WLAN_CFGID_SET_WHITE_LIST_SSIDHIDEN,   {0, 0},     dmac_config_set_white_lst_ssidhiden},
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    {WLAN_CFGID_SET_MGMT_FRAME_FILTERS,     {0, 0},     dmac_config_set_mgmt_frame_filters},
    {WLAN_CFGID_GET_STA_DIAG_INFO,          {0, 0},     dmac_config_get_sta_diag_info},
    {WLAN_CFGID_GET_VAP_DIAG_INFO,          {0, 0},     dmac_config_get_vap_diag_info},
    {WLAN_CFGID_SET_SENSING_BSSID,          {0, 0},     dmac_config_set_sensing_bssid},
    {WLAN_CFGID_GET_SENSING_BSSID_INFO,     {0, 0},     dmac_config_get_sensing_bssid_info},
#endif
#ifdef _PRE_WLAN_FEATURE_11V
    {WLAN_CFGID_FBT_CHANGE_TO_OTHER_AP,     {0, 0},     dmac_config_fbt_change_to_other_ap},
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    {WLAN_CFGID_AMPDU_TX_ON,                {0, 0},     dmac_config_set_ampdu_tx_hw_on},
#endif
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    {WLAN_CFGID_DYN_CALI_CFG,               {0, 0},     dmac_config_set_dyn_cali_param},
#endif
    {WLAN_CFGID_SET_AUTO_PROTECTION,        {0, 0},     dmac_config_set_auto_protection},
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
    {WLAN_CFGID_USER_EXTEND_ENABLE,         {0, 0},     dmac_config_user_extend_enable},
#endif
#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
    {WLAN_CFGID_PACKET_CAPTURE_SWITCH,      {0, 0},     dmac_config_packet_capture_switch},
#endif
#ifdef _PRE_WLAN_11K_STAT
    {WLAN_CFGID_QUERY_STAT_INFO,             {0, 0},    dmac_config_query_stat_info},
#endif
    {WLAN_CFGID_SET_BW_FIXED,              {0, 0},         dmac_config_set_bw_fixed},

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
        {WLAN_CFGID_BSD_CONFIG,      {0, 0},     dmac_config_bsd},
#endif

#if defined(_PRE_DEBUG_MODE) && defined(_PRE_WLAN_FEATURE_11V)
        {WLAN_CFGID_11V_TX_QUERY,       {0, 0},     dmac_trigger_tx_bsst_query},
        {WLAN_CFGID_11V_TX_REQUEST,     {0, 0},     dmac_trigger_tx_bsst_request},
#endif
#ifdef _PRE_WLAN_WEB_CMD_COMM
    {WLAN_CFGID_GET_HW_FLOW_STAT,              {0, 0},         dmac_config_get_hw_flow_stat},
    {WLAN_CFGID_GET_WME_STAT,              {0, 0},         dmac_config_get_wme_stat},
    {WLAN_CFGID_GET_ANT_RSSI,              {0, 0},         dmac_config_get_ant_rssi},
    {WLAN_CFGID_ANT_RSSI_REPORT,              {0, 0},         dmac_config_ant_rssi_report},
#ifdef _PRE_WLAN_11K_STAT
    {WLAN_CFGID_GET_TX_DELAY_AC,              {0, 0},         dmac_config_get_tx_delay_ac},
#endif
#endif

#ifdef _PRE_WLAN_PRODUCT_1151V200
    {WLAN_CFGID_80M_RTS_DEBUG,      {0, 0},   dmac_config_80m_rts_debug},
#endif

#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
        {WLAN_CFGID_WAVEAPP_32PLUS_USER_ENABLE,      {0, 0},   dmac_config_waveapp_32plus_user_enable},
#endif
#ifdef _PRE_WLAN_FEATURE_DBDC
    {WLAN_CFGID_DBDC_DEBUG_SWITCH,   {0, 0},                   dmac_config_dbdc_debug_switch},
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
    {WLAN_CFGID_FTM_DBG,                {0, 0},                   dmac_config_ftm_dbg},
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_PM_DEBUG_SWITCH,        {0, 0},                   dmac_config_pm_debug_switch},
#endif

    {WLAN_CFGID_BUTT,               {0, 0},             OAL_PTR_NULL},
};
oal_uint32 dmac_get_config_debug_arrysize(oal_void)
{
    return OAL_ARRAY_SIZE(g_ast_dmac_config_syn_debug);
}

#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

