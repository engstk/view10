


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_ext_if.h"
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
#include "mpw0_poweron.h"
#include "hal_dscr.h"
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "hal_mac.h"
#endif
#include "oal_sdio.h"
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "pm_extern.h"
#include "dmac_psm_sta.h"

#endif

#include "dmac_config.h"
#include "dmac_main.h"
#include "dmac_power.h"
#include "dmac_dft.h"



#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CONFIG_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*软件legacy rate映射索引*/
OAL_CONST oal_uint8 g_auc_legacy_rate_idx_table[WLAN_LEGACY_RATE_VALUE_BUTT]=
{
/*WLAN_LEGACY_11b_RESERVED1       = 0*/  1,
/*WLAN_SHORT_11b_2M_BPS           = 1*/  2,
/*WLAN_SHORT_11b_5_HALF_M_BPS     = 2*/  5,
/*WLAN_SHORT_11b_11_M_BPS         = 3*/  11,

/*WLAN_LONG_11b_1_M_BPS           = 4*/  1,
/*WLAN_LONG_11b_2_M_BPS           = 5*/  2,
/*WLAN_LONG_11b_5_HALF_M_BPS      = 6*/  5,
/*WLAN_LONG_11b_11_M_BPS          = 7*/  11,

/*WLAN_LEGACY_OFDM_48M_BPS        = 8*/   48,
/*WLAN_LEGACY_OFDM_24M_BPS        = 9*/   24,
/*WLAN_LEGACY_OFDM_12M_BPS        = 10*/  12,
/*WLAN_LEGACY_OFDM_6M_BPS         = 11*/  6,
/*WLAN_LEGACY_OFDM_54M_BPS        = 12*/  54,
/*WLAN_LEGACY_OFDM_36M_BPS        = 13*/  36,
/*WLAN_LEGACY_OFDM_18M_BPS        = 14*/  18,
/*WLAN_LEGACY_OFDM_9M_BPS         = 15*/  9,
};


dmac_config_rom_cb g_st_dmac_config_rom_cb = {OAL_PTR_NULL,
                                              OAL_PTR_NULL,
                                              dmac_config_reg_write_test16,
                                              dmac_config_reg_display_test16};

/*****************************************************************************
3 外部函数声明
*****************************************************************************/

/*****************************************************************************
 4 函数实现
*****************************************************************************/
oal_int8 dmac_config_get_free_power(mac_vap_stru *pst_mac_vap);

oal_uint32  dmac_get_cmd_one_arg(oal_int8 *pc_cmd, oal_int8 *pc_arg, oal_uint32 *pul_cmd_offset)
{
    oal_int8   *pc_cmd_copy;
    oal_uint32  ul_pos = 0;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pc_cmd) || (OAL_PTR_NULL == pc_arg) || (OAL_PTR_NULL == pul_cmd_offset)))
    {
        OAM_ERROR_LOG3(0, OAM_SF_CFG,
                       "{dmac_get_cmd_one_arg::param null, pc_cmd=%d pc_arg=%d pul_cmd_offset=%d.}",
                       pc_cmd, pc_arg, pul_cmd_offset);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pc_cmd_copy = pc_cmd;

    /* 去掉字符串开始的空格 */
    while (' ' == *pc_cmd_copy)
    {
        ++pc_cmd_copy;
    }

    while ((' ' != *pc_cmd_copy) && ('\0' != *pc_cmd_copy))
    {
        pc_arg[ul_pos] = *pc_cmd_copy;
        ++ul_pos;
        ++pc_cmd_copy;

        if (OAL_UNLIKELY(ul_pos >= DMAC_HIPRIV_CMD_NAME_MAX_LEN))
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_get_cmd_one_arg::ul_pos=%d", ul_pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    pc_arg[ul_pos]  = '\0';

    /* 字符串到结尾，返回错误码 */
    if (0 == ul_pos)
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{dmac_get_cmd_one_arg::ul_pos=0.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pul_cmd_offset = (oal_uint32)(pc_cmd_copy - pc_cmd);

    return OAL_SUCC;
}

oal_uint32  dmac_config_set_qap_msdu_lifetime(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32  ul_ac;
    oal_uint32  ul_value;
    oal_uint32 *pul_param;

    oal_uint16  us_lifetime_bk = 0;
    oal_uint16  us_lifetime_be = 0;
    oal_uint16  us_lifetime_vi = 0;
    oal_uint16  us_lifetime_vo = 0;
    oal_uint16  us_pre_value = 0;

    dmac_vap_stru               *pst_dmac_vap;

    pul_param = (oal_uint32 *)puc_param;

    ul_ac     = pul_param[1];
    ul_value  = pul_param[2];

    if (ul_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_msdu_lifetime::invalid param, ul_ac=%d ul_value=%d.", ul_ac, ul_value);
        return OAL_FAIL;
    }

    mac_mib_set_QAPEDCATableMSDULifetime(pst_mac_vap, (oal_uint8)ul_ac, ul_value);

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_wfa_cfg_aifsn::mac_res_get_dmac_vap fail,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }


    switch (ul_ac)
    {
        case WLAN_WME_AC_BK:
            hal_vap_get_machw_edca_bkbe_lifetime(pst_dmac_vap->pst_hal_vap, &us_lifetime_be, &us_pre_value);
            hal_vap_set_machw_edca_bkbe_lifetime(pst_dmac_vap->pst_hal_vap, us_lifetime_be, (oal_uint16)ul_value);
            break;

        case WLAN_WME_AC_BE:
            hal_vap_get_machw_edca_bkbe_lifetime(pst_dmac_vap->pst_hal_vap, &us_pre_value, &us_lifetime_bk);
            hal_vap_set_machw_edca_bkbe_lifetime(pst_dmac_vap->pst_hal_vap, (oal_uint16)ul_value, us_lifetime_bk);
            break;

        case WLAN_WME_AC_VI:
            hal_vap_get_machw_edca_vivo_lifetime(pst_dmac_vap->pst_hal_vap, &us_lifetime_vo, &us_pre_value);
            hal_vap_set_machw_edca_vivo_lifetime(pst_dmac_vap->pst_hal_vap, us_lifetime_vo, (oal_uint16)ul_value);
            break;

        case WLAN_WME_AC_VO:
            hal_vap_get_machw_edca_vivo_lifetime(pst_dmac_vap->pst_hal_vap, &us_pre_value, &us_lifetime_vi);
            hal_vap_set_machw_edca_vivo_lifetime(pst_dmac_vap->pst_hal_vap, (oal_uint16)ul_value, us_lifetime_vi);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

oal_uint32  dmac_send_sys_event(
                mac_vap_stru                     *pst_mac_vap,
                wlan_cfgid_enum_uint16            en_cfg_id,
                oal_uint16                        us_len,
                oal_uint8                        *puc_param)
{
    oal_uint32                  ul_ret;
    frw_event_mem_stru         *pst_event_mem;
    dmac_to_hmac_cfg_msg_stru  *pst_syn_msg;
    frw_event_stru             *pst_event;

    pst_event_mem = FRW_EVENT_ALLOC((OAL_SIZEOF(dmac_to_hmac_cfg_msg_stru) - 4) + us_len);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_send_sys_event::pst_event_mem null.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填充事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                        FRW_EVENT_TYPE_HOST_SDT_REG,
                        DMAC_TO_HMAC_SYN_CFG,
                        ((OAL_SIZEOF(dmac_to_hmac_cfg_msg_stru) - 4) + us_len),
                        FRW_EVENT_PIPELINE_STAGE_1,
                        pst_mac_vap->uc_chip_id,
                        pst_mac_vap->uc_device_id,
                        pst_mac_vap->uc_vap_id);

    pst_syn_msg = (dmac_to_hmac_cfg_msg_stru *)pst_event->auc_event_data;

    DMAC_INIT_SYN_MSG_HDR(pst_syn_msg, en_cfg_id, us_len);

    /* 填写配置同步消息内容 */
    oal_memcopy(pst_syn_msg->auc_msg_body, puc_param, us_len);

    /* 抛出事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_send_sys_event::frw_event_dispatch_event failed[%d].}",ul_ret);
        FRW_EVENT_FREE(pst_event_mem);
        return ul_ret;
    }

    FRW_EVENT_FREE(pst_event_mem);

    return OAL_SUCC;
}



oal_void  dmac_config_set_dscr_pgl(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    pst_dmac_vap->st_tx_alg.st_tx_power.bit_pa_gain_level3 = (oal_uint8)l_value;
}


oal_void  dmac_config_set_dscr_mtpgl(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    pst_dmac_vap->st_tx_alg.st_tx_power.bit_pa_gain_level0 = (oal_uint8)l_value;
}


oal_void  dmac_config_set_dscr_ta(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap)
{
    pst_dmac_vap->st_tx_alg.st_antenna_params.uc_tx_rts_antenna = (oal_uint8)l_value;
}


oal_void  dmac_config_set_dscr_ra(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap)
{
    pst_dmac_vap->st_tx_alg.st_antenna_params.uc_rx_ctrl_antenna = (oal_uint8)l_value;
}


oal_void  dmac_config_set_dscr_cc(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap)
{
    hal_to_dmac_device_stru         *pst_hal_device_base;
    wlan_phy_protocol_enum_uint8    en_curr_prot;

    pst_hal_device_base = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device_base))
    {
       OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                             "{dmac_config_set_dscr_cc::pst_hal_device_base null.}");
       return;
    }

    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.st_rate.en_channel_code = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.st_rate.en_channel_code  = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.st_rate.en_channel_code  = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->st_tx_data_bcast.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].st_rate.en_channel_code  = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_rate.en_channel_code  = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].st_rate.en_channel_code  = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_rate.en_channel_code  = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.st_rate.en_channel_code = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->st_tx_alg_vht.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.st_rate.en_channel_code = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->st_tx_alg_ht.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.st_rate.en_channel_code = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->st_tx_alg_11ag.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.st_rate.en_channel_code = (oal_uint8)l_value;
            en_curr_prot = pst_dmac_vap->st_tx_alg_11b.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_cc::invalid frame type[%02x].}", uc_type);
            return;
    }

    /* 常发模式下在线配置 */
    if (OAL_SWITCH_ON == pst_hal_device_base->bit_al_tx_flag)
    {
       hal_set_tx_dscr_field(pst_hal_device_base, (oal_uint8)l_value, HAL_RF_TEST_CHAN_CODE);
    }

    /* 设置值回显 */
    if ((1 == l_value) && (WLAN_11B_PHY_PROTOCOL_MODE == en_curr_prot || WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE == en_curr_prot))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                         "{dmac_config_set_dscr_cc::can not set channel code to 1 in non-HT mode.}");
    }
    else if ((1 == l_value) || (0 == l_value))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_cc::channel code=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_cc::invalid channel code=%d", l_value);
    }
}


oal_void dmac_config_set_dscr_special(oal_int32 l_value, hal_tx_txop_alg_stru *pst_tx_txop_alg, oal_uint8   uc_rate_num)
{
    pst_tx_txop_alg->ast_per_rate[uc_rate_num].ul_value = (oal_uint32)l_value;

    if (pst_tx_txop_alg->ast_per_rate[uc_rate_num].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode <= WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE) // 0:11b  1:ofdm
    {
        pst_tx_txop_alg->st_rate.en_channel_code = 0;  // 11b ofdm 信道编码为BCC；1 :LDPC
        pst_tx_txop_alg->st_rate.uc_extend_spatial_streams = 0;
        pst_tx_txop_alg->st_rate.dyn_bandwidth_in_non_ht = 0;
        pst_tx_txop_alg->st_rate.dyn_bandwidth_in_non_ht_exist = 0;
        pst_tx_txop_alg->st_rate.ch_bandwidth_in_non_ht_exist = 0;
        pst_tx_txop_alg->st_rate.uc_smoothing = 0;
        pst_tx_txop_alg->st_rate.en_sounding_mode = WLAN_NO_SOUNDING;
    }

    if (WLAN_HT_PHY_PROTOCOL_MODE != pst_tx_txop_alg->ast_per_rate[uc_rate_num].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode) // 非HT mode,清0
    {
        pst_tx_txop_alg->st_rate.bit_lsig_txop = 0;
    }

    return;
}


oal_void  dmac_config_set_dscr_data(oal_int32 l_value, oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap, oal_uint8   uc_rate_num)
{

    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->st_tx_alg), uc_rate_num);
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->st_tx_data_mcast), uc_rate_num);
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->st_tx_data_bcast), uc_rate_num);
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G]), uc_rate_num);
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G]), uc_rate_num);
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G]), uc_rate_num);
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G]), uc_rate_num);
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->st_tx_alg_vht), uc_rate_num);
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->st_tx_alg_ht), uc_rate_num);
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->st_tx_alg_11ag), uc_rate_num);
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            dmac_config_set_dscr_special(l_value, &(pst_dmac_vap->st_tx_alg_11b), uc_rate_num);
            break;
    #endif

        default:
            break;

    }
}


oal_void  dmac_config_set_dscr_data0(oal_int32 l_value, oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8   uc_rate_num = 0;

    dmac_config_set_dscr_data(l_value, uc_type, pst_dmac_vap, uc_rate_num);

    return;
}


oal_void  dmac_config_set_dscr_data1(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8   uc_rate_num = 1;

    dmac_config_set_dscr_data(l_value, uc_type, pst_dmac_vap, uc_rate_num);

    return;
}



oal_void dmac_config_set_dscr_data2(oal_int32 l_value, oal_uint8 uc_type,dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8   uc_rate_num = 2;

    dmac_config_set_dscr_data(l_value, uc_type, pst_dmac_vap, uc_rate_num);

    return;
}



oal_void  dmac_config_set_dscr_data3(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8   uc_rate_num = 3;

    dmac_config_set_dscr_data(l_value, uc_type, pst_dmac_vap, uc_rate_num);

    return;

}



oal_void  dmac_config_set_dscr_power(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    oal_uint32 *pul_txpower;

    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->st_tx_alg.st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->st_tx_data_mcast.st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->st_tx_data_bcast.st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->st_tx_alg_vht.st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->st_tx_alg_ht.st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->st_tx_alg_11ag.st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pul_txpower  = (oal_uint32 *)(&(pst_dmac_vap->st_tx_alg_11b.st_tx_power));
            *pul_txpower = (oal_uint32)l_value;
            break;
    #endif

        default:
            break;
    }
}


oal_void  dmac_config_set_dscr_shortgi(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
#if defined (_PRE_WLAN_CHIP_TEST) || defined (_PRE_WLAN_FEATURE_ALWAYS_TX)
    hal_to_dmac_device_stru         *pst_hal_device_base;

    pst_hal_device_base = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device_base))
    {
       OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                             "{dmac_config_set_dscr_cc::pst_hal_device_base null.}");
       return;
    }
#endif

    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            pst_dmac_vap->st_tx_alg.ast_per_rate[1].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            pst_dmac_vap->st_tx_alg.ast_per_rate[2].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            pst_dmac_vap->st_tx_alg.ast_per_rate[3].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_shortgi::shortGI has been set to %d", pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_short_gi_enable);
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.ast_per_rate[0].rate_bit_stru.bit_short_gi_enable = (oal_uint8)l_value;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_shortgi::uc_type=%d", uc_type);
            return;
    }

#if defined (_PRE_WLAN_CHIP_TEST) || defined (_PRE_WLAN_FEATURE_ALWAYS_TX)
    /* 常发模式下在线配置 */
    if (OAL_SWITCH_ON == pst_hal_device_base->bit_al_tx_flag)
    {
       hal_set_tx_dscr_field(pst_hal_device_base, pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].ul_value, HAL_RF_TEST_DATA_RATE_ZERO);
    }

    if ((0 == l_value) || (1 == l_value))
    {
        pst_dmac_vap->uc_short_gi = (oal_uint8)l_value;
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_shortgi::short gi enable l_value=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_shortgi::invalid short gi enable l_value=%d", l_value);
    }
#endif
}


oal_void  dmac_config_set_dscr_preamble_mode(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
#if defined (_PRE_WLAN_CHIP_TEST) || defined (_PRE_WLAN_FEATURE_ALWAYS_TX)
        hal_to_dmac_device_stru         *pst_hal_device_base;

        pst_hal_device_base = pst_dmac_vap->pst_hal_device;
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device_base))
        {
           OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                                 "{dmac_config_set_dscr_cc::pst_hal_device_base null.}");
           return;
        }
#endif

    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            pst_dmac_vap->st_tx_alg.ast_per_rate[1].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            pst_dmac_vap->st_tx_alg.ast_per_rate[2].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            pst_dmac_vap->st_tx_alg.ast_per_rate[3].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_preamble_mode::preamble has been set to %d", pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_preamble_mode);
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.ast_per_rate[0].rate_bit_stru.bit_preamble_mode = (oal_uint8)l_value;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_preamble_mode::invalid frame type=%d", uc_type);
            return;
    }

#if defined (_PRE_WLAN_CHIP_TEST) || defined (_PRE_WLAN_FEATURE_ALWAYS_TX)
    /* 常发模式下在线配置 */
    if (OAL_SWITCH_ON == pst_hal_device_base->bit_al_tx_flag)
    {
       hal_set_tx_dscr_field(pst_hal_device_base, pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].ul_value, HAL_RF_TEST_DATA_RATE_ZERO);
    }

    if ((0 == l_value) || (1 == l_value))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_preamble_mode::l_value=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_preamble_mode::invalid l_value=%d", l_value);
    }
#endif
}



oal_void  dmac_config_set_dscr_rtscts(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            pst_dmac_vap->st_tx_alg.ast_per_rate[1].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            pst_dmac_vap->st_tx_alg.ast_per_rate[2].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            pst_dmac_vap->st_tx_alg.ast_per_rate[3].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_rtscts::RTS/CTS has been set to %d", pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable);
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.ast_per_rate[0].rate_bit_stru.bit_rts_cts_enable = (oal_uint8)l_value;
            break;
    #endif

        default:
            break;
    }
}


oal_void  dmac_config_set_dscr_lsigtxop(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.st_rate.bit_lsig_txop = (oal_uint8)l_value;
            break;
    #endif

        default:
            break;
    }
}


oal_void  dmac_config_set_dscr_smooth(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.st_rate.uc_smoothing = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.st_rate.uc_smoothing  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.st_rate.uc_smoothing  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].st_rate.uc_smoothing  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_rate.uc_smoothing  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].st_rate.uc_smoothing  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_rate.uc_smoothing  = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.st_rate.uc_smoothing = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.st_rate.uc_smoothing = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.st_rate.uc_smoothing = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.st_rate.uc_smoothing = (oal_uint8)l_value;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_smooth::invalid frame type=%d", uc_type);
            return;
    }

    if ((0 == l_value) || (1 == l_value))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_smooth::l_value=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_smooth::invalid l_value=%d", l_value);
    }
}


oal_void  dmac_config_set_dscr_snding(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.st_rate.en_sounding_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.st_rate.en_sounding_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.st_rate.en_sounding_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].st_rate.en_sounding_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_rate.en_sounding_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].st_rate.en_sounding_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_rate.en_sounding_mode  = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.st_rate.en_sounding_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.st_rate.en_sounding_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.st_rate.en_sounding_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.st_rate.en_sounding_mode = (oal_uint8)l_value;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_snding::invalid frame type=%d", uc_type);
            return;
    }

    /* 打印TX描述符 */
    if ((l_value >= 0) && (l_value <= 3))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_snding::l_value=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_snding::invalid l_value=%d", l_value);
    }
}


oal_void  dmac_config_set_dscr_txbf(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_txbf_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.bit_txbf_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.ast_per_rate[0].rate_bit_stru.bit_txbf_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_txbf_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_txbf_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_txbf_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_txbf_mode  = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.ast_per_rate[0].rate_bit_stru.bit_txbf_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.ast_per_rate[0].rate_bit_stru.bit_txbf_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.ast_per_rate[0].rate_bit_stru.bit_txbf_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.ast_per_rate[0].rate_bit_stru.bit_txbf_mode = (oal_uint8)l_value;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_txbf::invalid frame type=%d", uc_type);
            return;
    }

    /* 打印TX描述符 */
    if ((l_value >= 0) && (l_value <= 3))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_txbf::l_value=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_txbf::invalid l_value=%d", l_value);
    }
}


