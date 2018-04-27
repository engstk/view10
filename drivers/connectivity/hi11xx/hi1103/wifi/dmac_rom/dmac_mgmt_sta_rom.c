


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

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_MGMT_STA_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_mgmt_sta_cb g_st_dmac_mgmt_sta_rom_cb = {OAL_PTR_NULL,
                                              OAL_PTR_NULL};


/*****************************************************************************
  3 函数实现
*****************************************************************************/


oal_uint32 dmac_mgmt_wmm_update_edca_machw_sta(frw_event_mem_stru  *pst_event_mem)
{
    frw_event_stru                      *pst_event;
    frw_event_hdr_stru                  *pst_event_hdr;
    dmac_ctx_sta_asoc_set_edca_reg_stru *pst_reg_params;
    mac_device_stru                     *pst_device;
    dmac_vap_stru                       *pst_dmac_vap;
    wlan_wme_ac_type_enum_uint8          en_ac_type;
    hal_to_dmac_device_stru             *pst_hal_device = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_WMMAC
    mac_user_stru                       *pst_mac_user;
    oal_uint8                            uc_wmm_ac_loop;
#endif

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_WMM, "{dmac_mgmt_wmm_update_edca_machw_sta::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event               = frw_get_event_stru(pst_event_mem);
    pst_event_hdr           = &(pst_event->st_event_hdr);
    pst_reg_params          = (dmac_ctx_sta_asoc_set_edca_reg_stru *)pst_event->auc_event_data;

    /* 获取device结构的信息 */
    pst_device = mac_res_get_dev(pst_event_hdr->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_WMM, "{dmac_mgmt_wmm_update_edca_machw_sta::pst_device[idx=%d] null.}", pst_event_hdr->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_reg_params->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_WMM, "{dmac_mgmt_wmm_update_edca_machw_sta::pst_dmac_vap[idx=%d] null.}", pst_reg_params->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (MAC_WMM_SET_PARAM_TYPE_DEFAULT == pst_reg_params->en_set_param_type)
    {
        mac_mib_set_dot11QosOptionImplemented(&pst_dmac_vap->st_vap_base_info, OAL_FALSE);

        
   #if 0
        /*去使能EDCA*/
        hal_disable_machw_edca(pst_device->pst_device_stru);

        mac_mib_set_dot11QosOptionImplemented(&pst_dmac_vap->st_vap_base_info, OAL_FALSE);

        /* 设置VO默认参数 */
        hal_vap_set_machw_aifsn_ac(pst_dmac_vap->pst_hal_vap, WLAN_WME_AC_VO, DMAC_WMM_VO_DEFAULT_DECA_AIFSN);

        hal_vap_set_edca_machw_cw(pst_dmac_vap->pst_hal_vap,
                                 (oal_uint8)mac_mib_get_QAPEDCATableCWmax(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_VO),
                                 (oal_uint8)mac_mib_get_QAPEDCATableCWmin(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_VO),
                                  WLAN_WME_AC_VO);
        hal_vap_set_machw_prng_seed_val_all_ac(pst_dmac_vap->pst_hal_vap);
   #endif

        return OAL_SUCC;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_memcopy((oal_uint8*)&pst_dmac_vap->st_vap_base_info.pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac, (oal_uint8*)&pst_reg_params->ast_wlan_mib_qap_edac, (OAL_SIZEOF(wlan_mib_Dot11QAPEDCAEntry_stru) * WLAN_WME_AC_BUTT));
#endif

#ifdef _PRE_WLAN_FEATURE_WMMAC
    pst_mac_user = mac_res_get_mac_user(pst_dmac_vap->st_vap_base_info.us_assoc_vap_id);
    if (OAL_PTR_NULL != pst_mac_user)
    {
        /* User不为空时，针对每一个AC，更新EDCA参数，初始化TS状态 */
        for (uc_wmm_ac_loop = 0; uc_wmm_ac_loop < WLAN_WME_AC_BUTT; uc_wmm_ac_loop++)
        {
            pst_mac_user->st_ts_info[uc_wmm_ac_loop].en_ts_status =
            (OAL_TRUE == mac_mib_get_QAPEDCATableMandatory(&(pst_dmac_vap->st_vap_base_info), uc_wmm_ac_loop)) ?
              MAC_TS_INIT : MAC_TS_NONE;
        }
    }
#endif //_PRE_WLAN_FEATURE_WMMAC

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_WMM, "{dmac_mgmt_wmm_update_edca_machw_sta:: hal device nullpointer!!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_enable_machw_edca(pst_hal_device);

    /* alg edca opt 生效时, 不配置BEACON中参数 */
    if((HAL_ALG_INTF_DET_ADJINTF_NO != pst_hal_device->st_hal_alg_stat.en_adj_intf_state)
      || (OAL_TRUE == pst_hal_device->st_hal_alg_stat.en_co_intf_state))
    {
        return OAL_SUCC;
    }
    /* 更新edca寄存器参数 */
    hal_vap_set_machw_aifsn_all_ac(pst_dmac_vap->pst_hal_vap,
                                   (oal_uint8)mac_mib_get_QAPEDCATableAIFSN(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_BK),
                                   (oal_uint8)mac_mib_get_QAPEDCATableAIFSN(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_BE),
                                   (oal_uint8)mac_mib_get_QAPEDCATableAIFSN(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_VI),
                                   (oal_uint8)mac_mib_get_QAPEDCATableAIFSN(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_VO));

    for (en_ac_type = 0; en_ac_type < WLAN_WME_AC_BUTT; en_ac_type++)
    {
        hal_vap_set_edca_machw_cw(pst_dmac_vap->pst_hal_vap,
                                 (oal_uint8)mac_mib_get_QAPEDCATableCWmax(&pst_dmac_vap->st_vap_base_info, en_ac_type),
                                 (oal_uint8)mac_mib_get_QAPEDCATableCWmin(&pst_dmac_vap->st_vap_base_info, en_ac_type),
                                  en_ac_type);
    }

    /* TXOP不使能时,同步AP参数;否则使用配置值 */
    if(OAL_FALSE == pst_device->en_txop_enable)
    {
        hal_vap_set_machw_txop_limit_bkbe(pst_dmac_vap->pst_hal_vap,
                                          (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_BE),
                                          (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_BK));
    }

    hal_vap_set_machw_txop_limit_vivo(pst_dmac_vap->pst_hal_vap,
                                      (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_VO),
                                      (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_VI));

    /*DTS: 1102 beacon帧中EDCA参数中没有LIFETIME值，STA根据本地mib值更新(mib值为0),
    注释掉这个配置，按照寄存器配置来生效即可 */

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    hal_vap_set_machw_edca_bkbe_lifetime(pst_dmac_vap->pst_hal_vap,
                                         (oal_uint16)mac_mib_get_QAPEDCATableMSDULifetime(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_BE),
                                         (oal_uint16)mac_mib_get_QAPEDCATableMSDULifetime(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_BK));
    hal_vap_set_machw_edca_vivo_lifetime(pst_dmac_vap->pst_hal_vap,
                                         (oal_uint16)mac_mib_get_QAPEDCATableMSDULifetime(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_VO),
                                         (oal_uint16)mac_mib_get_QAPEDCATableMSDULifetime(&pst_dmac_vap->st_vap_base_info, WLAN_WME_AC_VI));
#endif


    if (OAL_PTR_NULL != g_st_dmac_mgmt_sta_rom_cb.update_edca_sta_cb)
    {
        g_st_dmac_mgmt_sta_rom_cb.update_edca_sta_cb(pst_event_mem, pst_dmac_vap);
    }

    return OAL_SUCC;
}


oal_void  dmac_chan_adjust_bandwidth_sta(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 *pen_bandwidth)
{
    wlan_channel_bandwidth_enum_uint8   en_curr_bandwidth;
    wlan_channel_bandwidth_enum_uint8   en_announced_bandwidth;
    en_curr_bandwidth      = pst_mac_vap->st_channel.en_bandwidth;
    en_announced_bandwidth = pst_mac_vap->st_ch_switch_info.en_new_bandwidth;
    *pen_bandwidth = en_curr_bandwidth;

    /* 如果当前带宽模式与新带宽模式相同，则直接返回 */
    if (en_announced_bandwidth == en_curr_bandwidth)
    {
        return;
    }

    if (WLAN_BAND_WIDTH_20M == en_announced_bandwidth)
    {
        *pen_bandwidth = WLAN_BAND_WIDTH_20M;
    }
    else   /* 新带宽模式不是20MHz，则STA侧带宽模式需要根据自身能力进行匹配 */
    {
        /* 使能40MHz */
        /* (1) 用户开启"40MHz运行"特性(即STA侧 dot11FortyMHzOperationImplemented为true) */
        /* (2) AP在40MHz运行 */
        if (OAL_TRUE == mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap))
        {
            switch (en_announced_bandwidth)
            {
                case WLAN_BAND_WIDTH_40PLUS:
                case WLAN_BAND_WIDTH_80PLUSPLUS:
                case WLAN_BAND_WIDTH_80PLUSMINUS:
                    *pen_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                    break;

                case WLAN_BAND_WIDTH_40MINUS:
                case WLAN_BAND_WIDTH_80MINUSPLUS:
                case WLAN_BAND_WIDTH_80MINUSMINUS:
                    *pen_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                    break;

                default:
                    *pen_bandwidth = WLAN_BAND_WIDTH_20M;
                    break;
            }
        }

        /* 使能80MHz */
        /* (1) 用户支持80MHz带宽(即STA侧 dot11VHTChannelWidthOptionImplemented为0) */
        /* (2) MAC DEVICE支持80MHz */
        if (OAL_TRUE == mac_mib_get_VHTOptionImplemented(pst_mac_vap))
        {
            *pen_bandwidth = mac_vap_get_bandwith(mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap), en_announced_bandwidth);
        }
    }
}

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST

