
#ifdef __cplusplus
#if    __cplusplus
extern "C" {
#endif
#endif

#ifdef    _PRE_WLAN_FEATURE_PACKET_CAPTURE
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_pkt_capture.h"
#include "dmac_alg.h"
//#include "dmac_main.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_PKT_CAPTURE_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/



/*****************************************************************************
  3 函数实现
*****************************************************************************/


OAL_STATIC  oal_uint16  dmac_encap_ctl_rts(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_mac,
                               oal_uint32 ul_data_rate, oal_uint32 ul_cts_rate, oal_uint32 ul_ack_rate)
{
    oal_uint8       *puc_origin = puc_buffer;


    /*************************************************************************/
    /*                        control Frame Format                           */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration  |RA    |TA      |FCS|                          */
    /* --------------------------------------------------------------------  */
    /* | 2           |2         |6     |6       |4    |                        */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                        设置帧头                                      */
    /*************************************************************************/
    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(puc_buffer, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_CTL| WLAN_FC0_SUBTYPE_RTS);

    /* 设置duration=ack的时长+cts的时长+数据帧的时长+3*SIFS时长 */
    *(oal_uint16 *)(puc_buffer + 2) = (oal_uint16)(((100<<3) * 1000)/ul_data_rate + DMAC_PKT_CAP_CTS_ACK_TIME3 + DMAC_PKT_CAP_CTS_ACK_TIME3 + 3 * SIFSTIME);

    /* 设置地址1 */
    oal_set_mac_addr(puc_buffer + WLAN_HDR_ADDR1_OFFSET, puc_mac);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(puc_buffer + WLAN_HDR_ADDR2_OFFSET, pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    puc_buffer += MAC_80211_CTL_HEADER_LEN;

    return (oal_uint16)(puc_buffer - puc_origin);
}


OAL_STATIC  oal_uint16  dmac_encap_ctl_cts(oal_uint8 *puc_buffer, oal_uint8 *puc_mac,
                                           oal_uint16 us_rts_duration, oal_uint32 ul_cts_rate)
{
    oal_uint8       *puc_origin = puc_buffer;
    oal_uint8        uc_cts_time;

    /*************************************************************************/
    /*                        control Frame Format                           */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration  |RA    |FCS|                          */
    /* --------------------------------------------------------------------  */
    /* | 2           |2         |6     |4  |                        */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                        设置帧头                                      */
    /*************************************************************************/

    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(puc_buffer, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_CTL| WLAN_FC0_SUBTYPE_CTS);

    /* 根据cts的速率, 设置duration */
    if (ul_cts_rate >= DMAC_PKT_CAP_SIFS_RATE3)
    {
        uc_cts_time = DMAC_PKT_CAP_CTS_ACK_TIME3;
    }
    else if (ul_cts_rate >= DMAC_PKT_CAP_SIFS_RATE2 && ul_cts_rate < DMAC_PKT_CAP_SIFS_RATE3)
    {
        uc_cts_time = DMAC_PKT_CAP_CTS_ACK_TIME2;
    }
    else
    {
        uc_cts_time = DMAC_PKT_CAP_CTS_ACK_TIME1;
    }
    *(oal_uint16 *)(puc_buffer + 2) = (oal_uint16)(us_rts_duration - SIFSTIME - uc_cts_time);

    /* 设置地址1 */
    oal_set_mac_addr(puc_buffer + WLAN_HDR_ADDR1_OFFSET, puc_mac);

    puc_buffer += (ACK_CTS_FRAME_LEN - WLAN_HDR_FCS_LENGTH);

    return (oal_uint16)(puc_buffer - puc_origin);
}


OAL_STATIC  oal_uint16  dmac_encap_ctl_ack(oal_uint8 *puc_buffer, oal_uint8 *puc_mac)
{
    oal_uint8       *puc_origin = puc_buffer;

    /*************************************************************************/
    /*                        control Frame Format                           */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration  |RA    |FCS|                          */
    /* --------------------------------------------------------------------  */
    /* | 2           |2         |6     |4  |                        */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                        设置帧头                                      */
    /*************************************************************************/

    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(puc_buffer, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_CTL| WLAN_FC0_SUBTYPE_ACK);

    /* 设置duration为0 */
    *(oal_uint16 *)(puc_buffer + 2) = (oal_uint16)0x00;

    /* 设置地址1 */
    oal_set_mac_addr(puc_buffer + WLAN_HDR_ADDR1_OFFSET, puc_mac);

    puc_buffer += (ACK_CTS_FRAME_LEN - WLAN_HDR_FCS_LENGTH);

    return (oal_uint16)(puc_buffer - puc_origin);
}



OAL_STATIC  oal_uint16  dmac_encap_ctl_ba(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_buff, oal_uint8 *puc_mac, oal_uint8 uc_lut_index)
{
    oal_uint16               us_ba_ctl = 0;
    oal_uint32               ul_addr_h;
    oal_uint32               ul_addr_l;
    oal_uint32               ul_ba_para;
    oal_uint32               ul_bitmap_h;
    oal_uint32               ul_bitmap_l;
    oal_uint32               ul_winstart ;
    hal_to_dmac_device_stru *pst_hal_device;
    dmac_device_stru        *pst_dmac_dev;

    pst_dmac_dev = dmac_res_get_mac_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_dev)
    {
        OAM_ERROR_LOG0(0, OAM_SF_FRAME_FILTER, "{dmac_encap_ba::pst_dmac_dev is null}\r\n");
        return 0;
    }
    pst_hal_device = pst_dmac_dev->past_hal_device[0];
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_FRAME_FILTER, "{dmac_encap_ba::pst_hal_device is null}\r\n");
        return 0;
    }

    /*************************************************************************/
    /*                     BlockAck  Frame Format                             */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|RA|TA|BA Control|BA Info              |FCS|  */
    /* |             |        |  |  |          |                     |   |  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |2         |var                  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /* 设置subtype为BA */
    mac_hdr_set_frame_control(puc_buff, (oal_uint16)WLAN_PROTOCOL_VERSION | WLAN_FC0_TYPE_CTL | WLAN_FC0_SUBTYPE_BA);

    /* 设置duration为0 */
    *(oal_uint16 *)(puc_buff + 2) = (oal_uint16)0x00;

    /* 设置RA */
    oal_set_mac_addr(puc_buff + WLAN_HDR_ADDR1_OFFSET, puc_mac);

    /* 设置TA */
    oal_set_mac_addr(puc_buff + WLAN_HDR_ADDR2_OFFSET, pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    /* BA Control field */
    us_ba_ctl = (oal_uint16)(0 << 12);
    /* BA Policy is set to Normal Ack */
    us_ba_ctl &= ~BIT0;
    /* 非multi tid */
    us_ba_ctl &= ~BIT1;
    /* compressed bitmap */
    us_ba_ctl |= BIT2;
    /* 设置BA Control */
    puc_buff[16] = us_ba_ctl & 0xFF;
    puc_buff[17] = (us_ba_ctl >> 8) & 0xFF;

    /* 获取硬件填的ba参数 */
    hal_get_machw_ba_params(pst_hal_device,
                            uc_lut_index,
                            &ul_addr_h,
                            &ul_addr_l,
                            &ul_bitmap_h,
                            &ul_bitmap_l,
                            &ul_ba_para);

    /* 获取并设置ssn */
    ul_winstart  = (ul_ba_para & 0x0000FFF0);
    *(oal_uint16 *)(puc_buff + 18) = (oal_uint16)ul_winstart;

    /* 获取并设置bitmap */
    puc_buff[20] = ul_bitmap_l & 0xFF;
    puc_buff[21] = (ul_bitmap_l >> 8) & 0xFF;
    puc_buff[22] = (ul_bitmap_l >> 16) & 0xFF;
    puc_buff[23] = (ul_bitmap_l >> 24) & 0xFF;
    puc_buff[24] = ul_bitmap_h & 0xFF;
    puc_buff[25] = (ul_bitmap_h >> 8) & 0xFF;
    puc_buff[26] = (ul_bitmap_h >> 16) & 0xFF;
    puc_buff[27] = (ul_bitmap_h >> 24) & 0xFF;

    /* BA control占2个字节, BA info占2+8总共10个字节 */
    return MAC_80211_CTL_HEADER_LEN + 2 + 10;
}


OAL_STATIC  oal_void  dmac_next_circle_buff(oal_uint16 *pus_circle_buf_index)
{
    /* 继续处理下一个循环buffer */
    if ((WLAN_PACKET_CAPTURE_CIRCLE_BUF_DEPTH - 1) == *(pus_circle_buf_index))
    {
        (*pus_circle_buf_index) = 0;
    }
    else
    {
        (*pus_circle_buf_index)++;
    }
}


