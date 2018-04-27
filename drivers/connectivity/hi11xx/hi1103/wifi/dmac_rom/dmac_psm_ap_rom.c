    


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_net.h"
#include "wlan_spec.h"
#include "mac_frame.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "dmac_vap.h"
#include "dmac_main.h"
#include "dmac_user.h"
#include "dmac_psm_ap.h"
#include "dmac_uapsd.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_rx_data.h"
#include "hal_ext_if.h"
#include "dmac_blockack.h"
#include "dmac_tx_complete.h"
#include "dmac_beacon.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "dmac_p2p.h"
#endif
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_PSM_AP_ROM_C
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/


oal_void dmac_psm_set_more_data(oal_netbuf_stru *pst_net_buf)
{
    mac_tx_ctl_stru     *pst_tx_ctrl;
    mac_ieee80211_frame_stru    *pst_frame_hdr;

    pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);
    pst_frame_hdr = MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctrl);
    pst_frame_hdr->st_frame_control.bit_more_data = 0x01;
}


oal_void dmac_psm_set_local_bitmap(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_uint8 uc_bitmap_flg)
{
    oal_uint16          us_user_assoc_id;
    oal_uint8           uc_tim_byte_idx;
    oal_uint8           uc_tim_bit_mask;
    oal_uint8           uc_tim_offset;
    oal_uint8           uc_PVBitmap_len;
    oal_uint8           uc_tim_min_offset;
    oal_uint8           uc_tim_max_offset;
    oal_uint8           uc_index;

    if (OAL_PTR_NULL == pst_dmac_vap || OAL_PTR_NULL == pst_dmac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_set_local_bitmap::param null.}");
        return;
    }
    /***************************************************************************
     ---------------------------------------------------------------------------
     |TIM bitmap len |Bitmap Control            |Partial Virtual Bitmap|
     ---------------------------------------------------------------------------
     |1              |1 (bit1-7=offset bit0=BMC)|1~251                 |
     ---------------------------------------------------------------------------
    ***************************************************************************/

    /* the Partial Virtual Bitmap field consists of octets numbered N1 to N2
       of the traffic indication virtual bitmap, where N1 is the largest even
       number such that bits numbered 1 to (N1 * 8) - 1 in the bitmap are all
       0 and N2 is the smallest number such that bits numbered (N2 + 1) * 8 to
       2007 in the bitmap are all 0. In this  case, the Bitmap Offset subfield
       value contains the number N1/2 */
    uc_PVBitmap_len  = pst_dmac_vap->puc_tim_bitmap[0];
    uc_tim_offset    = 2 + (pst_dmac_vap->puc_tim_bitmap[1] & (oal_uint8)(~BIT0));

    /* 长度校验 */
    if (uc_PVBitmap_len + uc_tim_offset > pst_dmac_vap->uc_tim_bitmap_len)
    {
        OAM_ERROR_LOG3(0, OAM_SF_PWR, "{dmac_psm_set_local_bitmap::tim_offset[%d] + len[%d] >= bitmap_len[%d].}",
                       uc_tim_offset, uc_PVBitmap_len, pst_dmac_vap->uc_tim_bitmap_len);
        OAL_MEMZERO(pst_dmac_vap->puc_tim_bitmap, pst_dmac_vap->uc_tim_bitmap_len);
        /* TIM bitmap len is default 1*/
        pst_dmac_vap->puc_tim_bitmap[0] = 1;
        uc_PVBitmap_len = 1;
    }

    /* 如果是组播用户，修改Bitmap Control的bit0，
       否则根据用户关联id找到用户在Partial Virtual Bitmap中的字节位置与比特位置 */
    if (OAL_TRUE == pst_dmac_user->st_user_base_info.en_is_multi_user)
    {
        uc_tim_byte_idx          = 1;
        uc_tim_bit_mask          = (oal_uint8)(BIT0);
    }
    else
    {
        us_user_assoc_id         = pst_dmac_user->st_user_base_info.us_assoc_id;
        uc_tim_byte_idx          = 2 + (oal_uint8)(us_user_assoc_id >> 3);
        uc_tim_bit_mask          = (oal_uint8)(BIT0 << (us_user_assoc_id & 0x07));
    }
    if (uc_tim_byte_idx >= pst_dmac_vap->uc_tim_bitmap_len)
    {
        OAM_ERROR_LOG3(0, OAM_SF_PWR, "{dmac_psm_set_local_bitmap::usr[%d] tim_byte_idx[%d] >= bitmap_len[%d].}",
                       pst_dmac_user->st_user_base_info.us_assoc_id, uc_tim_byte_idx, pst_dmac_vap->uc_tim_bitmap_len);
        return;
    }
    /* 修改相应的bit的值 */
    if (0 == uc_bitmap_flg)
    {
        pst_dmac_vap->puc_tim_bitmap[uc_tim_byte_idx] &= (oal_uint8)(~uc_tim_bit_mask);
    }
    else
    {
        pst_dmac_vap->puc_tim_bitmap[uc_tim_byte_idx] |= (oal_uint8)uc_tim_bit_mask;
    }

    /* 若是组播，则不涉及修改dtim */
    if (1 == uc_tim_byte_idx)
    {
        return;
    }
    uc_tim_min_offset = OAL_MIN(uc_tim_byte_idx, uc_tim_offset);
    uc_tim_max_offset = OAL_MAX(uc_tim_byte_idx, uc_tim_offset + uc_PVBitmap_len - 1);
    /* 找到最小的非0的作为tim_offset(必须是偶数)，为了减少循环次数，只比较本次修改所涉及的字节 */
    uc_tim_offset     = 2;
    for (uc_index = uc_tim_min_offset; uc_index <= uc_tim_max_offset; uc_index++)
    {
        if (0 != pst_dmac_vap->puc_tim_bitmap[uc_index])
        {
            uc_tim_offset = uc_index & (~1);
            break;
        }
    }

    /* 找到最大的非0的用来计算PVBitmap_len */
    for (uc_index = uc_tim_max_offset; uc_index > uc_tim_offset; uc_index--)
    {
        if (0 != pst_dmac_vap->puc_tim_bitmap[uc_index])
        {
            break;
        }
    }
    /*更新PVBitmap_len及Bitmap Control(bit1-bit7表示offset；bit0表示BMC */
    pst_dmac_vap->puc_tim_bitmap[0] = (uc_index - uc_tim_offset) + 1;
    pst_dmac_vap->puc_tim_bitmap[1] &= (oal_uint8)(BIT0);
    pst_dmac_vap->puc_tim_bitmap[1] += uc_tim_offset - 2;
}
#if 0

oal_uint8 dmac_psm_get_local_bitmap(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{
    oal_uint16          us_user_assoc_id;
    oal_uint8           uc_byte_idx;
    oal_uint8           uc_bit_mask = (oal_uint8)(BIT0);

    /* Bitmap Control的bit0表示组播用户 */
    if (pst_dmac_user->st_user_base_info.en_is_multi_user)
    {
        return ((pst_dmac_vap->puc_tim_bitmap[1] & uc_bit_mask) == uc_bit_mask);
    }

    /* 根据用户关联id找到用户在bitmap中的字节位置与比特位置 */
    us_user_assoc_id = pst_dmac_user->st_user_base_info.us_assoc_id;
    uc_byte_idx      = 2 + (oal_uint8)(us_user_assoc_id >> 3);
    uc_bit_mask      = (oal_uint8)(BIT0 << (us_user_assoc_id & 0x07));
    if (uc_byte_idx >= pst_dmac_vap->uc_tim_bitmap_len)
    {
        return 0;
    }

    return ((pst_dmac_vap->puc_tim_bitmap[uc_byte_idx] & uc_bit_mask) == uc_bit_mask);
}


oal_uint8 dmac_psm_get_bitmap_len(dmac_vap_stru *pst_dmac_vap)
{
    return pst_dmac_vap->puc_tim_bitmap[0];
}


oal_uint8 dmac_psm_get_bitmap_offset(dmac_vap_stru *pst_dmac_vap)
{
    return (oal_uint8)(2 + (pst_dmac_vap->puc_tim_bitmap[1] & (oal_uint8)(~BIT0)));
}
#endif




oal_uint32 dmac_psm_user_ps_structure_init(dmac_user_stru *pst_dmac_user)
{
    dmac_user_ps_stru *pst_ps_structure;

    pst_ps_structure = &(pst_dmac_user->st_ps_structure);

    oal_spin_lock_init(&(pst_ps_structure->st_lock_ps));

    oal_netbuf_list_head_init(&(pst_ps_structure->st_ps_queue_head));

    oal_atomic_set(&pst_ps_structure->uc_mpdu_num, 0);

    pst_ps_structure->en_is_pspoll_rsp_processing = OAL_FALSE;

    pst_ps_structure->uc_ps_time_count = 0;

#ifdef _PRE_WLAN_DFT_STAT
    /* 申请维测统计结构的内存 */
    pst_ps_structure->pst_psm_stats = (dmac_user_psm_stats_stru *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,
                                                                               OAL_SIZEOF(dmac_user_psm_stats_stru),
                                                                               OAL_TRUE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_ps_structure->pst_psm_stats))
    {
        OAM_ERROR_LOG1(0, OAM_SF_PWR, "{dmac_psm_user_ps_structure_init::alloc pst_ps_stats mem fail, size[%d].}", OAL_SIZEOF(dmac_user_psm_stats_stru));
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(pst_ps_structure->pst_psm_stats, OAL_SIZEOF(dmac_user_psm_stats_stru));
#endif

    return OAL_SUCC;
}



oal_uint8 dmac_psm_pkt_need_buff(mac_device_stru *pst_mac_device, dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_net_buf)
{
    mac_tx_ctl_stru                *pst_tx_ctrl;
    mac_ieee80211_frame_stru       *pst_mac_header;
    oal_uint8                       uc_mgmt_type;
    oal_uint8                       uc_mgmt_subtype;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* p2p noa节能也需要缓存,原因是节能回退IV号乱序导致丢包 */
    if ((!IS_AP(&pst_dmac_vap->st_vap_base_info)) && (!IS_P2P_CL(&pst_dmac_vap->st_vap_base_info)))
#else
    if (!IS_AP(&pst_dmac_vap->st_vap_base_info))
#endif
    {
        return OAL_FALSE;
    }

    pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);

    if (OAL_TRUE == MAC_GET_CB_IS_FROM_PS_QUEUE(pst_tx_ctrl))
    {
        return OAL_FALSE;
    }

    if (OAL_FALSE == MAC_GET_CB_IS_MCAST(pst_tx_ctrl))
    {
        /* 如果是单播帧，则有两种情况下该帧不需要缓存:
           1、用户不节能
           2、1102不处于noa节能状态 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* 需要考虑P2P GO */
        if(IS_AP(&pst_dmac_vap->st_vap_base_info))
        {
            if ((OAL_FALSE == pst_dmac_user->bit_ps_mode) && (OAL_FALSE == pst_mac_device->st_p2p_info.en_p2p_ps_pause))
            {
                return OAL_FALSE;
            }
        }
        /* P2P CL的noa节能也需要缓存*/
        else if(IS_P2P_CL(&pst_dmac_vap->st_vap_base_info) && (OAL_FALSE == pst_mac_device->st_p2p_info.en_p2p_ps_pause))
        {
                return OAL_FALSE;
        }
#else
        if (OAL_FALSE == pst_dmac_user->bit_ps_mode)
        {
            return OAL_FALSE;
        }
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
        /* 首先必须满足用户睡眠了 */
        if (IS_AP(&pst_dmac_vap->st_vap_base_info) && (OAL_TRUE == mac_frame_is_null_data(pst_net_buf)))
        {
            return OAL_TRUE;
        }
#endif
    }
    else
    {
        /* 如果是组播帧，则有三种情况下该帧不需要缓存:
           1、所有关联用户都不节能
           2、1102不处于noa节能状态 */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
        /* p2p noa节能也需要缓存*/
        if(IS_AP(&pst_dmac_vap->st_vap_base_info))
        {
            if ((0 == pst_dmac_vap->uc_ps_user_num) && (OAL_FALSE== pst_mac_device->st_p2p_info.en_p2p_ps_pause))
            {
                return OAL_FALSE;
            }
        }
        else if(IS_P2P_CL(&pst_dmac_vap->st_vap_base_info) && (OAL_FALSE == pst_mac_device->st_p2p_info.en_p2p_ps_pause))
        {
                return OAL_FALSE;
        }
#else
        if (0 == pst_dmac_vap->uc_ps_user_num)
        {
            return OAL_FALSE;
        }
#endif
    }

    /* ACS AP扫描发probe req时不需要节能 */
    pst_mac_header  = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_net_buf);
    uc_mgmt_type    = mac_frame_get_type_value((oal_uint8 *)pst_mac_header);
    uc_mgmt_subtype = mac_frame_get_subtype_value((oal_uint8 *)pst_mac_header);

    if (WLAN_WME_AC_MGMT == MAC_GET_CB_WME_AC_TYPE(pst_tx_ctrl) && WLAN_MANAGEMENT == uc_mgmt_type && WLAN_PROBE_REQ == uc_mgmt_subtype)
    {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}



oal_uint32 dmac_psm_ps_enqueue(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_net_buf)
{
    oal_netbuf_stru        *pst_next_net_buf     = OAL_PTR_NULL;
    mac_tx_ctl_stru        *pst_tx_ctrl          = OAL_PTR_NULL;
    oal_int32               l_ps_mpdu_num        = 0;
#ifdef _PRE_WLAN_DFT_STAT
    dmac_user_psm_stats_stru *pst_psm_stats;
#endif

    pst_tx_ctrl = (mac_tx_ctl_stru *)OAL_NETBUF_CB(pst_net_buf);

    /* APUT需要设置bitmap的条件:1.单播数据但用户节能 2.组播数据并且至少有1个用户节能 */
    if(IS_AP(&pst_dmac_vap->st_vap_base_info) &&
        (((pst_dmac_user->bit_ps_mode == OAL_TRUE)&&(OAL_FALSE == MAC_GET_CB_IS_MCAST(pst_tx_ctrl)))
         ||((OAL_TRUE == MAC_GET_CB_IS_MCAST(pst_tx_ctrl))&&(0 != pst_dmac_vap->uc_ps_user_num))))
    {
        dmac_psm_update_beacon(pst_dmac_vap, pst_dmac_user);
    }

    /* 对节能队列进行操作，加锁保护 */
    oal_spin_lock(&pst_dmac_user->st_ps_structure.st_lock_ps);

    /* 更新节能队列中mpdu的个数 */
    l_ps_mpdu_num = oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num);
    l_ps_mpdu_num += MAC_GET_CB_MPDU_NUM(pst_tx_ctrl);
    oal_atomic_set(&pst_dmac_user->st_ps_structure.uc_mpdu_num, l_ps_mpdu_num);

    /* 将所有netbuf都挂到节能队列尾部 */
    while (OAL_PTR_NULL != pst_net_buf)
    {
        pst_next_net_buf = oal_get_netbuf_next(pst_net_buf);
        oal_netbuf_add_to_list_tail(pst_net_buf, &pst_dmac_user->st_ps_structure.st_ps_queue_head);
        pst_net_buf = pst_next_net_buf;
    }

    oal_spin_unlock(&pst_dmac_user->st_ps_structure.st_lock_ps);

#ifdef _PRE_WLAN_DFT_STAT
    pst_psm_stats = pst_dmac_user->st_ps_structure.pst_psm_stats;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_psm_stats))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PWR, "{dmac_psm_enqueue::psm_stats is null.}");
        return OAL_SUCC;
    }
    pst_psm_stats->ul_psm_enqueue_succ_cnt     += MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl);
#endif

    OAM_INFO_LOG3(0, OAM_SF_PWR, "{dmac_psm_enqueue::user[%d] enqueue %d & total %d.}",
                  pst_dmac_user->st_user_base_info.us_assoc_id, MAC_GET_CB_NETBUF_NUM(pst_tx_ctrl), l_ps_mpdu_num);

    return OAL_SUCC;
}

oal_uint32  dmac_psm_tx_set_more_data(dmac_user_stru *pst_dmac_user,
                                               mac_tx_ctl_stru *pst_tx_cb)
{
    oal_int32               l_ps_mpdu_num;
    oal_bool_enum_uint8     en_tid_empty;

    if ((OAL_TRUE == pst_dmac_user->bit_ps_mode)
        && (OAL_TRUE == pst_dmac_user->st_ps_structure.en_is_pspoll_rsp_processing))
    {
        l_ps_mpdu_num = oal_atomic_read(&pst_dmac_user->st_ps_structure.uc_mpdu_num);
        en_tid_empty  = dmac_psm_is_tid_empty(pst_dmac_user);

        if ((0 != l_ps_mpdu_num) || (OAL_FALSE == en_tid_empty))
        {
            MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_cb)->st_frame_control.bit_more_data = 0x01;
        }
    }

    return OAL_SUCC;
}


oal_void dmac_psm_update_beacon(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user)
{
    /* 更新用户的pvb */
    dmac_psm_set_local_bitmap(pst_dmac_vap, pst_dmac_user, 1);
#ifdef _PRE_WLAN_MAC_BUGFIX_MCAST_HW_Q
    dmac_encap_beacon(pst_dmac_vap, pst_dmac_vap->pauc_beacon_buffer[pst_dmac_vap->uc_beacon_idx], &(pst_dmac_vap->us_beacon_len));
#endif
}

oal_void dmac_psm_set_ucast_mgmt_tx_rate(dmac_vap_stru *pst_dmac_vap,
                                                    wlan_channel_band_enum_uint8 en_band,
                                                    oal_uint8 uc_legacy_rate,
                                                    wlan_phy_protocol_enum_uint8 en_protocol_mode)
{
    /* 参数合法性检查 */
    if ((OAL_PTR_NULL == pst_dmac_vap) || (en_band >= WLAN_BAND_BUTT))
    {
        OAM_ERROR_LOG2(0, OAM_SF_PWR, "{dmac_psm_set_ucast_mgmt_tx_rate::input param error, pst_dmac_vap[%p], band[%d].}",
                       pst_dmac_vap, en_band);
        return;
    }
    /* 只需要设置0级速率,1 2 3还是原来的值,发不成功硬件可以降速发送null帧 */
    pst_dmac_vap->ast_tx_mgmt_ucast[en_band].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_legacy_rate   = uc_legacy_rate;
    pst_dmac_vap->ast_tx_mgmt_ucast[en_band].ast_per_rate[0].rate_bit_stru.un_nss_rate.st_legacy_rate.bit_protocol_mode = en_protocol_mode;
    return;
}

oal_void  dmac_change_null_data_rate(dmac_vap_stru *pst_dmac_vap,dmac_user_stru *pst_dmac_user,oal_uint8 *uc_protocol_mode,oal_uint8 *uc_legacy_rate)
{
    switch (pst_dmac_user->st_user_base_info.en_avail_protocol_mode)
    {
        /* 11b 1M */
        case WLAN_LEGACY_11B_MODE:
            if (WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)
            {
                *uc_protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
                *uc_legacy_rate   = 0x1;
#ifdef _PRE_WLAN_FEATURE_P2P
                /* P2P 设备接收到管理帧 */
                if (!IS_LEGACY_VAP((&pst_dmac_vap->st_vap_base_info)))
                {
                    *uc_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
                    *uc_legacy_rate   = 0xB;
                }
#endif
            }
            else
            {
                *uc_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
                *uc_legacy_rate   = 0xB;
            }
            break;

        /* OFDM 6M */
        case WLAN_MIXED_ONE_11G_MODE:
        case WLAN_HT_MODE:
        case WLAN_LEGACY_11A_MODE:
        case WLAN_LEGACY_11G_MODE:
        case WLAN_MIXED_TWO_11G_MODE:
        case WLAN_VHT_MODE:
        case WLAN_HT_11G_MODE:
            *uc_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            *uc_legacy_rate   = 0xB;
            break;

        /* OFDM 24M */
        case WLAN_HT_ONLY_MODE:
        case WLAN_VHT_ONLY_MODE:
            *uc_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            *uc_legacy_rate   = 0x9;
            break;

        default:
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX,
                             "{dmac_change_null_data_rate::invalid en_protocol[%d].}", pst_dmac_user->st_user_base_info.en_avail_protocol_mode);
            return;
    }


}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


