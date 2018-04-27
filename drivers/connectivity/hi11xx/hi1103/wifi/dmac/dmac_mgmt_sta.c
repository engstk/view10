


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "mac_ie.h"
#include "dmac_mgmt_sta.h"
#include "dmac_main.h"
#include "oal_net.h"
#include "dmac_chan_mgmt.h"
#include "dmac_device.h"
#include "dmac_resource.h"
#include "dmac_alg_if.h"
#include "dmac_mgmt_sta.h"
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#endif
#include "dmac_scan.h"
#include "dmac_config.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_tx_complete.h"
#include "dmac_power.h"
#include "dmac_beacon.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_MGMT_STA_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/


oal_void  dmac_chan_multi_select_channel_mac(mac_vap_stru *pst_mac_vap, oal_uint8 uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    mac_device_stru             *pst_device;
    mac_channel_stru             st_channel;
    oal_uint8                    uc_old_chan_number;
    oal_uint8                    uc_idx;
    oal_uint32                   ul_ret;
    hal_to_dmac_device_stru     *pst_hal_device;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
        "{dmac_chan_multi_select_channel_mac::pst_device null,device_id=%d.}", pst_mac_vap->uc_device_id);
        return;
    }

    if (0 == pst_device->uc_vap_num)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_chan_multi_select_channel_mac::none vap.}");
        return;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_chan_multi_select_channel_mac::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return;
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_chan_multi_select_channel_mac:: Switching from chan[%d] BW[%d] mode To chan[%d] BW[%d].}",
                     pst_hal_device->uc_current_chan_number, pst_mac_vap->st_channel.en_bandwidth, uc_channel, en_bandwidth);

    /* 更新VAP下的主20MHz信道号、带宽模式、信道索引 */
    ul_ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band, uc_channel, &uc_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_chan_multi_select_channel_mac::mac_get_channel_idx_from_num failed[%d].}", ul_ret);
        return;
    }

    uc_old_chan_number = pst_mac_vap->st_channel.uc_chan_number;
    pst_mac_vap->st_channel.uc_chan_number = uc_channel;
    pst_mac_vap->st_channel.en_bandwidth   = en_bandwidth;
    pst_mac_vap->st_channel.uc_chan_idx    = uc_idx;

    /* sta和ap的user都需要刷新带宽，并通知算法，并同步到host */
    dmac_chan_update_user_bandwidth(pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 使能去使能雷达检测 */
    if(IS_AP(pst_mac_vap) &&(OAL_TRUE == mac_vap_get_dfs_enable(pst_mac_vap)))
    {
        dmac_dfs_radar_detect_check(pst_hal_device, pst_device, pst_mac_vap);
    }
#endif

    /* 刷新发送功率 */
    dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_INIT);

    /* 通知算法信道改变 */
    dmac_alg_cfg_channel_notify(pst_mac_vap, CH_BW_CHG_TYPE_MOVE_WORK);
    /* 通知算法带宽改变 */
    dmac_alg_cfg_bandwidth_notify(pst_mac_vap, CH_BW_CHG_TYPE_MOVE_WORK);

