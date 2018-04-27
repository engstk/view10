


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_mem.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "hal_ext_if.h"
#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#include "dmac_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_VAP_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
oal_uint8 g_uc_host_large_amsdu_enable = OAL_FALSE;
#endif

#ifdef _PRE_WLAN_FEATURE_11AX

oal_void  mac_vap_init_mib_11ax(mac_vap_stru  *pst_mac_vap)
{
    mac_device_stru                 *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_11AX, "{mac_vap_init_mib_11ax::pst_mac_dev[%d] null.}", pst_mac_vap->uc_device_id);

        return;
    }

    /*MAC Capabilities Info*/
    mac_mib_set_HEOptionImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_he_HTControlFieldSupported(pst_mac_vap,OAL_TRUE);
    mac_mib_set_he_TWTOptionActivated(pst_mac_vap,OAL_TRUE);
    mac_mib_set_he_OperatingModeIndication(pst_mac_vap,OAL_TRUE);
    mac_mib_set_he_TriggerMacPaddingDuration(pst_mac_vap,MAC_TRIGGER_FRAME_PADDING_DURATION16us);/*16us*/
    mac_mib_set_he_MaxAMPDULength(pst_mac_vap,7);
    mac_mib_set_he_MultiBSSIDImplemented(pst_mac_vap,OAL_TRUE);
    mac_mib_set_he_BSRSupport(pst_mac_vap,OAL_FALSE);

    /*PHY Capabilities Info*/
    if(WLAN_BAND_CAP_2G_5G == pst_mac_dev->en_band_cap)
    {
        mac_mib_set_he_DualBandSupport(pst_mac_vap,OAL_TRUE);/*支持双频*/
    }
    mac_mib_set_he_LDPCCodingInPayload(pst_mac_vap,MAC_DEVICE_GET_CAP_LDPC(pst_mac_dev));/*支持LDPC编码*/
    mac_mib_set_he_SUBeamformer(pst_mac_vap,MAC_DEVICE_GET_CAP_SUBFER(pst_mac_dev));
    mac_mib_set_he_SUBeamformee(pst_mac_vap,MAC_DEVICE_GET_CAP_SUBFEE(pst_mac_dev));
    mac_mib_set_he_MUBeamformer(pst_mac_vap,MAC_DEVICE_GET_CAP_MUBFER(pst_mac_dev));
    mac_mib_set_he_SRPBaseSR(pst_mac_vap,OAL_TRUE);/*支持SRP-Based SR*/

    /*Tx Rx MCS NSS*/
    mac_mib_set_he_HighestNSS(pst_mac_vap,MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
}



OAL_STATIC oal_void  mac_vap_init_11ax_mcs_singlenss(mac_vap_stru           *pst_mac_vap,
                                                     wlan_channel_bandwidth_enum_uint8    en_bandwidth)
{
    mac_tx_max_he_mcs_map_stru         *pst_tx_max_mcs_map;
    mac_tx_max_he_mcs_map_stru         *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_tx_mcs_map(pst_mac_vap));

    /* 20MHz带宽的情况下，支持MCS0-MCS9 ==03 AX支持最大支持情况待确认*/
    if (WLAN_BAND_WIDTH_20M == en_bandwidth)
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        mac_mib_set_he_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_20M_11AX);
        mac_mib_set_he_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_20M_11AX);
    }
    /* 40MHz带宽的情况下，支持MCS0-MCS9 */
    else if ((WLAN_BAND_WIDTH_40MINUS == en_bandwidth) || (WLAN_BAND_WIDTH_40PLUS == en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        mac_mib_set_he_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11AX);
        mac_mib_set_he_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_40M_11AX);
    }

    /* 80MHz带宽的情况下，支持MCS0-MCS9 */
    else if ((WLAN_BAND_WIDTH_80MINUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_80MINUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_80PLUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_80PLUSPLUS == en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        mac_mib_set_he_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_80M_11AX);
        mac_mib_set_he_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_80M_11AX);
    }
}



