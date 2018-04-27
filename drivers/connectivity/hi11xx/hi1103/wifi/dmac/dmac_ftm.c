


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_FTM


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "oal_util.h"
#include "mac_resource.h"
#include "mac_frame.h"
#include "mac_ie.h"
#include "dmac_vap.h"
#include "hal_ext_if.h"
#include "dmac_ftm.h"
#include "mac_device.h"
#include "mac_vap.h"
#include "dmac_scan.h"
#include "dmac_tx_bss_comm.h"
#include "hal_ext_if.h"
#include "dmac_tx_complete.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_FTM_C

/*****************************************************************************
  2 静态函数声明
*****************************************************************************/


/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
oal_uint32 g_aul_ftm_correct_time[FTM_TIME_BUTT] =
    {0,
     0,
     0,
     0
    };

dmac_ftm_stru             g_st_ftm = {0};              /*与FTM相关变量*/

/*****************************************************************************
  4 函数实现
*****************************************************************************/


oal_void  dmac_set_ftm_correct_time(dmac_vap_stru *pst_dmac_vap, mac_set_ftm_time_stru st_ftm_time)
{
    g_aul_ftm_correct_time[FTM_TIME_1] = st_ftm_time.ul_ftm_correct_time1;
    g_aul_ftm_correct_time[FTM_TIME_2] = st_ftm_time.ul_ftm_correct_time2;
    g_aul_ftm_correct_time[FTM_TIME_3] = st_ftm_time.ul_ftm_correct_time3;
    g_aul_ftm_correct_time[FTM_TIME_4] = st_ftm_time.ul_ftm_correct_time4;
}


oal_uint32 dmac_ftm_get_distance(dmac_vap_stru *pst_dmac_vap, oal_uint64 *ull_distance)
{
    oal_uint64                     ull_temp_distance = 0;
    oal_uint64                     ull_temp_t1 = 0;
    oal_uint64                     ull_temp_t2 = 0;
    oal_uint64                     ull_temp_t3 = 0;
    oal_uint64                     ull_temp_t4 = 0;
    oal_uint64                     ull_temp_td = 0;
    oal_uint8                      uc_index = 0;
    dmac_ftm_initiator_stru       *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    uc_index = past_ftm_init[0].uc_follow_up_dialog_token % MAC_FTM_TIMER_CNT;
    /*检测对应的索引的uc_dialog_token是否一致，防止已被覆盖*/
    if (past_ftm_init[0].ast_ftm_timer[uc_index].uc_dialog_token != past_ftm_init[0].uc_follow_up_dialog_token)
    {
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                        "{dmac_ftm_get_distance:: devic_dialog_token[%d]!=ftm_follow_updialog_token[%d]!!!!.}",
                        past_ftm_init[0].ast_ftm_timer[uc_index].uc_dialog_token,
                        past_ftm_init[0].uc_follow_up_dialog_token);

        OAL_MEMZERO(&past_ftm_init[0].ast_ftm_timer[uc_index], OAL_SIZEOF(ftm_timer_stru));
        return OAL_FAIL;
    }

    ull_temp_t1 = past_ftm_init[0].ast_ftm_timer[uc_index].ull_t1;
    ull_temp_t2 = past_ftm_init[0].ast_ftm_timer[uc_index].ull_t2;
    ull_temp_t3 = past_ftm_init[0].ast_ftm_timer[uc_index].ull_t3;
    ull_temp_t4 = past_ftm_init[0].ast_ftm_timer[uc_index].ull_t4;

    if ((ull_temp_t4 <= ull_temp_t1)||(ull_temp_t3 <= ull_temp_t2)
        ||((ull_temp_t4 - ull_temp_t1) <= (ull_temp_t3 - ull_temp_t2)))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "dmac_ftm_get_distance::t1 t2 t3 t4 not available!!!");

        OAL_MEMZERO(&past_ftm_init[0].ast_ftm_timer[uc_index], OAL_SIZEOF(ftm_timer_stru));
        return OAL_FAIL;
    }

    /* ((t4-t1)-(t3-t2))/2 * 10^-12 * (3*10^8) 时间单位换算成秒 */
    /* 即 ((t4-t1)-(t3-t2)) *3 / 2 * 10^-4 单位米*/
    /* 换成单位1/4096 m 即 ((t4-t1)-(t3-t2)) *6 /10 */
    //ull_temp_distance = ((ull_temp_t4 - ull_temp_t1)-(ull_temp_t3 - ull_temp_t2)) * 6 / 10;

    ull_temp_td = ((ull_temp_t4 - ull_temp_t1)-(ull_temp_t3 - ull_temp_t2)) >> 1;
    /*单位0.25mm*/
    ull_temp_distance = (ull_temp_td * 12) /10;

    //ull_temp_distance = ull_temp_distance - (oal_uint32)(pst_dmac_vap->st_ftm.ul_ftm_cali_time << 1);

    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "dmac_ftm_get_distance::FTM measured distances is [%d], ull_temp_td[%d], uc_dialog_token[%d] ",
                     ull_temp_distance,
                     ull_temp_td,
                     past_ftm_init[0].ast_ftm_timer[uc_index].uc_dialog_token);

    /*lint -save -e685 *//*lint -save -e568 */
    //if((ull_temp_distance < FTM_MIN_DETECTABLE_DISTANCE)
    //    ||(ull_temp_distance > FTM_MAX_DETECTABLE_DISTANCE))
    //{
    //    OAM_ERROR_LOG0(0, OAM_SF_FTM,
    //            "dmac_ftm_get_distance:: distance is not correct!!!");
        //return OAL_FAIL;//wxf
    //}
    /*lint -restore *//*lint -restore */

    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "dmac_ftm_get_distance::ull_t1[L0x%x], ull_t2[L0x%x], ull_t3[L0x%x], ull_t4[L0x%x].",
                     (oal_uint32)ull_temp_t1,
                     (oal_uint32)ull_temp_t2,
                     (oal_uint32)ull_temp_t3,
                     (oal_uint32)ull_temp_t4);

    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "dmac_ftm_get_distance::ull_t1[H0x%x], ull_t2[H0x%x], ull_t3[H0x%x], ull_t4[H0x%x].",
                     (oal_uint32)(ull_temp_t1 >> 32),
                     (oal_uint32)(ull_temp_t2 >> 32),
                     (oal_uint32)(ull_temp_t3 >> 32),
                     (oal_uint32)(ull_temp_t4 >> 32));

    OAL_MEMZERO(&past_ftm_init[0].ast_ftm_timer[uc_index], OAL_SIZEOF(ftm_timer_stru));

    *ull_distance = ull_temp_distance;

    return OAL_SUCC;
}


oal_uint32 dmac_process_ftm_ack_complete(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                     *pst_event;
    frw_event_hdr_stru                 *pst_event_hdr;
    mac_device_stru                    *pst_mac_dev;
    dmac_vap_stru                      *pst_dmac_vap;
    oal_uint64                          ull_tmp_t2 = 0;
    oal_uint64                          ull_tmp_t3 = 0;
    oal_uint8                           uc_dialog_token = 0;
    oal_uint8                           uc_index = 0;
    oal_int8                            c_time_intp = 0;
    hal_wlan_ftm_t2t3_rx_event_stru    *pst_ftm_t2t3;
    dmac_ftm_initiator_stru            *past_ftm_init;

    pst_event               = frw_get_event_stru(pst_event_mem);
    pst_event_hdr           = &(pst_event->st_event_hdr);
    pst_ftm_t2t3            = (hal_wlan_ftm_t2t3_rx_event_stru *)(pst_event->auc_event_data);

    OAM_WARNING_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_FTM, "{dmac_process_ftm_tx_ack_complete::get t2 t3.}");

    pst_mac_dev = mac_res_get_dev(pst_event_hdr->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_FTM, "{dmac_process_ftm_tx_ack_complete::pst_mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = mac_res_get_dmac_vap(pst_event_hdr->uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(pst_event_hdr->uc_vap_id, OAM_SF_FTM, "{dmac_process_ftm_tx_ack_complete::pst_dmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    /*此次FTM帧的dialog */
    uc_dialog_token = pst_ftm_t2t3->uc_dialog_token;
    uc_index = uc_dialog_token % MAC_FTM_TIMER_CNT;
    past_ftm_init[0].ast_ftm_timer[uc_index].uc_dialog_token = uc_dialog_token;

    /*初始化*/
    past_ftm_init[0].ast_ftm_timer[uc_index].ull_t2 = 0;
    past_ftm_init[0].ast_ftm_timer[uc_index].ull_t3 = 0;

    /* 写入T2 */
    ull_tmp_t2 = hal_check_ftm_t2(pst_dmac_vap->pst_hal_device, pst_ftm_t2t3->ull_t2);
    if (0 == ull_tmp_t2 )
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "dmac_process_ftm_tx_ack_complete:  t2 is zero!");
		return OAL_FAIL;
    }

    c_time_intp = hal_get_ftm_t2_intp(pst_dmac_vap->pst_hal_device, pst_ftm_t2t3->ull_t2);

    /* 计算时间，单位皮秒 */
    past_ftm_init[0].ast_ftm_timer[uc_index].ull_t2 = (oal_uint64)((oal_uint64)(((oal_int64)ull_tmp_t2 * PERIOD_OF_FTM_TIMER
                                                   - c_time_intp) + g_aul_ftm_correct_time[FTM_TIME_2]) << 10);
    /* 写入T3 */
    ull_tmp_t3 = hal_get_ftm_time(pst_dmac_vap->pst_hal_device, pst_ftm_t2t3->ull_t3);
    if (0 == ull_tmp_t3 )
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "dmac_process_ftm_tx_ack_complete:  t3 is zero!");
		return OAL_FAIL;
    }

    /* 计算时间，单位皮秒 */
    past_ftm_init[0].ast_ftm_timer[uc_index].ull_t3 = (oal_uint64)((ull_tmp_t3  * PERIOD_OF_FTM_TIMER - g_aul_ftm_correct_time[FTM_TIME_3]) << 10);

    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                   "{dmac_process_ftm_ack_complete::wxf get t3 - t2 = [%d], uc_dialog_token[%d], c_time_intp [%d].}",
                   past_ftm_init[0].ast_ftm_timer[uc_index].ull_t3 - past_ftm_init[0].ast_ftm_timer[uc_index].ull_t2,
                   uc_dialog_token,
                   c_time_intp);

    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "dmac_process_ftm_ack_complete::wxf ull_t2[H0x%x], ull_t2[L0x%x], ull_t3[H0x%x], ull_t3[L0x%x].",
                     (oal_uint32)(pst_ftm_t2t3->ull_t2 >> 32),
                     (oal_uint32)pst_ftm_t2t3->ull_t2,
                     (oal_uint32)(pst_ftm_t2t3->ull_t3 >> 32),
                     (oal_uint32)pst_ftm_t2t3->ull_t3);

    return OAL_SUCC;
}


oal_uint32 dmac_save_tx_ftm_time(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_session_id)
{
    oal_uint8                        uc_index = 0;
    oal_uint8                        uc_dialog_token =pst_dmac_vap->pst_ftm->ast_ftm_rsp[uc_session_id].uc_dialog_token_ack;
    oal_int8                         c_time_intp = 0;
    oal_uint64                       ull_t1;
    oal_uint64                       ull_t4;
    hal_to_dmac_device_stru         *pst_hal_device = pst_dmac_vap->pst_hal_device;
    ftm_timer_stru                  *past_ftm_timer = pst_dmac_vap->pst_ftm->ast_ftm_rsp[uc_session_id].ast_ftm_timer;

    OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_save_tx_ftm_timer::get t1 t4.}");

    /* 记录dialog token 对应的时间索引*/
    uc_index = uc_dialog_token % MAC_FTM_TIMER_CNT;

    /*初始化*/
    past_ftm_timer[uc_index].ull_t1 = 0;
    past_ftm_timer[uc_index].ull_t4 = 0;
    past_ftm_timer[uc_index].uc_dialog_token = uc_dialog_token;

    /* 写入T1 */
    ull_t1 = hal_get_ftm_time(pst_dmac_vap->pst_hal_device, pst_hal_device->ull_t1);
    if (0 == ull_t1)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "dmac_save_tx_ftm_time:  tod is zero!");
		return OAL_FAIL;
    }

    /* 计算时间，单位皮秒 */
    past_ftm_timer[uc_index].ull_t1 = (oal_uint64)((ull_t1 * PERIOD_OF_FTM_TIMER + g_aul_ftm_correct_time[FTM_TIME_1]) << 10);

    /* 写入T4 */
    ull_t4 = hal_check_ftm_t4(pst_dmac_vap->pst_hal_device, pst_hal_device->ull_t4);
    if (0 == ull_t4 )
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "dmac_save_tx_ftm_time:  toa is zero!");
		return OAL_FAIL;
    }

    c_time_intp = hal_get_ftm_t4_intp(pst_dmac_vap->pst_hal_device, pst_hal_device->ull_t4);

    /* 计算时间，单位皮秒 */
    past_ftm_timer[uc_index].ull_t4 = (oal_uint64)((oal_uint64)(((oal_int64)ull_t4 * PERIOD_OF_FTM_TIMER
                                                      - c_time_intp) - g_aul_ftm_correct_time[FTM_TIME_4]) << 10);


    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                   "{dmac_save_tx_ftm_timer::get t4 - t1 = [%d], uc_dialog_token[%d], c_time_intp [%d].}",
                   past_ftm_timer[uc_index].ull_t4 - past_ftm_timer[uc_index].ull_t1,
                   uc_dialog_token,
                   c_time_intp);

    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "dmac_save_tx_ftm_time::ull_t1[H0x%x], ull_t1[L0x%x], ull_t4[H0x%x], ull_t4[L0x%x].",
                     (oal_uint32)(pst_hal_device->ull_t1 >> 32),
                     (oal_uint32)pst_hal_device->ull_t1,
                     (oal_uint32)(pst_hal_device->ull_t4 >> 32),
                     (oal_uint32)pst_hal_device->ull_t4);

    return OAL_SUCC;
}


oal_uint32 dmac_check_tx_ftm(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buf)
{
    oal_int8                         c_session_id = -1;
    oal_uint8                        uc_session_id = 0;
    mac_ieee80211_frame_stru        *puc_mac_header  = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_buf);
    oal_uint8                       *puc_mac_payload = oal_netbuf_data(pst_buf);
    dmac_ftm_responder_stru         *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;

    if (OAL_TRUE == mac_is_ftm_frame(pst_buf))
    {
        c_session_id = dmac_ftm_find_session_index(pst_dmac_vap, MAC_FTM_RESPONDER_MODE, puc_mac_header->auc_address1);
        if(c_session_id < 0)
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_check_tx_ftm::session_id error!!!}");
            return OAL_FAIL;
        }
        uc_session_id = (oal_uint8)c_session_id;
        if(oal_compare_mac_addr(past_ftm_rsp[uc_session_id].auc_mac_ra, puc_mac_header->auc_address1))
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_check_tx_ftm::session_id error!!!}");
            return OAL_FAIL;
        }

        past_ftm_rsp[uc_session_id].uc_dialog_token_ack = puc_mac_payload[FTM_FRAME_DIALOG_TOKEN_OFFSET];
        dmac_save_tx_ftm_time(pst_dmac_vap, uc_session_id);
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


oal_void dmac_ftm_prepare_basic_scan_params(dmac_vap_stru *pst_dmac_vap, mac_scan_req_stru   *pst_scan_req)
{
    dmac_ftm_initiator_stru            *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    pst_scan_req->en_bss_type = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    pst_scan_req->en_scan_mode = WLAN_SCAN_MODE_FTM_REQ;
    oal_set_mac_addr(pst_scan_req->auc_bssid, past_ftm_init[0].auc_bssid);
    pst_scan_req->uc_bssid_num = 1;
    pst_scan_req->uc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;
    pst_scan_req->uc_scan_func = MAC_SCAN_FUNC_MEAS;
    pst_scan_req->uc_max_scan_count_per_channel = 1;
    pst_scan_req->uc_max_send_probe_req_count_per_channel = 0;
    /*被动扫描*/
    pst_scan_req->en_scan_type = WLAN_SCAN_TYPE_PASSIVE;
    oal_memcopy(&pst_scan_req->ast_channel_list[0], &past_ftm_init[0].st_channel_ftm, OAL_SIZEOF(mac_channel_stru));
    pst_scan_req->uc_channel_nums = 1;
    pst_scan_req->uc_probe_delay = 0;
    oal_set_mac_addr(pst_scan_req->auc_sour_mac_addr, mac_mib_get_StationID(&pst_dmac_vap->st_vap_base_info));
}


