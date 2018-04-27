


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"

#ifdef _PRE_WLAN_ALG_ENABLE
#include "alg_ext_if.h"
#endif

#include "dmac_ext_if.h"
#include "dmac_device.h"
#include "dmac_main.h"
#include "dmac_config.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_mgmt_classifier.h"
#include "dmac_mgmt_sta.h"
#include "dmac_rx_data.h"
#include "dmac_tx_complete.h"
#include "dmac_beacon.h"
#include "dmac_scan.h"
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "dmac_data_acq.h"
#endif
#include "dmac_chan_mgmt.h"
#include "dmac_blockack.h"
#include "dmac_scan.h"

#ifdef _PRE_WLAN_FEATURE_AP_PM
#include "dmac_ap_pm.h"
#endif

#include "frw_task.h"

#include "dmac_test_main.h"

#ifdef _PRE_WLAN_DFT_STAT
#include "dmac_dft.h"
#endif

#include "dmac_reset.h"

#include "dmac_rx_filter.h"

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
#include "dmac_auto_adjust_freq.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
#include "dmac_btcoex.h"
#endif

#include "dmac_hcc_adapt.h"
#include "mac_board.h"
#ifdef _PRE_WLAN_FEATURE_P2P
#include "dmac_p2p.h"
#endif

#include "dmac_dfx.h"


#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "oal_profiling.h"
#endif
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "pm_extern.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "dmac_ftm.h"
#endif

#ifdef _PRE_WLAN_11K_STAT
#include "dmac_stat.h"
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
#include "dmac_bsd.h"
#endif

#include "dmac_power.h"

#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_MAIN_C

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
//数采每次最多可上报数据长度，以32bit为单位
#define SAMPLE_DATA_MAX_RPT_LEN            (100)
#endif

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
oal_uint8 g_auc_ip_filter_btable[MAC_MAX_IP_FILTER_BTABLE_SIZE];  /* rx ip过滤功能的黑名单 */
#endif //_PRE_WLAN_FEATURE_IP_FILTER

#ifdef _PRE_WLAN_REALTIME_CALI
oal_uint16 g_us_dync_cali_num = 0;
#endif

#if (_PRE_PRODUCT_ID_HI1151 ==_PRE_PRODUCT_ID)
OAL_STATIC dmac_timeStamp_stru g_ast_dmac_timestamp[WLAN_FRW_MAX_NUM_CORES];
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_int32 dmac_hcc_adapt_init(oal_void);
#endif

#if defined(_PRE_WLAN_FEATURE_HILINK_TEMP_PROTECT) || defined(_PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT)
#define DMAC_PUBLIC_TIMER_PERIOD        1000
frw_timeout_stru                        g_st_dmac_pub_timer;             /* dmac一秒通用定时器 */
#endif

#ifdef _PRE_WLAN_MAC_ADDR_EDCA_FIX
extern oal_void hwifi_get_edca_fix_cfg(oal_void);
#endif

#if defined (_PRE_WLAN_FEATURE_RX_AGGR_EXTEND) || defined (_PRE_FEATURE_WAVEAPP_CLASSIFY)
extern oal_void hwifi_get_wavap_mac_cfg(oal_void);
#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)


oal_uint32 dmac_cfg_vap_init_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru        *pst_event;             /* 事件结构体 */
    frw_event_stru        *pst_rate_event;
    mac_device_stru       *pst_device;
    frw_event_mem_stru    *pst_event_mem_rate;

    //OAM_INFO_LOG0(0, OAM_SF_ANY, "{dmac_cfg_vap_init_event::func enter.}");

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_cfg_vap_init_event::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_cfg_vap_init_event::pst_device is null ,device_id:%d.}", pst_event->st_event_hdr.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pst_device->uc_cfg_vap_id != pst_event->st_event_hdr.uc_vap_id)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_cfg_vap_init_event::unexpected cfg vap id[%d] should be[%d].}",
                       pst_event->st_event_hdr.uc_vap_id, pst_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_ADD_VAP_INDX_UNSYNC;
    }

    /* 1102 需要在device初始化成功后同步速率级 */
    pst_event_mem_rate = FRW_EVENT_ALLOC(0);
	if (OAL_PTR_NULL == pst_event_mem_rate)
	{
		OAL_IO_PRINT("dmac_cfg_vap_init_event: FRW_EVENT_ALLOC result = OAL_PTR_NULL.\n");
		return OAL_FAIL;
	}

    pst_rate_event = frw_get_event_stru(pst_event_mem_rate);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_rate_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CRX,
                       DMAC_WLAN_CRX_EVENT_SUB_TYPE_INIT,
                       WLAN_MEM_EVENT_SIZE1,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_event->st_event_hdr.uc_chip_id,
                       pst_event->st_event_hdr.uc_device_id,
                       pst_event->st_event_hdr.uc_vap_id);

	if (OAL_SUCC != dmac_init_event_process(pst_event_mem_rate))
	{
		OAL_IO_PRINT("dmac_cfg_vap_init_event: dmac_init_event_process result = fale.\n");
		FRW_EVENT_FREE(pst_event_mem_rate);
		return OAL_FAIL;
	}

	FRW_EVENT_FREE(pst_event_mem_rate);

    return OAL_SUCC;
}
#endif


#if 0
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 dmac_init_event_create_cfg_vap(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru        *pst_event;             /* 事件结构体 */
    mac_device_stru       *pst_device;
    oal_uint8              uc_vap_idex = 0;
    dmac_vap_stru         *pst_dmac_vap;

    //OAM_INFO_LOG0(0, OAM_SF_ANY, "{dmac_init_event_create_cfg_vap::func enter.}");

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_init_event_create_cfg_vap::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_init_event_create_cfg_vap::pst_device is null ,device_id:%d.}", pst_event->st_event_hdr.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 分配dmac vap内存空间 */
    mac_res_alloc_dmac_vap(&uc_vap_idex);
    pst_device->uc_cfg_vap_id = uc_vap_idex;

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_device->uc_cfg_vap_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_init_event_create_cfg_vap::pst_dmac_vap is null ,vap_id:%d.}", pst_device->uc_cfg_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*VAP 初始清零 */
    OAL_MEMZERO(((oal_uint8 *)pst_dmac_vap), OAL_SIZEOF(dmac_vap_stru));

    /* 初始化cfg_vap信息 */
    pst_dmac_vap->st_vap_base_info.uc_chip_id     = pst_device->uc_chip_id;
    pst_dmac_vap->st_vap_base_info.uc_device_id   = pst_device->uc_device_id;
    pst_dmac_vap->st_vap_base_info.uc_vap_id      = pst_device->uc_cfg_vap_id;
    pst_dmac_vap->st_vap_base_info.en_vap_mode    = WLAN_VAP_MODE_CONFIG;

    /* 此时，dmac device初始化完成 */
    pst_device->en_device_state = OAL_TRUE;
    //OAM_INFO_LOG0(0, OAM_SF_ANY, "{dmac_init_event_create_cfg_vap::func out.}");

    return OAL_SUCC;
}
#endif
#endif //mac公共函数整改


#ifdef _PRE_WLAN_REALTIME_CALI

