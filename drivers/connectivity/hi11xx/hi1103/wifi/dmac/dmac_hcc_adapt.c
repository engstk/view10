


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "mac_resource.h"
#include "dmac_hcc_adapt.h"
#include "dmac_vap.h"
#include "mac_device.h"
#include "oal_hcc_slave_if.h"
#include "oal_profiling.h"
#include "dmac_config.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_HCC_ADAPT_C

extern oal_uint32  dmac_release_tid_buffs(oal_void *pst_dmac_vap, oal_void *pst_device, oal_uint32 ul_nums);

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
extern oal_uint32 dmac_config_set_device_freq(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param);
#endif
extern oal_int32 hi110x_device_module_init(oal_void);

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* hcc层对驱动层的回调钩子 */
dmac_hcc_adapt_handle    g_hcc_callback_handle;

extern oal_uint16 g_us_download_rate_limit_pps;

/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint32 dmac_proc_wlan_drx_event_tx_adapt_ram(frw_event_mem_stru *pst_event_mem)
{
    frw_event_hdr_stru              *pst_event_hdr;
    dmac_wlan_drx_event_stru        *pst_wlan_rx;
    oal_netbuf_stru                 *pst_tmp_netbuf = OAL_PTR_NULL;
    oal_netbuf_stru                 *pst_curr_netbuf;
    mac_rx_ctl_stru                 *pst_rx_cb;
    mac_ether_header_stru           *pst_ether_hdr = OAL_PTR_NULL;
    oal_uint8                        uc_mac_header[MAX_MAC_HEAD_LEN] = {0};
    oal_uint16                       us_payload_len = WLAN_LARGE_NETBUF_SIZE;
    oal_uint16                       us_loop;
    oal_uint16                       us_netbuf_num;
    oal_uint16                       us_frame_payload_len = WLAN_LARGE_NETBUF_SIZE;
    oal_uint16                       us_cur_msdu_len = WLAN_LARGE_NETBUF_SIZE;
    oal_uint16                       us_amsdu_buffer = 1;
    oal_uint8                        uc_mac_header_len = MAX_MAC_HEAD_LEN;
    oal_uint8                        uc_user_index = 0;

    pst_event_hdr   = frw_get_event_hdr(pst_event_mem);
    pst_wlan_rx     = (dmac_wlan_drx_event_stru *)frw_get_event_payload(pst_event_mem);
    pst_curr_netbuf = pst_wlan_rx->pst_netbuf;
    us_netbuf_num   = pst_wlan_rx->us_netbuf_num;

    for (us_loop = 0; us_loop < us_netbuf_num; us_loop++)
    {
        struct hcc_transfer_param param;
        if(NULL == pst_curr_netbuf)
        {
            /*netbuf list num < list head's num?*/
            OAM_ERROR_LOG3(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac had post %d netbufs,but list num is:%d,miss %d}",
                                                                us_loop, us_netbuf_num, us_netbuf_num - us_loop);
            break;
        }

        pst_rx_cb = (mac_rx_ctl_stru *)oal_netbuf_cb(pst_curr_netbuf);

        if(OAL_FALSE == pst_rx_cb->bit_amsdu_enable)
        {
            //非AMSDU接收描述符中的payload len
            us_payload_len = pst_rx_cb->us_frame_len - pst_rx_cb->uc_mac_header_len;
        }
        else/* for amsdu adapt */
        {
            /* AMSDU首帧处理 */
            if (OAL_TRUE == pst_rx_cb->bit_is_first_buffer)
            {
                uc_user_index     = MAC_GET_RX_CB_TA_USER_IDX(pst_rx_cb);
                uc_mac_header_len = pst_rx_cb->uc_mac_header_len;
                us_frame_payload_len = pst_rx_cb->us_frame_len - uc_mac_header_len;
                /* MPDU包含的SKB数 */
                us_amsdu_buffer   = pst_rx_cb->bit_buff_nums;

                /* AMSDU包含多个SKB需拷贝MAC HEAD */
                if (1 < us_amsdu_buffer)
                {
                    oal_memcopy(uc_mac_header, oal_netbuf_header(pst_curr_netbuf), uc_mac_header_len);
                }
            }
            else
            {
                pst_rx_cb->uc_mac_vap_id = pst_event_hdr->uc_vap_id;
                MAC_GET_RX_CB_TA_USER_IDX(pst_rx_cb) = uc_user_index;
                pst_rx_cb->uc_mac_header_len = uc_mac_header_len;
                oal_memcopy(oal_netbuf_header(pst_curr_netbuf), uc_mac_header, uc_mac_header_len);
            }

            /* AMSDU包含多个SKB并且当前MPDU只包含一个MSDU,则计算当前msdu真实长度 */
            if ((1 < us_amsdu_buffer) && (1 == pst_rx_cb->uc_msdu_in_buffer))
            {
                pst_ether_hdr = (mac_ether_header_stru *)oal_netbuf_data(pst_curr_netbuf);
                us_cur_msdu_len  = ((pst_ether_hdr->us_ether_type & 0xFF) << 8) | ((pst_ether_hdr->us_ether_type >> 8));
                us_cur_msdu_len += ETHER_HDR_LEN;
                us_frame_payload_len -= us_cur_msdu_len;
#if 0
                OAM_WARNING_LOG4(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac : ***start*** head:%d,cur len:%d,left len:%d,amsdu skb num:%d}",
                                                                uc_mac_header_len,
                                                                us_fst_msdu_len,
                                                                us_frame_payload_len,
                                                                us_amsdu_buffer);
                OAM_ERROR_LOG4(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac xxxxxxxxxxxxxxxx framelen[%d] curlen:0x%x,d[0x%x]s[0x%x]}",
                                                            pst_rx_cb->us_frame_len,
                                                            pst_ether_hdr->us_ether_type,
                                                            pst_ether_hdr->auc_ether_dhost[0],
                                                            pst_ether_hdr->auc_ether_shost[0]);
#endif
            }
            else
            {
                us_cur_msdu_len = WLAN_LARGE_NETBUF_SIZE;
            }

            /* 标记amsdu尾帧 */
            if(1 < us_amsdu_buffer)
            {
                us_amsdu_buffer -= 1;
                /* 当前msdu帧长,如果skb包含多个msdu则默认使用最大内存长 */
                us_payload_len = OAL_MIN(WLAN_LARGE_NETBUF_SIZE, us_cur_msdu_len);
            }
            else
            {
                pst_rx_cb->bit_is_last_buffer = 1;
                us_payload_len = OAL_MIN(WLAN_LARGE_NETBUF_SIZE, us_frame_payload_len);
            }

