


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_power.h"
#include "dmac_alg.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_POWER_C
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/


oal_uint32  dmac_pow_get_user_target_tx_power(mac_user_stru *pst_mac_user,
                                 hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param, oal_int16 *ps_tx_pow)
{
    mac_vap_stru                        *pst_mac_vap           = OAL_PTR_NULL;
    hal_to_dmac_device_stru             *pst_hal_device        = OAL_PTR_NULL;
    hal_user_pow_info_stru              *pst_hal_user_pow_info = OAL_PTR_NULL;

    oal_uint8                            uc_output_rate_index  = 0;
    oal_uint8                            uc_cur_rate_pow_idx   = 0;

    oal_int16                            s_tx_pow;
    oal_uint32                           ul_ret;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user
       ||  OAL_PTR_NULL == pst_tx_dscr_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY,
                       "{dmac_power_get_user_target_tx_power:: ERROR INFO: pst_mac_user=0x%p, pst_tx_dscr_param=0x%p.}",
                       pst_mac_user, pst_tx_dscr_param);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = mac_res_get_mac_vap(pst_mac_user->uc_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
         OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_power_get_user_target_tx_power: mac_res_get_mac_vap fail, pst_mac_vap is NULL!}");
         return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_power_get_user_target_tx_power::pst_hal_device is NULL}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_user_pow_info = DMAC_USER_GET_POW_INFO(pst_mac_user);

    /* 获取发送速率在vap rate_pow code table中的索引号,失败的话是因为获取速率索引失败，直接返回 */
    ul_ret = dmac_alg_get_vap_rate_idx_for_tx_power(pst_mac_user, pst_tx_dscr_param, &uc_output_rate_index ,&uc_cur_rate_pow_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_power_get_user_target_tx_power: alg_autorate_get_vap_rate_index_for_tx_power fail!}");
        return OAL_FAIL;
    }

    /* 获取功率 */
    hal_device_get_tx_pow_from_rate_idx(pst_hal_device, pst_hal_user_pow_info, pst_mac_vap->st_channel.en_band,
                                pst_mac_vap->st_channel.uc_chan_number, uc_cur_rate_pow_idx,
                                &(pst_tx_dscr_param->st_tx_power), &s_tx_pow);

    *ps_tx_pow = s_tx_pow;

    return OAL_SUCC;
}


