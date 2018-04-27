

#ifndef __DMAC_MGMT_STA_H__
#define __DMAC_MGMT_STA_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "dmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_MGMT_STA_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define  DMAC_WMM_VO_DEFAULT_DECA_AIFSN   2
/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
typedef oal_uint32 (*update_edca_sta)(frw_event_mem_stru  *pst_event_mem, dmac_vap_stru *pst_dmac_vap);
typedef oal_uint8  (*need_obss_scan)(mac_device_stru *pst_dev, mac_vap_stru *pst_mac_vap);



typedef struct
{
    update_edca_sta update_edca_sta_cb;
    need_obss_scan  need_obss_scan_cb;

}dmac_mgmt_sta_cb;
extern dmac_mgmt_sta_cb g_st_dmac_mgmt_sta_rom_cb;
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


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_void  dmac_chan_adjust_bandwidth_sta(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 *pen_bandwidth);
extern oal_uint32 dmac_mgmt_wmm_update_edca_machw_sta(frw_event_mem_stru  *pst_event_mem);
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
extern oal_uint8  dmac_mgmt_need_obss_scan(mac_vap_stru *pst_mac_vap);
#endif
extern oal_uint32 dmac_sta_up_update_ht_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len, mac_user_stru *pst_mac_user);
extern oal_uint32 dmac_sta_up_update_vht_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, oal_uint16 us_frame_len, mac_user_stru *pst_mac_user);
extern oal_void  dmac_chan_multi_select_channel_mac(mac_vap_stru *pst_mac_vap, oal_uint8 uc_channel, wlan_channel_bandwidth_enum_uint8 en_bandwidth);
extern oal_void  dmac_chan_sta_switch_channel(mac_vap_stru *pst_mac_vap);

#ifdef _PRE_WLAN_FEATURE_11AX
extern oal_void dmac_sta_up_update_spatial_reuse_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload,oal_uint16 us_frame_len,mac_user_stru *pst_mac_user);
extern oal_uint32 dmac_sta_up_update_he_oper_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload,oal_uint16   us_frame_len,mac_user_stru *pst_mac_user);
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_mgmt_sta.h */