#if 0
            OAM_WARNING_LOG4(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac : @@@isfirst:%d,skb:%d,msdu:%d,islast:%d}",
                                                            pst_rx_cb->bit_is_first_buffer,
                                                            pst_rx_cb->bit_buff_nums,
                                                            pst_rx_cb->uc_msdu_in_buffer,
                                                            pst_rx_cb->bit_is_last_buffer);
            OAM_WARNING_LOG4(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac : @@@head:%d,paylen:%d,frame:%d,mac hdr:%d}",
                                                            uc_mac_header_len,
                                                            us_payload_len,
                                                            pst_rx_cb->us_frame_len,
                                                            pst_rx_cb->bit_mac_header_len);
#endif

            //因为没有单buffer长度(描述符里是总长度) SDIO发送最大长度
            pst_rx_cb->us_frame_len = 0;
            //host netbuf不成链 单buffer处理
            pst_rx_cb->bit_buff_nums = 1;
        }

        if(us_payload_len > WLAN_LARGE_NETBUF_SIZE)
        {
            OAM_ERROR_LOG4(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac ########***start***: paylen:%d,frame:%d,mac hdr:%d,is amsdu[%d]}",
                                                            us_payload_len,
                                                            pst_rx_cb->us_frame_len,
                                                            pst_rx_cb->uc_mac_header_len,
                                                            pst_rx_cb->bit_amsdu_enable);
            if (pst_ether_hdr != OAL_PTR_NULL)
            {
                OAM_WARNING_LOG3(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac  len:0x%x,d[0x%x]s[0x%x]}",
                                                            pst_ether_hdr->us_ether_type,
                                                            pst_ether_hdr->auc_ether_dhost[0],
                                                            pst_ether_hdr->auc_ether_shost[0]);
            }
            OAM_WARNING_LOG4(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac : isfirst:%d,skb:%d,msdu:%d,islast:%d}",
                                                            pst_rx_cb->bit_is_first_buffer,
                                                            pst_rx_cb->bit_buff_nums,
                                                            pst_rx_cb->uc_msdu_in_buffer,
                                                            pst_rx_cb->bit_is_last_buffer);
            OAM_WARNING_LOG4(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac : ---end--- head:%d,cur len:%d,left len:%d,amsdu skb num:%d}",
                                                            uc_mac_header_len,
                                                            us_cur_msdu_len,
                                                            us_frame_payload_len,
                                                            us_amsdu_buffer);

            us_payload_len = WLAN_LARGE_NETBUF_SIZE;
        }

        pst_tmp_netbuf = pst_curr_netbuf;

        /* 指向netbuf链表的下一个netbuf */
        pst_curr_netbuf = pst_curr_netbuf->next;

        if (OAL_SUCC != dmac_adapt_tx_param_and_netbuf_hdr_init(pst_event_mem,&param,pst_tmp_netbuf))
        {
            OAM_ERROR_LOG0(0,OAM_SF_ANY,"{dmac_proc_wlan_drx_event_tx_adapt::dmac_adapt_tx_param_and_netbuf_hdr_init failed}");
            return OAL_ERR_CODE_PTR_NULL;
        }

        hcc_slave_tx(pst_tmp_netbuf, us_payload_len, &param);
    }

    if(us_loop != us_netbuf_num)
    {
        OAM_ERROR_LOG1(pst_event_hdr->uc_vap_id, OAM_SF_RX, "{dmac wlan drx mem leak,still reamin %d netbufs}",
                                                        us_netbuf_num - us_loop);
    }

    return OAL_SUCC;

}