oal_uint32  dmac_pow_tx_power_process(dmac_vap_stru *pst_dmac_vap, mac_user_stru *pst_user,
                                        mac_tx_ctl_stru *pst_cb, hal_tx_txop_alg_stru *pst_txop_param)
{
#ifndef WIN32
    oal_uint8                           uc_rate_level_idx;
    oal_uint32                          ul_ret = OAL_SUCC;
    oal_uint8                           uc_rate_to_pow_code_idx = 0;
    oal_uint8                           uc_pow_level;
    oal_bool_enum_uint8                 en_ar_enable = OAL_FALSE;
    oal_uint8                           auc_ar_rate_idx[HAL_TX_RATE_MAX_NUM];
    hal_user_pow_info_stru              *pst_user_pow_info = OAL_PTR_NULL;
    hal_to_dmac_device_stru             *pst_hal_device    = OAL_PTR_NULL;
    oal_uint8                           uc_protocol;
    hal_channel_assemble_enum_uint8     en_bw;
    oal_uint32                          aul_pow_code[HAL_TX_RATE_MAX_NUM];
    oal_uint8                           auc_pow_level[HAL_TX_RATE_MAX_NUM] = {HAL_POW_MAX_POW_LEVEL};//四个速率等级均初始化为最大功率档位

    /* 判断入参是否为空 */
    if (OAL_UNLIKELY((NULL == (pst_dmac_vap)) || (NULL == (pst_user)) || (NULL == (pst_cb)) || (NULL == (pst_txop_param))))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_pow_tx_power_process::input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 多播帧、管理帧算法不设置功率 */
    if ((OAL_TRUE == MAC_GET_CB_IS_MCAST(pst_cb)) ||
        (!(MAC_GET_CB_IS_DATA_FRAME(pst_cb))) ||
            (DMAC_USER_ALG_SMARTANT_NULLDATA_PROBE == MAC_GET_CB_IS_PROBE_DATA(pst_cb)))
    {
        return OAL_SUCC;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);

    /* 功率RF寄存器控时不设置功率 */
    if(OAL_TRUE == pst_hal_device->en_pow_rf_reg_ctl_flag)
    {
        return OAL_SUCC;
    }

    pst_user_pow_info = DMAC_USER_GET_POW_INFO(pst_user);
    pst_user_pow_info->pst_txop_param = pst_txop_param;
    if (OAL_PTR_NULL == DMAC_USER_GET_POW_TABLE(pst_user))
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_pow_tx_power_process::user_get_pow_table is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 通知算法，获取Autorate状态 */
    ul_ret = dmac_alg_get_user_rate_idx_for_tx_power(pst_user, &en_ar_enable, auc_ar_rate_idx);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY,
                       "{dmac_pow_tx_power_process::dmac_alg_get_rate_idx_for_tx_power fail ul_ret[%d]!}", ul_ret);
        return ul_ret;
    }

    /* 根据帧类型, 设置第一个功率等级的索引，其他功率等级按照最大档设置 */
    auc_pow_level[0] = pst_hal_device->uc_fix_power_level;

    /*获取发送帧每一个等级速率的功率信息*/
    for (uc_rate_level_idx = 0; uc_rate_level_idx < HAL_TX_RATE_MAX_NUM; uc_rate_level_idx++)
    {
        /*1. 获取某速率等级的速率索引*/
        if (OAL_FALSE == en_ar_enable)
        {
            uc_protocol = pst_txop_param->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode;
            en_bw       = pst_txop_param->st_rate.en_channel_bandwidth;
            hal_device_get_fix_rate_pow_code_idx(uc_protocol, g_auc_bw_idx[en_bw], &(pst_txop_param->ast_per_rate[0]), &uc_rate_to_pow_code_idx, 0xFF);
        }
        else
        {
            uc_rate_to_pow_code_idx = auc_ar_rate_idx[uc_rate_level_idx];
        }

        /*2. 获取该速率的功率等级*/
        uc_pow_level = auc_pow_level[uc_rate_level_idx];

        /*3. 获取某速率不同功率等级的pow code*/
        aul_pow_code[uc_rate_level_idx] = (oal_uint32)HAL_POW_GET_POW_CODE_FROM_TABLE(pst_user_pow_info->pst_rate_pow_table, uc_rate_to_pow_code_idx, uc_pow_level)

    }

    /* 输出,填写发送描述符功率 */
    hal_pow_set_pow_code_idx_in_tx_power(&(pst_txop_param->st_tx_power), aul_pow_code);
#endif

    return OAL_SUCC;

}


