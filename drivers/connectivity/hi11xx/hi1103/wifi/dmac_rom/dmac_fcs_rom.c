
#ifdef  __cplusplus
#if     __cplusplus
extern  "C" {
#endif
#endif

/*****************************************************************************
  1 头文件包含
 *****************************************************************************/
#include    "wlan_spec.h"
#include    "mac_device.h"
#include    "dmac_main.h"
#include    "dmac_mgmt_bss_comm.h"
#include    "mac_regdomain.h"
#include    "dmac_tx_bss_comm.h"

#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_sta_pm.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FCS_ROM_C

/*****************************************************************************
  2 全局变量定义
 *****************************************************************************/

/*****************************************************************************
  3 函数实现
****************************************************************************/
oal_void mac_fcs_notify_chain_init(mac_fcs_notify_chain_stru *pst_chain)
{
    oal_memset(pst_chain, 0, sizeof(mac_fcs_notify_chain_stru));
}

oal_uint32    mac_fcs_init(mac_fcs_mgr_stru  *pst_fcs_mgr,
                            oal_uint8         uc_chip_id,
                            oal_uint8         uc_device_id)
{
    oal_uint8        uc_idx;

    if (OAL_PTR_NULL == pst_fcs_mgr)
    {
        return OAL_FAIL;
    }

    if ((uc_chip_id >= WLAN_CHIP_MAX_NUM_PER_BOARD) || (uc_device_id >= MAC_RES_MAX_DEV_NUM))
    {
        return OAL_FAIL;
    }


    oal_memset(pst_fcs_mgr, 0, sizeof(mac_fcs_mgr_stru));

    oal_spin_lock_init(&pst_fcs_mgr->st_lock);

    pst_fcs_mgr->en_fcs_state   = MAC_FCS_STATE_STANDBY;
    pst_fcs_mgr->pst_fcs_cfg    = OAL_PTR_NULL;
    pst_fcs_mgr->uc_chip_id     = uc_chip_id;
    pst_fcs_mgr->uc_device_id   = uc_device_id;
    pst_fcs_mgr->uc_fcs_cnt     = 0;

    for (uc_idx = 0; uc_idx < MAC_FCS_NOTIFY_TYPE_BUTT; uc_idx++)
    {
        mac_fcs_notify_chain_init(pst_fcs_mgr->ast_notify_chain + uc_idx);
    }

    mac_fcs_verify_init();

    return OAL_SUCC;
}


mac_fcs_err_enum_uint8  mac_fcs_request(mac_fcs_mgr_stru             *pst_fcs_mgr,
                                        mac_fcs_state_enum_uint8     *puc_state,
                                        mac_fcs_cfg_stru             *pst_fcs_cfg)
{
    mac_fcs_err_enum_uint8  en_ret;
    //oal_uint                ul_irq_save;

    if (pst_fcs_mgr == OAL_PTR_NULL)
    {
        return MAC_FCS_ERR_NULL_PTR;
    }

    /* 02mips优化，51&02 fcs暂无并发，注掉 */
    //oal_spin_lock_irq_save(&pst_fcs_mgr->st_lock, &ul_irq_save);

    if (puc_state != OAL_PTR_NULL)
    {
        *puc_state = pst_fcs_mgr->en_fcs_state;
    }

    /*lint -save -e825 */
    switch (pst_fcs_mgr->en_fcs_state)
    {
        case    MAC_FCS_STATE_STANDBY    :
                    pst_fcs_mgr->en_fcs_state   = MAC_FCS_STATE_REQUESTED;
                    en_ret                      = MAC_FCS_SUCCESS;
                    break;

        case    MAC_FCS_STATE_IN_PROGESS :
                    if (pst_fcs_cfg != OAL_PTR_NULL)
                    {
                        *pst_fcs_cfg = *pst_fcs_mgr->pst_fcs_cfg;
                    }
                    /* no break  */

        case    MAC_FCS_STATE_REQUESTED :
                    en_ret = MAC_FCS_ERR_BUSY;
                    break;

        default:
                    en_ret = MAC_FCS_ERR_UNKNOWN_ERR;
                    break;
    }
    /*lint -restore */

    //oal_spin_unlock_irq_restore(&pst_fcs_mgr->st_lock, &ul_irq_save);

    return en_ret;
}

oal_void    mac_fcs_release(mac_fcs_mgr_stru *pst_fcs_mgr)
{
#if 0
    if (pst_fcs_mgr != OAL_PTR_NULL)
    {
        oal_uint    ul_irq_save;

        oal_spin_lock_irq_save(&pst_fcs_mgr->st_lock, &ul_irq_save);
        pst_fcs_mgr->en_fcs_state   = MAC_FCS_STATE_STANDBY;
        oal_spin_unlock_irq_restore(&pst_fcs_mgr->st_lock, &ul_irq_save);
    }
#endif

    pst_fcs_mgr->en_fcs_state = MAC_FCS_STATE_STANDBY;
}


oal_void  mac_fcs_flush_event_by_channel(mac_device_stru *pst_mac_device, mac_channel_stru *pst_chl)
{
    oal_uint8               uc_vap_idx;
    mac_vap_stru           *pst_mac_vap;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_SCAN, "pst_mac_vap null.", pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (mac_fcs_is_same_channel(&(pst_mac_vap->st_channel), pst_chl))
        {
            frw_event_vap_flush_event(pst_mac_vap->uc_vap_id, FRW_EVENT_TYPE_WLAN_TX_COMP, OAL_FALSE);
        }
    }
}