oal_uint32  dmac_rf_realtime_cali_timeout(oal_void * p_arg)
{
    mac_device_stru           *pst_mac_device = (mac_device_stru *)p_arg;
    hal_to_dmac_device_stru   *pst_hal_device;
    oal_uint32                 ul_status;
    oal_uint8                  uc_vap_idx;
    dmac_vap_stru             *pst_dmac_vap = OAL_PTR_NULL;
    mac_vap_stru              *pst_mac_vap = OAL_PTR_NULL;

    pst_dmac_vap = mac_res_get_dmac_vap(pst_mac_device->auc_vap_id[0]);
    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        return  OAL_SUCC;
    }

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device) )
    {
       OAM_ERROR_LOG0(0, OAM_SF_CALIBRATE,  "dmac_rf_realtime_cali_timeout:pst_hal_device PTR NULL!");
       return OAL_FAIL;
    }

    /* 动态校准500次后周期改为WLAN_REALTIME_CALI_INTERVAL */
    if (500 == g_us_dync_cali_num && OAL_TRUE == pst_hal_device->en_dync_txpower_flag)
    {
        FRW_TIMER_STOP_TIMER(&pst_mac_device->st_realtime_cali_timer);

        FRW_TIMER_CREATE_TIMER(&(pst_mac_device->st_realtime_cali_timer),
                                dmac_rf_realtime_cali_timeout,
                                WLAN_REALTIME_CALI_INTERVAL,
                                pst_mac_device,
                                OAL_TRUE,
                                OAM_MODULE_ID_DMAC,
                                pst_mac_device->ul_core_id);

        g_us_dync_cali_num++; /* 动态校准次数计数 */
    }
    else if (500 > g_us_dync_cali_num && OAL_TRUE == pst_hal_device->en_dync_txpower_flag)
    {
        g_us_dync_cali_num++; /* 动态校准次数计数 */
    }

    hal_get_wow_enable_status(pst_hal_device, &ul_status);
    if (0 != ul_status)
    {
       OAM_ERROR_LOG0(0, OAM_SF_CALIBRATE,  "dmac_rf_realtime_cali_timeout:cali in WOW mode");
       return OAL_FAIL;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);

        /* 查询当前工作vap进行动态校准 */
        if ((OAL_PTR_NULL != pst_mac_vap) &&
            (MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state) &&
            (pst_mac_vap->st_channel.uc_chan_number == pst_hal_device->st_wifi_channel_status.uc_chan_number))
        {
            /* 动态校准 */
            hal_rf_cali_realtime(pst_hal_device);

            /* 刷新发送功率 */
            dmac_pow_set_vap_tx_power(pst_mac_vap, HAL_POW_SET_TYPE_REFRESH);

            return OAL_SUCC;

        }
    }

    return OAL_FAIL;
}
#endif


#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32 dmac_init_event_process(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru        *pst_event;             /* 事件结构体 */
    mac_device_stru       *pst_mac_device;
    dmac_vap_stru         *pst_dmac_vap;
    dmac_tx_event_stru    *pst_dtx_event;
    oal_netbuf_stru       *pst_netbuf;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_init_event_process::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    pst_mac_device = mac_res_get_dev(pst_event->st_event_hdr.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_init_event_process::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_event->st_event_hdr.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_init_event_process::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* p2p时，有可能下发两次事件 */
    if (FRW_INIT_STATE_ALL_SUCC == frw_get_init_state())
    {
        return OAL_SUCC;
    }

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CRX,
                       DMAC_WLAN_CRX_EVENT_SUB_TYPE_INIT,
                       WLAN_MEM_EVENT_SIZE1,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       0,0,0);

    /* 申请netbuf，存放速率级信息 */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, sizeof(mac_data_rate_stru) * MAC_DATARATES_PHY_80211G_NUM, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_init_event_process::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*************************************************************************/
    /*            netbuf data域的上报的data rate                */
    /* --------------------------------------------------------------------  */
    /*              | st_mac_rates_11g info     |                            */
    /* --------------------------------------------------------------------  */
    /*              | sizeof(mac_data_rate_stru) |                            */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /* 清空cb字段 */
    OAL_MEMZERO(oal_netbuf_cb(pst_netbuf), OAL_TX_CB_LEN);

    /* 将速率集的信息拷贝到netbuf中，上报hmac */
    oal_memcopy((oal_uint8 *)((oal_uint8 *)(OAL_NETBUF_DATA(pst_netbuf))),
                (oal_uint8 *)(pst_mac_device->st_mac_rates_11g),
                sizeof(mac_data_rate_stru) * MAC_DATARATES_PHY_80211G_NUM);

    /* 业务事件信息 */
    pst_dtx_event               = (dmac_tx_event_stru *)pst_event->auc_event_data;
    pst_dtx_event->pst_netbuf   = pst_netbuf;
    pst_dtx_event->us_frame_len = sizeof(mac_data_rate_stru) * MAC_DATARATES_PHY_80211G_NUM;

    OAL_IO_PRINT("chip0, chip_ver[0x%x]\n", g_pst_mac_board->ast_chip[0].ul_chip_ver);
    OAL_IO_PRINT("pst_dtx_event->us_frame_len[%u]\n", pst_dtx_event->us_frame_len);

    frw_event_dispatch_event(pst_event_mem);

#ifdef _PRE_WLAN_FEATURE_STA_PM
#if (_PRE_WLAN_CHIP_ASIC != _PRE_WLAN_CHIP_VERSION)
    g_us_PmWifiSleepRfPwrOn = WLAN_NOT_SLEEP;
#endif
#endif

    frw_set_init_state(FRW_INIT_STATE_ALL_SUCC);

    return OAL_SUCC;
}
#else

OAL_STATIC oal_uint32  dmac_init_hardware(mac_board_stru *pst_board)
{
    oal_uint8                  uc_chip_idx;
    oal_uint8                  uc_device;
    oal_uint8                  uc_device_max;
    mac_device_stru           *pst_dev = OAL_PTR_NULL;
    hal_to_dmac_device_stru   *pst_hal_device;
    oal_uint8                  uc_chip_id_bitmap = pst_board->uc_chip_id_bitmap;
    oal_uint32                 ul_ret;

    while (0 != uc_chip_id_bitmap)
    {
        /* 获取最右边一位为1的位数，此值即为chip的数组下标 */
        uc_chip_idx = oal_bit_find_first_bit_one_byte(uc_chip_id_bitmap);
        if (OAL_UNLIKELY(uc_chip_idx >= WLAN_CHIP_MAX_NUM_PER_BOARD))
        {
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_init_hardware::invalid uc_chip_idx[%d] uc_chip_id_bitmap=%d.}",
                           uc_chip_idx, pst_board->uc_chip_id_bitmap);

            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        /* HAL接口获取支持device个数 */
        ul_ret = hal_chip_get_device_num(uc_chip_idx, &uc_device_max);

        if(uc_device_max > WLAN_DEVICE_MAX_NUM_PER_CHIP)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_init_hardware::array overflow,uc_device count[%d] is exceeded.}",
                uc_device_max);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        for (uc_device = 0; uc_device < uc_device_max; uc_device++)
        {
            ul_ret = hal_chip_get_hal_device(0, uc_device, &pst_hal_device);
            if (OAL_SUCC != ul_ret || OAL_PTR_NULL == pst_hal_device)
            {
                continue;
            }

            pst_dev = mac_res_get_dev(pst_hal_device->uc_mac_device_id);
            if (OAL_PTR_NULL == pst_dev)
            {
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "dmac_init_hardware::pst_dev NULL");
                continue;
            }

            /* 使能PA和PHY的工作 */
            hal_enable_machw_phy_and_pa(pst_hal_device);
#ifdef _PRE_WLAN_REALTIME_CALI
            if ((OAL_TRUE == pst_hal_device->en_dync_txpower_flag) && (pst_dev != OAL_PTR_NULL))
            {
                FRW_TIMER_CREATE_TIMER(&(pst_dev->st_realtime_cali_timer),
                                        dmac_rf_realtime_cali_timeout,
                                        WLAN_REALTIME_CALI_INTERVAL,
                                        pst_dev,
                                        OAL_TRUE,
                                        OAM_MODULE_ID_DMAC,
                                        pst_dev->ul_core_id);
            }
#endif


        }
        /* 清除对应的bitmap位 */
        oal_bit_clear_bit_one_byte(&uc_chip_id_bitmap, uc_chip_idx);
    }

    return OAL_SUCC;
}


