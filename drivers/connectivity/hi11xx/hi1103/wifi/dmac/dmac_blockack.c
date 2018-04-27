


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
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
#include "dmac_user_extend.h"
#endif
#include "dmac_tx_complete.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_BLOCKACK_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
/****************************************************************************
内部宏定义
*****************************************************************************/
#define REPLACE_AHEAD_USER_NUM  4    /* 表示替换算法提前多少用户做替换 */


OAL_STATIC OAL_INLINE oal_uint8   dmac_find_rx_ba_seq(mac_chip_rx_aggr_extend_stru *pst_aggr_extend,
                                                                oal_uint8  uc_rx_ba_session_num,
                                                                oal_uint8  ba_lut_index)
{
    oal_uint8 uc_index;
    for(uc_index=0;uc_index < uc_rx_ba_session_num; uc_index++)
    {
        if(pst_aggr_extend->auc_rx_ba_seq_to_lut_index_map[uc_index] == ba_lut_index)
        {
            return uc_index;
        }
    }
    return HAL_MAX_RX_BA_LUT_SIZE;
}


OAL_STATIC OAL_INLINE oal_uint8 dmac_find_active_ba_index_prev(mac_chip_rx_aggr_extend_stru *pst_aggr_extend,
                                                                oal_uint8  uc_rx_ba_session_num,
                                                                oal_uint8  ba_seq,
                                                                oal_uint8* puc_rx_ba_lut_idx_status_table)
{
    oal_uint8               uc_count;
    oal_uint8               uc_rx_ba_index;
    /* 查找当前收到的BA之前的HAL_MAX_RX_BA_LUT_SIZE-32个表项 找到当前处于active状态的BA 该BA需要被替换出去 */
    for(uc_count=0; uc_count < (uc_rx_ba_session_num - WLAN_MAX_RX_BA); uc_count++)
    {
        if(ba_seq == 0)
        {
            ba_seq = uc_rx_ba_session_num-1;
        }
        else
        {
            ba_seq--;
        }
        uc_rx_ba_index = pst_aggr_extend->auc_rx_ba_seq_to_lut_index_map[ba_seq];
        if(oal_is_active_lut_index(puc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE, uc_rx_ba_index))
        {
            return uc_rx_ba_index;
        }
    }
    return HAL_MAX_RX_BA_LUT_SIZE;
}


OAL_STATIC OAL_INLINE oal_uint8 dmac_find_inactive_ba_index_next(mac_chip_rx_aggr_extend_stru *pst_aggr_extend,
                                                                oal_uint8  uc_rx_ba_session_num,
                                                                oal_uint8  ba_seq,
                                                                oal_uint8* puc_rx_ba_lut_idx_status_table)
{
    oal_uint8               uc_count;
    oal_uint8               uc_rx_ba_index;
    /* 查找当前收到的BA之后的REPLACE_AHEAD_USER_NUM个表项 找到当前处于inactive状态的BA 该BA需要被替换进去 */
    for(uc_count=0; uc_count < REPLACE_AHEAD_USER_NUM ; uc_count++)
    {
        ba_seq++;
        if(ba_seq == uc_rx_ba_session_num)
        {
            ba_seq = 0;
        }

        uc_rx_ba_index = pst_aggr_extend->auc_rx_ba_seq_to_lut_index_map[ba_seq];
        if(!oal_is_active_lut_index(puc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE, uc_rx_ba_index))
        {
            return uc_rx_ba_index;
        }
    }
    return HAL_MAX_RX_BA_LUT_SIZE;
}



OAL_STATIC OAL_INLINE oal_void dmac_collect_rx_ba_seq(mac_chip_rx_aggr_extend_stru *pst_aggr_extend, oal_uint8  ba_lut_index)
{
    oal_uint8 uc_index;

    if(0 == pst_aggr_extend->uc_rx_ba_seq_index)
    {
        pst_aggr_extend->auc_rx_ba_seq_to_lut_index_map[pst_aggr_extend->uc_rx_ba_seq_index] = ba_lut_index;
        OAM_WARNING_LOG2(0, OAM_SF_BA, "{dmac_collect_rx_ba_seq::rx ba index: [%d-->%d].}",pst_aggr_extend->uc_rx_ba_seq_index, ba_lut_index);
        pst_aggr_extend->uc_rx_ba_seq_index++;
    }
    else
    {
        for(uc_index=0; uc_index<pst_aggr_extend->uc_rx_ba_seq_index; uc_index++)
        {
            if(pst_aggr_extend->auc_rx_ba_seq_to_lut_index_map[uc_index] == ba_lut_index)
            {
                pst_aggr_extend->uc_rx_ba_seq_index = 0;
                OAM_WARNING_LOG1(0, OAM_SF_BA, "{dmac_collect_rx_ba_seq::rx ba index collect failed[same with %d]: reinit.}", uc_index);
                return;
            }
        }
        pst_aggr_extend->auc_rx_ba_seq_to_lut_index_map[pst_aggr_extend->uc_rx_ba_seq_index] = ba_lut_index;
        OAM_WARNING_LOG2(0, OAM_SF_BA, "{dmac_collect_rx_ba_seq::rx ba index: [%d-->%d].}",pst_aggr_extend->uc_rx_ba_seq_index, ba_lut_index);
        pst_aggr_extend->uc_rx_ba_seq_index++;

    }
}


OAL_STATIC OAL_INLINE oal_void dmac_ba_addrhl_to_mac(oal_uint8 *puc_mac_addr,
                                                oal_uint32 ul_addr_h,
                                                oal_uint32 ul_addr_l)
{
    puc_mac_addr[0] = ((ul_addr_h) >> 8) & 0xFF;
    puc_mac_addr[1] = (ul_addr_h)& 0xFF;
    puc_mac_addr[2] = (ul_addr_l) >> 24;
    puc_mac_addr[3] = ((ul_addr_l) >> 16) & 0xFF;
    puc_mac_addr[4] = ((ul_addr_l) >> 8) & 0xFF;
    puc_mac_addr[5] = (ul_addr_l)& 0xFF;
}


