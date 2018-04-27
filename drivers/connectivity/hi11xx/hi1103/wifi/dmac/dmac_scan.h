
#ifndef __DMAC_WITP_SCAN_H__
#define __DMAC_WITP_SCAN_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "wlan_spec.h"
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "mac_device.h"
#include "mac_vap.h"
#include "dmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_SCAN_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMAC_SCAN_CHANNEL_STATICS_PERIOD_US     (10 * 1000)    /* MAC信道负载统计周期 10ms */
#define DMAC_SCAN_CHANNEL_MEAS_PERIOD_MS        10             /* PHY空闲信道估计窗长 10ms */
#define DMAC_SCAN_CTS_MAX_DURATION              32767
#define DMAC_SCAN_MAX_TIMER                     60*1000     /*支持最大的timer时间*/
#define DMAC_SCAN_CHANNEL_DWELL_TIME_MARGIN     2           /* ms */
#define DMAC_SCAN_P2PGO_SEND_BEACON_TIME        10          /* ms, go tbtt中断后过多久切信道 */
#define DMAC_SCAN_DBAC_SCAN_DELTA_TIME          1500        /* ms */
#define DMAC_SCAN_GO_MAX_SCAN_TIME              300         /* ms */
#define DMAC_SCAN_CHANENL_NUMS_TO_PRINT_SWITCH_INFO   14    /* 扫描信道数少于此才打印出切换到XX信道的信息 */
#define DMAC_SCAN_TIMER_DEVIATION_TIME           20        /* 扫描定时器的执行误差时间ms */
#define DMAC_SCAN_MAX_AP_NUM_TO_GNSS            32

//[0-1000], 越大表示信道越忙
/*
//理论上
信道测量时间 = 信道空闲时间+ //能量低于CCA门限
空口的干扰时间+ //能量高于CCA门限，但是无法解析的能量信号(临频/叠频干扰)
信道发送时间+ //自身竞争获取到的发送时间
信道接收时间+ //空口中别的节点发送的时间
蓝牙中断的时间(optional)//蓝牙共存时，蓝牙收发占用的时间

所以通过测量到的信道空闲时间就可以用来表征当前信道的繁忙度。
*/
#define DMAC_SCAN_GET_DUTY_CYC_RATIO(_pst_chan_result, _ul_total_free_time)\
    (((_ul_total_free_time)>((_pst_chan_result)->ul_total_stats_time_us))   \
    ?0:((((_pst_chan_result)->ul_total_stats_time_us)-(_ul_total_free_time))\
    *1000/((_pst_chan_result)->ul_total_stats_time_us)))

/* 判断每个周期的统计时间是否越界 */
#define DMAC_SCAN_GET_VALID_FREE_TIME(_ul_trx_time, _ul_total_stat_time, _ul_total_free_time) \
    (((_ul_total_stat_time) >= ((_ul_trx_time) + (_ul_total_free_time)))   \
    ? (_ul_total_free_time) : (((_ul_total_stat_time) <= (_ul_trx_time)) ? 0 : ((_ul_total_stat_time) - (_ul_trx_time))))

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum
{
    SCAN_CHECK_ASSOC_CHANNEL_LINKLOSS      = 0,
    SCAN_CHECK_ASSOC_CHANNEL_CSA           = 1,

    SCAN_CHECK_ASSOC_CHANNEL_BUTT
}scan_check_assoc_channel_enum;
typedef oal_uint8 scan_check_assoc_channel_enum_uint8;


/*****************************************************************************
  4 全局变量声明
****************************************************************************/