OAL_STATIC  oal_void  dmac_fill_radiotap(ieee80211_radiotap_stru *pst_radiotap, oal_uint32 ul_data_rate, hal_statistic_stru *pst_per_rate,
                                         mac_vap_stru *pst_mac_vap, dmac_rx_ctl_stru *pst_cb_ctrl, oam_ota_frame_direction_type_enum_uint8 uc_direct, oal_int8 c_rssi_ampdu)
{
    oal_uint8                        uc_flags           = DMAC_IEEE80211_RADIOTAP_F_FCS;         /* FCS at end位必为1 */
    oal_uint8                        uc_data_rate       = 0;        /* data rate信息, 11ag和11b协议时该字段有效 */
    oal_uint16                       us_ch_freq;
    oal_uint16                       us_ch_type;
    oal_int8                         c_ssi_signal       = 0;
    oal_int8                         c_ssi_noise;
    oal_int16                        s_signal_quality;
    oal_uint8                        uc_mcs_info_known  = 0;        /* mcs信息, 11n协议时该字段有效 */
    oal_uint8                        uc_mcs_info_flags  = 0;
    oal_uint8                        uc_mcs_info_rate   = 0;
    oal_uint16                       us_vht_known       = 0;        /* vht信息, 11ac协议时该字段有效 */
    oal_uint8                        uc_vht_flags       = 0;
    oal_uint8                        uc_vht_bandwidth   = 0;
    oal_uint8                        uc_vht_mcs_nss[4]  = {0};
    oal_uint8                        uc_vht_coding = 0;
    oal_uint8                        uc_vht_group_id = 0;
    oal_uint16                       us_vht_partial_aid = 0;

    mac_ieee80211_frame_stru        *pst_mac_head       = OAL_PTR_NULL;
    mac_regclass_info_stru          *pst_regdom_info    = OAL_PTR_NULL;

    /* 填写fields字段里的flags成员 */
    pst_mac_head = (mac_ieee80211_frame_stru *)((oal_uint8 *)pst_radiotap + OAL_SIZEOF(ieee80211_radiotap_stru));
    if (1 == pst_per_rate->bit_preamble)
    {
        uc_flags = uc_flags | DMAC_IEEE80211_RADIOTAP_F_SHORTPRE;
    }
    if (1 == pst_mac_head->st_frame_control.bit_more_frag)
    {
        uc_flags = uc_flags | DMAC_IEEE80211_RADIOTAP_F_FRAG;
    }
    if (1 == pst_per_rate->uc_short_gi)
    {
        uc_flags = uc_flags | DMAC_IEEE80211_RADIOTAP_F_SHORTGI;
    }

    /* 填写fields字段中的其他成员ch_freq、ch_type、ssi_signal、ssi_noise、signal_quality */
    if (OAM_OTA_FRAME_DIRECTION_TYPE_TX == uc_direct)
    {
        pst_regdom_info = mac_get_channel_num_rc_info(pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_chan_number);
        /* 发送的SSI采用所在信道管制域规定的最大发送功率, noise统一为-95dBm */
        if (OAL_PTR_NULL != pst_regdom_info)
        {
            c_ssi_signal = (oal_int8)pst_regdom_info->uc_max_reg_tx_pwr;
        }
        c_ssi_noise  = DMAC_PKT_CAP_TX_NOISE;
        s_signal_quality = c_ssi_signal - DMAC_PKT_CAP_SIGNAL_OFFSET;

        /* 发送的信道直接通过mac vap的st_channel成员获得 */
        if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
        {
            us_ch_freq = 5 * pst_mac_vap->st_channel.uc_chan_number + WLAN_2G_CENTER_FREQ_BASE;
            us_ch_type = (oal_uint16)DMAC_IEEE80211_CHAN_2GHZ | (oal_uint16)DMAC_IEEE80211_CHAN_DYN;
        }
        else
        {
            us_ch_freq = 5 * pst_mac_vap->st_channel.uc_chan_number + WLAN_5G_CENTER_FREQ_BASE;
            us_ch_type = (oal_uint16)DMAC_IEEE80211_CHAN_5GHZ | (oal_uint16)DMAC_IEEE80211_CHAN_OFDM;
        }
    }
    else
    {
        if (HAL_RX_FCS_ERROR == pst_cb_ctrl->st_rx_status.bit_dscr_status)
        {
            /* CRC校验结果错误, BAD_FCS位置1 */
            uc_flags = uc_flags | DMAC_IEEE80211_RADIOTAP_F_BADFCS;
            /* 增加一个对FCS校验错误帧个数的统计 */
        }
        if (0 != pst_cb_ctrl->st_rx_statistic.c_rssi_dbm)
        {
            c_ssi_signal = pst_cb_ctrl->st_rx_statistic.c_rssi_dbm;
        }
        else
        {
            c_ssi_signal = c_rssi_ampdu;
        }
        /* snr_ant是以0.5dB为单位，实际使用前需要先除以2，且超出正常snr表示范围的用最小snr表示 */
        /*lint -e702*/
        c_ssi_noise = c_ssi_signal - (pst_cb_ctrl->st_rx_statistic.c_snr_ant0 >> 2) - (pst_cb_ctrl->st_rx_statistic.c_snr_ant1 >> 2);
        /*lint +e702*/
        if ((c_ssi_noise >= DMAC_PKT_CAP_NOISE_MAX) || (c_ssi_noise < DMAC_PKT_CAP_NOISE_MIN))
        {
            c_ssi_noise = DMAC_PKT_CAP_NOISE_MIN;
        }
        s_signal_quality = c_ssi_signal - DMAC_PKT_CAP_SIGNAL_OFFSET;

        /* 接收的信道需通过查找接收描述符里接收帧的信道获得 */
        if (pst_cb_ctrl->st_rx_info.uc_channel_number < 36)
        {
            us_ch_freq = 5 * pst_cb_ctrl->st_rx_info.uc_channel_number + WLAN_2G_CENTER_FREQ_BASE;
            us_ch_type = (oal_uint16)DMAC_IEEE80211_CHAN_2GHZ | (oal_uint16)DMAC_IEEE80211_CHAN_DYN;
        }
        else
        {
            us_ch_freq = 5 * pst_cb_ctrl->st_rx_info.uc_channel_number + WLAN_5G_CENTER_FREQ_BASE;
            us_ch_type = (oal_uint16)DMAC_IEEE80211_CHAN_5GHZ | (oal_uint16)DMAC_IEEE80211_CHAN_OFDM;
        }
    }

    /* 填写fields字段中的速率信息, 根据radiotap的要求, 11n时mcs_info有效、11ac时vht_info有效、11ag和11b时data_rate有效 */
    if (WLAN_HT_PHY_PROTOCOL_MODE == pst_per_rate->un_nss_rate.st_ht_rate.bit_protocol_mode)
    {
        uc_mcs_info_known = (oal_uint8)DMAC_IEEE80211_RADIOTAP_MCS_HAVE_BW | (oal_uint8)DMAC_IEEE80211_RADIOTAP_MCS_HAVE_MCS | (oal_uint8)DMAC_IEEE80211_RADIOTAP_MCS_HAVE_GI |
                            (oal_uint8)DMAC_IEEE80211_RADIOTAP_MCS_HAVE_FMT | (oal_uint8)DMAC_IEEE80211_RADIOTAP_MCS_HAVE_FEC;
        /* 描述符里BW只有20, 20L和20U, 并没有40M的选项 */
        if (WLAN_BAND_ASSEMBLE_20M != pst_per_rate->uc_bandwidth)
        {
            uc_mcs_info_flags = uc_mcs_info_flags | DMAC_IEEE80211_RADIOTAP_MCS_BW_40;
        }
        if (1 == pst_per_rate->uc_short_gi)
        {
            uc_mcs_info_flags = uc_mcs_info_flags | DMAC_IEEE80211_RADIOTAP_MCS_SGI;
        }
        if (1 == pst_per_rate->bit_preamble)
        {
            uc_mcs_info_flags = uc_mcs_info_flags | DMAC_IEEE80211_RADIOTAP_MCS_FMT_GF;
        }
        if (1 == pst_per_rate->bit_channel_code)
        {
            uc_mcs_info_flags = uc_mcs_info_flags | DMAC_IEEE80211_RADIOTAP_MCS_FEC_LDPC;
        }
        uc_mcs_info_flags = uc_mcs_info_flags | (pst_per_rate->bit_stbc << DMAC_IEEE80211_RADIOTAP_MCS_STBC_SHIFT);
        /* 填写对应的mcs速率信息 */
        uc_mcs_info_rate = pst_per_rate->un_nss_rate.st_ht_rate.bit_ht_mcs;
    }
    else if (WLAN_VHT_PHY_PROTOCOL_MODE == pst_per_rate->un_nss_rate.st_ht_rate.bit_protocol_mode)
    {
        us_vht_known = (oal_uint16)DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_STBC | (oal_uint16)DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_TXOP_PS_NA | (oal_uint16)DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_GI |
                       (oal_uint16)DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_BEAMFORMED | (oal_uint16)DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH | (oal_uint16)DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_GROUP_ID | (oal_uint16)DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_PARTIAL_AID;
        /* vht对应的flags信息, 包括STBC、Short GI等 */
        if (0 != pst_per_rate->bit_stbc)
        {
            uc_vht_flags = uc_vht_flags | DMAC_IEEE80211_RADIOTAP_VHT_FLAG_STBC;
        }
        if (1 == pst_per_rate->uc_short_gi)
        {
            uc_vht_flags = uc_vht_flags | DMAC_IEEE80211_RADIOTAP_VHT_FLAG_SGI;
        }
        /* 填写对应的vht带宽信息 */
        if (WLAN_BAND_ASSEMBLE_20M == pst_per_rate->uc_bandwidth)
        {
            uc_vht_bandwidth = uc_vht_bandwidth | DMAC_IEEE80211_RADIOTAP_VHT_BW_20;
        }
        else if (WLAN_BAND_ASSEMBLE_40M == pst_per_rate->uc_bandwidth || WLAN_BAND_ASSEMBLE_40M_DUP == pst_per_rate->uc_bandwidth)
        {
            uc_vht_bandwidth = uc_vht_bandwidth | DMAC_IEEE80211_RADIOTAP_VHT_BW_40;
        }
        else if (WLAN_BAND_ASSEMBLE_80M == pst_per_rate->uc_bandwidth || WLAN_BAND_ASSEMBLE_80M_DUP == pst_per_rate->uc_bandwidth)
        {
            uc_vht_bandwidth = uc_vht_bandwidth | DMAC_IEEE80211_RADIOTAP_VHT_BW_80;
        }
        else
        {
            uc_vht_bandwidth = uc_vht_bandwidth | DMAC_IEEE80211_RADIOTAP_VHT_BW_160;
        }
        /* 填写对应的vht速率信息、编码方式 */
        uc_vht_mcs_nss[0] = (pst_per_rate->un_nss_rate.st_vht_nss_mcs.bit_vht_mcs << 4) + (pst_per_rate->un_nss_rate.st_vht_nss_mcs.bit_nss_mode + 1);
        if (1 == pst_per_rate->bit_channel_code)
        {
            uc_vht_coding = uc_vht_coding | DMAC_IEEE80211_RADIOTAP_CODING_LDPC_USER0;
        }
    }
    else
    {
        uc_data_rate = (oal_uint8)(ul_data_rate / DMAC_PKT_CAP_RATE_UNIT);
    }

    pst_radiotap->st_radiotap_header.uc_it_version = PKTHDR_RADIOTAP_VERSION;
    pst_radiotap->st_radiotap_header.us_it_len     = OAL_SIZEOF(ieee80211_radiotap_stru);
    pst_radiotap->st_radiotap_header.uc_it_pad     = 0;
    pst_radiotap->st_radiotap_header.ul_it_present = (oal_uint32)DMAC_IEEE80211_RADIOTAP_TSFT | (oal_uint32)DMAC_IEEE80211_RADIOTAP_FLAGS | (oal_uint32)DMAC_IEEE80211_RADIOTAP_RATE | (oal_uint32)DMAC_IEEE80211_RADIOTAP_CHANNEL |
                    (oal_uint32)DMAC_IEEE80211_RADIOTAP_DBM_ANTSIGNAL | (oal_uint32)DMAC_IEEE80211_RADIOTAP_DBM_ANTNOISE | (oal_uint32)DMAC_IEEE80211_RADIOTAP_LOCK_QUALITY | (oal_uint32)DMAC_IEEE80211_RADIOTAP_MCS | (oal_uint32)DMAC_IEEE80211_RADIOTAP_VHT;

    pst_radiotap->st_radiotap_fields.uc_flags           = uc_flags;
    pst_radiotap->st_radiotap_fields.uc_data_rate       = uc_data_rate;
    pst_radiotap->st_radiotap_fields.us_channel_freq    = us_ch_freq;
    pst_radiotap->st_radiotap_fields.us_channel_type    = us_ch_type;
    pst_radiotap->st_radiotap_fields.c_ssi_signal       = c_ssi_signal;
    pst_radiotap->st_radiotap_fields.c_ssi_noise        = c_ssi_noise;
    pst_radiotap->st_radiotap_fields.s_signal_quality   = s_signal_quality;

    pst_radiotap->st_radiotap_fields.uc_mcs_info_known  = uc_mcs_info_known;
    pst_radiotap->st_radiotap_fields.uc_mcs_info_flags  = uc_mcs_info_flags;
    pst_radiotap->st_radiotap_fields.uc_mcs_info_rate   = uc_mcs_info_rate;

    pst_radiotap->st_radiotap_fields.us_vht_known       = us_vht_known;
    pst_radiotap->st_radiotap_fields.uc_vht_flags       = uc_vht_flags;
    pst_radiotap->st_radiotap_fields.uc_vht_bandwidth   = uc_vht_bandwidth;
    pst_radiotap->st_radiotap_fields.uc_vht_mcs_nss[0]  = uc_vht_mcs_nss[0];
    pst_radiotap->st_radiotap_fields.uc_vht_mcs_nss[1]  = uc_vht_mcs_nss[1];
    pst_radiotap->st_radiotap_fields.uc_vht_mcs_nss[2]  = uc_vht_mcs_nss[2];
    pst_radiotap->st_radiotap_fields.uc_vht_mcs_nss[3]  = uc_vht_mcs_nss[3];
    pst_radiotap->st_radiotap_fields.uc_vht_coding      = uc_vht_coding;
    pst_radiotap->st_radiotap_fields.uc_vht_group_id    = uc_vht_group_id;
    pst_radiotap->st_radiotap_fields.us_vht_partial_aid = us_vht_partial_aid;
}