OAL_STATIC OAL_INLINE oal_void dmac_check_replace_lut_ahead(mac_vap_stru *pst_vap,
                                            dmac_user_stru         *pst_dmac_user,
                                            mac_chip_stru          *pst_mac_chip,
                                            oal_uint8  uc_rx_ba_session_num,
                                            oal_uint8  ba_lut_index)
{
    dmac_vap_stru          *pst_dmac_vap;
    oal_uint8               uc_rx_ba_seq;
    oal_uint8               uc_ba_lut_index;
    oal_uint8               uc_ba_lut_replace_index;
    oal_uint8               uc_hal_to_dmac_lut_index;
    mac_chip_ba_lut_stru   *pst_ba_lut;
    oal_uint32              ul_addr_h;
    oal_uint32              ul_addr_l;
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
    oal_uint32              ul_ret;
    dmac_user_stru         *pst_dmac_user_candi;
    dmac_user_stru         *pst_user_demand;
    oal_uint8               auc_mac_addr[6];
    oal_uint16              us_user_idx;
#endif

    uc_rx_ba_seq = dmac_find_rx_ba_seq(pst_mac_chip->pst_rx_aggr_extend, uc_rx_ba_session_num, ba_lut_index);
    if(HAL_MAX_RX_BA_LUT_SIZE == uc_rx_ba_seq)
    {
        return;
    }

    /* 找到之前已发包且处于active状态的用户，该用户可以暂时被替换出去 */
    uc_ba_lut_index = dmac_find_active_ba_index_prev(pst_mac_chip->pst_rx_aggr_extend, uc_rx_ba_session_num, uc_rx_ba_seq, pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table);
    if(HAL_MAX_RX_BA_LUT_SIZE == uc_ba_lut_index)
    {
        return;
    }

    /* 找到即将发包且处于inactive状态的用户，该用户需要替换进来 */
    uc_ba_lut_replace_index = dmac_find_inactive_ba_index_next(pst_mac_chip->pst_rx_aggr_extend, uc_rx_ba_session_num, uc_rx_ba_seq, pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table);

    if(HAL_MAX_RX_BA_LUT_SIZE == uc_ba_lut_replace_index)
    {
        return;
    }

    //OAM_WARNING_LOG3(pst_vap->uc_vap_id, OAM_SF_BA, "{dmac_ba_filter_serv::uc_rx_ba_seq=%d  [%d] replaced by [%d]: }",uc_rx_ba_seq, uc_ba_lut_index, uc_ba_lut_replace_index);

    for(uc_hal_to_dmac_lut_index=0; uc_hal_to_dmac_lut_index < WLAN_MAX_RX_BA; uc_hal_to_dmac_lut_index++)
    {
        if(pst_mac_chip->pst_rx_aggr_extend->auc_hal_to_dmac_lut_index_map[uc_hal_to_dmac_lut_index] == uc_ba_lut_index)
        {
            pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_vap->uc_vap_id);

            #ifdef _PRE_WLAN_FEATURE_USER_EXTEND
            if(pst_dmac_user->st_user_base_info.st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_NO_ENCRYP)
            {
                dmac_ba_addrhl_to_mac(auc_mac_addr,pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_ba_lut_index].ul_addr_h,
                                      pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_ba_lut_index].ul_addr_l);
                ul_ret = mac_vap_find_user_by_macaddr(pst_vap, auc_mac_addr, &us_user_idx);
                if(OAL_SUCC != ul_ret)
                {
                    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_check_replace_lut_ahead::find_user_by_macaddr failed[%d].}", ul_ret);
                    break;
                }
                pst_dmac_user_candi = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);

                dmac_ba_addrhl_to_mac(auc_mac_addr,pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_ba_lut_replace_index].ul_addr_h,
                                      pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_ba_lut_replace_index].ul_addr_l);
                ul_ret = mac_vap_find_user_by_macaddr(pst_vap, auc_mac_addr, &us_user_idx);
                if(OAL_SUCC != ul_ret)
                {
                    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_check_replace_lut_ahead::find_user_by_macaddr failed[%d].}", ul_ret);
                    break;
                }
                pst_user_demand = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);

                dmac_user_replace(pst_user_demand, pst_dmac_user_candi);
            }
            #endif

            //保存需要替换的ba lut
            pst_ba_lut = &(pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_ba_lut_index]);
            hal_get_machw_ba_params(pst_dmac_vap->pst_hal_device,
                                uc_hal_to_dmac_lut_index,
                                &ul_addr_h,
                                &ul_addr_l,
                                &(pst_ba_lut->ul_bitmap_h), &(pst_ba_lut->ul_bitmap_l),
                                &(pst_ba_lut->ul_ba_param));
            oal_reset_lut_index_status(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE, uc_ba_lut_index);

            // 替换之前保存的ba lut
            pst_ba_lut = &(pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_ba_lut_replace_index]);
            hal_restore_machw_ba_params_with_bitmap(pst_dmac_vap->pst_hal_device,
                                uc_hal_to_dmac_lut_index,
                                pst_ba_lut->ul_addr_h,
                                pst_ba_lut->ul_addr_l,
                                pst_ba_lut->ul_ba_param,
                                pst_ba_lut->ul_bitmap_h,
                                pst_ba_lut->ul_bitmap_l);
            pst_mac_chip->pst_rx_aggr_extend->auc_hal_to_dmac_lut_index_map[uc_hal_to_dmac_lut_index] = uc_ba_lut_replace_index;
            oal_set_lut_index_status(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE,  uc_ba_lut_replace_index);

            break;
        }
    }
}
#endif


oal_uint16  dmac_ba_calculate_min_mpdu_len(dmac_user_stru *pst_dmac_user, hal_tx_txop_alg_stru *pst_txop_alg)
{
    oal_uint16                      us_min_mpdu_len = 0;
    wlan_phy_protocol_enum_uint8    en_protocl_mode;
    hal_channel_assemble_enum_uint8 en_channel_bandwidth;
    oal_bool_enum_uint8             en_short_gi_enable;
    oal_uint8                       uc_shift_idx;
    oal_uint8                       uc_shift;

    if (0 == pst_dmac_user->st_user_base_info.st_ht_hdl.uc_min_mpdu_start_spacing)
    {
        return us_min_mpdu_len;
    }

    en_protocl_mode         = pst_txop_alg->ast_per_rate[0].rate_bit_stru.un_nss_rate.st_vht_nss_mcs.bit_protocol_mode;
    en_channel_bandwidth    = pst_txop_alg->st_rate.en_channel_bandwidth;
    en_short_gi_enable      = pst_txop_alg->ast_per_rate[0].rate_bit_stru.bit_short_gi_enable;

    if (WLAN_HT_PHY_PROTOCOL_MODE == en_protocl_mode)
    {
        us_min_mpdu_len = dmac_ba_get_min_len_ht(pst_txop_alg, en_protocl_mode, en_channel_bandwidth, en_short_gi_enable);
    }
    else if (WLAN_VHT_PHY_PROTOCOL_MODE == en_protocl_mode)
    {
        us_min_mpdu_len = dmac_ba_get_min_len_vht(pst_txop_alg, en_protocl_mode, en_channel_bandwidth, en_short_gi_enable);
    }

    if (0 == us_min_mpdu_len)
    {
        return us_min_mpdu_len;
    }

    uc_shift = (MAC_MAX_START_SPACING - pst_dmac_user->st_user_base_info.st_ht_hdl.uc_min_mpdu_start_spacing);

    /* start spacing为2的整数次幂，实际的min mpdu len为最大值右移对应的个数 */
    for (uc_shift_idx = 0; uc_shift_idx < uc_shift; uc_shift_idx++)
    {
        us_min_mpdu_len += 1;
        us_min_mpdu_len = us_min_mpdu_len >> 1;
    }

    us_min_mpdu_len = OAL_GET_4BYTE_ALIGN_VALUE(us_min_mpdu_len);

    return us_min_mpdu_len;
}


