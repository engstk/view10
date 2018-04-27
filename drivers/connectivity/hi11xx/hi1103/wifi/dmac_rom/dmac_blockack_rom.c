


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_blockack.h"
#include "dmac_main.h"
#include "dmac_rx_data.h"
#include "dmac_tid.h"
#include "dmac_mgmt_bss_comm.h"
#include "dmac_user.h"
#include "oal_net.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_BLOCKACK_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 11n速率对应最大start spacing的mpdu len值的表 */
OAL_CONST oal_uint16 g_aus_ht_20mhz_rate_lut[WLAN_HT_MCS_BUTT] =
{13, 26, 39, 52, 78, 104, 117, 130, 26, 50, 78, 104, 156, 208, 234, 260};

OAL_CONST oal_uint16 g_aus_ht_20mhz_rate_lut_shortgi[WLAN_HT_MCS_BUTT] =
{15, 29, 44, 58, 87, 116, 130, 145, 29, 58, 87, 114, 174, 232, 260, 289};

OAL_CONST oal_uint16 g_aus_ht_40mhz_rate_lut[WLAN_HT_MCS_BUTT] =
{27, 54, 81, 108, 162, 243, 972, 270, 54, 108, 162, 216, 324, 432, 486, 540};

OAL_CONST oal_uint16 g_aus_ht_40mhz_rate_lut_shortgi[WLAN_HT_MCS_BUTT] =
{30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600};

/* 11ac速率对应最大start spacing的mpdu len值的表 */
OAL_CONST oal_uint16 g_aus_vht_20mhz_rate_lut[WLAN_TRIPLE_NSS][WLAN_VHT_MCS_BUTT] =
{
    {13, 26, 39, 52, 78, 104, 117, 130, 156},
    {26, 52, 78, 104, 156, 209, 234, 260, 312},
};

OAL_CONST oal_uint16 g_aus_vht_20mhz_rate_lut_shortgi[WLAN_TRIPLE_NSS][WLAN_VHT_MCS_BUTT] =
{
    {15, 29, 44, 58, 87, 116, 130, 145, 174, 0},
    {29, 58, 87, 116, 174, 232, 260, 289, 347, 0},
};

OAL_CONST oal_uint16 g_aus_vht_40mhz_rate_lut[WLAN_TRIPLE_NSS][WLAN_VHT_MCS_BUTT] =
{
    {27, 54, 81, 108, 162, 216, 243, 270, 324, 360},
    {54, 108, 162, 216, 324, 432, 486, 540, 648, 720},
};

OAL_CONST oal_uint16 g_aus_vht_40mhz_rate_lut_shortgi[WLAN_TRIPLE_NSS][WLAN_VHT_MCS_BUTT] =
{
    {30, 60, 90, 120, 180, 240, 270, 300, 360, 400},
    {60, 120, 180, 240, 360, 480, 540, 600, 720, 800},
};

OAL_CONST oal_uint16 g_aus_vht_80mhz_rate_lut[WLAN_TRIPLE_NSS][WLAN_VHT_MCS_BUTT] =
{
    {59, 117, 176, 234, 351, 468, 527, 585, 702, 780},
    {117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560},
};

OAL_CONST oal_uint16 g_aus_vht_80mhz_rate_lut_shortgi[WLAN_TRIPLE_NSS][WLAN_VHT_MCS_BUTT] =
{
    {65, 130, 195, 260, 390, 520, 585, 650, 780, 867},
    {130, 260, 390, 520, 780, 1040, 1170, 1300, 1560, 1734},
};

dmac_ba_cb g_st_dmac_ba_rom_cb = {OAL_PTR_NULL,
                                  OAL_PTR_NULL,
                                  dmac_ba_send_bar};

