

#ifndef __DMAC_VAP_H__
#define __DMAC_VAP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "hal_ext_if.h"
#include "mac_vap.h"
#include "dmac_user.h"
#include "dmac_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_VAP_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMAC_MAX_SW_RETRIES         10  /* 数据报文重传次数 */
#define DMAC_MGMT_MAX_SW_RETRIES    3   /* 管理报文重传次数 */
#define DMAC_MAX_AMPDU_LENGTH_PERFOMANCE_COUNT    32   /* 进行计数的最大的聚合长度*/

#define DMAX_FTM_RANGE_ENTRY_COUNT                 15
#define DMAX_FTM_ERROR_ENTRY_COUNT                 11
#define DMAX_MINIMUN_AP_COUNT                      14
#define MAX_FTM_SESSION                            8     /*FTM同时在线的回话*/

#define DMAC_BW_VERIFY_MAX_THRESHOLD               60

#if(_PRE_WLAN_FEATURE_PMF == _PRE_PMF_HW_CCMP_BIP)

#define IS_OPEN_PMF_REG(_pst_dmac_vap)  (0 != ((_pst_dmac_vap)->ul_user_pmf_status))
#endif

#define GET_CURRENT_LINKLOSS_CNT(_pst_dmac_vap)                      (_pst_dmac_vap->st_linkloss_info.ast_linkloss_info[_pst_dmac_vap->st_linkloss_info.en_linkloss_mode].uc_link_loss)
#define GET_CURRENT_LINKLOSS_THRESHOLD(_pst_dmac_vap)                (_pst_dmac_vap->st_linkloss_info.ast_linkloss_info[_pst_dmac_vap->st_linkloss_info.en_linkloss_mode].uc_linkloss_threshold)
#define GET_CURRENT_LINKLOSS_INT_THRESHOLD(_pst_dmac_vap)            (_pst_dmac_vap->st_linkloss_info.auc_int_linkloss_threshold[_pst_dmac_vap->st_linkloss_info.en_linkloss_mode])
#define GET_CURRENT_LINKLOSS_MIN_THRESHOLD(_pst_dmac_vap)            (_pst_dmac_vap->st_linkloss_info.auc_min_linkloss_threshold[_pst_dmac_vap->st_linkloss_info.en_linkloss_mode])
#define GET_CURRENT_LINKLOSS_THRESHOLD_DECR(_pst_dmac_vap)           (_pst_dmac_vap->st_linkloss_info.auc_linkloss_threshold_decr[_pst_dmac_vap->st_linkloss_info.en_linkloss_mode])

#define MAKE_CURRENT_LINKLOSS_THRESHOLD(_pst_dmac_vap, en_linkloss_mode) ((100*g_auc_int_linkloss_threshold[en_linkloss_mode] / _pst_dmac_vap->st_linkloss_info.ul_dot11BeaconPeriod))

#define INCR_CURRENT_LINKLOSS_CNT(_pst_dmac_vap)                      GET_CURRENT_LINKLOSS_CNT(_pst_dmac_vap)++
#define GET_CURRENT_LINKLOSS_MODE(_pst_dmac_vap)                     (_pst_dmac_vap->st_linkloss_info.en_linkloss_mode)

#define GET_LINKLOSS_CNT(_pst_dmac_vap, en_linkloss_mode)            (_pst_dmac_vap->st_linkloss_info.ast_linkloss_info[en_linkloss_mode].uc_link_loss)
#define GET_LINKLOSS_THRESHOLD(_pst_dmac_vap, en_linkloss_mode)      (_pst_dmac_vap->st_linkloss_info.ast_linkloss_info[en_linkloss_mode].uc_linkloss_threshold)
#define GET_LINKLOSS_INT_THRESHOLD(_pst_dmac_vap, en_linkloss_mode)  (_pst_dmac_vap->st_linkloss_info.auc_int_linkloss_threshold[en_linkloss_mode])
#define GET_LINKLOSS_MIN_THRESHOLD(_pst_dmac_vap, en_linkloss_mode)  (_pst_dmac_vap->st_linkloss_info.auc_min_linkloss_threshold[en_linkloss_mode])
#define GET_LINKLOSS_THRESHOLD_DECR(_pst_dmac_vap, en_linkloss_mode) (_pst_dmac_vap->st_linkloss_info.auc_linkloss_threshold_decr[en_linkloss_mode])

#define DMAC_IS_LINKLOSS(_pst_dmac_vap)                              (GET_CURRENT_LINKLOSS_CNT(_pst_dmac_vap) > GET_CURRENT_LINKLOSS_THRESHOLD(_pst_dmac_vap))

#define DMAC_MAX_TX_SUCCESSIVE_FAIL_PRINT_THRESHOLD_BTCOEX    (40)   /* 连续发送失败的打印RF寄存器门限*/
#define DMAC_MAX_TX_SUCCESSIVE_FAIL_PRINT_THRESHOLD    (20)   /* 连续发送失败的打印RF寄存器门限*/
#define GET_PM_FSM_OWNER(_pst_handler) ((dmac_vap_stru *)((_pst_handler)->st_oal_fsm.p_oshandler))/* 获取vap指针 */
#define GET_PM_STATE(_pst_handler)     ((_pst_handler)->st_oal_fsm.uc_cur_state)/* 获取当前低功耗状态机的状态 */

#define MAC_GET_DMAC_VAP(_pst_mac_vap)    ((dmac_vap_stru *)_pst_mac_vap)
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum
{
    DMAC_STA_BW_SWITCH_EVENT_CHAN_SYNC = 0,
    DMAC_STA_BW_SWITCH_EVENT_BEACON_20M,
    DMAC_STA_BW_SWITCH_EVENT_BEACON_40M,
    DMAC_STA_BW_SWITCH_EVENT_RX_UCAST_DATA_COMPLETE,
    DMAC_STA_BW_SWITCH_EVENT_USER_DEL,
    DMAC_STA_BW_SWITCH_EVENT_RSV,
    DMAC_STA_BW_SWITCH_EVENT_BUTT
}wlan_sta_bw_switch_event_enum;
typedef oal_uint8 wlan_sta_bw_switch_event_enum_uint8;

/* beacon帧索引枚举 */
typedef enum
{
    DMAC_VAP_BEACON_BUFFER1,
    DMAC_VAP_BEACON_BUFFER2,

    DMAC_VAP_BEACON_BUFFER_BUTT
}dmac_vap_beacon_buffer_enum;
/* 按照不同聚合长度分类的索引的枚举值*/
/*0:1~14 */
/*1:15~17 */
/*2:18~30 */
/*3:31~32 */
typedef enum
{
    DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_0,
    DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_1,
    DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_2,
    DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_3,
    DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_BUTT
}dmac_count_by_ampdu_length_enum;
/* 统计的AMPDU的门限值枚举值*/
typedef enum
{
    DMAC_AMPDU_LENGTH_COUNT_LEVEL_1 = 1,
    DMAC_AMPDU_LENGTH_COUNT_LEVEL_14 = 14,
    DMAC_AMPDU_LENGTH_COUNT_LEVEL_15 = 15,
    DMAC_AMPDU_LENGTH_COUNT_LEVEL_17 = 17,
    DMAC_AMPDU_LENGTH_COUNT_LEVEL_18 = 18,
    DMAC_AMPDU_LENGTH_COUNT_LEVEL_30 = 30,
    DMAC_AMPDU_LENGTH_COUNT_LEVEL_31 = 31,
    DMAC_AMPDU_LENGTH_COUNT_LEVEL_32 = 32
}dmac_count_by_ampdu_length_level_enum;