oal_uint8  dmac_mgmt_is_active_htsta(mac_vap_stru *pst_mac_vap)
{
    //TODO 目前该函数始终返回true, 如有必要在此处加上判断sta是否为活跃ht sta的代码
    return OAL_TRUE;
}


oal_uint8  dmac_mgmt_need_obss_scan(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru                     *pst_device;

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_2040, "dmac_mgmt_need_obss_scan::null param");
        return OAL_FALSE;
    }

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SCAN,
            "{dmac_mgmt_need_obss_scan::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    if (0 != pst_device->st_p2p_info.uc_p2p_goclient_num)
    {
        return OAL_FALSE;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (mac_vap_is_vsta(pst_mac_vap))
    {
        return OAL_FALSE;
    }
#endif

    if (OAL_PTR_NULL != g_st_dmac_mgmt_sta_rom_cb.need_obss_scan_cb)
    {
        return g_st_dmac_mgmt_sta_rom_cb.need_obss_scan_cb(pst_device, pst_mac_vap);
    }

    if ( (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode) &&
         ((MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state) || (MAC_VAP_STATE_PAUSE == pst_mac_vap->en_vap_state)) &&
         (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)&&
         (OAL_TRUE == mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap)) &&
         (OAL_TRUE == mac_mib_get_2040BSSCoexistenceManagementSupport(pst_mac_vap)) &&
         (OAL_TRUE == mac_vap_get_peer_obss_scan(pst_mac_vap)) &&
         (OAL_TRUE == dmac_mgmt_is_active_htsta(pst_mac_vap))
          )
     {
        return OAL_TRUE;
     }

    return OAL_FALSE;
}
#endif


