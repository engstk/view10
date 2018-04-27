


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "mac_frame.h"
#include "mac_resource.h"
#include "mac_ie.h"
#include "dmac_ext_if.h"
#include "dmac_main.h"
#include "dmac_vap.h"
#include "dmac_rx_data.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_blockack.h"
#include "dmac_psm_ap.h"
#include "dmac_mgmt_ap.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_scan.h"
#include "dmac_alg_if.h"
#include "mac_vap.h"
#include "dmac_11w.h"
#include "dmac_dft.h"
#include "dmac_mgmt_sta.h"
#include "dmac_p2p.h"
#include "oal_net.h"
#include "dmac_beacon.h"
#include "dmac_dft.h"
#include "dmac_chan_mgmt.h"
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_psm_sta.h"
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
#include "dmac_p2p.h"
#endif

#ifdef _PRE_WLAN_FEATURE_11K
#include "dmac_11k.h"
#endif


#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_device.h"
#include "dmac_resource.h"
#endif


#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "pm_extern.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "dmac_ftm.h"
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#include "dmac_opmode.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "dmac_smps.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "dmac_wmmac.h"
#endif
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#include "dmac_bsd.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_MGMT_CLASSIFIER_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_MAC_BUGFIX_TSF_SYNC

oal_void dmac_rx_mgmt_update_tsf(dmac_vap_stru *pst_dmac_vap,
                                            mac_ieee80211_frame_stru *pst_frame_hdr,
                                            mac_device_stru    *pst_mac_device,
                                            oal_netbuf_stru    *pst_netbuf)
{
    /* 扫描状态、STA UP/PAUSE状态下收到Beacon和probe rsp帧更新TSF */
    if (OAL_TRUE == pst_mac_device->st_scan_params.bit_is_p2p0_scan)
    {
        return;
    }

    if ((MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state)  ||
        ((WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
          && ((MAC_VAP_STATE_PAUSE  == pst_dmac_vap->st_vap_base_info.en_vap_state)
#ifdef _PRE_WLAN_FEATURE_ROAM
          || (MAC_VAP_STATE_ROAMING == pst_dmac_vap->st_vap_base_info.en_vap_state)
#endif
          || (MAC_VAP_STATE_UP == pst_dmac_vap->st_vap_base_info.en_vap_state))))
    {
        if (!OAL_MEMCMP(pst_frame_hdr->auc_address3, pst_dmac_vap->st_vap_base_info.auc_bssid, WLAN_MAC_ADDR_LEN))
        {
             if (WLAN_BEACON == pst_frame_hdr->st_frame_control.bit_sub_type)
             {
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
                dmac_get_tsf_from_bcn(pst_dmac_vap, pst_netbuf);
#endif
                hal_sta_tsf_save(pst_dmac_vap->pst_hal_vap, OAL_TRUE);
             }
             else if (WLAN_PROBE_RSP == pst_frame_hdr->st_frame_control.bit_sub_type)
             {
                 /* 规避部分AP发送的beacon帧和probe rsp帧中携带的tsf值不一致的问题 */
                 hal_sta_tsf_restore(pst_dmac_vap->pst_hal_vap);
             }
        }
        else
        {
            if ((WLAN_PROBE_RSP == pst_frame_hdr->st_frame_control.bit_sub_type)
            || (WLAN_BEACON == pst_frame_hdr->st_frame_control.bit_sub_type))
            {
                hal_sta_tsf_restore(pst_dmac_vap->pst_hal_vap);
            }
        }
    }

}
#endif


oal_bool_enum_uint8 dmac_rx_multi_mgmt_pre_process(mac_device_stru    *pst_mac_device,
                                                                mac_vap_stru     *pst_mac_vap,
                                                                oal_uint8         uc_channel_number,
                                                                oal_uint8         uc_mgmt_type)
{
    oal_bool_enum_uint8          en_need_copy = OAL_TRUE;

    /* 判断接收到的beacon / probe req 是否允许复制给其他vap 处理 */
    switch (uc_mgmt_type)
    {
        case (WLAN_FC0_SUBTYPE_PROBE_REQ | WLAN_FC0_TYPE_MGT):
        {
            //OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RX,
            //    "{dmac_rx_multi_mgmt_pre_process::rx probe req}");
            if (IS_AP(pst_mac_vap)
                && uc_channel_number != pst_mac_vap->st_channel.uc_chan_number)
            {
                en_need_copy = OAL_FALSE;
            }
            else if (IS_STA(pst_mac_vap))
            {
                if (IS_LEGACY_VAP(pst_mac_vap))
                {
                    en_need_copy = OAL_FALSE;
                }
                else
                {
                    if(OAL_FALSE == mac_device_is_listening(pst_mac_device))
                    {
                        en_need_copy = OAL_FALSE;
                    }
                }
            }
            break;
        }
        default:
        {
            en_need_copy = OAL_TRUE;
            break;
        }
    }

    return en_need_copy;
}




oal_uint32 dmac_join_set_dtim_reg_event_process(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                      *pst_event;
    frw_event_hdr_stru                  *pst_event_hdr;
    dmac_ctx_set_dtim_tsf_reg_stru      *pst_reg_params;
    mac_device_stru                     *pst_device;
    dmac_vap_stru                       *pst_dmac_vap;


    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN, "{dmac_join_set_dtim_reg_event_process::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event               = frw_get_event_stru(pst_event_mem);
    pst_event_hdr           = &(pst_event->st_event_hdr);
    pst_reg_params          = (dmac_ctx_set_dtim_tsf_reg_stru *)pst_event->auc_event_data;

    /* 获取device结构的信息 */
    pst_device = mac_res_get_dev(pst_event_hdr->uc_device_id);

    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_SCAN, "{dmac_join_set_dtim_reg_event_process::pst_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_SCAN, "{dmac_join_set_reg_event_process::pst_dmac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }


    /* 写bssid 寄存器 */
    hal_set_sta_bssid(pst_dmac_vap->pst_hal_vap,pst_reg_params->auc_bssid);

    /* */
    mac_vap_set_bssid(&pst_dmac_vap->st_vap_base_info, pst_reg_params->auc_bssid);

    /* 写tsf 寄存器 */
    if(OAL_TRUE == (oal_uint32)pst_reg_params->us_tsf_bit0)
    {
       /* enable tbtt */
       hal_enable_tsf_tbtt(pst_dmac_vap->pst_hal_vap, OAL_FALSE);
    }

    return OAL_SUCC;

}


oal_uint32  dmac_asoc_set_reg_event_process(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                      *pst_event;
    frw_event_hdr_stru                  *pst_event_hdr;
    dmac_ctx_asoc_set_reg_stru          *pst_reg_params;
    dmac_user_stru                      *pst_dmac_user;
    hal_to_dmac_device_stru             *pst_hal_device = OAL_PTR_NULL;

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{dmac_asoc_set_reg_event_process::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event               = frw_get_event_stru(pst_event_mem);
    pst_event_hdr           = &(pst_event->st_event_hdr);
    pst_reg_params          = (dmac_ctx_asoc_set_reg_stru *)pst_event->auc_event_data;


    pst_dmac_user = mac_res_get_dmac_user(pst_reg_params->uc_user_index);

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_ASSOC,
            "{dmac_join_set_reg_event_process::pst_dmac_user[%d] null.}", pst_reg_params->uc_user_index);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取hal device结构的信息 */
    pst_hal_device = dmac_user_get_hal_device(&(pst_dmac_user->st_user_base_info));
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ASSOC, "{dmac_join_set_reg_event_process::dmac_user_get_hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 支持QOS 遍历寄存器*/
    hal_machw_seq_num_index_update_per_tid(pst_hal_device, pst_dmac_user->uc_lut_index, OAL_TRUE);

    return OAL_SUCC;

}



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