/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint32  dmac_ba_check_rx_aggr(mac_vap_stru *pst_vap, mac_ieee80211_frame_stru *pst_frame_hdr)
{
    /* 该vap是否是ht */
    if (OAL_FALSE == mac_mib_get_HighThroughputOptionImplemented(pst_vap))
    {
        //OAM_INFO_LOG0(pst_vap->uc_vap_id, OAM_SF_BA, "{dmac_ba_check_rx_aggr::ht not supported by this vap.}\r\n");
        return OAL_FAIL;
    }

    /* 判断该帧是不是qos帧 */
    if ((WLAN_FC0_SUBTYPE_QOS | WLAN_FC0_TYPE_DATA) != ((oal_uint8 *)pst_frame_hdr)[0])
    {
        //OAM_INFO_LOG0(pst_vap->uc_vap_id, OAM_SF_BA, "{dmac_ba_check_rx_aggr::not qos data.}\r\n");
        return OAL_FAIL;
    }

    /* 判断该帧是不是组播 */
    if (mac_is_grp_addr(pst_frame_hdr->auc_address1))
    {
        //OAM_INFO_LOG0(pst_vap->uc_vap_id, OAM_SF_BA, "{dmac_ba_check_rx_aggr::group data.}\r\n");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


oal_void dmac_nontxop_txba_num_updata(dmac_user_stru *pst_dmac_user, oal_uint8 uc_tid, oal_bool_enum_uint8 en_is_addba)
{
    oal_uint32       ul_max_ampdu_length;
    dmac_tid_stru   *pst_tid = &(pst_dmac_user->ast_tx_tid_queue[uc_tid]);
    mac_device_stru *pst_device = mac_res_get_dev(pst_dmac_user->st_user_base_info.uc_device_id);

    if(OAL_PTR_NULL == pst_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_BA, "{dmac_nontxop_txba_num_updata::pst_device null!}");
        return;
    }

    if ((WLAN_VHT_MODE == pst_dmac_user->st_user_base_info.en_cur_protocol_mode)
      || (WLAN_VHT_ONLY_MODE == pst_dmac_user->st_user_base_info.en_cur_protocol_mode))
    {
        ul_max_ampdu_length = pst_tid->st_ht_tx_hdl.ul_ampdu_max_size_vht;
    }
    else
    {
        ul_max_ampdu_length = pst_tid->st_ht_tx_hdl.us_ampdu_max_size;
    }

    if(ul_max_ampdu_length > 9000 && pst_tid->pst_ba_tx_hdl->uc_ampdu_max_num > 8)
    {
        if(en_is_addba)
        {
            pst_device->uc_tx_ba_num++;
        }
        else
        {
            if (0 != pst_device->uc_tx_ba_num)
            {
                pst_device->uc_tx_ba_num--;
            }
            else
            {
                OAM_ERROR_LOG0(0, OAM_SF_BA, "{dmac_nontxop_txba_num_updata::tx_ba_num make a mistake!}");
            }
        }
    }

    return;
}



oal_uint16  dmac_ba_get_min_len_ht(hal_tx_txop_alg_stru *pst_txop_alg,
                                                                wlan_phy_protocol_enum_uint8    en_protocl_mode,
                                                                hal_channel_assemble_enum_uint8 en_channel_bandwidth,
                                                                oal_bool_enum_uint8 en_short_gi_enable)
{
    oal_uint16                      us_min_mpdu_len = 0;
    wlan_ht_mcs_enum_uint8          en_ht_mcs;


    if (OAL_PTR_NULL != g_st_dmac_ba_rom_cb.min_len_ht_cb)
    {
        return g_st_dmac_ba_rom_cb.min_len_ht_cb(pst_txop_alg,en_protocl_mode,en_channel_bandwidth,en_short_gi_enable);
    }

    en_ht_mcs = pst_txop_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs;

    if (en_ht_mcs >= WLAN_HT_MCS_BUTT  && en_ht_mcs != WLAN_HT_MCS32)
    {
        OAM_ERROR_LOG1(0, OAM_SF_BA, "{dmac_ba_get_min_len_ht::invalid en_ht_mcs[%d].}", en_ht_mcs);
        return us_min_mpdu_len;
    }

    if (WLAN_HT_MCS32 == en_ht_mcs)
    {
        if (OAL_FALSE == en_short_gi_enable)
        {
            us_min_mpdu_len = WLAN_MIN_MPDU_LEN_FOR_MCS32;
        }
        else
        {
            us_min_mpdu_len = WLAN_MIN_MPDU_LEN_FOR_MCS32_SHORTGI;
        }
    }
    else if (WLAN_BAND_ASSEMBLE_20M == en_channel_bandwidth)
    {
        if (OAL_FALSE == en_short_gi_enable)
        {
            us_min_mpdu_len = g_aus_ht_20mhz_rate_lut[en_ht_mcs];
        }
        else
        {
            us_min_mpdu_len = g_aus_ht_20mhz_rate_lut_shortgi[en_ht_mcs];
        }
    }
    else if (WLAN_BAND_ASSEMBLE_40M == en_channel_bandwidth)
    {
        if (OAL_FALSE == en_short_gi_enable)
        {
            us_min_mpdu_len = g_aus_ht_40mhz_rate_lut[en_ht_mcs];
        }
        else
        {
            us_min_mpdu_len = g_aus_ht_40mhz_rate_lut_shortgi[en_ht_mcs];
        }
    }


    return us_min_mpdu_len;
}


oal_uint16  dmac_ba_get_min_len_vht(hal_tx_txop_alg_stru *pst_txop_alg,
                                                                wlan_phy_protocol_enum_uint8    en_protocl_mode,
                                                                hal_channel_assemble_enum_uint8 en_channel_bandwidth,
                                                                oal_bool_enum_uint8 en_short_gi_enable)
{
    wlan_vht_mcs_enum_uint8         en_vht_mcs;
    wlan_nss_enum_uint8             en_nss;
    oal_uint16                      us_min_mpdu_len = 0;

    if (OAL_PTR_NULL != g_st_dmac_ba_rom_cb.min_len_vht_cb)
    {
        return g_st_dmac_ba_rom_cb.min_len_vht_cb(pst_txop_alg,en_protocl_mode,en_channel_bandwidth,en_short_gi_enable);
    }

    en_vht_mcs  = pst_txop_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_vht_mcs;
    en_nss      = pst_txop_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_nss_mode;
    /*OAM_INFO_LOG2(0, OAM_SF_BA, "{dmac_ba_get_min_len_vht::en_vht_mcs=%d en_nss=%d.}", en_vht_mcs, en_nss); */

    if (en_vht_mcs >= WLAN_VHT_MCS_BUTT || en_nss >= WLAN_TRIPLE_NSS)
    {
        OAM_ERROR_LOG2(0, OAM_SF_BA, "{dmac_ba_get_min_len_vht::array over flow, en_vht_mcs=%d en_nss=%d.}", en_vht_mcs, en_nss);

        return us_min_mpdu_len;
    }

    if (WLAN_BAND_ASSEMBLE_20M == en_channel_bandwidth)
    {
        if (OAL_FALSE == en_short_gi_enable)
        {
            us_min_mpdu_len = g_aus_vht_20mhz_rate_lut[en_nss][en_vht_mcs];
        }
        else
        {
            us_min_mpdu_len = g_aus_vht_20mhz_rate_lut_shortgi[en_nss][en_vht_mcs];
        }
    }
    else if (WLAN_BAND_ASSEMBLE_40M == en_channel_bandwidth)
    {
        if (OAL_FALSE == en_short_gi_enable)
        {
            us_min_mpdu_len = g_aus_vht_40mhz_rate_lut[en_nss][en_vht_mcs];
        }
        else
        {
            us_min_mpdu_len = g_aus_vht_40mhz_rate_lut_shortgi[en_nss][en_vht_mcs];
        }
    }
    else if(WLAN_BAND_ASSEMBLE_80M == en_channel_bandwidth)
    {
        if (OAL_FALSE == en_short_gi_enable)
        {
            us_min_mpdu_len = g_aus_vht_80mhz_rate_lut[en_nss][en_vht_mcs];
        }
        else
        {
            us_min_mpdu_len = g_aus_vht_80mhz_rate_lut_shortgi[en_nss][en_vht_mcs];
        }
    }

    return us_min_mpdu_len;
}


oal_void  dmac_move_ba_window_ahead(dmac_ba_tx_stru *pst_ba_hdl, oal_uint16 us_lsn)
{
    OAL_MEMZERO(pst_ba_hdl->aul_tx_buf_bitmap, OAL_SIZEOF(oal_uint32) * DMAC_TX_BUF_BITMAP_WORDS);
    dmac_ba_update_start_seq_num(pst_ba_hdl, us_lsn + 1);
}


oal_uint8  dmac_ba_get_aggr_mpdu_num_limit(dmac_tid_stru *pst_tid_queue)
{
    oal_uint8 uc_mpdu_num = 0;
    dmac_ba_tx_stru *pst_tx_ba;
    oal_uint16  us_baw_end = 0;
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    dmac_vap_stru            *pst_dmac_vap;
    hal_to_dmac_device_stru  *pst_hal_device;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_tid_queue->uc_vap_id);
    if(OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_WARNING_LOG0(pst_tid_queue->uc_vap_id, OAM_SF_BA, "{dmac_ba_get_aggr_mpdu_num_limit::pst_dmac_vap null.}");
        return 1;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if(OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_tid_queue->uc_vap_id, OAM_SF_BA, "{dmac_ba_get_aggr_mpdu_num_limit::pst_hal_device null.}");
        return 1;
    }

    if(OAL_TRUE == pst_hal_device->en_ampdu_tx_hw_en)
    {
        return WLAN_AMPDU_TX_MAX_NUM;
    }
#endif

    pst_tx_ba = pst_tid_queue->pst_ba_tx_hdl;
    if (OAL_PTR_NULL == pst_tx_ba)
    {
        return 1;
    }

    us_baw_end = DMAC_BA_SEQNO_ADD(pst_tx_ba->us_baw_start, pst_tx_ba->us_baw_size);
    us_baw_end = DMAC_BA_SEQNO_SUB(us_baw_end, 1);

    uc_mpdu_num = (oal_uint8)DMAC_BA_SEQNO_SUB(us_baw_end, pst_tx_ba->us_last_seq_num);

    return uc_mpdu_num;
}