oal_void  dmac_config_set_dscr_stbc(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
#if (WLAN_MAX_NSS_NUM == WLAN_SINGLE_NSS)
    OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_null::unsupported nss mode");
#else
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.bit_stbc_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.bit_stbc_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.ast_per_rate[0].rate_bit_stru.bit_stbc_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_stbc_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_stbc_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].ast_per_rate[0].rate_bit_stru.bit_stbc_mode  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].ast_per_rate[0].rate_bit_stru.bit_stbc_mode  = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.ast_per_rate[0].rate_bit_stru.bit_stbc_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.ast_per_rate[0].rate_bit_stru.bit_stbc_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.ast_per_rate[0].rate_bit_stru.bit_stbc_mode = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.ast_per_rate[0].rate_bit_stru.bit_stbc_mode = (oal_uint8)l_value;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_stbc::invalid frame type=%d", uc_type);
            return;
    }

    /* 打印TX描述符 */
    if ((l_value >= 0) && (l_value <= 3))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_stbc::l_valuee=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_stbc::invalid l_value=%d", l_value);
    }
#endif
}


oal_void  dmac_config_get_dscr_ess(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8   uc_ess_num;

    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            uc_ess_num = pst_dmac_vap->st_tx_alg.st_rate.uc_extend_spatial_streams;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            uc_ess_num = pst_dmac_vap->st_tx_data_mcast.st_rate.uc_extend_spatial_streams;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            uc_ess_num = pst_dmac_vap->st_tx_data_bcast.st_rate.uc_extend_spatial_streams;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            uc_ess_num = pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].st_rate.uc_extend_spatial_streams;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            uc_ess_num = pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_rate.uc_extend_spatial_streams;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            uc_ess_num = pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].st_rate.uc_extend_spatial_streams;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            uc_ess_num = pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_rate.uc_extend_spatial_streams;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            uc_ess_num = pst_dmac_vap->st_tx_alg_vht.st_rate.uc_extend_spatial_streams;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            uc_ess_num = pst_dmac_vap->st_tx_alg_ht.st_rate.uc_extend_spatial_streams;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            uc_ess_num = pst_dmac_vap->st_tx_alg_11ag.st_rate.uc_extend_spatial_streams;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            uc_ess_num = pst_dmac_vap->st_tx_alg_11b.st_rate.uc_extend_spatial_streams;
            break;
    #endif

        default:
            uc_ess_num = 0;
            break;
    }

    /* 打印TX描述符 */
    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_get_dscr_ess::uc_ess_num=%d", uc_ess_num);
}


oal_void  dmac_config_set_dscr_dyn_bw(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.st_rate.dyn_bandwidth_in_non_ht = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.st_rate.dyn_bandwidth_in_non_ht = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.st_rate.dyn_bandwidth_in_non_ht  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].st_rate.dyn_bandwidth_in_non_ht   = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_rate.dyn_bandwidth_in_non_ht   = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].st_rate.dyn_bandwidth_in_non_ht   = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_rate.dyn_bandwidth_in_non_ht   = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.st_rate.dyn_bandwidth_in_non_ht = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.st_rate.dyn_bandwidth_in_non_ht = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.st_rate.dyn_bandwidth_in_non_ht = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.st_rate.dyn_bandwidth_in_non_ht = (oal_uint8)l_value;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_dyn_bw::invalid frame type=%d", uc_type);
            return;
    }

    if ((0 == l_value) || (1 == l_value))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_dyn_bw::l_value=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_dyn_bw::invalid l_value=%d", l_value);
    }
}


oal_void  dmac_config_set_dscr_dyn_bw_exist(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.st_rate.dyn_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.st_rate.dyn_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.st_rate.dyn_bandwidth_in_non_ht_exist  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].st_rate.dyn_bandwidth_in_non_ht_exist   = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_rate.dyn_bandwidth_in_non_ht_exist   = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].st_rate.dyn_bandwidth_in_non_ht_exist   = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_rate.dyn_bandwidth_in_non_ht_exist   = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.st_rate.dyn_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.st_rate.dyn_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.st_rate.dyn_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.st_rate.dyn_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_dyn_bw_exist::invalid frame type=%d", uc_type);
            return;
    }

    if ((0 == l_value) || (1 == l_value))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_dyn_bw_exist::l_value=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_dyn_bw_exist::invalid l_value=%d", l_value);
    }
}


oal_void  dmac_config_set_dscr_ch_bw_exist(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.st_rate.ch_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MCAST_DATA:
            pst_dmac_vap->st_tx_data_mcast.st_rate.ch_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_BCAST_DATA:
            pst_dmac_vap->st_tx_data_bcast.st_rate.ch_bandwidth_in_non_ht_exist  = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_2G].st_rate.ch_bandwidth_in_non_ht_exist   = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_rate.ch_bandwidth_in_non_ht_exist   = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_2G].st_rate.ch_bandwidth_in_non_ht_exist   = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
            pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_rate.ch_bandwidth_in_non_ht_exist   = (oal_uint8)l_value;
            break;
    #ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
        case MAC_VAP_CONFIG_VHT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_vht.st_rate.ch_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_HT_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_ht.st_rate.ch_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11AG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11ag.st_rate.ch_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
        case MAC_VAP_CONFIG_11B_UCAST_DATA:
            pst_dmac_vap->st_tx_alg_11b.st_rate.ch_bandwidth_in_non_ht_exist = (oal_uint8)l_value;
            break;
    #endif

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_ch_bw_exist::invalid frame type=%d", uc_type);
            return;
    }

    if ((0 == l_value) || (1 == l_value))
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_ch_bw_exist::l_value=%d", l_value);
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_ch_bw_exist::invalid l_valueype=%d", l_value);
    }
}


oal_void  dmac_vap_set_legacy_rate(dmac_vap_stru * OAL_CONST pst_dmac_vap,  hal_tx_txop_alg_stru *pst_tx_alg, oal_int32 l_value)
{
    mac_cfg_non_ht_rate_stru            *pst_dmac_set_non_ht_rate_param;
    pst_dmac_set_non_ht_rate_param = (mac_cfg_non_ht_rate_stru*)(&l_value);

    if(pst_tx_alg->st_rate.en_channel_bandwidth > WLAN_BAND_ASSEMBLE_20M)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_set_legacy_rate::802.11a/b/g can only run at 20MHz bandwidth,use 'set_ucast_data bw 20'");
        return;
    }
    OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_vap_set_legacy_rate::protocol [%d],rate[%d]!}\r\n", pst_dmac_set_non_ht_rate_param->en_protocol_mode,pst_dmac_set_non_ht_rate_param->en_rate);

    /*协议切换，重置前导码*/
    if(pst_dmac_set_non_ht_rate_param->en_protocol_mode != pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode
    ||pst_dmac_set_non_ht_rate_param->en_protocol_mode != pst_tx_alg->ast_per_rate[1].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode
    ||pst_dmac_set_non_ht_rate_param->en_protocol_mode != pst_tx_alg->ast_per_rate[2].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode
    ||pst_dmac_set_non_ht_rate_param->en_protocol_mode != pst_tx_alg->ast_per_rate[3].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode)
    {
        pst_tx_alg->ast_per_rate[0].rate_bit_stru.bit_preamble_mode = 0;
        pst_tx_alg->ast_per_rate[1].rate_bit_stru.bit_preamble_mode = 0;
        pst_tx_alg->ast_per_rate[2].rate_bit_stru.bit_preamble_mode = 0;
        pst_tx_alg->ast_per_rate[3].rate_bit_stru.bit_preamble_mode = 0;
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_set_legacy_rate::protocol mode is changed, resset preamble mode to 0");
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_set_legacy_rate::plz make sure ampdu is turned off! use 'wlan0 ampdu_tx_on 0' if you wanna do so");
    }

    /*协议为802.11ag 不适用前导码*/
    if(WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE == pst_dmac_set_non_ht_rate_param->en_protocol_mode)
    {
        pst_tx_alg->ast_per_rate[0].rate_bit_stru.bit_preamble_mode = 0;
        pst_tx_alg->ast_per_rate[1].rate_bit_stru.bit_preamble_mode = 0;
        pst_tx_alg->ast_per_rate[2].rate_bit_stru.bit_preamble_mode = 0;
        pst_tx_alg->ast_per_rate[3].rate_bit_stru.bit_preamble_mode = 0;
    }

    /*使用长前导码，支持速率1M*/
    if (WLAN_LEGACY_11b_RESERVED1 == pst_dmac_set_non_ht_rate_param->en_rate)
    {
        pst_tx_alg->ast_per_rate[0].rate_bit_stru.bit_preamble_mode = WLAN_LEGACY_11B_DSCR_LONG_PREAMBLE;
        pst_tx_alg->ast_per_rate[1].rate_bit_stru.bit_preamble_mode = WLAN_LEGACY_11B_DSCR_LONG_PREAMBLE;
        pst_tx_alg->ast_per_rate[2].rate_bit_stru.bit_preamble_mode = WLAN_LEGACY_11B_DSCR_LONG_PREAMBLE;
        pst_tx_alg->ast_per_rate[3].rate_bit_stru.bit_preamble_mode = WLAN_LEGACY_11B_DSCR_LONG_PREAMBLE;
    }

    pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate = pst_dmac_set_non_ht_rate_param->en_rate;
    pst_tx_alg->ast_per_rate[1].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate = pst_dmac_set_non_ht_rate_param->en_rate;
    pst_tx_alg->ast_per_rate[2].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate = pst_dmac_set_non_ht_rate_param->en_rate;
    pst_tx_alg->ast_per_rate[3].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate = pst_dmac_set_non_ht_rate_param->en_rate;

    pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = pst_dmac_set_non_ht_rate_param->en_protocol_mode;
    pst_tx_alg->ast_per_rate[1].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = pst_dmac_set_non_ht_rate_param->en_protocol_mode;
    pst_tx_alg->ast_per_rate[2].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = pst_dmac_set_non_ht_rate_param->en_protocol_mode;
    pst_tx_alg->ast_per_rate[3].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = pst_dmac_set_non_ht_rate_param->en_protocol_mode;
    OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_set_legacy_rate::set 802.11a/b/g rate successful!rate=[%d], rate index[%d]",
       g_auc_legacy_rate_idx_table[pst_dmac_set_non_ht_rate_param->en_rate], pst_dmac_set_non_ht_rate_param->en_rate);
    return;
}


oal_void  dmac_config_set_dscr_legacy_rate(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
        {
            dmac_vap_set_legacy_rate(pst_dmac_vap, &(pst_dmac_vap->st_tx_alg), l_value);
            break;
        }
        case MAC_VAP_CONFIG_MCAST_DATA:
        {
            dmac_vap_set_legacy_rate(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_mcast), l_value);
            break;
        }
        case MAC_VAP_CONFIG_BCAST_DATA:
        {
            dmac_vap_set_legacy_rate(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_bcast), l_value);
            break;
        }
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
        {
            dmac_vap_set_legacy_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_ucast[0]), l_value);
            break;
        }
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
        {
            dmac_vap_set_legacy_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_ucast[1]), l_value);
            break;
        }
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
        {
            dmac_vap_set_legacy_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_bmcast[0]), l_value);
            break;
        }
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
        {
            dmac_vap_set_legacy_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_bmcast[1]), l_value);
            break;
        }
        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_legacy_rate::invalid frame type=%d", uc_type);
            break;
    }
}


oal_void  dmac_vap_set_mcs_rate(dmac_vap_stru * OAL_CONST pst_dmac_vap,  hal_tx_txop_alg_stru *pst_tx_alg, oal_int32 l_value)
{
    if(pst_tx_alg->st_rate.en_channel_bandwidth > WLAN_BAND_ASSEMBLE_40M_DUP)
    {
       OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_set_mcs_rate::802.11n can't run at over 40MHz bandwidth,use 'set_ucast_data bw 40'");
       return;
    }

    /* 如果此时工作在单流，配置双流速率失效 */
    if((pst_dmac_vap->st_vap_base_info.en_vap_rx_nss == WLAN_SINGLE_NSS)&&
        ((oal_uint8)l_value >= WLAN_HT_MCS8) && ((oal_uint8)l_value != WLAN_HT_MCS32))
    {
       OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_set_mcs_rate::set 802.11n mcs%d fail on WLAN_SINGLE_NSS!", l_value);
       return;
    }

    /*协议切换，重置前导码*/
    if(WLAN_HT_PHY_PROTOCOL_MODE != pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode
    ||WLAN_HT_PHY_PROTOCOL_MODE != pst_tx_alg->ast_per_rate[1].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode
    ||WLAN_HT_PHY_PROTOCOL_MODE != pst_tx_alg->ast_per_rate[2].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode
    ||WLAN_HT_PHY_PROTOCOL_MODE != pst_tx_alg->ast_per_rate[3].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode)
    {
       pst_tx_alg->ast_per_rate[0].rate_bit_stru.bit_preamble_mode = 0;
       pst_tx_alg->ast_per_rate[1].rate_bit_stru.bit_preamble_mode = 0;
       pst_tx_alg->ast_per_rate[2].rate_bit_stru.bit_preamble_mode = 0;
       pst_tx_alg->ast_per_rate[3].rate_bit_stru.bit_preamble_mode = 0;
       OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_set_mcs_rate::protocol mode is changed, resset preamble mode to 0");
    }

    pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs = (oal_uint8)l_value;
    pst_tx_alg->ast_per_rate[1].rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs = (oal_uint8)l_value;
    pst_tx_alg->ast_per_rate[2].rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs = (oal_uint8)l_value;
    pst_tx_alg->ast_per_rate[3].rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs = (oal_uint8)l_value;
    pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
    pst_tx_alg->ast_per_rate[1].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
    pst_tx_alg->ast_per_rate[2].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
    pst_tx_alg->ast_per_rate[3].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;

    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_set_mcs_rate::set 802.11n mcs%d successful!", l_value);
}


oal_void  dmac_config_set_dscr_mcs(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
        {
            dmac_vap_set_mcs_rate(pst_dmac_vap, &(pst_dmac_vap->st_tx_alg), l_value);
            break;
        }
        case MAC_VAP_CONFIG_MCAST_DATA:
        {
            dmac_vap_set_mcs_rate(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_mcast), l_value);
            break;
        }
        case MAC_VAP_CONFIG_BCAST_DATA:
        {
            dmac_vap_set_mcs_rate(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_bcast), l_value);
            break;
        }
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
        {
            dmac_vap_set_mcs_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_ucast[0]), l_value);
            break;
        }
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
        {
            dmac_vap_set_mcs_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_ucast[1]), l_value);
            break;
        }
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
        {
            dmac_vap_set_mcs_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_bmcast[0]), l_value);
            break;
        }
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
        {
            dmac_vap_set_mcs_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_bmcast[1]), l_value);
            break;
        }
        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_mcs::invalid frame type=%d", uc_type);
            break;
    }
}


oal_void  dmac_vap_set_mcsac_rate(dmac_vap_stru * OAL_CONST pst_dmac_vap,  hal_tx_txop_alg_stru *pst_tx_alg, oal_int32 l_value)
{
    pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs = (oal_uint8)l_value;
    pst_tx_alg->ast_per_rate[1].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs = (oal_uint8)l_value;
    pst_tx_alg->ast_per_rate[2].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs = (oal_uint8)l_value;
    pst_tx_alg->ast_per_rate[3].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs = (oal_uint8)l_value;
    pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
    pst_tx_alg->ast_per_rate[1].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
    pst_tx_alg->ast_per_rate[2].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
    pst_tx_alg->ast_per_rate[3].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
    pst_tx_alg->ast_per_rate[0].rate_bit_stru.bit_preamble_mode = 0;
    pst_tx_alg->ast_per_rate[1].rate_bit_stru.bit_preamble_mode = 0;
    pst_tx_alg->ast_per_rate[2].rate_bit_stru.bit_preamble_mode = 0;
    pst_tx_alg->ast_per_rate[3].rate_bit_stru.bit_preamble_mode = 0;

    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_vap_set_mcsac_rate::set 802.11ac mcs%d successful!", l_value);
}


oal_void  dmac_config_set_dscr_mcsac(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
        {
            dmac_vap_set_mcsac_rate(pst_dmac_vap, &(pst_dmac_vap->st_tx_alg), l_value);
            break;
        }
        case MAC_VAP_CONFIG_MCAST_DATA:
        {
            dmac_vap_set_mcsac_rate(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_mcast), l_value);
            break;
        }
        case MAC_VAP_CONFIG_BCAST_DATA:
        {
            dmac_vap_set_mcsac_rate(pst_dmac_vap, &(pst_dmac_vap->st_tx_data_bcast), l_value);
            break;
        }
        case MAC_VAP_CONFIG_UCAST_MGMT_2G:
        {
            dmac_vap_set_mcsac_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_ucast[0]), l_value);
            break;
        }
        case MAC_VAP_CONFIG_UCAST_MGMT_5G:
        {
            dmac_vap_set_mcsac_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_ucast[1]), l_value);
            break;
        }
        case MAC_VAP_CONFIG_MBCAST_MGMT_2G:
        {
            dmac_vap_set_mcsac_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_bmcast[0]), l_value);
            break;
        }
        case MAC_VAP_CONFIG_MBCAST_MGMT_5G:
        {
            dmac_vap_set_mcsac_rate(pst_dmac_vap, &(pst_dmac_vap->ast_tx_mgmt_bmcast[1]), l_value);
            break;
        }
        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_mcsac::invalid frame type=%d", uc_type);
            break;
    }
}


oal_void  dmac_config_set_dscr_nss(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            /*只有11ac可以配置nss*/
            if(WLAN_VHT_PHY_PROTOCOL_MODE == pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode
            && WLAN_VHT_PHY_PROTOCOL_MODE == pst_dmac_vap->st_tx_alg.ast_per_rate[1].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode
            && WLAN_VHT_PHY_PROTOCOL_MODE == pst_dmac_vap->st_tx_alg.ast_per_rate[2].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode
            && WLAN_VHT_PHY_PROTOCOL_MODE == pst_dmac_vap->st_tx_alg.ast_per_rate[3].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode)
            {
                pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_nss_mode = (oal_uint8)l_value;
                pst_dmac_vap->st_tx_alg.ast_per_rate[1].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_nss_mode = (oal_uint8)l_value;
                pst_dmac_vap->st_tx_alg.ast_per_rate[2].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_nss_mode = (oal_uint8)l_value;
                pst_dmac_vap->st_tx_alg.ast_per_rate[3].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_nss_mode = (oal_uint8)l_value;
                OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_nss::set 802.11ac nss%d successful!", pst_dmac_vap->st_tx_alg.ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_nss_mode+1);
            }
            else
            {
                OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_nss::unsupported protocol");
            }
            break;
        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_nss::invalid frame type=%d", uc_type);
            return;
    }
}


oal_void  dmac_config_set_dscr_bw(oal_int32 l_value,oal_uint8 uc_type, dmac_vap_stru *pst_dmac_vap)
{
    switch (uc_type)
    {
        case MAC_VAP_CONFIG_UCAST_DATA:
            pst_dmac_vap->st_tx_alg.st_rate.en_channel_bandwidth = (oal_uint8)l_value;
            (WLAN_BAND_ASSEMBLE_AUTO == pst_dmac_vap->st_tx_alg.st_rate.en_channel_bandwidth)?(pst_dmac_vap->bit_bw_cmd = 0):(pst_dmac_vap->bit_bw_cmd = 1);
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_bandwidth::set successful!value=%d", pst_dmac_vap->st_tx_alg.st_rate.en_channel_bandwidth);
            break;
        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_config_set_dscr_bw::invalid frame type=%d", uc_type);
            return;
    }
}

oal_void  dmac_config_reg_display_test(mac_vap_stru *pst_mac_vap, dmac_reg_info_stru *pst_reg_info)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint32                      ul_addr;
    oal_uint32                      ul_val = 0;

    if (pst_reg_info->ul_start_addr % 4 != 0 || pst_reg_info->ul_end_addr % 4 != 0)
    {
        return;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_reg_display_test::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return;
    }

    for (ul_addr = pst_reg_info->ul_start_addr; ul_addr <= pst_reg_info->ul_end_addr; ul_addr += 4)
    {
        hal_reg_info(pst_hal_device, ul_addr, &ul_val);
        OAM_WARNING_LOG3(0, OAM_SF_CFG, "{dmac_config_reg_display_test::device[%d], reg_info addr=0x%x, value=0x%x",
                                pst_hal_device->uc_device_id, ul_addr, ul_val);
        OAL_IO_PRINT("dmac_config_reg_display_test::reg_info addr=0x%x, value=0x%x\r\n", ul_addr, ul_val);
    }
}
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))