oal_uint32 dmac_init_event_process(frw_event_mem_stru *pst_event_mem)
{
    oal_uint32             ul_relt;
    frw_event_stru        *pst_event;             /* 事件结构体 */

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_init_event_process::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event = frw_get_event_stru(pst_event_mem);

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                       FRW_EVENT_TYPE_WLAN_CRX,
                       DMAC_WLAN_CRX_EVENT_SUB_TYPE_INIT,
                       WLAN_MEM_EVENT_SIZE1,
                       FRW_EVENT_PIPELINE_STAGE_1,
                       0,0,0);

    frw_event_dispatch_event(pst_event_mem);

    ul_relt = dmac_init_hardware(g_pst_mac_board);

    return ul_relt;
}
#endif
#ifdef _PRE_WLAN_FEATURE_DATA_SAMPLE

OAL_STATIC oal_uint32  dmac_sdt_recv_sample_cmd(frw_event_mem_stru *pst_event_mem)
{
    oal_uint32                   ul_count;
    oal_uint32                   ul_reg_num;
    oal_uint32                   ul_unreport_len;
    oal_uint16                   us_report_regs;
    oal_uint16                   us_finish_state;
    oal_uint16                   us_payload_len;
    frw_event_stru              *pst_event_down;
    frw_event_stru              *pst_event_up;
    frw_event_mem_stru          *pst_event_memory;
    dmac_sdt_sample_frame_stru  *pst_sample_frame;
    dmac_sdt_sample_frame_stru  *pst_sample_frame_up;
    dmac_vap_stru               *pst_dmac_vap;
    OAL_STATIC oal_uint32       *pul_start_addr = OAL_PTR_NULL;

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_event_down     = frw_get_event_stru(pst_event_mem);
    pst_sample_frame = (dmac_sdt_sample_frame_stru *)pst_event_down->auc_event_data;
    pst_dmac_vap = mac_res_get_dmac_vap(pst_event_down->st_event_hdr.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_event_down->st_event_hdr.uc_vap_id, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    ul_reg_num = pst_sample_frame->ul_reg_num;

    /*sdt下发消息ul_count为0时准备数采内存
     否则表示读取数采数据,ul_count为读取数据的起始序号*/
    if (0 == pst_sample_frame->ul_count)
    {
        hal_set_sample_memory(pst_dmac_vap->pst_hal_device, &pul_start_addr, &ul_reg_num);
        if (OAL_PTR_NULL == pul_start_addr)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::set sample memory fail!}");
            return OAL_FAIL;
        }

        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::set sample memory succ, start_addr[0x%x], reg num [%d].!}", pul_start_addr, ul_reg_num);

        pst_event_memory = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_sdt_sample_frame_stru));
        if (OAL_PTR_NULL == pst_event_memory)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::set sample memory fail!}");
            return OAL_ERR_CODE_PTR_NULL;
        }
        pst_event_up = frw_get_event_stru(pst_event_memory);
        pst_sample_frame_up = (dmac_sdt_sample_frame_stru *)pst_event_up->auc_event_data;
        /* 填写事件头 */
        FRW_EVENT_HDR_INIT(&(pst_event_up->st_event_hdr),
                           FRW_EVENT_TYPE_HOST_SDT_REG,
                           DMAC_TO_HMAC_SYN_UP_SAMPLE_DATA,
                           OAL_SIZEOF(dmac_sdt_sample_frame_stru),
                           FRW_EVENT_PIPELINE_STAGE_1,
                           pst_event_down->st_event_hdr.uc_chip_id,
                           pst_event_down->st_event_hdr.uc_device_id,
                           pst_event_down->st_event_hdr.uc_vap_id);
        pst_sample_frame_up->ul_reg_num =  ul_reg_num;
        pst_sample_frame_up->ul_type = DMAC_RX_SAMPLE;
        pst_sample_frame_up->ul_count = 0;

        frw_event_dispatch_event(pst_event_memory);
        FRW_EVENT_FREE(pst_event_memory);
        return OAL_SUCC;
    }
    else if(pst_sample_frame->ul_count <= ul_reg_num)
    {
        hal_get_sample_state(pst_dmac_vap->pst_hal_device, &us_finish_state);
        /*packetram 设置为总线访问*/
        hal_set_pktmem_bus_access(pst_dmac_vap->pst_hal_device);

        if (us_finish_state)
        {
            /*剩余未上报数据不少于最大可上报长度按最大长度上报，否则上报剩余全部数据*/
            /*长度以32bit为单位*/
            ul_count = pst_sample_frame->ul_count;
            ul_unreport_len = (ul_reg_num - ul_count) + 1;
            us_report_regs = ul_unreport_len >= SAMPLE_DATA_MAX_RPT_LEN ? SAMPLE_DATA_MAX_RPT_LEN : (oal_uint16)ul_unreport_len;

            /*dmac_sdt_sample_frame_stru内包含一个4字节的空间用于存放数据*/
            us_payload_len = (oal_uint16)((us_report_regs - 1)*OAL_SIZEOF(oal_uint32) + OAL_SIZEOF(dmac_sdt_sample_frame_stru));
            pst_event_memory = FRW_EVENT_ALLOC(us_payload_len);
            if (OAL_PTR_NULL == pst_event_memory)
            {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::pst_event_memory null！us_payload_len = %d}", us_payload_len);

                return OAL_ERR_CODE_PTR_NULL;
            }
            pst_event_up = frw_get_event_stru(pst_event_memory);
            pst_sample_frame_up = (dmac_sdt_sample_frame_stru *)pst_event_up->auc_event_data;

            /* 填写事件头 */
            FRW_EVENT_HDR_INIT(&(pst_event_up->st_event_hdr),
                                       FRW_EVENT_TYPE_HOST_SDT_REG,
                                       DMAC_TO_HMAC_SYN_UP_SAMPLE_DATA,
                                       us_payload_len,
                                       FRW_EVENT_PIPELINE_STAGE_1,
                                       pst_event_down->st_event_hdr.uc_chip_id,
                                       pst_event_down->st_event_hdr.uc_device_id,
                                       pst_event_down->st_event_hdr.uc_vap_id);
            pst_sample_frame_up->ul_reg_num = ul_reg_num;
            pst_sample_frame_up->ul_type = DMAC_RX_SAMPLE;
            /*ul_count从1开始计数，计算读取数据起始地址时需-1*/

            /*lint -save -e669 */
            oal_memcopy(&pst_sample_frame_up->ul_event_data, pul_start_addr + ul_count - 1, us_report_regs * OAL_SIZEOF(oal_uint32));
            /*lint -restore */

            /*把本次上报的长度更新到ul_count*/
            ul_count += us_report_regs;
            pst_sample_frame_up->ul_count = ul_count;
            frw_event_dispatch_event(pst_event_memory);
            FRW_EVENT_FREE(pst_event_memory);

            /*如果数采数据全部上报完成则释放数采内存*/
            if (ul_count > ul_reg_num)
            {
                hal_free_sample_mem(pst_dmac_vap->pst_hal_device);
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::Sample data report done!!!}");
            }
        }
        else
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::sample is not done.}");
            hal_free_sample_mem(pst_dmac_vap->pst_hal_device);
            return OAL_FAIL;
        }
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::ul_count = %d is invalid!}", pst_sample_frame->ul_count);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_RF_AUTOCALI
OAL_STATIC oal_uint32  dmac_sdt_recv_autocali_cmd(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                  *pst_event_down;
    dmac_sdt_autocali_frame_stru    *pst_autocali_frame;

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAL_IO_PRINT("dmac_sdt_recv_autocali_cmd::pst_event_mem null.\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_event_down     = frw_get_event_stru(pst_event_mem);
    pst_autocali_frame = (dmac_sdt_autocali_frame_stru *)pst_event_down->auc_event_data;

    if (AUTOCALI_SWITCH == pst_autocali_frame->ul_type)
    {
        hal_rf_cali_auto_switch(pst_autocali_frame->auc_msg_data[0]);
    }
    else if(AUTOCALI_ACK == pst_autocali_frame->ul_type)
    {
        hal_rf_cali_auto_mea_done(pst_autocali_frame->auc_msg_data[0],
                                  pst_autocali_frame->auc_msg_data[1],
                                  pst_autocali_frame->auc_msg_data[2],
                                  pst_autocali_frame->auc_msg_data[3]);
    }
    else
    {
        OAL_IO_PRINT("dmac_sdt_recv_autocali_cmd::msg type error.\r\n");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


oal_uint32 dmac_autocali_to_hmac(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_event_hdr = &(pst_event->st_event_hdr);

    FRW_EVENT_HDR_MODIFY_PIPELINE_AND_TYPE_AND_SUBTYPE(pst_event_hdr, FRW_EVENT_TYPE_HOST_SDT_REG,DMAC_TO_HMAC_AUTOCALI_DATA);
    /* 分发事件 */
    frw_event_dispatch_event(pst_event_mem);

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS

OAL_STATIC oal_uint32  dmac_sdt_recv_psd_cmd(frw_event_mem_stru *pst_event_mem)
{
    oal_uint32                   ul_count;
    oal_uint32                   ul_reg_num;
    oal_uint32                   ul_unreport_len;
    oal_uint16                   us_report_regs;
    oal_uint16                   us_payload_len;
    frw_event_stru              *pst_event_down;
    frw_event_stru              *pst_event_up;
    frw_event_mem_stru          *pst_event_memory;
    dmac_sdt_sample_frame_stru  *pst_sample_frame;
    dmac_sdt_sample_frame_stru  *pst_sample_frame_up;
    dmac_vap_stru               *pst_dmac_vap;
    hal_to_dmac_device_stru     *pst_hal_device;
    OAL_STATIC oal_uint32       *pul_start_addr = OAL_PTR_NULL;

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_psd_cmd::pst_event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event_down = frw_get_event_stru(pst_event_mem);
    pst_dmac_vap  = mac_res_get_dmac_vap(pst_event_down->st_event_hdr.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CFG, "{dmac_sdt_recv_psd_cmd::vap id [%d] but dmac vap is null.}",pst_event_down->st_event_hdr.uc_vap_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_dmac_vap);
    pst_sample_frame = (dmac_sdt_sample_frame_stru *)pst_event_down->auc_event_data;
    ul_reg_num = pst_sample_frame->ul_reg_num;

    /* sdt下发消息ul_count为0时准备PSD 内存
     否则表示读PSD数据,ul_count为读取数据的起始序号 */
    if (0 == pst_sample_frame->ul_count)
    {
        hal_set_psd_memory(pst_hal_device, &pul_start_addr);
        if (OAL_PTR_NULL == pul_start_addr)
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_psd_cmd::set sample memory fail!}");
            return OAL_FAIL;
        }
        /* PSD准备 */

#if 0
        hal_set_psd_nb_det_en(pst_hal_device, 1);
        hal_set_psd_11b_det_en(pst_hal_device, 1);
        hal_set_psd_ofdm_det_en(pst_hal_device, 1);
        /* 1:业务场景，0: 非业务场景 */
        hal_set_psd_wifi_work_en(pst_hal_device, 1);
        hal_set_force_reg_clk_on(pst_hal_device, 1);
        hal_set_sync_data_path_div_num(pst_hal_device, 0);
        hal_set_psd_the_num_nb(pst_hal_device, 7);
        hal_set_psd_the_power_nb(pst_hal_device, 6);
        hal_set_psd_the_rssi_nb(pst_hal_device, -70);
        hal_set_psd_the_bottom_noise(pst_hal_device, -80);
        hal_set_up_fft_psd_en(pst_hal_device, 1);
#endif
        /* 最后配set_psd_en,使能PSD上报 */
        hal_set_psd_en(pst_hal_device, 1);

        pst_event_memory = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_sdt_sample_frame_stru));
        if (OAL_PTR_NULL == pst_event_memory)
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_event_up = frw_get_event_stru(pst_event_memory);
        pst_sample_frame_up = (dmac_sdt_sample_frame_stru *)pst_event_up->auc_event_data;
        pst_hal_device->uc_psd_status = OAL_FALSE;
        /* 填写事件头 */
        FRW_EVENT_HDR_INIT(&(pst_event_up->st_event_hdr),
                           FRW_EVENT_TYPE_HOST_SDT_REG,
                           DMAC_TO_HMAC_SYN_UP_SAMPLE_DATA,
                           OAL_SIZEOF(dmac_sdt_sample_frame_stru),
                           FRW_EVENT_PIPELINE_STAGE_1,
                           pst_event_down->st_event_hdr.uc_chip_id,
                           pst_event_down->st_event_hdr.uc_device_id,
                           pst_event_down->st_event_hdr.uc_vap_id);
        pst_sample_frame_up->ul_reg_num =  ul_reg_num;
        pst_sample_frame_up->ul_type = DMAC_RX_PSD;
        pst_sample_frame_up->ul_count = 0;

        frw_event_dispatch_event(pst_event_memory);
        FRW_EVENT_FREE(pst_event_memory);
        return OAL_SUCC;
    }
    else if(pst_sample_frame->ul_count <= ul_reg_num)
    {
        if (pst_hal_device->uc_psd_status)
        {
            /*剩余未上报数据不少于最大可上报长度按最大长度上报，否则上报剩余全部数据*/
            /*长度以32bit为单位*/
            ul_count = pst_sample_frame->ul_count;
            ul_unreport_len = (ul_reg_num - ul_count) + 1;
            us_report_regs = ul_unreport_len >= SAMPLE_DATA_MAX_RPT_LEN ? SAMPLE_DATA_MAX_RPT_LEN : (oal_uint16)ul_unreport_len;

            /*dmac_sdt_psd_frame_stru内包含一个4字节的空间用于存放数据*/
            us_payload_len = (oal_uint16)((us_report_regs - 1)*OAL_SIZEOF(oal_uint32) + OAL_SIZEOF(dmac_sdt_sample_frame_stru));
            pst_event_memory = FRW_EVENT_ALLOC(us_payload_len);
            if (OAL_PTR_NULL == pst_event_memory)
            {
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_sdt_recv_sample_cmd::pst_event_memory null！us_payload_len = %d}", us_payload_len);

                return OAL_ERR_CODE_PTR_NULL;
            }
            pst_event_up = frw_get_event_stru(pst_event_memory);
            pst_sample_frame_up = (dmac_sdt_sample_frame_stru *)pst_event_up->auc_event_data;

            /* 填写事件头 */
            FRW_EVENT_HDR_INIT(&(pst_event_up->st_event_hdr),
                                       FRW_EVENT_TYPE_HOST_SDT_REG,
                                       DMAC_TO_HMAC_SYN_UP_SAMPLE_DATA,
                                       us_payload_len,
                                       FRW_EVENT_PIPELINE_STAGE_1,
                                       pst_event_down->st_event_hdr.uc_chip_id,
                                       pst_event_down->st_event_hdr.uc_device_id,
                                       pst_event_down->st_event_hdr.uc_vap_id);
            pst_sample_frame_up->ul_reg_num = ul_reg_num;
            pst_sample_frame_up->ul_type = DMAC_RX_PSD;
            /*ul_count从1开始计数，计算读取数据起始地址时需-1*/
            oal_memcopy(&pst_sample_frame_up->ul_event_data, pul_start_addr + ul_count - 1, us_report_regs*OAL_SIZEOF(oal_uint32));

            /*把本次上报的长度更新到ul_count*/
            ul_count += us_report_regs;
            pst_sample_frame_up->ul_count = ul_count;
            frw_event_dispatch_event(pst_event_memory);
            FRW_EVENT_FREE(pst_event_memory);

            /*如果数采数据全部上报完成则释放数采内存*/
            if (ul_count == ul_reg_num)
            {
                hal_free_psd_mem(pst_hal_device);
            }
        }
        else
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_psd_cmd::sample is not done.}");
            return OAL_FAIL;
        }
    }
    else
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_sdt_recv_psd_cmd::ul_count = %d is invalid!}", pst_sample_frame->ul_count);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