oal_uint32 dmac_sta_start_scan_for_ftm(dmac_vap_stru *pst_dmac_vap, oal_uint16 us_scan_time)
{
    mac_device_stru     *pst_mac_device;
    mac_scan_req_stru    st_scan_req;

    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_start_scan_for_accept_ftm::pst_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    //如果ftm过程中发生扫描
    dmac_scan_abort(pst_mac_device);

    dmac_ftm_prepare_basic_scan_params(pst_dmac_vap, &st_scan_req);

    /*扫描时间*/
    st_scan_req.us_scan_time = us_scan_time;

    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_start_scan_for_accept_ftm::time is [%d].}", us_scan_time);

    dmac_scan_handle_scan_req_entry(pst_mac_device, pst_dmac_vap, &st_scan_req);

    return OAL_SUCC;
}


oal_void  dmac_ftm_set_format_and_bandwidth(wlan_bw_cap_enum_uint8 en_band_cap,
                                                          wlan_phy_protocol_enum_uint8  en_protocol_mode,
                                                          mac_ftm_parameters_ie_stru *pst_mac_ftmp)
{
    /*Field  Value                Format Bandwidth (MHz)
      0      No preference        No preference
      1-3    Reserved             Reserved
      4      Non-HT               5
      5      Reserved             Reserved
      6      Non-HT               10
      7      Reserved             Reserved
      8      Non-HT,              20
             excluding
             Clause 15 (DSSS
             PHY specification
             for the 2.4 GHz
             band designated
             for ISM
             applications) and
             Clause 16 (High
             rate direct
             sequence spread
             spectrum (HR/
             DSSS) PHY
             specification)
      9      HT mixed             20
      10     VHT                  20
      11     HT mixed             40
      12     VHT                  40
      13     VHT                  80
      14     VHT                  80+80
      15     VHT (two             160
             separate RF LOs)

      16     VHT (single RF       160
             LO)

      17-30  Reserved             Reserved
      31     DMG                  2160
      32–63 Reserved             Reserved*/

    if((en_band_cap == WLAN_BW_CAP_20M) && (en_protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE))
    {
        pst_mac_ftmp->bit_format_and_bandwidth = 9;
    }
    else if((en_band_cap == WLAN_BW_CAP_20M) && (en_protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE))
    {
        pst_mac_ftmp->bit_format_and_bandwidth = 10;
    }
    else if((en_band_cap == WLAN_BW_CAP_40M) && (en_protocol_mode == WLAN_HT_PHY_PROTOCOL_MODE))
    {
        pst_mac_ftmp->bit_format_and_bandwidth = 11;
    }
    else if((en_band_cap == WLAN_BW_CAP_40M) && (en_protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE))
    {
        pst_mac_ftmp->bit_format_and_bandwidth = 12;
    }
    else if((en_band_cap == WLAN_BW_CAP_80M) && (en_protocol_mode == WLAN_VHT_PHY_PROTOCOL_MODE))
    {
        pst_mac_ftmp->bit_format_and_bandwidth = 13;
    }
    else
    {
        pst_mac_ftmp->bit_format_and_bandwidth = 9;
    }

}


oal_void  dmac_ftm_get_format_and_bandwidth(wlan_bw_cap_enum_uint8 *pen_band_cap,
                                                          wlan_phy_protocol_enum_uint8  *pen_protocol_mode,
                                                          mac_ftm_parameters_ie_stru *pst_mac_ftmp)
{
    /*Field  Value                Format Bandwidth (MHz)
      0      No preference        No preference
      1-3    Reserved             Reserved
      4      Non-HT               5
      5      Reserved             Reserved
      6      Non-HT               10
      7      Reserved             Reserved
      8      Non-HT,              20
             excluding
             Clause 15 (DSSS
             PHY specification
             for the 2.4 GHz
             band designated
             for ISM
             applications) and
             Clause 16 (High
             rate direct
             sequence spread
             spectrum (HR/
             DSSS) PHY
             specification)
      9      HT mixed             20
      10     VHT                  20
      11     HT mixed             40
      12     VHT                  40
      13     VHT                  80
      14     VHT                  80+80
      15     VHT (two             160
             separate RF LOs)

      16     VHT (single RF       160
             LO)

      17-30  Reserved             Reserved
      31     DMG                  2160
      32–63 Reserved             Reserved*/

    switch(pst_mac_ftmp->bit_format_and_bandwidth)
    {
        case 9:
            *pen_band_cap = WLAN_BW_CAP_20M;
            *pen_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
        case 10:
            *pen_band_cap = WLAN_BW_CAP_20M;
            *pen_protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        case 11:
            *pen_band_cap = WLAN_BW_CAP_40M;
            *pen_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
        case 12:
            *pen_band_cap = WLAN_BW_CAP_40M;
            *pen_protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        case 13:
            *pen_band_cap = WLAN_BW_CAP_80M;
            *pen_protocol_mode = WLAN_VHT_PHY_PROTOCOL_MODE;
            break;
        default:
            *pen_band_cap = WLAN_BW_CAP_20M;
            *pen_protocol_mode = WLAN_HT_PHY_PROTOCOL_MODE;
            break;
    }

}


oal_uint16  dmac_encap_ftm_req_mgmt(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buffer)
{
    oal_uint8                     *puc_mac_header          = oal_netbuf_header(pst_buffer);
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint8                     *puc_payload_addr        = oal_netbuf_data(pst_buffer);
#else
    oal_uint8                     *puc_payload_addr        = puc_mac_header + MAC_80211_FRAME_LEN;
#endif
    //oal_uint8                      uc_chan_idx     = 0;
    //oal_uint16                     us_ie_len_idx   = 0;
    oal_uint16                     us_index        = 0;
    mac_vap_stru                  *pst_mac_vap     = &pst_dmac_vap->st_vap_base_info;
    dmac_ftm_initiator_stru       *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;
    oal_uint16                     us_burst_cnt    = past_ftm_init[0].us_burst_cnt;

    mac_ftm_parameters_ie_stru    *pst_mac_ftmp;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID)*/
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, past_ftm_init[0].auc_bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, past_ftm_init[0].auc_bssid);

    /*************************************************************************************/
    /*                       	FTM Request frame - Frame Body                           */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Trigger | LCI Measurement Request(optional)|            */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1       |Variable                          |            */
    /* --------------------------------------------------------------------------------- */
    /* |LCI Report (optional) |                                                          */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    |                                    */
    /* --------------------------------------------------------------------------------- */
    /* |Location Civic MeasuremenRequest (optional) |                                    */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    |                                    */
    /* --------------------------------------------------------------------------------- */
    /* |Fine Timing MeasuremenParameters (optional) |                                    */
    /* --------------------------------------------------------------------------------- */
    /* |Variable                                    |                                    */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/

    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_PUBLIC;           /* Category */
    puc_payload_addr[us_index++] = MAC_PUB_FTM_REQ;                      /* Public Action */

    puc_payload_addr[us_index++] = 1;                                    /* Trigger */

    /*****************************************************************************************************/
    /*                   LCI Measurement Request (Measurement Request IE)                                */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req |*/
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             |*/
    /* ------------------------------------------------------------------------------------------------- */
    /*                                                                                                   */
    /*****************************************************************************************************/

    /* 封装Measurement Request IE  */
    if(past_ftm_init[0].en_lci_ie == OAL_TRUE)
    {
        puc_payload_addr[us_index++] = MAC_EID_MEASREQ;                   /* Element ID */
        puc_payload_addr[us_index++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        puc_payload_addr[us_index++] = 1;                                 /* Measurement Token */
        puc_payload_addr[us_index++] = 0;                                 /* Measurement Req Mode */
        puc_payload_addr[us_index++] = RM_RADIO_MEASUREMENT_LCI;          /* Measurement Type */

    }

    /* 封装Measurement Request IE  */
    if(past_ftm_init[0].en_location_civic_ie == OAL_TRUE)
    {
        puc_payload_addr[us_index++] = MAC_EID_MEASREQ;                   /* Element ID */
        puc_payload_addr[us_index++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        puc_payload_addr[us_index++] = 1;                                 /* Measurement Token */
        puc_payload_addr[us_index++] = 0;                                 /* Measurement Req Mode */
        puc_payload_addr[us_index++] = RM_RADIO_MEASUREMENT_LOCATION_CIVIC;    /* Measurement Type */
    }

    /*******************************************************************/
    /*                    Fine Timing Measurement Parameters element   */
    /* --------------------------------------------------------------- */
    /* |Element ID |Length |Fine Timing Measurement Parameters|        */
    /* --------------------------------------------------------------- */
    /* |1          |1      | 9                                |        */
    /* --------------------------------------------------------------- */
    /*                                                                 */
    /*******************************************************************/

    /***********************************************************************************************************************/
    /*                    Fine Timing Measurement Parameters                                                               */
    /* ------------------------------------------------------------------------------------------------------------------- */
    /* |B0             B1 |B2 B6 |B7       | B8                   B11 |B12        B15 |B16       B23 |B24           B39 |  */
    /* ------------------------------------------------------------------------------------------------------------------- */
    /* |Status Indication |Value |Reserved |Number of Bursts Exponent |Burst Duration |Min Delta FTM |Partial TSF Timer |  */
    /*                                                                                                                     */
    /* ------------------------------------------------------------------------------------------------------------------- */
    /* |B40                             |B41          |B42  |B43        B47 |B48  B49 |B50              B55 |B56      B71 |*/
    /* ------------------------------------------------------------------------------------------------------------------- */
    /* |Partial TSF Timer No Preference |ASAP Capable |ASAP |FTMs per Burst |Reserved |Format And Bandwidth |Burst Period |*/
    /*                                                                                                                     */
    /***********************************************************************************************************************/

    /* 封装 Fine Timing Measurement Parameters element  */
    pst_mac_ftmp = (mac_ftm_parameters_ie_stru *)&(puc_payload_addr[us_index]);

    pst_mac_ftmp->uc_eid = MAC_EID_FTMP;
    pst_mac_ftmp->uc_len = MAC_FTMP_LEN;

    pst_mac_ftmp->bit_status_indication = 0;                                              /* Reserved */
    pst_mac_ftmp->bit_value = 0;                                                          /* Reserved */

    /*回合个数(2^number_of_bursts_exponent)*/
    pst_mac_ftmp->bit_number_of_bursts_exponent = 0;
    while(us_burst_cnt >> 1)
    {
        pst_mac_ftmp->bit_number_of_bursts_exponent++;
        us_burst_cnt = us_burst_cnt >> 1;
    }

    pst_mac_ftmp->bit_burst_duration = 11;                                                 /* 128ms 暂时用最大的*/
    pst_mac_ftmp->us_partial_tsf_timer = 0;
    pst_mac_ftmp->bit_partial_tsf_timer_no_preference = 1;
    pst_mac_ftmp->bit_asap_capable = 1;
    pst_mac_ftmp->bit_asap = past_ftm_init[0].en_asap;

    /*获取带宽*/
    dmac_ftm_set_format_and_bandwidth(past_ftm_init[0].en_band_cap, past_ftm_init[0].en_prot_format, pst_mac_ftmp);
    /*每个burst发ftm帧的个数，ftm帧之间的间隔，以及burst的间隔*/
    if(past_ftm_init[0].uc_ftms_per_burst_cmd > 2)
    {
        pst_mac_ftmp->bit_ftms_per_burst = past_ftm_init[0].uc_ftms_per_burst_cmd;
        pst_mac_ftmp->uc_min_delta_ftm = 100;                                                   /* 单位100us*/
        pst_mac_ftmp->us_burst_period = past_ftm_init[0].uc_ftms_per_burst_cmd;             /* 单位100ms*/
    }
    else
    {
        pst_mac_ftmp->bit_ftms_per_burst = 2;
        pst_mac_ftmp->uc_min_delta_ftm = 100;                                                   /* 单位100us*/
        pst_mac_ftmp->us_burst_period = 1;                                                      /* 单位100ms*/
    }

    us_index = us_index + OAL_SIZEOF(mac_ftm_parameters_ie_stru);

    return (oal_uint16)(us_index + MAC_80211_FRAME_LEN);      /* [false alarm]:fortify误报,返回是无符号数  */
}


oal_uint32  dmac_sta_send_ftm_req(dmac_vap_stru *pst_dmac_vap)
{
    oal_netbuf_stru                    *pst_netbuf = OAL_PTR_NULL;
    mac_tx_ctl_stru                    *pst_tx_ctl;
    oal_uint32                          ul_ret;
    oal_uint16                          us_frame_len = 0;
    dmac_ftm_initiator_stru            *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    /* 申请管理帧内存 */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_SMGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_sta_send_ftm_req::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);
    oal_set_netbuf_prev(pst_netbuf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_netbuf, OAL_PTR_NULL);

    /* 封装iftmr帧 */
    us_frame_len = dmac_encap_ftm_req_mgmt(pst_dmac_vap, pst_netbuf);

    if (us_frame_len > WLAN_SMGMT_NETBUF_SIZE)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_sta_send_ftm_req:: probably memory used cross-border.}");
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    oal_netbuf_put(pst_netbuf, us_frame_len);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    OAL_MEMZERO(pst_tx_ctl, OAL_NETBUF_CB_SIZE());
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len;

    ul_ret = mac_vap_set_cb_tx_user_idx(&(pst_dmac_vap->st_vap_base_info), pst_tx_ctl, past_ftm_init[0].auc_bssid);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "(dmac_sta_send_ftm_req::fail to find user by xx:xx:xx:0x:0x:0x.}",
        past_ftm_init[0].auc_bssid[3],
        past_ftm_init[0].auc_bssid[4],
        past_ftm_init[0].auc_bssid[5]);
    }
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_MGMT;


    /* 调用发送管理帧接口 */
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_netbuf, us_frame_len);
    if (OAL_SUCC != ul_ret)
    {
        oal_netbuf_free(pst_netbuf);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint16  dmac_encap_ftm_mgmt(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buffer, oal_uint8 uc_session_id)
{
    oal_uint8                                 *puc_mac_header              = oal_netbuf_header(pst_buffer);
    oal_uint8                                 *puc_payload_addr            = mac_netbuf_get_payload(pst_buffer);
    mac_vap_stru                              *pst_mac_vap                 = &pst_dmac_vap->st_vap_base_info;
    oal_uint16                                 us_index                    = 0;
    mac_ftm_parameters_ie_stru                *pst_mac_ftmp;
    oal_uint16                                *pus_time_error;
    oal_uint32                                *pul_tsf;
    dmac_ftm_responder_stru                   *past_ftm_rsp                = pst_dmac_vap->pst_ftm->ast_ftm_rsp;
    oal_uint16                                 us_burst_cnt                = past_ftm_rsp[uc_session_id].us_burst_cnt;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/
    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* 设置地址1*/
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, past_ftm_rsp[uc_session_id].auc_mac_ra);

    /* 设置地址2为自己的MAC地址 */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    /* 地址3 bssid */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_dmac_vap->st_vap_base_info.auc_bssid);

    /***************************************************************************************************/
    /*                        Fine Timing Measurement frame format                                     */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Category |Public Action |Dialog Token |Follow Up Dialog Token |TOD |TOA |TOD Error |TOA Error |*/
    /* ----------------------------------------------------------------------------------------------- */
    /* |1        |1             |1            |1                      |6   |6   |2         |2         |*/
    /* ----------------------------------------------------------------------------------------------- */
    /* |LCI Report (optional) |                                                                        */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Location Civic MeasuremenRequest (optional) |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Fine Timing MeasuremenParameters (optional) |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /* |FTM Synchronization Information (optional) |                                                   */
    /* ----------------------------------------------------------------------------------------------- */
    /* |Variable                                    |                                                  */
    /* ----------------------------------------------------------------------------------------------- */
    /*                                                                                                 */
    /***************************************************************************************************/
    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_PUBLIC;           /* Category */
    puc_payload_addr[us_index++] = MAC_PUB_FTM;                          /* Public Action */

    puc_payload_addr[us_index++] = past_ftm_rsp[uc_session_id].uc_dialog_token;
    puc_payload_addr[us_index++] = past_ftm_rsp[uc_session_id].uc_follow_up_dialog_token;

    oal_memcopy(&puc_payload_addr[us_index], &past_ftm_rsp[uc_session_id].ull_tod, FTM_FRAME_TOD_LENGTH);
    us_index += FTM_FRAME_TOD_LENGTH;

    oal_memcopy(&puc_payload_addr[us_index], &past_ftm_rsp[uc_session_id].ull_toa, FTM_FRAME_TOA_LENGTH);
    us_index += FTM_FRAME_TOA_LENGTH;

    pus_time_error = (oal_uint16 *)&puc_payload_addr[us_index];
    *pus_time_error = past_ftm_rsp[uc_session_id].us_tod_error;
    us_index += FTM_FRAME_TOD_ERROR_LENGTH;

    pus_time_error = (oal_uint16 *)&puc_payload_addr[us_index];
    *pus_time_error = past_ftm_rsp[uc_session_id].us_toa_error;
    us_index += FTM_FRAME_TOA_ERROR_LENGTH;

    /*****************************************************************************************************/
    /*                   LCI Measurement Request (Measurement Request IE)                                */
    /* ------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |Measurement Token| Measurement Req Mode|Measurement Type  | Measurement Req |*/
    /* ------------------------------------------------------------------------------------------------- */
    /* |1          |1      | 1               | 1                   |1                  |var             |*/
    /* ------------------------------------------------------------------------------------------------- */
    /*                                                                                                   */
    /*****************************************************************************************************/

    /* 封装Measurement Request IE  */
    if(past_ftm_rsp[uc_session_id].en_lci_ie == OAL_TRUE)
    {
        puc_payload_addr[us_index++] = MAC_EID_MEASREQ;                   /* Element ID */
        puc_payload_addr[us_index++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        puc_payload_addr[us_index++] = 1;                                 /* Measurement Token */
        puc_payload_addr[us_index++] = 0;                                 /* Measurement Req Mode */
        puc_payload_addr[us_index++] = RM_RADIO_MEASUREMENT_LCI;          /* Measurement Type */

    }

    /* 封装Measurement Request IE  */
    if(past_ftm_rsp[uc_session_id].en_location_civic_ie == OAL_TRUE)
    {
        puc_payload_addr[us_index++] = MAC_EID_MEASREQ;                   /* Element ID */
        puc_payload_addr[us_index++] = MAC_MEASUREMENT_REQUEST_IE_OFFSET; /* Length */
        puc_payload_addr[us_index++] = 1;                                 /* Measurement Token */
        puc_payload_addr[us_index++] = 0;                                 /* Measurement Req Mode */
        puc_payload_addr[us_index++] = RM_RADIO_MEASUREMENT_LOCATION_CIVIC;    /* Measurement Type */
    }

    /*******************************************************************/
    /*                    Fine Timing Measurement Parameters element   */
    /* --------------------------------------------------------------- */
    /* |Element ID |Length |Fine Timing Measurement Parameters|        */
    /* --------------------------------------------------------------- */
    /* |1          |1      | 9                                |        */
    /* --------------------------------------------------------------- */
    /*                                                                 */
    /*******************************************************************/

    /***********************************************************************************************************************/
    /*                    Fine Timing Measurement Parameters                                                               */
    /* ------------------------------------------------------------------------------------------------------------------- */
    /* |B0             B1 |B2 B6 |B7       | B8                   B11 |B12        B15 |B16       B23 |B24           B39 |  */
    /* ------------------------------------------------------------------------------------------------------------------- */
    /* |Status Indication |Value |Reserved |Number of Bursts Exponent |Burst Duration |Min Delta FTM |Partial TSF Timer |  */
    /*                                                                                                                     */
    /* ------------------------------------------------------------------------------------------------------------------- */
    /* |B40                             |B41          |B42  |B43        B47 |B48  B49 |B50              B55 |B56      B71 |*/
    /* ------------------------------------------------------------------------------------------------------------------- */
    /* |Partial TSF Timer No Preference |ASAP Capable |ASAP |FTMs per Burst |Reserved |Format And Bandwidth |Burst Period |*/
    /*                                                                                                                     */
    /***********************************************************************************************************************/
    if(past_ftm_rsp[uc_session_id].en_ftm_parameters == OAL_TRUE)
    {
        /* 封装 Fine Timing Measurement Parameters element  */
        pst_mac_ftmp = (mac_ftm_parameters_ie_stru *)&(puc_payload_addr[us_index]);

        pst_mac_ftmp->uc_eid = MAC_EID_FTMP;
        pst_mac_ftmp->uc_len = MAC_FTMP_LEN;
        /*回合个数(2^number_of_bursts_exponent)*/
        pst_mac_ftmp->bit_number_of_bursts_exponent = 0;
        while(us_burst_cnt >> 1)
        {
            pst_mac_ftmp->bit_number_of_bursts_exponent++;
            us_burst_cnt = us_burst_cnt >> 1;
        }

        pst_mac_ftmp->bit_burst_duration = 11;                                                 /* 128ms 暂时用最大的*/
        pst_mac_ftmp->us_partial_tsf_timer = 0;
        pst_mac_ftmp->bit_partial_tsf_timer_no_preference = 1;
        pst_mac_ftmp->bit_asap_capable = 1;
        pst_mac_ftmp->bit_asap = past_ftm_rsp[uc_session_id].en_asap;
        /*设置带宽协议*/
        dmac_ftm_set_format_and_bandwidth(past_ftm_rsp[uc_session_id].en_band_cap,
                                          past_ftm_rsp[uc_session_id].uc_prot_format,
                                          pst_mac_ftmp);

        /*每个burst发ftm帧的个数，ftm帧之间的间隔，以及burst的间隔*/
        pst_mac_ftmp->bit_ftms_per_burst = past_ftm_rsp[uc_session_id].uc_ftms_per_burst_save;
        pst_mac_ftmp->uc_min_delta_ftm = past_ftm_rsp[uc_session_id].uc_min_delta_ftm;
        pst_mac_ftmp->us_burst_period = past_ftm_rsp[uc_session_id].us_burst_period;

        pst_mac_ftmp->bit_status_indication = OAL_TRUE;                                              /* Reserved */
        pst_mac_ftmp->bit_value = 0;                                                          /* Reserved */

        us_index = us_index + OAL_SIZEOF(mac_ftm_parameters_ie_stru);
    }

    /*************************************************************/
    /*         FTM Synchronization Information element           */
    /* --------------------------------------------------------- */
    /* |Element ID |Length |Element ID Extension | TSF Sync Info|*/
    /* --------------------------------------------------------- */
    /* |1          |1      | 1                   | 4            |*/
    /* --------------------------------------------------------- */
    /*                                                           */
    /*************************************************************/

    if(past_ftm_rsp[uc_session_id].en_ftm_synchronization_information == OAL_TRUE)
    {
        puc_payload_addr[us_index++] = MAC_EID_FTMSI;                   /* Element ID */
        puc_payload_addr[us_index++] = 5;                               /* Length */
        puc_payload_addr[us_index++] = MAC_EID_EXT_FTMSI;

        pul_tsf = (oal_uint32 *)&puc_payload_addr[us_index];
        *pul_tsf = past_ftm_rsp[uc_session_id].ul_tsf;
        us_index += 4;
    }

    return (oal_uint16)(us_index + MAC_80211_FRAME_LEN);      /* [false alarm]:fortify误报,返回是无符号数  */
}