#ifdef _PRE_WLAN_FEATURE_STA_PM
typedef enum _PM_DEBUG_MSG_TYPE_{
    PM_MSG_WAKE_TO_ACTIVE = 0,
    PM_MSG_WAKE_TO_DOZE,
    PM_MSG_ACTIVE_TO_DOZE,
    PM_MSG_TBTT_CNT, /* tbtt count */
    PM_MSG_PSM_BEACON_CNT,
    PM_MSG_BEACON_TIMEOUT_CNT,
    PM_MSG_PROCESS_DOZE_CNT,
    PM_MSG_BCN_TOUT_SLEEP,
    PM_MSG_DEEP_DOZE_CNT = 8,
    PM_MSG_LIGHT_DOZE_CNT,
    PM_MSG_LAST_DTIM_SLEEP,
    PM_MSG_SCAN_DIS_ALLOW_SLEEP,
    PM_MSG_DBAC_DIS_ALLOW_SLEEP,
    PM_MSG_FREQ_DIS_ALLOW_SLEEP,
    PM_MSG_BCNTIMOUT_DIS_ALLOW_SLEEP,
    PM_MSG_HOST_AWAKE,
    PM_MSG_DTIM_AWAKE,
    PM_MSG_TIM_AWAKE,
    /* 超时定时器处理 */
    PM_MSG_PSM_TIMEOUT_PM_OFF  = 18,
    PM_MSG_PSM_TIMEOUT_QUEUE_NO_EMPTY,
    PM_MSG_PSM_RESTART_A,
    PM_MSG_PSM_RESTART_B,
    PM_MSG_PSM_RESTART_C,
    PM_MSG_PSM_TIMEOUT_PKT_CNT  =23,
    PM_MSG_PSM_RESTART_P2P_PAUSE,
    PM_MSG_PSM_P2P_SLEEP,
    PM_MSG_PSM_P2P_AWAKE,
    PM_MSG_PSM_P2P_PS,
    PM_MSG_NULL_NOT_SLEEP,
    PM_MSG_DTIM_TMOUT_SLEEP,
    PM_MSG_SINGLE_BCN_RX_CNT    = 30,
    PM_MSG_BCN_TIMEOUT_SET_RX_CHAIN,       /* beacon超时后设置bcn rx chain的计数 */
    PM_MSG_COUNT = 32
}PM_DEBUG_MSG_TYPE;
#endif

typedef oal_uint8 dmac_vap_beacon_buffer_enum_uint8;

typedef enum
{
    DMAC_DBDC_START,
    DMAC_DBDC_STOP,
    DMAC_DBDC_STATE_BUTT
}dmac_dbdc_state_enum;
typedef oal_uint8 dmac_dbdc_state_enum_uint8;

typedef enum
{
    DMAC_STA_BW_SWITCH_FSM_INIT = 0,
    DMAC_STA_BW_SWITCH_FSM_NORMAL,
    DMAC_STA_BW_SWITCH_FSM_VERIFY20M,  //20m校验
    DMAC_STA_BW_SWITCH_FSM_VERIFY40M,  //40m校验
    DMAC_STA_BW_SWITCH_FSM_INVALID,
    DMAC_STA_BW_SWITCH_FSM_BUTT
}dmac_sta_bw_switch_fsm_enum;

typedef enum
{
    DMAC_STA_BW_VERIFY_20M_TO_40M = 0,
    DMAC_STA_BW_VERIFY_40M_TO_20M,
    DMAC_STA_BW_VERIFY_SWITCH_BUTT
}dmac_sta_bw_switch_type_enum;
typedef oal_uint8 dmac_sta_bw_switch_type_enum_enum_uint8;


#ifdef _PRE_WLAN_DFT_STAT
#define   DMAC_VAP_DFT_STATS_PKT_INCR(_member, _cnt)        ((_member) += (_cnt))
#define   DMAC_VAP_DFT_STATS_PKT_SET_ZERO(_member)        ((_member) = (0))
#else
#define   DMAC_VAP_DFT_STATS_PKT_INCR(_member, _cnt)
#define   DMAC_VAP_DFT_STATS_PKT_SET_ZERO(_member)
#endif
#define   DMAC_VAP_STATS_PKT_INCR(_member, _cnt)            ((_member) += (_cnt))

#define   DMAC_VAP_GET_HAL_CHIP(_pst_vap)                  (((dmac_vap_stru *)(_pst_vap))->pst_hal_chip)
#define   DMAC_VAP_GET_HAL_DEVICE(_pst_vap)                (((dmac_vap_stru *)(_pst_vap))->pst_hal_device)
#define   DMAC_VAP_GET_HAL_VAP(_pst_vap)                   (((dmac_vap_stru *)(_pst_vap))->pst_hal_vap)
#define   DMAC_VAP_GET_FAKEQ(_pst_vap)                     (((dmac_vap_stru *)(_pst_vap))->ast_tx_dscr_queue_fake)
#define   DMAC_VAP_GET_IN_TBTT_OFFSET(_pst_vap)            (((dmac_vap_stru *)(_pst_vap))->us_in_tbtt_offset)

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#define   DMAC_VAP_GET_BTCOEX_STATUS(_pst_vap)             (&(DMAC_VAP_GET_HAL_CHIP(_pst_vap)->st_btcoex_btble_status))
#define   DMAC_VAP_GET_BTCOEX_STATISTICS(_pst_vap)         (&(DMAC_VAP_GET_HAL_CHIP(_pst_vap)->st_btcoex_statistics))
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
#define   DMAC_VAP_GET_VAP_M2S(_pst_vap)                   (&(((dmac_vap_rom_stru *)(MAC_GET_DMAC_VAP(_pst_vap)->_rom))->st_dmac_vap_m2s))
#define   DMAC_VAP_GET_VAP_M2S_RX_STATISTICS(_pst_vap)     (&(DMAC_VAP_GET_VAP_M2S(_pst_vap)->st_dmac_vap_m2s_rx_statistics))
#define   DMAC_VAP_GET_VAP_M2S_ACTION_ORI_TYPE(_pst_vap)   (DMAC_VAP_GET_VAP_M2S(_pst_vap)->en_m2s_switch_ori_type)
#define   DMAC_VAP_GET_VAP_M2S_ACTION_SEND_STATE(_pst_vap) (DMAC_VAP_GET_VAP_M2S(_pst_vap)->en_action_send_state)
#define   DMAC_VAP_GET_VAP_M2S_ACTION_SEND_CNT(_pst_vap)   (DMAC_VAP_GET_VAP_M2S(_pst_vap)->uc_action_send_cnt)
#endif


#define   DMAC_VAP_GET_POW_INFO(_pst_vap)       &(((dmac_vap_stru *)(_pst_vap))->st_vap_pow_info)
#define   DMAC_VAP_GET_POW_TABLE(_pst_vap)      ((((dmac_vap_stru *)(_pst_vap))->st_vap_pow_info).pst_rate_pow_table)

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
/* vap tx参数，从架构分析文档获得成员 */
typedef struct
{
    oal_uint16  us_rts_threshold;       /* rts阈值 */
    oal_uint8   uc_mcast_rate;          /* 广播速率 */
    oal_uint8   auc_resv[1];            /* 字节对齐 */
}dmac_vap_tx_param_stru;

#ifdef _PRE_WLAN_FEATURE_ROAM

#define ROAM_TRIGGER_COUNT_THRESHOLD           (5)
#define ROAM_TRIGGER_RSSI_NE80_DB              (-80)
#define ROAM_TRIGGER_RSSI_NE75_DB              (-75)
#define ROAM_TRIGGER_RSSI_NE70_DB              (-70)
#define ROAM_TRIGGER_INTERVAL_10S              (10 * 1000)
#define ROAM_TRIGGER_INTERVAL_15S              (15 * 1000)
#define ROAM_TRIGGER_INTERVAL_20S              (20 * 1000)
#define ROAM_WPA_CONNECT_INTERVAL_TIME         (5 * 1000)    /* 漫游与正常关联之间的时间间隔，WIFI+ 上层切换频繁 */

typedef struct
{
    oal_int8    c_trigger_2G;           /* 2G漫游触发门限   */
    oal_int8    c_trigger_5G;           /* 5G漫游触发门限   */
    oal_uint8   auc_recv[2];
    oal_uint32  ul_cnt;                 /* 漫游触发器计数       */
    oal_uint32  ul_time_stamp;          /* 漫游触发器时间戳     */
    oal_uint32  ul_ip_obtain_stamp;     /* 上层获取IP地址时间戳 */
    oal_uint32  ul_ip_addr_obtained;    /* IP地址是否已获取 */
}dmac_vap_roam_trigger_stru;
#endif //_PRE_WLAN_FEATURE_ROAM

