


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_auto_adjust_freq.h"
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
#include "pm_extern.h"
#include "dmac_psm_sta.h"
#include "oal_main.h"
#endif


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_AUTO_ADJUST_FREQ_C



/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_pps_statistics_stru g_device_pps_statistics = {0};

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
/*device主频类型*/
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
oal_uint16 g_device_speed_freq[][FREQ_BUTT] = {
    {PM_40MHZ,PM_160MHZ,PM_240MHZ,PM_480MHZ},     /*WLAN_BW_20*/
    {PM_40MHZ,PM_160MHZ,PM_240MHZ,PM_480MHZ},    /*WLAN_HT_BW_40*/
    {PM_40MHZ,PM_160MHZ,PM_240MHZ,PM_480MHZ},   /*WLAN_VHT_BW_40*/
    {PM_80MHZ,PM_160MHZ,PM_240MHZ,PM_480MHZ},   /*WLAN_VHT_BW_80*/
};
#elif (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
oal_uint16 g_device_speed_freq[][FREQ_BUTT] = {
    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_240MHZ},     /*WLAN_BW_20*/
    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_320MHZ},    /*WLAN_HT_BW_40*/
    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_320MHZ},    /*WLAN_VHT_BW_40*/
    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_480MHZ},   /*WLAN_VHT_BW_80*/
    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_640MHZ},   /*WLAN_VHT_BW_160*/

    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_240MHZ},   /*WLAN_BW_20_MIMO*/
    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_320MHZ},    /*WLAN_HT_BW_40_MIMO*/
    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_320MHZ},   /*WLAN_VHT_BW_40_MIMO*/
    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_480MHZ},   /*WLAN_VHT_BW_80_MIMO*/
    {PM_40MHZ,  PM_80MHZ,  PM_240MHZ, PM_640MHZ},   /*WLAN_VHT_BW_160_MIMO*/
};
typedef struct _FREQ_AUTO_SET_STATE_STRU
{
    oal_uint16 freq_current;
    oal_uint16 freq_next;
    oal_uint8  freq_flag;
}FREQ_AUTO_SET_STATE_STRU;

FREQ_AUTO_SET_STATE_STRU g_st_freq_set_state = {0};
device_main_loop_mem g_auto_set_freq_testcase_main;
oal_uint32 g_loop_count = 0;
#endif
/*由定制化进行初始化*/
device_pps_freq_level_stru g_device_ba_pps_freq_level[] = {
    /*pps门限                   CPU主频level */
    {PPS_VALUE_0,          FREQ_IDLE},
    {PPS_VALUE_1,          FREQ_MIDIUM},
    {PPS_VALUE_2,          FREQ_HIGHER},
    {PPS_VALUE_3,          FREQ_HIGHEST},
};

device_pps_freq_level_stru g_device_no_ba_pps_freq_level[] = {
    /*pps门限                   CPU主频level */
    {NO_BA_PPS_VALUE_0,    FREQ_IDLE},
    {NO_BA_PPS_VALUE_1,    FREQ_MIDIUM},
    {NO_BA_PPS_VALUE_2,    FREQ_HIGHER},
    {NO_BA_PPS_VALUE_3,    FREQ_HIGHEST},
};

/* device调频控制结构体 */
dmac_freq_control_stru g_device_freq_type = {0};