oal_void  dmac_config_report_efuse_reg(mac_vap_stru *pst_mac_vap)
{
    oal_uint32                      ul_addr;
    oal_uint16                      us_val = 0;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
    oal_uint16                      g_us_efuse_reg_buffer[16];
#else
    oal_uint16                      g_us_efuse_reg_buffer[32];
#endif
    oal_uint8                       uc_reg_num = 0;
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_report_efuse_reg::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
    for (ul_addr = HI1102_SOC_GLB_CTL_RB_EFUSE_RD_DATA0_REG; ul_addr <= HI1102_SOC_GLB_CTL_RB_EFUSE_RD_DATA15_REG; ul_addr += 4)
#else
    for (ul_addr = HI1103_PMU2_CMU_IR_EFUSE_RD_DATA0_REG; ul_addr <= HI1103_PMU2_CMU_IR_EFUSE_RD_DATA31_REG; ul_addr += 4)
#endif
    {
        hal_reg_info16(pst_hal_device, ul_addr, &us_val);
        g_us_efuse_reg_buffer[uc_reg_num] = us_val;
        uc_reg_num++;
    }

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_REG_INFO, OAL_SIZEOF(g_us_efuse_reg_buffer), (oal_uint8 *)&g_us_efuse_reg_buffer);
}

#endif


oal_uint32  dmac_config_reg_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int8             *pc_token;
    oal_int8             *pc_end;
    oal_int8             *pc_ctx;
    oal_int8             *pc_sep = " ";
    oal_bool_enum_uint8   en_reg_info32 = OAL_TRUE;

    dmac_reg_info_stru    st_dmac_reg_info = {0};

    /* 选择读取单位(32/16) */
    pc_token = oal_strtok((oal_int8 *)puc_param, pc_sep, &pc_ctx);
    if (NULL == pc_token)
    {
        return OAL_FAIL;
    }

    /*lint -e960*/
    /* 兼容原有"regtype(soc/mac/phy/all) startaddr endaddr"命令格式，默认32位读取寄存器 */
    if (0 == oal_strcmp(pc_token, "all"))
    {
#ifdef _PRE_WLAN_DFT_STAT
        dmac_dft_report_all_ota_state(pst_mac_vap);
#endif
        return OAL_SUCC;
    }
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
    /*重要: 产线读取efuse字段信息*/
    if (0 == oal_strcmp(pc_token, "efuse"))
    {
        dmac_config_report_efuse_reg(pst_mac_vap);
        return OAL_SUCC;
    }
#endif
    if ((0 != oal_strcmp(pc_token, "soc")) &&
    (0 != oal_strcmp(pc_token, "mac")) &&
    (0 != oal_strcmp(pc_token, "phy")))
    {
        if (0 == oal_strcmp(pc_token, "16"))
        {
            en_reg_info32 = OAL_FALSE;
        }

        /* 获取要读取的寄存器类型 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (NULL == pc_token)
        {
            return OAL_FAIL;
        }
    }

    /*lint +e960*/

    st_dmac_reg_info.pc_reg_type = pc_token;

    /* 获取起始地址 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (NULL == pc_token)
    {
        return OAL_FAIL;
    }

    st_dmac_reg_info.ul_start_addr = (oal_uint32)oal_strtol(pc_token, &pc_end, 16);

    /* 获取终止地址 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (NULL == pc_token)
    {
        return OAL_FAIL;
    }

    st_dmac_reg_info.ul_end_addr = (oal_uint32)oal_strtol(pc_token, &pc_end, 16);

    if (OAL_TRUE == en_reg_info32)
    {
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
            /*结束地址与开始地址一样，只读取一个寄存器的值，这时候会把值上报 */
        if(st_dmac_reg_info.ul_end_addr == st_dmac_reg_info.ul_start_addr)
        {
            dmac_config_reg_report(pst_mac_vap, st_dmac_reg_info.ul_start_addr);
#ifdef _PRE_WLAN_PRODUCT_1151V200
            dmac_config_reg_display_test(pst_mac_vap, &st_dmac_reg_info);
#endif
            return OAL_SUCC;
        }
#endif
        /*lint -e960*/
        dmac_config_reg_display_test(pst_mac_vap, &st_dmac_reg_info);
        /*lint +e960*/
        return OAL_SUCC;
    }
    /*lint -e960*/
    g_st_dmac_config_rom_cb.p_dmac_config_reg_display_test16(pst_mac_vap, &st_dmac_reg_info);
    /*lint +e960*/
    return OAL_SUCC;
}


oal_void  dmac_config_reg_write_test(mac_vap_stru *pst_mac_vap, dmac_reg_write_stru *pst_reg_write)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_int8                        ac_buf[64] = {0};

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_reg_write_test::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return;
    }

    if (pst_reg_write->ul_addr % 4 != 0)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_config_reg_write_test::ul_addr=%d", pst_reg_write->ul_addr);
        return;
    }

    /*lint -save -e718 */
    /*lint -save -e746 */
    OAL_SPRINTF((char*)ac_buf, OAL_SIZEOF(ac_buf), "reg_write:addr=0x%08x, val=0x%08x.\n", pst_reg_write->ul_addr, pst_reg_write->ul_val);
    /*lint -restore */
    /*lint -restore */
    OAM_WARNING_LOG3(0, OAM_SF_CFG, "{dmac_config_reg_write_test::device[%d], reg_write:addr=0x%08x, val=0x%08x.",
                                pst_hal_device->uc_device_id, pst_reg_write->ul_addr, pst_reg_write->ul_val);
    oam_print(ac_buf);

    hal_reg_write(pst_hal_device, pst_reg_write->ul_addr, pst_reg_write->ul_val);
}


oal_uint32  dmac_config_set_bss_type(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    /* 设置mib值 */
    return mac_mib_set_bss_type(pst_mac_vap, uc_len, puc_param);
}

oal_uint32  dmac_config_set_ssid(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    /* 设置mib值 */
    return mac_mib_set_ssid(pst_mac_vap, uc_len, puc_param);
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_config_set_shortgi(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    shortgi_cfg_stru    *shortgi_cfg;

    shortgi_cfg = (shortgi_cfg_stru *)puc_param;

    switch(shortgi_cfg->uc_shortgi_type)
    {
        case SHORTGI_20_CFG_ENUM:

            if (0 != shortgi_cfg->uc_enable)
            {
                mac_mib_set_ShortGIOptionInTwentyImplemented(pst_mac_vap, OAL_TRUE);
            }
            else
            {
                mac_mib_set_ShortGIOptionInTwentyImplemented(pst_mac_vap, OAL_FALSE);
            }
            break;

        case SHORTGI_40_CFG_ENUM:

            if (0 != shortgi_cfg->uc_enable)
            {
                mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_TRUE);
            }
            else
            {
                mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_FALSE);
            }
            break;

        case SHORTGI_80_CFG_ENUM:

            if (0 != shortgi_cfg->uc_enable)
            {
                mac_mib_set_VHTShortGIOptionIn80Implemented(pst_mac_vap, OAL_TRUE);
            }
            else
            {
                mac_mib_set_VHTShortGIOptionIn80Implemented(pst_mac_vap, OAL_FALSE);
            }
            break;
        default:
            break;

    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_STA_PM
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD

oal_uint32 dmac_config_set_uapsd_para(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_uapsd_sta_stru   *pst_uapsd_info = (mac_cfg_uapsd_sta_stru *)puc_param;

    /* uc_max_sp_len */
    if (pst_uapsd_info->uc_max_sp_len > 6)
    {
       OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_set_uapsd_para::uc_max_sp_len[%d] > 6!}\r\n", pst_uapsd_info->uc_max_sp_len);
       return OAL_FAIL;
    }

    mac_vap_set_uapsd_para(pst_mac_vap, pst_uapsd_info);

    return OAL_SUCC;
}
#endif
#endif


oal_uint32  dmac_config_ch_status_sync(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru *pst_mac_dev;

    if (!pst_mac_vap || !puc_param)
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{dmac_config_ch_status_sync::vap=%p param=%p}", pst_mac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (!pst_mac_dev)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_config_ch_status_sync::null device, id=%d}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_len != OAL_SIZEOF(pst_mac_dev->st_ap_channel_list))
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{dmac_config_ch_status_sync::invalid len=%d sizeof=%d}", uc_len, OAL_SIZEOF(pst_mac_dev->st_ap_channel_list));
        return OAL_FAIL;
    }

    oal_memcopy((oal_uint8 *)(pst_mac_dev->st_ap_channel_list), puc_param, uc_len);

    return OAL_SUCC;
}
#endif  /* _PRE_MULTI_CORE_MODE_OFFLOAD_DMAC */


oal_uint32  dmac_config_user_asoc_state_syn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

    mac_h2d_user_asoc_state_stru *pst_user_asoc_info;
    mac_user_stru                *pst_mac_user;

    pst_user_asoc_info = (mac_h2d_user_asoc_state_stru *)puc_param;

    OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_config_user_asoc_state_syn::us_user_idx = %d, user state = %d.}",
                  pst_user_asoc_info->us_user_idx, pst_user_asoc_info->en_asoc_state);

    /* 获取DMAC模块用户结构体 */
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_user_asoc_info->us_user_idx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TX, "{dmac_config_user_asoc_state_syn::pst_mac_user null.user idx [%d]}", pst_user_asoc_info->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 同步user关联状态 */
    mac_user_set_asoc_state(pst_mac_user, pst_user_asoc_info->en_asoc_state);
#endif
    return OAL_SUCC;
}

oal_void  dmac_config_get_tx_rate_info(hal_tx_txop_alg_stru     *pst_tx_alg,
                                                        mac_data_rate_stru       *pst_mac_rates_11g,
                                                        mac_rate_info_stru       *pst_rate_info)
{
    oal_uint32                          ul_loop = 0;
    oal_uint8                           uc_tx_dscr_protocol_type = 0;
    wlan_legacy_rate_value_enum_uint8   en_legacy_rate = 0;

    /* 初始化清零 */
    OAL_MEMZERO(pst_rate_info, OAL_SIZEOF(*pst_rate_info));

    /* 获取的protocol type取值范围为0-3,其中0为11b type, 1为legacy OFDM type，2为HT type，3为VHT type */
    uc_tx_dscr_protocol_type = pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode;
    switch(uc_tx_dscr_protocol_type)
    {
        /* 11b or legacy OFDM type */
        case 0:
        case 1:
            en_legacy_rate = pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate;

            for (ul_loop = 0; ul_loop < MAC_DATARATES_PHY_80211G_NUM; ul_loop++)
            {
                if (en_legacy_rate == pst_mac_rates_11g[ul_loop].uc_phy_rate)
                {
                    pst_rate_info->legacy = pst_mac_rates_11g[ul_loop].uc_mbps;
                    break;
                }
            }

            /* 未查找到对应的legacy速率，可能是软件配置错误 */
            if (ul_loop >= MAC_DATARATES_PHY_80211G_NUM)
            {
                OAM_ERROR_LOG2(0, OAM_SF_ANY,
                               "{dmac_config_get_tx_rate_info::protocol_type[%d], legacy_rate[%d] is invalid, may be software config error.}",
                               uc_tx_dscr_protocol_type, en_legacy_rate);
            }
            break;

        /* HT type */
        case 2:
            pst_rate_info->flags |= MAC_RATE_INFO_FLAGS_MCS;
            pst_rate_info->mcs = pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs;
            if (pst_tx_alg->ast_per_rate[0].rate_bit_stru.bit_short_gi_enable)
            {
                pst_rate_info->flags |= MAC_RATE_INFO_FLAGS_SHORT_GI;
            }
            break;

        /* VHT type */
        case 3:
            pst_rate_info->flags |= MAC_RATE_INFO_FLAGS_VHT_MCS;
            pst_rate_info->mcs = pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs;
            if (pst_tx_alg->ast_per_rate[0].rate_bit_stru.bit_short_gi_enable)
            {
                pst_rate_info->flags |= MAC_RATE_INFO_FLAGS_SHORT_GI;
            }

            /* 设置nss mode */
            pst_rate_info->nss = pst_tx_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_nss_mode + 1;
            break;

        default:
            OAM_ERROR_LOG1(0, OAM_SF_RSSI,
                           "{dmac_config_get_tx_rate_info:: protocol type[%d] invalid, software config error.}",
                           uc_tx_dscr_protocol_type);
            break;
    }

    /* 根据信道宽度，置对应的标记位 */
    switch (pst_tx_alg->st_rate.en_channel_bandwidth)
    {
        case WLAN_BAND_ASSEMBLE_40M:
        case WLAN_BAND_ASSEMBLE_40M_DUP:
            pst_rate_info->flags |= MAC_RATE_INFO_FLAGS_40_MHZ_WIDTH;
            break;

        case WLAN_BAND_ASSEMBLE_80M:
        case WLAN_BAND_ASSEMBLE_80M_DUP:
            pst_rate_info->flags |= MAC_RATE_INFO_FLAGS_80_MHZ_WIDTH;
            break;

        case WLAN_BAND_ASSEMBLE_160M:
        case WLAN_BAND_ASSEMBLE_160M_DUP:
            pst_rate_info->flags |= MAC_RATE_INFO_FLAGS_160_MHZ_WIDTH;
            break;

        case WLAN_BAND_ASSEMBLE_80M_80M:
            pst_rate_info->flags |= MAC_RATE_INFO_FLAGS_80P80_MHZ_WIDTH;
            break;

        default:
            /* do nothing */
            break;
    }

    OAM_INFO_LOG4(0, OAM_SF_ANY,
                     "{dmac_config_get_tx_rate_info::protocol_type[%d],legacy_rate[%d],short_gi[%d],bandwidth[%d].}",
                     uc_tx_dscr_protocol_type, en_legacy_rate,
                     pst_tx_alg->ast_per_rate[0].rate_bit_stru.bit_short_gi_enable,
                     pst_tx_alg->st_rate.en_channel_bandwidth);

    OAM_INFO_LOG4(0, OAM_SF_ANY,
                     "{dmac_config_get_tx_rate_info::legacy[%d]:mcs[%d]:flags[%d]:nss[%d].}",
                     pst_rate_info->legacy, pst_rate_info->mcs, pst_rate_info->flags, pst_rate_info->nss);

    return;
}

oal_uint32  dmac_config_proc_query_sta_info_event(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru                         *pst_mac_device;
    dmac_vap_stru                           *pst_dmac_vap;
    dmac_user_stru                          *pst_dmac_user;
    dmac_query_request_event                *pst_query_requset_event;
    dmac_query_station_info_response_event   st_query_station_info;
#ifdef _PRE_WLAN_FEATURE_STA_PM
    mac_sta_pm_handler_stru                 *pst_mac_sta_pm_handle;
#endif
    hal_to_dmac_device_stru                 *pst_hal_device = OAL_PTR_NULL;

    pst_query_requset_event     = (dmac_query_request_event *)puc_param;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{dmac_config_proc_query_sta_info_event::pst_mac_device is null, device_id[%d].}",
                         pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    if (OAL_QUERY_STATION_INFO_EVENT == pst_query_requset_event->query_event)
    {
        pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
        if (OAL_PTR_NULL == pst_dmac_vap)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_proc_query_sta_info_event::pst_dmac_vap null.}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
        {
            /*station_info_extend赋值*/
        #ifdef _PRE_WLAN_FEATURE_STA_PM
            pst_mac_sta_pm_handle = &pst_dmac_vap->st_sta_pm_handler;
            if (OAL_FALSE == pst_mac_sta_pm_handle->en_is_fsm_attached)
            {
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_config_proc_query_sta_info_event::pm fsm not attached.}");
                return OAL_FAIL;
            }
            st_query_station_info.st_station_info_extend.ul_bcn_cnt = pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_PSM_BEACON_CNT];
            st_query_station_info.st_station_info_extend.ul_bcn_tout_cnt = pst_mac_sta_pm_handle->aul_pmDebugCount[PM_MSG_BEACON_TIMEOUT_CNT];
        #else
            st_query_station_info.st_station_info_extend.ul_bcn_cnt = 0;
            st_query_station_info.st_station_info_extend.ul_bcn_tout_cnt = 0;
        #endif

            pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);

            st_query_station_info.st_station_info_extend.uc_distance = pst_hal_device->st_hal_alg_stat.en_alg_distance_stat;
            st_query_station_info.st_station_info_extend.uc_cca_intr = pst_hal_device->st_hal_alg_stat.en_adj_intf_state;

            /* sta查询信息赋值 */
            st_query_station_info.query_event        = pst_query_requset_event->query_event;
            /*从dmac获取RSSI,snr*/
            st_query_station_info.c_signal           = (oal_int8)oal_get_real_rssi(pst_dmac_vap->st_query_stats.s_signal);
            st_query_station_info.st_station_info_extend.c_snr_ant0 = pst_dmac_vap->st_query_stats.c_snr_ant0;
            st_query_station_info.st_station_info_extend.c_snr_ant1 = pst_dmac_vap->st_query_stats.c_snr_ant1;

            /*数据包统计，与维测不能放在同一个预编译宏下面*/
            st_query_station_info.ul_rx_packets      = pst_dmac_vap->st_query_stats.ul_drv_rx_pkts;
            st_query_station_info.ul_tx_packets      = pst_dmac_vap->st_query_stats.ul_hw_tx_pkts;
            st_query_station_info.ul_rx_bytes        = pst_dmac_vap->st_query_stats.ul_drv_rx_bytes;
            st_query_station_info.ul_tx_bytes        = pst_dmac_vap->st_query_stats.ul_hw_tx_bytes;
            st_query_station_info.ul_tx_failed       = pst_dmac_vap->st_query_stats.ul_tx_failed;
            st_query_station_info.ul_tx_retries      = pst_dmac_vap->st_query_stats.ul_tx_retries;
            st_query_station_info.ul_rx_dropped_misc = pst_dmac_vap->st_query_stats.ul_rx_dropped_misc;

        }
        else if(WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
        {
            pst_dmac_user = mac_vap_get_dmac_user_by_addr(pst_mac_vap, pst_query_requset_event->auc_query_sta_addr);
            if (OAL_PTR_NULL == pst_dmac_user)
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_proc_query_sta_info_event::pst_dmac_user null.}");
                return OAL_ERR_CODE_PTR_NULL;
            }

            /* sta查询信息赋值 */
            st_query_station_info.query_event        = pst_query_requset_event->query_event;
            /*从算法处获取RSSI*/
            st_query_station_info.c_signal           = (oal_int8)(pst_dmac_user->st_query_stats.l_signal);

            /*lint -e571*/
            OAM_INFO_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX,
                             "{dmac_config_proc_query_sta_info_event::get rssi = %d in ap mode!", st_query_station_info.c_signal);
            /*lint +e571*/

            /*数据包统计，与维测不能放在同一个预编译宏下面*/
            st_query_station_info.ul_rx_packets      = pst_dmac_user->st_query_stats.ul_drv_rx_pkts;
            st_query_station_info.ul_tx_packets      = pst_dmac_user->st_query_stats.ul_hw_tx_pkts;
            st_query_station_info.ul_rx_bytes        = pst_dmac_user->st_query_stats.ul_drv_rx_bytes;
            st_query_station_info.ul_tx_bytes        = pst_dmac_user->st_query_stats.ul_hw_tx_bytes;
            st_query_station_info.ul_tx_failed       = pst_dmac_user->st_query_stats.ul_tx_failed;
            st_query_station_info.ul_tx_retries      = pst_dmac_user->st_query_stats.ul_tx_retries;
            st_query_station_info.ul_rx_dropped_misc = pst_dmac_user->st_query_stats.ul_rx_dropped_misc;
        }
        else
        {
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_RSSI,
                             "{dmac_config_proc_query_sta_info_event::vap_mode[%d] don't support query statistics info.}",
                             pst_mac_vap->en_vap_mode);
            return OAL_FAIL;
        }

        /* 获取发送速率 */
        dmac_config_get_tx_rate_info(&(pst_dmac_vap->st_tx_alg), &(pst_mac_device->st_mac_rates_11g[0]), &(st_query_station_info.st_txrate));

        if (g_st_dmac_config_rom_cb.p_dmac_config_proc_query_sta_info_event)
        {
            (g_st_dmac_config_rom_cb.p_dmac_config_proc_query_sta_info_event)(pst_mac_vap, uc_len, puc_param, &st_query_station_info);
        }

        dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_QUERY_STATION_STATS, OAL_SIZEOF(dmac_query_station_info_response_event), (oal_uint8 *)&st_query_station_info);

    }

    return OAL_SUCC;
}