oal_uint32  dmac_ftm_rsp_send_ftm(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_session_id)
{
    oal_netbuf_stru                    *pst_netbuf = OAL_PTR_NULL;
    mac_tx_ctl_stru                    *pst_tx_ctl;
    oal_uint32                          ul_ret;
    oal_uint16                          us_frame_len = 0;
    dmac_ftm_responder_stru            *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;

    /* 申请管理帧内存 */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_SMGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_send_ftm::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);
    oal_set_netbuf_prev(pst_netbuf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_netbuf, OAL_PTR_NULL);

    /* 封装iftmr帧 */
    us_frame_len = dmac_encap_ftm_mgmt(pst_dmac_vap, pst_netbuf, uc_session_id);

    if (us_frame_len > WLAN_SMGMT_NETBUF_SIZE)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_send_ftm:: probably memory used cross-border.}");
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    oal_netbuf_put(pst_netbuf, us_frame_len);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    OAL_MEMZERO(pst_tx_ctl, OAL_NETBUF_CB_SIZE());
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len;

    ul_ret = mac_vap_set_cb_tx_user_idx(&(pst_dmac_vap->st_vap_base_info), pst_tx_ctl, past_ftm_rsp[uc_session_id].auc_mac_ra);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "(dmac_send_ftm::fail to find user by xx:xx:xx:0x:0x:0x.}",
        past_ftm_rsp[uc_session_id].auc_mac_ra[3],
        past_ftm_rsp[uc_session_id].auc_mac_ra[4],
        past_ftm_rsp[uc_session_id].auc_mac_ra[5]);
    }

    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_MGMT;


    /* 调用发送管理帧接口 */
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_netbuf, us_frame_len);
    if (OAL_SUCC != ul_ret)
    {
        oal_netbuf_free(pst_netbuf);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint16  dmac_encap_ftm_trigger_mgmt(oal_void *pst_vap, oal_netbuf_stru *pst_buffer)
{
    oal_uint8                     *puc_mac_header          = oal_netbuf_header(pst_buffer);
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint8                     *puc_payload_addr        = oal_netbuf_data(pst_buffer);
#else
    oal_uint8                     *puc_payload_addr        = puc_mac_header + MAC_80211_FRAME_LEN;
#endif
    oal_uint16                     us_index                = 0;
    dmac_vap_stru                 *pst_dmac_vap            = (dmac_vap_stru *)pst_vap;
    mac_vap_stru                  *pst_mac_vap             = &pst_dmac_vap->st_vap_base_info;
    dmac_ftm_initiator_stru       *past_ftm_init           = pst_dmac_vap->pst_ftm->ast_ftm_init;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID)*/
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, past_ftm_init[0].auc_bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, past_ftm_init[0].auc_bssid);



    /*************************************************************************************/
    /*                       	FTM Request frame - Frame Body                           */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |Trigger |                                               */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |1       |                                               */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/

    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_PUBLIC;           /* Category */
    puc_payload_addr[us_index++] = MAC_PUB_FTM_REQ;                      /* Public Action */

    puc_payload_addr[us_index++] = 1;                                    /* Trigger */

    return (oal_uint16)(us_index + MAC_80211_FRAME_LEN);      /* [false alarm]:fortify误报,返回是无符号数  */
}


oal_void  dmac_set_ftm_burst_duration(dmac_vap_stru *pst_dmac_vap, mac_ftm_parameters_ie_stru *pst_mac_ftmp)
{

}


oal_void  dmac_get_ftm_burst_duration(dmac_vap_stru *pst_dmac_vap, mac_ftm_parameters_ie_stru *pst_mac_ftmp)
{
    /*-----------------------*/
    /*Value | Represents     */
    /*-----------------------*/
    /*  0-1 |   Reserved     */
    /*-----------------------*/
    /*  2   |   250 μs      */
    /*-----------------------*/
    /*  3   |   500 μs      */
    /*-----------------------*/
    /*  4   |   1 ms         */
    /*-----------------------*/
    /*  5   |   2 ms         */
    /*-----------------------*/
    /*  6   |   4 ms         */
    /*-----------------------*/
    /*  7   |   8 ms         */
    /*-----------------------*/
    /*  8   |   16 ms        */
    /*-----------------------*/
    /*  9   |   32 ms        */
    /*-----------------------*/
    /*  10  |   64 ms        */
    /*-----------------------*/
    /*  11  |   128 ms       */
    /*-----------------------*/
    /*  12–14| Reserved     */
    /*-----------------------*/
    /*  15    | No preference*/
    /*-----------------------*/
    dmac_ftm_initiator_stru            *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    if((pst_mac_ftmp->bit_burst_duration <= 11) && (pst_mac_ftmp->bit_burst_duration >= 4))
    {
        past_ftm_init[0].uc_burst_duration = (oal_uint8)BIT(pst_mac_ftmp->bit_burst_duration - 4);
    }
    else
    {
        past_ftm_init[0].uc_burst_duration = 15;
    }
}


oal_uint32  dmac_ftm_send_trigger(dmac_vap_stru *pst_dmac_vap)
{
    oal_netbuf_stru             *pst_mgmt_buf;
    mac_tx_ctl_stru             *pst_tx_ctl;
    oal_uint32                   ul_ret;
    oal_uint16                   us_mgmt_len;
    mac_vap_stru                *pst_mac_vap;
    dmac_ftm_initiator_stru     *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    pst_mac_vap = &pst_dmac_vap->st_vap_base_info;

    /* 申请管理帧内存 */
    pst_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_mgmt_buf)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_FTM, "{mac_ftm_send_trigger::alloc netbuf failed.}");
        OAL_MEM_INFO_PRINT(OAL_MEM_POOL_ID_NETBUF);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(pst_mgmt_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_mgmt_buf, OAL_PTR_NULL);

    OAL_MEM_NETBUF_TRACE(pst_mgmt_buf, OAL_TRUE);

    /* 封装ftm trigger帧 */
    us_mgmt_len = dmac_encap_ftm_trigger_mgmt(pst_mac_vap, pst_mgmt_buf);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_mgmt_buf);

    OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_MGMT;

    if (oal_memcmp(past_ftm_init[0].auc_bssid, BROADCAST_MACADDR, OAL_SIZEOF(BROADCAST_MACADDR)))
    {
        /* 发送单播探测帧 */
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) =(oal_uint8)pst_mac_vap->us_assoc_vap_id;
    }
    else
    {
        MAC_GET_CB_IS_MCAST(pst_tx_ctl) = OAL_TRUE;
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl) =(oal_uint8)pst_mac_vap->us_multi_user_idx; /* ftm trigger帧是广播帧 */
    }

    /* 调用发送管理帧接口 */
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, us_mgmt_len);
    if (OAL_SUCC != ul_ret)
    {
        oal_netbuf_free(pst_mgmt_buf);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint16  dmac_sta_ftm_bust_period_func(dmac_vap_stru *pst_dmac_vap)
{
    dmac_ftm_initiator_stru            *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    past_ftm_init[0].en_ftm_trigger = OAL_TRUE;

    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_sta_ftm_bust_period_func::us_burst_cnt [%d].}", past_ftm_init[0].us_burst_cnt);

    dmac_sta_start_scan_for_ftm(pst_dmac_vap, past_ftm_init[0].uc_burst_duration);

    if(0 < past_ftm_init[0].us_burst_cnt)
    {
        past_ftm_init[0].us_burst_cnt--;
    }

    return OAL_SUCC;
}