oal_uint32  dmac_ba_filter_serv(
                mac_vap_stru                   *pst_vap,
                dmac_user_stru                 *pst_dmac_user,
                dmac_rx_ctl_stru               *pst_cb_ctrl,
                mac_ieee80211_frame_stru       *pst_frame_hdr)
{
    dmac_vap_stru          *pst_dmac_vap;
    dmac_tid_stru          *pst_tid_queue;
    dmac_ba_rx_stru        *pst_ba_rx_hdl;
    oal_uint8               uc_tid;
    oal_bool_enum_uint8     en_is_4addr;
    oal_uint8               uc_is_tods;
    oal_uint8               uc_is_from_ds;
    oal_uint32              ul_ret;
#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    mac_device_stru        *pst_device;
    mac_chip_stru          *pst_mac_chip;
    oal_uint8               uc_rx_ba_session_num;
    oal_uint8               uc_index;
#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
    dmac_user_stru         *pst_user_demand;
    dmac_user_stru         *pst_user_candi;
    oal_uint16              us_user_idx;
    oal_uint8               auc_mac_addr[6];
    oal_uint8               uc_idx;
#endif
#endif


    if (OAL_UNLIKELY(
        OAL_PTR_NULL == pst_dmac_user ||
        OAL_PTR_NULL == pst_cb_ctrl ||
        OAL_PTR_NULL == pst_frame_hdr ||
        OAL_PTR_NULL == pst_vap))
    {
        OAM_ERROR_LOG2(0, OAM_SF_BA, "{pst_dmac_user[0x%x],pst_cb_ctrl[0x%x]}",
                       pst_dmac_user, pst_cb_ctrl);
        OAM_ERROR_LOG2(0, OAM_SF_BA, "{dmac_ba_filter_serv::param null.pst_frame_hdr[0x%x],pst_vap[0x%x]}",
                       pst_frame_hdr, pst_vap);

        if ((OAL_PTR_NULL != pst_frame_hdr) && (OAL_PTR_NULL == pst_dmac_user))
        {
            OAM_ERROR_LOG3(0, OAM_SF_BA, "{dmac_ba_filter_serv:: source addr[0-2] %02X:%02X:%02X}",
                           pst_frame_hdr->auc_address2[0], pst_frame_hdr->auc_address2[1], pst_frame_hdr->auc_address2[2]);
            OAM_ERROR_LOG3(0, OAM_SF_BA, "{dmac_ba_filter_serv:: source addr[3-5] %02X:%02X:%02X}",
                           pst_frame_hdr->auc_address2[3], pst_frame_hdr->auc_address2[4], pst_frame_hdr->auc_address2[5]);
        }

        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_ret = dmac_ba_check_rx_aggr(pst_vap, pst_frame_hdr);
    if (OAL_SUCC != ul_ret)
    {
        return OAL_SUCC;
    }

    /* 考虑四地址情况获取报文的tid */
    uc_is_tods    = mac_hdr_get_to_ds((oal_uint8 *)pst_frame_hdr);
    uc_is_from_ds = mac_hdr_get_from_ds((oal_uint8 *)pst_frame_hdr);
    en_is_4addr   = uc_is_tods && uc_is_from_ds;

    uc_tid = mac_get_tid_value((oal_uint8 *)pst_frame_hdr, en_is_4addr);

    pst_tid_queue = &(pst_dmac_user->ast_tx_tid_queue[uc_tid]);
    pst_ba_rx_hdl = &pst_tid_queue->st_ba_rx_hdl;

#ifdef _PRE_DEBUG_MODE
    if (OAL_PTR_NULL == pst_tid_queue->pst_tid_ampdu_stat)
    {
        OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_BA, "{dmac_ba_filter_serv::ampdu_stat is null! }");
        return OAL_FAIL;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    /* 获取device结构 */
    pst_device = mac_res_get_dev(pst_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_chip = mac_res_get_mac_chip(pst_vap->uc_chip_id);
    if(OAL_PTR_NULL == pst_mac_chip)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 收集wave发包顺序规律 并按这个规律提前在下一轮发包前替换BA表项 32用户以上才需要执行此替换策略 */
    uc_rx_ba_session_num = pst_device->uc_rx_ba_session_num;

    /* 第一阶段wave会按用户顺序每个用户发一个非聚合包  此阶段我们需要确保用户加密表项和BA表项状态一致*/
    if ((uc_rx_ba_session_num > WLAN_MAX_RX_BA) && (OAL_FALSE == pst_cb_ctrl->st_rx_status.bit_AMPDU))
    {
     #ifdef _PRE_WLAN_FEATURE_USER_EXTEND
        /* 找到用户加密表项和BA表项状态不一致的做交换 */
        for(uc_index=0; uc_index<uc_rx_ba_session_num; uc_index++)
        {
            if(oal_is_active_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE,uc_index))
            {
                dmac_ba_addrhl_to_mac(auc_mac_addr,pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_index].ul_addr_h,
                              pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_index].ul_addr_l);
                ul_ret = mac_vap_find_user_by_macaddr(pst_vap, auc_mac_addr, &us_user_idx);
                if(OAL_SUCC != ul_ret)
                {
                    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_ba_filter_serv::find_user_by_macaddr failed[%d].}", ul_ret);
                    break;
                }
                pst_user_demand = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
                /* 已找到BA表项active 但用户状态没active的用户*/
                if(OAL_FALSE == pst_user_demand->bit_active_user)
                {
                    for(uc_idx=0; uc_idx<uc_rx_ba_session_num; uc_idx++)
                    {
                        if(!oal_is_active_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE,uc_idx))
                        {
                            dmac_ba_addrhl_to_mac(auc_mac_addr,pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_idx].ul_addr_h,
                                          pst_mac_chip->pst_rx_aggr_extend->ast_rx_ba_lut_entry[uc_idx].ul_addr_l);
                            ul_ret = mac_vap_find_user_by_macaddr(pst_vap, auc_mac_addr, &us_user_idx);
                            if(OAL_SUCC != ul_ret)
                            {
                                OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_ba_filter_serv::find_user_by_macaddr failed[%d].}", ul_ret);
                                break;
                            }
                            pst_user_candi = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
                            /* 找到BA表项没active 但用户状态却active的用户 */
                            if(OAL_TRUE == pst_user_candi->bit_active_user)
                            {
                                dmac_user_replace(pst_user_demand, pst_user_candi);
                                break;
                            }
                        }
                    }

                    if(uc_idx == uc_rx_ba_session_num)
                    {
                         /* 没找到BA表项没active 但用户状态却active的用户 直接active用户 */
                         dmac_user_active(pst_user_demand);
                    }
                    break;
                }
            }
        }
    #endif
        pst_mac_chip->pst_rx_aggr_extend->us_rx_ba_seq_phase_one_count++;
        if(pst_mac_chip->pst_rx_aggr_extend->us_rx_ba_seq_phase_one_count >= 20)
        {
            /* 连续20次收到非聚合包 判断是新一轮发包，需要重新收集发包顺序 */
            pst_mac_chip->pst_rx_aggr_extend->uc_prev_handle_ba_index = 0xFF;
            pst_mac_chip->pst_rx_aggr_extend->uc_rx_ba_seq_index = 0;
            oal_memset(pst_mac_chip->pst_rx_aggr_extend->auc_rx_ba_seq_to_lut_index_map, 0xFF, HAL_MAX_RX_BA_LUT_SIZE);
        }
    }

    if ((OAL_TRUE == pst_cb_ctrl->st_rx_status.bit_AMPDU) && (uc_rx_ba_session_num > WLAN_MAX_RX_BA))
    {
        /* 进入聚合发包阶段 重置us_rx_ba_seq_phase_one_count */
        pst_mac_chip->pst_rx_aggr_extend->us_rx_ba_seq_phase_one_count = 0;

        /* 收集发包顺序 */
        if(pst_mac_chip->pst_rx_aggr_extend->uc_rx_ba_seq_index < uc_rx_ba_session_num)
        {
            if((0 == pst_mac_chip->pst_rx_aggr_extend->uc_rx_ba_seq_index) ||
               (pst_mac_chip->pst_rx_aggr_extend->auc_rx_ba_seq_to_lut_index_map[pst_mac_chip->pst_rx_aggr_extend->uc_rx_ba_seq_index - 1] != pst_ba_rx_hdl->uc_lut_index))
            {
                /* 检测聚合发包顺序, 第一次或者lut index 跟上一次不同才调用 过滤掉同一个聚合帧的情况 */
                dmac_collect_rx_ba_seq(pst_mac_chip->pst_rx_aggr_extend, pst_ba_rx_hdl->uc_lut_index);
                if(pst_mac_chip->pst_rx_aggr_extend->uc_rx_ba_seq_index == uc_rx_ba_session_num)
                {
                    /* 收集完成，提前替换即将发包用户 */
                    for(uc_index=0; uc_index<REPLACE_AHEAD_USER_NUM; uc_index++)
                    {
                       dmac_check_replace_lut_ahead(pst_vap, pst_dmac_user,pst_mac_chip,
                           uc_rx_ba_session_num,
                           pst_mac_chip->pst_rx_aggr_extend->auc_rx_ba_seq_to_lut_index_map[uc_rx_ba_session_num-1]);
                    }
                    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_vap->uc_vap_id);
                    OAM_WARNING_LOG0(0, OAM_SF_BA, "{dmac_collect_rx_ba_seq collect done}");
                }
            }
            /* 聚合收集阶段, 如果遇到没在BA lut表项里的上报帧 直接丢弃 */
            if(!oal_is_active_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE,pst_ba_rx_hdl->uc_lut_index))
            {
                //OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA, "{dmac_ba_filter_serv::drop  %d  due to inactive ba lut }",pst_ba_rx_hdl->uc_lut_index);
                return OAL_FAIL;
            }
        }
        else  if(pst_mac_chip->pst_rx_aggr_extend->uc_rx_ba_seq_index == uc_rx_ba_session_num)
        {
            if(pst_mac_chip->pst_rx_aggr_extend->uc_prev_handle_ba_index != pst_ba_rx_hdl->uc_lut_index)
            {
                /* check这次是否需要提前替换ba lut */
                dmac_check_replace_lut_ahead(pst_vap, pst_dmac_user,pst_mac_chip, uc_rx_ba_session_num, pst_ba_rx_hdl->uc_lut_index);

                pst_mac_chip->pst_rx_aggr_extend->uc_prev_handle_ba_index  = pst_ba_rx_hdl->uc_lut_index;

                /* wave最后发包阶段,发包顺序可能会跟统计的不一致 如果遇到没在BA lut表项里的上报帧 直接丢弃 */
                if(!oal_is_active_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE,pst_ba_rx_hdl->uc_lut_index))
                {
                    //OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_BA, "{dmac_ba_filter_serv:: encount no active ba lut  %d and drop frame}",pst_ba_rx_hdl->uc_lut_index);
                    return OAL_FAIL;
                }
            }
        }
    }