/*修改此结构体需要同步通知SDT，否则上报无法解析*/
typedef struct
{
    oal_int16    s_signal;           /* 驱动接收包记录的RSSI值 */
    oal_int16    s_free_power;       /*底噪*/

    oal_uint32   ul_drv_rx_pkts;     /* 驱动接收数据包数目 */
    oal_uint32   ul_hw_tx_pkts;      /* 发送完成中断上报发送成功的个数 */
    oal_uint32   ul_drv_rx_bytes;    /* 驱动接收字节数，不包括80211头尾 */
    oal_uint32   ul_hw_tx_bytes;     /* 发送完成中断上报发送成功字节数 */
    oal_uint32   ul_tx_retries;      /*发送重传次数*/
    oal_uint32   ul_rx_dropped_misc; /*接收失败次数*/
    oal_uint32   ul_tx_failed;     /* 发送失败次数，仅仅统计数据帧 */
//    oal_rate_info_stru st_txrate; /*vap当前速率*/
    oal_int8     c_snr_ant0;         /* 天线0上上报的SNR */
    oal_int8     c_snr_ant1;         /* 天线1上上报的SNR */
    oal_uint8    auc_reserv[2];

    /*维测需要增加较多的维测，使用维测预编译宏包着*/
#ifdef _PRE_WLAN_DFT_STAT
    /***************************************************************************
                                接收包统计
    ***************************************************************************/

    /* 接收流程遇到严重错误(描述符异常等)，释放所有MPDU的统计 */
    oal_uint32  ul_rx_ppdu_dropped;                             /* 硬件上报的vap_id错误，释放的MPDU个数 */

    /* 软件统计的接收到MPDU个数，正常情况下应该与MAC硬件统计值相同 */
    oal_uint32  ul_rx_mpdu_total_num;                           /* 接收流程上报到软件进行处理的MPDU总个数 */

    /* MPDU级别进行处理时发生错误释放MPDU个数统计 */
    oal_uint32  ul_rx_ta_check_dropped;                         /* 检查发送端地址异常，释放一个MPDU */
    oal_uint32  ul_rx_key_search_fail_dropped;                  /*  */
    oal_uint32  ul_rx_tkip_mic_fail_dropped;                    /* 接收描述符status为 tkip mic fail释放MPDU */
    oal_uint32  ul_rx_replay_fail_dropped;                      /* 重放攻击，释放一个MPDU */
    oal_uint32  ul_rx_security_check_fail_dropped;              /* 加密检测失败*/
    oal_uint32  ul_rx_alg_process_dropped;                      /* 算法处理返回失败 */
    oal_uint32  ul_rx_null_frame_dropped;                       /* 接收到空帧释放(之前节能特性已经对其进行处理) */
    oal_uint32  ul_rx_abnormal_dropped;                         /* 其它异常情况释放MPDU */
    oal_uint32  ul_rx_mgmt_mpdu_num_cnt;                         /* 接收到的管理帧和控制帧统计*/
    oal_uint32  ul_rx_mgmt_abnormal_dropped;                    /* 接收到管理帧出现异常，比如vap或者dev为空等 */

    /***************************************************************************
                                发送包统计
    ***************************************************************************/
    oal_uint32  ul_drv_tx_pkts;     /* 驱动发包数目，交给硬件的数目 */
    oal_uint32  ul_drv_tx_bytes;    /* 驱动发包字节数，不算80211头尾 */
    /* 发送流程发生异常导致释放的数据包统计，MSDU级别 */
    oal_uint32  ul_tx_abnormal_mpdu_dropped;                    /* 异常情况释放MPDU个数，指vap或者user为空等异常 */
    /* 发送完成中发送成功与失败的数据包统计，MPDU级别 */
    oal_uint32  ul_tx_mpdu_succ_num;                            /* 发送MPDU总个数 */
    oal_uint32  ul_tx_mpdu_fail_num;                            /* 发送MPDU失败个数 */
    oal_uint32  ul_tx_ampdu_succ_num;                           /* 发送成功的AMPDU总个数 */
    oal_uint32  ul_tx_mpdu_in_ampdu;                            /* 属于AMPDU的MPDU发送总个数 */
    oal_uint32  ul_tx_ampdu_fail_num;                           /* 发送AMPDU失败个数 */
    oal_uint32  ul_tx_mpdu_fail_in_ampdu;                       /* 属于AMPDU的MPDU发送失败个数 */
    oal_uint32  aul_tx_count_per_apmpdu_length[DMAC_COUNT_BY_AMPDU_LENGTH_INDEX_BUTT];/*针对不同聚合长度的帧统计发送的个数*/
    oal_uint32  ul_tx_cts_fail;                                  /*发送rts失败的统计*/
    oal_uint8   uc_tx_successive_mpdu_fail_num;                  /*连续发送失败的统计*/
    oal_uint8   uc_reserve[3];                                   /*保留字节*/
#endif
    oal_uint8   _rom[4];
}dmac_vap_query_stats_stru;

typedef oal_uint8 dmac_linkloss_status_enum_uint8;

typedef struct
{
    oal_uint8                     uc_linkloss_threshold;       /*  LinkLoss门限  */
    oal_uint8                     uc_link_loss;                /*  LinkLoss计数器 */
    oal_uint8                     auc_resv[2];
}vap_linkloss_stru;

typedef struct
{
    vap_linkloss_stru             ast_linkloss_info[WLAN_LINKLOSS_MODE_BUTT];   /* linkloss计数结构*/

    oal_uint8   auc_int_linkloss_threshold[WLAN_LINKLOSS_MODE_BUTT];            /* 各个场景门限beacon计算初始值 */
    oal_uint8   auc_min_linkloss_threshold[WLAN_LINKLOSS_MODE_BUTT];            /* 各个场景门限最小值 */
    oal_uint8   auc_linkloss_threshold_decr[WLAN_LINKLOSS_MODE_BUTT];           /* 接收或者发送失败门限减值*/

    oal_uint32                                ul_dot11BeaconPeriod;             /* 记录dot11BeaconPeriod是否变化*/

    oal_uint8                                 uc_linkloss_times;                /* 记录linkloss当前门限值相对于正常的倍数，反映beacon的周期倍数 */
    wlan_linkloss_mode_enum_uint8             en_linkloss_mode;                 /*  linkloss场景*/
    oal_bool_enum_uint8                       en_roam_scan_done;                /* 记录是否发生过漫游扫描*/
    oal_bool_enum_uint8                       en_vowifi_report;                 /* 记录是否上报vowifi状态切换请求*/

    oal_uint8                                 uc_allow_send_probe_req_cnt;      /* 发送过prob req的次数*/
    oal_uint8                                 auc_resv[3];
}dmac_vap_linkloss_stru;

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
#define DMAC_MAX_IPV4_ENTRIES         8
#define DMAC_MAX_IPV6_ENTRIES         8

typedef union
{
    oal_uint32                    ul_value;
    oal_uint8                     auc_value[OAL_IPV4_ADDR_SIZE];
}un_ipv4_addr;

typedef struct
{
    un_ipv4_addr        un_local_ip;
    un_ipv4_addr        un_mask;
}dmac_vap_ipv4_addr_stru;

typedef struct
{
    oal_uint8                         auc_ip_addr[OAL_IPV6_ADDR_SIZE];
}dmac_vap_ipv6_addr_stru;

typedef struct
{
    dmac_vap_ipv4_addr_stru           ast_ipv4_entry[DMAC_MAX_IPV4_ENTRIES];
    dmac_vap_ipv6_addr_stru           ast_ipv6_entry[DMAC_MAX_IPV6_ENTRIES];
}dmac_vap_ip_entries_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
typedef struct
{
    oal_uint16  aus_last_qos_seq_num[WLAN_TID_MAX_NUM];         /* 记录该Proxy STA保存的接收来自Root Ap的前一个QoS帧的seq num */
    oal_uint16  us_non_qos_seq_num;                             /* 记录该Proxy STA保存的接收来自Root Ap的前一个非QOS帧的seq num */
    oal_uint8   uc_lut_idx;
    oal_uint8   auc_resv[1];
} dmac_vap_psta_stru;
#define dmac_vap_psta_lut_idx(vap)   ((vap)->st_psta.uc_lut_idx)
#endif

typedef struct
{
    oal_fsm_stru                         st_oal_fsm;                   /* 状态机 */
    oal_bool_enum_uint8                  en_is_fsm_attached;           /* 状态机是否已经注册 */
    oal_uint8                            uc_20M_Verify_fail_cnt;
    oal_uint8                            uc_40M_Verify_fail_cnt;
    oal_uint8                            auc_rsv[1];
}dmac_sta_bw_switch_fsm_info_stru;

#ifdef _PRE_WLAN_FEATURE_FTM
typedef struct
{
    oal_uint64      ull_t1;
    oal_uint64      ull_t2;
    oal_uint64      ull_t3;
    oal_uint64      ull_t4;
    oal_uint8       uc_dialog_token;
    oal_uint8       uc_resv[3];
}ftm_timer_stru;

typedef struct
{
    oal_uint32                          ul_measurement_start_time;
    oal_uint8                           auc_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint32                          bit_range                     :24,
                                        bit_max_range_error_exponent  :8;
    oal_uint8                           auc_reserve[1];
} ftm_range_entry_stru;

typedef struct
{
    oal_uint32                          ul_measurement_start_time;
    oal_uint8                           auc_bssid[WLAN_MAC_ADDR_LEN];
    oal_uint8                           uc_error_code;
} ftm_error_entry_stru;