oal_uint32 dmac_pow_set_vap_spec_frame_tx_power(dmac_vap_stru *pst_dmac_vap)
{
#ifndef WIN32
    hal_to_dmac_vap_stru                    *pst_hal_vap        = OAL_PTR_NULL;
    hal_to_dmac_device_stru                 *pst_hal_device     = OAL_PTR_NULL;
    hal_rate_pow_code_gain_table_stru       *pst_rate_pow_table = OAL_PTR_NULL;
    oal_uint8                                uc_rate_pow_idx = 0;
    oal_uint8                                uc_data_rate;
    oal_uint8                                uc_band;
    wlan_vap_mode_enum_uint8                 en_vap_mode;
    oal_uint8                                uc_input_pow_level_idx = 0;
    oal_uint8                                uc_pow_level_idx;
    hal_tx_txop_tx_power_stru               *pst_tx_power;
    oal_uint32                               ul_temp_pow_code  = 0;

    pst_hal_vap  = pst_dmac_vap->pst_hal_vap;
    uc_band      = pst_dmac_vap->st_vap_base_info.st_channel.en_band;
    en_vap_mode  = pst_dmac_vap->st_vap_base_info.en_vap_mode;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_pow_set_vap_spec_frame_tx_power:pst_hal_device null!}");
        return OAL_FAIL;
    }

    /* 读取管理帧等级 */
    uc_pow_level_idx = pst_hal_device->uc_mag_mcast_frm_power_level;
    pst_rate_pow_table = DMAC_VAP_GET_POW_TABLE(pst_dmac_vap);
    if(OAL_PTR_NULL == pst_rate_pow_table)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_pow_set_vap_spec_frame_tx_power:: get pst_rate_pow_table is NULL ! }");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* RF LIMIT power需要基于Max Power进行增强，因此初始pow code需要使用max power的POW code */
    if(HAL_POW_RF_LIMIT_POW_LEVEL == uc_pow_level_idx)
    {
        uc_input_pow_level_idx = uc_pow_level_idx;
        uc_pow_level_idx       = HAL_POW_MAX_POW_LEVEL;
    }

    /* 设置Beacon帧发送模式(含POW code), 2G 1M, 5G 6M */
    if (WLAN_VAP_MODE_BSS_AP == en_vap_mode)
    {
        hal_get_bcn_rate(pst_hal_vap, &uc_data_rate);
        hal_pow_get_spec_frame_data_rate_idx(uc_data_rate,&uc_rate_pow_idx);
        ul_temp_pow_code = (oal_uint32)HAL_POW_GET_POW_CODE_FROM_TABLE(pst_rate_pow_table, uc_rate_pow_idx, uc_pow_level_idx);
        hal_set_bcn_phy_tx_mode(pst_hal_vap, ul_temp_pow_code);
    }

    /* 设置单播管理帧功率,2G 1M, 5G 6M */
    uc_data_rate = HAL_POW_GET_LEGACY_RATE(&(pst_dmac_vap->ast_tx_mgmt_ucast[uc_band].ast_per_rate[0]));
    hal_pow_get_spec_frame_data_rate_idx(uc_data_rate, &uc_rate_pow_idx);

    pst_tx_power = (hal_tx_txop_tx_power_stru *)(&(pst_dmac_vap->ast_tx_mgmt_ucast[uc_band].st_tx_power));
    ul_temp_pow_code = (oal_uint32)HAL_POW_GET_POW_CODE_FROM_TABLE(pst_rate_pow_table, uc_rate_pow_idx, uc_pow_level_idx);
    HAL_POW_SET_RF_LIMIT_POW(uc_input_pow_level_idx, ul_temp_pow_code);
    hal_pow_set_pow_code_idx_same_in_tx_power(pst_tx_power, ul_temp_pow_code);

    /* 设置广播/组播管理帧功率,2G 1M, 5G 6M  */
    uc_data_rate = HAL_POW_GET_LEGACY_RATE(&(pst_dmac_vap->ast_tx_mgmt_bmcast[uc_band].ast_per_rate[0]));
    hal_pow_get_spec_frame_data_rate_idx(uc_data_rate, &uc_rate_pow_idx);

    pst_tx_power  = (hal_tx_txop_tx_power_stru *)(&(pst_dmac_vap->ast_tx_mgmt_bmcast[uc_band].st_tx_power));
    ul_temp_pow_code = (oal_uint32)HAL_POW_GET_POW_CODE_FROM_TABLE(pst_rate_pow_table, uc_rate_pow_idx, uc_pow_level_idx);
    HAL_POW_SET_RF_LIMIT_POW(uc_input_pow_level_idx, ul_temp_pow_code);
    hal_pow_set_pow_code_idx_same_in_tx_power(pst_tx_power, ul_temp_pow_code);

    /*注:在ＳＴＡ模式，扫描信道时，仅在初始化信道为2.4G channel1的时候进入本函数，
    会导致后续5G信道扫描时，没有配置对应速率的功率，因此在该场景，同时配置5G功率*/
    if((WLAN_VAP_MODE_BSS_STA == en_vap_mode) && (WLAN_BAND_2G == uc_band))
    {
        /* 设置单播管理帧功率,2G 1M, 5G 6M */
        uc_data_rate = HAL_POW_GET_LEGACY_RATE(&(pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].ast_per_rate[0]));
        hal_pow_get_spec_frame_data_rate_idx(uc_data_rate, &uc_rate_pow_idx);

        pst_tx_power  = (hal_tx_txop_tx_power_stru *)(&(pst_dmac_vap->ast_tx_mgmt_ucast[WLAN_BAND_5G].st_tx_power));
        ul_temp_pow_code = (oal_uint32)HAL_POW_GET_POW_CODE_FROM_TABLE(pst_rate_pow_table, uc_rate_pow_idx, uc_pow_level_idx);
        HAL_POW_SET_RF_LIMIT_POW(uc_input_pow_level_idx, ul_temp_pow_code);
        hal_pow_set_pow_code_idx_same_in_tx_power(pst_tx_power, ul_temp_pow_code);

        /* 设置广播/组播管理帧功率,2G 1M, 5G 6M  */
        uc_data_rate = HAL_POW_GET_LEGACY_RATE(&(pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].ast_per_rate[0]));
        hal_pow_get_spec_frame_data_rate_idx(uc_data_rate, &uc_rate_pow_idx);

        pst_tx_power  = (hal_tx_txop_tx_power_stru *)(&(pst_dmac_vap->ast_tx_mgmt_bmcast[WLAN_BAND_5G].st_tx_power));
        ul_temp_pow_code = (oal_uint32)HAL_POW_GET_POW_CODE_FROM_TABLE(pst_rate_pow_table, uc_rate_pow_idx, uc_pow_level_idx);
        HAL_POW_SET_RF_LIMIT_POW(uc_input_pow_level_idx, ul_temp_pow_code);
        hal_pow_set_pow_code_idx_same_in_tx_power(pst_tx_power, ul_temp_pow_code);
    }

    /* 设置组播数据帧功率 1M, 6M, 24M*/
    uc_data_rate = HAL_POW_GET_LEGACY_RATE(&(pst_dmac_vap->st_tx_data_mcast.ast_per_rate[0]));
    hal_pow_get_spec_frame_data_rate_idx(uc_data_rate, &uc_rate_pow_idx);

    pst_tx_power  = (hal_tx_txop_tx_power_stru *)(&(pst_dmac_vap->st_tx_data_mcast.st_tx_power));
    ul_temp_pow_code = (oal_uint32)HAL_POW_GET_POW_CODE_FROM_TABLE(pst_rate_pow_table, uc_rate_pow_idx, uc_pow_level_idx);
    hal_pow_set_pow_code_idx_same_in_tx_power(pst_tx_power, ul_temp_pow_code);

    /*设置广播数据帧 1M, 6M, 24M*/
    uc_data_rate = HAL_POW_GET_LEGACY_RATE(&(pst_dmac_vap->st_tx_data_bcast.ast_per_rate[0]));
    hal_pow_get_spec_frame_data_rate_idx(uc_data_rate, &uc_rate_pow_idx);

    pst_tx_power  = (hal_tx_txop_tx_power_stru *)(&(pst_dmac_vap->st_tx_data_bcast.st_tx_power));
    ul_temp_pow_code = (oal_uint32)HAL_POW_GET_POW_CODE_FROM_TABLE(pst_rate_pow_table, uc_rate_pow_idx, uc_pow_level_idx);
    hal_pow_set_pow_code_idx_same_in_tx_power(pst_tx_power, ul_temp_pow_code);

    //oal_dump_stack();