#endif


    /* BA会话没有建立 */
    if (DMAC_BA_COMPLETE != pst_ba_rx_hdl->en_ba_conn_status)
    {
        OAM_TID_AMPDU_STATS_INCR(pst_tid_queue->pst_tid_ampdu_stat->ul_ba_recipient_no_ba_session, 1);

        if (OAL_TRUE == pst_cb_ctrl->st_rx_status.bit_AMPDU)
        {
            OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA, "{dmac_ba_filter_serv::ba not created but recv ampdu,ba status: %d.}",pst_ba_rx_hdl->en_ba_conn_status);

            OAM_TID_AMPDU_STATS_INCR(pst_tid_queue->pst_tid_ampdu_stat->ul_ba_recipient_recv_ampdu_no_ba, 1);

            pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_vap->uc_vap_id);

            pst_tid_queue->uc_rx_wrong_ampdu_num++;

            if (WLAN_RX_RESET_BA_THREHOLD == pst_tid_queue->uc_rx_wrong_ampdu_num)
            {
                pst_tid_queue->en_is_delba_ing = OAL_FALSE;
                pst_tid_queue->uc_rx_wrong_ampdu_num = 0;

                OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_BA, "{dmac_ba_filter_serv: rx_wrong_ampdu_num exceed threshold, Del Ba. tid[%d]}",uc_tid);
                dmac_mgmt_delba(pst_dmac_vap, pst_dmac_user, uc_tid, MAC_RECIPIENT_DELBA, MAC_QSTA_SETUP_NOT_DONE);
            }
        }
    }

    return OAL_SUCC;

}


oal_void  dmac_ba_reset_rx_handle(mac_device_stru *pst_device, dmac_user_stru *pst_dmac_user, dmac_ba_rx_stru *pst_ba_rx_hdl)
{
#if (!defined(_PRE_PRODUCT_ID_HI110X_DEV))
    hal_to_dmac_device_stru        *pst_hal_device;
#endif
#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    mac_chip_stru                  *pst_mac_chip;
#endif

    if (OAL_PTR_NULL == pst_dmac_user)
    {
        return;
    }

    /* 110x兼容HG530不删除硬件接收BA信息 */
#if (!defined(_PRE_PRODUCT_ID_HI110X_DEV))
    pst_hal_device = dmac_user_get_hal_device(&(pst_dmac_user->st_user_base_info));
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_CFG, "{dmac_ba_reset_rx_handle::pst_hal_device null}");
        return;
    }

    if (MAC_BA_POLICY_IMMEDIATE == pst_ba_rx_hdl->uc_ba_policy)
    {
    #ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
          pst_mac_chip = mac_res_get_mac_chip(pst_device->uc_chip_id);
          if(OAL_PTR_NULL == pst_mac_chip)
          {
              return;
          }
          if(oal_is_active_lut_index(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE, pst_ba_rx_hdl->uc_lut_index))
          {
              hal_remove_machw_ba_lut_entry(pst_hal_device, pst_ba_rx_hdl->uc_lut_index);
              oal_reset_lut_index_status(pst_mac_chip->st_lut_table.auc_rx_ba_lut_idx_status_table, HAL_MAX_RX_BA_LUT_SIZE,pst_ba_rx_hdl->uc_lut_index);
          }
    #else
           hal_remove_machw_ba_lut_entry(pst_hal_device, pst_ba_rx_hdl->uc_lut_index);
    #endif
    }
#endif

    pst_ba_rx_hdl->uc_lut_index = 0xFF;
    pst_ba_rx_hdl->en_ba_conn_status = DMAC_BA_INIT;
}