typedef struct
{
    oal_uint8                           uc_range_entry_count;
    ftm_range_entry_stru                ast_ftm_range_entry[DMAX_FTM_RANGE_ENTRY_COUNT];
    oal_uint8                           uc_error_entry_count;
    ftm_error_entry_stru                ast_ftm_error_entry[DMAX_FTM_ERROR_ENTRY_COUNT];
    oal_uint8                           auc_reserve[2];
} ftm_range_report_stru;

typedef struct
{
    oal_uint16                          us_start_delay;
    oal_uint8                           auc_reserve[1];
    oal_uint8                           uc_minimum_ap_count;
    oal_uint8                           auc_bssid[DMAX_MINIMUN_AP_COUNT][WLAN_MAC_ADDR_LEN];
    oal_uint8                           auc_channel[DMAX_MINIMUN_AP_COUNT];
} ftm_range_request_stru;

typedef struct
{
    oal_void        *pst_dmac_vap;
    oal_uint8        uc_session_id;
}ftm_timeout_arg_stru;

typedef struct
{
    oal_uint16                          us_burst_cnt;                                    /*回合个数*/
    oal_uint8                           uc_ftms_per_burst;                               /*每个回合FTM帧的个数*/
    oal_uint8                           uc_burst_duration;                               /*回合持续时间*/

    oal_uint16                          us_burst_period;                                 /*回合之间的间隔，单位100ms*/
    oal_bool_enum_uint8                 en_ftm_initiator;                                /*STA that supports fine timing measurement as an initiator*/
    oal_bool_enum_uint8                 en_asap;                                         /*指示 as soon as posible*/

    wlan_phy_protocol_enum_uint8        en_prot_format;                                  /*指示 协议类型*/
    wlan_bw_cap_enum_uint8              en_band_cap;                                     /*指示 带宽*/
    oal_bool_enum_uint8                 en_lci_ie;                                       /*指示 携带LCI Measurement request/response elements */
    oal_bool_enum_uint8                 en_location_civic_ie;                            /*指示 携带Location Civic Measurement request/response elements */

    oal_uint8                           uc_ftm_entry_count;                              /*需要与多少个AP交互*/
    oal_uint8                           en_report_range;
    oal_uint16                          us_partial_tsf_timer;

    oal_uint32                          ul_tsf_sync_info;
    frw_timeout_stru                    st_ftm_tsf_timer;                                /* ftm回合开始定时器 */
    mac_channel_stru                    st_channel_ftm;                                  /* FTM交互所在信道*/
    mac_channel_stru                    st_channel_work;                                 /* 工作信道*/

    oal_uint8                           auc_bssid[WLAN_MAC_ADDR_LEN];                    /* FTM交互AP的BSSID*/
    oal_bool_enum_uint8                 en_iftmr;
    oal_bool_enum_uint8                 en_ftm_trigger;

    ftm_range_report_stru               st_ftm_range_report;                             /* FTM测量结果*/
    ftm_range_request_stru              st_ftm_range_request;                            /* FTM测量要求*/

    oal_uint32                          bit_range                    :24,
                                        bit_max_range_rrror_rxponent :8;

    oal_uint8                           uc_dialog_token;
    oal_uint8                           uc_follow_up_dialog_token;
    oal_bool_enum_uint8                 en_cali;
    oal_uint8                           uc_ftms_per_burst_cmd;                          /*命令设置每个回合FTM帧的个数*/

    oal_uint32                          ul_ftm_cali_time;
    ftm_timer_stru                      ast_ftm_timer[MAC_FTM_TIMER_CNT];
    ftm_timeout_arg_stru                st_arg;
} dmac_ftm_initiator_stru;
typedef struct
{
    oal_uint8                           uc_ftms_per_burst;                               /*每个回合FTM帧的个数*/
    oal_uint8                           uc_ftms_per_burst_save;
    oal_uint16                          us_burst_cnt;                                    /*回合个数*/

    wlan_phy_protocol_enum_uint8        uc_prot_format;                                  /*指示 协议类型*/
    wlan_bw_cap_enum_uint8              en_band_cap;                                     /*指示 带宽*/
    oal_uint8                           auc_resv[2];

    mac_channel_stru                    st_channel_ftm;                                  /* FTM交互所在信道*/

    oal_bool_enum_uint8                 en_received_iftmr;
    oal_uint8                           uc_min_delta_ftm;                                 /* 单位100us*/
    oal_bool_enum_uint8                 en_ftm_responder;                                /*STA that supports fine timing measurement as an responder*/
    oal_bool_enum_uint8                 en_asap;                                         /*指示 as soon as posible*/

    frw_timeout_stru                    st_ftm_tsf_timer;                                /* ftm回合开始定时器 */
    frw_timeout_stru                    st_ftm_all_burst;                                /* ftm整个回合时器 */

    oal_uint32                          bit_range                    :24,
                                        bit_max_range_rrror_rxponent :8;

    oal_uint8                           auc_mac_ra[WLAN_MAC_ADDR_LEN];
    oal_uint8                           uc_dialog_token;
    oal_uint8                           uc_follow_up_dialog_token;

    oal_bool_enum_uint8                 en_lci_ie;
    oal_bool_enum_uint8                 en_location_civic_ie;
    oal_bool_enum_uint8                 en_ftm_parameters;
    oal_bool_enum_uint8                 en_ftm_synchronization_information;

    oal_uint64                          ull_tod;
    oal_uint64                          ull_toa;

    oal_uint16                          us_tod_error;
    oal_uint16                          us_toa_error;

    oal_uint32                          ul_tsf;

    oal_uint8                           uc_dialog_token_ack;
    oal_uint8                           uc_burst_duration;                               /*回合持续时间*/
    oal_uint16                          us_burst_period;                                 /*回合之间的间隔，单位100ms*/

    ftm_timer_stru                      ast_ftm_timer[MAC_FTM_TIMER_CNT];

    ftm_timeout_arg_stru                st_arg;
} dmac_ftm_responder_stru;

typedef struct
{
    dmac_ftm_initiator_stru            ast_ftm_init[MAX_FTM_SESSION];
    dmac_ftm_responder_stru            ast_ftm_rsp[MAX_FTM_SESSION];
}dmac_ftm_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_11K
typedef struct mac_rrm_info_tag
{
    mac_action_rm_rpt_stru              *pst_rm_rpt_action;
    mac_meas_rpt_ie_stru                *pst_meas_rpt_ie;           /* Measurement Report IE Addr */
    mac_bcn_rpt_stru                    *pst_bcn_rpt_item;          /* Beacon Report Addr */
    oal_netbuf_stru                     *pst_rm_rpt_mgmt_buf;       /* Report Frame Addr for Transfer */
    mac_scan_req_stru                   *pst_scan_req;

    oal_uint8                            uc_quiet_count;
    oal_uint8                            uc_quiet_period;
    oal_mac_quiet_state_uint8            en_quiet_state;
    oal_uint8                            uc_link_dialog_token;

    oal_uint8                            uc_ori_max_reg_pwr;
    oal_uint8                            uc_local_pwr_constraint;
    oal_uint8                            uc_ori_max_pwr_flush_flag;
    oal_uint8                            uc_rsv;

    oal_int8                             c_link_tx_pwr_used;
    oal_int8                             c_link_max_tx_pwr;
    oal_uint32                           aul_act_meas_start_time[2];
    oal_uint16                           us_quiet_duration;

    oal_uint16                           us_quiet_offset;
    oal_uint16                           us_rm_rpt_action_len;      /* Report Frame Length for Transfer */

    oal_dlist_head_stru                  st_meas_rpt_list;
    mac_bcn_req_info_stru                st_bcn_req_info;
    frw_timeout_stru                     st_quiet_timer;    /* quiet定时器，每次进入quiet时启动，quiet duration后超时处理退出quiet */
    frw_timeout_stru                     st_offset_timer;   /* 最后一个tbtt中断开始时启动offset定时器，offset时间后超时处理进入quiet，并启动quiet定时器 */

    oal_uint8                            _rom[4];
}mac_rrm_info_stru;
#endif //_PRE_WLAN_FEATURE_11K

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING

//vap级别的band steering特性数据结构
typedef struct{
    oal_uint8               uc_enable;              //表示该vap是否是steering模式的vap
    oal_uint8               auc_reserv[3];
}dmac_vap_bsd_stru;

#endif