#endif
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER

oal_uint32  dmac_pow_change_mgmt_power_process(mac_vap_stru *pst_vap, oal_int8 c_rssi)
{
    hal_to_dmac_device_stru                 *pst_hal_device         = OAL_PTR_NULL;
    hal_vap_pow_info_stru                   *pst_hal_vap_pow_info   = OAL_PTR_NULL;
    oal_uint32                               ul_ret = OAL_SUCC;

    /* 配置用户级别的功率调整 */
    if (0 == pst_vap->us_user_nums)
    {
        return OAL_SUCC;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_PWR, "{dmac_pow_change_mgmt_power_process::pst_hal_device null!}");
        return OAL_FAIL;
    }

    /* 如果TPC算法未使能则不作功率更新 */
    if (OAL_TRUE == pst_hal_device->en_pow_rf_reg_ctl_flag)
    {
        return OAL_SUCC;
    }

    /* 获取vap级别功率信息结构体 */
    pst_hal_vap_pow_info = DMAC_VAP_GET_POW_INFO(pst_vap);

    /* 先恢复成正常的发射功率 */
    pst_hal_device->uc_mag_mcast_frm_power_level = HAL_POW_MAX_POW_LEVEL;
    pst_hal_device->uc_control_frm_power_level   = HAL_POW_MAX_POW_LEVEL;

    /* 判断RSSI是否在合理的上报范围 */
    if ((OAL_RSSI_SIGNAL_MIN < c_rssi) && (OAL_RSSI_SIGNAL_MAX > c_rssi))
    {
        if(WLAN_CLOSE_DISTANCE_RSSI < c_rssi)
        {
            /*设置非数据帧的发送功率*/
            pst_hal_device->uc_mag_mcast_frm_power_level = HAL_POW_MIN_POW_LEVEL;
            pst_hal_device->uc_control_frm_power_level   = HAL_POW_MIN_POW_LEVEL;
        }
        else if(WLAN_FAR_DISTANCE_RSSI > c_rssi)
        {
            /*设置非数据帧的发送功率*/
            pst_hal_device->uc_mag_mcast_frm_power_level = HAL_POW_RF_LIMIT_POW_LEVEL;
            pst_hal_device->uc_control_frm_power_level   = HAL_POW_RF_LIMIT_POW_LEVEL;
        }
    }

    /* 设置控制帧的发射功率 */
    hal_pow_set_band_spec_frame_tx_power(pst_hal_device, pst_hal_device->st_wifi_channel_status.en_band,
                                         pst_hal_device->st_wifi_channel_status.uc_chan_idx, pst_hal_vap_pow_info->pst_rate_pow_table);

    ul_ret = dmac_pow_set_vap_spec_frame_tx_power(MAC_GET_DMAC_VAP(pst_vap));
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_PWR, "{dmac_pow_change_mgmt_power_process::set vap spec frame failed[%d].", ul_ret);
        return ul_ret;
    }

    /* 维测，根据rx mgmt的rssi，确认回复的管理帧的功率 */
    OAM_INFO_LOG2(pst_vap->uc_vap_id, OAM_SF_PWR, "{dmac_pow_change_mgmt_power_process::mgmt rx's RSSI = %d dBm, management power level = %d.}",
                      c_rssi, pst_hal_device->uc_mag_mcast_frm_power_level);

    return OAL_SUCC;
}
#endif