#ifdef _PRE_WLAN_FEATURE_DBAC
    ul_ret = mac_fcs_dbac_state_check(pst_device);
    if ((mac_is_dbac_running(pst_device)) && (MAC_FCS_DBAC_NEED_CLOSE == ul_ret))
    {
        /* DBAC场景下,切换信道后为同信道,需要关闭DBAC,重新设置2个vap最大带宽 */
        dmac_alg_update_dbac_fcs_config(pst_mac_vap);
        dmac_vap_down_notify(pst_mac_vap);

        st_channel = pst_mac_vap->st_channel;
        dmac_chan_select_real_channel(pst_device, &st_channel, st_channel.uc_chan_number);

        /* 切换信道不需要清fifo，传入FALSE */
        dmac_mgmt_switch_channel(pst_hal_device, &st_channel, OAL_FALSE);
    }
    else if((mac_is_dbac_running(pst_device)) && (MAC_FCS_DBAC_NEED_OPEN == ul_ret))
    {
        /* DBAC场景下,切换信道后，仍是异信道,只需要更新dbac参数 */
        dmac_alg_update_dbac_fcs_config(pst_mac_vap);
        if(pst_hal_device->uc_current_chan_number == uc_old_chan_number)
        {
            /* 如果当前时序是wlan时序，则由于wlan信道变化,切换后不能发送报文 */
            dmac_vap_pause_tx(pst_mac_vap);
        }
    }
    else if((!mac_is_dbac_running(pst_device)) && (MAC_FCS_DBAC_NEED_OPEN == ul_ret))
    {
        /* 非DBAC场景下,切换信道后为异信道, 需要启动DBAC */
        /* wlan vap刚切离信道，需要暂停发送. */
        dmac_vap_pause_tx(pst_mac_vap);
        dmac_alg_vap_up_notify(pst_mac_vap);

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_chan_multi_select_channel_mac:: start dbac,hal chan is %d.}",
                         pst_hal_device->uc_current_chan_number);

        if(pst_hal_device->uc_current_chan_number == pst_mac_vap->st_channel.uc_chan_number)
        {
            /* 如果DBAC启动初始化wlan vap，则重新resume wlan vap发送,否则由DBAC做resume vap的动作 */
            mac_vap_resume_tx(pst_mac_vap);
        }
    }
    else
#endif
    {
        /* 变量初始化为当前vap信息 */
        st_channel = pst_mac_vap->st_channel;

        /* 选择需要设置的信道信息 */
        dmac_chan_select_real_channel(pst_device, &st_channel, st_channel.uc_chan_number);

        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                     "{dmac_chan_multi_select_channel_mac:: vap bw[%d], new set bandwith[%d], hal_device bandwidth=[%d].",
                     pst_mac_vap->st_channel.en_bandwidth, st_channel.en_bandwidth,
                     pst_hal_device->st_wifi_channel_status.en_bandwidth);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* 切换信道不需要清fifo，传入FALSE */
        dmac_mgmt_switch_channel(pst_hal_device, &st_channel, OAL_FALSE);
#else
        dmac_mgmt_switch_channel(pst_hal_device, &st_channel, OAL_TRUE);
#endif
    }
}


oal_void  dmac_chan_sta_switch_channel(mac_vap_stru *pst_mac_vap)
{
    wlan_channel_bandwidth_enum_uint8   en_new_bandwidth = WLAN_BAND_WIDTH_20M;
    dmac_user_stru                     *pst_dmac_user;
    dmac_vap_stru                      *pst_dmac_vap;
    oal_bool_enum                       en_is_support_bw;

    if (WLAN_BAND_WIDTH_BUTT != pst_mac_vap->st_ch_switch_info.en_new_bandwidth)
    {
        dmac_chan_adjust_bandwidth_sta(pst_mac_vap, &en_new_bandwidth);
    }
    else
    {/*csa中没有带宽参数变化，使用之前带宽参数*/
        en_new_bandwidth = pst_mac_vap->st_channel.en_bandwidth;
    }

    en_is_support_bw = mac_regdomain_channel_is_support_bw(en_new_bandwidth, pst_mac_vap->st_ch_switch_info.uc_new_channel);
    if(OAL_FALSE == en_is_support_bw)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CSA,"{dmac_chan_sta_switch_channel::channel[%d] is not support bw[%d],set 20MHz}",
        pst_mac_vap->st_ch_switch_info.uc_new_channel, en_new_bandwidth);
        en_new_bandwidth = WLAN_BAND_WIDTH_20M;
    }

    oal_memcopy(&(pst_mac_vap->st_ch_switch_info.st_old_channel), &pst_mac_vap->st_channel, OAL_SIZEOF(pst_mac_vap->st_ch_switch_info.st_old_channel));

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if(OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CSA,"{dmac_chan_sta_switch_channel:: pst_dmac_vap is NULL,return}");
        return;
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{dmac_chan_sta_switch_channel:: csa Switching from chan[%d] BW[%d] mode To chan[%d] BW[%d].}",
                     pst_mac_vap->st_ch_switch_info.st_old_channel.uc_chan_number,
                     pst_mac_vap->st_ch_switch_info.st_old_channel.en_bandwidth,
                     pst_mac_vap->st_ch_switch_info.uc_new_channel, en_new_bandwidth);

    dmac_vap_linkloss_clean(pst_dmac_vap);

    /* 禁止硬件全部发送直到STA信道切换完毕 */
    dmac_chan_disable_machw_tx(pst_mac_vap);

    /* 切换信道和带宽 */
    dmac_chan_multi_select_channel_mac(pst_mac_vap, pst_mac_vap->st_ch_switch_info.uc_new_channel, en_new_bandwidth);

    /* 设置该变量，避免STA在信道切换时发生link loss */
    //pst_mac_vap->st_ch_switch_info.en_waiting_for_ap           = OAL_TRUE;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    dmac_switch_complete_notify(pst_mac_vap, OAL_FALSE);
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
    /* 信道切换完成按照正常dtim睡眠唤醒 */
    dmac_psm_update_dtime_period(pst_mac_vap, (oal_uint8)mac_mib_get_dot11dtimperiod(pst_mac_vap),
                                    mac_mib_get_BeaconPeriod(pst_mac_vap));