/*****************************************************************************
  6 消息定义
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
#pragma pack(1)
typedef struct
{
    oal_uint8              uc_ch_valid_num;
    mac_channel_stru       ast_wlan_channel[WLAN_MAX_CHANNEL_NUM];
} dmac_gscan_params_stru;
#pragma pack()

typedef oal_void  (*p_dmac_scan_get_ch_statics_measurement_result_cb)(hal_to_dmac_device_stru *pst_hal_device, hal_ch_statics_irq_event_stru  *pst_stats_result);
typedef oal_void (*p_dmac_scan_calcu_channel_ratio_cb)(hal_to_dmac_device_stru   *pst_hal_device);

typedef struct
{
    p_dmac_scan_get_ch_statics_measurement_result_cb  p_dmac_scan_get_ch_statics_measurement_result;
    p_dmac_scan_calcu_channel_ratio_cb                p_dmac_scan_calcu_channel_ratio;
}dmac_scan_rom_cb;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 外部函数声明
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32 dmac_trigger_csa_scan(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_scan_start_obss_timer(mac_vap_stru *pst_mac_vap);
extern oal_void dmac_scan_destroy_obss_timer(dmac_vap_stru *pst_dmac_vap);

extern oal_uint32  dmac_scan_proc_scan_complete_event(dmac_vap_stru *pst_dmac_vap,
                                                      mac_scan_status_enum_uint8 en_scan_rsp_status);
extern oal_uint32  dmac_scan_mgmt_filter(dmac_vap_stru *pst_dmac_vap, oal_void *p_param, oal_bool_enum_uint8 *pen_report_bss, oal_uint8 *pen_go_on);
extern oal_uint32  dmac_scan_proc_sched_scan_req_event(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_scan_handle_scan_req_entry(mac_device_stru    *pst_mac_device,
                                                                 dmac_vap_stru      *pst_dmac_vap,
                                                                 mac_scan_req_stru  *pst_scan_req_params);
extern oal_uint32  dmac_scan_proc_scan_req_event(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_scan_process_scan_req_event(frw_event_mem_stru *pst_event_mem);
extern oal_void   dmac_scan_switch_channel_off(mac_device_stru *pst_mac_device);
extern oal_bool_enum_uint8  dmac_scan_switch_channel_back(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device);
extern oal_uint32  dmac_switch_channel_off(
                mac_device_stru     *pst_mac_device,
                mac_vap_stru        *pst_mac_vap,
                mac_channel_stru    *pst_dst_chl,
                oal_uint16           us_protect_time);
extern oal_void dmac_scan_begin(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device);
extern oal_void dmac_scan_end(mac_device_stru *pst_mac_device);
extern oal_void  dmac_scan_prepare_end(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device);
extern oal_void  dmac_scan_abort(mac_device_stru *pst_mac_device);
extern oal_uint32  dmac_scan_after_p2pgo_send_noa(void *p_arg);


/* 中断事件处理函数 */
extern oal_uint32 dmac_scan_channel_statistics_complete(frw_event_mem_stru *pst_event_mem);
//extern oal_void dmac_scan_radar_detected(mac_device_stru *pst_mac_device, hal_radar_det_event_stru *pst_radar_det_info);

/* 初始化及释放函数 */
extern oal_uint32 dmac_scan_init(mac_device_stru *pst_mac_device);
extern oal_uint32 dmac_scan_exit(mac_device_stru *pst_mac_device);

/* 外部函数引用 */
extern oal_uint32  dmac_scan_send_probe_req_frame(dmac_vap_stru *pst_dmac_vap,
                                            oal_uint8 *puc_bssid,
                                            oal_int8 *pc_ssid);

/* 停止定时器 */
extern oal_uint32  dmac_scan_stop_pno_sched_scan_timer(void *p_arg);
extern oal_void dmac_dbac_switch_channel_off(mac_device_stru  *pst_mac_device,
                                                mac_vap_stru   *pst_mac_vap1,
                                                mac_vap_stru   *pst_mac_vap2,
                                                mac_channel_stru  *pst_dst,
                                                oal_uint16  us_protect_time);
extern oal_void dmac_scan_switch_home_channel_work(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device);
extern oal_void dmac_scan_handle_switch_channel_back(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device, hal_scan_params_stru *pst_hal_scan_params);
#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
extern oal_void  dmac_scan_init_bss_info_list(mac_device_stru *pst_mac_dev);
extern oal_void dmac_scan_update_gscan_vap_id(mac_vap_stru *pst_mac_vap, oal_uint8 en_is_add_vap);
extern oal_uint32 dmac_ipc_irq_event(frw_event_mem_stru *pst_event_mem);
#endif
extern oal_void dmac_scan_calcu_channel_ratio(hal_to_dmac_device_stru   *pst_hal_device);
extern oal_void dmac_scan_one_channel_start(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_is_scan_start);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_scan.h */
