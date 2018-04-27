


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "hal_ext_if.h"
#include "hal_device.h"
#include "hal_device_fsm.h"
#include "dmac_alg.h"
#include "dmac_vap.h"
#include "dmac_scan.h"
#include "dmac_tx_complete.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_smps.h"
#include "dmac_opmode.h"
#include "dmac_config.h"
#include "dmac_fcs.h"
#include "dmac_m2s.h"
#include "dmac_txopps.h"
#include "dmac_power.h"
#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "hal_pm.h"
#include "dmac_psm_sta.h"
#endif
#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_M2S_C

/*****************************************************************************
  1 函数申明
*****************************************************************************/
OAL_STATIC oal_void dmac_m2s_state_idle_entry(oal_void *p_ctx);
OAL_STATIC oal_void dmac_m2s_state_idle_exit(oal_void *p_ctx);
OAL_STATIC oal_uint32 dmac_m2s_state_idle_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_void dmac_m2s_state_mimo_entry(oal_void *p_ctx);
OAL_STATIC oal_void dmac_m2s_state_mimo_exit(oal_void *p_ctx);
OAL_STATIC oal_uint32 dmac_m2s_state_mimo_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_void dmac_m2s_state_miso_entry(oal_void *p_ctx);
OAL_STATIC oal_void dmac_m2s_state_miso_exit(oal_void *p_ctx);
OAL_STATIC oal_uint32 dmac_m2s_state_miso_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_void dmac_m2s_state_siso_entry(oal_void *p_ctx);
OAL_STATIC oal_void dmac_m2s_state_siso_exit(oal_void *p_ctx);
OAL_STATIC oal_uint32 dmac_m2s_state_siso_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_void dmac_m2s_state_simo_entry(oal_void *p_ctx);
OAL_STATIC oal_void dmac_m2s_state_simo_exit(oal_void *p_ctx);
OAL_STATIC oal_uint32 dmac_m2s_state_simo_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data);
OAL_STATIC oal_uint32 dmac_m2s_fsm_trans_to_state(hal_m2s_fsm_stru *pst_m2s_fsm, oal_uint8 uc_state);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 各状态入口出口函数 */
OAL_STATIC oal_fsm_state_info  g_ast_hal_m2s_fsm_info[] = {
    {
        HAL_M2S_STATE_IDLE,
        "IDLE",
        dmac_m2s_state_idle_entry,
        dmac_m2s_state_idle_exit,
        dmac_m2s_state_idle_event,
    },
    {
        HAL_M2S_STATE_SISO,
        "SISO",
        dmac_m2s_state_siso_entry,
        dmac_m2s_state_siso_exit,
        dmac_m2s_state_siso_event,
    },
    {
        HAL_M2S_STATE_MIMO,
        "MIMO",
        dmac_m2s_state_mimo_entry,
        dmac_m2s_state_mimo_exit,
        dmac_m2s_state_mimo_event,
    },
    {
        HAL_M2S_STATE_MISO,
        "MISO",
        dmac_m2s_state_miso_entry,
        dmac_m2s_state_miso_exit,
        dmac_m2s_state_miso_event,
    },
    {
        HAL_M2S_STATE_SIMO,
        "SIMO",
        dmac_m2s_state_simo_entry,
        dmac_m2s_state_simo_exit,
        dmac_m2s_state_simo_event,
    },
};

/*****************************************************************************
  3 函数定义
*****************************************************************************/

oal_void  dmac_m2s_mgr_param_info(hal_to_dmac_device_stru *pst_hal_device)
{
    OAM_WARNING_LOG_ALTER(0, OAM_SF_M2S,
        "{dmac_m2s_switch_mgr_param_info::m2s_type[%d],cur state[%d],mode_stru[%d],miso_hold[%d].}",
        4, GET_HAL_M2S_SWITCH_TPYE(pst_hal_device), GET_HAL_M2S_CUR_STATE(pst_hal_device),
        GET_HAL_M2S_MODE_TPYE(pst_hal_device), GET_HAL_DEVICE_M2S_DEL_SWI_MISO_HOLD(pst_hal_device));

    OAM_WARNING_LOG_ALTER(0, OAM_SF_M2S,
        "{dmac_m2s_switch_mgr_param_info::nss_num[%d]uc_phy_chain[%d]single_txrx_chain[%d]uc_rf_chain[%d]uc_phy2dscr_chain[%d].}",
        5, pst_hal_device->st_cfg_cap_info.en_nss_num,
        pst_hal_device->st_cfg_cap_info.uc_phy_chain, pst_hal_device->st_cfg_cap_info.uc_single_tx_chain,
        pst_hal_device->st_cfg_cap_info.uc_rf_chain, pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain);
}


hal_m2s_state_uint8  dmac_m2s_event_state_classify(hal_m2s_event_tpye_uint16 en_m2s_event)
{
    hal_m2s_state_uint8    uc_new_m2s_state = HAL_M2S_STATE_BUTT;

    /* 后续根据业务需求，进行对应case添加 TBD */
    switch(en_m2s_event)
    {
        /* 并发扫描切换到SIMO,软件还是mimo,硬件切siso, 业务场景下不交叉，siso采用c0 */
        case HAL_M2S_EVENT_SCAN_BEGIN:
            uc_new_m2s_state = HAL_M2S_STATE_SIMO;
            break;

        /* 并发扫描结束切换回MIMO,软件和硬件都是mimo */
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_BT_SISO_TO_MIMO:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_MIMO:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_MIMO:
        case HAL_M2S_EVENT_DBDC_STOP:
        case HAL_M2S_EVENT_DBDC_SISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
            uc_new_m2s_state = HAL_M2S_STATE_MIMO;
            break;

        /* RSSI切换到MISO,软件切换到SISO,硬件还是mimo */
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO:
        case HAL_M2S_EVENT_ANT_RSSI_SISO_TO_MISO:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0:
            uc_new_m2s_state = HAL_M2S_STATE_MISO;
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C0_SISO:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_DBDC_MIMO_TO_SISO:
        case HAL_M2S_EVENT_DBDC_START:
        case HAL_M2S_EVENT_BT_MIMO_TO_SISO:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C1_SISO:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0:
            uc_new_m2s_state = HAL_M2S_STATE_SISO;
            break;

        default:
           OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_m2s_event_state_classify: en_m2s_event[%d] error!}", en_m2s_event);
    }

    return uc_new_m2s_state;
}


hal_m2s_state_uint8  dmac_m2s_chain_state_classify(hal_to_dmac_device_stru *pst_hal_device)
{
    /* 先判断软件状态 */
    if (WLAN_PHY_CHAIN_DOUBLE == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain)
    {
        if(WLAN_PHY_CHAIN_DOUBLE == pst_hal_device->st_cfg_cap_info.uc_phy_chain)
        {
            return HAL_M2S_STATE_MIMO;
        }
        else
        {
            return HAL_M2S_STATE_SIMO;
        }
    }
    else /* 软件处于siso */
    {
        if(WLAN_PHY_CHAIN_DOUBLE == pst_hal_device->st_cfg_cap_info.uc_phy_chain)
        {
            return HAL_M2S_STATE_MISO;
        }
        else
        {
            return HAL_M2S_STATE_SISO;
        }
    }
}


wlan_m2s_trigger_mode_enum_uint8  dmac_m2s_event_trigger_mode_classify(hal_m2s_event_tpye_uint16 en_m2s_event)
{
    wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;

    switch(en_m2s_event)
    {
        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
        case HAL_M2S_EVENT_SCAN_END:
            /* 快速扫描，需要单独判断快速扫描开启时候置标记，因为共用了SCAN_BEGIN标记 */
            //uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_FAST_SCAN;
            break;

        case HAL_M2S_EVENT_DBDC_START:
        case HAL_M2S_EVENT_DBDC_STOP:
        case HAL_M2S_EVENT_DBDC_SISO_TO_MIMO:
        case HAL_M2S_EVENT_DBDC_MIMO_TO_SISO:
            uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_DBDC;
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C1_SISO:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C0_SISO:
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO:
        case HAL_M2S_EVENT_ANT_RSSI_SISO_TO_MISO:
            uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_RSSI_SNR;
            break;

        case HAL_M2S_EVENT_BT_SISO_TO_MIMO:
        case HAL_M2S_EVENT_BT_MIMO_TO_SISO:
            uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BTCOEX;
            break;

        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
            uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_COMMAND;
            break;

        case HAL_M2S_EVENT_TEST_SISO_C0_TO_MIMO:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_MIMO:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1:
            uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_TEST;
            break;

        default:
           OAM_WARNING_LOG1(0, OAM_SF_M2S, "{dmac_m2s_event_trigger_mode_classify: en_m2s_event[%d] other service!}", en_m2s_event);
    }

    return uc_trigger_mode;
}


oal_void  dmac_m2s_update_switch_mgr_param(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    /* 后续根据业务需求，进行对应case添加 TBD */
    switch(en_m2s_event)
    {
        /* 并发扫描切换到SIMO,软件还是mimo,硬件切siso, 业务场景下不交叉，siso采用c0 */
        case HAL_M2S_EVENT_SCAN_BEGIN:
            pst_hal_device->st_cfg_cap_info.en_nss_num         = WLAN_DOUBLE_NSS;
            pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain  = WLAN_PHY_CHAIN_DOUBLE;
            pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = WLAN_PHY_CHAIN_ZERO;
            pst_hal_device->st_cfg_cap_info.uc_rf_chain        = WLAN_RF_CHAIN_ZERO;
            pst_hal_device->st_cfg_cap_info.uc_phy_chain       = WLAN_PHY_CHAIN_ZERO;
            break;

        /* 并发扫描结束切换回MIMO,软件和硬件都是mimo */
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_MIMO:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
            pst_hal_device->st_cfg_cap_info.en_nss_num         = WLAN_DOUBLE_NSS;
            pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain  = WLAN_PHY_CHAIN_DOUBLE;
            pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = WLAN_PHY_CHAIN_ZERO;
            pst_hal_device->st_cfg_cap_info.uc_rf_chain        = WLAN_RF_CHAIN_DOUBLE;
            pst_hal_device->st_cfg_cap_info.uc_phy_chain       = WLAN_PHY_CHAIN_DOUBLE;
            break;

        /* C0 MISO 例如RSSI切换到MISO,软件切换到SISO,硬件还是mimo */
        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO:
        case HAL_M2S_EVENT_ANT_RSSI_SISO_TO_MISO:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0:
            pst_hal_device->st_cfg_cap_info.en_nss_num         = WLAN_SINGLE_NSS;
            pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain  = WLAN_PHY_CHAIN_ZERO;
            pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = WLAN_PHY_CHAIN_ZERO;
            pst_hal_device->st_cfg_cap_info.uc_rf_chain        = WLAN_RF_CHAIN_DOUBLE;
            pst_hal_device->st_cfg_cap_info.uc_phy_chain       = WLAN_PHY_CHAIN_DOUBLE;
            break;

        /* C1 MISO */
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1:
            pst_hal_device->st_cfg_cap_info.en_nss_num         = WLAN_SINGLE_NSS;
            pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain  = WLAN_PHY_CHAIN_ONE;
            pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = WLAN_PHY_CHAIN_ONE;
            pst_hal_device->st_cfg_cap_info.uc_rf_chain        = WLAN_RF_CHAIN_DOUBLE;
            pst_hal_device->st_cfg_cap_info.uc_phy_chain       = WLAN_PHY_CHAIN_DOUBLE;
            break;

        /* 主phy0的siso,软件和硬件都切siso */
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C0_SISO:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_DBDC_MIMO_TO_SISO:
        case HAL_M2S_EVENT_DBDC_START:
        case HAL_M2S_EVENT_BT_MIMO_TO_SISO:
            pst_hal_device->st_cfg_cap_info.en_nss_num         = WLAN_SINGLE_NSS;
            pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain  = WLAN_PHY_CHAIN_ZERO;
            pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = WLAN_PHY_CHAIN_ZERO;
            pst_hal_device->st_cfg_cap_info.uc_rf_chain        = WLAN_RF_CHAIN_ZERO;
            pst_hal_device->st_cfg_cap_info.uc_phy_chain       = WLAN_PHY_CHAIN_ZERO;
            break;

        /* 主phy1的siso,软件和硬件都切siso */
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C1_SISO:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
            pst_hal_device->st_cfg_cap_info.en_nss_num         = WLAN_SINGLE_NSS;
            pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain  = WLAN_PHY_CHAIN_ONE;
            pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = WLAN_PHY_CHAIN_ONE;
            pst_hal_device->st_cfg_cap_info.uc_rf_chain        = WLAN_RF_CHAIN_ONE;
            pst_hal_device->st_cfg_cap_info.uc_phy_chain       = WLAN_PHY_CHAIN_ONE;
            break;

        default:
           OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_m2s_update_switch_mgr_param: en_m2s_event[%d] error!}", en_m2s_event);
    }

    dmac_m2s_mgr_param_info(pst_hal_device);
}


oal_bool_enum_uint8 dmac_m2s_switch_priority_check(hal_to_dmac_device_stru *pst_hal_device,
    wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode, hal_m2s_state_uint8 uc_new_m2s_state)
{
    oal_bool_enum_uint8 en_allow = OAL_TRUE; /* 初始申请业务允许 */

    /* 1.对于切换miso/siso 需要判断其他业务是否已经在切换 */
    if(HAL_M2S_STATE_BUTT == uc_new_m2s_state)
    {
        OAM_ERROR_LOG2(0, OAM_SF_M2S, "{dmac_m2s_switch_priority_check::uc_trigger_mode[%d] uc_new_m2s_state[%d]error.}", uc_trigger_mode, uc_new_m2s_state);
    }
    else if(HAL_M2S_STATE_MIMO == uc_new_m2s_state || HAL_M2S_STATE_MISO == uc_new_m2s_state)
    {
        /* 要切换到miso 或者mimo时，如果有其他业务已经在执行了，状态机都不能执行(其他业务在执行，都是在siso状态，不能恢复)
         具体业务可以根据是恢复到mimo时，如果失败，仍可以释放自己业务侧功能资源，需要一起联调 TBD */
        switch(uc_trigger_mode)
        {
            /* dbdc先来，再来并发扫描是允许的 */
            case WLAN_M2S_TRIGGER_MODE_FAST_SCAN:
                if(0 != (*(oal_uint8 *)(&pst_hal_device->st_hal_m2s_fsm.st_m2s_mode)& (BIT2 | BIT3 | BIT4 | BIT5)))
                {
                    en_allow = OAL_FALSE;
                }
              break;

            case WLAN_M2S_TRIGGER_MODE_DBDC:
            case WLAN_M2S_TRIGGER_MODE_RSSI_SNR:
            case WLAN_M2S_TRIGGER_MODE_BTCOEX:
            case WLAN_M2S_TRIGGER_MODE_COMMAND:
            case WLAN_M2S_TRIGGER_MODE_TEST:
                /* 处于本状态才允许 */
                if(0 != ((*(oal_uint8 *)(&pst_hal_device->st_hal_m2s_fsm.st_m2s_mode)& (~uc_trigger_mode))))
                {
                    en_allow = OAL_FALSE;
                }
              break;

            default:
                OAM_WARNING_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_priority_check::uc_trigger_mode[%d] error.}", uc_trigger_mode);
              break;
         }
    }
    else if(HAL_M2S_STATE_SIMO == uc_new_m2s_state || HAL_M2S_STATE_SISO == uc_new_m2s_state)
    {
        /* 要切换到simo 或者siso时，如果有其他业务已经在执行了， 一般只要不是处于miso状态，都可以执行，这个miso会在其他逻辑判断 */
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_switch_priority_check::uc_trigger_mode[%d] uc_new_m2s_state[%d]error.}", uc_trigger_mode, uc_new_m2s_state);
    }

    return en_allow;
}


oal_void dmac_m2s_switch_back_to_mixo_check(hal_m2s_fsm_stru *pst_m2s_fsm, hal_to_dmac_device_stru *pst_hal_device,
    wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode, hal_m2s_state_uint8 *pen_new_state)
{
    if(HAL_M2S_STATE_MIMO == GET_HAL_M2S_CUR_STATE(pst_hal_device) || HAL_M2S_STATE_BUTT == GET_HAL_M2S_CUR_STATE(pst_hal_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_back_to_mixo_check::cur_m2s_state[%d] error.}", GET_HAL_M2S_CUR_STATE(pst_hal_device));
        return;
    }

    /* 要切换到miso 或者mimo时，如果有其他业务已经在执行了，状态机都不能执行(其他业务在执行，都是在siso状态，不能恢复)
     具体业务可以根据是恢复到mimo时，如果失败，仍可以释放自己业务侧功能资源，需要一起联调 TBD */

    /* 1.清零本业务状态 */
    GET_HAL_M2S_MODE_TPYE(pst_hal_device) &= (~uc_trigger_mode);

    /* 2.判断是否状态已经是清零，根据存在的业务，切换到新的状态，不一定是回到mimo */
    /*(1)simo退出，如果有其他btcoex或者test，切换到siso,否则切换到mimo
      (2)miso退出，如果有其他btcoex或者test，切换到siso，有command，保持miso
      (3)siso退出，如果有其他btcoex，test，command，保持siso */

    OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_switch_back_to_mixo_check::uc_cur_m2s_state[%d] mode_stru[%d].}",
        *pen_new_state, *(oal_uint8 *)(&pst_hal_device->st_hal_m2s_fsm.st_m2s_mode));

    /* 没有其他业务时，可以回到mimo */
    if(0 == (*(oal_uint8 *)(&pst_hal_device->st_hal_m2s_fsm.st_m2s_mode)& (BIT1 | BIT2 | BIT3 | BIT4 | BIT5)))
    {
        dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_MIMO);
    }
    else
    {
        if(HAL_M2S_STATE_SIMO == GET_HAL_M2S_CUR_STATE(pst_hal_device))
        {
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_MIMO);

            /* 1. 当前是simo，期待回mimo，有command业务需要回到mimo再切miso，不要直接miso，本身跳转不支持 */
            if(OAL_TRUE == HAL_M2S_CHECK_COMMAND_ON(pst_hal_device))
            {
                dmac_m2s_handle_event(pst_hal_device, HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0, 0, OAL_PTR_NULL);
            }
        }
        else if(HAL_M2S_STATE_MISO == GET_HAL_M2S_CUR_STATE(pst_hal_device))
        {
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_MIMO);

            /* 1. 当前是miso，期待回mimo，有test业务需要回到mimo再切siso，不要直接siso，本身跳转不支持 */
            if(OAL_TRUE == HAL_M2S_CHECK_TEST_ON(pst_hal_device))
            {
                dmac_m2s_handle_event(pst_hal_device, HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0, 0, OAL_PTR_NULL);
            }
        }
        else
        {
            /* 1. 当前是mimo，期待回mimo，有test业务保持siso */
            if(OAL_TRUE == HAL_M2S_CHECK_TEST_ON(pst_hal_device) || OAL_TRUE == HAL_M2S_CHECK_COMMAND_ON(pst_hal_device))
            {
                OAM_WARNING_LOG1(pst_hal_device->uc_device_id, OAM_SF_M2S, "{dmac_m2s_switch_back_to_mixo_check:: need to keep siso mode_stru[%d]!}",
                    GET_HAL_M2S_MODE_TPYE(pst_hal_device));
            }
            else
            {
                dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_MIMO);

            }
        }
    }
}


oal_bool_enum_uint8  dmac_m2s_switch_apply_and_confirm(hal_to_dmac_device_stru *pst_hal_device,
    oal_uint16 us_m2s_type, wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode)
{
    hal_m2s_state_uint8    uc_new_m2s_state = HAL_M2S_STATE_BUTT;
    oal_bool_enum_uint8    en_allow = OAL_TRUE;

    /* 1.根据切换业务，确认新状态类型 */
    uc_new_m2s_state = dmac_m2s_event_state_classify(us_m2s_type);
    if(HAL_M2S_STATE_BUTT == uc_new_m2s_state)
    {
        return OAL_FALSE;
    }

    /* 2. old和new状态一致，直接返回true给外层并提示,可以继续执行，例如虽然状态不跳转，但是可以在siso时切换通道 */
    if(uc_new_m2s_state == GET_HAL_M2S_CUR_STATE(pst_hal_device))
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_switch_apply_and_confirm: cur state[%d] conform us_m2s_type[%d]!}",
            GET_HAL_M2S_CUR_STATE(pst_hal_device), us_m2s_type);
        return OAL_TRUE;
    }

    /* 3.需要判断业务类型，比如蓝牙场景在c1 siso，rssi逻辑等业务不能执行 command和test的切c0 siso不能支持  TBD 也业务自己判断 */

    return en_allow;
}


