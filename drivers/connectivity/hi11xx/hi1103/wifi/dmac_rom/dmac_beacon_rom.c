


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "wlan_spec.h"
#include "wlan_types.h"
#include "oal_types.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "oam_main.h"

#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
#include "pm_extern.h"
#endif

#include "hal_ext_if.h"
#include "mac_frame.h"
#include "mac_ie.h"
#include "mac_vap.h"
#include "mac_frame.h"


#include "dmac_main.h"
#include "dmac_device.h"
#include "dmac_beacon.h"
#include "dmac_chan_mgmt.h"
#include "dmac_psm_ap.h"
#include "dmac_mgmt_sta.h"
#include "dmac_mgmt_ap.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_device.h"
#include "dmac_scan.h"
#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_test_main.h"
#include "dmac_lpm_test.h"
#endif
#ifdef _PRE_WLAN_DFT_STAT
#include "mac_device.h"
#include "dmac_dft.h"
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#include "dmac_sta_pm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_AP_PM
#include "dmac_ap_pm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
#include "dmac_p2p.h"
#endif
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "dmac_reset.h"
#endif
#include "dmac_config.h"
#include "dmac_tx_bss_comm.h"
#ifdef _PRE_WLAN_FEATURE_11K
#include "dmac_11k.h"
#endif
#include "dmac_vap.h"
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "dmac_smps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "dmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
#include "dmac_pkt_capture.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_BEACON_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_beacon_cb   g_st_beacon_rom_cb = {OAL_PTR_NULL};

/* 2.4G频段 信道与中心频率映射 */
OAL_CONST mac_freq_channel_map_stru g_dmac_ast_freq_map_2g[MAC_CHANNEL_FREQ_2_BUTT] =
{
    { 2412, 1, 0},
    { 2417, 2, 1},
    { 2422, 3, 2},
    { 2427, 4, 3},
    { 2432, 5, 4},
    { 2437, 6, 5},
    { 2442, 7, 6},
    { 2447, 8, 7},
    { 2452, 9, 8},
    { 2457, 10, 9},
    { 2462, 11, 10},
    { 2467, 12, 11},
    { 2472, 13, 12},
    { 2484, 14, 13},
};
/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*lint -save -e506 */

oal_uint32  dmac_ch_status_sync_event(mac_vap_stru *pst_mac_vap, mac_ap_ch_info_stru *past_ap_channel_list)
{
    frw_event_mem_stru                      *pst_event_mem;          /* 申请事件返回的内存指针 */
    frw_event_stru                          *pst_event;              /* 指向申请事件的payload指针 */
    oal_uint32                               ul_ret;

    pst_event_mem = FRW_EVENT_ALLOC_BIG(OAL_SIZEOF(mac_ap_ch_info_stru)*MAC_MAX_SUPP_CHANNEL);
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ch_status_sync_event: alloc event failed! size=%d}", OAL_SIZEOF(mac_ap_ch_info_stru)*MAC_MAX_SUPP_CHANNEL);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_SDT_REG,
                       DMAC_TO_HMAC_CH_STATUS_INFO_SYN,
                       OAL_SIZEOF(mac_ap_ch_info_stru)*MAC_MAX_SUPP_CHANNEL,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    oal_memcopy(frw_get_event_payload(pst_event_mem), (oal_uint8 *)past_ap_channel_list, OAL_SIZEOF(mac_ap_ch_info_stru)*MAC_MAX_SUPP_CHANNEL);

    /* 分发 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_ch_status_sync_event::frw_event_dispatch_event fail, ul_ret=%d}", ul_ret);
    }

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return ul_ret;
}
/*lint -restore */