OAL_STATIC oal_void  mac_vap_init_11ax_mcs_doublenss(mac_vap_stru *pst_mac_vap,
                wlan_channel_bandwidth_enum_uint8    en_bandwidth)
{
    mac_tx_max_he_mcs_map_stru         *pst_tx_max_mcs_map;
    mac_rx_max_he_mcs_map_stru         *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_he_mcs_map_stru *)(mac_mib_get_ptr_he_tx_mcs_map(pst_mac_vap));

    /* 20MHz带宽的情况下，支持MCS0-MCS8 */
    if (WLAN_BAND_WIDTH_20M == en_bandwidth)
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        mac_mib_set_he_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_20M_11AX);
        mac_mib_set_he_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_20M_11AX);
    }

    /* 40MHz带宽的情况下，支持MCS0-MCS9 */
    else if ((WLAN_BAND_WIDTH_40MINUS == en_bandwidth) || (WLAN_BAND_WIDTH_40PLUS == en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11AX);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_40M_11AX);
    }

    /* 80MHz带宽的情况下，支持MCS0-MCS9 */
    else if ((WLAN_BAND_WIDTH_80MINUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_80MINUSPLUS== en_bandwidth)
             || (WLAN_BAND_WIDTH_80PLUSMINUS== en_bandwidth)
             || (WLAN_BAND_WIDTH_80PLUSPLUS== en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AX_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_80M_11AX);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_80M_11AX);
    }
}



oal_void mac_vap_init_11ax_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev)
{
    /* 先将TX RX MCSMAP初始化为所有空间流都不支持 0xFFFF*/
    mac_mib_set_he_rx_mcs_map(pst_mac_vap, 0xFFFF);
    mac_mib_set_he_tx_mcs_map(pst_mac_vap, 0xFFFF);

    if (WLAN_SINGLE_NSS == MAC_DEVICE_GET_NSS_NUM(pst_mac_dev))
    {
        /* 1个空间流的情况 */
        mac_vap_init_11ax_mcs_singlenss(pst_mac_vap, pst_mac_vap->st_channel.en_bandwidth);
    }
    else if (WLAN_DOUBLE_NSS == MAC_DEVICE_GET_NSS_NUM(pst_mac_dev))
    {
        /* 2个空间流的情况 */
        mac_vap_init_11ax_mcs_doublenss(pst_mac_vap, pst_mac_vap->st_channel.en_bandwidth);
    }
    else
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_11AX, "{mac_vap_init_11ax_rates::invalid en_nss_num[%d].}", MAC_DEVICE_GET_NSS_NUM(pst_mac_dev));
    }

}

#else
oal_void  mac_vap_init_mib_11ax(mac_vap_stru  *pst_mac_vap)
{
    return;
}

oal_void mac_vap_init_11ax_rates(mac_vap_stru *pst_mac_vap, mac_device_stru *pst_mac_dev)
{
    return;
}

#endif

oal_void mac_vap_init_mib_11n_rom_cb(mac_vap_stru *pst_mac_vap)
{
    /* txbf能力信息 注:11n txbf用宏区分 */
#if (defined(_PRE_WLAN_FEATURE_TXBF) && defined(_PRE_WLAN_FEATURE_TXBF_HT))
    mac_device_stru               *pst_dev;

    pst_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dev)
    {
        MAC_WARNING_LOG(pst_mac_vap->uc_vap_id, "mac_vap_init_mib_11n_rom_cb: pst_dev is null ptr");
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_mib_11n_rom_cb::pst_dev null.}");

        return;
    }

    mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFER(pst_dev));
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFEE(pst_dev));
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_DELAYED);
    mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, HT_BFEE_NTX_SUPP_ANTA_NUM);
#else
    mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
    mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_INCAPABLE);
    mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, 1);
#endif
}


oal_void mac_vap_init_11ac_mcs_singlenss_rom_cb(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
#ifdef _PRE_WLAN_FEATURE_160M
    mac_tx_max_mcs_map_stru         *pst_tx_max_mcs_map;
    mac_rx_max_mcs_map_stru         *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_rx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_tx_mcs_map(pst_mac_vap));

    if ((WLAN_BAND_WIDTH_160PLUSPLUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160PLUSPLUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160PLUSMINUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160PLUSMINUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160MINUSPLUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160MINUSPLUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160MINUSMINUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160MINUSMINUSMINUS== en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_160M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_SINGLE_NSS_160M_11AC);
    }
#endif
}