/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint8 dmac_get_device_bw_type(void)
{
    oal_uint8     uc_vap_idx;
    oal_uint8     uc_device;
    oal_uint8     uc_device_max;
    mac_vap_stru *pst_mac_vap;
    wlan_auto_freq_bw_enum_uint8 uc_auto_freq_bw_type = WLAN_BW_20;
    mac_device_stru     *pst_mac_device;
    mac_chip_stru       *pst_mac_chip;

    pst_mac_chip = dmac_res_get_mac_chip(0);

    /* OAL接口获取支持device个数 */
    uc_device_max = oal_chip_get_device_num(pst_mac_chip->ul_chip_ver);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        pst_mac_device = mac_res_get_dev(pst_mac_chip->auc_device_id[uc_device]);
        if (OAL_PTR_NULL == pst_mac_device)
        {
            continue;
        }

        for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
        {
            pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);

            if (OAL_PTR_NULL == pst_mac_vap)
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_get_device_freq_type pst_mac_vap is null.}");
                continue;
            }
            if ((MAC_VAP_STATE_UP != pst_mac_vap->en_vap_state) &&
                (MAC_VAP_STATE_PAUSE != pst_mac_vap->en_vap_state))
            {
                continue;
            }


            switch (pst_mac_vap->en_protocol)
            {
                /*11ABGN*/
                case WLAN_LEGACY_11A_MODE:
                case WLAN_LEGACY_11B_MODE:
                case WLAN_LEGACY_11G_MODE:
                case WLAN_MIXED_ONE_11G_MODE:
                case WLAN_MIXED_TWO_11G_MODE:
                case WLAN_HT_MODE:
                case WLAN_HT_ONLY_MODE:
                case WLAN_HT_11G_MODE:
                {
                    if ((WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth) ||
                        (WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth))
                    {
                        uc_auto_freq_bw_type = OAL_MAX(uc_auto_freq_bw_type, WLAN_HT_BW_40);
                    }
                    break;
                }

                /*11AC*/
                case WLAN_VHT_MODE:
                case WLAN_VHT_ONLY_MODE:
                {
                    if ((WLAN_BAND_WIDTH_20M == pst_mac_vap->st_channel.en_bandwidth) ||
                        (WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth) ||
                        (WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth))
                    {
                        uc_auto_freq_bw_type = OAL_MAX(uc_auto_freq_bw_type, WLAN_VHT_BW_40);
                    }

                    else if((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) &&
                            (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS))
                    {
                        uc_auto_freq_bw_type = OAL_MAX(uc_auto_freq_bw_type, WLAN_VHT_BW_80);
                    }

#ifdef _PRE_WLAN_FEATURE_160M
                    else if((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS)  &&
                            (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS))
                    {
                        uc_auto_freq_bw_type = OAL_MAX(uc_auto_freq_bw_type, WLAN_VHT_BW_160);
                    }
#endif
                    else
                    {
                        uc_auto_freq_bw_type = OAL_MAX(uc_auto_freq_bw_type, WLAN_VHT_BW_80);
                    }

                    break;
                }

                /*11AX*/
#ifdef _PRE_WLAN_FEATURE_11AX
              case WLAN_HE_MODE:
                {
                    break;
                }
#endif

                default:
                   break;

            }

        }

    }

    return uc_auto_freq_bw_type;
}


oal_uint32 dmac_get_device_freq_value(oal_device_freq_type_enum_uint8 uc_device_freq_type,wlan_auto_freq_bw_enum_uint8 uc_auto_freq_bw_nss_type,oal_uint16* pusdevice_freq_value)
{
    if (uc_device_freq_type > FREQ_HIGHEST)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_get_device_freq_value:para error,uc_device_freq_type = %d.}",uc_device_freq_type);
        return OAL_FAIL;
    }
    if (uc_auto_freq_bw_nss_type >= WLAN_BW_BUTT)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_get_device_freq_value:para error,uc_auto_freq_bw_nss_type = %d.}",uc_auto_freq_bw_nss_type);
        return OAL_FAIL;
    }
    *pusdevice_freq_value = g_device_speed_freq[uc_auto_freq_bw_nss_type][uc_device_freq_type];

    return OAL_SUCC;
}


oal_void dmac_auto_set_freq(oal_uint16 us_device_freq)
{

#ifdef _PRE_WLAN_DOWNLOAD_PM
    if (g_us_download_rate_limit_pps)
    {
        us_device_freq = PM_40MHZ;
    }
#endif

    if (PM_FREQ_MAX < us_device_freq)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_auto_set_freq:Wrong freq[%d] larger than max[%d].}", us_device_freq, PM_FREQ_MAX);
        return;
    }

    /* set freq in PM_Driver_Crg_WlanChgCpuFreq */
    PM_WLAN_SetMaxCpuFreq(us_device_freq);

    if (us_device_freq < PM_80MHZ)
    {
        g_device_wlan_pm_timeout       = DMAC_PSM_TIMER_IDLE_TIMEOUT;
#ifdef _PRE_WLAN_DOWNLOAD_PM
        g_pm_timer_restart_cnt         = (g_us_download_rate_limit_pps ? 1 : g_ps_fast_check_cnt);
#else
        g_pm_timer_restart_cnt         = g_ps_fast_check_cnt;
#endif
    }
    else
    {
        g_device_wlan_pm_timeout       = DMAC_PSM_TIMER_BUSY_TIMEOUT;
        g_pm_timer_restart_cnt         = DMAC_PSM_TIMER_BUSY_CNT;
    }

    PM_WLAN_SwitchToState(PM_WPF_ID, (PM_WPF_WK | PM_WLAN_GetMaxCpuFreq()));

}