oal_uint32  dmac_chan_sync_event(mac_vap_stru *pst_mac_vap, dmac_set_chan_stru *pst_set_chan)
{
    frw_event_mem_stru                      *pst_event_mem;          /* 申请事件返回的内存指针 */
    frw_event_stru                          *pst_event;              /* 指向申请事件的payload指针 */
    oal_uint32                               ul_ret;

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_set_chan_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_chan_sysnc_event: alloc event failed! size=%d}", OAL_SIZEOF(dmac_set_chan_stru));

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_SDT_REG,
                       DMAC_TO_HMAC_BANDWIDTH_INFO_SYN,
                       OAL_SIZEOF(dmac_set_chan_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);

    oal_memcopy(&pst_set_chan->st_channel, &pst_mac_vap->st_channel, OAL_SIZEOF(mac_channel_stru));
    oal_memcopy(&pst_set_chan->st_ch_switch_info, &pst_mac_vap->st_ch_switch_info, OAL_SIZEOF(mac_ch_switch_info_stru));
    pst_set_chan->en_dot11FortyMHzIntolerant = mac_mib_get_FortyMHzIntolerant(pst_mac_vap);

    /* ROM钩子,返回成功则继续处理下面逻辑 */
    if (OAL_PTR_NULL != g_st_beacon_rom_cb.chan_event_cb)
    {
        if (OAL_RETURN == g_st_beacon_rom_cb.chan_event_cb(pst_event_mem, pst_mac_vap,pst_set_chan))
        {
            return OAL_SUCC;
        }
    }


    oal_memcopy(frw_get_event_payload(pst_event_mem), (oal_uint8 *)pst_set_chan, OAL_SIZEOF(dmac_set_chan_stru));

    /* 分发 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_chan_sysnc_event::frw_event_dispatch_event fail, ul_ret=%d}", ul_ret);
    }

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return ul_ret;
}


oal_uint32  dmac_protection_sync_event(mac_vap_stru *pst_mac_vap, mac_h2d_protection_stru *pst_h2d_prot)
{
    frw_event_mem_stru                      *pst_event_mem;          /* 申请事件返回的内存指针 */
    frw_event_stru                          *pst_event;              /* 指向申请事件的payload指针 */
    oal_uint32                               ul_ret;

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(mac_h2d_protection_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_chan_sysnc_event: alloc event failed! size=%d}", OAL_SIZEOF(mac_h2d_protection_stru));

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获得事件指针 */
    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_SDT_REG,
                       DMAC_TO_HMAC_PROTECTION_INFO_SYN,
                       OAL_SIZEOF(mac_h2d_protection_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_vap->uc_chip_id,
                       pst_mac_vap->uc_device_id,
                       pst_mac_vap->uc_vap_id);


    oal_memcopy(frw_get_event_payload(pst_event_mem), (oal_uint8 *)pst_h2d_prot, OAL_SIZEOF(mac_h2d_protection_stru));

    /* 分发 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_chan_sysnc_event::frw_event_dispatch_event fail, ul_ret=%d}", ul_ret);
    }

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

    return ul_ret;
}


oal_void dmac_init_dtim_count_ap(dmac_vap_stru *pst_dmac_vap)
{
    if (0 == mac_mib_get_dot11dtimperiod(&pst_dmac_vap->st_vap_base_info))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_WIFI_BEACON, "{dmac_init_dtim_count_ap::DTIM period error!}");
        return;
    }
    pst_dmac_vap->uc_dtim_count = (oal_uint8)(mac_mib_get_dot11dtimperiod(&pst_dmac_vap->st_vap_base_info) - 1);
}


oal_void dmac_update_dtim_count_ap(dmac_vap_stru *pst_dmac_vap)
{
    /* 将vap中保存的dtim_count值赋给DTIM Count字段，每次都会减1，在区间[0,DTIM_period]中循环 */
    if (0 == pst_dmac_vap->uc_dtim_count)
    {
        pst_dmac_vap->uc_dtim_count = (oal_uint8)(mac_mib_get_dot11dtimperiod(&pst_dmac_vap->st_vap_base_info) - 1);
    }
    else
    {
        pst_dmac_vap->uc_dtim_count--;
    }
}