#ifdef _PRE_WLAN_FEATURE_VOWIFI
typedef struct
{
    oal_uint8           uc_rssi_trigger_cnt;        /* 统计信号质量连续超出门限的次数 【1，100】*/
    oal_uint8           auc_resv[3];
    oal_uint64          ull_rssi_timestamp_ms;     /* 时间戳，ms级别 */
    oal_uint64          ull_arp_timestamp_ms;      /* arp req device侧探测失败(tx 5s后无任何数据接收) */

    oal_uint32   ul_arp_rx_succ_pkts;  /* TX arp_req时刻记录的当时rx_succ_pkts值 */
    oal_uint32   ul_tx_failed;         /* 发送失败最终丢弃的次数，仅仅统计数据帧  */
    oal_uint32   ul_tx_total;          /* 发送总计，仅仅统计数据帧  */

    oal_uint8    _rom[4];
}mac_vowifi_status_stru;

#endif /* _PRE_WLAN_FEATURE_VOWIFI */

/* STA 侧pm管理结构定义*/
#ifdef _PRE_WLAN_FEATURE_STA_PM
typedef struct
{
    oal_uint8    en_active_null_wait :1;   /* STA发送NULL帧给AP提示进入ACTIVE 状态 */
    oal_uint8    en_doze_null_wait   :1;   /* STA发送NULL帧给AP提示进入doze状态*/
    oal_uint8    bit_resv            :6;
} dmac_vap_null_wait_stru;

#ifdef _PRE_DEBUG_MODE
typedef struct
{
    oal_uint32 ul_wmmpssta_tacdl;       /* Trigger enabled Downlink traffic exception.          */
    oal_uint32 ul_wmmpssta_dacul;        /* Delivery enabled Uplink traffic exception.             */
    oal_uint32 ul_wmmpssta_spsw;        /* Wait for service period start.                       */
    oal_uint32 ul_wmmpssta_sps;         /* Service period start                                 */
    oal_uint32 ul_wmmpssta_spe;         /* Service period end                                   */
    oal_uint32 ul_wmmpssta_trigsp;      /* Trigger service period                               */
    oal_uint32 ul_wmmpssta_trspnr;      /* Trigger service period is not required               */
}dmac_wmmps_info_stru;
#elif defined(_PRE_PSM_DEBUG_MODE)
#endif

typedef struct _mac_sta_pm_handler
{
    oal_fsm_stru                    st_oal_fsm;                                  /*节能状态机*/
    frw_timeout_stru                st_inactive_timer;                          /* 定时器 */
    frw_timeout_stru                st_mcast_timer;                             /* 接收广播组播超时定时器 */

#ifdef _PRE_WLAN_DOWNLOAD_PM
    oal_uint32                      ul_rx_cnt ;                                 /*wifi下载功耗测试版本，接收报文单独统计限速*/
#endif
    oal_uint32                      ul_tx_rx_activity_cnt;                      /* ACTIVE统计值，由超时进入DOZE复位 */
    oal_uint32                      ul_activity_timeout;                        /* 睡眠超时定时器超时时间 */
    oal_uint32                      ul_ps_keepalive_cnt;                        /* STA侧节能状态下keepalive机制统计接收beacon数*/
    oal_uint32                      ul_ps_keepalive_max_num;                    /* STA侧节能状态下keepalive机制最大接收beacon数 */

    oal_uint8                       uc_vap_ps_mode;                             /*  sta当前省电模式 */

    oal_uint8                       en_beacon_frame_wait            :1;         /* 提示接收beacon帧 */
    oal_uint8                       en_more_data_expected           :1;         /* 提示AP中有更多的缓存帧 */
    oal_uint8                       en_send_null_delay              :1;         /* 延迟发送NULL帧开关 */
    oal_uint8                       bit_resv                        :1;
    oal_uint8                       en_direct_change_to_active      :1;         /* FAST模式下直接唤醒的数据包切active状态 */
    oal_uint8                       en_hw_ps_enable                 :1;         /* 开启全系统低功耗/协议栈低功耗 */
    oal_uint8                       en_ps_back_active_pause         :1;         /* ps back 延迟发送唤醒null帧 */
    oal_uint8                       en_ps_back_doze_pause           :1;         /* ps back 延迟发送睡眠null帧 */

    oal_uint8                       uc_timer_fail_doze_trans_cnt;               /* 超时函数内发null切doze失败次数 */
    oal_uint8                       uc_state_fail_doze_trans_cnt;               /* 切doze时,由于条件不满足失败计数 */

    oal_uint8                       uc_beacon_fail_doze_trans_cnt;              /* 收beacon 投票睡眠却却无法睡下去的计数 */
    oal_uint8                       uc_doze_event;                              /* 记录切状态的事件类型 */
    oal_uint8                       uc_awake_event;
    oal_uint8                       uc_active_event;

#ifdef _PRE_WLAN_FEATURE_STA_UAPSD
    oal_uint8                       uc_eosp_timeout_cnt;                        /* uapsd省电中TBTT计数器 */
    oal_uint8                       uc_uaspd_sp_status;                         /* UAPSD的状态 */
#endif
    oal_uint8                       uc_doze_null_retran_cnt;
    oal_uint8                       uc_active_null_retran_cnt;

    dmac_vap_null_wait_stru         st_null_wait;                               /* STA发送NULL帧切换状态时，各状态NULL帧等待状态的结构体*/
#ifdef _PRE_DEBUG_MODE
    dmac_wmmps_info_stru            st_wmmps_info;                              /* STA侧uapsd的维测信息 */
#endif
    oal_uint32                      aul_pmDebugCount[PM_MSG_COUNT];
    oal_uint32                      ul_psm_pkt_cnt;
    oal_uint8                       uc_psm_timer_restart_cnt;                   /* 重启睡眠定时器的count */
    oal_uint8                       uc_can_sta_sleep;                           /* 协议允许切到doze,是否能投票睡眠 */
    oal_uint16                      us_mcast_timeout;                           /* 接收广播组播定时器超时时间 */
    oal_bool_enum_uint8             en_is_fsm_attached;                         /*状态机是否已经注册*/
    oal_bool_enum_uint8             en_beacon_counting;
    oal_uint8                       uc_max_skip_bcn_cnt;                        /* 最大允许跳过beacon次数 */
    oal_uint8                       uc_tbtt_cnt_since_full_bcn;                 /* 距离上次接收完整beacon的tbtt cnt计数 */
    oal_uint8                       _rom[2];
} mac_sta_pm_handler_stru;
#endif

/*pm管理结构定义*/
#ifdef _PRE_WLAN_FEATURE_AP_PM
typedef struct _mac_pm_handler{
    oal_fsm_stru         st_oal_fsm;               /*节能状态机*/
    oal_uint32           ul_pwr_arbiter_id;        /*arbiter id*/
    oal_uint32           bit_pwr_sleep_en:1,       /*自动睡眠使能*/
                         bit_pwr_wow_en:1,         /*wow功能使能*/
                         bit_pwr_siso_en:1,        /*自动单通道模式使能,在没有用户或者流量较低时切换*/
                         bit_rsv:29;
    oal_uint32           ul_max_inactive_time;     /*work态下最大不活跃时间*/
    oal_uint32           ul_inactive_time;         /*持续不活跃时间*/
    oal_uint32           ul_user_check_count;      /*无用户检查次数*/
    oal_uint32           ul_idle_beacon_txpower;   /*idle态下beaocn txpower*/
    frw_timeout_stru     st_inactive_timer;        /*活跃检测定时器,同时复用为是否有用户的检查定时器*/
    oal_bool_enum_uint8  en_is_fsm_attached;       /*状态机是否已经注册*/
    oal_uint8            _rom[4];
} mac_pm_handler_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
typedef struct
{
    frw_timeout_stru bt_coex_low_rate_timer;
    frw_timeout_stru bt_coex_statistics_timer;
    frw_timeout_stru bt_coex_sco_statistics_timer;
    oal_bool_enum_uint8 en_rx_rate_statistics_flag;
    oal_bool_enum_uint8 en_rx_rate_statistics_timeout;
    oal_bool_enum_uint8 en_sco_rx_rate_statistics_flag;
    oal_uint8 uc_resv[1];
} dmac_vap_btcoex_rx_statistics_stru;

typedef struct
{
    frw_timeout_stru bt_coex_priority_timer;                 /* 读取寄存器周期定时器 */
    frw_timeout_stru bt_coex_occupied_timer;                 /* 周期拉高occupied信号线，保证WiFi不被BT抢占 */
    oal_uint32 ul_ap_beacon_count;
    oal_uint32 ul_timestamp;
    oal_uint8 uc_beacon_miss_cnt;
    oal_uint8 uc_occupied_times;
    oal_uint8 uc_prio_occupied_state;
    oal_uint8 uc_linkloss_occupied_times;
    oal_uint8 uc_linkloss_index;
    oal_uint8 auc_resv[3];
} dmac_vap_btcoex_occupied_stru;