oal_uint32  dmac_psd_data_handler(frw_event_mem_stru *pst_event_mem)
{
    hal_common_irq_event_stru            *pst_common_irq_event;
    frw_event_stru                       *pst_event;
    hal_to_dmac_device_stru              *pst_hal_device;
    //oal_uint8                            *puc_netbuf;
    wlan_channel_bandwidth_enum_uint8     en_bandwidth;
    pst_event  = frw_get_event_stru(pst_event_mem);
    pst_common_irq_event       = (hal_common_irq_event_stru *)(pst_event->auc_event_data);
    pst_hal_device             = pst_common_irq_event->pst_hal_device;

    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_psd_data_handler::pst_hal_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    //dmac_alg_cfg_intf_det_psd_notify(pst_hal_device);
#if 0
    /* netbuf的申请在sdt下发的命令中，此处不需要返回Error打印 */
    if (OAL_PTR_NULL == pst_hal_device->pst_psd_netbuf)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_psd_data_handler::pst_psd_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    puc_netbuf = (oal_uint8 *)oal_netbuf_data(pst_hal_device->pst_psd_netbuf);
    if (OAL_PTR_NULL == puc_netbuf)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_psd_data_handler::net_buf data null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
#endif

    en_bandwidth = pst_hal_device->st_wifi_channel_status.en_bandwidth;
    /* 获取PSD结果 */
    hal_get_psd_info(pst_hal_device);
    hal_get_psd_data(pst_hal_device, en_bandwidth);
    /* PSD中断处理时使能位写1 */
    hal_set_psd_en(pst_hal_device, 0);


    return OAL_SUCC;
}
#endif
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)