wlan_nss_enum_uint8  dmac_m2s_get_bss_max_nss(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_netbuf,
                                                 oal_uint16 us_frame_len, oal_bool_enum_uint8 en_assoc_status)
{
    oal_uint8           *puc_frame_body;
    oal_uint8           *puc_ie = OAL_PTR_NULL;
    oal_uint8           *puc_ht_mcs_bitmask;
    wlan_nss_enum_uint8  en_nss = WLAN_SINGLE_NSS;
    oal_uint16           us_vht_mcs_map;
    oal_uint16           us_msg_idx = 0;
    oal_uint16           us_offset;

    /* 获取帧体起始指针 */
    puc_frame_body = MAC_GET_RX_PAYLOAD_ADDR(0, pst_netbuf);

    /* 设置Beacon帧的field偏移量 */
    us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /* 查ht ie */
    puc_ie = mac_find_ie(MAC_EID_HT_CAP, puc_frame_body + us_offset, us_frame_len - us_offset);
    if (OAL_PTR_NULL != puc_ie)
    {
        us_msg_idx = MAC_IE_HDR_LEN + MAC_HT_CAPINFO_LEN + MAC_HT_AMPDU_PARAMS_LEN;
        puc_ht_mcs_bitmask = &puc_ie[us_msg_idx];
        for (en_nss = WLAN_SINGLE_NSS; en_nss < WLAN_NSS_LIMIT; en_nss++)
        {
            if (0 == puc_ht_mcs_bitmask[en_nss])
            {
                break;
            }
        }
        if(WLAN_SINGLE_NSS == en_nss)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "dmac_m2s_get_bss_max_nss::ht cap's mcs error!");
        }
        else
        {
            en_nss--;
        }

        /* 如果是考虑up状态，并且是ht协议时，按照此值返回 */
        if(OAL_TRUE == en_assoc_status &&
            (WLAN_HT_MODE == pst_mac_vap->en_protocol || WLAN_HT_ONLY_MODE == pst_mac_vap->en_protocol ||
             WLAN_HT_11G_MODE == pst_mac_vap->en_protocol))
        {
            return en_nss;
        }
    }

    /* 查vht ie */
    puc_ie = mac_find_ie(MAC_EID_VHT_CAP, puc_frame_body + us_offset, us_frame_len - us_offset);
    if (OAL_PTR_NULL != puc_ie)
    {
        us_msg_idx = MAC_IE_HDR_LEN + MAC_VHT_CAP_INFO_FIELD_LEN;

        us_vht_mcs_map = OAL_MAKE_WORD16(puc_ie[us_msg_idx], puc_ie[us_msg_idx + 1]);

        for (en_nss = WLAN_SINGLE_NSS; en_nss < WLAN_NSS_LIMIT; en_nss++)
        {
            if (WLAN_INVALD_VHT_MCS == WLAN_GET_VHT_MAX_SUPPORT_MCS(us_vht_mcs_map & 0x3))
            {
                break;
            }
            us_vht_mcs_map >>= 2;
        }
        if(WLAN_SINGLE_NSS == en_nss)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "dmac_m2s_get_bss_max_nss::ht cap's mcs error!");
        }
        else
        {
            en_nss--;
        }

        /* 如果是考虑up状态，并且是vht协议时，按照此值返回 */
        if(OAL_TRUE == en_assoc_status &&
            (WLAN_VHT_MODE == pst_mac_vap->en_protocol || WLAN_VHT_ONLY_MODE == pst_mac_vap->en_protocol
#ifdef _PRE_WLAN_FEATURE_11AX
                 || WLAN_HE_MODE == pst_mac_vap->en_protocol
#endif
            ))
        {
            return en_nss;
        }
    }

    return en_nss;
}


oal_uint8 dmac_m2s_scan_get_num_sounding_dim(oal_netbuf_stru *pst_netbuf, oal_uint16 us_frame_len)
{
    oal_uint8      *puc_vht_cap_ie = OAL_PTR_NULL;
    oal_uint8      *puc_frame_body = OAL_PTR_NULL;
    oal_uint16      us_offset;
    oal_uint16      us_vht_cap_filed_low;
    oal_uint16      us_vht_cap_filed_high;
    oal_uint32      ul_vht_cap_field;
    oal_uint16      us_msg_idx;
    oal_uint8       uc_num_sounding_dim = 0;

    /* 获取帧体起始指针 */
    puc_frame_body = MAC_GET_RX_PAYLOAD_ADDR(0, pst_netbuf);

    /* 设置Beacon帧的field偏移量 */
    us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    puc_vht_cap_ie = mac_find_ie(MAC_EID_VHT_CAP, puc_frame_body + us_offset, us_frame_len - us_offset);
    if(OAL_PTR_NULL == puc_vht_cap_ie)
    {
        return uc_num_sounding_dim;
    }

    us_msg_idx = MAC_IE_HDR_LEN;

    /* 解析VHT capablities info field */
    us_vht_cap_filed_low    = OAL_MAKE_WORD16(puc_vht_cap_ie[us_msg_idx], puc_vht_cap_ie[us_msg_idx + 1]);
    us_vht_cap_filed_high   = OAL_MAKE_WORD16(puc_vht_cap_ie[us_msg_idx + 2], puc_vht_cap_ie[us_msg_idx + 3]);
    ul_vht_cap_field        = OAL_MAKE_WORD32(us_vht_cap_filed_low, us_vht_cap_filed_high);
    /* 解析num_sounding_dim */
    uc_num_sounding_dim = ((ul_vht_cap_field & (BIT18 | BIT17 | BIT16)) >> 16);
    return uc_num_sounding_dim;
}


oal_bool_enum_uint8 dmac_m2s_get_bss_support_opmode(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_frame_body, oal_uint16 us_frame_len)
{
    oal_uint8                  *puc_ie;
    oal_uint8                   us_offset;

    us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    puc_ie = mac_find_ie(MAC_EID_EXT_CAPS, puc_frame_body + us_offset, us_frame_len - us_offset);
    if(OAL_PTR_NULL == puc_ie || MAC_HT_EXT_CAP_OPMODE_LEN > puc_ie[1])
    {
        return OAL_FALSE;
    }
    return ((mac_ext_cap_ie_stru *)(puc_ie + MAC_IE_HDR_LEN))->bit_operating_mode_notification;
}


oal_void dmac_m2s_show_blacklist_in_list(hal_to_dmac_device_stru *pst_hal_device)
{
    hal_device_m2s_mgr_stru            *pst_device_m2s_mgr;
    hal_m2s_mgr_blacklist_stru         *pst_device_m2s_mgr_blacklist;
    oal_uint8                           uc_index;

    pst_device_m2s_mgr = GET_HAL_DEVICE_M2S_MGR(pst_hal_device);

    for (uc_index = 0; uc_index < pst_device_m2s_mgr->uc_blacklist_bss_cnt; uc_index++)
    {
        pst_device_m2s_mgr_blacklist = &(pst_device_m2s_mgr->ast_m2s_blacklist[uc_index]);

        OAM_WARNING_LOG_ALTER(pst_hal_device->uc_device_id, OAM_SF_M2S,
            "{dmac_m2s_show_blacklist_in_list::Find in blacklist, ap index[%d],action type[%d],addr->%02x:XX:XX:%02x:%02x:%02x.}",
            6, uc_index, pst_device_m2s_mgr_blacklist->en_action_type,
            pst_device_m2s_mgr_blacklist->auc_user_mac_addr[0], pst_device_m2s_mgr_blacklist->auc_user_mac_addr[3],
            pst_device_m2s_mgr_blacklist->auc_user_mac_addr[4], pst_device_m2s_mgr_blacklist->auc_user_mac_addr[5]);
    }
}


oal_uint32 dmac_m2s_check_blacklist_in_list(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_index)
{
    oal_uint8                           uc_index;
    hal_to_dmac_device_stru            *pst_hal_device;
    hal_device_m2s_mgr_stru            *pst_device_m2s_mgr;
    hal_m2s_mgr_blacklist_stru         *pst_device_m2s_mgr_blacklist;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_check_blacklist_in_list:: DMAC_VAP_GET_HAL_DEVICE null}");
        return OAL_FAIL;
    }

    pst_device_m2s_mgr = GET_HAL_DEVICE_M2S_MGR(pst_hal_device);

     /* 初始化为无效index */
    *puc_index = WLAN_BLACKLIST_MAX;

    for (uc_index = 0; uc_index < WLAN_BLACKLIST_MAX; uc_index++)
    {
        pst_device_m2s_mgr_blacklist = &(pst_device_m2s_mgr->ast_m2s_blacklist[uc_index]);

        if (0 == oal_compare_mac_addr(pst_device_m2s_mgr_blacklist->auc_user_mac_addr, pst_mac_vap->auc_bssid))
        {
            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                "{dmac_m2s_check_blacklist_in_list::Find in blacklist, addr->%02x:XX:XX:%02x:%02x:%02x.}",
                pst_mac_vap->auc_bssid[0], pst_mac_vap->auc_bssid[3], pst_mac_vap->auc_bssid[4], pst_mac_vap->auc_bssid[5]);

            *puc_index = uc_index;
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}


OAL_STATIC oal_void dmac_m2s_add_and_update_blacklist_to_list(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint8                          uc_index;
    hal_to_dmac_device_stru            *pst_hal_device;
    mac_vap_stru                       *pst_mac_vap;
    hal_device_m2s_mgr_stru            *pst_device_m2s_mgr;
    hal_m2s_mgr_blacklist_stru         *pst_device_m2s_mgr_blacklist;

    pst_mac_vap    = &pst_dmac_vap->st_vap_base_info;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_add_and_update_blacklist_to_list:: DMAC_VAP_GET_HAL_DEVICE null}");
        return;
    }

    pst_device_m2s_mgr = GET_HAL_DEVICE_M2S_MGR(pst_hal_device);

    if (pst_device_m2s_mgr->uc_blacklist_bss_index >= WLAN_BLACKLIST_MAX)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_add_and_update_blacklist_to_list::already reach max num:%d.}", WLAN_BLACKLIST_MAX);
        pst_device_m2s_mgr->uc_blacklist_bss_index = 0;
    }

    /* 看该ap是否在列表中，在的话刷新切换模式即可，不在的话，需要重新添加 */
    if(OAL_TRUE == dmac_m2s_check_blacklist_in_list(pst_mac_vap, &uc_index))
    {
        pst_device_m2s_mgr_blacklist = &(pst_device_m2s_mgr->ast_m2s_blacklist[uc_index]);

        /* 如果已经是策略3，就不刷新 */
        if(HAL_M2S_ACTION_TYPR_NONE == pst_device_m2s_mgr_blacklist->en_action_type)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_add_and_update_blacklist_to_list:: HAL_M2S_ACTION_TYPR_NONE already in MISO.}");
        }
        else
        {
#if 1
            /* smps和opmode都不支持，需要保持在miso状态，后续该设备是none的话，都只能切换到miso状态 */
            /* 第二次添加采用none模式 */
            pst_device_m2s_mgr_blacklist->en_action_type = HAL_M2S_ACTION_TYPR_NONE;

            /* 当前关联ap是none用户，需要把hal device刷新成切换保持miso状态，不探测,需要考虑多sta+gc共存sta去关联时，需要遍历是否需要清标记 */
            GET_HAL_DEVICE_M2S_DEL_SWI_MISO_HOLD(pst_hal_device) = OAL_TRUE;

            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                "{dmac_m2s_add_and_update_blacklist_to_list::ap use SWITCH_MODE_NONE, keep at miso, addr->%02x:XX:XX:%02x:%02x:%02x.}",
                pst_mac_vap->auc_bssid[0], pst_mac_vap->auc_bssid[3], pst_mac_vap->auc_bssid[4], pst_mac_vap->auc_bssid[5]);
#else

            /* 满足切换方案要求，直接miso切换到mimo，然后直接到siso即可 */
            (WLAN_PHY_CHAIN_ZERO == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain)?
                (en_event_type = HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0): (en_event_type = HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1);

            /* 直接从miso切换mimo到siso，即可，说明该设备对opmode和smps支持都不好，直接不发action帧，靠对端降低到siso */
            dmac_m2s_handle_event(pst_hal_device, HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO, 0, OAL_PTR_NULL);

            /* 第二次添加采用none模式 */
            pst_device_m2s_mgr_blacklist->en_m2s_switch_mode = HAL_M2S_SWITCH_MODE_NONE;

            OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                "{dmac_m2s_add_and_update_blacklist_to_list::ap use SWITCH_MODE_NONE, MIMO to SISO, addr->%02x:XX:XX:%02x:%02x:%02x.}",
                pst_mac_vap->auc_bssid[0], pst_mac_vap->auc_bssid[3], pst_mac_vap->auc_bssid[4], pst_mac_vap->auc_bssid[5]);

            dmac_m2s_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), en_event_type, 0, OAL_PTR_NULL);
#endif
        }
    }
    else
    {
        pst_device_m2s_mgr_blacklist = &(pst_device_m2s_mgr->ast_m2s_blacklist[pst_device_m2s_mgr->uc_blacklist_bss_index]);
        oal_set_mac_addr(pst_device_m2s_mgr_blacklist->auc_user_mac_addr, pst_mac_vap->auc_bssid);

        /* 第一次添加是采用OTHER方式,为了兼容小米3路由器，首次是smps话，此时直接到none,
           首次是opmode的话,才继续发smps 在none的话，就保持在miso状态了 配置命令需要把miso和siso对等来看，防止不执行 */
       (HAL_M2S_ACTION_TYPR_SMPS == DMAC_VAP_GET_VAP_M2S_ACTION_ORI_TYPE(pst_mac_vap))?
        (pst_device_m2s_mgr_blacklist->en_action_type = HAL_M2S_ACTION_TYPR_OPMODE) :(pst_device_m2s_mgr_blacklist->en_action_type = HAL_M2S_ACTION_TYPR_SMPS);

        pst_device_m2s_mgr->uc_blacklist_bss_index++;

        /* 最多记录WLAN_BLACKLIST_MAX个用户，新增直接覆盖第一个，总数保持不变 */
        if(WLAN_BLACKLIST_MAX > pst_device_m2s_mgr->uc_blacklist_bss_cnt)
        {
            pst_device_m2s_mgr->uc_blacklist_bss_cnt++;
        }

        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
            "{dmac_m2s_add_and_update_blacklist_to_list::new ap write in blacklist, addr->%02x:XX:XX:%02x:%02x:%02x.}",
            pst_mac_vap->auc_bssid[0], pst_mac_vap->auc_bssid[3], pst_mac_vap->auc_bssid[4], pst_mac_vap->auc_bssid[5]);

        /* 换一种action帧发送,看是否对端能退化到siso数据方式 */
        dmac_m2s_send_action_frame(pst_mac_vap);
        dmac_m2s_switch_protect_trigger(pst_dmac_vap);
    }
}


oal_bool_enum_uint8 dmac_m2s_assoc_vap_find_in_device_blacklist(hal_to_dmac_device_stru *pst_hal_device)
{
    mac_vap_stru                       *pst_mac_vap    = OAL_PTR_NULL;
    oal_uint8                           uc_vap_idx     = 0;
    oal_uint8                           uc_index       = 0;
    oal_uint8                           uc_up_vap_num;
    oal_uint8                           auc_mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    hal_m2s_mgr_blacklist_stru         *pst_device_m2s_mgr_blacklist;

    /* 只看自己的hal device上找 */
    uc_up_vap_num = hal_device_find_all_up_vap(pst_hal_device, auc_mac_vap_id);
    for (uc_vap_idx = 0; uc_vap_idx < uc_up_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = mac_res_get_mac_vap(auc_mac_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG0(auc_mac_vap_id[uc_vap_idx], OAM_SF_M2S, "{dmac_m2s_command_switch_event_type_query::pst_mac_vap null.}");
            continue;
        }

        if(OAL_TRUE == dmac_m2s_check_blacklist_in_list(pst_mac_vap, &uc_index))
        {
            /* 找到了一个黑名单关联ap */
            pst_device_m2s_mgr_blacklist = &(GET_HAL_DEVICE_M2S_MGR(pst_mac_vap)->ast_m2s_blacklist[uc_index]);

            /* 如果需要切换成none模式，后续保持miso状态 */
            if(HAL_M2S_ACTION_TYPR_NONE == pst_device_m2s_mgr_blacklist->en_action_type)
            {
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}


oal_void dmac_m2s_action_frame_type_query(mac_vap_stru *pst_mac_vap, hal_m2s_action_type_uint8 *pen_action_type)
{
    oal_uint8                           uc_index;
    hal_to_dmac_device_stru            *pst_hal_device;
    hal_device_m2s_mgr_stru            *pst_device_m2s_mgr;
    hal_m2s_mgr_blacklist_stru         *pst_device_m2s_mgr_blacklist;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_action_frame_type_query:: DMAC_VAP_GET_HAL_DEVICE null.}");
        return;
    }

    pst_device_m2s_mgr = GET_HAL_DEVICE_M2S_MGR(pst_hal_device);

    if(OAL_TRUE == dmac_m2s_check_blacklist_in_list(pst_mac_vap, &uc_index))
    {
        pst_device_m2s_mgr_blacklist = &(pst_device_m2s_mgr->ast_m2s_blacklist[uc_index]);

        /* 关联ap在黑名单之中，需要按照新的action帧发送类型来发送 */
       *pen_action_type = pst_device_m2s_mgr_blacklist->en_action_type;
    }
}


oal_void dmac_m2s_send_action_complete_check(mac_vap_stru *pst_mac_vap, mac_tx_ctl_stru *pst_tx_ctl)
{
    /* 发送完成中断succ时候，判断cb帧类型符合smps或者opmode，m2s切换逻辑继续执行下去 */
    if (MAC_GET_CB_IS_OPMODE_FRAME(pst_tx_ctl) || MAC_GET_CB_IS_SMPS_FRAME(pst_tx_ctl))
    {
        DMAC_VAP_GET_VAP_M2S_ACTION_SEND_STATE(pst_mac_vap) = OAL_TRUE;
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_send_action_complete_check:: action send succ.}");
    }
}


oal_void  dmac_m2s_send_action_frame(mac_vap_stru *pst_mac_vap)
{
    hal_m2s_action_type_uint8      en_action_type  = HAL_M2S_ACTION_TYPR_BUTT;

    /* 暂时只有STA模式才支持发送action帧 */
    if ((MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state || MAC_VAP_STATE_PAUSE == pst_mac_vap->en_vap_state))
    {
        /* 1.初始切换方案 */
        /* 存在对端beacon携带vht，最终关联在ht协议下，仍然需要发送smps帧提高兼容性 */
        if(OAL_TRUE == mac_mib_get_OperatingModeNotificationImplemented(pst_mac_vap) &&
            (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_opmode) &&
            (WLAN_VHT_MODE == pst_mac_vap->en_protocol || WLAN_VHT_ONLY_MODE == pst_mac_vap->en_protocol
#ifdef _PRE_WLAN_FEATURE_11AX
            || WLAN_HE_MODE == pst_mac_vap->en_protocol
#endif
        ))
        {
            en_action_type = HAL_M2S_ACTION_TYPR_OPMODE;
        }
        else
        {
            en_action_type = HAL_M2S_ACTION_TYPR_SMPS;
        }

        /* 2.刷新sta初始切换action配置模式，用于后续兼容性路由器的特殊处理 */
        DMAC_VAP_GET_VAP_M2S_ACTION_ORI_TYPE(pst_mac_vap) = en_action_type;

        /* 3.根据黑名单进一步更新帧发送类型 */
        dmac_m2s_action_frame_type_query(pst_mac_vap, &en_action_type);

#if 0
        /* 4.切换保护处于临时从miso切回mimo时，不需要发action帧 */
        if(OAL_TRUE == GET_HAL_DEVICE_M2S_SWITCH_PROT(DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap)) &&
            WLAN_DOUBLE_NSS == DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap)->st_cfg_cap_info.en_nss_num)
        {
            en_action_type = HAL_M2S_ACTION_TYPR_NONE;
        }