typedef struct
{
    oal_uint8                            auc_null_qosnull_frame[32];  /* null&qos null,取最大长度 */
    oal_uint16                           bit_cfg_coex_tx_vap_index     :4;     /* 03新增premmpt帧配置参数 */
    oal_uint16                           bit_cfg_coex_tx_qos_null_tid  :4;
    oal_uint16                           bit_rsv                       :3;
    oal_uint16                           bit_cfg_coex_tx_peer_index    :5;
} dmac_vap_btcoex_null_preempt_stru;

typedef struct
{
    dmac_vap_btcoex_rx_statistics_stru   st_dmac_vap_btcoex_rx_statistics;
    dmac_vap_btcoex_occupied_stru        st_dmac_vap_btcoex_occupied;
    dmac_vap_btcoex_null_preempt_stru    st_dmac_vap_btcoex_null_preempt_param;
    hal_coex_hw_preempt_mode_enum_uint8  en_all_abort_preempt_type;
    oal_uint8                            _rom[4];
} dmac_vap_btcoex_stru;
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
typedef struct
{
    frw_timeout_stru      m2s_delay_switch_statistics_timer;
    oal_uint16            us_rx_nss_mimo_count;
    oal_uint16            us_rx_nss_siso_count;
    oal_uint16            us_rx_nss_ucast_count;
    oal_bool_enum_uint8   en_rx_nss_statistics_start_flag;
} dmac_vap_m2s_rx_statistics_stru;

typedef struct
{
    dmac_vap_m2s_rx_statistics_stru   st_dmac_vap_m2s_rx_statistics;    /* STA当前切换初始模式不满足要求时，开启统计变量 */
    hal_m2s_action_type_uint8         en_m2s_switch_ori_type;           /* STA当前关联ap的切换初始模式 */
    oal_bool_enum_uint8               en_action_send_state;             /* STA action帧发送是否成功 */
    oal_uint8                         uc_action_send_cnt;
} dmac_vap_m2s_stru;
#endif

typedef enum
{
    DMAC_BEACON_TX_POLICY_SINGLE    = 0,
    DMAC_BEACON_TX_POLICY_SWITCH    = 1,
    DMAC_BEACON_TX_POLICY_DOUBLE    = 2,
}dmac_beacon_tx_policy_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT

#define DMAC_SENSING_BSSID_LIST_MAX_MEMBER_CNT  16

typedef enum mac_sensing_bssid_operate
{
    OAL_SENSING_BSSID_OPERATE_DEL = 0,
    OAL_SENSING_BSSID_OPERATE_ADD = 1,

    OAL_SENSING_BSSID_OPERATE_BUTT
} mac_sensing_bssid_operate_en;
typedef oal_uint8 oal_en_sensing_bssid_operate_uint8;

typedef struct mac_sensing_bssid
{
    oal_uint8  auc_mac_addr[OAL_MAC_ADDR_LEN];
    oal_en_sensing_bssid_operate_uint8  en_operation;   /** 0删除， 1添加 */
    oal_uint8  reserved;
} dmac_sensing_bssid_cfg_stru;

typedef struct
{
    oal_spin_lock_stru     st_lock;
    oal_dlist_head_stru    st_list_head;
    oal_uint8              uc_member_nums;
    oal_uint8              auc_reserve[3];
} dmac_sensing_bssid_list_stru;

typedef struct
{
    oal_dlist_head_stru  st_dlist;
    oal_uint8            auc_mac_addr[WLAN_MAC_ADDR_LEN];   /*对应的MAC地址 */
    oal_int8             c_rssi;
    oal_uint8            uc_reserved;
    oal_uint32           ul_timestamp;
} dmac_sensing_bssid_list_member_stru;

typedef struct
{
    oal_uint8            auc_mac_addr[WLAN_MAC_ADDR_LEN];   /*对应的MAC地址 */
    oal_int8             c_rssi;
    oal_uint8            uc_reserved;
    oal_uint32           ul_timestamp;
} dmac_query_sensing_bssid_stru;

#endif

typedef struct
{
#ifdef _PRE_WLAN_FEATURE_M2S
    dmac_vap_m2s_stru     st_dmac_vap_m2s;
#else
    oal_uint8             auc_resv[4];
#endif
}dmac_vap_rom_stru;
extern dmac_vap_rom_stru g_dmac_vap_rom[];