oal_void  dmac_ba_reset_tx_handle(mac_device_stru *pst_device, dmac_ba_tx_stru **ppst_tx_ba, oal_uint8 uc_tid)
{
    dmac_tid_stru                        *pst_tid;
    dmac_user_stru                       *pst_dmac_user;
    mac_chip_stru                        *pst_mac_chip;
    hal_to_dmac_device_stru              *pst_hal_device = OAL_PTR_NULL;

    if ((OAL_PTR_NULL == pst_device) || (OAL_PTR_NULL == *ppst_tx_ba))
    {
        //OAM_INFO_LOG0(0, OAM_SF_BA, "{dmac_ba_reset_tx_handle::pst_hal_device or ppst_tx_ba null.}\r\n");
        return;
    }

    pst_mac_chip = dmac_res_get_mac_chip(pst_device->uc_chip_id);
    if(OAL_PTR_NULL == pst_mac_chip)
    {
        return;
    }

    oal_del_lut_index(pst_mac_chip->st_lut_table.auc_tx_ba_index_table, (*ppst_tx_ba)->uc_tx_ba_lut);
    (*ppst_tx_ba)->en_is_ba = OAL_FALSE;

    /* pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user((*ppst_tx_ba)->st_alarm_data.us_mac_user_idx); */
    pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user((*ppst_tx_ba)->us_mac_user_idx);
    if(OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_WARNING_LOG1(0, OAM_SF_BA, "{dmac_ba_reset_tx_handle::pst_dmac_user[%d] null.}", (*ppst_tx_ba)->us_mac_user_idx);
        return;
    }

    pst_hal_device = dmac_user_get_hal_device(&(pst_dmac_user->st_user_base_info));
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_ANY, "{dmac_ba_reset_tx_handle::dmac_user_get_hal_device null}");
        return;
    }

    pst_tid = &(pst_dmac_user->ast_tx_tid_queue[uc_tid]);

    dmac_nontxop_txba_num_updata(pst_dmac_user, uc_tid, OAL_FALSE);

    /* 删除与该TX-BA句柄相关的HT信息 */
    dmac_reset_tx_ba_state_prot(pst_hal_device, pst_dmac_user, uc_tid);

    OAL_MEM_FREE(*ppst_tx_ba, OAL_TRUE);
    pst_tid->pst_ba_tx_hdl = OAL_PTR_NULL;

    /* 删除时，开启TID队列状态 */
    dmac_tid_resume(pst_hal_device, pst_tid, DMAC_TID_PAUSE_RESUME_TYPE_BA);
}



oal_uint32  dmac_ba_send_bar(dmac_ba_tx_stru *pst_tx_ba_handle, dmac_user_stru *pst_dmac_user, dmac_tid_stru *pst_tid_queue)
{
    dmac_vap_stru   *pst_dmac_vap;
    oal_netbuf_stru *pst_netbuf;
    oal_uint16       us_bar_len = 0;
    mac_tx_ctl_stru *pst_tx_ctl;
    oal_uint32       ul_ret;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_dmac_user->st_user_base_info.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BA, "{dmac_ba_send_bar::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请bar帧内存 */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA, "{dmac_ba_send_bar::alloc netbuff failed.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);

    oal_set_netbuf_prev(pst_netbuf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_netbuf, OAL_PTR_NULL);

    us_bar_len = dmac_ba_encap_blockack_req(pst_dmac_vap, pst_netbuf, pst_tx_ba_handle, pst_tid_queue->uc_tid);
    if (0 == us_bar_len)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA, "{dmac_ba_send_bar::us_bar_len=0.}");
        oal_netbuf_free(pst_netbuf);

        return OAL_FAIL;
    }

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    OAL_MEMZERO(pst_tx_ctl, OAL_SIZEOF(mac_tx_ctl_stru));

    MAC_GET_CB_EVENT_TYPE(pst_tx_ctl)            = FRW_EVENT_TYPE_HOST_CRX;
    MAC_GET_CB_TX_USER_IDX(pst_tx_ctl)           = pst_dmac_user->st_user_base_info.us_assoc_id;
    MAC_SET_CB_IS_BAR(pst_tx_ctl, OAL_TRUE);
    MAC_GET_CB_WME_TID_TYPE(pst_tx_ctl) = pst_tid_queue->uc_tid;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl)   = WLAN_WME_AC_MGMT;
	MAC_SET_CB_FRAME_HEADER_ADDR(pst_tx_ctl, (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf));
    MAC_GET_CB_WLAN_FRAME_TYPE(pst_tx_ctl)            = WLAN_CONTROL;
    MAC_GET_CB_WLAN_FRAME_SUBTYPE(pst_tx_ctl) = WLAN_BLOCKACK_REQ;

    /* 调用发送管理帧接口 */
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_netbuf, us_bar_len);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BA, "{dmac_ba_send_bar::dmac_tx_mgmt failed[%d].}", ul_ret);
        oal_netbuf_free(pst_netbuf);
        return ul_ret;
    }

    /* 暂停该tid的调度等业务 */
    dmac_tid_pause(pst_tid_queue, DMAC_TID_PAUSE_RESUME_TYPE_BA);

    return OAL_SUCC;
}


oal_void dmac_clear_tx_qos_seq_num(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    hal_set_tx_sequence_num(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index, 0, 0, 0, 0);
#else
    oal_uint8 uc_tid_loop;

    /* 遍历TID清空，LUT表清空功能会清零所有LUT INDEX */
    for(uc_tid_loop = 0; uc_tid_loop < WLAN_TID_MAX_NUM; uc_tid_loop++)
    {
        hal_set_tx_sequence_num(pst_dmac_vap->pst_hal_device, pst_dmac_user->uc_lut_index, uc_tid_loop, OAL_TRUE, 0, 0);
    }
#endif

}


oal_void dmac_clear_tx_nonqos_seq_num(dmac_vap_stru *pst_dmac_vap)
{
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    hal_set_tx_sequence_num(pst_dmac_vap->pst_hal_device, 0, 0, 0, 0, pst_dmac_vap->pst_hal_vap->uc_vap_id);
#endif

}

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW

oal_void dmac_ba_tx_save_ampdu_para(dmac_user_stru *pst_dmac_user,
                                                  dmac_ba_tx_stru *pst_ba_tx_hdl,
                                                  hal_ba_para_stru *pst_ba_para,
                                                  oal_uint8   uc_tidno)
{
    oal_uint8                           uc_bitmap_index;
    oal_uint8                           uc_shift;
    oal_uint32                          ul_mask = 0;

    pst_ba_para->uc_lut_index = pst_ba_tx_hdl->uc_tx_ba_lut;
    pst_ba_para->uc_peer_lut_index = pst_dmac_user->uc_lut_index;
    pst_ba_para->uc_tid       = uc_tidno;
    pst_ba_para->uc_mmss      = pst_dmac_user->st_user_base_info.st_ht_hdl.uc_min_mpdu_start_spacing;
    pst_ba_para->us_win_size  = pst_ba_tx_hdl->us_baw_size;
    pst_ba_para->us_seq_num   = pst_dmac_user->aus_txseqs[uc_tidno];
    pst_ba_para->us_ssn       = pst_ba_tx_hdl->us_baw_start;

    /* 更新最大聚合个数----硬件:64 软件:32 */
    pst_ba_tx_hdl->uc_ampdu_max_num = OAL_MAX((oal_uint8)pst_ba_tx_hdl->us_baw_size, 1);

#if 1
    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: lut[%d]index[%d]tid[%d]mmss[%d]!}",
          pst_ba_para->uc_lut_index,
          pst_ba_para->uc_peer_lut_index,
          pst_ba_para->uc_tid,
          pst_ba_para->uc_mmss);

    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: ssn[%d]seq[%d]win[%d]lastseq[%d]!}",
          pst_ba_para->us_ssn,
          pst_ba_para->us_seq_num,
          pst_ba_para->us_win_size,
          pst_ba_tx_hdl->us_last_seq_num);

    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: bit0[0x%x]bit1[0x%x]bit2[0x%x]bit3[0x%x]!}",
          pst_ba_tx_hdl->aul_tx_buf_bitmap[0],
          pst_ba_tx_hdl->aul_tx_buf_bitmap[1],
          pst_ba_tx_hdl->aul_tx_buf_bitmap[2],
          pst_ba_tx_hdl->aul_tx_buf_bitmap[3]);