oal_uint32 dmac_sta_up_update_ht_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload,
                                                   oal_uint16 us_frame_len,mac_user_stru *pst_mac_user)
{
    oal_uint32             ul_change             = MAC_NO_CHANGE;
    oal_uint8             *puc_ie                = OAL_PTR_NULL;

    puc_ie = mac_find_ie(MAC_EID_HT_OPERATION, puc_payload, us_frame_len);
    if (OAL_PTR_NULL != puc_ie)
    {
        ul_change  = mac_proc_ht_opern_ie(pst_mac_vap, puc_ie, pst_mac_user);
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    puc_ie = mac_find_ie(MAC_EID_OBSS_SCAN, puc_payload, us_frame_len);
    if (OAL_PTR_NULL != puc_ie)
    {
        /* 处理 Overlapping BSS Scan Parameters IE */
        mac_ie_proc_obss_scan_ie(pst_mac_vap, puc_ie);
    }
    else
    {
        /* 找不到OBSS IE，将OBSS扫描标志置为False，放在else分支而不放在查找OBSS IE之前是为了避免之前已经置为TRUE，
           实际有OBSS IE，但在查找之前置为FALSE引入其他问题*/
        mac_vap_set_peer_obss_scan(pst_mac_vap, OAL_FALSE);
    }
#endif /* _PRE_WLAN_FEATURE_20_40_80_COEXIST */

    return ul_change;
}

oal_uint32 dmac_sta_up_update_vht_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload,
                                                   oal_uint16   us_frame_len,mac_user_stru *pst_mac_user)
{
    oal_uint8        *puc_vht_opern_ie;
    oal_uint32        ul_change = MAC_NO_CHANGE;

    /* 支持11ac，才进行后续的处理 */
    if (OAL_FALSE == mac_mib_get_VHTOptionImplemented(pst_mac_vap))
    {
        return ul_change;
    }

    puc_vht_opern_ie = mac_find_ie(MAC_EID_VHT_OPERN, puc_payload, us_frame_len);
    if (OAL_PTR_NULL == puc_vht_opern_ie)
    {
        return ul_change;
    }

    ul_change = mac_ie_proc_vht_opern_ie(pst_mac_vap, puc_vht_opern_ie, pst_mac_user);

    return ul_change;
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