oal_void dmac_set_tim_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    dmac_vap_stru  *pst_dmac_vap;
    oal_uint8       uc_tim_offset = 0;
    oal_uint8       uc_PVBitmap_len;

    /***************************************************************************
     ---------------------------------------------------------------------------
     |EID |Len |DTIM Count |DTIM Period |Bitmap Control |Partial Virtual Bitmap|
     ---------------------------------------------------------------------------
     |1   |1   |1          |1           |1              |1~251                 |
     ---------------------------------------------------------------------------
    ***************************************************************************/

    pst_dmac_vap = (dmac_vap_stru *)pst_vap;

    uc_PVBitmap_len  = pst_dmac_vap->puc_tim_bitmap[0];
    uc_tim_offset    = 2 + (pst_dmac_vap->puc_tim_bitmap[1] & (oal_uint8)(~BIT0));
    if (uc_PVBitmap_len + uc_tim_offset > pst_dmac_vap->uc_tim_bitmap_len)
    {
        OAM_ERROR_LOG3(0, OAM_SF_WIFI_BEACON, "{dmac_set_tim_ie::puc_tim_bitmap[0x%x] = 0x%x bitmap_len[%d] .}",
                       pst_dmac_vap->puc_tim_bitmap, OAL_NTOH_32(*(oal_uint32 *)(pst_dmac_vap->puc_tim_bitmap)), pst_dmac_vap->uc_tim_bitmap_len);
        uc_PVBitmap_len = 1;
    }

    puc_buffer[0] = MAC_EID_TIM;
    puc_buffer[1] = uc_PVBitmap_len + MAC_TIM_LEN_EXCEPT_PVB;
    puc_buffer[2] = pst_dmac_vap->uc_dtim_count;
    puc_buffer[3] = (oal_uint8)mac_mib_get_dot11dtimperiod(&pst_dmac_vap->st_vap_base_info);
    puc_buffer[4] = pst_dmac_vap->puc_tim_bitmap[1];

#ifdef _PRE_WLAN_MAC_BUGFIX_MCAST_HW_Q
    /* 保存Beacon帧中DTIM Control bit0的值，用于下次TBTT中断到来时来判断是否需要发送缓存广播和组播帧 */
    pst_dmac_vap->en_dtim_ctrl_bit0 = (oal_bool_enum_uint8)(pst_dmac_vap->puc_tim_bitmap[1] & BIT0);
#endif
    /* 将vap本地的bitmap不为0的部分复制到beacon帧相应空间，作为Partial Virtual Bitmap */
    oal_memcopy(&puc_buffer[5], pst_dmac_vap->puc_tim_bitmap + uc_tim_offset, uc_PVBitmap_len);


    *puc_ie_len = MAC_IE_HDR_LEN + MAC_TIM_LEN_EXCEPT_PVB + uc_PVBitmap_len;
}



oal_uint32  dmac_beacon_free(dmac_vap_stru *pst_dmac_vap)
{
    OAL_MEM_FREE(pst_dmac_vap->pauc_beacon_buffer[0], OAL_TRUE);
    OAL_MEM_FREE(pst_dmac_vap->pauc_beacon_buffer[1], OAL_TRUE);

    pst_dmac_vap->pauc_beacon_buffer[0] = OAL_PTR_NULL;
    pst_dmac_vap->pauc_beacon_buffer[1] = OAL_PTR_NULL;

    /* 关闭tsf */
    hal_disable_tsf_tbtt(pst_dmac_vap->pst_hal_vap);

    return OAL_SUCC;
}


oal_void  dmac_tbtt_exception_handler(hal_to_dmac_device_stru  *pst_hal_device)
{
    hal_dfr_err_opern_stru          *pst_dfr_err;
    hal_mac_error_type_enum_uint8    en_error_id;

    /* 遍历所有错误类型 */
    for (en_error_id = 0; en_error_id < HAL_MAC_ERROR_TYPE_BUTT; en_error_id++)
    {
        pst_dfr_err = &(pst_hal_device->st_dfr_err_opern[en_error_id]);

        pst_dfr_err->us_tbtt_cnt++;

        if (pst_dfr_err->us_tbtt_cnt >= 10)
        {
            pst_dfr_err->us_tbtt_cnt = 0;

            /* 每10次TBTT中断，将此计数清零 */
            pst_dfr_err->us_err_cnt = 0;
        }
    }
}