oal_void dmac_auto_set_device_freq(oal_void)
{
    wlan_auto_freq_bw_enum_uint8 uc_auto_freq_bw_type = WLAN_BW_20;
    oal_uint16                   us_device_freq = PM_40MHZ;
    oal_uint32                    uc_ret;
    dmac_freq_control_stru       *pst_freq_handle;
    wlan_nss_enum_uint8           uc_max_nss_num; /*空间流个数*/
    wlan_auto_freq_bw_enum_uint8 uc_auto_freq_bw_nss_type;

    pst_freq_handle = dmac_get_auto_freq_handle();

    /* 相等不需要调频 */
    if(pst_freq_handle->uc_curr_freq_level == pst_freq_handle->uc_req_freq_level)
    {
        return;
    }

    uc_auto_freq_bw_type = dmac_get_device_bw_type();

    uc_max_nss_num       = dmac_get_device_nss_type();

    uc_auto_freq_bw_nss_type = dmac_get_device_bw_nss_type(uc_auto_freq_bw_type, uc_max_nss_num);

    /*通过带宽类型和SISO/MIMO模式类型，获取设备需要调频的频率值--us_device_freq*/
    uc_ret = dmac_get_device_freq_value(pst_freq_handle->uc_req_freq_level, uc_auto_freq_bw_nss_type, &us_device_freq);
    if(OAL_SUCC != uc_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_auto_set_device_freq: get us_device_freq fail[%d].}",us_device_freq);
        return;
    }

    dmac_auto_set_freq(us_device_freq);

    OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_auto_set_freq:change freq [%d] to [%d].}", pst_freq_handle->uc_curr_freq_level, pst_freq_handle->uc_req_freq_level);
    pst_freq_handle->uc_curr_freq_level = pst_freq_handle->uc_req_freq_level;

}


extern oal_uint16 g_usUsedMemForStop;

oal_void dmac_auto_freq_netbuf_notify(oal_uint32 ul_free_cnt)
{
    dmac_freq_control_stru   *pst_freq_handle;

    pst_freq_handle = dmac_get_auto_freq_handle();

    /* 如果不使能，则不设置 */
    if(OAL_FALSE == pst_freq_handle->uc_auto_freq_enable)
    {
        return;
    }
    /* 如果低功耗睡眠，则不设置 */
    if(OAL_FALSE == pst_freq_handle->uc_pm_enable)
    {
        return;
    }

    /* 触发高优先级流控时，调高频率 */
    if ((ul_free_cnt <= (WLAN_AUTO_FREQ_NETBUF_THRESHOLD + WLAN_AUTO_FREQ_NETBUF_THRESHOLD)) && (pst_freq_handle->uc_curr_freq_level != FREQ_HIGHEST))
    //if ((ul_free_cnt <= (g_usUsedMemForStop + WLAN_AUTO_FREQ_NETBUF_THRESHOLD + 1)) && (pst_freq_handle->uc_curr_freq_level != FREQ_HIGHEST))
    {
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_auto_freq_netbuf_notify:g_usUsedMemForStop[%d],ul_free_cnt[%d].}", g_usUsedMemForStop,ul_free_cnt);
        pst_freq_handle->uc_req_freq_level = FREQ_HIGHEST;
        dmac_auto_set_device_freq();
    }
}


oal_void dmac_set_auto_freq_process_func(oal_void)
{
    struct oal_process_func_handler    *pst_func_handle;

    pst_func_handle = oal_get_device_auto_freq_handle();
    pst_func_handle->p_auto_freq_judge_func = dmac_auto_freq_netbuf_notify;
}


oal_void dmac_auto_freq_pps_process(oal_uint32 ul_pkt_count)
{
    dmac_freq_control_stru       *pst_freq_handle;

    pst_freq_handle = dmac_get_auto_freq_handle();

    if (pst_freq_handle->uc_req_freq_level == pst_freq_handle->uc_curr_freq_level)
    {
        pst_freq_handle->ul_pps_loop_count = 0;
        return;
    }

    if(pst_freq_handle->uc_req_freq_level < pst_freq_handle->uc_curr_freq_level)
    {
        /*连续MAX_DEGRADE_FREQ_TIME_THRESHOLD后才降频，保证性能*/
        pst_freq_handle->ul_pps_loop_count++;
        if(0 != ul_pkt_count)
        {
            if(pst_freq_handle->ul_pps_loop_count >= WLAN_AUTO_FREQ_DATA_LOOP_THRESHOLD)
            {
                pst_freq_handle->ul_pps_loop_count = 0;
                dmac_auto_set_device_freq();
            }
        }
        else
        {
            if(pst_freq_handle->ul_pps_loop_count >= WLAN_AUTO_FREQ_NO_DATA_LOOP_THRESHOLD)
            {
                pst_freq_handle->ul_pps_loop_count = 0;
                dmac_auto_set_device_freq();
            }
        }
    }
    else
    {
        /*升频不等待，立即执行保证性能*/
        pst_freq_handle->ul_pps_loop_count = 0;

        /* 当需要升频时，一次性burst到最高频;然后再根据流量下调频率 */
        if(pst_freq_handle->uc_curr_freq_level < FREQ_HIGHER)
        {
            pst_freq_handle->uc_req_freq_level = FREQ_HIGHER;
        }
        else
        {
            pst_freq_handle->uc_req_freq_level = FREQ_HIGHEST;
        }

        dmac_auto_set_device_freq();
    }

}