OAL_STATIC  oal_void  dmac_report_packet(oal_netbuf_stru *pst_netbuf, oal_uint16 us_rt_len, oal_uint16 us_mac_header_len,
                                         oal_uint16 us_mac_body_len, oal_bool_enum_uint8 en_report_sdt_switch,
                                         oam_ota_frame_direction_type_enum_uint8 uc_direct)
{
    hw_ker_wifi_sniffer_packet_s    st_packet;

    if (OAL_TRUE == en_report_sdt_switch)
    {
        oam_report_80211_frame(BROADCAST_MACADDR, (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf) + us_rt_len, (oal_uint8)us_mac_header_len,
                               (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf) + us_rt_len + us_mac_header_len,(oal_uint16)(us_mac_header_len + us_mac_body_len),uc_direct);
        return;
    }
    st_packet.ul_manufacturerid  = 0;
    st_packet.puc_radiotapheader = (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf);
    st_packet.ul_rhlen           = (oal_uint16)us_rt_len;
    st_packet.puc_macheader      = (oal_uint8 *)(st_packet.puc_radiotapheader + st_packet.ul_rhlen);

    /* Frame */
    st_packet.ul_macheaderlen = us_mac_header_len;
    st_packet.puc_databuff    = (oal_uint8 *)(st_packet.puc_macheader + st_packet.ul_macheaderlen);
    st_packet.ul_datalen      = us_mac_body_len;

    oal_wifi_mirror_pkt(&st_packet);
}