oal_void dmac_pow_set_vap_tx_power(mac_vap_stru *pst_mac_vap, hal_pow_set_type_enum_uint8 uc_type)
{
#ifndef WIN32
    dmac_vap_stru                       *pst_dmac_vap       = OAL_PTR_NULL;
    hal_to_dmac_device_stru             *pst_hal_device     = OAL_PTR_NULL;
    hal_vap_pow_info_stru               *pst_vap_pow_info   = OAL_PTR_NULL;
    mac_regclass_info_stru              *pst_regdom_info;
    oal_uint8                            uc_protocol;
    oal_uint8                            uc_cur_ch_num;
    wlan_channel_band_enum_uint8         en_freq_band;
    wlan_channel_bandwidth_enum_uint8    en_bandwidth;
    oal_uint8                            uc_chan_idx;
    mac_channel_stru                    *pst_channel;                                     /* vap所在的信道 */

    pst_dmac_vap = (dmac_vap_stru*)pst_mac_vap;
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_pow_set_vap_tx_power::pst_dmac_vap null}");
        return;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_pow_set_vap_tx_power::pst_hal_device null}");
        return;
    }

    /* 获取vap级别功率信息结构体 */
    pst_vap_pow_info = DMAC_VAP_GET_POW_INFO(pst_mac_vap);

    if(HAL_POW_SET_TYPE_INIT == uc_type)
    {
        /* 调试开关默认关闭 */
        pst_dmac_vap->st_vap_pow_info.en_debug_flag = OAL_FALSE;
    }

    pst_channel = &pst_mac_vap->st_channel;
    /*获取信道号，带宽，2G/5G*/
    uc_chan_idx   = pst_channel->uc_chan_idx;
    uc_cur_ch_num = pst_channel->uc_chan_number;
    en_bandwidth  = pst_channel->en_bandwidth;
    en_freq_band  = pst_channel->en_band;