oal_bool_enum dmac_is_ba_setup(oal_void)
{
    oal_uint8                  uc_vap_idx;
    oal_uint8                  uc_device;
    oal_uint8                  uc_device_max;
    mac_device_stru           *pst_mac_device;
    mac_vap_stru               *pst_mac_vap;
    mac_user_stru              *pst_user;
    dmac_user_stru             *pst_dmac_user;
    oal_dlist_head_stru        *pst_entry;
    dmac_tid_stru              *pst_tid_queue;
    oal_uint32                  ul_tid_idx;
    mac_chip_stru              *pst_mac_chip;

    pst_mac_chip = dmac_res_get_mac_chip(0);

    /* OAL接口获取支持device个数 */
    uc_device_max = oal_chip_get_device_num(pst_mac_chip->ul_chip_ver);
    if(uc_device_max > WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hmac_is_device_ba_setup uc_device_max is %d,more than %d.}", uc_device_max, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);
        return OAL_FALSE;
    }

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        pst_mac_device = mac_res_get_dev(pst_mac_chip->auc_device_id[uc_device]);
        if (OAL_PTR_NULL == pst_mac_device)
        {
            continue;
        }

        for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
        {
            pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
            if (OAL_PTR_NULL == pst_mac_vap)
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_is_ba_setup pst_mac_vap is null.}");
                continue;
            }
            if ((MAC_VAP_STATE_UP != pst_mac_vap->en_vap_state) &&
                (MAC_VAP_STATE_PAUSE != pst_mac_vap->en_vap_state))
            {
                continue;
            }

            /* 遍历vap下所有用户,pause tid 队列 */
            OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
            {
                pst_user      = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

                pst_dmac_user = MAC_GET_DMAC_USER(pst_user);

                /* 查看ba 句柄 */
                for (ul_tid_idx = 0; ul_tid_idx < WLAN_TID_MAX_NUM; ul_tid_idx++)
                {
                    pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[ul_tid_idx]);
                    if (DMAC_BA_COMPLETE == pst_tid_queue->st_ba_rx_hdl.en_ba_conn_status)
                    {
                        return OAL_TRUE;
                    }
                }
            }
        }
    }
    return OAL_FALSE;
}

/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_ba_get_aggr_mpdu_num_limit);
/*lint +e578*//*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