#endif

        /* 5.开启action帧发送开关 */
        DMAC_VAP_GET_VAP_M2S_ACTION_SEND_STATE(pst_mac_vap) = OAL_FALSE;

        /* 6.发送action帧 */
        if(HAL_M2S_ACTION_TYPR_OPMODE == en_action_type)
        {
        #ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
            dmac_mgmt_send_opmode_notify_action(pst_mac_vap, pst_mac_vap->en_vap_rx_nss, OAL_TRUE);
        #endif
        }
        else if(HAL_M2S_ACTION_TYPR_SMPS == en_action_type)
        {
        #ifdef _PRE_WLAN_FEATURE_SMPS
            dmac_smps_send_action(pst_mac_vap, MAC_M2S_CALI_SMPS_MODE(pst_mac_vap->en_vap_rx_nss), OAL_TRUE);
        #endif
        }
        else
        {
#if 0
            //最终方案是none，如果初始方案是opmode的话，就还是发smps，  小米3是初始发smps，none时不发帧性能最优
            if(HAL_M2S_ACTION_TYPR_OPMODE == DMAC_VAP_GET_VAP_M2S_ORI_MODE(pst_mac_vap))
            {
            #ifdef _PRE_WLAN_FEATURE_SMPS
                dmac_smps_send_action(pst_mac_vap, MAC_M2S_CALI_SMPS_MODE(pst_mac_vap->en_vap_rx_nss), OAL_TRUE);
            #endif
            }
#endif
        }
    }
}


oal_uint32 dmac_m2s_d2h_device_info_syn(mac_device_stru *pst_mac_device)
{
    oal_uint32                  ul_ret;
    mac_device_m2s_stru         st_m2s_device;
    mac_vap_stru               *pst_mac_vap;

    /* 注意仅为了获取配置VAP */
    pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->uc_cfg_vap_id);
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG2(0, OAM_SF_M2S, "{dmac_m2s_d2h_device_info_syn::Cannot find cfg_vap by vapID[%d],when devID[%d].}",
            pst_mac_device->uc_cfg_vap_id, pst_mac_device->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* m2s device信息同步hmac,目前只有硬切换时候的nss能力需要同步(可能被改变)st_device_cap其他能力 TBD */
    /* 软硬切换都同步上去，只是软切换部分能力维持不变 */
    st_m2s_device.uc_device_id   = pst_mac_device->uc_device_id;
    st_m2s_device.en_nss_num     = MAC_DEVICE_GET_NSS_NUM(pst_mac_device);
    st_m2s_device.en_smps_mode   = MAC_DEVICE_GET_MODE_SMPS(pst_mac_device);

    /***************************************************************************
        抛事件到HMAC层, 同步USER m2s能力到HMAC
    ***************************************************************************/
    ul_ret = dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_DEVICE_M2S_INFO_SYN, OAL_SIZEOF(st_m2s_device), (oal_uint8 *)(&st_m2s_device));
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                         "{dmac_m2s_d2h_device_info_syn::dmac_send_sys_event failed[%d].}", ul_ret);
    }

    return OAL_SUCC;
}


oal_uint32 dmac_m2s_d2h_vap_info_syn(mac_vap_stru *pst_mac_vap)
{
    wlan_mib_mimo_power_save_enum_uint8 en_smps_mode;
    oal_uint32                          ul_ret;
    mac_vap_m2s_stru                    st_m2s_vap;

    /* m2s vap信息同步hmac */
    st_m2s_vap.uc_vap_id   = pst_mac_vap->uc_vap_id;

    /* 软切换之后，重新关联上，vap的nss能力要根据hal device的nss能力重新刷新 */
    mac_vap_set_rx_nss(pst_mac_vap, DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap)->st_cfg_cap_info.en_nss_num);

    st_m2s_vap.en_vap_rx_nss  = pst_mac_vap->en_vap_rx_nss;
#ifdef _PRE_WLAN_FEATURE_SMPS
    /* GO等重新vap init之后，切换到siso时候还是赋值的mimo的smps，此时需要根据vap nss取小 */
    en_smps_mode = mac_mib_get_smps(pst_mac_vap);
    en_smps_mode = OAL_MIN(en_smps_mode, MAC_M2S_CALI_SMPS_MODE(pst_mac_vap->en_vap_rx_nss));

    mac_mib_set_smps(pst_mac_vap, en_smps_mode);

    st_m2s_vap.en_sm_power_save = en_smps_mode;
#endif
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
    st_m2s_vap.en_transmit_stagger_sounding = mac_mib_get_TransmitStaggerSoundingOptionImplemented(pst_mac_vap);
#endif
    st_m2s_vap.en_tx_stbc = mac_mib_get_TxSTBCOptionImplemented(pst_mac_vap);
    st_m2s_vap.en_vht_ctrl_field_supported = mac_mib_get_vht_ctrl_field_cap(pst_mac_vap); /* 代码目前都是写FALSE,后续再看优化整改需求 */
#ifdef _PRE_WLAN_FEATURE_TXBF
    st_m2s_vap.en_tx_vht_stbc_optionimplemented = mac_mib_get_VHTTxSTBCOptionImplemented(pst_mac_vap);
    st_m2s_vap.en_vht_number_sounding_dimensions = mac_mib_get_VHTNumberSoundingDimensions(pst_mac_vap);
    st_m2s_vap.en_vht_su_beamformer_optionimplemented = mac_mib_get_VHTSUBeamformerOptionImplemented(pst_mac_vap);
#endif

    /* 软硬件切换类型同步到host侧 */
    st_m2s_vap.en_m2s_type = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap)->st_hal_m2s_fsm.en_m2s_type;

    /* 关键信息同步提示 */
    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                       "{dmac_m2s_d2h_vap_info_syn::en_vap_rx_nss:%d,en_sm_power_save:%d, en_m2s_type[%d].}",
                         st_m2s_vap.en_vap_rx_nss, st_m2s_vap.en_sm_power_save, st_m2s_vap.en_m2s_type);

    /***************************************************************************
        抛事件到HMAC层, 同步USER m2s能力到HMAC
    ***************************************************************************/
    ul_ret = dmac_send_sys_event(pst_mac_vap, WLAN_CFGID_VAP_M2S_INFO_SYN, OAL_SIZEOF(st_m2s_vap), (oal_uint8 *)(&st_m2s_vap));
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                         "{dmac_m2s_d2h_vap_info_syn::dmac_send_sys_event failed[%d].}", ul_ret);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_m2s_rx_ucast_nss_count_handle(mac_vap_stru *pst_mac_vap)
{
    hal_to_dmac_device_stru            *pst_hal_device;
    dmac_vap_m2s_rx_statistics_stru    *pst_dmac_vap_m2s_rx_statistics;
    dmac_vap_stru                      *pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);
    hal_m2s_event_tpye_uint16           en_event_type;

    pst_dmac_vap_m2s_rx_statistics = DMAC_VAP_GET_VAP_M2S_RX_STATISTICS(pst_mac_vap);

    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                       "{dmac_m2s_rx_ucast_nss_count_handle::pst_dmac_vap_m2s_rx_statistics->us_rx_nss_mimo_count:%d,pst_dmac_vap_m2s_rx_statistics->us_rx_nss_siso_counte:%d.}",
                         pst_dmac_vap_m2s_rx_statistics->us_rx_nss_mimo_count, pst_dmac_vap_m2s_rx_statistics->us_rx_nss_siso_count);

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
       OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_rx_ucast_nss_count_handle: pst_hal_device is null ptr}");
       return OAL_ERR_CODE_PTR_NULL;
    }

    /*
       在切换后2s之后，3s统计时间内，需要对端不发mimo包(至少有siso包)
       1.该功能需要上层配合，连续发送10个ping包， 测试时候需要跑流或者ping包
       2. 没有mimo包，有很多siso包正常，目前门限都是5个
       3.针对环境可能比较差，对端只有siso包这个场景，暂时不考虑，理论上也不太可能
    */
    if(pst_dmac_vap_m2s_rx_statistics->us_rx_nss_mimo_count < M2S_MIMO_RX_CNT_THRESHOLD)
    {
        /* 满足切换方案要求，直接miso切换到siso即可 */
        (WLAN_PHY_CHAIN_ZERO == pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain)?
            (en_event_type = HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0): (en_event_type = HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1);

        dmac_m2s_handle_event(DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap), en_event_type, 0, OAL_PTR_NULL);

        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_rx_ucast_nss_count_handle::delay switch to siso SUCC.}");
    }
    else
    {
        /* 对端还是发mimo数据为主，说明对端对此action帧处理不好，需要换切换方案,加入黑名单 */
        dmac_m2s_add_and_update_blacklist_to_list(pst_dmac_vap);
    }

    return OAL_SUCC;
}


oal_void dmac_m2s_rx_rate_nss_process (mac_vap_stru *pst_vap,
    dmac_rx_ctl_stru *pst_cb_ctrl, mac_ieee80211_frame_stru *pst_frame_hdr)
{
    dmac_vap_m2s_rx_statistics_stru    *pst_dmac_vap_m2s_rx_statistics;
    wlan_phy_protocol_enum_uint8        en_phy_protocol;

    pst_dmac_vap_m2s_rx_statistics = DMAC_VAP_GET_VAP_M2S_RX_STATISTICS(pst_vap);

    /* 1.统计开启时做统计，有数据收发之后才开始统计 */
    if(OAL_FALSE == pst_dmac_vap_m2s_rx_statistics->en_rx_nss_statistics_start_flag)
    {
        return;
    }

    /* 2.vip帧或者广播帧不统计，可能属于siso帧 */
    if(OAL_TRUE == pst_cb_ctrl->st_rx_info.bit_is_key_frame || ETHER_IS_MULTICAST(pst_frame_hdr->auc_address1))
    {
        return;
    }

    /* 3.获取数据帧的nss类型，主要是判断mimo帧情况，以及siso是不是通
         (1)不需要启动定时器，因为可能本身处于低功耗，没有数据收发，那就在miso下保持足够久
         (2)有数据收发了，统计连续10个数据帧，判断这10个包的mimo和siso情况 */
    pst_dmac_vap_m2s_rx_statistics->us_rx_nss_ucast_count++;

    en_phy_protocol = pst_cb_ctrl->st_rx_statistic.un_nss_rate.st_legacy_rate.bit_protocol_mode;
    if(WLAN_HT_PHY_PROTOCOL_MODE == en_phy_protocol)
    {
        if(pst_cb_ctrl->st_rx_statistic.un_nss_rate.st_ht_rate.bit_ht_mcs >= WLAN_HT_MCS8 &&
            pst_cb_ctrl->st_rx_statistic.un_nss_rate.st_ht_rate.bit_ht_mcs != WLAN_HT_MCS32)
        {
            pst_dmac_vap_m2s_rx_statistics->us_rx_nss_mimo_count++;
        }
        else
        {
            pst_dmac_vap_m2s_rx_statistics->us_rx_nss_siso_count++;
        }
    }
    else if(WLAN_VHT_PHY_PROTOCOL_MODE == en_phy_protocol)
    {
        if(pst_cb_ctrl->st_rx_statistic.un_nss_rate.st_vht_nss_mcs.bit_nss_mode == 1)
        {
            pst_dmac_vap_m2s_rx_statistics->us_rx_nss_mimo_count++;
        }
        else
        {
            pst_dmac_vap_m2s_rx_statistics->us_rx_nss_siso_count++;
        }
    }
    else
    {
        pst_dmac_vap_m2s_rx_statistics->us_rx_nss_siso_count++;
    }

    /* 4. 达到数据统计门限，开始计算 */
    if(M2S_RX_UCAST_CNT_THRESHOLD == pst_dmac_vap_m2s_rx_statistics->us_rx_nss_ucast_count)
    {
        pst_dmac_vap_m2s_rx_statistics->en_rx_nss_statistics_start_flag = OAL_FALSE;
        dmac_m2s_rx_ucast_nss_count_handle(pst_vap);
    }
}


oal_uint32 dmac_m2s_delay_switch_nss_statistics_callback(oal_void *p_arg)
{
    hal_to_dmac_device_stru            *pst_hal_device;
    dmac_vap_m2s_rx_statistics_stru    *pst_dmac_vap_m2s_rx_statistics;
    mac_vap_stru                       *pst_mac_vap;

    pst_mac_vap = (mac_vap_stru *)p_arg;
    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);

    pst_dmac_vap_m2s_rx_statistics = DMAC_VAP_GET_VAP_M2S_RX_STATISTICS(pst_mac_vap);

    /* 1.业务结束 */
    if (OAL_FALSE == GET_HAL_DEVICE_M2S_SWITCH_PROT(pst_hal_device))
    {
        return OAL_SUCC;
    }

    /* 2.action帧发送成功，才继续执行下去，否则继续发action帧，并再次打开统计 */
    if(OAL_FALSE == DMAC_VAP_GET_VAP_M2S_ACTION_SEND_STATE(pst_mac_vap) &&
        M2S_ACTION_SENT_CNT_THRESHOLD > DMAC_VAP_GET_VAP_M2S_ACTION_SEND_CNT(pst_mac_vap))
    {
        dmac_m2s_send_action_frame(pst_mac_vap);

        /* 初始是0，这里进行累加 */
        DMAC_VAP_GET_VAP_M2S_ACTION_SEND_CNT(pst_mac_vap)++;

        FRW_TIMER_CREATE_TIMER(&(pst_dmac_vap_m2s_rx_statistics->m2s_delay_switch_statistics_timer),
                           dmac_m2s_delay_switch_nss_statistics_callback,
                           M2S_RX_STATISTICS_START_TIME,
                           (oal_void *)pst_mac_vap,
                           OAL_FALSE,
                           OAM_MODULE_ID_DMAC,
                           pst_mac_vap->ul_core_id);
    }
    else
    {
        /* 3. action帧发送成功，清空统计值，开始统计nss */
        pst_dmac_vap_m2s_rx_statistics->us_rx_nss_mimo_count  = 0;
        pst_dmac_vap_m2s_rx_statistics->us_rx_nss_siso_count  = 0;
        pst_dmac_vap_m2s_rx_statistics->us_rx_nss_ucast_count = 0;
        pst_dmac_vap_m2s_rx_statistics->en_rx_nss_statistics_start_flag = OAL_TRUE;
        DMAC_VAP_GET_VAP_M2S_ACTION_SEND_CNT(pst_mac_vap)     = 0;
    }

    return OAL_SUCC;
}


oal_void dmac_m2s_switch_protect_trigger(dmac_vap_stru *pst_dmac_vap)
{
    dmac_vap_m2s_rx_statistics_stru *pst_dmac_vap_m2s_rx_statistics;

    if(!IS_STA(&pst_dmac_vap->st_vap_base_info))
    {
        return;
    }

    pst_dmac_vap_m2s_rx_statistics = DMAC_VAP_GET_VAP_M2S_RX_STATISTICS(pst_dmac_vap);

    /* 1.nss统计暂时功能打开,等超时定时到期才开始统计 */
    pst_dmac_vap_m2s_rx_statistics->en_rx_nss_statistics_start_flag = OAL_FALSE;

    /* 2.创建统计定时器,1s之后开始统计双流数据包个数,会有部分设备，硬件队列还有部分mimo包要先发完才切换到siso发送 */
    if(OAL_TRUE == pst_dmac_vap_m2s_rx_statistics->m2s_delay_switch_statistics_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_dmac_vap_m2s_rx_statistics->m2s_delay_switch_statistics_timer));
    }

    FRW_TIMER_CREATE_TIMER(&(pst_dmac_vap_m2s_rx_statistics->m2s_delay_switch_statistics_timer),
                           dmac_m2s_delay_switch_nss_statistics_callback,
                           M2S_RX_STATISTICS_START_TIME,
                           (oal_void *)(&pst_dmac_vap->st_vap_base_info),
                           OAL_FALSE,
                           OAM_MODULE_ID_DMAC,
                           pst_dmac_vap->st_vap_base_info.ul_core_id);
}


oal_uint32 dmac_m2s_disassoc_state_syn(mac_vap_stru *pst_mac_vap)
{
    dmac_vap_m2s_stru                  *pst_dmac_vap_m2s;
    dmac_vap_m2s_rx_statistics_stru    *pst_dmac_vap_m2s_rx_statistics;
    hal_to_dmac_device_stru            *pst_hal_device;

    if(!IS_STA(pst_mac_vap))
    {
        return OAL_SUCC;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(0, OAM_SF_M2S, "{dmac_m2s_disassoc_state_syn: pst_hal_device is null ptr.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap_m2s = DMAC_VAP_GET_VAP_M2S(pst_mac_vap);

    pst_dmac_vap_m2s_rx_statistics = &(pst_dmac_vap_m2s->st_dmac_vap_m2s_rx_statistics);
    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_dmac_vap_m2s_rx_statistics->m2s_delay_switch_statistics_timer));

    /* 1.统计信息统一清零 */
    OAL_MEMZERO(pst_dmac_vap_m2s, OAL_SIZEOF(dmac_vap_m2s_stru));

    /* 2.刷新hal device下是否保持miso标记 */
    if(OAL_FALSE == dmac_m2s_assoc_vap_find_in_device_blacklist(pst_hal_device))
    {
        GET_HAL_DEVICE_M2S_DEL_SWI_MISO_HOLD(pst_hal_device) = OAL_FALSE;
    }

    return OAL_SUCC;
}


oal_void  dmac_m2s_update_user_capbility(mac_user_stru *pst_mac_user, mac_vap_stru *pst_mac_vap)
{
    wlan_nss_enum_uint8        en_old_avail_num_spatial_stream;

    /* pause tid */
    //dmac_user_pause(pst_dmac_user);
    //主动切换，只改变vap能力，不修改user本身能力
    //mac_user_set_sm_power_save(pst_user, MAC_M2S_CALI_SMPS_MODE(en_nss));

    /* 自身是大于HT支持切换，但是user关联状态为非ht和vht，或者对端不支持mimo，也不能支持切换，并且提示异常 */
    if ((WLAN_SINGLE_NSS == pst_mac_user->en_user_num_spatial_stream)||
        (WLAN_HT_MODE > pst_mac_user->en_cur_protocol_mode))
    {
        OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
            "{dmac_m2s_update_vap_capbility::user isnot support HT/VHT user id[%d],vap protocol[%d],user_nss[%d],user protocol[%d]!}",
             pst_mac_user->us_assoc_id, pst_mac_vap->en_vap_mode,
             pst_mac_user->en_user_num_spatial_stream, pst_mac_user->en_protocol_mode);

        return;
    }

    /* 用户空间流 */
    en_old_avail_num_spatial_stream           = pst_mac_user->en_avail_num_spatial_stream;

    /* 更新之后的user空间流能力 */
    pst_mac_user->en_avail_num_spatial_stream = OAL_MIN(pst_mac_vap->en_vap_rx_nss, pst_mac_user->en_user_num_spatial_stream);

    /* 用户空间流能力没有发生变化也需要同步，可能此时hal phy chain能力变化，特定算法模块需要对应处理 */
#ifdef _PRE_WLAN_FEATURE_BTCOEX
    /* user空间流变化通知btcoex刷新速率门限 */
    dmac_btcoex_user_spatial_stream_change_notify(pst_mac_vap, pst_mac_user);
#endif
    /* 调用算法钩子函数,目前只是通知autorate，后续增加TXBF TBD */
    dmac_alg_cfg_user_spatial_stream_notify(pst_mac_user);

    /* 用户空间流能力发生变化，需要通知算法 */
    if(en_old_avail_num_spatial_stream != pst_mac_user->en_avail_num_spatial_stream)
    {
        /* user能力同步到hmac */
        if (OAL_SUCC != dmac_config_d2h_user_m2s_info_syn(pst_mac_vap, pst_mac_user))
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                      "{dmac_m2s_update_user_capbility::dmac_config_d2h_user_m2s_info_syn failed.}");
        }
    }
}