oal_uint32  dmac_sta_ftm_bust_period_timeout(void *p_arg)
{
    dmac_vap_stru             *pst_dmac_vap;
    dmac_ftm_initiator_stru   *past_ftm_init;

    if (OAL_PTR_NULL == p_arg)
    {
        OAM_ERROR_LOG0(0, OAM_SF_FTM, "{dmac_sta_ftm_bust_period_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)p_arg;
    past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    dmac_sta_ftm_bust_period_func(pst_dmac_vap);

    if(past_ftm_init[0].us_burst_cnt == 0)
    {
        FRW_TIMER_DESTROY_TIMER(&(past_ftm_init[0].st_ftm_tsf_timer));
        dmac_save_ftm_range(pst_dmac_vap);
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


oal_void  dmac_sta_ftm_start_bust(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint32                           ul_timeout = 0;
    dmac_ftm_initiator_stru             *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    ul_timeout = past_ftm_init[0].us_burst_period * 100;

    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                    "{dmac_sta_ftm_start_bust::ftm bust start sucess. us_burst_cnt[%d], ftms_per_burst[%d], ul_timeout[%d]}",
                    past_ftm_init[0].us_burst_cnt,
                    past_ftm_init[0].uc_ftms_per_burst,
                    ul_timeout);

    if((OAL_FALSE == past_ftm_init[0].st_ftm_tsf_timer.en_is_registerd) && (past_ftm_init[0].us_burst_cnt > 1))
    {
        FRW_TIMER_CREATE_TIMER(&(past_ftm_init[0].st_ftm_tsf_timer),
                               dmac_sta_ftm_bust_period_timeout,
                               ul_timeout,
                               (oal_void *)pst_dmac_vap,
                               OAL_TRUE,
                               OAM_MODULE_ID_DMAC,
                               pst_dmac_vap->st_vap_base_info.ul_core_id);

    }

    if(past_ftm_init[0].us_burst_cnt)
    {
        dmac_sta_ftm_bust_period_func(pst_dmac_vap);
    }
}


oal_void  dmac_sta_ftm_wait_start_burst(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint16                          ul_tsf_start_timeout;
    dmac_ftm_initiator_stru            *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    //待讨论wxf
    ul_tsf_start_timeout = past_ftm_init[0].us_partial_tsf_timer - (oal_uint16)(past_ftm_init[0].ul_tsf_sync_info >> 10);

    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_wait_start_burst::ftm wait start,tsf_start_timeout[%d].}", ul_tsf_start_timeout);

    if(ul_tsf_start_timeout)
    {
        if (OAL_FALSE == past_ftm_init[0].st_ftm_tsf_timer.en_is_registerd)
        {
            FRW_TIMER_CREATE_TIMER(&(past_ftm_init[0].st_ftm_tsf_timer),
                                   dmac_sta_ftm_wait_start_burst_timeout,
                                   ul_tsf_start_timeout,
                                   (oal_void *)pst_dmac_vap,
                                   OAL_FALSE,
                                   OAM_MODULE_ID_DMAC,
                                   pst_dmac_vap->st_vap_base_info.ul_core_id);
        }
    }
    else
    {
        dmac_sta_ftm_wait_start_burst_timeout((oal_void *)pst_dmac_vap);
    }

}


oal_uint32  dmac_sta_ftm_wait_start_burst_timeout(void *p_arg)
{
    dmac_vap_stru        *pst_dmac_vap;

    if (OAL_PTR_NULL == p_arg)
    {
        OAM_ERROR_LOG0(0, OAM_SF_FTM, "{dmac_sta_ftm_wait_start_burst_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)p_arg;

    dmac_sta_ftm_start_bust(pst_dmac_vap);

    return OAL_SUCC;
}


oal_void  dmac_save_error_ftm_session(dmac_vap_stru *pst_dmac_vap, mac_ftm_parameters_ie_stru *pst_mac_ftmp)
{
    ftm_range_report_stru               *pst_ftm_range_report;
    oal_uint8                            uc_index;
    dmac_ftm_initiator_stru             *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    pst_ftm_range_report = &past_ftm_init[0].st_ftm_range_report;
    uc_index =  pst_ftm_range_report->uc_error_entry_count;

    pst_ftm_range_report->ast_ftm_error_entry[uc_index].ul_measurement_start_time = past_ftm_init[0].ul_tsf_sync_info;
    oal_memcopy(pst_ftm_range_report->ast_ftm_error_entry[uc_index].auc_bssid, past_ftm_init[0].auc_bssid, OAL_SIZEOF(past_ftm_init[0].auc_bssid));
    pst_ftm_range_report->ast_ftm_error_entry[uc_index].uc_error_code = pst_mac_ftmp->bit_status_indication;

    pst_ftm_range_report->uc_error_entry_count++;

    dmac_rrm_meas_ftm(pst_dmac_vap);
}


oal_void  dmac_save_ftm_range(dmac_vap_stru *pst_dmac_vap)
{
    ftm_range_report_stru               *pst_ftm_range_report;
    oal_uint8                            uc_index;
    dmac_ftm_initiator_stru             *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    pst_ftm_range_report = &past_ftm_init[0].st_ftm_range_report;
    uc_index =  pst_ftm_range_report->uc_range_entry_count;

    pst_ftm_range_report->ast_ftm_range_entry[uc_index].ul_measurement_start_time = past_ftm_init[0].ul_tsf_sync_info;
    oal_memcopy(pst_ftm_range_report->ast_ftm_range_entry[uc_index].auc_bssid, past_ftm_init[0].auc_bssid, OAL_SIZEOF(past_ftm_init[0].auc_bssid));
    pst_ftm_range_report->ast_ftm_range_entry[uc_index].bit_range = past_ftm_init[0].bit_range;
    pst_ftm_range_report->ast_ftm_range_entry[uc_index].bit_max_range_error_exponent = past_ftm_init[0].bit_max_range_rrror_rxponent;

    pst_ftm_range_report->uc_range_entry_count++;

    dmac_rrm_meas_ftm(pst_dmac_vap);
}


oal_void  dmac_ftm_get_cali(dmac_vap_stru *pst_dmac_vap)
{
    oal_uint32                     ul_ftm_cali_rx_time = 0;
    oal_uint32                     ul_ftm_cali_tx_time = 0;
    oal_uint32                     ul_ftm_cali_rx_intp_time = 0;
    oal_int8                       c_time_intp = 0;
    dmac_ftm_initiator_stru       *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    hal_get_ftm_cali_rx_time(pst_dmac_vap->pst_hal_device, &ul_ftm_cali_rx_time);
    hal_get_ftm_cali_tx_time(pst_dmac_vap->pst_hal_device, &ul_ftm_cali_tx_time);

    hal_get_ftm_cali_rx_intp_time(pst_dmac_vap->pst_hal_device, &ul_ftm_cali_rx_intp_time);

    /*24:20 上报ftm time的插值结果,有符号*/
    if ((ul_ftm_cali_rx_intp_time&0x1F) >= 0x10)
    {
        c_time_intp = (oal_int8)((ul_ftm_cali_rx_intp_time&0x1F) - 32);
    }
    else
    {
        c_time_intp = (oal_int8)(ul_ftm_cali_rx_intp_time&0x1F);
    }

    ul_ftm_cali_rx_time = (oal_uint32)((oal_int32)(ul_ftm_cali_rx_time >> 1)*12 - c_time_intp);
    ul_ftm_cali_tx_time = (oal_uint32)(ul_ftm_cali_tx_time*6);

    past_ftm_init[0].ul_ftm_cali_time = ul_ftm_cali_rx_time - ul_ftm_cali_tx_time;

    OAM_WARNING_LOG2(0, OAM_SF_ANY,
                   "{dmac_ftm_get_cali::wxf get ul_ftm_cali_time = [%d], c_time_intp[%d].}",
                   past_ftm_init[0].ul_ftm_cali_time,
                   c_time_intp);
}


oal_uint32  dmac_sta_rx_ftm1(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_data, oal_uint16 us_frame_len)
{
    oal_uint8                      *puc_ie;
    mac_ftm_parameters_ie_stru     *pst_mac_ftmp;
    dmac_ftm_initiator_stru        *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    puc_ie = mac_find_ie_ext_ie(MAC_EID_FTMSI, MAC_EID_EXT_FTMSI, &puc_data[FTM_FRAME_OPTIONAL_IE_OFFSET], us_frame_len - FTM_FRAME_OPTIONAL_IE_OFFSET);
    if(puc_ie != OAL_PTR_NULL)
    {
        past_ftm_init[0].ul_tsf_sync_info = *(oal_uint32 *)&puc_ie[FTM_FRAME_TSF_SYNC_INFO_OFFSET];
    }

    puc_ie = mac_find_ie(MAC_EID_FTMP, &puc_data[FTM_FRAME_OPTIONAL_IE_OFFSET], us_frame_len - FTM_FRAME_OPTIONAL_IE_OFFSET);
    if(puc_ie != OAL_PTR_NULL)
    {
        pst_mac_ftmp = (mac_ftm_parameters_ie_stru *)puc_ie;

        if(pst_mac_ftmp->bit_status_indication != OAL_TRUE)
        {
            OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_sta_rx_ftm1::ftm session ends,status_indication[%d]!}", pst_mac_ftmp->bit_status_indication);
            dmac_save_error_ftm_session(pst_dmac_vap, pst_mac_ftmp);
            return OAL_FALSE;
        }

        /*回合个数(2^number_of_bursts_exponent)*/
        if(pst_mac_ftmp->bit_number_of_bursts_exponent >= 15)
        {
            pst_mac_ftmp->bit_number_of_bursts_exponent = 0;
        }
        past_ftm_init[0].us_burst_cnt = BIT(pst_mac_ftmp->bit_number_of_bursts_exponent);

        /*每个回合FTM帧的个数*/
        if(pst_mac_ftmp->bit_ftms_per_burst > 2)
        {
            past_ftm_init[0].uc_ftms_per_burst = pst_mac_ftmp->bit_ftms_per_burst;
        }
        else
        {
            past_ftm_init[0].uc_ftms_per_burst = 2;
        }

        /*回合持续时间*/
        dmac_get_ftm_burst_duration(pst_dmac_vap, pst_mac_ftmp);

        /*回合之间的间隔*/
        if(pst_mac_ftmp->us_burst_period)
        {
            past_ftm_init[0].us_burst_period = pst_mac_ftmp->us_burst_period;
        }
        else
        {
            past_ftm_init[0].us_burst_period = 2; //默认200ms
        }

        if(!pst_mac_ftmp->bit_partial_tsf_timer_no_preference)
        {
            past_ftm_init[0].us_partial_tsf_timer = pst_mac_ftmp->us_partial_tsf_timer;
        }

        dmac_ftm_get_format_and_bandwidth(&past_ftm_init[0].en_band_cap, &past_ftm_init[0].en_prot_format, pst_mac_ftmp);

        OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                 "{dmac_ap_up_rx_ftm_req:: us_burst_cnt[%d], uc_ftms_per_burst[%d], us_burst_period[%d]100ms.}",
                 past_ftm_init[0].us_burst_cnt,
                 past_ftm_init[0].uc_ftms_per_burst,
                 past_ftm_init[0].us_burst_period);
    }
    /*else
    {
         OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_sta_rx_ftm1::ftm1 is error}");
         dmac_save_error_ftm_session(pst_dmac_vap, pst_mac_ftmp);
         return OAL_FALSE;
    }*/

    return OAL_SUCC;

}


oal_void  dmac_sta_rx_ftmk(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_data, oal_uint16 us_frame_len)
{
    oal_uint64                      ull_distance_rst;
    oal_uint32                      ul_ret;
    mac_device_stru                *pst_mac_dev;
    oal_uint8                       uc_index = 0;
    dmac_ftm_initiator_stru        *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    pst_mac_dev = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if(OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_sta_rx_ftmk::pst_mac_dev null.}");
        return ;
    }

    /********************************************************************************************/
    /*            Fine Timing Measurement frame format                                          */
    /* -----------------------------------------------------------------------------------------*/
    /* |Category|Public Action|Dialog Token|Follow Up Dialog Token|TOD |TOA|TOD Error|TOA Error|*/
    /* -----------------------------------------------------------------------------------------*/
    /* | 1      |1            |1           |1                     |6   |6  |2        |2        |*/
    /* -----------------------------------------------------------------------------------------*/
    /*                                                                                          */
    /********************************************************************************************/

    /* follow up dialog token不为0且与dialog token对应时说明不是一次会话中的第一帧，帧中带有tod和toa，可以进行距离计算 */
    if(puc_data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET] != past_ftm_init[0].uc_dialog_token)
    {
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                         "dmac_sta_rx_ftmk::error! follow up [%d] should be previous dialog token[%d]!!!",
                         puc_data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET],
                         past_ftm_init[0].uc_dialog_token);

    }

    /* 记录dialog token 对应的时间索引*/
    /* ftm帧记录的t1 和t4为上一次的时间*/
    past_ftm_init[0].uc_dialog_token = puc_data[FTM_FRAME_DIALOG_TOKEN_OFFSET];
    past_ftm_init[0].uc_follow_up_dialog_token = puc_data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET];
    uc_index = past_ftm_init[0].uc_follow_up_dialog_token % MAC_FTM_TIMER_CNT;

    /* 记录tod(t1)*/
    oal_memcopy(&(past_ftm_init[0].ast_ftm_timer[uc_index].ull_t1),&puc_data[FTM_FRAME_TOD_OFFSET],6);
    past_ftm_init[0].ast_ftm_timer[uc_index].ull_t1 &= FTM_TMIE_MASK;

    /* 记录toa(t4)*/
    oal_memcopy(&(past_ftm_init[0].ast_ftm_timer[uc_index].ull_t4),&puc_data[FTM_FRAME_TOA_OFFSET],6);
    past_ftm_init[0].ast_ftm_timer[uc_index].ull_t4 &= FTM_TMIE_MASK;

    OAM_WARNING_LOG2(0, OAM_SF_ANY,
                   "{dmac_sta_rx_ftmk::wxf get t4 - t1 = [%d],uc_follow_up_dialog_token =[%d].}",
                   past_ftm_init[0].ast_ftm_timer[uc_index].ull_t4 - past_ftm_init[0].ast_ftm_timer[uc_index].ull_t1,
                   past_ftm_init[0].uc_follow_up_dialog_token);

    /*获取该STAUT的差值 */
    dmac_ftm_get_cali(pst_dmac_vap);

    /* 帧中带有tod和toa，可以进行距离计算 */
    ul_ret = dmac_ftm_get_distance(pst_dmac_vap, &ull_distance_rst);
    if(OAL_TRUE == ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "dmac_sta_rx_ftmk::distance = %d",ull_distance_rst);
        if(past_ftm_init[0].bit_range)
        {
            past_ftm_init[0].bit_range = (past_ftm_init[0].bit_range + ull_distance_rst) >> 1;
        }
        else
        {
            past_ftm_init[0].bit_range |= ull_distance_rst;
        }

        //误差
        //pst_dmac_vap->st_ftm.bit_max_range_rrror_rxponent = 14;
    }
    else
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "dmac_sta_rx_ftmk::can't get distance, return error");
    }

    return;

}