oal_void mac_vap_init_11ac_mcs_doublenss_rom_cb(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
 #ifdef _PRE_WLAN_FEATURE_160M
    mac_tx_max_mcs_map_stru         *pst_tx_max_mcs_map;
    mac_rx_max_mcs_map_stru         *pst_rx_max_mcs_map;

    /* 获取mib值指针 */
    pst_rx_max_mcs_map = (mac_rx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_rx_mcs_map(pst_mac_vap));
    pst_tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(mac_mib_get_ptr_vht_tx_mcs_map(pst_mac_vap));

    if ((WLAN_BAND_WIDTH_160PLUSPLUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160PLUSPLUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160PLUSMINUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160PLUSMINUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160MINUSPLUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160MINUSPLUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160MINUSMINUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_160MINUSMINUSMINUS== en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        mac_mib_set_us_rx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_160M_11AC);
        mac_mib_set_us_tx_highest_rate(pst_mac_vap, MAC_MAX_RATE_DOUBLE_NSS_160M_11AC);
    }
#endif
}


#ifdef _PRE_WLAN_FEATURE_11V
/* 默认支持11v 如需关闭请上层调用接口 */
OAL_STATIC oal_void  mac_vap_init_mib_11v(mac_vap_stru  *pst_vap)
{
    mac_mib_set_MgmtOptionBSSTransitionActivated(pst_vap, OAL_TRUE);
    mac_mib_set_MgmtOptionBSSTransitionImplemented(pst_vap, OAL_TRUE);
    mac_mib_set_WirelessManagementImplemented(pst_vap, OAL_TRUE);
}
#endif

oal_void mac_init_mib_rom_cb(mac_vap_stru *pst_mac_vap)
{
#ifdef _PRE_WLAN_FEATURE_11V
    /* 11k */
    mac_vap_init_mib_11v(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* txopps 初始化关闭状态 */
    mac_mib_set_txopps(pst_mac_vap, OAL_FALSE);
#endif


}

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
oal_void mac_blacklist_free_pointer(mac_vap_stru *pst_mac_vap, mac_blacklist_info_stru *pst_blacklist_info)
{
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        g_auc_valid_blacklist_idx[pst_blacklist_info->uc_blacklist_device_index] &= ~(oal_uint8)((oal_uint32)1 << pst_blacklist_info->uc_blacklist_vap_index);
        OAL_MEMZERO(pst_blacklist_info, OAL_SIZEOF(mac_blacklist_info_stru));
        pst_blacklist_info->uc_blacklist_device_index = 0xFF;
        pst_blacklist_info->uc_blacklist_vap_index = 0xFF;
    }
}
#endif
#ifdef _PRE_WLAN_FEATURE_PROXY_ARP

oal_err_code_enum_uint32 mac_proxy_init_vap(mac_vap_stru  *pst_mac_vap)
{
    pst_mac_vap->pst_vap_proxyarp = OAL_PTR_NULL;
    return OAL_SUCC;
}
#endif


oal_uint32 mac_vap_init(
                mac_vap_stru               *pst_vap,
                oal_uint8                   uc_chip_id,
                oal_uint8                   uc_device_id,
                oal_uint8                   uc_vap_id,
                mac_cfg_add_vap_param_stru *pst_param)
{
    oal_uint32                     ul_loop;
    wlan_mib_ieee802dot11_stru    *pst_mib_info;
    mac_device_stru               *pst_mac_device = mac_res_get_dev(uc_device_id);
    oal_uint8                     *puc_addr;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_vap_init::pst_mac_device[%d] null!}", uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_vap->uc_chip_id     = uc_chip_id;
    pst_vap->uc_device_id   = uc_device_id;
    pst_vap->uc_vap_id      = uc_vap_id;
    pst_vap->en_vap_mode    = pst_param->en_vap_mode;
    pst_vap->ul_core_id     = pst_mac_device->ul_core_id;

    pst_vap->bit_has_user_bw_limit  = OAL_FALSE;
    pst_vap->bit_vap_bw_limit   = 0;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    pst_vap->bit_voice_aggr     = OAL_TRUE;
#else
    pst_vap->bit_voice_aggr     = OAL_FALSE;
