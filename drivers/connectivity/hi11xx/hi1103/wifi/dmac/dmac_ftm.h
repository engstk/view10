

#ifndef __DMAC_FTM__
#define __DMAC_FTM__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_FTM

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oal_mem.h"
#include "mac_vap.h"
#include "dmac_user.h"
#include "dmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_FTM_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define     PERIOD_OF_FTM_TIMER                                 6 //6250    /* 皮秒 当前为fpga版本*/

#define     PHY_TX_ADJUTST_VAL                                  0       /* 发送phy至connector时间 */
#define     PHY_RX_ADJUTST_VAL                                  0       /* 接收connector至phy时间 */

#define     T1_FINE_ADJUST_VAL                                  0       /* 实际测量校准值 */
#define     T2_FINE_ADJUST_VAL                                  0
#define     T3_FINE_ADJUST_VAL                                  0
#define     T4_FINE_ADJUST_VAL                                  0

#define     FTM_FRAME_DIALOG_TOKEN_OFFSET                       2       /* ftm帧dialog token偏移 */
#define     FTM_FRAME_IE_OFFSET                                 3       /* ftm req帧ie偏移 */
#define     FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET              3       /* ftm帧follow up dialog token偏移 */
#define     FTM_FRAME_TOD_OFFSET                                4       /* ftm帧tod偏移 */
#define     FTM_FRAME_TOA_OFFSET                                10      /* ftm帧toa偏移 */
#define     FTM_FRAME_OPTIONAL_IE_OFFSET                        20
#define     FTM_FRAME_TSF_SYNC_INFO_OFFSET                      3
#define     FTM_RANGE_IE_OFFSET                                 3
#define     FTM_REQ_TRIGGER_OFFSET                              2       /* ftm req帧Trigger偏移 */

#define     FTM_WAIT_TIMEOUT                                    10          /*等待接收FTM_1的时间*/

#define     FTM_MIN_DETECTABLE_DISTANCE                         0          /*FTM最小可检测范围*/
#define     FTM_MAX_DETECTABLE_DISTANCE                         8192000    /*FTM最大可检测范围 2km 换算单位为1/4096m*/

#define     FTM_FRAME_TOD_LENGTH                                6
#define     FTM_FRAME_TOA_LENGTH                                6
#define     FTM_FRAME_TOD_ERROR_LENGTH                          2
#define     FTM_FRAME_TOA_ERROR_LENGTH                          2
#define     FTM_TMIE_MASK                                       0xFFFFFFFFFFFF /*48位*/
#define     MAX_REPEATER_NUM                                    3            /* 支持的最大定位ap数量 */

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
typedef enum
{
    FTM_TIME_1             = 0,
    FTM_TIME_2             = 1,
    FTM_TIME_3             = 2,
    FTM_TIME_4             = 3,

    FTM_TIME_BUTT,
}ftm_time_enum;
/* 三种业务类型 */
typedef enum
{
    NO_LOCATION    = 0,
    ROOT_AP        = 1,
    REPEATER       = 2,
    STA            = 3,
    LOCATION_TYPE_BUTT
}oal_location_type_enum;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern    oal_uint32          g_aul_ftm_correct_time[FTM_TIME_BUTT];
extern    dmac_ftm_stru       g_st_ftm;
extern    oal_uint8           guc_csi_loop_flag;

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
    oal_uint8             uc_location_type;                       /*定位类型 0:关闭定位 1:root ap;2:repeater;3station*/
    oal_uint8             auc_location_ap[MAX_REPEATER_NUM][WLAN_MAC_ADDR_LEN];    /*各location ap的mac地址，第一个为root */
    oal_uint8             auc_station[WLAN_MAC_ADDR_LEN];          /*STA MAC地址，暂时只支持一个sta的测量*/
} dmac_location_stru;

typedef struct
{
    oal_uint8                uc_category;
    oal_uint8                uc_action_code;
    oal_uint8                auc_oui[3];
    oal_uint8                uc_eid;
    oal_uint8                uc_lenth;
    oal_uint8                uc_location_type;
    oal_uint8                auc_mac_server[WLAN_MAC_ADDR_LEN];
    oal_uint8                auc_mac_client[WLAN_MAC_ADDR_LEN];
    oal_uint8                auc_payload[4];
}dmac_location_event_stru;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/

extern oal_uint32 dmac_process_ftm_ack_complete(frw_event_mem_stru * pst_event_mem);
extern oal_uint32  dmac_sta_rx_ftm(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_uint32 dmac_check_tx_ftm(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buf);
extern oal_void dmac_rrm_parse_ftm_range_req(dmac_vap_stru *pst_dmac_vap, mac_meas_req_ie_stru  *pst_meas_req_ie);
extern oal_uint32  dmac_sta_send_ftm_req(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32  dmac_ftm_rsp_send_ftm(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_session_id);
extern oal_uint32  dmac_ftm_send_trigger(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32 dmac_sta_start_scan_for_ftm(dmac_vap_stru *pst_dmac_vap, oal_uint16 us_scan_time);
extern oal_void  dmac_save_ftm_range(dmac_vap_stru *pst_dmac_vap);
extern oal_uint32  dmac_sta_ftm_wait_start_burst_timeout(void *p_arg);
extern oal_void dmac_rrm_meas_ftm(dmac_vap_stru *pst_dmac_vap);
extern oal_uint16  dmac_encap_ftm_mgmt(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buffer, oal_uint8 uc_session_id);
extern oal_void  dmac_ftm_rsp_rx_ftm_req(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
extern oal_void  dmac_tx_set_ftm_ctrl_dscr(dmac_vap_stru *pst_dmac_vap, hal_tx_dscr_stru * p_tx_dscr, oal_netbuf_stru *pst_netbuf);
extern oal_void  dmac_set_ftm_correct_time(dmac_vap_stru *pst_dmac_vap, mac_set_ftm_time_stru st_ftm_time);
extern oal_uint32  dmac_ftm_rsp_set_parameter(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_session_id);
extern oal_void  dmac_sta_ftm_start_bust(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_vap_ftm_int(dmac_vap_stru *pst_dmac_vap);
extern oal_int8 dmac_ftm_find_session_index(dmac_vap_stru *pst_dmac_vap, mac_ftm_mode_enum_uint8 en_ftm_mode, oal_uint8 auc_peer_mac[WLAN_MAC_ADDR_LEN]);
extern oal_void dmac_ftm_enable_session_index(dmac_vap_stru *pst_dmac_vap, mac_ftm_mode_enum_uint8 en_ftm_mode, oal_uint8 auc_peer_mac[WLAN_MAC_ADDR_LEN], oal_uint8 uc_session_id);
extern oal_bool_enum_uint8 dmac_check_ftm_switch_channel(dmac_vap_stru *pst_dmac_vap, mac_ftm_mode_enum_uint8 en_ftm_mode, oal_uint8 uc_chan_number);
extern oal_uint32 dmac_location_rssi_process(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf, oal_uint8 c_rssi);
extern oal_uint32 dmac_location_csi_process(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_ta, oal_uint32 ul_csi_info_len, oal_uint32 **pul_csi_start_addr);


#endif
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_p2p.h */