oal_void  dmac_m2s_update_vap_capbility(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap)
{
    hal_to_dmac_device_stru *pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    oal_dlist_head_stru     *pst_entry      = OAL_PTR_NULL;
    dmac_vap_stru           *pst_dmac_vap   = OAL_PTR_NULL;
    mac_user_stru           *pst_mac_user   = OAL_PTR_NULL;
    oal_bool_enum            en_send_action = OAL_TRUE;

    /* vap提前删除，指针已经置为空 */
    if(OAL_PTR_NULL == pst_hal_device)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_update_vap_capbility::the hal devcie is null!}");
        return;
    }
    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

    /* mimo/siso切换时，更新已关联sta唤醒收bcn的通道 */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode
      && (MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state || MAC_VAP_STATE_PAUSE == pst_mac_vap->en_vap_state))
    {
#ifdef _PRE_WLAN_FEATURE_SINGLE_RF_RX_BCN
        dmac_psm_update_bcn_rf_chain(pst_dmac_vap, oal_get_real_rssi(pst_dmac_vap->st_query_stats.s_signal));
        OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{dmac_m2s_update_vap_capbility:bcn_rx_chain[%d],rssi[%d]}",
                 pst_dmac_vap->pst_hal_vap->st_pm_info.uc_bcn_rf_chain, oal_get_real_rssi(pst_dmac_vap->st_query_stats.s_signal));
#else
        hal_pm_set_bcn_rf_chain(pst_dmac_vap->pst_hal_vap, pst_hal_device->st_cfg_cap_info.uc_rf_chain);
#endif
    }

    //如果切换到通道2的siso，此时也需要初始化发送参数通道，需要继续走到init_rate 继续走下去; siso支持继续走，本身不支持mimo，不允许切到mimo
    if ((pst_mac_vap->en_protocol < WLAN_HT_MODE)&&(WLAN_DOUBLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_update_vap_capbility::the vap is not support HT/VHT!}");
        return;
    }

    //启动时候需要更新同步到host侧，这里会相等，不做约束，保证其他业务正常
    /* 1. 改变vap下空间流个数能力,用于后续用户接入时与user能力做交集通知算法,工作模式通知ie字段填写 TBD 哪些mib要根据siso变化，找小川确认*/
    /* 空间流能力不变，可能是c0 siso切换到c1 siso，需要支持 */
    if(pst_hal_device->st_cfg_cap_info.en_nss_num == pst_mac_vap->en_vap_rx_nss)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
            "{dmac_m2s_update_vap_capbility::en_nss value[%d] is the same with pst_mac_vap.}", pst_mac_vap->en_vap_rx_nss);
        en_send_action = OAL_FALSE;
        //return;
    }

    mac_vap_set_rx_nss(pst_mac_vap, pst_hal_device->st_cfg_cap_info.en_nss_num);

    /* 2.修改HT能力 */
#ifdef _PRE_WLAN_FEATURE_SMPS
      /* 修改smps能力, 直接通知smps的来修改流程复杂，这里mimo/siso 相关mib能力统一管理 */
      mac_mib_set_smps(pst_mac_vap, MAC_M2S_CALI_SMPS_MODE(pst_hal_device->st_cfg_cap_info.en_nss_num));
#endif
      /* 修改tx STBC能力 */
      mac_mib_set_TxSTBCOptionImplemented(pst_mac_vap,
          pst_hal_device->st_cfg_cap_info.en_tx_stbc_is_supp & MAC_DEVICE_GET_CAP_TXSTBC(pst_mac_device));
#ifdef _PRE_WLAN_FEATURE_TXBF_HT
      mac_mib_set_TransmitStaggerSoundingOptionImplemented(pst_mac_vap,
          pst_hal_device->st_cfg_cap_info.en_su_bfmer_is_supp & MAC_DEVICE_GET_CAP_SUBFER(pst_mac_device));
#endif
      mac_mib_set_vht_ctrl_field_cap(pst_mac_vap, pst_hal_device->st_cfg_cap_info.en_nss_num); /* 代码目前都是写FALSE,后续再看优化整改需求 */
#ifdef _PRE_WLAN_FEATURE_TXBF
      /* 没有对应hal device的mib值，暂时按照nss来赋值， 后续看是否优化 */
      mac_mib_set_VHTNumberSoundingDimensions(pst_mac_vap,
          OAL_MIN(pst_hal_device->st_cfg_cap_info.en_nss_num, MAC_DEVICE_GET_NSS_NUM(pst_mac_device)));
      mac_mib_set_VHTSUBeamformerOptionImplemented(pst_mac_vap,
          pst_hal_device->st_cfg_cap_info.en_su_bfmer_is_supp & MAC_DEVICE_GET_CAP_SUBFER(pst_mac_device));
      mac_mib_set_VHTTxSTBCOptionImplemented(pst_mac_vap,
          pst_hal_device->st_cfg_cap_info.en_tx_stbc_is_supp & MAC_DEVICE_GET_CAP_TXSTBC(pst_mac_device));
#endif

    /* 硬切换 */
    if(WLAN_M2S_TYPE_HW == pst_hal_device->st_hal_m2s_fsm.en_m2s_type)
    {
        /* 4. 更新速率集，vap的空间流，暂时未看到其他需要更新的地方 */
        mac_vap_init_rates(pst_mac_vap);
    }

    /* 5. 重新初始化数据帧/管理帧发送参数 */
    /* 初始化单播数据帧的发送参数 */
    dmac_vap_init_tx_ucast_data_frame(MAC_GET_DMAC_VAP(pst_mac_vap));

    /* 初始化除单播数据帧以外所有帧的发送参数 */
    dmac_vap_init_tx_frame_params(MAC_GET_DMAC_VAP(pst_mac_vap), OAL_TRUE);

    /* 6.同步vap 的m2s能力到hmac */
    if (OAL_SUCC != dmac_m2s_d2h_vap_info_syn(pst_mac_vap))
    {
         OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                  "{dmac_m2s_update_vap_capbility::dmac_m2s_d2h_vap_info_syn failed.}");
    }

    if(WLAN_M2S_TYPE_SW == pst_hal_device->st_hal_m2s_fsm.en_m2s_type)
    {
        /* 7.刷新用户信息，并通知算法更改tx参数 */
        OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_mac_vap->st_mac_user_list_head))
        {
            pst_mac_user = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
            /*lint -save -e774 */
            if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_user))
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_update_vap_capbility::pst_mac_user null pointer.}");
                continue;
            }
            /*lint -restore */

            dmac_m2s_update_user_capbility(pst_mac_user, pst_mac_vap);

            /* DBDC更新vap以及user能力不需要发action帧后面虎哥优化 */
            if(WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode && OAL_TRUE == en_send_action)
            {
                /* 8.(1)处于siso状态，并且切换保护使能，启动切换保护触发
                     (2)如果已经是标记需要hold用户的device(该vap是blacklist的none)，也不需要开启切换保护 */
                if(OAL_TRUE == GET_HAL_DEVICE_M2S_SWITCH_PROT(pst_hal_device) &&
                    WLAN_SINGLE_NSS == pst_hal_device->st_cfg_cap_info.en_nss_num &&
                    OAL_FALSE == GET_HAL_DEVICE_M2S_DEL_SWI_MISO_HOLD(pst_hal_device))
                {
                    dmac_m2s_switch_protect_trigger(pst_dmac_vap);
                }

                /* 9. 发送action帧，通知对端修改m2s能力 */
                dmac_m2s_send_action_frame(pst_mac_vap);
            }
        }
    }
}


oal_void  dmac_m2s_switch_update_vap_capbility(hal_to_dmac_device_stru *pst_hal_device)
{
    mac_device_stru         *pst_mac_device = OAL_PTR_NULL;
    mac_vap_stru            *pst_mac_vap    = OAL_PTR_NULL;
    oal_uint8                uc_vap_idx     = 0;

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
       OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_update_vap_capbility: mac device is null ptr. device id:%d}", pst_hal_device->uc_mac_device_id);
       return;
    }

    /* 遍历mac device下所有业务vap，刷新所有属于本hal device的vap的能力 */
    /* 注意:硬切换会重新去关联，不能采用hal_device_find_all_up_vap来获取本hal device上的vap，不能覆盖扫描/常发常收模式 */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG0(pst_mac_device->auc_vap_id[uc_vap_idx], OAM_SF_M2S, "{dmac_m2s_switch_update_vap_capbility::the vap is null!}");
            continue;
        }

        /* 不属于本hal device的vap不进行mimo-siso操作(辅hal device本身不支持mimo) */
        if(pst_hal_device != MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_device)
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
               "{dmac_m2s_switch_update_vap_capbility::the vap not belong to this hal device!}");
            continue;
        }

        /* 更新vap能力 */
        dmac_m2s_update_vap_capbility(pst_mac_device, pst_mac_vap);
    }
}


oal_void  dmac_m2s_switch_update_hal_chain_capbility(hal_to_dmac_device_stru *pst_hal_device, oal_bool_enum_uint8 en_set_channel)
{
    /* 设置对应phy和ana dbb通道映射关系 */
    hal_set_phy_rf_chain_relation(pst_hal_device, en_set_channel);

    OAM_WARNING_LOG4(0, OAM_SF_M2S, "{dmac_m2s_switch_update_hal_chain_capbility::phy_chain[%d],uc_phy2hw_chain[%d],single_tx_chain[%d],rf_chain[%d].}",
         pst_hal_device->st_cfg_cap_info.uc_phy2dscr_chain, pst_hal_device->st_cfg_cap_info.uc_phy_chain,
         pst_hal_device->st_cfg_cap_info.uc_single_tx_chain, pst_hal_device->st_cfg_cap_info.uc_rf_chain);
}


oal_void  dmac_m2s_switch_update_device_capbility(hal_to_dmac_device_stru *pst_hal_device, mac_device_stru *pst_mac_device)
{
    /* 1.更新算法类能力 */
    switch(pst_hal_device->st_cfg_cap_info.en_nss_num)
    {
        case WLAN_SINGLE_NSS:
            pst_hal_device->st_cfg_cap_info.en_tx_stbc_is_supp  = OAL_FALSE;
            pst_hal_device->st_cfg_cap_info.en_su_bfmer_is_supp = OAL_FALSE;
            break;

        case WLAN_DOUBLE_NSS:
            pst_hal_device->st_cfg_cap_info.en_tx_stbc_is_supp  = OAL_TRUE;
            pst_hal_device->st_cfg_cap_info.en_su_bfmer_is_supp = OAL_TRUE;
            break;

        default:
            OAM_WARNING_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_update_device_capbility: en_nss[%d] error!}", pst_hal_device->st_cfg_cap_info.en_nss_num);
            return;
    }

    if(WLAN_M2S_TYPE_HW == pst_hal_device->st_hal_m2s_fsm.en_m2s_type)
    {
        /* 2.硬切换准备，更换mac device的nss能力 */
        MAC_DEVICE_GET_NSS_NUM(pst_mac_device) = pst_hal_device->st_cfg_cap_info.en_nss_num;
    }

    /* 3.同步mac device 的smps能力到hmac */
    MAC_DEVICE_GET_MODE_SMPS(pst_mac_device) = MAC_M2S_CALI_SMPS_MODE(pst_hal_device->st_cfg_cap_info.en_nss_num);

    /* 4.同步device 的m2s能力到hmac */
    if (OAL_SUCC != dmac_m2s_d2h_device_info_syn(pst_mac_device))
    {
        OAM_WARNING_LOG0(0, OAM_SF_M2S,
               "{dmac_m2s_switch_update_device_capbility::dmac_m2s_d2h_device_info_syn failed.}");
    }

    OAM_WARNING_LOG3(0, OAM_SF_M2S, "{dmac_m2s_switch_update_device_capbility::nss_num[%d],mac_dev_nss[%d],mac_dev_smps_mode[%d].}",
         pst_hal_device->st_cfg_cap_info.en_nss_num, MAC_DEVICE_GET_NSS_NUM(pst_mac_device), MAC_DEVICE_GET_MODE_SMPS(pst_mac_device));

}


oal_void dmac_m2s_switch_same_channel_vaps_begin(hal_to_dmac_device_stru *pst_hal_device,
                   mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap1, mac_vap_stru *pst_mac_vap2)
{
    mac_vap_stru                   *pst_vap_sta;
    mac_fcs_mgr_stru               *pst_fcs_mgr;
    mac_fcs_cfg_stru               *pst_fcs_cfg;

    if (MAC_VAP_STATE_UP == pst_mac_vap1->en_vap_state)
    {
        /* 暂停vap业务 */
        dmac_vap_pause_tx(pst_mac_vap1);
    }

    if (MAC_VAP_STATE_UP == pst_mac_vap2->en_vap_state)
    {
        /* 暂停vap业务 */
        dmac_vap_pause_tx(pst_mac_vap2);
    }

    /* 处于低功耗状态，已经发了null帧，直接返回 */
    if(HAL_DEVICE_WORK_STATE == GET_HAL_DEVICE_STATE(pst_hal_device) &&
         HAL_DEVICE_WORK_SUB_STATE_ACTIVE != GET_WORK_SUB_STATE(pst_hal_device))
    {
        OAM_INFO_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_same_channel_vaps_begin::powersave state[%d].}", GET_WORK_SUB_STATE(pst_hal_device));
        return;
    }

    pst_fcs_mgr = dmac_fcs_get_mgr_stru(pst_mac_device);
    pst_fcs_cfg = MAC_DEV_GET_FCS_CFG(pst_mac_device);
    OAL_MEMZERO(pst_fcs_cfg, OAL_SIZEOF(mac_fcs_cfg_stru));

    pst_fcs_cfg->st_dst_chl = pst_hal_device->st_wifi_channel_status;
    pst_fcs_cfg->pst_hal_device = pst_hal_device;
    pst_fcs_cfg->pst_src_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_vap1);

    /* 同频双STA模式，需要起两次one packet */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap1->en_vap_mode && WLAN_VAP_MODE_BSS_STA == pst_mac_vap2->en_vap_mode)
    {
        /* 准备VAP1的fcs参数 */
        pst_fcs_cfg->st_src_chl = pst_mac_vap1->st_channel;
        dmac_fcs_prepare_one_packet_cfg(pst_mac_vap1, &(pst_fcs_cfg->st_one_packet_cfg), pst_hal_device->st_hal_scan_params.us_scan_time);

        /* 准备VAP2的fcs参数 */
        pst_fcs_cfg->st_src_chl2 = pst_mac_vap2->st_channel;
        dmac_fcs_prepare_one_packet_cfg(pst_mac_vap2, &(pst_fcs_cfg->st_one_packet_cfg2), pst_hal_device->st_hal_scan_params.us_scan_time);

        pst_fcs_cfg->st_one_packet_cfg2.us_timeout = MAC_FCS_DEFAULT_PROTECT_TIME_OUT2;     /* 减小第二次one packet的保护时长，从而减少总时长 */
        pst_fcs_cfg->pst_hal_device = pst_hal_device;

        dmac_fcs_start_enhanced_same_channel(pst_fcs_mgr, pst_fcs_cfg);
        mac_fcs_release(pst_fcs_mgr);
    }
    /* 同频STA+GO模式，同频STA+AP模式，只需要STA起一次one packet */
    else
    {
        if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap1->en_vap_mode)
        {
            pst_vap_sta = pst_mac_vap1;
        }
        else
        {
            pst_vap_sta = pst_mac_vap2;
        }

        pst_fcs_cfg->st_src_chl = pst_vap_sta->st_channel;
        dmac_fcs_prepare_one_packet_cfg(pst_vap_sta, &(pst_fcs_cfg->st_one_packet_cfg), pst_hal_device->st_hal_scan_params.us_scan_time);

        /* 调用FCS切信道接口 保存当前硬件队列的帧到扫描虚假队列 */
        dmac_fcs_start_same_channel(pst_fcs_mgr, pst_fcs_cfg, 0);
        mac_fcs_release(pst_fcs_mgr);
    }
}

#ifdef _PRE_WLAN_FEATURE_DBAC

oal_void dmac_m2s_switch_dbac_vaps_begin(hal_to_dmac_device_stru *pst_hal_device,
                     mac_device_stru  *pst_mac_device, mac_vap_stru  *pst_mac_vap1, mac_vap_stru *pst_mac_vap2)
{
    mac_fcs_mgr_stru               *pst_fcs_mgr;
    mac_fcs_cfg_stru               *pst_fcs_cfg;
    mac_vap_stru                   *pst_current_chan_vap;

    if (pst_hal_device->uc_current_chan_number == pst_mac_vap1->st_channel.uc_chan_number)
    {
        pst_current_chan_vap = pst_mac_vap1;
    }
    else
    {
        pst_current_chan_vap = pst_mac_vap2;
    }

    /* 暂停DBAC切信道 */
    if (mac_is_dbac_running(pst_mac_device))
    {
        dmac_alg_dbac_pause(pst_hal_device);
    }

    dmac_vap_pause_tx(pst_mac_vap1);
    dmac_vap_pause_tx(pst_mac_vap2);

    /* 处于低功耗状态，已经发了null帧，直接返回 */
    if(HAL_DEVICE_WORK_STATE == GET_HAL_DEVICE_STATE(pst_hal_device) &&
         HAL_DEVICE_WORK_SUB_STATE_ACTIVE != GET_WORK_SUB_STATE(pst_hal_device))
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_dbac_vaps_begin::powersave state[%d].}", GET_WORK_SUB_STATE(pst_hal_device));
        return;
    }

    pst_fcs_mgr = dmac_fcs_get_mgr_stru(pst_mac_device);
    pst_fcs_cfg = MAC_DEV_GET_FCS_CFG(pst_mac_device);
    OAL_MEMZERO(pst_fcs_cfg, OAL_SIZEOF(mac_fcs_cfg_stru));

    pst_fcs_cfg->st_dst_chl     = pst_hal_device->st_wifi_channel_status;
    pst_fcs_cfg->pst_hal_device = pst_hal_device;

    pst_fcs_cfg->pst_src_fake_queue = DMAC_VAP_GET_FAKEQ(pst_current_chan_vap);

    pst_fcs_cfg->st_src_chl = pst_hal_device->st_wifi_channel_status;
    dmac_fcs_prepare_one_packet_cfg(pst_current_chan_vap, &(pst_fcs_cfg->st_one_packet_cfg), pst_hal_device->st_hal_scan_params.us_scan_time);

    if (pst_hal_device->uc_current_chan_number != pst_current_chan_vap->st_channel.uc_chan_number)
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_switch_dbac_vap_begin::switch dbac. hal chan num:%d, curr vap chan num:%d. not same,do not send protect frame}",
                        pst_hal_device->uc_current_chan_number, pst_current_chan_vap->st_channel.uc_chan_number);
        pst_fcs_cfg->st_one_packet_cfg.en_protect_type = HAL_FCS_PROTECT_TYPE_NONE;
    }

    dmac_fcs_start_same_channel(pst_fcs_mgr, pst_fcs_cfg, 0);
    mac_fcs_release(pst_fcs_mgr);
}
#endif

#ifdef _PRE_WLAN_FEATURE_DBDC