#endif
    pst_vap->bit_one_tx_tcp_be  = OAL_FALSE;
    pst_vap->bit_one_rx_tcp_be  = OAL_FALSE;
    pst_vap->bit_no_tcp_or_udp  = OAL_FALSE;
    pst_vap->bit_bw_fixed       = OAL_FALSE;
    pst_vap->bit_use_rts_threshold = OAL_FALSE;

    oal_set_mac_addr_zero(pst_vap->auc_bssid);

    for (ul_loop = 0; ul_loop < MAC_VAP_USER_HASH_MAX_VALUE; ul_loop++)
    {
        oal_dlist_init_head(&(pst_vap->ast_user_hash[ul_loop]));
    }

    /* cache user 锁初始化 */
    oal_spin_lock_init(&pst_vap->st_cache_user_lock);

    oal_dlist_init_head(&pst_vap->st_mac_user_list_head);

    /* 初始化支持2.4G 11ac私有增强 */
#ifdef _PRE_WLAN_FEATURE_11AC2G
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    pst_vap->st_cap_flag.bit_11ac2g = pst_param->bit_11ac2g_enable;
#else
    pst_vap->st_cap_flag.bit_11ac2g = OAL_TRUE;
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* 根据定制化刷新2g ht40能力 */
    pst_vap->st_cap_flag.bit_disable_2ght40 = pst_param->bit_disable_capab_2ght40;
#else
    pst_vap->st_cap_flag.bit_disable_2ght40 = OAL_FALSE;
#endif

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    if (IS_STA(pst_vap) && (WLAN_LEGACY_VAP_MODE == pst_param->en_p2p_mode))
    {
        /* 仅LEGACY_STA支持 */
        pst_vap->st_cap_flag.bit_ip_filter = OAL_TRUE;
    }
    else
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */
    {
        pst_vap->st_cap_flag.bit_ip_filter = OAL_FALSE;
    }

/* 将hera用于设置ie的指针置空，当接口命令下发后再申请内存 */
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
    pst_vap->pst_rrm_ie_info        = OAL_PTR_NULL;
    pst_vap->pst_excap_ie_info      = OAL_PTR_NULL;
    pst_vap->pst_msta_ie_info       = OAL_PTR_NULL;