oal_uint32  dmac_send_disasoc_misc_event(mac_vap_stru *pst_mac_vap, oal_uint16 us_user_idx, oal_uint16 us_disasoc_reason)
{
    frw_event_mem_stru       *pst_event_mem;
    frw_event_stru           *pst_event;
    oal_uint32                ul_ret;
    dmac_disasoc_misc_stru    st_disasoc_misc;

    /* 申请事件内存 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_disasoc_misc_stru));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                "{dmac_send_disasoc_misc_event::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                            FRW_EVENT_TYPE_DMAC_MISC,
                            DMAC_MISC_SUB_TYPE_DISASOC,
                            WLAN_MEM_EVENT_SIZE1,
                            FRW_EVENT_PIPELINE_STAGE_1,
                            pst_mac_vap->uc_chip_id,
                            pst_mac_vap->uc_device_id,
                            pst_mac_vap->uc_vap_id);

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_WIFI_BEACON, "{dmac_send_disasoc_misc_event::user_id=%d reason=%d}",
                    us_user_idx, us_disasoc_reason);

    st_disasoc_misc.en_disasoc_reason = us_disasoc_reason;
    st_disasoc_misc.us_user_idx       = us_user_idx;
    oal_memcopy(pst_event->auc_event_data, &st_disasoc_misc, OAL_SIZEOF(dmac_disasoc_misc_stru));

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);

    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WIFI_BEACON, "{dmac_send_disasoc_misc_event::frw_event_dispatch_event failed[%d].}", ul_ret);
        FRW_EVENT_FREE(pst_event_mem);
        return ul_ret;
    }

    /* 释放事件 */
    FRW_EVENT_FREE(pst_event_mem);
    return OAL_SUCC;
}

oal_bool_enum_uint8 dmac_sta_edca_is_changed(
                mac_vap_stru  *pst_mac_vap,
                oal_uint8     *puc_frame_body,
                oal_uint16    us_frame_len)
{
    oal_uint8         uc_param_set_cnt;
    oal_uint8         uc_modify_flag = OAL_FALSE;
    const oal_uint8   *puc_edca_ie;

    puc_edca_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM, puc_frame_body, us_frame_len);

    if (OAL_PTR_NULL == puc_edca_ie)
    {
        return OAL_FALSE;
    }

    if (OAL_TRUE == mac_is_wmm_ie((oal_uint8*)puc_edca_ie))
    {
        uc_param_set_cnt = puc_edca_ie[DMAC_WMM_QOS_PARAMS_HDR_LEN] & 0x0F;

        if (uc_param_set_cnt != pst_mac_vap->uc_wmm_params_update_count)
        {
            OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_FRAME_FILTER,
                    "dmac_sta_edca_is_changed::edca is changed=%d, old_cnt=%d new_cnt=%d\r\n",
                    uc_modify_flag, pst_mac_vap->uc_wmm_params_update_count, uc_param_set_cnt);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            mac_vap_set_wmm_params_update_count(pst_mac_vap, uc_param_set_cnt);
#endif
            uc_modify_flag = OAL_TRUE;
        }
    }

    return uc_modify_flag;
}
#if 0

oal_bool_enum_uint8 dmac_sta_channel_is_changed(
                mac_vap_stru  *pst_mac_vap,
                oal_uint8     *puc_frame_body,
                oal_uint16    us_frame_len)
{
    oal_uint8        uc_channel;
    oal_uint8        uc_modify_flag;
    oal_uint8       *puc_dsss_ie;

    //puc_dsss_ie = mac_get_dsss_param_ie(puc_frame_body, us_frame_len, 0);
    puc_dsss_ie = mac_find_ie(MAC_EID_DSPARMS, puc_frame_body, us_frame_len);
    if (OAL_PTR_NULL == puc_dsss_ie)
    {
        return OAL_FALSE;
    }

    uc_channel = puc_dsss_ie[MAC_IE_HDR_LEN];

    if (uc_channel != pst_mac_vap->st_channel.uc_chan_number)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_FRAME_FILTER,
                "dmac_sta_channel_is_changed::AP channel is changed, old_channel=%d new_channel=%d\r\n",
                pst_mac_vap->st_channel.uc_chan_number, uc_channel);
        uc_modify_flag = OAL_TRUE;
    }
    else
    {
        uc_modify_flag = OAL_FALSE;
    }

    return uc_modify_flag;
}
#endif