oal_uint32 dmac_dbdc_vap_reg_shift(dmac_device_stru *pst_dmac_device, mac_vap_stru *pst_shift_vap,
                        hal_to_dmac_device_stru  *pst_shift_hal_device, hal_to_dmac_vap_stru *pst_shift_hal_vap)
{
    dmac_vap_stru                *pst_dmac_vap;
    oal_uint8                     uc_cwmax;
    oal_uint8                     uc_cwmin;
    wlan_wme_ac_type_enum_uint8   en_ac_type;
    oal_uint32                    ul_aid_value;
    oal_uint32                    ul_tsf_valh;
    oal_uint32                    ul_tsf_vall;
    oal_uint32                    ul_led_tbtt_timer;
    oal_uint32                    ul_beacon_rate;

    pst_dmac_vap = MAC_GET_DMAC_VAP(pst_shift_vap);

#ifdef _PRE_WLAN_FEATURE_P2P
    /* P2P 设置MAC 地址 */
    if ((WLAN_P2P_DEV_MODE == pst_shift_vap->en_p2p_mode) && (WLAN_VAP_MODE_BSS_STA == pst_shift_vap->en_vap_mode))
    {
        /* 设置p2p0 MAC 地址 */
        hal_vap_set_macaddr(pst_shift_hal_vap, pst_shift_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID);
    }
    else
    {
        /* 设置其他vap 的mac 地址 */
        hal_vap_set_macaddr(pst_shift_hal_vap, mac_mib_get_StationID(pst_shift_vap));
    }
#else
    /* 配置MAC地址 */
    hal_vap_set_macaddr(pst_shift_hal_vap, mac_mib_get_StationID(pst_shift_vap));
#endif

    /* ap模式涉及寄存器不是下面那些 */
    if (WLAN_VAP_MODE_BSS_AP == pst_shift_vap->en_vap_mode)
    {
        /* 配置MAC EIFS_TIME 寄存器 */
        hal_config_eifs_time(pst_shift_hal_device, pst_shift_vap->en_protocol);

        /* 使能PA_CONTROL的vap_control位 */
        hal_vap_set_opmode(pst_shift_hal_vap, pst_shift_vap->en_vap_mode);

        /* 设置beacon period */
        hal_vap_set_machw_beacon_period(pst_shift_hal_vap, (oal_uint16)mac_mib_get_BeaconPeriod(&pst_dmac_vap->st_vap_base_info));

        /* 更新beacon的发送参数 */
        if ((WLAN_BAND_2G == pst_shift_vap->st_channel.en_band) || (WLAN_BAND_5G == pst_shift_vap->st_channel.en_band))
        {
            ul_beacon_rate = pst_dmac_vap->ast_tx_mgmt_ucast[pst_shift_vap->st_channel.en_band].ast_per_rate[0].ul_value;
            hal_vap_set_beacon_rate(pst_shift_hal_vap, ul_beacon_rate);
        }
        else
        {
            OAM_WARNING_LOG1(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_dbdc_vap_reg_shift::en_band=%d!!!", pst_shift_vap->st_channel.en_band);
        }

#ifndef _PRE_WLAN_MAC_BUGFIX_MCAST_HW_Q
        /* 设置APUT的tbtt offset */
        hal_set_psm_tbtt_offset(pst_shift_hal_vap, pst_dmac_vap->us_in_tbtt_offset);

        /* beacon发送超时继续发送组播队列 */
        hal_set_bcn_timeout_multi_q_enable(pst_shift_hal_vap, OAL_TRUE);
#endif
    }
    else if (WLAN_VAP_MODE_BSS_STA == pst_shift_vap->en_vap_mode)
    {
        /* 1、staut模式寄存器搬移,手动挑入必须入网以及必须写入的寄存器 */
        hal_set_sta_bssid(pst_shift_hal_vap, pst_shift_vap->auc_bssid);
        hal_vap_set_psm_beacon_period(pst_shift_hal_vap, mac_mib_get_BeaconPeriod(pst_shift_vap)); /* 设置beacon周期 */
        hal_set_beacon_timeout_val(pst_shift_hal_device, pst_dmac_vap->us_beacon_timeout); /* beacon超时等待值 */
        //hal_init_pm_info(pst_shift_hal_vap);                        /* 低功耗重新初始化offset以及动态训练重新开始 */
        oal_memcopy(pst_shift_hal_vap->st_pm_info.ast_training_handle, pst_dmac_vap->pst_hal_vap->st_pm_info.ast_training_handle, OAL_SIZEOF(pst_shift_hal_vap->st_pm_info.ast_training_handle));
        pst_shift_hal_vap->st_pm_info.us_inner_tbtt_offset_mimo = pst_dmac_vap->pst_hal_vap->st_pm_info.us_inner_tbtt_offset_mimo;
        pst_shift_hal_vap->st_pm_info.us_inner_tbtt_offset_siso = pst_dmac_vap->pst_hal_vap->st_pm_info.us_inner_tbtt_offset_siso;
        hal_pm_set_tbtt_offset(pst_shift_hal_vap, 0);            /* 更新tbtt offset */
        hal_disable_beacon_filter(pst_shift_hal_device);            /* 关闭beacon帧过滤 */
        hal_enable_non_frame_filter(pst_shift_hal_device);          /* 打开non frame帧过滤 */
        hal_get_txop_ps_partial_aid(pst_dmac_vap->pst_hal_vap, &ul_aid_value);
        hal_set_txop_ps_partial_aid(pst_shift_hal_vap, ul_aid_value); /* 设置mac partial aid寄存器 */
        hal_set_mac_aid(pst_shift_hal_vap, pst_shift_vap->us_sta_aid);/* 设置aid寄存器 */
        //hal_machw_seq_num_index_update_per_tid(pst_shift_hal_device, pst_dmac_user->uc_lut_index, OAL_TRUE);*/
        hal_set_pmf_crypto(pst_shift_hal_vap, (oal_bool_enum_uint8)IS_OPEN_PMF_REG(pst_dmac_vap));

#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
        hal_cfg_rsp_dyn_bw(OAL_TRUE, pst_dmac_device->en_usr_bw_mode);
#endif

        for (en_ac_type = 0; en_ac_type < WLAN_WME_AC_BUTT; en_ac_type++)
        {
            hal_vap_get_edca_machw_cw(pst_dmac_vap->pst_hal_vap, &uc_cwmax, &uc_cwmin, en_ac_type);
            hal_vap_set_edca_machw_cw(pst_shift_hal_vap, uc_cwmax, uc_cwmin, en_ac_type);
        }

        /* 配置MAC EIFS_TIME 寄存器 */
        hal_config_eifs_time(pst_shift_hal_device, pst_shift_vap->en_protocol);

        /* 使能PA_CONTROL的vap_control位 */
        hal_vap_set_opmode(pst_shift_hal_vap, pst_shift_vap->en_vap_mode);

#ifdef _PRE_WLAN_FEATURE_TXOPPS
        /* 初始化TXOP PS */
        dmac_txopps_set_machw(pst_shift_vap);
#endif
        /* 设置P2P noa节能寄存器 */
        if (IS_P2P_NOA_ENABLED(pst_dmac_vap))
        {
            hal_vap_set_noa(pst_dmac_vap->pst_hal_vap, 0, 0, 0, 0);
            hal_vap_set_noa(pst_shift_hal_vap,
                    pst_dmac_vap->st_p2p_noa_param.ul_start_time,
                    pst_dmac_vap->st_p2p_noa_param.ul_duration,
                    pst_dmac_vap->st_p2p_noa_param.ul_interval,
                    pst_dmac_vap->st_p2p_noa_param.uc_count);
        }

        /* 设置P2P ops 寄存器 */
        if (IS_P2P_OPPPS_ENABLED(pst_dmac_vap))
        {
            hal_vap_set_ops(pst_dmac_vap->pst_hal_vap, 0, 0);
            hal_vap_set_ops(pst_shift_hal_vap, pst_dmac_vap->st_p2p_ops_param.en_ops_ctrl, pst_dmac_vap->st_p2p_ops_param.uc_ct_window);
        }
    }

    /* 根据迁移的vap决定是否要将11b分配给迁移到的hal device */
    if (WLAN_BAND_2G == pst_shift_vap->st_channel.en_band)
    {
        hal_set_11b_reuse_sel(pst_shift_hal_device);
    }

    /* 获取的pst_ori_hal_vap tsf */
    hal_vap_tsf_get_64bit(pst_dmac_vap->pst_hal_vap, &ul_tsf_valh, &ul_tsf_vall);

    /* 设置pst_shift_hal_vap的tsf */
    hal_vap_tsf_set_64bit(pst_shift_hal_vap, ul_tsf_valh, ul_tsf_vall);

    hal_vap_read_tbtt_timer(pst_dmac_vap->pst_hal_vap, &ul_led_tbtt_timer);

    /* 设置pst_shift_hal_vap的TBTT TIMER */
    hal_vap_write_tbtt_timer(pst_shift_hal_vap, ul_led_tbtt_timer);

    /* 加密使能 */
    hal_ce_enable_key(pst_shift_hal_device);

    /* 同步过TSF时间后开启tsf */
    hal_disable_tsf_tbtt(pst_dmac_vap->pst_hal_vap);
    hal_enable_tsf_tbtt(pst_shift_hal_vap, OAL_FALSE);

    return OAL_SUCC;
}

oal_void  dmac_vap_clear_hal_device_statis(mac_vap_stru *pst_shift_vap)
{
    dmac_user_stru            *pst_dmac_user;
    hal_to_dmac_device_stru   *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_shift_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_vap_clear_hal_device_statis::vap[%d]get hal device null!}", pst_shift_vap->uc_vap_id);
        return;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (WLAN_VAP_MODE_BSS_STA == pst_shift_vap->en_vap_mode)
    {
        pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_shift_vap->us_assoc_vap_id);
        if (OAL_PTR_NULL == pst_dmac_user)
        {
            OAM_WARNING_LOG1(pst_shift_vap->uc_vap_id, OAM_SF_DBDC,
                "{dmac_vap_clear_hal_device_statis::pst_dmac_user[%d] NULL.}", pst_shift_vap->us_assoc_vap_id);
            return;
        }

        dmac_full_phy_freq_user_del(pst_shift_vap, pst_dmac_user);
    }
    else if (WLAN_VAP_MODE_BSS_AP == pst_shift_vap->en_vap_mode)
    {
        dmac_full_phy_freq_user_del(pst_shift_vap, OAL_PTR_NULL);
    }
#endif

    if (pst_hal_device->uc_assoc_user_nums >= pst_shift_vap->us_user_nums)
    {
        pst_hal_device->uc_assoc_user_nums -=(oal_uint8)(pst_shift_vap->us_user_nums);
    }
    else
    {
        OAM_ERROR_LOG3(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_vap_clear_hal_device_statis::hal device[%d],hal device asoc user[%d]<vap user[%d]!!!}",
                            pst_hal_device->uc_device_id, pst_hal_device->uc_assoc_user_nums, pst_shift_vap->us_user_nums);
    }

}

oal_void  dmac_vap_add_hal_device_statis(mac_vap_stru *pst_shift_vap)
{
    dmac_user_stru            *pst_dmac_user;
    hal_to_dmac_device_stru   *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_shift_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG1(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_vap_add_hal_device_statis::vap[%d]get hal device null!}", pst_shift_vap->uc_vap_id);
        return;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if (WLAN_VAP_MODE_BSS_STA == pst_shift_vap->en_vap_mode)
    {
        pst_dmac_user = (dmac_user_stru *)mac_res_get_dmac_user(pst_shift_vap->us_assoc_vap_id);
        if (OAL_PTR_NULL == pst_dmac_user)
        {
            OAM_WARNING_LOG1(pst_shift_vap->uc_vap_id, OAM_SF_DBDC,
                "{dmac_vap_add_hal_device_statis::pst_dmac_user[%d] NULL.}", pst_shift_vap->us_assoc_vap_id);
            return;
        }

        dmac_full_phy_freq_user_add(pst_shift_vap, pst_dmac_user);
    }
    else if (WLAN_VAP_MODE_BSS_AP == pst_shift_vap->en_vap_mode)
    {
        dmac_full_phy_freq_user_add(pst_shift_vap, OAL_PTR_NULL);
    }
#endif

    pst_hal_device->uc_assoc_user_nums +=(oal_uint8)(pst_shift_vap->us_user_nums);

}


oal_uint32 dmac_dbdc_vap_hal_device_shift(dmac_device_stru  *pst_dmac_device, mac_vap_stru *pst_shift_vap)
{
    hal_to_dmac_device_stru      *pst_ori_hal_device;
    hal_to_dmac_device_stru      *pst_shift_hal_device;
    hal_to_dmac_vap_stru         *pst_ori_hal_vap;
    hal_to_dmac_vap_stru         *pst_shift_hal_vap;

    pst_ori_hal_device   = DMAC_VAP_GET_HAL_DEVICE(pst_shift_vap);
    pst_shift_hal_device = dmac_device_get_another_h2d_dev(pst_dmac_device, pst_ori_hal_device);
    if (OAL_PTR_NULL  == pst_shift_hal_device)
    {
        OAM_ERROR_LOG2(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "dmac_dbdc_vap_hal_device_shift::pst_shift_hal_device NULL, chip id[%d],ori hal device id[%d]",
                        pst_shift_vap->uc_chip_id, pst_ori_hal_device->uc_device_id);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ori_hal_vap      = DMAC_VAP_GET_HAL_VAP(pst_shift_vap);
    hal_get_hal_vap(pst_shift_hal_device, pst_ori_hal_vap->uc_vap_id, &pst_shift_hal_vap);

    /* 1、原hal device上的统计变量清除 */
    dmac_vap_clear_hal_device_statis(pst_shift_vap);

    /* 2、寄存器迁移 */
    dmac_dbdc_vap_reg_shift(pst_dmac_device, pst_shift_vap, pst_shift_hal_device, pst_shift_hal_vap);

    /* 3、hal device,hal vap指针替换 */
    DMAC_VAP_GET_HAL_DEVICE(pst_shift_vap) = pst_shift_hal_device;
    DMAC_VAP_GET_HAL_VAP(pst_shift_vap)    = pst_shift_hal_vap;

    /* 4、在新的hal device上注册统计 */
    dmac_vap_add_hal_device_statis(pst_shift_vap);

    /* 5、从原有hal device状态机上去注册 */
    hal_device_handle_event(pst_ori_hal_device, HAL_DEVICE_EVENT_VAP_DOWN, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)pst_ori_hal_vap);

    /* 6、通知算法,算法已迁移这里是否多余? */
    dmac_pow_set_vap_tx_power(pst_shift_vap, HAL_POW_SET_TYPE_INIT);
    dmac_alg_cfg_channel_notify(pst_shift_vap, CH_BW_CHG_TYPE_MOVE_WORK);
    dmac_alg_cfg_bandwidth_notify(pst_shift_vap, CH_BW_CHG_TYPE_MOVE_WORK);

    /* 7、注册到迁移到的hal device上去 */
    if (WLAN_VAP_MODE_BSS_STA == pst_shift_vap->en_vap_mode)
    {
        hal_device_handle_event(pst_shift_hal_device, HAL_DEVICE_EVENT_JOIN_COMP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)pst_shift_hal_vap);
    }
    hal_device_handle_event(pst_shift_hal_device, HAL_DEVICE_EVENT_VAP_UP, OAL_SIZEOF(hal_to_dmac_vap_stru), (oal_uint8 *)pst_shift_hal_vap);

    /* 8、在新的hal device上设置需要迁移的vap的channel,new hal device上只有一个up vap,否则不符合目前的流程 */
    if (hal_device_calc_up_vap_num(pst_shift_hal_device) != 1)
    {
        OAM_ERROR_LOG2(pst_shift_vap->uc_vap_id, OAM_SF_DBDC, "dmac_dbdc_vap_hal_device_shift::device id[%d],work bitmap[0x%x] up vap > 1!!!",
                        pst_shift_hal_device->uc_device_id, pst_shift_hal_device->ul_work_vap_bitmap);
        return OAL_FAIL;
    }

    dmac_mgmt_switch_channel(pst_shift_hal_device, &(pst_shift_vap->st_channel), OAL_TRUE);

    return OAL_SUCC;
}


oal_void dmac_dbdc_switch_vaps_begin(hal_to_dmac_device_stru *pst_hal_device,
                     mac_device_stru  *pst_mac_device, mac_vap_stru  *pst_mac_vap1, mac_vap_stru *pst_mac_vap2)
{
    mac_vap_stru                   *pst_up_vap;

    if (pst_hal_device->uc_current_chan_number == pst_mac_vap1->st_channel.uc_chan_number)
    {
        pst_up_vap    = pst_mac_vap1;
    }
    else
    {
        pst_up_vap    = pst_mac_vap2;
    }

    OAM_WARNING_LOG2(pst_up_vap->uc_vap_id, OAM_SF_DBDC, "dmac_dbdc_switch_vaps_begin::device id[%d],work bitmap[0x%x]",
                    pst_up_vap->uc_device_id, pst_hal_device->ul_work_vap_bitmap);

    /* up的vap需要pause并切离保护搬移到自己的虚假队列 */
    dmac_m2s_switch_vap_off(pst_hal_device, pst_mac_device, pst_up_vap);
}

oal_void dmac_dbdc_switch_vap_back(mac_device_stru *pst_mac_device, mac_vap_stru *pst_mac_vap)
{
    hal_to_dmac_device_stru        *pst_hal_device;

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);

    /*****************************************************************************
           hal device上vap恢复虚假队列的包到硬件队列,重新设置信道，恢复发送
    *****************************************************************************/
    OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_dbdc_switch_vap_back::hal device id[%d],current channel[%d]vap[%d]channel[%d]}",
                        pst_hal_device->uc_device_id, pst_hal_device->uc_current_chan_number, pst_mac_vap->uc_vap_id,pst_mac_vap->st_channel.uc_chan_number);

    /********************************************************************************************************
        dbdc迁移结束判断此hal device的信道与工作的vap是否一致,先统一重新配置,后续再看能否不配,lzhqi todo
        tx power是init 还是REFRESH?
    *********************************************************************************************************/
    //if (pst_hal_device->uc_current_chan_number != pst_mac_vap->st_channel.uc_chan_number)
    {
        /* 通知算法 */
        dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_INIT);
        dmac_alg_cfg_channel_notify(pst_mac_vap, CH_BW_CHG_TYPE_MOVE_WORK);
        dmac_alg_cfg_bandwidth_notify(pst_mac_vap, CH_BW_CHG_TYPE_MOVE_WORK);

        /* 切信道 */
        dmac_mgmt_switch_channel(pst_hal_device, &(pst_mac_vap->st_channel), OAL_TRUE);
    }

    /* 恢复home信道上被暂停的发送 */
    dmac_vap_resume_tx_by_chl(pst_mac_device, pst_hal_device, &(pst_hal_device->st_wifi_channel_status));
}

oal_void  dmac_dbdc_switch_vaps_back(mac_device_stru *pst_mac_device, mac_vap_stru *pst_keep_vap, mac_vap_stru *pst_shift_vap)
{
    /*****************************************************************************
                   需迁移hal device的vap,迁移结束处理(2G)
    *****************************************************************************/
    dmac_dbdc_switch_vap_back(pst_mac_device, pst_shift_vap);

    /*****************************************************************************
        保留在原有hal device上vap恢复虚假队列的包到硬件队列,重新设置信道(5G)
    *****************************************************************************/
    dmac_dbdc_switch_vap_back(pst_mac_device, pst_keep_vap);
}

oal_void dmac_dbdc_down_vap_back_to_master(mac_vap_stru  *pst_down_vap)
{
    hal_to_dmac_device_stru   *pst_slave_hal_device;
    hal_to_dmac_device_stru   *pst_master_hal_device;
    hal_to_dmac_vap_stru      *pst_ori_hal_vap;
    hal_to_dmac_vap_stru      *pst_shift_hal_vap;
    dmac_device_stru          *pst_dmac_device;
    dmac_vap_stru             *pst_dmac_vap = MAC_GET_DMAC_VAP(pst_down_vap);

    pst_dmac_device      = dmac_res_get_mac_dev(pst_down_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_down_vap->uc_device_id, OAM_SF_DBDC, "{dmac_dbdc_down_vap_back_to_master::pst_dmac_device null.}");
        return;
    }
    pst_slave_hal_device   = DMAC_VAP_GET_HAL_DEVICE(pst_down_vap);
    pst_master_hal_device = dmac_device_get_another_h2d_dev(pst_dmac_device, pst_slave_hal_device);
    if ((OAL_PTR_NULL == pst_master_hal_device) || (OAL_FALSE == (oal_uint8)HAL_CHECK_DEV_IS_MASTER(pst_master_hal_device)))
    {
        OAM_ERROR_LOG2(pst_down_vap->uc_vap_id, OAM_SF_DBDC, "dmac_dbdc_down_vap_back_to_master::master dev NULL or not master,chip id[%d],slave id[%d]",
                        pst_down_vap->uc_chip_id, pst_slave_hal_device->uc_device_id);
        return;
    }

    /* add vap时挂接的算法需再次回到主路 */
    dmac_alg_delete_vap_notify(pst_dmac_vap);

    pst_ori_hal_vap      = DMAC_VAP_GET_HAL_VAP(pst_down_vap);
    hal_get_hal_vap(pst_master_hal_device, pst_ori_hal_vap->uc_vap_id, &pst_shift_hal_vap);

    /* 替换hal device,hal vap指针算法不需要迁移回去 */
    pst_dmac_vap->pst_hal_device = pst_master_hal_device;
    pst_dmac_vap->pst_hal_vap    = pst_shift_hal_vap;

    dmac_alg_create_vap_notify(pst_dmac_vap);

    OAM_WARNING_LOG4(pst_down_vap->uc_vap_id, OAM_SF_DBDC, "dmac_dbdc_down_vap_back_to_master::down vap state[%d]master dev bitmap[0x%x]slave dev[%d]workbimap[0x%x]",
               pst_down_vap->en_vap_state, pst_master_hal_device->ul_work_vap_bitmap,
               pst_slave_hal_device->uc_device_id, pst_slave_hal_device->ul_work_vap_bitmap);

}

