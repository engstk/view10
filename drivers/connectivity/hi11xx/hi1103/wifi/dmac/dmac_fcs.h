

#ifndef __DMAC_FCS_H__
#define __DMAC_FCS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_commom_ops.h"
#include "mac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FCS_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* OFDM 12M: 0x004a0113 6M: 0x004b0113  11b 1M:  0x08000113 */
#define WLAN_PROT_DATARATE_CHN0_1M             (0x08000113)
#define WLAN_PROT_DATARATE_CHN1_1M             (0x08000123)
#define WLAN_PROT_DATARATE_CHN0_6M             (0x004b0113)
#define WLAN_PROT_DATARATE_CHN1_6M             (0x004b0123)
#define WLAN_PROT_DATARATE_CHN0_12M            (0x004a0113)
#define WLAN_PROT_DATARATE_CHN1_12M            (0x004a0123)
#define WLAN_PROT_DATARATE_CHN0_24M            (0x00490113)
#define WLAN_PROT_DATARATE_CHN1_24M            (0x00490123)

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


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern  oal_void mac_fcs_notify_chain_init(mac_fcs_notify_chain_stru *pst_chain);

extern  oal_uint32  mac_fcs_init(mac_fcs_mgr_stru *pst_fcs_mgr,
                                  oal_uint8        uc_chip_id,
                                  oal_uint8        uc_device_id);
extern  mac_fcs_err_enum_uint8  mac_fcs_request(mac_fcs_mgr_stru           *pst_fcs_mgr,
                                                mac_fcs_state_enum_uint8   *puc_state,
                                                mac_fcs_cfg_stru           *pst_fcs_cfg);
extern  oal_void    mac_fcs_release(mac_fcs_mgr_stru *pst_fcs_mgr);
extern  oal_uint32 mac_fcs_notify_chain_register (mac_fcs_mgr_stru              *pst_fcs_mgr,
                                                  mac_fcs_notify_type_enum_uint8 uc_notify_type,
                                                  mac_fcs_hook_id_enum_uint8     en_hook_id,
                                                  mac_fcs_notify_func            p_func);

extern oal_uint32  mac_fcs_wait_one_packet_done(mac_fcs_mgr_stru *pst_fcs_mgr);
extern oal_void  mac_fcs_flush_event_by_channel(mac_device_stru *pst_mac_device, mac_channel_stru *pst_chl);
extern  oal_uint32 mac_fcs_notify_chain_unregister(mac_fcs_mgr_stru              *pst_fcs_mgr,
                                                   mac_fcs_notify_type_enum_uint8 uc_notify_type,
                                                   mac_fcs_hook_id_enum_uint8     en_hook_id);
extern  oal_uint32  mac_fcs_notify_chain_destroy(mac_fcs_mgr_stru *pst_fcs_mgr);
extern mac_fcs_err_enum_uint8    dmac_fcs_start_enhanced(
                mac_fcs_mgr_stru            *pst_fcs_mgr,
                mac_fcs_cfg_stru            *pst_fcs_cfg);
extern  mac_fcs_err_enum_uint8    dmac_fcs_start(mac_fcs_mgr_stru *pst_fcs_mgr,
                                                mac_fcs_cfg_stru *pst_fcs_cfg,
                                                hal_one_packet_status_stru *pst_status);
extern mac_fcs_err_enum_uint8   dmac_fcs_start_same_channel( mac_fcs_mgr_stru  *pst_fcs_mgr, mac_fcs_cfg_stru *pst_fcs_cfg,
                                        hal_one_packet_status_stru  *pst_status);
extern mac_fcs_err_enum_uint8    dmac_fcs_start_enhanced_same_channel(
                mac_fcs_mgr_stru            *pst_fcs_mgr,
                mac_fcs_cfg_stru            *pst_fcs_cfg);
extern oal_uint32  dmac_fcs_set_prot_datarate(mac_vap_stru *pst_src_vap);
extern oal_void dmac_fcs_send_one_packet_start(mac_fcs_mgr_stru *pst_fcs_mgr,
                                           hal_one_packet_cfg_stru *pst_one_packet_cfg,
                                           hal_to_dmac_device_stru *pst_device,
                                           hal_one_packet_status_stru *pst_status,
                                           oal_bool_enum_uint8  en_ps);
extern oal_void  dmac_fcs_prepare_one_packet_cfg(
                    mac_vap_stru                *pst_mac_vap,
                    hal_one_packet_cfg_stru     *pst_one_packet_cfg,
                    oal_uint16                   us_protect_time);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_fcs.h */