oal_int8 dmac_config_get_free_power(mac_vap_stru *pst_mac_vap)
{
    oal_int8                            c_free_power;
    wlan_channel_bandwidth_enum_uint8   en_bw;
    wlan_chan_ratio_stru                *pst_chan_ratio;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    hal_to_dmac_device_stru             *pst_hal_device;
#else
    mac_device_stru                     *pst_mac_dev;
#endif

    en_bw = pst_mac_vap->st_channel.en_bandwidth;
    if(en_bw >= WLAN_BAND_WIDTH_BUTT)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_free_power::invalid bw=%d.}",en_bw);
        return DMAC_INVALID_POWER;
    }

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_hal_device = MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_device;
    pst_chan_ratio = &(pst_hal_device->st_chan_ratio);
#else
    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_get_free_power::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_chan_ratio = &(pst_mac_dev->st_chan_ratio);
#endif

    if(WLAN_BAND_WIDTH_20M == en_bw)
    {
        c_free_power = pst_chan_ratio->c_free_power_20M;
    }
    else if((WLAN_BAND_WIDTH_40PLUS == en_bw) || (WLAN_BAND_WIDTH_40MINUS == en_bw))
    {
        c_free_power = pst_chan_ratio->c_free_power_40M;
    }
    else
    {
        c_free_power = pst_chan_ratio->c_free_power_80M;
    }

    return c_free_power;
}


oal_uint32  dmac_config_query_rssi(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_user_stru             *pst_dmac_user;
    mac_cfg_query_rssi_stru    *pst_param;

    pst_param = (mac_cfg_query_rssi_stru *)puc_param;

    pst_dmac_user = mac_res_get_dmac_user(pst_param->us_user_id);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        pst_param->c_rssi       = -127;
    }
    else
    {
        pst_param->c_rssi       = oal_get_real_rssi(pst_dmac_user->s_rx_rssi);
    }
    pst_param->c_free_power     = dmac_config_get_free_power(pst_mac_vap);
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_QUERY_RSSI, OAL_SIZEOF(mac_cfg_query_rssi_stru), (oal_uint8 *)pst_param);

    return OAL_SUCC;
}


oal_uint32  dmac_config_query_psst(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_user_stru             *pst_dmac_user;
    mac_cfg_query_psst_stru    *pst_param;

    pst_param = (mac_cfg_query_psst_stru *)puc_param;

    pst_dmac_user = mac_res_get_dmac_user(pst_param->us_user_id);
    if (OAL_PTR_NULL == pst_dmac_user)
    {
        pst_param->uc_ps_st = DMAC_INVALID_PS_STATE;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_query_psst::INVALID ps state=%d.}",DMAC_INVALID_PS_STATE);
    }
    else
    {
        pst_param->uc_ps_st = pst_dmac_user->bit_ps_mode;
    }

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_QUERY_PSST, OAL_SIZEOF(mac_cfg_query_psst_stru), (oal_uint8 *)pst_param);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_WEB_CMD_COMM
#ifdef _PRE_WLAN_11K_STAT

oal_uint32  dmac_config_query_drop_num(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_user_stru                 *pst_dmac_user;
    mac_cfg_query_drop_num_stru    *pst_param;
    oal_uint8                       uc_tid          = 0;

    pst_param = (mac_cfg_query_drop_num_stru *)puc_param;
    //pst_param->ul_tx_dropped       = 0;
    OAL_MEMZERO(pst_param->aul_tx_dropped, OAL_SIZEOF(oal_uint32)*WLAN_WME_AC_BUTT);

    pst_dmac_user = mac_res_get_dmac_user(pst_param->us_user_id);
    if (OAL_PTR_NULL != pst_dmac_user)
    {
        for (uc_tid = 0; uc_tid < WLAN_TID_MAX_NUM; uc_tid++)//将tid对应到ac上
        {
            //pst_param->ul_tx_dropped += pst_dmac_user->pst_stat_count->aul_tx_dropped[uc_tid];
            pst_param->aul_tx_dropped[WLAN_WME_TID_TO_AC(uc_tid)] += pst_dmac_user->pst_stat_count->aul_tx_dropped[uc_tid];
        }
    }

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_QUERY_DROP_NUM, OAL_SIZEOF(mac_cfg_query_drop_num_stru), (oal_uint8 *)pst_param);

    return OAL_SUCC;
}


oal_uint32  dmac_config_query_tx_delay(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_user_stru                 *pst_dmac_user;
    mac_cfg_query_tx_delay_stru    *pst_param;
    oal_uint8                       uc_tid              = 0;
    dmac_stat_tid_tx_delay_stru    *pst_stat_tx_delay;
    oal_uint64                      ull_tx_delay_sum    = 0;                    /*发送时延:TID入队到上报发送完成中断的时间累加值*/
    oal_uint32                      ul_tx_delay_cnt     = 0;                    /*tx delay的统计个数*/

    pst_param = (mac_cfg_query_tx_delay_stru *)puc_param;
    pst_param->ul_max_tx_delay       = 0;
    pst_param->ul_min_tx_delay       = 0;
    pst_param->ul_ave_tx_delay       = 0;

    pst_dmac_user = mac_res_get_dmac_user(pst_param->us_user_id);
    if (OAL_PTR_NULL != pst_dmac_user)
    {
        for (uc_tid = 0; uc_tid < WLAN_TID_MAX_NUM; uc_tid++)
        {
            pst_stat_tx_delay = pst_dmac_user->pst_stat_tid_tx_delay + uc_tid;
            /*时延累计值*/
            ull_tx_delay_sum += pst_stat_tx_delay->ull_tx_delay_sum;
            ul_tx_delay_cnt += pst_stat_tx_delay->ul_tx_delay_cnt;
            /*时延极限值*/
            if(pst_param->ul_max_tx_delay < pst_stat_tx_delay->ul_max_tx_delay)
            {
                pst_param->ul_max_tx_delay = pst_stat_tx_delay->ul_max_tx_delay;
            }

            if (((0!= pst_stat_tx_delay->ul_min_tx_delay)
                    && ( pst_param->ul_min_tx_delay > pst_stat_tx_delay->ul_min_tx_delay ))
                    || (0 == pst_param->ul_min_tx_delay))
            {
                pst_param->ul_min_tx_delay = pst_stat_tx_delay->ul_min_tx_delay;
            }
        }
        //pst_param->ul_ave_tx_delay       = ull_tx_delay_sum / ul_tx_delay_cnt;
        if(0 != ul_tx_delay_cnt)
        {
            pst_param->ul_ave_tx_delay       = (oal_uint32)oal_div_u64(ull_tx_delay_sum, ul_tx_delay_cnt);
        }
    }

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_QUERY_TX_DELAY, OAL_SIZEOF(mac_cfg_query_tx_delay_stru), (oal_uint8 *)pst_param);

    return OAL_SUCC;
}
#endif
#endif


oal_uint32  dmac_config_query_rate(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_user_stru                      *pst_mac_user;
    dmac_user_stru                     *pst_dmac_user;
    mac_cfg_query_rate_stru            *pst_param;
    dmac_tx_normal_rate_stats_stru     *pst_rate;
    oal_uint32                          ul_ret;

    pst_param = (mac_cfg_query_rate_stru *)puc_param;

    pst_mac_user  = mac_res_get_mac_user(pst_param->us_user_id);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_query_rate: mac user[%d] is null ptr.", pst_param->us_user_id);
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_dmac_user = MAC_GET_DMAC_USER(pst_mac_user);

    ul_ret = dmac_tid_get_normal_rate_stats(pst_mac_user, 0, &pst_rate);
    if(ul_ret != OAL_SUCC)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_query_rate: dmac_tid_get_normal_rate_stats fail");
        return ul_ret;
    }

    pst_param->ul_tx_rate       = pst_rate->ul_rate_kbps;
    pst_param->ul_rx_rate       = pst_dmac_user->ul_rx_rate;
#ifdef _PRE_WLAN_DFT_STAT
    pst_param->uc_cur_per       = pst_rate->uc_per;
    pst_param->uc_bestrate_per  = pst_rate->uc_best_rate_per;
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    pst_param->ul_tx_rate_min   = pst_dmac_user->ul_tx_minrate;
    pst_param->ul_tx_rate_max   = pst_dmac_user->ul_tx_maxrate;
    pst_param->ul_rx_rate_min   = pst_dmac_user->ul_rx_rate_min;
    pst_param->ul_rx_rate_max   = pst_dmac_user->ul_rx_rate_max;

    pst_dmac_user->ul_rx_rate_min = 0;
    pst_dmac_user->ul_rx_rate_max = 0;
    pst_dmac_user->ul_tx_minrate = 0;
    pst_dmac_user->ul_tx_maxrate = 0;
#endif

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_QUERY_RATE, OAL_SIZEOF(mac_cfg_query_rate_stru), (oal_uint8 *)pst_param);

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT

oal_uint32  dmac_config_query_ani(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru    *pst_hal_device = OAL_PTR_NULL;
    mac_cfg_query_ani_stru     *pst_param;

    pst_param = (mac_cfg_query_ani_stru *)puc_param;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);

    pst_param->uc_device_distance       = pst_hal_device->st_hal_alg_stat.en_alg_distance_stat;
    pst_param->uc_intf_state_cca        = pst_hal_device->st_hal_alg_stat.en_adj_intf_state;
    pst_param->uc_intf_state_co         = pst_hal_device->st_hal_alg_stat.en_co_intf_state;

    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_QUERY_ANI, OAL_SIZEOF(mac_cfg_query_ani_stru), (oal_uint8 *)pst_param);

    return OAL_SUCC;
}

#endif

oal_uint32  dmac_config_set_shpreamble(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    /* 设置mib值 */
    mac_mib_set_shpreamble(pst_mac_vap, uc_len, puc_param);
    return OAL_SUCC;
}

oal_uint32  dmac_config_set_prot_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int32 l_value;

    l_value = *((oal_int32 *)puc_param);

    pst_mac_vap->st_protection.en_protection_mode = (oal_uint8)l_value;

    return OAL_SUCC;
}


oal_uint32  dmac_config_set_bintval(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru  *pst_dmac_vap;
    oal_int32       l_value;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_device_stru             *pst_mac_device;
    oal_uint8                    uc_vap_idx;
    mac_vap_stru*               pst_vap;
#endif

    l_value = *((oal_int32 *)puc_param);
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    pst_mac_device              = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_config_set_bintval::pst_mac_device[%d] is NULL!}", pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_device_set_beacon_interval(pst_mac_device, *((oal_uint32 *)puc_param));

    /* 遍历device下所有vap */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_vap)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SCAN, "{hmac_config_set_bintval::pst_mac_vap(%d) null.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        /* 只有AP VAP需要beacon interval */
        if ((WLAN_VAP_MODE_BSS_AP == pst_vap->en_vap_mode))
        {
             /* 设置mib值 */
            mac_mib_set_beacon_period(pst_vap, (oal_uint8)uc_len, puc_param);
        }
    }
#endif

    hal_vap_set_machw_beacon_period(pst_dmac_vap->pst_hal_vap, (oal_uint16)l_value);
    return OAL_SUCC;
}


oal_uint32  dmac_config_set_dtimperiod(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* 设置mib值 */
    mac_mib_set_dtim_period(pst_mac_vap, uc_len, puc_param);
#endif
    return OAL_SUCC;
}


oal_uint32  dmac_config_set_nobeacon(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8            uc_value;
    dmac_vap_stru       *pst_dmac_vap;

    uc_value      = (*((oal_int32 *)puc_param) == 0) ? 0 : 1;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_nobeacon::pst_dmac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (0 != uc_value)
    {
        hal_vap_beacon_suspend(pst_dmac_vap->pst_hal_vap);
    }
    else
    {
        hal_vap_beacon_resume(pst_dmac_vap->pst_hal_vap);
    }

    //OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_set_nobeacon succ!");

    return OAL_SUCC;
}

oal_uint32  dmac_config_set_cwmin(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8   uc_ac;
    oal_uint32 *pul_param;
    oal_uint32  ul_cwmin;

    pul_param = (oal_uint32 *)puc_param;

    uc_ac     = (oal_uint8)pul_param[1];
    ul_cwmin  = pul_param[2];

    if (uc_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_cwmin::invalid param, uc_ac=%d uc_cwmin=%d.", uc_ac, ul_cwmin);
        return OAL_FAIL;
    }

    mac_mib_set_EDCATableCWmin(pst_mac_vap, uc_ac, ul_cwmin);
    return OAL_SUCC;
}



oal_uint32  dmac_config_set_cwmax(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8   uc_ac;
    oal_uint32 *pul_param;
    oal_uint32  ul_cwmax;

    pul_param = (oal_uint32 *)puc_param;

    uc_ac     = (oal_uint8)pul_param[1];
    ul_cwmax  = pul_param[2];

    if (uc_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_cwmax::invalid param, uc_ac=%d ul_cwmax=%d.", uc_ac, ul_cwmax);
        return OAL_FAIL;
    }

    mac_mib_set_EDCATableCWmax(pst_mac_vap, uc_ac, ul_cwmax);

    return OAL_SUCC;
}


oal_uint32  dmac_config_set_aifsn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8   uc_ac;
    oal_uint32  ul_value;
    oal_uint32 *pul_param;

    pul_param = (oal_uint32 *)puc_param;

    uc_ac     = (oal_uint8)pul_param[1];
    ul_value  = pul_param[2];

    if (uc_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_aifsn::invalid param, ul_ac=%d ul_value=%d.", uc_ac, ul_value);
        return OAL_FAIL;
    }

    mac_mib_set_EDCATableAIFSN(pst_mac_vap, uc_ac, ul_value);

    return OAL_SUCC;
}



oal_uint32  dmac_config_set_txop_limit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8   uc_ac;
    oal_uint32  ul_value;
    oal_uint32 *pul_param;

    pul_param = (oal_uint32 *)puc_param;

    uc_ac     = (oal_uint8)pul_param[1];
    ul_value  = pul_param[2];

    if (uc_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_txop_limit::invalid param, uc_ac=%d ul_value=%d.", uc_ac, ul_value);
        return OAL_FAIL;
    }

    mac_mib_set_EDCATableTXOPLimit(pst_mac_vap, uc_ac, ul_value);

    return OAL_SUCC;
}

oal_uint32  dmac_config_set_edca_mandatory(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8   uc_ac;
    oal_uint8   uc_value;
    oal_uint32 *pul_param;

    pul_param = (oal_uint32 *)puc_param;

    uc_ac     = (oal_uint8)pul_param[1];
    uc_value  = (oal_uint8)pul_param[2];

    if (uc_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_edca_mandatory::invalid param, uc_ac=%d ul_value=%d.", uc_ac, uc_value);
        return OAL_FAIL;
    }

    mac_mib_set_EDCATableMandatory(pst_mac_vap, uc_ac, uc_value);

    return OAL_SUCC;
}



oal_uint32  dmac_config_set_qap_cwmin(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8   uc_ac;
    oal_uint32 *pul_param;
    oal_uint8   uc_cwmax = 0;
    oal_uint8   uc_cwmin = 0;
    oal_uint8   uc_cwmin_pre = 0;

    dmac_vap_stru               *pst_dmac_vap;

    pul_param = (oal_uint32 *)puc_param;

    uc_ac     = (oal_uint8)pul_param[1];
    uc_cwmin  = (oal_uint8)pul_param[2];

    if (uc_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_cwmin::invalid param, ul_ac=%d uc_cwmin=%d.", uc_ac, uc_cwmin);
        return OAL_FAIL;
    }

    mac_mib_set_QAPEDCATableCWmin(pst_mac_vap, uc_ac, (oal_uint32)uc_cwmin);

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_cwmin::mac_res_get_dmac_vap fail,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }


    hal_vap_get_edca_machw_cw(pst_dmac_vap->pst_hal_vap, &uc_cwmax, &uc_cwmin_pre, uc_ac);
    hal_vap_set_edca_machw_cw(pst_dmac_vap->pst_hal_vap, uc_cwmax, uc_cwmin, uc_ac);

    return OAL_SUCC;
}



oal_uint32  dmac_config_set_qap_cwmax(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8   uc_ac;
    oal_uint32 *pul_param;
    oal_uint8   uc_cwmax = 0;
    oal_uint8   uc_cwmin = 0;
    oal_uint8   uc_cwmax_pre = 0;

    dmac_vap_stru               *pst_dmac_vap;

    pul_param = (oal_uint32 *)puc_param;

    uc_ac     = (oal_uint8)pul_param[1];
    uc_cwmax  = (oal_uint8)pul_param[2];

    if (uc_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_cwmax::invalid param, uc_ac=%d uc_cwmax=%d.", uc_ac, uc_cwmax);
        return OAL_FAIL;
    }

    mac_mib_set_QAPEDCATableCWmax(pst_mac_vap, uc_ac, (oal_uint32)uc_cwmax);

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_cwmax::mac_res_get_dmac_vap fail,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }

    hal_vap_get_edca_machw_cw(pst_dmac_vap->pst_hal_vap, &uc_cwmax_pre, &uc_cwmin, uc_ac);
    hal_vap_set_edca_machw_cw(pst_dmac_vap->pst_hal_vap, uc_cwmax, uc_cwmin, uc_ac);

    return OAL_SUCC;
}


oal_uint32  dmac_config_set_qap_aifsn(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint8   uc_ac;
    oal_uint32  ul_value;
    oal_uint32 *pul_param;
    dmac_vap_stru               *pst_dmac_vap;

    pul_param = (oal_uint32 *)puc_param;

    uc_ac     = (oal_uint8)pul_param[1];
    ul_value  = pul_param[2];

    if (uc_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_aifsn::invalid param, uc_ac=%d ul_value=%d.", uc_ac, ul_value);
        return OAL_FAIL;
    }

    mac_mib_set_QAPEDCATableAIFSN(pst_mac_vap, uc_ac, ul_value);

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_aifsn::mac_res_get_dmac_vap fail,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }

    hal_vap_set_machw_aifsn_ac(pst_dmac_vap->pst_hal_vap, (wlan_wme_ac_type_enum_uint8)uc_ac, (oal_uint8)ul_value);

    return OAL_SUCC;
}




oal_uint32  dmac_config_set_qap_txop_limit(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32  ul_ac;
    oal_uint32  ul_value;
    oal_uint32 *pul_param;
    oal_uint16  us_txop_bk = 0;
    oal_uint16  us_txop_be = 0;
    oal_uint16  us_txop_vi = 0;
    oal_uint16  us_txop_vo = 0;
    oal_uint16  us_pre_value = 0;

    dmac_vap_stru               *pst_dmac_vap;

    pul_param = (oal_uint32 *)puc_param;

    ul_ac     = pul_param[1];
    ul_value  = pul_param[2];

    if (ul_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_txop_limit::invalid param, ul_ac=%d ul_value=%d.", ul_ac, ul_value);
        return OAL_FAIL;
    }

    mac_mib_set_QAPEDCATableTXOPLimit(pst_mac_vap, (oal_uint8)ul_ac, ul_value);

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_txop_limit::mac_res_get_dmac_vap fail,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }


    switch (ul_ac)
    {
        case WLAN_WME_AC_BK:
            hal_vap_get_machw_txop_limit_bkbe(pst_dmac_vap->pst_hal_vap, &us_txop_be, &us_pre_value);
            hal_vap_set_machw_txop_limit_bkbe(pst_dmac_vap->pst_hal_vap, us_txop_be, (oal_uint16)ul_value);
            break;

        case WLAN_WME_AC_BE:
            hal_vap_get_machw_txop_limit_bkbe(pst_dmac_vap->pst_hal_vap, &us_pre_value, &us_txop_bk);
            hal_vap_set_machw_txop_limit_bkbe(pst_dmac_vap->pst_hal_vap, (oal_uint16)ul_value, us_txop_bk);
            break;

        case WLAN_WME_AC_VI:
            hal_vap_get_machw_txop_limit_vivo(pst_dmac_vap->pst_hal_vap, &us_txop_vo, &us_pre_value);
            hal_vap_set_machw_txop_limit_vivo(pst_dmac_vap->pst_hal_vap, us_txop_vo, (oal_uint16)ul_value);
            break;

        case WLAN_WME_AC_VO:
            hal_vap_get_machw_txop_limit_vivo(pst_dmac_vap->pst_hal_vap, &us_pre_value, &us_txop_vi);
            hal_vap_set_machw_txop_limit_vivo(pst_dmac_vap->pst_hal_vap, (oal_uint16)ul_value, us_txop_vi);
            break;

        default:
            break;
    }

    return OAL_SUCC;
}