#endif

    switch(pst_vap->en_vap_mode)
    {
        case WLAN_VAP_MODE_CONFIG :
            pst_vap->uc_init_flag = MAC_VAP_VAILD;//CFG VAP也需置位保证不重复初始化
            return OAL_SUCC;
        case WLAN_VAP_MODE_BSS_STA:
        case WLAN_VAP_MODE_BSS_AP:
            /* 设置vap参数默认值 */
            pst_vap->us_assoc_vap_id = MAC_INVALID_USER_ID;
            pst_vap->us_cache_user_id = MAC_INVALID_USER_ID;
            pst_vap->uc_tx_power     = WLAN_MAX_TXPOWER;
            pst_vap->st_protection.en_protection_mode    = WLAN_PROT_NO;

            pst_vap->st_cap_flag.bit_dsss_cck_mode_40mhz = OAL_TRUE;

            /* 初始化特性标识 */
            pst_vap->st_cap_flag.bit_uapsd      = WLAN_FEATURE_UAPSD_IS_OPEN;
       #ifdef _PRE_WLAN_FEATURE_UAPSD
       #if defined(_PRE_PRODUCT_ID_HI110X_HOST) || defined(_PRE_PRODUCT_ID_HI110X_DEV)
            if (WLAN_VAP_MODE_BSS_AP == pst_vap->en_vap_mode)
            {
                pst_vap->st_cap_flag.bit_uapsd      = g_uc_uapsd_cap;
            }
       #endif
       #endif
            /* 初始化dpd能力 */
            pst_vap->st_cap_flag.bit_dpd_enbale = OAL_TRUE;

            pst_vap->st_cap_flag.bit_dpd_done   = OAL_FALSE;
            /* 初始化TDLS prohibited关闭 */
            pst_vap->st_cap_flag.bit_tdls_prohibited                = OAL_FALSE;
            /* 初始化TDLS channel switch prohibited关闭 */
            pst_vap->st_cap_flag.bit_tdls_channel_switch_prohibited = OAL_FALSE;

            /* 初始化KeepALive开关 */
            pst_vap->st_cap_flag.bit_keepalive   = OAL_TRUE;
            /* 初始化安全特性值 */
            //pst_vap->uc_80211i_mode              = OAL_FALSE;
            pst_vap->st_cap_flag.bit_wpa         = OAL_FALSE;
            pst_vap->st_cap_flag.bit_wpa2        = OAL_FALSE;

            mac_vap_set_peer_obss_scan(pst_vap, OAL_FALSE);

            //OAL_MEMZERO(&(pst_vap->st_key_mgmt), sizeof(mac_key_mgmt_stru));

            /* 初始化协议模式与带宽为非法值，需通过配置命令配置 */
            pst_vap->st_channel.en_band         = WLAN_BAND_BUTT;
            pst_vap->st_channel.en_bandwidth    = WLAN_BAND_WIDTH_BUTT;
            pst_vap->st_channel.uc_chan_number  = 0;
            pst_vap->en_protocol  = WLAN_PROTOCOL_BUTT;

            /*设置自动保护开启*/
            pst_vap->st_protection.bit_auto_protection = OAL_SWITCH_ON;

            OAL_MEMZERO(pst_vap->ast_app_ie, OAL_SIZEOF(mac_app_ie_stru) * OAL_APP_IE_NUM);
            /* 初始化vap wmm开关，默认打开 */
            pst_vap->en_vap_wmm = OAL_TRUE;

            /*设置初始化rx nss值,之后按协议初始化 */
            pst_vap->en_vap_rx_nss = WLAN_NSS_LIMIT;

            /* 设置VAP状态为初始状态INIT */
            mac_vap_state_change(pst_vap, MAC_VAP_STATE_INIT);

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
            mac_proxy_init_vap(pst_vap);
#endif /* #ifdef _PRE_WLAN_FEATURE_PROXY_ARP */

/* 清mac vap下的uapsd的状态,否则状态会有残留，导致host device uapsd信息不同步 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
        OAL_MEMZERO(&(pst_vap->st_sta_uapsd_cfg),OAL_SIZEOF(mac_cfg_uapsd_sta_stru));
#endif/* #ifdef _PRE_WLAN_FEATURE_STA_PM */
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
            if (WLAN_VAP_MODE_BSS_AP == pst_vap->en_vap_mode)
            {
                mac_tx_qos_enhance_list_init(pst_vap);
            }
#endif
            break;
        case WLAN_VAP_MODE_WDS:
            /* TBD 初始化wds特性标识 */
            break;
        case WLAN_VAP_MODE_MONITOER :
            /* TBD */
            break;
        case WLAN_VAP_HW_TEST:
            /* TBD */
            break;
        default :
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{mac_vap_init::invalid vap mode[%d].}", pst_vap->en_vap_mode);

            return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* 申请MIB内存空间，配置VAP没有MIB */
    if ((WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode) ||
        (WLAN_VAP_MODE_BSS_AP == pst_vap->en_vap_mode) ||
        (WLAN_VAP_MODE_WDS == pst_vap->en_vap_mode))
    {
        pst_vap->pst_mib_info = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_MIB, OAL_SIZEOF(wlan_mib_ieee802dot11_stru), OAL_TRUE);
        if (OAL_PTR_NULL == pst_vap->pst_mib_info)
        {
            OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init::pst_mib_info alloc null, size[%d].}", OAL_SIZEOF(wlan_mib_ieee802dot11_stru));
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        pst_mib_info = pst_vap->pst_mib_info;
        OAL_MEMZERO(pst_mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru));

        /* 设置mac地址 */
        puc_addr = mac_mib_get_StationID(pst_vap);
        oal_set_mac_addr(puc_addr, pst_mac_device->auc_hw_addr);
        puc_addr[WLAN_MAC_ADDR_LEN - 1] += uc_vap_id;

        /* 初始化mib值 */
        mac_init_mib(pst_vap);

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
        /* 从定制化获取是否开启AMSDU_AMPDU*/
        mac_mib_set_AmsduPlusAmpduActive(pst_vap, g_uc_host_large_amsdu_enable);
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI
        if (WLAN_LEGACY_VAP_MODE == pst_param->en_p2p_mode)
        {
            mac_vap_vowifi_init(pst_vap);
        }
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#ifdef _PRE_WLAN_FEATURE_TXBF
        if (OAL_TRUE == mac_mib_get_TransmitStaggerSoundingOptionImplemented(pst_vap))
        {
            pst_vap->st_txbf_add_cap.bit_exp_comp_txbf_cap = OAL_TRUE;
        }
        pst_vap->st_txbf_add_cap.bit_imbf_receive_cap  = 0;
        pst_vap->st_txbf_add_cap.bit_channel_est_cap   = 0;
        pst_vap->st_txbf_add_cap.bit_min_grouping      = 0;
        pst_vap->st_txbf_add_cap.bit_csi_bfee_max_rows = 0;
        pst_vap->bit_ap_11ntxbf                        = 0;
        pst_vap->st_cap_flag.bit_11ntxbf               = 1;