/*debug*/
oal_void dmac_event_pkts_dump(oal_void)
{
    oal_int32 i = 0;
    for(i = 0; i < FRW_EVENT_TYPE_BUTT;i++)
    {
        OAL_IO_PRINT("event:%d,pkts:%u\r\n",i,g_hcc_sched_event_pkts[i]);
    }
}


oal_void dmac_hcc_adapt_callback_register(oal_void)
{
    /* 注册回调钩子 */
    g_hcc_callback_handle.dmac_release_tid_buffs_func = dmac_release_tid_buffs;


}

oal_int32 dmac_hcc_adapt_init(oal_void)
{
    OAL_MEMZERO(g_hcc_sched_event_pkts, OAL_SIZEOF(g_hcc_sched_event_pkts));
    /* 注册回调钩子 */
    dmac_hcc_adapt_callback_register();

    return hcc_rx_register(hcc_get_default_handler(), HCC_ACTION_TYPE_WIFI,
              dmac_rx_wifi_post_action_function, dmac_rx_wifi_pre_action_function);
}
#endif

#if (defined(_PRE_PLAT_FEATURE_CUSTOMIZE) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE))
extern oal_uint8  g_auc_mac_device_radio_cap[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP];
extern oal_uint16 g_us_cali_mask;
extern oal_uint8  g_uc_cali_data_mask;
/*lint -save -e762*/
extern hal_cfg_cap_info_stru g_ast_hal_cfg_custom_stru[];
/*lint -restore */

oal_void dmac_hcc_config_adapt_perf_ini(oal_uint8 *puc_param)
{
    config_device_perf_h2d_stru   *pst_device_perf;
    pst_device_perf              = (config_device_perf_h2d_stru *)puc_param;

    dmac_config_sdio_flowctrl(NULL, 0, (oal_uint8 *)(pst_device_perf->ac_used_mem_param));

    /* 配置D2H SDIO聚合参数 */
    dmac_config_set_d2h_hcc_assemble_cnt(NULL, 0, (oal_uint8 *)&(pst_device_perf->uc_sdio_assem_d2h));
}

oal_void dmac_hcc_config_adapt_priv_cali_mask_ini(oal_uint8 *puc_param)
{
    g_us_cali_mask = *(oal_uint16 *)puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_cali_mask_ini::cali_mask[0x%x]",
                       g_us_cali_mask);
}
oal_void dmac_hcc_config_adapt_priv_calidata_mask_ini(oal_uint8 *puc_param)
{
    g_uc_cali_data_mask = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_calidata_mask_ini::calidata_mask[0x%x]",
                       g_uc_cali_data_mask);
}

#ifdef _PRE_WLAN_RF_AUTOCALI
extern oal_uint8 g_uc_autocali_mask;
oal_void dmac_hcc_config_adapt_priv_autocali_mask_ini(oal_uint8 *puc_param)
{
    g_uc_autocali_mask = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_autocali_mask_ini::autocali_mask[0x%x]",
                       g_uc_autocali_mask);
}
#endif // #ifdef _PRE_WLAN_RF_AUTOCALI

