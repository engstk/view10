

#ifndef __DMAC_POWER_H__
#define __DMAC_POWER_H__

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
#include "dmac_vap.h"
#include "dmac_ext_if.h"

#include "wlan_spec.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_POWER_H

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
extern oal_void dmac_pow_set_vap_tx_power(mac_vap_stru *pst_mac_vap, hal_pow_set_type_enum_uint8 uc_type);
//extern oal_void dmac_pow_init_vap_info(dmac_vap_stru *pst_dmac_vap);
extern oal_void dmac_pow_init_user_info(mac_vap_stru *pst_mac_vap, dmac_user_stru *pst_dmac_user);
extern oal_uint32 dmac_pow_tx_power_process(dmac_vap_stru *pst_dmac_vap, mac_user_stru *pst_user,
                                        mac_tx_ctl_stru *pst_cb, hal_tx_txop_alg_stru *pst_txop_param);
extern oal_uint32  dmac_pow_get_user_target_tx_power(mac_user_stru *pst_mac_user,
                                 hal_tx_dscr_ctrl_one_param *pst_tx_dscr_param, oal_int16 *ps_tx_pow);
#ifdef _PRE_WLAN_FEATURE_USER_RESP_POWER
extern oal_uint32  dmac_pow_change_mgmt_power_process(mac_vap_stru *pst_vap, oal_int8 c_rssi);
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_power.h */