oal_void dmac_sta_update_slottime(mac_vap_stru* pst_mac_vap, mac_user_stru *pst_mac_user, oal_uint8 *puc_payload, oal_uint16 us_msg_len)
{
    dmac_vap_stru              *pst_dmac_vap;
    mac_erp_params_stru        *pst_erp_ie;
    const oal_uint8            *puc_erp_ie;
    hal_to_dmac_device_stru    *pst_hal_device;

    pst_dmac_vap = (dmac_vap_stru*)pst_mac_vap;

    /* 保存non_erp信息,sta模式下，保存BSS中是否有erp站点的信息 */
    puc_erp_ie = mac_find_ie(MAC_EID_ERP, puc_payload, us_msg_len);
    if (OAL_PTR_NULL == puc_erp_ie)
    {
        return;
    }

    /* IE长度异常，不更新slottime类型 */
    if ( puc_erp_ie[1] < MAC_ERP_IE_LEN)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{dmac_sta_update_slottime::invalid erp ie len[%d].}", puc_erp_ie[1]);
        return;
    }

    /* NON_ERP站点进出时更新slottime类型 */
    pst_erp_ie = (mac_erp_params_stru*)&(puc_erp_ie[MAC_IE_HDR_LEN]);
    if (pst_dmac_vap->en_non_erp_exist != pst_erp_ie->bit_non_erp)
    {
        pst_dmac_vap->en_non_erp_exist = pst_erp_ie->bit_non_erp;

        pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
        if (OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{dmac_sta_update_slottime::hal dev null, fail to update slot time.}");
            return;
        }

        /* 有NON_ERP站点时，使用short slottime发数据；否则使用long slottime */
        hal_cfg_slottime_type(pst_hal_device, pst_erp_ie->bit_non_erp);
    }
}

oal_void dmac_sta_up_process_erp_ie(
                    oal_uint8               *puc_payload,
                    oal_uint16               us_msg_len,
                    mac_user_stru           *pst_mac_user)
{
    mac_erp_params_stru    *pst_erp_params;
    oal_uint8              *puc_erp_ie;

    /************************ ERP Element *************************************
    --------------------------------------------------------------------------
    |EID  |Len  |NonERP_Present|Use_Protection|Barker_Preamble_Mode|Reserved|
    --------------------------------------------------------------------------
    |B0-B7|B0-B7|B0            |B1            |B2                  |B3-B7   |
    --------------------------------------------------------------------------
    ***************************************************************************/

    puc_erp_ie = mac_find_ie(MAC_EID_ERP, puc_payload, us_msg_len);

    if (OAL_PTR_NULL == puc_erp_ie)
    {
        return;
    }

    /* IE长度校验 */
    if ( puc_erp_ie[1] < MAC_ERP_IE_LEN)
    {
        OAM_WARNING_LOG1(0, OAM_SF_RX, "{dmac_sta_up_process_erp_ie::invalid erp ie len[%d].}", puc_erp_ie[1]);
        return;
    }

    pst_erp_params = (mac_erp_params_stru *)(puc_erp_ie + MAC_IE_HDR_LEN);

    if (pst_mac_user->st_cap_info.bit_barker_preamble_mode != pst_erp_params->bit_preamble_mode)
    {
        OAM_WARNING_LOG2(0, OAM_SF_FRAME_FILTER,"dmac_sta_is_erp_ie_changed::ap new barker mode[%d],defalut[%d]",
                            pst_erp_params->bit_preamble_mode,pst_mac_user->st_cap_info.bit_barker_preamble_mode);

        /* 保存preamble mode */
        mac_user_set_barker_preamble_mode(pst_mac_user, pst_erp_params->bit_preamble_mode);
    }

    /* 检查erp保护是否改变 */
    if (pst_mac_user->st_cap_info.bit_erp_use_protect != pst_erp_params->bit_use_protection)
    {
        OAM_WARNING_LOG2(0, OAM_SF_FRAME_FILTER,"dmac_sta_is_erp_ie_changed::ap new erp protection[%d],defalut[%d]",
                            pst_erp_params->bit_use_protection,pst_mac_user->st_cap_info.bit_erp_use_protect);

        pst_mac_user->st_cap_info.bit_erp_use_protect = pst_erp_params->bit_use_protection;
    }
}