oal_void dmac_dbdc_renew_pm_tbtt_offset(dmac_dbdc_state_enum_uint8 en_dbdc_state)
{
    if (DMAC_DBDC_START == en_dbdc_state)
    {
        g_us_ext_inner_offset_diff     += HI1103_DBDC_EXT_INNER_OFFSET_DIFF;
        g_us_inner_tbtt_offset_mimo    += HI1103_DBDC_INNER_TBTT_OFFSET_RESV_DIFF;
        g_us_inner_tbtt_offset_siso    += HI1103_DBDC_INNER_TBTT_OFFSET_RESV_DIFF;
    #ifdef _PRE_PM_DYN_SET_TBTT_OFFSET
        g_us_inner_tbtt_offset_resv    += HI1103_DBDC_INNER_TBTT_OFFSET_RESV_DIFF;
    #endif
    }
    else if (DMAC_DBDC_STOP == en_dbdc_state)
    {
        /*内外部tbtt offset恢复正常值*/
        g_us_ext_inner_offset_diff      = HI1103_EXT_INNER_OFFSET_DIFF;
        g_us_inner_tbtt_offset_mimo     = PM_MIMO_STA_INNER_TBTT_OFFSET;
        g_us_inner_tbtt_offset_siso     = PM_SISO_STA_INNER_TBTT_OFFSET;
    #ifdef _PRE_PM_DYN_SET_TBTT_OFFSET
        g_us_inner_tbtt_offset_resv     = HI1103_INNER_TBTT_OFFSET_RESV;
    #endif
    }
}

oal_void dmac_dbdc_switch_device_end(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru   *pst_hal_device)
{
    oal_uint8                       uc_up_vap_num = 0;
    oal_uint8                       uc_vap_index  = 0;
    oal_uint8                       auc_mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {0};
    mac_vap_stru                   *pst_mac_vap;

    uc_up_vap_num = hal_device_find_all_up_vap(pst_hal_device, auc_mac_vap_id);
    for (uc_vap_index = 0; uc_vap_index < uc_up_vap_num; uc_vap_index++)
    {
        pst_mac_vap  = (mac_vap_stru *)mac_res_get_mac_vap(auc_mac_vap_id[uc_vap_index]);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_DBDC, "dmac_dbdc_switch_device_end::pst_mac_vap[%d] IS NULL.", auc_mac_vap_id[uc_vap_index]);
            continue;
        }

        dmac_dbdc_switch_vap_back(pst_mac_device, pst_mac_vap);
    }
}

oal_void dmac_dbdc_start_renew_dev(hal_to_dmac_device_stru  *pst_hal_device)
{
    oal_uint8                uc_up_vap_num;
    oal_uint8                uc_mac_vap_id;
    mac_vap_stru            *pst_mac_vap;
    hal_to_dmac_vap_stru    *pst_hal_vap;

    uc_up_vap_num = hal_device_calc_up_vap_num(pst_hal_device);
    if (uc_up_vap_num != 1)
    {
        OAM_ERROR_LOG1(0, OAM_SF_DBDC, "{dmac_dbdc_start_renew_dev::master hal dev up vap num[%d]!=1!!!}", uc_up_vap_num);
    }

    if (OAL_SUCC == hal_device_find_one_up_vap(pst_hal_device, &uc_mac_vap_id))
    {
        pst_mac_vap  = (mac_vap_stru *)mac_res_get_mac_vap(uc_mac_vap_id);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_DBDC, "{dmac_dbdc_start_renew_dev::mac_res_get_mac_vap[%d]null}", uc_mac_vap_id);
            return;
        }

        if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
        {
            pst_hal_vap = DMAC_VAP_GET_HAL_VAP(pst_mac_vap);
            pst_hal_vap->st_pm_info.us_inner_tbtt_offset_mimo += HI1103_DBDC_INNER_TBTT_OFFSET_RESV_DIFF;
            pst_hal_vap->st_pm_info.us_inner_tbtt_offset_siso += HI1103_DBDC_INNER_TBTT_OFFSET_RESV_DIFF;

            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_dbdc_start_renew_dev::inner mimo tbtt offset[%d], inner siso tbtt offset[%d]}",
                            pst_hal_vap->st_pm_info.us_inner_tbtt_offset_mimo, pst_hal_vap->st_pm_info.us_inner_tbtt_offset_siso);

            hal_pm_set_tbtt_offset(pst_hal_vap, 0);
        }
    }
}

OAL_STATIC oal_void dmac_dbdc_stop_renew_dev(hal_to_dmac_device_stru  *pst_hal_device)
{
    oal_uint8                uc_up_vap_num;
    oal_uint8                uc_mac_vap_id;
    mac_vap_stru            *pst_mac_vap;
    hal_to_dmac_vap_stru    *pst_hal_vap;

    /* 此时主路可能有一个或者没有up vap存在 */
    uc_up_vap_num = hal_device_calc_up_vap_num(pst_hal_device);
    if (uc_up_vap_num > 1)
    {
        OAM_ERROR_LOG1(0, OAM_SF_DBDC, "{dmac_dbdc_stop_renew_dev::master hal dev up vap num[%d] > 1!!!}", uc_up_vap_num);
    }

    if ((1 == uc_up_vap_num) && (OAL_SUCC == hal_device_find_one_up_vap(pst_hal_device, &uc_mac_vap_id)))
    {
        pst_mac_vap  = (mac_vap_stru *)mac_res_get_mac_vap(uc_mac_vap_id);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_DBDC, "{dmac_dbdc_stop_renew_dev::mac_res_get_mac_vap[%d]null}", uc_mac_vap_id);
            return;
        }

        if (pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_STA)
        {
            return;
        }

        pst_hal_vap = DMAC_VAP_GET_HAL_VAP(pst_mac_vap);
        if ((pst_hal_vap->st_pm_info.us_inner_tbtt_offset_mimo <= HI1103_DBDC_INNER_TBTT_OFFSET_RESV_DIFF) ||
                    (pst_hal_vap->st_pm_info.us_inner_tbtt_offset_siso <= HI1103_DBDC_INNER_TBTT_OFFSET_RESV_DIFF))
        {
            OAM_ERROR_LOG2(uc_mac_vap_id, OAM_SF_DBDC, "{dmac_dbdc_stop_renew_dev::inner mimo tbtt offset[%d],siso tbtt offset[%d],too short!!!}",
                                pst_hal_vap->st_pm_info.us_inner_tbtt_offset_mimo, pst_hal_vap->st_pm_info.us_inner_tbtt_offset_siso);
            return;
        }
        pst_hal_vap->st_pm_info.us_inner_tbtt_offset_mimo -= HI1103_DBDC_INNER_TBTT_OFFSET_RESV_DIFF;
        pst_hal_vap->st_pm_info.us_inner_tbtt_offset_siso -= HI1103_DBDC_INNER_TBTT_OFFSET_RESV_DIFF;

        OAM_WARNING_LOG2(uc_mac_vap_id, OAM_SF_DBDC, "{dmac_dbdc_stop_renew_dev::inner mimo tbtt offset[%d], inner siso tbtt offset[%d]}",
                        pst_hal_vap->st_pm_info.us_inner_tbtt_offset_mimo, pst_hal_vap->st_pm_info.us_inner_tbtt_offset_siso);

        hal_pm_set_tbtt_offset(pst_hal_vap, 0);
    }
}

OAL_STATIC oal_void dmac_dbdc_handle_stop_in_master(dmac_device_stru *pst_dmac_device, hal_to_dmac_device_stru *pst_master_hal_device)
{
    oal_uint8                      uc_up_vap_num;
    oal_uint8                      uc_mac_vap_id = 0;
    hal_to_dmac_device_stru       *pst_slave_hal_device;
    mac_vap_stru                  *pst_shift_mac_vap = OAL_PTR_NULL;

    if (!HAL_CHECK_DEV_IS_MASTER(pst_master_hal_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_DBDC, "{dmac_dbdc_handle_stop_in_master::dev[%d],not in mastet hal dev!!!}", pst_master_hal_device->uc_device_id);
        return;
    }

    /* 主路的去关联,辅路的需要切回主路然后再切到mimo */
    pst_slave_hal_device = dmac_device_get_another_h2d_dev(pst_dmac_device, pst_master_hal_device);
    uc_up_vap_num = hal_device_calc_up_vap_num(pst_slave_hal_device);
    if (uc_up_vap_num != 1)
    {
        OAM_ERROR_LOG3(0, OAM_SF_DBDC, "{dmac_dbdc_handle_stop_in_master::slave dev[%d]up vap[%d]!=1,bitmap[0x%x]}",
                            pst_slave_hal_device->uc_device_id, uc_up_vap_num, pst_slave_hal_device->ul_work_vap_bitmap);
        return;
    }

    if (OAL_SUCC == hal_device_find_one_up_vap(pst_slave_hal_device, &uc_mac_vap_id))
    {
        pst_shift_mac_vap  = (mac_vap_stru *)mac_res_get_mac_vap(uc_mac_vap_id);
    }

    if (OAL_PTR_NULL == pst_shift_mac_vap)
    {
        OAM_ERROR_LOG1(uc_mac_vap_id, OAM_SF_DBDC, "{dmac_dbdc_handle_stop_in_master::pst_shift_mac_vap[%d] null}", uc_mac_vap_id);
        return;
    }

    dmac_up_vap_change_hal_dev(pst_shift_mac_vap);

    OAM_WARNING_LOG2(pst_shift_mac_vap->uc_vap_id, OAM_SF_DBDC, "{dmac_dbdc_handle_stop_in_master::slave workbitmap[0x%x]master work_bitmap[0x%x]}",
        pst_slave_hal_device->ul_work_vap_bitmap, pst_master_hal_device->ul_work_vap_bitmap);
}

oal_void dmac_dbdc_handle_stop_event(hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint8                uc_vap_idx      = 0;
    mac_device_stru         *pst_mac_device  = OAL_PTR_NULL;
    dmac_device_stru        *pst_dmac_device = OAL_PTR_NULL;
    mac_vap_stru            *pst_mac_vap     = OAL_PTR_NULL;

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
       OAM_ERROR_LOG1(0, OAM_SF_DBDC, "{dmac_dbdc_handle_stop_event: mac device is null ptr. device id:%d}", pst_hal_device->uc_mac_device_id);
       return;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_mac_device->uc_device_id, OAM_SF_DBDC, "{dmac_dbdc_handle_siso_event::pst_dmac_device null.}");
        return;
    }

    dmac_dbdc_renew_pm_tbtt_offset(DMAC_DBDC_STOP);

    /* 最后11b都要分配给主路 */
    hal_set_11b_reuse_sel(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device));
    dmac_dbdc_stop_renew_dev(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device));
    dmac_dbdc_stop_renew_dev(DMAC_DEV_GET_SLA_HAL_DEV(pst_dmac_device));

    /* 主路的去关联,辅路的VAP需要切回主路并且切回mimo */
    if (HAL_CHECK_DEV_IS_MASTER(pst_hal_device))
    {
        dmac_dbdc_handle_stop_in_master(pst_dmac_device, pst_hal_device);

        return;
    }

    /* 辅路去关联需要将辅路上的vap切回主路,主路再siso->mimo */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG0(pst_mac_device->auc_vap_id[uc_vap_idx], OAM_SF_DBDC, "{dmac_dbdc_handle_siso_event::mac vap is null!}");
            continue;
        }

        /* 辅路的vap需要切回主路 */
        if(pst_hal_device != MAC_GET_DMAC_VAP(pst_mac_vap)->pst_hal_device)
        {
            continue;
        }

        dmac_dbdc_down_vap_back_to_master(pst_mac_vap);
    }
    OAM_WARNING_LOG2(0, OAM_SF_DBDC, "{dmac_dbdc_handle_stop_event::vap down in dev[%d],master work_bitmap[0x%x]}",
        pst_hal_device->uc_device_id, DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device)->ul_work_vap_bitmap);
}


oal_void dmac_dbdc_switch_vap_to_slave(dmac_device_stru  *pst_dmac_device, mac_vap_stru *pst_shift_mac_vap)
{
    // TODO: 漫游状态切dbdc,重新开始用before up接口是否ok?
    if ((MAC_VAP_STATE_UP == pst_shift_mac_vap->en_vap_state) || (MAC_VAP_STATE_PAUSE == pst_shift_mac_vap->en_vap_state))
    {
        dmac_up_vap_change_hal_dev(pst_shift_mac_vap);
    }
    else
    {
        dmac_vap_change_hal_dev_before_up(pst_shift_mac_vap, pst_dmac_device);
    }
}
#endif


oal_uint32  dmac_m2s_switch_vap_off(hal_to_dmac_device_stru *pst_hal_device,
    mac_device_stru *pst_mac_device, mac_vap_stru  *pst_mac_vap)
{
    mac_fcs_mgr_stru                    *pst_fcs_mgr;
    mac_fcs_cfg_stru                    *pst_fcs_cfg;
    hal_one_packet_status_stru           st_status;

    if (MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state)
    {
        /* 暂停vap业务 */
        dmac_vap_pause_tx(pst_mac_vap);
    }

    /* 外部保证入参不为空 */
    pst_fcs_mgr = dmac_fcs_get_mgr_stru(pst_mac_device);
    pst_fcs_cfg = MAC_DEV_GET_FCS_CFG(pst_mac_device);

    OAL_MEMZERO(pst_fcs_cfg, OAL_SIZEOF(mac_fcs_cfg_stru));

    pst_fcs_cfg->st_src_chl = pst_mac_vap->st_channel;
    pst_fcs_cfg->st_dst_chl = pst_mac_vap->st_channel;
    pst_fcs_cfg->pst_hal_device = pst_hal_device;

    pst_fcs_cfg->pst_src_fake_queue = DMAC_VAP_GET_FAKEQ(pst_mac_vap);

    /* 处于低功耗状态，已经发了null帧，直接返回 */
    if(HAL_DEVICE_WORK_STATE == GET_HAL_DEVICE_STATE(pst_hal_device) &&
         HAL_DEVICE_WORK_SUB_STATE_ACTIVE != GET_WORK_SUB_STATE(pst_hal_device))
    {
        OAM_INFO_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_vap_off::powersave state[%d].}", GET_WORK_SUB_STATE(pst_hal_device));
        return OAL_SUCC;
    }

    dmac_fcs_prepare_one_packet_cfg(pst_mac_vap, &(pst_fcs_cfg->st_one_packet_cfg), pst_hal_device->st_hal_scan_params.us_scan_time);

    /* 调用FCS切信道接口,保存当前硬件队列的帧到扫描虚假队列 */
    dmac_fcs_start_same_channel(pst_fcs_mgr, pst_fcs_cfg, &st_status);
    mac_fcs_release(pst_fcs_mgr);

    if (HAL_FCS_PROTECT_TYPE_NULL_DATA == pst_fcs_cfg->st_one_packet_cfg.en_protect_type  && !st_status.en_null_data_success)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_vap_off::null data failed, sending chan:%d}",
                         pst_fcs_cfg->st_src_chl.uc_chan_number);
    }

    return OAL_SUCC;
}


oal_uint32 dmac_m2s_switch_device_begin(mac_device_stru *pst_mac_device, hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint8                    uc_vap_idx;
    oal_uint8                    uc_up_vap_num;
    oal_uint8                    auc_mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE];
    mac_vap_stru                *pst_mac_vap[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE] = {OAL_PTR_NULL};

    /* Hal Device处于work状态才需要去检查up vap个数 */
    uc_up_vap_num = hal_device_find_all_up_vap(pst_hal_device, auc_mac_vap_id);
    for (uc_vap_idx = 0; uc_vap_idx < uc_up_vap_num; uc_vap_idx++)
    {
        pst_mac_vap[uc_vap_idx]  = (mac_vap_stru *)mac_res_get_mac_vap(auc_mac_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_mac_vap[uc_vap_idx])
        {
            OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_device_begin::pst_mac_vap[%d] IS NULL.}", auc_mac_vap_id[uc_vap_idx]);
            return OAL_ERR_CODE_PTR_NULL;
        }
    }

    if (0 == uc_up_vap_num)
    {
        /* 没有work的vap，表示现在处于扫描状态，前面已经被扫描abort了，都处于idle状态，可以直接做切换，不需要vap pause操作 */
    }
    else if (1 == uc_up_vap_num)
    {
        /* vap为m2s切换做准备 */
        dmac_m2s_switch_vap_off(pst_hal_device, pst_mac_device, pst_mac_vap[0]);
    }
    else if (2 == uc_up_vap_num)
    {
        /* vap为m2s切换做准备 */
        if (pst_mac_vap[0]->st_channel.uc_chan_number != pst_mac_vap[1]->st_channel.uc_chan_number)
        {
            if (OAL_TRUE == mac_is_dbac_running(pst_mac_device))
            {
                dmac_m2s_switch_dbac_vaps_begin(pst_hal_device, pst_mac_device, pst_mac_vap[0], pst_mac_vap[1]);
            }
            else
            {
                dmac_dbdc_switch_vaps_begin(pst_hal_device, pst_mac_device, pst_mac_vap[0], pst_mac_vap[1]);
            }
        }
        else
        {
            dmac_m2s_switch_same_channel_vaps_begin(pst_hal_device, pst_mac_device, pst_mac_vap[0], pst_mac_vap[1]);
        }
    }
    else
    {
        /* m2s不存在其他情况，51静态MIMO，暂时不考虑此接口 */
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_switch_device_begin: cannot support 3 and more vaps!}");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


oal_void  dmac_m2s_switch_device_end(hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint8           uc_up_vap_num;
    mac_device_stru    *pst_mac_device;

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch_device_end::pst_mac_device[%d] is null.}", pst_hal_device->uc_mac_device_id);
        return;
    }

    /* Hal Device处于work状态才需要去检查up vap个数 */
    uc_up_vap_num = hal_device_calc_up_vap_num(pst_hal_device);
    if (0 == uc_up_vap_num)
    {
        /* 处于扫描状态执行切换，没有执行pause，此处也不需要恢复发送，直接返回即可 */
        return;
    }

    if (mac_is_dbac_running(pst_mac_device))
    {
        /* dbac场景只需恢复dbac，由dbac自行切到工作信道 */
        dmac_alg_dbac_resume(pst_hal_device, OAL_TRUE);
        return;
    }

    /* 恢复home信道上被暂停的发送 */
    dmac_vap_resume_tx_by_chl(pst_mac_device, pst_hal_device, &(pst_hal_device->st_wifi_channel_status));
}


oal_void dmac_m2s_nss_and_bw_alg_notify(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user,
     oal_bool_enum_uint8 en_nss_change, oal_bool_enum_uint8 en_bw_change)
{
    hal_to_dmac_device_stru    *pst_hal_device;

    /* 带宽或者nss都没有变化，不需要刷新硬件 */
    if(OAL_FALSE == en_nss_change && OAL_FALSE == en_bw_change)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_nss_and_bw_alg_notify: nss and bw is not change.}");
        return;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_PTR_NULL == pst_hal_device)
    {
       OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S, "{dmac_m2s_nss_and_bw_alg_notify: pst_mac_device is null ptr.}");
       return;
    }


    /*1.若空间流能力发送变化，则调用算法钩子函数,如果带宽和空间流同时改变，要先调用空间流的算法函数*/
    if(OAL_TRUE == en_nss_change)
    {
        dmac_alg_cfg_user_spatial_stream_notify(pst_mac_user);
    }

    /* 2.opmode带宽改变通知算法,并同步带宽信息到HOST */
    if(OAL_TRUE == en_bw_change)
    {
        /* 调用算法改变带宽通知链 */
        dmac_alg_cfg_user_bandwidth_notify(pst_mac_vap, pst_mac_user);
    }

    /* user能力同步到hmac */
    if (OAL_SUCC != dmac_config_d2h_user_m2s_info_syn(pst_mac_vap, pst_mac_user))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_M2S,
                  "{dmac_m2s_nss_and_bw_alg_notify::dmac_config_d2h_user_m2s_info_syn failed.}");
    }

    OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_nss_and_bw_alg_notify: nss[%d]bw[%d] finish.}",
        en_nss_change, en_bw_change);
}