#endif
#ifdef _PRE_WLAN_FEATURE_1024QAM
        pst_vap->st_cap_flag.bit_1024qam               = 1;
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
        pst_vap->st_cap_flag.bit_opmode                = 1;
#endif

        /* sta以最大能力启用 */
        if (WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode)
        {
            /* 初始化sta协议模式为11ac */
            switch(pst_mac_device->en_protocol_cap)
            {
                case WLAN_PROTOCOL_CAP_LEGACY:
                case WLAN_PROTOCOL_CAP_HT:
                     pst_vap->en_protocol                = WLAN_HT_MODE;
                     break;

                case WLAN_PROTOCOL_CAP_VHT:
                     pst_vap->en_protocol                = WLAN_VHT_MODE;
                     break;
            #ifdef _PRE_WLAN_FEATURE_11AX
                 case WLAN_PROTOCOL_CAP_HE:
                     pst_vap->en_protocol                = WLAN_HE_MODE;
                     break;
            #endif

                default:
                     OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_init::en_protocol_cap[%d] is not supportted.}", pst_mac_device->en_protocol_cap);
                     return OAL_ERR_CODE_CONFIG_UNSUPPORT;
            }

            switch(MAC_DEVICE_GET_CAP_BW(pst_mac_device))
            {
                case WLAN_BW_CAP_20M:
                     MAC_VAP_GET_CAP_BW(pst_vap) = WLAN_BAND_WIDTH_20M;
                     break;

                case WLAN_BW_CAP_40M:
                     MAC_VAP_GET_CAP_BW(pst_vap) = WLAN_BAND_WIDTH_40MINUS;
                     break;

                case WLAN_BW_CAP_80M:
                     MAC_VAP_GET_CAP_BW(pst_vap) = WLAN_BAND_WIDTH_80PLUSMINUS;
                     break;

        #ifdef _PRE_WLAN_FEATURE_160M
                case WLAN_BW_CAP_160M:
                     MAC_VAP_GET_CAP_BW(pst_vap) = WLAN_BAND_WIDTH_160PLUSPLUSMINUS;
                     break;
        #endif

                default:
                     OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_init::en_bandwidth_cap[%d] is not supportted.}", MAC_DEVICE_GET_CAP_BW(pst_mac_device));
                     return OAL_ERR_CODE_CONFIG_UNSUPPORT;

            }

            switch(pst_mac_device->en_band_cap)
            {
                case WLAN_BAND_CAP_2G:
                     pst_vap->st_channel.en_band         = WLAN_BAND_2G;
                     break;

                case WLAN_BAND_CAP_5G:
                case WLAN_BAND_CAP_2G_5G:
                     pst_vap->st_channel.en_band         = WLAN_BAND_5G;
                     break;

                default:
                     OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_init::en_band_cap[%d] is not supportted.}", pst_mac_device->en_band_cap);
                     return OAL_ERR_CODE_CONFIG_UNSUPPORT;

            }

            if(OAL_SUCC != mac_vap_init_by_protocol(pst_vap, pst_vap->en_protocol))
            {
                mac_vap_free_mib(pst_vap);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            mac_vap_init_rates(pst_vap);
        }
    }

    pst_vap->uc_init_flag = MAC_VAP_VAILD;

    return OAL_SUCC;
}