#ifndef _PRE_WLAN_FEATURE_TPC_OPT
    if (0 == uc_cur_ch_num)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_pow_set_vap_tx_power::uc_cur_ch_num is [%d].}", uc_cur_ch_num);
        return;
    }
#endif

    /* 功率RF寄存器控时不作功率更新 */
    if (OAL_TRUE == pst_hal_device->en_pow_rf_reg_ctl_flag)
    {
        /* 若是切信道，需要更新寄存器配置 */
        if(HAL_POW_SET_TYPE_REFRESH == uc_type)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_pow_set_vap_tx_power::refresh rf regctl enable set regs!}");

            hal_pow_set_rf_regctl_enable(pst_hal_device, pst_hal_device->en_pow_rf_reg_ctl_flag);
            if (OAL_SWITCH_OFF == pst_hal_device->bit_al_tx_flag)
            {
                hal_rf_regctl_enable_set_regs(pst_hal_device, en_freq_band, uc_cur_ch_num, en_bandwidth);
            }
        }

        return;
    }

    if((HAL_POW_SET_TYPE_INIT == uc_type) || (HAL_POW_SET_TYPE_REFRESH == uc_type))
    {
#ifndef _PRE_WLAN_FEATURE_TPC_OPT
        if(IS_P2P_DEV(&(pst_dmac_vap->st_vap_base_info)))
        {
            hal_device_p2p_adjust_upc(pst_hal_device, uc_cur_ch_num, en_freq_band, en_bandwidth);
            return;
        }
#endif  /* _PRE_WLAN_FEATURE_TPC_OPT */

        /*获取国家码最大功率, 不区分速率*/
        pst_regdom_info = mac_get_channel_num_rc_info(en_freq_band, uc_cur_ch_num);

        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_regdom_info))
        {
#ifndef _PRE_WLAN_FEATURE_TPC_OPT
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_pow_set_vap_tx_power::this channel isnot support by this country.freq_band = %u,cur_ch_num = %u}",
                                en_freq_band, uc_cur_ch_num);
            pst_vap_pow_info->us_reg_pow = 0xFFFF;
#else
            // TODO:    降SAR后面需要单独处理

            /* 取MAX_Power = MIN[MAX_Power@11k, MAX_Power@TEST, MAX_Power@REG] */
            /* 默认取管制域最大发送功率 */
            pst_vap_pow_info->us_reg_pow = OAL_MIN(OAL_MIN(g_uc_sar_pwr_limit, MAC_RC_DEFAULT_MAX_TX_PWR), pst_mac_vap->uc_tx_power) * HAL_POW_PRECISION_SHIFT;

            OAM_WARNING_LOG_ALTER(pst_mac_vap->uc_vap_id, OAM_SF_PWR,
                "{dmac_pow_set_vap_tx_power::chn do not support this country band[%u] ch_num[%u] reg_pow[%d] pst_mac_vap->uc_tx_power[%d] sar_limit[%d]}",
                   5, en_freq_band, uc_cur_ch_num, pst_vap_pow_info->us_reg_pow, pst_mac_vap->uc_tx_power, g_uc_sar_pwr_limit);
#endif
        }
        else
        {
            /* 取SAR标准功率限制值、所处管制类限制功率、管制域实际最大功率三者中较小值 */
            pst_vap_pow_info->us_reg_pow = OAL_MIN(OAL_MIN(g_uc_sar_pwr_limit*HAL_POW_PRECISION_SHIFT,
                       OAL_MIN(pst_regdom_info->us_max_tx_pwr, pst_regdom_info->uc_max_reg_tx_pwr*HAL_POW_PRECISION_SHIFT)),
                       pst_mac_vap->uc_tx_power*HAL_POW_PRECISION_SHIFT);
        }

        /* 初始化pow code表单 */

