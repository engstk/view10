

#ifndef __DMAC_TID_H__
#define __DMAC_TID_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "dmac_ext_if.h"
#include "oam_ext_if.h"
#include "dmac_user.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_TID_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_EVENT
#define DMAC_EVENT_PAUSE_OR_RESUME_TID(_puc_macaddr, _uc_vap_id, en_event_type, _puc_string)  oam_event_report(_puc_macaddr, _uc_vap_id, OAM_MODULE_ID_DMAC, en_event_type, _puc_string)
#define DMAC_EVENT_PAUSE_OR_RESUME_USER(_puc_macaddr, _uc_vap_id, en_event_type, _puc_string) oam_event_report(_puc_macaddr, _uc_vap_id, OAM_MODULE_ID_DMAC, en_event_type, _puc_string)
#else
#define DMAC_EVENT_PAUSE_OR_RESUME_TID(_puc_macaddr, _uc_vap_id, en_event_type, _puc_string)
#define DMAC_EVENT_PAUSE_OR_RESUME_USER(_puc_macaddr, _uc_vap_id, en_event_type, _puc_string) ((void)(_puc_string))
#endif


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
typedef oal_uint8  (*del_mpdu_head)(mac_device_stru *pst_dev,
                                             dmac_tid_stru  *pst_tid_queue,  oal_uint16 us_mpdu_num);

typedef oal_uint8 (*del_mpdu_tail)(mac_device_stru *pst_dev,
                                              dmac_tid_stru *pst_tid_queue, oal_uint16 us_mpdu_num);

typedef oal_uint8  (*queue_enq_head)(mac_user_stru *pst_user, dmac_tid_stru *pst_tid_queue, oal_uint8 uc_mpdu_num);


typedef oal_void  (*tx_q_init)(dmac_tid_stru *past_tx_tid_queue, mac_user_stru *pst_user, oal_uint32 *pst_ret);
typedef oal_void  (*tx_q_exit)(mac_user_stru *pst_mac_user);
typedef oal_uint32  (*p_dmac_tid_resume_cb)(hal_to_dmac_device_stru *pst_hal_device, dmac_tid_stru *pst_tid, oal_uint8 uc_type);
typedef oal_void  (*p_dmac_tid_tx_enqueue_update_cb)(mac_device_stru *pst_device, dmac_tid_stru *pst_tid_queue, oal_uint8 uc_mpdu_num);

typedef struct
{
    del_mpdu_head del_mpdu_head_cb;
    del_mpdu_tail del_mpdu_tail_cb;
    queue_enq_head queue_enq_head_cb;
    tx_q_init      tx_q_init_cb;
    tx_q_exit      tx_q_exit_cb;
    p_dmac_tid_resume_cb  dmac_tid_resume_cb;
    p_dmac_tid_tx_enqueue_update_cb  dmac_tid_tx_enqueue_update_cb;
}dmac_tid_cb;
extern dmac_tid_cb g_st_dmac_tid_rom_cb;
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
  结构名  : dmac_tid_stru
  结构说明: tid缓存队列结构体定义
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
extern oal_uint32  dmac_tid_tx_queue_init(dmac_tid_stru *past_tid_queue, mac_user_stru *pst_user);
extern oal_void  dmac_tid_tx_enqueue_update(mac_device_stru *pst_device, dmac_tid_stru *pst_tid_queue, oal_uint8 uc_mpdu_num);
extern oal_void  dmac_tid_tx_dequeue_update(mac_device_stru *pst_device, dmac_tid_stru *pst_tid_queue, oal_uint8 uc_mpdu_num);
#if 0
extern oal_void  dmac_tid_flush_retry_frame(mac_device_stru *pst_device, dmac_tid_stru *pst_tid);
#endif
extern oal_uint32  dmac_tid_tx_queue_enqueue_head(mac_user_stru *pst_user, dmac_tid_stru *pst_tid_queue, oal_dlist_head_stru *pst_tx_dscr_list_hdr, oal_uint8 uc_mpdu_num);
extern oal_uint32  dmac_tid_clear(mac_user_stru *pst_mac_user, mac_device_stru *pst_mac_device);
extern oal_uint32  dmac_tid_get_normal_rate_stats(OAL_CONST mac_user_stru * OAL_CONST pst_mac_user, oal_uint8 uc_tid_id, dmac_tx_normal_rate_stats_stru **ppst_rate_stats_info);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_uint32  dmac_tid_tx_enqueue_tid_head(mac_user_stru *pst_user, dmac_tid_stru *pst_tid_queue, oal_netbuf_stru *pst_netbuf);
#endif
#ifdef _PRE_WLAN_DFT_EVENT

OAL_STATIC OAL_INLINE oal_void  dmac_tid_status_change_event_to_sdt(
                                     dmac_tid_stru        *pst_tid,
                                     oal_uint8             uc_is_tid_paused)
{
    oal_uint8       auc_event[50] = {0};

    auc_event[0] = uc_is_tid_paused;

    DMAC_EVENT_PAUSE_OR_RESUME_TID(BROADCAST_MACADDR,
                                   pst_tid->uc_vap_id,
                                   OAM_EVENT_PAUSE_OR_RESUME_TID,
                                   auc_event);

}
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_tid.h */