oal_uint32  dmac_sta_rx_ftm(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf)
{
    mac_device_stru                *pst_mac_dev;
    //oal_uint32                      ul_ret;
    dmac_rx_ctl_stru               *pst_rx_ctl;
    mac_rx_ctl_stru                *pst_rx_info;
    mac_ieee80211_frame_stru       *pst_frame_hdr;
    oal_uint8                      *puc_data;
    oal_uint16                      us_frame_len;
    oal_uint8                       auc_bssid[WLAN_MAC_ADDR_LEN] ={0};
    dmac_ftm_initiator_stru        *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    if(OAL_UNLIKELY((OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_netbuf)))
     {
         OAM_ERROR_LOG2(0, OAM_SF_FTM, "{dmac_sta_rx_ftm::param null!,pst_dmac_vap[%p],pst_netbuf[%p]", pst_dmac_vap, pst_netbuf);
         return OAL_ERR_CODE_PTR_NULL;
     }

    if(OAL_FALSE == past_ftm_init[0].en_ftm_initiator)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_sta_rx_ftm::not a ftm_initiator, ignored rx ftm.}");
        return OAL_SUCC;
    }

    pst_rx_ctl     = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_rx_info    = (mac_rx_ctl_stru *)(&(pst_rx_ctl->st_rx_info));

    /* 获取帧头信息 */
    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctl->st_rx_info));

    /* 获取帧体指针 */
    puc_data = MAC_GET_RX_PAYLOAD_ADDR(pst_rx_info,pst_netbuf);
    us_frame_len = MAC_GET_RX_CB_PAYLOAD_LEN(pst_rx_info);  /*帧体长度*/

    mac_get_address3((oal_uint8 *)pst_frame_hdr, auc_bssid);

    if(oal_memcmp(auc_bssid, past_ftm_init[0].auc_bssid, OAL_SIZEOF(auc_bssid)) != 0)
    {
        /*只处理指定BSSID的FTM帧*/
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
               "{dmac_sta_rx_ftm:: FTM's bssid[xx:xx:xx:xx:%d:%d] differ of [xx:xx:xx:xx:%d:%d].}",
               auc_bssid[4], auc_bssid[5],
               past_ftm_init[0].auc_bssid[4], past_ftm_init[0].auc_bssid[5]);

        return OAL_SUCC;
    }

    pst_mac_dev = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if(OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_sta_rx_ftm::pst_mac_dev null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "dmac_sta_rx_ftm::uc_dialog_token[%d], follow_up[%d]",
                     puc_data[FTM_FRAME_DIALOG_TOKEN_OFFSET],
                     puc_data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET]);

    /* 在FTM信道收到了正确的FTM帧:*/
    if(past_ftm_init[0].en_asap == OAL_FALSE)
    {
        /* 如果是NON_ASAP的情况则:1.收到的是FTM_1，则需要打断FTM扫描，切回工作信道等待回合开始*/
        /*                        2.收到的是FTM_K(K>1)*/
        if(0 == puc_data[FTM_FRAME_FOLLOWUP_DIALOG_TOKEN_OFFSET])
        {
            /*收到的是FTM_1*/
            /*记录dialog token */
            past_ftm_init[0].uc_dialog_token = puc_data[FTM_FRAME_DIALOG_TOKEN_OFFSET];

            OAL_MEMZERO(&(past_ftm_init[0].ast_ftm_timer), OAL_SIZEOF(past_ftm_init[0].ast_ftm_timer));

            dmac_scan_abort(pst_mac_dev);

            /*初始化保存ftm测量结构体*/
            if(OAL_SUCC == dmac_sta_rx_ftm1(pst_dmac_vap, puc_data, us_frame_len)) //多次ftm1?
            {
                OAL_MEMZERO(&past_ftm_init[0].st_ftm_range_report, OAL_SIZEOF(past_ftm_init[0].st_ftm_range_report));
                past_ftm_init[0].bit_range = 0;
                dmac_sta_ftm_wait_start_burst(pst_dmac_vap);
            }
        }
    }

    dmac_sta_rx_ftmk(pst_dmac_vap, puc_data, us_frame_len);

    return OAL_SUCC;
}


oal_uint32  dmac_ftm_wait_send_iftmr_timeout(oal_void *p_arg)
{
    dmac_vap_stru             *pst_dmac_vap;
    dmac_ftm_initiator_stru   *past_ftm_init;

    if (OAL_PTR_NULL == p_arg)
    {
        OAM_ERROR_LOG0(0, OAM_SF_FTM, "{dmac_ftm_wait_send_iftmr_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)p_arg;
    past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    if(OAL_TRUE == past_ftm_init[0].st_ftm_tsf_timer.en_is_registerd)
    {
        FRW_TIMER_DESTROY_TIMER(&(past_ftm_init[0].st_ftm_tsf_timer));
    }

    past_ftm_init[0].en_iftmr = OAL_TRUE;

    dmac_sta_start_scan_for_ftm(pst_dmac_vap, FTM_WAIT_TIMEOUT);

    return OAL_SUCC;
}


oal_uint32  dmac_ap_ftm_wait_send_ftm_timeout(oal_void *p_arg)
{
    dmac_vap_stru                       *pst_dmac_vap;
    ftm_timeout_arg_stru                *pst_arg = OAL_PTR_NULL;
    oal_uint8                            uc_session_id = 0;
    dmac_ftm_responder_stru             *past_ftm_rsp;

    if (OAL_PTR_NULL == p_arg)
    {
        OAM_ERROR_LOG0(0, OAM_SF_FTM, "{dmac_ap_ftm_wait_send_ftm_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_arg = (ftm_timeout_arg_stru *)p_arg;
    pst_dmac_vap = (dmac_vap_stru *)pst_arg->pst_dmac_vap;
    past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;
    uc_session_id = pst_arg->uc_session_id;

    dmac_ftm_rsp_set_parameter(pst_dmac_vap, uc_session_id);
    dmac_ftm_rsp_send_ftm(pst_dmac_vap, uc_session_id);

    past_ftm_rsp[uc_session_id].uc_ftms_per_burst -= 1;
    if(past_ftm_rsp[uc_session_id].uc_ftms_per_burst > 0)
    {
        FRW_TIMER_CREATE_TIMER(&(past_ftm_rsp[uc_session_id].st_ftm_tsf_timer),
                               dmac_ap_ftm_wait_send_ftm_timeout,
                               past_ftm_rsp[uc_session_id].uc_min_delta_ftm / 10,
                               (oal_void *)pst_arg,
                               OAL_FALSE,
                               OAM_MODULE_ID_DMAC,
                               pst_dmac_vap->st_vap_base_info.ul_core_id);
    }

    return OAL_SUCC;
}


oal_void  dmac_ftm_wait_send_iftmr(dmac_vap_stru *pst_dmac_vap, oal_uint16 us_start_delay)
{
    dmac_ftm_initiator_stru            *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_wait_send_iftmr::ftm wait send iftmr, us_start_delay[%d].}", us_start_delay);

    if (OAL_FALSE == past_ftm_init[0].st_ftm_tsf_timer.en_is_registerd)
    {
        FRW_TIMER_CREATE_TIMER(&(past_ftm_init[0].st_ftm_tsf_timer),
                               dmac_ftm_wait_send_iftmr_timeout,
                               us_start_delay,
                               (oal_void *)pst_dmac_vap,
                               OAL_TRUE,
                               OAM_MODULE_ID_DMAC,
                               pst_dmac_vap->st_vap_base_info.ul_core_id);

    }

}


oal_uint32  dmac_ftm_rsp_set_parameter(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_session_id)
{
    oal_uint8                       uc_index = 0;
    dmac_ftm_responder_stru        *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;
    ftm_timer_stru                 *past_ftm_timer = past_ftm_rsp[uc_session_id].ast_ftm_timer;

    past_ftm_rsp[uc_session_id].uc_follow_up_dialog_token = past_ftm_rsp[uc_session_id].uc_dialog_token;
    past_ftm_rsp[uc_session_id].uc_dialog_token ++;

    uc_index = past_ftm_rsp[uc_session_id].uc_follow_up_dialog_token % MAC_FTM_TIMER_CNT;

    /*检测对应的索引的uc_dialog_token是否一致，防止已被覆盖*/
    if (past_ftm_timer[uc_index].uc_dialog_token != past_ftm_rsp[uc_session_id].uc_follow_up_dialog_token)
    {
        OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                        "{dmac_ftm_rsp_set_parameter:: devic_dialog_token[%d]!=resp_follow_updialog_token[%d]!!!!.}",
                        past_ftm_timer[uc_index].uc_dialog_token,
                        past_ftm_rsp[uc_session_id].uc_follow_up_dialog_token);

        past_ftm_rsp[uc_session_id].ull_tod = 0;
        past_ftm_rsp[uc_session_id].ull_toa = 0;
    }
    else
    {
        /*根据每个回合帧数发送FTM*/
        past_ftm_rsp[uc_session_id].ull_tod = past_ftm_timer[uc_index].ull_t1;
        past_ftm_rsp[uc_session_id].ull_toa = past_ftm_timer[uc_index].ull_t4;

    }

    /*防止:t1 t4没有刷新，下一帧重用t1 t4*/
    past_ftm_timer[uc_index].ull_t1 = 0;
    past_ftm_timer[uc_index].ull_t4 = 0;

    /* 不携带 ie*/
    past_ftm_rsp[uc_session_id].en_lci_ie = OAL_FALSE;
    past_ftm_rsp[uc_session_id].en_location_civic_ie = OAL_FALSE;
    past_ftm_rsp[uc_session_id].en_ftm_parameters = OAL_TRUE;
    past_ftm_rsp[uc_session_id].en_ftm_synchronization_information = OAL_FALSE;

    return OAL_SUCC;
}



oal_void  dmac_ftm_rsp_wait_send_ftm(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_session_id)
{
    dmac_ftm_responder_stru       *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;
    ftm_timeout_arg_stru          *pst_arg = &past_ftm_rsp[uc_session_id].st_arg;

    dmac_ftm_rsp_set_parameter(pst_dmac_vap, uc_session_id);

    if (past_ftm_rsp[uc_session_id].uc_ftms_per_burst > 0)
    {
        dmac_ftm_rsp_send_ftm(pst_dmac_vap, uc_session_id);
        past_ftm_rsp[uc_session_id].uc_ftms_per_burst = past_ftm_rsp[uc_session_id].uc_ftms_per_burst - 1;
    }

    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "{dmac_ftm_rsp_wait_send_ftm::ftm wait send iftmr, time is [%d].}",
                     past_ftm_rsp[uc_session_id].uc_min_delta_ftm / 10);

    pst_arg->pst_dmac_vap = (oal_void *)pst_dmac_vap;
    pst_arg->uc_session_id = uc_session_id;

    if (OAL_FALSE == past_ftm_rsp[uc_session_id].st_ftm_tsf_timer.en_is_registerd)
    {
        FRW_TIMER_CREATE_TIMER(&(past_ftm_rsp[uc_session_id].st_ftm_tsf_timer),
                               dmac_ap_ftm_wait_send_ftm_timeout,
                               past_ftm_rsp[uc_session_id].uc_min_delta_ftm / 10,
                               (oal_void *)pst_arg,
                               OAL_FALSE,
                               OAM_MODULE_ID_DMAC,
                               pst_dmac_vap->st_vap_base_info.ul_core_id);

    }

}


oal_uint32  dmac_ftm_rsp_wait_end_ftm_timeout(oal_void *p_arg)
{
    dmac_vap_stru                       *pst_dmac_vap = OAL_PTR_NULL;
    ftm_timeout_arg_stru                *pst_arg = OAL_PTR_NULL;
    oal_uint8                            uc_session_id = 0;
    dmac_ftm_responder_stru             *past_ftm_rsp;

    if (OAL_PTR_NULL == p_arg)
    {
        OAM_ERROR_LOG0(0, OAM_SF_FTM, "{dmac_ftm_rsp_wait_end_ftm_timeout::p_arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_arg = (ftm_timeout_arg_stru *)p_arg;
    pst_dmac_vap = (dmac_vap_stru *)(pst_arg->pst_dmac_vap);
    past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;
    uc_session_id = pst_arg->uc_session_id;

    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_FTM, "{dmac_ftm_rsp_wait_end_ftm_timeout::p_arg error.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "{dmac_ftm_rsp_wait_end_ftm_timeout::end ftm, uc_session_id[%d].}",
                     uc_session_id);

    past_ftm_rsp[uc_session_id].en_received_iftmr = OAL_FALSE;

    FRW_TIMER_DESTROY_TIMER(&(past_ftm_rsp[uc_session_id].st_ftm_all_burst));

    /*结束ftm*/
    OAL_MEMZERO(&(past_ftm_rsp[uc_session_id]), OAL_SIZEOF(dmac_ftm_responder_stru));

    return OAL_SUCC;
}


oal_void  dmac_ftm_rsp_wait_end_ftm(dmac_vap_stru *pst_dmac_vap, oal_uint8 uc_session_id)
{
    oal_uint32                           ul_timeout = 0;
    dmac_ftm_responder_stru             *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;
    ftm_timeout_arg_stru                *pst_arg = &past_ftm_rsp[uc_session_id].st_arg;

    ul_timeout = past_ftm_rsp[uc_session_id].us_burst_cnt * past_ftm_rsp[uc_session_id].us_burst_period * 100;
    OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                     "{dmac_ftm_rsp_wait_end_ftm::ul_timeout[%d], uc_session_id[%d].}",
                     ul_timeout,
                     uc_session_id);

    pst_arg->pst_dmac_vap = (oal_void *)pst_dmac_vap;
    pst_arg->uc_session_id = uc_session_id;

    FRW_TIMER_CREATE_TIMER(&(past_ftm_rsp[uc_session_id].st_ftm_all_burst),
                           dmac_ftm_rsp_wait_end_ftm_timeout,
                           ul_timeout,
                           (oal_void *)pst_arg,
                           OAL_FALSE,
                           OAM_MODULE_ID_DMAC,
                           pst_dmac_vap->st_vap_base_info.ul_core_id);


}


oal_uint16  dmac_encap_ftm_range_report(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_buffer)
{
    oal_uint8                     *puc_mac_header          = oal_netbuf_header(pst_buffer);
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint8                     *puc_payload_addr        = oal_netbuf_data(pst_buffer);
#else
    oal_uint8                     *puc_payload_addr        = puc_mac_header + MAC_80211_FRAME_LEN;
#endif

    oal_uint8                     *puc_data;
    oal_uint16                     us_index        = 0;
    oal_uint16                     us_index_temp   = 0;
    mac_vap_stru                  *pst_mac_vap     = &pst_dmac_vap->st_vap_base_info;

    mac_meas_rpt_ie_stru          *pst_meas_rpt_ie;
    dmac_ftm_initiator_stru       *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;
    ftm_range_report_stru         *pst_ftm_range_report = &past_ftm_init[0].st_ftm_range_report;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /* 设置 Frame Control field */
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID)*/
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_mac_vap->auc_bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    /*************************************************************************************/
    /*                     Radio Measurement Report frame format                         */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Radio Measurement Action |Dialog Token | Measurement Report Elements|  */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1                       |1             |Variable                    |  */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/

    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_RADIO_MEASURMENT;    /* Category */
    puc_payload_addr[us_index++] = MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT;  /* Public Action */

    puc_payload_addr[us_index++] = 1;                                       /* Trigger */

    /*************************************************************************/
    /*                   Measurement Report IE - Frame Body         */
    /* --------------------------------------------------------------------- */
    /* |Element ID |Length |Meas Token| Meas Rpt Mode | Meas Type | Meas Rpt|*/
    /* --------------------------------------------------------------------- */
    /* |1          |1      | 1        |  1            | 1         | var      */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    pst_meas_rpt_ie = (mac_meas_rpt_ie_stru *)&puc_payload_addr[us_index];

    pst_meas_rpt_ie->uc_eid       = MAC_EID_MEASREP;
    pst_meas_rpt_ie->uc_token     = 1;
    pst_meas_rpt_ie->uc_rpttype   = RM_RADIO_MEASUREMENT_FTM_RANGE;
    //pst_meas_rpt_ie->st_rptmode   = 0;

    /******************************************************************************************/
    /*                  Fine Timing Measurement Range report                                  */
    /* ---------------------------------------------------------------------------------------*/
    /* |Range Entry Count |Range Entry |Error Entry Count| Error Entry |Optional Subelements |*/
    /* ---------------------------------------------------------------------------------------*/
    /* |1                 |M x 15      |1                | N x 11      | var                 |*/
    /* ---------------------------------------------------------------------------------------*/
    /*                                                                                        */
    /******************************************************************************************/
    puc_data = pst_meas_rpt_ie->auc_meas_rpt;
    /******************************************************************************/
    /*                   Range Entry                                              */
    /* ---------------------------------------------------------------------------*/
    /* |Measurement Start Time |BSSID |Range |Max Range Error Exponent |Reserved |*/
    /* ---------------------------------------------------------------------------*/
    /* |4                      |6     |3     |  1                     | 1        |*/
    /* ---------------------------------------------------------------------------*/
    /*                                                                            */
    /******************************************************************************/
    puc_data[us_index_temp++] = pst_ftm_range_report->uc_range_entry_count;
    oal_memcopy(&puc_data[us_index_temp], pst_ftm_range_report->ast_ftm_range_entry, pst_ftm_range_report->uc_range_entry_count * OAL_SIZEOF(ftm_range_entry_stru));
    us_index_temp += pst_ftm_range_report->uc_range_entry_count * OAL_SIZEOF(ftm_range_entry_stru);
    /***********************************************/
    /*                   Error Entry               */
    /* --------------------------------------------*/
    /* |Measurement Start Time |BSSID |Error Code |*/
    /* --------------------------------------------*/
    /* |4                      |6     |1          |*/
    /* --------------------------------------------*/
    /*                                             */
    /***********************************************/
    puc_data[us_index_temp++] = pst_ftm_range_report->uc_error_entry_count;
    oal_memcopy(&puc_data[us_index_temp], pst_ftm_range_report->ast_ftm_error_entry, pst_ftm_range_report->uc_error_entry_count * OAL_SIZEOF(ftm_error_entry_stru));
    us_index_temp += pst_ftm_range_report->uc_error_entry_count * OAL_SIZEOF(ftm_error_entry_stru);

    pst_meas_rpt_ie->uc_len = 3 + (oal_uint8)us_index_temp;

    us_index += (2 + pst_meas_rpt_ie->uc_len);

    return (oal_uint16)(us_index + MAC_80211_FRAME_LEN);      /* [false alarm]:fortify误报,返回是无符号数  */
}