oal_void dmac_set_auto_freq_init(oal_void)
{
    dmac_pps_statistics_stru *pst_pps_handle;
    frw_timeout_stru         *pst_timer;
    dmac_freq_control_stru       *pst_freq_handle;

    pst_freq_handle = dmac_get_auto_freq_handle();
    pst_pps_handle = dmac_get_pps_statistics_handle();

    pst_timer = &pst_pps_handle->timer;

    pst_freq_handle->uc_auto_freq_enable = OAL_TRUE;
    pst_freq_handle->uc_pm_enable = OAL_FALSE;
    pst_freq_handle->uc_curr_freq_level = FREQ_IDLE;
    pst_freq_handle->uc_req_freq_level = FREQ_HIGHEST;
    pst_freq_handle->ul_pps_loop_count = 0;

    pst_pps_handle->ul_pps_rate = 0;
    pst_pps_handle->ul_last_timeout = 0;
    pst_pps_handle->ul_hcc_rxtx_total = 0;

    /* 初始设为level0 */
    dmac_auto_set_device_freq();

    if(OAL_FALSE == pst_timer->en_is_registerd)
    {
        FRW_TIMER_CREATE_TIMER(pst_timer,
                           dmac_auto_freq_pps_timeout,
                           WLAN_AUTO_FREQ_THROUGHPUT_TIMEOUT,
                           OAL_PTR_NULL,
                           OAL_TRUE,
                           OAM_MODULE_ID_DMAC,
                           0);

       OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_set_auto_freq_init: timer start.}");
    }

    pst_pps_handle->uc_timer_reuse_count++;
}


oal_void dmac_set_auto_freq_exit(oal_void)
{
    dmac_freq_control_stru       *pst_freq_handle;

    pst_freq_handle = dmac_get_auto_freq_handle();

    dmac_set_auto_freq_deinit();

   pst_freq_handle->uc_req_freq_level = FREQ_IDLE;
   dmac_auto_set_device_freq();
}

#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
OAL_STATIC oal_void dmac_auto_set_freq_testcase_loop(oal_void)
{
    oal_uint16 usCurFreq;
    oal_uint16 usNextFreq;
    oal_uint16 usindex;
    oal_uint16 usFreqArrayNum = 0;
    oal_uint16 us_device_speed_freq[] = {PM_640MHZ, PM_480MHZ, PM_320MHZ,
                                         PM_320_1MHZ,PM_240MHZ, PM_160MHZ,
                                         PM_160_1MHZ,PM_120MHZ, PM_80MHZ,
                                         PM_40MHZ};

    g_loop_count++;
    if(g_loop_count%100)
    {
        return;
    }
    usFreqArrayNum = (sizeof(us_device_speed_freq)/sizeof(oal_uint16));

    usCurFreq = g_st_freq_set_state.freq_current;
    usNextFreq = g_st_freq_set_state.freq_next;

    if (!g_st_freq_set_state.freq_flag)
    {
        /*freq_flag=0，频率从curr切换到usindex*/
        if (usCurFreq != usNextFreq)
        {
            usindex = usNextFreq;
            usNextFreq = usNextFreq+1;
        }
        else
        {
            usindex = usNextFreq+1;
            usNextFreq = usNextFreq+2;
        }

        /*状态保存*/
        if(usNextFreq >= usFreqArrayNum)
        {
            g_st_freq_set_state.freq_current= (usCurFreq + 1)%usFreqArrayNum;
            g_st_freq_set_state.freq_next= 0;
        }
        else
        {
            g_st_freq_set_state.freq_next= usNextFreq;
        }
        g_st_freq_set_state.freq_flag= 1;

    }
    else
    {
        /*freq_flag=1，频率切换回之前的curr，状态无须保存*/
        g_st_freq_set_state.freq_flag= 0;
        usindex = usCurFreq;
    }

    if (usindex < usFreqArrayNum)
    {
        //OAM_WARNING_LOG4(0, OAM_SF_ANY, "dmac_auto_set_freq_testcase_loop:freq from %d to %d,next %d,flag %d",us_device_speed_freq[usCurFreq],us_device_speed_freq[usindex],us_device_speed_freq[g_st_freq_set_state.freq_next],g_st_freq_set_state.freq_flag);

        dmac_auto_set_freq(us_device_speed_freq[usindex]);
    }
}