oal_uint32  mac_vap_check_ap_usr_opern_bandwidth(mac_vap_stru *pst_mac_sta, mac_user_stru *pst_mac_user)
{
    mac_user_ht_hdl_stru                *pst_mac_ht_hdl;
    mac_vht_hdl_stru                    *pst_mac_vht_hdl;
    wlan_channel_bandwidth_enum_uint8    en_sta_current_bw;
    wlan_channel_bandwidth_enum_uint8    en_bandwidth_ap = WLAN_BAND_WIDTH_20M;
    oal_uint32                           ul_change = 0;
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_he_hdl_stru                    *pst_mac_he_hdl;
#endif

    /* 获取HT和VHT结构体指针 */
    pst_mac_ht_hdl  = &(pst_mac_user->st_ht_hdl);
    pst_mac_vht_hdl = &(pst_mac_user->st_vht_hdl);
#ifdef _PRE_WLAN_FEATURE_11AX
    pst_mac_he_hdl = &(pst_mac_user->st_he_hdl);
#endif

    en_sta_current_bw = MAC_VAP_GET_CAP_BW(pst_mac_sta);

    if (OAL_TRUE == pst_mac_vht_hdl->en_vht_capable)
    {
        en_bandwidth_ap = mac_get_bandwith_from_center_freq_seg0(pst_mac_vht_hdl->en_channel_width,
            pst_mac_sta->st_channel.uc_chan_number, pst_mac_vht_hdl->uc_channel_center_freq_seg0);
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if((OAL_TRUE == pst_mac_he_hdl->en_he_capable) &&
            (OAL_TRUE == pst_mac_he_hdl->st_he_oper_ie.st_he_oper_param.bit_vht_operation_info_present))
    {
        en_bandwidth_ap = mac_get_bandwith_from_center_freq_seg0(pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_channel_width,
            pst_mac_sta->st_channel.uc_chan_number, pst_mac_he_hdl->st_he_oper_ie.st_vht_operation_info.uc_center_freq_seg0);

    }
#endif

    /* ht 20/40M带宽的处理 */
    if ((OAL_TRUE == pst_mac_ht_hdl->en_ht_capable) && (en_bandwidth_ap <=  WLAN_BAND_WIDTH_40MINUS)
            && (OAL_TRUE == mac_mib_get_FortyMHzOperationImplemented(pst_mac_sta)))
    {
        /* 更新带宽模式 */
        en_bandwidth_ap = mac_get_bandwidth_from_sco(pst_mac_ht_hdl->bit_secondary_chan_offset);
    }

    if((en_sta_current_bw != en_bandwidth_ap) && (en_bandwidth_ap <= mac_mib_get_dot11VapMaxBandWidth(pst_mac_sta)))
    {
        /*防止每个beacon都去检查en_bandwidth_ap*/
        if(OAL_FALSE == mac_regdomain_channel_is_support_bw(en_bandwidth_ap, pst_mac_sta->st_channel.uc_chan_number))
        {
            en_bandwidth_ap = WLAN_BAND_WIDTH_20M;
            if(en_sta_current_bw != en_bandwidth_ap)
            {/* 防止刷屏打印*/
                OAM_WARNING_LOG2(pst_mac_sta->uc_vap_id, OAM_SF_ASSOC,"{mac_vap_check_ap_usr_opern_bandwidth::channel[%d] is not support bw[%d],set 20MHz}",
                pst_mac_sta->st_channel.uc_chan_number, en_bandwidth_ap);
            }
        }

        if(en_sta_current_bw != en_bandwidth_ap)
        {
            ul_change = MAC_BW_DIFF_AP_USER;
            OAM_WARNING_LOG3(pst_mac_sta->uc_vap_id, OAM_SF_FRAME_FILTER, "{mac_vap_check_ap_usr_opern_bandwidth:: SET MAC_BW_DIFF_AP_USER en_sta_current_bw = [%d], en_bandwidth_ap = [%d], ul_change = [%d]}",
            en_sta_current_bw , en_bandwidth_ap, ul_change);
        }
    }

    return ul_change;
}


oal_uint8  mac_vap_set_bw_check(mac_vap_stru *pst_mac_sta,
                                wlan_channel_bandwidth_enum_uint8 en_sta_new_bandwidth)
{
    wlan_channel_bandwidth_enum_uint8    en_band_with_sta_old;
    oal_uint8                            uc_change;

    en_band_with_sta_old = MAC_VAP_GET_CAP_BW(pst_mac_sta);
    MAC_VAP_GET_CAP_BW(pst_mac_sta) = en_sta_new_bandwidth;

    /* 判断是否需要通知硬件切换带宽 */
    uc_change = (en_band_with_sta_old == en_sta_new_bandwidth)? MAC_NO_CHANGE : MAC_BW_CHANGE;

    OAM_WARNING_LOG2(pst_mac_sta->uc_vap_id, OAM_SF_ASSOC, "mac_vap_set_bw_check::bw[%d]->[%d].",
                      en_band_with_sta_old, en_sta_new_bandwidth);

    return uc_change;
}

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE

oal_void mac_tx_qos_enhance_list_init(mac_vap_stru *pst_mac_vap)
{
    /* 入参检查 */
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_QOS, "{mac_tx_qos_enhance_list_init::null pst_mac_vap}");
        return ;
    }

    /* 初始化链表 */
    OAL_MEMZERO(&(pst_mac_vap->st_qos_enhance), OAL_SIZEOF(mac_qos_enhance_stru));
    oal_spin_lock_init(&(pst_mac_vap->st_qos_enhance.st_lock));
    oal_dlist_init_head(&(pst_mac_vap->st_qos_enhance.st_list_head));
    pst_mac_vap->st_qos_enhance.en_qos_enhance_enable = OAL_TRUE;

}