oal_uint32  dmac_config_set_qap_edca_mandatory(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32  ul_ac;
    oal_uint32  ul_value;
    oal_uint32 *pul_param;

    pul_param = (oal_uint32 *)puc_param;

    ul_ac     = pul_param[1];
    ul_value  = pul_param[2];

    if (ul_ac >= WLAN_WME_AC_BUTT)
    {
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_qap_edca_mandatory::invalid param, ul_ac=%d ul_value=%d.", ul_ac, ul_value);
        return OAL_FAIL;
    }

    mac_mib_set_QAPEDCATableMandatory(pst_mac_vap, (oal_uint8)ul_ac, (oal_uint8)ul_value);

    return OAL_SUCC;
}

oal_uint32  dmac_config_set_mib_by_bw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_cfg_mib_by_bw_param_stru *pst_cfg = (mac_cfg_mib_by_bw_param_stru*)puc_param;

    mac_vap_change_mib_by_bandwidth(pst_mac_vap, pst_cfg->en_band, pst_cfg->en_bandwidth);
#endif

    return OAL_SUCC;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  dmac_config_wfa_cfg_aifsn(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param)
{
    mac_edca_cfg_stru   *pst_edca_cfg_param;
    dmac_vap_stru               *pst_dmac_vap;

    /* 参数合法性判断 */
    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == puc_param))
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_config_wfa_cfg_aifsn::input params is invalid, %p, %p.}",
                         pst_mac_vap, puc_param);
        return OAL_FAIL;
    }

    pst_edca_cfg_param = (mac_edca_cfg_stru *)puc_param;
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_wfa_cfg_aifsn::mac_res_get_dmac_vap fail or pst_dmac_vap->pst_hal_vap NULL,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }

    hal_vap_set_machw_aifsn_ac_wfa(pst_dmac_vap->pst_hal_vap, pst_edca_cfg_param->en_ac, (oal_uint8)pst_edca_cfg_param->us_val, pst_edca_cfg_param->en_switch);
    return OAL_SUCC;
}


oal_uint32  dmac_config_wfa_cfg_cw(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param)
{
    mac_edca_cfg_stru   *pst_edca_cfg_param;
    dmac_vap_stru               *pst_dmac_vap;

    /* 参数合法性判断 */
    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == puc_param))
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_config_wfa_cfg_cw::input params is invalid, %p, %p.}",
                         pst_mac_vap, puc_param);
        return OAL_FAIL;
    }

    pst_edca_cfg_param = (mac_edca_cfg_stru *)puc_param;
    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_config_wfa_cfg_aifsn::mac_res_get_dmac_vap fail or pst_dmac_vap->pst_hal_vap NULL,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }

    hal_vap_set_edca_machw_cw_wfa(pst_dmac_vap->pst_hal_vap, (oal_uint8)pst_edca_cfg_param->us_val, pst_edca_cfg_param->en_ac, pst_edca_cfg_param->en_switch);
    return OAL_SUCC;
}

#endif /* DMAC_OFFLOAD */


oal_uint32  dmac_config_dump_all_rx_dscr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dump_all_rx_dscr::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_dump_all_rx_dscr(pst_hal_device);

    return OAL_SUCC;
}

oal_uint32  dmac_config_user_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    /* mips????????,??????? */
#ifndef _PRE_WLAN_PROFLING_MIPS
    dmac_user_stru                  *pst_dmac_user;
    mac_cfg_user_info_param_stru    *pst_dmac_event;
    oam_output_type_enum_uint8      en_output_type  = OAM_OUTPUT_TYPE_BUTT;
    oal_uint8                       uc_tid_index;
    //oal_uint32                      ul_tid_mpdu_num    = 0;

    /* 获取dmac user */
    pst_dmac_event = (mac_cfg_user_info_param_stru *)puc_param;
    pst_dmac_user  = (dmac_user_stru *)mac_res_get_dmac_user(pst_dmac_event->us_user_idx);

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_user_info::pst_dmac_user[%d] null.}",
            pst_dmac_event->us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "{dmac_config_user_info::us_assoc_id is %d, last_active_timestamp[%u]}",
                     pst_dmac_user->st_user_base_info.us_assoc_id, pst_dmac_user->ul_last_active_timestamp);
#if 0
    for (uc_tid_index = 0; uc_tid_index < WLAN_TID_MAX_NUM; uc_tid_index ++)
    {
        ul_tid_mpdu_num += pst_dmac_user->ast_tx_tid_queue[uc_tid_index].us_mpdu_num;
        if(pst_dmac_user->ast_tx_tid_queue[uc_tid_index].us_mpdu_num)
        {
            OAL_IO_PRINT("TID[%d],mpdu_num[%d],retry[%d],paused[%d]\r\n",
                   uc_tid_index,
                   pst_dmac_user->ast_tx_tid_queue[uc_tid_index].us_mpdu_num,
                   pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_retry_num,
                   pst_dmac_user->ast_tx_tid_queue[uc_tid_index].c_is_paused);
        }
    }
    OAL_IO_PRINT("dmac_config_user_info:totoal tid_mpdu_num %d\r\n", ul_tid_mpdu_num);
#endif
    oam_get_output_type(&en_output_type);
    if (OAM_OUTPUT_TYPE_SDT != en_output_type)
    {
#if 0
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_lut_index :        %d \n", pst_dmac_user->uc_lut_index);
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "bit_ps :              %d \n", pst_dmac_user->bit_ps_mode);
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "en_vip_flag :         %d \n", pst_dmac_user->en_vip_flag);
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "en_smartant_training :%d \n", pst_dmac_user->en_smartant_training);
        OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "\n");

        for (uc_tid_index = 0; uc_tid_index < WLAN_TID_MAX_NUM; uc_tid_index ++)
        {
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "tid               %d \n", uc_tid_index);
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_is_paused : %d \n", (oal_uint8)pst_dmac_user->ast_tx_tid_queue[uc_tid_index].c_is_paused);
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_num_dq : %d \n", pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_num_dq);
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_retry_num : %d \n", pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_retry_num);
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "us_mpdu_num : %d \n", pst_dmac_user->ast_tx_tid_queue[uc_tid_index].us_mpdu_num);
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "ul_mpdu_avg_len : %d \n", pst_dmac_user->ast_tx_tid_queue[uc_tid_index].ul_mpdu_avg_len);
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_num_tx_ba : %d \n", pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_num_tx_ba);
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_num_rx_ba : %d \n", pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_num_rx_ba);
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "pst_ba_tx_hdl : %u \n", (oal_uint32)pst_dmac_user->ast_tx_tid_queue[uc_tid_index].pst_ba_tx_hdl);
            OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "pst_ba_rx_hdl : %u \n\n", (oal_uint32)pst_dmac_user->ast_tx_tid_queue[uc_tid_index].pst_ba_rx_hdl);
        }

        OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "power save related user_info:\n");
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "mpdu num in ps_queue-->%d\n", oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num));
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "user ps mode is -->%d\n", pst_dmac_user->bit_ps_mode);
        OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "pspoll process state is -->%d\n", pst_dmac_user->st_ps_structure.en_is_pspoll_rsp_processing);
#endif

        OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_lut_index[%d]; bit_ps[%d];en_vip_flag[%d];en_smartant_training[%d]\n",
                      pst_dmac_user->uc_lut_index, pst_dmac_user->bit_ps_mode, pst_dmac_user->en_vip_flag, pst_dmac_user->en_smartant_training);

        for (uc_tid_index = 0; uc_tid_index < WLAN_TID_MAX_NUM; uc_tid_index++)
        {
            OAM_INFO_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "tid[%d]; uc_is_paused[%d]\n",
                          uc_tid_index, (oal_uint8)pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_is_paused);

            OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_num_dq[%d]; uc_retry_num[%d]; us_mpdu_num[%d]; ul_mpdu_avg_len[%d]\n",
                          pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_num_dq,
                          pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_retry_num,
                          pst_dmac_user->ast_tx_tid_queue[uc_tid_index].us_mpdu_num,
                          pst_dmac_user->ast_tx_tid_queue[uc_tid_index].ul_mpdu_avg_len);

            OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_num_tx_ba[%d]; uc_num_rx_ba[%d]; pst_ba_tx_hdl[%u]; rx ba status[%d]\n",
                          pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_num_tx_ba,
                          pst_dmac_user->ast_tx_tid_queue[uc_tid_index].uc_num_rx_ba,
                          (oal_uint32)pst_dmac_user->ast_tx_tid_queue[uc_tid_index].pst_ba_tx_hdl,
                          (oal_uint32)pst_dmac_user->ast_tx_tid_queue[uc_tid_index].st_ba_rx_hdl.en_ba_conn_status);
        }

        OAM_INFO_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "power save related user_info:mpdu num in ps_queue[%d]; user ps mode[%d]; pspoll process state[%d]\n",
                      oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num),
                      pst_dmac_user->bit_ps_mode,
                      pst_dmac_user->st_ps_structure.en_is_pspoll_rsp_processing);
    }
    else
    {
        oam_ota_report((oal_uint8 *)&pst_dmac_user->uc_lut_index,
                       (oal_uint16)(OAL_SIZEOF(dmac_user_stru) - OAL_SIZEOF(mac_user_stru)),
                       0, 0, OAM_OTA_TYPE_DMAC_USER);
    }
#endif
    return OAL_SUCC;
}

oal_uint32 dmac_config_set_log_level(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    // 设置device log level，业务添加处理逻辑
    return oam_log_set_vap_level(pst_mac_vap->uc_vap_id, *puc_param);
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
oal_uint32 dmac_config_set_log_lowpower(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    // 设置device log level，业务添加处理逻辑
    oal_uint8      uc_param = *(oal_uint8 *)puc_param;
    return oam_log_set_pm_enable((oal_uint8)uc_param) ;
}
/*****************************************************************************
 . . .  : dmac_config_get_ant
 ....  : ......
 ....  : pst_mac_vap: ..VAP...
             uc_len     : ....
             puc_param  : ..
 ....  : .
 . . .  : ...
 ....  :
 ....  :

 ....      :
  1..    .   : 2014.3.28.
    .    .   : zhangyu
    ....   : .....
*****************************************************************************/
oal_uint32  dmac_config_get_ant(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)

{
    oal_uint8                    uc_result;

    dmac_atcmdsrv_atcmd_response_event    st_atcmdsrv_dbb_num_info;

    oal_get_gpio_level(BCPU_GPIO_BASE, WLAN_ANT_GPIO, &uc_result);
    OAM_WARNING_LOG1(0, OAM_SF_PWR, "dmac_config_get_ant  ant:%d", uc_result);

    st_atcmdsrv_dbb_num_info.uc_event_id = OAL_ATCMDSRV_GET_ANT;
    st_atcmdsrv_dbb_num_info.ul_event_para = uc_result;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_GET_ANT, OAL_SIZEOF(dmac_atcmdsrv_atcmd_response_event), (oal_uint8 *)&st_atcmdsrv_dbb_num_info);


    return OAL_SUCC;
}


#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

oal_bool_enum_uint8 is_fcc_country(oal_int8* country_code)
{
    oal_int8   *ac_fcc_country[] = {"AR", "BR", "CA", "CL", "IN", "MX", "PR", "US", "VE", OAL_PTR_NULL};
    oal_int8  **pp_country = ac_fcc_country;

    while (*pp_country != OAL_PTR_NULL)
    {
        if (oal_memcmp(country_code, *(pp_country++), WLAN_COUNTRY_STR_LEN) == 0)
        {
            return OAL_TRUE;
        }
    }
    return OAL_FALSE;
}
#endif

oal_uint32  dmac_config_set_random_mac_oui(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru  *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_DFS, "dmac_config_set_random_mac_oui::pst_mac_device null.device:%u",pst_mac_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(uc_len < WLAN_RANDOM_MAC_OUI_LEN)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_config_set_random_mac_oui::len is short:%d.}", uc_len);
        return OAL_FAIL;
    }

    oal_memcopy(pst_mac_device->auc_mac_oui, puc_param, WLAN_RANDOM_MAC_OUI_LEN);

    OAM_WARNING_LOG3(0, OAM_SF_ANY, "{dmac_config_set_random_mac_oui::mac_ou:0x%.2x:%.2x:%.2x}\r\n",
                     pst_mac_device->auc_mac_oui[0],
                     pst_mac_device->auc_mac_oui[1],
                     pst_mac_device->auc_mac_oui[2]);

    return OAL_SUCC;
}
#if 0

oal_uint32  dmac_config_list_channel(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_regdomain_info_stru *pst_regdomain_info                     = OAL_PTR_NULL;
    oal_uint8                uc_chan_num;
    oal_uint8                uc_chan_idx;
    oal_uint32               ul_ret                                 = OAL_FAIL;
    oal_int8                 ac_string[OAM_PRINT_FORMAT_LENGTH];
    oal_int8                *pc_str_offset;
    mac_regclass_info_stru  *pst_rc_info;

    mac_get_regdomain_info(&pst_regdomain_info);

    OAL_SPRINTF(ac_string, sizeof(ac_string), "Country is: %s %s\n",
                pst_regdomain_info->ac_country,
                "2G(chan_num reg_tx_pwr max_tx_pwr):\n");
    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_2_BUTT; uc_chan_idx++)
    {
        pc_str_offset = ac_string + OAL_STRLEN(ac_string);
        ul_ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_2, uc_chan_idx);
        if (OAL_SUCC == ul_ret)
        {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_2, uc_chan_idx, &uc_chan_num);
            pst_rc_info = mac_get_channel_idx_rc_info(WLAN_BAND_2G, uc_chan_idx);
            OAL_IO_PRINT("%d,%d,%d\r\n", uc_chan_num, pst_rc_info->uc_max_reg_tx_pwr, pst_rc_info->uc_max_tx_pwr);
            OAL_SPRINTF(pc_str_offset, 10, "%d %d %d\n", uc_chan_num, pst_rc_info->uc_max_reg_tx_pwr, pst_rc_info->uc_max_tx_pwr);
        }
    }

    oam_print(ac_string);

    OAL_SPRINTF(ac_string, sizeof(ac_string), "%s", "5G(chan_num reg_tx_pwr max_tx_pwr):\n");
    for (uc_chan_idx = 0; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT/2; uc_chan_idx++)
    {
        pc_str_offset = ac_string + OAL_STRLEN(ac_string);
        ul_ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_chan_idx);
        if (OAL_SUCC == ul_ret)
        {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, uc_chan_idx, &uc_chan_num);
            pst_rc_info = mac_get_channel_idx_rc_info(WLAN_BAND_5G, uc_chan_idx);
            OAL_SPRINTF(pc_str_offset, 12, "%d %d %d\n", uc_chan_num, pst_rc_info->uc_max_reg_tx_pwr, pst_rc_info->uc_max_tx_pwr);
        }
    }
    oam_print(ac_string);

    ac_string[0] = '\0';
    for (uc_chan_idx = MAC_CHANNEL_FREQ_5_BUTT/2; uc_chan_idx < MAC_CHANNEL_FREQ_5_BUTT; uc_chan_idx++)
    {
        pc_str_offset = ac_string + OAL_STRLEN(ac_string);
        ul_ret = mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, uc_chan_idx);
        if (OAL_SUCC == ul_ret)
        {
            mac_get_channel_num_from_idx(MAC_RC_START_FREQ_5, uc_chan_idx, &uc_chan_num);
            pst_rc_info = mac_get_channel_idx_rc_info(WLAN_BAND_5G, uc_chan_idx);
            OAL_SPRINTF(pc_str_offset, 12, "%d %d %d\n", uc_chan_num, pst_rc_info->uc_max_reg_tx_pwr, pst_rc_info->uc_max_tx_pwr);
        }
    }
    oam_print(ac_string);
    return OAL_SUCC;
}
#endif
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32   dmac_config_set_regdomain_pwr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_regdomain_max_pwr_stru *pst_cfg;

    pst_cfg = (mac_cfg_regdomain_max_pwr_stru *)puc_param;

    mac_regdomain_set_max_power(pst_cfg->uc_pwr, pst_cfg->en_exceed_reg);

    return OAL_SUCC;

}
#endif
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX

oal_uint32  dmac_config_set_bw(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_tx_comp_stru            *pst_event_set_bw;
    dmac_vap_stru                   *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_bw::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }


     /* 设置数据 */
    pst_event_set_bw = (mac_cfg_tx_comp_stru *)puc_param;
    pst_dmac_vap->st_tx_data_mcast.st_rate.en_channel_bandwidth = pst_event_set_bw->uc_param;

    pst_dmac_vap->uc_bw_flag = pst_event_set_bw->uc_param;

    /* 常发模式下在线配置 */
    if (OAL_SWITCH_ON == pst_dmac_vap->pst_hal_device->bit_al_tx_flag)
    {
        hal_set_tx_dscr_field(pst_dmac_vap->pst_hal_device, pst_event_set_bw->uc_param, HAL_RF_TEST_BAND_WIDTH);
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX

oal_uint32 dmac_config_al_tx_set_addresses(
                mac_vap_stru                           *pst_vap,
                mac_ieee80211_frame_stru  *pst_hdr)
{
    /* From DS标识位设置 */
    mac_hdr_set_from_ds((oal_uint8 *)pst_hdr, 0);

    /* to DS标识位设置 */
    mac_hdr_set_to_ds((oal_uint8 *)pst_hdr, 1);

    /* Set Address1 field in the WLAN Header with BSSID */
    oal_set_mac_addr(pst_hdr->auc_address1, BROADCAST_MACADDR);


    /* Set Address2 field in the WLAN Header with the source address */
    oal_set_mac_addr(pst_hdr->auc_address2, mac_mib_get_StationID(pst_vap));

    /* Set Address3 field in the WLAN Header with the destination address */
    oal_set_mac_addr(pst_hdr->auc_address3, BROADCAST_MACADDR);

    return OAL_SUCC;
}


oal_uint32 dmac_config_al_tx_set_frame_ctrl(
                                                      mac_tx_ctl_stru                         *pst_tx_ctl,
                                                      mac_ieee80211_frame_stru  *pst_hdr)
{
    /* 设置帧控制字段 */
    mac_hdr_set_frame_control((oal_uint8 *)pst_hdr, WLAN_FC0_TYPE_DATA | WLAN_FC0_SUBTYPE_DATA);

    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl) = MAC_80211_FRAME_LEN;

    return OAL_SUCC;
}


oal_uint32 dmac_config_al_tx_packet(mac_vap_stru *pst_vap, oal_netbuf_stru *pst_buf, oal_uint32 ul_len)
{
    mac_tx_ctl_stru                     *pst_tx_ctl;       /* SKB CB */
    mac_ieee80211_qos_htc_frame_stru    *pst_hdr;

    /* 初始化CB tx rx字段 , CB字段在前面已经被清零， 在这里不需要重复对某些字段赋零值*/

    /* netbuff需要申请内存  */
    pst_tx_ctl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_buf);
    OAL_MEMZERO(pst_tx_ctl, OAL_SIZEOF(mac_tx_ctl_stru));
    MAC_GET_CB_MPDU_NUM(pst_tx_ctl)               = 1;
    MAC_GET_CB_NETBUF_NUM(pst_tx_ctl)             = 1;
    MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_ctl)             = WLAN_DATA_BASICTYPE;
    MAC_GET_CB_TX_VAP_INDEX(pst_tx_ctl)           = pst_vap->uc_vap_id;
    MAC_GET_CB_EVENT_TYPE(pst_tx_ctl)             = FRW_EVENT_TYPE_WLAN_DTX;
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl)            = pst_vap->us_multi_user_idx;
    MAC_GET_CB_FRAME_HEADER_LENGTH(pst_tx_ctl)    = MAC_80211_FRAME_LEN;
    MAC_GET_CB_IS_MCAST(pst_tx_ctl)               = OAL_TRUE;

    /* ack policy暂不用 */
    MAC_GET_CB_ACK_POLACY(pst_tx_ctl)            = WLAN_TX_NO_ACK;
    MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl) = WLAN_TIDNO_BCAST;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl)   = WLAN_WME_AC_VO;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    pst_hdr = (mac_ieee80211_qos_htc_frame_stru *)OAL_NETBUF_HEADER(pst_buf);