#endif

    /* MAC: 软件切至硬件，可以不填写bitmap,硬件第一次发送BAR处理 */
    /* MAC:ssn表示bitmap窗lsb所表示的seq,bitmap为1表示已被确认 */
    uc_bitmap_index = (oal_uint8)(pst_ba_tx_hdl->us_baw_head >> DMAC_TX_BUF_BITMAP_LOG2_WORD_SIZE);
    uc_shift  = pst_ba_tx_hdl->us_baw_head & DMAC_TX_BUF_BITMAP_WORD_MASK;

#if 1
    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: ssn[%d]bitmap[0x%x]head[%d]tail[%d]!}",
          pst_ba_tx_hdl->us_baw_start,
          uc_bitmap_index,
          pst_ba_tx_hdl->us_baw_head,
          pst_ba_tx_hdl->us_baw_tail);
#endif

#if 0
    if (3 == uc_bitmap_index)
    {
        /* 拼接lsb部分 */
        pst_ba_para->ul_bitmap_lsb = pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index] >> uc_shift;
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: lsb step1[0x%x]!}",pst_ba_para->ul_bitmap_lsb);
        pst_ba_para->ul_bitmap_lsb |= (pst_ba_tx_hdl->aul_tx_buf_bitmap[0] & ((1 << uc_shift)-1)) << (DMAC_TX_BUF_BITMAP_WORD_SIZE -uc_shift);
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: lsb step2[0x%x]!}",pst_ba_para->ul_bitmap_lsb);
        /* 拼接msb部分 */
        pst_ba_para->ul_bitmap_msb = pst_ba_tx_hdl->aul_tx_buf_bitmap[0] >> uc_shift;
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: msb step1[0x%x]!}",pst_ba_para->ul_bitmap_msb);
        pst_ba_para->ul_bitmap_msb |= (pst_ba_tx_hdl->aul_tx_buf_bitmap[1] & ((1 << uc_shift)-1)) << (DMAC_TX_BUF_BITMAP_WORD_SIZE -uc_shift);
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: msb step2[0x%x]!}",pst_ba_para->ul_bitmap_msb);
    }
    else if (2 == uc_bitmap_index)
    {
        /* 拼接lsb部分 */
        pst_ba_para->ul_bitmap_lsb = pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index] >> uc_shift;
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: lsb step1[0x%x]!}",pst_ba_para->ul_bitmap_lsb);
        pst_ba_para->ul_bitmap_lsb |= (pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index + 1] & ((1 << uc_shift)-1)) << (DMAC_TX_BUF_BITMAP_WORD_SIZE -uc_shift);
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: lsb step2[0x%x]!}",pst_ba_para->ul_bitmap_lsb);
        /* 拼接msb部分 */
        pst_ba_para->ul_bitmap_msb = pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index + 1] >> uc_shift;
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: msb step1[0x%x]!}",pst_ba_para->ul_bitmap_msb);
        pst_ba_para->ul_bitmap_msb |= (pst_ba_tx_hdl->aul_tx_buf_bitmap[0] & ((1 << uc_shift)-1)) << (DMAC_TX_BUF_BITMAP_WORD_SIZE -uc_shift);
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: msb step2[0x%x]!}",pst_ba_para->ul_bitmap_msb);
    }
    else
    {
        /* 拼接lsb部分 */
        pst_ba_para->ul_bitmap_lsb = pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index] >> uc_shift;
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: lsb step1[0x%x]!}",pst_ba_para->ul_bitmap_lsb);
        pst_ba_para->ul_bitmap_lsb |= (pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index + 1] & ((1 << uc_shift)-1)) << (DMAC_TX_BUF_BITMAP_WORD_SIZE -uc_shift);
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: lsb step2[0x%x]!}",pst_ba_para->ul_bitmap_lsb);
        /* 拼接msb部分 */
        pst_ba_para->ul_bitmap_msb = pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index + 1] >> uc_shift;
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: msb step1[0x%x]!}",pst_ba_para->ul_bitmap_msb);
        pst_ba_para->ul_bitmap_msb |= (pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index + 2] & ((1 << uc_shift)-1)) << (DMAC_TX_BUF_BITMAP_WORD_SIZE -uc_shift);
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: msb step2[0x%x]!}",pst_ba_para->ul_bitmap_msb);
    }

#else
    /* 拼接lsb部分 */
    pst_ba_para->ul_bitmap_lsb = pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index] >> uc_shift;
    //OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: lsb step1[0x%x],index[%d]!}",pst_ba_para->ul_bitmap_lsb,uc_bitmap_index);
    uc_bitmap_index++;
    uc_bitmap_index &= 0x3;
    pst_ba_para->ul_bitmap_lsb |= (pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index] & ((1U << uc_shift)-1)) << (DMAC_TX_BUF_BITMAP_WORD_SIZE -uc_shift);
    //OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: lsb step2[0x%x],index[%d]!}",pst_ba_para->ul_bitmap_lsb,uc_bitmap_index);

    /* 拼接msb部分 */
    pst_ba_para->ul_bitmap_msb = pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index] >> uc_shift;
    //OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: msb step1[0x%x],index[%d]!}",pst_ba_para->ul_bitmap_msb,uc_bitmap_index);
    uc_bitmap_index++;
    uc_bitmap_index &= 0x3;
    pst_ba_para->ul_bitmap_msb |= (pst_ba_tx_hdl->aul_tx_buf_bitmap[uc_bitmap_index] & ((1U << uc_shift)-1)) << (DMAC_TX_BUF_BITMAP_WORD_SIZE -uc_shift);
    //OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: msb step2[0x%x],index[%d]!}",pst_ba_para->ul_bitmap_msb,uc_bitmap_index);
#endif

    /* MAC: 确认逻辑与软件相反, */
    pst_ba_para->ul_bitmap_lsb = ~pst_ba_para->ul_bitmap_lsb;
    pst_ba_para->ul_bitmap_msb = ~pst_ba_para->ul_bitmap_msb;

    /* bitmap中有效的seq num范围,其余均清0表示未确认 */
    if (DMAC_TX_BUF_BITMAP_WORD_SIZE <= (pst_ba_tx_hdl->us_baw_tail - pst_ba_tx_hdl->us_baw_head))
    {
        ul_mask = (1U << (pst_ba_tx_hdl->us_baw_tail - DMAC_TX_BUF_BITMAP_WORD_SIZE)) - 1;
        pst_ba_para->ul_bitmap_msb &= ul_mask;
    }
    else
    {
        ul_mask = (1U << (pst_ba_tx_hdl->us_baw_tail - pst_ba_tx_hdl->us_baw_head)) - 1;
        pst_ba_para->ul_bitmap_lsb &= ul_mask;
        pst_ba_para->ul_bitmap_msb = 0;
    }

