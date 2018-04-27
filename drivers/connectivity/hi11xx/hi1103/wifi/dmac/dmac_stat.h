

#ifndef __DMAC_STAT_H__
#define __DMAC_STAT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_PERFORM_STAT

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "dmac_ext_if.h"
#include "hal_ext_if.h"
#include "frw_ext_if.h"
#include "dmac_user.h"
#include "dmac_main.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_STAT_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMAC_STAT_ITEM_LIMIT                    100     /* 时延，吞吐量，per最大统计周期数 */

#define DMAC_STAT_TID_DELAY_NODE_LIMIT          128     /* tid时延统计最大节点数量 */
#define DMAC_STAT_TID_PER_NODE_LIMIT            128     /* tid per统计最大节点数 */
#define DMAC_STAT_TID_THRPT_NODE_LIMIT          128     /* tid吞吐量统计最大节点数量 */
#define DMAC_STAT_USER_THRPT_NODE_LIMIT         32      /* user吞吐量统计最大节点数量 */
#define DMAC_STAT_VAP_THRPT_NODE_LIMIT          8       /* vap吞吐量统计最大节点数量 */
#define DMAC_STAT_TIMER_CYCLE_MS                100     /* 默认统计周期 */

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
#define DMAC_QOS_TIMER_CYCLE_MS                1000     /* qos enhance统计周期1000ms */
#endif

#ifdef _PRE_WLAN_11K_STAT
#define DMAC_STAT_TX_DELAY_HIST_BIN0_RANGE               10

#define DMAC_STAT_CALC_THRPT(_node_begin, _node_end,  _type)          \
        (((((_node_end)->aul_stat_sum[(_type)])-((_node_begin)->aul_stat_sum[(_type)]))<<3)  \
        /(_node_begin)->st_timer.ul_timeout)

#define DMAC_STAT_CALC_CNT(_node_begin, _node_end, _type) \
        (((_node_end)->aul_stat_cnt[(_type)])-((_node_begin)->aul_stat_cnt[(_type)]))

#define DMAC_STAT_CALC_RSSI(_node_begin, _node_end, _type)          \
        ((0 == DMAC_STAT_CALC_CNT(_node_begin, _node_end, _type))? 0: \
        (((_node_end)->aul_stat_sum[(_type)])-((_node_begin)->aul_stat_sum[(_type)]))  \
        /DMAC_STAT_CALC_CNT(_node_begin, _node_end, _type))

#define DMAC_DEV_STATS_PKT_INCR(_member, _cnt)            ((_member) += (_cnt))
#endif
/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
#ifdef _PRE_WLAN_11K_STAT
typedef enum
{
    DMAC_STAT_BSD_TX_THRPT      = 0,      /* 发送吞吐量*/
    DMAC_STAT_BSD_RX_THRPT      = 1,      /* 接收吞吐量 */
    DMAC_STAT_BSD_RX_RSSI       = 2,      /* 接收RSSI */

    DMAC_STAT_BSD_BUTT
}dmac_stat_bsd;
typedef oal_uint8 dmac_stat_bsd_enum_uint8;

typedef enum
{
    DMAC_STAT_MPDU_LVL      = 0,
    DMAC_STAT_FRM_LVL       = 1,

    DMAC_STAT_LVL_BUTT
}dmac_stat_lvl;
typedef oal_uint8 dmac_stat_lvl_uint8;

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


/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/* stat_node结构体 */
typedef struct
{
    oal_dlist_head_stru	        st_entry;                               /* 双向链表entry */
    frw_timeout_stru            st_timer;                               /* 定时器 */

    oal_bool_enum_uint8         uc_stat_flag;                           /* 统计是否使能 */
    oal_uint8                   uc_resv[3];                             /* 字节对齐 */

    dmac_stat_param_stru       *pst_stat_param;                          /* 统计参数 */
    oal_uint32                  us_total_item;                          /* 记录总条数 */
    oal_uint32                  us_curr_item;                           /* 当前第几条 */
    oal_uint32                  aul_stat_cnt[DMAC_STAT_PER_BUTT];            /* 某个统计周期次数 */
    oal_uint32                  aul_stat_sum[DMAC_STAT_PER_BUTT];            /* 某个周期内统计量的累计值 */
    dmac_stat_timeout_func      p_inner_func;                           /* 内部统计模块的函数指针 */

    oal_uint32                 *pul_stat_avg;                           /* 由CFG模块得到的统计平均值存放首地址 */
}dmac_stat_node_stru;