oal_uint32  dmac_m2s_switch(hal_to_dmac_device_stru       *pst_hal_device,
                                             wlan_nss_enum_uint8 en_nss,
                                             hal_m2s_event_tpye_uint16 en_m2s_event,
                                             oal_bool_enum_uint8   en_hw_switch)
{
    mac_device_stru               *pst_mac_device;
    oal_uint32                     ul_ret;

    if(WLAN_MAX_NSS_NUM < en_nss)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch::nss[%d] cannot support!.}", en_nss);
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    pst_mac_device = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
       OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch: mac device is null ptr. device id:%d}", pst_hal_device->uc_mac_device_id);
       return OAL_ERR_CODE_PTR_NULL;
    }

    /* 切换只能在主路执行 */
    if (HAL_DEVICE_ID_MASTER != pst_hal_device->uc_device_id)
    {
        OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_m2s_switch::current hal device id[%d],not in master.}", pst_hal_device->uc_device_id);
        return OAL_FAIL;
    }

    /* wifi可能处于扫描状态，暂时直接abort，如果后续扫描优先级高不允许切换，再考虑直接返回 */
    dmac_scan_abort(pst_mac_device);

    /* 1.保护切离 */
    if(WLAN_M2S_TYPE_SW == pst_hal_device->st_hal_m2s_fsm.en_m2s_type)
    {
        /* 做切换准备，硬件队列包缓存到虚假队列中 */
        ul_ret = dmac_m2s_switch_device_begin(pst_mac_device, pst_hal_device);
        if(OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_switch::dmac_m2s_switch_device_begin return fail.}");
            return OAL_FAIL;
        }
    }

    /* 设置标志，正在做切换的准备工作 */
    pst_hal_device->en_m2s_excute_flag = OAL_TRUE;

    /* 2.更新device 能力 */
    dmac_m2s_switch_update_device_capbility(pst_hal_device, pst_mac_device);

    /* 需要硬件切换才做 */
    if(OAL_TRUE == en_hw_switch)
    {
        /* 2.1更新hal device chain能力 */
        dmac_m2s_switch_update_hal_chain_capbility(pst_hal_device, OAL_TRUE);
    }

    /* 3.更新原来hal device下vap和user能力,vap能力基于hal device能力来, user能力基于vap能力来 */
    dmac_m2s_switch_update_vap_capbility(pst_hal_device);

#if 0 //设计下移之后，能力device vap user各自单独同步到host，该同步接口保留，后续不需要再删除
    /* 5.同步host做mac device能力更新 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_to_hmac_m2s_event_stru));
    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_switch::alloc event failed!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_SDT_REG,
                       DMAC_TO_HMAC_M2S_DATA,
                       OAL_SIZEOF(dmac_to_hmac_m2s_event_stru),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_mac_device->uc_chip_id,
                       pst_mac_device->uc_device_id,
                       0);

    pst_m2s_event = (dmac_to_hmac_m2s_event_stru *)(pst_event->auc_event_data);
    pst_m2s_event->en_m2s_nss   = en_nss;
    pst_m2s_event->uc_device_id = pst_mac_device->uc_device_id;

    if(OAL_SUCC != frw_event_dispatch_event(pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_switch::post event failed}");
    }

    FRW_EVENT_FREE(pst_event_mem);
#endif

    pst_hal_device->en_m2s_excute_flag = OAL_FALSE;

    if(WLAN_M2S_TYPE_SW == pst_hal_device->st_hal_m2s_fsm.en_m2s_type)
    {
        if (HAL_M2S_EVENT_DBDC_START == en_m2s_event)
        {
            /* DBDC START信道已经是后入网的那个vap的,如果后入网的去辅路,主路的信道需重新配置 */
            dmac_dbdc_switch_device_end(pst_mac_device, pst_hal_device);
        }
        else
        {
#ifdef _PRE_WLAN_FEATURE_BTCOEX
            /* 如果已经处于btcoex ps状态下, 不需要再执行restore动作 */
            if(HAL_BTCOEX_SW_POWSAVE_WORK == GET_HAL_BTCOEX_SW_PREEMPT_TYPE(pst_hal_device))
            {
                OAM_WARNING_LOG0(0, OAM_SF_M2S, "{dmac_m2s_switch::btcoex ps is working not need to switch end.}");
            }
            else
#endif
            {
                dmac_m2s_switch_device_end(pst_hal_device);
            }
        }
    }

    return OAL_SUCC;
}


oal_void dmac_m2s_idle_to_xixo(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    dmac_m2s_switch_update_hal_chain_capbility(pst_hal_device, OAL_FALSE);
}


oal_void dmac_m2s_xixo_to_idle(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    hal_device_free_all_rf_dev(pst_hal_device);
}


oal_void dmac_m2s_idle_to_idle(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    oal_uint32 ul_ret;

    /* 刷新配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 未申请rf，需要提前置标志 */
    RF_RES_SET_IS_MIMO(pst_hal_device->st_cfg_cap_info.uc_rf_chain);

    /* 软件切换到siso模式，硬件保持 */
    ul_ret = dmac_m2s_switch(pst_hal_device, WLAN_SINGLE_NSS, en_m2s_event, OAL_FALSE);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_mimo_to_miso:: FAIL en_m2s_event[%d], cur state[%d].}",
            en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
    }
}


oal_void dmac_m2s_mimo_to_miso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;
    oal_uint32 ul_ret;

    uc_trigger_mode = dmac_m2s_event_trigger_mode_classify(en_m2s_event);
    if(WLAN_M2S_TRIGGER_MODE_BUTT == uc_trigger_mode)
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_mimo_to_miso:: device[%d] trigger_mode[%d] error!}",
            pst_hal_device->uc_device_id, uc_trigger_mode);
        return;
    }

    /* 只要是切换到miso，就延迟切换保护触发，需要device开启切离保护, 继续切换到siso就恢复，后续是否执行切换保护，增加device级开关 */
    if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
    {
        GET_HAL_DEVICE_M2S_SWITCH_PROT(pst_hal_device) = OAL_TRUE;
    }

    /* 刷新配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 软件切换到siso模式，硬件保持在mimo */
    ul_ret = dmac_m2s_switch(pst_hal_device, WLAN_SINGLE_NSS, en_m2s_event, OAL_FALSE);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_mimo_to_miso:: FAIL en_m2s_event[%d], cur state[%d].}",
            en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
    }
}


oal_void dmac_m2s_miso_to_mimo(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;
    oal_uint32 ul_ret;

    uc_trigger_mode = dmac_m2s_event_trigger_mode_classify(en_m2s_event);
    if(WLAN_M2S_TRIGGER_MODE_BUTT == uc_trigger_mode)
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_miso_to_mimo:: device[%d] trigger_mode[%d] error!}",
            pst_hal_device->uc_device_id, uc_trigger_mode);
        return;
    }

    /* 只要是切换到mimo，延迟切换保护关闭 */
    if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
    {
        GET_HAL_DEVICE_M2S_SWITCH_PROT(pst_hal_device) = OAL_FALSE;
    }

    /* 刷新mimo配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* miso切换回mimo，硬件能力还是mimo的，不要动，直接切换软件能力 */
   ul_ret = dmac_m2s_switch(pst_hal_device, WLAN_DOUBLE_NSS, en_m2s_event, OAL_FALSE);
   if(OAL_SUCC != ul_ret)
   {
       OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_miso_to_mimo:: FAIL en_m2s_event[%d], cur state[%d].}",
           en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
   }
}


oal_void dmac_m2s_miso_to_siso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    /* 刷新miso配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 软件能力仍然是siso，硬件能力从mimo切换到siso */
    dmac_m2s_switch_update_hal_chain_capbility(pst_hal_device, OAL_TRUE);

    OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_miso_to_siso:: FINISH en_m2s_event[%d], cur state[%d].}",
        en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
}


oal_void dmac_m2s_siso_to_miso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    /* 刷新miso配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 软件仍然是siso状态，硬件切回mimo */
    dmac_m2s_switch_update_hal_chain_capbility(pst_hal_device, OAL_TRUE);

    OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_siso_to_miso:: FINISH en_m2s_event[%d], cur state[%d].}",
        en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
}


oal_void dmac_m2s_mimo_to_siso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    oal_uint32 ul_ret;

    /* 刷新siso配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 软件和硬件能力都刷到siso */
    ul_ret = dmac_m2s_switch(pst_hal_device, WLAN_SINGLE_NSS, en_m2s_event, OAL_TRUE);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_mimo_to_siso:: FAIL en_m2s_event[%d], cur state[%d].}",
            en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
    }
}


oal_void dmac_m2s_siso_to_mimo(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    wlan_m2s_trigger_mode_enum_uint8 uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;
    oal_uint32 ul_ret;

    uc_trigger_mode = dmac_m2s_event_trigger_mode_classify(en_m2s_event);
    if(WLAN_M2S_TRIGGER_MODE_BUTT == uc_trigger_mode)
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_siso_to_mimo:: device[%d] trigger_mode[%d] error!}",
            pst_hal_device->uc_device_id, uc_trigger_mode);
        return;
    }

    /* 只要是切换到mimo，延迟切换保护关闭 */
    if(WLAN_M2S_TRIGGER_MODE_COMMAND == uc_trigger_mode)
    {
        GET_HAL_DEVICE_M2S_SWITCH_PROT(pst_hal_device) = OAL_FALSE;
    }

    /* 刷新siso配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 软件和硬件能力都刷回mimo */
    ul_ret = dmac_m2s_switch(pst_hal_device, WLAN_DOUBLE_NSS, en_m2s_event, OAL_TRUE);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_siso_to_mimo:: FAIL en_m2s_event[%d], cur state[%d].}",
            en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
    }
}


oal_void dmac_m2s_miso_to_miso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    /* 刷新siso配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 硬件能力不变，软件能力从c0 siso和c1 siso切换 */
    /* 更新原来hal device下vap和user能力即可,vap能力基于hal device能力来, user能力基于vap能力来 */
    dmac_m2s_switch_update_vap_capbility(pst_hal_device);

    OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_miso_to_miso:: FINISH en_m2s_event[%d], cur state[%d].}",
        en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
}


oal_void dmac_m2s_siso_to_siso(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    oal_uint32 ul_ret;

    /* 刷新siso配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 软件和硬件能力都从siso刷新到另一侧siso */
    ul_ret = dmac_m2s_switch(pst_hal_device, WLAN_SINGLE_NSS, en_m2s_event, OAL_TRUE);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_siso_to_siso:: FAIL en_m2s_event[%d], cur state[%d].}",
            en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
    }
}


oal_void dmac_m2s_mimo_to_simo(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    /* 刷新siso配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 软件仍然是mimo状态，硬件切到simo */
    dmac_m2s_switch_update_hal_chain_capbility(pst_hal_device, OAL_TRUE);

    OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_mimo_to_simo:: FINISH en_m2s_event[%d], cur state[%d].}",
        en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
}


oal_void dmac_m2s_simo_to_mimo(hal_to_dmac_device_stru *pst_hal_device, hal_m2s_event_tpye_uint16 en_m2s_event)
{
    /* 刷新siso配置 */
    dmac_m2s_update_switch_mgr_param(pst_hal_device, en_m2s_event);

    /* 软件仍然是siso状态，硬件切回mimo */
    dmac_m2s_switch_update_hal_chain_capbility(pst_hal_device, OAL_TRUE);

    OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_mimo_to_simo:: FINISH en_m2s_event[%d], cur state[%d].}",
        en_m2s_event, GET_HAL_M2S_CUR_STATE(pst_hal_device));
}


oal_uint32  dmac_get_hal_m2s_state(hal_to_dmac_device_stru *pst_hal_device)
{
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_get_hal_m2s_state::hal_to_dmac_device_stru NULL}");
        return HAL_DEVICE_IDLE_STATE;
    }

    if (OAL_FALSE == pst_hal_device->st_hal_dev_fsm.uc_is_fsm_attached)
    {
        OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_get_hal_m2s_state::hal device id[%d],m2s fsm not attached}", pst_hal_device->uc_device_id);
        return HAL_DEVICE_IDLE_STATE;
    }

    return GET_HAL_M2S_CUR_STATE(pst_hal_device);
}


OAL_STATIC oal_uint32 dmac_m2s_fsm_trans_to_state(hal_m2s_fsm_stru *pst_m2s_fsm, oal_uint8 uc_state)
{
    oal_fsm_stru   *pst_oal_fsm = &(pst_m2s_fsm->st_oal_fsm);

    return oal_fsm_trans_to_state(pst_oal_fsm, uc_state);
}


OAL_STATIC  oal_void dmac_m2s_state_idle_entry(oal_void *p_ctx)
{
    hal_to_dmac_device_stru      *pst_hal_device;
    hal_m2s_fsm_stru             *pst_m2s_fsm;
    hal_m2s_event_tpye_uint16     us_m2s_event;

    pst_m2s_fsm = (hal_m2s_fsm_stru *)p_ctx;
    pst_hal_device  = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_state_idle_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    us_m2s_event = pst_m2s_fsm->st_oal_fsm.us_last_event;

    switch(us_m2s_event)
    {
        /* 进入idle状态，需要释放rf资源即可 */
        case HAL_M2S_EVENT_IDLE_BEGIN:
            hal_device_free_all_rf_dev(pst_hal_device);
            break;

        default:
           OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_state_idle_entry: hal dev[%d] us_m2s_event[%d] error!}",pst_hal_device->uc_device_id, us_m2s_event);
    }

    OAM_WARNING_LOG3(0, OAM_SF_M2S, "{dmac_m2s_state_idle_entry::hal dev[%d],event[%d],change to idle,last state[%d]}",
                        pst_hal_device->uc_device_id, pst_m2s_fsm->st_oal_fsm.us_last_event, pst_m2s_fsm->st_oal_fsm.uc_cur_state);

}


OAL_STATIC  oal_void dmac_m2s_state_idle_exit(oal_void *p_ctx)
{
    /* TBD */
}


