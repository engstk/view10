


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_csa_sta.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CSA_STA_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint8 dmac_sta_csa_fsm_get_current_state(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->st_sta_csa_fsm_info.st_oal_fsm.uc_cur_state;
}



oal_bool_enum dmac_sta_csa_is_in_waiting(mac_vap_stru *pst_mac_vap)
{
    if((dmac_sta_csa_fsm_get_current_state(pst_mac_vap) >= WLAN_STA_CSA_FSM_START) &&
        (dmac_sta_csa_fsm_get_current_state(pst_mac_vap) <= WLAN_STA_CSA_FSM_WAIT))
    {
        return OAL_TRUE;
    }
    else
    {
        return OAL_FALSE;
    }
}


oal_uint32 dmac_sta_csa_fsm_post_event(mac_vap_stru* pst_mac_vap, oal_uint16 us_type, oal_uint16 us_datalen, oal_uint8* pst_data)
{
    mac_sta_csa_fsm_info_stru       *pst_handler = OAL_PTR_NULL;
    oal_uint32                      ul_ret;

    if(us_type >= WLAN_STA_CSA_EVENT_BUTT)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "dmac_sta_csa_fsm_post_event:: event type[%d] error, NULL!",us_type);
        return OAL_FAIL;
    }

    pst_handler = &(pst_mac_vap->st_sta_csa_fsm_info);

    if (OAL_FALSE == pst_handler->en_is_fsm_attached)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "dmac_sta_csa_fsm_post_event::csa_fsm_attached = %d!",pst_handler->en_is_fsm_attached);
        return OAL_FAIL;
    }

    ul_ret = oal_fsm_event_dispatch(&(pst_handler->st_oal_fsm), us_type, us_datalen, pst_data);

    return ul_ret;
}



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