oal_void mac_tx_clean_qos_enhance_list(mac_vap_stru *pst_mac_vap)
{

    mac_qos_enhance_stru               *pst_qos_enhance;
    mac_qos_enhance_sta_stru           *pst_qos_enhance_sta = OAL_PTR_NULL;
    oal_dlist_head_stru                *pst_sta_list_entry;
    oal_dlist_head_stru                *pst_sta_list_entry_temp;

    /* 入参检查 */
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_QOS, "{mac_tx_qos_enhance_list_init::null pst_mac_vap}");
        return ;
    }

    pst_qos_enhance = &(pst_mac_vap->st_qos_enhance);

    if (0 == pst_qos_enhance->uc_qos_enhance_sta_count)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{mac_tx_qos_enhance_list_init::no sta in qos list while clean.}");
        return ;
    }

    /* 对链表操作前加锁 */
    oal_spin_lock(&(pst_qos_enhance->st_lock));

    /* 遍历链表 */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_sta_list_entry, pst_sta_list_entry_temp, &(pst_qos_enhance->st_list_head))
    {
        pst_qos_enhance_sta = OAL_DLIST_GET_ENTRY(pst_sta_list_entry, mac_qos_enhance_sta_stru, st_qos_enhance_entry);
        oal_dlist_delete_entry(&(pst_qos_enhance_sta->st_qos_enhance_entry));
        OAL_MEM_FREE(pst_qos_enhance_sta, OAL_TRUE);
    }

    /* 释放锁 */
    oal_spin_unlock(&(pst_qos_enhance->st_lock));

    pst_mac_vap->st_qos_enhance.uc_qos_enhance_sta_count = 0;

}


mac_qos_enhance_sta_stru* mac_tx_find_qos_enhance_list(mac_vap_stru *pst_mac_vap ,oal_uint8 *puc_sta_member_addr)
{
    mac_qos_enhance_sta_stru         *pst_qos_enhance_sta = OAL_PTR_NULL;
    oal_dlist_head_stru              *pst_sta_list_entry;
    oal_dlist_head_stru              *pst_sta_list_entry_temp;
    mac_qos_enhance_stru             *pst_qos_enhance;

    /* 入参检查 */
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_QOS, "{mac_tx_find_qos_enhance_list::null pst_mac_vap}");
        return OAL_PTR_NULL;
    }

    pst_qos_enhance = &(pst_mac_vap->st_qos_enhance);

    if (0 == pst_qos_enhance->uc_qos_enhance_sta_count)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_QOS, "{mac_tx_qos_enhance_list_init::no sta in qos list while find.}");
        return OAL_PTR_NULL;
    }

    /* 遍历链表 */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_sta_list_entry, pst_sta_list_entry_temp, &(pst_qos_enhance->st_list_head))
    {
        pst_qos_enhance_sta = OAL_DLIST_GET_ENTRY(pst_sta_list_entry, mac_qos_enhance_sta_stru, st_qos_enhance_entry);
        if (!oal_compare_mac_addr(puc_sta_member_addr, pst_qos_enhance_sta->auc_qos_enhance_mac))
        {
            return pst_qos_enhance_sta;
        }
    }

    return OAL_PTR_NULL;
}
#endif

/*lint -e19*/
oal_module_symbol(mac_vap_set_bw_check);
oal_module_symbol(mac_vap_init);
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
oal_module_symbol(mac_blacklist_free_pointer);
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
oal_module_symbol(mac_tx_find_qos_enhance_list);
#endif
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