oal_uint32 dmac_protection_set_rtscts_mechanism(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_flag, wlan_prot_mode_enum_uint8 en_prot_mode)
{
    mac_cfg_rts_tx_param_stru   st_rts_tx_param;
    hal_cfg_rts_tx_param_stru   st_hal_rts_tx_param;
    hal_to_dmac_device_stru    *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_protection_set_rtscts_mechanism::pst_hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_protection_set_rts_tx_param(pst_mac_vap,en_flag,en_prot_mode,&st_rts_tx_param);

    /*mac参数赋值给hal参数*/
    oal_memcopy(&st_hal_rts_tx_param, &st_rts_tx_param, OAL_SIZEOF(st_hal_rts_tx_param));

    hal_set_rts_rate_params(pst_hal_device, &st_hal_rts_tx_param);

    /*数据帧/管理帧发送时候，需要根据bit_rts_cts_protect_mode值填写发送描述符中的RTS/CTS enable位*/
    pst_mac_vap->st_protection.bit_rts_cts_protect_mode = en_flag;

    return OAL_SUCC;
}

oal_uint32 dmac_protection_set_erp_protection(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_flag)
{
    return dmac_protection_set_rtscts_mechanism(pst_mac_vap, en_flag, WLAN_PROT_ERP);
}


oal_uint32 dmac_protection_set_ht_protection(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_flag)
{
    oal_uint32      ul_ret = OAL_SUCC;
    oal_bool_enum   en_lsigtxop = OAL_FALSE;

    en_lsigtxop = mac_protection_lsigtxop_check(pst_mac_vap);
    /*优先使用lsigtxop保护，开销小*/
    if (OAL_TRUE == en_lsigtxop)
    {
        mac_protection_set_lsig_txop_mechanism(pst_mac_vap, en_flag);
    }
    else
    {
        ul_ret = dmac_protection_set_rtscts_mechanism(pst_mac_vap, en_flag, WLAN_PROT_HT);
        if(OAL_SUCC != ul_ret)
        {
            return ul_ret;
        }
    }

    return ul_ret;
}


oal_bool_enum_uint8  dmac_sta_11ntxbf_is_changed(mac_user_stru *pst_mac_user,oal_uint8 *puc_frame_body,oal_uint16  us_frame_len)
{
#ifdef _PRE_WLAN_FEATURE_TXBF
    mac_11ntxbf_vendor_ie_stru      *pst_vendor_ie;
    oal_uint8                       *puc_txbf_vendor_ie;

    /* 更新11n txbf能力 */
    puc_txbf_vendor_ie = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_EID_11NTXBF, puc_frame_body, us_frame_len);
    if (puc_txbf_vendor_ie != OAL_PTR_NULL)
    {
        /* 检测到vendor ie*/
        pst_vendor_ie = (mac_11ntxbf_vendor_ie_stru *)puc_txbf_vendor_ie;
        if (pst_mac_user->st_cap_info.bit_11ntxbf != pst_vendor_ie->st_11ntxbf.bit_11ntxbf)
        {
            OAM_WARNING_LOG2(0, OAM_SF_FRAME_FILTER,"dmac_sta_is_txbf_changed::ap new 11ntxbf[%d],defalut[%d]",
                                    pst_vendor_ie->st_11ntxbf.bit_11ntxbf,pst_mac_user->st_cap_info.bit_11ntxbf);
            pst_mac_user->st_cap_info.bit_11ntxbf = pst_vendor_ie->st_11ntxbf.bit_11ntxbf;
            return OAL_TRUE;
        }
    }
#endif
    return OAL_FALSE;
}




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