OAL_STATIC oal_uint32 dmac_m2s_state_idle_event(oal_void  *p_ctx, oal_uint16  us_event,
                                    oal_uint16 us_event_data_len,  oal_void  *p_event_data)
{
    hal_m2s_fsm_stru                 *pst_m2s_fsm = (hal_m2s_fsm_stru *)p_ctx;
    hal_to_dmac_device_stru          *pst_hal_device = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);
    hal_m2s_state_uint8               uc_m2s_state;
    wlan_m2s_trigger_mode_enum_uint8  uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;

    uc_trigger_mode = dmac_m2s_event_trigger_mode_classify(us_event);

    /* 1. 刷新对应业务标记 */
    if(WLAN_M2S_TRIGGER_MODE_BUTT != uc_trigger_mode)
    {
        GET_HAL_M2S_MODE_TPYE(pst_hal_device) |= uc_trigger_mode;
    }

    /* 除了hal idle end，其他业务触发事件都是修改对应的chain能力 */
    switch(us_event)
    {
        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_WORK_BEGIN:
            uc_m2s_state = dmac_m2s_chain_state_classify(pst_hal_device);

            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, uc_m2s_state);

            #if 0 //并发扫描怎么使用，需要对一下
            if(HAL_M2S_EVENT_SCAN_BEGIN == us_event)
            {
                dmac_device_stru *pst_dmac_device = dmac_res_get_mac_dev(pst_hal_device->uc_mac_device_id);
                hal_device_handle_event(dmac_device_get_another_h2d_dev(pst_dmac_device, pst_hal_device), HAL_DEVICE_EVENT_SCAN_RESUME, 0, OAL_PTR_NULL);
            }
            #endif
        break;

        default:
            uc_m2s_state = dmac_m2s_event_state_classify(us_event);

            /* 是恢复到mimo，那就是业务结束，需要清标记 */
            if(HAL_M2S_STATE_MIMO == uc_m2s_state)
            {
                GET_HAL_M2S_MODE_TPYE(pst_hal_device) &= (~uc_trigger_mode);
            }

            /* 其他事件来了需要刷新软硬件能力，不申请释放rf，还是处于idle状态，等状态跳变时候再做切换 rf能力申请 */
            dmac_m2s_idle_to_idle(pst_hal_device, us_event);

            OAM_WARNING_LOG2(pst_hal_device->uc_device_id, OAM_SF_M2S, "{dmac_m2s_state_idle_event::event[%d]uc_m2s_state[%d] no handle!!!}",
                us_event, uc_m2s_state);
        break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_m2s_state_mimo_entry(oal_void *p_ctx)
{
    hal_to_dmac_device_stru      *pst_hal_device;
    hal_m2s_fsm_stru             *pst_m2s_fsm;
    hal_m2s_event_tpye_uint16     us_m2s_event;

    pst_m2s_fsm = (hal_m2s_fsm_stru *)p_ctx;
    pst_hal_device  = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_state_mimo_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    us_m2s_event = pst_m2s_fsm->st_oal_fsm.us_last_event;

    switch(us_m2s_event)
    {
        case HAL_M2S_EVENT_DBDC_STOP:
        case HAL_M2S_EVENT_DBDC_SISO_TO_MIMO:
            /* m2s状态机清除dbdc状态 */
            dmac_m2s_siso_to_mimo(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_TEST_SISO_C0_TO_MIMO:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_MIMO:
            /* m2s状态机清除test */
            dmac_m2s_siso_to_mimo(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
            /* m2s状态机清除command */
            dmac_m2s_siso_to_mimo(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
            /* m2s状态机清除test */
            dmac_m2s_miso_to_mimo(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO:
            dmac_m2s_miso_to_mimo(pst_hal_device, us_m2s_event);
            break;

        /* scan结束需要恢复硬件能力 */
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
            /* m2s状态机清除并发扫描 */
            dmac_m2s_simo_to_mimo(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_WORK_BEGIN:
            dmac_m2s_idle_to_xixo(pst_hal_device, us_m2s_event);
            break;

        default:
            OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_state_mimo_entry: hal dev[%d] us_m2s_event[%d] error!}",pst_hal_device->uc_device_id, us_m2s_event);
    }

    OAM_WARNING_LOG3(0, OAM_SF_M2S, "{dmac_m2s_state_mimo_entry::hal dev[%d],event[%d],change to mimo,last state[%d]}",
                        pst_hal_device->uc_device_id, us_m2s_event, pst_m2s_fsm->st_oal_fsm.uc_cur_state);
}


OAL_STATIC oal_void dmac_m2s_state_mimo_exit(oal_void *p_ctx)
{
    /* TBD */
}


OAL_STATIC oal_uint32 dmac_m2s_state_mimo_event(oal_void  *p_ctx,
                                                             oal_uint16 us_event,
                                                             oal_uint16 us_event_data_len,
                                                             oal_void  *p_event_data)
{
    dmac_device_stru                 *pst_dmac_device;
    hal_to_dmac_device_stru          *pst_hal_device;
    hal_m2s_fsm_stru                 *pst_m2s_fsm = (hal_m2s_fsm_stru *)p_ctx;
    wlan_m2s_trigger_mode_enum_uint8  uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;

    pst_hal_device  = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);

    uc_trigger_mode = dmac_m2s_event_trigger_mode_classify(us_event);

    /* 1. 刷新对应业务标记 */
    if(WLAN_M2S_TRIGGER_MODE_BUTT != uc_trigger_mode)
    {
        GET_HAL_M2S_MODE_TPYE(pst_hal_device) |= uc_trigger_mode;
    }

    switch (us_event)
    {
        /* hal device进入idle状态，m2s也进入idle状态，释放rf资源 */
        case HAL_M2S_EVENT_IDLE_BEGIN:
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_IDLE);
          break;

        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_MISO);
          break;

        case HAL_M2S_EVENT_DBDC_MIMO_TO_SISO:
        case HAL_M2S_EVENT_DBDC_START:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_SISO);
          break;

        case HAL_M2S_EVENT_SCAN_BEGIN:
            pst_dmac_device = dmac_res_get_mac_dev(pst_hal_device->uc_mac_device_id);
            if (OAL_TRUE == pst_dmac_device->en_is_fast_scan)
            {
                /* m2s状态机使能并发扫描 */
                GET_HAL_M2S_MODE_TPYE(pst_hal_device) |= WLAN_M2S_TRIGGER_MODE_FAST_SCAN;

                dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_SIMO);
                hal_device_handle_event(dmac_device_get_another_h2d_dev(pst_dmac_device, pst_hal_device), HAL_DEVICE_EVENT_SCAN_RESUME, 0, OAL_PTR_NULL);
            }
          break;

        /* mimo状态也会收到此事件，非并发扫描时 */
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
        case HAL_M2S_EVENT_WORK_BEGIN:
          break;

        default:
            OAM_ERROR_LOG1(pst_hal_device->uc_device_id, OAM_SF_M2S, "{dmac_m2s_state_mimo_event:: us_event[%d] INVALID!}", us_event);
          break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_m2s_state_siso_entry(oal_void *p_ctx)
{
    hal_to_dmac_device_stru      *pst_hal_device;
    hal_m2s_fsm_stru             *pst_m2s_fsm;
    hal_m2s_event_tpye_uint16     us_m2s_event;

    pst_m2s_fsm = (hal_m2s_fsm_stru *)p_ctx;
    pst_hal_device  = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);

    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_state_siso_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    us_m2s_event = pst_m2s_fsm->st_oal_fsm.us_last_event;

    switch(us_m2s_event)
    {
        case HAL_M2S_EVENT_DBDC_MIMO_TO_SISO:
            dmac_m2s_mimo_to_siso(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_MIMO_TO_SISO_C1:
            dmac_m2s_mimo_to_siso(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_SISO_C1:
            dmac_m2s_mimo_to_siso(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C0_SISO:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C1_SISO:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
            dmac_m2s_miso_to_siso(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_DBDC_START:
            dmac_m2s_mimo_to_siso(pst_hal_device, us_m2s_event);//先主路切到siso
            //dmac_dbdc_switch_vap_to_slave(pst_hal_device);//再将主路的vap切到辅路去
            break;

        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_WORK_BEGIN:
            dmac_m2s_idle_to_xixo(pst_hal_device, us_m2s_event);
            break;

        default:
           OAM_WARNING_LOG2(0, OAM_SF_M2S, "{dmac_m2s_state_siso_entry: hal dev[%d] us_m2s_event[%d] error!}",pst_hal_device->uc_device_id, us_m2s_event);
    }

    OAM_WARNING_LOG3(0, OAM_SF_M2S, "{dmac_m2s_state_siso_entry::hal dev[%d],event[%d],change to siso,last state[%d]}",
                        pst_hal_device->uc_device_id, us_m2s_event, pst_m2s_fsm->st_oal_fsm.uc_cur_state);
}


OAL_STATIC oal_void dmac_m2s_state_siso_exit(oal_void *p_ctx)
{

}


OAL_STATIC oal_uint32 dmac_m2s_state_siso_event(oal_void  *p_ctx,
                                                         oal_uint16  us_event,
                                                         oal_uint16 us_event_data_len,
                                                         oal_void  *p_event_data)
{
    hal_m2s_fsm_stru                 *pst_m2s_fsm;
    hal_to_dmac_device_stru          *pst_hal_device;
    dmac_device_stru                 *pst_dmac_device;
    wlan_m2s_trigger_mode_enum_uint8  uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;
    hal_m2s_state_uint8               en_new_state;

    pst_m2s_fsm     = (hal_m2s_fsm_stru *)p_ctx;
    pst_hal_device  = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);

    pst_dmac_device = dmac_res_get_mac_dev(pst_hal_device->uc_mac_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(pst_hal_device->uc_mac_device_id, OAM_SF_M2S, "{dmac_m2s_state_siso_event::pst_dmac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    uc_trigger_mode = dmac_m2s_event_trigger_mode_classify(us_event);

    /* 1. 刷新对应业务标记 */
    if(WLAN_M2S_TRIGGER_MODE_BUTT != uc_trigger_mode)
    {
        GET_HAL_M2S_MODE_TPYE(pst_hal_device) |= uc_trigger_mode;
    }

    /* 2.计划回到mixo, 对应业务结束置回标记 */
    switch (us_event)
    {
        /* hal device进入idle状态，m2s也进入idle状态，释放rf资源 */
        case HAL_M2S_EVENT_IDLE_BEGIN:
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_IDLE);
          break;

        /* 接收功率周期性触发探测 */
        case HAL_M2S_EVENT_ANT_RSSI_SISO_TO_MISO:
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_MISO);
          break;

        /* NONE方式下仍打开了切换保护，此时不处理即可 */
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
            OAM_WARNING_LOG0(pst_hal_device->uc_device_id, OAM_SF_M2S, "{dmac_m2s_state_siso_event::delay switch already succ to siso.}");
          break;

          /* 触发回到mimo */
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_MIMO:
        case HAL_M2S_EVENT_TEST_SISO_C1_TO_MIMO:
        case HAL_M2S_EVENT_DBDC_SISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_SISO_TO_MIMO:
            /* 业务结束申请状态切换管理 */
            dmac_m2s_switch_back_to_mixo_check(pst_m2s_fsm, pst_hal_device, uc_trigger_mode, &en_new_state);
          break;

        case HAL_M2S_EVENT_TEST_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_TEST_SISO_C0_TO_SISO_C1:
        case HAL_M2S_EVENT_COMMAND_SISO_C1_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_SISO_C0_TO_SISO_C1:
              /* c0到c1之间的切换，状态机不变，只是抛事件做通道重新配置 */
              dmac_m2s_siso_to_siso(pst_hal_device, us_event);
          break;

        /* siso下收到dbdc start不需要mimo切siso,直接主路vap切到辅路 */
        case HAL_M2S_EVENT_DBDC_START:
            //dmac_dbdc_switch_vap_to_slave(pst_hal_device);
          break;

        case HAL_M2S_EVENT_DBDC_STOP:
            /* 辅路的m2s fsm始终在siso状态，不需要动 */
            /* 主路的m2s fsm切回mimo */
            dmac_m2s_fsm_trans_to_state(&(DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device)->st_hal_m2s_fsm), HAL_M2S_STATE_MIMO);
          break;

        /* siso状态收到扫描事件不需要处理, scan恢复到work的话，也不需要处理 */
        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_WORK_BEGIN:
          break;

        default:
            OAM_ERROR_LOG1(pst_hal_device->uc_device_id, OAM_SF_M2S, "{dmac_m2s_state_siso_event:: us_event[%d] INVALID!}", us_event);
          break;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_m2s_state_miso_entry(oal_void *p_ctx)
{
    hal_to_dmac_device_stru      *pst_hal_device;
    hal_m2s_fsm_stru             *pst_m2s_fsm;
    hal_m2s_event_tpye_uint16     us_m2s_event;

    pst_m2s_fsm = (hal_m2s_fsm_stru *)p_ctx;
    pst_hal_device  = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);

    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_state_miso_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    us_m2s_event = pst_m2s_fsm->st_oal_fsm.us_last_event;

    switch(us_m2s_event)
    {
        case HAL_M2S_EVENT_ANT_RSSI_SISO_TO_MISO:
            dmac_m2s_siso_to_miso(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_ANT_RSSI_MIMO_TO_MISO:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MIMO_TO_MISO_C1:
            dmac_m2s_mimo_to_miso(pst_hal_device, us_m2s_event);
            break;

        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_WORK_BEGIN:
            dmac_m2s_idle_to_xixo(pst_hal_device, us_m2s_event);
            break;

        default:
            OAM_ERROR_LOG2(0, OAM_SF_M2S, "{dmac_m2s_state_miso_entry: hal dev[%d] us_m2s_event[%d] error!}",pst_hal_device->uc_device_id, us_m2s_event);
            return;
    }

    OAM_WARNING_LOG3(0, OAM_SF_M2S, "{dmac_m2s_state_miso_entry::hal dev[%d],event[%d],change to miso,last state[%d]}",
                    pst_hal_device->uc_device_id, us_m2s_event, pst_m2s_fsm->st_oal_fsm.uc_cur_state);
}


OAL_STATIC oal_void dmac_m2s_state_miso_exit(oal_void *p_ctx)
{
    /* 打开双通道, 协议依然为SISO */
}


OAL_STATIC oal_uint32 dmac_m2s_state_miso_event(oal_void  *p_ctx,
                                                            oal_uint16  us_event,
                                                            oal_uint16 us_event_data_len,
                                                            oal_void  *p_event_data)
{
    hal_to_dmac_device_stru          *pst_hal_device;
    hal_m2s_fsm_stru                 *pst_m2s_fsm;
    wlan_m2s_trigger_mode_enum_uint8  uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;
    hal_m2s_state_uint8               en_new_state;

    pst_m2s_fsm = (hal_m2s_fsm_stru *)p_ctx;
    pst_hal_device  = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);

    uc_trigger_mode = dmac_m2s_event_trigger_mode_classify(us_event);

    /* 1. 刷新对应业务标记 */
    if(WLAN_M2S_TRIGGER_MODE_BUTT != uc_trigger_mode)
    {
        GET_HAL_M2S_MODE_TPYE(pst_hal_device) |= uc_trigger_mode;
    }

    switch (us_event)
    {
        /* hal device进入idle状态，m2s也进入idle状态，释放rf资源 */
        case HAL_M2S_EVENT_IDLE_BEGIN:
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_IDLE);
          break;

        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C0_SISO:
        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_C1_SISO:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_SISO_C1:
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_SISO);
          break;

        case HAL_M2S_EVENT_ANT_RSSI_MISO_TO_MIMO:
        case HAL_M2S_EVENT_COMMAND_MISO_TO_MIMO:
            /* 允许切回misx，就切回，否则保持在siso状态 */
            dmac_m2s_switch_back_to_mixo_check(pst_m2s_fsm, pst_hal_device, uc_trigger_mode, &en_new_state);
          break;

        case HAL_M2S_EVENT_COMMAND_MISO_C1_TO_MISO_C0:
        case HAL_M2S_EVENT_COMMAND_MISO_C0_TO_MISO_C1:
              /* c0到c1之间的切换，状态机不变，只是抛事件做通道重新配置 */
              dmac_m2s_miso_to_miso(pst_hal_device, us_event);
          break;

        /* mimo状态也会收到此事件，非并发扫描时 */
        case HAL_M2S_EVENT_WORK_BEGIN:
        case HAL_M2S_EVENT_SCAN_BEGIN:
        case HAL_M2S_EVENT_SCAN_END:
        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
          break;

        default:
            OAM_ERROR_LOG2(pst_hal_device->uc_device_id, OAM_SF_M2S, "{dmac_m2s_state_miso_event:cur state[%d] wrong event[%d].}",
                GET_HAL_M2S_CUR_STATE(pst_hal_device), us_event);
    }

    return OAL_SUCC;

}

OAL_STATIC oal_void dmac_m2s_state_simo_entry(oal_void *p_ctx)
{
    hal_to_dmac_device_stru      *pst_hal_device;
    hal_m2s_fsm_stru             *pst_m2s_fsm;
    hal_m2s_event_tpye_uint16     us_m2s_event;
    dmac_device_stru             *pst_dmac_device;

    pst_m2s_fsm = (hal_m2s_fsm_stru *)p_ctx;
    pst_hal_device  = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);

    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_M2S, "{dmac_m2s_state_simo_entry::hal_to_dmac_device_stru null.}");
        return;
    }

    /* 获取m2s事件类型 */
    us_m2s_event = pst_m2s_fsm->st_oal_fsm.us_last_event;

    switch(us_m2s_event)
    {
        /* 并发扫描和普通前景背景扫描需要区分开类型 TBD */
        case HAL_M2S_EVENT_SCAN_BEGIN:
            pst_dmac_device = dmac_res_get_mac_dev(pst_hal_device->uc_mac_device_id);
            if (OAL_TRUE == pst_dmac_device->en_is_fast_scan)
            {
                dmac_m2s_mimo_to_simo(pst_hal_device, us_m2s_event);
            }
            else
            {
                dmac_m2s_idle_to_xixo(pst_hal_device, us_m2s_event);
            }
            break;

        case HAL_M2S_EVENT_WORK_BEGIN:
            dmac_m2s_idle_to_xixo(pst_hal_device, us_m2s_event);
            break;

        default:
            OAM_ERROR_LOG2(0, OAM_SF_M2S, "{dmac_m2s_state_simo_entry: hal dev[%d] us_m2s_event[%d] error!}",pst_hal_device->uc_device_id, us_m2s_event);
            return;
    }

    OAM_WARNING_LOG3(0, OAM_SF_M2S, "{dmac_m2s_state_simo_entry::hal dev[%d],event[%d],change to simo,last state[%d]}",
                        pst_hal_device->uc_device_id, us_m2s_event, pst_m2s_fsm->st_oal_fsm.uc_cur_state);
}


OAL_STATIC oal_void dmac_m2s_state_simo_exit(oal_void *p_ctx)
{
    return;
}


OAL_STATIC oal_uint32 dmac_m2s_state_simo_event(oal_void  *p_ctx,
                                                            oal_uint16  us_event,
                                                            oal_uint16 us_event_data_len,
                                                            oal_void  *p_event_data)
{
    hal_to_dmac_device_stru          *pst_hal_device;
    dmac_device_stru                 *pst_dmac_device;
    hal_m2s_fsm_stru                 *pst_m2s_fsm = (hal_m2s_fsm_stru *)p_ctx;;
    wlan_m2s_trigger_mode_enum_uint8  uc_trigger_mode = WLAN_M2S_TRIGGER_MODE_BUTT;
    hal_m2s_state_uint8               en_new_state;

    pst_hal_device  = (hal_to_dmac_device_stru *)(pst_m2s_fsm->st_oal_fsm.p_oshandler);
    pst_dmac_device = dmac_res_get_mac_dev(pst_hal_device->uc_mac_device_id);

    uc_trigger_mode = dmac_m2s_event_trigger_mode_classify(us_event);

    /* 1. 刷新对应业务标记 */
    if(WLAN_M2S_TRIGGER_MODE_BUTT != uc_trigger_mode)
    {
        GET_HAL_M2S_MODE_TPYE(pst_hal_device) |= uc_trigger_mode;
    }

    switch (us_event)
    {
        /* hal device进入idle状态，m2s也进入idle状态，释放rf资源 */
        case HAL_M2S_EVENT_IDLE_BEGIN:
            dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_IDLE);
          break;

        case HAL_M2S_EVENT_SCAN_CHANNEL_BACK:
            /*抛事件给另一个hal device状态机,暂停此hal device的扫描,让出rf资源供主路回home channel工作 */
            hal_device_handle_event(dmac_device_get_another_h2d_dev(pst_dmac_device, pst_hal_device), HAL_DEVICE_EVENT_SCAN_PAUSE, 0, OAL_PTR_NULL);

            /* 允许切回misx，就切回，否则保持在siso状态 */
            dmac_m2s_switch_back_to_mixo_check(pst_m2s_fsm, pst_hal_device, uc_trigger_mode, &en_new_state);

          break;

        case HAL_M2S_EVENT_SCAN_END:
            /* 允许切回misx，就切回，否则保持在siso状态 */
            dmac_m2s_switch_back_to_mixo_check(pst_m2s_fsm, pst_hal_device, uc_trigger_mode, &en_new_state);

          break;

        case HAL_M2S_EVENT_WORK_BEGIN:
          break;

        default:
            OAM_ERROR_LOG2(pst_hal_device->uc_device_id, OAM_SF_M2S, "{dmac_m2s_state_simo_event:cur state[%d] wrong event[%d].}",
                GET_HAL_M2S_CUR_STATE(pst_hal_device), us_event);
    }

    return OAL_SUCC;
}


oal_uint32 dmac_m2s_handle_event(hal_to_dmac_device_stru *pst_hal_device, oal_uint16 us_type,
    oal_uint16 us_datalen, oal_uint8* pst_data)
{
    oal_uint32                 ul_ret;
    hal_m2s_fsm_stru          *pst_m2s_fsm;

    pst_m2s_fsm = &(pst_hal_device->st_hal_m2s_fsm);
    if (OAL_FALSE == pst_m2s_fsm->uc_is_fsm_attached)
    {
        OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_m2s_handle_event::hal device[%d]fsm not attached!}", pst_hal_device->uc_device_id);
        return OAL_FAIL;
    }

    ul_ret = oal_fsm_event_dispatch(&(pst_m2s_fsm->st_oal_fsm), us_type, us_datalen, pst_data);
    if (ul_ret != OAL_SUCC)
    {
        OAM_ERROR_LOG3(0, OAM_SF_M2S, "{dmac_m2s_handle_event::state[%d]dispatch event[%d]not succ[%d]!}",
                        GET_HAL_M2S_CUR_STATE(pst_hal_device), us_type, ul_ret);
    }

    return ul_ret;
}


oal_void dmac_m2s_fsm_attach(hal_to_dmac_device_stru   *pst_hal_device)
{
    oal_uint8                auc_fsm_name[6] = {0};
    oal_uint32               ul_ret;
    hal_m2s_fsm_stru        *pst_m2s_fsm  = OAL_PTR_NULL;
    hal_m2s_state_uint8      uc_state = HAL_M2S_STATE_IDLE;  /* 初始工作状态 */

    if (OAL_TRUE == pst_hal_device->st_hal_m2s_fsm.uc_is_fsm_attached)
    {
        OAM_WARNING_LOG1(0, OAM_SF_M2S, "{dmac_m2s_fsm_attach::hal device id[%d]fsm have attached.}", pst_hal_device->uc_device_id);
        return;
    }

    pst_m2s_fsm = &(pst_hal_device->st_hal_m2s_fsm);

    /* 因为hal device初始是idle状态状态，m2s也需要初始为此状态 */
    //uc_state = dmac_m2s_chain_state_classify(pst_hal_device);

    OAL_MEMZERO(pst_m2s_fsm, OAL_SIZEOF(hal_m2s_fsm_stru));

    /* 准备一个唯一的fsmname */
    auc_fsm_name[0] = (oal_uint8)pst_hal_device->ul_core_id;
    auc_fsm_name[1] = pst_hal_device->uc_chip_id;
    auc_fsm_name[2] = pst_hal_device->uc_mac_device_id;
    auc_fsm_name[3] = pst_hal_device->uc_device_id;

    ul_ret = oal_fsm_create((oal_void*)pst_hal_device,
                              auc_fsm_name,
                              pst_m2s_fsm,
                              &(pst_m2s_fsm->st_oal_fsm),
                              uc_state,
                              g_ast_hal_m2s_fsm_info,
                              OAL_SIZEOF(g_ast_hal_m2s_fsm_info)/OAL_SIZEOF(oal_fsm_state_info));

    if (OAL_SUCC == ul_ret)
    {
        /* oal fsm create succ */
        pst_m2s_fsm->uc_is_fsm_attached    = OAL_TRUE;
        pst_m2s_fsm->en_m2s_type           = WLAN_M2S_TYPE_SW;
        OAL_MEMZERO(&pst_m2s_fsm->st_m2s_mode, OAL_SIZEOF(wlan_m2s_mode_stru)); /* 暂时还没有业务申请切换 */
    }
}


oal_void dmac_m2s_fsm_detach(hal_to_dmac_device_stru *pst_hal_device)
{
    hal_m2s_fsm_stru         *pst_m2s_fsm    = OAL_PTR_NULL;

    pst_m2s_fsm = &(pst_hal_device->st_hal_m2s_fsm);

    if (OAL_FALSE == pst_m2s_fsm->uc_is_fsm_attached)
    {
        OAM_ERROR_LOG1(0, OAM_SF_M2S, "{dmac_m2s_fsm_detach::hal device id[%d]fsm not attatched}", pst_hal_device->uc_device_id);
        return;
    }

    /* 不是IDLE状态切换到IDLE状态 */
    if (GET_HAL_M2S_CUR_STATE(pst_hal_device) != HAL_M2S_STATE_IDLE)
    {
        dmac_m2s_fsm_trans_to_state(pst_m2s_fsm, HAL_M2S_STATE_IDLE);
    }

    return;
}



oal_void dmac_m2s_mgmt_switch(hal_to_dmac_device_stru *pst_hal_device, dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_single_tx_chain)
{
    pst_hal_device->st_cfg_cap_info.uc_single_tx_chain = uc_single_tx_chain;
    /* 1.修订寄存器控响应帧发送通道 */
    hal_update_datarate_by_chain(pst_hal_device);
    /* 2.修订描述符控管理帧发送通道 */
    /* 初始化单播管理帧参数 */
    dmac_vap_init_tx_mgmt_rate(pst_dmac_vap, pst_dmac_vap->ast_tx_mgmt_ucast);

    /* 初始化组播、广播管理帧参数 */
    dmac_vap_init_tx_mgmt_rate(pst_dmac_vap, pst_dmac_vap->ast_tx_mgmt_bmcast);
}

#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
