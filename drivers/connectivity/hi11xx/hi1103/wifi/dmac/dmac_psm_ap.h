

#ifndef __DMAC_PSM_AP_H__
#define __DMAC_PSM_AP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "mac_vap.h"
#include "dmac_vap.h"
#include "dmac_user.h"


/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define MAX_MPDU_NUM_IN_PS_QUEUE    128

#define DMAC_PSM_CHANGE_USER_PS_STATE(_bit_ps, _en_val)  ((_bit_ps) = (_en_val))

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
extern oal_uint32  dmac_psm_resv_ps_poll(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user);
extern oal_void dmac_psm_set_local_bitmap(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_uint8 uc_bitmap_flg);
#if 0
extern oal_uint8 dmac_psm_get_local_bitmap(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user);
extern oal_uint8 dmac_psm_get_bitmap_offset(dmac_vap_stru *pst_dmac_vap);
extern oal_uint8 dmac_psm_get_bitmap_len(dmac_vap_stru *pst_dmac_vap);
#endif
extern oal_uint32 dmac_psm_send_null_data(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_bool_enum_uint8 en_ps);
extern oal_uint32 dmac_psm_user_ps_structure_init(dmac_user_stru *pst_dmac_user);
extern oal_uint32  dmac_psm_user_ps_structure_destroy(dmac_user_stru *pst_dmac_user);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_void dmac_psm_delete_ps_queue_head(dmac_user_stru *pst_dmac_user,oal_uint32 ul_psm_delete_num);
#endif
extern oal_netbuf_stru* dmac_psm_dequeue_first_mpdu(dmac_user_ps_stru  *pst_ps_structure);
extern oal_void dmac_psm_set_more_data(oal_netbuf_stru *pst_net_buf);
extern oal_void dmac_psm_clear_ps_queue(dmac_user_stru *pst_dmac_user);
extern oal_void dmac_psm_queue_flush(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user);
extern oal_uint32 dmac_psm_ps_enqueue(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_net_buf);
extern oal_uint8 dmac_psm_pkt_need_buff(mac_device_stru *pst_mac_device, dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_net_buf);
extern oal_void dmac_psm_rx_process(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_net_buf);
extern oal_uint32  dmac_psm_reset(frw_event_mem_stru *pst_event_mem);
extern oal_uint32  dmac_psm_tx_set_more_data(dmac_user_stru *pst_dmac_user,
                                               mac_tx_ctl_stru *pst_tx_cb);
#ifdef _PRE_WLAN_MAC_BUGFIX_PSM_BACK
extern oal_uint32  dmac_psm_flush_txq_to_tid(mac_device_stru *pst_mac_device,dmac_vap_stru  *pst_dmac_vap,dmac_user_stru *pst_dmac_user);
#endif

extern oal_void  dmac_change_null_data_rate(dmac_vap_stru *pst_dmac_vap,dmac_user_stru *pst_dmac_user,oal_uint8 *uc_protocol_mode,oal_uint8 *uc_legacy_rate);
oal_uint32  dmac_psm_doze(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user);

extern oal_void dmac_psm_update_beacon(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user);
extern oal_void dmac_psm_set_ucast_mgmt_tx_rate(dmac_vap_stru *pst_dmac_vap,
                                                    wlan_channel_band_enum_uint8 en_band,
                                                    oal_uint8 uc_legacy_rate,
                                                    wlan_phy_protocol_enum_uint8 en_protocol_mode);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_psm_ap.h */