oal_uint32 dmac_pkt_cap_tx(dmac_packet_stru *pst_dmac_packet, hal_to_dmac_device_stru *pst_hal_device)
{
    dmac_mem_circle_buf_stru        *pst_buf_current    = OAL_PTR_NULL;
    hal_tx_dscr_stru                *pst_current_dscr   = OAL_PTR_NULL;
    oal_netbuf_stru                 *pst_netbuf         = OAL_PTR_NULL;
    mac_tx_ctl_stru                 *pst_tx_ctl_cb      = OAL_PTR_NULL;
    mac_ieee80211_frame_stru        *pst_mac_frame_head = OAL_PTR_NULL;
    oal_netbuf_stru                 *pst_new_netbuf     = OAL_PTR_NULL;
    oal_uint16                       us_mac_header_len;
    oal_uint16                       us_mac_body_len;
    oal_uint32                       ul_mac_total_len;
    oal_uint8                        uc_vap_id;
    dmac_vap_stru                   *pst_dmac_vap       = OAL_PTR_NULL;
    ieee80211_radiotap_stru         *pst_radiotap       = OAL_PTR_NULL;
    hal_statistic_stru               st_per_rate;
    oal_uint32                       ul_data_rate;
    hal_tx_dscr_ctrl_one_param       st_rate_param;
    hal_tx_txop_rate_params_stru     st_phy_tx_mode;

    oal_uint8                        uc_tx_rate_rank;
    oal_uint8                        uc_tx_count;
    oal_uint8                        uc_tsf_offset;
    oal_uint32                       ul_seq;
    oal_uint8                        uc_ack_time;
    oal_uint16                       us_ba_time;
    oal_uint32                       ul_ret;

    hal_tx_dscr_stru                *pst_mpdu_dscr      = OAL_PTR_NULL;         /* 给AMPDU中非第一个MPDU使用的发送描述符 */
    oal_netbuf_stru                 *pst_mpdu_netbuf    = OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::pst_hal_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 从dmac_device中获取当前要处理的循环buffer地址 */
    pst_buf_current = (dmac_mem_circle_buf_stru *)((oal_uint8 *)pst_dmac_packet->pul_circle_buf_start + pst_dmac_packet->us_circle_buf_index*OAL_SIZEOF(dmac_mem_circle_buf_stru));

    while (OAL_PTR_NULL != pst_buf_current)
    {
        /* 获取循环buffer当前位置以及当前位置的描述符虚地址pst_current_dscr */
        pst_buf_current = (dmac_mem_circle_buf_stru *)((oal_uint8 *)pst_dmac_packet->pul_circle_buf_start + pst_dmac_packet->us_circle_buf_index*OAL_SIZEOF(dmac_mem_circle_buf_stru));

        /* 如果当前循环buffer的状态位无效, 退出循环 */
        if (0 == pst_buf_current->bit_status)
        {
            break;
        }
        if (0 == pst_buf_current->uc_timestamp)
        {
            /* 循环buffer状态位有效, 但出现时间戳为0的异常, 上报错误到sdt */
            OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::tx_timestamp equals zero.}");
        }
        if (OAL_PTR_NULL == pst_buf_current->pul_link_addr)
        {
            OAM_ERROR_LOG1(0, OAM_SF_PKT_CAP, "{pst_buf_current::status is valid, but the pul_link_addr is NULL. buffer idx[%d]}", pst_dmac_packet->us_circle_buf_index);
            /* 继续处理下一个循环buffer */
            dmac_next_circle_buff(&(pst_dmac_packet->us_circle_buf_index));
            pst_buf_current->bit_status = 0;
            continue;
        }

        /* 获取循环buffer对应的当前描述符、描述符对应的netbuf、netbuf对应的cb字段, 及cb对应的mac帧头 */
        pst_current_dscr = (hal_tx_dscr_stru *)OAL_DSCR_PHY_TO_VIRT((oal_uint32)pst_buf_current->pul_link_addr);
        if (OAL_PTR_NULL == pst_current_dscr || OAL_UNLIKELY(OAL_FALSE == pst_current_dscr->bit_is_first))
        {
            OAM_WARNING_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::pst_current_dscr is NULL, or is not the first mpdu of ppdu.}");
            /* 继续处理下一个循环buffer */
            dmac_next_circle_buff(&(pst_dmac_packet->us_circle_buf_index));
            pst_buf_current->bit_status = 0;
            continue;
        }
        pst_netbuf = pst_current_dscr->pst_skb_start_addr;
        if (OAL_PTR_NULL == pst_netbuf)
        {
            OAM_WARNING_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::pst_netbuf is NULL.}");
            /* 继续处理下一个循环buffer */
            dmac_next_circle_buff(&(pst_dmac_packet->us_circle_buf_index));
            pst_buf_current->bit_status = 0;
            continue;
        }
        pst_tx_ctl_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
        if (OAL_PTR_NULL == pst_tx_ctl_cb)
        {
            OAM_WARNING_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::pst_tx_ctl_cb is null.}");
            /* 继续处理下一个循环buffer */
            dmac_next_circle_buff(&(pst_dmac_packet->us_circle_buf_index));
            pst_buf_current->bit_status = 0;
            continue;
        }
        pst_mac_frame_head = pst_tx_ctl_cb->pst_frame_header;
        if (OAL_PTR_NULL == pst_mac_frame_head)
        {
            OAM_WARNING_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::pst_mac_frame_head is null.}");
            /* 继续处理下一个循环buffer */
            dmac_next_circle_buff(&(pst_dmac_packet->us_circle_buf_index));
            pst_buf_current->bit_status = 0;
            continue;
        }

        /* 通过hal层对应的描述符获取mac vap id, 再获得mac vap */
        dmac_tx_get_vap_id(pst_hal_device, pst_current_dscr, &uc_vap_id);
        //pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_vap_id);
        pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(uc_vap_id);
        if (OAL_PTR_NULL == pst_dmac_vap)
        {
            /* MAC VAP ID为0, 则是配置VAP, 无MIB项 */
            OAM_WARNING_LOG1(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::dmac vap[id %d] is null.}", uc_vap_id);
            /* 继续处理下一个循环buffer */
            dmac_next_circle_buff(&(pst_dmac_packet->us_circle_buf_index));
            continue;
        }

        /* 获取发送帧的速率, 需根据第几次发送找到对应速率等级 */
        hal_tx_get_dscr_phy_tx_mode_param(pst_current_dscr, &st_phy_tx_mode);
        hal_tx_get_dscr_ctrl_one_param(pst_hal_device, pst_current_dscr, &st_rate_param);

        uc_tx_count = pst_buf_current->bit_tx_cnt;
        /* 根据循环buffer记录当前帧的发送次数填充帧头的retry位为1 */
        if (uc_tx_count > 0)
        {
            pst_mac_frame_head->st_frame_control.bit_retry = 1;
        }
        for (uc_tx_rate_rank = 0; uc_tx_rate_rank < HAL_TX_RATE_MAX_NUM; uc_tx_rate_rank++)
        {
            if (uc_tx_count <= st_rate_param.ast_per_rate[uc_tx_rate_rank].rate_bit_stru.bit_tx_count)
            {
                break;
            }
            uc_tx_count -= st_rate_param.ast_per_rate[uc_tx_rate_rank].rate_bit_stru.bit_tx_count;
        }
        if (HAL_TX_RATE_MAX_NUM == uc_tx_rate_rank)
        {
            uc_tx_rate_rank = HAL_TX_RATE_MAX_NUM - 1;
        }
        st_per_rate.un_nss_rate.st_ht_rate.bit_ht_mcs        = st_rate_param.ast_per_rate[uc_tx_rate_rank].rate_bit_stru.un_nss_rate.st_ht_rate.bit_ht_mcs;
        st_per_rate.un_nss_rate.st_ht_rate.bit_protocol_mode = st_rate_param.ast_per_rate[uc_tx_rate_rank].rate_bit_stru.un_nss_rate.st_ht_rate.bit_protocol_mode;
        st_per_rate.uc_short_gi      = st_rate_param.ast_per_rate[uc_tx_rate_rank].rate_bit_stru.bit_short_gi_enable;
        st_per_rate.bit_preamble     = st_rate_param.ast_per_rate[uc_tx_rate_rank].rate_bit_stru.bit_preamble_mode;
        st_per_rate.bit_stbc         = st_rate_param.ast_per_rate[uc_tx_rate_rank].rate_bit_stru.bit_stbc_mode;
        st_per_rate.bit_channel_code = st_phy_tx_mode.en_channel_code;
        st_per_rate.uc_bandwidth     = st_phy_tx_mode.en_channel_bandwidth;
        ul_ret = dmac_alg_get_rate_kbps(pst_hal_device, &st_per_rate, &ul_data_rate);
        if (OAL_ERR_CODE_PTR_NULL == ul_ret)
        {
            OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::get rate failed.}");
            /* 若获取速率失败, 2G默认速率为1Mbps, 5G默认速率为6Mbps */
            if (WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)
            {
                ul_data_rate = DMAC_PKT_CAP_2G_RATE;
            }
            else
            {
                ul_data_rate = DMAC_PKT_CAP_5G_RATE;
            }
        }

        /* 判断是否是RTS, 0表示RTS, pst_new_netbuf用来存上报的netbuf */
        if (0 == pst_buf_current->bit_frm_type)
        {
            /* 组RTS帧, 组帧顺序按照radiotap、帧头、payload */
            pst_new_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, OAL_SIZEOF(ieee80211_radiotap_stru) + MAC_80211_CTL_HEADER_LEN, OAL_NETBUF_PRIORITY_MID);
            if (OAL_PTR_NULL == pst_new_netbuf)
            {
                OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::pst_new_netbuf to request memory failed.}");
                /* 继续处理下一个循环buffer */
                dmac_next_circle_buff(&(pst_dmac_packet->us_circle_buf_index));
                pst_buf_current->bit_status = 0;
                continue;
            }
            OAL_MEMZERO((oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf), OAL_SIZEOF(ieee80211_radiotap_stru));
            /* 若mac vap为ERP保护模式, 则前三级发送速率为5.5Mbps, 最后一级为1Mbps, 否则前三级为24Mbps, 最后一级为6Mbps */
            if ((HAL_TX_RATE_MAX_NUM - 1) == uc_tx_rate_rank)
            {
                if(WLAN_PROT_ERP == pst_dmac_vap->st_vap_base_info.st_protection.en_protection_mode)
                {
                    ul_data_rate = DMAC_PKT_CAP_2G_RATE;
                }
                else
                {
                    ul_data_rate = DMAC_PKT_CAP_5G_RATE;
                }
            }
            else
            {
                if(WLAN_PROT_ERP == pst_dmac_vap->st_vap_base_info.st_protection.en_protection_mode)
                {
                    ul_data_rate = (oal_uint32)(5.5 * DMAC_PKT_CAP_2G_RATE);
                }
                else
                {
                    ul_data_rate = 4 * DMAC_PKT_CAP_5G_RATE;
                }
            }
            /* 统计MAC帧头和帧体的长度 */
            us_mac_header_len = dmac_encap_ctl_rts(&pst_dmac_vap->st_vap_base_info, (oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru), pst_mac_frame_head->auc_address1, ul_data_rate, ul_data_rate, ul_data_rate);
            us_mac_body_len   = 0;
            ul_mac_total_len  = us_mac_header_len + us_mac_body_len;
            /* 若是驱动发的RTS帧, 修改协议模式为11ag */
            st_per_rate.un_nss_rate.st_legacy_rate.bit_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
        }
        else
        {
            /* 统计MAC帧头和帧体的长度 */
            us_mac_header_len = pst_tx_ctl_cb->uc_frame_header_length;
            us_mac_body_len   = pst_tx_ctl_cb->us_mpdu_payload_len;
            ul_mac_total_len  = us_mac_header_len + us_mac_body_len;
            pst_new_netbuf    = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, OAL_SIZEOF(ieee80211_radiotap_stru) + ul_mac_total_len, OAL_NETBUF_PRIORITY_MID);
            if (OAL_PTR_NULL == pst_new_netbuf)
            {
                OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::pst_new_netbuf to request memory failed.}");
                /* 继续处理下一个循环buffer */
                dmac_next_circle_buff(&(pst_dmac_packet->us_circle_buf_index));
                pst_buf_current->bit_status = 0;
                continue;
            }
            OAL_MEMZERO((oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf), OAL_SIZEOF(ieee80211_radiotap_stru));
            /* 判断该帧是否为数据帧 */
            if (MAC_GET_CB_IS_DATA_FRAME(pst_tx_ctl_cb))
            {
                /* 将数据帧拷贝至netbuf时, 需要从帧头位置拷贝起, 而不是netbuf中的data指针 */
                oal_memcopy((oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru), (oal_uint8 *)pst_mac_frame_head, us_mac_header_len);
                oal_memcopy((oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru) + us_mac_header_len, (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf), us_mac_body_len);
                pst_mac_frame_head = (mac_ieee80211_frame_stru *)((oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru));
                /* 非QOS数据帧seq num也是由硬件填写, 需读寄存器去更新seq num */
                if (OAL_TRUE != (oal_bool_enum_uint8)MAC_GET_CB_IS_QOS_DATA(pst_tx_ctl_cb))
                {
                    hal_get_tx_sequence_num(pst_dmac_vap->pst_hal_device, 0, 0, 0, 0,&ul_seq);
                    if (0 == ul_seq)
                    {
                        pst_mac_frame_head->bit_seq_num = 4095;
                    }
                    pst_mac_frame_head->bit_seq_num = ul_seq - 1;
                }
            }
            else
            {
                /* 将管理帧所指向的netbuf的复制到新申请的netbuf中, 并更新seq num */
                oal_memcopy((oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru), OAL_NETBUF_DATA(pst_netbuf), ul_mac_total_len);
                hal_get_tx_sequence_num(pst_dmac_vap->pst_hal_device, 0, 0, 0, 0,&ul_seq);
                pst_mac_frame_head = (mac_ieee80211_frame_stru *)((oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru));
                if (0 == ul_seq)
                {
                    pst_mac_frame_head->bit_seq_num = 4095;
                }
                pst_mac_frame_head->bit_seq_num = ul_seq - 1;
            }
            /* 补充tx帧的duration字段 */
            if (ul_data_rate >= DMAC_PKT_CAP_SIFS_RATE3)
            {
                uc_ack_time = DMAC_PKT_CAP_CTS_ACK_TIME3;
            }
            else if (ul_data_rate >= DMAC_PKT_CAP_SIFS_RATE2 && ul_data_rate < DMAC_PKT_CAP_SIFS_RATE3)
            {
                uc_ack_time = DMAC_PKT_CAP_CTS_ACK_TIME2;
            }
            else
            {
                uc_ack_time = DMAC_PKT_CAP_CTS_ACK_TIME1;
            }
            *(oal_uint16 *)((oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru) + 2) = SIFSTIME + uc_ack_time;
        }
        /* 将其他信息写入ieee80211_radiotap_stru */
        pst_radiotap = (ieee80211_radiotap_stru *)OAL_NETBUF_DATA(pst_new_netbuf);
        dmac_fill_radiotap(pst_radiotap, ul_data_rate, &st_per_rate, &pst_dmac_vap->st_vap_base_info, OAL_PTR_NULL, OAM_OTA_FRAME_DIRECTION_TYPE_TX, 0);
        pst_radiotap->st_radiotap_fields.ull_timestamp = (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[7] << 56) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[6] << 48) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[5] << 40) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[4] << 32) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[3] << 24) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[2] << 16) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[1] << 8) +
                                              (oal_uint64)pst_buf_current->uc_timestamp[0];
        dmac_report_packet(pst_new_netbuf, (oal_uint16)OAL_SIZEOF(ieee80211_radiotap_stru), (oal_uint16)us_mac_header_len, (oal_uint16)us_mac_body_len,
                           pst_dmac_packet->en_report_sdt_switch, OAM_OTA_FRAME_DIRECTION_TYPE_TX);
        /* 统计上报的帧个数 */
        pst_dmac_packet->ul_total_report_pkt_num++;

        /* 判断是否为AMPDU, 是则需要补充完整之后的MPDU帧, 且需要保证此时不是从RTS的tx描述符取的 */
        if (1 == pst_current_dscr->bit_is_ampdu && 0 != pst_buf_current->bit_frm_type)
        {
            hal_get_tx_dscr_next(pst_hal_device, pst_current_dscr, &pst_mpdu_dscr);
            uc_tsf_offset = 1;
            while (OAL_PTR_NULL != pst_mpdu_dscr)
            {
                pst_netbuf = pst_mpdu_dscr->pst_skb_start_addr;
                if (OAL_PTR_NULL == pst_netbuf)
                {
                    OAM_WARNING_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::pst_netbuf_ampdu is null.}");
                    oam_report_dscr(BROADCAST_MACADDR, (oal_uint8 *)pst_mpdu_dscr, (oal_uint16)WLAN_MEM_SHARED_TX_DSCR_SIZE1, OAM_OTA_TX_DSCR_TYPE);
                    break;
                }
                pst_tx_ctl_cb = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
                pst_mac_frame_head = pst_tx_ctl_cb->pst_frame_header;

                us_mac_header_len = pst_tx_ctl_cb->uc_frame_header_length;
                us_mac_body_len   = pst_tx_ctl_cb->us_mpdu_payload_len;
                ul_mac_total_len  = us_mac_header_len + us_mac_body_len;
                pst_mpdu_netbuf    = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, OAL_SIZEOF(ieee80211_radiotap_stru) + ul_mac_total_len, OAL_NETBUF_PRIORITY_MID);
                if (OAL_PTR_NULL == pst_mpdu_netbuf)
                {
                    OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_tx::pst_new_netbuf to request memory failed.}");
                    break;
                }
                OAL_MEMZERO((oal_uint8 *)OAL_NETBUF_DATA(pst_mpdu_netbuf), OAL_SIZEOF(ieee80211_radiotap_stru));
                /* 如果AMPDU里第一个MPDU为重传, 那之后的MPDU均为重传帧 */
                if (pst_buf_current->bit_tx_cnt > 0)
                {
                    pst_mac_frame_head->st_frame_control.bit_retry = 1;
                }
                /* 将数据帧拷贝至netbuf时, 需要从帧头位置拷贝起, 而不是netbuf中的data指针 */
                oal_memcopy((oal_uint8 *)OAL_NETBUF_DATA(pst_mpdu_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru), (oal_uint8 *)pst_mac_frame_head, us_mac_header_len);
                oal_memcopy((oal_uint8 *)OAL_NETBUF_DATA(pst_mpdu_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru) + us_mac_header_len, (oal_uint8 *)OAL_NETBUF_DATA(pst_netbuf), us_mac_body_len);
                /* 补充tx帧的duration字段 */
                if (ul_data_rate >= DMAC_PKT_CAP_SIFS_RATE3)
                {
                    us_ba_time = DMAC_PKT_CAP_BA_TIME3;
                }
                else if (ul_data_rate >= DMAC_PKT_CAP_SIFS_RATE2 && ul_data_rate < DMAC_PKT_CAP_SIFS_RATE3)
                {
                    us_ba_time = DMAC_PKT_CAP_BA_TIME2;
                }
                else
                {
                    us_ba_time = DMAC_PKT_CAP_BA_TIME1;
                }
                pst_mac_frame_head->bit_duration_value = SIFSTIME + us_ba_time;
                /* 将其他信息写入ieee80211_radiotap_stru */
                pst_radiotap = (ieee80211_radiotap_stru *)OAL_NETBUF_DATA(pst_mpdu_netbuf);
                dmac_fill_radiotap(pst_radiotap, ul_data_rate, &st_per_rate, &pst_dmac_vap->st_vap_base_info, OAL_PTR_NULL, OAM_OTA_FRAME_DIRECTION_TYPE_TX, 0);
                pst_radiotap->st_radiotap_fields.ull_timestamp = (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[7] << 56) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[6] << 48) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[5] << 40) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[4] << 32) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[3] << 24) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[2] << 16) +
                                             (oal_uint64)((oal_uint64)pst_buf_current->uc_timestamp[1] << 8) +
                                              (oal_uint64)pst_buf_current->uc_timestamp[0] + (oal_uint64)uc_tsf_offset;
                dmac_report_packet(pst_mpdu_netbuf, (oal_uint16)OAL_SIZEOF(ieee80211_radiotap_stru), (oal_uint16)us_mac_header_len, (oal_uint16)us_mac_body_len,
                           pst_dmac_packet->en_report_sdt_switch, OAM_OTA_FRAME_DIRECTION_TYPE_TX);
                /* 统计上报的帧个数 */
                pst_dmac_packet->ul_total_report_pkt_num++;

                oal_netbuf_free(pst_mpdu_netbuf);
                hal_get_tx_dscr_next(pst_hal_device, pst_mpdu_dscr, &pst_mpdu_dscr);
                uc_tsf_offset++;
            }
        }

        /* 处理完当前有效buffer之后，清空当前buffer状态位 */
        pst_buf_current->bit_status = 0;
        oal_netbuf_free(pst_new_netbuf);

        /* 继续处理下一个循环buffer */
        dmac_next_circle_buff(&(pst_dmac_packet->us_circle_buf_index));
    }
    return OAL_SUCC;
}