#else
    MAC_SET_CB_80211_MAC_HEAD_TYPE(pst_tx_ctl, 1);/*指示mac头部在skb中*/
    pst_hdr = (mac_ieee80211_qos_htc_frame_stru *)(OAL_NETBUF_HEADER(pst_buf)- (MAC_80211_QOS_HTC_4ADDR_FRAME_LEN - (ETHER_HDR_LEN - SNAP_LLC_FRAME_LEN)));
#endif

    MAC_GET_CB_MPDU_LEN(pst_tx_ctl)   = (oal_uint16)ul_len;
    pst_hdr->bit_qc_amsdu = OAL_FALSE;

    /* 挂接802.11头,在data_tx里会将802.11头全部清0 */
    dmac_config_al_tx_set_frame_ctrl(pst_tx_ctl,(mac_ieee80211_frame_stru *)pst_hdr);
    dmac_config_al_tx_set_addresses(pst_vap, (mac_ieee80211_frame_stru *)pst_hdr);
    MAC_SET_CB_FRAME_HEADER_ADDR(pst_tx_ctl, (mac_ieee80211_frame_stru *)pst_hdr);

    return OAL_SUCC;
}

oal_netbuf_stru*  dmac_config_create_al_tx_packet(oal_uint32 ul_size,
                                                    mac_rf_payload_enum_uint8        en_payload_flag)
{
    oal_netbuf_stru         *pst_buf;
    oal_uint32               ul_loop = 0;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    oal_uint32               l_reserve = 256;
#endif
    oal_uint8                       *puc_data;

    /* 申请netbuf */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if(WLAN_LARGE_NETBUF_SIZE >= ul_size)
    {
        pst_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, (oal_uint16)ul_size, OAL_NETBUF_PRIORITY_MID);
    }
    else
    {
        pst_buf = OAL_MEM_MULTI_NETBUF_ALLOC((oal_uint16)ul_size, OAL_MULTI_NETBUF_NORMAL);
    }
#else
    pst_buf = oal_netbuf_alloc(ul_size+l_reserve, (oal_int32)l_reserve, 4);
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_buf))
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "dmac_config_create_al_tx_packet::alloc Fail");
        return OAL_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    oal_netbuf_put(pst_buf, ul_size);
#endif

    puc_data = OAL_NETBUF_DATA(pst_buf);
    switch (en_payload_flag)
    {
        case RF_PAYLOAD_ALL_ZERO:
            OAL_MEMZERO(puc_data, ul_size);
            break;
        case RF_PAYLOAD_ALL_ONE:
            oal_memset(puc_data, 0xFF, ul_size);
            break;
        case RF_PAYLOAD_RAND:
            puc_data[0] = oal_gen_random(18, 1);
            for (ul_loop = 1; ul_loop < ul_size; ul_loop++)
            {
                puc_data[ul_loop] = oal_gen_random(18, 0);
            }
            break;
        default:
            break;
    }

    pst_buf->next = OAL_PTR_NULL;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    pst_buf->prev = OAL_PTR_NULL;
#endif
    OAL_MEMZERO(oal_netbuf_cb(pst_buf), OAL_TX_CB_LEN);

    return pst_buf;
}

oal_uint32  dmac_config_set_always_tx_num(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_vap->pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_tx::pst_dmac_vap or pst_dmac_vap->pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap->pst_hal_device->ul_al_tx_thr = *(oal_uint32 *)puc_param;
    pst_dmac_vap->pst_hal_device->ul_al_tx_num = 0;

    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_tx::al_tx_tr==%u.}", pst_dmac_vap->pst_hal_device->ul_al_tx_thr);
    /* 使能发送完成中断 */
    hal_enable_tx_comp(pst_dmac_vap->pst_hal_device);

    return OAL_SUCC;
}



oal_uint32  dmac_config_set_always_tx_hw_cfg(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_al_tx_hw_cfg_stru               *pst_al_tx_hw_cfg;
    hal_to_dmac_device_stru                 *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_always_tx_hw_cfg::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_al_tx_hw_cfg = (mac_cfg_al_tx_hw_cfg_stru *)puc_param;

    hal_al_tx_hw_cfg(pst_hal_device, pst_al_tx_hw_cfg->ul_mode, pst_al_tx_hw_cfg->ul_rate);

    return OAL_SUCC;
}

#endif /* #ifdef _PRE_WLAN_FEATURE_ALWAYS_TX */

#ifdef _PRE_WLAN_FEATURE_11K

oal_uint32  dmac_config_bcn_table_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint8                        uc_switch;

    uc_switch = puc_param[0];

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_bcn_table_switch::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_dmac_vap->bit_bcn_table_switch = uc_switch;
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_bcn_table_switch:para val[%d]!}", uc_switch);
    return OAL_SUCC;
}
#endif //_PRE_WLAN_FEATURE_11K

oal_uint32  dmac_config_reg_write(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int8             *pc_token;
    oal_int8             *pc_end;
    oal_int8             *pc_ctx;
    oal_int8             *pc_sep = " ";
    oal_bool_enum_uint8   en_reg_info32 = OAL_TRUE;
    dmac_reg_write_stru   st_dmac_reg_write = {0};

    /* 选择写操作单位(32/16) */
    pc_token = oal_strtok((oal_int8 *)puc_param, pc_sep, &pc_ctx);
    if (NULL == pc_token)
    {
        return OAL_FAIL;
    }

    /*lint -e960*/
    /* 兼容原有" regtype(soc/mac/phy) addr val"命令格式，默认32位读取寄存器 */
    if ((0 != oal_strcmp(pc_token, "soc")) && (0 != oal_strcmp(pc_token, "mac")) && (0 != oal_strcmp(pc_token, "phy")) )
    {
        if (0 == oal_strcmp(pc_token, "16"))
        {
            en_reg_info32 = OAL_FALSE;
        }

        /* 获取要读取的寄存器类型 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (NULL == pc_token)
        {
            return OAL_FAIL;
        }
    }
    /*lint +e960*/
    st_dmac_reg_write.pc_reg_type = pc_token;

    /* 获取寄存器地址 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (NULL == pc_token)
    {
        return OAL_FAIL;
    }

    st_dmac_reg_write.ul_addr = (oal_uint32)oal_strtol(pc_token, &pc_end, 16);

    /* 获取需要写入的值 */
    pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
    if (NULL == pc_token)
    {
        return OAL_FAIL;
    }

    if (OAL_TRUE == en_reg_info32)
    {
        st_dmac_reg_write.ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 16);
        /*lint -e960*/
        dmac_config_reg_write_test(pst_mac_vap, &st_dmac_reg_write);
        /*lint +e960*/
        return OAL_SUCC;
    }

    st_dmac_reg_write.ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 16);

    g_st_dmac_config_rom_cb.p_dmac_config_reg_write_test16(pst_mac_vap, &st_dmac_reg_write);

    return OAL_SUCC;
}

oal_uint32  dmac_config_vap_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru               *pst_dmac_vap   = OAL_PTR_NULL;
    hal_to_dmac_device_stru     *pst_hal_device = OAL_PTR_NULL;
    mac_device_stru             *pst_mac_device = OAL_PTR_NULL;
    mac_user_stru               *pst_mac_user   = OAL_PTR_NULL;

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    oal_uint8                    uc_hipriv_ack = OAL_FALSE;
#endif

    pst_dmac_vap    = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_info::pst_dmac_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_info:: pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_info:: pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_EQUIPMENT_TEST
    /* 51产测需求 */
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    OAL_IO_PRINT("al_tx:  %d\t al_rx: %d\n"
                 "txch:   %d\t rxch:  %d\n"
                 "rate/mcs/mcsac: %d\n"
                "\n******************************************************\n\n",
                pst_hal_device->bit_al_tx_flag, pst_hal_device->bit_al_rx_flag,
                pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.bit_tx_chain_selection,
                pst_hal_device->st_cfg_cap_info.uc_phy_chain,
                DMAC_GET_VAP_RATE((pst_dmac_vap->uc_protocol_rate_dscr >> 6) & 0x3,
                pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0].rate_bit_stru.un_nss_rate));
#endif
    /* 命令成功返回Success */
    uc_hipriv_ack = OAL_TRUE;
    dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_CHIP_CHECK_SWITCH, OAL_SIZEOF(oal_uint8), &uc_hipriv_ack);
#endif

    if ((WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)&&
        (OAL_PTR_NULL != (pst_mac_user = mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id))))
    {
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "vap bandwidth=%d, user bandwidth_cap=%d,avail bandwidth=%d, cur bandwidth=%d.",
                       pst_mac_vap->st_channel.en_bandwidth,
                       pst_mac_user->en_bandwidth_cap, pst_mac_user->en_avail_bandwidth, pst_mac_user->en_cur_bandwidth);
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "user id=%d,user nss_num=%d,user avail nss=%d,user avail bf nss=%d.",
                       pst_mac_vap->us_assoc_vap_id, pst_mac_user->en_user_num_spatial_stream,
                       pst_mac_user->en_avail_num_spatial_stream, pst_mac_user->en_avail_bf_num_spatial_stream);
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "mac_vap_state=%d, mac vap nss_num=%d, mac_device nss=%d, hal device nss=%d.",
                       pst_mac_vap->en_vap_state, pst_mac_vap->en_vap_rx_nss, MAC_DEVICE_GET_NSS_NUM(pst_mac_device), pst_hal_device->st_cfg_cap_info.en_nss_num);
    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "device phy_chain=%d, device single_tx_chain=%d, device rf_chain=%d.",
                       pst_hal_device->st_cfg_cap_info.uc_phy_chain, pst_hal_device->st_cfg_cap_info.uc_single_tx_chain, pst_hal_device->st_cfg_cap_info.uc_rf_chain);
    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_vap_info::current mac vap id[%d]in device[%d]state[%d]device workbitmap[%d]}",
                             pst_mac_vap->uc_vap_id, pst_dmac_vap->pst_hal_device->uc_device_id,
                             GET_HAL_DEVICE_STATE(pst_dmac_vap->pst_hal_device),pst_dmac_vap->pst_hal_device->ul_work_vap_bitmap);

    //print protection info
    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "en_protection_mode=%d. uc_obss_non_erp_aging_cnt=%d. uc_obss_non_ht_aging_cnt=%d. bit_auto_protection=%d",
                                                                                                                pst_mac_vap->st_protection.en_protection_mode,
                                                                                                                pst_mac_vap->st_protection.uc_obss_non_erp_aging_cnt,
                                                                                                                pst_mac_vap->st_protection.uc_obss_non_ht_aging_cnt,
                                                                                                                pst_mac_vap->st_protection.bit_auto_protection);

    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "bit_obss_non_erp_present=%d. bit_obss_non_ht_present=%d. bit_rts_cts_protect_mode=%d. bit_lsig_txop_protect_mode=%d.",
                                                                                                                pst_mac_vap->st_protection.bit_obss_non_erp_present,
                                                                                                                pst_mac_vap->st_protection.bit_obss_non_ht_present,
                                                                                                                pst_mac_vap->st_protection.bit_rts_cts_protect_mode,
                                                                                                                pst_mac_vap->st_protection.bit_lsig_txop_protect_mode);

    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_sta_no_short_slot_num=%d. uc_sta_no_short_preamble_num=%d. uc_sta_non_erp_num=%d. uc_sta_non_ht_num=%d.",
                                                                                                                pst_mac_vap->st_protection.uc_sta_no_short_slot_num,
                                                                                                                pst_mac_vap->st_protection.uc_sta_no_short_preamble_num,
                                                                                                                pst_mac_vap->st_protection.uc_sta_non_erp_num,
                                                                                                                pst_mac_vap->st_protection.uc_sta_non_ht_num);

    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "uc_sta_non_gf_num=%d. uc_sta_20M_only_num=%d. uc_sta_no_40dsss_cck_num=%d. uc_sta_no_lsig_txop_num=%d. ",
                                                                                                                pst_mac_vap->st_protection.uc_sta_non_gf_num,
                                                                                                                pst_mac_vap->st_protection.uc_sta_20M_only_num,
                                                                                                                pst_mac_vap->st_protection.uc_sta_no_40dsss_cck_num,
                                                                                                                pst_mac_vap->st_protection.uc_sta_no_lsig_txop_num);

    return OAL_SUCC;
}


oal_void  dmac_config_set_machw_wmm(hal_to_dmac_vap_stru *pst_hal_vap, mac_vap_stru *pst_mac_vap)
{
    wlan_wme_ac_type_enum_uint8     en_ac_type;
    /* 设置AIFSN */
    hal_vap_set_machw_aifsn_all_ac(pst_hal_vap,
                                   (oal_uint8)mac_mib_get_QAPEDCATableAIFSN(pst_mac_vap, WLAN_WME_AC_BK),
                                   (oal_uint8)mac_mib_get_QAPEDCATableAIFSN(pst_mac_vap, WLAN_WME_AC_BE),
                                   (oal_uint8)mac_mib_get_QAPEDCATableAIFSN(pst_mac_vap, WLAN_WME_AC_VI),
                                   (oal_uint8)mac_mib_get_QAPEDCATableAIFSN(pst_mac_vap, WLAN_WME_AC_VO));

    /* cwmin cwmax */
    for (en_ac_type = 0; en_ac_type < WLAN_WME_AC_BUTT; en_ac_type++)
    {
        hal_vap_set_edca_machw_cw(pst_hal_vap,
                                 (oal_uint8)mac_mib_get_QAPEDCATableCWmax(pst_mac_vap, en_ac_type),
                                 (oal_uint8)mac_mib_get_QAPEDCATableCWmin(pst_mac_vap, en_ac_type),
                                  en_ac_type);
    }

#if 0
    hal_vap_set_machw_cw_bk(pst_hal_vap,
                            (oal_uint8)pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[WLAN_WME_AC_BK].ul_dot11QAPEDCATableCWmax,
                            (oal_uint8)pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[WLAN_WME_AC_BK].ul_dot11QAPEDCATableCWmin);
    hal_vap_set_machw_cw_be(pst_hal_vap,
                            (oal_uint8)pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[WLAN_WME_AC_BE].ul_dot11QAPEDCATableCWmax,
                            (oal_uint8)pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[WLAN_WME_AC_BE].ul_dot11QAPEDCATableCWmin);
    hal_vap_set_machw_cw_vi(pst_hal_vap,
                            (oal_uint8)pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[WLAN_WME_AC_VI].ul_dot11QAPEDCATableCWmax,
                            (oal_uint8)pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[WLAN_WME_AC_VI].ul_dot11QAPEDCATableCWmin);
    hal_vap_set_machw_cw_vo(pst_hal_vap,
                            (oal_uint8)pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[WLAN_WME_AC_VO].ul_dot11QAPEDCATableCWmax,
                            (oal_uint8)pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[WLAN_WME_AC_VO].ul_dot11QAPEDCATableCWmin);
#endif
    /* txop */
    hal_vap_set_machw_txop_limit_bkbe(pst_hal_vap,
                                      (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(pst_mac_vap, WLAN_WME_AC_BE),
                                      (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(pst_mac_vap, WLAN_WME_AC_BK));
    hal_vap_set_machw_txop_limit_vivo(pst_hal_vap,
                                      (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(pst_mac_vap, WLAN_WME_AC_VO),
                                      (oal_uint16)mac_mib_get_QAPEDCATableTXOPLimit(pst_mac_vap, WLAN_WME_AC_VI));


}

oal_void  dmac_config_set_wmm_open_cfg(hal_to_dmac_vap_stru *pst_hal_vap, mac_wme_param_stru  *pst_wmm)
{
    wlan_wme_ac_type_enum_uint8     en_ac_type;

    if(OAL_PTR_NULL == pst_hal_vap || OAL_PTR_NULL == pst_wmm)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_wmm_open_cfg::param null.}");

        return;
    }
    for (en_ac_type = 0; en_ac_type < WLAN_WME_AC_BUTT; en_ac_type++)
    {
        hal_vap_set_edca_machw_cw(pst_hal_vap,
                                 (oal_uint8)(pst_wmm[en_ac_type].ul_logcwmax),
                                 (oal_uint8)(pst_wmm[en_ac_type].ul_logcwmin),
                                  en_ac_type);

        hal_vap_set_machw_aifsn_ac(pst_hal_vap, en_ac_type, (oal_uint8)pst_wmm[en_ac_type].ul_aifsn);
    }

#if 0
    hal_vap_set_machw_cw_bk(pst_hal_vap, (oal_uint8)(pst_wmm[WLAN_WME_AC_BK].ul_logcwmax), (oal_uint8)(pst_wmm[WLAN_WME_AC_BK].ul_logcwmin));
    hal_vap_set_machw_cw_be(pst_hal_vap, (oal_uint8)pst_wmm[WLAN_WME_AC_BE].ul_logcwmax, (oal_uint8)pst_wmm[WLAN_WME_AC_BE].ul_logcwmin);
    hal_vap_set_machw_cw_vi(pst_hal_vap, (oal_uint8)pst_wmm[WLAN_WME_AC_VI].ul_logcwmax, (oal_uint8)pst_wmm[WLAN_WME_AC_VI].ul_logcwmin);
    hal_vap_set_machw_cw_vo(pst_hal_vap, (oal_uint8)pst_wmm[WLAN_WME_AC_VO].ul_logcwmax, (oal_uint8)pst_wmm[WLAN_WME_AC_VO].ul_logcwmin);

    hal_vap_set_machw_aifsn_ac(pst_hal_vap, WLAN_WME_AC_BK, (oal_uint8)pst_wmm[WLAN_WME_AC_BK].ul_aifsn);
    hal_vap_set_machw_aifsn_ac(pst_hal_vap, WLAN_WME_AC_BE, (oal_uint8)pst_wmm[WLAN_WME_AC_BE].ul_aifsn);
    hal_vap_set_machw_aifsn_ac(pst_hal_vap, WLAN_WME_AC_VI, (oal_uint8)pst_wmm[WLAN_WME_AC_VI].ul_aifsn);
    hal_vap_set_machw_aifsn_ac(pst_hal_vap, WLAN_WME_AC_VO, (oal_uint8)pst_wmm[WLAN_WME_AC_VO].ul_aifsn);
#endif

    hal_vap_set_machw_txop_limit_bkbe(pst_hal_vap, (oal_uint16)pst_wmm[WLAN_WME_AC_BE].ul_txop_limit, (oal_uint16)pst_wmm[WLAN_WME_AC_BK].ul_txop_limit);
    hal_vap_set_machw_txop_limit_vivo(pst_hal_vap, (oal_uint16)pst_wmm[WLAN_WME_AC_VO].ul_txop_limit, (oal_uint16)pst_wmm[WLAN_WME_AC_VI].ul_txop_limit);

}


oal_void  dmac_config_set_wmm_close_cfg(hal_to_dmac_vap_stru *pst_hal_vap, mac_wme_param_stru  *pst_wmm)
{
    if(OAL_PTR_NULL == pst_hal_vap || OAL_PTR_NULL == pst_wmm)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_wmm_close_cfg::param null.}");

        return;
    }
   // hal_vap_set_machw_cw_vo(pst_hal_vap, (oal_uint8)pst_wmm[WLAN_WME_AC_BE].ul_logcwmax, (oal_uint8)pst_wmm[WLAN_WME_AC_BE].ul_logcwmin);
    hal_vap_set_edca_machw_cw(pst_hal_vap, (oal_uint8)pst_wmm[WLAN_WME_AC_BE].ul_logcwmax, (oal_uint8)pst_wmm[WLAN_WME_AC_BE].ul_logcwmin, WLAN_WME_AC_VO);
    hal_vap_set_machw_aifsn_ac(pst_hal_vap, WLAN_WME_AC_VO, (oal_uint8)pst_wmm[WLAN_WME_AC_BE].ul_aifsn);
    hal_vap_set_machw_txop_limit_vivo(pst_hal_vap, (oal_uint16)pst_wmm[WLAN_WME_AC_BE].ul_txop_limit, (oal_uint16)pst_wmm[WLAN_WME_AC_VI].ul_txop_limit);
}

