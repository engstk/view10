

#ifndef __DMAC_DEVICE_H__
#define __DMAC_DEVICE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "mac_device.h"
#include "dmac_alg_if.h"

#ifdef  _PRE_WLAN_FEATURE_GREEN_AP
#include "dmac_green_ap.h"
#endif

#ifdef  _PRE_WLAN_FEATURE_PACKET_CAPTURE
#include "dmac_pkt_capture.h"
#endif

#ifdef _PRE_WLAN_FEATURE_HWBW_20_40
#include "dmac_acs.h"
#endif
#include "dmac_scan.h"
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_DEVICE_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAX_MAC_ERR_IN_TBTT 5   /*每一个tbtt间隙允许出现的最大错误数*/

#define DMAC_DEV_GET_MST_HAL_DEV(_pst_dmac_device)     ((_pst_dmac_device)->past_hal_device[HAL_DEVICE_ID_MASTER])
#define DMAC_DEV_GET_SLA_HAL_DEV(_pst_dmac_device)     (dmac_device_get_another_h2d_dev((_pst_dmac_device), (_pst_dmac_device)->past_hal_device[HAL_DEVICE_ID_MASTER]))

#define DMAC_RSSI_LIMIT_DELTA  (5)  //新用户关联rssi门限为a，已关联用户被剔除rssi门限为b，a = b+DMAC_RSSI_LIMIT_DELTA; 配置命令传进来的为b. delta默认为5，可修改

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


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
    wlan_nss_enum_uint8                     en_nss_num;              /* Nss 空间流个数 */
    oal_uint8                               uc_phy_chain;             /* 发送通道 */
    oal_uint8                               uc_single_tx_chain;
    /* 管理帧采用单通道发送时选择的通道(单通道时要配置和uc_phy_chain一致),或者用于phy接收通道配置 */
    oal_uint8                               uc_rf_chain;             /* 接收通道 */
} dmac_device_capability_stru;


#ifdef _PRE_WLAN_FEATURE_BAND_STEERING

#define     BSD_VAP_LOAD_SAMPLE_MAX_CNT         16      //保存采样的负载值最大个数，注意此值为2的N次方
typedef struct
{
    oal_uint16  aus_buf[BSD_VAP_LOAD_SAMPLE_MAX_CNT];
    oal_uint32  ul_size;
    oal_uint32  ul_in;
    oal_uint32  ul_out;
}dmac_device_bsd_ringbuf;

typedef struct
{
    oal_uint8                   uc_state;               //已经初始化的标记
    oal_uint8                   auc_reserve[3];
    //frw_timeout_stru            st_load_sample_timer;   //负载的采样定时器，定时器超时函数负责启动新一轮
                                                        //的信道扫描，并获取此时间内的信道扫描结果
    dmac_device_bsd_ringbuf     st_sample_result_buf;   //保存采样结果的环形缓冲区
    //mac_scan_req_stru           st_scan_req_param;      //扫描参数
}dmac_device_bsd_stru;

#endif

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
#pragma pack(1)
typedef struct
{
    oal_uint8   auc_bssid[WLAN_MAC_ADDR_LEN];   /* 网络bssid */
    oal_uint8   uc_channel_num;
    oal_int8    c_rssi;                       /* bss的信号强度 */
    oal_uint8   uc_serving_flag;
    oal_uint8   uc_rtt_unit;
    oal_uint8   uc_rtt_acc;
    oal_uint32  ul_rtt_value;
}wlan_ap_measurement_info_stru;

/* 上报给gnss的扫描结果结构体 */
typedef struct
{
    oal_uint32                     ul_interval_from_last_scan;
    oal_uint8                      uc_ap_valid_number;
    wlan_ap_measurement_info_stru  ast_wlan_ap_measurement_info[DMAC_SCAN_MAX_AP_NUM_TO_GNSS];
}dmac_gscan_report_info_stru;
#pragma pack()

typedef struct
{
    oal_dlist_head_stru            st_entry;                    /* 链表指针 */
    wlan_ap_measurement_info_stru  st_wlan_ap_measurement_info; /*上报gnss的扫描信息 */
}dmac_scanned_bss_info_stru;

typedef struct
{
    oal_uint32                     ul_scan_end_timstamps;/* 记录此次扫描的时间戳,一次扫描记录一次,不按扫到的ap分别记录 */
    oal_dlist_head_stru            st_dmac_scan_info_list;
    oal_dlist_head_stru            st_scan_info_res_list;  /* 扫描信息存储资源链表 */
    dmac_scanned_bss_info_stru     ast_scan_bss_info_member[DMAC_SCAN_MAX_AP_NUM_TO_GNSS];
}dmac_scan_for_gnss_stru;
#endif

#ifdef _PRE_FEATURE_FAST_AGING
#define DMAC_FAST_AGING_NUM_LIMIT (10)
#define DMAC_FAST_AGING_TIMEOUT   (1000)