oal_uint32  dmac_pkt_cap_rx(dmac_rx_ctl_stru *pst_cb_ctrl, oal_netbuf_stru *pst_netbuf, dmac_packet_stru *pst_dmac_packet, hal_to_dmac_device_stru *pst_hal_device, oal_int8 c_rssi_ampdu)
{
    oal_netbuf_stru               *pst_new_netbuf       = OAL_PTR_NULL;     /* 保存接收帧的netbuf */
    ieee80211_radiotap_stru       *pst_radiotap         = OAL_PTR_NULL;
    oal_netbuf_stru               *pst_make_netbuf      = OAL_PTR_NULL;     /* 保存构造帧的netbuf */
    ieee80211_radiotap_stru       *pst_make_radiotap    = OAL_PTR_NULL;
    mac_ieee80211_frame_stru      *pst_mac_frame_head   = OAL_PTR_NULL;
    oal_uint8                      uc_ack_len;
    oal_uint8                      uc_cts_len;
    oal_uint16                     us_frame_len;
    oal_uint8                     *puc_vap_mac_addr     = OAL_PTR_NULL;
    mac_vap_stru                  *pst_mac_vap          = OAL_PTR_NULL;
    hal_statistic_stru             st_per_rate;
    oal_uint32                     ul_rate_kbps;                            /* 接收帧的速率 */
    oal_uint32                     ul_ack_rate;                             /* 构造的ACK的速率 */
    hal_to_dmac_vap_stru          *pst_hal_vap          = OAL_PTR_NULL;
    oal_uint32                     ul_ret;
    oal_uint8                      uc_mac_vap_id;
    oal_uint16                     us_rts_duration;

    if ((OAL_PTR_NULL == pst_cb_ctrl) || (OAL_PTR_NULL == pst_netbuf) || (OAL_PTR_NULL == pst_dmac_packet) || (OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_rx::input argument detect failed.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 获取当前描述符对应的帧的帧长 */
    us_frame_len = pst_cb_ctrl->st_rx_info.us_frame_len;
    /* 申请pst_new_netbuf存储接收帧的radiotap和帧内容 */
    pst_new_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, OAL_SIZEOF(ieee80211_radiotap_stru) + us_frame_len, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_new_netbuf)
    {
        OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_rx::pst_new_netbuf request memory failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    OAL_MEMZERO((oal_uint8 *)OAL_NETBUF_DATA(pst_new_netbuf), OAL_SIZEOF(ieee80211_radiotap_stru));
    oal_memcopy(OAL_NETBUF_DATA(pst_new_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru), OAL_NETBUF_DATA(pst_netbuf), us_frame_len);

    /* 先判断hal vap id是否有效, 再通过hal vap id找到mac vap id, 然后找到mac vap结构, 然后在mib项里找到当前vap的mac地址 */
    if (HAL_VAP_ID_IS_VALID(pst_cb_ctrl->st_rx_info.bit_vap_id))
    {
        hal_get_hal_vap(pst_hal_device, pst_cb_ctrl->st_rx_info.bit_vap_id, &pst_hal_vap);
        if (OAL_PTR_NULL == pst_hal_vap)
        {
            OAM_WARNING_LOG1(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_rx:get vap faild, hal vap id=%u}", pst_cb_ctrl->st_rx_info.bit_vap_id);
            /* 获取hal vap失败, 直接使用配置vap id */
            uc_mac_vap_id = 0;
        }
        else
        {
            uc_mac_vap_id = pst_hal_vap->uc_mac_vap_id;
        }
    }
    else
    {
        /* hal vap id无效, 则使用配置vap id, 至此可以判断是其他BSS发的帧, 无需构造SIFS响应帧 */
        uc_mac_vap_id = 0;
    }
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_mac_vap_id);
    if (OAL_PTR_NULL == pst_mac_vap)
    {
        /* MAC VAP ID为0, 则是配置VAP, 无MIB项, 后续函数调用使用MIB项会出现空指针访问 */
        OAM_WARNING_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_rx::pst_mac_vap is null.}");
        /* 释放pst_new_netbuf */
        oal_netbuf_free(pst_new_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取接收帧的速率 */
    st_per_rate.un_nss_rate.st_ht_rate.bit_ht_mcs        = pst_cb_ctrl->st_rx_statistic.un_nss_rate.st_ht_rate.bit_ht_mcs;
    st_per_rate.un_nss_rate.st_ht_rate.bit_protocol_mode = pst_cb_ctrl->st_rx_statistic.un_nss_rate.st_ht_rate.bit_protocol_mode;
    st_per_rate.uc_short_gi      = pst_cb_ctrl->st_rx_status.bit_GI;
    st_per_rate.bit_preamble     = pst_cb_ctrl->st_rx_status.bit_preabmle;
    st_per_rate.bit_stbc         = pst_cb_ctrl->st_rx_status.bit_STBC;
    st_per_rate.bit_channel_code = pst_cb_ctrl->st_rx_status.bit_channel_code;
    st_per_rate.uc_bandwidth     = pst_cb_ctrl->st_rx_status.bit_freq_bandwidth_mode;
    ul_ret = dmac_alg_get_rate_kbps(pst_hal_device, &st_per_rate, &ul_rate_kbps);
    if (OAL_ERR_CODE_PTR_NULL == ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_rx::get rate failed.}");
        /* 若获取速率失败, 默认速率为6Mbps */
        ul_rate_kbps = DMAC_PKT_CAP_5G_RATE;
    }

    /* 填充radiotap */
    pst_radiotap = (ieee80211_radiotap_stru *)OAL_NETBUF_DATA(pst_new_netbuf);
    dmac_fill_radiotap(pst_radiotap, ul_rate_kbps, &st_per_rate, pst_mac_vap, pst_cb_ctrl, OAM_OTA_FRAME_DIRECTION_TYPE_RX, c_rssi_ampdu);
    pst_radiotap->st_radiotap_fields.ull_timestamp = (oal_uint64)pst_cb_ctrl->st_rx_status.ul_tsf_timestamp;

    /* 上报当前描述符对应的帧 */
    dmac_report_packet(pst_new_netbuf, (oal_uint16)OAL_SIZEOF(ieee80211_radiotap_stru), (oal_uint16)pst_cb_ctrl->st_rx_info.uc_mac_header_len,
                       (oal_uint16)(pst_cb_ctrl->st_rx_info.us_frame_len - pst_cb_ctrl->st_rx_info.uc_mac_header_len),
                        pst_dmac_packet->en_report_sdt_switch, OAM_OTA_FRAME_DIRECTION_TYPE_RX);

    /* 统计抓包个数 */
    pst_dmac_packet->ul_total_report_pkt_num++;

    pst_mac_frame_head = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_cb_ctrl->st_rx_info));
    if  (OAL_PTR_NULL == pst_mac_frame_head)
    {
        OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "dmac_pkt_cap_rx::mac frame head is null.");
        /* 释放pst_new_netbuf */
        oal_netbuf_free(pst_new_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* MAC VAP ID为0, 则是配置VAP, 无MIB项, 后续函数调用使用MIB项会出现空指针访问, 所以增加VAP ID的值判断 */
    if (0 == uc_mac_vap_id)
    {
        /* 释放pst_new_netbuf */
        oal_netbuf_free(pst_new_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }
    puc_vap_mac_addr = mac_mib_get_StationID(pst_mac_vap);

    /* 判断帧是否是发给本BSS的 */
    if (0 == oal_compare_mac_addr(pst_mac_frame_head->auc_address1, puc_vap_mac_addr))
    {
        /* 判断接收的是控制帧还是数据帧/管理帧 */
        if (WLAN_FC0_TYPE_CTL == mac_get_frame_type(OAL_NETBUF_DATA(pst_netbuf)))
        {
            /* 如果是RTS则软件需要构造一个CTS, 并发送 */
            if(WLAN_RTS == mac_frame_get_subtype_value(OAL_NETBUF_DATA(pst_netbuf)))
            {
                /* 申请pst_make_netbuf存储构造的SIFS响应帧 */
                pst_make_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, OAL_SIZEOF(ieee80211_radiotap_stru) + ACK_CTS_FRAME_LEN - WLAN_HDR_FCS_LENGTH, OAL_NETBUF_PRIORITY_MID);
                if (OAL_PTR_NULL == pst_make_netbuf)
                {
                    OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_rx::pst_make_netbuf request memory failed.}");
                    oal_netbuf_free(pst_new_netbuf);
                    return OAL_ERR_CODE_ALLOC_MEM_FAIL;
                }
                OAL_MEMZERO((oal_uint8 *)OAL_NETBUF_DATA(pst_make_netbuf), OAL_SIZEOF(ieee80211_radiotap_stru));
                /* 调用CTS的构造函数, 并将pst_make_radiotap指向pst_make_netbuf的data头 */
                us_rts_duration     = pst_mac_frame_head->bit_duration_value;
                uc_cts_len          = (oal_uint8)dmac_encap_ctl_cts((oal_uint8 *)OAL_NETBUF_DATA(pst_make_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru), pst_mac_frame_head->auc_address2, us_rts_duration, ul_rate_kbps);
                pst_make_radiotap   = (ieee80211_radiotap_stru *)(OAL_NETBUF_DATA(pst_make_netbuf));
                /* 填充构造帧的radiotap */
                dmac_fill_radiotap(pst_make_radiotap, ul_rate_kbps, &st_per_rate, pst_mac_vap, OAL_PTR_NULL, OAM_OTA_FRAME_DIRECTION_TYPE_TX, 0);
                pst_make_radiotap->st_radiotap_fields.ull_timestamp = (oal_uint64)pst_cb_ctrl->st_rx_status.ul_tsf_timestamp + SIFSTIME;

                /* 构造的CTS帧上报 */
                dmac_report_packet(pst_make_netbuf, (oal_uint16)OAL_SIZEOF(ieee80211_radiotap_stru), (oal_uint16)uc_cts_len,
                                    0, pst_dmac_packet->en_report_sdt_switch, OAM_OTA_FRAME_DIRECTION_TYPE_TX);

                /* 统计上报的帧个数 */
                pst_dmac_packet->ul_total_report_pkt_num++;
                /* 释放pst_make_netbuf */
                oal_netbuf_free(pst_make_netbuf);
            }
        }
        else
        {
            /* 判断是否为AMPDU, 是则需要回BA, 否则回ACK */
            if (1 != pst_cb_ctrl->st_rx_status.bit_AMPDU)
            {
                pst_make_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, OAL_SIZEOF(ieee80211_radiotap_stru) + ACK_CTS_FRAME_LEN - WLAN_HDR_FCS_LENGTH, OAL_NETBUF_PRIORITY_MID);
                if (OAL_PTR_NULL == pst_make_netbuf)
                {
                    OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_pkt_cap_rx::pst_make_netbuf request memory failed.}");
                    oal_netbuf_free(pst_new_netbuf);
                    return OAL_ERR_CODE_ALLOC_MEM_FAIL;
                }
                OAL_MEMZERO((oal_uint8 *)OAL_NETBUF_DATA(pst_make_netbuf), OAL_SIZEOF(ieee80211_radiotap_stru));
                /* 调用ACK的构造函数, 并将pst_make_radiotap指向pst_make_netbuf的data头 */
                uc_ack_len          = (oal_uint8)dmac_encap_ctl_ack(OAL_NETBUF_DATA(pst_make_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru), pst_mac_frame_head->auc_address2);
                pst_make_radiotap   = (ieee80211_radiotap_stru *)(OAL_NETBUF_DATA(pst_make_netbuf));
                /* 构造ACK的速率大于24采用24Mbps, 小于24且大于6采用6Mbps, 小于6采用1Mbps, 同时注意降协议 */
                st_per_rate.un_nss_rate.st_ht_rate.bit_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
                if (ul_rate_kbps >= DMAC_PKT_CAP_SIFS_RATE3)
                {
                    ul_ack_rate = DMAC_PKT_CAP_SIFS_RATE3;
                }
                else if (ul_rate_kbps >= DMAC_PKT_CAP_SIFS_RATE2 && ul_rate_kbps < DMAC_PKT_CAP_SIFS_RATE3)
                {
                    ul_ack_rate = DMAC_PKT_CAP_SIFS_RATE2;
                }
                else
                {
                    ul_ack_rate = DMAC_PKT_CAP_SIFS_RATE1;
                    st_per_rate.un_nss_rate.st_ht_rate.bit_protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
                }
                /* 填充构造ACK的radiotap */
                dmac_fill_radiotap(pst_make_radiotap, ul_ack_rate, &st_per_rate, pst_mac_vap, OAL_PTR_NULL, OAM_OTA_FRAME_DIRECTION_TYPE_TX, 0);
                pst_make_radiotap->st_radiotap_fields.ull_timestamp = (oal_uint64)pst_cb_ctrl->st_rx_status.ul_tsf_timestamp +
                       (oal_uint64)(((oal_uint32)(pst_cb_ctrl->st_rx_info.us_frame_len - pst_cb_ctrl->st_rx_info.uc_mac_header_len) << 3) * 1000 / ul_rate_kbps) + SIFSTIME;

                /* 构造的ACK帧上报 */
                dmac_report_packet(pst_make_netbuf, (oal_uint16)OAL_SIZEOF(ieee80211_radiotap_stru), (oal_uint16)uc_ack_len,
                                    0, pst_dmac_packet->en_report_sdt_switch, OAM_OTA_FRAME_DIRECTION_TYPE_TX);

                /* 统计上报的帧个数 */
                pst_dmac_packet->ul_total_report_pkt_num++;
                /* 释放pst_make_netbuf */
                oal_netbuf_free(pst_make_netbuf);
            }
        }
    }
    /* 释放接收帧申请的pst_new_netbuf */
    oal_netbuf_free(pst_new_netbuf);
    return OAL_SUCC;
}