/* dmac vap */
typedef struct dmac_vap_tag
{
    mac_vap_stru                     st_vap_base_info;                                  /* 基本VAP信息 */

    oal_uint32                       ul_active_id_bitmap;                               /* 活跃user的bitmap */

    oal_uint8                       *pauc_beacon_buffer[DMAC_VAP_BEACON_BUFFER_BUTT];   /* VAP下挂两个beacon帧 */
    oal_uint8                        uc_beacon_idx;                                     /* 当前放入硬件beacon帧索引值 */
    oal_uint8                        uc_rsv1;
    oal_uint16                       us_beacon_len;                                     /* beacon帧的长度 */

    hal_to_dmac_vap_stru            *pst_hal_vap;                                       /* hal vap结构 */
    hal_to_dmac_device_stru         *pst_hal_device;                                    /* hal device结构体以免二次获取 */
    hal_to_dmac_chip_stru           *pst_hal_chip;                                      /* hal chip结构体以免二次获取 */

    dmac_vap_linkloss_stru           st_linkloss_info;                                  /* linkloss机制相关信息 */

    oal_bool_enum_uint8              en_is_host_vap;                                    /* TRUE:主VAP，FALSE:从VAP */
    oal_uint8                        uc_default_ant_bitmap;                             /* 默认天线组合bitmap, 可以填写到描述符中 */
    oal_uint8                        uc_sw_retry_limit;
    oal_uint8                        en_multi_user_multi_ac_flag;			            /* 多用户多优先级时是否使能拥塞控制*/

    oal_traffic_type_enum_uint8      uc_traffic_type;                                   /* 业务类型，是否有多用户多优先级 */
    oal_uint8                        uc_sta_pm_open_by_host;                            /* sta 低功耗状态: HOST侧是否打开了低功耗 */
    oal_uint8                        uc_cfg_pm_mode;                                    /* 手动挡保存的低功耗模式 */
    oal_uint8                        uc_sta_pm_close_status;                            /* sta 低功耗状态, 包含多个模块的低功耗控制信息 */


#ifdef _PRE_WLAN_FEATURE_WEB_CFG_FIXED_RATE
    hal_tx_txop_alg_stru             st_tx_alg_vht;                                     /* VHT单播数据帧发送参数 */
    hal_tx_txop_alg_stru             st_tx_alg_ht;                                      /* HT单播数据帧发送参数 */
    hal_tx_txop_alg_stru             st_tx_alg_11ag;                                    /* 11a/g单播数据帧发送参数 */
    hal_tx_txop_alg_stru             st_tx_alg_11b;                                     /* 11b单播数据帧发送参数 */

    union
    {
        oal_uint8                    uc_mode_param_valid;                               /* 是否有针对特定模式的单播数据帧参数配置生效(0=无, 大于0=有) */
        struct{
            oal_uint8                bit_vht_param_vaild  : 1;                          /* VHT单播数据帧参数配置是否生效(0=不生效, 1=生效) */
            oal_uint8                bit_ht_param_vaild   : 1;                          /* HT单播数据帧参数配置是否生效(0=不生效, 1=生效) */
            oal_uint8                bit_11ag_param_vaild : 1;                          /* 11a/g单播数据帧参数配置是否生效(0=不生效, 1=生效) */
            oal_uint8                bit_11b_param_vaild  : 1;                          /* 11b单播数据帧参数配置是否生效(0=不生效, 1=生效) */
            oal_uint8                bit_reserve          : 4;
        }st_spec_mode;
    }un_mode_valid;
    oal_uint8                        auc_resv1[3];
#endif

    hal_tx_txop_alg_stru             st_tx_alg;                                         /* 单播数据帧发送参数 */
    hal_tx_txop_alg_stru             st_tx_data_mcast;                                  /* 组播数据帧参数 */
    hal_tx_txop_alg_stru             st_tx_data_bcast;                                  /* 广播数据帧参数*/
    hal_tx_txop_alg_stru             ast_tx_mgmt_ucast[WLAN_BAND_BUTT];                 /* 单播管理帧参数*/
    hal_tx_txop_alg_stru             ast_tx_mgmt_bmcast[WLAN_BAND_BUTT];                /* 组播、广播管理帧参数*/

    oal_void                        *p_alg_priv;                                        /* VAP级别算法私有结构体 */

    oal_uint8                       *puc_tim_bitmap;                                    /* 本地保存的tim_bitmap，AP模式有效 */
    oal_uint8                        uc_tim_bitmap_len;
    oal_uint8                        uc_ps_user_num;                                    /* 处于节能模式的用户的数目，AP模式有效 */
    oal_uint8                        uc_dtim_count;
    oal_uint8                        uc_uapsd_max_depth;                                /* U-APSD节能队列的最大深度*/
    dmac_beacon_tx_policy_enum_uint8 en_beacon_tx_policy;                               /* beacon发送策略, 0-单通道, 1-双通道轮流(如果有),2-双通道(如果有)*/
    oal_bool_enum_uint8              en_dtim_ctrl_bit0;                                 /* 用于表示DTIM Control字段的比特0是否置1了 */
    /* 重传次数上限 */
    oal_uint8                        bit_bw_cmd:1,                                      /* 是否使能配置数据带宽的命令 0:No  1:Yes */
                                     bit_beacon_timeout_times:7;                        /* sta等待beacon超时计数 */

#if defined(_PRE_WLAN_FEATURE_AP_PM) || defined(_PRE_WLAN_FEATURE_STA_PM)
    oal_netbuf_stru                 *pst_wow_probe_resp;                                /* wow使能时,准备的probe response帧*/
    oal_netbuf_stru                 *pst_wow_null_data;                                 /* wow使能时,准备的null data帧,STA模式时采用*/
    oal_uint16                       us_wow_probe_resp_len;
    oal_uint8                        auc_resv2[1];
#endif
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    dmac_vap_psta_stru               st_psta;
#endif

    oal_uint32                       ul_obss_scan_timer_remain;       /* 02最大定时器超时65s, OBSS扫描定时器可能为几分钟，通过计数器来实现大定时器*/
    oal_uint8                        uc_obss_scan_timer_started;
    oal_uint8                        uc_bcn_tout_max_cnt;             /* beacon连续收不到最大睡眠次数 */
#ifdef _PRE_WLAN_FEATURE_STA_PM
    oal_uint8                        uc_null_frm_ofdm_succ_cnt;
    oal_uint8                        uc_null_frm_cnt;
#else
    oal_uint8                        uac_resv3[2];
#endif  /* _PRE_WLAN_FEATURE_STA_PM */

    wlan_channel_bandwidth_enum_uint8 en_40M_bandwidth;               /* 记录ap在切换到20M之前的带宽 */
    oal_uint8                         uc_ps_poll_pending;
#ifdef _PRE_WLAN_FEATURE_MAC_PARSE_TIM
    oal_uint16                        us_tim_pos;                     /* beacon帧中TIM IE相对Payload的偏移, 用于MAC解析 */
#else
    oal_uint8                         uac_resv4[2];
#endif

    /* 常发测试使用 */
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    oal_uint8                        uc_protocol_rate_dscr;           /* 发送描述符中协议与速率位组合值，用于常发模式下更改帧长 */
    oal_uint8                        uc_bw_flag;                      /* 发送描述符中40M标志 */
    oal_uint8                        uc_short_gi;                     /* short gi是否使能 */
#endif
#ifdef _PRE_WLAN_FEATURE_TSF_SYNC
    oal_uint8                        uc_beacon_miss_cnt;              /* beacon miss 计数 */
#else
    oal_uint8                        uac_resv5[1];
#endif

#if(_PRE_WLAN_FEATURE_PMF == _PRE_PMF_HW_CCMP_BIP)
    oal_uint32                       ul_user_pmf_status;              /* 记录此vap下user pmf使能的情况，供控制硬件vap是否打开加解密开关 */
#endif

#if defined (_PRE_WLAN_FEATURE_STA_PM)
    mac_sta_pm_handler_stru          st_sta_pm_handler;               /* sta侧pm管理结构定义 */
#endif

#if defined(_PRE_WLAN_FEATURE_AP_PM)
    mac_pm_handler_stru              st_pm_handler;                   /* ap侧pm管理结构定义 */
#endif
    /*统计信息+信息上报新增字段，修改这个字段，必须修改SDT才能解析正确*/
    dmac_vap_query_stats_stru        st_query_stats;

    hal_to_dmac_vap_stru            *pst_p2p0_hal_vap;                /* p2p0 hal vap结构 */
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_cfg_p2p_noa_param_stru       st_p2p_noa_param;
    mac_cfg_p2p_ops_param_stru       st_p2p_ops_param;
#endif

#if 0 //现在逻辑是所有vap能力按照本hal device来，后续存在本hal device上有mimo也有siso的vap的场景时，再放开该设计
    oal_uint8                        uc_vap_tx_chain;                                /* 默认使用的发送通道，单播数据帧仅初始化使用，业务中有TXCS算法填写，其他数据管理帧均按初始化值填写 */
    oal_uint8                        uc_vap_single_tx_chain;                         /* 管理帧等默认使用的哪个单通道 */
#endif

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    oal_switch_enum_uint8             en_arp_offload_switch;         /* ARP offload的开关 */
    oal_uint8                         auc_resv9[1];
    dmac_vap_ip_entries_stru         *pst_ip_addr_info;              /* Host侧IPv4和IPv6地址 */
#endif

#ifdef _PRE_WLAN_FEATURE_TSF_SYNC
    oal_uint16                          us_sync_tsf_value;
    oal_uint64                          ul_old_beacon_timestamp;
#endif
#ifdef _PRE_WLAN_FEATURE_ROAM
    dmac_vap_roam_trigger_stru          st_roam_trigger;
#endif  //_PRE_WLAN_FEATURE_ROAM
#ifdef _PRE_WLAN_FEATURE_11K
    mac_rrm_info_stru                  *pst_rrm_info;
#endif //_PRE_WLAN_FEATURE_11K

#ifdef _PRE_WLAN_FEATURE_FTM
    dmac_ftm_stru                           *pst_ftm;
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    dmac_vap_btcoex_stru                st_dmac_vap_btcoex;
#endif

#ifdef _PRE_WLAN_FEATURE_VOWIFI
    //mac_vowifi_status_stru             st_vowifi_status;
    mac_vowifi_status_stru              *pst_vowifi_status;
#endif /* _PRE_WLAN_FEATURE_VOWIFI */

    oal_uint16                      us_ext_tbtt_offset;             /* 外部tbtt offset配置值*/
    oal_uint16                      us_in_tbtt_offset;              /* 内部tbtt offset配置值*/
    oal_uint16                      us_beacon_timeout;              /* beacon timeout配置值*/
    oal_uint8                       uc_psm_dtim_period;             /* 实际采用的dtim period*/
    oal_uint8                       uc_psm_auto_dtim_cnt;           /* 自动dtim的计数器*/
    oal_uint16                      us_psm_listen_interval;         /* 实际采用的listen_interval*/

    oal_bool_enum_uint8             en_non_erp_exist;               /* sta模式下，是否有non_erp station */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    oal_uint8                       uc_sta_keepalive_cnt;           /* sta模式下，定时器周期计数*/
#endif

#ifdef _PRE_WLAN_FEATURE_TXBF_HW
    oal_uint8                        *puc_vht_bfee_buff;            /* 保存txbf vht sounding使用的buffer,vap相关 */
#endif
    frw_timeout_stru                 st_obss_aging_timer;           /* OBSS保护老化定时器 */
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    frw_timeout_stru                 st_40M_recovery_timer;        /* 40M恢复定时器 */
#endif

#ifdef _PRE_WLAN_FEATURE_11K
    oal_uint8                        bit_bcn_table_switch: 1;
    oal_uint8                        bit_11k_enable      : 1;
    oal_uint8                        bit_11v_enable      : 1;
    oal_uint8                        bit_rsv1            : 5;
#else
    oal_uint8                        auc_resv11[1];
#endif
#ifdef _PRE_WLAN_FEATURE_11R
    oal_uint8                        bit_11r_enable      : 1;
    oal_uint8                        bit_rsv4            : 7;
#else
    oal_uint8                        auc_resv12[1];
#endif
    oal_switch_enum_uint8            en_auth_received;           /* 接收了auth */
    oal_switch_enum_uint8            en_assoc_rsp_received;      /* 接收了assoc */

    hal_tx_dscr_queue_header_stru    ast_tx_dscr_queue_fake[HAL_TX_QUEUE_NUM];

#ifdef _PRE_WLAN_11K_STAT
    dmac_stat_frm_rpt_stru           *pst_stat_frm_rpt;
    dmac_stat_count_tid_stru         *pst_stat_count_tid;
    dmac_stat_count_stru             *pst_stat_count;
    dmac_stat_tsc_rpt_stru           *pst_stat_tsc_rpt;
    dmac_stat_tid_tx_delay_stru      *pst_stat_tid_tx_delay;
    dmac_stat_cap_flag_stru           st_stat_cap_flag;
#endif

#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
    dmac_sensing_bssid_list_stru     st_sensing_bssid_list;
#endif
#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
    frw_timeout_stru                  st_qos_enhance_timer;  /* qos_enhance更新定时器 */
#endif
    hal_vap_pow_info_stru             st_vap_pow_info;       /* VAP级别功率信息结构体 */
    oal_uint8                         *pst_sta_bw_switch_fsm;  /* 带宽切换状态机 */

    /* ROM化后资源扩展指针 */
    oal_void                           *_rom;
}dmac_vap_stru;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