oal_void dmac_hcc_config_adapt_priv_radio_cap_ini(oal_uint8 * puc_param)
{
    oal_memcopy(g_auc_mac_device_radio_cap, puc_param, OAL_SIZEOF(g_auc_mac_device_radio_cap));

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_radio_cap_ini::radio_cap[%d]",
                g_auc_mac_device_radio_cap[0]);
}

oal_void dmac_hcc_config_adapt_priv_bw_max_with_ini(oal_uint8 * puc_param)
{
    g_pst_mac_device_capability[0].en_channel_width = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_bw_max_with_ini::mac device0 bw_max_with[%d]",
                g_pst_mac_device_capability[0].en_channel_width);
}

oal_void dmac_hcc_config_adapt_priv_ldpc_coding_ini(oal_uint8 *puc_param)
{
    g_pst_mac_device_capability[0].en_ldpc_is_supp = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_ldpc_coding_ini::mac device0 ldpc coding[%d]",
                g_pst_mac_device_capability[0].en_ldpc_is_supp);
}

oal_void dmac_hcc_config_adapt_priv_rx_stbc_ini(oal_uint8 *puc_param)
{
    g_pst_mac_device_capability[0].en_rx_stbc_is_supp = *puc_param;

    g_ast_hal_cfg_custom_stru[0].en_rx_stbc_is_supp = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_rx_stbc_ini::mac device0 rx stbc[%d]",
                g_pst_mac_device_capability[0].en_rx_stbc_is_supp);
}

oal_void dmac_hcc_config_adapt_priv_tx_stbc_ini(oal_uint8 *puc_param)
{
    g_pst_mac_device_capability[0].en_tx_stbc_is_supp = *puc_param;

    g_ast_hal_cfg_custom_stru[0].en_tx_stbc_is_supp = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_rx_stbc_ini::mac device0 rx stbc[%d]",
                g_pst_mac_device_capability[0].en_tx_stbc_is_supp);
}

oal_void dmac_hcc_config_adapt_priv_su_bfer_ini(oal_uint8 *puc_param)
{
    g_pst_mac_device_capability[0].en_su_bfmer_is_supp = *puc_param;

    g_ast_hal_cfg_custom_stru[0].en_su_bfmer_is_supp = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_su_bfer_ini::mac device0 su bfer[%d]",
               g_pst_mac_device_capability[0].en_su_bfmer_is_supp);
}

oal_void dmac_hcc_config_adapt_priv_su_bfee_ini(oal_uint8 *puc_param)
{
    g_pst_mac_device_capability[0].en_su_bfmee_is_supp = *puc_param;

    g_ast_hal_cfg_custom_stru[0].en_su_bfmee_is_supp = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_rx_stbc_ini::mac device0 su bfee[%d]",
               g_pst_mac_device_capability[0].en_su_bfmee_is_supp);
}

oal_void dmac_hcc_config_adapt_priv_mu_bfer_ini(oal_uint8 *puc_param)
{
    g_pst_mac_device_capability[0].en_mu_bfmer_is_supp = *puc_param;

    g_ast_hal_cfg_custom_stru[0].en_mu_bfmer_is_supp = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_rx_stbc_ini::mac device0 mu bfer[%d]",
               g_ast_hal_cfg_custom_stru[0].en_mu_bfmer_is_supp);
}

oal_void dmac_hcc_config_adapt_priv_mu_bfee_ini(oal_uint8 *puc_param)
{
    g_pst_mac_device_capability[0].en_mu_bfmee_is_supp = *puc_param;

    g_ast_hal_cfg_custom_stru[0].en_mu_bfmee_is_supp = *puc_param;

    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_rx_stbc_ini::mac device0 mu bfee[%d]",
               g_ast_hal_cfg_custom_stru[0].en_mu_bfmee_is_supp);

}
#ifdef _PRE_WLAN_DOWNLOAD_PM
oal_void dmac_hcc_config_adapt_priv_download_pm_ini(oal_uint8 *puc_param)
{
    g_us_download_rate_limit_pps = *(oal_uint16 *)puc_param;
    OAM_WARNING_LOG1(0, OAM_SF_CFG, "dmac_hcc_config_adapt_priv_download_pm_ini::download_rate_limit [%d]pps",
               g_us_download_rate_limit_pps);
}
#endif
#ifdef _PRE_WLAN_FEATURE_TXOPPS
oal_void dmac_hcc_config_adapt_priv_txopps_switch_ini(oal_uint8 *puc_param)
{
    g_ast_hal_cfg_custom_stru[0].en_txopps_is_supp = *puc_param;
    g_ast_hal_cfg_custom_stru[1].en_txopps_is_supp = *puc_param;
    OAM_WARNING_LOG1(0, OAM_SF_CUSTOM, "dmac_hcc_config_adapt_priv_txopps_switch_ini::en_txopps_is_supp [%d]pps",
               g_ast_hal_cfg_custom_stru[0].en_txopps_is_supp);
}
#endif