oal_void dmac_auto_set_freq_testcase_init(oal_void)
{
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "dmac_auto_set_freq_testcase_init");
    g_auto_set_freq_testcase_main.func = dmac_auto_set_freq_testcase_loop;
    device_main_loop_hook_register(&g_auto_set_freq_testcase_main);
    return ;
}
oal_void dmac_auto_set_freq_testcase_exit(oal_void)
{
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "dmac_auto_set_freq_testcase_exit");
    g_st_freq_set_state.freq_current = 0;
    g_st_freq_set_state.freq_next = 0;
    g_st_freq_set_state.freq_flag = 0;
    device_main_loop_hook_unregister(&g_auto_set_freq_testcase_main);
    return ;
}

#endif

#endif



oal_uint32 dmac_auto_freq_pps_timeout(void *prg)
{
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    dmac_freq_control_stru       *pst_freq_handle;
#endif
    dmac_pps_statistics_stru *pst_pps_handle;
    oal_uint32                ul_cur_time = 0;
    oal_uint32                ul_return_total_count = 0;

    pst_pps_handle = dmac_get_pps_statistics_handle();

    ul_return_total_count = pst_pps_handle->ul_hcc_rxtx_total;

    ul_cur_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    if (ul_cur_time > pst_pps_handle->ul_last_timeout)
    {
        pst_pps_handle->ul_pps_rate = (ul_return_total_count << 10) / (ul_cur_time - pst_pps_handle->ul_last_timeout);

        //OAM_WARNING_LOG2(0, OAM_SF_ANY, "{dmac_auto_freq_pps_timeout:DATA RATE [%d].,time[%d]}",
       //          pst_pps_handle->ul_pps_rate, (ul_cur_time - pst_pps_handle->ul_last_timeout));
    }

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    pst_freq_handle = dmac_get_auto_freq_handle();

    if (OAL_TRUE == pst_freq_handle->uc_auto_freq_enable)
    {
        /* 根据吞吐量获取调频级别 */
        dmac_auto_freq_set_pps_level(pst_pps_handle->ul_pps_rate);

        dmac_auto_freq_pps_process(ul_return_total_count);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    /* green ap处理函数 */
    dmac_green_ap_pps_process(pst_pps_handle->ul_pps_rate);
#endif

    pst_pps_handle->ul_hcc_rxtx_total = 0;
    pst_pps_handle->ul_last_timeout = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    return OAL_SUCC;
}


oal_void dmac_set_auto_freq_pps_start(oal_void)
{
    frw_timeout_stru         *pst_timer;
    dmac_pps_statistics_stru *pst_pps_handle;

    pst_pps_handle = dmac_get_pps_statistics_handle();
    pst_timer = &pst_pps_handle->timer;

    if(OAL_FALSE == pst_timer->en_is_registerd)
    {
        FRW_TIMER_CREATE_TIMER(pst_timer,
                           dmac_auto_freq_pps_timeout,
                           WLAN_AUTO_FREQ_THROUGHPUT_TIMEOUT,
                           OAL_PTR_NULL,
                           OAL_TRUE,
                           OAM_MODULE_ID_DMAC,
                           0);

       OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_set_auto_freq_pps_reuse: timer start.}");
    }

    pst_pps_handle->uc_timer_reuse_count ++;
}


oal_void dmac_set_auto_freq_pps_stop(oal_void)
{
    frw_timeout_stru         *pst_timer;
    dmac_pps_statistics_stru *pst_pps_handle;

    pst_pps_handle = dmac_get_pps_statistics_handle();
    pst_timer = &pst_pps_handle->timer;

    if(OAL_TRUE == pst_timer->en_is_registerd)
    {
        pst_pps_handle->uc_timer_reuse_count --;

        /* 由最后退出的模块删除定时器 */
        if (0 == pst_pps_handle->uc_timer_reuse_count)
        {
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(pst_timer);

            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_set_auto_freq_pps_reuse_deinit: timer exit.}");
        }
    }
}
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