#ifndef _PRE_WLAN_FEATURE_TPC_OPT
        hal_device_init_vap_pow_code(pst_hal_device, pst_vap_pow_info, uc_cur_ch_num, en_freq_band, en_bandwidth, uc_type);
#else
        hal_device_init_vap_pow_code(pst_hal_device, pst_vap_pow_info, uc_chan_idx, en_freq_band, en_bandwidth, uc_type);
#endif
    }

    /* 常发模式打开按设置功率配置发送功率 */
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    if (OAL_SWITCH_ON == pst_dmac_vap->st_vap_base_info.bit_al_tx_flag)
    {
        uc_protocol = (pst_dmac_vap->uc_protocol_rate_dscr >> 6) & 0x3;
        hal_device_set_pow_al_tx(pst_hal_device, pst_vap_pow_info, uc_protocol, &pst_dmac_vap->st_tx_data_mcast);
        return;
    }
#endif

    OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TPC, "{dmac_pow_set_vap_tx_power::uc_pow_code[0x%x].}", *(oal_uint64 *)&(pst_dmac_vap->st_tx_data_mcast.st_tx_power));

    if(OAL_PTR_NULL == pst_vap_pow_info->pst_rate_pow_table)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_TPC, "{dmac_pow_set_vap_tx_power::pst_rate_pow_table null, uc_type[%d]}",
                       uc_type);
        return;
    }

    if(uc_type != HAL_POW_SET_TYPE_MAG_LVL_CHANGE)
    {
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
        /* 设置resp帧发射功率 */
        hal_pow_set_resp_frame_tx_power(pst_hal_device,
                                        pst_hal_device->st_wifi_channel_status.en_band,
                                        pst_hal_device->st_wifi_channel_status.uc_chan_idx,
                                        pst_vap_pow_info->pst_rate_pow_table);

        /* 设置该band的特殊帧的发射功率 */
        hal_pow_set_band_spec_frame_tx_power(pst_hal_device,
                                             pst_hal_device->st_wifi_channel_status.en_band,
                                             pst_hal_device->st_wifi_channel_status.uc_chan_idx,
                                             pst_vap_pow_info->pst_rate_pow_table);
#else
        /* 设置resp帧发射功率 */
        hal_pow_set_resp_frame_tx_power(pst_hal_device, en_freq_band, uc_cur_ch_num, pst_vap_pow_info->pst_rate_pow_table);

        /* 设置该band的特殊帧的发射功率 */
        hal_pow_set_band_spec_frame_tx_power(pst_hal_device, en_freq_band, uc_cur_ch_num, pst_vap_pow_info->pst_rate_pow_table);
#endif
    }

    if((HAL_POW_SET_TYPE_INIT == uc_type) ||
        ((uc_type != HAL_POW_SET_TYPE_CTL_LVL_CHANGE) && (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)))
    {
        /*如果设备工作在STA模式，则在配置信道的时候设置管理帧的功率*/
        dmac_pow_set_vap_spec_frame_tx_power(pst_dmac_vap);
    }
#endif
}


oal_void dmac_pow_init_user_info(mac_vap_stru *pst_mac_vap, dmac_user_stru *pst_dmac_user)
{
    OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_pow_init_user_info::init user pow info[%p]!}", DMAC_VAP_GET_POW_TABLE(pst_mac_vap));
    pst_dmac_user->st_user_pow_info.pst_rate_pow_table = DMAC_VAP_GET_POW_TABLE(pst_mac_vap);
}
#if 0

oal_void dmac_pow_init_vap_info(dmac_vap_stru *pst_dmac_vap)
{
    hal_vap_pow_info_stru               *pst_vap_pow_info   = OAL_PTR_NULL;

    OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_pow_init_vap_info::init vap pow info!}");

    /* 获取vap级别功率信息结构体 */
    pst_vap_pow_info = DMAC_VAP_GET_POW_INFO(pst_dmac_vap);

    pst_vap_pow_info->pst_rate_pow_table = g_ast_rate_pow_table_2g;
}
#endif






#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