#if 1
    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: mask[0x%x]shift[%d]lsb[0x%x]msb[0x%x]!}",
          ul_mask,
          uc_shift,
          pst_ba_para->ul_bitmap_lsb,
          pst_ba_para->ul_bitmap_msb);
#endif
}


oal_void dmac_ba_tx_restore_ampdu_para(dmac_user_stru *pst_dmac_user,
                                                     dmac_ba_tx_stru *pst_ba_tx_hdl,
                                                     hal_ba_para_stru *pst_ba_para,
                                                     oal_uint8      uc_tidno)
{
    //oal_uint16          us_bitmap_index;
    //oal_uint32          us_cindex;

#if 1
    if (pst_dmac_user->aus_txseqs[uc_tidno] != pst_ba_para->us_seq_num)
    {
        OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_ba_tx_restore_ampdu_para:: softseq[%d]hwseq[%d]!}",
              pst_dmac_user->aus_txseqs[uc_tidno],
              pst_ba_para->us_seq_num);
    }
#endif

    /* 更新最大聚合个数----硬件:64 软件:32 */
    pst_ba_tx_hdl->uc_ampdu_max_num = OAL_MAX((oal_uint8)(pst_ba_tx_hdl->us_baw_size/WLAN_AMPDU_TX_SCHD_STRATEGY), 1);

    /* 获取mac当前seq num */
    pst_dmac_user->aus_txseqs[uc_tidno] = pst_ba_para->us_seq_num;
    /* 对于没有被确认的帧,都已被回收至TID处理,因此BA句柄简化为初始化处理 */
    dmac_move_ba_window_ahead(pst_ba_tx_hdl, pst_ba_para->us_seq_num - 1);

    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_restore_ampdu_para:: lut[%d]index[%d]hwseq[%d]lastseq[%d]!}",
          pst_ba_para->uc_lut_index,
          pst_ba_para->uc_peer_lut_index,
          pst_dmac_user->aus_txseqs[uc_tidno],
          pst_ba_tx_hdl->us_last_seq_num);

    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_restore_ampdu_para:: bawhead[%d]bawtail[%d]ssn[0x%x]tid[0x%x]!}",
          pst_ba_tx_hdl->us_baw_head,
          pst_ba_tx_hdl->us_baw_tail,
          pst_ba_tx_hdl->us_baw_start,
          uc_tidno);

#if 0
    pst_ba_tx_hdl->us_last_seq_num = pst_dmac_user->aus_txseqs[uc_tidno] - 1;

    /* 清空bitmap,默认都已确认 */
    for (us_bitmap_index = 0; us_bitmap_index < DMAC_TX_BUF_BITMAP_WORDS; us_bitmap_index++)
    {
        pst_ba_tx_hdl->aul_tx_buf_bitmap[us_bitmap_index] = 0;
    }

    /* 对于没有被确认的帧,都已被回收至TID处理,因此BA句柄简化为初始化处理 */
#if 0
    /* MAC:ssn表示bitmap窗lsb所表示的seq; bitmap为1表示已被确认,与驱动逻辑相反 */
    pst_ba_tx_hdl->us_baw_start = pst_ba_para->us_ssn;
    pst_ba_tx_hdl->aul_tx_buf_bitmap[0] = ~(pst_ba_para->ul_bitmap_lsb);
    pst_ba_tx_hdl->aul_tx_buf_bitmap[1] = ~(pst_ba_para->ul_bitmap_msb);
#else
    /* 规避:默认所有发送帧都已经确认,ssn从下一个序列号开始 */
    pst_ba_tx_hdl->us_baw_start = pst_dmac_user->aus_txseqs[uc_tidno];
#endif

    /* 默认保持bitmap窗首,对应baw start */
    pst_ba_tx_hdl->us_baw_head = 0;
    us_bitmap_index  = DMAC_BA_INDEX(pst_ba_tx_hdl->us_baw_start, pst_dmac_user->aus_txseqs[uc_tidno]);
    us_cindex = (oal_uint16)((pst_ba_tx_hdl->us_baw_head + us_bitmap_index) & (DMAC_TID_MAX_BUFS - 1));
    pst_ba_tx_hdl->us_baw_tail = (oal_uint16)(pst_ba_tx_hdl->us_baw_head + us_cindex);

    /* 更新bitmap窗 */
    dmac_ba_update_baw(pst_ba_tx_hdl, pst_ba_tx_hdl->us_baw_start - 1);
#if 1
    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_restore_ampdu_para:: lut[%d]index[%d]hwseq[%d]lastseq[%d]!}",
          pst_ba_para->uc_lut_index,
          pst_ba_para->uc_peer_lut_index,
          pst_dmac_user->aus_txseqs[uc_tidno],
          pst_ba_tx_hdl->us_last_seq_num);
    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_restore_ampdu_para:: ssn[%d]bitmapindex[%d]cindex[%d]ssn[%d]!}",
          pst_ba_tx_hdl->us_baw_start,
          us_bitmap_index,
          us_cindex,
          pst_ba_para->us_ssn);
    OAM_WARNING_LOG4(0, OAM_SF_CFG, "{dmac_ba_tx_restore_ampdu_para:: bawhead[%d]bawtail[%d]lsb[0x%x]msb[0x%x]!}",
          pst_ba_tx_hdl->us_baw_head,
          pst_ba_tx_hdl->us_baw_tail,
          pst_ba_tx_hdl->aul_tx_buf_bitmap[0],
          pst_ba_tx_hdl->aul_tx_buf_bitmap[1]);
#endif
#endif


}


oal_void dmac_ba_switch_sw_to_hw(mac_vap_stru *pst_mac_vap)
{
    oal_uint8                           uc_tidno;
    oal_uint8                           uc_vap_idx;
    oal_uint8                           uc_device;
    oal_uint8                           uc_device_max;
    mac_user_stru                      *pst_mac_user;
    dmac_user_stru                     *pst_dmac_user;
    mac_chip_stru                      *pst_mac_chip;
    mac_device_stru                    *pst_mac_device;
    oal_dlist_head_stru                *pst_entry;
    oal_dlist_head_stru                *pst_next_entry;
    hal_to_dmac_device_stru            *pst_hal_dev;
    dmac_ba_tx_stru                    *pst_ba_tx_hdl;
    hal_ba_para_stru                    st_ba_para = {0};

    pst_mac_chip = mac_res_get_mac_chip(pst_mac_vap->uc_chip_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_chip))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_ba_tx_save_ampdu_para:: pst_mac_chip null!}");
        return;
    }

    uc_device_max = oal_chip_get_device_num(pst_mac_chip->ul_chip_ver);

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
                continue;
            }

            OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_next_entry, &(pst_mac_vap->st_mac_user_list_head))
            {
                pst_mac_user = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

                pst_dmac_user = mac_res_get_dmac_user(pst_mac_user->us_assoc_id);
                if (OAL_PTR_NULL == pst_dmac_user)
                {
                    continue;
                }

                pst_hal_dev = dmac_user_get_hal_device(pst_mac_user);
                if (OAL_PTR_NULL == pst_hal_dev)
                {
                    continue;
                }

                for (uc_tidno = 0; uc_tidno < WLAN_TID_MAX_NUM; uc_tidno++)
                {
                    pst_ba_tx_hdl = pst_dmac_user->ast_tx_tid_queue[uc_tidno].pst_ba_tx_hdl;
                    if (OAL_PTR_NULL == pst_ba_tx_hdl)
                    {
                        continue;
                    }

                    if (DMAC_BA_COMPLETE != pst_ba_tx_hdl->en_ba_conn_status)
                    {
                        continue;
                    }

                    dmac_ba_tx_save_ampdu_para(pst_dmac_user, pst_ba_tx_hdl, &st_ba_para, uc_tidno);

                    hal_save_tx_ba_para(pst_hal_dev, &st_ba_para);
                }
            }
        }
    }

}