oal_uint32  dmac_send_ftm_range_report(dmac_vap_stru *pst_dmac_vap)
{
    oal_netbuf_stru                    *pst_netbuf = OAL_PTR_NULL;
    mac_tx_ctl_stru                    *pst_tx_ctl;
    oal_uint32                          ul_ret;
    oal_uint16                          us_frame_len = 0;

    /* 申请管理帧内存 */
    pst_netbuf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_MEM_NETBUF_SIZE2, OAL_NETBUF_PRIORITY_MID);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_send_ftm_range_report::pst_netbuf null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEM_NETBUF_TRACE(pst_netbuf, OAL_TRUE);
    oal_set_netbuf_prev(pst_netbuf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_netbuf, OAL_PTR_NULL);

    /* 封装ftm range report帧 */
    us_frame_len = dmac_encap_ftm_range_report(pst_dmac_vap, pst_netbuf);

    oal_netbuf_put(pst_netbuf, us_frame_len);

    /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    OAL_MEMZERO(pst_tx_ctl, OAL_NETBUF_CB_SIZE());
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl) = us_frame_len;

    ul_ret = mac_vap_set_cb_tx_user_idx(&(pst_dmac_vap->st_vap_base_info), pst_tx_ctl, pst_dmac_vap->st_vap_base_info.auc_bssid);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "(dmac_send_ftm_range_report::fail to find user by xx:xx:xx:0x:0x:0x.}",
        pst_dmac_vap->st_vap_base_info.auc_bssid[3],
        pst_dmac_vap->st_vap_base_info.auc_bssid[4],
        pst_dmac_vap->st_vap_base_info.auc_bssid[5]);
    }
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_MGMT;


    /* 调用发送管理帧接口 */
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_netbuf, us_frame_len);
    if (OAL_SUCC != ul_ret)
    {
        oal_netbuf_free(pst_netbuf);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_void dmac_rrm_meas_ftm(dmac_vap_stru *pst_dmac_vap)
{
    ftm_range_request_stru              *pst_ftm_range_request;
    oal_uint8                            uc_ap_count;
    oal_uint8                           *puc_bssi;
    oal_uint8                            uc_channel;
    oal_uint16                           us_start_delay;
    dmac_ftm_initiator_stru             *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;
    //oal_uint8                            uc_index = 0;

    if(past_ftm_init[0].en_report_range == OAL_FALSE)
    {
        return;
    }

    pst_ftm_range_request = &past_ftm_init[0].st_ftm_range_request;
    uc_ap_count = past_ftm_init[0].uc_ftm_entry_count;
    us_start_delay = pst_ftm_range_request->us_start_delay;

    if(uc_ap_count)
    {
        past_ftm_init[0].uc_ftm_entry_count--;

        puc_bssi = pst_ftm_range_request->auc_bssid[uc_ap_count-1];
        uc_channel = pst_ftm_range_request->auc_channel[uc_ap_count-1];

        if(uc_channel <= MAC_CHANNEL_FREQ_2_BUTT)
        {
            past_ftm_init[0].st_channel_ftm.en_band = WLAN_BAND_2G;
            past_ftm_init[0].st_channel_ftm.en_bandwidth = WLAN_BAND_WIDTH_20M;
        }
        else
        {
            past_ftm_init[0].st_channel_ftm.en_band = WLAN_BAND_5G;
            past_ftm_init[0].st_channel_ftm.en_bandwidth = WLAN_BAND_WIDTH_40PLUS;
        }

        past_ftm_init[0].st_channel_ftm.uc_chan_number = uc_channel;

        mac_get_channel_idx_from_num(past_ftm_init[0].st_channel_ftm.en_band,
                                     past_ftm_init[0].st_channel_ftm.uc_chan_number,
                                     &past_ftm_init[0].st_channel_ftm.uc_chan_idx);

        oal_memcopy(past_ftm_init[0].auc_bssid, puc_bssi, OAL_SIZEOF(past_ftm_init[0].auc_bssid));

        past_ftm_init[0].en_lci_ie = OAL_FALSE;
        past_ftm_init[0].en_location_civic_ie = OAL_FALSE;

        past_ftm_init[0].us_burst_cnt = 1;

        dmac_ftm_wait_send_iftmr(pst_dmac_vap, us_start_delay);

    }
    else
    {
        past_ftm_init[0].en_report_range = OAL_FALSE;
        //发送ftm range report
        dmac_send_ftm_range_report(pst_dmac_vap);
    }
}


oal_void dmac_rrm_parse_ftm_range_req(dmac_vap_stru *pst_dmac_vap, mac_meas_req_ie_stru  *pst_meas_req_ie)
{
    oal_uint8                           *puc_data;
    oal_uint8                           *puc_ie;
    oal_uint8                            uc_index = 0;
    oal_uint8                            uc_ap_cnt = 0;
    oal_uint8                            uc_ap_index = 0;
    oal_uint8                            us_len_find;
    mac_ftm_range_req_ie_stru           *pst_mac_ftm_range_req_ie_stru;
    mac_neighbor_rpt_ie_stru            *pst_neighbor_rpt_ie;
    dmac_ftm_initiator_stru             *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;

    if(OAL_FALSE == past_ftm_init[0].en_ftm_initiator)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_rrm_parse_ftm_range_req::not a ftm_initiator, ignored rx ftm.}");
        return;
    }

    pst_mac_ftm_range_req_ie_stru = (mac_ftm_range_req_ie_stru *)pst_meas_req_ie->auc_meas_req;

    /**************************************************************************/
    /*                 Fine Timing Measurement Range request                  */
    /* -----------------------------------------------------------------------*/
    /* |Randomization Interval|Minimum AP Count Number |FTM Range Subelements|*/
    /* -----------------------------------------------------------------------*/
    /* |2                     |1                       | variable            |*/
    /* -----------------------------------------------------------------------*/
    /*                                                                        */
    /**************************************************************************/

    past_ftm_init[0].st_ftm_range_request.us_start_delay = pst_mac_ftm_range_req_ie_stru->us_randomization_interval;

    past_ftm_init[0].st_ftm_range_request.uc_minimum_ap_count = pst_mac_ftm_range_req_ie_stru->uc_minimum_ap_count;

    /* 从Subelements中获取Maximum Age和Neighbor Report  */
    /*Subelement ID   Name             Extensible
      0–3            Reserved
      4               Maximum Age      Yes
      5-51            Reserved
      52              Neighbor Report  Subelements
      53–220         Reserved
      221             Vendor Specific
      222–255        Reserved*/

    /******************************************************************************************************************/
    /*                   Neighbor Report element                                                                      */
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |Element ID |Length |BSSID |BSSID Information |Operating Class |Channel Number |PHY Type |Optional Subelements|*/
    /* -------------------------------------------------------------------------------------------------------------- */
    /* |1          |1      |6     |4                 |1               |1             |1         |variable            |*/
    /* -------------------------------------------------------------------------------------------------------------- */
    /*                                                                                                                */
    /******************************************************************************************************************/
    puc_data = pst_mac_ftm_range_req_ie_stru->auc_ftm_range_subelements;

    us_len_find = (pst_meas_req_ie->uc_len - MAC_MEASUREMENT_REQUEST_IE_OFFSET) - FTM_RANGE_IE_OFFSET;

    for(uc_ap_cnt = 0; uc_ap_cnt < past_ftm_init[0].st_ftm_range_request.uc_minimum_ap_count; uc_ap_cnt++)
    {
        puc_ie =  mac_find_ie(MAC_EID_NEIGHBOR_REPORT, &puc_data[uc_index], us_len_find);
        if(puc_ie != OAL_PTR_NULL)
        {
            pst_neighbor_rpt_ie = (mac_neighbor_rpt_ie_stru *)puc_ie;

            oal_memcopy(past_ftm_init[0].st_ftm_range_request.auc_bssid[uc_ap_index++], pst_neighbor_rpt_ie->auc_bssid, WLAN_MAC_ADDR_LEN);

            past_ftm_init[0].st_ftm_range_request.auc_channel[uc_ap_index++] = pst_neighbor_rpt_ie->uc_channum;

            uc_index += (pst_neighbor_rpt_ie->uc_len + MAC_IE_HDR_LEN);
            us_len_find -= (pst_neighbor_rpt_ie->uc_len + MAC_IE_HDR_LEN);
        }
    }

    if(uc_ap_index < uc_ap_cnt)
    {
        past_ftm_init[0].st_ftm_range_request.uc_minimum_ap_count = uc_ap_index - 1;
    }

    past_ftm_init[0].uc_ftm_entry_count = past_ftm_init[0].st_ftm_range_request.uc_minimum_ap_count;

    past_ftm_init[0].en_report_range = OAL_TRUE;

    dmac_rrm_meas_ftm(pst_dmac_vap);
}


oal_void  dmac_ftm_rsp_rx_ftm_req(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf)
{
    dmac_rx_ctl_stru               *pst_rx_ctl;
    mac_rx_ctl_stru                *pst_rx_info;
    mac_ieee80211_frame_stru       *pst_frame_hdr;
    oal_uint8                      *puc_data        = OAL_PTR_NULL;;
    oal_uint8                      *puc_payload     = OAL_PTR_NULL;
    oal_uint16                      us_frame_len;
    oal_uint8                       auc_mac_ra[WLAN_MAC_ADDR_LEN] ={0};
    mac_ftm_parameters_ie_stru     *pst_mac_ftmp;
    oal_int8                        c_session_id = -1;
    oal_uint8                       uc_session_id = 0;
    dmac_ftm_responder_stru        *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;

    if(OAL_UNLIKELY((OAL_PTR_NULL == pst_dmac_vap) || (OAL_PTR_NULL == pst_netbuf)))
     {
         OAM_ERROR_LOG2(0, OAM_SF_FTM, "{dmac_ftm_rsp_rx_ftm_req::param null!,pst_dmac_vap[%p],pst_netbuf[%p]", pst_dmac_vap, pst_netbuf);
         return;
     }

    /*ftm responder*/
    if (OAL_FALSE == mac_mib_get_FineTimingMsmtRespActivated(&pst_dmac_vap->st_vap_base_info))
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_ftm_rsp_rx_ftm_req::not a ftm_responder, ignored rx ftm.}");
        return;
    }

    pst_rx_ctl     = (dmac_rx_ctl_stru *)oal_netbuf_cb(pst_netbuf);
    pst_rx_info    = (mac_rx_ctl_stru *)(&(pst_rx_ctl->st_rx_info));

    /* 获取帧头信息 */
    pst_frame_hdr = (mac_ieee80211_frame_stru *)mac_get_rx_cb_mac_hdr(&(pst_rx_ctl->st_rx_info));

    /* 获取帧体指针 */
    puc_payload = MAC_GET_RX_PAYLOAD_ADDR(pst_rx_info,pst_netbuf);
    us_frame_len = MAC_GET_RX_CB_PAYLOAD_LEN(pst_rx_info);  /*帧体长度*/

    mac_get_address2((oal_uint8 *)pst_frame_hdr, auc_mac_ra);

    /*查找session id*/
    c_session_id = dmac_ftm_find_session_index(pst_dmac_vap, MAC_FTM_RESPONDER_MODE, auc_mac_ra);
    if (c_session_id < 0)
    {
        return ;
    }

    uc_session_id = (oal_uint8)c_session_id;
    dmac_ftm_enable_session_index(pst_dmac_vap, MAC_FTM_RESPONDER_MODE, auc_mac_ra, uc_session_id);

    /*FTM Trigger*/
    if (past_ftm_rsp[uc_session_id].en_received_iftmr == OAL_TRUE)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_rsp_rx_ftm_req:: rx FTM Trigger.}");

        if (puc_payload[FTM_REQ_TRIGGER_OFFSET] != OAL_TRUE)
        {
            /*结束ftm*/
            OAL_MEMZERO(&(past_ftm_rsp[uc_session_id]), OAL_SIZEOF(dmac_ftm_responder_stru));
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                           "{dmac_ftm_rsp_rx_ftm_req:: end of sending ftm.}");
            return;
        }

        past_ftm_rsp[uc_session_id].uc_ftms_per_burst = past_ftm_rsp[uc_session_id].uc_ftms_per_burst_save;
        dmac_ftm_rsp_wait_send_ftm(pst_dmac_vap, uc_session_id);
    }

    /*iFTMR*/
    puc_data = puc_payload;
    pst_mac_ftmp = (mac_ftm_parameters_ie_stru *)mac_find_ie(MAC_EID_FTMP, &puc_data[FTM_FRAME_IE_OFFSET], (oal_int32)(us_frame_len - FTM_FRAME_IE_OFFSET));
    if (pst_mac_ftmp != OAL_PTR_NULL)
    {
        if (past_ftm_rsp[uc_session_id].en_received_iftmr == OAL_FALSE)
        {
            if (puc_payload[FTM_REQ_TRIGGER_OFFSET] != OAL_TRUE)
            {
                /*结束ftm*/
                OAL_MEMZERO(&(past_ftm_rsp[uc_session_id]), OAL_SIZEOF(dmac_ftm_responder_stru));
                OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                               "{dmac_ftm_rsp_rx_ftm_req:: end of sending ftm.}");
                return;
            }
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_ftm_rsp_rx_ftm_req:: rx iFTMr.}");

            past_ftm_rsp[uc_session_id].en_received_iftmr = OAL_TRUE;

            past_ftm_rsp[uc_session_id].uc_dialog_token = 0;
            past_ftm_rsp[uc_session_id].uc_follow_up_dialog_token = 0;

            /*回合个数(2^number_of_bursts_exponent)*/
            if(pst_mac_ftmp->bit_number_of_bursts_exponent >= 15)
            {
                pst_mac_ftmp->bit_number_of_bursts_exponent = 0;
            }
            past_ftm_rsp[uc_session_id].us_burst_cnt = BIT(pst_mac_ftmp->bit_number_of_bursts_exponent);
            /*每个回合FTM帧的个数*/
            if(pst_mac_ftmp->bit_ftms_per_burst > 2)
            {
                past_ftm_rsp[uc_session_id].uc_ftms_per_burst_save = pst_mac_ftmp->bit_ftms_per_burst;
            }
            else
            {
                past_ftm_rsp[uc_session_id].uc_ftms_per_burst_save = 2;
            }
            /*FTM帧的间隔时间*/
            if(pst_mac_ftmp->uc_min_delta_ftm)
            {
                past_ftm_rsp[uc_session_id].uc_min_delta_ftm = pst_mac_ftmp->uc_min_delta_ftm;
            }
            else
            {
                past_ftm_rsp[uc_session_id].uc_min_delta_ftm = 100;
            }
            /*回合间隔*/
            if(pst_mac_ftmp->us_burst_period)
            {
                past_ftm_rsp[uc_session_id].us_burst_period = pst_mac_ftmp->us_burst_period;
            }
            else
            {
                past_ftm_rsp[uc_session_id].us_burst_period = (past_ftm_rsp[uc_session_id].uc_ftms_per_burst_save * past_ftm_rsp[uc_session_id].uc_min_delta_ftm / 1000) + 2;
            }

            /*asap*/
            past_ftm_rsp[uc_session_id].en_asap = pst_mac_ftmp->bit_asap;

            dmac_ftm_get_format_and_bandwidth(&(past_ftm_rsp[uc_session_id].en_band_cap),
                                              &(past_ftm_rsp[uc_session_id].uc_prot_format),
                                              pst_mac_ftmp);

            OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                             "{dmac_ftm_rsp_rx_ftm_req:: us_burst_cnt[%d], uc_ftms_per_burst_save[%d], uc_min_delta_ftm[%d]0.1ms, us_burst_period[%d]100ms.}",
                             past_ftm_rsp[uc_session_id].us_burst_cnt,
                             past_ftm_rsp[uc_session_id].uc_ftms_per_burst_save,
                             past_ftm_rsp[uc_session_id].uc_min_delta_ftm,
                             past_ftm_rsp[uc_session_id].us_burst_period);


            dmac_ftm_rsp_wait_end_ftm(pst_dmac_vap, uc_session_id);

            if (past_ftm_rsp[uc_session_id].en_asap == OAL_FALSE)
            {
                OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_rsp_rx_ftm_req:: en_asap OAL_FALSE.}");
                /* 获取TSF*/
                hal_vap_tsf_get_32bit(pst_dmac_vap->pst_hal_vap, &(past_ftm_rsp[uc_session_id].ul_tsf));

                dmac_ftm_rsp_set_parameter(pst_dmac_vap, uc_session_id);
                past_ftm_rsp[uc_session_id].ull_tod = 0;
                past_ftm_rsp[uc_session_id].ull_toa = 0;
                past_ftm_rsp[uc_session_id].en_ftm_parameters = OAL_TRUE;

                dmac_ftm_rsp_send_ftm(pst_dmac_vap, uc_session_id);
            }
            else
            {
                OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_rsp_rx_ftm_req:: en_asap OAL_TRUE.}");
                past_ftm_rsp[uc_session_id].uc_ftms_per_burst = past_ftm_rsp[uc_session_id].uc_ftms_per_burst_save;
                dmac_ftm_rsp_wait_send_ftm(pst_dmac_vap, uc_session_id);
            }
        }
    }
    else
    {
        /*结束ftm*/
        OAL_MEMZERO(&(past_ftm_rsp[uc_session_id]), OAL_SIZEOF(dmac_ftm_responder_stru));
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_rsp_rx_ftm_req:: iftmr do not have ftmp ie !!!.}");
        return;
    }
}