oal_uint32  dmac_pkt_cap_ba(dmac_packet_stru *pst_dmac_packet, dmac_rx_ctl_stru *pst_cb_ctrl, hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint32                      ul_ba_len;
    oal_netbuf_stru                *pst_make_netbuf = OAL_PTR_NULL;
    mac_vap_stru                   *pst_mac_vap;
    mac_ieee80211_qos_frame_stru   *pst_mac_head;
    ieee80211_radiotap_stru        *pst_radiotap;
    hal_statistic_stru              st_per_rate;
    oal_uint32                      ul_rate_kbps;
    oal_uint32                      ul_ba_kbps;
    oal_uint8                      *puc_vap_mac_addr;
    oal_uint32                      ul_ret;
    hal_to_dmac_vap_stru           *pst_hal_vap;
    oal_uint16                      us_user_idx = 0xFFFF;
    dmac_user_stru                 *pst_ta_dmac_user;
    oal_uint8                       uc_lut_index;

    /* 先判断hal vap id是否有效, 再通过hal vap id找到mac vap id, 然后找到mac vap结构, 然后在mib项里找到当前vap的mac地址 */
    if (HAL_VAP_ID_IS_VALID(pst_cb_ctrl->st_rx_info.bit_vap_id))
    {
        hal_get_hal_vap(pst_hal_device, pst_cb_ctrl->st_rx_info.bit_vap_id, &pst_hal_vap);
        if (OAL_PTR_NULL == pst_hal_vap)
        {
            OAM_WARNING_LOG1(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_ba:get vap faild, hal vap id=%d}", pst_cb_ctrl->st_rx_info.bit_vap_id);
            return OAL_ERR_CODE_PTR_NULL;
        }
    }
    else
    {
        /* hal vap id不合法, 则不是发给本BSS的聚合帧, 不回BA */
        OAM_WARNING_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_ba::hal vap id is invalid.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_hal_vap->uc_mac_vap_id);
    if ((OAL_PTR_NULL == pst_mac_vap) ||(0 == pst_hal_vap->uc_mac_vap_id))
    {
        /* MAC VAP ID为0, 则是配置VAP, 无MIB项, 后续函数调用使用MIB项会出现空指针访问, 所以增加VAP ID的值判断 */
        OAM_WARNING_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_ba::pst_mac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /* 直接使用QOS帧的帧结构 */
    pst_mac_head = (mac_ieee80211_qos_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_cb_ctrl->st_rx_info));
    if (OAL_PTR_NULL == pst_mac_head)
    {
        OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "dmac_pkt_cap_ba::mac frame head is null.");
        return OAL_ERR_CODE_PTR_NULL;
    }
    puc_vap_mac_addr = mac_mib_get_StationID(pst_mac_vap);

    /* 判断是否是发给本bss的帧 */
    if(0 == oal_compare_mac_addr(pst_mac_head->auc_address1, puc_vap_mac_addr))
    {
        /* 通过user的mac地址查找对应user的index, 最终获得dmac user结构体 */
        ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, pst_mac_head->auc_address2, &us_user_idx);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_TX, "{dmac_pkt_cap_ba::us_user_idx is invalid [%u].}", us_user_idx);
            return OAL_ERR_CODE_PTR_NULL;
        }
        pst_ta_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);
        if (OAL_PTR_NULL == pst_ta_dmac_user)
        {
            OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_pkt_cap_ba::pst_ta_dmac_user is NULL.}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        /* 通过user所处的发送tid缓存队列, 找到lut index */
        uc_lut_index    = pst_ta_dmac_user->ast_tx_tid_queue[pst_mac_head->bit_qc_tid].st_ba_rx_hdl.uc_lut_index;
        pst_make_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, OAL_SIZEOF(ieee80211_radiotap_stru) + MAC_80211_CTL_HEADER_LEN + 12, OAL_NETBUF_PRIORITY_MID);
        if (OAL_PTR_NULL == pst_make_netbuf)
        {
            OAM_WARNING_LOG0(0, OAM_SF_TX, "{dmac_pkt_cap_ba::alloc netbuff failed.}");
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }
        OAL_MEMZERO((oal_uint8 *)OAL_NETBUF_DATA(pst_make_netbuf), OAL_SIZEOF(ieee80211_radiotap_stru));
        /* 构造ba帧 */
        ul_ba_len = dmac_encap_ctl_ba(pst_mac_vap, OAL_NETBUF_DATA(pst_make_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru), pst_mac_head->auc_address2, uc_lut_index);

        /* 获取接收帧的速率 */
        st_per_rate.un_nss_rate.st_ht_rate.bit_ht_mcs        = pst_cb_ctrl->st_rx_statistic.un_nss_rate.st_ht_rate.bit_ht_mcs;
        st_per_rate.un_nss_rate.st_ht_rate.bit_protocol_mode = pst_cb_ctrl->st_rx_statistic.un_nss_rate.st_ht_rate.bit_protocol_mode;
        st_per_rate.uc_short_gi         = pst_cb_ctrl->st_rx_status.bit_GI;
        st_per_rate.bit_preamble        = pst_cb_ctrl->st_rx_status.bit_preabmle;
        st_per_rate.bit_stbc            = pst_cb_ctrl->st_rx_status.bit_STBC;
        st_per_rate.bit_channel_code    = pst_cb_ctrl->st_rx_status.bit_channel_code;
        st_per_rate.uc_bandwidth        = pst_cb_ctrl->st_rx_status.bit_freq_bandwidth_mode;
        ul_ret = dmac_alg_get_rate_kbps(pst_hal_device, &st_per_rate, &ul_rate_kbps);
        if (OAL_ERR_CODE_PTR_NULL == ul_ret)
        {
            OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_ba::get rate failed.}");
            /* 若获取速率失败, 默认速率为6Mbps */
            ul_rate_kbps = DMAC_PKT_CAP_5G_RATE;
        }

        /* 构造BA的速率大于24采用24Mbps, 小于24且大于6采用6Mbps, 小于6采用1Mbps, 同时注意降协议 */
        st_per_rate.un_nss_rate.st_ht_rate.bit_protocol_mode = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
        if (ul_rate_kbps >= DMAC_PKT_CAP_SIFS_RATE3)
        {
            ul_ba_kbps = DMAC_PKT_CAP_SIFS_RATE3;
        }
        else if (ul_rate_kbps >= DMAC_PKT_CAP_SIFS_RATE2 && ul_rate_kbps < DMAC_PKT_CAP_SIFS_RATE3)
        {
            ul_ba_kbps = DMAC_PKT_CAP_SIFS_RATE2;
        }
        else
        {
            ul_ba_kbps = DMAC_PKT_CAP_SIFS_RATE1;
            st_per_rate.un_nss_rate.st_ht_rate.bit_protocol_mode = WLAN_11B_PHY_PROTOCOL_MODE;
        }
        /* 填充radiotap */
        pst_radiotap = (ieee80211_radiotap_stru *)OAL_NETBUF_DATA(pst_make_netbuf);
        dmac_fill_radiotap(pst_radiotap, ul_ba_kbps, &st_per_rate, pst_mac_vap, pst_cb_ctrl, OAM_OTA_FRAME_DIRECTION_TYPE_TX, 0);
        pst_radiotap->st_radiotap_fields.ull_timestamp = (oal_uint64)pst_cb_ctrl->st_rx_status.ul_tsf_timestamp +
                   (oal_uint64)(((oal_uint32)(pst_cb_ctrl->st_rx_info.us_frame_len - pst_cb_ctrl->st_rx_info.uc_mac_header_len) << 3) * 1000 / ul_rate_kbps) + SIFSTIME;

        /* 上报当前描述符对应的ba帧 */
        dmac_report_packet(pst_make_netbuf, (oal_uint16)OAL_SIZEOF(ieee80211_radiotap_stru), (oal_uint16)MAC_80211_CTL_HEADER_LEN,
                            (oal_uint16)(ul_ba_len - MAC_80211_CTL_HEADER_LEN), pst_dmac_packet->en_report_sdt_switch, OAM_OTA_FRAME_DIRECTION_TYPE_TX);

        /* 统计上报的帧个数 */
        pst_dmac_packet->ul_total_report_pkt_num++;

        oal_netbuf_free(pst_make_netbuf);
    }
    return OAL_SUCC;
}