oal_void dmac_hcc_config_adapt_ini_butt(oal_uint8 * puc_param)
{
    /* TBD: 新增定制化配置信息 */
    OAM_WARNING_LOG0(0, OAM_SF_CFG, "dmac_hcc_config_adapt_perf_ini_butt::TBD new customization to be added");
    return;
}

oal_int32 dmac_rx_custom_post_data_function(oal_uint8 stype,
                                             hcc_netbuf_stru* pst_hcc_netbuf, oal_uint8 *pst_context)
{
    oal_netbuf_stru                      *pst_netbuf;
    hmac_to_dmac_cfg_custom_data_stru     st_syn_msg;
    custom_cfgid_enum_uint32              en_syn_id  =0;      /* 同步配置ID*/
    oal_uint16                            us_req_idx = 0;
    oal_uint8                            *puc_data;

    OAL_BUG_ON(NULL == pst_hcc_netbuf);

    pst_netbuf = pst_hcc_netbuf->pst_netbuf;
    puc_data = OAL_NETBUF_HCC_PAYLOAD(pst_hcc_netbuf->pst_netbuf);

    OAL_IO_PRINT("dmac_rx_custom_post_data_function stype[%d] len[%d]!\r\n", stype, pst_hcc_netbuf->len);

    if ((CUSTOM_CFGID_INI_ID != stype) && (CUSTOM_CFGID_PRIV_INI_ID != stype))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_rx_custom_post_data_function::stype[%d] invalid!}", stype);
        OAL_MEM_NETBUF_FREE(pst_netbuf);
        return -OAL_EINVAL;
    }

    /* 定制化消息的格式如下:                                                 */
    /* +-------------------------------------------------------------------+ */
    /* | CFGID0    |DATA0 Length| DATA0 Value | ......................... |  */
    /* +-------------------------------------------------------------------+ */
    /* | 4 Bytes   |4 Byte      | DATA  Length| ......................... |  */
    /* +-------------------------------------------------------------------+ */

    while (us_req_idx < (pst_hcc_netbuf->len))
    {
        /* 获取一个配置结构数据头 */
        oal_memcopy(&st_syn_msg, puc_data + us_req_idx, CUSTOM_MSG_DATA_HDR_LEN);
        en_syn_id   =  st_syn_msg.en_syn_id;
        /* 根据cfgid配置相关定制化参数 */
        switch(en_syn_id)
        {
           /**********************  私有定制--begin  ***********************************/
            case CUSTOM_CFGID_PRIV_INI_RADIO_CAP_ID:
            /* 动态/静态DBDC */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set radio cap len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_radio_cap_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_BW_MAX_WITH_ID:
            /* mac device最大带宽能力 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set bw_max_with len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_bw_max_with_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_LDPC_CODING_ID:
            /* mac device支持LDPC能力 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set ldpc len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_ldpc_coding_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_RX_STBC_ID:
            /* mac device支持rx stbc能力 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set rx_stbc len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_rx_stbc_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_TX_STBC_ID:
            /* mac device支持tx stbc能力 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set tx_stbc len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_tx_stbc_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_SU_BFER_ID:
            /* mac device支持su bfer能力 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set su_bfer len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_su_bfer_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_SU_BFEE_ID:
            /* mac device支持su bfee能力 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set su_bfee len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_su_bfee_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_MU_BFER_ID:
            /* mac device支持mu bfer能力 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set mu_bfer len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_mu_bfer_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_MU_BFEE_ID:
            /* mac device支持mu bfee能力 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set mu_bfee len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_mu_bfee_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_CALI_MASK_ID:
            /* 开机校准开关 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set cali mask len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_cali_mask_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_CALI_DATA_MASK_ID:
            /* 校准数据上传下发开关 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set cali data mask len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_calidata_mask_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_PRIV_INI_AUTOCALI_MASK_ID:
            /* 自动化校准开关 */
            #ifdef _PRE_WLAN_RF_AUTOCALI
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set autocali mask len[%u]!\r\n", st_syn_msg.ul_len);
                dmac_hcc_config_adapt_priv_autocali_mask_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
            #endif // #ifdef _PRE_WLAN_RF_AUTOCALI
                break;

            case CUSTOM_CFGID_PRIV_INI_DOWNLOAD_RATELIMIT_PPS:
            #ifdef _PRE_WLAN_DOWNLOAD_PM
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set download pm!\r\n");
                dmac_hcc_config_adapt_priv_download_pm_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
            #endif
                break;

            case CUSTOM_CFGID_PRIV_INI_TXOPPS_SWITCH_ID:
            #ifdef _PRE_WLAN_FEATURE_TXOPPS
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set TXOPPS!\r\n");
                dmac_hcc_config_adapt_priv_txopps_switch_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
            #endif
                break;

            case CUSTOM_CFGID_PRIV_INI_OVER_TEMPER_PROTECT_THRESHOLD_ID:
            #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set over temper param!\r\n");
                dmac_config_set_cus_over_temper_rf(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
            #endif
                break;

           /**********************私有定制--end  ***************************************/

            case CUSTOM_CFGID_INI_FREQ_ID:
            #ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set freq!\r\n");
                dmac_config_set_device_freq(NULL, 0, puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
            #endif //#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
                break;

            case CUSTOM_CFGID_INI_PERF_ID:
                /* 性能 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set perf!\r\n");
                dmac_hcc_config_adapt_perf_ini(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_INI_LINKLOSS_ID:
                /* linkloss门限 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set linkloss!\r\n");
                dmac_config_set_linkloss_threshold(NULL, 0, puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_INI_PM_SWITCH_ID:
                /* 低功耗 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function set pm switch!\r\n");
                dmac_config_set_pm_switch(NULL, 0, puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;
            case CUSTOM_CFGID_INI_PS_FAST_CHECK_CNT_ID:
                dmac_config_set_ps_check_cnt(NULL, 0, puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            case CUSTOM_CFGID_INI_ENDING_ID:
                /* 定制化配置命令下发结束标志,不再继续执行device侧的定制化配置 */
                OAL_IO_PRINT("dmac_rx_custom_post_data_function finish!\r\n");
                OAL_MEM_NETBUF_FREE(pst_netbuf);
                //继续执行device初始化
                hi110x_device_module_init();
                OAM_WARNING_LOG0(0, OAM_SF_ANY, "dmac_rx_custom_post_data_function, customization finish!");
                return OAL_SUCC;

            case CUSTOM_CFGID_INI_BUTT:
                /* TBD: 预留新增device定制化参数 */
                dmac_hcc_config_adapt_ini_butt(puc_data + us_req_idx + CUSTOM_MSG_DATA_HDR_LEN);
                break;

            default:
                /* 异常情况，停止下发定制化参数 */
                OAL_MEM_NETBUF_FREE(pst_netbuf);
                OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_rx_custom_post_data_function, custom en_syn_id[%d] is abnormal", en_syn_id);
                return -OAL_EINVAL;
        }

        us_req_idx += (oal_uint16)(st_syn_msg.ul_len + CUSTOM_MSG_DATA_HDR_LEN);   /* 指向下一个配置消息 */
    }

    OAL_IO_PRINT("dmac_rx_custom_post_data_function custom data len[%d] req_idx[%d]\r\n", pst_hcc_netbuf->len, us_req_idx);

    if (NULL != pst_netbuf)
    {
        OAL_MEM_NETBUF_FREE(pst_netbuf);
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "dmac_rx_custom_post_data_function, netbuf len[%d]stype[%d]",
                          pst_hcc_netbuf->len, stype);
    }
    return OAL_SUCC;
}
#endif // #if (defined(_PRE_PLAT_FEATURE_CUSTOMIZE) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE))

#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS) || defined(_PRE_WLAN_RF_AUTOCALI)

oal_uint32  dmac_sdt_recv_sample_cmd_tx_adapt(frw_event_mem_stru *pst_event_mem)
{
    return dmac_hcc_tx_event_convert_to_netbuf(pst_event_mem, NULL, (oal_uint32)((frw_get_event_stru(pst_event_mem))->st_event_hdr.us_length) - OAL_SIZEOF(frw_event_hdr_stru));
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

