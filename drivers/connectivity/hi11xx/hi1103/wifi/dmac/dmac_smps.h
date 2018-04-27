

#ifndef __DMAC_SMPS_H__
#define __DMAC_SMPS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "mac_resource.h"
#include "dmac_ext_if.h"
#include "mac_device.h"
#include "mac_vap.h"
#include "dmac_user.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_SMPS_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/


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
//extern oal_void dmac_smps_update_device_capbility(mac_vap_stru *pst_mac_vap);
//extern oal_void dmac_smps_check_rx_action(hal_to_dmac_device_stru *pst_hal_device, oal_netbuf_stru* pst_buf);
extern oal_void dmac_smps_send_action(mac_vap_stru *pst_mac_vap,
       wlan_mib_mimo_power_save_enum_uint8  en_smps_mode, oal_bool_enum_uint8 en_bool);
extern oal_void dmac_smps_update_user_capbility(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user);

extern oal_uint32 dmac_check_smps_field(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, oal_uint32 ul_msg_len, mac_user_stru *pst_mac_user);

extern oal_uint32  dmac_mgmt_rx_smps_frame(mac_vap_stru *pst_mac_vap, dmac_user_stru *pst_dmac_user, oal_uint8 *puc_frame_payload);
#endif /* end of _PRE_WLAN_FEATURE_SMPS*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hmac_smps.h */