OAL_STATIC OAL_INLINE wlan_vap_mode_enum_uint8  dmac_vap_get_bss_type(mac_vap_stru *pst_vap)
{
    return pst_vap->en_vap_mode;
}

#ifdef _PRE_WLAN_FEATURE_PROXYSTA

OAL_STATIC OAL_INLINE oal_void dmac_psta_init_vap(dmac_vap_stru *pst_dmac_vap)
{
    OAL_MEMZERO(&pst_dmac_vap->st_psta, OAL_SIZEOF(pst_dmac_vap->st_psta));
}

OAL_STATIC  OAL_INLINE  oal_void  dmac_psta_update_lut_range(mac_device_stru *pst_dev, dmac_vap_stru *pst_dmac_vap, oal_uint16 *us_start, oal_uint16 *us_stop)
{
    mac_vap_stru *pst_vap = &pst_dmac_vap->st_vap_base_info;

    if (mac_is_proxysta_enabled(pst_dev))
    {
        if(mac_vap_is_vsta(pst_vap))
        {
            *us_start = dmac_vap_psta_lut_idx(pst_dmac_vap);
            *us_stop  = dmac_vap_psta_lut_idx(pst_dmac_vap) + 1; // 32 is not valid for proxysta
        }
        else
        {
            *us_start = 0;
            *us_stop  = HAL_PROXYSTA_MAX_BA_LUT_SIZE; // hardware spec
        }
    }
    // else do nothing
}

#endif

OAL_STATIC OAL_INLINE oal_uint8 dmac_vap_get_hal_device_id(dmac_vap_stru *pst_dmac_vap)
{
    return pst_dmac_vap->pst_hal_vap->uc_device_id;
}


OAL_STATIC OAL_INLINE oal_void dmac_vap_set_hal_device_id(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_hal_dev_id)
{
    pst_dmac_vap->pst_hal_vap->uc_device_id = uc_hal_dev_id;
}


OAL_STATIC OAL_INLINE hal_to_dmac_device_stru *dmac_user_get_hal_device(mac_user_stru *pst_mac_user)
{
    dmac_vap_stru       *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_user->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        return OAL_PTR_NULL;
    }

    return DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
}


OAL_STATIC OAL_INLINE hal_to_dmac_chip_stru *dmac_user_get_hal_chip(mac_user_stru *pst_mac_user)
{
    dmac_vap_stru       *pst_dmac_vap;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_user->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        return OAL_PTR_NULL;
    }

    return DMAC_VAP_GET_HAL_CHIP(pst_dmac_vap);
}

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint8 dmac_sta_bw_fsm_get_current_state(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_sta_bw_switch_fsm_attach(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_sta_bw_switch_fsm_detach(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_sta_bw_switch_fsm_init(dmac_vap_stru  *pst_dmac_vap);
extern oal_uint8 dmac_sta_bw_switch_need_new_verify(dmac_vap_stru *pst_dmac_vap, wlan_bw_cap_enum_uint8  en_bw_becaon_new);
extern oal_uint32 dmac_sta_bw_switch_fsm_post_event(dmac_vap_stru* pst_dmac_vap, oal_uint16 us_type, oal_uint16 us_datalen, oal_uint8* pst_data);
extern wlan_bw_cap_enum_uint8 dmac_sta_bw_rx_assemble_to_bandwith(hal_channel_assemble_enum_uint8 uc_bw);
extern oal_uint32  dmac_vap_init(
                       dmac_vap_stru              *pst_vap,
                       oal_uint8                   uc_chip_id,
                       oal_uint8                   uc_device_id,
                       oal_uint8                   uc_vap_id,
                       mac_cfg_add_vap_param_stru *pst_param);
extern oal_uint32  dmac_vap_init_tx_frame_params(dmac_vap_stru *pst_dmac_vap, oal_bool_enum_uint8  en_mgmt_rate_init_flag);
extern oal_void  dmac_vap_init_tx_mgmt_rate(dmac_vap_stru *pst_dmac_vap, hal_tx_txop_alg_stru *pst_tx_mgmt_cast);
extern oal_uint32  dmac_vap_init_tx_ucast_data_frame(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32  dmac_vap_sta_reset(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32  mac_vap_pause_tx(mac_vap_stru *pst_vap);
extern oal_uint32  mac_vap_resume_tx(mac_vap_stru *pst_vap);
extern oal_void  dmac_vap_pause_tx(mac_vap_stru *pst_mac_vap);

extern oal_void  dmac_vap_pause_tx_by_chl(mac_device_stru *pst_device, mac_channel_stru *pst_src_chl);
extern oal_void  dmac_vap_resume_tx_by_chl(mac_device_stru *pst_device, hal_to_dmac_device_stru *pst_hal_device,  mac_channel_stru *pst_dst_channel);
extern oal_void  dmac_vap_update_bi_from_hw(mac_vap_stru *pst_mac_vap);
extern oal_void  dmac_vap_init_tx_data_rate_ucast(dmac_vap_stru *pst_dmac_vap,oal_uint8 uc_protocol_mode, oal_uint8 uc_legacy_rate);
extern oal_uint32  dmac_vap_is_in_p2p_listen(mac_vap_stru *pst_mac_vap);
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#ifdef _PRE_WLAN_FEATURE_VOWIFI
extern void dmac_vap_vowifi_init(dmac_vap_stru *pst_dmac_vap);
extern void dmac_vap_vowifi_exit(dmac_vap_stru *pst_dmac_vap);
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#ifdef _PRE_WLAN_FEATURE_DBAC
extern oal_void dmac_vap_restart_dbac(dmac_vap_stru  *pst_dmac_vap);
#endif
extern oal_void  dmac_one_packet_send_null_data(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap, oal_bool_enum_uint8 en_ps);
#endif
extern oal_void dmac_vap_down_notify(mac_vap_stru *pst_down_vap);
extern oal_uint32  dmac_vap_fake_queue_empty_assert(mac_vap_stru *pst_mac_vap, oal_uint32 ul_file_id);
extern oal_uint32  dmac_vap_clear_fake_queue(mac_vap_stru  *pst_mac_vap);
extern oal_uint32  dmac_vap_save_tx_queue(mac_vap_stru *pst_mac_vap);
extern oal_uint32  dmac_vap_restore_tx_queue(mac_vap_stru *pst_mac_vap);
extern oal_bool_enum_uint8  dmac_vap_is_fakeq_empty(mac_vap_stru *pst_mac_vap);
extern oal_void   dmac_vap_update_snr_info(dmac_vap_stru *pst_dmac_vap, dmac_rx_ctl_stru *pst_rx_ctrl, mac_ieee80211_frame_stru *pst_frame_hdr);
extern oal_void dmac_vap_work_set_channel(dmac_vap_stru *pst_dmac_vap);
#ifdef _PRE_WLAN_FEATURE_DBDC
extern oal_uint32  dmac_vap_change_hal_dev(mac_vap_stru *pst_shift_vap, mac_dbdc_debug_switch_stru *pst_dbdc_debug_switch);
extern oal_void dmac_vap_dbdc_start(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap);
extern oal_void dmac_vap_dbdc_stop(mac_device_stru *pst_mac_device, mac_vap_stru *pst_down_vap);
extern oal_uint32 dmac_vap_change_hal_dev_before_up(mac_vap_stru *pst_shift_vap, oal_void *pst_dmac_device);
extern oal_uint32  dmac_up_vap_change_hal_dev(mac_vap_stru *pst_shift_mac_vap);
extern oal_bool_enum_uint8 dmac_dbdc_channel_check(mac_channel_stru *pst_channel1, mac_channel_stru *pst_channel2);

#endif
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
extern oal_void dmac_vap_init_sensing_bssid_list(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_vap_clear_sensing_bssid_list(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32 dmac_vap_update_sensing_bssid_list(mac_vap_stru *pst_mac_vap, dmac_sensing_bssid_cfg_stru *pst_sensing_bssid);
#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_vap.h */