oal_uint32  mac_fcs_wait_one_packet_done(mac_fcs_mgr_stru *pst_fcs_mgr)
{
    oal_uint32 ul_delay_cnt = 0;

    while (OAL_TRUE != pst_fcs_mgr->en_fcs_done)
    {
        /* en_fcs_done will be set 1 in one_packet_done_isr */
        oal_udelay(10);

        ul_delay_cnt++;

        /* 10us*1000 = 10ms */
        if (ul_delay_cnt > MAC_ONE_PACKET_TIME_OUT)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "wait one packet done timeout > 10ms !");
            return OAL_FAIL;
        }
    }

    return OAL_SUCC;
}

oal_void dmac_fcs_send_one_packet_start(mac_fcs_mgr_stru *pst_fcs_mgr,
                                            hal_one_packet_cfg_stru *pst_one_packet_cfg,
                                            hal_to_dmac_device_stru *pst_device,
                                         hal_one_packet_status_stru *pst_status,
                                                oal_bool_enum_uint8  en_ps)
{
    oal_uint32                  ul_ret;
    mac_ieee80211_frame_stru   *pst_mac_header;

    /* 准备报文 */
    if (HAL_FCS_PROTECT_TYPE_NULL_DATA == pst_one_packet_cfg->en_protect_type)
    {
        pst_mac_header = (mac_ieee80211_frame_stru *)pst_one_packet_cfg->auc_protect_frame;
        pst_mac_header->st_frame_control.bit_power_mgmt = en_ps;
    }

    pst_fcs_mgr->en_fcs_done    = OAL_FALSE;
    mac_fcs_verify_timestamp(MAC_FCS_STAGE_ONE_PKT_START);

    /* 启动发送 */
    hal_one_packet_start(pst_device, pst_one_packet_cfg);
#if ((_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)||(_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION)) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
    pst_fcs_mgr->en_fcs_done = OAL_TRUE;
#endif

    /* 等待发送结束 */
    ul_ret = mac_fcs_wait_one_packet_done(pst_fcs_mgr);
    if (OAL_SUCC != ul_ret)
    {
        hal_show_fsm_info(pst_device);
    }

    mac_fcs_verify_timestamp(MAC_FCS_STAGE_ONE_PKT_DONE);
    if (OAL_PTR_NULL != pst_status)
    {
        hal_one_packet_get_status(pst_device, pst_status);
    }
}
/*lint -e578*//*lint -e19*/
oal_module_symbol(mac_fcs_init);
oal_module_symbol(mac_fcs_request);
oal_module_symbol(mac_fcs_release);
oal_module_symbol(dmac_fcs_send_one_packet_start);

#ifdef  __cplusplus
#if     __cplusplus
    }
#endif
#endif