oal_void dmac_ba_switch_hw_to_sw(mac_vap_stru *pst_mac_vap)
{
    oal_uint8                           uc_tidno;
    oal_uint8                           uc_vap_idx;
    oal_uint8                           uc_device;
    oal_uint8                           uc_device_max;
    mac_user_stru                      *pst_mac_user;
    dmac_user_stru                     *pst_dmac_user;
    mac_chip_stru                      *pst_mac_chip;
    mac_device_stru                    *pst_mac_device;
    oal_dlist_head_stru                *pst_entry;
    oal_dlist_head_stru                *pst_next_entry;
    hal_to_dmac_device_stru            *pst_hal_dev;
    dmac_ba_tx_stru                    *pst_ba_tx_hdl;
    hal_ba_para_stru                    st_ba_para = {0};

    pst_mac_chip = mac_res_get_mac_chip(pst_mac_vap->uc_chip_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_chip))
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{dmac_ba_tx_restore_ampdu_para:: pst_mac_chip null!}");
        return;
    }

    uc_device_max = oal_chip_get_device_num(pst_mac_chip->ul_chip_ver);

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
                continue;
            }

            OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_next_entry, &(pst_mac_vap->st_mac_user_list_head))
            {
                pst_mac_user = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);

                pst_dmac_user = mac_res_get_dmac_user(pst_mac_user->us_assoc_id);
                if (OAL_PTR_NULL == pst_dmac_user)
                {
                    continue;
                }

                pst_hal_dev = dmac_user_get_hal_device(pst_mac_user);
                if (OAL_PTR_NULL == pst_hal_dev)
                {
                    continue;
                }

                for (uc_tidno = 0; uc_tidno < WLAN_TID_MAX_NUM; uc_tidno++)
                {
                    pst_ba_tx_hdl = pst_dmac_user->ast_tx_tid_queue[uc_tidno].pst_ba_tx_hdl;
                    if (OAL_PTR_NULL == pst_ba_tx_hdl)
                    {
                        continue;
                    }

                    if (DMAC_BA_COMPLETE != pst_ba_tx_hdl->en_ba_conn_status)
                    {
                        continue;
                    }

                    /* 刷掉硬件队列帧 */
                    dmac_tx_restore_txq_to_tid(pst_hal_dev, pst_dmac_user, uc_tidno);

                    /* 恢复BA */
                    st_ba_para.uc_lut_index = pst_ba_tx_hdl->uc_tx_ba_lut;
                    st_ba_para.uc_peer_lut_index = pst_dmac_user->uc_lut_index;
                    st_ba_para.uc_tid  = uc_tidno;
                    hal_get_tx_ba_para(pst_hal_dev, &st_ba_para);

                    dmac_ba_tx_restore_ampdu_para(pst_dmac_user, pst_ba_tx_hdl, &st_ba_para, uc_tidno);
                }
            }
        }
    }
}


oal_void dmac_ba_set_ampdu_ctrl(mac_vap_stru *pst_mac_vap, oal_uint8 uc_enable_tx_hw, oal_uint8 uc_snd_type)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint8                       uc_device_id;
    oal_uint8                       uc_hal_dev_num_per_chip;

    /* 不对外直接暴露规格宏 */
    hal_chip_get_device_num(pst_mac_vap->uc_chip_id, &uc_hal_dev_num_per_chip);

    for (uc_device_id = 0; uc_device_id < uc_hal_dev_num_per_chip; uc_device_id++)
    {
        hal_get_hal_to_dmac_device(pst_mac_vap->uc_chip_id, uc_device_id, &pst_hal_device);
        if (OAL_PTR_NULL == pst_hal_device)
        {
            OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_config_set_ampdu_tx_hw_on::hal_get_hal_to_dmac_device fail chip id[%d],device id[%d]}",
                            pst_mac_vap->uc_chip_id, uc_device_id);
            continue;
        }

        hal_set_ampdu_tx_hw_on(pst_hal_device, uc_enable_tx_hw, uc_snd_type);

        pst_hal_device->en_ampdu_tx_hw_en = uc_enable_tx_hw;
        pst_hal_device->en_ampdu_partial_resnd = uc_snd_type;
    }

}


oal_void dmac_ba_tx_ampdu_switch(mac_vap_stru *pst_mac_vap, oal_uint8 uc_enable_tx_hw, oal_uint8 uc_snd_type)
{
    hal_to_dmac_device_stru        *pst_hal_device;
    oal_uint8                       uc_device_id;
    oal_uint8                       uc_hal_dev_num_per_chip;

    hal_chip_get_device_num(pst_mac_vap->uc_chip_id, &uc_hal_dev_num_per_chip);

    /* pause chip 所有用户 */
    dmac_chip_pause_all_user(pst_mac_vap);

    /* disable PA */
    for (uc_device_id = 0; uc_device_id < uc_hal_dev_num_per_chip; uc_device_id++)
    {
        hal_get_hal_to_dmac_device(pst_mac_vap->uc_chip_id, uc_device_id, &pst_hal_device);
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        hal_disable_machw_phy_and_pa(pst_hal_device);

        /* 处理发送完成中断 */
        hal_flush_tx_complete_irq(pst_hal_device);

        /* 清空发送完成事件队列*/
        frw_event_flush_event_queue(FRW_EVENT_TYPE_WLAN_TX_COMP);
    }

    if (OAL_TRUE == uc_enable_tx_hw)
    {
        /* 将每个聚合TID的seq num、bitmap配置LUT */
        dmac_ba_switch_sw_to_hw(pst_mac_vap);
    }
    else
    {
        dmac_ba_switch_hw_to_sw(pst_mac_vap);
    }

    /* sw/hw ampdu switch */
    dmac_ba_set_ampdu_ctrl(pst_mac_vap, uc_enable_tx_hw, uc_snd_type);

    /* enable PA */
    for (uc_device_id = 0; uc_device_id < uc_hal_dev_num_per_chip; uc_device_id++)
    {
        hal_get_hal_to_dmac_device(pst_mac_vap->uc_chip_id, uc_device_id, &pst_hal_device);
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        hal_recover_machw_phy_and_pa(pst_hal_device);
    }

    /* resume chip 所有用户 */
    dmac_chip_resume_all_user(pst_mac_vap);

}

#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