typedef struct
{
    frw_timeout_stru                    st_timer;           /* 快速老化定时器 */
    oal_bool_enum_uint8                 en_enable;
    oal_uint8                           uc_count_limit;
    oal_uint16                          us_timeout_ms;
}dmac_fast_aging_stru;
#endif
/* dmac device结构体，记录只保存在dmac的device公共信息 */
typedef struct
{
    mac_device_stru                    *pst_device_base_info;                   /* 指向公共部分mac device */

#if (WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS)
    oal_uint8                           uc_fake_vap_id;
    oal_uint8                           auc_bfer_rev[3];
#endif
#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
    oal_uint8                           uc_rsp_frm_rate_val;
    oal_bool_enum_uint8                 en_state_in_sw_ctrl_mode;
    hal_channel_assemble_enum_uint8     en_usr_bw_mode;
    oal_uint8                           auc_rsv[1];
#endif

    oal_bool_enum_uint8                 en_is_fast_scan;     /* 是否是并发扫描 */
    oal_bool_enum_uint8                 en_fast_scan_enable; /*是否可以并发,XX原因即使硬件支持也不能快速扫描*/
    oal_bool_enum_uint8                 en_dbdc_enable;      /* 软件是否支持dbdc */
    oal_uint8                           uc_gscan_mac_vap_id;  /* gscan 的扫描vap */

#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    dmac_green_ap_mgr_stru              st_green_ap_mgr;                     /*device结构下的green ap特性结构体*/
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
    dmac_device_bsd_stru                st_bsd;
#endif

    hal_to_dmac_device_stru            *past_hal_device[WLAN_DEVICE_MAX_NUM_PER_CHIP];//03动态dbdc 两个hal device,02/51/03静态dbdc一个hal device
    hal_to_dmac_chip_stru              *pst_hal_chip;                        /* 硬chip结构指针，HAL提供，用于逻辑和物理chip的对应 */

#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
    dmac_packet_stru                    st_pkt_capture_stat;                 /*device结构下的抓包特性结构体*/
#endif

#ifdef _PRE_WLAN_11K_STAT
    dmac_stat_count_stru                *pst_stat_count;
#endif

    //rssi cfg
    mac_cfg_rssi_limit_stru              st_rssi_limit;

#ifdef _PRE_WLAN_FEATURE_DFS
    oal_uint32                           ul_dfs_cnt;                          /*检测到的雷达个数*/
#endif

#ifdef _PRE_WLAN_FEATURE_HWBW_20_40
    dmac_acs_2040_user                  st_acs_2040_user;
#endif

#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
    dmac_scan_for_gnss_stru             st_scan_for_gnss_info;
#endif
    /* ROM化后资源扩展指针 */
    oal_void                           *_rom;
#ifdef _PRE_FEATURE_FAST_AGING
    dmac_fast_aging_stru                st_fast_aging;
#endif
}dmac_device_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
#define   DMAC_DEVICE_GET_HAL_CHIP(_pst_device)                  (((dmac_device_stru *)(_pst_device))->pst_hal_chip)


OAL_STATIC OAL_INLINE mac_fcs_mgr_stru* dmac_fcs_get_mgr_stru(mac_device_stru *pst_device)
{
#if IS_DEVICE
    return  &pst_device->st_fcs_mgr;
#else
    return OAL_PTR_NULL;
#endif
}


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern  oal_uint32 dmac_device_exception_report_timeout_fn(oal_void *p_arg);

/*****************************************************************************
  10.1 公共结构体初始化、删除
*****************************************************************************/

extern oal_uint32  dmac_board_exit(mac_board_stru *pst_board);
extern oal_uint32  dmac_board_init(mac_board_stru *pst_board);

/*****************************************************************************
  10.2 公共成员访问部分
*****************************************************************************/
extern oal_uint32  dmac_mac_error_overload(mac_device_stru *pst_mac_device, hal_mac_error_type_enum_uint8 en_error_id);
extern oal_void  dmac_mac_error_cnt_clr(mac_device_stru *pst_mac_device);
extern oal_void  dmac_mac_error_cnt_inc(mac_device_stru *pst_mac_device, oal_uint8 uc_mac_int_bit);
extern hal_to_dmac_device_stru*  dmac_device_get_another_h2d_dev(dmac_device_stru *pst_dmac_device, hal_to_dmac_device_stru *pst_ori_hal_dev);
extern oal_bool_enum_uint8 dmac_device_is_dynamic_dbdc(dmac_device_stru *pst_dmac_device);
extern oal_bool_enum_uint8 dmac_device_is_support_double_hal_device(dmac_device_stru *pst_dmac_device);
extern hal_to_dmac_device_stru *dmac_device_get_work_hal_device(dmac_device_stru *pst_dmac_device);
extern oal_uint8  dmac_device_check_fake_queues_empty(oal_uint8 uc_device_id);
extern oal_void  dmac_free_hd_tx_dscr_queue(hal_to_dmac_device_stru *pst_hal_dev_stru);
extern oal_bool_enum_uint8 dmac_device_check_is_vap_in_assoc(mac_device_stru  *pst_mac_device);
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_device.h */
