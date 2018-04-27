


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_sdio_comm.h"
#include "oal_sdio.h"
#include "oal_types.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "oal_util.h"
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
#include "hisi_customize_wifi.h"
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "hal_rf.h"
#endif
#endif /* _PRE_PLAT_FEATURE_CUSTOMIZE */
#include "oal_mem.h"
#ifdef _PRE_WLAN_ALG_ENABLE
#include "alg_ext_if.h"
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "hal_mac.h"
#endif
#include "mac_data.h"
#include "mac_device.h"
#include "mac_ie.h"
#include "mac_regdomain.h"
#include "mac_vap.h"
#include "mac_board.h"
#include "dmac_ext_if.h"
#include "dmac_main.h"
#include "dmac_vap.h"
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
#include "oal_gpio.h"
#include "oal_sdio.h"

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
#include "cali_data.h"
#include "pm_extern.h"
#include "dmac_hcc_adapt.h"
#endif
#ifdef _PRE_WLAN_CHIP_TEST
#include "dmac_test_main.h"
#include "dmac_lpm_test.h"
#include "dmac_frame_filter_test.h"
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

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
#include "mpw0_poweron.h"
#endif

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

#ifdef    _PRE_WLAN_FEATURE_GREEN_AP
#include "dmac_green_ap.h"
#endif
#ifdef _PRE_WLAN_FEATURE_SMPS
#include "dmac_smps.h"
#endif

#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
#include "dmac_auto_cali.h"
#endif

#include "dmac_vap.h"
#include "wlan_mib.h"
#ifdef _PRE_WLAN_ONLINE_DPD
#include "hal_dpd.h"
#endif

#ifdef _PRE_WLAN_11K_STAT
#include "dmac_stat.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#include "dmac_bsd.h"
#endif

#include "dmac_power.h"
#include "dmac_csa_sta.h"
#ifdef _PRE_WLAN_WEB_CMD_COMM
#include "dmac_pkt_capture.h"
#include "hal_commom_ops.h"
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
#include "dmac_tx_qos_enhance.h"
#endif

#include "dmac_m2s.h"
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
#include "dmac_alg_if.h"
#endif

#ifdef _PRE_WLAN_FEATURE_FTM
#include "dmac_ftm.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CONFIG_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
extern oal_uint16 g_usUsedMemForStop;
extern oal_uint16 g_usUsedMemForstart;
#endif
#if (defined(_PRE_PLAT_FEATURE_CUSTOMIZE) || defined(_PRE_WLAN_WEB_CMD_COMM))
/* 功率表相关参数 */
extern oal_uint8 g_auc_int_linkloss_threshold[WLAN_LINKLOSS_MODE_BUTT];
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "oal_hcc_slave_if.h"
#endif
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#ifdef _PRE_WLAN_CFGID_DEBUG
extern dmac_config_syn_stru g_ast_dmac_config_syn_debug[];
extern oal_uint32 dmac_get_config_debug_arrysize(oal_void);
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
#ifdef _PRE_WLAN_FEATURE_STA_PM
extern oal_uint8 g_uc_mac_pa_switch;
#endif
#if  (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
extern oal_uint8 g_uc_rf_switch_cfg;
#endif
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
OAL_STATIC wlan_channel_band_enum_uint8        g_en_pow_freq_band;
#endif

oal_uint32 g_ul_al_ampdu_num   = WLAN_AMPDU_TX_MAX_NUM; /*ampdu 常发聚合长度*/

/*****************************************************************************
  信道与协议模式映射表
*****************************************************************************/
OAL_CONST dmac_config_channel_bw_map_stru g_ast_channel_bw_map_2G[MAC_CHANNEL_FREQ_2_BUTT] =
{
    {1,  WLAN_BAND_WIDTH_40PLUS},
    {2,  WLAN_BAND_WIDTH_40PLUS},
    {3,  WLAN_BAND_WIDTH_40PLUS},
    {4,  WLAN_BAND_WIDTH_40PLUS},
    {5,  WLAN_BAND_WIDTH_40PLUS},
    {6,  WLAN_BAND_WIDTH_40PLUS},
    {7,  WLAN_BAND_WIDTH_40PLUS},
    {8,  WLAN_BAND_WIDTH_40MINUS},
    {9,  WLAN_BAND_WIDTH_40MINUS},
    {10, WLAN_BAND_WIDTH_40MINUS},
    {11, WLAN_BAND_WIDTH_40MINUS},
    {12, WLAN_BAND_WIDTH_40MINUS},
    {13, WLAN_BAND_WIDTH_40MINUS},
    {14, WLAN_BAND_WIDTH_40MINUS}
};

OAL_CONST dmac_config_channel_bw_map_stru g_ast_channel_bw_map_5G[MAC_CHANNEL_FREQ_5_BUTT] =
{
    {36,  WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS},
    {40,  WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS},
    {44,  WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS},
    {48,  WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS},

    {52,  WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS},
    {56,  WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS},
    {60,  WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS},
    {64,  WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS},

    {100, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS},
    {104, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS},
    {108, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS},
    {112, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS},

    {116, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS},
    {120, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS},
    {124, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS},
    {128, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS},

    {132, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS},
    {136, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS},
    {140, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS},
    {144, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS},

    {149, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS},
    {153, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS},
    {157, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS},
    {161, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS},

    {165, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS},

    {184, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS},
    {188, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS},
    {192, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS},
    {196, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS},
};

/*****************************************************************************
  3 静态函数声明
*****************************************************************************/
OAL_STATIC oal_uint32 dmac_config_reduce_sar(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
OAL_STATIC oal_uint32  dmac_config_get_version(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_WEB_CMD_COMM
OAL_STATIC oal_uint32   dmac_config_get_ko_version(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
OAL_STATIC oal_uint32   dmac_config_get_pwr_ref(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
OAL_STATIC oal_uint32   dmac_config_get_bcast_rate(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
oal_void dmac_config_set_wmm_register(mac_vap_stru *pst_mac_vap,  oal_bool_enum_uint8 en_wmm);
OAL_STATIC dmac_set_dscr_func  g_dmac_config_set_dscr_param[] =
{
    dmac_config_set_dscr_pgl,       /* 已废弃 */
    dmac_config_set_dscr_mtpgl,     /* 已废弃 */
    dmac_config_set_dscr_ta,        /* ta：tx antena，对应发送描述符15行的TXRTS Antenna */
    dmac_config_set_dscr_ra,        /* ra: rx antena, 对应发送描述符15行的RXCTRL Antenna */
    dmac_config_set_dscr_cc,        /* cc: channel code, 对应发送描述符13行的channel code */
    dmac_config_set_dscr_data0,     /* data0：对应发送描述符14行，32bit 10进制值 */
    dmac_config_set_dscr_data1,     /* data1：对应发送描述符19行， */
    dmac_config_set_dscr_data2,     /* data2：对应发送描述符20行， */
    dmac_config_set_dscr_data3,     /* data3：对应发送描述符21行， */
    dmac_config_set_dscr_power,     /* tx power: 对应发送描述符22行 */
    dmac_config_set_dscr_shortgi,         /* 配置short GI或long GI*/
    dmac_config_set_dscr_preamble_mode,   /* 配置preamble mode*/
    dmac_config_set_dscr_rtscts,         /* 配置rts/cts是否使能*/
    dmac_config_set_dscr_lsigtxop,       /* 配置lsig txop是否使能*/
    dmac_config_set_dscr_smooth,        /* 配置接收端是否对信道矩阵做平滑 */
    dmac_config_set_dscr_snding,        /* 配置Sounding模式 */
    dmac_config_set_dscr_txbf,          /* 配置txbf模式 */
    dmac_config_set_dscr_stbc,          /* 配置stbc模式 */
    dmac_config_get_dscr_ess,           /* 读取扩展空间流 */
    dmac_config_set_dscr_dyn_bw,        /* 配置DYN_BANDWIDTH_IN_NON_HT,rts/cts dynamic sygnaling 认证,STATIC:011/DYNAMIC:111 */
    dmac_config_set_dscr_dyn_bw_exist,  /* 配置DYN_BANDWIDTH_IN_NON_HT exist */
    dmac_config_set_dscr_ch_bw_exist,   /* 配置CH_BANDWIDTH_IN_NON_HT exist*/
    dmac_config_set_dscr_legacy_rate,  /*配置11a/b/g速率*/
    dmac_config_set_dscr_mcs,  /*配置11n速率*/
    dmac_config_set_dscr_mcsac, /*配置11ac速率*/
    dmac_config_set_dscr_nss, /*配置空间流*/
    dmac_config_set_dscr_bw, /*配置带宽*/
};

oal_uint32 g_ul_first_timestamp = 0;    /*记录性能统计第一次时间戳*/

/*****************************************************************************
3 外部函数声明
*****************************************************************************/

/*****************************************************************************

 4 函数实现
*****************************************************************************/

oal_uint32  dmac_config_set_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_mode_param_stru     *pst_prot_param;
    dmac_vap_stru               *pst_dmac_vap;
    mac_device_stru             *pst_mac_device;
    mac_channel_stru             st_channel;
    oal_uint                     ul_irq_flag;


    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);



    /* 获取dmac vap结构体 */
    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    #if 0 // gaolin：ACS运行之后，会设置不同的信道，DMAC不能旁路，否则导致系band和channel错位
    /* 已经配置过时，不需要再配置*/
    if (WLAN_BAND_WIDTH_BUTT != pst_mac_device->en_max_bandwidth)
    {
        return OAL_SUCC;
    }
    #endif

    pst_prot_param = (mac_cfg_mode_param_stru *)puc_param;

#ifdef _PRE_WLAN_ONLINE_DPD
    if (pst_prot_param->en_protocol == WLAN_LEGACY_11B_MODE)
    {
        hal_dpd_cfr_set_11b(pst_dmac_vap->pst_hal_device, OAL_TRUE);
    }
    else
    {
        hal_dpd_cfr_set_11b(pst_dmac_vap->pst_hal_device, OAL_FALSE);
    }
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 记录协议模式, band, bandwidth到mac_vap下 */
    pst_mac_vap->en_protocol                              = pst_prot_param->en_protocol;
    pst_mac_vap->st_channel.en_band                       = pst_prot_param->en_band;
    pst_mac_vap->st_channel.en_bandwidth                  = pst_prot_param->en_bandwidth;
    pst_mac_vap->st_ch_switch_info.en_user_pref_bandwidth = pst_prot_param->en_bandwidth;

#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((WLAN_VHT_MODE == pst_mac_vap->en_protocol)
        && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band))
    {
        mac_vap_set_11ac2g(pst_mac_vap, OAL_TRUE);
    }
    else
    {
        mac_vap_set_11ac2g(pst_mac_vap, OAL_FALSE);
    }
#endif



    /* 根据协议更新vap能力 */
    mac_vap_init_by_protocol(pst_mac_vap, pst_prot_param->en_protocol);

    /* 更新device的频段及最大带宽信息 */
    pst_mac_device->en_max_bandwidth = pst_prot_param->en_bandwidth;
    pst_mac_device->en_max_band      = pst_prot_param->en_band;

#endif
#ifdef _PRE_WLAN_FEATURE_TXBF
    if ((pst_mac_vap->en_protocol >= WLAN_HT_MODE)
        && (OAL_TRUE == MAC_DEVICE_GET_CAP_SUBFEE(pst_mac_device)))
    {
        pst_mac_vap->st_cap_flag.bit_11ntxbf = OAL_TRUE;
    }
    else
    {
        pst_mac_vap->st_cap_flag.bit_11ntxbf = OAL_FALSE;
    }
#endif

    hal_disable_machw_phy_and_pa(pst_dmac_vap->pst_hal_device);

    /* 关中断，挂起硬件发送需要关中断 */
    oal_irq_save(&ul_irq_flag, OAL_5115IRQ_DMSC);

    /* 调hal接口设置频段 */
    hal_set_freq_band(pst_dmac_vap->pst_hal_device, pst_prot_param->en_band);

    /* 调hal接口设置带宽 */
#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
    /*dummy*/
#else
    if (pst_prot_param->en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{dmac_config_set_mode:: fpga is not support 80M.}\r\n");
        pst_prot_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
#endif

    /* 多个VAP情况下 根据实际最大的带宽设置硬件带宽 */
    st_channel = pst_mac_vap->st_channel;

    dmac_chan_select_real_channel(pst_mac_device, &st_channel, st_channel.uc_chan_number);
    pst_mac_device->en_max_bandwidth = st_channel.en_bandwidth;
    hal_set_bandwidth_mode(pst_dmac_vap->pst_hal_device, st_channel.en_bandwidth);
    if (HAL_ALWAYS_RX_RESERVED == pst_dmac_vap->pst_hal_device->bit_al_rx_flag)
    {
        hal_set_primary_channel(pst_dmac_vap->pst_hal_device, st_channel.uc_chan_number, st_channel.en_band, st_channel.uc_chan_idx, st_channel.en_bandwidth);
    }

    /* 开中断 */
    oal_irq_restore(&ul_irq_flag, OAL_5115IRQ_DMSC);

    hal_clear_tx_hw_queue(pst_dmac_vap->pst_hal_device);

    /* 清fifo之后，也要删除tx队列中所有帧 */
    dmac_clear_tx_queue(pst_dmac_vap->pst_hal_device);

    hal_recover_machw_phy_and_pa(pst_dmac_vap->pst_hal_device);

    /* 通知算法 */
    dmac_alg_cfg_bandwidth_notify(pst_mac_vap, CH_BW_CHG_TYPE_MOVE_WORK);

#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE
    if(WLAN_BAND_5G == pst_mac_device->en_max_band)
    {
        hal_radar_config_reg_bw(pst_dmac_vap->pst_hal_device, pst_dmac_vap->pst_hal_device->st_dfs_radar_filter.en_radar_type,
            pst_prot_param->en_bandwidth);
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_mac_addr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_staion_id_param_stru  *pst_param;
    dmac_vap_stru                 *pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    hal_to_dmac_device_stru       *pst_hal_device;
#endif
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                     uc_hipriv_ack = OAL_FALSE;
#endif

    pst_param = (mac_cfg_staion_id_param_stru *)puc_param;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P 设置MAC 地址mib 值需要区分P2P DEV 或P2P_CL/P2P_GO,P2P_DEV MAC 地址设置到p2p0 MIB 中 */
    if (pst_param->en_p2p_mode == WLAN_P2P_DEV_MODE)
    {
        /* 如果是p2p0 device，则配置MAC 地址到auc_p2p0_dot11StationID 成员中 */
        oal_set_mac_addr(mac_mib_get_p2p0_dot11StationID(pst_mac_vap),
                        pst_param->auc_station_id);
    }
    else
#endif
    {
        /* 设置mib值, Station_ID */
        mac_mib_set_station_id(pst_mac_vap, OAL_SIZEOF(mac_cfg_staion_id_param_stru), puc_param);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_mac_addr::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_vap_is_vsta(pst_mac_vap))
    {
        /* 设置mib值, Station_ID */
        mac_mib_set_station_id(pst_mac_vap, OAL_SIZEOF(mac_cfg_staion_id_param_stru), puc_param);

        /* 将Proxy STA的mac 地址写入到peer addr 寄存器中,这样hal_vap_id与Proxy STA的mac_addr是相对应的 */
        hal_ce_add_peer_macaddr(pst_hal_device, pst_dmac_vap->pst_hal_vap->uc_vap_id, mac_mib_get_StationID(pst_mac_vap));
    }
    else
    {
        /* hal设置mac地址 */
        hal_vap_set_macaddr(pst_dmac_vap->pst_hal_vap, pst_param->auc_station_id);
        OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_PROXYSTA,"{dmac_config_set_mac_addr::set mac addr succ!}");
    }
#else
    /* hal设置mac地址 */
#ifdef _PRE_WLAN_FEATURE_P2P
    if (pst_param->en_p2p_mode == WLAN_P2P_DEV_MODE)
    {
        /* 设置p2p0 网络地址，需要设置到p2p0_hal_vap 中，其他类型MAC 地址设置到hal_vap 中 */
        hal_vap_set_macaddr(pst_dmac_vap->pst_p2p0_hal_vap, pst_param->auc_station_id);
    }
    else
#endif
    {
        hal_vap_set_macaddr(pst_dmac_vap->pst_hal_vap, pst_param->auc_station_id);
    }
    //OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,"{dmac_config_set_mac_addr::set mac addr succ!}");
#endif

    /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif

    return OAL_SUCC;
}


oal_uint32  dmac_config_set_freq(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int32               l_value;
    mac_device_stru        *pst_mac_device;
    oal_uint8               uc_channel_idx;
    oal_uint32              ul_ret;
#if  defined(_PRE_WLAN_HW_TEST) ||  defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST)
    mac_cfg_mode_param_stru     st_prot_param;
    oal_uint8                   uc_msg_len = 0;
#endif
    hal_to_dmac_device_stru        *pst_hal_device;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_cfg_channel_param_stru l_channel_param;
#endif

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_freq::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    l_value = *((oal_int32 *)puc_param);

#ifdef _PRE_WIFI_DMT
    if (l_value >= 36)
    {
        g_ul_dmt_scan_flag = 1;   /* DMT用例扫描根据信道进行扫描，而不进行全信道扫描   */
    }
#endif

    ul_ret = mac_get_channel_idx_from_num(pst_mac_vap->st_channel.en_band, (oal_uint8)l_value, &uc_channel_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{dmac_config_set_freq::mac_get_channel_idx_from_num failed[%d], band[%d], channel num[%d].}", ul_ret, pst_mac_vap->st_channel.en_band, (oal_uint8)l_value);
        return ul_ret;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    pst_mac_vap->st_channel.uc_chan_number = (oal_uint8)l_value;
    pst_mac_vap->st_channel.uc_chan_idx         = uc_channel_idx;
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

#if 0
    pst_mac_device->uc_max_channel = (oal_uint8)l_value;    /* hi1102-cb allways set at both side */
#else
    mac_device_get_channel(pst_mac_device, &l_channel_param);
    l_channel_param.uc_channel = (oal_uint8)l_value;
    mac_device_set_channel(pst_mac_device, &l_channel_param);
#endif

#endif

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    /* 根据信道设置40M和80M带宽模式 */
    if (WLAN_BAND_WIDTH_40M == pst_mac_vap->st_channel.en_bandwidth ||  WLAN_BAND_WIDTH_80M == pst_mac_vap->st_channel.en_bandwidth)
    {
        if (WLAN_BAND_WIDTH_40M == pst_mac_vap->st_channel.en_bandwidth)
        {
            if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
            {
                pst_mac_vap->st_channel.en_bandwidth = g_ast_channel_bw_map_2G[uc_channel_idx].en_bandwidth_40;
            }
            else if (WLAN_BAND_5G == pst_mac_vap->st_channel.en_band)
            {
                pst_mac_vap->st_channel.en_bandwidth = g_ast_channel_bw_map_5G[uc_channel_idx].en_bandwidth_40;
            }
        }
        else if(WLAN_BAND_WIDTH_80M == pst_mac_vap->st_channel.en_bandwidth)
        {
            if (WLAN_BAND_5G == pst_mac_vap->st_channel.en_band)
            {
                pst_mac_vap->st_channel.en_bandwidth = g_ast_channel_bw_map_5G[uc_channel_idx].en_bandwidth_80;
            }
        }

        /* 更新带宽信息调用dmac_config_set_mode处理 */
        st_prot_param.en_band       = pst_mac_vap->st_channel.en_band;
        st_prot_param.en_bandwidth  = pst_mac_vap->st_channel.en_bandwidth;
        st_prot_param.en_protocol   = pst_mac_vap->en_protocol;
        dmac_config_set_mode(pst_mac_vap, uc_msg_len, (oal_uint8 *)&st_prot_param);
    }
#endif

    /* 非IDLE状态直接配置信道 */
    /* 常收状态下直接配置信道 */
    if ((HAL_DEVICE_IDLE_STATE != GET_HAL_DEVICE_STATE(pst_hal_device)) || (HAL_ALWAYS_RX_RESERVED == pst_hal_device->bit_al_rx_flag))
    {
        dmac_vap_work_set_channel(MAC_GET_DMAC_VAP(pst_mac_vap));
    }

#ifdef _PRE_WLAN_DFT_EVENT
    oam_event_chan_report((oal_uint8)l_value);
#endif

#ifdef _PRE_WLAN_HW_TEST
    if (HAL_ALWAYS_RX_RESERVED == pst_hal_device->bit_al_rx_flag)
    {
        if (WLAN_BAND_2G == (pst_mac_vap->st_channel.en_band) && (WLAN_VHT_MODE == pst_mac_vap->en_protocol))
        {
            hal_set_phy_tx_scale(pst_hal_device, OAL_TRUE);

            st_prot_param.en_band       = pst_mac_vap->st_channel.en_band;
            st_prot_param.en_bandwidth  = pst_mac_vap->st_channel.en_bandwidth;
            st_prot_param.en_protocol   = pst_mac_vap->en_protocol;

            dmac_config_set_mode(pst_mac_vap, uc_msg_len, (oal_uint8 *)&st_prot_param);
        }
        else
        {
            hal_set_phy_tx_scale(pst_hal_device, OAL_FALSE);
        }
    }
#endif

#ifdef _PRE_WLAN_REALTIME_CALI
    /* 动态校准次数清零 */
    g_us_dync_cali_num = 0;

    /* 动态校准周期改为 WLAN_REALTIME_CALI_INTERVAL_INIT */
    if (OAL_TRUE == pst_hal_device->en_dync_txpower_flag)
    {
        FRW_TIMER_STOP_TIMER(&pst_mac_device->st_realtime_cali_timer);

        FRW_TIMER_CREATE_TIMER(&(pst_mac_device->st_realtime_cali_timer),
                                dmac_rf_realtime_cali_timeout,
                                WLAN_REALTIME_CALI_INTERVAL_INIT,
                                pst_mac_device,
                                OAL_TRUE,
                                OAM_MODULE_ID_DMAC,
                                pst_mac_device->ul_core_id);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 使能去使能雷达检测 */
    if ((WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)&&(OAL_TRUE == mac_vap_get_dfs_enable(pst_mac_vap)))
    {
        oal_bool_enum_uint8      en_enable_dfs;
        en_enable_dfs = mac_is_ch_in_radar_band(pst_mac_device->en_max_band, pst_mac_vap->st_channel.uc_chan_idx);
        hal_enable_radar_det(pst_hal_device, en_enable_dfs);
#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE
        if ( OAL_TRUE == pst_hal_device->st_dfs_radar_filter.st_dfs_normal_pulse_det.st_timer.en_is_enabled)
        {
            /* 取消定时器 */
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hal_device->st_dfs_radar_filter.st_dfs_normal_pulse_det.st_timer));
            pst_hal_device->st_dfs_radar_filter.st_dfs_normal_pulse_det.en_timer_start = OAL_FALSE;
        }
#endif
    }
#endif

    return OAL_SUCC;
}

oal_uint32  dmac_config_stop_sched_scan(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                         *pst_mac_device;
    mac_pno_sched_scan_mgmt_stru            *pst_pno_scan_mgmt;

    /* 获取mac device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    /* 判断pno是否已经停止，如果停止，则pno扫描的管理结构体指针为null */
    if (OAL_PTR_NULL == pst_mac_device->pst_pno_sched_scan_mgmt)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SCAN, "{dmac_config_stop_sched_scan::pno sched scan already stop.}");
        return OAL_SUCC;
    }

    /* 获取pno扫描管理结构体 */
    pst_pno_scan_mgmt = pst_mac_device->pst_pno_sched_scan_mgmt;

    /* 删除PNO调度扫描定时器 */
    dmac_scan_stop_pno_sched_scan_timer((void *)pst_pno_scan_mgmt);

    /* 停止本次PNO调度扫描并调用一次扫描结束 */
    if ((MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state) &&
        (WLAN_SCAN_MODE_BACKGROUND_PNO == pst_mac_device->st_scan_params.en_scan_mode))
    {
        dmac_scan_abort(pst_mac_device);
    }

    /* 释放PNO管理结构体内存 */
    OAL_MEM_FREE(pst_mac_device->pst_pno_sched_scan_mgmt, OAL_TRUE);
    pst_mac_device->pst_pno_sched_scan_mgmt = OAL_PTR_NULL;

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_config_stop_sched_scan::stop schedule scan success.}");

    return OAL_SUCC;
}


oal_uint32  dmac_config_scan_abort(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru     *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    /* 如果扫描正在进行则停止扫描 */
    if (MAC_SCAN_STATE_RUNNING == pst_mac_device->en_curr_scan_state)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_config_scan_abort::stop scan.}");
        dmac_scan_abort(pst_mac_device);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_vap_state_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru              *pst_dmac_vap;
    mac_device_stru            *pst_mac_device;
    hal_to_dmac_device_stru    *pst_hal_device;
    mac_vap_state_enum_uint8   en_new_vap_state;
    mac_vap_state_enum_uint8   en_old_vap_state;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
    en_new_vap_state = (mac_vap_state_enum_uint8)(*puc_param);
    en_old_vap_state = pst_mac_vap->en_vap_state;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
       OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_state_syn::pst_hal_device is NULL!}");
       return OAL_FAIL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_vap_state_change(pst_mac_vap, en_new_vap_state);
#endif

    /* staut模式hal状态机只关心up,fakeup两个状态 */
    if(WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        switch (en_new_vap_state)
        {
            case MAC_VAP_STATE_STA_FAKE_UP:
                if (en_old_vap_state != en_new_vap_state)
                {
                    dmac_vap_down_notify(pst_mac_vap);/* staut vap down时需要通知算法并做一些fake队列的清理工作 */

                #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
                    pst_dmac_vap->st_vap_base_info.st_cap_flag.bit_keepalive   =  OAL_FALSE;  /* 关闭keepalive */
                #endif
                }
                hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_VAP_DOWN, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
            break;

            case MAC_VAP_STATE_STA_JOIN_COMP:
                //mac vap迁移到MAC_VAP_STATE_STA_JOIN_COMP状态时，如果对应的mac_device正在扫描，那么需要先将扫描停止，
                //因为hal处于扫描状态时无法处理HAL_DEVICE_EVENT_JOIN_COMP事件
                if (mac_is_dbac_enabled(pst_mac_device))
                {
                    dmac_scan_abort(pst_mac_device);
                }
                hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_JOIN_COMP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
                dmac_vap_work_set_channel(pst_dmac_vap); /*sta关联前，hal dev状态机为work状态下配置工作信道*/

            #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
                oal_set_mac_addr(pst_mac_device->st_scan_params.auc_sour_mac_addr,
                                mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info));
                if (OAL_SUCC != dmac_scan_send_probe_req_frame(pst_dmac_vap, pst_mac_vap->auc_bssid, (oal_int8 *)mac_mib_get_DesiredSSID(pst_mac_vap)))
                {
                    OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                                    "{dmac_config_vap_state_syn::dmac_scan_send_probe_req_frame failed.}");
                }
            #endif
            break;

            case MAC_VAP_STATE_UP:
                dmac_sta_csa_fsm_post_event(pst_mac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OAL_PTR_NULL);

            #ifdef _PRE_WLAN_FEATURE_STA_PM
                dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_KEEPALIVE, 0, OAL_PTR_NULL);  /* STA 模式下只有UP状态才开启keepalive定时器 */
            #endif
            #ifdef _PRE_WLAN_FEATURE_BTCOEX
                if(HAL_BTCOEX_SW_POWSAVE_WORK == GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device))
                {
                    /* 如果设备在关联过程中已经处于ps状态, 子状态置为connect状态 */
                    GET_HAL_BTCOEX_SW_PREEMPT_SUBTYPE(pst_hal_device) = HAL_BTCOEX_SW_POWSAVE_SUB_CONNECT;
                }
            #endif
                hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_VAP_UP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
            #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
                hal_init_pm_info(pst_dmac_vap->pst_hal_vap);
            #ifdef  _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
                dmac_psm_update_bcn_rf_chain(pst_dmac_vap, oal_get_real_rssi(pst_dmac_vap->st_query_stats.s_signal));
                OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_vap_state_syn:bcn_rx_chain[%d] rssi[%d]",
                             pst_dmac_vap->pst_hal_vap->st_pm_info.uc_bcn_rf_chain, oal_get_real_rssi(pst_dmac_vap->st_query_stats.s_signal));
            #endif
                /* 设置接收beacon超时中断的时间 */
                hal_set_beacon_timeout_val(pst_hal_device, pst_dmac_vap->us_beacon_timeout);
            #endif
            break;

#ifdef _PRE_WLAN_FEATURE_ROAM
            case MAC_VAP_STATE_ROAMING:
                if(OAL_TRUE == dmac_sta_csa_is_in_waiting(pst_mac_vap))
                {
                    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_vap_state_syn:vap_state into MAC_VAP_STATE_ROAMING, csa status back to INIT ");
                    dmac_sta_csa_fsm_post_event(pst_mac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OAL_PTR_NULL);
                }
            break;
#endif
            default:
                OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_state_syn::old state[%d]->new state[%d]}", en_old_vap_state, en_new_vap_state);
            break;
        }
    }
    /* aput模式hal状态机只关心up和非up状态 */
    else if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        if (MAC_VAP_STATE_UP == en_new_vap_state)
        {
            //mac vap迁移到MAC_VAP_STATE_UP状态时，如果对应的mac_device正在扫描，那么需要先将扫描停止，
            //因为hal处于扫描状态时无法处理HAL_DEVICE_EVENT_VAP_UP事件
            if (mac_is_dbac_enabled(pst_mac_device))
            {
                dmac_scan_abort(pst_mac_device);
            }

        // TODO: 考虑继续往前放,放到启动aput之前
        #ifdef _PRE_WLAN_FEATURE_DBDC
            dmac_vap_dbdc_start(pst_mac_device, pst_mac_vap);
        #endif
            hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_VAP_UP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
            dmac_vap_work_set_channel(pst_dmac_vap);/* aput up后，hal dev状态机为work状态下配置工作信道 */
        }
        else
        {
            hal_device_handle_event(pst_hal_device, HAL_DEVICE_EVENT_VAP_DOWN, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
        }
    }

    return OAL_SUCC;
}


oal_uint32 dmac_config_user_cap_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_user_stru              *pst_mac_user;
    mac_h2d_usr_cap_stru       *pst_mac_h2d_usr_cap;
    pst_mac_h2d_usr_cap = (mac_h2d_usr_cap_stru *)puc_param;
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_mac_h2d_usr_cap->us_user_idx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_config_user_cap_syn::pst_mac_user null.user idx [%d]}", pst_mac_h2d_usr_cap->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }


#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    dmac_11w_update_users_status(MAC_GET_DMAC_VAP(pst_mac_vap), pst_mac_user, pst_mac_h2d_usr_cap->st_user_cap_info.bit_pmf_active);
#endif /* #if(_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT) */

    oal_memcopy(&pst_mac_user->st_cap_info, &pst_mac_h2d_usr_cap->st_user_cap_info, OAL_SIZEOF(mac_user_cap_info_stru));

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    dmac_config_btcoex_assoc_state_syn(pst_mac_vap, pst_mac_user);
#endif
#ifdef _PRE_WLAN_FEATURE_SMARTANT
    dmac_config_dual_antenna_vap_check(pst_mac_vap);
#endif

#endif /* (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) */
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET

oal_uint32 dmac_config_pk_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_cfg_pk_mode_stru *pst_pk_mode_info;

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_param))
    {
        OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_config_pk_mode::puc_param null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_pk_mode_info = (mac_cfg_pk_mode_stru *)puc_param;


    OAM_WARNING_LOG_ALTER(0, OAM_SF_CFG,
    "{dmac_config_pk_mode::update Pk_Mode(%d), rx_bytes(%u), tx_bytes(%u), dur_time(%u), bw(%d), protocol(%d), nss(%d)}",
    7,
    pst_pk_mode_info->en_is_pk_mode,
    pst_pk_mode_info->ul_rx_bytes,
    pst_pk_mode_info->ul_tx_bytes,
    pst_pk_mode_info->ul_dur_time,
    pst_pk_mode_info->en_curr_bw_cap,
    pst_pk_mode_info->en_curr_protocol_cap,
    pst_pk_mode_info->en_curr_num_spatial_stream);

    g_st_wifi_pk_mode_status.ul_rx_bytes = pst_pk_mode_info->ul_rx_bytes;
    g_st_wifi_pk_mode_status.ul_tx_bytes = pst_pk_mode_info->ul_tx_bytes;
    g_st_wifi_pk_mode_status.ul_dur_time = pst_pk_mode_info->ul_dur_time;
    g_st_wifi_pk_mode_status.en_is_pk_mode    = pst_pk_mode_info->en_is_pk_mode;
    g_st_wifi_pk_mode_status.en_curr_bw_cap   = pst_pk_mode_info->en_curr_bw_cap;
    g_st_wifi_pk_mode_status.en_curr_protocol_cap       = pst_pk_mode_info->en_curr_protocol_cap;
    g_st_wifi_pk_mode_status.en_curr_num_spatial_stream = pst_pk_mode_info->en_curr_num_spatial_stream;

#ifdef _PRE_WLAN_FEATURE_INTF_DET
    /* 下行PK mode */
    if (g_st_wifi_pk_mode_status.ul_rx_bytes > (oal_uint32)(g_st_wifi_pk_mode_status.ul_tx_bytes << 1))
    {
        g_st_wifi_pk_mode_status.en_is_pk_mode = OAL_TRUE;
    }
    else  /* 其他情况不进入PK mode */
    {
        g_st_wifi_pk_mode_status.en_is_pk_mode = OAL_FALSE;
    }

    dmac_alg_cfg_intf_det_pk_mode_notify(MAC_GET_DMAC_VAP(pst_mac_vap), g_st_wifi_pk_mode_status.en_is_pk_mode);
#endif
#endif /* #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) */

    return OAL_SUCC;
}
#endif

OAL_STATIC oal_uint32  dmac_config_user_rate_info_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

    mac_h2d_usr_rate_info_stru        *pst_usr_info;
    dmac_user_stru                    *pst_dmac_user;



    pst_usr_info = (mac_h2d_usr_rate_info_stru *)puc_param;


    /* 获取DMAC模块用户结构体 */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_usr_info->us_user_idx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_config_user_rate_info_syn::pst_dmac_user null.user idx [%d]}", pst_usr_info->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_set_protocol_mode(&(pst_dmac_user->st_user_base_info), pst_usr_info->en_protocol_mode);

    /* 同步legacy速率集信息 */
    mac_user_set_avail_op_rates(&pst_dmac_user->st_user_base_info, pst_usr_info->uc_avail_rs_nrates, pst_usr_info->auc_avail_rs_rates);

    /* 同步ht速率集信息 */
    mac_user_set_ht_hdl(&pst_dmac_user->st_user_base_info, &pst_usr_info->st_ht_hdl);

    /* 同步vht速率集信息 */
    mac_user_set_vht_hdl(&pst_dmac_user->st_user_base_info, &pst_usr_info->st_vht_hdl);

#ifdef _PRE_WLAN_FEATURE_11AX
    /*同步he 信息*/
    mac_user_set_he_hdl(&pst_dmac_user->st_user_base_info, &pst_usr_info->st_he_hdl);
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* 信息同步后续处理 */
    mac_vap_update_txopps(pst_mac_vap, &pst_dmac_user->st_user_base_info);
#endif
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_sta_usr_info_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_h2d_usr_info_stru           *pst_usr_info;
    mac_user_stru                   *pst_mac_user;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_user_ht_hdl_stru     st_ht_hdl;
#endif
    pst_usr_info = (mac_h2d_usr_info_stru *)puc_param;

    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_usr_info->us_user_idx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_config_sta_usr_info_syn::pst_mac_user null.user idx [%d]}", pst_usr_info->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 更新mac地址，漫游时mac会更新 */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        oal_set_mac_addr(pst_mac_user->auc_user_mac_addr, pst_mac_vap->auc_bssid);
    }

    /* 同步USR带宽 */
    mac_user_set_bandwidth_cap(pst_mac_user, pst_usr_info->en_bandwidth_cap);
    mac_user_set_bandwidth_info(pst_mac_user, pst_usr_info->en_avail_bandwidth, pst_usr_info->en_cur_bandwidth);

    mac_user_get_ht_hdl(pst_mac_user, &st_ht_hdl);
    st_ht_hdl.uc_max_rx_ampdu_factor    = pst_usr_info->uc_arg1;
    st_ht_hdl.uc_min_mpdu_start_spacing = pst_usr_info->uc_arg2;
    mac_user_set_ht_hdl(pst_mac_user, &st_ht_hdl);

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

    /* 同步user pmf的能力 */
    dmac_11w_update_users_status(MAC_GET_DMAC_VAP(pst_mac_vap), pst_mac_user, pst_usr_info->en_user_pmf);
#endif /* #if(_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT) */

    /* 同步协议模式 */
    mac_user_set_avail_protocol_mode(pst_mac_user, pst_usr_info->en_avail_protocol_mode);

    mac_user_set_cur_protocol_mode(pst_mac_user, pst_usr_info->en_avail_protocol_mode);
    mac_user_set_protocol_mode(pst_mac_user, pst_usr_info->en_protocol_mode);
    mac_user_set_asoc_state(pst_mac_user, pst_usr_info->en_user_asoc_state);

    /* 初始化slottime */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        dmac_user_init_slottime(pst_mac_vap, pst_mac_user);
    }

#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
    dmac_user_update_sw_ctrl_rsp(pst_mac_vap, pst_mac_user);
#endif /* _PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV */
#endif /* _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE */

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_config_vap_info_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_h2d_vap_info_stru   *pst_vap_info;
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
    dmac_vap_stru           *pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
#endif

    pst_vap_info = (mac_h2d_vap_info_stru *)puc_param;

    /* 同步vap信息 */
    pst_mac_vap->us_sta_aid = pst_vap_info->us_sta_aid;

    mac_vap_set_uapsd_cap(pst_mac_vap, pst_vap_info->uc_uapsd_cap);


#ifdef _PRE_WLAN_FEATURE_TXOPPS
    mac_vap_set_txopps(pst_mac_vap, pst_vap_info->en_txop_ps);
#endif /* #ifdef _PRE_WLAN_FEATURE_TXOPPS */

    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {

#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
        /* 判断aid有效性 */
        if (0 == pst_vap_info->us_sta_aid || pst_vap_info->us_sta_aid >= 2008)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_config_vap_info_syn::aid invalid[%d]}", pst_vap_info->us_sta_aid);
            return OAL_FAIL;
        }

        /* STA关联时设置aid寄存器 */
        hal_set_mac_aid(pst_dmac_vap->pst_hal_vap, pst_mac_vap->us_sta_aid);
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
        /* 依据vap下TXOP PS开关，重设寄存器开关 */
        dmac_txopps_set_machw(pst_mac_vap);
#endif /* #ifdef _PRE_WLAN_FEATURE_TXOPPS */
    }

#endif
    return OAL_SUCC;
}


oal_uint32  dmac_config_d2h_user_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    oal_uint32                  ul_ret = OAL_SUCC;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_d2h_syn_info_stru       st_mac_d2h_info;

    /* 带宽信息同步到hmac */
    st_mac_d2h_info.en_avail_bandwidth = pst_mac_user->en_avail_bandwidth;
    st_mac_d2h_info.en_cur_bandwidth   = pst_mac_user->en_cur_bandwidth;
    st_mac_d2h_info.en_bandwidth_cap   = pst_mac_user->en_bandwidth_cap;
    st_mac_d2h_info.us_user_idx        = pst_mac_user->us_assoc_id;

    /* 信道信息同步到hmac */
    oal_memcopy(&(st_mac_d2h_info.st_channel), &(pst_mac_vap->st_channel), OAL_SIZEOF(mac_channel_stru));

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                       "{dmac_config_d2h_user_info_syn::en_avail_bandwidth:%d,en_cur_bandwidth:%d.}",
                         pst_mac_user->en_avail_bandwidth,
                         pst_mac_user->en_cur_bandwidth);
    /***************************************************************************
        抛事件到HMAC层, 同步USER最新状态到HMAC
    ***************************************************************************/
    ul_ret = dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_USR_INFO_SYN, OAL_SIZEOF(st_mac_d2h_info), (oal_uint8 *)(&st_mac_d2h_info));
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_d2h_user_info_syn::dmac_send_sys_event failed[%d],user_id[%d].}",
                    ul_ret, pst_mac_user->us_assoc_id);
    }
#endif

    return ul_ret;
}


oal_uint32  dmac_config_d2h_vap_mib_syn(mac_vap_stru *pst_mac_vap)
{
    oal_uint32                         ul_ret = OAL_SUCC;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    hal_to_dmac_device_stru           *pst_hal_device;
    /* If need sync device mib to host, pls add it to the following structure.
     * at the same time, get other mib parameters from API mac_mib_get_*** in case host get wrong mib parameters */
    mac_d2h_mib_update_info_stru       st_mac_d2h_mib_update_info;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_d2h_vap_mib_syn::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(&st_mac_d2h_mib_update_info, sizeof(st_mac_d2h_mib_update_info));

#ifdef _PRE_WLAN_FEATURE_11AX
    st_mac_d2h_mib_update_info.en_11ax_cap             = mac_mib_get_HEOptionImplemented(pst_mac_vap);
#endif
    st_mac_d2h_mib_update_info.en_radar_detector_cap   = pst_hal_device->st_cfg_cap_info.en_radar_detector_is_supp;
    st_mac_d2h_mib_update_info.en_wlan_bw_max          = mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap);
    st_mac_d2h_mib_update_info.en_11n_sounding         = pst_hal_device->st_cfg_cap_info.en_11n_sounding;
    st_mac_d2h_mib_update_info.en_green_field          = pst_hal_device->st_cfg_cap_info.en_green_field;
    st_mac_d2h_mib_update_info.en_mu_beamformee_cap    = mac_mib_get_VHTMUBeamformeeOptionImplemented(pst_mac_vap);
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    st_mac_d2h_mib_update_info.en_txopps_is_supp       = pst_hal_device->st_cfg_cap_info.en_txopps_is_supp;
#endif
    /* add other mib parameters from here */
    st_mac_d2h_mib_update_info.us_beacon_period        = (oal_uint16)mac_mib_get_BeaconPeriod(pst_mac_vap);
    st_mac_d2h_mib_update_info.uc_su_bfee_num          = mac_mib_get_VHTBeamformeeNTxSupport(pst_mac_vap);
    st_mac_d2h_mib_update_info.en_40m_shortgi          = mac_mib_get_ShortGIOptionInFortyImplemented(pst_mac_vap);

    /***************************************************************************
        抛事件到HMAC层, 同步mib能力到HMAC
    ***************************************************************************/
    ul_ret = dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_VAP_MIB_UPDATE, OAL_SIZEOF(st_mac_d2h_mib_update_info), (oal_uint8 *)(&st_mac_d2h_mib_update_info));
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_d2h_vap_mib_syn::dmac_send_sys_event failed[%d].}",ul_ret);
    }
#endif

    return ul_ret;
}


oal_uint32  dmac_config_vap_mib_update(mac_vap_stru *pst_mac_vap)
{
    oal_uint32                         ul_ret = OAL_SUCC;
    hal_to_dmac_device_stru           *pst_hal_device;
    oal_bool_enum_uint8                en_he_cap;
    mac_device_stru                   *pst_mac_dev;
    wlan_bw_cap_enum_uint8             en_vap_bw_cap;
    oal_bool_enum_uint8                en_mu_beamformee_cap = OAL_FALSE;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_mib_update::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_mib_update::pst_mac_dev[%d] null.}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if((OAL_TRUE == pst_hal_device->st_cfg_cap_info.en_is_supp_11ax) && (IS_LEGACY_STA(pst_mac_vap)))
    {
        en_he_cap = OAL_TRUE;
    }
    else
    {
        en_he_cap = OAL_FALSE;
    }

    /*set mib BandWidth*/
    if(MAC_DEVICE_GET_CAP_BW(pst_mac_dev) > pst_hal_device->st_cfg_cap_info.en_wlan_bw_max)
    {
        en_vap_bw_cap = pst_hal_device->st_cfg_cap_info.en_wlan_bw_max;
    }
    else
    {
        en_vap_bw_cap = MAC_DEVICE_GET_CAP_BW(pst_mac_dev);
    }
    mac_mib_set_dot11VapMaxBandWidth(pst_mac_vap,en_vap_bw_cap);
    /*更新VHT相关mib能力 */
    mac_mib_set_VHTChannelWidthOptionImplemented(pst_mac_vap, mac_device_trans_bandwith_to_vht_capinfo(mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap)));
    if (mac_mib_get_dot11VapMaxBandWidth(pst_mac_vap) >= WLAN_BW_CAP_160M)
    {
        mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, WLAN_HAL0_VHT_SGI_SUPP_160_80P80);
    }
    else
    {
        mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap, OAL_FALSE);
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                       "{dmac_config_vap_mib_update::vap_mode=[%d],en_11ax_cap=[%d],en_11n_sounding=[%d],en_radar_cap=[%d].}",
                       pst_mac_vap->en_vap_mode, en_he_cap, pst_hal_device->st_cfg_cap_info.en_11n_sounding,
                       pst_hal_device->st_cfg_cap_info.en_radar_detector_is_supp);
    /*set 11ax mib*/
#ifdef _PRE_WLAN_FEATURE_11AX
    mac_mib_set_HEOptionImplemented(pst_mac_vap, en_he_cap);
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    MAC_VAP_GET_SUPPORT_DFS(pst_mac_vap) = pst_hal_device->st_cfg_cap_info.en_radar_detector_is_supp;
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 定制化文件仅控制staut是否支持 txopps */
    if (IS_LEGACY_STA(pst_mac_vap))
#endif /* #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) */
    {
        mac_mib_set_txopps(pst_mac_vap,pst_hal_device->st_cfg_cap_info.en_txopps_is_supp);
    }
#endif

    /*11n sounding mib设置*/
#if (defined(_PRE_WLAN_FEATURE_TXBF) && defined(_PRE_WLAN_FEATURE_TXBF_HT))
    if(OAL_TRUE == pst_hal_device->st_cfg_cap_info.en_11n_sounding)
    {
        mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFER(pst_mac_dev));
        mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, MAC_DEVICE_GET_CAP_SUBFEE(pst_mac_dev));
        mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_DELAYED);
        mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, HT_BFEE_NTX_SUPP_ANTA_NUM);

    }
    else
    {
        mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap, WLAN_MIB_HT_ECBF_INCAPABLE);
        mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap, 1);

    }
#endif

    /*green filed mib设置*/
    /*暂时未使用，使用时打开*/
    //mac_mib_set_HTGreenfieldOptionImplemented(pst_mac_vap,pst_hal_device->st_cfg_cap_info.en_green_field);

#ifdef _PRE_WLAN_FEATURE_TXBF
    /*仅支持发送能力*/
    if((OAL_TRUE == MAC_DEVICE_GET_CAP_MUBFEE(pst_mac_dev)) && (OAL_TRUE == pst_hal_device->st_cfg_cap_info.en_mu_bfmee_is_supp))
    {
        en_mu_beamformee_cap = OAL_TRUE;
    }
    mac_mib_set_VHTMUBeamformeeOptionImplemented(pst_mac_vap, en_mu_beamformee_cap);
#endif

    mac_mib_set_VHTBeamformeeNTxSupport(pst_mac_vap, pst_hal_device->st_cfg_cap_info.uc_su_bfee_num);

    /* device sync mib info to host */
    dmac_config_d2h_vap_mib_syn(pst_mac_vap);

    return ul_ret;
}


oal_uint32  dmac_config_vap_cap_update(mac_vap_stru *pst_mac_vap)
{
    oal_uint32                         ul_ret = OAL_SUCC;
    hal_to_dmac_device_stru           *pst_hal_device;
    mac_cap_flag_stru                  st_mac_cap_flag;


    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_cap_update::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    st_mac_cap_flag.bit_1024qam = pst_hal_device->st_cfg_cap_info.en_1024qam_is_supp;
    st_mac_cap_flag.bit_nb = pst_hal_device->st_cfg_cap_info.en_nb_is_supp;
    pst_mac_vap->st_cap_flag.bit_1024qam = !!st_mac_cap_flag.bit_1024qam;
    pst_mac_vap->st_cap_flag.bit_nb = st_mac_cap_flag.bit_nb;

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{dmac_config_vap_mib_update::vap_mode=[%d],en_11024_cap=[%d],en_nb_cap=[%d].}",
                     pst_mac_vap->en_vap_mode, st_mac_cap_flag.bit_1024qam, pst_mac_vap->st_cap_flag.bit_nb);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /***************************************************************************
        抛事件到HMAC层, 同步VAP能力到HMAC
    ***************************************************************************/
    ul_ret = dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_VAP_CAP_UPDATE, OAL_SIZEOF(mac_cap_flag_stru), (oal_uint8 *)(&st_mac_cap_flag));
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_cap_update::dmac_send_sys_event failed[%d].}",ul_ret);
    }
#endif

    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_IP_FILTER

oal_void dmac_clear_ip_filter_btable(mac_vap_stru *pst_mac_vap)
{
    /* 清空黑名单 */
    g_pst_mac_board->st_rx_ip_filter.uc_btable_items_num = 0;
    if (OAL_PTR_NULL == g_pst_mac_board->st_rx_ip_filter.pst_filter_btable)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_clear_ip_filter_btable::The pst_filter_btable is NULL!!}");
        return;
    }
    OAL_MEMZERO((oal_uint8 *)(g_pst_mac_board->st_rx_ip_filter.pst_filter_btable), MAC_MAX_IP_FILTER_BTABLE_SIZE);
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_clear_ip_filter_btable::Btable clear done.}");
}


oal_uint32 dmac_update_ip_filter_btable(mac_ip_filter_cmd_stru *pst_cmd_info)
{
    oal_uint8    uc_items_idx;
    oal_uint8    uc_add_items_num;

    if (OAL_PTR_NULL == g_pst_mac_board->st_rx_ip_filter.pst_filter_btable)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_update_ip_filter_btable::The pst_filter_btable is NULL!!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    g_pst_mac_board->st_rx_ip_filter.uc_btable_items_num = 0;
    OAL_MEMZERO((oal_uint8 *)(g_pst_mac_board->st_rx_ip_filter.pst_filter_btable), MAC_MAX_IP_FILTER_BTABLE_SIZE);
    uc_add_items_num = OAL_MIN(g_pst_mac_board->st_rx_ip_filter.uc_btable_size, pst_cmd_info->uc_item_count);
    for(uc_items_idx = 0; uc_items_idx < uc_add_items_num; uc_items_idx++)
    {
        g_pst_mac_board->st_rx_ip_filter.pst_filter_btable[uc_items_idx].uc_protocol = pst_cmd_info->ast_filter_items_items[uc_items_idx].uc_protocol;
        g_pst_mac_board->st_rx_ip_filter.pst_filter_btable[uc_items_idx].us_port = pst_cmd_info->ast_filter_items_items[uc_items_idx].us_port;
    }
    g_pst_mac_board->st_rx_ip_filter.uc_btable_items_num = uc_add_items_num;
    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_update_ip_filter_btable::Btable update done,renew %d items.}", uc_add_items_num);

    return OAL_SUCC;
}


oal_uint32 dmac_config_update_ip_filter(frw_event_mem_stru *pst_event_mem)
{
    oal_uint32                  ul_ret;
    frw_event_stru             *pst_event;
    dmac_tx_event_stru         *pst_dtx_event;
    mac_ip_filter_cmd_stru     *pst_cmd_info;
    mac_vap_stru               *pst_mac_vap;


    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CALIBRATE, "{dmac_config_update_ip_filter::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event        = frw_get_event_stru(pst_event_mem);
    pst_dtx_event    = (dmac_tx_event_stru *)pst_event->auc_event_data;
    if(OAL_PTR_NULL == pst_dtx_event->pst_netbuf)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_config_update_ip_filter::The cmd_info is NULL!.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cmd_info = (mac_ip_filter_cmd_stru *)OAL_NETBUF_DATA(pst_dtx_event->pst_netbuf);
    pst_mac_vap  = mac_res_get_mac_vap(pst_event->st_event_hdr.uc_vap_id);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_config_update_ip_filter::Can not find mac_vap %d, ignore the cmd!.}",
                        pst_event->st_event_hdr.uc_vap_id);
        oal_netbuf_free(pst_dtx_event->pst_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 功能开关未打开，不处理参数配置动作 */
    if (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_ip_filter)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_config_update_ip_filter::Func not enable, ignore the cmd!.}");
        oal_netbuf_free(pst_dtx_event->pst_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* step2, 解析命令，更新控制参数 */
    if (MAC_IP_FILTER_ENABLE == pst_cmd_info->en_cmd)
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_config_update_ip_filter::Change state from [%d] to [%d].}",
                        g_pst_mac_board->st_rx_ip_filter.en_state,
                        ((OAL_TRUE == pst_cmd_info->en_enable)? MAC_RX_IP_FILTER_WORKING : MAC_RX_IP_FILTER_STOPED));
        g_pst_mac_board->st_rx_ip_filter.en_state = (OAL_TRUE == pst_cmd_info->en_enable)? MAC_RX_IP_FILTER_WORKING : MAC_RX_IP_FILTER_STOPED;
    }
    else if(MAC_IP_FILTER_UPDATE_BTABLE == pst_cmd_info->en_cmd)
    {
        /* 更新黑名单 */
        ul_ret = dmac_update_ip_filter_btable(pst_cmd_info);
        if (OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_config_update_ip_filter::update_ip_filter_btable FAIL!!}");
            oal_netbuf_free(pst_dtx_event->pst_netbuf);
            return ul_ret;
        }
    }
    else if (MAC_IP_FILTER_CLEAR == pst_cmd_info->en_cmd)
    {
        /* 清空黑名单 */
        dmac_clear_ip_filter_btable(pst_mac_vap);

    }
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_config_update_ip_filter::Command does not support!cmd_len %d.}", pst_cmd_info->en_cmd);
        oal_netbuf_free(pst_dtx_event->pst_netbuf);
        return OAL_FAIL;
    }

    oal_netbuf_free(pst_dtx_event->pst_netbuf);

    return OAL_SUCC;
}

#endif //_PRE_WLAN_FEATURE_IP_FILTER

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ

oal_uint32 dmac_config_set_device_freq(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    dmac_freq_control_stru *device_freq_handle;
    config_device_freq_h2d_stru   *pst_device_freq;
    oal_uint8               uc_index;
    device_pps_freq_level_stru*    device_ba_pps_freq_level;

    pst_device_freq = (config_device_freq_h2d_stru *)puc_param;
    device_freq_handle = dmac_get_auto_freq_handle();
    device_ba_pps_freq_level = dmac_get_ba_pps_freq_level();

    /* 如果是同步数据 */
    if (FREQ_SYNC_DATA == pst_device_freq->uc_set_type)
    {
        for(uc_index = 0; uc_index < 4; uc_index++)
        {
            device_ba_pps_freq_level->ul_speed_level = pst_device_freq->st_device_data[uc_index].ul_speed_level;
            device_ba_pps_freq_level->ul_cpu_freq_level = pst_device_freq->st_device_data[uc_index].ul_cpu_freq_level;
            device_ba_pps_freq_level++;
        }
    }
    else if(FREQ_SET_MODE == pst_device_freq->uc_set_type)
    {
        /* 上层下发的使能 */
       if (OAL_FALSE == pst_device_freq->uc_device_freq_enable)
       {
           dmac_set_auto_freq_deinit();

           /* 不使能device调频则最高频率运行 */
           device_freq_handle->uc_req_freq_level = FREQ_HIGHEST;
           dmac_auto_set_device_freq();
       }
       else
       {
            dmac_set_auto_freq_init();

            /* 每次使能需要重新刷 */
            for(uc_index = 0; uc_index < 4; uc_index++)
            {
                device_ba_pps_freq_level->ul_speed_level = pst_device_freq->st_device_data[uc_index].ul_speed_level;
                device_ba_pps_freq_level->ul_cpu_freq_level = pst_device_freq->st_device_data[uc_index].ul_cpu_freq_level;
                device_ba_pps_freq_level++;
            }
       }

       OAM_WARNING_LOG1(0, OAM_SF_RSSI,"{dmac_config_set_device_freq::enable mode[%d][1:enable,0:disable].}", device_freq_handle->uc_auto_freq_enable);
    }
    else if (FREQ_GET_FREQ == pst_device_freq->uc_set_type)
    {
        OAM_WARNING_LOG1(0, OAM_SF_RSSI,"{dmac_config_set_device_freq::cpu level[%d].}", PM_WLAN_GetMaxCpuFreq());
        for(uc_index = 0; uc_index < 4; uc_index++)
        {
            OAM_WARNING_LOG2(0, OAM_SF_RSSI,"{dmac_config_set_device_freq::devive pkts[%d]freq level[%d]].}",
            device_ba_pps_freq_level->ul_speed_level, device_ba_pps_freq_level->ul_cpu_freq_level);
            device_ba_pps_freq_level++;
        }
    }
    else if(FREQ_SET_FREQ == pst_device_freq->uc_set_type)/* 调试用设置调频档位 */
    {
        device_freq_handle->uc_req_freq_level = pst_device_freq->uc_set_freq;
        dmac_auto_set_device_freq();
    }
    else if(FREQ_SET_PLAT_FREQ == pst_device_freq->uc_set_type)/* 调试用直接设置CPU频率,必须先关闭调频功能 */
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_auto_set_freq:change CPU freq[%d] to [%d].}", PM_WLAN_GetMaxCpuFreq(), pst_device_freq->uc_set_freq);
        dmac_auto_set_freq(pst_device_freq->uc_set_freq);
    }

    else if(FREQ_SET_FREQ_TC_EN == pst_device_freq->uc_set_type)/* 调试用遍历CPU频率,必须先关闭调频功能 */
    {
        dmac_auto_set_freq_testcase_init();
    }
    else if(FREQ_SET_FREQ_TC_EXIT== pst_device_freq->uc_set_type)
    {
        dmac_auto_set_freq_testcase_exit();
    }
#endif

    return OAL_SUCC;
}
#endif


oal_uint32 dmac_config_set_rx_ampdu_amsdu(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_set_tlv_stru          *pst_config_para;
    hal_to_dmac_device_stru       *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_rx_ampdu_amsdu::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_config_para = (mac_cfg_set_tlv_stru*)puc_param;

    if (pst_config_para->ul_value != 0)
    {
        /* 开启amsdu去使能模式 */
        hal_enable_rx_amsdu_mode(pst_hal_device);
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_rx_ampdu_amsdu::enabled}");
    }
    else
    {
        /* 关闭amsdu去使能模式 */
        hal_disable_rx_amsdu_mode(pst_hal_device);
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_rx_ampdu_amsdu::disabled}");
    }

    return OAL_SUCC;
}


#ifdef _PRE_WLAN_FEATURE_MONITOR

OAL_STATIC oal_uint32  dmac_config_set_addr_filter(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8                       uc_value;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_addr_filter::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_value = (*((oal_int32 *)puc_param) == 0) ? 0 : 1;

    if (0 != uc_value)
    {
        /* 开启monitor模式，不根据帧类型过滤 */
        hal_enable_monitor_mode(pst_hal_device);
    }
    else
    {
        /* 关闭monitor模式，根据帧类型过滤 */
        hal_disable_monitor_mode(pst_hal_device);
    }

    return OAL_SUCC;
}
#endif
#if 0

OAL_STATIC oal_uint32  dmac_config_set_msdu_lifetime(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32  ul_ac;
    oal_uint32  ul_value;
    oal_uint32 *pul_param;

    pul_param = (oal_uint32 *)puc_param;

    ul_ac     = pul_param[1];
    ul_value  = pul_param[2];

    if (ul_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_msdu_lifetime::invalid param, ul_ac=%d ul_value=%d.", ul_ac, ul_value);
        return OAL_FAIL;
    }

    pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[ul_ac].ul_dot11EDCATableMSDULifetime = ul_value;

    return OAL_SUCC;
}
#endif

OAL_STATIC oal_uint32  dmac_config_set_channel(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_channel_param_stru      *pst_channel_param;
    mac_device_stru                 *pst_mac_device;
    oal_uint8                        uc_channel_idx = 0;
    oal_uint32                       ul_beacon_rate;
    hal_to_dmac_device_stru         *pst_hal_device;
    oal_uint32                       ul_ret;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_channel::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_channel_param = (mac_cfg_channel_param_stru *)puc_param;

    ul_ret = mac_get_channel_idx_from_num(pst_channel_param->en_band, pst_channel_param->uc_channel, &uc_channel_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_channel::invalid channel idx: %d}", uc_channel_idx);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    pst_mac_vap->st_channel.uc_chan_number  = pst_channel_param->uc_channel;
    pst_mac_vap->st_channel.en_band         = pst_channel_param->en_band;
    pst_mac_vap->st_channel.en_bandwidth    = pst_channel_param->en_bandwidth;

    pst_mac_vap->st_channel.uc_chan_idx = uc_channel_idx;

    if(MAC_VAP_STATE_UP != pst_mac_vap->en_vap_state)
    {
        pst_mac_vap->en_vap_state = MAC_VAP_STATE_AP_WAIT_START;
    }

    mac_device_set_channel(pst_mac_device, pst_channel_param);
#endif

    /* 更新beacon的发送参数 */
    if ((WLAN_BAND_2G == pst_channel_param->en_band) || (WLAN_BAND_5G == pst_channel_param->en_band))
    {
        ul_beacon_rate = MAC_GET_DMAC_VAP(pst_mac_vap)->ast_tx_mgmt_bmcast[pst_channel_param->en_band].ast_per_rate[0].ul_value;
        hal_vap_set_beacon_rate(MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_vap, ul_beacon_rate);
    }
    else
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_channel::en_band=%d", pst_channel_param->en_band);
    }

    /* disable TSF */
    hal_disable_tsf_tbtt(MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_vap);

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 使能去使能雷达检测 */
    if ((WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)&&(OAL_TRUE == mac_vap_get_dfs_enable(pst_mac_vap)))
    {
        oal_bool_enum_uint8      en_enable_dfs;
        en_enable_dfs = mac_is_ch_in_radar_band(pst_mac_device->en_max_band, pst_mac_vap->st_channel.uc_chan_idx);
        hal_enable_radar_det(pst_hal_device, en_enable_dfs);
#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE
        if ( OAL_TRUE == pst_hal_device->st_dfs_radar_filter.st_dfs_normal_pulse_det.st_timer.en_is_enabled )
        {
            /* 取消定时器 */
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hal_device->st_dfs_radar_filter.st_dfs_normal_pulse_det.st_timer));
            pst_hal_device->st_dfs_radar_filter.st_dfs_normal_pulse_det.en_timer_start = OAL_FALSE;
        }
#endif
    }
#endif

    /* 调hal接口设置带宽 */
#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
    /*dummy*/
#else
    if (pst_channel_param->en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{dmac_config_set_channel:: fpga is not support 80M.}\r\n");
        pst_channel_param->en_bandwidth = WLAN_BAND_WIDTH_20M;
    }
#endif

    /* 非IDLE状态直接配置信道 */
    if (HAL_DEVICE_IDLE_STATE != GET_HAL_DEVICE_STATE(pst_hal_device))
    {
        dmac_vap_work_set_channel(MAC_GET_DMAC_VAP(pst_mac_vap));
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    
#ifdef _PRE_WLAN_FEATURE_DBAC
    if(mac_is_dbac_enabled(pst_mac_device))
    {

    }
    else
#endif
    {
        hal_enable_tsf_tbtt(MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_vap, OAL_FALSE);
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_beacon(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    dmac_vap_stru                   *pst_dmac_vap;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_beacon_param_stru           *pst_beacon_param;
#endif






#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

    pst_beacon_param = (mac_beacon_param_stru*)puc_param;

    /* 1102适配新内核start ap和change beacon接口复用此接口，不同的是change beacon时，不在设置beacon周期
       和dtim周期，因此，change beacon时，interval和dtim period参数为全零，此时不应该被设置到mib中 */
    if ((0 != pst_beacon_param->l_dtim_period) || (0 != pst_beacon_param->l_interval))
    {
        /* 设置mib */
        mac_mib_set_dot11dtimperiod(pst_mac_vap, (oal_uint32)pst_beacon_param->l_dtim_period);
        mac_mib_set_BeaconPeriod(pst_mac_vap, (oal_uint32)pst_beacon_param->l_interval);
    }

    /* 根据下发的参数，更改vap隐藏ssid的能力位信息 */
    mac_vap_set_hide_ssid(pst_mac_vap, pst_beacon_param->uc_hidden_ssid);

    mac_vap_set_security(pst_mac_vap, pst_beacon_param);

    /* 设置short gi */
    mac_mib_set_ShortGIOptionInTwentyImplemented(pst_mac_vap, pst_beacon_param->en_shortgi_20);
    mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, pst_beacon_param->en_shortgi_40);
    mac_mib_set_VHTShortGIOptionIn80Implemented(pst_mac_vap, pst_beacon_param->en_shortgi_80);

    mac_vap_init_by_protocol(pst_mac_vap, pst_beacon_param->en_protocol);
    mac_vap_init_rates(pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_11AC2G
    if ((WLAN_VHT_MODE == pst_mac_vap->en_protocol)
        && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band))
    {
        mac_vap_set_11ac2g(pst_mac_vap, OAL_TRUE);
    }
    else
    {
        mac_vap_set_11ac2g(pst_mac_vap, OAL_FALSE);
    }
#endif

#endif

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    dmac_vap_init_tx_frame_params(pst_dmac_vap, OAL_TRUE);
    dmac_vap_init_tx_ucast_data_frame(pst_dmac_vap);

    hal_vap_set_machw_beacon_period(pst_dmac_vap->pst_hal_vap, (oal_uint16)mac_mib_get_BeaconPeriod(pst_mac_vap));

    return OAL_SUCC;
}



OAL_STATIC oal_uint32  dmac_config_add_user(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint16                      us_user_idx;
    mac_cfg_add_user_param_stru    *pst_add_user;
    dmac_user_stru                 *pst_dmac_user;
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint32                      ul_tid_idx;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_user_ht_hdl_stru            st_ht_hdl;
#endif
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                        uc_hipriv_ack = OAL_FALSE;
#endif


    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_add_user::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }


    pst_add_user = (mac_cfg_add_user_param_stru *)puc_param;

    us_user_idx = pst_add_user->us_user_idx;

    /* 获取dmac user */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_add_user::pst_dmac_user[%d] null.}", us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* TBD hmac_config_add_user此接口删除，相应调用需要整改，duankaiyong&guyanjie */

    /* 设置qos域，后续如有需要可以通过配置命令参数配置 */
    mac_user_set_qos(&pst_dmac_user->st_user_base_info, OAL_TRUE);

    /* 设置HT域 */
    mac_user_get_ht_hdl(&pst_dmac_user->st_user_base_info, &st_ht_hdl);
    st_ht_hdl.en_ht_capable = pst_add_user->en_ht_cap;

    if (OAL_TRUE == pst_add_user->en_ht_cap)
    {
        pst_dmac_user->st_user_base_info.en_cur_protocol_mode                = WLAN_HT_MODE;
        pst_dmac_user->st_user_base_info.en_avail_protocol_mode              = WLAN_HT_MODE;
    }

    /* 设置HT相关的信息:应该在关联的时候赋值 这个值配置的合理性有待考究 2012->page:786 */
    st_ht_hdl.uc_min_mpdu_start_spacing = 6;
    st_ht_hdl.uc_max_rx_ampdu_factor    = 3;
    mac_user_set_ht_hdl(&pst_dmac_user->st_user_base_info, &st_ht_hdl);

    mac_user_set_asoc_state(&pst_dmac_user->st_user_base_info, MAC_USER_STATE_ASSOC);
#endif


    /* 初始化dmac_ht_handle_stru结构体中的uc_ampdu_max_size值 */
    for (ul_tid_idx = 0; ul_tid_idx < WLAN_TID_MAX_NUM; ul_tid_idx++)
    {
        pst_dmac_user->ast_tx_tid_queue[ul_tid_idx].st_ht_tx_hdl.us_ampdu_max_size = 65535;
    }

    /* lut idx已在创建时申请，此处写到硬件中去 */
    hal_machw_seq_num_index_update_per_tid(pst_hal_device,
                                           pst_dmac_user->uc_lut_index,
                                           OAL_TRUE);

    /* 配置命令模拟添加用户，完成后将STA VAP置为UP状态 */
    if ((WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)&&
        (IS_LEGACY_VAP(pst_mac_vap)))
    {
        /*将vap状态改变信息上报*/
        mac_vap_state_change(pst_mac_vap, MAC_VAP_STATE_UP);
    }

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#if(_PRE_OS_VERSION ==_PRE_OS_VERSION_WIN32)   //for UT,UT需要通过配置命令模拟用户的关联
    dmac_bsd_user_add_handle(MAC_GET_DMAC_VAP(pst_mac_vap),pst_dmac_user);
#endif
#endif
    /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_del_user(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint16                      us_user_idx;
    mac_cfg_del_user_param_stru    *pst_del_user;
    dmac_user_stru                 *pst_dmac_user;
    mac_device_stru                *pst_mac_device;
    mac_chip_stru                  *pst_mac_chip;

    pst_del_user = (mac_cfg_del_user_param_stru *)puc_param;

    us_user_idx = pst_del_user->us_user_idx;

    /* 获取dmac user */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_config_del_user::pst_dmac_user[%d] null.}", us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    /* 归还RA LUT IDX */
    pst_mac_chip = dmac_res_get_mac_chip(pst_mac_device->uc_chip_id);
    if(OAL_PTR_NULL == pst_mac_chip)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
    mac_user_del_ra_lut_index(pst_mac_chip->st_lut_table.auc_ra_lut_index_table, pst_dmac_user->uc_lut_index);
    hal_ce_del_peer_macaddr(MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_device, pst_dmac_user->uc_lut_index);
    /* 配置命令模拟删除用户，完成后将STA VAP置为FAKE_UP状态 */
    if ((WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)&&
        (IS_LEGACY_VAP(pst_mac_vap)))

    {
        /*将vap状态改变信息上报*/
        mac_vap_state_change(pst_mac_vap, MAC_VAP_STATE_STA_FAKE_UP);
    }

    dmac_alg_del_assoc_user_notify(MAC_GET_DMAC_VAP(pst_mac_vap), pst_dmac_user);

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#if(_PRE_OS_VERSION ==_PRE_OS_VERSION_WIN32)   //for UT,UT需要通过配置命令模拟用户的删除
    dmac_bsd_user_del_handle(MAC_GET_DMAC_VAP(pst_mac_vap),pst_dmac_user);
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
    hal_pow_del_machw_resp_power_lut_entry(MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_device, pst_dmac_user->uc_lut_index);
#endif

    return OAL_SUCC;
}
oal_uint32  dmac_cali_hmac2dmac_recv(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    frw_event_hdr_stru         *pst_event_hdr;
    oal_netbuf_stru            *pst_cali_data_netbuf;
    dmac_tx_event_stru         *pst_dtx_event;
    dmac_vap_stru              *pst_dmac_vap;
    hal_to_dmac_device_stru    *pst_hal_device;
   //oal_uint8                  *puc_content;
   //oal_uint32                  ul_byte;
#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
    oal_uint8                  *puc_cali_data;
#endif

   OAM_INFO_LOG0(0, OAM_SF_CALIBRATE, "{dmac_cali_hmac2dmac_recv function called.}");
   //OAL_IO_PRINT("dmac_cali_hmac2dmac_recv: start\r\n");

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CALIBRATE, "{dmac_cali_hmac2dmac_recv::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event            = frw_get_event_stru(pst_event_mem);
    pst_event_hdr        = &(pst_event->st_event_hdr);
    pst_dtx_event        = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_cali_data_netbuf = pst_dtx_event->pst_netbuf;
    pst_dmac_vap  = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event_hdr->uc_vap_id);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG1(0, OAM_SF_CALIBRATE, "{dmac_cali_hmac2dmac_recv::pst_dmac_vap[%d] is NULL!}", pst_event_hdr->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_CALIBRATE, "{dmac_cali_hmac2dmac_recv::pst_hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if 0
    puc_content = (oal_uint8 *)puc_cali_data;

    for (ul_byte = 0; ul_byte < OAL_SIZEOF(oal_cali_param_stru); ul_byte+=4)
    {
        OAL_IO_PRINT("%02X %02X %02X %02X\r\n", puc_content[ul_byte], puc_content[ul_byte+1],
                      puc_content[ul_byte+2], puc_content[ul_byte+3]);
    }

    for(idx = 0; idx < DPD_CALI_LUT_LENGTH; idx++)
    {
      OAM_ERROR_LOG1(0, OAM_SF_CALIBRATE, "{dmac_dpd_data_processed_recv  received: 0x%X}", pst_dpd_cali_data_processed->us_dpd_data[idx]);
    }
#endif
    // write dpd data back to register

#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
    puc_cali_data = (oal_uint8 *)OAL_NETBUF_DATA(pst_cali_data_netbuf);
    hal_cali_send_func(pst_hal_device, puc_cali_data, pst_dtx_event->us_frame_len, pst_dtx_event->us_remain);
#endif

    oal_netbuf_free(pst_cali_data_netbuf);
    return OAL_SUCC;
}


oal_uint32  dmac_app_ie_h2d_recv(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                *pst_event;
    frw_event_hdr_stru            *pst_event_hdr;
    oal_netbuf_stru               *pst_app_ie_netbuf;
    dmac_tx_event_stru            *pst_dtx_event;
    mac_vap_stru                  *pst_mac_vap;
    oal_uint8                     *puc_app_ie_data;
    oal_uint8                     *puc_app_ie_body;
    oal_uint32                     ul_ret;
    oal_app_ie_stru                st_app_ie;
    oal_uint8                      uc_app_ie_header_len;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_app_ie_h2d_recv::pst_event_mem is null,return.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event            = frw_get_event_stru(pst_event_mem);
    pst_event_hdr        = &(pst_event->st_event_hdr);
    pst_dtx_event        = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_app_ie_netbuf    = pst_dtx_event->pst_netbuf;
    pst_mac_vap          = (mac_vap_stru *)mac_res_get_mac_vap(pst_event_hdr->uc_vap_id);

    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_app_ie_h2d_recv::vap id is  %d.}", pst_event_hdr->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if(OAL_PTR_NULL == pst_app_ie_netbuf)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_app_ie_h2d_recv::net_buffer is  null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_app_ie_header_len = OAL_SIZEOF(oal_app_ie_stru) - OAL_SIZEOF(st_app_ie.auc_ie)/OAL_SIZEOF(st_app_ie.auc_ie[0]);

    /*帧长校验*/
    if((pst_dtx_event->us_frame_len < uc_app_ie_header_len) || (pst_dtx_event->us_frame_len > WLAN_LARGE_NETBUF_SIZE))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "{dmac_app_ie_h2d_recv:: frame_len = [%d],invailid len return.}",
        pst_dtx_event->us_frame_len);
        oal_netbuf_free(pst_app_ie_netbuf);
        return OAL_FAIL;
    }

    /*取出header*/
    puc_app_ie_data = (oal_uint8 *)OAL_NETBUF_DATA(pst_app_ie_netbuf);
    if(OAL_PTR_NULL == puc_app_ie_data)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_app_ie_h2d_recv::puc_app_ie_data is  null.}");
        oal_netbuf_free(pst_app_ie_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(&st_app_ie, OAL_SIZEOF(oal_app_ie_stru));
    /*获取app ie 数据头*/
    oal_memcopy((oal_uint8 *)&st_app_ie, puc_app_ie_data, uc_app_ie_header_len);

    /*ie_len 检查*/
    if(st_app_ie.ul_ie_len > WLAN_WPS_IE_MAX_SIZE)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_app_ie_h2d_recv::app_ie_len(%d) > WLAN_WPS_IE_MAX_SIZE(%d), app_ie_len invalid.}",
        st_app_ie.ul_ie_len, WLAN_WPS_IE_MAX_SIZE);
        oal_netbuf_free(pst_app_ie_netbuf);
        return OAL_FAIL;
    }

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_app_ie_h2d_recv::p2p_ie_type=[%d], p2p_ie_len=[%d].}",
      st_app_ie.en_app_ie_type, st_app_ie.ul_ie_len);

    /*获取app ie 数据*/
    puc_app_ie_body = puc_app_ie_data + uc_app_ie_header_len;
    oal_memcopy(st_app_ie.auc_ie, puc_app_ie_body, st_app_ie.ul_ie_len);

    ul_ret = mac_vap_save_app_ie(pst_mac_vap, &st_app_ie, st_app_ie.en_app_ie_type);
    if (OAL_SUCC != ul_ret)
    {
        oal_netbuf_free(pst_app_ie_netbuf);
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_app_ie_h2d_recv::mac_vap_save_app_ie failed[%d], en_type[%d], len[%d].}",
                    ul_ret,st_app_ie.en_app_ie_type,st_app_ie.ul_ie_len);
        return ul_ret;
    }

    oal_netbuf_free(pst_app_ie_netbuf);
    return OAL_SUCC;
}


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
enum
{
    CHECK_LTE_GPIO_INIT            = 0,    /* 初始化 */
    CHECK_LTE_GPIO_LOW             = 1,    /* 设置为低电平 */
    CHECK_LTE_GPIO_HIGH            = 2,    /*设置为高电平 */
    CHECK_LTE_GPIO_RESUME          = 3,    /*恢复寄存器设置 */
    CHECK_LTE_GPIO_DEV_LEVEL       = 4,    /*读取device GPIO管脚电平值*/
    CHECK_LTE_GPIO_BUTT
};

OAL_STATIC oal_uint32 dmac_config_lte_gpio_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                           ul_mode_value;
    oal_uint8                            uc_gpio_level = 0;
    oal_uint8                            uc_lte_check_result = 0;
    dmac_atcmdsrv_atcmd_response_event   st_atcmdsrv_lte_gpio_check;

    UNREF_PARAM(CHECK_LTE_GPIO_BUTT)
    ul_mode_value = *(oal_uint32 *)puc_param;
    OAM_INFO_LOG1(0, 0, "dmac_config_lte_gpio_mode enter, device gpio mode is %d", ul_mode_value);
    /*初始化device gpio*/
    if(CHECK_LTE_GPIO_INIT == ul_mode_value)
    {
        hal_set_lte_gpio_mode(ul_mode_value);
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        /* VICTORIA only use two coe pin here */
        ConfigGpioPin(GPIO_BASE_ADDR,GPIO_3,GPIO_INOUT_IN, GPIO_TYPE_IO);
        ConfigGpioPin(GPIO_BASE_ADDR,GPIO_4,GPIO_INOUT_IN, GPIO_TYPE_IO);
        /* seven gpio pins, reserved for other product */
        //ConfigGpioPin(GPIO_BASE_ADDR,GPIO_5,GPIO_INOUT_IN, GPIO_TYPE_IO);
        //ConfigGpioPin(GPIO_BASE_ADDR,GPIO_6,GPIO_INOUT_IN, GPIO_TYPE_IO);
        //ConfigGpioPin(GPIO_BASE_ADDR,GPIO_7,GPIO_INOUT_IN, GPIO_TYPE_IO);
        //ConfigGpioPin(GPIO_BASE_ADDR,GPIO_8,GPIO_INOUT_IN, GPIO_TYPE_IO);
        //ConfigGpioPin(GPIO_BASE_ADDR,GPIO_9,GPIO_INOUT_IN, GPIO_TYPE_IO);
        //ConfigGpioPin(GPIO_BASE_ADDR,GPIO_10,GPIO_INOUT_IN, GPIO_TYPE_IO);
        //ConfigGpioPin(GPIO_BASE_ADDR,GPIO_11,GPIO_INOUT_IN, GPIO_TYPE_IO);
#else
        ConfigGpioPin(GPIO_BASE_ADDR,GPIO_2,GPIO_INOUT_IN, GPIO_TYPE_IO);
        ConfigGpioPin(GPIO_BASE_ADDR,GPIO_5,GPIO_INOUT_IN, GPIO_TYPE_IO);
        ConfigGpioPin(GPIO_BASE_ADDR,GPIO_6,GPIO_INOUT_IN, GPIO_TYPE_IO);
#endif
#endif
    }
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
    else if(CHECK_LTE_GPIO_LOW == ul_mode_value)/*将GPIO全部设置为0*/
    {
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_3,GPIO_LEVEL_LOW);
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_4,GPIO_LEVEL_LOW);
        /* seven gpio pins, reserved for other product */
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_5,GPIO_LEVEL_LOW);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_6,GPIO_LEVEL_LOW);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_7,GPIO_LEVEL_LOW);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_8,GPIO_LEVEL_LOW);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_9,GPIO_LEVEL_LOW);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_10,GPIO_LEVEL_LOW);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_11,GPIO_LEVEL_LOW);
#else
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_2,GPIO_LEVEL_LOW);
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_5,GPIO_LEVEL_LOW);
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_6,GPIO_LEVEL_LOW);
#endif
    }
    else if(CHECK_LTE_GPIO_HIGH == ul_mode_value) /*??GPIOè?2?éè???a1*/
    {
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_3,GPIO_LEVEL_HIGH);
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_4,GPIO_LEVEL_HIGH);
        /* seven gpio pins, reserved for other product */
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_5,GPIO_LEVEL_HIGH);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_6,GPIO_LEVEL_HIGH);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_7,GPIO_LEVEL_HIGH);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_8,GPIO_LEVEL_HIGH);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_9,GPIO_LEVEL_HIGH);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_10,GPIO_LEVEL_HIGH);
        //oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_11,GPIO_LEVEL_HIGH);
#else
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_2,GPIO_LEVEL_HIGH);
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_5,GPIO_LEVEL_HIGH);
        oal_set_gpio_level(GPIO_BASE_ADDR,GPIO_6,GPIO_LEVEL_HIGH);
#endif
    }
    else if (CHECK_LTE_GPIO_DEV_LEVEL == ul_mode_value)
    {
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_3, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 3) & 0xFF);
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_4, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 4) & 0xFF);
        /*
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_5, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 5) & 0xFF);
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_6, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 6) & 0xFF);
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_7, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 7) & 0xFF);
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_8, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 8) & 0xFF);
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_9, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 9) & 0xFF);
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_10, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 10) & 0xFF);
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_11, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 11) & 0xFF);
        */
#else
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_2, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 2) & 0xFF);
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_5, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 5) & 0xFF);
        oal_get_gpio_level(GPIO_BASE_ADDR, GPIO_6, &uc_gpio_level);
        uc_lte_check_result |= ((uc_gpio_level << 6) & 0xFF);
#endif
    }
#endif
    else if(CHECK_LTE_GPIO_RESUME == ul_mode_value) /*???′??′??÷éè??*/
    {
        hal_set_lte_gpio_mode(ul_mode_value);
    }
    else
    {
        return OAL_FAIL;
    }
    st_atcmdsrv_lte_gpio_check.uc_event_id = OAL_ATCMDSRV_LTE_GPIO_CHECK;
    st_atcmdsrv_lte_gpio_check.uc_reserved = uc_lte_check_result;
    OAM_WARNING_LOG1(0, 0, "dmac_config_lte_gpio_mode::gpio_check_result is 0x%x", uc_lte_check_result);
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHECK_LTE_GPIO, OAL_SIZEOF(dmac_atcmdsrv_atcmd_response_event), (oal_uint8 *)&st_atcmdsrv_lte_gpio_check);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_M2S

OAL_STATIC oal_void dmac_connect_ant_change(dmac_vap_stru *pst_dmac_vap,
                                                        mac_conn_security_stru *pst_conn)
{
    hal_to_dmac_device_stru *pst_hal_device;
    oal_uint8                  uc_rssi_abs = 0;
    oal_int8                   c_ant0_rssi;
    oal_int8                   c_ant1_rssi;
    hal_rx_ant_rssi_mgmt_stru  *pst_hal_rx_ant_rssi_mgmt;


    pst_hal_device = pst_dmac_vap->pst_hal_device;

    if(OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{dmac_connect_ant_change::pst_hal_device null.}");
        return;
    }

    pst_hal_rx_ant_rssi_mgmt = GET_HAL_DEVICE_RX_ANT_RSSI_MGMT(pst_hal_device);

    /* 不为MIMO，不处理 */
    if (HAL_M2S_STATE_MIMO != GET_HAL_M2S_CUR_STATE(pst_hal_device))
    {
        return;
    }

    if(OAL_TRUE != pst_hal_rx_ant_rssi_mgmt->en_ant_rssi_sw)
    {
        return;
    }

    oal_rssi_smooth(&pst_hal_rx_ant_rssi_mgmt->s_ant0_rssi_smth, pst_conn->c_ant0_rssi);
    oal_rssi_smooth(&pst_hal_rx_ant_rssi_mgmt->s_ant1_rssi_smth, pst_conn->c_ant1_rssi);

    c_ant0_rssi = oal_get_real_rssi(pst_hal_rx_ant_rssi_mgmt->s_ant0_rssi_smth);
    c_ant1_rssi = oal_get_real_rssi(pst_hal_rx_ant_rssi_mgmt->s_ant1_rssi_smth);

    uc_rssi_abs = (oal_uint8)OAL_ABSOLUTE_SUB(c_ant0_rssi, c_ant1_rssi);

    if (OAL_TRUE == pst_hal_rx_ant_rssi_mgmt->en_log_print)
    {
        OAM_WARNING_LOG4(0, OAM_SF_CFG, "dmac_connect_ant_change::c_rssi_abs[%d],ant0[%d],ant1[%d],rssi_th[%d]",
                           uc_rssi_abs,(oal_int32)c_ant0_rssi,(oal_int32)c_ant1_rssi,pst_hal_rx_ant_rssi_mgmt->uc_rssi_th);
    }

    if(uc_rssi_abs > pst_hal_rx_ant_rssi_mgmt->uc_rssi_th)
    {
        if(c_ant0_rssi > c_ant1_rssi)
        {
            /* 切到c0 */
            dmac_m2s_mgmt_switch(pst_hal_device,pst_dmac_vap,WLAN_TX_CHAIN_ZERO);
            if (OAL_TRUE == pst_hal_rx_ant_rssi_mgmt->en_log_print)
            {
                OAM_WARNING_LOG0(0, OAM_SF_CFG,"dmac_connect_ant_change::use c0");
            }
        }
        else
        {
            /* 切到c1 */
            dmac_m2s_mgmt_switch(pst_hal_device,pst_dmac_vap,WLAN_TX_CHAIN_ONE);
            if (OAL_TRUE == pst_hal_rx_ant_rssi_mgmt->en_log_print)
            {
                OAM_WARNING_LOG0(0, OAM_SF_CFG,"dmac_connect_ant_change::use c1");
            }
        }
    }
}
#endif

OAL_STATIC oal_uint32 dmac_config_connect(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint32                                   ul_ret;
    mac_conn_security_stru                      *pst_conn;
    dmac_vap_stru                               *pst_dmac_vap;
#ifdef _PRE_WLAN_FEATURE_11R
    oal_uint8                                    uc_akm_type;
    oal_uint32                                   ul_akm_suite;
#endif

    pst_conn = (mac_conn_security_stru *)puc_param;

    ul_ret = mac_vap_init_privacy(pst_mac_vap, pst_conn);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_connect::mac_11i_init_privacy fail [%d].}", ul_ret);
        return ul_ret;
    }

    /* 更新上报的RSSI */
    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    /* 处理connect时，初始化一次rssi的值，保证dmac_vap下保存的值最新 */
    pst_dmac_vap->st_query_stats.s_signal = OAL_RSSI_INIT_MARKER;

    oal_rssi_smooth(&(pst_dmac_vap->st_query_stats.s_signal), pst_conn->c_rssi);

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,"dmac_config_connect::c_ant0_rssi[%d],c_ant1_rssi[%d]",pst_conn->c_ant0_rssi,pst_conn->c_ant1_rssi);
#ifdef _PRE_WLAN_FEATURE_M2S
    dmac_connect_ant_change(pst_dmac_vap,pst_conn);
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_11R
    if(OAL_TRUE == pst_dmac_vap->bit_11r_enable)
    {
        ul_akm_suite = OAL_NTOH_32(pst_conn->st_crypto.aul_akm_suite[0]);
        uc_akm_type  = ul_akm_suite & 0xFF;
        if ((WLAN_AUTH_SUITE_FT_1X == uc_akm_type) || (WLAN_AUTH_SUITE_FT_PSK == uc_akm_type) ||
                (WLAN_AUTH_SUITE_FT_SHA256 == uc_akm_type))
        {
            mac_mib_set_AuthenticationMode(pst_mac_vap, WLAN_WITP_AUTH_FT);
        }

        mac_mib_init_ft_cfg(pst_mac_vap, pst_conn->auc_mde);
    }
#endif //_PRE_WLAN_FEATURE_11R

    /* 通知算法 */
    dmac_alg_cfg_start_connect_notify(pst_mac_vap, pst_conn->c_rssi);
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_BTCOEX

OAL_STATIC oal_uint32 dmac_config_print_btcoex_status(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru   *pst_dmac_vap;
    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);


    hal_get_btcoex_statistic(pst_dmac_vap->pst_hal_device, OAL_FALSE);
    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_config_btcoex_preempt_type(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_btcoex_preempt_mgr_stru *pst_btcoex_preempt_mgr;
    hal_to_dmac_device_stru    *pst_hal_device;
    dmac_vap_btcoex_stru        *pst_dmac_vap_btcoex;
    dmac_vap_stru               *pst_dmac_vap = OAL_PTR_NULL;
    mac_user_stru               *pst_mac_user = OAL_PTR_NULL;
    oal_dlist_head_stru         *pst_entry;

    pst_btcoex_preempt_mgr = (mac_btcoex_preempt_mgr_stru*)puc_param;

    /* 硬件preempt */
    if(0 == pst_btcoex_preempt_mgr->uc_cfg_preempt_mode)
    {
        pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

        pst_dmac_vap_btcoex = &(pst_dmac_vap->st_dmac_vap_btcoex);

        pst_dmac_vap_btcoex->en_all_abort_preempt_type = (hal_coex_hw_preempt_mode_enum_uint8)pst_btcoex_preempt_mgr->uc_cfg_preempt_type;

        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_COEX, "{dmac_config_btcoex_preempt_type::hw preempt_type[%d]!}",
                pst_dmac_vap_btcoex->en_all_abort_preempt_type);

        /* AP按照第一个用户生效，因为null和qos null只能针对特定用户，实际ap只用self-cts */
        OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
        {
            pst_mac_user = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

            /* 用户为空，表示未关联状态，此时只能采用0和1方式,配置的方式2和3只能等关联时候生效到硬件 */
            /*lint -save -e774 */
            if (OAL_PTR_NULL == pst_mac_user)
            {
                if(HAL_BTCOEX_HW_POWSAVE_SELFCTS >= pst_dmac_vap_btcoex->en_all_abort_preempt_type)
                {
                    dmac_btcoex_init_preempt(pst_mac_vap, pst_mac_user);
                }
            }
            else
            {
                dmac_btcoex_init_preempt(pst_mac_vap, pst_mac_user);
            }
            /*lint -restore */

            break;
        }
    }
    /* 软件preempt */
    else
    {
        pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
        if (OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG0(0, OAM_SF_COEX, "{dmac_config_btcoex_preempt_type::pst_hal_device null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        *(oal_uint8*)(&(pst_hal_device->st_btcoex_sw_preempt.st_sw_preempt_mode)) = pst_btcoex_preempt_mgr->uc_cfg_preempt_type;

        OAM_WARNING_LOG1(0, OAM_SF_COEX, "{dmac_config_btcoex_preempt_type::sw preempt_type[0x%x]!}",
            *(oal_uint8*)(&(pst_hal_device->st_btcoex_sw_preempt.st_sw_preempt_mode)));
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_config_btcoex_set_perf_param(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_btcoex_mgr_stru            *pst_btcoex_mgr;

    pst_btcoex_mgr = (mac_btcoex_mgr_stru*)puc_param;

    if(0 == pst_btcoex_mgr->uc_cfg_btcoex_mode)
    {
        dmac_btcoex_perf_param_show();
    }
    else
    {
        dmac_btcoex_perf_param_update(pst_btcoex_mgr);
    }

    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_FEATURE_LTECOEX

OAL_STATIC oal_uint32 dmac_config_ltecoex_mode_set(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_chip_stru          *pst_hal_chip;
    hal_to_dmac_device_stru        *pst_hal_device;

    OAM_WARNING_LOG1(0, OAM_SF_COEX, "{dmac_config_ltecoex_mode_set::ltecoex_mode:%d}" , *puc_param);

    pst_hal_chip = DMAC_VAP_GET_HAL_CHIP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_chip)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_COEX, "{dmac_config_ltecoex_mode_set:: DMAC_VAP_GET_HAL_CHIP null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_COEX, "{dmac_config_ltecoex_mode_set:: DMAC_VAP_GET_HAL_DEVICE null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(*puc_param)
    {
        /*设置LTE共存开启标志位*/
        pst_hal_chip->ul_lte_coex_status = 1;

        /*使能 mac的共存功能，COEX_ABORT_CTRL 的 bit0置为1*/
        hal_set_btcoex_sw_all_abort_ctrl(pst_hal_device, 1);

        /*使能PTA对lte业务的处理*/
        hal_ltecoex_req_mask_ctrl(0);
    }
    else
    {
        /*设置LTE共存关闭标志位*/
        pst_hal_chip->ul_lte_coex_status = 0;

        /*如果BT没有开启,才关闭mac的共存功能*/
        if(0 == pst_hal_chip->st_btcoex_btble_status.un_bt_status.st_bt_status.bit_bt_on)
        {
            hal_set_btcoex_sw_all_abort_ctrl(pst_hal_device, 0);
            hal_set_btcoex_hw_rx_priority_dis(pst_hal_device, 1);
        }

        /*屏蔽PTA对lte业务的处理*/
        hal_ltecoex_req_mask_ctrl(0xF);
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC oal_uint32  dmac_config_set_dscr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_set_dscr_param_stru     *pst_event_set_dscr;
    dmac_vap_stru                   *pst_dmac_vap;

    pst_event_set_dscr = (mac_cfg_set_dscr_param_stru *)puc_param;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);


#ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
    /* 若输入默认的set_ucast_data命令, 则将所有针对特定协议的配置valid标志清0 */
    if (pst_event_set_dscr->en_type == MAC_VAP_CONFIG_UCAST_DATA)
    {
        pst_dmac_vap->un_mode_valid.uc_mode_param_valid = 0;
    }
    /* 否则, 如果是输入新的set_mode_ucast_data命令, 则将对应协议的配置valid标志置1 */
    else if (pst_event_set_dscr->en_type >= MAC_VAP_CONFIG_MODE_UCAST_DATA)
    {
        switch (pst_event_set_dscr->en_type)
        {
            case MAC_VAP_CONFIG_VHT_UCAST_DATA:
                pst_dmac_vap->un_mode_valid.st_spec_mode.bit_vht_param_vaild = 1;
                break;
            case MAC_VAP_CONFIG_HT_UCAST_DATA:
                pst_dmac_vap->un_mode_valid.st_spec_mode.bit_ht_param_vaild = 1;
                break;
            case MAC_VAP_CONFIG_11AG_UCAST_DATA:
                pst_dmac_vap->un_mode_valid.st_spec_mode.bit_11ag_param_vaild = 1;
                break;
            case MAC_VAP_CONFIG_11B_UCAST_DATA:
                pst_dmac_vap->un_mode_valid.st_spec_mode.bit_11b_param_vaild = 1;
                break;
            default:
                OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr::pst_event_set_dscr->en_type = %u, invalid}", pst_event_set_dscr->en_type);
                return OAL_FAIL;
        }
    }
#endif

    g_dmac_config_set_dscr_param[pst_event_set_dscr->uc_function_index](pst_event_set_dscr->l_value,
                                                                        pst_event_set_dscr->en_type,
                                                                        pst_dmac_vap);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_GREEN_AP

OAL_STATIC oal_uint32 dmac_config_set_green_ap_en(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8      uc_param = *(oal_uint8 *)puc_param;

    OAL_IO_PRINT("dmac_config_set_green_ap_en %d\r\n", uc_param);

    if (0 == uc_param)
    {
        dmac_green_ap_disable(pst_mac_vap->uc_device_id);
    }
    else if (1 == uc_param)
    {
        dmac_green_ap_enable(pst_mac_vap->uc_device_id);
    }
    else if (2 == uc_param)
    {
        dmac_green_ap_dump_info(pst_mac_vap->uc_device_id);
    }
    else if (3 == uc_param)
    {
        dmac_green_ap_set_debug_mode(pst_mac_vap->uc_device_id, 1);
    }
    else if (4 == uc_param)
    {
        dmac_green_ap_set_debug_mode(pst_mac_vap->uc_device_id, 0);
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC oal_uint32  dmac_config_set_country(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_regdomain_info_stru *pst_mac_regdom;
    mac_regdomain_info_stru *pst_regdomain_info;
    oal_uint8                uc_rc_num;
    oal_uint32               ul_size;
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hal_to_dmac_device_stru  *pst_hal_device = OAL_PTR_NULL;
    oal_uint8                 uc_device_nums;
    oal_uint8                 uc_device_id;
#endif



    pst_mac_regdom = (mac_regdomain_info_stru *)puc_param;
    /* 获取管制类的个数 */
    uc_rc_num = pst_mac_regdom->uc_regclass_num;
    /* 计算配置命令 */
    ul_size = OAL_SIZEOF(mac_regclass_info_stru) * uc_rc_num + MAC_RD_INFO_LEN;
    /*获取管制域全局变量*/
    mac_get_regdomain_info(&pst_regdomain_info);
    /* 更新管制域信息 */
    oal_memcopy(pst_regdomain_info, pst_mac_regdom, ul_size);
    /* 更新信道的管制域信息 */
    mac_init_channel_list();

#ifdef _PRE_SUPPORT_ACS
    dmac_acs_report_support_chan(pst_mac_vap);
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hal_chip_get_device_num(pst_mac_vap->uc_chip_id, &uc_device_nums);
    for (uc_device_id = 0; uc_device_id < uc_device_nums; uc_device_id++)
    {
        hal_get_hal_to_dmac_device(pst_mac_vap->uc_chip_id, uc_device_id, &pst_hal_device);
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        /* 更新FCC认证要求国家标志位 */
        pst_hal_device->uc_fcc_country = pst_mac_regdom->en_fcc_country;
    }
#endif

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_country::country code=%c%c fcc_flag[%d].}",
                     (oal_uint8)pst_mac_regdom->ac_country[0],(oal_uint8)pst_mac_regdom->ac_country[1],
                     pst_mac_regdom->en_fcc_country);

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_DFS

OAL_STATIC oal_uint32  dmac_config_set_country_for_dfs(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_dfs_domain_enum_uint8       en_dfs_domain  = *puc_param;
    hal_to_dmac_device_stru        *pst_hal_device = OAL_PTR_NULL;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
#ifdef _PRE_WLAN_FEATURE_DOUBLE_CHIP
    mac_device_stru                *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    /* 2G芯片不需要进行雷达相关配置 */
    if (g_uc_wlan_double_chip_2g_id == pst_mac_device->uc_chip_id)
    {
        return OAL_SUCC;
    }
#endif
#endif
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if(OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_country_for_dfs::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_radar_config_reg(pst_hal_device, en_dfs_domain);

#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE
    pst_hal_device->st_dfs_radar_filter.en_log_switch                     = 0;
    pst_hal_device->st_dfs_radar_filter.en_radar_type                     = en_dfs_domain;
    pst_hal_device->st_dfs_radar_filter.uc_chirp_cnt                      = 0;

    pst_hal_device->st_dfs_radar_filter.uc_chirp_cnt_total                = 0;
    pst_hal_device->st_dfs_radar_filter.en_chirp_timeout                  = OAL_FALSE;
    pst_hal_device->st_dfs_radar_filter.st_dfs_normal_pulse_det.en_timeout       = OAL_FALSE;

    pst_hal_device->st_dfs_radar_filter.en_crazy_report_cnt               = OAL_FALSE;
    pst_hal_device->st_dfs_radar_filter.uc_chirp_cnt_for_crazy_report_det = 0;

    pst_hal_device->st_dfs_radar_filter.st_timer_disable_chirp_det.ul_timeout = 2*60*1000;
    pst_hal_device->st_dfs_radar_filter.st_timer_crazy_report_det.ul_timeout  = 20*1000;

    /*使能normal pulse定时器*/
    pst_hal_device->st_dfs_radar_filter.st_dfs_normal_pulse_det.en_is_enabled = OAL_TRUE;

    /*使能chirp crazy report定时器*/
    pst_hal_device->st_dfs_radar_filter.en_crazy_report_is_enabled            = OAL_TRUE;

    /*误报过滤条件*/
    pst_hal_device->st_dfs_radar_filter.st_dfs_pulse_check_filter.en_fcc_chirp_pow_diff         = OAL_TRUE;
    pst_hal_device->st_dfs_radar_filter.st_dfs_pulse_check_filter.en_fcc_chirp_duration_diff    = OAL_TRUE;
    pst_hal_device->st_dfs_radar_filter.st_dfs_pulse_check_filter.en_fcc_type4_duration_diff    = OAL_TRUE;
    pst_hal_device->st_dfs_radar_filter.st_dfs_pulse_check_filter.en_fcc_chirp_eq_duration_num  = OAL_TRUE;
#endif

    pst_hal_device->st_dfs_radar_filter.ul_last_burst_timestamp           = 0;
    pst_hal_device->st_dfs_radar_filter.uc_chirp_wow_wake_flag            = 0;
    pst_hal_device->st_dfs_radar_filter.ul_last_burst_timestamp_for_chirp = 0;
    pst_hal_device->st_dfs_radar_filter.en_chirp_enable                   = 1;
    pst_hal_device->st_dfs_radar_filter.ul_time_threshold                 = 100;
    switch(en_dfs_domain)
    {
        case MAC_DFS_DOMAIN_ETSI:
            pst_hal_device->st_dfs_radar_filter.ul_chirp_cnt_threshold  = ETSI_RADAR_CHIRP_CNT;
            pst_hal_device->st_dfs_radar_filter.ul_chirp_time_threshold = 100;
            pst_hal_device->st_dfs_radar_filter.ul_time_threshold       = 100;
            break;
        case MAC_DFS_DOMAIN_FCC:
            pst_hal_device->st_dfs_radar_filter.ul_chirp_cnt_threshold  = FCC_RADAR_CHIRP_CNT;
            pst_hal_device->st_dfs_radar_filter.ul_chirp_time_threshold = 12000;
            pst_hal_device->st_dfs_radar_filter.ul_time_threshold       = 200;
            break;
       case MAC_DFS_DOMAIN_MKK:
            pst_hal_device->st_dfs_radar_filter.ul_chirp_cnt_threshold  = MKK_RADAR_CHIRP_CNT;
            pst_hal_device->st_dfs_radar_filter.ul_chirp_time_threshold = 12000;
            pst_hal_device->st_dfs_radar_filter.ul_time_threshold       = 200;
            break;
        default :
            break;
    }

    OAM_WARNING_LOG0(0, OAM_SF_DFS, "{dmac_config_set_country_for_dfs::set radar filter params.}");

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_TPC

OAL_STATIC oal_uint32 dmac_config_reduce_sar(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint8                       auc_sar_ctrl_params[HAL_CUS_NUM_OF_SAR_PARAMS<<1];
    oal_uint8                       auc_sar_params[WLAN_RF_CHANNEL_NUMS][HAL_CUS_NUM_OF_SAR_PARAMS] = {{0}};
    oal_uint8                       uc_band_idx;
    oal_uint8                       uc_rf_idx;
    oal_uint8                       uc_param_idx = 0;

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_TPC, "{dmac_config_reduce_sar::ENTER!}");
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    oal_memcopy(auc_sar_ctrl_params, puc_param, OAL_SIZEOF(auc_sar_ctrl_params));
    for (uc_rf_idx = 0; uc_rf_idx < WLAN_RF_CHANNEL_NUMS; uc_rf_idx++)
    {
        for (uc_band_idx = 0; uc_band_idx < WLAN_5G_SUB_BAND_NUM; uc_band_idx++)
        {
            auc_sar_params[uc_rf_idx][uc_band_idx] = auc_sar_ctrl_params[uc_param_idx];
            uc_param_idx++;
        }
    }
    auc_sar_params[0][uc_band_idx]  = auc_sar_ctrl_params[uc_param_idx];
    auc_sar_params[1][uc_band_idx]  = auc_sar_ctrl_params[uc_param_idx+1];
    if (0 == oal_memcmp(g_auc_sar_ctrl_params, auc_sar_params, OAL_SIZEOF(auc_sar_params)))
    {
        return OAL_SUCC;
    }

    oal_memcopy(g_auc_sar_ctrl_params, auc_sar_params, OAL_SIZEOF(auc_sar_params));
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
    hal_update_upc_amend_by_sar(pst_hal_device);
#endif

    return OAL_SUCC;
}
#endif


#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
OAL_STATIC oal_uint32  dmac_config_set_rate(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_non_ht_rate_stru        *pst_event_set_rate;
    dmac_vap_stru                   *pst_dmac_vap;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_rate::pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }


    /* 设置数据 */
    pst_event_set_rate = (mac_cfg_non_ht_rate_stru *)puc_param;
    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate = pst_event_set_rate->en_rate;
    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = pst_event_set_rate->en_protocol_mode;

    /* 使用长前导码，兼容1Mbps */
    if (WLAN_LEGACY_11b_RESERVED1 == pst_event_set_rate->en_rate)
    {
        pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.bit_preamble_mode = 1;
    }

    /* 更新协议速率 */
    pst_dmac_vap->uc_protocol_rate_dscr = (oal_uint8)((pst_event_set_rate->en_protocol_mode << 6) | pst_event_set_rate->en_rate);

    /* 根据模式、频点、带宽、速率来设置DAC和LPF增益 */
    hal_set_dac_lpf_gain(pst_dmac_vap->pst_hal_device, pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.en_bandwidth, pst_mac_vap->st_channel.uc_chan_number, pst_event_set_rate->en_protocol_mode, pst_event_set_rate->en_rate);

    /* 常发模式下在线配置 */
    if (OAL_SWITCH_ON == pst_dmac_vap->pst_hal_device->bit_al_tx_flag)
    {
        hal_set_tx_dscr_field(pst_dmac_vap->pst_hal_device, pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].ul_value, HAL_RF_TEST_DATA_RATE_ZERO);

        /* TPC根据MCS配置功率描述符 */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
        hal_device_set_pow_al_tx(pst_dmac_vap->pst_hal_device, DMAC_VAP_GET_POW_INFO(pst_dmac_vap),
                            pst_event_set_rate->en_protocol_mode, &(pst_dmac_vap->st_tx_data_mcast));
#endif
    }

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_rate::en_rate=%d,protocol=%d.",
                    pst_event_set_rate->en_rate, pst_event_set_rate->en_protocol_mode);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_mcs(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_tx_comp_stru            *pst_event_set_mcs;
    dmac_vap_stru                   *pst_dmac_vap;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_mcs::pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*默认MF模式*/
    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.bit_preamble_mode = 0;

    pst_event_set_mcs = (mac_cfg_tx_comp_stru *)puc_param;
    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs = pst_event_set_mcs->uc_param;

    hal_set_dac_lpf_gain(pst_dmac_vap->pst_hal_device, pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.en_bandwidth, pst_mac_vap->st_channel.uc_chan_number, pst_event_set_mcs->en_protocol_mode, pst_event_set_mcs->uc_param);

    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = pst_event_set_mcs->en_protocol_mode;

    /* 更新速率及协议模式 */
    pst_dmac_vap->uc_protocol_rate_dscr = (oal_uint8)((pst_event_set_mcs->en_protocol_mode << 6) | pst_event_set_mcs->uc_param);
#ifdef _PRE_WLAN_ONLINE_DPD
    hal_dpd_cfr_set_mcs(pst_dmac_vap->pst_hal_device, pst_event_set_mcs->uc_param);

#endif


    /* 常发模式下在线配置 */
    if (OAL_SWITCH_ON == pst_dmac_vap->pst_hal_device->bit_al_tx_flag)
    {
        hal_set_tx_dscr_field(pst_dmac_vap->pst_hal_device, pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].ul_value, HAL_RF_TEST_DATA_RATE_ZERO);

        /* TPC根据MCS配置功率描述符 */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
        hal_device_set_pow_al_tx(pst_dmac_vap->pst_hal_device, DMAC_VAP_GET_POW_INFO(pst_dmac_vap),
                            pst_event_set_mcs->en_protocol_mode, &(pst_dmac_vap->st_tx_data_mcast));
#endif
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_mcs::tx dscr mcs=%d.", pst_event_set_mcs->uc_param);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_mcsac(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_tx_comp_stru            *pst_event_set_mcsac;
    dmac_vap_stru                   *pst_dmac_vap;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_mcsac::pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }


    /* 设置数据 */
    pst_event_set_mcsac = (mac_cfg_tx_comp_stru *)puc_param;
    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs = pst_event_set_mcsac->uc_param;
    pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = pst_event_set_mcsac->en_protocol_mode;

    /* 更新协议速率 */
    pst_dmac_vap->uc_protocol_rate_dscr = (oal_uint8)((pst_event_set_mcsac->en_protocol_mode << 6) | pst_event_set_mcsac->uc_param);
    hal_set_dac_lpf_gain(pst_dmac_vap->pst_hal_device, pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.en_bandwidth, pst_mac_vap->st_channel.uc_chan_number, pst_event_set_mcsac->en_protocol_mode, pst_event_set_mcsac->uc_param);

    /* 常发模式下在线配置 */
    if (OAL_SWITCH_ON == pst_dmac_vap->pst_hal_device->bit_al_tx_flag)
    {
        hal_set_tx_dscr_field(pst_dmac_vap->pst_hal_device, pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].ul_value, HAL_RF_TEST_DATA_RATE_ZERO);

        /* TPC根据MCS配置功率描述符 */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
        hal_device_set_pow_al_tx(pst_dmac_vap->pst_hal_device, DMAC_VAP_GET_POW_INFO(pst_dmac_vap),
                            pst_event_set_mcsac->en_protocol_mode, &(pst_dmac_vap->st_tx_data_mcast));
#endif
    }

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_mcs::tx dscr mcsac=%d.", pst_event_set_mcsac->uc_param);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX


OAL_STATIC oal_uint32 dmac_config_al_tx_packet_send(mac_vap_stru *pst_vap, oal_netbuf_stru *pst_buf)
{
    oal_uint32 ul_ret;

    ul_ret = dmac_tx_process_data(((dmac_vap_stru *)pst_vap)->pst_hal_device, (dmac_vap_stru *)pst_vap, pst_buf);

    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_al_tx_packet_send::dmac_tx_process_data failed[%d].}", ul_ret);
        dmac_tx_excp_free_netbuf(pst_buf);
        return ul_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32  dmac_config_al_tx_bcast_pkt(mac_vap_stru *pst_mac_vap, oal_uint32 ul_len)
{
    oal_netbuf_stru                *pst_netbuf;
    oal_uint32                      ul_ret;
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1151) && !(defined _PRE_PC_LINT)
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
#endif
    /* 组包 */
    pst_netbuf = dmac_config_create_al_tx_packet(ul_len, (oal_uint8)pst_mac_vap->bit_payload_flag);

    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_bcast_pkt::return null!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1151) && !(defined _PRE_PC_LINT)
    /* 保存常发内存 */
    pst_hal_device->pst_altx_netbuf = pst_netbuf;
#endif

    /* 填写cb和mac头 */
    ul_ret = dmac_config_al_tx_packet(pst_mac_vap, pst_netbuf, ul_len);

    /* 发送数据 */
    ul_ret |= dmac_config_al_tx_packet_send(pst_mac_vap, pst_netbuf);

    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_bcast_pkt::hmac_tx_lan_to_wlan return error %d!}\r\n", ul_ret);
    }

    return ul_ret;
}

OAL_STATIC oal_uint32  dmac_config_stop_always_tx(mac_vap_stru *pst_mac_vap)
{
    hal_to_dmac_device_stru         *pst_hal_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_stop_always_tx::pst_mac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_stop_always_tx::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap->st_cap_flag.bit_keepalive = OAL_TRUE;

    hal_rf_test_disable_al_tx(pst_hal_device);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    hal_config_update_rate_pow_table(pst_hal_device);        /* resume rate_pow_table */
    //dmac_config_update_scaling_reg();        /* resume phy scaling reg */
#endif

    pst_mac_vap->bit_al_tx_flag = OAL_SWITCH_OFF;

    pst_hal_device->ul_al_tx_thr = 0;
    pst_hal_device->bit_al_tx_flag = OAL_SWITCH_OFF;

#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1151) && !(defined _PRE_PC_LINT)
    /* 释放内存 */
    OAL_MEM_MULTI_NETBUF_FREE(pst_hal_device->pst_altx_netbuf);
#endif
    return OAL_SUCC;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_uint32  dmac_config_always_tx_set_hal(hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint                ul_flag = 0;

    /*关闭中断*/
    oal_irq_save(&ul_flag, OAL_5115IRQ_DMSC);

    /* 挂起硬件发送 */
    hal_set_machw_tx_suspend(pst_hal_device);

    /* 停止mac phy接收 */
    hal_disable_machw_phy_and_pa(pst_hal_device);

#ifdef _PRE_WLAN_MAC_BUGFIX_RESET
    /* 复位前延迟50us */
    oal_udelay(50);
#endif

    /*首先清空发送完成事件队列*/
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_TX_COMP);
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_CRX);
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_DRX);

    /* flush硬件5个发送队列 */
    dmac_tx_reset_flush(pst_hal_device);

    /*清除硬件发送缓冲区*/
    hal_clear_hw_fifo(pst_hal_device);

    /*复位macphy*/
    hal_reset_phy_machw(pst_hal_device,
                         HAL_RESET_HW_TYPE_ALL,
                         HAL_RESET_MAC_ALL,
                         OAL_FALSE,
                         OAL_FALSE);

    if (HAL_ALWAYS_TX_DISABLE == pst_hal_device->bit_al_tx_flag)
    {
        hal_rf_test_disable_al_tx(pst_hal_device);

    }
    else
    {
        hal_rf_test_enable_al_tx(pst_hal_device, OAL_PTR_NULL);
    }

    /*避免复位过程中接收描述符队列异常，重新初始化接收描述符队列*/
    dmac_reset_rx_dscr_queue_flush(pst_hal_device);

    /* 恢复 mac phy接收*/
    hal_recover_machw_phy_and_pa(pst_hal_device);

    /* 使能硬件发送 */
    hal_set_machw_tx_resume(pst_hal_device);

    /*使能中断*/
    oal_irq_restore(&ul_flag, OAL_5115IRQ_DMSC);

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  dmac_config_set_always_tx(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_tx_comp_stru            *pst_event_set_al_tx;
    hal_to_dmac_device_stru         *pst_hal_device;
    mac_device_stru                 *pst_device;
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint32                       ul_ret;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8                        uc_pm_off;
#endif

    pst_event_set_al_tx = (mac_cfg_tx_comp_stru *)puc_param;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);

    /* 如果新下发的命令开关状态没变，则直接返回 */
    if (pst_event_set_al_tx->uc_param == pst_mac_vap->bit_al_tx_flag)
    {
        return OAL_SUCC;
    }

    /* 设置常发模式标志 */
    mac_vap_set_al_tx_flag(pst_mac_vap, pst_event_set_al_tx->uc_param);
    mac_vap_set_al_tx_payload_flag(pst_mac_vap, pst_event_set_al_tx->en_payload_flag);
    pst_hal_device->bit_al_tx_flag = pst_event_set_al_tx->uc_param;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    uc_pm_off = 0;
    dmac_config_set_pm_switch(pst_mac_vap, 0, &uc_pm_off);
#else
    dmac_config_always_tx_set_hal(pst_hal_device);
#endif/*(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)*/

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_config_set_always_tx::pst_device[%d] is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    /* 根据长发是否打开配置动态校准功率偏移值*/
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
    hal_set_dyn_cali_pow_offset(pst_dmac_vap->pst_hal_device, pst_hal_device->bit_al_tx_flag);
#endif

    if (HAL_ALWAYS_TX_RF == pst_hal_device->bit_al_tx_flag)
    {
#if defined(_PRE_WLAN_FEATURE_STA_PM)
        if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
        {
            hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_JOIN_COMP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
        }
#endif

        dmac_vap_work_set_channel(pst_dmac_vap);
        pst_mac_vap->st_cap_flag.bit_keepalive = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
        /* 常发打开，关闭弱干扰免疫算法,并将agc unlock门限配置为0x0 */
        ul_ret = dmac_alg_anti_intf_switch(pst_hal_device, OAL_FALSE);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_tx: dmac_alg_anti_intf_switch fail!}");
        }
        hal_set_agc_unlock_min_th(pst_hal_device, 0, 0);
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
        pst_hal_device->en_dfr_enable = OAL_FALSE;
#endif

#ifdef _PRE_WLAN_ONLINE_DPD
        /* "0 6 1" 表示通道0，6执行命令HI1103_DPD_CMD_TOGGLE_SIGNAL，1告知DPD模块长发开启 */
        //hal_dpd_config(pst_hal_device, "0 6 1");
#endif

        ul_ret = dmac_config_al_tx_bcast_pkt(pst_mac_vap, pst_event_set_al_tx->ul_payload_len);
        return ul_ret;
    }
    else if (HAL_ALWAYS_TX_DISABLE == pst_hal_device->bit_al_tx_flag)
    {
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
        /* 常发关闭，打开弱干扰免疫算法 */
        ul_ret = dmac_alg_anti_intf_switch(pst_hal_device, 2);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_tx: dmac_alg_anti_intf_switch fail!}");
        }
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
        /* 打开dfr开关 */
        pst_hal_device->en_dfr_enable = OAL_TRUE;
#endif
        dmac_config_stop_always_tx(pst_mac_vap);

#ifdef _PRE_WLAN_ONLINE_DPD
        /* "0 6 1" 表示通道0，6执行命令HI1103_DPD_CMD_TOGGLE_SIGNAL，0告知DPD模块长发关闭 */
        //hal_dpd_config(pst_hal_device, "0 6 0");
#endif

#if defined(_PRE_WLAN_FEATURE_STA_PM)
        hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_VAP_DOWN, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
#endif
    }
    return OAL_SUCC;
}
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
OAL_STATIC oal_uint32  dmac_is_calipower_chnum_valid(oal_uint8 uc_band, oal_uint8 uc_ch_num)
{
    if(WLAN_BAND_CAP_2G == uc_band)
    {
        if(uc_ch_num < 3)
        {
            return OAL_SUCC;
        }
    }
    else if(WLAN_BAND_CAP_5G == uc_band)
    {
        if(uc_ch_num < 7 && uc_ch_num > 0)
        {
            return OAL_SUCC;
        }
    }
    return OAL_ERR_CODE_INVALID_CONFIG;
}

OAL_STATIC oal_uint32  dmac_config_cali_power(mac_vap_stru *pst_mac_vap,oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_cali_power_stru     *pst_event_set_cali_power;
    dmac_vap_stru               *pst_dmac_vap;
    mac_cfg_show_pd_paras_stru  st_polynomial_paras = {0};
    oal_uint32                  ul_ret;
    mac_device_stru             *pst_mac_device;

    pst_event_set_cali_power = (mac_cfg_cali_power_stru *)puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_cali_power::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(pst_event_set_cali_power->uc_ch != 2)
    {
        ul_ret = dmac_is_calipower_chnum_valid(pst_mac_device->en_band_cap, pst_event_set_cali_power->uc_freq);
        if (OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_cali_power::dmac_is_calipower_chnum_valid[%d] failed[%d].}", pst_event_set_cali_power->uc_freq, ul_ret);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        hal_set_cali_power(pst_dmac_vap->pst_hal_device, pst_event_set_cali_power->uc_ch,
                           pst_event_set_cali_power->uc_freq, pst_event_set_cali_power->as_power, pst_mac_device->en_band_cap, &ul_ret);
        if (OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_cali_power::hal_set_cali_power failed[%d].}", ul_ret);
            return ul_ret;
        }
    }
    else
    {
        hal_get_polynomial_params(pst_dmac_vap->pst_hal_device, st_polynomial_paras.as_polynomial_paras,&st_polynomial_paras.l_length, pst_mac_device->en_band_cap);
        dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CALI_POWER, OAL_SIZEOF(mac_cfg_show_pd_paras_stru), (oal_uint8 *)&st_polynomial_paras);
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32  dmac_config_get_cali_power(mac_vap_stru *pst_mac_vap,oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_cali_power_stru     *pst_event_set_cali_power;
    dmac_vap_stru               *pst_dmac_vap;
    mac_cfg_show_pd_paras_stru  st_polynomial_paras = {0};
    oal_uint32                  ul_ret;
    mac_device_stru             *pst_mac_device;

    pst_event_set_cali_power = (mac_cfg_cali_power_stru *)puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_cali_power::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(pst_event_set_cali_power->uc_ch != 2)
    {
        ul_ret = dmac_is_calipower_chnum_valid(pst_mac_device->en_band_cap, pst_event_set_cali_power->uc_freq);
        if (OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_cali_power::dmac_is_calipower_chnum_valid[%d] failed[%d].}", pst_event_set_cali_power->uc_freq, ul_ret);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        hal_get_cali_power(pst_dmac_vap->pst_hal_device, pst_event_set_cali_power->uc_ch,
                           pst_event_set_cali_power->uc_freq, pst_event_set_cali_power->as_power, pst_mac_device->en_band_cap);
        st_polynomial_paras.l_length = WLAN_DYNC_CALI_POW_PD_PARAM_NUM;
        oal_memcopy(st_polynomial_paras.as_polynomial_paras,
                pst_event_set_cali_power->as_power,
                OAL_SIZEOF(pst_event_set_cali_power->as_power));
    }
    else
    {
        hal_get_all_cali_power(pst_dmac_vap->pst_hal_device, st_polynomial_paras.as_polynomial_paras,&st_polynomial_paras.l_length, pst_mac_device->en_band_cap);

    }
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_CALI_POWER, OAL_SIZEOF(mac_cfg_show_pd_paras_stru), (oal_uint8 *)&st_polynomial_paras);

    return OAL_SUCC;
}

//OAL_STATIC oal_uint32  dmac_config_get_polynomial_params(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
//{

//}

OAL_STATIC oal_uint32  dmac_config_set_polynomial_param(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_polynomial_para_stru    *pst_event_set_polynomial_para;
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint32                      ul_ret;
    mac_device_stru             *pst_mac_device;

    pst_event_set_polynomial_para = (mac_cfg_polynomial_para_stru *)puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_polynomial_param::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    ul_ret = dmac_is_calipower_chnum_valid(pst_mac_device->en_band_cap, pst_event_set_polynomial_para->uc_freq);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_polynomial_param::dmac_is_calipower_chnum_valid[%d] failed[%d].}", pst_event_set_polynomial_para->uc_freq, ul_ret);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    hal_set_polynomial_param(pst_dmac_vap->pst_hal_device,
        pst_event_set_polynomial_para->as_polynomial_para[0], pst_event_set_polynomial_para->uc_freq, pst_mac_device->en_band_cap);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_get_upc_params(mac_vap_stru *pst_mac_vap,oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru               *pst_dmac_vap;
    mac_cfg_show_upc_paras_stru  st_upc_paras = {0};
    mac_device_stru             *pst_mac_device;
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_cali_power::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }


    hal_get_upc_params(pst_dmac_vap->pst_hal_device, st_upc_paras.aus_upc_paras,&st_upc_paras.ul_length, pst_mac_device->en_band_cap);
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_UPC_PARA, OAL_SIZEOF(mac_cfg_show_upc_paras_stru), (oal_uint8 *)&st_upc_paras);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_upc_params(mac_vap_stru *pst_mac_vap,oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_show_upc_paras_stru     *pst_event_set_upc_para;
    dmac_vap_stru                   *pst_dmac_vap;
    mac_device_stru                 *pst_mac_device;
    oal_uint8                       uc_cali_type = 5;  //5对应是51 hal中HI1151V2_CALI_TYPE_TX_PWR宏定义

    pst_event_set_upc_para = (mac_cfg_show_upc_paras_stru *)puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_polynomial_param::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_set_upc_params(pst_dmac_vap->pst_hal_device, pst_event_set_upc_para->aus_upc_paras, pst_mac_device->en_band_cap);

    /* 触发重新校准 */
    dmac_config_auto_cali(pst_mac_vap, uc_len, &uc_cali_type);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_load_mode(mac_vap_stru *pst_mac_vap,oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_polynomial_param::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_set_dyn_cali_pow_offset(pst_dmac_vap->pst_hal_device, *puc_param);

    return OAL_SUCC;
}

#endif


OAL_STATIC oal_uint32  dmac_config_get_dieid(mac_vap_stru *pst_mac_vap,oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru               *pst_dmac_vap;
    mac_cfg_show_dieid_stru     st_dieid = {0};

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_cali_power::pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_get_dieid(pst_dmac_vap->pst_hal_device, st_dieid.aul_dieid, &st_dieid.ul_length);

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_DIEID, OAL_SIZEOF(mac_cfg_show_dieid_stru), (oal_uint8 *)&st_dieid);

    return OAL_SUCC;
}
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC oal_uint32  dmac_config_set_always_tx_51(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_tx_comp_stru            *pst_event_set_al_tx;
    hal_to_dmac_device_stru         *pst_hal_device;
    mac_device_stru                 *pst_device;
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint32                       ul_ret;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8                        uc_pm_off;
#endif
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                        uc_hipriv_ack = OAL_FALSE;
#endif

    pst_event_set_al_tx = (mac_cfg_tx_comp_stru *)puc_param;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);

    /* 如果新下发的命令开关状态没变，则直接返回 */
    if ( pst_event_set_al_tx->uc_param == pst_mac_vap->bit_al_tx_flag )
    {
        /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
        uc_hipriv_ack = OAL_TRUE;
        dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif
        return OAL_SUCC;
    }

    /* 设置常发模式标志 */
    mac_vap_set_al_tx_flag(pst_mac_vap, pst_event_set_al_tx->uc_param);
    mac_vap_set_al_tx_payload_flag(pst_mac_vap, pst_event_set_al_tx->en_payload_flag);
    pst_hal_device->bit_al_tx_flag = pst_event_set_al_tx->uc_param;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    uc_pm_off = 0;
    dmac_config_set_pm_switch(pst_mac_vap, 0, &uc_pm_off);
#if defined(_PRE_WLAN_FEATURE_STA_PM)
    dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_ENABLE_FRONT_END, 0, OAL_PTR_NULL);
#endif
#else
    dmac_config_always_tx_set_hal(pst_hal_device);
#endif/*(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)*/

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_config_set_always_tx::pst_device[%d] is NULL!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (HAL_ALWAYS_TX_RF == pst_hal_device->bit_al_tx_flag)
    {
        dmac_vap_work_set_channel(pst_dmac_vap);
        pst_mac_vap->st_cap_flag.bit_keepalive = OAL_FALSE;
        /* hal_rf_test_enable_al_tx寄存器和描述符在dmac_tx_data中完成  */

        /* 更新TPC code表单并配置功率描述符 */
        ul_ret = dmac_alg_cfg_channel_notify(pst_mac_vap, CH_BW_CHG_TYPE_REFRESH);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_tx:dmac_config_al_tx_set_pow fail!}");
            return ul_ret;
        }

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
        /* 常发打开，关闭弱干扰免疫算法,并将agc unlock门限配置为0x0 */
        ul_ret = dmac_alg_anti_intf_switch(pst_hal_device, OAL_FALSE);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_tx: dmac_alg_anti_intf_switch fail!}");
        }
        hal_set_agc_unlock_min_th(pst_hal_device, 0, 0);
#endif

#ifdef _PRE_WLAN_ONLINE_DPD
        /* "0 6 1" 表示通道0，6执行命令HI1103_DPD_CMD_TOGGLE_SIGNAL，1告知DPD模块长发开启 */
        hal_dpd_config(pst_hal_device, "0 6 1");
#endif
        return dmac_config_al_tx_bcast_pkt(pst_mac_vap, pst_event_set_al_tx->ul_payload_len);

    }
    else if (HAL_ALWAYS_TX_DISABLE == pst_hal_device->bit_al_tx_flag)
    {
        hal_tx_dscr_stru                *pst_base_dscr;
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_hal_device))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_stop_always_tx::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        pst_mac_vap->st_cap_flag.bit_keepalive = OAL_TRUE;
        /* 停止常发时将TX描述符的NEXT清空*/
        pst_base_dscr = OAL_DLIST_GET_ENTRY(pst_dmac_vap->pst_hal_device->ast_tx_dscr_queue[WLAN_WME_AC_VO].st_header.pst_next, hal_tx_dscr_stru, st_entry);
        hal_tx_ctrl_dscr_unlink(pst_dmac_vap->pst_hal_device, pst_base_dscr);

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
        hal_config_update_rate_pow_table(pst_hal_device);        /* resume rate_pow_table */
        //dmac_config_update_scaling_reg();                    /* resume phy scaling reg */
#endif

        pst_mac_vap->bit_al_tx_flag = OAL_SWITCH_OFF;

        pst_dmac_vap->pst_hal_device->ul_al_tx_thr = 0;
        pst_dmac_vap->pst_hal_device->bit_al_tx_flag = OAL_SWITCH_OFF;
    }

    /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif
    return OAL_SUCC;
}
#endif


oal_uint32  dmac_al_tx_complete_event_handler(frw_event_mem_stru *pst_event_mem)
{
    hal_to_dmac_device_stru         *pst_hal_device;
    frw_event_stru                  *pst_event;
    hal_tx_complete_event_stru      *pst_tx_comp_event;
    mac_device_stru                 *pst_mac_device;
    mac_vap_stru                    *pst_mac_vap;
    oal_uint8                        uc_hipriv_ack;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_al_tx_complete_event_handler::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 从事件结构体中获取硬件上报的描述符地址和发送描述符个数 */
    pst_event         = frw_get_event_stru(pst_event_mem);
    pst_tx_comp_event = (hal_tx_complete_event_stru *)pst_event->auc_event_data;
    pst_hal_device    = pst_tx_comp_event->pst_hal_device;

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_TX, "{dmac_al_tx_complete_event_handler::pst_mac_device null. device_id=%d.}", pst_hal_device->uc_mac_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    //此处有问题吗?
    pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[0]);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_device->auc_vap_id[0], OAM_SF_TX, "{dmac_al_tx_complete_event_handler::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dmac_config_stop_always_tx(pst_mac_vap);

    dmac_tx_complete_free_dscr(pst_tx_comp_event->pst_base_dscr);

    /* 通知hmac关闭常发 */
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, HAL_TX_COMP_SUB_TYPE_AL_TX, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
    return OAL_SUCC;

}


oal_uint32  dmac_config_set_always_tx_hw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_al_tx_hw_stru           *pst_al_tx_hw;
    dmac_vap_stru                   *pst_dmac_vap;
    hal_al_tx_hw_stru                st_hal_al_tx_hw;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8                        uc_pm_off;
#endif

    pst_al_tx_hw = (mac_cfg_al_tx_hw_stru *)puc_param;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_tx::pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 设置常发模式标志 */
    /* 重复配置，直接返回 */
    if (pst_al_tx_hw->bit_switch == pst_dmac_vap->pst_hal_device->uc_al_tx_hw)
    {
        return OAL_SUCC;
    }

    pst_dmac_vap->pst_hal_device->uc_al_tx_hw = pst_al_tx_hw->bit_switch;
    if (0 == pst_al_tx_hw->bit_switch)
    {
        hal_disable_machw_phy_and_pa(pst_dmac_vap->pst_hal_device);
        return OAL_SUCC;
    }
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    uc_pm_off = 0;
    dmac_config_set_pm_switch(pst_mac_vap, 0, &uc_pm_off);
#endif

#if defined(_PRE_WLAN_FEATURE_STA_PM)
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_JOIN_COMP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
    }
    hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_VAP_UP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
#endif

    dmac_vap_work_set_channel(pst_dmac_vap);

    st_hal_al_tx_hw.un_ctrl.ul_ctrl = 0;
    st_hal_al_tx_hw.un_ctrl.st_ctrl.bit_en      = pst_al_tx_hw->bit_switch;
    st_hal_al_tx_hw.un_ctrl.st_ctrl.bit_mode    = pst_al_tx_hw->bit_flag;
    st_hal_al_tx_hw.un_ctrl.st_ctrl.bit_content = pst_al_tx_hw->uc_content;
    st_hal_al_tx_hw.ul_mpdu_len = pst_al_tx_hw->ul_len;
    st_hal_al_tx_hw.ul_times = pst_al_tx_hw->ul_times;
    st_hal_al_tx_hw.ul_ifs = pst_al_tx_hw->ul_ifs;
    /* 开启常发前可通过rate或mcs命令配置硬件常发速率 */
    st_hal_al_tx_hw.pst_rate = &pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0];

    hal_al_tx_hw(pst_dmac_vap->pst_hal_device, &st_hal_al_tx_hw);
    hal_set_machw_tx_resume(pst_dmac_vap->pst_hal_device);
    return OAL_SUCC;
}

#endif /* #ifdef _PRE_WLAN_FEATURE_ALWAYS_TX */

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

oal_void dmac_pow_cfg_show_log_work(oal_work_stru *pst_work)
{
    mac_vap_stru        *pst_mac_vap;
    mac_device_stru     *pst_mac_device;
    dmac_device_stru    *pst_dmac_device;

    pst_mac_vap = OAL_CONTAINER_OF(pst_work, mac_vap_stru, show_pow_work);
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_pow_cfg_show_log_work::pst_mac_device is null}");
        return;
    }
    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if(OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_pow_cfg_show_log_work::pst_dmac_device is null}");
        return;
    }
    hal_pow_cfg_show_log(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device), DMAC_VAP_GET_POW_INFO(pst_mac_vap), g_en_pow_freq_band, 0);
}
#endif



OAL_STATIC oal_uint32  dmac_config_set_tx_pow(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifndef WIN32
    mac_cfg_set_tx_pow_param_stru      *pst_event_set_tx_pow;
    hal_to_dmac_device_stru            *pst_hal_device;
    wlan_channel_band_enum_uint8        en_freq_band;
    oal_uint8                           uc_cur_ch_num;
    wlan_channel_bandwidth_enum_uint8   en_bandwidth;

    pst_event_set_tx_pow = (mac_cfg_set_tx_pow_param_stru *)puc_param;
    pst_hal_device       = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);

    en_freq_band  = pst_mac_vap->st_channel.en_band;
    uc_cur_ch_num = pst_mac_vap->st_channel.uc_chan_number;
    en_bandwidth  = pst_mac_vap->st_channel.en_bandwidth;

    switch (pst_event_set_tx_pow->en_type)
    {
        case MAC_SET_POW_RF_REG_CTL:
            /*lint -save -e730 */
            hal_pow_set_rf_regctl_enable(pst_hal_device, !!(pst_event_set_tx_pow->auc_value[0]));
            /*lint -restore */
            if (OAL_SWITCH_OFF == pst_hal_device->bit_al_tx_flag)
            {
                hal_rf_regctl_enable_set_regs(pst_hal_device, en_freq_band, uc_cur_ch_num, en_bandwidth);
            }
            break;

        case MAC_SET_POW_FIX_LEVEL:
            pst_hal_device->uc_fix_power_level = pst_event_set_tx_pow->auc_value[0];
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_tx_pow::uc_fix_power_level = %d}", pst_event_set_tx_pow->auc_value);//lint !e571
            break;

        case MAC_SET_POW_MAG_LEVEL:
            pst_hal_device->uc_mag_mcast_frm_power_level = pst_event_set_tx_pow->auc_value[0];
            dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_MAG_LVL_CHANGE);
            break;

        case MAC_SET_POW_CTL_LEVEL:
            pst_hal_device->uc_control_frm_power_level = pst_event_set_tx_pow->auc_value[0];
            dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_CTL_LVL_CHANGE);
            break;

        case MAC_SET_POW_AMEND:
        #ifndef _PRE_WLAN_FEATURE_TPC_OPT
            pst_hal_device->s_upc_amend += pst_event_set_tx_pow->auc_value[0];
            hal_device_amend_upc_code(pst_hal_device, en_freq_band, uc_cur_ch_num, en_bandwidth);
        #else
            hal_device_update_upc_amend(pst_hal_device, pst_event_set_tx_pow->auc_value[0]);
        #endif
            dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_REFRESH);
            break;

        case MAC_SET_POW_NO_MARGIN:
            hal_pow_cfg_no_margin_pow_mode(pst_hal_device, (oal_uint8)pst_event_set_tx_pow->auc_value[0]);
            dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_INIT);
            break;

        case MAC_SET_POW_SHOW_LOG:
        #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            g_en_pow_freq_band = en_freq_band;
            OAL_INIT_WORK(&pst_mac_vap->show_pow_work, dmac_pow_cfg_show_log_work);
            oal_workqueue_schedule(&(pst_mac_vap->show_pow_work));
        #else
            hal_pow_cfg_show_log(pst_hal_device, DMAC_VAP_GET_POW_INFO(pst_mac_vap), en_freq_band, pst_event_set_tx_pow->auc_value[0]);
        #endif
            break;

        case MAC_SET_POW_SAR_LVL_DEBUG:
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_TPC, "{dmac_config_set_tx_pow::MAC_SET_POW_SAR_LVL_DEBUG}");
        #ifdef _PRE_WLAN_FEATURE_TPC_OPT
            dmac_config_reduce_sar(pst_mac_vap, 0, pst_event_set_tx_pow->auc_value);
        #endif
            break;

        default:
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_tx_pow::pst_event_set_tx_pow->en_type = %u, invalid}", pst_event_set_tx_pow->en_type);
            return OAL_FAIL;
    }
#endif

    return OAL_SUCC;

}

#ifdef _PRE_WLAN_NARROW_BAND

OAL_STATIC oal_uint32  dmac_config_set_narrow_bw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    mac_cfg_narrow_bw_stru          *pst_nrw_bw;
    hal_to_dmac_device_stru         *pst_hal_device;

    pst_nrw_bw = (mac_cfg_narrow_bw_stru *)puc_param;



    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_narrow_bw::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 重复配置，直接返回 */
    if (pst_nrw_bw->en_open == pst_hal_device->en_narrow_bw_open
    && pst_nrw_bw->en_bw  == pst_hal_device->uc_narrow_bw)
    {
        return OAL_SUCC;
    }

    pst_mac_vap->st_nb = *pst_nrw_bw;

    pst_hal_device->en_narrow_bw_open = pst_nrw_bw->en_open;
    pst_hal_device->uc_narrow_bw = pst_nrw_bw->en_bw;

    return OAL_SUCC;
}


#endif /* #ifdef _PRE_WLAN_NARROW_BAND */


#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)

OAL_STATIC oal_uint32  dmac_config_set_cali_vref(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_set_cali_vref_stru      *pst_cali_vref;

    /* 设置数据 */
    pst_cali_vref = (mac_cfg_set_cali_vref_stru *)puc_param;
    if (OAL_PTR_NULL == pst_cali_vref)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_cali_vref::pst_cali_vref null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_rf_cali_set_vref(pst_mac_vap->st_channel.en_band,pst_cali_vref->uc_chain_idx,
                            pst_cali_vref->uc_band_idx, pst_cali_vref->us_vref_value);

    return OAL_SUCC;

}
#endif


oal_uint32  dmac_config_alrxtx_set_pm(mac_vap_stru *pst_mac_vap, oal_uint32 ul_switch)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_bool_enum_uint8              en_pm_switch;

    en_pm_switch = ((OAL_SWITCH_OFF == ul_switch) ? OAL_TRUE : OAL_FALSE);
    dmac_config_set_pm_switch(pst_mac_vap, 0, &en_pm_switch);
#endif

    return OAL_SUCC;
}


oal_uint32  dmac_config_always_rx_set_hal(hal_to_dmac_device_stru *pst_hal_device_base, oal_uint8 uc_switch)
{
    oal_uint                ul_flag = 0;

    /*关闭中断*/
    oal_irq_save(&ul_flag, OAL_5115IRQ_DMSC);

    /* 挂起硬件发送 */
    hal_set_machw_tx_suspend(pst_hal_device_base);

    /* 停止mac phy接收 */
    hal_disable_machw_phy_and_pa(pst_hal_device_base);

#ifdef _PRE_WLAN_MAC_BUGFIX_RESET
    /* 复位前延迟50us */
    oal_udelay(50);
#endif

    /*首先清空发送完成事件队列*/
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_TX_COMP);
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_CRX);
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_DRX);

    /* flush硬件5个发送队列 */
    dmac_tx_reset_flush(pst_hal_device_base);

    /*清除硬件发送缓冲区*/
    hal_clear_hw_fifo(pst_hal_device_base);

    hal_psm_clear_mac_rx_isr(pst_hal_device_base);

    /*复位macphy*/
    hal_reset_phy_machw(pst_hal_device_base,
                        HAL_RESET_HW_TYPE_MAC_PHY,
                        HAL_RESET_MAC_ALL,
                        OAL_FALSE,
                        OAL_FALSE);

    /*置标识bit*/
    pst_hal_device_base->bit_al_rx_flag = uc_switch;

    /*避免复位过程中接收描述符队列异常，重新初始化接收描述符队列*/
    //dmac_reset_rx_dscr_queue_flush(pst_hal_device_base);
    if (uc_switch)
    {
        /* 释放所有正常接收描述符 */
        hal_rx_destroy_dscr_queue(pst_hal_device_base);

        /* 初始化常收接收描述符队列 */
        hal_al_rx_init_dscr_queue(pst_hal_device_base);
    }
    else
    {
        /* 释放所有常收接收描述符队列 */
        hal_al_rx_destroy_dscr_queue(pst_hal_device_base);

        /* 初始化正常描述符队列 */
        hal_rx_init_dscr_queue(pst_hal_device_base,OAL_TRUE);
    }

    /*使能中断*/
    oal_irq_restore(&ul_flag, OAL_5115IRQ_DMSC);

    /* 恢复 mac phy接收*/
    hal_recover_machw_phy_and_pa(pst_hal_device_base);

    /* 使能硬件发送 */
    hal_set_machw_tx_resume(pst_hal_device_base);

    /*配置寄存器*/
    hal_config_always_rx(pst_hal_device_base, uc_switch);

    return OAL_SUCC;
}

oal_uint32  dmac_config_set_always_rx(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;
    hal_to_dmac_device_stru         *pst_hal_device_base;
#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
    mac_device_stru                 *pst_mac_device;
#endif //_PRE_WLAN_FEATURE_ANTI_INTERF
    oal_uint8                        uc_al_rx_flag = 0;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);


    pst_hal_device_base = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device_base))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_rx::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 聚合帧个数更新，即使常收指令连续两次相同也要更新，以使常发生效 */
    pst_hal_device_base->bit_al_txrx_ampdu_num = g_ul_al_ampdu_num;

    /* 如果新下发的命令开关状态没变，则直接返回 */
    if(*(oal_bool_enum_uint8 *)puc_param == pst_hal_device_base->bit_al_rx_flag)
    {
        return OAL_SUCC;
    }

    /* 设置常收模式标志 */
    uc_al_rx_flag = *(oal_bool_enum_uint8 *)puc_param;
    pst_mac_vap->bit_al_rx_flag = uc_al_rx_flag;

    dmac_config_alrxtx_set_pm(pst_mac_vap, uc_al_rx_flag);

#ifdef _PRE_WLAN_FEATURE_STA_PM
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_JOIN_COMP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
    }
    hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_VAP_UP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
#endif  /* _PRE_WLAN_FEATURE_STA_PM */

    dmac_vap_work_set_channel(pst_dmac_vap);

    if ((HAL_ALWAYS_RX_RESERVED == uc_al_rx_flag)
    || (HAL_ALWAYS_RX_DISABLE == uc_al_rx_flag))
    {
        dmac_config_always_rx_set_hal(pst_hal_device_base, uc_al_rx_flag);
    }

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_config_set_always_rx::pst_mac_device[%d] is NULL!}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (HAL_ALWAYS_RX_RESERVED == uc_al_rx_flag)
    {
        /* 常收，关闭弱干扰免疫算法 */
        dmac_alg_anti_intf_switch(pst_dmac_vap->pst_hal_device, OAL_FALSE);

    }
    else
    {
        /* 非常收模式，打开弱干扰免疫算法 */
        dmac_alg_anti_intf_switch(pst_dmac_vap->pst_hal_device, 2);
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_voe_enable(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint8                        uc_switch;

    uc_switch = puc_param[0];

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    //pst_dmac_vap->bit_voe_enable = uc_switch;
#ifdef _PRE_WLAN_FEATURE_11K
    pst_dmac_vap->bit_11k_enable = ((uc_switch & 0x07) & BIT2) ? OAL_TRUE : OAL_FALSE;
    pst_dmac_vap->bit_11v_enable = ((uc_switch & 0x07) & BIT1) ? OAL_TRUE : OAL_FALSE;
#endif
#ifdef _PRE_WLAN_FEATURE_11R
    pst_dmac_vap->bit_11r_enable = ((uc_switch & 0x07) & BIT0) ? OAL_TRUE : OAL_FALSE;
#endif
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_voe_enable:para val[%d]!}", uc_switch);
    return OAL_SUCC;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC oal_uint32  dmac_config_always_rx_set_hal_51(hal_to_dmac_device_stru *pst_hal_device_base, oal_uint8 uc_switch)
{
    oal_uint    ul_flag = 0;

    /*关闭中断*/
    oal_irq_save(&ul_flag, OAL_5115IRQ_DMSC);

    /* 挂起硬件发送 */
    hal_set_machw_tx_suspend(pst_hal_device_base);

    /* 停止mac phy接收 */
    hal_disable_machw_phy_and_pa(pst_hal_device_base);

    /*首先清空发送完成事件队列*/
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_TX_COMP);
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_CRX);
    frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_DRX);

    /* flush硬件5个发送队列 */
    dmac_tx_reset_flush(pst_hal_device_base);

    /*清除硬件发送缓冲区*/
    hal_clear_hw_fifo(pst_hal_device_base);

    /*复位macphy*/
    hal_reset_phy_machw(pst_hal_device_base,
                        HAL_RESET_HW_TYPE_MAC_PHY,
                        HAL_RESET_MAC_ALL,
                        OAL_FALSE,
                        OAL_FALSE);

    /*置标识bit*/
    pst_hal_device_base->bit_al_rx_flag = uc_switch;

    /*避免复位过程中接收描述符队列异常，重新初始化接收描述符队列*/
    //dmac_reset_rx_dscr_queue_flush(pst_hal_device_base);
    if(uc_switch)
    {
        /* 释放所有正常接收描述符 */
        hal_rx_destroy_dscr_queue(pst_hal_device_base);

        /* 初始化常收接收描述符队列 */
        hal_al_rx_init_dscr_queue(pst_hal_device_base);
    }
    else
    {
        /* 释放所有常收接收描述符队列 */
        hal_al_rx_destroy_dscr_queue(pst_hal_device_base);

        /* 初始化正常描述符队列 */
        hal_rx_init_dscr_queue(pst_hal_device_base,OAL_TRUE);
    }

    /*使能中断*/
    oal_irq_restore(&ul_flag, OAL_5115IRQ_DMSC);

    /* 恢复 mac phy接收*/
    hal_enable_machw_phy_and_pa(pst_hal_device_base);

    /* 使能硬件发送 */
    hal_set_machw_tx_resume(pst_hal_device_base);

    /*配置寄存器*/
    hal_config_always_rx_new(pst_hal_device_base, uc_switch);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_set_always_rx_51(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;
    hal_to_dmac_device_stru         *pst_hal_device_base;
    //mac_device_stru                 *pst_mac_device;
    oal_uint8                        uc_al_rx_flag = 0;
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                        uc_hipriv_ack = OAL_FALSE;
#endif

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_rx::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device_base = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device_base))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_rx::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 聚合帧个数更新，即使常收指令连续两次相同也要更新，以使常发生效 */
    pst_hal_device_base->bit_al_txrx_ampdu_num = g_ul_al_ampdu_num;

    /* 如果新下发的命令开关状态没变，则直接返回 */
    if(*(oal_bool_enum_uint8 *)puc_param == pst_hal_device_base->bit_al_rx_flag)
    {
    /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
        uc_hipriv_ack = OAL_TRUE;
        dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif
        return OAL_SUCC;
    }

    /* 设置常收模式标志 */
    uc_al_rx_flag = *(oal_bool_enum_uint8 *)puc_param;

    dmac_config_alrxtx_set_pm(pst_mac_vap, uc_al_rx_flag);

    //pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
#ifdef _PRE_WLAN_FEATURE_STA_PM
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_JOIN_COMP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
    }
    hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_VAP_UP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_config_set_always_rx::pst_dmac_vap->pst_hal_device[%d] is NULL!}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hal_pm_enable_front_end(pst_dmac_vap->pst_hal_device, OAL_TRUE);
#endif  /* _PRE_WLAN_FEATURE_STA_PM */
#endif

    dmac_vap_work_set_channel(pst_dmac_vap);

    if ((HAL_ALWAYS_RX_RESERVED == uc_al_rx_flag)
        || (HAL_ALWAYS_RX_DISABLE == uc_al_rx_flag))
    {
        dmac_config_always_rx_set_hal_51(pst_hal_device_base, uc_al_rx_flag);
    }

#ifdef _PRE_WLAN_FEATURE_ANTI_INTERF
    if (HAL_ALWAYS_RX_RESERVED == uc_al_rx_flag)
    {
        /* 常收，关闭弱干扰免疫算法 */
        dmac_alg_anti_intf_switch(pst_dmac_vap->pst_hal_device, OAL_FALSE);
    }
    else
    {
        /* 非常收模式，打开弱干扰免疫算法 */
        //dmac_alg_anti_intf_switch(pst_mac_device, 2);
    }
#endif

    /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif

    return OAL_SUCC;
}
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
oal_void dmac_config_reg_report(mac_vap_stru *pst_mac_vap, oal_uint32 ul_addr)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint32                      ul_value;

    if (ul_addr % 4 != 0)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_reg_report::not mod 4");
        return;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_reg_report::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return;
    }

    hal_reg_info(pst_hal_device, ul_addr, &ul_value);

    OAM_WARNING_LOG2(0, OAM_SF_CFG, "dmac_config_reg_report reg addr=0x%08x, value=0x%08x\n", ul_addr, ul_value);

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_REG_INFO, OAL_SIZEOF(ul_value), (oal_uint8 *)&ul_value);
}
#endif

OAL_STATIC oal_uint32  dmac_config_pcie_pm_level(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_cfg_pcie_pm_level_stru      *pst_pcie_pm_level;
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_pcie_pm_level::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 参数判断 */
    pst_pcie_pm_level = (mac_cfg_pcie_pm_level_stru *)puc_param;

    hal_set_pcie_pm_level(pst_hal_device, pst_pcie_pm_level->uc_pcie_pm_level);
#endif

    return OAL_SUCC;
}
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))

oal_uint32  dmac_config_sdio_flowctrl(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int8             *pc_token;
    oal_int8             *pc_end;
    oal_int8             *pc_ctx;               /* 用于记录本次截断后的字符串首地址 */
    oal_int8             *pc_sep = " ";         /* 截断字符列表:这里以空格截断 */
    oal_uint16            us_UsedMemForStop;
    oal_uint16            us_UsedMemForstart;

    /* 获取流控关闭水线 */
    /* strtok首次调用第一个参数需传入源字符串 */
    pc_token = oal_strtok((oal_int8 *)puc_param, pc_sep, &pc_ctx);
    if (NULL == pc_token)
    {
        return OAL_FAIL;
    }

    us_UsedMemForstart = (oal_uint16)oal_strtol(pc_token, &pc_end, 10);

    /* 获取流控开启水线 */
    /* 后续strtok调用首参需传入空指针 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (NULL == pc_token)
    {
        return OAL_FAIL;
    }
#ifdef CONFIG_SDIO_MSG_FLOWCTRL
    us_UsedMemForStop = (oal_uint16)oal_strtol(pc_token, &pc_end, 10);
    /* cfg_hisi.ini参数有效性判断 */
    g_usUsedMemForstart  = (us_UsedMemForstart >= 1 && us_UsedMemForstart <= OAL_SDIO_FLOWCTRL_MAX) ? us_UsedMemForstart : g_usUsedMemForstart;
    g_usUsedMemForStop  = (us_UsedMemForStop >= 1 && us_UsedMemForStop <= OAL_SDIO_FLOWCTRL_MAX) ? us_UsedMemForStop : g_usUsedMemForStop;

    OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_config_sdio_flowctrl::UsedMemForStop=%d, UsedMemForstart=%d.", us_UsedMemForStop, us_UsedMemForstart);
#endif
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_ONLINE_DPD

OAL_STATIC oal_uint32 dmac_config_dpd(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dpd::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_dpd_config(pst_hal_device, puc_param);

    return OAL_SUCC;
}

#endif
#ifdef _PRE_WLAN_FEATURE_SMARTANT
oal_uint32 gul_dual_antenna_enable = 0;

oal_uint32 dmac_dual_antenna_register_dmac_misc_event(hal_dmac_misc_sub_type_enum en_event_type, oal_uint32 (*p_func)(frw_event_mem_stru *))
{
    if(en_event_type >= HAL_EVENT_DMAC_MISC_SUB_TYPE_BUTT || NULL == p_func)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMART_ANTENNA, "dmac_alg_register_dmac_misc_event fail");
        return  OAL_FAIL;
    }
    g_ast_dmac_misc_event_sub_table[en_event_type].p_func = p_func;
    return OAL_SUCC;
}



oal_uint32  dmac_dual_antenna_unregister_dmac_misc_event(hal_dmac_misc_sub_type_enum en_event_type)
{
    if(en_event_type >= HAL_EVENT_DMAC_MISC_SUB_TYPE_BUTT)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMART_ANTENNA, "dmac_alg_unregister_dmac_misc_event fail");
        return  OAL_FAIL;
    }
    g_ast_dmac_misc_event_sub_table[en_event_type].p_func = NULL;
    return OAL_SUCC;
}


oal_uint32 dmac_dual_antenna_notify_alg(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    hal_to_dmac_device_stru    *pst_hal_device   = OAL_PTR_NULL;
    oal_uint32                 *pul_status;
    oal_uint8                   uc_hal_device_id = 0;
    oal_uint8                   uc_chip_id       = 0;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMART_ANTENNA, "{dmac_dual_antenna_notify_alg::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    //从 hi1102_dual_antenna_post_event 抛过来的事件中传到事件头的device id即 hal device id, 因此可以直接用
    pst_event = frw_get_event_stru(pst_event_mem);

    uc_chip_id       = pst_event->st_event_hdr.uc_chip_id;
    uc_hal_device_id = pst_event->st_event_hdr.uc_device_id;   //注意此处的是hal device id

    hal_get_hal_to_dmac_device(uc_chip_id, uc_hal_device_id, &pst_hal_device);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMART_ANTENNA, "{dmac_dual_antenna_notify_alg::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#if 1
    pul_status = (oal_uint32 *)pst_event->auc_event_data;
    OAM_WARNING_LOG1(0, OAM_SF_SMART_ANTENNA, "notify alg, status:%d.", *pul_status);
    dmac_alg_cfg_dual_antenna_state_notify(pst_hal_device, *pul_status);
#endif
    return OAL_SUCC;
}


oal_uint32 dmac_config_dual_antenna_vap_check(mac_vap_stru *pst_mac_vap)
{
    hal_to_dmac_chip_stru                   *pst_hal_chip;
    hal_to_dmac_device_stru                 *pst_hal_device;
    mac_device_stru                         *pst_mac_device;
    mac_vap_stru                            *pst_mac_vap_temp = OAL_PTR_NULL;
    hal_dual_antenna_check_status_stru      *pst_dual_antenna_check_status;
    oal_uint8                                uc_vap_idx;
    oal_uint32                               ul_result;
    oal_uint8                                uc_no_vap = 1;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMART_ANTENNA, "{dmac_config_dual_antenna_vap_check::pst_mac_device null.}");
        return DUAL_ANTENNA_ALG_CLOSE;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMART_ANTENNA, "{dmac_config_dual_antenna_vap_check:: DMAC_VAP_GET_HAL_DEVICE null}");
        return DUAL_ANTENNA_ALG_CLOSE;
    }

    pst_hal_chip = DMAC_VAP_GET_HAL_CHIP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_chip)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMART_ANTENNA, "{dmac_config_dual_antenna_vap_check:: DMAC_VAP_GET_HAL_CHIP null}");
        return DUAL_ANTENNA_ALG_CLOSE;
    }

    pst_dual_antenna_check_status = &(pst_hal_chip->st_dual_antenna_check_status);

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap_temp = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap_temp))
        {
            OAM_WARNING_LOG1(0, OAM_SF_SMART_ANTENNA, "{dmac_config_dual_antenna_vap_check::vap is null! vap id is %d}", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        uc_no_vap = 0;

        if ((WLAN_VAP_MODE_BSS_AP == pst_mac_vap_temp->en_vap_mode)
            || (WLAN_P2P_CL_MODE == pst_mac_vap_temp->en_p2p_mode))
        {
            pst_dual_antenna_check_status->bit_vap_mode = 1;
        }
        else if ((WLAN_LEGACY_VAP_MODE == pst_mac_vap_temp->en_p2p_mode)
                && (WLAN_BAND_5G == pst_mac_vap_temp->st_channel.en_band))
        {
            pst_dual_antenna_check_status->bit_vap_mode = 1;
        }
        else
        {
            pst_dual_antenna_check_status->bit_vap_mode = 0;
        }

        if (pst_dual_antenna_check_status->bit_vap_mode)
        {
            OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_SMART_ANTENNA,
                                "{dmac_config_dual_antenna_vap_mode_check::dual_antenna on 1, vap_mode: %d, p2p_mode: %d, channel: %d.}",
                                pst_mac_vap_temp->en_vap_mode, pst_mac_vap_temp->en_p2p_mode, pst_mac_vap_temp->st_channel.en_band);
            break;
        }
    }

    pst_dual_antenna_check_status->bit_vap_mode |= uc_no_vap;

    hal_dual_antenna_switch(pst_hal_device, pst_dual_antenna_check_status->bit_vap_mode, 0, &ul_result);

    return DUAL_ANTENNA_AVAILABLE;
}


oal_uint32 dmac_config_dual_antenna_check(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                      ul_ret;
    hal_to_dmac_device_stru        *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dual_antenna_check::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    hal_dual_antenna_check(pst_hal_device, &ul_ret);
    return ul_ret;
}


oal_uint32 dmac_dual_antenna_set_ant(oal_uint8 uc_param)
{
    oal_uint32           ul_result = DUAL_ANTENNA_ALG_CLOSE;
    dmac_device_stru    *pst_dmac_device;
    pst_dmac_device = dmac_res_get_mac_dev(0);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMART_ANTENNA, "{dmac_dual_antenna_set_ant::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_dual_antenna_switch(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device), uc_param, 1, &ul_result);
    OAM_WARNING_LOG2(0, OAM_SF_SMART_ANTENNA, "{dmac_dual_antenna_set_ant::dual_antenna set to %d, result:%d.}",
                    uc_param, ul_result);
    return ul_result;
}

oal_uint32 dmac_dual_antenna_set_ant_at(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_param)
{
    oal_uint32 ul_result = DUAL_ANTENNA_ALG_CLOSE;
    hal_dual_antenna_switch_at(pst_hal_device, uc_param, &ul_result);
    OAM_WARNING_LOG2(0, OAM_SF_SMART_ANTENNA, "{dmac_dual_antenna_set_ant_at::dual_antenna set to %d, result:%d.}",
                    uc_param, ul_result);
    return ul_result;
}


oal_uint32 dmac_config_dual_antenna_set_ant(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru     *pst_hal_device;

    if(OAL_PTR_NULL == puc_param)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SMART_ANTENNA, "{dmac_config_dual_antenna_set_ant:: point null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dual_antenna_set_ant::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    return dmac_dual_antenna_set_ant_at(pst_hal_device, *puc_param);
}


oal_uint32 dmac_dual_antenna_init(oal_void)
{
    if (OAL_SUCC != dmac_dual_antenna_register_dmac_misc_event(HAL_EVENT_DMAC_DUAL_ANTENNA_SWITCH, dmac_dual_antenna_notify_alg))
    {
        OAM_ERROR_LOG0(0, OAM_SF_SMART_ANTENNA, "{dmac_device_init HAL_EVENT_DMAC_DUAL_ANTENNA_SWITCH fail!}");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


oal_uint32  dmac_config_get_ant_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_atcmdsrv_ant_info_response_event       st_atcmdsrv_ant_info;
    oal_uint8                                   uc_ant_type;                            //当前所用天线，0为WIFI主天线，1为分级天线
    oal_uint32                                  ul_last_ant_change_time_ms;                //上次切换天线时刻
    oal_uint32                                  ul_ant_change_number;                   //天线切换次数
    oal_uint32                                  ul_main_ant_time_s;                       //用在WIFI主天线时长
    oal_uint32                                  ul_aux_ant_time_s;                        //用在从天线(分级天线)时长
    oal_uint32                                  ul_total_time_s;                        //WIFI开启时长
    dmac_alg_cfg_get_ant_info_notify(pst_mac_vap, &uc_ant_type, &ul_last_ant_change_time_ms,
        &ul_ant_change_number, &ul_main_ant_time_s, &ul_aux_ant_time_s, &ul_total_time_s);
    st_atcmdsrv_ant_info.uc_event_id                = OAL_ATCMDSRV_GET_ANT_INFO;
    st_atcmdsrv_ant_info.uc_ant_type                = uc_ant_type;
    st_atcmdsrv_ant_info.ul_last_ant_change_time_ms = ul_last_ant_change_time_ms;
    st_atcmdsrv_ant_info.ul_ant_change_number       = ul_ant_change_number;
    st_atcmdsrv_ant_info.ul_main_ant_time_s         = ul_main_ant_time_s;
    st_atcmdsrv_ant_info.ul_aux_ant_time_s          = ul_aux_ant_time_s;
    st_atcmdsrv_ant_info.ul_total_time_s            = ul_total_time_s;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_ANT_INFO,
        OAL_SIZEOF(dmac_atcmdsrv_ant_info_response_event), (oal_uint8 *)&st_atcmdsrv_ant_info);
    return OAL_SUCC;
}
oal_uint32  dmac_config_double_ant_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_query_response_event                    st_atcmdsrv_double_ant_switch;
    hal_to_dmac_device_stru                     *pst_hal_device;
    oal_uint32                                   ul_ret = OAL_FAIL;
    oal_uint8 uc_switch;
    oal_uint8 uc_by_ini;

    uc_switch = *puc_param;
    uc_by_ini = *(puc_param + 1);

    if (uc_by_ini)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SMART_ANTENNA, "{dmac_config_double_ant_switch::ini, enable: %d.}", uc_switch);
        gul_dual_antenna_enable = uc_switch;

        /* 使能中断 */
        hal_dual_antenna_init();
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    ul_ret = dmac_alg_cfg_double_ant_switch_notify(pst_hal_device, uc_switch);

    st_atcmdsrv_double_ant_switch.query_event   = OAL_ATCMDSRV_DOUBLE_ANT_SW;
    st_atcmdsrv_double_ant_switch.reserve[0]    = (oal_int8)ul_ret;//调用之后的返回值
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_DOUBLE_ANT_SW,
        OAL_SIZEOF(dmac_query_response_event), (oal_uint8 *)&st_atcmdsrv_double_ant_switch);
    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI

OAL_STATIC oal_uint32  dmac_config_vowifi_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32            ul_ret;
    mac_cfg_vowifi_stru  *pst_cfg_vowifi;
    dmac_vap_stru        *pst_dmac_vap;



    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);


    if ((OAL_PTR_NULL == pst_dmac_vap->pst_vowifi_status)||
        (OAL_PTR_NULL == pst_mac_vap->pst_vowifi_cfg_param))
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vowifi_info::null param,pst_vowifi_status=%p pst_vowifi_cfg_param=%p.}",
                       pst_dmac_vap->pst_vowifi_status, pst_mac_vap->pst_vowifi_cfg_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_cfg_vowifi = (mac_cfg_vowifi_stru *)puc_param;
    ul_ret = mac_vap_set_vowifi_param(pst_mac_vap, pst_cfg_vowifi->en_vowifi_cfg_cmd, pst_cfg_vowifi->uc_value);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vowifi_info::param[%d] set failed[%d].}", pst_cfg_vowifi->en_vowifi_cfg_cmd, ul_ret);
    }

    /* 更新vowifi模式，同时初始化相关统计值 */
    dmac_vap_vowifi_init(pst_dmac_vap);
    return ul_ret;
}

oal_uint32  dmac_config_vowifi_report(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint32       ul_ret;
    mac_vap_stru    *pst_mac_vap;
    dmac_user_stru  *pst_dmac_user;

    /* 目前仅Legacy sta支持这种操作 */
    pst_mac_vap = &pst_dmac_vap->st_vap_base_info;
    if ((OAL_PTR_NULL == pst_dmac_vap->pst_vowifi_status)||
        (OAL_PTR_NULL == pst_mac_vap->pst_vowifi_cfg_param))
    {
        return OAL_SUCC;
    }

    /* 设备up，且使能了vowifi状态才能触发切换vowifi状态 */
    if ((MAC_VAP_STATE_UP != pst_mac_vap->en_vap_state) ||
        (VOWIFI_DISABLE_REPORT == pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode))
    {
        return OAL_SUCC;
    }

    /* "申请vowifi逻辑切换"仅上报一次直到重新更新vowifi模式 */
    if (OAL_FALSE == pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_reported)
    {
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_VOWIFI, "{dmac_config_vowifi_report::Mode[%d],rssi_thres[%d],period_ms[%d],trigger_count[%d].}",
                        pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode,
                        ((VOWIFI_LOW_THRES_REPORT == pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode)? pst_mac_vap->pst_vowifi_cfg_param->c_rssi_low_thres : pst_mac_vap->pst_vowifi_cfg_param->c_rssi_high_thres),
                        pst_mac_vap->pst_vowifi_cfg_param->us_rssi_period_ms,
                        pst_mac_vap->pst_vowifi_cfg_param->uc_trigger_count_thres);

        ul_ret = dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_VOWIFI_REPORT, 0, OAL_PTR_NULL);
        if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_VOWIFI, "{dmac_config_vowifi_report::dmac_send_sys_event failed[%d].}",ul_ret);
            return ul_ret;
        }

        pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_reported = OAL_TRUE;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_VOWIFI, "{dmac_config_vowifi_report::vowifi been reported once!}");
    }

    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_mac_vap->us_assoc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_VOWIFI, "{dmac_config_vowifi_report::pst_dmac_user[%d] NULL.}", pst_mac_vap->us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新用于监测的时间戳*/
    pst_dmac_vap->pst_vowifi_status->uc_rssi_trigger_cnt   = 0;
    pst_dmac_vap->pst_vowifi_status->ull_rssi_timestamp_ms = OAL_TIME_GET_STAMP_MS();
    pst_dmac_vap->pst_vowifi_status->ul_tx_failed        = pst_dmac_user->st_query_stats.ul_tx_failed;
    pst_dmac_vap->pst_vowifi_status->ul_tx_total         = pst_dmac_user->st_query_stats.ul_tx_total;

    return OAL_SUCC;
}

#endif /* _PRE_WLAN_FEATURE_VOWIFI */

#ifdef _PRE_WLAN_FEATURE_TXBF
#if (WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS)


OAL_STATIC oal_void dmac_clr_fake_vap(mac_vap_stru *pst_mac_vap, hal_to_dmac_device_stru *pst_hal_device)
{
    dmac_device_stru               *pst_dmac_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == pst_hal_device))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_clr_fake_vap::pointer NULL.}");
        return;
    }

    pst_dmac_device   = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_clr_fake_vap:: pst_dmac_device null. device_id[%d]}", pst_mac_vap->uc_device_id);
        return;
    }
    if (0xFF != pst_dmac_device->uc_fake_vap_id)
    {
        hal_clr_fake_vap(pst_hal_device, pst_dmac_device->uc_fake_vap_id);
    }
}


OAL_STATIC oal_void dmac_set_fake_vap(mac_vap_stru *pst_mac_vap, hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint8                       uc_fake_vap_id;
    dmac_device_stru               *pst_dmac_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == pst_hal_device))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_set_fake_vap::pointer NULL.}");
        return;
    }

    pst_dmac_device   = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_set_fake_vap:: pst_dmac_device null. device_id[%d]}", pst_mac_vap->uc_device_id);
        return;
    }

    uc_fake_vap_id = 0xFF;
    hal_get_fake_vap_id(pst_hal_device, &uc_fake_vap_id);
    pst_dmac_device->uc_fake_vap_id = uc_fake_vap_id;
    if (0xFF != uc_fake_vap_id)
    {
        hal_set_fake_vap(pst_hal_device, uc_fake_vap_id);
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_set_fake_vap::no fake vap id.}");
    }

}
#endif
#endif

OAL_STATIC oal_uint32 dmac_vap_add_single_hal_vap(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru  *pst_dmac_vap,
                                     hal_to_dmac_vap_stru **pst_hal_vap_out, mac_cfg_add_vap_param_stru *pst_param)
{
    oal_uint8                       uc_vap_idx;
    mac_vap_stru                   *pst_mac_vap;
    hal_to_dmac_vap_stru           *pst_hal_vap;
#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
    dmac_device_stru               *pst_dmac_device;
#endif

    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_add_single_hal_device::add hal_vap failed. pst_hal_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);
    uc_vap_idx = pst_param->uc_vap_id;

#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
    /* 抓包模式开关打开, 此时第16个VAP即ext ap11将被禁用 */
    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if ((DMAC_PKT_CAP_NORMAL != pst_dmac_device->st_pkt_capture_stat.uc_capture_switch) && (WLAN_HAL_EXT_AP11_VAP_ID == uc_vap_idx))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_add_single_hal_device::add hal_vap failed, packet capture mode is open.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
#endif

#if defined (_PRE_WLAN_FEATURE_PROXYSTA)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_psta_init_vap(&pst_dmac_vap->st_vap_base_info, pst_param);
#endif
    dmac_psta_init_vap(pst_dmac_vap);

    /* 如果是Proxy STA，则在hal 配置为Proxy STA模式，这个模式只在hal层使用 */
    hal_add_vap(pst_hal_device, mac_vap_is_vsta(&pst_dmac_vap->st_vap_base_info) ? WLAN_VAP_MODE_PROXYSTA : pst_param->en_vap_mode, uc_vap_idx, &pst_hal_vap);
#elif defined (_PRE_WLAN_FEATURE_P2P)
    /* 同一个vap ，能够挂接多个hal_vap */
    /* 由于MAC 硬件对于P2P 相关寄存器的限制。
    *  GO 只能创建在hal_vap_id = 1,
    *  CL 只能创建在hal_vap_id = 5,
    *  需要将P2P 模式传到hal_add_vap 函数中  */
    /*
    *  |7      4|3      0|
    *  |p2p_mode|vap_mode|
    */
    hal_add_vap(pst_hal_device, (pst_param->en_vap_mode) | (oal_uint8)(pst_param->en_p2p_mode << 4) ,
                uc_vap_idx, &pst_hal_vap);
#else
    hal_add_vap(pst_hal_device, pst_param->en_vap_mode, uc_vap_idx, &pst_hal_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_HILINK
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_hilink_init_vap(&(pst_dmac_vap->st_vap_base_info));
#endif
#endif
    /* 统一在最外层释放前面已经申请的资源 */
    if (OAL_PTR_NULL == pst_hal_vap)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_add_single_hal_device::add hal_vap failed. pst_hal_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if defined (_PRE_WLAN_FEATURE_PROXYSTA)
    if (mac_vap_is_vsta(pst_mac_vap))
    {
        dmac_vap_psta_lut_idx(pst_dmac_vap) = pst_hal_vap->uc_vap_id;
    }
    else
    {
        hal_vap_set_macaddr(pst_hal_vap, mac_mib_get_StationID(pst_mac_vap));
    }
#elif defined (_PRE_WLAN_FEATURE_P2P)
    /* P2P 设置MAC 地址 */
    if ((WLAN_P2P_DEV_MODE == pst_param->en_p2p_mode) && (WLAN_VAP_MODE_BSS_STA == pst_param->en_vap_mode))
    {
        /* 设置p2p0 MAC 地址 */
        hal_vap_set_macaddr(pst_hal_vap, pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID);
    }
    else
    {
        /* 设置其他vap 的mac 地址 */
        hal_vap_set_macaddr(pst_hal_vap, mac_mib_get_StationID(pst_mac_vap));
    }
#else
    /* 配置MAC地址 */
    hal_vap_set_macaddr(pst_hal_vap, mac_mib_get_StationID(pst_mac_vap));
#endif

    /* 配置硬件WMM参数 */
    dmac_config_set_machw_wmm(pst_hal_vap, pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
    if (WLAN_VAP_MODE_BSS_STA == pst_param->en_vap_mode)
    {
        hal_set_mac_aid(pst_hal_vap, 0);
    }
#endif
    *pst_hal_vap_out = pst_hal_vap;

    return OAL_SUCC;
}

OAL_STATIC oal_void dmac_vap_del_single_hal_vap(hal_to_dmac_device_stru *pst_hal_device, hal_to_dmac_vap_stru *pst_hal_vap, oal_uint8 *puc_para)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    wlan_p2p_mode_enum_uint8        en_p2p_mode = WLAN_LEGACY_VAP_MODE;
    mac_cfg_del_vap_param_stru     *pst_del_vap_param = (mac_cfg_del_vap_param_stru *)puc_para;
#endif

    if ((OAL_PTR_NULL == pst_hal_device) ||(OAL_PTR_NULL == pst_hal_vap))
    {
        return;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    en_p2p_mode       = pst_del_vap_param->en_p2p_mode;
    if ((WLAN_P2P_GO_MODE == en_p2p_mode) || (WLAN_P2P_CL_MODE == en_p2p_mode))
    {
        /* 停止P2P 节能寄存器 */
        hal_vap_set_noa(pst_hal_vap, 0, 0, 0, 0);
        hal_vap_set_ops(pst_hal_vap, 0, 0);
    }
#endif
    /* 直接调用hal_del_vap */
    hal_del_vap(pst_hal_device, pst_hal_vap->en_vap_mode, pst_hal_vap->uc_vap_id);
}

OAL_STATIC oal_void dmac_vap_del_hal_vap(dmac_device_stru *pst_dmac_device, dmac_vap_stru  *pst_dmac_vap, oal_uint8 *puc_para)
{
    oal_uint8                       uc_dev_idx;
    oal_uint8                       uc_hal_dev_num_per_chip;
    hal_to_dmac_device_stru        *pst_hal_device;
    hal_to_dmac_vap_stru           *pst_hal_vap;                                       /* hal vap结构 */
    oal_uint8                       uc_hal_vap_id = HAL_MAX_VAP_NUM;

#ifdef _PRE_WLAN_FEATURE_P2P
    mac_cfg_del_vap_param_stru     *pst_del_vap_param = (mac_cfg_del_vap_param_stru *)puc_para;

    /* 删p2p dev的时候必定是先删去了p2p cl */
    if (WLAN_P2P_DEV_MODE == pst_del_vap_param->en_p2p_mode)
    {
        if (pst_dmac_vap->pst_p2p0_hal_vap != pst_dmac_vap->pst_hal_vap)
        {
           OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_del_hal_vap::del p2p dev,cl exist!!!,p2p hal id[%d],dmac vap hal vap id[%d]}",
                    pst_dmac_vap->pst_p2p0_hal_vap->uc_vap_id, pst_dmac_vap->pst_hal_vap->uc_vap_id);
        }
    }
#endif

    uc_hal_vap_id = pst_dmac_vap->pst_hal_vap->uc_vap_id;

    /* 不对外直接暴露规格宏 */
    hal_chip_get_device_num(pst_dmac_vap->st_vap_base_info.uc_chip_id, &uc_hal_dev_num_per_chip);
    if(uc_hal_dev_num_per_chip > WLAN_DEVICE_MAX_NUM_PER_CHIP)
    {
        OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_del_hal_vap::uc_hal_dev_num_per_chip is %d,more than %d",
                    uc_hal_dev_num_per_chip, WLAN_DEVICE_MAX_NUM_PER_CHIP);
        return;
    }

    for (uc_dev_idx = 0; uc_dev_idx < uc_hal_dev_num_per_chip; uc_dev_idx++)
    {
        pst_hal_device = pst_dmac_device->past_hal_device[uc_dev_idx];
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        hal_get_hal_vap(pst_hal_device, uc_hal_vap_id, &pst_hal_vap);
        dmac_vap_del_single_hal_vap(pst_hal_device, pst_hal_vap, puc_para);
    }
}

OAL_STATIC oal_uint32 dmac_vap_add_hal_vap(dmac_device_stru *pst_dmac_device, dmac_vap_stru  *pst_dmac_vap,
                                                hal_to_dmac_vap_stru **pst_hal_vap_out, mac_cfg_add_vap_param_stru *pst_param)
{
    oal_uint8                       uc_hal_dev_num_per_chip;
    oal_uint8                       uc_add_dev_idx;
    oal_uint8                       uc_del_dev_idx;
    hal_to_dmac_device_stru        *pst_hal_device;
    hal_to_dmac_vap_stru           *pst_hal_vap[WLAN_DEVICE_MAX_NUM_PER_CHIP] = {OAL_PTR_NULL};
    oal_uint32                      ul_ret = OAL_SUCC;

    /* 不对外直接暴露规格宏 */
    hal_chip_get_device_num(pst_dmac_vap->st_vap_base_info.uc_chip_id, &uc_hal_dev_num_per_chip);

    if(uc_hal_dev_num_per_chip > WLAN_DEVICE_MAX_NUM_PER_CHIP)
    {
        OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_add_hal_vap::uc_hal_dev_num_per_chip=%d more than MAX_NUM=%.}", uc_hal_dev_num_per_chip, WLAN_DEVICE_MAX_NUM_PER_CHIP);
        return OAL_FAIL;
    }

    /* 添加hal vap: idx=0 添加master; idx=1 添加slave*/
    for (uc_add_dev_idx = 0; uc_add_dev_idx < uc_hal_dev_num_per_chip; uc_add_dev_idx++)
    {
        pst_hal_device = pst_dmac_device->past_hal_device[uc_add_dev_idx];
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        /* dmac device 初始化的时候必须保证0是挂的主路的hal device,1挂的是辅路的hal device */
        ul_ret = dmac_vap_add_single_hal_vap(pst_hal_device, pst_dmac_vap, &pst_hal_vap[uc_add_dev_idx], pst_param);
        if (OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_add_hal_vap::hal device[%d],vap[%d]add hal vap [%d]failed.}",
                                pst_hal_device->uc_device_id, pst_dmac_vap->st_vap_base_info.uc_vap_id, uc_add_dev_idx);

            if (uc_add_dev_idx != 0)
            {
                //删除前面注册的hal device
                for (uc_del_dev_idx = 0; uc_del_dev_idx < uc_add_dev_idx; uc_del_dev_idx++)
                {
                    dmac_vap_del_single_hal_vap(pst_dmac_device->past_hal_device[uc_del_dev_idx], pst_hal_vap[0], (oal_uint8 *)pst_param);
                }
            }

            return ul_ret;
        }
        if (uc_add_dev_idx != 0)
        {
            //申请出的hal vap id异常
            if (pst_hal_vap[uc_add_dev_idx]->uc_vap_id != pst_hal_vap[uc_add_dev_idx-1]->uc_vap_id)
            {
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_add_hal_vap::master and slave id is not the same.}");

                //删除前面注册的hal device
                for (uc_del_dev_idx = 0; uc_del_dev_idx < uc_add_dev_idx; uc_del_dev_idx++)
                {
                    dmac_vap_del_single_hal_vap(pst_dmac_device->past_hal_device[uc_del_dev_idx], pst_hal_vap[0], (oal_uint8 *)pst_param);
                }

                return OAL_FAIL;
            }
        }
    }
    //动态dbdc返回主路的,非dbdc返回hal device0的
    //静态dbdc只跑一次,返回自己挂接的hal device上的hal vap
    *pst_hal_vap_out = pst_hal_vap[HAL_DEVICE_ID_MASTER];

    return OAL_SUCC;
}

OAL_STATIC oal_void dmac_vap_del_hal_device(dmac_vap_stru  *pst_dmac_vap, oal_uint8 *puc_para)
{
    dmac_device_stru               *pst_dmac_device;
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_cfg_del_vap_param_stru     *pst_del_vap_param = (mac_cfg_del_vap_param_stru *)puc_para;
#endif

    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "dmac_vap_del_hal_device::dmac device is null ptr. device id:%d",
                        pst_dmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    dmac_vap_del_hal_vap(pst_dmac_device, pst_dmac_vap, puc_para);

#ifdef _PRE_WLAN_FEATURE_P2P
    if ((WLAN_P2P_GO_MODE == pst_del_vap_param->en_p2p_mode) || (WLAN_P2P_CL_MODE == pst_del_vap_param->en_p2p_mode))
    {
        pst_dmac_device->pst_device_base_info->st_p2p_info.en_p2p_ps_pause = OAL_FALSE;
        OAL_MEMZERO(&(pst_dmac_vap->st_p2p_ops_param), OAL_SIZEOF(mac_cfg_p2p_ops_param_stru));
        OAL_MEMZERO(&(pst_dmac_vap->st_p2p_noa_param), OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));
    }

    /* 如果是p2p client, hal_vap改为p2p0*/
    if (WLAN_P2P_CL_MODE == pst_del_vap_param->en_p2p_mode)
    {
        DMAC_VAP_GET_HAL_VAP(pst_dmac_vap) = pst_dmac_vap->pst_p2p0_hal_vap;
        return;
    }
    else if (WLAN_P2P_DEV_MODE == pst_del_vap_param->en_p2p_mode)
    {
        pst_dmac_vap->pst_p2p0_hal_vap = OAL_PTR_NULL;
    }
#endif /* _PRE_WLAN_FEATURE_P2P */

    DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap) = OAL_PTR_NULL;
    DMAC_VAP_GET_HAL_VAP(pst_dmac_vap)    = OAL_PTR_NULL;
    DMAC_VAP_GET_HAL_CHIP(pst_dmac_vap)   = OAL_PTR_NULL;
}

OAL_STATIC oal_uint32 dmac_vap_add_hal_device(dmac_device_stru *pst_dmac_device, dmac_vap_stru *pst_dmac_vap, mac_cfg_add_vap_param_stru *pst_param)
{
    oal_uint32                      ul_ret              = OAL_SUCC;
    hal_to_dmac_vap_stru           *pst_hal_vap         = OAL_PTR_NULL;
    hal_to_dmac_device_stru        *pst_orig_hal_device = OAL_PTR_NULL;
    hal_to_dmac_chip_stru          *pst_hal_chip        = OAL_PTR_NULL;
#ifdef _PRE_WLAN_FEATURE_DBDC
    hal_to_dmac_vap_stru           *pst_shift_hal_vap = OAL_PTR_NULL;
    hal_to_dmac_device_stru        *pst_shift_hal_device;
#endif
    ul_ret = dmac_vap_add_hal_vap(pst_dmac_device, pst_dmac_vap, &pst_hal_vap, pst_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_add_hal_vap::dynamic DBDC add hal vap failed.}");
        return ul_ret;
    }

    /* 挂接指针统一放到最后,业务vap默认挂接到主路 */
#ifdef _PRE_WLAN_FEATURE_P2P
    /* 分别初始化P2P CL 和P2P DEV */
    if (WLAN_P2P_DEV_MODE == pst_param->en_p2p_mode)
    {
        pst_dmac_vap->pst_p2p0_hal_vap = pst_hal_vap;
    }
#endif /* _PRE_WLAN_FEATURE_P2P */

    /*根据芯片id，获取hal_to_dmac_chip_stru结构体*/
    ul_ret = hal_chip_get_chip(pst_dmac_vap->st_vap_base_info.uc_chip_id, &pst_hal_chip);
    if ((OAL_SUCC != ul_ret)||(OAL_PTR_NULL == pst_hal_chip))
    {
        /* 获取hal chip失败，打印ERROR，但是这里只是挂指针，允许继续挂，使用的地方处理 */
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_add_hal_device::get_hal_chip failed.}");
    }

    DMAC_VAP_GET_HAL_VAP(pst_dmac_vap)    = pst_hal_vap;

    /* 根据hal vap来获取hal device */
    ul_ret = hal_chip_get_hal_device(pst_hal_vap->uc_chip_id, pst_hal_vap->uc_device_id, &pst_orig_hal_device);
    if ((OAL_SUCC != ul_ret)||(OAL_PTR_NULL == pst_orig_hal_device))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_add_hal_device::get_hal_device failed.}");
        return OAL_FAIL;
    }

    /* 保存到dmac vap结构体中，方便获取 */
    DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap) = pst_orig_hal_device;
    DMAC_VAP_GET_HAL_CHIP(pst_dmac_vap)   = pst_hal_chip;

#ifdef _PRE_WLAN_FEATURE_DBDC
    /* 需要一开始就挂接在辅路的要做m2s操作 */
    if (HAL_DEVICE_ID_SLAVE == pst_param->uc_dst_hal_dev_id)
    {
#ifdef _PRE_WLAN_FEATURE_M2S
        dmac_m2s_switch(pst_orig_hal_device, WLAN_SINGLE_NSS, HAL_M2S_EVENT_DBDC_MIMO_TO_SISO, OAL_TRUE);
#endif
        pst_shift_hal_device = dmac_device_get_another_h2d_dev(pst_dmac_device, pst_orig_hal_device);
        if (OAL_PTR_NULL == pst_shift_hal_device)
        {
            OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_DBDC,
                            "dmac_vap_add_hal_device::pst_shift_hal_device NULL,orig id[%d]",pst_orig_hal_device->uc_device_id);
            return OAL_ERR_CODE_PTR_NULL;
        }

        hal_get_hal_vap(pst_shift_hal_device, pst_hal_vap->uc_vap_id, &pst_shift_hal_vap);

        /* 未关联切到辅路暂时将11b分配给辅路 */
        hal_set_11b_reuse_sel(pst_shift_hal_device);

        /* hal device,hal vap指针替换 */
        DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap) = pst_shift_hal_device;
        DMAC_VAP_GET_HAL_VAP(pst_dmac_vap)    = pst_shift_hal_vap;
#ifdef _PRE_WLAN_FEATURE_P2P
        if (WLAN_P2P_DEV_MODE == pst_param->en_p2p_mode)
        {
            pst_dmac_vap->pst_p2p0_hal_vap = pst_shift_hal_vap;
        }
#endif /* _PRE_WLAN_FEATURE_P2P */

        /* 两路都要恢复发送 */
        dmac_m2s_switch_device_end(pst_shift_hal_device);
        dmac_m2s_switch_device_end(pst_orig_hal_device);
    }
#endif

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_P2P

OAL_STATIC oal_uint32  dmac_add_p2p_cl_vap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                *pst_device;
    dmac_device_stru               *pst_dmac_device;
    mac_cfg_add_vap_param_stru     *pst_param;
    dmac_vap_stru                  *pst_dmac_vap;
    oal_uint8                       uc_vap_idx;
    mac_vap_stru                   *pst_mac_cfg_vap;
    oal_uint32                      ul_ret;

    pst_param = (mac_cfg_add_vap_param_stru *)puc_param;

    /* 获取device */
    pst_mac_cfg_vap = (mac_vap_stru*)mac_res_get_mac_vap(pst_param->uc_cfg_vap_indx);
    if (OAL_PTR_NULL == pst_mac_cfg_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_P2P, "{dmac_add_p2p_cl_vap::pst_cfg_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device = mac_res_get_dev(pst_mac_cfg_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_mac_cfg_vap->uc_vap_id, OAM_SF_P2P, "{dmac_add_p2p_cl_vap::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_cfg_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_cfg_vap->uc_vap_id, OAM_SF_P2P, "{dmac_add_p2p_cl_vap::pst_dmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 由于P2P CL 和P2P DEV 共用VAP，故P2P CL 不需要申请内存，也不需要初始化dmac vap 结构 */
    uc_vap_idx     = pst_device->st_p2p_info.uc_p2p0_vap_idx;/* uc_p2p0_vap_idx在创建p2p0 时初始化 */
    pst_dmac_vap   = (dmac_vap_stru *)mac_res_get_dmac_vap(uc_vap_idx);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(uc_vap_idx, OAM_SF_P2P, "{dmac_add_p2p_cl_vap::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检查hmac和dmac创建vap idx是否一致 */
    if (uc_vap_idx != pst_param->uc_vap_id)
    {
        OAM_ERROR_LOG2(pst_mac_cfg_vap->uc_vap_id, OAM_SF_P2P, "{dmac_add_p2p_cl_vap::HMAC and DMAC vap indx not same!.saved_vap_id[%d], new_vap_id[%d]}",
                        uc_vap_idx, pst_param->uc_vap_id);
        return OAL_ERR_CODE_ADD_VAP_INDX_UNSYNC;
    }

    /* 根据device个数添加hal vap */
    ul_ret = dmac_vap_add_hal_device(pst_dmac_device, pst_dmac_vap, pst_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_add_p2p_cl_vap::dmac_vap_add_hal_device failed[%d].}",ul_ret);
        return ul_ret;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* device下sta个数加1 */
    if (WLAN_VAP_MODE_BSS_STA == pst_param->en_vap_mode)
    {
        /* 初始化us_assoc_vap_id为最大值代表ap未关联 */
        mac_vap_set_assoc_id(pst_mac_vap, MAC_INVALID_USER_ID);

    #ifdef _PRE_WLAN_FEATURE_STA_PM
        /* 初始化P2P CLIENT 节能状态机 */
        dmac_pm_sta_attach(pst_dmac_vap);
    #endif
    }
    mac_vap_set_p2p_mode(&pst_dmac_vap->st_vap_base_info, pst_param->en_p2p_mode);
    mac_inc_p2p_num(&pst_dmac_vap->st_vap_base_info);
#endif

    if(WLAN_VAP_MODE_BSS_STA == pst_param->en_vap_mode)
    {
        dmac_sta_csa_fsm_attach(pst_dmac_vap);
    }

    /* 初始化业务VAP，区分AP、STA和WDS模式 */
    dmac_alg_create_vap_notify(pst_dmac_vap);

    /* 检查hmac和dmac创建muti user idx是否一致 */
    if (pst_param->us_muti_user_id != pst_mac_vap->us_multi_user_idx)
    {
        OAM_ERROR_LOG0(pst_mac_cfg_vap->uc_vap_id, OAM_SF_P2P, "{dmac_add_p2p_cl_vap::HMAC and DMAC muti user indx not same!.}");
        return OAL_ERR_CODE_ADD_MULTI_USER_INDX_UNSYNC;
    }
    /*初始化p2p节能参数*/
    OAL_MEMZERO(&(pst_dmac_vap->st_p2p_ops_param), OAL_SIZEOF(mac_cfg_p2p_ops_param_stru));
    OAL_MEMZERO(&(pst_dmac_vap->st_p2p_noa_param), OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_add_p2p_cl_vap::add vap [%d] succ.}", uc_vap_idx);
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_del_p2p_cl_vap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                  *pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_STA_PM
    /* 节能状态机删除*/
    dmac_pm_sta_detach(pst_dmac_vap);
#endif
    dmac_sta_csa_fsm_detach(pst_dmac_vap);
    /* 根据hal device个数删除hal vap */
    dmac_vap_del_hal_device(pst_dmac_vap, puc_param);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_dec_p2p_num(pst_mac_vap);
#endif

    /* 删除p2p cl 后，需要将dmac vap 结构中hal_vap 指针指向p2p0_hal_vap,以便p2p0 发送数据 */
    mac_vap_set_p2p_mode(&pst_dmac_vap->st_vap_base_info, WLAN_P2P_DEV_MODE);
    //pst_dmac_vap->st_vap_base_info.uc_p2p_gocl_hal_vap_id = pst_dmac_vap->st_vap_base_info.uc_p2p0_hal_vap_id;
    oal_memcopy(mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info),
                mac_mib_get_p2p0_dot11StationID(&pst_dmac_vap->st_vap_base_info),
                WLAN_MAC_ADDR_LEN);

    /* 删除与算法相关的接口 */
    dmac_alg_delete_vap_notify(pst_dmac_vap);

    if (WLAN_VAP_MODE_CONFIG == pst_mac_vap->en_vap_mode)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_del_p2p_cl_vap::config vap should not be here.}");
        return OAL_FAIL;
    }

    OAM_WARNING_LOG1(0, OAM_SF_P2P, "{dmac_del_p2p_cl_vap::del vap [%d] succ.}", pst_mac_vap->uc_vap_id);

    return OAL_SUCC;
}

#endif


OAL_STATIC oal_uint32  dmac_config_add_vap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_device_stru               *pst_dmac_device;
    mac_cfg_add_vap_param_stru     *pst_param;
    dmac_vap_stru                  *pst_dmac_vap;
    oal_uint32                      ul_ret;
    oal_uint8                       uc_vap_idx;
    mac_vap_stru                   *pst_mac_cfg_vap;
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    mac_vap_stru                   *pst_msta;
#endif
    mac_device_stru                *pst_mac_device;

    pst_param = (mac_cfg_add_vap_param_stru *)puc_param;

#ifdef _PRE_WLAN_FEATURE_P2P
    if (WLAN_P2P_CL_MODE == pst_param->en_p2p_mode)
    {
        return dmac_add_p2p_cl_vap(pst_mac_vap, uc_len, puc_param);
    }
#endif
    /* 获取device */
    pst_mac_cfg_vap = (mac_vap_stru*)mac_res_get_mac_vap(pst_param->uc_cfg_vap_indx);
    if (OAL_PTR_NULL == pst_mac_cfg_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_add_vap::add vap failed. pst_cfg_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_cfg_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_cfg_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_add_vap::add vap failed. pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_vap_idx = pst_param->uc_vap_id;
    /* 分配dmac vap内存空间 */
    ul_ret = mac_res_alloc_dmac_vap(uc_vap_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(pst_mac_cfg_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_add_vap::mac_res_alloc_dmac_vap error.}");
        return ul_ret;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(uc_vap_idx);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_add_vap::add vap failed. pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if(MAC_VAP_VAILD == pst_dmac_vap->st_vap_base_info.uc_init_flag)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                        "{dmac_config_add_vap::add vap maybe double init!}");
    }
#endif
#ifdef _PRE_WLAN_FEATURE_UAPSD
    if (WLAN_VAP_MODE_BSS_AP == pst_param->en_vap_mode)
    {
        g_uc_uapsd_cap = pst_param->bit_uapsd_enable;
    }
#endif
    ul_ret = dmac_vap_init(pst_dmac_vap,
                           pst_dmac_device->pst_device_base_info->uc_chip_id,
                           pst_dmac_device->pst_device_base_info->uc_device_id,
                           uc_vap_idx,
                           pst_param);
    if (OAL_SUCC != ul_ret)
    {
        if (OAL_PTR_NULL != pst_dmac_vap->st_vap_base_info.pst_mib_info)
        {
            OAL_MEM_FREE(pst_dmac_vap->st_vap_base_info.pst_mib_info, OAL_TRUE);
            pst_dmac_vap->st_vap_base_info.pst_mib_info = OAL_PTR_NULL;
        }
        /* 释放资源池 */
        mac_res_free_mac_vap(uc_vap_idx);
        return ul_ret;
    }
    /* 静态/动态DBDC添加hal vap */
    ul_ret = dmac_vap_add_hal_device(pst_dmac_device, pst_dmac_vap, pst_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_add_vap::dmac_vap_add_hal_vap failed.vap id[%d],device id[%d]}",
                            pst_dmac_vap->st_vap_base_info.uc_vap_id, pst_dmac_vap->st_vap_base_info.uc_device_id);

        if (OAL_PTR_NULL != pst_dmac_vap->st_vap_base_info.pst_mib_info)
        {
            OAL_MEM_FREE(pst_dmac_vap->st_vap_base_info.pst_mib_info, OAL_TRUE);
            pst_dmac_vap->st_vap_base_info.pst_mib_info = OAL_PTR_NULL;
        }

        /* 释放资源池 */
        mac_res_free_mac_vap(uc_vap_idx);
        return ul_ret;
    }

#ifdef _PRE_WLAN_FEATURE_P2P
    /* 分别初始化P2P CL 和P2P DEV */
    if (WLAN_P2P_DEV_MODE == pst_param->en_p2p_mode)
    {
        pst_dmac_device->pst_device_base_info->st_p2p_info.uc_p2p0_vap_idx = pst_mac_vap->uc_vap_id;
    }
#endif /* _PRE_WLAN_FEATURE_P2P */

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_device_set_vap_id(pst_dmac_device->pst_device_base_info, &(pst_dmac_vap->st_vap_base_info),uc_vap_idx, pst_param->en_vap_mode, pst_param->en_p2p_mode, OAL_TRUE);
#endif
    /* 根据device en_wmm设置vap的wmm功能 */
    pst_mac_device = pst_dmac_device->pst_device_base_info;
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_cfg_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_add_vap::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    dmac_config_set_wmm_register(&pst_dmac_vap->st_vap_base_info, pst_mac_device->en_wmm);
    /* 初始化linkloss检测工具 */
    dmac_vap_linkloss_init(pst_dmac_vap);

#ifdef _PRE_WLAN_FEATURE_FTM
    /* 初始化ftm 结构体*/
    dmac_vap_ftm_int(pst_dmac_vap);
#endif

    /* 初始化业务VAP，区分AP、STA和WDS模式 */
    switch(pst_param->en_vap_mode)
    {
        case WLAN_VAP_MODE_BSS_AP:
            /* 执行特性初始化vap的函数 */
            /* 执行算法相关VAP初始化接口 */
            dmac_alg_create_vap_notify(pst_dmac_vap);

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
            //dmac_bsd_vap_add_handle(pst_dmac_vap,pst_param);
#endif

        #ifdef _PRE_WLAN_FEATURE_UAPSD
            pst_dmac_vap->uc_uapsd_max_depth = DMAC_UAPSD_QDEPTH_DEFAULT;
            #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
                mac_vap_set_uapsd_en(&pst_dmac_vap->st_vap_base_info, OAL_TRUE);
            #endif
        #endif

        #ifdef  _PRE_WLAN_FEATURE_AP_PM
            if(OAL_TRUE == pst_dmac_device->pst_device_base_info->en_pm_enable)
            {
                dmac_pm_ap_attach(pst_dmac_vap);
            }
        #endif

        #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            dmac_full_phy_freq_user_add(&(pst_dmac_vap->st_vap_base_info), OAL_PTR_NULL);
        #endif

        #ifndef _PRE_WLAN_MAC_BUGFIX_MCAST_HW_Q
            /* 设置APUT的tbtt offset */
            hal_set_psm_tbtt_offset(pst_dmac_vap->pst_hal_vap, pst_dmac_vap->us_in_tbtt_offset);

            /* beacon发送超时继续发送组播队列 */
            hal_set_bcn_timeout_multi_q_enable(pst_dmac_vap->pst_hal_vap, OAL_TRUE);
        #endif

        #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
            hal_vap_set_opmode(pst_dmac_vap->pst_hal_vap, pst_dmac_vap->st_vap_base_info.en_vap_mode);
        #endif
            break;

        case WLAN_VAP_MODE_BSS_STA:
            /* 执行特性初始化vap的函数 */
            /* 执行算法相关VAP初始化接口 */
            if(IS_LEGACY_VAP(&pst_dmac_vap->st_vap_base_info))
            {
                dmac_alg_create_vap_notify(pst_dmac_vap);
            #ifdef  _PRE_WLAN_FEATURE_STA_PM
                /* P2P DEVICE 和 P2P client 公用vap 结构，针对P2P DEVICE 不注册节能状态机 */
                dmac_pm_sta_attach(pst_dmac_vap);
            #endif
            #ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
                dmac_scan_update_gscan_vap_id(pst_mac_vap, OAL_TRUE);
            #endif
                dmac_sta_csa_fsm_attach(pst_dmac_vap);
                /* 带宽切换状态机初始化 */
                dmac_sta_bw_switch_fsm_attach(pst_dmac_vap);
            }

            /* 初始化STA AID 为 0 */
        #if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            mac_vap_set_aid(&(pst_dmac_vap->st_vap_base_info), 0);
        #endif

            break;

        case WLAN_VAP_MODE_WDS:

            break;

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{dmac_config_add_vap::invalid vap mode[%d].", pst_param->en_vap_mode);
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 申请创建dmac组播用户 */
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (mac_vap_is_vsta(&pst_dmac_vap->st_vap_base_info))
    {
        pst_msta = mac_find_main_proxysta(pst_dmac_device->pst_device_base_info);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_msta))
        {
            /* 目前proxysta方案msta要先创建，msta未创建，先创建vsta不合理，后续不好管理vsta不创建组播用户 */
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PROXYSTA, "{dmac_config_add_vap::msta is null, vsta cannot create.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        else
        {
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PROXYSTA, "{dmac_config_add_vap::vsta multi user id is[%d], the same with msta.}",  pst_msta->us_multi_user_idx);
            mac_vap_set_multi_user_idx(pst_mac_vap, pst_msta->us_multi_user_idx);
        }
    }
    else
#endif
    {
        dmac_user_add_multi_user(pst_mac_vap, pst_param->us_muti_user_id);
        mac_vap_set_multi_user_idx(pst_mac_vap, pst_param->us_muti_user_id);
        /* 检查hmac和dmac创建muti user idx是否一致 */
        if (pst_param->us_muti_user_id != pst_mac_vap->us_multi_user_idx)
        {
            OAM_ERROR_LOG0(pst_mac_cfg_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_add_vap::HMAC and DMAC muti user indx not same!.}");
            return OAL_ERR_CODE_ADD_MULTI_USER_INDX_UNSYNC;
        }
    }

    dmac_vap_init_tx_mgmt_rate(pst_dmac_vap, pst_dmac_vap->ast_tx_mgmt_bmcast);

    /* vap创建之后，mib能力需要根据hal device重新刷新并同步到host侧 用于切换到siso之后，go等vap重新创建 */
    /* 用于切换到siso之后，go等vap重新创建 ,开关机时候，vap m2s能力同步到host */
#ifdef _PRE_WLAN_FEATURE_M2S
    if(WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        if (OAL_SUCC != dmac_m2s_d2h_vap_info_syn(&pst_dmac_vap->st_vap_base_info))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                     "{dmac_config_add_vap::dmac_m2s_d2h_vap_info_syn failed.}");
        }
    }
#endif

    dmac_config_vap_mib_update(pst_mac_vap);

    /* vap相关能力cap能力同步 */
    dmac_config_vap_cap_update(pst_mac_vap);

    dmac_clear_tx_nonqos_seq_num(pst_dmac_vap);

#if(_PRE_WLAN_FEATURE_PMF == _PRE_PMF_HW_CCMP_BIP)
    pst_dmac_vap->ul_user_pmf_status = 0;
#endif /* #if(_PRE_WLAN_FEATURE_PMF == _PRE_PMF_HW_CCMP_BIP) */

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_add_vap::add vap succ. mode[%d]state[%d],hal vap id[%d]in hal device[%d].}",
                       pst_mac_vap->en_vap_mode, pst_mac_vap->en_vap_state,
                       pst_dmac_vap->pst_hal_vap->uc_vap_id, pst_dmac_vap->pst_hal_vap->uc_device_id);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_PROXYSTA

oal_uint32 dmac_config_proxysta_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32       ul_value;
    mac_device_stru *pst_mac_device;
    hal_to_dmac_device_stru        *pst_hal_device;



    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_proxysta_switch::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_value = *(oal_uint32 *)puc_param;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_is_proxysta_enabled(pst_mac_device) = ul_value ? OAL_TRUE : OAL_FALSE;
#endif

    hal_set_proxysta_enable(pst_hal_device, (oal_int32)ul_value);

    return  OAL_SUCC;
}
#endif


#if 0

OAL_STATIC oal_uint32  dmac_config_tdls_prohibited(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8         uc_value;

    uc_value = *((oal_uint8 *)puc_param);

    /* 配置tdls prohibited,1为开启禁止,0为关闭禁止 */
    mac_vap_set_tdls_prohibited(pst_mac_vap, uc_value);
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_tdls_channel_switch_prohibited(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint8         uc_value;

    uc_value = *((oal_uint8 *)puc_param);

    mac_vap_set_tdls_channel_switch_prohibited(pst_mac_vap, uc_value);

    //OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_tdls_channel_switch_prohibited succ!");
#endif
    return OAL_SUCC;
}
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_config_set_txpower(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int32                       l_value;
    mac_regclass_info_stru          *pst_regdom_info;
    oal_uint8                       uc_cur_ch_num;
    wlan_channel_band_enum_uint8    en_freq_band;
    oal_uint8                       uc_value_tx_power = 0;

    l_value = *((oal_int32 *)puc_param);

    /* 设置管制域最大功率以控制TPC发送最大功率*/
    en_freq_band  = pst_mac_vap->st_channel.en_band;
    uc_cur_ch_num = pst_mac_vap->st_channel.uc_chan_number;

    pst_regdom_info = mac_get_channel_num_rc_info(en_freq_band, uc_cur_ch_num);
    if (OAL_PTR_NULL == pst_regdom_info)
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_txpower::this channel isnot support by this country.freq_band = %d,cur_ch_num = %d}",
                            en_freq_band,uc_cur_ch_num);
    }
    else
    {
        pst_regdom_info->us_max_tx_pwr = (oal_uint16)l_value;
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_set_txpower::max_reg_tx_pwr[%d], max_tx_pwr[%d]!}\r\n",
                      pst_regdom_info->uc_max_reg_tx_pwr, pst_regdom_info->us_max_tx_pwr);

        /* 设置vap的发送功率 */
        uc_value_tx_power = (oal_uint8)OAL_MIN(pst_regdom_info->us_max_tx_pwr/10, pst_regdom_info->uc_max_reg_tx_pwr);
    }

    mac_vap_set_tx_power(pst_mac_vap, uc_value_tx_power);

    /* 刷新发送功率 */
    dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_REFRESH);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_config_offload_start_vap(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_param)
{
    mac_cfg_start_vap_param_stru *pst_start_vap_param;

    pst_start_vap_param = (mac_cfg_start_vap_param_stru *)puc_param;

    pst_mac_vap->st_channel.en_band = pst_start_vap_param->uc_band;
    pst_mac_vap->st_channel.en_bandwidth = pst_start_vap_param->uc_bandwidth;
    pst_mac_vap->l_ifindex = pst_start_vap_param->l_ifindex;
    mac_vap_init_by_protocol(pst_mac_vap, pst_start_vap_param->uc_protocol);

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        /* 设置bssid */
        mac_vap_set_bssid(pst_mac_vap, mac_mib_get_StationID(pst_mac_vap));

        /*将vap状态改变信息上报*/
        mac_vap_state_change(pst_mac_vap, MAC_VAP_STATE_UP);

    }
    else if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {

#ifdef _PRE_WLAN_FEATURE_P2P
        /* P2P0 和P2P-P2P0 共VAP 结构，P2P CL UP时候，不需要设置vap 状态 */
        if (WLAN_P2P_CL_MODE != pst_start_vap_param->en_p2p_mode)
#endif
        {
        #ifdef _PRE_WLAN_FEATURE_ROAM
            if (MAC_VAP_STATE_ROAMING != pst_mac_vap->en_vap_state)
        #endif //_PRE_WLAN_FEATURE_ROAM
            {
                pst_mac_vap->st_channel.uc_chan_number = 0;
                mac_vap_state_change(pst_mac_vap, MAC_VAP_STATE_STA_FAKE_UP);
            }
        }
    }

    /* 初始化速率集 */
    mac_vap_init_rates(pst_mac_vap);
    return OAL_SUCC;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
OAL_STATIC oal_void dmac_set_pm_para(mac_vap_stru *pst_mac_vap, oal_uint8 uc_pm_switch, oal_uint8 uc_mac_pa_switch, oal_uint8 uc_rf_cfg)
{
    dmac_vap_stru *pst_dmac_vap;

    g_pm_switch = uc_pm_switch;

    if (OAL_TRUE == uc_pm_switch)
    {
        PM_WLAN_EnableDeepSleep();
    }

#ifdef _PRE_WLAN_FEATURE_STA_PM
    g_uc_mac_pa_switch = uc_mac_pa_switch;
#endif

#if  (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
    g_uc_rf_switch_cfg = uc_rf_cfg;
#endif

    if (pst_mac_vap)
    {
        pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
#ifdef _PRE_WLAN_FEATURE_STA_PM
        DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap)->st_hal_dev_fsm.bit_mac_pa_switch_en = g_uc_mac_pa_switch;
#endif

#if  (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
        hal_rf_dev_set_ops(g_uc_rf_switch_cfg);
#endif
    }
    OAM_WARNING_LOG4(0, OAM_SF_PWR, "dmac_config_set_pm_switch:pm_switch[%d], light_switch[%d], uc_mac_pa_switch[%d], uc_rf_switch[%d].", uc_pm_switch, PM_WLAN_IsDeepSleepDisable(),uc_mac_pa_switch, uc_rf_cfg);

}
#endif
oal_uint32 dmac_config_set_pm_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1103_DEV)
    // 设置device log level，业务添加处理逻辑
    g_pm_switch = *(oal_uint8 *)puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "dmac_config_set_pm_switch %d", g_pm_switch);
#else
    switch(*puc_param)
    {
        case ALL_DISABLE:
            dmac_set_pm_para(pst_mac_vap, OAL_FALSE, OAL_FALSE, HAL_RF_ALWAYS_POWER_ON);
        break;
        case ALL_ENABLE:
            dmac_set_pm_para(pst_mac_vap, OAL_TRUE, OAL_TRUE, HAL_RF_SWITCH_ALL_LDO);
        break;
        case MAC_PA_SWTICH_EN_RF_SWTICH_EN:
            dmac_set_pm_para(pst_mac_vap, OAL_FALSE, OAL_TRUE, HAL_RF_SWITCH_ALL_LDO);
        break;
        case MAC_PA_SWTICH_EN_RF_ALWAYS_ON:
            dmac_set_pm_para(pst_mac_vap, OAL_FALSE, OAL_TRUE, HAL_RF_ALWAYS_POWER_ON);
        break;
        case LIGHT_SLEEP_SWITCH_EN:
            PM_WLAN_DisbaleDeepSleep();
            OAM_WARNING_LOG1(0, OAM_SF_PWR, "dmac_config_set_pm_switch:light_switch %d", PM_WLAN_IsDeepSleepDisable());
        break;
        default:
            OAM_WARNING_LOG1(0, OAM_SF_PWR, "dmac_config_set_pm_switch:invalid param %d", *puc_param);
        break;
    }

#endif
    return OAL_SUCC ;
}

oal_uint32 dmac_config_set_ps_check_cnt(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    g_ps_fast_check_cnt = *(oal_uint8 *)puc_param;
    g_pm_timer_restart_cnt = g_ps_fast_check_cnt;

    OAM_WARNING_LOG1(0, OAM_SF_PWR, "dmac_config_set_ps_check_cnt %d", g_ps_fast_check_cnt);

    return OAL_SUCC ;
}


#endif


OAL_STATIC oal_uint32  dmac_config_del_vap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    dmac_vap_stru                  *pst_dmac_vap = OAL_PTR_NULL;
    oal_uint8                       uc_vap_id = 0;
    mac_device_stru                *pst_device;

#ifdef _PRE_WLAN_FEATURE_P2P
    mac_cfg_del_vap_param_stru     *pst_del_vap_param;
    wlan_p2p_mode_enum_uint8        en_p2p_mode = WLAN_LEGACY_VAP_MODE;
#endif

    pst_dmac_vap   = MAC_GET_DMAC_VAP(pst_mac_vap);
    uc_vap_id      = pst_mac_vap->uc_vap_id;

    OAM_WARNING_LOG4(uc_vap_id, OAM_SF_CFG, "{dmac_config_del_vap::vap_mode[%d] state[%d] p2p_mode[%d] device_id[%d].}",
                      pst_mac_vap->en_vap_mode, pst_mac_vap->en_vap_state, pst_mac_vap->en_p2p_mode, pst_mac_vap->uc_device_id);

    if(WLAN_VAP_MODE_CONFIG == pst_mac_vap->en_vap_mode)
    {
        /* 如果是配置vap，说明已经删除(mode=0)，直接warning返回即可，不影响业务 */
        OAM_WARNING_LOG0(uc_vap_id, OAM_SF_CFG, "{dmac_config_del_vap::en_vap_mode is CONFIG.}");
        return OAL_FAIL;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(uc_vap_id, OAM_SF_CFG, "{dmac_config_del_vap::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if(_PRE_WLAN_FEATURE_PMF == _PRE_PMF_HW_CCMP_BIP)
    pst_dmac_vap->ul_user_pmf_status = 0;
#endif /* #if(_PRE_WLAN_FEATURE_PMF == _PRE_PMF_HW_CCMP_BIP) */

    // 增强可靠性，vap删除时强制关闭TSF
    hal_disable_tsf_tbtt(pst_dmac_vap->pst_hal_vap);

    /* 清除事件队列中可能有的tbtt事件 */
    frw_event_vap_flush_event(uc_vap_id, FRW_EVENT_TYPE_TBTT, OAL_TRUE);

#ifdef _PRE_WLAN_FEATURE_P2P
    pst_del_vap_param = (mac_cfg_del_vap_param_stru *)puc_param;
    en_p2p_mode       = pst_del_vap_param->en_p2p_mode;
    if (WLAN_P2P_CL_MODE == en_p2p_mode)
    {
        return dmac_del_p2p_cl_vap(pst_mac_vap, uc_len, puc_param);
    }

    /* 非p2p0 */
    if (en_p2p_mode != WLAN_P2P_DEV_MODE)
#endif
    {
        if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
        {
            #ifdef _PRE_WLAN_FEATURE_STA_PM
            /* STA节能状态机删除*/
            dmac_pm_sta_detach(pst_dmac_vap);
            #endif
            dmac_sta_csa_fsm_detach(pst_dmac_vap);
            /* 带宽切换状态机注销 */
            dmac_sta_bw_switch_fsm_detach(pst_dmac_vap);
        #ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
            dmac_scan_update_gscan_vap_id(pst_mac_vap, OAL_FALSE);
        #endif
        }

        if (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode)
        {
            /* 节能状态机删除*/
#ifdef _PRE_WLAN_FEATURE_AP_PM
            dmac_pm_ap_deattach(pst_dmac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
            //dmac_bsd_vap_del_handle(pst_dmac_vap);
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
            dmac_full_phy_freq_user_del(&(pst_dmac_vap->st_vap_base_info), OAL_PTR_NULL);
#endif
            /*删除AP 保护老化定时器*/
            dmac_protection_stop_timer(pst_dmac_vap);
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
            /*删除 40M恢复定时器*/
            dmac_chan_stop_40M_recovery_timer(pst_dmac_vap);
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
            dmac_tx_qos_enhance_detach(pst_dmac_vap);
#endif
        }

        /* 删除与算法相关的接口 */
        dmac_alg_delete_vap_notify(pst_dmac_vap);
    }

#ifdef _PRE_WLAN_11K_STAT
    dmac_stat_exit_vap(pst_dmac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_VOWIFI
    dmac_vap_vowifi_exit(pst_dmac_vap);
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* offload下删除组播用户 */
    dmac_config_del_multi_user(pst_mac_vap, OAL_SIZEOF(oal_uint16), (oal_uint8 *)&pst_mac_vap->us_multi_user_idx);
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    if (OAL_PTR_NULL != pst_dmac_vap->pst_ip_addr_info)
    {
        OAL_MEM_FREE(pst_dmac_vap->pst_ip_addr_info, OAL_TRUE);
        pst_dmac_vap->pst_ip_addr_info = OAL_PTR_NULL;
    }
#endif

    /* vap buffer内存释放 */
#ifdef _PRE_WLAN_FEATURE_TXBF_HW
    if (OAL_PTR_NULL != pst_dmac_vap->puc_vht_bfee_buff)
    {
        OAL_MEM_FREE(pst_dmac_vap->puc_vht_bfee_buff, OAL_TRUE);
        pst_dmac_vap->puc_vht_bfee_buff = OAL_PTR_NULL;
    }
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_vap_exit(pst_mac_vap);
#endif

    pst_device     = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_del_vap::mac_res_get_dev failed.}");
        return OAL_FAIL;
    }

    /* 最后1个vap删除时，清除device级带宽信息 */
    if (0 == pst_device->uc_vap_num)
    {
        pst_device->en_40MHz_intol_bit_recd = OAL_FALSE;
    }

    /* 释放tim_bitmap */
    if (OAL_PTR_NULL != pst_dmac_vap->puc_tim_bitmap)
    {
        OAL_MEM_FREE(pst_dmac_vap->puc_tim_bitmap, OAL_TRUE);
        pst_dmac_vap->puc_tim_bitmap = OAL_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_11K
    /* 删除相关定时器并释放rrm info */
    if (OAL_PTR_NULL != pst_dmac_vap->pst_rrm_info)
    {
        if (pst_dmac_vap->pst_rrm_info->st_offset_timer.en_is_registerd)
        {
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_dmac_vap->pst_rrm_info->st_offset_timer);
        }
        if (pst_dmac_vap->pst_rrm_info->st_quiet_timer.en_is_registerd)
        {
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_dmac_vap->pst_rrm_info->st_quiet_timer);
        }
        OAL_MEM_FREE(pst_dmac_vap->pst_rrm_info, OAL_TRUE);
        pst_dmac_vap->pst_rrm_info = OAL_PTR_NULL;
    }
#endif //_PRE_WLAN_FEATURE_11K

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 共存恢复优先级 */
    dmac_btcoex_set_wlan_priority(pst_mac_vap, OAL_FALSE, 0);
    dmac_btcoex_set_occupied_period(pst_dmac_vap, 0);
#endif

#ifdef _PRE_WLAN_FEATURE_SMARTANT
    dmac_config_dual_antenna_vap_check(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    dmac_vap_clear_sensing_bssid_list(pst_dmac_vap);
#endif
    /* 静态/动态DBDC删除hal vap */
    dmac_vap_del_hal_device(pst_dmac_vap, puc_param);

    /* 删除vap时清空fakeq */
    dmac_vap_clear_fake_queue(pst_mac_vap);


    mac_res_free_mac_vap(uc_vap_id);
    OAM_WARNING_LOG2(uc_vap_id, OAM_SF_CFG, "{dmac_config_del_vap:: del vap[%d] success, multe user idx[%d].}", uc_vap_id, pst_mac_vap->us_multi_user_idx);

    return OAL_SUCC;
}

#ifdef HI110x_EDA
oal_void dmac_eda_test()
{
    dmac_vap_stru           *pst_dmac_vap;
    mac_sta_pm_handler_stru *pst_pm_handler;
    hal_to_dmac_vap_stru    *pst_hal_vap;
    mac_cfg_add_vap_param_stru st_param;
    mac_channel_stru           st_channel_stru;
    oal_uint8       auc_mac_addr[] = {0,0,0,0,0,0};
    oal_uint8       auc_bssid[] = {0x58,0x6d,0x8f,0xfe,0x26,0x04};

    /* 打桩入参 */
    st_param.en_vap_mode        = WLAN_VAP_MODE_BSS_STA;
    st_param.uc_cfg_vap_indx    = 0;
    st_param.uc_vap_id          = 1;
    st_param.us_muti_user_id    = 0;
    st_param.en_p2p_mode        = WLAN_LEGACY_VAP_MODE;

    pst_dmac_vap = mac_res_get_dmac_vap(1);
    if (pst_dmac_vap->st_vap_base_info.uc_init_flag != MAC_VAP_VAILD)
    {
        if (OAL_SUCC != dmac_config_add_vap(&(pst_dmac_vap->st_vap_base_info), 10, (oal_uint8*)(&st_param)))
        {
            PRINT("dmac_config_add_vap fail"NEWLINE);
            return;
        }
    }

    mac_mib_set_powermanagementmode(&(pst_dmac_vap->st_vap_base_info), WLAN_MIB_PWR_MGMT_MODE_PWRSAVE);
    pst_dmac_vap->st_vap_base_info.us_sta_aid = 1;
    pst_dmac_vap->st_vap_base_info.en_vap_state = MAC_VAP_STATE_UP;

    /* 低功耗相关mac寄存器配置 */
    pst_hal_vap = pst_dmac_vap->pst_hal_vap;
    hal_vap_set_macaddr(pst_hal_vap, auc_mac_addr);
    hal_set_sta_bssid(pst_hal_vap, auc_bssid);
    hal_vap_set_psm_beacon_period(pst_hal_vap, 20);                       /* 设置beacon周期 */
    hal_set_beacon_timeout_val(pst_dmac_vap->pst_hal_device, 1);             /* beacon超时等待值 */
    hal_pm_set_bcn_rf_chain(pst_hal_vap, 1);
    hal_set_psm_tbtt_offset(pst_hal_vap, PM_DEFAULT_STA_INTER_TBTT_OFFSET);
    hal_set_psm_ext_tbtt_offset(pst_hal_vap, PM_DEFAULT_EXT_TBTT_OFFSET);
    hal_set_sta_dtim_period(pst_hal_vap, 1);
    hal_set_psm_listen_interval(pst_hal_vap, 1);

    /* 使能PA_CONTROL的vap_control位 */
    hal_vap_set_opmode(pst_hal_vap, WLAN_VAP_MODE_BSS_AP);
    hal_enable_tsf_tbtt(pst_hal_vap, OAL_FALSE);
    hal_vap_set_opmode(pst_hal_vap, WLAN_VAP_MODE_BSS_STA);

    /* 同步vap状态到hal,hal device状态机切work状态 */
    hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_JOIN_COMP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));
    hal_device_handle_event(pst_dmac_vap->pst_hal_device, HAL_DEVICE_EVENT_VAP_UP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)(pst_dmac_vap->pst_hal_vap));

    /* 配置信道 */
    st_channel_stru.en_band = 1;
    st_channel_stru.en_bandwidth = 0;
    st_channel_stru.uc_chan_idx = 21;
    st_channel_stru.uc_chan_number = 153;
    dmac_mgmt_switch_channel(pst_dmac_vap->pst_hal_device, &st_channel_stru, OAL_TRUE);

    /* 协议低功耗状态机打桩doze状态 */
    pst_pm_handler = &pst_dmac_vap->st_sta_pm_handler;
    pst_pm_handler->en_hw_ps_enable = OAL_TRUE;
    pst_dmac_vap->uc_cfg_pm_mode = MIN_FAST_PS;
    pst_pm_handler->uc_can_sta_sleep = OAL_TRUE;

    oal_fsm_trans_to_state(&pst_pm_handler->st_oal_fsm, STA_PWR_SAVE_STATE_AWAKE);

    if(OAL_TRUE == pst_pm_handler->st_inactive_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_pm_handler->st_inactive_timer));
    }

    PRINT("dmac_eda_test end"NEWLINE);
}
#endif


OAL_STATIC oal_uint32  dmac_config_start_vap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru               *pst_dmac_vap;
    oal_uint32                    ul_ret;
    wlan_protocol_enum_uint8      en_protocol;
    hal_to_dmac_device_stru      *pst_hal_device;
    mac_cfg_start_vap_param_stru *pst_start_vap_param;
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                     uc_hipriv_ack = OAL_FALSE;
#endif
#ifdef _PRE_PRODUCT_ID_HI110X_DEV
    oal_int8                      ac_ssid[WLAN_SSID_MAX_LEN] = {'\0'};
#endif


    pst_start_vap_param = (mac_cfg_start_vap_param_stru *)puc_param;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    ul_ret = dmac_config_offload_start_vap(pst_mac_vap, puc_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_start_vap::dmac_config_offload_start_vap failed[%d].}", ul_ret);
        return ul_ret;
    }
#endif
    en_protocol    = pst_mac_vap->en_protocol;
    pst_hal_device = pst_dmac_vap->pst_hal_device;

    /* 配置MAC EIFS_TIME 寄存器 */
    hal_config_eifs_time(pst_hal_device, en_protocol);

#ifdef _PRE_WLAN_FEATURE_TXBF
#if (WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS)
    dmac_clr_fake_vap(pst_mac_vap, pst_hal_device);
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P0 和P2P-P2P0 共VAP 结构，P2P CL UP时候，不需要设置vap 状态 */
    if (WLAN_P2P_DEV_MODE == pst_start_vap_param->en_p2p_mode)
    {
        /* 使能PA_CONTROL的vap_control位: bit0~3对应AP0~3 bit4对应sta */
        hal_vap_set_opmode(pst_dmac_vap->pst_p2p0_hal_vap, pst_dmac_vap->st_vap_base_info.en_vap_mode);
    }
    else
#endif
    {
        /* 使能PA_CONTROL的vap_control位: bit0~3对应AP0~3 bit4对应sta */
        hal_vap_set_opmode(pst_dmac_vap->pst_hal_vap, pst_dmac_vap->st_vap_base_info.en_vap_mode);
    }

#ifdef _PRE_WLAN_FEATURE_TXBF
#if (WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS)
    dmac_set_fake_vap(pst_mac_vap, pst_hal_device);
#endif
#endif

    /* 初始化除单播数据帧以外所有帧的发送参数 */
    dmac_vap_init_tx_frame_params(pst_dmac_vap, pst_start_vap_param->en_mgmt_rate_init_flag);

#ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
    /* 初始化默认不采用针对特定协议的固定速率配置 */
    pst_dmac_vap->un_mode_valid.uc_mode_param_valid = 0;
    OAL_MEMZERO(&pst_dmac_vap->st_tx_alg_vht, OAL_SIZEOF(hal_tx_txop_alg_stru));
    OAL_MEMZERO(&pst_dmac_vap->st_tx_alg_ht, OAL_SIZEOF(hal_tx_txop_alg_stru));
    OAL_MEMZERO(&pst_dmac_vap->st_tx_alg_11ag, OAL_SIZEOF(hal_tx_txop_alg_stru));
    OAL_MEMZERO(&pst_dmac_vap->st_tx_alg_11b, OAL_SIZEOF(hal_tx_txop_alg_stru));
#endif

    /* 初始化单播数据帧的发送参数 */
    dmac_vap_init_tx_ucast_data_frame(pst_dmac_vap);

    /* 初始化vap侧发送功率 */
    dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_INIT);

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        /* 创建beacon帧 */

        /* 入网优化，不同频段下的能力不一样 */
        if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
        {
//            mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE);
            mac_mib_set_SpectrumManagementRequired(pst_mac_vap, OAL_FALSE);
        }
        else
        {
//            mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_LONG_PREAMBLE);
            mac_mib_set_SpectrumManagementRequired(pst_mac_vap, OAL_TRUE);
        }

#ifdef _PRE_WLAN_HW_TEST
        /*1151常收不走此流程*/
        if(HAL_ALWAYS_RX_RESERVED != pst_dmac_vap->pst_hal_device->bit_al_rx_flag)
#endif
        {
            ul_ret = dmac_beacon_alloc(pst_dmac_vap);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_start_vap::dmac_beacon_alloc failed[%d].}", ul_ret);
                return ul_ret;
            }
        }

        dmac_alg_vap_up_notify(pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
        dmac_bsd_vap_up_handle(pst_dmac_vap);
#endif
    }

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    /* 初始化TXOP PS */
    dmac_txopps_set_machw(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* AP模式需要在start_vap时候通知bt，sta模式在关联成功后通知 */
    dmac_btcoex_vap_up_handle(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_STA_PM
    {
        /* 修改p2p ci 问题。P2P start vap，打开前端RF接收。 */
        dmac_pm_sta_post_event(pst_dmac_vap, STA_PWR_EVENT_ENABLE_FRONT_END, 0, OAL_PTR_NULL);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    /* 启动green ap */
    dmac_green_ap_switch_auto(pst_mac_vap->uc_device_id, DMAC_GREEN_AP_VAP_UP);
#endif
    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                     "{dmac_config_start_vap::start vap success.vap_mode[%d] state[%d], p2p mode[%d]bw[%d]",
                     pst_mac_vap->en_vap_mode,
                     pst_mac_vap->en_vap_state,
                     pst_mac_vap->en_p2p_mode,
                     MAC_VAP_GET_CAP_BW(pst_mac_vap));

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CHAN, "{dmac_config_start_vap::Channel=%d, bandwidth=%d.}",
                    pst_mac_vap->st_channel.uc_chan_number, pst_mac_vap->st_channel.en_bandwidth);


    /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif

#ifdef _PRE_PRODUCT_ID_HI110X_DEV
    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        ul_ret = dmac_scan_send_probe_req_frame(pst_dmac_vap, BROADCAST_MACADDR, ac_ssid);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_config_start_vap::dmac_mgmt_send_probe_request failed[%d].}", ul_ret);
        }

#ifdef _PRE_WLAN_FEATURE_BTCOEX
        /* ap模式需要在启动时候判断是否启动ps */
        dmac_btcoex_ps_stop_check_and_notify(pst_hal_device);
#endif
    }
#endif /* _PRE_PRODUCT_ID_HI110X_DEV */

    /* vap创建之后，mib能力需要根据hal device重新刷新并同步到host侧 用于切换到siso之后，go等vap重新创建 */
    /* 用于切换到siso之后，go等vap重新创建 ,开关机时候，vap m2s能力同步到host */
#ifdef _PRE_WLAN_FEATURE_M2S
    if(WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        if (OAL_SUCC != dmac_m2s_d2h_vap_info_syn(&pst_dmac_vap->st_vap_base_info))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                     "{dmac_config_start_vap::dmac_m2s_d2h_vap_info_syn failed.}");
        }
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_down_vap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru       *pst_mac_device;
    dmac_user_stru        *pst_dmac_user;
    dmac_vap_stru         *pst_dmac_vap;
    mac_user_stru         *pst_user_tmp;
    oal_dlist_head_stru   *pst_entry;
    oal_dlist_head_stru   *pst_dlist_tmp;
    dmac_ctx_del_user_stru *pst_del_user_payload;
    frw_event_mem_stru    *pst_event_mem;
    frw_event_stru        *pst_event;
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8              uc_hipriv_ack = OAL_FALSE;
#endif

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    pst_dmac_vap  = MAC_GET_DMAC_VAP(pst_mac_vap);
    pst_dmac_user = (dmac_user_stru *)mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_down_vap::pst_multi_dmac_user[%d] null.}",
            pst_mac_vap->us_multi_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    dmac_scan_destroy_obss_timer(pst_dmac_vap);
#endif

    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(frw_event_stru));
    /* 返回值检查 */
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_FRW, "{dmac_config_down_vap:: FRW_EVENT_ALLOC failed!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(0 != pst_mac_vap->us_user_nums)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_down_vap::pst_mac_vap->us_user_nums[%d] isnot zero.}",
            pst_mac_vap->us_user_nums);

        /* 遍历vap下所有用户, 删除用户, 防止host侧删除用户异常，造成device用户未删除成功，在down的时候做一次删除保护 */
        OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
        {
            pst_user_tmp      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
            /*lint -save -e774 */
            if (OAL_PTR_NULL == pst_user_tmp)
            {
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_down_vap::pst_user_tmp null.}");
                continue;
            }
            /*lint -restore */

            pst_event = frw_get_event_stru(pst_event_mem);
            pst_del_user_payload = (dmac_ctx_del_user_stru *)pst_event->auc_event_data;
            pst_del_user_payload->us_user_idx = pst_user_tmp->us_assoc_id;
            pst_del_user_payload->en_ap_type  = MAC_AP_TYPE_BUTT;
            oal_memcopy(pst_del_user_payload->auc_user_mac_addr, pst_user_tmp->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);

            dmac_user_del(pst_event_mem);
        }
    }

    /* 释放事件内存 */
    FRW_EVENT_FREE(pst_event_mem);

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    if (!mac_vap_is_vsta(pst_mac_vap))
#endif
    {
        /* 删除组播用户tid队列中的所有信息 */
        dmac_tid_clear(&pst_dmac_user->st_user_base_info, pst_mac_device);

        /* 清除PA_CONTROL的vap_control位: bit0~3对应AP0~3 bit4对应sta */
        hal_vap_clr_opmode(pst_dmac_vap->pst_hal_vap, pst_dmac_vap->st_vap_base_info.en_vap_mode);
    }

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        /* 节能队列不为空的情况下，释放节能队列中的mpdu */
        dmac_psm_clear_ps_queue(pst_dmac_user);

#ifdef _PRE_WLAN_FEATURE_AP_PM
        /* vap down时退出wow，可以配置网页 */
        if (OAL_TRUE == pst_mac_device->en_pm_enable)
        {
            dmac_pm_post_event(pst_dmac_vap, AP_PWR_EVENT_VAP_DOWN, 0, OAL_PTR_NULL);
        }
#endif

        /* 释放beacon帧 */
        dmac_beacon_free(pst_dmac_vap);

        dmac_vap_down_notify(pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
        dmac_bsd_vap_down_handle(pst_dmac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
        hal_enable_radar_det(pst_dmac_vap->pst_hal_device, 0);
#endif
    }
    else if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        /* 关闭sta tsf定时器 */
        hal_disable_tsf_tbtt(pst_dmac_vap->pst_hal_vap);
#ifdef _PRE_WLAN_FEATURE_AP_PM
        /* ap+sta模式,sta down,退出wow */
        if (OAL_TRUE == pst_mac_device->en_pm_enable)
        {
            dmac_pm_post_event(pst_dmac_vap, AP_PWR_EVENT_VAP_DOWN, 0, OAL_PTR_NULL);
        }
#endif
    }
    else
    {

    }

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    /*删除 40M恢复定时器*/
    dmac_chan_stop_40M_recovery_timer(pst_dmac_vap);
#endif

    /* 通知扫描特性 */
    dmac_mgmt_scan_vap_down(pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    dmac_green_ap_switch_auto(pst_mac_device->uc_device_id, DMAC_GREEN_AP_VAP_DOWN);
#endif

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_down_vap::down vap succ.vap mode[%d] state[%d].}",
                      pst_mac_vap->en_vap_mode, pst_mac_vap->en_vap_state);

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif

    return OAL_SUCC;
}

oal_void dmac_config_set_wmm_register(mac_vap_stru *pst_mac_vap,  oal_bool_enum_uint8 en_wmm)
{
    dmac_vap_stru           *pst_dmac_vap;
    mac_wme_param_stru      *pst_wmm;
    oal_uint                 ul_irq_flag;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_dmac_vap->pst_hal_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_wmm_register::pst_dmac_vap->pst_hal_vap NULL,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return;
    }

    pst_wmm = mac_get_wmm_cfg(pst_mac_vap->en_vap_mode);

    /* 关中断，挂起硬件发送需要关中断 */
    oal_irq_save(&ul_irq_flag, OAL_5115IRQ_DCSWR);

    /* 挂起硬件发送 */
    /* hal_set_machw_tx_suspend(pst_hal_device); */

    /* 获取时间戳 */
    /* hal_vap_tsf_get_32bit(pst_hal_vap, &ul_tsf); */

     /* 触发硬件abort */
    /* hal_set_tx_abort_en(pst_hal_device, 1); */
    /* 关闭wmm */
    if(!en_wmm)
    {
        /* 回收所有数据 */
        dmac_flush_txq_to_tid_of_vo(pst_dmac_vap->pst_hal_device);

        
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        /* 关闭WMM */
        hal_disable_machw_edca(pst_dmac_vap->pst_hal_device);
#endif

        /* 重新设置WMM参数 */
        dmac_config_set_wmm_close_cfg(pst_dmac_vap->pst_hal_vap, pst_wmm);

        /* 关WMM，mib信息位中的Qos位置0 */
        mac_mib_set_dot11QosOptionImplemented(pst_mac_vap, OAL_FALSE);
    }
    else
    {
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
        /* 打开WMM */
        hal_enable_machw_edca(pst_dmac_vap->pst_hal_device);
#endif

        /* 重新设置WMM参数 */
        dmac_config_set_wmm_open_cfg(pst_dmac_vap->pst_hal_vap, pst_wmm);

        /* 开WMM，mib信息位中的Qos位置1 */
        mac_mib_set_dot11QosOptionImplemented(pst_mac_vap, OAL_TRUE);
    }

    /* 退出abort */
    /* hal_set_tx_abort_en(pst_hal_device, 0); */

    /* 重新设置硬件发送 */
    /* hal_set_machw_tx_resume(pst_hal_device); */

    /* 再次获取时间戳 */
    /* hal_vap_tsf_get_32bit(pst_hal_vap, &ul_tsf_passed); */

    /* 开中断 */
    oal_irq_restore(&ul_irq_flag, OAL_5115IRQ_DCSWR);
}


OAL_STATIC oal_uint32  dmac_config_wmm_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                *pst_mac_device;
    oal_bool_enum_uint8            en_wmm = OAL_FALSE;
    oal_uint8                      uc_vap_idx;
    mac_vap_stru*                  pst_vap;

    en_wmm = *(oal_bool_enum_uint8 *)puc_param;
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    /* 配置状态没有变化，不再进行后续流程 */
    if(en_wmm == pst_mac_device->en_wmm)
    {
        return OAL_SUCC;
    }
    /* 设置dev中的wmm_en，使能或者关闭4通道 */
    pst_mac_device->en_wmm = en_wmm;

    /* 遍历所有vap */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_wmm_switch::pst_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }
        dmac_config_set_wmm_register(pst_vap, en_wmm);
    }
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_get_version(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru     *pst_hal_device;
    oal_uint32                   ul_hw_version      = 0;
    oal_uint32                   ul_hw_version_data = 0;
    oal_uint32                   ul_hw_version_num  = 0;
    oal_uint8                   *puc_sw_version = (oal_uint8 *)WITP_BUILD_VERSION_ENV;//lint !e122
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    oal_int8                     ac_tmp_buff[DMAC_MAX_VERSION_SIZE] = {0};
#endif
    oal_int8                    *pac_tmp_buff;

    dmac_atcmdsrv_atcmd_response_event    st_atcmdsrv_dbb_num_info;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_version::DMAC_VAP_GET_HAL_DEVICE null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    pac_tmp_buff = ac_tmp_buff;
#else
    pac_tmp_buff = oal_memalloc(DMAC_MAX_VERSION_SIZE);
    if (!pac_tmp_buff)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    /* 获取版本号 */
    hal_get_hw_version(pst_hal_device, &ul_hw_version, &ul_hw_version_data, &ul_hw_version_num);

    /*lint -save -e718 */
    /*lint -save -e746 */
    OAL_SPRINTF((char*)pac_tmp_buff, DMAC_MAX_VERSION_SIZE, "Software Version: %s. \nFPGA Version: %04x-%04x-%02x.\n", puc_sw_version, ul_hw_version, ul_hw_version_data, ul_hw_version_num);
    /*lint -restore */
    /*lint -restore */
    oam_print(pac_tmp_buff);
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1103_DEV)
    oal_free(pac_tmp_buff);
#endif

    st_atcmdsrv_dbb_num_info.uc_event_id = OAL_ATCMDSRV_DBB_NUM_INFO_EVENT;
    st_atcmdsrv_dbb_num_info.ul_event_para = ul_hw_version;
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_version::hw_version[0x%08x]}", ul_hw_version);

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_VERSION, OAL_SIZEOF(dmac_atcmdsrv_atcmd_response_event), (oal_uint8 *)&st_atcmdsrv_dbb_num_info);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_SMPS

oal_uint32 dmac_config_set_vap_smps_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_smps_mode_stru                 *pst_smps_mode;
    mac_device_stru                        *pst_mac_device;



    pst_smps_mode    = (mac_cfg_smps_mode_stru *)puc_param;

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    if (OAL_TRUE != mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_config_set_vap_smps_mode:: not support HT.}");
        return OAL_FAIL;
    }

    if(pst_smps_mode->en_smps_mode > pst_mac_device->en_mac_smps_mode)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_config_set_vap_smps_mode:: vap mode[%d] beyond device smps mode[%d].}",
                   pst_smps_mode->en_smps_mode, pst_mac_device->en_mac_smps_mode);
        return OAL_FAIL;
    }

    /* 更新vap smps能力 */
    pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave = pst_smps_mode->en_smps_mode;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_config_set_vap_smps_mode:: set vap smps mode[%d][1.static 2.dynamic 3.mimo].}",
                     pst_smps_mode->en_smps_mode);

   // dmac_smps_update_device_capbility(pst_mac_vap);

    return OAL_SUCC;
}


oal_uint32  dmac_config_set_smps_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_dlist_head_stru                  *pst_entry;
    mac_device_stru                      *pst_mac_device;
    mac_vap_stru                         *pst_mac_vap_tmp;
    mac_user_stru                        *pst_user;
    oal_uint8                             uc_vap_idx;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    oal_uint32                            ul_ret;
#endif
    mac_cfg_smps_mode_stru                st_smps_mode;
    wlan_mib_mimo_power_save_enum_uint8   en_smps_mode;
    wlan_nss_enum_uint8                   en_avail_num_spatial_stream;
    hal_to_dmac_device_stru              *pst_hal_device;

    st_smps_mode.en_smps_mode = (wlan_mib_mimo_power_save_enum_uint8)*((oal_uint32*)puc_param);
    en_smps_mode = st_smps_mode.en_smps_mode;

    /* 获取device,pst_mac_vap为配置vap */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    ul_ret = mac_device_find_smps_mode_en(pst_mac_device, en_smps_mode);
    if (OAL_TRUE != ul_ret)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_config_set_smps_mode::device donot change smps mode.}");
        return OAL_FAIL;
    }
#endif

    /* 根据SMPS mode确认采用单流还是双流来发送，通知算法 */
    en_avail_num_spatial_stream = (WLAN_MIB_MIMO_POWER_SAVE_STATIC == en_smps_mode)? WLAN_SINGLE_NSS: WLAN_DOUBLE_NSS;

    /* 遍历device下所有业务vap，刷新所有vap的SMPS能力 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap_tmp = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_mac_vap_tmp)
        {
            continue;
        }

        if (OAL_TRUE != mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap_tmp))
        {
            continue;
        }

        /* 改变vap下空间流个数能力 */
        mac_vap_set_rx_nss(pst_mac_vap_tmp, en_avail_num_spatial_stream);

        /* 设置mib项 */
        mac_vap_set_smps(pst_mac_vap_tmp, en_smps_mode);

        /* 遍历vap下所有user，发送SMPS action帧 */
        OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
        {
             pst_user      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

             /* 非ht模式用户不发action帧 */
             if (WLAN_HT_MODE == pst_user->en_cur_protocol_mode
             || WLAN_HT_ONLY_MODE == pst_user->en_cur_protocol_mode
             || WLAN_HT_11G_MODE == pst_user->en_cur_protocol_mode)
             {
                 /* 刷新user smps和空间流能力 */
                 mac_user_set_sm_power_save(pst_user, en_smps_mode);
                 pst_user->en_avail_num_spatial_stream = en_avail_num_spatial_stream;

                 /* 调用算法钩子函数  smps的钩子，暂时和opmode用一致的，再确认 */
                 dmac_alg_cfg_user_spatial_stream_notify(pst_user);

                 if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap_tmp->en_vap_mode)
                 {
                     dmac_smps_send_action(pst_mac_vap, en_smps_mode, OAL_FALSE);
                     /* 如何保证对端收到了此帧，可能需要起一个定时器，多发几帧，作为优化点 */
                 }
             }
        }
    }

    /* 配置device SMPS软件能力 */
    pst_mac_device->en_mac_smps_mode = en_smps_mode;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_config_set_smps_mode::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 写SMPS控制寄存器 */
    hal_set_smps_mode(pst_hal_device, en_smps_mode);
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_config_set_smps_mode:: set smps mode[%d][1.static 2.dynamic 3.mimo].}",
                      pst_mac_device->en_mac_smps_mode);

    return OAL_SUCC;
}

#if 0

oal_uint32  dmac_config_get_smps_mode_en(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                     *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{dmac_config_get_smps_mode_en::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_SMPS,
                  "{dmac_config_get_smps_mode_en::device smps_mode[%d] vap smps_mode[%d] 0:STATIC 1:DYNAMIC 3: MIMO!}",
                  pst_mac_device->en_mac_smps_mode, mac_mib_get_smps(pst_mac_vap));

    return OAL_SUCC;
}
#endif
#endif


oal_uint32 dmac_config_cancel_remain_on_channel(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_scan_req_stru       *pst_scan_req_params;
    mac_device_stru         *pst_mac_device;

    /* 获取mac dev */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    /* 1.判断当前是否正在扫描，如果不是在扫描，直接返回 */
    if (MAC_SCAN_STATE_RUNNING != pst_mac_device->en_curr_scan_state)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_config_cancel_remain_on_channel::scan is not running.vap state[%d]}",pst_mac_vap->en_vap_state);
        return OAL_SUCC;
    }

    pst_scan_req_params = &(pst_mac_device->st_scan_params);

    /* 2.判断当前扫描的vap是不是p2p的，如果不是，直接返回，如果是，判断是否在监听，不是直接返回 */
    if ((pst_mac_vap->uc_vap_id != pst_scan_req_params->uc_vap_id) ||
        (MAC_SCAN_FUNC_P2P_LISTEN != pst_scan_req_params->uc_scan_func))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN,
                        "{dmac_config_cancel_remain_on_channel::other vap is scanning, scan_func[%d].}",
                        pst_scan_req_params->uc_scan_func);
        return OAL_SUCC;
    }

    /* 调用扫描完成 */
    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{dmac_config_cancel_remain_on_channel::stop scan.}");

    dmac_scan_abort(pst_mac_device);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_STA_PM

oal_void dmac_set_sta_pm_on(mac_vap_stru *pst_mac_vap, oal_uint8 uc_vap_ps_mode)
{
    dmac_vap_stru                   *pst_dmac_vap;
    mac_sta_pm_handler_stru         *pst_sta_pm_handle;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;
    if (OAL_FALSE == pst_sta_pm_handle->en_is_fsm_attached)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_set_sta_pm_on::sta pm fsm not attached.}");
        return;
    }

    /* 如果当前低功耗模式与需设置的模式一样，协议上不再重复设置低功耗 */
    if ((uc_vap_ps_mode == pst_sta_pm_handle->uc_vap_ps_mode))
    {
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_set_sta_pm_on:the same ps mode:[%d],hw_ps:[%d]}", pst_sta_pm_handle->uc_vap_ps_mode, pst_sta_pm_handle->en_hw_ps_enable);
        return;
    }

    /* STA PM 的状态清理 */
    dmac_sta_initialize_psm_globals(pst_sta_pm_handle);

    if (uc_vap_ps_mode == NO_POWERSAVE)
    {
        pst_sta_pm_handle->uc_vap_ps_mode = uc_vap_ps_mode;
        mac_mib_set_powermanagementmode(pst_mac_vap, WLAN_MIB_PWR_MGMT_MODE_ACTIVE);
    }

    /* 确保低功耗模式和定时器同步,开低功耗必须在关联上的时候才设置模式,重启activity定时器 */
    if ((MAC_VAP_STATE_UP == pst_dmac_vap->st_vap_base_info.en_vap_state) || (MAC_VAP_STATE_PAUSE == pst_dmac_vap->st_vap_base_info.en_vap_state))
    {
        pst_sta_pm_handle->uc_vap_ps_mode = uc_vap_ps_mode;
        if (uc_vap_ps_mode != NO_POWERSAVE)
        {
            mac_mib_set_powermanagementmode(pst_mac_vap, WLAN_MIB_PWR_MGMT_MODE_PWRSAVE);

            /* 打开低功耗时需要更新keepalive计数 */
            dmac_psm_update_keepalive(pst_dmac_vap);
        }

        dmac_psm_start_activity_timer(pst_dmac_vap,pst_sta_pm_handle);
    }
    else
    {
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,"{dmac_set_sta_pm_on::vap state:[%d],assoc aid:[%d] not start timer}",pst_dmac_vap->st_vap_base_info.en_vap_state,pst_dmac_vap->st_vap_base_info.us_sta_aid);
        return;
    }

    OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_set_sta_pm_on:ps mode:[%d],aid[%d]}", uc_vap_ps_mode,pst_dmac_vap->st_vap_base_info.us_sta_aid);
}
 
 oal_uint32 dmac_config_set_sta_pm_on(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
 {
    mac_cfg_ps_open_stru            *pst_sta_pm_open = OAL_PTR_NULL;
    dmac_vap_stru                   *pst_dmac_vap = OAL_PTR_NULL;
    oal_uint8                        uc_final_open_sta_pm = 0;
    oal_uint32                       ul_ret = OAL_SUCC;
    oal_uint8                        uc_cfg_pm_mode;

    mac_sta_pm_handler_stru         *pst_sta_pm_handler;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_sta_pm_open = (mac_cfg_ps_open_stru *)puc_param;

    if (WLAN_VAP_MODE_BSS_STA != pst_mac_vap->en_vap_mode)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR,"{dmac_config_set_sta_pm_on::vap mode is:[%d] not sta}",pst_mac_vap->en_vap_mode);
        return OAL_SUCC;
    }

    ul_ret = dmac_psm_check_module_ctrl(pst_mac_vap, pst_sta_pm_open->uc_pm_ctrl_type, pst_sta_pm_open->uc_pm_enable, &uc_final_open_sta_pm);

    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_sta_pm_on::dmac_psm_check_module_ctrl failed, ul_ret=%d.}", ul_ret);
        return ul_ret;
    }

    if (MAC_STA_PM_SWITCH_ON == uc_final_open_sta_pm)
    {
        uc_cfg_pm_mode = pst_dmac_vap->uc_cfg_pm_mode;

        /*保证开低功耗时的pm mode不为 NO_POWERSAVE */
        if (NO_POWERSAVE == uc_cfg_pm_mode)
        {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG,"{dmac_config_set_sta_pm_on::modual[%d]want to open[%d]pm,but pm mode is no powersave}",pst_sta_pm_open->uc_pm_ctrl_type,pst_sta_pm_open->uc_pm_enable);
            uc_cfg_pm_mode = MIN_FAST_PS;
        }
    }
    else
    {
        pst_sta_pm_handler = &pst_dmac_vap->st_sta_pm_handler;
        if (OAL_TRUE == pst_sta_pm_handler->en_is_fsm_attached
            && GET_PM_STATE(pst_sta_pm_handler) != STA_PWR_SAVE_STATE_ACTIVE)
        {
            dmac_pm_sta_state_trans(pst_sta_pm_handler, STA_PWR_SAVE_STATE_ACTIVE, STA_PWR_EVENT_DEASSOCIATE);/* 再设状态 */
        }

        /* 关闭低功耗时,低功耗模式切回NO_POWERSAVE */
        uc_cfg_pm_mode = NO_POWERSAVE;

    }

    dmac_set_sta_pm_on(pst_mac_vap, uc_cfg_pm_mode);

    return OAL_SUCC;

 }
#ifdef _PRE_PSM_DEBUG_MODE

#if (defined(SW_DEBUG)&& !defined(WIN32))
 extern  oal_uint8 g_pm_wlan_debug;
#endif
 oal_uint32 dmac_show_ps_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
 {
     mac_cfg_ps_info_stru            *pst_ps_info;
     dmac_vap_stru                   *pst_dmac_vap;
     mac_sta_pm_handler_stru         *pst_sta_pm_handle;
     oal_uint8                        uc_psm_info_enable;  //开启psm的维测输出
     oal_uint8                        uc_psm_debug_mode;
     oal_uint8                        uc_doze_trans_flag;
     oal_uint32                       uiCnt;

     pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

     pst_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;
     if (OAL_FALSE == pst_sta_pm_handle->en_is_fsm_attached)
     {
         OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_show_ps_info::sta pm fsm not attached.}");
         return OAL_FAIL;
     }

     pst_ps_info = (mac_cfg_ps_info_stru *)puc_param;
     uc_psm_info_enable = pst_ps_info->uc_psm_info_enable;
     uc_psm_debug_mode  = pst_ps_info->uc_psm_debug_mode;


     /* 切doze的条件 */
     uc_doze_trans_flag = (pst_sta_pm_handle->en_beacon_frame_wait) | (pst_sta_pm_handle->st_null_wait.en_doze_null_wait << 1) | (pst_sta_pm_handle->en_more_data_expected << 2)
                  | (pst_sta_pm_handle->st_null_wait.en_active_null_wait << 3) | (pst_sta_pm_handle->en_direct_change_to_active << 4);

     if (1 == uc_psm_info_enable)
     {
         for(uiCnt= 0; uiCnt < PM_MSG_COUNT; uiCnt++)
         {
             OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{pm debug msg[%d] [%d]}",uiCnt,pst_sta_pm_handle->aul_pmDebugCount[uiCnt]);
         }
     }
     /* 增加配置命令2查看低功耗关键维测 */
     else if (2 == uc_psm_info_enable)
     {
         OAM_WARNING_LOG_ALTER(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_PWR,"{dmac_show_ps_info::now pm state:[%d],ps mode:[%d],don't doze reason[0x%x],aid[%d]}",4,
                      GET_PM_STATE(pst_sta_pm_handle),pst_sta_pm_handle->uc_vap_ps_mode,uc_doze_trans_flag,pst_dmac_vap->st_vap_base_info.us_sta_aid);

         OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_PWR,"{full phy freq user[%d],psm timer:timout[%d],cnt[%d].}",pst_dmac_vap->pst_hal_device->uc_full_phy_freq_user_cnt,g_device_wlan_pm_timeout,g_pm_timer_restart_cnt,
                            pst_sta_pm_handle->uc_psm_timer_restart_cnt);

         dmac_pm_key_info_dump(pst_dmac_vap);
     }
     else
     {
         OAL_MEMZERO(&(pst_sta_pm_handle->aul_pmDebugCount),OAL_SIZEOF(pst_sta_pm_handle->aul_pmDebugCount));
         g_deepsleep_fe_awake_cnt   = 0;
         g_lightsleep_fe_awake_cnt  = 0;
     }

     if (OAL_TRUE == uc_psm_debug_mode)
     {
        #if (defined(SW_DEBUG)&& !defined(WIN32))
         g_pm_wlan_debug = 1;
        #endif

        #ifdef _PRE_PM_TIME_DEBUG
         g_pm_time_debug = 1;
        #endif
     }
     else
     {
        #if (defined(SW_DEBUG)&& !defined(WIN32))
         g_pm_wlan_debug = 0;
        #endif


        #ifdef _PRE_PM_TIME_DEBUG
         g_pm_time_debug = 0;
        #endif
     }

     return OAL_SUCC;
}
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_P2P

oal_uint32  dmac_config_set_p2p_ps_ops(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru               *pst_dmac_vap;
    hal_to_dmac_vap_stru        *pst_hal_vap;
    mac_cfg_p2p_ops_param_stru  *pst_p2p_ops;


    pst_p2p_ops = (mac_cfg_p2p_ops_param_stru *)puc_param;
    /* 获取hal_vap 结构体 */
    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    /*  sanity check, CTwindow 7bit*/
    if(pst_p2p_ops->uc_ct_window < 127)
    {
        /*wpa_supplicant只设置ctwindow size,保存dmac,不写寄存器*/
        pst_dmac_vap->st_p2p_ops_param.uc_ct_window= pst_p2p_ops->uc_ct_window;
        return OAL_SUCC;
    }
    /*  sanity check, opps ctrl 1bit*/
    if(pst_p2p_ops->en_ops_ctrl <  2)
    {
        if((pst_p2p_ops->en_ops_ctrl ==  0)&& IS_P2P_OPPPS_ENABLED(pst_dmac_vap))
        {
            /* 恢复发送 */
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_FALSE);
        }
        pst_dmac_vap->st_p2p_ops_param.en_ops_ctrl = pst_p2p_ops->en_ops_ctrl;
    }
    else
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_set_p2p_ps_ops:invalid ops ctrl value[%d]\r\n",
                           pst_p2p_ops->en_ops_ctrl);
        return OAL_FAIL;
    }
    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_set_p2p_ps_ops:ctrl[%d] ct_window[%d] vap state[%d]\r\n",
                pst_dmac_vap->st_p2p_ops_param.en_ops_ctrl,
                pst_dmac_vap->st_p2p_ops_param.uc_ct_window,
                pst_mac_vap->en_vap_state);
    pst_hal_vap  = pst_dmac_vap->pst_hal_vap;
    /* 设置P2P ops 寄存器 */
    hal_vap_set_ops(pst_hal_vap, pst_dmac_vap->st_p2p_ops_param.en_ops_ctrl, pst_dmac_vap->st_p2p_ops_param.uc_ct_window);
    return OAL_SUCC;
}


oal_uint32  dmac_config_set_p2p_ps_noa(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru               *pst_dmac_vap;
    mac_cfg_p2p_noa_param_stru  *pst_p2p_noa;
    oal_uint32                   ul_current_tsf_lo;
    mac_device_stru             *pst_mac_device;

    pst_p2p_noa = (mac_cfg_p2p_noa_param_stru *)puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if(IS_P2P_GO(pst_mac_vap))
    {
#ifdef _PRE_WLAN_FEATURE_DBAC
        if (mac_is_dbac_running(pst_mac_device))
        {
            /* dbac运行中 go的noa由dbac接管 */
            return OAL_SUCC;
        }
#endif

        /*如果是GO, interval设为beacon interval*/
        pst_p2p_noa->ul_interval = pst_mac_device->ul_beacon_interval * 1024;
    }

    /* 获取dmac_vap 结构体 */
    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    /* 保存参数用于encap probe rsp,beacon*/
    if (pst_p2p_noa->uc_count != 0)
    {
        pst_dmac_vap->st_p2p_noa_param.uc_count = pst_p2p_noa->uc_count;
        pst_dmac_vap->st_p2p_noa_param.ul_duration = pst_p2p_noa->ul_duration;
        pst_dmac_vap->st_p2p_noa_param.ul_interval = pst_p2p_noa->ul_interval;
        hal_vap_tsf_get_32bit(pst_dmac_vap->pst_hal_vap, &ul_current_tsf_lo);
        pst_p2p_noa->ul_start_time += ul_current_tsf_lo;
        pst_dmac_vap->st_p2p_noa_param.ul_start_time = pst_p2p_noa->ul_start_time;
    }
    else
    {
        if(IS_P2P_NOA_ENABLED(pst_dmac_vap))
        {
            /* 恢复发送 */
            dmac_p2p_handle_ps(pst_dmac_vap, OAL_FALSE);
        }

        OAL_MEMZERO(&(pst_dmac_vap->st_p2p_noa_param), OAL_SIZEOF(mac_cfg_p2p_noa_param_stru));
    }
    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_set_p2p_ps_noa:start_time:%d, duration:%d, interval:%d, count:%d\r\n",
                    pst_dmac_vap->st_p2p_noa_param.ul_start_time,
                    pst_dmac_vap->st_p2p_noa_param.ul_duration,
                    pst_dmac_vap->st_p2p_noa_param.ul_interval,
                    pst_dmac_vap->st_p2p_noa_param.uc_count);
    /* 设置P2P noa 寄存器 */
    hal_vap_set_noa(pst_dmac_vap->pst_hal_vap,
                    pst_dmac_vap->st_p2p_noa_param.ul_start_time,
                    pst_dmac_vap->st_p2p_noa_param.ul_duration,
                    pst_dmac_vap->st_p2p_noa_param.ul_interval,
                    pst_dmac_vap->st_p2p_noa_param.uc_count);
    return OAL_SUCC;
}
#endif

#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) || defined(_PRE_WLAN_FEATURE_SMPS)
#if 0

OAL_STATIC oal_uint32 dmac_config_update_user_m2s_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_user_m2s_stru     *pst_user_m2s;
    mac_user_stru         *pst_mac_user;

    pst_user_m2s = (mac_user_m2s_stru *)puc_param;
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_user_m2s->us_user_idx);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "dmac_config_update_user_m2s_info::mac user(idx=%d) is null!", pst_user_m2s->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_avail_bf_num_spatial_stream(pst_mac_user, pst_user_m2s->en_avail_bf_num_spatial_stream);
    mac_user_set_avail_num_spatial_stream(pst_mac_user, pst_user_m2s->en_avail_num_spatial_stream);
    mac_user_set_bandwidth_info(pst_mac_user, pst_user_m2s->en_avail_bandwidth, pst_user_m2s->en_cur_bandwidth);
#ifdef _PRE_WLAN_FEATURE_SMPS
    mac_user_set_sm_power_save(pst_mac_user, pst_user_m2s->en_cur_smps_mode);
#endif

    /* 调用算法钩子函数 */
    dmac_alg_cfg_user_spatial_stream_notify(pst_mac_user);

    /* 调用算法钩子函数 */
    dmac_alg_cfg_user_bandwidth_notify(pst_mac_vap, pst_mac_user);

    return OAL_SUCC;
}
#endif


oal_uint32 dmac_config_d2h_user_m2s_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user)
{
    oal_uint32              ul_ret;
    mac_user_m2s_stru       st_m2s_user;

    /* m2s user信息同步hmac */
    st_m2s_user.en_user_num_spatial_stream     = pst_mac_user->en_user_num_spatial_stream;
    st_m2s_user.en_avail_num_spatial_stream    = pst_mac_user->en_avail_num_spatial_stream;
    st_m2s_user.en_avail_bf_num_spatial_stream = pst_mac_user->en_avail_bf_num_spatial_stream;
    st_m2s_user.en_avail_bandwidth             = pst_mac_user->en_avail_bandwidth;
    st_m2s_user.en_cur_bandwidth               = pst_mac_user->en_cur_bandwidth;
    st_m2s_user.us_user_idx                    = pst_mac_user->us_assoc_id;
#ifdef _PRE_WLAN_FEATURE_SMPS
    st_m2s_user.en_cur_smps_mode               = pst_mac_user->st_ht_hdl.bit_sm_power_save;
#endif

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                       "{dmac_config_d2h_user_m2s_info_syn::en_cur_bandwidth:%d, use_nss:%d, avail_num_spatial:%d, bit spms mode:%d.}",
                         pst_mac_user->en_cur_bandwidth, pst_mac_user->en_user_num_spatial_stream,
                         pst_mac_user->en_avail_num_spatial_stream, pst_mac_user->st_ht_hdl.bit_sm_power_save);

    /***************************************************************************
        抛事件到HMAC层, 同步m2s USER最新状态到HMAC
    ***************************************************************************/
    ul_ret = dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_USER_M2S_INFO_SYN, OAL_SIZEOF(st_m2s_user), (oal_uint8 *)(&st_m2s_user));
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                         "{dmac_config_d2h_user_m2s_info_syn::dmac_send_sys_event failed[%d],user_id[%d].}", ul_ret, pst_mac_user->us_assoc_id);
        return ul_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_M2S

OAL_STATIC oal_uint32  dmac_config_vap_m2s_info_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_vap_m2s_stru           *pst_vap_m2s_info;

    pst_vap_m2s_info = (mac_vap_m2s_stru *)puc_param;

    /* 同步opmode mib能力 */
    pst_mac_vap->st_cap_flag.bit_opmode = pst_vap_m2s_info->en_support_opmode;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                       "{dmac_config_vap_m2s_info_syn::pst_mac_vap->st_cap_flag.bit_opmode:%d.}",
                         pst_mac_vap->st_cap_flag.bit_opmode);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM

OAL_STATIC oal_uint32  dmac_roam_reset_ba(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, mac_device_stru *pst_mac_device)
{
    dmac_tid_stru                          *pst_tid_queue;
    oal_uint8                               uc_tid_idx;

    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx ++)
    {
        pst_tid_queue = &pst_dmac_user->ast_tx_tid_queue[uc_tid_idx];

        /* 释放BA相关的内容 */
        if (DMAC_BA_COMPLETE == pst_tid_queue->st_ba_rx_hdl.en_ba_conn_status)
        {
            dmac_ba_reset_rx_handle(pst_mac_device, pst_dmac_user, &pst_tid_queue->st_ba_rx_hdl);
            OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{dmac_roam_reset_ba:: TID[%d]:reset rx ba.}", uc_tid_idx);
        }

        if (OAL_PTR_NULL != pst_tid_queue->pst_ba_tx_hdl)
        {
            dmac_ba_reset_tx_handle(pst_mac_device, &(pst_tid_queue->pst_ba_tx_hdl), uc_tid_idx);
            OAM_WARNING_LOG1(0, OAM_SF_ROAM, "{dmac_roam_reset_ba:: TID[%d]:reset tx ba.}", uc_tid_idx);
        }

    }

    return OAL_SUCC;
}

oal_uint32  dmac_config_roam_enable(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8                                uc_roaming_mode;
    mac_device_stru                         *pst_mac_device;
    mac_user_stru                           *pst_mac_user;

#if defined(_PRE_WLAN_FEATURE_BTCOEX) || defined(_PRE_WLAN_FEATURE_SMARTANT)
    hal_to_dmac_device_stru                 *pst_hal_device;
    hal_to_dmac_chip_stru                   *pst_hal_chip;
#ifdef _PRE_WLAN_FEATURE_SMARTANT
    oal_uint32                               ul_dual_antenna_result;
#endif
#endif

#if defined(_PRE_WLAN_FEATURE_BTCOEX) || defined(_PRE_WLAN_FEATURE_SMARTANT)
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{dmac_config_roam_enable:: pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_chip = DMAC_VAP_GET_HAL_CHIP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_chip)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{dmac_config_roam_enable:: pst_hal_chip null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    uc_roaming_mode  = (oal_uint8)*puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{dmac_config_roam_enable::pst_mac_user[%d] null.}",
            pst_mac_vap->us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((uc_roaming_mode == 1) && (pst_mac_vap->en_vap_state == MAC_VAP_STATE_UP || pst_mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE))
    {

        /* 删除tid队列中的所有信息
        dmac_tid_clear(pst_mac_user, pst_mac_device);*/
        dmac_user_pause(MAC_GET_DMAC_USER(pst_mac_user));
        dmac_roam_reset_ba(MAC_GET_DMAC_VAP(pst_mac_vap), MAC_GET_DMAC_USER(pst_mac_user), pst_mac_device);

        /*通知算法用户下线，在dbac场景下减少在关联期间的信道切换操作 */
        dmac_vap_down_notify(pst_mac_vap);

        /* 漫游过程中可能发生了睡眠，增加维测，漫游前平台睡眠计数 */
        pfn_wlan_dumpsleepcnt();

        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{dmac_config_roam_enable:: [MAC_VAP_STATE_UP]->[MAC_VAP_STATE_ROAMING]}");
        pst_mac_vap->en_vap_state = MAC_VAP_STATE_ROAMING;
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        /* 需要临时暂停ps机制 */
        dmac_btcoex_ps_pause_check_and_notify(pst_hal_device);
        dmac_btcoex_set_wlan_priority(pst_mac_vap, OAL_TRUE, BTCOEX_PRIO_TIMEOUT_100MS);
#endif //_PRE_WLAN_FEATURE_BTCOEX
#ifdef _PRE_WLAN_FEATURE_SMARTANT
        pst_hal_chip->st_dual_antenna_check_status.bit_roam = OAL_TRUE;
        hal_dual_antenna_switch(pst_hal_device, 1, 0, &ul_dual_antenna_result);
        OAM_WARNING_LOG1(0, OAM_SF_SMART_ANTENNA, "{dmac_config_roam_enable::dual_antenna to 1, result:%d.}", ul_dual_antenna_result);
#endif
        if(OAL_TRUE == dmac_sta_csa_is_in_waiting(pst_mac_vap))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "dmac_config_roam_enable:vap_state into MAC_VAP_STATE_ROAMING, csa status back to INIT ");
            dmac_sta_csa_fsm_post_event(pst_mac_vap, WLAN_STA_CSA_EVENT_TO_INIT, 0, OAL_PTR_NULL);
        }
    }
    else if ((uc_roaming_mode == 0) && (pst_mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING))
    {
        /* 漫游过程中可能发生了睡眠，增加维测，漫游后平台睡眠计数 */
        pfn_wlan_dumpsleepcnt();

        dmac_roam_update_framer(MAC_GET_DMAC_VAP(pst_mac_vap), MAC_GET_DMAC_USER(pst_mac_user));
        dmac_user_resume(MAC_GET_DMAC_USER(pst_mac_user));
#ifdef _PRE_WLAN_FEATURE_SMARTANT
        pst_hal_chip->st_dual_antenna_check_status.bit_roam = OAL_FALSE;
        hal_dual_antenna_switch(pst_hal_device, 0, 0, &ul_dual_antenna_result);
        OAM_WARNING_LOG1(0, OAM_SF_SMART_ANTENNA, "{dmac_config_roam_enable::dual_antenna roam open, alg result:%d.}", ul_dual_antenna_result);
#endif

        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{dmac_config_roam_enable:: [MAC_VAP_STATE_ROAMING]->[MAC_VAP_STATE_UP]}");
        pst_mac_vap->en_vap_state = MAC_VAP_STATE_UP;

        /* 漫游结束后，刷掉occupied_period以保证BT竞争到 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
        /* 需要临时暂停ps机制 */
        dmac_btcoex_ps_pause_check_and_notify(pst_hal_device);
        dmac_btcoex_set_wlan_priority(pst_mac_vap, OAL_FALSE, 0);
        dmac_btcoex_set_occupied_period(MAC_GET_DMAC_VAP(pst_mac_vap), 0);
#endif

    }
    else
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{dmac_config_roam_enable::unexpect state[%d] or mode[%d]}",
                       pst_mac_vap->en_vap_state, uc_roaming_mode);
    }
    return OAL_SUCC;
}

oal_uint32  dmac_config_set_roam_trigger(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_roam_trigger_stru   *pst_roam_trigger;
    dmac_vap_stru           *pst_dmac_vap;



    pst_roam_trigger  = (mac_roam_trigger_stru *)puc_param;
    pst_dmac_vap      = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_dmac_vap->st_roam_trigger.c_trigger_2G = pst_roam_trigger->c_trigger_2G;
    pst_dmac_vap->st_roam_trigger.c_trigger_5G = pst_roam_trigger->c_trigger_5G;

    /* 设置门限时，reset统计值，重新设置门限后，可以马上触发一次漫游 */
    pst_dmac_vap->st_roam_trigger.ul_cnt        = 0;
    pst_dmac_vap->st_roam_trigger.ul_time_stamp = 0;

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{dmac_config_set_roam_trigger, trigger[%d, %d]}",
                     pst_roam_trigger->c_trigger_2G, pst_roam_trigger->c_trigger_5G);

    return OAL_SUCC;
}


oal_uint32  dmac_config_roam_hmac_sync_dmac(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_user_stru                   *pst_mac_user;
    mac_h2d_roam_sync_stru          *pst_sync_param = OAL_PTR_NULL;
    mac_device_stru                 *pst_mac_device;

#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
    dmac_vap_stru                   *pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
#endif

    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        /* 漫游中会遇到 kick user 的情况，降低 log level*/
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
                       "{dmac_config_roam_hmac_sync_dmac::pst_mac_user[%d] null.}", pst_mac_vap->us_assoc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_sync_param = (mac_h2d_roam_sync_stru *)puc_param;

    if(OAL_TRUE == pst_sync_param->ul_back_to_old)
    {
        /* 恢复原来bss相关信息 */
        pst_mac_vap->us_sta_aid = pst_sync_param->us_sta_aid;

        oal_memcopy(&(pst_mac_vap->st_channel), &pst_sync_param->st_channel, OAL_SIZEOF(mac_channel_stru));
        oal_memcopy(&(pst_mac_user->st_cap_info), &pst_sync_param->st_cap_info, OAL_SIZEOF(mac_user_cap_info_stru));
        oal_memcopy(&(pst_mac_user->st_key_info), &pst_sync_param->st_key_info, OAL_SIZEOF(mac_key_mgmt_stru));
        oal_memcopy(&(pst_mac_user->st_user_tx_info),&pst_sync_param->st_user_tx_info, OAL_SIZEOF(mac_user_tx_param_stru));

        /* 在漫游过程中可能又建立了聚合，因此回退时需要删除掉 */
        dmac_user_pause(MAC_GET_DMAC_USER(pst_mac_user));
        dmac_roam_reset_ba(MAC_GET_DMAC_VAP(pst_mac_vap), MAC_GET_DMAC_USER(pst_mac_user), pst_mac_device);
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
        /* 判断aid有效性 */
        if (0 == pst_sync_param->us_sta_aid || pst_sync_param->us_sta_aid >= 2008)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_config_roam_hmac_sync_dmac::aid invalid[%d]}", pst_sync_param->us_sta_aid);
            return OAL_FAIL;
        }
        /* STA漫游回old bss时重新配置aid寄存器，防止被意外覆盖 */
        if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
        {
            hal_set_mac_aid(pst_dmac_vap->pst_hal_vap, pst_mac_vap->us_sta_aid);
        }
#endif
    }

    /* 设置用户8021x端口合法性的状态为合法 */
    mac_user_set_port(pst_mac_user, OAL_TRUE);

    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_roam_hmac_sync_dmac:: Sync Done!!}");

    return OAL_SUCC;
}


oal_uint32  dmac_config_roam_succ_h2d_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru   *pst_dmac_vap           = OAL_PTR_NULL;
    wlan_roam_main_band_state_enum_uint8  uc_roam_band_state;



    uc_roam_band_state = *(wlan_roam_main_band_state_enum_uint8 *)puc_param;

    /* 漫游成功之后，如果是5G，03下需要 */
    pst_dmac_vap  = MAC_GET_DMAC_VAP(pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* 如果是从2g漫游到5g,需要清楚vap统计信息 */
    if((WLAN_ROAM_MAIN_BAND_STATE_2TO5 == uc_roam_band_state)||
        (WLAN_ROAM_MAIN_BAND_STATE_5TO2 == uc_roam_band_state))
    {
        dmac_btcoex_roam_succ_handler(pst_dmac_vap->pst_hal_device, pst_mac_vap);
    }
#endif

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ROAM, "{dmac_config_roam_succ_h2d_syn:: uc_roam_band_state = %d!!}",uc_roam_band_state);

    return OAL_SUCC;
}

#endif  //_PRE_WLAN_FEATURE_ROAM
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)

OAL_STATIC oal_uint32 dmac_config_set_txrx_chain(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru               *pst_dmac_vap;
    oal_uint8                    uc_chain;
    mac_device_stru             *pst_mac_device;

    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_txrx_chain:: pst_mac_vap or puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取device */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_txrx_chain::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    uc_chain = *puc_param;


    if (0 == uc_chain)
    {
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain  = WLAN_TX_CHAIN_ZERO;
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy_chain       = WLAN_TX_CHAIN_ZERO;
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = WLAN_TX_CHAIN_ZERO;
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_rf_chain        = WLAN_RF_CHAIN_ZERO;
        g_l_rf_channel_num              = 1;
        g_l_rf_single_tran              = 0;
    }
    else if (1 == uc_chain)
    {
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain  = WLAN_TX_CHAIN_ONE;
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy_chain       = WLAN_TX_CHAIN_ONE;
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = WLAN_TX_CHAIN_ONE;
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_rf_chain        = WLAN_RF_CHAIN_ONE;
        g_l_rf_channel_num              = 2;
        g_l_rf_single_tran              = 1;
    }
    else if (2 == uc_chain)
    {
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain  = WLAN_TX_CHAIN_DOUBLE;
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_phy_chain       = WLAN_TX_CHAIN_DOUBLE;
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = WLAN_TX_CHAIN_ZERO;
        pst_dmac_vap->pst_hal_device->st_cfg_cap_info.uc_rf_chain        = WLAN_RF_CHAIN_DOUBLE;
        g_l_rf_channel_num              = 2;
        g_l_rf_single_tran              = 0;
    }
    else
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "dmac_config_set_txrx_chain:uc_chain = %d\n", uc_chain);
    }

    //pst_dmac_vap->uc_vap_tx_chain = pst_mac_device->uc_tx_chain;

    hal_set_txrx_chain(pst_dmac_vap->pst_hal_device);

    /* 初始化单播数据帧的发送参数 */
    dmac_vap_init_tx_ucast_data_frame(pst_dmac_vap);

    /* 初始化除单播数据帧以外所有帧的发送参数 */
    dmac_vap_init_tx_frame_params(pst_dmac_vap, OAL_TRUE);

#ifdef _PRE_WLAN_CFGID_DEBUG
    if (OAL_TRUE == mac_is_dbac_running(pst_mac_device))
    {
        oal_uint8                    auc_buf[128];
        mac_ioctl_alg_config_stru   *pst_alg_config = (mac_ioctl_alg_config_stru *)auc_buf;

        OAL_SPRINTF((char*)(auc_buf + sizeof(mac_ioctl_alg_config_stru)), 128 - sizeof(mac_ioctl_alg_config_stru), "dbac update_prot_chain");
        pst_alg_config->uc_argc = 2;
        pst_alg_config->auc_argv_offset[0] = 0;
        pst_alg_config->auc_argv_offset[1] = 5;
        dmac_config_alg(pst_mac_vap, 30, auc_buf);
    }
#endif

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_RF_CALI

OAL_STATIC oal_uint32 dmac_config_set_2g_txrx_path(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru     *pst_hal_device;
    oal_uint8                    uc_vap_idx;
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                    uc_hipriv_ack = OAL_FALSE;
#endif
    mac_device_stru             *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    /* 获取hal device */
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_2g_txrx_path::device id [%d],but pst_hal_device null.}",pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 遍历device查找2g vap更新通路配置 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);

        if ((OAL_PTR_NULL != pst_mac_vap) &&
            (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band))
        {
            hal_set_2g_txrx_path(pst_hal_device,pst_mac_vap->st_channel.uc_chan_idx,
                         pst_mac_vap->st_channel.en_bandwidth,*puc_param);

            /* 刷新发送功率 */
            dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_REFRESH);

            /* 命令成功返回Success */
#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
            uc_hipriv_ack = OAL_TRUE;
            dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif

            OAL_IO_PRINT("2G switch to path %d\n",*puc_param);

            return OAL_SUCC;

        }
    }

    return OAL_FAIL;
}
#endif
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

oal_uint32 dmac_config_load_ini_power_gain(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

    hal_load_ini_power_gain();

    /* 更新发射功率表 */
    dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_INIT);

    return OAL_SUCC;
}


oal_uint32 dmac_config_set_linkloss_threshold(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    if (OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_linkloss_threshold:: pointer is null,pst_mac_vap is NULL");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_memcopy(g_auc_int_linkloss_threshold, puc_param, OAL_SIZEOF(g_auc_int_linkloss_threshold));

    OAM_WARNING_LOG3(0, OAM_SF_CFG,
        "{dmac_config_set_linkloss_threshold:: g_auc_int_linkloss_threshold is changed uc_linkloss_threshold_normal:%d ,uc_linkloss_threshold_bt:%d, uc_linkloss_threshold_dbac:%d",
        g_auc_int_linkloss_threshold[WLAN_LINKLOSS_MODE_NORMAL], g_auc_int_linkloss_threshold[WLAN_LINKLOSS_MODE_BT], g_auc_int_linkloss_threshold[WLAN_LINKLOSS_MODE_DBAC]);

    return OAL_SUCC;
}


oal_uint32 dmac_config_set_all_log_level(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32      ul_ret = 0;
    oal_uint8       uc_vap_idx;
    oal_uint8       uc_level;


    uc_level = (oal_uint8)(*puc_param);

    for (uc_vap_idx = 0; uc_vap_idx < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vap_idx++)
    {
        ul_ret += oam_log_set_vap_level(uc_vap_idx, uc_level);

        if (OAL_SUCC != ul_ret)
        {
            return ul_ret;
        }
    }
    return OAL_SUCC;
}

oal_uint32 dmac_config_set_d2h_hcc_assemble_cnt(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32      ul_val;

    if (OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_d2h_hcc_assemble_cnt:: pointer is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_val = (oal_uint32)(*puc_param);
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    hcc_set_tx_count(ul_val);
#else
    OAL_REFERENCE(ul_val);
#endif
    return OAL_SUCC;
}


oal_uint32 dmac_config_set_cus_rf(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru    *pst_hal_device = OAL_PTR_NULL;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_cus_rf::pst_hal_device null pointer}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_config_custom_rf(pst_hal_device, puc_param);
#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
    hal_set_rf_custom_reg(pst_hal_device);
#endif
    return OAL_SUCC;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)

oal_uint32 dmac_config_set_cus_over_temper_rf(oal_uint8 *puc_param)
{
    if (OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_cus_over_temper_rf::puc_param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_set_cus_over_temper_rf(puc_param);
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI

oal_uint32 dmac_config_set_cus_dyn_cali(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    return hal_config_custom_dyn_cali(puc_param);
}
#endif


oal_uint32 dmac_config_set_cus_dts_cali(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1151)
    oal_int32     ret;
    hal_to_dmac_device_stru    *pst_hal_device;
    oal_uint32    ul_run_time;
    oal_uint32    ul_start_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
#endif

    hal_config_custom_dts_cali(puc_param);

#if (_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1151)
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_cus_dts_cali::pst_hal_device null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 停止PA和PHY的工作 */
    hal_disable_machw_phy_and_pa(pst_hal_device);
#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
    /* 初始化PHY */
    //hal_initialize_phy(&pst_device->st_hal_device_base);
    hal_set_rf_custom_reg(pst_hal_device);
#endif
    /* FPGA zhangyu Debug */
    /* 初始化RF系统 */
    hal_initialize_rf_sys(pst_hal_device);

#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    hal_init_dyn_cali_tx_pow(pst_hal_device);
#endif

    /*lint -save -e666 */
    ul_run_time = OAL_TIME_GET_RUNTIME(ul_start_timestamp,(oal_uint32)OAL_TIME_GET_STAMP_MS());
    /*lint -restore */

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_cus_dts_cali:cali dev id[%d]dts cali time[%u]ms}",
                        pst_hal_device->uc_device_id, ul_run_time);
    OAL_IO_PRINT("{dmac_config_set_cus_dts_cali:dts cali time[%u]ms\r\n}", ul_run_time);

    //SDIO_SendMsgSync(D2H_MSG_WLAN_READY);
    ret = hcc_bus_send_message2host(D2H_MSG_WLAN_READY);
    if(OAL_SUCC != ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_cus_dts_cali::send wlan ready failed ret=%d}", ret);
    }
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TPC_OPT

oal_void dmac_config_update_scaling_reg(mac_vap_stru *pst_mac_vap)
{
    oal_uint8                            uc_scale_idx  = 0;
    oal_uint8                            uc_device_id;
    oal_uint8                            uc_hal_dev_num_per_chip;
    hal_cfg_custom_nvram_params_stru    *past_src;
    hal_to_dmac_device_stru             *pst_hal_device;
    dmac_device_stru                    *pst_dmac_device;

    hal_config_get_cus_nvram_params(&past_src);

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_update_scaling_reg::pst_dmac_device null.}");
        return;
    }

    /* 不对外直接暴露规格宏 */
    hal_chip_get_device_num(pst_mac_vap->uc_chip_id, &uc_hal_dev_num_per_chip);

    for (uc_device_id = 0; uc_device_id < uc_hal_dev_num_per_chip; uc_device_id++)
    {
        pst_hal_device = pst_dmac_device->past_hal_device[uc_device_id];
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        for (uc_scale_idx = 0 ; uc_scale_idx < NUM_OF_NV_MAX_TXPOWER ; uc_scale_idx++)
        {
            HAL_DEV_GET_PER_RATE_DBB_SCALING_CTRL(pst_hal_device, uc_scale_idx) = past_src[uc_scale_idx].us_dbb_scale;

            OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CUSTOM, "{dmac_config_update_scaling_reg::aus_dbb_scale[%d]=0x%x!}",
                           uc_scale_idx, past_src[uc_scale_idx].us_dbb_scale);
        }

        hal_config_update_scaling_reg(pst_hal_device, 0);
    }
}


oal_void dmac_config_update_dsss_scaling_reg(hal_to_dmac_device_stru *pst_hal_device, hal_alg_user_distance_enum_uint8 en_dmac_device_distance_enum)
{
    oal_uint8                             uc_switch = 0;
    hal_config_get_far_dist_dsss_scale_promote_switch(&uc_switch);
    /* 超远距DSSS SCALE PROMOTE使能开关没有打开或者当前正在降SAR(sar功率小于阈值) */
    if (
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    (0 == uc_switch) ||
#endif
        (g_uc_sar_pwr_limit < HAL_SAR_PWR_LIMIT_THRESHOLD))
    {
        return;
    }

    /* 1_2M_scaling值如果并不比5.5_11M_scaling值小，则直接返回 */
    if (HAL_DEV_GET_PER_RATE_DBB_SCALING_CTRL(pst_hal_device, 0) >= HAL_DEV_GET_PER_RATE_DBB_SCALING_CTRL(pst_hal_device, 1))
    {
        return;
    }

    hal_config_update_dsss_scaling_reg(pst_hal_device, HAL_DEV_GET_TX_PWR_CTRL(pst_hal_device).aus_dbb_scale_per_rate_ctrl, en_dmac_device_distance_enum);
}
#else

oal_void dmac_config_update_scaling_reg(mac_vap_stru *pst_mac_vap)
{
    hal_cfg_custom_nvram_params_stru    *past_src;
    hal_to_dmac_device_stru             *pst_hal_device;
    dmac_device_stru                    *pst_dmac_device;
    oal_uint8                            uc_device_id;
    oal_uint8                            uc_scale_idx = 0;
    oal_uint8                            uc_hal_dev_num_per_chip;
    oal_uint16                           aus_dbb_scale[NUM_OF_NV_MAX_TXPOWER] = {0};

    hal_config_get_cus_nvram_params(&past_src);

    for (uc_scale_idx = 0 ; uc_scale_idx < NUM_OF_NV_MAX_TXPOWER ; uc_scale_idx++ )
    {
        aus_dbb_scale[uc_scale_idx] = past_src[uc_scale_idx].us_dbb_scale;
        OAM_INFO_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_update_scaling_reg::uc_scale_idx[%d], aus_dbb_scale[uc_scale_idx]=0x%x, uc_max_txpower[uc_scale_idx]=0x%x!}\r\n", uc_scale_idx, aus_dbb_scale[uc_scale_idx], past_src[uc_scale_idx].uc_max_txpower);
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_update_scaling_reg::pst_dmac_device null.}");
        return;
    }

    /* 不对外直接暴露规格宏 */
    hal_chip_get_device_num(pst_mac_vap->uc_chip_id, &uc_hal_dev_num_per_chip);

    for (uc_device_id = 0; uc_device_id < uc_hal_dev_num_per_chip; uc_device_id++)
    {
        pst_hal_device = pst_dmac_device->past_hal_device[uc_device_id];
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        hal_config_update_scaling_reg(pst_hal_device, aus_dbb_scale);
    }
}


oal_void dmac_config_update_dsss_scaling_reg(hal_to_dmac_device_stru *pst_hal_device, hal_alg_user_distance_enum_uint8 en_dmac_device_distance_enum)
{
    hal_cfg_custom_nvram_params_stru     *past_src;
    oal_uint8                             uc_scale_idx = 0;
    oal_uint8                             uc_switch = 0;
    oal_uint16                            aus_dbb_scale[NUM_OF_NV_MAX_TXPOWER] = {0};

    hal_config_get_cus_nvram_params(&past_src);
    hal_config_get_far_dist_dsss_scale_promote_switch(&uc_switch);

    /* 超远距DSSS SCALE PROMOTE使能开关没有打开或者当前正在降SAR(sar功率小于阈值) */
    if (
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    (0 == uc_switch) ||
#endif
        (g_uc_sar_pwr_limit < HAL_SAR_PWR_LIMIT_THRESHOLD))
    {
        return;
    }

    /* 1_2M_scaling值如果并不比5.5_11M_scaling值小，则直接返回 */
    if (past_src[0].us_dbb_scale >= past_src[1].us_dbb_scale)
    {
        return;
    }

    for (uc_scale_idx = 0 ; uc_scale_idx < NUM_OF_NV_MAX_TXPOWER ; uc_scale_idx++ )
    {
        aus_dbb_scale[uc_scale_idx] = past_src[uc_scale_idx].us_dbb_scale;
    }

    hal_config_update_dsss_scaling_reg(pst_hal_device, aus_dbb_scale, en_dmac_device_distance_enum);
}
#endif


oal_uint32 dmac_config_set_cus_nvram_params(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8                            uc_device_id;
    oal_uint8                            uc_hal_dev_num_per_chip;
    hal_to_dmac_device_stru             *pst_hal_device;
    dmac_device_stru                    *pst_dmac_device;

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);

    /* 不对外直接暴露规格宏 */
    hal_chip_get_device_num(pst_mac_vap->uc_chip_id, &uc_hal_dev_num_per_chip);

    for (uc_device_id = 0; uc_device_id < uc_hal_dev_num_per_chip; uc_device_id++)
    {
        pst_hal_device = pst_dmac_device->past_hal_device[uc_device_id];
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        hal_config_set_cus_nvram_params(pst_hal_device, puc_param);

        /* update rate_pow_table */
        hal_config_update_rate_pow_table(pst_hal_device);
    }

    /* update phy scaling reg */
    dmac_config_update_scaling_reg(pst_mac_vap);

    return OAL_SUCC;
}


oal_uint32 dmac_config_customize_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_device_stru     *pst_device;
    dmac_vap_stru       *pst_dmac_vap;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);


    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);

#else
    oal_uint32           ul_cnt;
    hal_cfg_custom_nvram_params_stru        *pst_cfg_nvram;

    hal_config_get_cus_nvram_params(&pst_cfg_nvram);
    if (0 == *puc_param)
    {
        for (ul_cnt = 0; ul_cnt < NUM_OF_NV_NORMAL_MAX_TXPOWER; ul_cnt++)
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "normal_nvram_params %d{%d %d 0x%x}",
                             ul_cnt,
                             pst_cfg_nvram[ul_cnt].uc_index,
                             pst_cfg_nvram[ul_cnt].uc_max_txpower,
                             pst_cfg_nvram[ul_cnt].us_dbb_scale);
        }
    }
    else
    {
        for (ul_cnt = NUM_OF_NV_NORMAL_MAX_TXPOWER; ul_cnt < NUM_OF_NV_MAX_TXPOWER; ul_cnt++)
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "nvram_params %d{%d %d 0x%x}",
                            ul_cnt,
                            pst_cfg_nvram[ul_cnt].uc_index,
                            pst_cfg_nvram[ul_cnt].uc_max_txpower,
                            pst_cfg_nvram[ul_cnt].us_dbb_scale);
        }
    }
#endif

    return OAL_SUCC;
}
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */


OAL_STATIC oal_uint32  dmac_protection_update_from_user(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32                         ul_ret = OAL_SUCC;
    mac_h2d_protection_stru           *pst_h2d_prot;

    pst_h2d_prot = (mac_h2d_protection_stru*)puc_param;

    oal_memcopy((oal_uint8*)&pst_mac_vap->st_protection, (oal_uint8*)&pst_h2d_prot->st_protection,
                OAL_SIZEOF(mac_protection_stru));

    mac_mib_set_HtProtection(pst_mac_vap, pst_h2d_prot->en_dot11HTProtection);
    mac_mib_set_RifsMode(pst_mac_vap, pst_h2d_prot->en_dot11RIFSMode);
    mac_mib_set_LsigTxopFullProtectionActivated(pst_mac_vap, pst_h2d_prot->en_dot11LSIGTXOPFullProtectionActivated);
    mac_mib_set_NonGFEntitiesPresent(pst_mac_vap, pst_h2d_prot->en_dot11NonGFEntitiesPresent);

    if(WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        ul_ret = dmac_protection_update_mib_ap(pst_mac_vap);
    }
    //else if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    //{
    //    ul_ret = dmac_set_protection_mode(pst_mac_vap, WLAN_PROT_NO);
    //}

    return ul_ret;
}

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST

OAL_STATIC oal_uint32  dmac_40m_intol_update_ap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_bandwidth_stru          *pst_band_prot;
    dmac_vap_stru               *pst_dmac_vap;
    mac_device_stru             *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_band_prot = (mac_bandwidth_stru*)puc_param;
    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_2040, "{dmac_40m_intol_update_ap::vap mode=%d,current bw=%d,updated bw=%d,en_40M_intol_user=%d}",
                     pst_mac_vap->en_vap_mode,
                     pst_mac_vap->st_channel.en_bandwidth,
                     pst_band_prot->en_40M_bandwidth,
                     pst_band_prot->en_40M_intol_user);

    /*只有AP需要处理*/
    if(WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)
    {
        return OAL_SUCC;
    }

    if(WLAN_BAND_WIDTH_BUTT != pst_band_prot->en_40M_bandwidth)
    {
        pst_dmac_vap->en_40M_bandwidth = pst_band_prot->en_40M_bandwidth;

        /*hostapd配置的40m切换成20m需要开启定时器*/
        if(((WLAN_BAND_WIDTH_40PLUS == pst_dmac_vap->en_40M_bandwidth)
            ||(WLAN_BAND_WIDTH_40MINUS == pst_dmac_vap->en_40M_bandwidth))
            &&(WLAN_BAND_WIDTH_20M == pst_mac_vap->st_channel.en_bandwidth))
        {
            dmac_chan_start_40M_recovery_timer(pst_dmac_vap);
        }
        /*hostapd配置20m 关闭定时器*/
        if(WLAN_BAND_WIDTH_20M == pst_dmac_vap->en_40M_bandwidth)
        {
            dmac_chan_stop_40M_recovery_timer(pst_dmac_vap);
        }
    }

    if(OAL_TRUE == pst_band_prot->en_40M_intol_user)
    {
        mac_mib_set_FortyMHzIntolerant(pst_mac_vap, OAL_TRUE);
        dmac_chan_start_40M_recovery_timer(pst_dmac_vap);
        if ((WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth)
            ||(WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth))
        {
            /* AP准备切换置20MHz运行 */
            dmac_chan_multi_switch_to_20MHz_ap(pst_dmac_vap);
        }
        else
        {
            pst_mac_device->en_40MHz_intol_bit_recd = OAL_TRUE;
        }
    }

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL

oal_uint32  dmac_config_get_hipkt_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_get_hipkt_stat();

    return OAL_SUCC;
}
#endif
/*****************************************************************************
    HMAC到DMAC配置同步事件操作函数表
*****************************************************************************/
OAL_STATIC dmac_config_syn_stru g_ast_dmac_config_syn[] =
{
    /* 同步ID                    保留2个字节            函数操作 */
    {WLAN_CFGID_BSS_TYPE,           {0, 0},         dmac_config_set_bss_type},
    {WLAN_CFGID_ADD_VAP,            {0, 0},         dmac_config_add_vap},
    {WLAN_CFGID_START_VAP,          {0, 0},         dmac_config_start_vap},
    {WLAN_CFGID_DEL_VAP,            {0, 0},         dmac_config_del_vap},
    {WLAN_CFGID_DOWN_VAP,           {0, 0},         dmac_config_down_vap},
    {WLAN_CFGID_MODE,               {0, 0},         dmac_config_set_mode},
    {WLAN_CFGID_CURRENT_CHANEL,     {0, 0},         dmac_config_set_freq},
    {WLAN_CFGID_STATION_ID,         {0, 0},         dmac_config_set_mac_addr},
    {WLAN_CFGID_SSID,               {0, 0},         dmac_config_set_ssid},
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    {WLAN_CFGID_GET_HIPKT_STAT,     {0, 0},         dmac_config_get_hipkt_stat},
#endif
    {WLAN_CFGID_VAP_STATE_SYN,      {0, 0},         dmac_config_vap_state_syn},
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_SHORTGI,            {0, 0},         dmac_config_set_shortgi},   /* hi1102-cb add sync */
    {WLAN_CFGID_REGDOMAIN_PWR,      {0, 0},         dmac_config_set_regdomain_pwr},
#endif
    {WLAN_CFGID_SCAN_ABORT,         {0, 0},         dmac_config_scan_abort},
    {WLAN_CFGID_STOP_SCHED_SCAN,    {0, 0},         dmac_config_stop_sched_scan},
    {WLAN_CFGID_USER_ASOC_STATE_SYN,{0, 0},         dmac_config_user_asoc_state_syn},
    {WLAN_CFGID_USER_RATE_SYN,      {0, 0},         dmac_config_user_rate_info_syn},
    {WLAN_CFGID_USR_INFO_SYN,       {0, 0},         dmac_config_sta_usr_info_syn},
    {WLAN_CFGID_STA_VAP_INFO_SYN,   {0, 0},         dmac_config_vap_info_syn},
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_SYNC_CH_STATUS,     {0, 0},         dmac_config_ch_status_sync},
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    {WLAN_CFGID_VAP_M2S_INFO_SYN,   {0, 0},         dmac_config_vap_m2s_info_syn},
#endif

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    {WLAN_CFGID_STA_TXOP_AID,         {0, 0},      dmac_txopps_set_machw_partialaid},
#endif

    {WLAN_CFGID_SHORT_PREAMBLE,     {0, 0},         dmac_config_set_shpreamble},
#ifdef _PRE_WLAN_FEATURE_MONITOR
    {WLAN_CFGID_ADDR_FILTER,        {0, 0},         dmac_config_set_addr_filter},
#endif
    {WLAN_CFGID_PROT_MODE,          {0, 0},         dmac_config_set_prot_mode},
    {WLAN_CFGID_BEACON_INTERVAL,    {0, 0},         dmac_config_set_bintval},
    {WLAN_CFGID_NO_BEACON,          {0, 0},         dmac_config_set_nobeacon},
    {WLAN_CFGID_DTIM_PERIOD,        {0, 0},         dmac_config_set_dtimperiod},
#if 0
    {WLAN_CFGID_OTA_SWITCH,         {0, 0},         dmac_config_ota_switch},
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_TX_POWER,           {0, 0},         dmac_config_set_txpower},
    {WLAN_CFGID_WFA_CFG_AIFSN,         {0, 0},      dmac_config_wfa_cfg_aifsn},
    {WLAN_CFGID_WFA_CFG_CW,         {0, 0},         dmac_config_wfa_cfg_cw},
    {WLAN_CFGID_CHECK_LTE_GPIO,         {0, 0},     dmac_config_lte_gpio_mode},
#endif /* DMAC_OFFLOAD */


#ifdef _PRE_WLAN_FEATURE_UAPSD
    {WLAN_CFGID_UAPSD_EN,           {0, 0},         dmac_config_set_uapsden},
    {WLAN_CFGID_UAPSD_UPDATE,       {0, 0},         dmac_config_set_uapsd_update},
#endif
    {WLAN_CFGID_EDCA_TABLE_CWMIN,           {0, 0},         dmac_config_set_cwmin},
    {WLAN_CFGID_EDCA_TABLE_CWMAX,           {0, 0},         dmac_config_set_cwmax},
    {WLAN_CFGID_EDCA_TABLE_AIFSN,           {0, 0},         dmac_config_set_aifsn},
    {WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT,      {0, 0},         dmac_config_set_txop_limit},
    //{WLAN_CFGID_EDCA_TABLE_MSDU_LIFETIME,   {0, 0},         dmac_config_set_msdu_lifetime},
    {WLAN_CFGID_EDCA_TABLE_MANDATORY,       {0, 0},         dmac_config_set_edca_mandatory},
    {WLAN_CFGID_QEDCA_TABLE_CWMIN,          {0, 0},         dmac_config_set_qap_cwmin},
    {WLAN_CFGID_QEDCA_TABLE_CWMAX,          {0, 0},         dmac_config_set_qap_cwmax},
    {WLAN_CFGID_QEDCA_TABLE_AIFSN,          {0, 0},         dmac_config_set_qap_aifsn},
    {WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT,     {0, 0},         dmac_config_set_qap_txop_limit},
    {WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME,  {0, 0},         dmac_config_set_qap_msdu_lifetime},
    {WLAN_CFGID_QEDCA_TABLE_MANDATORY,      {0, 0},         dmac_config_set_qap_edca_mandatory},

    {WLAN_CFGID_VAP_INFO,                   {0, 0},         dmac_config_vap_info},
    {WLAN_CFGID_ADD_USER,                   {0, 0},         dmac_config_add_user},
    {WLAN_CFGID_DEL_USER,                   {0, 0},         dmac_config_del_user},
    {WLAN_CFGID_DEL_MULTI_USER,             {0, 0},         dmac_config_del_multi_user},
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    {WLAN_CFGID_BTCOEX_STATUS_PRINT,        {0, 0},         dmac_config_print_btcoex_status},
    {WLAN_CFGID_BTCOEX_PREEMPT_TYPE,        {0, 0},         dmac_config_btcoex_preempt_type},
    {WLAN_CFGID_BTCOEX_SET_PERF_PARAM,      {0, 0},         dmac_config_btcoex_set_perf_param},
#endif
#ifdef _PRE_WLAN_FEATURE_LTECOEX
    {WLAN_CFGID_LTECOEX_MODE_SET,           {0, 0},         dmac_config_ltecoex_mode_set},
#endif

    {WLAN_CFGID_SET_LOG_LEVEL,              {0, 0},         dmac_config_set_log_level},
#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    {WLAN_CFGID_GREEN_AP_EN,                {0, 0},         dmac_config_set_green_ap_en},
#endif

#ifdef _PRE_WLAN_FEATURE_AP_PM
    {WLAN_CFGID_STA_SCAN_CONNECT,         {0, 0},           dmac_pm_ap_sta_scan_connect_event},
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_SET_LOG_PM,                 {0, 0},         dmac_config_set_log_lowpower},
    {WLAN_CFGID_SET_PM_SWITCH,              {0, 0},         dmac_config_set_pm_switch},
#ifdef _PRE_WLAN_FEATURE_SMARTANT
    {WLAN_CFGID_SET_ANT,              {0, 0},         dmac_config_dual_antenna_set_ant},
    {WLAN_CFGID_GET_ANT_INFO,         {0, 0},         dmac_config_get_ant_info},
    {WLAN_CFGID_DOUBLE_ANT_SW,        {0, 0},         dmac_config_double_ant_switch},
#endif
    {WLAN_CFGID_GET_ANT,              {0, 0},         dmac_config_get_ant},
#endif
    /*wpa-wpa2*/
    {WLAN_CFGID_ADD_KEY,           {0, 0},              dmac_config_11i_add_key},
    //{WLAN_CFGID_ADD_KEY_KEY,       {0, 0},              dmac_config_11i_add_key},
    //{WLAN_CFGID_ADD_KEY_SEQ,       {0, 0},              dmac_config_11i_add_key_seq},
    {WLAN_CFGID_REMOVE_KEY,        {0, 0},              dmac_config_11i_remove_key},
    {WLAN_CFGID_DEFAULT_KEY,       {0, 0},              dmac_config_11i_set_default_key},
    //{WLAN_CFGID_ADD_WEP_KEY,       {0, 0},              dmac_config_wep_add_key},
    {WLAN_CFGID_ADD_WEP_ENTRY,     {0, 0},              dmac_config_wep_add_entry},
    {WLAN_CFGID_REMOVE_WEP_KEY,    {0, 0},              dmac_config_wep_remove_key},
    {WLAN_CFGID_CONNECT_REQ,        {0, 0},             dmac_config_connect},

#ifdef _PRE_WLAN_FEATURE_WAPI
    {WLAN_CFGID_ADD_WAPI_KEY,       {0, 0},             dmac_config_wapi_add_key},
#endif

    {WLAN_CFGID_INIT_SECURTIY_PORT, {0, 0},             dmac_config_11i_init_port},
    {WLAN_CFGID_DUMP_ALL_RX_DSCR,  {0, 0},              dmac_config_dump_all_rx_dscr},
    {WLAN_CFGID_USER_INFO,         {0, 0},              dmac_config_user_info},
    {WLAN_CFGID_SET_DSCR,          {0, 0},              dmac_config_set_dscr},
    {WLAN_CFGID_COUNTRY,           {0, 0},              dmac_config_set_country},
    {WLAN_CFGID_SET_RANDOM_MAC_OUI, {0, 0},             dmac_config_set_random_mac_oui},
#ifdef _PRE_WLAN_FEATURE_DFS
    {WLAN_CFGID_COUNTRY_FOR_DFS,   {0, 0},              dmac_config_set_country_for_dfs},
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    {WLAN_CFGID_SET_RATE,          {0, 0},              dmac_config_set_rate},
    {WLAN_CFGID_SET_MCS,           {0, 0},              dmac_config_set_mcs},
    {WLAN_CFGID_SET_MCSAC,         {0, 0},              dmac_config_set_mcsac},
    {WLAN_CFGID_SET_BW,            {0, 0},              dmac_config_set_bw},
#endif

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    {WLAN_CFGID_SET_ALWAYS_TX,           {0, 0},        dmac_config_set_always_tx},
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    {WLAN_CFGID_SET_ALWAYS_TX_51,        {0, 0},        dmac_config_set_always_tx_51},
#endif
    {WLAN_CFGID_SET_ALWAYS_TX_NUM,       {0, 0},        dmac_config_set_always_tx_num},
    {WLAN_CFGID_SET_ALWAYS_TX_HW_CFG,    {0, 0},        dmac_config_set_always_tx_hw_cfg},
    {WLAN_CFGID_SET_ALWAYS_TX_HW,        {0, 0},        dmac_config_set_always_tx_hw},
#endif

    {WLAN_CFGID_SET_TX_POW,        {0, 0},              dmac_config_set_tx_pow},

#ifdef _PRE_WLAN_NARROW_BAND
    {WLAN_CFGID_NARROW_BW,        {0, 0},               dmac_config_set_narrow_bw},
#endif

    {WLAN_CFGID_SET_ALWAYS_RX,     {0, 0},              dmac_config_set_always_rx},
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    {WLAN_CFGID_SET_ALWAYS_RX_51,  {0, 0},              dmac_config_set_always_rx_51},
#endif
    {WLAN_CFGID_PCIE_PM_LEVEL,     {0, 0},              dmac_config_pcie_pm_level},
    {WLAN_CFGID_REG_INFO,          {0, 0},              dmac_config_reg_info},
    {WLAN_CFGID_GET_BG_NOISE,      {0, 0},              dmac_config_bg_noise_info},

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
    {WLAN_CFGID_SDIO_FLOWCTRL,     {0, 0},              dmac_config_sdio_flowctrl},
#endif
    {WLAN_CFGID_REG_WRITE,         {0, 0},              dmac_config_reg_write},
#ifdef _PRE_WLAN_ONLINE_DPD

    {WLAN_CFGID_DPD,         {0, 0},                    dmac_config_dpd},
#endif
    {WLAN_CFGID_CFG80211_SET_CHANNEL,      {0, 0},      dmac_config_set_channel},
    {WLAN_CFGID_CFG80211_SET_MIB_BY_BW,    {0, 0},      dmac_config_set_mib_by_bw},

    {WLAN_CFGID_CFG80211_CONFIG_BEACON,    {0, 0},      dmac_config_set_beacon},

#if 0
    {WLAN_CFGID_TDLS_PROHI,                {0, 0},      dmac_config_tdls_prohibited},
    {WLAN_CFGID_TDLS_CHASWI_PROHI,         {0, 0},      dmac_config_tdls_channel_switch_prohibited},
#endif
    {WLAN_CFGID_WMM_SWITCH,                {0, 0},      dmac_config_wmm_switch},

#ifdef _PRE_SUPPORT_ACS
    {WLAN_CFGID_ACS_PARAM,          {0, 0},             dmac_acs_recv_msg},
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    {WLAN_CFGID_RADARTOOL,          {0, 0},             dmac_config_dfs_radartool},
#endif

    {WLAN_CFGID_PROTECTION_UPDATE_STA_USER,     {0, 0},            dmac_protection_update_from_user},
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    {WLAN_CFGID_40M_INTOL_UPDATE,     {0, 0},            dmac_40m_intol_update_ap},
#endif
    {WLAN_CFGID_GET_VERSION,         {0, 0},            dmac_config_get_version},
    {WLAN_CFGID_RX_FCS_INFO,         {0, 0},            dmac_config_rx_fcs_info},

#ifdef _PRE_WLAN_FEATURE_SMPS
    {WLAN_CFGID_SET_SMPS,            {0, 0},            dmac_config_set_smps_mode},
//    {WLAN_CFGID_GET_SMPS_EN,         {0, 0},            dmac_config_get_smps_mode_en},
    {WLAN_CFGID_SET_VAP_SMPS,        {0, 0},            dmac_config_set_vap_smps_mode},
#endif

    {WLAN_CFGID_SET_WPS_P2P_IE,    {0, 0},              dmac_config_set_app_ie_to_vap},
    {WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL,  {0, 0},  dmac_config_cancel_remain_on_channel},
#ifdef _PRE_WLAN_FEATURE_STA_PM
    {WLAN_CFGID_SET_PS_MODE,         {0, 0},                 dmac_config_set_sta_ps_mode},  /* hi1102 pspoll配置命令 */
#ifdef _PRE_PSM_DEBUG_MODE
    {WLAN_CFGID_SHOW_PS_INFO,        {0, 0},                 dmac_show_ps_info},/* sta psm 维测统计信息 */
#endif
    {WLAN_CFGID_SET_PSM_PARAM,       {0, 0},                 dmac_set_psm_param},/* sta psm tbtt offset/listen interval 设置 */
    {WLAN_CFGID_SET_STA_PM_ON,        {0, 0},                dmac_config_set_sta_pm_on},/* sta psm tbtt offset/listen interval 设置 */

#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    {WLAN_CFGID_SET_UAPSD_PARA,        {0, 0},              dmac_config_set_uapsd_para},
#endif
#endif

    {WLAN_CFGID_QUERY_STATION_STATS, {0, 0},                dmac_config_proc_query_sta_info_event},
    {WLAN_CFGID_QUERY_RSSI,         {0, 0},                 dmac_config_query_rssi},
    {WLAN_CFGID_QUERY_PSST,         {0, 0},                 dmac_config_query_psst},
#ifdef _PRE_WLAN_WEB_CMD_COMM
#ifdef _PRE_WLAN_11K_STAT
    {WLAN_CFGID_QUERY_DROP_NUM,     {0, 0},                 dmac_config_query_drop_num},
    {WLAN_CFGID_QUERY_TX_DELAY,     {0, 0},                 dmac_config_query_tx_delay},
#endif
#endif
    {WLAN_CFGID_QUERY_RATE,         {0, 0},                 dmac_config_query_rate},

#ifdef _PRE_WLAN_DFT_STAT
    {WLAN_CFGID_QUERY_ANI,          {0, 0},                 dmac_config_query_ani},
#endif

#ifdef _PRE_WLAN_FEATURE_P2P
    {WLAN_CFGID_SET_P2P_PS_OPS,     {0, 0},                 dmac_config_set_p2p_ps_ops},
    {WLAN_CFGID_SET_P2P_PS_NOA,     {0, 0},                 dmac_config_set_p2p_ps_noa},
#endif

    {WLAN_CFGID_NSS,                {0, 0},             dmac_config_nss},
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    {WLAN_CFGID_PROXYSTA_SWITCH,     {0, 0},            dmac_config_proxysta_switch},
#endif

    {WLAN_CFGID_USER_CAP_SYN,       {0, 0},             dmac_config_user_cap_syn},
#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    {WLAN_CFGID_SET_IP_ADDR,                {0, 0},     dmac_config_set_ip_addr},
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
    {WLAN_CFGID_SET_ROAMING_MODE,       {0, 0},         dmac_config_roam_enable},
    {WLAN_CFGID_SET_ROAM_TRIGGER,       {0, 0},         dmac_config_set_roam_trigger},
    {WLAN_CFGID_ROAM_HMAC_SYNC_DMAC,    {0, 0},         dmac_config_roam_hmac_sync_dmac},
    {WLAN_CFGID_ROAM_NOTIFY_STATE,      {0, 0},         dmac_config_roam_notify_state},
    {WLAN_CFGID_ROAM_SUCC_H2D_SYNC,     {0, 0},         dmac_config_roam_succ_h2d_syn},
#endif //_PRE_WLAN_FEATURE_ROAM
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    {WLAN_CFGID_SET_TXRX_CHAIN,         {0, 0},         dmac_config_set_txrx_chain},
#ifdef _PRE_WLAN_RF_CALI
    {WLAN_CFGID_SET_2G_TXRX_PATH,       {0, 0},         dmac_config_set_2g_txrx_path},
#endif
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    {WLAN_CFGID_SUSPEND_ACTION_SYN,        {0, 0},      dmac_config_suspend_action_sync},
    {WLAN_CFGID_SET_DEVICE_PKT_STAT,         {0, 0},         dmac_config_set_pkts_stat},
    {WLAN_CFGID_SET_DEVICE_MEM_FREE_STAT,    {0, 0},         dmac_config_set_mem_stat},


#endif
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    {WLAN_CFGID_SET_DEVICE_FREQ,   {0, 0},         dmac_config_set_device_freq},
#endif

    {WLAN_CFGID_SET_RX_AMPDU_AMSDU,   {0, 0},      dmac_config_set_rx_ampdu_amsdu},

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#ifdef _PRE_WLAN_FIT_BASED_REALTIME_CALI
    {WLAN_CFGID_SET_CUS_DYN_CALI_PARAM,     {0, 0},       dmac_config_set_cus_dyn_cali},
#endif
    {WLAN_CFGID_LOAD_INI_PWR_GAIN,          {0, 0},       dmac_config_load_ini_power_gain},
    {WLAN_CFGID_SET_ALL_LOG_LEVEL,          {0, 0},       dmac_config_set_all_log_level},
    {WLAN_CFGID_SET_CUS_RF,                 {0, 0},       dmac_config_set_cus_rf},
    {WLAN_CFGID_SET_CUS_DTS_CALI,           {0, 0},       dmac_config_set_cus_dts_cali},
    {WLAN_CFGID_SET_CUS_NVRAM_PARAM,        {0, 0},       dmac_config_set_cus_nvram_params},
    /* show customize info */
    {WLAN_CFGID_SHOW_DEV_CUSTOMIZE_INFOS,   {0, 0},       dmac_config_customize_info},
#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    {WLAN_CFGID_VOWIFI_INFO,                {0, 0},       dmac_config_vowifi_info},
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#if defined(_PRE_WLAN_FEATURE_EQUIPMENT_TEST) && (defined _PRE_WLAN_FIT_BASED_REALTIME_CALI)
    {WLAN_CFGID_CALI_POWER,                  {0, 0},         dmac_config_cali_power},
    {WLAN_CFGID_GET_CALI_POWER,              {0, 0},         dmac_config_get_cali_power},
    {WLAN_CFGID_SET_POLYNOMIAL_PARA,         {0, 0},         dmac_config_set_polynomial_param},
    {WLAN_CFGID_GET_UPC_PARA,                {0, 0},         dmac_config_get_upc_params},
    {WLAN_CFGID_SET_UPC_PARA,                {0, 0},         dmac_config_set_upc_params},
    {WLAN_CFGID_SET_LOAD_MODE,               {0, 0},         dmac_config_set_load_mode},
#endif

    {WLAN_CFGID_GET_DIEID,                  {0, 0},         dmac_config_get_dieid},
#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
    {WLAN_CFGID_AUTO_CALI,                  {0, 0},         dmac_config_auto_cali},
    {WLAN_CFGID_SET_CALI_VREF,              {0, 0},         dmac_config_set_cali_vref},
#endif
#ifdef _PRE_WLAN_FEATURE_11K
    {WLAN_CFGID_BCN_TABLE_SWITCH,           {0, 0},     dmac_config_bcn_table_switch},
#endif
    {WLAN_CFGID_VOE_ENABLE,                 {0, 0},     dmac_config_voe_enable},
#ifdef _PRE_WLAN_FEATURE_TPC
    {WLAN_CFGID_REDUCE_SAR,                 {0, 0},     dmac_config_reduce_sar},
#endif
#ifdef _PRE_WLAN_WEB_CMD_COMM
    {WLAN_CFGID_GET_TEMP,                   {0, 0},     dmac_config_get_temp},
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
    {WLAN_CFGID_GET_BSD,                    {0, 0},     dmac_config_get_bsd},
#endif
    {WLAN_CFGID_GET_KO_VERSION,             {0, 0},     dmac_config_get_ko_version},
    {WLAN_CFGID_GET_PWR_REF,                {0, 0},     dmac_config_get_pwr_ref},
    {WLAN_CFGID_GET_BCAST_RATE,             {0, 0},     dmac_config_get_bcast_rate},
#endif
#ifdef _PRE_FEATURE_WAVEAPP_CLASSIFY
    {WLAN_CFGID_GET_WAVEAPP_FLAG,           {0, 0},     dmac_config_get_waveapp_flag},
#endif
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    {WLAN_CFGID_SYNC_PK_MODE,               {0, 0},     dmac_config_pk_mode},
#endif
    {WLAN_CFGID_BUTT,                          {0, 0},     OAL_PTR_NULL},
};

#ifdef _PRE_WLAN_WEB_CMD_COMM

oal_uint32  dmac_config_get_temp(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                  *pst_dmac_vap;
    oal_int32                           *pl_param;
#ifndef WIN32
    hal_to_dmac_device_stru     *pst_hal_device;
#ifdef _PRE_WLAN_PRODUCT_1151V200
    oal_int16                           s_temp;
#else
    oal_int8                           c_temp;
#endif
#endif

    pl_param = (oal_int32 *)puc_param;
    pst_dmac_vap = (dmac_vap_stru *)MAC_GET_DMAC_VAP(pst_mac_vap);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_config_proc_query_sta_info_event::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }


    /* 读取温度码字 */
#ifndef WIN32
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
#ifdef _PRE_WLAN_PRODUCT_1151V200
    hal_get_rf_temp_tsens(pst_hal_device,  &s_temp);
    *pl_param = s_temp;
#else
    hal_get_rf_temp(pst_hal_device, (oal_uint8*)&c_temp);
    switch(c_temp)
    {
        case 0:     *pl_param =-20;        break;
        case 1:     *pl_param = -7;        break;
        case 2:     *pl_param = 18;        break;
        case 3:     *pl_param = 43;        break;
        case 4:     *pl_param = 67;        break;
        case 5:     *pl_param = 93;        break;
        case 6:     *pl_param = 115;       break;
        case 7:     *pl_param = 125;       break;

        default:    *pl_param = 0;         break;
    }
#endif
#else
    *pl_param = 0;
#endif

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_TEMP, OAL_SIZEOF(oal_int32), (oal_uint8 *)pl_param);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING

oal_uint32  dmac_config_get_bsd(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_query_bsd_stru      st_param;

    st_param.uc_bsd = dmac_bsd_get_capability();
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_BSD, OAL_SIZEOF(mac_cfg_query_bsd_stru), (oal_uint8 *)&st_param);

    return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  dmac_config_get_ko_version(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru     *pst_hal_device;
    oal_uint32                   ul_hw_version = 0;
    oal_uint32                   ul_hw_version_data = 0;
    oal_uint32                   ul_hw_version_num = 0;
    oal_uint8                   *puc_sw_version = (oal_uint8 *)WITP_BUILD_VERSION_ENV;//lint !e122
    oal_int8                    *pac_tmp_buff;
    mac_cfg_param_char_stru      st_param;
    oal_uint32                   ul_len;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_ko_version::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pac_tmp_buff = oal_memalloc(DMAC_MAX_VERSION_SIZE);
    if (!pac_tmp_buff)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取FPGA版本 */
    hal_get_hw_version(pst_hal_device, &ul_hw_version, &ul_hw_version_data, &ul_hw_version_num);

    OAL_SPRINTF((char*)pac_tmp_buff, DMAC_MAX_VERSION_SIZE, "Software Version: %s. \nFPGA Version: %04x-%04x-%02x.\n", puc_sw_version, ul_hw_version, ul_hw_version_data, ul_hw_version_num);
    oam_print(pac_tmp_buff);
    oal_free(pac_tmp_buff);

    ul_len = OAL_STRLEN((oal_int8 *)puc_sw_version);

    // trim
    if ((ul_len >=1) && (ul_len >= OAL_SIZEOF(st_param.auc_buff)))
    {
        ul_len = OAL_SIZEOF(st_param.auc_buff) - 1;
    }
    oal_memcopy(st_param.auc_buff, puc_sw_version, ul_len);
    st_param.l_buff_len = (oal_int32)ul_len;
    st_param.auc_buff[ul_len] = 0;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_KO_VERSION, OAL_SIZEOF(mac_cfg_param_char_stru), (oal_uint8 *)&st_param);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_get_bcast_rate(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru             *pst_hal_device;
    dmac_vap_stru                       *pst_dmac_vap;
    mac_cfg_param_char_stru             st_param;
    hal_statistic_stru                  st_per_rate;
    oal_uint32                          ul_data_rate;
    oal_uint32                          ul_ret;
    hal_tx_txop_per_rate_params_union   *pst_per_rate;
    wlan_phy_protocol_enum_uint8        en_protocol_mode;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_bcast_rate::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)MAC_GET_DMAC_VAP(pst_mac_vap);


    pst_per_rate = &(pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0]);
    en_protocol_mode = pst_per_rate->rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode;
    if(WLAN_HT_PHY_PROTOCOL_MODE == en_protocol_mode)
    {
        st_per_rate.un_nss_rate.st_ht_rate.bit_ht_mcs        = pst_per_rate->rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs;
        st_per_rate.un_nss_rate.st_ht_rate.bit_protocol_mode = pst_per_rate->rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode;
    }
    else if(WLAN_VHT_PHY_PROTOCOL_MODE == en_protocol_mode)
    {
        st_per_rate.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs       = pst_per_rate->rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs;
        st_per_rate.un_nss_rate.st_vht_nss_mcs.bit_nss_mode      = pst_per_rate->rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_nss_mode;
        st_per_rate.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode = pst_per_rate->rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode;
    }
    else
    {
        st_per_rate.un_nss_rate.st_legacy_rate.bit_legacy_rate   = pst_per_rate->rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate;
        st_per_rate.un_nss_rate.st_legacy_rate.bit_protocol_mode = pst_per_rate->rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
    }
    st_per_rate.uc_short_gi      = pst_per_rate->rate_bit_stru.bit_short_gi_enable;
    st_per_rate.bit_preamble     = pst_per_rate->rate_bit_stru.bit_preamble_mode;
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV))
    st_per_rate.bit_stbc         = pst_per_rate->rate_bit_stru.bit_stbc_mode;
#endif
    st_per_rate.bit_channel_code = pst_dmac_vap->st_tx_data_mcast.st_rate.en_channel_code;
    st_per_rate.uc_bandwidth     = pst_dmac_vap->st_tx_data_mcast.st_rate.en_channel_bandwidth;

    ul_ret = dmac_alg_get_rate_kbps(pst_hal_device, &st_per_rate, &ul_data_rate);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_get_bcast_rate::get rate failed.}");
        return ul_ret;
    }
    OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_config_get_bcast_rate::ul_data_rate=%d.}",ul_data_rate);

    st_param.l_buff_len = OAL_SIZEOF(oal_uint32);
    ul_data_rate = ul_data_rate/1000;
    st_param.auc_buff[0] = ul_data_rate & 0xFF;
    st_param.auc_buff[1] = (ul_data_rate >> 8) & 0xFF;
    st_param.auc_buff[2] = (ul_data_rate >> 16) & 0xFF;
    st_param.auc_buff[3] = (ul_data_rate >> 24) & 0xFF;

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_BCAST_RATE, OAL_SIZEOF(mac_cfg_param_char_stru), (oal_uint8 *)&st_param);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_config_get_pwr_ref(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_param_char_stru         st_param;
    oal_int32                       l_cnt;
    oal_int32                       l_ret;

    l_ret = OAL_MIN(HAL_POW_RATE_POW_CODE_TABLE_LEN, WLAN_IWPRIV_MAX_BUFF_LEN);
    OAL_MEMZERO(st_param.auc_buff, WLAN_IWPRIV_MAX_BUFF_LEN);
    for (l_cnt = 0; l_cnt < l_ret; l_cnt++)
    {
        st_param.auc_buff[l_cnt] = ((g_auc_rate_pow_table_margin[l_cnt][pst_mac_vap->st_channel.en_band])/10);
    }
    st_param.l_buff_len = l_ret;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_PWR_REF, OAL_SIZEOF(mac_cfg_param_char_stru), (oal_uint8 *)&st_param);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_FEATURE_WAVEAPP_CLASSIFY

oal_uint32  dmac_config_get_waveapp_flag(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_bool_enum_uint8     en_flag;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

    hal_to_dmac_device_stru             *pst_hal_device;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_waveapp_flag::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    en_flag = pst_hal_device->en_test_is_on_waveapp_flag;
#else
    en_flag = OAL_FALSE;
#endif
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_WAVEAPP_FLAG, OAL_SIZEOF(oal_bool_enum_uint8), (oal_uint8 *)&en_flag);
    return OAL_SUCC;
}
#endif


#ifdef _PRE_WLAN_ONLINE_DPD
oal_uint32 dmac_dpd_to_hmac(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;

    pst_event               = frw_get_event_stru(pst_event_mem);
    pst_event_hdr           = &(pst_event->st_event_hdr);
    OAM_ERROR_LOG0(0, 0, "dmac_dpd_to_hmac");

    //OAL_IO_PRINT("dmac_cali_to_hmac start\r\n");
    FRW_EVENT_HDR_MODIFY_PIPELINE_AND_SUBTYPE(pst_event_hdr, DMAC_TO_HMAC_DPD);
    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);

    return OAL_SUCC;
}


oal_uint32  dmac_cali_corram_recv(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    oal_netbuf_stru            *pst_cali_data_netbuf;
    oal_uint8                  *puc_cali_data;
    dmac_tx_event_stru         *pst_dtx_event;

    OAL_STATIC oal_uint8        uc_pkt_num = 0;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CALIBRATE, "{dmac_cali_hmac2dmac_recv::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event           = frw_get_event_stru(pst_event_mem);
    pst_dtx_event       = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_cali_data_netbuf = pst_dtx_event->pst_netbuf;

    puc_cali_data = (oal_uint8 *)OAL_NETBUF_DATA(pst_cali_data_netbuf);

    // write dpd data back to register
    if (uc_pkt_num != 1)
    {
        uc_pkt_num++;
        hal_corram_write_func(puc_cali_data, pst_dtx_event->us_frame_len, pst_dtx_event->us_remain);
    }
    else
    {
        uc_pkt_num++;
        hal_corram_write_func(puc_cali_data, pst_dtx_event->us_frame_len, pst_dtx_event->us_remain);
        uc_pkt_num = 0;
    }

    oal_netbuf_free(pst_cali_data_netbuf);
    return OAL_SUCC;
}


#endif

oal_void  dmac_config_reg_write_test16(mac_vap_stru *pst_mac_vap, dmac_reg_write_stru *pst_reg_write)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_int8                        ac_buf[64] = {0};

    if (pst_reg_write->ul_addr % 2 != 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_config_reg_write_test::ul_addr=%d", pst_reg_write->ul_addr);
        return;
    }

    /*lint -save -e516 */
    /*lint -save -e515 */
    OAL_SPRINTF((char*)ac_buf, OAL_SIZEOF(ac_buf), "reg_write:addr=0x%08x, val=0x%04x.\n", pst_reg_write->ul_addr, pst_reg_write->ul_val);
    /*lint -restore */
    /*lint -restore */

    OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_config_reg_write_test::reg_write:addr=0x%08x, val=0x%04x.", pst_reg_write->ul_addr, pst_reg_write->ul_val);
    oam_print(ac_buf);

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_reg_write_test::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) && (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION) && (!defined(_PRE_PC_LINT))
#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
    if((0 == oal_strcmp(pst_reg_write->pc_reg_type, "ssi0"))
        || (0 == oal_strcmp(pst_reg_write->pc_reg_type, "ssi1")))
    {
        OAL_REG_WRITE16(pst_reg_write->ul_addr, (oal_uint16)pst_reg_write->ul_val);
        return;
    }
#else
    if(0 == oal_strcmp(pst_reg_write->pc_reg_type, "ssi0"))
    {
        ssi_write_reg(HI1103_B100_SSI0_MST_RB_BASE_ADDR, pst_reg_write->ul_addr, (oal_uint16)pst_reg_write->ul_val);
        return;
    }
    if(0 == oal_strcmp(pst_reg_write->pc_reg_type, "ssi1"))
    {
    #ifdef _PRE_WLAN_1103_RF
        ssi_write_reg(HI1103_B100_SSI0_MST_RB_BASE_ADDR, pst_reg_write->ul_addr, (oal_uint16)pst_reg_write->ul_val);
    #else
        ssi_write_reg(HI1103_B100_SSI1_MST_RB_BASE_ADDR, pst_reg_write->ul_addr, (oal_uint16)pst_reg_write->ul_val);
    #endif
        return;
    }
#endif
#endif

    hal_reg_write16(pst_hal_device, pst_reg_write->ul_addr, (oal_uint16)pst_reg_write->ul_val);

}

oal_void  dmac_config_reg_display_test16(mac_vap_stru *pst_mac_vap, dmac_reg_info_stru *pst_reg_info)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint32                      ul_addr;
    oal_uint16                      us_val = 0;

    if (pst_reg_info->ul_start_addr % 2 != 0 || pst_reg_info->ul_end_addr % 2 != 0)
    {
        OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_config_reg_display_test16::not mod 2, start[%08x], end[%08x].", pst_reg_info->ul_start_addr, pst_reg_info->ul_end_addr);
        return;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_reg_display_test16::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return;
    }

    for (ul_addr = pst_reg_info->ul_start_addr; ul_addr <= pst_reg_info->ul_end_addr; ul_addr += 2)
    {
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) && (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION) && (!defined(_PRE_PC_LINT))
#if (_PRE_WLAN_CHIP_ASIC == _PRE_WLAN_CHIP_VERSION)
        if ((0 == oal_strcmp(pst_reg_info->pc_reg_type, "ssi0"))
            || (0 == oal_strcmp(pst_reg_info->pc_reg_type, "ssi1")))
        {
            us_val =(oal_uint16)OAL_REG_READ16(ul_addr);
        }
#else
        if (0 == oal_strcmp(pst_reg_info->pc_reg_type, "ssi0"))
        {
            ssi_read_reg(HI1103_B100_SSI0_MST_RB_BASE_ADDR, ul_addr, &us_val);
        }
        else if (0 == oal_strcmp(pst_reg_info->pc_reg_type, "ssi1"))
        {
        #ifdef _PRE_WLAN_1103_RF
            ssi_read_reg(HI1103_B100_SSI0_MST_RB_BASE_ADDR, ul_addr, &us_val);
        #else
            ssi_read_reg(HI1103_B100_SSI1_MST_RB_BASE_ADDR, ul_addr, &us_val);
        #endif
        }
#endif
        else
#endif
        {
            hal_reg_info16(pst_hal_device, ul_addr, &us_val);
        }
        OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_config_reg_display_test16::reg_info addr=0x%x, value=0x%x", ul_addr, us_val);
        OAL_IO_PRINT("dmac_config_reg_display_test16::reg_info addr=0x%x, value=0x%x\r\n", ul_addr, us_val);
    }
}

/*Get the cfgid entry*/
OAL_STATIC dmac_config_syn_stru* dmac_config_get_cfgid_map(dmac_config_syn_stru* pst_cfgid_map,
                                                    oal_uint16 en_cfgid,
                                                    oal_uint32 ul_cfgid_nums)
{
    oal_uint16 us_cfgid;
    dmac_config_syn_stru* pst_current_cfgid;

    for(us_cfgid = 0; us_cfgid < ul_cfgid_nums; us_cfgid++)
    {
        pst_current_cfgid = pst_cfgid_map + us_cfgid;
        if (pst_current_cfgid->en_cfgid == en_cfgid)
        {
            return pst_current_cfgid;
        }
    }

    return NULL;
}



oal_uint32  dmac_event_config_syn(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru             *pst_event;
    frw_event_hdr_stru         *pst_event_hdr;
    hmac_to_dmac_cfg_msg_stru  *pst_hmac2dmac_msg;
    mac_vap_stru               *pst_mac_vap;
    mac_device_stru            *pst_mac_device;
    oal_uint32                  ul_ret;
    dmac_config_syn_stru*       pst_cfgid_entry;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_event_config_syn::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件 */
    pst_event         = frw_get_event_stru(pst_event_mem);
    pst_event_hdr     = &(pst_event->st_event_hdr);
    pst_hmac2dmac_msg = (hmac_to_dmac_cfg_msg_stru *)pst_event->auc_event_data;

    //OAM_INFO_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{dmac_event_config_syn::a dmac config syn event occur, cfg_id=%d.}", pst_hmac2dmac_msg->en_syn_id);
    /* 获取dmac vap */
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_event_hdr->uc_vap_id);

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{dmac_event_config_syn::pst_mac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取mac device */
    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{dmac_event_config_syn::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if(OAL_TRUE == MAC_DEV_IS_RESET_IN_PROGRESS(pst_mac_device))
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{dmac_event_config_syn::MAC_DEV_IS_RESET_IN_PROGRESS.}");

        return OAL_FAIL;
    }

    /* 获得cfg id对应的操作函数 */
    pst_cfgid_entry = dmac_config_get_cfgid_map((dmac_config_syn_stru*)g_ast_dmac_config_syn,
                                                pst_hmac2dmac_msg->en_syn_id,
                                                OAL_ARRAY_SIZE(g_ast_dmac_config_syn));
    if(NULL == pst_cfgid_entry)
    {
#ifdef _PRE_WLAN_CFGID_DEBUG
        pst_cfgid_entry = dmac_config_get_cfgid_map((dmac_config_syn_stru*)g_ast_dmac_config_syn_debug,
                                                pst_hmac2dmac_msg->en_syn_id,
                                                dmac_get_config_debug_arrysize());
        if(NULL == pst_cfgid_entry)
        {
            OAM_ERROR_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{dmac_event_config_syn::invalid en_cfgid[%d].", pst_hmac2dmac_msg->en_syn_id);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
#else
        OAM_ERROR_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{dmac_event_config_syn::invalid en_cfgid[%d].", pst_hmac2dmac_msg->en_syn_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
#endif
    }

    /* 异常情况，p_set_func 为空 */
    if (NULL == pst_cfgid_entry->p_set_func)
    {
        OAM_ERROR_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_CFG, "{dmac_event_config_syn::invalid p_set_func cfgid[%d].", pst_hmac2dmac_msg->en_syn_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 执行操作函数 */
    ul_ret = pst_cfgid_entry->p_set_func(pst_mac_vap, (oal_uint8)(pst_hmac2dmac_msg->us_len), (oal_uint8 *)pst_hmac2dmac_msg->auc_msg_body);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG2(pst_event_hdr->uc_vap_id, OAM_SF_CFG,
                         "{dmac_event_config_syn::p_set_func failed, ul_ret=%d en_syn_id=%d.", ul_ret, pst_hmac2dmac_msg->en_syn_id);
        return ul_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_PRODUCT_1151V200

oal_int32 dmac_bgnoise_cal(oal_uint32 ul_reg_value)
{
#define   FLOAT_TO_INT_ADJUST   100     //将计算中用到的变量乘以100，用于将浮点计算转换成整形计算
    oal_int32     l_ADC_power;
    oal_int32     l_ext_lna;
    oal_int32     l_lna;
    oal_int32     al_lna[8] = {0,-250,350,950,1550,2150,2750,3350};
    oal_int32     l_vga;
    oal_int32     l_result = 400;       //excel表格中  (power_ref - lpf_gain)*c_float_adjust。power_ref 是固定值10， lpf_gain是固定值6
    oal_int32     l_temp;

    if (ul_reg_value & 0x100)//判断符号位
    {
        l_ADC_power = (((oal_int32)(ul_reg_value&0xfff))-512)*((oal_int32)(FLOAT_TO_INT_ADJUST/4));
    }
    else
    {
        l_ADC_power = ((oal_int32)(ul_reg_value&0xff))*((oal_int32)(FLOAT_TO_INT_ADJUST/4));
    }

    l_temp     = ((ul_reg_value & 0xf000) >> 12);
    l_ext_lna  = (l_temp/8 ? 13 : -8);
    l_ext_lna  *= FLOAT_TO_INT_ADJUST;
    l_lna      = l_temp & 7;         //对8取余数

    l_temp = (oal_int32)((ul_reg_value & 0xff0000) >> 16);
    l_vga  = (l_temp > 26) ? (l_temp - 64) : l_temp;
    l_vga  *= FLOAT_TO_INT_ADJUST;

    l_result = (((l_result + l_ADC_power) - l_ext_lna) - al_lna[l_lna]) - l_vga;
    return -l_result;

}
#endif


oal_uint32  dmac_config_bg_noise_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#ifdef _PRE_WLAN_PRODUCT_1151V200
#define CH0_ADC_PIN_CODE_RPT_ADRR  (0x20039744)
#define CH1_ADC_PIN_CODE_RPT_ADRR  (0x20038d68)

    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint32                      ul_val;
    oal_int32                       l_bgnoise_val;
    mac_cfg_param_char_stru         st_param;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_bg_noise_info::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }

    OAL_MEMZERO(&st_param, OAL_SIZEOF(mac_cfg_param_char_stru));
    st_param.l_buff_len = 4;
    do
    {
        hal_reg_info(pst_hal_device, CH0_ADC_PIN_CODE_RPT_ADRR, &ul_val);
    }while(ul_val == 0x1000);
    l_bgnoise_val = dmac_bgnoise_cal(ul_val);
    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_bg_noise_info::vap id [%d],reg_addr:0x%x,reg_value:0x%x bg_value:%d\r\n}",pst_mac_vap->uc_vap_id,CH0_ADC_PIN_CODE_RPT_ADRR,ul_val,l_bgnoise_val);
    st_param.auc_buff[0] = (oal_uint8)(l_bgnoise_val/100);//获取底噪整数部分
    st_param.auc_buff[1] = (oal_uint8)(l_bgnoise_val%100);//获取底噪小数部分

    do
    {
        hal_reg_info(pst_hal_device, CH1_ADC_PIN_CODE_RPT_ADRR, &ul_val);
    }while(ul_val == 0x1000);
    l_bgnoise_val = dmac_bgnoise_cal(ul_val);
    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_bg_noise_info::vap id [%d],reg_addr:0x%x ,reg_value:0x%x bg_value:%d\r\n}",pst_mac_vap->uc_vap_id,CH1_ADC_PIN_CODE_RPT_ADRR,ul_val,l_bgnoise_val);
    st_param.auc_buff[2] = (oal_uint8)(l_bgnoise_val/100);
    st_param.auc_buff[3] = (oal_uint8)(l_bgnoise_val%100);
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_BG_NOISE, OAL_SIZEOF(mac_cfg_param_char_stru), (oal_uint8 *)&st_param);
    return OAL_SUCC;
#else
    return OAL_SUCC;
#endif
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

