

#ifndef __DMAC_OPMODE_H__
#define __DMAC_OPMODE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "mac_resource.h"
#include "mac_device.h"
#include "mac_user.h"
#include "mac_frame.h"
#include "mac_ie.h"
#include "dmac_user.h"
#include "dmac_vap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_OPMODE_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
typedef oal_void (*encap_opmode_action)(mac_vap_stru *pst_mac_vap,
             oal_netbuf_stru *pst_netbuf, oal_uint16 *puc_len, wlan_nss_enum_uint8 en_nss);



typedef struct
{
    encap_opmode_action  encap_opmode_action_cb;

}dmac_opmode_cb;
extern dmac_opmode_cb g_st_dmac_opmode_rom_cb;
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
extern  oal_void dmac_mgmt_encap_opmode_notify_action(mac_vap_stru *pst_mac_vap,
        oal_netbuf_stru *pst_netbuf, oal_uint16 *puc_len, oal_bool_enum_uint8 en_bool, wlan_nss_enum_uint8 en_nss);
extern oal_uint32  dmac_mgmt_send_opmode_notify_action(mac_vap_stru *pst_mac_vap, wlan_nss_enum_uint8 en_nss, oal_bool_enum_uint8 en_bool);
extern oal_uint32 dmac_check_opmode_notify(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_payload, oal_uint32 ul_msg_len, mac_user_stru *pst_mac_user);
extern oal_uint32  dmac_mgmt_rx_opmode_notify_frame(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_netbuf);
extern oal_uint32 dmac_ie_proc_opmode_notify(mac_user_stru *pst_mac_user, mac_vap_stru *pst_mac_vap, mac_opmode_notify_stru *pst_opmode_notify);
#endif /* end of _PRE_WLAN_FEATURE_OPMODE_NOTIFY*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_smps.h */