OAL_STATIC oal_uint32  dmac_sdt_recv_cmd(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                  *pst_event_down;
    dmac_sdt_sample_frame_stru      *pst_sample_frame;
    oal_uint32                       ul_ret = OAL_SUCC;

    pst_event_down = frw_get_event_stru(pst_event_mem);
    pst_sample_frame = (dmac_sdt_sample_frame_stru *)pst_event_down->auc_event_data;

    if (DMAC_RX_SAMPLE == pst_sample_frame->ul_type)
    {
#ifdef _PRE_WLAN_FEATURE_DATA_SAMPLE
        ul_ret = dmac_sdt_recv_sample_cmd(pst_event_mem);
#endif
    }
    else if (DMAC_RX_PSD == pst_sample_frame->ul_type)
    {
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
        ul_ret = dmac_sdt_recv_psd_cmd(pst_event_mem);
#endif
    }
    else
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_sdt_recv_cmd:: sdt send unknow type [%d].}", pst_sample_frame->ul_type);
    }

    return ul_ret;
}
#endif

#if (_PRE_PRODUCT_ID_HI1151 ==_PRE_PRODUCT_ID)

OAL_STATIC oal_uint32 dmac_timestamp_update_timer(oal_void* p_NULL)
{
    oal_uint32 ul_timestamp = 0;   /* 获取硬时钟计数 */
    oal_uint32 ul_offset_ns = 0;   /* 时间差 */
    oal_uint32 ul_usec      = 0;   /* us 计数 */
    oal_uint32 ul_core_id;

    ul_core_id = OAL_GET_CORE_ID();

    ul_timestamp = oal_5115timer_get_10ns();
    ul_offset_ns = g_ast_dmac_timestamp[ul_core_id].ul_last_timestamp - ul_timestamp;
    ul_usec = (oal_uint32)g_ast_dmac_timestamp[ul_core_id].st_timestamp_us.i_usec + ul_offset_ns/100; /*微妙us*/

    /*更新秒*/
    while(ul_usec >= 1000000)
    {
        ul_usec -= 1000000;
        g_ast_dmac_timestamp[ul_core_id].st_timestamp_us.i_sec++;
    }
    /*更新微秒*/
    g_ast_dmac_timestamp[ul_core_id].st_timestamp_us.i_usec = (oal_int32)ul_usec;

    g_ast_dmac_timestamp[ul_core_id].ul_last_timestamp = ul_timestamp;

    return OAL_TRUE;
}

oal_void dmac_timestamp_get(oal_time_us_stru *pst_usec)
{
    oal_uint32  ul_core_id;
    ul_core_id = OAL_GET_CORE_ID();

    dmac_timestamp_update_timer(OAL_PTR_NULL);
    pst_usec->i_sec  = g_ast_dmac_timestamp[ul_core_id].st_timestamp_us.i_sec;
    pst_usec->i_usec = g_ast_dmac_timestamp[ul_core_id].st_timestamp_us.i_usec;
}

#endif


oal_void dmac_timestamp_init(oal_void)
{
#if (_PRE_PRODUCT_ID_HI1151 ==_PRE_PRODUCT_ID)
    oal_uint32  ul_core_id;

    OAL_MEMZERO((void *)g_ast_dmac_timestamp, OAL_SIZEOF(g_ast_dmac_timestamp));
    for(ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++)
    {
        if (OAL_FALSE == g_ast_dmac_timestamp[ul_core_id].st_timer.en_is_registerd)
        {
            oal_time_get_stamp_us(&g_ast_dmac_timestamp[ul_core_id].st_timestamp_us);
            g_ast_dmac_timestamp[ul_core_id].ul_last_timestamp = oal_5115timer_get_10ns();

            FRW_TIMER_CREATE_TIMER(&g_ast_dmac_timestamp[ul_core_id].st_timer,
                                   dmac_timestamp_update_timer,
                                   1000,               /* 1000ms触发一次 */
                                   (oal_void*)OAL_PTR_NULL,
                                   OAL_TRUE,
                                   OAM_MODULE_ID_DMAC,
                                   ul_core_id);
        }
    }
#endif
}


oal_void dmac_timestamp_exit(oal_void)
{
#if (_PRE_PRODUCT_ID_HI1151 ==_PRE_PRODUCT_ID)
    oal_uint32      ul_core_id;

    for(ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++)
    {
        if (OAL_TRUE== g_ast_dmac_timestamp[ul_core_id].st_timer.en_is_registerd)
        {
    	    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&g_ast_dmac_timestamp[ul_core_id].st_timer);
        }
    }
#endif
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_void dmac_event_fsm_tx_adapt_subtable_register(oal_void)
{
    /* 注册WLAN_DRX事件子表 */
    g_ast_hmac_wlan_drx_event_sub_table[DMAC_WLAN_DRX_EVENT_SUB_TYPE_RX_DATA].p_tx_adapt_func = dmac_proc_wlan_drx_event_tx_adapt_ram;
    g_ast_hmac_wlan_drx_event_sub_table[DMAC_WLAN_DRX_EVENT_SUB_TYPE_TKIP_MIC_FAILE].p_tx_adapt_func  = dmac_proc_tkip_mic_fail_tx_adapt;

    /* 注册TBTT事件子表 */
    g_ast_hmac_tbtt_event_sub_table[DMAC_TBTT_EVENT_SUB_TYPE].p_tx_adapt_func = dmac_hcc_tx_convert_event_to_netbuf_uint16;

    /* 注册MISC事件子表 */
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_RADAR_DETECT].p_tx_adapt_func = dmac_hcc_tx_convert_event_to_netbuf_uint16;
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_DISASOC].p_tx_adapt_func      = dmac_proc_disasoc_misc_event_tx_adapt;
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_CALI_TO_HMAC].p_tx_adapt_func = dmac_cali2hmac_misc_event_tx_adapt;