oal_uint32   dmac_config_rx_fcs_info(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    hal_to_dmac_device_stru        *pst_hal_device;
//#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    dmac_atcmdsrv_atcmd_response_event   st_atcmdsrv_rx_pkcg;
//#endif
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_rx_fcs_info::hal dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    OAL_IO_PRINT("dmac_config_rx_fcs_info:packets info, succ[%u], fail[%u], rssi[%d]\r\n", pst_hal_device->ul_rx_normal_mdpu_succ_num, pst_hal_device->ul_rx_ppdu_fail_num,
                    oal_get_real_rssi(pst_hal_device->s_always_rx_rssi));
#endif
    OAM_WARNING_LOG2(0, OAM_SF_CFG, "dmac_config_rx_fcs_info:packets info, succ[%d], fail[%d]\n", pst_hal_device->ul_rx_normal_mdpu_succ_num, pst_hal_device->ul_rx_ppdu_fail_num);

    /*装备测试需要将接收正确的包数上报到host侧*/
//#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        st_atcmdsrv_rx_pkcg.uc_event_id = OAL_ATCMDSRV_GET_RX_PKCG;
        st_atcmdsrv_rx_pkcg.ul_event_para = pst_hal_device->ul_rx_normal_mdpu_succ_num;
        st_atcmdsrv_rx_pkcg.ul_fail_num = pst_hal_device->ul_rx_ppdu_fail_num;
        st_atcmdsrv_rx_pkcg.s_always_rx_rssi = oal_get_real_rssi(pst_hal_device->s_always_rx_rssi);
        dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_RX_FCS_INFO, OAL_SIZEOF(dmac_atcmdsrv_atcmd_response_event), (oal_uint8 *)&st_atcmdsrv_rx_pkcg);
//#endif

    /* 读后清零 */
    pst_hal_device->ul_rx_normal_mdpu_succ_num = 0;
    pst_hal_device->ul_rx_ppdu_fail_num = 0;


    return OAL_SUCC;
}

oal_uint32 dmac_config_set_app_ie_to_vap(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_app_ie_stru         *pst_app_ie;

    if ((OAL_PTR_NULL == puc_param) || (OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_config_set_app_ie_to_vap::param is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_app_ie = (oal_app_ie_stru *)puc_param;

    return mac_vap_save_app_ie(pst_mac_vap, pst_app_ie, pst_app_ie->en_app_ie_type);
}
#ifdef _PRE_WLAN_FEATURE_STA_PM

oal_uint32  dmac_config_set_sta_ps_mode(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru                   *pst_dmac_vap;
    mac_cfg_ps_mode_param_stru      *pst_ps_mode_param;

    pst_ps_mode_param = (mac_cfg_ps_mode_param_stru *)puc_param;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_set_sta_ps_mode::mac_res_get_dmac_vap fail,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }

    if (pst_ps_mode_param->uc_vap_ps_mode < NUM_PS_MODE)
    {
        pst_dmac_vap->uc_cfg_pm_mode = pst_ps_mode_param->uc_vap_ps_mode;
    }
    else
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR,"dmac set pm mode[%d]> max mode fail",pst_ps_mode_param->uc_vap_ps_mode);
    }

    return OAL_SUCC;

}
 
oal_uint32 dmac_set_psm_param(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_ps_param_stru           *pst_ps_param;
    dmac_vap_stru                   *pst_dmac_vap;
    oal_uint16                       us_beacon_timeout;
    oal_uint16                       us_tbtt_offset;
    oal_uint16                       us_ext_tbtt_offset;
    oal_uint16                       us_dtim3_on;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_set_psm_param::mac_res_get_dmac_vap fail,vap_id:%u.}",
                         pst_mac_vap->uc_vap_id);
        return OAL_FAIL;
    }

    pst_ps_param = (mac_cfg_ps_param_stru *)puc_param;
    us_beacon_timeout = pst_ps_param->us_beacon_timeout;
    us_tbtt_offset = pst_ps_param->us_tbtt_offset;
    us_ext_tbtt_offset = pst_ps_param->us_ext_tbtt_offset;
    us_dtim3_on        = pst_ps_param->us_dtim3_on;

    /* beacon timeout value */
    if (us_beacon_timeout != 0)
    {
        //hal_set_psm_listen_interval(pst_dmac_vap->pst_hal_vap, us_listen_interval);
        //hal_set_psm_listen_interval_count(pst_dmac_vap->pst_hal_vap, us_listen_interval);
        pst_dmac_vap->us_beacon_timeout = us_beacon_timeout;
        hal_set_beacon_timeout_val(pst_dmac_vap->pst_hal_device, us_beacon_timeout);
    }

    /* INTER TBTT OFFSET */
    if (us_tbtt_offset != 0)
    {
        pst_dmac_vap->us_in_tbtt_offset = us_tbtt_offset;
        hal_set_psm_tbtt_offset(pst_dmac_vap->pst_hal_vap, us_tbtt_offset);
    }

    /* EXT TBTT OFFSET*/
    if (us_ext_tbtt_offset != 0)
    {
        pst_dmac_vap->us_ext_tbtt_offset = us_ext_tbtt_offset;
        hal_set_psm_ext_tbtt_offset(pst_dmac_vap->pst_hal_vap, us_ext_tbtt_offset);
    }

    g_uc_max_powersave = (0==us_dtim3_on)?0:1;

    g_st_dmac_psm_sta_rom_cb.p_dmac_psm_update_dtime_period(pst_mac_vap,
                                    (oal_uint8)mac_mib_get_dot11dtimperiod(pst_mac_vap),
                                    mac_mib_get_BeaconPeriod(pst_mac_vap));

    g_st_dmac_psm_sta_rom_cb.p_dmac_psm_update_keepalive(pst_dmac_vap);

    return OAL_SUCC;
}

 
oal_uint32 dmac_psm_check_module_ctrl(mac_vap_stru *pst_mac_vap, mac_pm_ctrl_type_enum_uint8 pm_ctrl_type, mac_pm_switch_enum_uint8 pm_enable, oal_uint8 *puc_psm_result)
{
    oal_uint32      ul_sta_pm_close_status;
    dmac_vap_stru  *pst_dmac_vap            = OAL_PTR_NULL;

    if(OAL_PTR_NULL == pst_mac_vap ||
       OAL_PTR_NULL == puc_psm_result)
    {
        OAM_ERROR_LOG2(0, OAM_SF_PWR,"{dmac_psm_check_module_ctrl::Param NULL, pst_mac_vap = 0x%X, puc_psm_result = 0x%X.}",
                         pst_mac_vap,puc_psm_result);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap    = mac_res_get_dmac_vap(pst_mac_vap->uc_vap_id);

    if(OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_PWR,"{dmac_psm_check_module_ctrl::pst_dmac_vap is NULL.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR,"{dmac_psm_check_module_ctrl::module[%d],open/close[%d],last open_by_host[%d],close status[%d].}",pm_ctrl_type, pm_enable, pst_dmac_vap->uc_sta_pm_open_by_host,pst_dmac_vap->uc_sta_pm_close_status);

    ul_sta_pm_close_status = pst_dmac_vap->uc_sta_pm_close_status;

    if(MAC_STA_PM_SWITCH_ON == pm_enable)
    {
        //清空 pm_close 对应模块的 bit 位
        ul_sta_pm_close_status &= (~(oal_uint32)(1U<<(oal_uint32)pm_ctrl_type));
        if(MAC_STA_PM_CTRL_TYPE_HOST == pm_ctrl_type)
        {
            pst_dmac_vap->uc_sta_pm_open_by_host = MAC_STA_PM_SWITCH_ON;
        }
    }
    else if(MAC_STA_PM_SWITCH_OFF == pm_enable)
    {
        //置 pm_close 对应模块的 bit 位
        ul_sta_pm_close_status |= (1U<<(oal_uint32)pm_ctrl_type);
        if(MAC_STA_PM_CTRL_TYPE_HOST == pm_ctrl_type)
        {
            pst_dmac_vap->uc_sta_pm_open_by_host = MAC_STA_PM_SWITCH_OFF;
        }
    }
    else
    {
        //外部参数检查
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR,"{dmac_psm_check_module_ctrl::pm_enable = %d ERROR!!}",pm_enable);
    }

    pst_dmac_vap->uc_sta_pm_close_status = (oal_uint8)ul_sta_pm_close_status;

    /* 如果有模块关闭低功耗，则关闭低功耗 */
    if(0 != pst_dmac_vap->uc_sta_pm_close_status)
    {
        *puc_psm_result = MAC_STA_PM_SWITCH_OFF;
    }
    else
    {
        /* HOST 侧是否已经打开低功耗*/
        *puc_psm_result = pst_dmac_vap->uc_sta_pm_open_by_host;
    }

    return OAL_SUCC;
}
#endif


oal_uint32 dmac_config_nss(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_user_nss_stru  *pst_user_nss;
    mac_user_stru      *pst_mac_user;

    pst_user_nss = (mac_user_nss_stru *)puc_param;
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_user_nss->us_user_idx);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "dmac_config_spatial_stream::mac user[%d] is null!",
            pst_user_nss->us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_set_num_spatial_stream(pst_mac_user, pst_user_nss->en_user_num_spatial_stream);
    mac_user_set_avail_num_spatial_stream(pst_mac_user, pst_user_nss->en_avail_num_spatial_stream);
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD

oal_uint32 dmac_config_ip_add(dmac_vap_stru *pst_dmac_vap, dmac_ip_addr_config_stru *pst_ip_addr_info)
{
    mac_vap_stru              *pst_mac_vap       = &pst_dmac_vap->st_vap_base_info;
    oal_uint32                 ul_loop;
    oal_bool_enum_uint8        en_comp           = OAL_FALSE;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap->pst_ip_addr_info))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_ip_add::IP record array memory is not alloced.}");
        return OAL_FAIL;
    }

    if (DMAC_CONFIG_IPV4 == pst_ip_addr_info->en_type)
    {
        for (ul_loop = 0; ul_loop < DMAC_MAX_IPV4_ENTRIES; ul_loop++)
        {
            if ((OAL_FALSE == en_comp) && (0 == (pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_local_ip.ul_value)))
            {
                en_comp = OAL_TRUE; /* 增加完成 */
                oal_memcopy(pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_local_ip.auc_value, pst_ip_addr_info->auc_ip_addr, OAL_IPV4_ADDR_SIZE);
                oal_memcopy(pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_mask.auc_value, pst_ip_addr_info->auc_mask_addr, OAL_IPV4_ADDR_SIZE);
            }
            else if (0 == oal_memcmp(pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_local_ip.auc_value, pst_ip_addr_info->auc_ip_addr, OAL_IPV4_ADDR_SIZE))
            {
                if (OAL_TRUE == en_comp)
                {
                    oal_memset(pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_local_ip.auc_value, 0, OAL_IPV4_ADDR_SIZE);
                    oal_memset(pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_mask.auc_value, 0, OAL_IPV4_ADDR_SIZE);
                }
                else
                {
                    en_comp = OAL_TRUE;
                }
            }
        }
    }
    else if (DMAC_CONFIG_IPV6 == pst_ip_addr_info->en_type)
    {
        for (ul_loop = 0; ul_loop < DMAC_MAX_IPV6_ENTRIES; ul_loop++)
        {
            if ((OAL_FALSE == en_comp) && OAL_IPV6_IS_UNSPECIFIED_ADDR(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr))
            {
                en_comp = OAL_TRUE; /* 增加完成 */
                oal_memcopy(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr, pst_ip_addr_info->auc_ip_addr, OAL_IPV6_ADDR_SIZE);
            }
            else if (0 == oal_memcmp(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr, pst_ip_addr_info->auc_ip_addr, OAL_IPV6_ADDR_SIZE))
            {
                if (OAL_TRUE == en_comp)
                {
                    oal_memset(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr, 0, OAL_IPV6_ADDR_SIZE);
                }
                else
                {
                    en_comp = OAL_TRUE;
                }
            }
        }
    }
    else
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_ip_add::IP type[%d] is wrong.}", pst_ip_addr_info->en_type);
        return OAL_FAIL;
    }

    if (OAL_FALSE == en_comp)
    {
        if (DMAC_CONFIG_IPV4 == pst_ip_addr_info->en_type)
        {
            OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_ip_add::Add IPv4 address[%d.X.X.%d] failed, there is no empty array.}",
                           ((oal_uint8 *)&(pst_ip_addr_info->auc_ip_addr))[0],
                           ((oal_uint8 *)&(pst_ip_addr_info->auc_ip_addr))[3]);
        }
        else if (DMAC_CONFIG_IPV6 == pst_ip_addr_info->en_type)
        {
            OAM_ERROR_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_ip_add::Add IPv6 address[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x] failed, there is no empty array.}",
                           OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_ip_addr_info->auc_ip_addr))[0]),
                           OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_ip_addr_info->auc_ip_addr))[1]),
                           OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_ip_addr_info->auc_ip_addr))[6]),
                           OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_ip_addr_info->auc_ip_addr))[7]));
        }
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


oal_uint32 dmac_config_ip_del(dmac_vap_stru *pst_dmac_vap, dmac_ip_addr_config_stru *pst_ip_addr_info)
{
    mac_vap_stru              *pst_mac_vap       = &pst_dmac_vap->st_vap_base_info;
    oal_uint32                 ul_loop;
    oal_bool_enum_uint8        en_comp           = OAL_FALSE;

    if (DMAC_CONFIG_IPV4 == pst_ip_addr_info->en_type)
    {
        for (ul_loop = 0; ul_loop < DMAC_MAX_IPV4_ENTRIES; ul_loop++)
        {
            if ((OAL_FALSE == en_comp) && (0 == oal_memcmp(pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_local_ip.auc_value, pst_ip_addr_info->auc_ip_addr, OAL_IPV4_ADDR_SIZE)))
            {
                en_comp = OAL_TRUE; /* 删除完成 */
                oal_memset(pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_local_ip.auc_value, 0, OAL_IPV4_ADDR_SIZE);
                oal_memset(pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_mask.auc_value, 0, OAL_IPV4_ADDR_SIZE);
                break;
            }
        }
    }
    else if (DMAC_CONFIG_IPV6 == pst_ip_addr_info->en_type)
    {
        for (ul_loop = 0; ul_loop < DMAC_MAX_IPV6_ENTRIES; ul_loop++)
        {
            if ((OAL_FALSE == en_comp) && (0 == oal_memcmp(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr, pst_ip_addr_info->auc_ip_addr, OAL_IPV6_ADDR_SIZE)))
            {
                en_comp = OAL_TRUE; /* 删除完成 */
                oal_memset(pst_dmac_vap->pst_ip_addr_info->ast_ipv6_entry[ul_loop].auc_ip_addr, 0, OAL_IPV6_ADDR_SIZE);
                break;
            }
        }
    }
    else
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_ip_del::IP type[%d] is wrong.}", pst_ip_addr_info->en_type);
        return OAL_FAIL;
    }

    if (OAL_FALSE == en_comp)
    {
        if (DMAC_CONFIG_IPV4 == pst_ip_addr_info->en_type)
        {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_ip_del::Delete IPv4 address[%d.X.X.%d] failed, there is not the IP address.}",
                           ((oal_uint8 *)&(pst_ip_addr_info->auc_ip_addr))[0],
                           ((oal_uint8 *)&(pst_ip_addr_info->auc_ip_addr))[3]);
        }
        else if (DMAC_CONFIG_IPV6 == pst_ip_addr_info->en_type)
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_ip_del::Delete IPv6 address[%04x:%04x:XXXX:XXXX:XXXX:XXXX:%04x:%04x] failed, there is not the IP address.}",
                           OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_ip_addr_info->auc_ip_addr))[0]),
                           OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_ip_addr_info->auc_ip_addr))[1]),
                           OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_ip_addr_info->auc_ip_addr))[6]),
                           OAL_NET2HOST_SHORT(((oal_uint16 *)&(pst_ip_addr_info->auc_ip_addr))[7]));
        }
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


oal_uint32 dmac_config_set_ip_addr(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru             *pst_dmac_vap      = MAC_GET_DMAC_VAP(pst_mac_vap);
    dmac_ip_addr_config_stru  *pst_ip_addr_info  = (dmac_ip_addr_config_stru *)puc_param;

    /* vap已经删除，ip资源已经释放 */
    if (MAC_VAP_STATE_BUTT == pst_mac_vap->en_vap_state)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_set_ip_addr::IP [%d] proc has down.}", pst_ip_addr_info->en_oper);
        return OAL_SUCC;
    }

    switch (pst_ip_addr_info->en_oper)
    {
        case DMAC_IP_ADDR_ADD:
        {
            return dmac_config_ip_add(pst_dmac_vap, pst_ip_addr_info);
        }

        case DMAC_IP_ADDR_DEL:
        {
            return dmac_config_ip_del(pst_dmac_vap, pst_ip_addr_info);
        }

        default:
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_set_ip_addr::IP operation[%d] is wrong.}", pst_ip_addr_info->en_oper);
            break;
        }
    }
    return OAL_FAIL;
}
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM

oal_void dmac_frame_modify_bssid(oal_netbuf_stru *pst_netbuf, oal_uint8 *puc_bssid)
{
    mac_ieee80211_qos_htc_frame_addr4_stru   *pst_mac_hdr;
    mac_tx_ctl_stru                          *pst_tx_ctl;
    oal_uint8                                 uc_is_tods;
    oal_uint8                                 uc_is_from_ds;

    pst_mac_hdr = (mac_ieee80211_qos_htc_frame_addr4_stru *)oal_netbuf_data(pst_netbuf);
    pst_tx_ctl  = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    uc_is_tods    = mac_hdr_get_to_ds((oal_uint8 *)pst_mac_hdr);
    uc_is_from_ds = mac_hdr_get_from_ds((oal_uint8 *)pst_mac_hdr);

    /*************************************************************************/
    /*                  80211 MAC HEADER                                     */
    /* --------------------------------------------------------------------- */
    /* | To   | From |  ADDR1 |  ADDR2 | ADDR3  | ADDR3  | ADDR4  | ADDR4  | */
    /* | DS   |  DS  |        |        | MSDU   | A-MSDU |  MSDU  | A-MSDU | */
    /* ----------------------------------------------------------------------*/
    /* |  0   |   0  |  RA=DA |  TA=SA | BSSID  |  BSSID |   N/A  |   N/A  | */
    /* |  0   |   1  |  RA=DA |TA=BSSID|   SA   |  BSSID |   N/A  |   N/A  | */
    /* |  1   |   0  |RA=BSSID| RA=SA  |   DA   |  BSSID |   N/A  |   N/A  | */
    /* |  1   |   1  |  RA    |   TA   |   DA   |  BSSID |   SA   |  BSSID | */
    /*************************************************************************/

    if ((0 == uc_is_tods) && (0 == uc_is_from_ds))
    {
        oal_set_mac_addr(pst_mac_hdr->auc_address3, puc_bssid);
        return;
    }

    if ((0 == uc_is_tods) && (1 == uc_is_from_ds))
    {
        oal_set_mac_addr(pst_mac_hdr->auc_address2, puc_bssid);
        if (MAC_GET_CB_IS_AMSDU(pst_tx_ctl))
        {
            oal_set_mac_addr(pst_mac_hdr->auc_address3, puc_bssid);
        }
        return;
    }

    if ((1 == uc_is_tods) && (0 == uc_is_from_ds))
    {
        oal_set_mac_addr(pst_mac_hdr->auc_address1, puc_bssid);
        if (MAC_GET_CB_IS_AMSDU(pst_tx_ctl))
        {
            oal_set_mac_addr(pst_mac_hdr->auc_address3, puc_bssid);
        }
        return;
    }

    if ((1 == uc_is_tods) && (1 == uc_is_from_ds))
    {
        if (MAC_GET_CB_IS_AMSDU(pst_tx_ctl))
        {
            oal_set_mac_addr(pst_mac_hdr->auc_address3, puc_bssid);
            oal_set_mac_addr(pst_mac_hdr->auc_address4, puc_bssid);
        }
        return;
    }

    return;
}

