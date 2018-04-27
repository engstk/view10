

#ifndef    __DMAC_PKT_CAPTURE_H__
#define    __DMAC_PKT_CAPTURE_H__

#ifdef  __cplusplus
#if     __cplusplus
extern  "C" {
#endif
#endif

#ifdef    _PRE_WLAN_FEATURE_PACKET_CAPTURE
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "dmac_vap.h"
#include "dmac_ext_if.h"
#include "wlan_types.h"
#include "mac_vap.h"
#include "mac_frame.h"
#include "hal_commom_ops.h"
#include "oal_net.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_PKT_CAPTURE_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* 2G和5G的默认速率 */
#define DMAC_PKT_CAP_2G_RATE    1000
#define DMAC_PKT_CAP_5G_RATE    6000

/* 响应帧对应的速率选择 */
#define DMAC_PKT_CAP_SIFS_RATE1     1000
#define DMAC_PKT_CAP_SIFS_RATE2     6000
#define DMAC_PKT_CAP_SIFS_RATE3     24000

/* Radiotap版本 */
#define PKTHDR_RADIOTAP_VERSION     0

/* 其他抓包相关宏 */
#define DMAC_PKT_CAP_TX_NOISE       -95
#define DMAC_PKT_CAP_SIGNAL_OFFSET  -94
#define DMAC_PKT_CAP_NOISE_MAX      0
#define DMAC_PKT_CAP_NOISE_MIN      -100
#define DMAC_PKT_CAP_RATE_UNIT      500

/* CTS\ACK帧的时长 */
#define DMAC_PKT_CAP_CTS_ACK_TIME1      132
#define DMAC_PKT_CAP_CTS_ACK_TIME2      40
#define DMAC_PKT_CAP_CTS_ACK_TIME3      28

/* RTS帧的时长 */
#define DMAC_PKT_CAP_RTS_TIME1      48
#define DMAC_PKT_CAP_RTS_TIME2      28

/* BA帧的时长 */
#define DMAC_PKT_CAP_BA_TIME1       276
#define DMAC_PKT_CAP_BA_TIME2       64
#define DMAC_PKT_CAP_BA_TIME3       32

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* 抓包模式的三种类型 */
typedef enum
{
    DMAC_PKT_CAP_NORMAL     = 0,
    DMAC_PKT_CAP_MIX        = 1,
    DMAC_PKT_CAP_MONITOR    = 2,

    DMAC_PKT_CAP_BUTT
}dmac_pkt_cap_type;
typedef oal_uint8 dmac_pkt_cap_type_uint8;

/* Radiotap头的present */
typedef enum
{
    DMAC_IEEE80211_RADIOTAP_TSFT = 0x00000001,
    DMAC_IEEE80211_RADIOTAP_FLAGS = 0x00000002,
    DMAC_IEEE80211_RADIOTAP_RATE = 0x00000004,
    DMAC_IEEE80211_RADIOTAP_CHANNEL = 0x00000008,

    DMAC_IEEE80211_RADIOTAP_DBM_ANTSIGNAL = 0x00000020,
    DMAC_IEEE80211_RADIOTAP_DBM_ANTNOISE = 0x00000040,
    DMAC_IEEE80211_RADIOTAP_LOCK_QUALITY = 0x00000080,

    DMAC_IEEE80211_RADIOTAP_MCS = 0x00080000,
    DMAC_IEEE80211_RADIOTAP_VHT = 0x00200000,

    DMAC_IEEE80211_RADIOTAP_BUTT
}ieee80211_radiotap_it_present;
typedef oal_uint32 ieee80211_radiotap_it_present_uint32;

/* Radiotap扩展部分的flags */
typedef enum
{
    DMAC_IEEE80211_RADIOTAP_F_CFP = 0x01,
    DMAC_IEEE80211_RADIOTAP_F_SHORTPRE = 0x02,
    DMAC_IEEE80211_RADIOTAP_F_WEP = 0x04,
    DMAC_IEEE80211_RADIOTAP_F_FRAG = 0x08,
    DMAC_IEEE80211_RADIOTAP_F_FCS = 0x10,
    DMAC_IEEE80211_RADIOTAP_F_DATAPAD = 0x20,
    DMAC_IEEE80211_RADIOTAP_F_BADFCS = 0x40,
    DMAC_IEEE80211_RADIOTAP_F_SHORTGI = 0x80,

    DMAC_IEEE80211_RADIOTAP_F_BUTT
}ieee80211_radiotap_flags;
typedef oal_uint8 ieee80211_radiotap_flags_uint8;

/* Radiotap扩展部分的channel types */
typedef enum
{
    DMAC_IEEE80211_CHAN_CCK = 0x0020,
    DMAC_IEEE80211_CHAN_OFDM = 0x0040,
    DMAC_IEEE80211_CHAN_2GHZ = 0x0080,
    DMAC_IEEE80211_CHAN_5GHZ = 0x0100,
    DMAC_IEEE80211_CHAN_DYN = 0x0400,
    DMAC_IEEE80211_CHAN_HALF = 0x4000,
    DMAC_IEEE80211_CHAN_QUARTER = 0x8000,

    DMAC_IEEE80211_CHAN_BUTT
}ieee80211_radiotap_channel_types;
typedef oal_uint16 ieee80211_radiotap_channel_types_uint16;

/* Radiotap扩展部分的mcs info的子成员known */
typedef enum
{
    DMAC_IEEE80211_RADIOTAP_MCS_HAVE_BW = 0x01,
    DMAC_IEEE80211_RADIOTAP_MCS_HAVE_MCS = 0x02,
    DMAC_IEEE80211_RADIOTAP_MCS_HAVE_GI = 0x04,
    DMAC_IEEE80211_RADIOTAP_MCS_HAVE_FMT = 0x08,
    DMAC_IEEE80211_RADIOTAP_MCS_HAVE_FEC = 0x10,
    DMAC_IEEE80211_RADIOTAP_MCS_HAVE_STBC = 0x20,

    DMAC_IEEE80211_RADIOTAP_MCS_HAVE_BUTT
}ieee80211_radiotap_mcs_have;
typedef oal_uint8 ieee80211_radiotap_mcs_have_uint8;

/* Radiotap扩展部分的mcs info的子成员flags */
typedef enum
{
    DMAC_IEEE80211_RADIOTAP_MCS_BW_MASK = 0x03,
    DMAC_IEEE80211_RADIOTAP_MCS_BW_20 = 0,
    DMAC_IEEE80211_RADIOTAP_MCS_BW_40 = 1,
    DMAC_IEEE80211_RADIOTAP_MCS_BW_20L = 2,
    DMAC_IEEE80211_RADIOTAP_MCS_BW_20U = 3,

    DMAC_IEEE80211_RADIOTAP_MCS_SGI = 0x04,
    DMAC_IEEE80211_RADIOTAP_MCS_FMT_GF = 0x08,
    DMAC_IEEE80211_RADIOTAP_MCS_FEC_LDPC = 0x10,
    DMAC_IEEE80211_RADIOTAP_MCS_STBC_MASK = 0x60,
    DMAC_IEEE80211_RADIOTAP_MCS_STBC_SHIFT = 5,

    DMAC_IEEE80211_RADIOTAP_MCS_BUTT
}ieee80211_radiotap_mcs_flags;
typedef oal_uint8 ieee80211_radiotap_mcs_flags_uint8;

/* Radiotap扩展部分的vht info的子成员known */
typedef enum
{
    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_STBC = 0x0001,
    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_TXOP_PS_NA = 0x0002,
    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_GI = 0x0004,
    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_SGI_NSYM_DIS = 0x0008,
    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_LDPC_EXTRA_OFDM_SYM = 0x0010,
    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_BEAMFORMED = 0x0020,
    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH = 0x0040,
    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_GROUP_ID = 0x0080,
    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_PARTIAL_AID = 0x0100,

    DMAC_IEEE80211_RADIOTAP_VHT_KNOWN_BUTT
}ieee80211_radiotap_vht_known;
typedef oal_uint16 ieee80211_radiotap_vht_known_uint16;

/* Radiotap扩展部分的vht info的子成员flags */
typedef enum
{
    DMAC_IEEE80211_RADIOTAP_VHT_FLAG_STBC = 0x01,
    DMAC_IEEE80211_RADIOTAP_VHT_FLAG_TXOP_PS_NA = 0x02,
    DMAC_IEEE80211_RADIOTAP_VHT_FLAG_SGI = 0x04,
    DMAC_IEEE80211_RADIOTAP_VHT_FLAG_SGI_NSYM_M10_9 = 0x08,
    DMAC_IEEE80211_RADIOTAP_VHT_FLAG_LDPC_EXTRA_OFDM_SYM = 0x10,
    DMAC_IEEE80211_RADIOTAP_VHT_FLAG_BEAMFORMED = 0x20,

    DMAC_IEEE80211_RADIOTAP_VHT_FLAG_BUTT
}ieee80211_radiotap_vht_flags;
typedef oal_uint8 ieee80211_radiotap_vht_flags_uint8;

/* Radiotap扩展部分的vht info的子成员bandwidth */
typedef enum
{
    DMAC_IEEE80211_RADIOTAP_VHT_BW_20   = 0,
    DMAC_IEEE80211_RADIOTAP_VHT_BW_40   = 1,
    DMAC_IEEE80211_RADIOTAP_VHT_BW_80   = 4,
    DMAC_IEEE80211_RADIOTAP_VHT_BW_160  = 11,

    DMAC_IEEE80211_RADIOTAP_VHT_BW_BUTT
}ieee80211_radiotap_vht_bandwidth;
typedef oal_uint8 ieee80211_radiotap_vht_bandwidth_uint8;

/*  */
typedef enum
{
    DMAC_IEEE80211_RADIOTAP_CODING_LDPC_USER0 = 0x01,
    DMAC_IEEE80211_RADIOTAP_CODING_LDPC_USER1 = 0x02,
    DMAC_IEEE80211_RADIOTAP_CODING_LDPC_USER2 = 0x04,
    DMAC_IEEE80211_RADIOTAP_CODING_LDPC_USER3 = 0x08,

    DMAC_IEEE80211_RADIOTAP_CODING_LDPC_BUTT
}ieee80211_radiotap_vht_coding;
typedef oal_uint8 ieee80211_radiotap_vht_coding_uint8;


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
typedef struct
{
    oal_uint8     uc_it_version;            /* 使用radiotap header的主要版本, 目前总是为0 */
    oal_uint8     uc_it_pad;                /* 目前并未使用, 只是为了4字节对齐 */
    oal_uint16    us_it_len;                /* radiotap的长度, 包括：header+fields */
    oal_uint32    ul_it_present;            /* 通过bit位标明fields有哪些成员 */
}ieee80211_radiotap_header_stru;

typedef struct
{
    oal_uint64    ull_timestamp;            /* 当前帧的时间戳, 单位为us */
    oal_uint8     uc_flags;                 /* 标志位 */
    oal_uint8     uc_data_rate;             /* TX/RX数据速率, 单位为500Kbps */
    oal_uint16    us_channel_freq;          /* AP所在信道的中心频点, 单位MHz */
    oal_uint16    us_channel_type;          /* 信道类型, 标识5G还是2G */
    oal_int8      c_ssi_signal;             /* 信号强度, 单位为dBm */
    oal_int8      c_ssi_noise;              /* 噪声强度, 单位为dBm */
    oal_int16     s_signal_quality;         /* 具体意义不详, 产品给的计算方法是RSSI + 94 */
    oal_uint8     uc_mcs_info_known;        /* mcs信息, 11n协议时该字段有效 */
    oal_uint8     uc_mcs_info_flags;
    oal_uint8     uc_mcs_info_rate;
    oal_uint16    us_vht_known;             /* vht信息, 11ac协议时该字段有效 */
    oal_uint8     uc_vht_flags;
    oal_uint8     uc_vht_bandwidth;
    oal_uint8     uc_vht_mcs_nss[4];
    oal_uint8     uc_vht_coding;
    oal_uint8     uc_vht_group_id;
    oal_uint16    us_vht_partial_aid;
}ieee80211_radiotap_fields_stru;

typedef struct
{
    ieee80211_radiotap_header_stru  st_radiotap_header;     /* radiotap头结构体 */
    ieee80211_radiotap_fields_stru  st_radiotap_fields;     /* radiotap扩充结构体 */
}ieee80211_radiotap_stru;

typedef struct
{
    oal_uint32                 *pul_circle_buf_start;        /* 发送过程抓包的循环buffer起始地址 */
    oal_uint16                  us_circle_buf_depth;         /* 发送过程抓包的循环buffer深度 */
    oal_uint16                  us_circle_buf_index;         /* device结构下的抓包特性循环buffer的index */
    oal_uint8                   uc_capture_switch;           /* 软件的抓包特性对应的模式开关 */
    oal_bool_enum_uint8         en_report_sdt_switch;        /* 软件的抓包信息是否上报到sdt的开关 */
    oal_uint16                  us_reserved;                 /* 保留字段 */
    oal_uint32                  ul_total_report_pkt_num;     /* 抓包模式下总共上报的抓包个数 */
}dmac_packet_stru;

/* 抓包模式下的循环buffer结构体 */
typedef struct
{
    oal_uint32     *pul_link_addr;                  /* 发送描述符物理地址 */
    oal_uint8       uc_peer_index;                  /* 用户索引号, 暂无使用 */
    oal_uint8       bit_tx_cnt      :   4,          /* 当前帧第几次发送 */
                    bit_q_type      :   3,          /* 发送队列号, 暂无使用 */
                    bit_frm_type    :   1;          /* 帧类型, 0为RTS, 1为非RTS */
    oal_uint16      bit_reserved    :   15,         /* 保留字段 */
                    bit_status      :   1;          /* buffer信息有效标识 */
    oal_uint8       uc_timestamp[8];                /* 发送的时间戳, mac将发还未发的时间 */
}dmac_mem_circle_buf_stru;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32  dmac_pkt_cap_tx(dmac_packet_stru *pst_dmac_packet, hal_to_dmac_device_stru *pst_hal_device);
extern oal_uint32  dmac_pkt_cap_rx(dmac_rx_ctl_stru *pst_cb_ctrl, oal_netbuf_stru *pst_netbuf, dmac_packet_stru *pst_dmac_packet, hal_to_dmac_device_stru *pst_hal_device, oal_int8 c_rssi_ampdu);
extern oal_uint32  dmac_pkt_cap_ba(dmac_packet_stru *pst_dmac_packet, dmac_rx_ctl_stru *pst_cb_ctrl, hal_to_dmac_device_stru *pst_hal_device);
extern oal_uint32  dmac_pkt_cap_beacon(dmac_packet_stru *pst_dmac_packet, dmac_vap_stru *pst_dmac_vap);
extern oal_void  dmac_tx_get_vap_id(hal_to_dmac_device_stru *pst_hal_device, hal_tx_dscr_stru *pst_tx_dscr, oal_uint8 *puc_vap_id);


#endif /* _PRE_WLAN_FEATURE_PACKET_CAPTURE */

#ifdef  __cplusplus
#if     __cplusplus
    }
#endif
#endif

#endif