oal_void  dmac_tx_set_ftm_ctrl_dscr(dmac_vap_stru *pst_dmac_vap, hal_tx_dscr_stru * p_tx_dscr, oal_netbuf_stru *pst_netbuf)
{
    mac_ieee80211_frame_stru       *pst_frame_hdr;
    oal_uint8                      *puc_data;
    //oal_uint16                      us_frame_len;
    //mac_ftm_parameters_ie_stru     *pst_mac_ftmp;
    hal_to_dmac_device_stru        *pst_hal_device;
    //mac_tx_ctl_stru                *pst_tx_ctl;
    oal_int8                        c_session_id = -1;
    oal_uint8                       uc_session_id = 0;
    dmac_ftm_initiator_stru        *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;
    dmac_ftm_responder_stru        *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;

    pst_hal_device = pst_dmac_vap->pst_hal_device;
    if (OAL_PTR_NULL == pst_hal_device)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_TX, "{dmac_tx_set_ftm_ctrl_dscr::pst_hal_device null.}");
        return;
    }

    /* 获取帧头信息 */
    pst_frame_hdr = (mac_ieee80211_frame_stru *)oal_netbuf_header(pst_netbuf);
    //pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_netbuf);

    /* 获取帧体指针 */
    puc_data = oal_netbuf_payload(pst_netbuf);
    //us_frame_len = MAC_GET_CB_MPDU_LEN(pst_tx_ctl);  /*帧体长度*/

    if ((WLAN_ACTION != pst_frame_hdr->st_frame_control.bit_sub_type)
       ||(MAC_ACTION_CATEGORY_PUBLIC != (puc_data[MAC_ACTION_OFFSET_CATEGORY]))
       ||((MAC_PUB_FTM_REQ != (puc_data[MAC_ACTION_OFFSET_ACTION]))&&(MAC_PUB_FTM != (puc_data[MAC_ACTION_OFFSET_ACTION]))))
    {
        //只需处理FTM和FTM req帧
        return;
    }

    /*init 设置iFTMR帧 tx环回描述符*/
    if (MAC_PUB_FTM_REQ == (puc_data[MAC_ACTION_OFFSET_ACTION]))
    {
        if (past_ftm_init[0].en_iftmr == OAL_TRUE)
        {
            //有Fine Timing Measurement Parameters 则是iFTMR
            OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_tx_set_ftm_ctrl_dscr::now hal_tx_set_ftm_ctrl_dscr");
            hal_set_ftm_cali(pst_hal_device, p_tx_dscr, past_ftm_init[0].en_cali);
        }

        //设置发送描述符中的tx_desc_freq_bandwidth_mode
        hal_set_ftm_bandwidth(pst_hal_device, p_tx_dscr, past_ftm_init[0].en_band_cap);
        //设置发送协议
        hal_set_ftm_protocol(pst_hal_device, p_tx_dscr, past_ftm_init[0].en_prot_format);
    }

    /*rsp 设置FTM帧 硬件重传次数*/
    if (MAC_PUB_FTM == (puc_data[MAC_ACTION_OFFSET_ACTION]))
    {
        hal_set_ftm_tx_cnt(pst_hal_device, p_tx_dscr, 1);

        c_session_id = dmac_ftm_find_session_index(pst_dmac_vap, MAC_FTM_RESPONDER_MODE, pst_frame_hdr->auc_address1);
        if (c_session_id < 0)
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_tx_set_ftm_ctrl_dscr::session_id error!!!}");
            return;
        }
        uc_session_id = (oal_uint8)c_session_id;
        if(oal_compare_mac_addr(past_ftm_rsp[uc_session_id].auc_mac_ra, pst_frame_hdr->auc_address1))
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_tx_set_ftm_ctrl_dscr::session_id error!!!}");
            return;
        }
        //设置发送描述符中的tx_desc_freq_bandwidth_mode
        hal_set_ftm_bandwidth(pst_hal_device, p_tx_dscr, past_ftm_rsp[uc_session_id].en_band_cap);
        //设置发送协议
        hal_set_ftm_protocol(pst_hal_device, p_tx_dscr, past_ftm_rsp[uc_session_id].uc_prot_format);
    }
}


oal_void dmac_vap_ftm_int(dmac_vap_stru *pst_dmac_vap)
{
    pst_dmac_vap->pst_ftm = &g_st_ftm;
}


oal_int8 dmac_ftm_find_session_index(dmac_vap_stru *pst_dmac_vap, mac_ftm_mode_enum_uint8 en_ftm_mode, oal_uint8 auc_peer_mac[WLAN_MAC_ADDR_LEN])
{
    dmac_ftm_initiator_stru       *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;
    dmac_ftm_responder_stru       *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;
    oal_uint8                      uc_index = 0;
    oal_int8                       c_index = -1;

    if (MAC_FTM_RESPONDER_MODE == en_ftm_mode)
    {
        for (uc_index = 0; uc_index < MAX_FTM_SESSION; uc_index++)
        {
            if (OAL_TRUE == past_ftm_rsp[uc_index].en_ftm_responder)
            {
                if(!oal_compare_mac_addr(past_ftm_rsp[uc_index].auc_mac_ra, auc_peer_mac))
                {
                    return (oal_int8)uc_index;
                }
            }
            else
            {
                c_index = (oal_int8)uc_index;
            }
        }

        if(c_index < 0)
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_find_session_index::sessions are all used!!!}");
        }
        return c_index;
    }

    if (MAC_FTM_INITIATOR_MODE == en_ftm_mode)
    {
        for (uc_index = 0; uc_index < MAX_FTM_SESSION; uc_index++)
        {
            if (OAL_TRUE == past_ftm_init[uc_index].en_ftm_initiator)
            {
                if(!oal_compare_mac_addr(past_ftm_init[uc_index].auc_bssid, auc_peer_mac))
                {
                    return (oal_int8)uc_index;
                }
            }
            else
            {
                c_index = (oal_int8)uc_index;
            }
        }

        if(c_index < 0)
        {
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_find_session_index::sessions are all used!!!}");
        }
        return c_index;
    }

    OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_find_session_index::en_ftm_mode[%d] error!!!}", en_ftm_mode);
    return c_index;
}


oal_void dmac_ftm_enable_session_index(dmac_vap_stru *pst_dmac_vap, mac_ftm_mode_enum_uint8 en_ftm_mode,
                                                   oal_uint8 auc_peer_mac[WLAN_MAC_ADDR_LEN], oal_uint8 uc_session_id)
{
    dmac_ftm_initiator_stru       *past_ftm_init = pst_dmac_vap->pst_ftm->ast_ftm_init;
    dmac_ftm_responder_stru       *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;


    if (MAC_FTM_RESPONDER_MODE == en_ftm_mode)
    {
        if(!oal_compare_mac_addr(past_ftm_rsp[uc_session_id].auc_mac_ra, auc_peer_mac))
        {
            oal_memcopy(past_ftm_rsp[uc_session_id].auc_mac_ra, auc_peer_mac, WLAN_MAC_ADDR_LEN);
            past_ftm_rsp[uc_session_id].en_received_iftmr = OAL_FALSE;
        }

        past_ftm_rsp[uc_session_id].en_ftm_responder = OAL_TRUE;
    }

    if (MAC_FTM_INITIATOR_MODE == en_ftm_mode)
    {
        if(!oal_compare_mac_addr(past_ftm_init[uc_session_id].auc_bssid, auc_peer_mac))
        {
            oal_memcopy(past_ftm_init[uc_session_id].auc_bssid, auc_peer_mac, WLAN_MAC_ADDR_LEN);
        }

        past_ftm_init[uc_session_id].en_ftm_initiator = OAL_TRUE;
    }

    OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_ftm_enable_session_index::en_ftm_mode[%d] error!!!}", en_ftm_mode);
    return;
}


oal_bool_enum_uint8 dmac_check_ftm_switch_channel(dmac_vap_stru *pst_dmac_vap, mac_ftm_mode_enum_uint8 en_ftm_mode, oal_uint8 uc_chan_number)
{
    //dmac_ftm_initiator_stru            *past_ftm_init = &(pst_dmac_vap->pst_ftm->ast_ftm_init);
    dmac_ftm_responder_stru            *past_ftm_rsp = pst_dmac_vap->pst_ftm->ast_ftm_rsp;
    oal_uint8                           uc_index = 0;

    /*AP作为init 和resp都不容许切信道*/
    if (WLAN_VAP_MODE_BSS_AP == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        return OAL_FALSE;
    }

    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        /*关联状态作为init 和resp都不容许切信道*/
        if(MAC_VAP_STATE_UP == pst_dmac_vap->st_vap_base_info.en_vap_state)
        {
            return OAL_FALSE;
        }

        /*未关联状态resp数组下都没有与之不同的信道，可以切换。否则不容许切信道*/
        if (MAC_FTM_RESPONDER_MODE == en_ftm_mode)
        {
            for (uc_index = 0; uc_index < MAX_FTM_SESSION; uc_index++)
            {
                if ((OAL_TRUE == past_ftm_rsp[uc_index].en_ftm_responder)
                     && (uc_chan_number != past_ftm_rsp[uc_index].st_channel_ftm.uc_chan_number))
                {
                    return OAL_FALSE;
                }
                else
                {
                    continue;
                }
            }

            return OAL_TRUE;
        }

        if ((MAC_FTM_INITIATOR_MODE == en_ftm_mode)
             || (MAC_FTM_MIX_MODE == en_ftm_mode))
        {
            return OAL_FALSE;
        }
    }

    return OAL_FALSE;
}
dmac_location_stru gst_location_info; /* 放在dmac vap下? */
oal_uint8          guc_csi_fragment = 0;
oal_uint8          guc_csi_loop_flag = 0;


oal_uint32 dmac_location_send_event_to_host(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_netbuf)
{
    oal_uint32                  ul_ret;
    frw_event_mem_stru         *pst_event_mem;
    dmac_wlan_crx_event_stru   *pst_crx_event;
    frw_event_stru             *pst_event;

    /* 申请事件内存 */
    pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(pst_crx_event));
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_event_mem))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_FTM,
                       "{dmac_location_send_event_to_host::pst_event_mem fail.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event                 = frw_get_event_stru(pst_event_mem);
    pst_crx_event             = (dmac_wlan_crx_event_stru *)pst_event->auc_event_data;
    pst_crx_event->pst_netbuf = pst_netbuf;

    /* 填写事件头 */
    FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                            FRW_EVENT_TYPE_WLAN_CRX,
                            DMAC_WLAN_CRX_EVENT_SUB_TYPE_RX,
                            OAL_SIZEOF(pst_crx_event),
                            FRW_EVENT_PIPELINE_STAGE_1,
                            pst_mac_vap->uc_chip_id,
                            pst_mac_vap->uc_device_id,
                            pst_mac_vap->uc_vap_id);

    /* 分发事件 */
    ul_ret = frw_event_dispatch_event(pst_event_mem);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_FTM,
                        "{dmac_location_send_event_to_host::frw_event_dispatch_event failed[%d].}", ul_ret);
    }

    /* 释放事件 */
    FRW_EVENT_FREE(pst_event_mem);

    return ul_ret;
}


oal_uint32 dmac_location_rssi_process(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf, oal_uint8 c_rssi)
{
    dmac_location_stru         *pst_location;
    mac_ieee80211_frame_stru   *pst_frame_hdr;
    oal_uint8                  *puc_mgmt_hdr;
    oal_uint8                  *puc_payload;
    oal_uint8                  *puc_local_addr;
    oal_netbuf_stru            *pst_mgmt_buf;
    dmac_location_event_stru   *pst_location_event;
    mac_tx_ctl_stru            *pst_tx_ctl;
    oal_uint32                  ul_ret;
    mac_rx_ctl_stru            *pst_rx_ctrl;

    mac_rx_ctl_stru            *pst_rx_ctrl_old;

    pst_location = &gst_location_info;
    /* 调试用，测量时默认为root ap */
    gst_location_info.uc_location_type = ROOT_AP;
    gst_location_info.auc_location_ap[0][0] = 1;
    gst_location_info.auc_location_ap[0][1] = 1;
    gst_location_info.auc_location_ap[0][2] = 1;
    gst_location_info.auc_location_ap[0][3] = 1;
    gst_location_info.auc_location_ap[0][4] = 1;
    gst_location_info.auc_location_ap[0][5] = 1;

    if (NO_LOCATION == pst_location->uc_location_type)
    {
        return OAL_SUCC;
    }

    pst_frame_hdr      = (mac_ieee80211_frame_stru *)OAL_NETBUF_HEADER(pst_netbuf);
    puc_payload        = OAL_NETBUF_PAYLOAD(pst_netbuf);
    pst_rx_ctrl_old    = (mac_rx_ctl_stru  *)oal_netbuf_cb(pst_netbuf);

    /* 只处理FTM frame帧的rssi信息 */
    if (WLAN_ACTION != pst_frame_hdr->st_frame_control.bit_sub_type ||
        MAC_ACTION_CATEGORY_PUBLIC != puc_payload[MAC_ACTION_OFFSET_CATEGORY] ||
        MAC_PUB_FTM != puc_payload[MAC_ACTION_OFFSET_ACTION])
    {
        return OAL_SUCC;
    }

    /* 申请管理帧内存 */
    pst_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_SMGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_mgmt_buf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                       "{dmac_location_rssi_process::Alloc mem fail .}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_set_netbuf_prev(pst_mgmt_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_mgmt_buf,OAL_PTR_NULL);
    OAL_MEM_NETBUF_TRACE(pst_mgmt_buf, OAL_TRUE);

    puc_mgmt_hdr          = oal_netbuf_header(pst_mgmt_buf);
    pst_location_event    = (dmac_location_event_stru *)mac_netbuf_get_payload(pst_mgmt_buf);

    puc_local_addr = mac_vap_get_mac_addr(&pst_dmac_vap->st_vap_base_info);

    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(puc_mgmt_hdr, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);
    mac_hdr_set_fragment_number(puc_mgmt_hdr, 0);
#if 1
    /* 设置RA为ROOTAP */
    oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR1_OFFSET, pst_location->auc_location_ap);

    /* 设置TA为自己的MAC地址 */
    oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR2_OFFSET, pst_frame_hdr->auc_address2);

    /* 设置BSSID为ROOTAP */
    oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR3_OFFSET, pst_location->auc_location_ap);
