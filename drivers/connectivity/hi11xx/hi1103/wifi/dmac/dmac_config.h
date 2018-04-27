

#ifndef __DMAC_CONFIG_H__
#define __DMAC_CONFIG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "frw_ext_if.h"
#include "mac_vap.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "dmac_p2p.h"
#endif
#include "dmac_alg_if.h"
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CONFIG_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMAC_HIPRIV_CMD_NAME_MAX_LEN  16                             /* 字符串中每个单词的最大长度 */
#define DMAC_MAX_VERSION_SIZE         200

#define DMAC_GET_VAP_RATE(_uc_protocol,_un_nss_rate)                        \
        ((WLAN_VHT_PHY_PROTOCOL_MODE == (_uc_protocol)) ? (_un_nss_rate.st_vht_nss_mcs.bit_vht_mcs):          \
         (WLAN_HT_PHY_PROTOCOL_MODE == (_uc_protocol)) ? (_un_nss_rate.st_ht_rate.bit_ht_mcs):  \
         g_auc_legacy_rate_idx_table[(_un_nss_rate.st_legacy_rate.bit_legacy_rate)])


#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE
#define ETSI_RADAR_CHIRP_CNT       1        /* ETSI chirp雷达中断阈值 */
#define FCC_RADAR_CHIRP_CNT        3        /* FCC chirp雷达中断阈值 */
#define MKK_RADAR_CHIRP_CNT        3        /* MKK chirp雷达中断阈值 */
#else
#define ETSI_RADAR_CHIRP_CNT       3        /* ETSI chirp雷达中断阈值 */
#define FCC_RADAR_CHIRP_CNT        4        /* FCC chirp雷达中断阈值 */
#define MKK_RADAR_CHIRP_CNT        4        /* MKK chirp雷达中断阈值 */
#endif

#define DMAC_INVALID_POWER      -128
#define DMAC_INVALID_PS_STATE   2           /* 非法的省功耗状态 */
#ifdef _PRE_WLAN_FEATURE_CSI
#define WLAN_CSI_BUFFER_SIZE            (4096)    /* CSI信息预留4k */
#endif

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
enum pm_switch_cfg{
    ALL_DISABLE                   = 0,          /* 平台关低功耗+rf和mac pa常开 */
    ALL_ENABLE                    = 1,          /* 平台开低功耗+业务控rf和mac pa */
    MAC_PA_SWTICH_EN_RF_SWTICH_EN = 2,          /* 平台关低功耗+业务控rf和mac pa */
    MAC_PA_SWTICH_EN_RF_ALWAYS_ON = 3,          /* 平台关低功耗+业务控mac pa+rf常开 */
    LIGHT_SLEEP_SWITCH_EN         = 4,          /* 平台关深睡+业务控mac pa+rf常开 */
};


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern mac_phy_debug_switch_stru  *g_pst_mac_phy_debug_switch;
#ifndef _PRE_WLAN_FEATURE_TPC_OPT
extern oal_uint8 g_auc_rate_pow_table_margin[HAL_POW_RATE_POW_CODE_TABLE_LEN][WLAN_BAND_BUTT];
#endif

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* 信道与带宽模式映射 */
typedef struct
{
    oal_uint8                           uc_channel;     /* 信道 */
    wlan_channel_bandwidth_enum_uint8   en_bandwidth_40;/* 带宽 */
    wlan_channel_bandwidth_enum_uint8   en_bandwidth_80;/* 带宽 */
    oal_uint8                           auc_resv;
}dmac_config_channel_bw_map_stru;

typedef struct
{
    oal_int8*     pc_reg_type;
    oal_uint32    ul_addr;
    oal_uint32    ul_val;    /* 写32位寄存器 */
}dmac_reg_write_stru;

typedef struct
{
    oal_int8*     pc_reg_type;
    oal_uint32    ul_start_addr;
    oal_uint32    ul_end_addr;    /* 写32位寄存器 */
}dmac_reg_info_stru;

typedef oal_uint32 (*p_dmac_config_dfs_radartool_cb)(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
typedef oal_uint32 (*p_dmac_config_proc_query_sta_info_event_cb)(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param, dmac_query_station_info_response_event *pst_query_station_info);
typedef oal_void  (*p_dmac_config_reg_write_test16_cb)(mac_vap_stru *pst_mac_vap, dmac_reg_write_stru *pst_reg_write);
typedef oal_void  (*p_dmac_config_reg_display_test16_cb)(mac_vap_stru *pst_mac_vap, dmac_reg_info_stru *pst_reg_info);

typedef struct
{
    p_dmac_config_dfs_radartool_cb                p_dmac_config_dfs_radartool;
    p_dmac_config_proc_query_sta_info_event_cb    p_dmac_config_proc_query_sta_info_event;
    p_dmac_config_reg_write_test16_cb             p_dmac_config_reg_write_test16;
    p_dmac_config_reg_display_test16_cb           p_dmac_config_reg_display_test16;
}dmac_config_rom_cb;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
extern oal_uint32  dmac_config_set_qap_cwmin(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);

#ifdef _PRE_WLAN_FEATURE_DFS
extern oal_uint32  dmac_config_dfs_radartool(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param);
#endif
extern oal_uint32  dmac_event_config_syn(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_config_set_qap_cwmax(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_qap_aifsn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_qap_txop_limit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_get_cmd_one_arg(oal_int8 *pc_cmd, oal_int8 *pc_arg, oal_uint32 *pul_cmd_offset);
extern oal_uint32  dmac_config_set_qap_msdu_lifetime(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_FEATURE_SMARTANT
extern oal_uint32  dmac_dual_antenna_set_ant(oal_uint8 uc_param);
extern oal_uint32  dmac_dual_antenna_set_ant_at(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_param);
extern oal_uint32  dmac_config_dual_antenna_set_ant(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_dual_antenna_vap_check(mac_vap_stru *pst_mac_vap);
#endif
#ifdef _PRE_WLAN_FEATURE_P2P
extern  oal_uint32  dmac_config_set_p2p_ps_noa(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern  oal_uint32  dmac_config_set_p2p_ps_ops(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
extern  oal_uint32  dmac_config_set_ip_addr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_ONLINE_DPD

extern oal_uint32  dmac_cali_corram_recv(frw_event_mem_stru *pst_event_mem);
#endif
extern oal_uint32  dmac_app_ie_h2d_recv(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_cali_hmac2dmac_recv(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 dmac_cali_to_hmac(frw_event_mem_stru *pst_event_mem);
extern oal_void  dmac_config_get_tx_rate_info(hal_tx_txop_alg_stru    *pst_tx_alg,
                                       mac_data_rate_stru      *pst_mac_rates_11g,
                                       mac_rate_info_stru      *pst_rate_info);
#ifdef _PRE_WLAN_ONLINE_DPD
extern oal_uint32 dmac_dpd_to_hmac(frw_event_mem_stru *pst_event_mem);
#endif

#ifdef _PRE_WLAN_FEATURE_PKT_MEM_OPT
extern oal_uint32 dmac_pkt_mem_opt_stat_event_process(frw_event_mem_stru *pst_event_mem);
#endif /*_PRE_WLAN_FEATURE_PKT_MEM_OPT */
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
extern oal_void dmac_config_update_scaling_reg(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_config_update_dsss_scaling_reg(hal_to_dmac_device_stru *pst_hal_device, hal_alg_user_distance_enum_uint8 en_dmac_device_distance_enum);
extern oal_uint32 dmac_config_set_d2h_hcc_assemble_cnt(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_sdio_flowctrl(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_linkloss_threshold(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_bool_enum_uint8 is_fcc_country(oal_int8* country_code);
extern oal_uint32 dmac_config_set_cus_over_temper_rf(oal_uint8 *puc_param);
#endif  /* _PRE_PLAT_FEATURE_CUSTOMIZE */
extern oal_uint32  dmac_send_sys_event(mac_vap_stru *pst_mac_vap,
                                                wlan_cfgid_enum_uint16 en_cfg_id,
                                                oal_uint16 us_len,
                                                oal_uint8 *puc_param);
extern oal_uint32  dmac_config_d2h_user_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
extern oal_uint32  dmac_config_d2h_vap_mib_syn(mac_vap_stru *pst_mac_vap);
extern oal_uint32  dmac_config_vap_mib_update(mac_vap_stru *pst_mac_vap);
extern oal_uint32  dmac_config_vap_cap_update(mac_vap_stru *pst_mac_vap);
#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) || defined(_PRE_WLAN_FEATURE_SMPS)
extern oal_uint32 dmac_config_d2h_user_m2s_info_syn(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI
extern oal_uint32  dmac_config_vowifi_report(dmac_vap_stru *pst_dmac_vap);
#endif /* _PRE_WLAN_FEATURE_VOWIFI */

#ifdef _PRE_WLAN_CFGID_DEBUG
extern oal_uint32  dmac_config_alg(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_user_vip(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_vap_host(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_dump_ba_bitmap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_get_mpdu_num(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_ota_beacon_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_ota_rx_dscr_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_all_ota(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_oam_output(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_probe_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_80211_mcast_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_dump_tx_dscr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_feature_log(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_get_thruput(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_hide_ssid(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_bw_fixed(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_thruput_bypass(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_vap_nss(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_ampdu_aggr_num(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_show_device_memleak(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_PROFLING_MIPS
extern oal_uint32 dmac_config_set_mips(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_show_mips(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
extern  oal_uint32  dmac_config_enable_arp_offload(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern  oal_uint32  dmac_config_show_arpoffload_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
extern oal_uint32 dmac_config_enable_2040bss(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
extern oal_uint32  dmac_config_set_opmode_notify(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_get_user_rssbw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_DFT_STAT
extern oal_uint32  dmac_config_report_vap_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_phy_stat_en(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_dbb_env_param(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_performance_log_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_report_all_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
extern oal_uint32  dmac_config_usr_queue_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
extern oal_uint32 dmac_config_set_ampdu_tx_hw_on(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
extern oal_uint32  dmac_config_beacon_chain_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_uint32 dmac_config_set_stbc_cap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_ldpc_cap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_dump_timer(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_80211_ucast_switch(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_report_vap_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_pkts_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_mem_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_DFT_STAT
extern oal_uint32  dmac_config_phy_stat_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_machw_stat_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_report_mgmt_stat(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
extern oal_uint32 dmac_config_wmmac_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#endif
extern oal_uint32  dmac_config_pause_tid(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_send_bar(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_dump_rx_dscr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_rssi_limit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
extern oal_uint32 dmac_config_enable_pmf(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif

extern oal_uint32  dmac_config_set_nss(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_SMARTANT
extern oal_uint32 dmac_dual_antenna_init(oal_void);
oal_uint32 dmac_config_dual_antenna_check(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_alg_cfg_get_ant_info_notify(mac_vap_stru *pst_vap, oal_uint8 *puc_param,
    oal_uint32 *pul_param1, oal_uint32 *pul_param2, oal_uint32 *pul_param3, oal_uint32 *pul_param4, oal_uint32 *pul_param5);
extern oal_uint32 dmac_config_get_ant_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_alg_cfg_double_ant_switch_notify(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_param);
extern oal_uint32 dmac_config_double_ant_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
extern  oal_uint32  dmac_al_tx_complete_event_handler(frw_event_mem_stru *pst_event_mem);
#endif
extern  oal_uint32  dmac_config_set_protection(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_WEB_CMD_COMM
extern oal_uint32  dmac_config_get_temp(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
oal_uint32  dmac_config_get_bsd(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#endif
#ifdef _PRE_FEATURE_WAVEAPP_CLASSIFY
extern oal_uint32  dmac_config_get_waveapp_flag(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
extern oal_void  dmac_config_reg_display_test(mac_vap_stru *pst_mac_vap, dmac_reg_info_stru *pst_reg_info);
extern oal_void  dmac_config_reg_display_test16(mac_vap_stru *pst_mac_vap, dmac_reg_info_stru *pst_reg_info);
extern oal_uint32  dmac_config_reg_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_bg_noise_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_bss_type(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_ssid(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_shpreamble(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_prot_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_bintval(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_nobeacon(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_dtimperiod(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_cwmin(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_cwmax(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_aifsn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_txop_limit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_query_rssi(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_query_psst(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_WEB_CMD_COMM
#ifdef _PRE_WLAN_11K_STAT
extern oal_uint32  dmac_config_query_drop_num(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_query_tx_delay(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#endif
extern oal_uint32  dmac_config_query_rate(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_user_asoc_state_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_void  dmac_config_reg_write_test(mac_vap_stru *pst_mac_vap, dmac_reg_write_stru *pst_reg_write);
extern oal_void  dmac_config_reg_write_test16(mac_vap_stru *pst_mac_vap, dmac_reg_write_stru *pst_reg_write);
extern oal_void  dmac_config_set_dscr_pgl(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_mtpgl(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_ta(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_ra(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_cc(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_special(oal_int32 l_value, hal_tx_txop_alg_stru *pst_tx_txop_alg, oal_uint8   uc_rate_num);
extern oal_void  dmac_config_set_dscr_data(oal_int32 l_value, oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap, oal_uint8   uc_rate_num);
extern oal_void  dmac_config_set_dscr_data0(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_data1(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_data2(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_data3(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_power(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_shortgi(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_preamble_mode(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_rtscts(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_lsigtxop(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_smooth(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_snding(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_txbf(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_stbc(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_get_dscr_ess(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_dyn_bw(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_dyn_bw_exist(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_ch_bw_exist(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_legacy_rate(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_mcs(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_mcsac(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_nss(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_config_set_dscr_bw(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap);
extern oal_uint32 dmac_config_nss(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_app_ie_to_vap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_reg_write(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_vap_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_void  dmac_config_set_machw_wmm(hal_to_dmac_vap_stru *pst_hal_vap, mac_vap_stru *pst_mac_vap);
extern oal_void  dmac_config_set_wmm_open_cfg(hal_to_dmac_vap_stru *pst_hal_vap, mac_wme_param_stru  *pst_wmm);
extern oal_void  dmac_config_set_wmm_close_cfg(hal_to_dmac_vap_stru *pst_hal_vap, mac_wme_param_stru  *pst_wmm);
extern oal_uint32 dmac_config_rx_fcs_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_edca_mandatory(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_qap_edca_mandatory(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_log_level(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_dump_all_rx_dscr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_user_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_random_mac_oui(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_always_rx(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_mib_by_bw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_proc_query_sta_info_event(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);

#if defined (_PRE_WLAN_CHIP_TEST) || defined (_PRE_WLAN_FEATURE_ALWAYS_TX)
extern oal_uint32  dmac_config_set_bw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_11K
extern oal_uint32  dmac_config_bcn_table_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
extern oal_uint32  dmac_roam_update_framer(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user);
extern oal_uint32  dmac_config_roam_notify_state(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_roam_succ_h2d_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_DFT_STAT
extern oal_uint32  dmac_config_query_ani(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
extern oal_netbuf_stru*  dmac_config_create_al_tx_packet(oal_uint32 ul_size, mac_rf_payload_enum_uint8 en_payload_flag);
extern oal_uint32  dmac_config_set_always_tx_num(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_always_tx_hw_cfg(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_always_tx_hw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_al_tx_packet(mac_vap_stru *pst_vap, oal_netbuf_stru *pst_buf, oal_uint32 ul_len);
#endif
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
extern oal_void dmac_clear_ip_filter_btable(mac_vap_stru *pst_mac_vap);
extern oal_uint32 dmac_config_update_ip_filter(frw_event_mem_stru *pst_event_mem);
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_uint32  dmac_config_set_shortgi(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_ch_status_sync(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_txpower(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32   dmac_config_set_regdomain_pwr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_wfa_cfg_aifsn(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_wfa_cfg_cw(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_log_lowpower(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_pm_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern     oal_uint32 dmac_config_set_ps_check_cnt(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_get_ant(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_suspend_action_sync(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);

#ifdef _PRE_WLAN_FEATURE_STA_PM
extern oal_uint32 dmac_set_psm_param(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32  dmac_config_set_sta_ps_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
extern oal_uint32 dmac_config_set_sta_pm_on(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
extern oal_uint32 dmac_config_set_uapsd_para(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
#endif
extern oal_uint32 dmac_psm_check_module_ctrl(mac_vap_stru *pst_mac_vap, mac_pm_ctrl_type_enum_uint8 pm_ctrl_type, mac_pm_switch_enum_uint8 pm_enable, oal_uint8 *puc_psm_result);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
oal_void dmac_config_reg_report(mac_vap_stru *pst_mac_vap, oal_uint32 ul_addr);
#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_config */