oal_uint32  dmac_pkt_cap_beacon(dmac_packet_stru *pst_dmac_packet, dmac_vap_stru *pst_dmac_vap)
{
    oal_uint32                   ul_cur_vap_timestamp[2];
    oal_uint32                   ul_ext_ap_eleven_timestamp;
    oal_uint8                   *puc_cur_bcn_buf;
    hal_to_dmac_vap_stru         st_ext_ap_eleven_vap;
    oal_netbuf_stru             *pst_make_netbuf;
    ieee80211_radiotap_stru     *pst_radiotap;
    oal_uint32                   ul_cur_bcn_rate;
    oal_uint32                   ul_cur_bcn_tx_mode;
    hal_statistic_stru           st_per_rate;
    oal_uint32                   ul_rate_kbps;
    oal_uint32                   ul_seq;
    mac_ieee80211_frame_stru    *pst_mac_header;
    oal_uint32                   ul_ret;

    /* 1、获取对应VAP的tsf值, 以及相应beacon的seq num */
    hal_vap_tsf_get_64bit(pst_dmac_vap->pst_hal_vap, &ul_cur_vap_timestamp[1], &ul_cur_vap_timestamp[0]);
    hal_get_tx_sequence_num(pst_dmac_vap->pst_hal_device, 0, 0, 0, 0,&ul_seq);

    /* 2、获取EXT AP11的tsf值, 填写对应发送时间radiotap */
    st_ext_ap_eleven_vap.uc_vap_id      = WLAN_HAL_EXT_AP11_VAP_ID;
    st_ext_ap_eleven_vap.uc_chip_id     = pst_dmac_vap->pst_hal_vap->uc_chip_id;
    st_ext_ap_eleven_vap.uc_device_id   = pst_dmac_vap->pst_hal_vap->uc_device_id;
    hal_vap_tsf_get_32bit(&st_ext_ap_eleven_vap, &ul_ext_ap_eleven_timestamp);

    /* 3、获取需要上报的beacon帧 pst_dmac_vap->pauc_beacon_buffer[pst_dmac_vap->uc_beacon_idx], 更新beacon帧的beacon timestamp */
    pst_make_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, OAL_SIZEOF(ieee80211_radiotap_stru) + pst_dmac_vap->us_beacon_len, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_make_netbuf)
    {
        OAM_ERROR_LOG0(0, OAM_SF_TX, "{dmac_tbtt_event_ap::pst_make_netbuf to request memory failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    OAL_MEMZERO((oal_uint8 *)OAL_NETBUF_DATA(pst_make_netbuf), OAL_SIZEOF(ieee80211_radiotap_stru));
    puc_cur_bcn_buf = pst_dmac_vap->pauc_beacon_buffer[pst_dmac_vap->uc_beacon_idx];
    oal_memcopy((oal_uint8 *)OAL_NETBUF_DATA(pst_make_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru), puc_cur_bcn_buf, pst_dmac_vap->us_beacon_len);
    *(oal_uint64 *)((oal_uint8 *)OAL_NETBUF_DATA(pst_make_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru) + MAC_80211_FRAME_LEN) = ((oal_uint64)ul_cur_vap_timestamp[1] << 32) + (oal_uint64)ul_cur_vap_timestamp[0];

    pst_mac_header = (mac_ieee80211_frame_stru *)((oal_uint8 *)OAL_NETBUF_DATA(pst_make_netbuf) + OAL_SIZEOF(ieee80211_radiotap_stru));
    if (0 == ul_seq)
    {
        pst_mac_header->bit_seq_num = 4095;
    }
    pst_mac_header->bit_seq_num = ul_seq - 1;

    /* 4、获取对应VAP的beacon帧发送速率-对应寄存器 */
    hal_get_bcn_info(pst_dmac_vap->pst_hal_vap, &ul_cur_bcn_rate, &ul_cur_bcn_tx_mode);
    st_per_rate.uc_bandwidth     = WLAN_BAND_WIDTH_20M;
    st_per_rate.uc_short_gi      = (ul_cur_bcn_rate & BIT28) >> 28;
    st_per_rate.bit_preamble     = (ul_cur_bcn_rate & BIT27) >> 27;
    st_per_rate.bit_channel_code = (ul_cur_bcn_tx_mode & BIT2) >> 2;
    st_per_rate.un_nss_rate.st_legacy_rate.bit_protocol_mode = (ul_cur_bcn_rate & (BIT22 | BIT23)) >> 22;
    st_per_rate.un_nss_rate.st_legacy_rate.bit_legacy_rate   = (ul_cur_bcn_rate & (BIT16 | BIT17 | BIT18 | BIT19)) >> 16;
    ul_ret = dmac_alg_get_rate_kbps(pst_dmac_vap->pst_hal_device, &st_per_rate, &ul_rate_kbps);
    if (OAL_ERR_CODE_PTR_NULL == ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_PKT_CAP, "{dmac_pkt_cap_beacon::get rate failed.}");
        /* 若获取速率失败, 2G默认速率为1Mbps, 5G默认速率为6Mbps */
        if (WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)
        {
            ul_rate_kbps = DMAC_PKT_CAP_2G_RATE;
        }
        else
        {
            ul_rate_kbps = DMAC_PKT_CAP_5G_RATE;
        }
    }

    /* 5、填写radiotap头 */
    pst_radiotap = (ieee80211_radiotap_stru *)OAL_NETBUF_DATA(pst_make_netbuf);
    dmac_fill_radiotap(pst_radiotap, ul_rate_kbps, &st_per_rate, &(pst_dmac_vap->st_vap_base_info), OAL_PTR_NULL, OAM_OTA_FRAME_DIRECTION_TYPE_TX, 0);
    pst_radiotap->st_radiotap_fields.ull_timestamp = (oal_uint64)ul_ext_ap_eleven_timestamp;

    /* 上报Beacon帧 */
    dmac_report_packet(pst_make_netbuf, (oal_uint16)OAL_SIZEOF(ieee80211_radiotap_stru), (oal_uint16)MAC_80211_FRAME_LEN,
                       (oal_uint16)(pst_dmac_vap->us_beacon_len - MAC_80211_FRAME_LEN), pst_dmac_packet->en_report_sdt_switch, OAM_OTA_FRAME_DIRECTION_TYPE_TX);
    /* 统计上报的帧个数 */
    pst_dmac_packet->ul_total_report_pkt_num++;
    oal_netbuf_free(pst_make_netbuf);

    return OAL_SUCC;
}


/*lint -e578*//*lint -e19*/
oal_module_symbol(dmac_pkt_cap_tx);
oal_module_symbol(dmac_pkt_cap_rx);
oal_module_symbol(dmac_pkt_cap_ba);
oal_module_symbol(dmac_pkt_cap_beacon);
/*lint +e578*//*lint +e19*/


#endif /* _PRE_WLAN_FEATURE_PACKET_CAPTURE */

#ifdef  __cplusplus
#if     __cplusplus
    }
#endif
#endif