oal_uint32  dmac_roam_update_framer(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{
    hal_to_dmac_device_stru                *pst_hal_device;
    dmac_tid_stru                          *pst_tid_queue;
    oal_dlist_head_stru                    *pst_dscr_entry;
    hal_tx_dscr_stru                       *pst_tx_dscr;
    oal_netbuf_stru                        *pst_netbuf;
    oal_uint16                              us_mpdu_num;
    oal_uint8                               uc_tid_idx;

    pst_hal_device = pst_dmac_vap->pst_hal_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ROAM, "{dmac_user_update_framer::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_tid_idx = 0; uc_tid_idx < WLAN_TID_MAX_NUM; uc_tid_idx ++)
    {
        pst_tid_queue = &pst_dmac_user->ast_tx_tid_queue[uc_tid_idx];
#ifdef _PRE_WLAN_FEATURE_TX_DSCR_OPT
        if (OAL_FALSE == oal_dlist_is_empty(&pst_tid_queue->st_retry_q))
        {
            /* 重传队列非空 */
            us_mpdu_num = 0;
            OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, &pst_tid_queue->st_retry_q)
            {
                pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);

                pst_netbuf  = pst_tx_dscr->pst_skb_start_addr;
                dmac_frame_modify_bssid(pst_netbuf, pst_dmac_user->st_user_base_info.auc_user_mac_addr);
                us_mpdu_num++;
            }
            OAM_WARNING_LOG2(0, OAM_SF_ROAM, "{dmac_roam_update_framer:: TID[%d]:%d mpdu is updated in retry_q.}", uc_tid_idx, us_mpdu_num);
        }
        if (OAL_FALSE == oal_netbuf_list_empty(&pst_tid_queue->st_buff_head))
        {
            /* netbuf队列非空 */
            us_mpdu_num = 0;
            OAL_NETBUF_SEARCH_FOR_EACH(pst_netbuf, &pst_tid_queue->st_buff_head)
            {
                dmac_frame_modify_bssid(pst_netbuf, pst_dmac_user->st_user_base_info.auc_user_mac_addr);
                us_mpdu_num++;
            }
            OAM_WARNING_LOG2(0, OAM_SF_ROAM, "{dmac_roam_update_framer:: TID[%d]:%d mpdu is updated in buff_q.}", uc_tid_idx, us_mpdu_num);
        }
#else
        if (OAL_FALSE == oal_dlist_is_empty(&pst_tid_queue->st_hdr))
        {
            us_mpdu_num = 0;
            OAL_DLIST_SEARCH_FOR_EACH(pst_dscr_entry, &pst_tid_queue->st_retry_q)
            {
                pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_dscr_entry, hal_tx_dscr_stru, st_entry);

                pst_netbuf  = pst_tx_dscr->pst_skb_start_addr;
                dmac_frame_modify_bssid(pst_netbuf, pst_dmac_user->st_user_base_info.auc_user_mac_addr);
                us_mpdu_num++;
            }
            OAM_WARNING_LOG2(0, OAM_SF_ROAM, "{dmac_roam_update_framer:: TID[%d]:%d mpdu is updated in hdr_q.}", uc_tid_idx, us_mpdu_num);
        }
#endif /* _PRE_WLAN_FEATURE_TX_DSCR_OPT */

    }

    return OAL_SUCC;
}

oal_uint32  dmac_config_roam_notify_state(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    dmac_vap_stru   *pst_dmac_vap           = OAL_PTR_NULL;
    oal_uint32       ul_ip_addr_obtained    = OAL_FALSE;

    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == puc_param))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_config_roam_notify_state::param null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ip_addr_obtained     = *(oal_uint32 *)puc_param;
    pst_dmac_vap            = MAC_GET_DMAC_VAP(pst_mac_vap);

    pst_dmac_vap->st_roam_trigger.ul_ip_addr_obtained = ul_ip_addr_obtained;
    pst_dmac_vap->st_roam_trigger.ul_ip_obtain_stamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_PWR, "{dmac_config_roam_notify_state:: ul_ip_addr_obtained = %d!!}",ul_ip_addr_obtained);

    return OAL_SUCC;
}

#endif  //_PRE_WLAN_FEATURE_ROAM

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 dmac_config_suspend_action_sync(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_device_stru             *pst_mac_device;
    mac_cfg_suspend_stru        *pst_suspend;

    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == puc_param)
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{dmac_config_suspend_action_sync:: pointer is null,pst_mac_vap[0x%x], puc_param[0x%x] .}", pst_mac_vap, puc_param);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_suspend_action_sync::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_suspend = (mac_cfg_suspend_stru *)puc_param;

    pst_mac_device->uc_in_suspend = pst_suspend->uc_in_suspend; //亮暗屏状态

    pst_mac_device->uc_arpoffload_switch = pst_suspend->uc_arpoffload_switch; //arp 开关

    /*暗屏实施dtim2策略*/
    dmac_psm_max_powersave_enable(pst_mac_device);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_PKT_MEM_OPT
oal_bool_enum_uint8 g_en_pkt_opt_switch = 0;
oal_uint32 dmac_pkt_mem_opt_stat_event_process(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru          *pst_event;
    frw_event_hdr_stru      *pst_event_hdr;
    oal_bool_enum_uint8      en_dscr_opt_state;
    mac_device_stru         *pst_mac_device;
    hal_to_dmac_device_stru *pst_hal_device;
    dmac_vap_stru           *pst_dmac_vap;

    /* 获取事件、事件头以及事件payload结构体 */
    pst_event       = frw_get_event_stru(pst_event_mem);
    pst_event_hdr   = &(pst_event->st_event_hdr);

    en_dscr_opt_state = pst_event->auc_event_data[0];

    pst_mac_device = (mac_device_stru*)mac_res_get_dev(pst_event_hdr->uc_device_id);
    if(OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_ANY, "{dmac_dscr_opt_stat_event_process::mac device is null.}");
        return OAL_FAIL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_ANY, "{dmac_dscr_opt_stat_event_process::pst_dmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if(OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_ANY, "{dmac_dscr_opt_stat_event_process::hal device is null.}");
        return OAL_FAIL;
    }

    if (OAL_TRUE == g_en_pkt_opt_switch)
    {
        if(en_dscr_opt_state)
        {
            pst_hal_device->us_rx_normal_dscr_cnt = g_us_normal_rx_max_buffs_opt;
        }
        else
        {
            pst_hal_device->us_rx_normal_dscr_cnt = g_us_normal_rx_max_buffs;
        }

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_dscr_opt_stat_event_process::rx dscr opt change to %d. }", pst_hal_device->us_rx_normal_dscr_cnt);

    }

    return OAL_SUCC;
}
#endif
#ifdef _PRE_WLAN_FEATURE_DFS

oal_int32  mac_dfs_set_radar_th(mac_device_stru *pst_mac_device,
       hal_to_dmac_device_stru *pst_hal_device, oal_int32 l_th)
{
    if(OAL_SUCC == hal_set_radar_th_reg(pst_hal_device, l_th))
    {
        pst_mac_device->st_dfs.st_dfs_info.l_radar_th = l_th;
        return OAL_SUCC;
    }

    //设置到硬件寄存器
    return OAL_FAIL;
}


oal_int32  mac_dfs_get_radar_th(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_devcie)
{
    oal_int32   l_th;
    hal_get_radar_th_reg(pst_hal_devcie, &l_th);
    pst_mac_device->st_dfs.st_dfs_info.l_radar_th = l_th;
    return l_th;
}


oal_uint32  dmac_config_dfs_radartool(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param)
{
#ifndef _PRE_WLAN_PROFLING_MIPS
    mac_device_stru       *pst_mac_device;
    oal_int8              *pc_token;
    oal_int8              *pc_end;
    oal_int8              *pc_ctx;
    oal_int8              *pc_sep = " ";
    oal_bool_enum_uint8    en_val;
    oal_uint32             ul_val;
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_int32              l_th;
    oal_uint8              uc_dummy_radar_type;
    wlan_channel_bandwidth_enum_uint8   en_width = WLAN_BAND_WIDTH_BUTT;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_dfs_radartool::vap id [%d],DMAC_VAP_GET_HAL_DEVICE null}",pst_mac_vap->uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取命令类型 */
    pc_token = oal_strtok((oal_int8 *)puc_param, pc_sep, &pc_ctx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (0 == oal_strcmp(pc_token, "dfsenable"))
    {
        /* 获取DFS使能开关*/
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        mac_dfs_set_dfs_enable(pst_mac_device, en_val);
    }
    else if (0 == oal_strcmp(pc_token, "cacenable"))
    {
        /* 获取CAC检测使能开关*/
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        mac_dfs_set_cac_enable(pst_mac_device, en_val);
    }
    else if (0 == oal_strcmp(pc_token, "dfsdebug"))
    {
        /* 获取debug level */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 16);

        mac_dfs_set_debug_level(pst_mac_device, (oal_uint8)ul_val);
    }
    else if(0 == oal_strcmp(pc_token, "offchannum"))
    {
        /* 获取OFF-CHAN CAC检测信道*/
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        mac_dfs_set_offchan_number(pst_mac_device, ul_val);
    }
    else if(0 == oal_strcmp(pc_token, "ctsdura"))
    {
        /* 获取cts duration */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_mac_device->st_dfs.st_dfs_info.uc_cts_duration = (oal_uint8)oal_strtol(pc_token, &pc_end, 10);
    }
    else if(0 == oal_strcmp(pc_token, "radarfilter"))
    {
        /* 获取雷达过滤 chirp_enable */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);

        pst_hal_device->st_dfs_radar_filter.en_chirp_enable = (oal_bool_enum_uint8)ul_val;

        /* 获取雷达过滤 chirp_cnt */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);

        pst_hal_device->st_dfs_radar_filter.ul_chirp_cnt_threshold = ul_val;

        /* 获取雷达过滤 chirp threshold */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);

        pst_hal_device->st_dfs_radar_filter.ul_chirp_time_threshold = ul_val;

        /* 获取雷达过滤 threshold */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);

        pst_hal_device->st_dfs_radar_filter.ul_time_threshold = ul_val;

        pst_hal_device->st_dfs_radar_filter.ul_last_burst_timestamp = 0;
        pst_hal_device->st_dfs_radar_filter.ul_last_burst_timestamp_for_chirp = 0;
    }
    else if(0 == oal_strcmp(pc_token, "radarfilter_get"))
    {
        OAM_WARNING_LOG3(0, OAM_SF_DFS, "dmac_config_dfs_radartool::radarfilter chirp_enable=%d,chirp_cnt_th=%d,chirp_time_th=%d.",
            pst_hal_device->st_dfs_radar_filter.en_chirp_enable,
            pst_hal_device->st_dfs_radar_filter.ul_chirp_cnt_threshold,
            pst_hal_device->st_dfs_radar_filter.ul_chirp_time_threshold);

        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::radarfilter other type time_th=%d,chirp_cnt_th=%d,chirp_time_th=%d.",
             pst_hal_device->st_dfs_radar_filter.ul_time_threshold);
    }
    else if(0 == oal_strcmp(pc_token, "enabletimer"))
    {
        /* 获取屏蔽误报定时器时间 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);
        pst_mac_device->us_dfs_timeout = (oal_uint16)ul_val;
        //OAM_INFO_LOG1(0, OAM_SF_DFS, "[DFS]enable timer: %d. ", pst_mac_device->us_dfs_timeout);
    }
    else if(0 == oal_strcmp(pc_token, "offchanenable"))
    {
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        /* 1102 DBAC todo 默认开启DBAC时导致GO不发送beacon帧*/
        hal_enable_tsf_tbtt(MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_vap, OAL_FALSE);
#endif
    }
    else if (0 == oal_strcmp(pc_token, "cac"))
    {
        /* 获取频段 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (0 == oal_strcmp(pc_token, "weather"))
        {
            en_val = OAL_TRUE;
        }
        else if (0 == oal_strcmp(pc_token, "nonweather"))
        {
            en_val = OAL_FALSE;
        }
        else
        {
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        /* 获取CAC检测时间 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);

        mac_dfs_set_cac_time(pst_mac_device, ul_val, en_val);
    }
    else if(0 == oal_strcmp(pc_token, "operntime"))
    {
        /* 获取OFF-CHAN CAC检测工作信道驻留时间 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        mac_dfs_set_opern_chan_time(pst_mac_device, ul_val);
    }
    else if(0 == oal_strcmp(pc_token, "offchantime"))
    {
        /* 获取OFF-CHAN CAC检测OFF信道驻留时间 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        mac_dfs_set_off_chan_time(pst_mac_device, ul_val);
    }
    else if(0 == oal_strcmp(pc_token, "dfstrig"))
    {
        /* 获取检测到radar的类型 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            uc_dummy_radar_type = 1;
        }
        else
        {
            uc_dummy_radar_type = (oal_uint8)oal_strtol(pc_token, &pc_end, 10);
        }

        OAM_WARNING_LOG0(0, OAM_SF_DFS, "dmac_config_dfs_radartool::dfstrig.");
        hal_trig_dummy_radar(pst_hal_device, uc_dummy_radar_type);
    }
    else if(0 == oal_strcmp(pc_token, "set_next_chan"))
    {
        /* 获取下一条信道 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);

        if(mac_is_channel_num_valid(MAC_RC_START_FREQ_5, (oal_uint8)ul_val) != OAL_SUCC)
        {
            OAM_WARNING_LOG0(0, OAM_SF_DFS, "dmac_config_dfs_radartool::next chan should be a valid channel.");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (pc_token)
        {
            en_width = mac_vap_str_to_width(pc_token);
        }

        if(WLAN_BAND_WIDTH_BUTT == en_width)
        {
            OAM_WARNING_LOG0(0, OAM_SF_DFS, "dmac_config_dfs_radartool::next chan width mode invalid or not set.");
        }

        mac_dfs_set_next_radar_ch(pst_mac_device,(oal_uint8)ul_val,en_width);
    }
    else if(0 == oal_strcmp(pc_token, "set_5g_channel_bitmap"))
    {
        /* 获取bitmap */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 16);

        if(0 == ul_val)
        {
            OAM_WARNING_LOG0(0, OAM_SF_DFS, "dmac_config_dfs_radartool::5G channel bitmap should not be zero.");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::set 5G channel bitmap 0x%x.",ul_val);
        mac_dfs_set_ch_bitmap(pst_mac_device,ul_val);
    }
    else if(0 == oal_strcmp(pc_token, "set_radar_th"))
    {
       /* 获取雷达检测门限 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        l_th = (oal_int32)oal_strtol(pc_token, &pc_end, 10);
        if(OAL_FAIL == mac_dfs_set_radar_th(pst_mac_device, pst_hal_device, l_th))
        {
           OAM_WARNING_LOG0(0, OAM_SF_DFS, "dmac_config_dfs_radartool::mac_dfs_set_radar_th failed.");
        }
        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::set_radar_th=%d.",l_th);
    }
    else if(0 == oal_strcmp(pc_token, "get_radar_th"))
    {
        l_th = mac_dfs_get_radar_th(pst_mac_device, pst_hal_device);
        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::get_radar_th=%d.",l_th);

        OAL_IO_PRINT("current radar th = %d\r\n",l_th);
    }
    else if(0 == oal_strcmp(pc_token, "offcactime"))
    {
        /* 获取频段 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        if (0 == oal_strcmp(pc_token, "weather"))
        {
            en_val = OAL_TRUE;
        }
        else if (0 == oal_strcmp(pc_token, "nonweather"))
        {
            en_val = OAL_FALSE;
        }
        else
        {
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        /* 获取off CAC检测时间 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);

        mac_dfs_set_off_cac_time(pst_mac_device, ul_val, en_val);
    }
    else if(0 == oal_strcmp(pc_token, "non_occupancy_period"))
    {
        /* 获取不可占用时间 */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_val = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);

        mac_dfs_set_non_occupancy_period_time(pst_mac_device, ul_val);
        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::non_occupancy_period=%d.", ul_val);
    }
#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE
    else if (0 == oal_strcmp(pc_token, "log_switch"))
    {
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        pst_hal_device->st_dfs_radar_filter.en_log_switch = en_val;

        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::log_switch=%d.", en_val);
    }
    else if (0 == oal_strcmp(pc_token, "read_pulse"))
    {
        hal_radar_pulse_info_stru                   st_pulse_info;
        hal_pulse_info_stru                        *pst_pulse_info;
        oal_uint32                                  ul_loop;
        oal_uint32                                  ul_pri;

        OAM_WARNING_LOG0(0, OAM_SF_DFS, "dmac_config_dfs_radartool::read_pulse");

        OAL_MEMZERO(&st_pulse_info, OAL_SIZEOF(hal_radar_pulse_info_stru));
        hal_radar_get_pulse_info(pst_hal_device, &st_pulse_info);
        hal_radar_clean_pulse_buf(pst_hal_device);

        pst_pulse_info = st_pulse_info.ast_pulse_info;

        if (0 == st_pulse_info.ul_pulse_cnt)
        {
            return OAL_SUCC;
        }

        OAL_IO_PRINT("{duration: %d, timestamp: %u, power: %d, intvl: %u, pulse_type: %d}\r\n",
                        pst_pulse_info[0].us_duration,
                        pst_pulse_info[0].ul_timestamp,
                        pst_pulse_info[0].us_power,
                        0,
                        pst_pulse_info[0].uc_type);
        for (ul_loop = 1; ul_loop < st_pulse_info.ul_pulse_cnt; ul_loop++)
        {
            ul_pri = (pst_pulse_info[ul_loop].ul_timestamp >= pst_pulse_info[ul_loop-1].ul_timestamp)
                            ? (pst_pulse_info[ul_loop].ul_timestamp - pst_pulse_info[ul_loop-1].ul_timestamp)
                            : (pst_pulse_info[ul_loop].ul_timestamp + 0xFFFFF - pst_pulse_info[ul_loop-1].ul_timestamp);
            OAL_IO_PRINT("{duration: %d, timestamp: %u, power: %d, intvl: %u, pulse_type: %d}\r\n",
                         pst_pulse_info[ul_loop].us_duration,
                         pst_pulse_info[ul_loop].ul_timestamp,
                         pst_pulse_info[ul_loop].us_power,
                         ul_pri,
                         pst_pulse_info[ul_loop].uc_type);
        }
    }
    else if(0 == oal_strcmp(pc_token, "pulse_check_filter"))
    {
        /* fcc chirp pulse duration diff */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        pst_hal_device->st_dfs_radar_filter.st_dfs_pulse_check_filter.en_fcc_chirp_duration_diff= en_val;
        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::en_fcc_chirp_duration_diff=%d.", en_val);

        /* fcc chirp pulse pow diff */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        pst_hal_device->st_dfs_radar_filter.st_dfs_pulse_check_filter.en_fcc_chirp_pow_diff= en_val;
        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::en_fcc_chirp_pow_diff=%d.", en_val);

        /* fcc chirp pulse same duration num check*/
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        pst_hal_device->st_dfs_radar_filter.st_dfs_pulse_check_filter.en_fcc_chirp_eq_duration_num= en_val;
        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::en_fcc_chirp_eq_duration_num=%d.", en_val);

        /* fcc type4 duration diff */
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);

        pst_hal_device->st_dfs_radar_filter.st_dfs_pulse_check_filter.en_fcc_type4_duration_diff= en_val;
        OAM_WARNING_LOG1(0, OAM_SF_DFS, "dmac_config_dfs_radartool::en_fcc_type4_duration_diff=%d.", en_val);

    }
#endif
    else
    {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (g_st_dmac_config_rom_cb.p_dmac_config_dfs_radartool)
    {
        (g_st_dmac_config_rom_cb.p_dmac_config_dfs_radartool)(pst_mac_vap, us_len, puc_param);
    }
#endif
    return OAL_SUCC;
}
#endif

oal_uint32 dmac_cali_to_hmac(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;

    pst_event               = frw_get_event_stru(pst_event_mem);
    pst_event_hdr           = &(pst_event->st_event_hdr);

    //OAL_IO_PRINT("dmac_cali_to_hmac start\r\n");
    FRW_EVENT_HDR_MODIFY_PIPELINE_AND_SUBTYPE(pst_event_hdr, DMAC_MISC_SUB_TYPE_CALI_TO_HMAC);
    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);

    return OAL_SUCC;
}

/*lint -e578*//*lint -e19*/
#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
oal_module_symbol(dmac_config_set_qap_cwmin);
oal_module_symbol(dmac_config_set_qap_cwmax);
oal_module_symbol(dmac_config_set_qap_aifsn);
#endif
/*lint +e578*//*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