#ifdef _PRE_WLAN_ONLINE_DPD
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_TO_HMAC_DPD].p_tx_adapt_func = dmac_dpd_tx_adapt;
#endif

#ifdef _PRE_WLAN_FEATURE_ROAM
    g_ast_hmac_wlan_misc_event_sub_table[DMAC_MISC_SUB_TYPE_ROAM_TRIGGER].p_tx_adapt_func = dmac_proc_roam_trigger_event_tx_adapt;
#endif //_PRE_WLAN_FEATURE_ROAM

    /* 注册WLAN_CRX事件子表 */
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_INIT].p_tx_adapt_func = dmac_proc_init_event_process_tx_adapt;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_RX].p_tx_adapt_func = dmac_proc_crx_event_tx_adapt;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_DELBA].p_tx_adapt_func = dmac_proc_mgmt_rx_delba_event_tx_adapt;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPR_CH_SWITCH_COMPLETE].p_tx_adapt_func = dmac_switch_to_new_chan_complete_tx_adapt;
#ifdef _PRE_WLAN_FEATURE_DBAC
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPR_DBAC].p_tx_adapt_func = dmac_dbac_status_notify_tx_adapt;
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_EVERY_SCAN_RESULT].p_tx_adapt_func = dmac_scan_report_scanned_bss_tx_adapt;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_DISASSOC].p_tx_adapt_func = dmac_rx_send_event_tx_adapt;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_DEAUTH].p_tx_adapt_func   = dmac_rx_send_event_tx_adapt;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_ALG_INFO_SYN].p_tx_adapt_func = dmac_alg_syn_info_adapt;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_VOICE_AGGR].p_tx_adapt_func   = dmac_alg_voice_aggr_adapt;
#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_OBSS_SCAN_COMP].p_tx_adapt_func = dmac_scan_proc_obss_scan_comp_event_tx_adapt;
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_FLOWCTL
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_FLOWCTL_BACKP].p_tx_adapt_func = dmac_alg_flowctl_backp_tx_adapt;
#endif

    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_SCAN_COMP].p_tx_adapt_func = dmac_scan_proc_scan_comp_event_tx_adapt;
    g_ast_hmac_wlan_crx_event_sub_table[DMAC_WLAN_CRX_EVENT_SUB_TYPE_CHAN_RESULT].p_tx_adapt_func = dmac_proc_chan_result_event_tx_adapt;

    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_DEL_BA].p_tx_adapt_func = dmac_proc_event_del_ba_tx_adapt;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_SYN_CFG].p_tx_adapt_func = dmac_proc_event_config_syn_tx_adapt;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_SYN_UP_REG_VAL].p_tx_adapt_func = dmac_sdt_recv_reg_cmd_tx_adapt;
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_SYN_UP_SAMPLE_DATA].p_tx_adapt_func = dmac_sdt_recv_sample_cmd_tx_adapt;
#endif
#ifdef _PRE_WLAN_RF_AUTOCALI
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_AUTOCALI_DATA].p_tx_adapt_func = dmac_sdt_recv_sample_cmd_tx_adapt;
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_M2S_DATA].p_tx_adapt_func = dmac_m2s_sync_event_adapt;
#endif

#ifdef _PRE_WLAN_CHIP_TEST_ALG
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_ALG_TEST].p_tx_adapt_func = dmac_alg_ct_result_tx_adapt;
#endif
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_BANDWIDTH_INFO_SYN].p_tx_adapt_func = dmac_chan_sync_event_adapt;

    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_PROTECTION_INFO_SYN].p_tx_adapt_func = dmac_chan_protection_event_adapt;
    g_ast_hmac_wlan_ctx_event_sub_table[DMAC_TO_HMAC_CH_STATUS_INFO_SYN].p_tx_adapt_func = dmac_ch_status_event_adapt;
}


OAL_STATIC oal_void dmac_event_fsm_rx_adapt_subtable_register(oal_void)
{
    /*default reset to dmac_hcc_rx_convert_netbuf_to_event_default*/
    frw_event_sub_rx_adapt_table_init(g_ast_dmac_host_crx_table,
                                        OAL_SIZEOF(g_ast_dmac_host_crx_table)/OAL_SIZEOF(frw_event_sub_table_item_stru),
                                        dmac_hcc_rx_convert_netbuf_to_event_default);

    frw_event_sub_rx_adapt_table_init(g_ast_dmac_wlan_ctx_event_sub_table,
                                        OAL_SIZEOF(g_ast_dmac_wlan_ctx_event_sub_table)/OAL_SIZEOF(frw_event_sub_table_item_stru),
                                        dmac_hcc_rx_convert_netbuf_to_event_default);


    /* 注册DMAC模块HOST_DRX事件处理函数表 */
    g_ast_dmac_tx_host_drx[DMAC_TX_HOST_DRX].p_rx_adapt_func = dmac_process_rx_data_event_adapt_default;

    /* 注册DMAC模块WLAN_CTX事件处理函数表 */
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT].p_rx_adapt_func          = dmac_process_rx_data_event_adapt_default;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ].p_rx_adapt_func = dmac_scan_proc_scan_req_event_rx_adapt;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_HMAC2DMAC].p_rx_adapt_func = dmac_recv_event_netbuf_rx_adapt;

    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_APP_IE_H2D].p_rx_adapt_func = dmac_recv_event_netbuf_rx_adapt;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_DPD_HMAC2DMAC].p_rx_adapt_func = dmac_recv_event_netbuf_rx_adapt;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCHED_SCAN_REQ].p_rx_adapt_func = dmac_scan_proc_sched_scan_req_event_rx_adapt;
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_IP_FILTER].p_rx_adapt_func = dmac_recv_event_netbuf_rx_adapt;
#endif //_PRE_WLAN_FEATURE_IP_FILTER

    /* 注册DMAC模块WLAN_DTX事件 */
}
#endif


OAL_STATIC oal_void  dmac_event_fsm_action_subtable_register(oal_void)
{
    /* 注册DMAC模块HOST_CRX事件 */
    g_ast_dmac_host_crx_table[HMAC_TO_DMAC_SYN_INIT].p_func = dmac_init_event_process;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

#if 0
    /* OFFLOAD情况下用与配置vap同步 */
    g_ast_dmac_host_crx_table[HMAC_TO_DMAC_SYN_CREATE_CFG_VAP].p_func = dmac_init_event_create_cfg_vap;
#else
    g_ast_dmac_host_crx_table[HMAC_TO_DMAC_SYN_CREATE_CFG_VAP].p_func = dmac_cfg_vap_init_event;
#endif

#endif

    g_ast_dmac_host_crx_table[HMAC_TO_DMAC_SYN_CFG].p_func = dmac_event_config_syn;
    g_ast_dmac_host_crx_table[HMAC_TO_DMAC_SYN_REG].p_func = dmac_sdt_recv_reg_cmd;
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    g_ast_dmac_host_crx_table[HMAC_TO_DMAC_SYN_SAMPLE].p_func = dmac_sdt_recv_cmd;
#endif
#ifdef _PRE_WLAN_RF_AUTOCALI
    g_ast_dmac_host_crx_table[HMAC_TO_DMAC_AUTOCALI_CMD].p_func = dmac_sdt_recv_autocali_cmd;
#endif

    /* 注册DMAC模块HOST_CRX事件 */
    g_ast_dmac_tx_host_drx[DMAC_TX_HOST_DRX].p_func = dmac_tx_process_data_event;

    /* 注册DMAC模块WLAN_DTX事件处理函数表 */
    g_ast_dmac_tx_wlan_dtx[DMAC_TX_WLAN_DTX].p_func = dmac_tx_process_data_event;

    /* 注册DMAC模块WLAN_CTX事件处理函数表 */
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_MGMT].p_func                   = dmac_tx_process_mgmt_event;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_ADD_USER].p_func               = dmac_user_add;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_NOTIFY_ALG_ADD_USER].p_func    = dmac_user_add_notify_alg;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_DEL_USER].p_func               = dmac_user_del;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_BA_SYNC].p_func                = dmac_rx_process_sync_event;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_PRIV_REQ].p_func               = dmac_rx_process_priv_req_event;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ].p_func               = dmac_scan_proc_scan_req_event;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCHED_SCAN_REQ].p_func         = dmac_scan_proc_sched_scan_req_event;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_RESET_PSM].p_func              = dmac_psm_reset;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_SET_REG].p_func           = dmac_join_set_reg_event_process;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_JOIN_DTIM_TSF_REG].p_func      = dmac_join_set_dtim_reg_event_process;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_ASOC_WRITE_REG].p_func         = dmac_asoc_set_reg_event_process;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_STA_SET_EDCA_REG].p_func       = dmac_mgmt_wmm_update_edca_machw_sta;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CONN_RESULT].p_func            = dmac_mgmt_conn_result_event;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_HMAC2DMAC].p_func         = dmac_cali_hmac2dmac_recv;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_APP_IE_H2D].p_func             = dmac_app_ie_h2d_recv;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_SWITCH_TO_NEW_CHAN].p_func     = dmac_chan_initiate_switch_to_new_channel;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WALN_CTX_EVENT_SUB_TYPR_SELECT_CHAN].p_func            = dmac_chan_sync;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WALN_CTX_EVENT_SUB_TYPR_DISABLE_TX].p_func             = dmac_chan_disable_machw_tx_event_process;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WALN_CTX_EVENT_SUB_TYPR_ENABLE_TX].p_func              = dmac_chan_enable_machw_tx_event_process;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPR_RESTART_NETWORK].p_func        = dmac_chan_restart_network_after_switch_event;
#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_IP_FILTER].p_func              = dmac_config_update_ip_filter;
#endif //_PRE_WLAN_FEATURE_IP_FILTER
#ifdef _PRE_WLAN_FEATURE_DFS
#ifdef _PRE_WLAN_FEATURE_OFFCHAN_CAC
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPR_SWITCH_TO_OFF_CHAN].p_func     = dmac_dfs_switch_to_offchan_event_process;
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPR_SWITCH_TO_HOME_CHAN].p_func    = dmac_dfs_switch_back_event_process;
#endif
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPR_DFS_TEST].p_func               = dmac_dfs_test;
#endif

#ifdef _PRE_WLAN_FEATURE_EDCA_OPT_AP
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPR_EDCA_OPT].p_func = dmac_edca_opt_stat_event_process;
#endif

#ifdef _PRE_WLAN_FEATURE_PKT_MEM_OPT
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_DSCR_OPT].p_func = dmac_pkt_mem_opt_stat_event_process;
#endif

    /* 注册DMAC模块WLAN_DRX事件子表 */
    g_ast_dmac_wlan_drx_event_sub_table[HAL_WLAN_DRX_EVENT_SUB_TYPE_RX].p_func = dmac_rx_process_data_event;

    /* 注册DMAC模块WLAN_CRX事件子表 */
    g_ast_dmac_wlan_crx_event_sub_table[HAL_WLAN_CRX_EVENT_SUB_TYPE_RX].p_func = dmac_rx_process_data_event;

    /* 注册DMAC模块TX_COMP事件子表 */
    g_ast_dmac_tx_comp_event_sub_table[HAL_TX_COMP_SUB_TYPE_TX].p_func = dmac_tx_complete_event_handler;
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    g_ast_dmac_tx_comp_event_sub_table[HAL_TX_COMP_SUB_TYPE_AL_TX].p_func = dmac_al_tx_complete_event_handler;
#endif

    /* 注册DMAC模块TBTT事件字表 */
    g_ast_dmac_tbtt_event_sub_table[HAL_EVENT_TBTT_SUB_TYPE].p_func = dmac_tbtt_event_handler;

    /* 注册DMAC模块ERR事件子表 */
    g_ast_dmac_high_prio_event_sub_table[HAL_EVENT_ERROR_IRQ_MAC_ERROR].p_func = dmac_mac_error_process_event;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    g_ast_dmac_high_prio_event_sub_table[HAL_EVENT_ERROR_IRQ_SOC_ERROR].p_func = dmac_soc_error_process_event;
#endif
    /* 注册DMAC模块MISC事件字表 */
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_MISC_CH_STATICS_COMP].p_func = dmac_scan_channel_statistics_complete;
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_CALI_TO_HMAC].p_func = dmac_cali_to_hmac;

#ifdef _PRE_WLAN_ONLINE_DPD
    g_ast_dmac_wlan_ctx_event_sub_table[DMAC_WLAN_CTX_EVENT_SUB_TYPE_DPD_HMAC2DMAC].p_func     = dmac_cali_corram_recv;

    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_DPD_TO_HMAC].p_func = dmac_dpd_to_hmac;

#endif

#ifdef _PRE_WLAN_RF_AUTOCALI
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_AUTOCALI_TO_HMAC].p_func = dmac_autocali_to_hmac;
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_MISC_FTM_ACK_COMPLETE].p_func = dmac_process_ftm_ack_complete;
#endif
#ifdef _PRE_WLAN_FEATURE_AP_PM
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_MISC_WOW_WAKE].p_func = ap_pm_wow_host_wake_event;
#endif

#ifdef _PRE_WLAN_DFT_REG
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_REG_REPORT].p_func = dmac_reg_report;
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_MISC_RADAR_DETECTED].p_func  = dmac_dfs_radar_detect_event;
#endif

    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_BEACON_TIMEOUT].p_func = dmac_beacon_timeout_event_hander;

#ifdef _PRE_WLAN_FEATURE_P2P
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_P2P_NOA_ABSENT_START].p_func  = dmac_p2p_noa_absent_start_event;
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_P2P_NOA_ABSENT_END].p_func  = dmac_p2p_noa_absent_end_event;
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_P2P_CTWINDOW_END].p_func  = dmac_p2p_oppps_ctwindow_end_event;
#endif
#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_MISC_GREEN_AP].p_func  = dmac_green_ap_timer_event_handler;
#endif
#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_MISC_PSD_COMPLETE].p_func  = dmac_psd_data_handler;
#endif
#if 0
#ifdef _PRE_WLAN_FEATURE_CSI
        g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_MISC_CSI_COMPLETE].p_func  = dmac_csi_data_handler;
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_GNSS_SCAN
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_MISC_IPC_IRQ].p_func  = dmac_ipc_irq_event;
#endif
#ifdef _PRE_WLAN_FEATURE_NO_FRM_INT
    g_ast_dmac_misc_event_sub_table[HAL_EVENT_DMAC_MISC_BCN_NO_FRM].p_func = dmac_bcn_no_frm_event_hander;
#endif
}


oal_uint32  dmac_event_fsm_register(oal_void)
{

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

    /*注册所有事件的tx adapt子表*/
    dmac_event_fsm_tx_adapt_subtable_register();

    /*注册所有事件的rx adapt子表*/
    dmac_event_fsm_rx_adapt_subtable_register();
#endif

    /*注册所有事件的执行函数子表*/
    dmac_event_fsm_action_subtable_register();

    event_fsm_table_register();

    return OAL_SUCC;
}



oal_void  dmac_alg_config_event_register(oal_uint32 p_func(frw_event_mem_stru *))
{
    g_ast_dmac_host_crx_table[HMAC_TO_DMAC_SYN_ALG].p_func = p_func;
}


oal_void  dmac_alg_config_event_unregister(oal_void)
{
    g_ast_dmac_host_crx_table[HMAC_TO_DMAC_SYN_ALG].p_func = OAL_PTR_NULL;
}


#ifdef _PRE_WLAN_DFT_REG


oal_uint32 dmac_reg_report(frw_event_mem_stru *pst_event_mem)
{

    oam_reg_report();
    return OAL_SUCC;
}

#endif

#ifdef _PRE_WLAN_MAC_ADDR_EDCA_FIX

oal_void dmac_edca_custom_init(oal_void)
{
    hwifi_get_edca_fix_cfg();
}
#endif

#if defined (_PRE_WLAN_FEATURE_RX_AGGR_EXTEND) || defined (_PRE_FEATURE_WAVEAPP_CLASSIFY)

oal_void dmac_wavap_custom_init(oal_void)
{
    hwifi_get_wavap_mac_cfg();
}
#endif

/*lint -save -e578 -e19 */
DEFINE_GET_BUILD_VERSION_FUNC(dmac);
/*lint -restore*/


oal_int32  dmac_main_init(oal_void)
{
    frw_init_enum_uint16 en_init_state = 0;
    oal_uint32 ul_ret;

    OAL_RET_ON_MISMATCH(dmac, -OAL_EFAIL);

    /*lint -save -e522 */
    dmac_param_check();
    /*lint -restore */

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    if(OAL_SUCC != dmac_hcc_adapt_init())
    {
        return -OAL_EFAIL;
    }
#endif

    /* 资源池初始化*/
    //WLAN_EDA_TRACE_TAG(0x4221UL);
    ul_ret = mac_res_init();
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_main_init: mac_res_init return err code %d\n", ul_ret);
        frw_timer_delete_all_timer();
        OAL_BUG_ON(1);
        return -OAL_EFAIL;//lint !e527
    }
    /* dmac资源池初始化 */
    dmac_res_init();
    //WLAN_EDA_TRACE_TAG(0x4222UL);
    dmac_event_fsm_register();

    //WLAN_EDA_TRACE_TAG(0x4223UL);
    en_init_state = frw_get_init_state();
    /* 说明: 在offload方案下，在host侧DMAC也可能存在(部分MIB相关信息挂接)，此时DMAC直接依赖FRW模块，此处状态判断需要修改 */
    if ((FRW_INIT_STATE_BUTT == en_init_state) || (en_init_state < FRW_INIT_STATE_HAL_SUCC))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_main_init:en_init_state error %d\n", en_init_state);
        frw_timer_delete_all_timer();
        event_fsm_unregister();
        mac_res_exit();

        OAL_BUG_ON(1);
        return -OAL_EFAIL;//lint !e527
    }

    //WLAN_EDA_TRACE_TAG(0x4224UL);
    /* 如果初始化状态处于配置VAP成功前的状态，表明此次为DMAC第一次初始化，即重加载或启动初始化 */
    if (en_init_state < FRW_INIT_STATE_DMAC_CONFIG_VAP_SUCC)
    {
#if (_PRE_PRODUCT_ID_HI1151 ==_PRE_PRODUCT_ID)
        dmac_timestamp_init();
#endif

        ul_ret = dmac_board_init(g_pst_mac_board);
        if (ul_ret != OAL_SUCC)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_main_init:mac_board_init return fail %d\n.", ul_ret);

            OAL_IO_PRINT("dmac_main_init:dmac_board_init [%u]\r\n",ul_ret );
            frw_timer_delete_all_timer();
            event_fsm_unregister();
            mac_res_exit();

            OAL_BUG_ON(1);
            return -OAL_EFAIL;//lint !e527
        }

        frw_set_init_state(FRW_INIT_STATE_DMAC_CONFIG_VAP_SUCC);

#ifdef _PRE_WLAN_PERFORM_STAT
        /* 性能统计模块初始化 */
        ul_ret = dmac_stat_init();
        if (OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "dmac_stat_init failed \n");
            return OAL_FAIL;
        }
#endif

#ifdef _PRE_WLAN_11K_STAT
        dmac_stat_init_device();
#endif

       dmac_test_init();

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
       dmac_bsd_init();
#endif

#ifdef _PRE_WLAN_FEATURE_DAQ
        dmac_data_acq_init();
#endif

        dmac_dfx_init();

        dmac_reset_init();

#ifdef _PRE_WLAN_FEATURE_BTCOEX
        dmac_btcoex_init();
#endif

#ifdef _PRE_WLAN_FEATURE_SMARTANT
        dmac_dual_antenna_init();
#endif
#ifdef _PRE_WLAN_PROFLING_MIPS
        oal_profiling_mips_init();
#endif

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
        dmac_set_auto_freq_process_func();
#endif

#if defined(_PRE_WLAN_FEATURE_HILINK_TEMP_PROTECT) || defined(_PRE_WLAN_FEATURE_HILINK_HERA_PRODUCT)
        FRW_TIMER_CREATE_TIMER(&g_st_dmac_pub_timer,
                               dmac_user_pub_timer_callback_func,
                               DMAC_PUBLIC_TIMER_PERIOD,
                               NULL,
                               OAL_TRUE,
                               OAM_MODULE_ID_DMAC,
                               OAL_GET_CORE_ID());
#endif

#ifdef _PRE_WLAN_MAC_ADDR_EDCA_FIX
        /* 初始化定制化固定edca */
        dmac_edca_custom_init();
#endif

#if defined (_PRE_WLAN_FEATURE_RX_AGGR_EXTEND) || defined (_PRE_FEATURE_WAVEAPP_CLASSIFY)
         /* 初始化定制化中仪器mac地址*/
         dmac_wavap_custom_init();
#endif

        return OAL_SUCC;
    }

    /* TBD 迭代10 康国昌修改 如果初始化状态为配置VAP成功后的状态，则表明本次为配置触发的初始化，
       需要遍历所有业务VAP，并检查其状态；如未初始化，则需要初始化其相关内容
       如支持特性接口挂接 */
    //WLAN_EDA_TRACE_TAG(0x4225UL);

    return OAL_SUCC;
}


oal_void  dmac_main_exit(oal_void)
{
    oal_uint32           ul_ret;

#ifdef _PRE_WLAN_FEATURE_DAQ
    dmac_data_acq_exit();
#endif

#ifdef _PRE_WLAN_PERFORM_STAT
    /* 性能统计模块卸载 */
    ul_ret = dmac_stat_exit();
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_main_exit::dmac_stat_exit failed.}");

        return;
    }
#endif
#ifdef _PRE_WLAN_11K_STAT
    dmac_stat_exit_device();
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
       dmac_bsd_exit();
#endif

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    dmac_set_auto_freq_exit();
#endif

    event_fsm_unregister();

#if (_PRE_PRODUCT_ID_HI1151 ==_PRE_PRODUCT_ID)
    dmac_timestamp_exit();
#endif

#if 0
    ul_ret = mac_board_exit(&g_st_dmac_board);
    if (OAL_SUCC != ul_ret)
    {
        return ;
    }
#else
    ul_ret = dmac_board_exit(g_pst_mac_board);
    if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_main_exit::hmac_board_exit failed[%d].}", ul_ret);
        return ;
    }
#endif

    frw_set_init_state(FRW_INIT_STATE_HAL_SUCC);

    mac_res_exit();

    dmac_dfx_exit();

#ifdef _PRE_WLAN_FEATURE_BTCOEX
    dmac_btcoex_exit();
#endif

    frw_timer_clean_timer(OAM_MODULE_ID_DMAC);

    return ;
}

/*lint -e578*//*lint -e19*/
#if (_PRE_PRODUCT_ID_HI1151==_PRE_PRODUCT_ID)
oal_module_init(dmac_main_init);
oal_module_exit(dmac_main_exit);
oal_module_symbol(dmac_timestamp_get);
#endif
oal_module_symbol(dmac_main_init);
oal_module_symbol(dmac_main_exit);
oal_module_symbol(dmac_alg_config_event_register);
oal_module_symbol(dmac_alg_config_event_unregister);
/*lint +e578*//*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