#ifdef _PRE_WLAN_FEATURE_NO_FRM_INT
    /* 恢复最大允许跳过的beaocn数 */
    if (IS_LEGACY_STA(pst_mac_vap))
    {
        pst_dmac_vap->st_sta_pm_handler.uc_max_skip_bcn_cnt = DMAC_PSM_MAX_SKIP_BCN_CNT;
    }
#endif
#endif

    dmac_chan_enable_machw_tx(pst_mac_vap);

    /* 切完信道后删除BA，防止切换信道过程中将硬件队列清空导致BA移窗异常 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_mac_vap->us_assoc_vap_id);
    if(OAL_PTR_NULL != pst_dmac_user)
    {
        dmac_tx_delete_ba(pst_dmac_user);
    }
}

#ifdef _PRE_WLAN_FEATURE_11AX

oal_void dmac_sta_up_update_spatial_reuse_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload,
                                                   oal_uint16 us_frame_len,mac_user_stru *pst_mac_user)
{
    oal_uint32                                       ul_ret;
    oal_uint8                                       *puc_ie;
    mac_frame_he_spatial_reuse_parameter_set_ie_stru st_srp_ie_value;

    puc_ie = mac_find_ie_ext_ie(MAC_EID_HE,MAC_EID_EXT_HE_SRP, puc_payload, us_frame_len);
    if (OAL_PTR_NULL == puc_ie)
    {
        return;
    }

    /* 支持11ax，才进行后续的处理 */
    if (OAL_FALSE == mac_mib_get_HEOptionImplemented(pst_mac_vap))
    {
        return;
    }

    OAL_MEMZERO(&st_srp_ie_value, OAL_SIZEOF(st_srp_ie_value));
    ul_ret = mac_ie_parse_spatial_reuse_parameter(puc_ie,&st_srp_ie_value);
    if(OAL_SUCC != ul_ret)
    {
        return;
    }

    //TODO

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_11AX,"{dmac_sta_up_update_spatial_reuse_params:: bit_srp_disallowed= %d}",
        st_srp_ie_value.st_sr_control.bit_srp_disallowed);

    return;
}



oal_uint32 dmac_sta_up_update_he_oper_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload,
                                                   oal_uint16   us_frame_len,mac_user_stru *pst_mac_user)
{
    oal_uint8        *puc_he_opern_ie;
    oal_uint32        ul_change = MAC_NO_CHANGE;

    /* 支持11ax，才进行后续的处理 */
    if (OAL_FALSE == mac_mib_get_HEOptionImplemented(pst_mac_vap))
    {
        return ul_change;
    }

    puc_he_opern_ie = mac_find_ie_ext_ie(MAC_EID_HE, MAC_EID_EXT_HE_OPERATION, puc_payload, us_frame_len);
    if (OAL_PTR_NULL == puc_he_opern_ie)
    {
        return ul_change;
    }

    ul_change = mac_ie_proc_he_opern_ie(pst_mac_vap, puc_he_opern_ie, pst_mac_user);

    return ul_change;
}


#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