typedef struct
{
    oal_uint32              ul_node_num;           /* 统计节点数量 */
    oal_dlist_head_stru     st_stat_node_dlist;    /* 统计节点双向链表头节点 */
}dmac_stat_stru;

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32  dmac_stat_init(oal_void);
extern oal_uint32  dmac_stat_exit(oal_void);
extern oal_uint32  dmac_stat_tid_delay(dmac_tid_stru *pst_dmac_tid);
extern oal_uint32  dmac_stat_tx_thrpt(dmac_user_stru *pst_dmac_user, oal_uint8 uc_tidno, hal_tx_dscr_ctrl_one_param st_tx_dscr_param);
extern oal_uint32  dmac_stat_rx_thrpt(frw_event_hdr_stru *pst_event_hdr, mac_vap_stru *pst_vap, dmac_rx_ctl_stru *pst_rx_ctl);
extern oal_uint32  dmac_stat_display(mac_stat_module_enum_uint16     en_module_id,
                                      mac_stat_type_enum_uint8       en_stat_type,
                                      oal_void                      *p_void);

#ifdef _PRE_WLAN_11K_STAT
extern oal_uint32  dmac_user_stat_tid_delay(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
    mac_tx_ctl_stru *pst_tx_ctl,  oal_time_us_stru *pst_time);
extern oal_uint32  dmac_user_stat_tx_delay(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_tidno,
        mac_tx_ctl_stru *pst_tx_ctl, oal_time_us_stru *pst_time);
extern oal_uint32 dmac_user_stat_tx_frm_info(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
        oal_uint8 uc_tidno, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param, oal_uint8 bit_is_ampdu);
#ifdef _PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT
extern oal_uint32 dmac_user_stat_tx_mcs_count(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
    mac_tx_ctl_stru* pst_cb, hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param);
#endif
extern oal_uint32  dmac_user_stat_tx_mpdu_info(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
        mac_tx_ctl_stru* pst_cb, oal_uint8 uc_dscr_status, oal_bool_enum_uint8 en_is_discarded);
extern oal_uint32  dmac_user_stat_rx_info(frw_event_hdr_stru *pst_event_hdr, mac_vap_stru *pst_vap, dmac_rx_ctl_stru *pst_rx_ctrl, oal_uint8 uc_stat_lvl);
extern oal_uint32  dmac_user_stat_tx_dropped_mpdu_num(dmac_user_stru *pst_dmac_user, dmac_vap_stru *pst_dmac_vap,
    mac_device_stru *pst_mac_device, oal_uint8 uc_tidno);
extern oal_uint32 dmac_stat_rx_duplicate_num(dmac_user_stru *pst_dmac_user);

extern oal_uint32  dmac_stat_init_user(dmac_user_stru *pst_dmac_user);
extern oal_uint32  dmac_stat_exit_user(dmac_user_stru *pst_dmac_user);

extern oal_uint32  dmac_stat_init_vap(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32  dmac_stat_exit_vap(dmac_vap_stru *pst_dmac_vap);

extern oal_int32  dmac_stat_init_device(oal_void);
extern oal_int32  dmac_stat_exit_device(oal_void);
extern oal_uint32 dmac_stat_rx_tid_fcs_error(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_hal_vap_id, dmac_rx_ctl_stru *pst_rx_ctrl);
#endif

#ifdef _PRE_WLAN_FEATURE_QOS_ENHANCE
extern oal_uint32 dmac_stat_tx_sta_thrpt(mac_user_stru *pst_user);
extern oal_uint32 dmac_stat_qos_init_user(dmac_user_stru *pst_dmac_user);
extern oal_uint32 dmac_stat_qos_exit_user(dmac_user_stru *pst_dmac_user);
#endif

#if defined _PRE_WLAN_PRODUCT_1151V200 && defined _PRE_WLAN_RX_DSCR_TRAILER
extern oal_void dmac_stat_update_ant_rssi(hal_to_dmac_device_stru *pst_hal_device, oal_uint8 uc_hal_vap_id, hal_rx_statistic_stru st_rx_stat);
#endif
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_stat.h */