#endif
    /* 设置TA为自己的MAC地址 */
    //oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR2_OFFSET, pst_frame_hdr->auc_address2);

    mac_set_seq_num(puc_mgmt_hdr, 0);

    pst_location_event->uc_category        = MAC_ACTION_CATEGORY_PUBLIC;
    pst_location_event->uc_action_code     = MAC_PUB_VENDOR_SPECIFIC;
    pst_location_event->uc_eid             = MAC_HISI_LOCATION_RSSI_IE;
    pst_location_event->uc_lenth           = 22;
    pst_location_event->auc_oui[0]         = 0xAC;
    pst_location_event->auc_oui[1]         = 0x85;
    pst_location_event->auc_oui[2]         = 0x3D;

    pst_location_event->uc_location_type = MAC_HISI_LOCATION_RSSI_IE;
    /* 无论发送还是接收，本VAP为server端 */
    if (0 == oal_memcmp(puc_local_addr, pst_frame_hdr->auc_address1, OAL_MAC_ADDR_LEN))
    {
        oal_set_mac_addr(pst_location_event->auc_mac_server, pst_frame_hdr->auc_address1);
        oal_set_mac_addr(pst_location_event->auc_mac_client, pst_frame_hdr->auc_address2);
    }
    else
    {
        oal_set_mac_addr(pst_location_event->auc_mac_server, pst_frame_hdr->auc_address2);
        oal_set_mac_addr(pst_location_event->auc_mac_client, pst_frame_hdr->auc_address1);
    }
    pst_location_event->auc_payload[0]   = c_rssi;
    oal_netbuf_set_len(pst_mgmt_buf, 45);

    pst_rx_ctrl = (mac_rx_ctl_stru  *)oal_netbuf_cb(pst_mgmt_buf);

    pst_rx_ctrl->us_frame_len = 45;
    pst_rx_ctrl->uc_mac_header_len  = pst_rx_ctrl_old->uc_mac_header_len;

    OAM_ERROR_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                          "{dmac_location_rssi_process::mac_header [%d] netbuf-len = %d.}",pst_rx_ctrl->uc_mac_header_len,oal_netbuf_get_len(pst_mgmt_buf));
    if (ROOT_AP == pst_location->uc_location_type)
    {
        /* root ap模式下将采集到的rssi组成通用的action帧抛事件到hmac进行保存 */
        /* Vendor Public Action Header| EID |Length |OUI | type | mac_s | mac_c | rssi */
        ul_ret = dmac_location_send_event_to_host(&pst_dmac_vap->st_vap_base_info, pst_mgmt_buf);
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                          "{dmac_location_rssi_process::dmac send rssi event to host.}");
        if (OAL_SUCC != ul_ret)
         {
            oal_netbuf_free(pst_mgmt_buf);
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                           "{dmac_location_rssi_process::tx event failed.}");
        }
    }
#if 1
    else
    {
        /* repeater或STA模式下将采集到的rssi组成私有action帧发送到root ap */
        /* Vendor Public Action Header| EID |Length |OUI | type | mac_s | mac_c | rssi */

        /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
        pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_mgmt_buf);

        OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl)  = pst_dmac_vap->st_vap_base_info.us_multi_user_idx;/* TBD */
        MAC_GET_CB_IS_MCAST(pst_tx_ctl)     = OAL_FALSE;
        MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl)  = WLAN_WME_AC_MGMT;

        /* 调用发送管理帧接口 */
        ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, 45);
        if (OAL_SUCC != ul_ret)
        {
            oal_netbuf_free(pst_mgmt_buf);
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                           "{dmac_location_rssi_process::tx action failed.}");
        }
    }
#endif

    return ul_ret;
    //return OAL_SUCC;
}

oal_uint32 dmac_location_csi_process(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_ta, oal_uint32 ul_csi_info_len, oal_uint32 **pul_csi_start_addr)
{
    dmac_location_stru         *pst_location;
    oal_netbuf_stru            *pst_mgmt_buf;
    oal_uint8                  *puc_mgmt_hdr;
    dmac_location_event_stru   *pst_location_event;
    oal_uint8                  *puc_local_addr;
    mac_tx_ctl_stru            *pst_tx_ctl;
    oal_uint32                  ul_ret = OAL_SUCC;
    oal_uint32                  ul_curr_report_len = 0;
    oal_uint32                  ul_count = 0;
    oal_uint32                  ul_unreport_len;

    mac_rx_ctl_stru            *pst_rx_ctrl;

    pst_location = &gst_location_info;
    /* 调试用，测量时默认为root ap */
    gst_location_info.uc_location_type = ROOT_AP;
    gst_location_info.auc_location_ap[0][0] = 1;
    gst_location_info.auc_location_ap[0][1] = 1;
    gst_location_info.auc_location_ap[0][2] = 1;
    gst_location_info.auc_location_ap[0][3] = 1;
    gst_location_info.auc_location_ap[0][4] = 1;
    gst_location_info.auc_location_ap[0][5] = 1;

    guc_csi_fragment = 0;

    if(OAL_PTR_NULL == puc_ta || OAL_PTR_NULL == *pul_csi_start_addr)
    {

        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_location_csi_process::PTR null .}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    //OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_location_csi_process::csi_start_addr = 0x%p,ul_csi_info_len = %d .}",*pul_csi_start_addr,ul_csi_info_len);

    if (NO_LOCATION == pst_location->uc_location_type)
    {
        return OAL_SUCC;
    }
    while(ul_count < ul_csi_info_len)
    {
        ul_unreport_len = ul_csi_info_len - ul_count;
        //ul_unreport_len = 0;
        ul_curr_report_len = (ul_unreport_len >= 1400)? 1400 : ul_unreport_len;

        //OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_location_csi_process::curr_len = %d count = %d.}",ul_curr_report_len,ul_count);
        pst_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
        if (OAL_PTR_NULL == pst_mgmt_buf)
        {
            OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_location_csi_process::Alloc mem fail,len = %d .}",ul_curr_report_len);
            oal_netbuf_free(pst_mgmt_buf);
            return OAL_ERR_CODE_PTR_NULL;
        }
        oal_set_netbuf_prev(pst_mgmt_buf, OAL_PTR_NULL);
        oal_set_netbuf_next(pst_mgmt_buf,OAL_PTR_NULL);
        OAL_MEM_NETBUF_TRACE(pst_mgmt_buf, OAL_TRUE);

        puc_mgmt_hdr          = oal_netbuf_header(pst_mgmt_buf);
        pst_location_event    = (dmac_location_event_stru *)mac_netbuf_get_payload(pst_mgmt_buf);

        puc_local_addr = mac_vap_get_mac_addr(&pst_dmac_vap->st_vap_base_info);

        /* 帧控制字段全为0，除了type和subtype */
        mac_hdr_set_frame_control(puc_mgmt_hdr, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);
        mac_hdr_set_fragment_number(puc_mgmt_hdr, 0);

        /* 设置RA为ROOTAP */
        oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR1_OFFSET, pst_location->auc_location_ap);

        /* 设置TA为自己的MAC地址 */
        oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR2_OFFSET, puc_local_addr);

        /* 设置BSSID为ROOTAP */
        oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR3_OFFSET, pst_location->auc_location_ap);
        mac_set_seq_num(puc_mgmt_hdr, 0);

        pst_location_event->uc_category        = MAC_ACTION_CATEGORY_PUBLIC;
        pst_location_event->uc_action_code     = MAC_PUB_VENDOR_SPECIFIC;
        pst_location_event->uc_eid             = MAC_HISI_LOCATION_CSI_IE;
        pst_location_event->uc_lenth           = 0;
        pst_location_event->auc_oui[0]         = 0xAC;
        pst_location_event->auc_oui[1]         = 0x85;
        pst_location_event->auc_oui[2]         = 0x3D;

        pst_location_event->uc_location_type = MAC_HISI_LOCATION_CSI_IE;
        oal_set_mac_addr(pst_location_event->auc_mac_server, puc_local_addr);
        oal_set_mac_addr(pst_location_event->auc_mac_client, puc_ta);
        pst_location_event->auc_payload[0]   = guc_csi_fragment++;

        pst_location_event->auc_payload[1]   = guc_csi_loop_flag;
        /* 当前上报长度小于1400，即认为是分片报文的最后一片，供host保存文件使用 */
        if(ul_curr_report_len < 1400)
        {
            pst_location_event->auc_payload[2] = 1;
        }
        else
        {
            pst_location_event->auc_payload[2] = 0;
        }
        oal_memcopy(pst_location_event->auc_payload + 3, (oal_void *)(*pul_csi_start_addr + (ul_count >> 2)), ul_curr_report_len);
        oal_netbuf_set_len(pst_mgmt_buf, (ul_curr_report_len + 47));
        ul_count += ul_curr_report_len;

        pst_rx_ctrl = (mac_rx_ctl_stru  *)oal_netbuf_cb(pst_mgmt_buf);

        pst_rx_ctrl->us_frame_len = (oal_uint16)(ul_curr_report_len + 47);
        pst_rx_ctrl->uc_mac_header_len  = 24;

        /* Vendor Public Action Header| EID |Length |OUI | type | mac_s | mac_c | csi */
        if (ROOT_AP == pst_location->uc_location_type)
        {
            /* root ap模式下将采集到的csi抛事件到hmac进行保存 */
            /* Vendor Public Action Header| EID |Length |OUI | type | mac_s | mac_c | csi */
            ul_ret = dmac_location_send_event_to_host(&pst_dmac_vap->st_vap_base_info, pst_mgmt_buf);

            if (OAL_SUCC != ul_ret)
            {
                oal_netbuf_free(pst_mgmt_buf);
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,"{dmac_location_csi_process::tx event buf_1 failed.}");
            }
        }

        else
        {
            /* repeater或STA模式下将采集到的csi组成私有action帧发送到root ap */
            /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
            pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_mgmt_buf);

            OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));
            MAC_GET_CB_TX_USER_IDX(pst_tx_ctl)  = pst_dmac_vap->st_vap_base_info.us_multi_user_idx;/*TBD*/
            MAC_GET_CB_IS_MCAST(pst_tx_ctl)     = OAL_FALSE;
            MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl)  = WLAN_WME_AC_MGMT;

            /* 调用发送管理帧接口 */
            //ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, (ul_curr_report_len + 45));
            ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, 45);
            if (OAL_SUCC != ul_ret)
            {
                oal_netbuf_free(pst_mgmt_buf);
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM, "{dmac_location_csi_process::tx action_1 failed.}");
            }
        }
    }
    return ul_ret;
}


oal_uint32 dmac_location_ftm_process(dmac_vap_stru *pst_dmac_vap, dmac_ftm_initiator_stru *pst_initiator)
{
    dmac_location_stru         *pst_location;
    oal_uint8                  *puc_mgmt_hdr;
    oal_uint8                  *puc_local_addr;
    oal_netbuf_stru            *pst_mgmt_buf;
    dmac_location_event_stru   *pst_location_event;
    mac_tx_ctl_stru            *pst_tx_ctl;
    oal_uint32                  ul_ret;

    pst_location = &gst_location_info;

    if (NO_LOCATION == pst_location->uc_location_type)
    {
        return OAL_SUCC;
    }

    /* 申请管理帧内存 */
    pst_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_SHORT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_mgmt_buf)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                       "{dmac_location_ftm_process::Alloc mem fail .}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_set_netbuf_prev(pst_mgmt_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_mgmt_buf,OAL_PTR_NULL);
    OAL_MEM_NETBUF_TRACE(pst_mgmt_buf, OAL_TRUE);

    puc_mgmt_hdr          = oal_netbuf_header(pst_mgmt_buf);
    pst_location_event    = (dmac_location_event_stru *)mac_netbuf_get_payload(pst_mgmt_buf);

    puc_local_addr = mac_vap_get_mac_addr(&pst_dmac_vap->st_vap_base_info);

    /* 帧控制字段全为0，除了type和subtype */
    mac_hdr_set_frame_control(puc_mgmt_hdr, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);
    mac_hdr_set_fragment_number(puc_mgmt_hdr, 0);

    /* 设置RA为ROOTAP */
    oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR1_OFFSET, pst_location->auc_location_ap);

    /* 设置TA为自己的MAC地址 */
    oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR2_OFFSET, puc_local_addr);

    /* 设置BSSID为ROOTAP */
    oal_set_mac_addr(puc_mgmt_hdr + WLAN_HDR_ADDR3_OFFSET, pst_location->auc_location_ap);
    mac_set_seq_num(puc_mgmt_hdr, 0);

    pst_location_event->uc_category        = MAC_ACTION_CATEGORY_PUBLIC;
    pst_location_event->uc_action_code     = MAC_PUB_VENDOR_SPECIFIC;
    pst_location_event->uc_eid             = MAC_HISI_LOCATION_FTM_IE;
    pst_location_event->uc_lenth           = 22;
    pst_location_event->auc_oui[0]         = 0xAC;
    pst_location_event->auc_oui[1]         = 0x85;
    pst_location_event->auc_oui[2]         = 0x3D;

    /* Vendor Public Action Header| EID |Length |OUI | type | mac_s | mac_c | t1 | t2 | t3 | t4 */
    pst_location_event->uc_location_type = MAC_HISI_LOCATION_FTM_IE;

    oal_set_mac_addr(pst_location_event->auc_mac_server, puc_local_addr);
    //oal_set_mac_addr(pst_location_event->auc_mac_client, puc_ta);

    pst_location_event->auc_payload[0]   = 0;
    /* TBD t1 t2 t3 t4 */
    oal_netbuf_set_len(pst_mgmt_buf, 68);

    if (ROOT_AP == pst_location->uc_location_type)
    {
        /* root ap模式下将采集到的rssi组成通用的action帧抛事件到hmac进行保存 */
        ul_ret = dmac_location_send_event_to_host(&pst_dmac_vap->st_vap_base_info, pst_mgmt_buf);
        if (OAL_SUCC != ul_ret)
        {
            oal_netbuf_free(pst_mgmt_buf);
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                           "{dmac_location_ftm_process::tx event failed.}");
        }
    }
    else
    {
        /* repeater或STA模式下将采集到的rssi组成私有action帧发送到root ap */

        /* 填写netbuf的cb字段，供发送管理帧和发送完成接口使用 */
        pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_mgmt_buf);

        OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));
        MAC_GET_CB_TX_USER_IDX(pst_tx_ctl)  = pst_dmac_vap->st_vap_base_info.us_multi_user_idx;/*TBD*/
        MAC_GET_CB_IS_MCAST(pst_tx_ctl)     = OAL_FALSE;
        MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl)  = WLAN_WME_AC_MGMT;

        /* 调用发送管理帧接口 */
        ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, 45);
        if (OAL_SUCC != ul_ret)
        {
            oal_netbuf_free(pst_mgmt_buf);
            OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_FTM,
                           "{dmac_location_ftm_process::tx action failed.}");
        }
    }

    return ul_ret;
}


#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif



