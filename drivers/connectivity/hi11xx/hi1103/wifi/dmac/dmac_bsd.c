

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_BAND_STEERING
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_sdio_comm.h"
#include "oal_sdio.h"
#include "oal_types.h"
#include "oal_util.h"
#include "dmac_bsd.h"
#include "dmac_vap.h"
#include "frw_ext_if.h"
#include "dmac_scan.h"
#include "dmac_resource.h"
#include "hal_ext_if.h"
#include "dmac_beacon.h"
#include "dmac_alg_if.h"
#include "dmac_tx_bss_comm.h"
#if defined(_PRE_WLAN_PERFORM_STAT) && defined(_PRE_WLAN_11K_STAT)
#include "dmac_ext_if.h"
#include "dmac_stat.h"
#else
#error "the BSD feature dependent _PRE_WLAN_PERFORM_STAT and _PRE_WLAN_11K_STAT,please define it!"
#endif
#include "dmac_config.h"

#ifdef _PRE_WLAN_FEATURE_11V
#include "dmac_11v.h"
#endif



#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_BSD_C

//#define _PRE_DEBUG_MODE      //维测信息的编译宏

//steering的超时时间
#define BSD_STEERING_TIMEOUT    60*1000        //steering超时时间 60s


bsd_mgmt_stru   g_st_bsd_mgmt;

//oal_void* p_check_addr = OAL_PTR_NULL;      //for debug


//默认配置参数
OAL_STATIC OAL_CONST bsd_cfg_stru  dmac_bsd_default_cfg =
{
    /*.ul_vap_load_busy_th =   */800,
    /*.ul_vap_load_free_th =   */200,
    /*.ul_sta_steering_thr_th = */4,        //kbps
    /*.ul_steering_fre_th =   */ 2,         //unit:cnt/30min
    /*.ul_steering_lock_time = */180,       //unit:min
    /*.c_5g_sta_rssi_th =  */-80,           //unit:db
    /*.uc_reserv =  */0,
    /*.us_new_user_freeze_time =  */120,                //默认120s，uint:s
    /*.ul_steering_blocktime = */BSD_STEERING_TIMEOUT   //默认60*1000 uint:ms
};


//默认配置参数
OAL_STATIC bsd_cfg_param_index_stru  ast_dmac_bsd_cfg_str[CFG_PARAM_INDEX_BUTT] =
{
    {"busy_th", CFG_PARAM_INDEX_BUSY_TH},
    {"free_th", CFG_PARAM_INDEX_FREE_TH},
    {"thru_th", CFG_PARAM_INDEX_THRU_TH},
    {"rssi_th", CFG_PARAM_INDEX_RSSI_TH},
    {"sfre_th", CFG_PARAM_INDEX_SFRE_TH},
    {"lock_time", CFG_PARAM_INDEX_LOCK_TIME},
    {"new_user_lock_time", CFG_PARAM_INDEX_NEW_USER_LOCK_TIME},
    {"block_time", CFG_PARAM_INDEX_BLOCK_TIME},
};

//宏定义
#define DMAC_BSD_CALCULATE_HASH_VALUE(_puc_mac_addr)\
    ((_puc_mac_addr[0] + _puc_mac_addr[1]           \
    + _puc_mac_addr[2] + _puc_mac_addr[3]           \
    + _puc_mac_addr[4] + _puc_mac_addr[5])          \
    & (BSD_STA_INFO_HASH_TBL_SIZE - 1))


//[0-1000], 越大表示信道越忙
/*
//理论上
信道测量时间 = 信道空闲时间+        //能量低于CCA门限
               空口的干扰时间+      //能量高于CCA门限，但是无法解析的能量信号(临频/叠频干扰)
               信道发送时间+        //自身竞争获取到的发送时间
               信道接收时间+        //空口中别的节点发送的时间
               蓝牙中断的时间(optional)//蓝牙共存时，蓝牙收发占用的时间


所以通过测量到的信道空闲时间就可以用来表征当前信道的繁忙度。
*/
#define DMAC_BSD_DET_GET_LOAD_RATIO(_pst_chan_result, _ul_total_free_time)\
    (((_ul_total_free_time)>((_pst_chan_result)->ul_total_stats_time_us))   \
    ?0:((((_pst_chan_result)->ul_total_stats_time_us)-(_ul_total_free_time))\
    *1000/((_pst_chan_result)->ul_total_stats_time_us)))

#define BSD_BUSY_TH(pbsd_ap) ((pbsd_ap)->st_bsd_config.ul_vap_load_busy_th)
#define BSD_FREE_TH(pbsd_ap) ((pbsd_ap)->st_bsd_config.ul_vap_load_free_th)
#define BSD_RSSI_TH(pbsd_ap) ((pbsd_ap)->st_bsd_config.c_5g_sta_rssi_th)
#define BSD_THRU_TH(pbsd_ap) ((pbsd_ap)->st_bsd_config.ul_sta_steering_thr_th)


//连续切换失败次数大于此阀值时，该STA将被标记为steer unfrinedly,不再调度该用户进行steering
#define BSD_STEER_UNFRIENDLY_TH         2

//BSD AP配置的阀值参数中定义信道空闲与繁忙的阀值之间的差值阀值
#define BSD_CFG_FREE_BUSY_DELTA_TH  100

//用户的状态统计时间
#define DMAC_BSD_USER_STAT_TIME     500

//bsd task调度周期
#define BSD_TASK_TIMEOUT    15000        //默认15s调度一次

/* 关联阶段的阻塞特性失效时间*/
#define DMAC_BSD_PRE_STEER_INVALID_TIMEOUT      60*1000

/*关联阶段steer超时时间 单位:ms*/
#define DMAC_BSD_PRE_STEER_TIMEOUT      10*1000

//2s之内收到某用户在两个不同频段的probe req则认为该sta具备双频能力
#define DMAC_BSD_STA_CAP_CLASSIFY_TIME_TH   2*1000

//同频段内连续收到某用户在同一个频段内5次probe req,则暂时认为该sta只具备单频能力
#define DMAC_BSD_STA_CAP_CLASSIFY_CNT_TH    5

//判断切换频段的时间窗
#define DMAC_BSD_STEER_FRE_PERIOD    30*60*1000

#ifdef _PRE_DEBUG_MODE
//OAL_STATIC oal_void dmac_bsd_dump_nodeinfo(oal_uint8 *puc_node_addr);
#endif

OAL_STATIC  oal_void dmac_bsd_ringbuf_init(dmac_device_bsd_ringbuf *pbuf)
{
    OAL_MEMZERO(pbuf, sizeof(dmac_device_bsd_ringbuf));
    pbuf->ul_size =  BSD_VAP_LOAD_SAMPLE_MAX_CNT;
    pbuf->ul_in = pbuf->ul_out = 0;
    return;
}


OAL_STATIC oal_uint32 mac_bsd_ringbuf_get(dmac_device_bsd_ringbuf *pst_buf, oal_uint16 *pus_value,oal_uint32 ul_len)
{
    oal_uint32 ul_l;

    ul_len = OAL_MIN(ul_len, pst_buf->ul_in - pst_buf->ul_out);
    if(ul_len)
    {
        ul_l = OAL_MIN(ul_len, pst_buf->ul_size - (pst_buf->ul_out & (pst_buf->ul_size - 1)));
        oal_memcopy((oal_void*)pus_value, (oal_void*)&pst_buf->aus_buf[(pst_buf->ul_out & (pst_buf->ul_size - 1))], ul_l*OAL_SIZEOF(oal_uint16));
        oal_memcopy((oal_void*)&pus_value[ul_l], (oal_void*)pst_buf->aus_buf, (ul_len - ul_l)*OAL_SIZEOF(oal_uint16));
        pst_buf->ul_out += ul_len;
    }
    return ul_len;
}


OAL_STATIC oal_uint16 mac_bsd_ringbuf_get_last(dmac_device_bsd_ringbuf *pst_buf)
{
    if(pst_buf->ul_out)
    {
        return pst_buf->aus_buf[(pst_buf->ul_out-1) & (pst_buf->ul_size - 1)];
    }

    return 0;
}



OAL_STATIC oal_void dmac_bsd_ringbuf_put(dmac_device_bsd_ringbuf *pst_buf, oal_uint16 us_value)
{
    if(pst_buf->ul_size == (pst_buf->ul_in - pst_buf->ul_out))
    {
        //full
        pst_buf->aus_buf[pst_buf->ul_in & (pst_buf->ul_size - 1)] = us_value;
        pst_buf->ul_in ++;
        pst_buf->ul_out ++;
    }
    else
    {
        pst_buf->aus_buf[pst_buf->ul_in & (pst_buf->ul_size - 1)] = us_value;
        pst_buf->ul_in ++;
    }
    return;
}



oal_void dmac_bsd_device_load_scan_cb(oal_void *p_param)
{
    wlan_scan_chan_stats_stru       *pst_chan_result;
    mac_device_stru                 *pst_mac_device;
    dmac_device_stru                *pst_dmac_device;
    oal_uint16                      us_ratio;

    /* 判断入参合法性 */
    if (OAL_PTR_NULL == p_param)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_load_scan_cb: input pointer is null!}");
        return;
    }

    //BSD特性动态开关关闭或者BSD AP没有up起来都直接返回
    if((!g_st_bsd_mgmt.en_switch)
     ||(!g_st_bsd_mgmt.st_bsd_sched_timer.en_is_registerd))
    {
         return;
    }
    pst_mac_device = (mac_device_stru*)p_param;
    pst_dmac_device = dmac_res_get_mac_dev(pst_mac_device->uc_device_id);
    if(OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_load_scan_cb: pst_dmac_device null!}");
        return;
    }

    pst_chan_result = &((DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device))->st_chan_result);

    /*获取统计的信道结果:繁忙度/千分之X  越大越忙*/
    if(0 == pst_chan_result->ul_total_stats_time_us)
    {
        us_ratio = 0;
    }
    else
    {
        us_ratio = (oal_uint16)(DMAC_BSD_DET_GET_LOAD_RATIO(pst_chan_result, pst_chan_result->ul_total_free_time_20M_us));
    }

    dmac_bsd_ringbuf_put(&(pst_dmac_device->st_bsd.st_sample_result_buf),us_ratio);

    return;
}



oal_void  dmac_bsd_init(oal_void)
{
    oal_uint32      ul_idx;

    OAL_MEMZERO((oal_void*)&g_st_bsd_mgmt,OAL_SIZEOF(bsd_mgmt_stru));

    oal_dlist_init_head(&g_st_bsd_mgmt.st_bsd_ap_list_head);

    //OAL_MEMZERO((oal_void*)&g_st_bsd_mgmt.st_sta_info_tbl,OAL_SIZEOF(bsd_sta_tbl_stru));

    for(ul_idx = 0; ul_idx < BSD_STA_INFO_HASH_TBL_SIZE; ul_idx++)
    {
        oal_dlist_init_head(&g_st_bsd_mgmt.st_sta_info_tbl.ast_hash_tbl[ul_idx]);
    }

    oal_dlist_init_head(&g_st_bsd_mgmt.st_sta_info_tbl.st_list_head);

    OAL_MEMZERO((oal_void*)&g_st_bsd_mgmt.st_steering_ctl.auc_mac_addr,WLAN_MAC_ADDR_LEN);
    g_st_bsd_mgmt.st_steering_ctl.uc_state = 0;
    g_st_bsd_mgmt.en_switch = OAL_TRUE;     //默认开启
    g_st_bsd_mgmt.bit_load_status_switch = 0;
    g_st_bsd_mgmt.bit_pre_steer_swtich = 1;
    g_st_bsd_mgmt.bit_steer_mode = 0;       //默认使用mode 0进行band steering
    g_st_bsd_mgmt.bit_cfg_switch = 0;       //Bandsteering的配置开关默认关闭

    oal_dlist_init_head(&g_st_bsd_mgmt.st_pre_steering_ctl.st_pre_steering_list);

    g_st_bsd_mgmt.st_pre_steering_ctl.pst_cache_user_node = OAL_PTR_NULL;
}


oal_void  dmac_bsd_exit(oal_void)
{
    return;
}



OAL_STATIC oal_uint32  dmac_bsd_config_set(bsd_ap_stru *pst_bsd_ap,OAL_CONST bsd_cfg_stru *pst_cfg)
{
    //检查配置参数基本的约束
    if(((pst_cfg->ul_vap_load_busy_th - pst_cfg->ul_vap_load_free_th) < BSD_CFG_FREE_BUSY_DELTA_TH)
       ||(pst_cfg->ul_vap_load_busy_th > DMAC_BSD_CFG_LOAD_TH_MAX))
    {
        return OAL_FAIL;
    }

    if(pst_cfg->c_5g_sta_rssi_th > 0)
    {
        return OAL_FAIL;
    }


    oal_memcopy((oal_void*)&(pst_bsd_ap->st_bsd_config), (oal_void*)pst_cfg, OAL_SIZEOF(bsd_cfg_stru));
	return OAL_SUCC;
}



OAL_STATIC oal_uint32 dmac_bsd_device_load_monitor_init(dmac_vap_stru  *pst_dmac_vap)
{
    dmac_device_stru             *pst_dmac_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_load_monitor_init::pst_dmac_vap null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_vap_load_monitor_init::dev null, dev id[%d].}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(pst_dmac_device->st_bsd.uc_state)
    {
        return OAL_SUCC;
    }

    dmac_bsd_ringbuf_init(&(pst_dmac_device->st_bsd.st_sample_result_buf));
    pst_dmac_device->st_bsd.uc_state = 1;

    return OAL_SUCC;
}


OAL_STATIC oal_void dmac_bsd_device_load_monitor_deinit(dmac_vap_stru  *pst_dmac_vap)
{
    return;
}
#if 0

OAL_STATIC oal_uint32  dmac_bsd_stat_user_info_timer_handle(dmac_stat_param_stru * pst_output_param)
{
	return OAL_SUCC;
}
#endif


OAL_STATIC oal_uint32  dmac_bsd_user_state_init(dmac_vap_stru  *pst_dmac_vap,dmac_user_stru *pst_dmac_user)
{
    pst_dmac_user->st_bsd.ull_last_rx_bytes = pst_dmac_user->st_dmac_thrpt_stat_info.ull_rx_bytes;
    pst_dmac_user->st_bsd.ull_last_tx_bytes = pst_dmac_user->st_dmac_thrpt_stat_info.ull_tx_bytes;
    pst_dmac_user->st_bsd.ul_associate_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    pst_dmac_user->st_bsd.en_just_assoc_flag = OAL_TRUE;
    return OAL_SUCC;
}



OAL_STATIC bsd_ap_stru *dmac_bsd_vap_find(dmac_vap_stru *pst_src_vap)
{
    bsd_ap_stru                 *pst_bsd_ap;
    oal_dlist_head_stru         *pst_tmp_node;

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_src_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_find:param null!}");
        return OAL_PTR_NULL;
    }

    /* 遍历所有bsd ap */
    OAL_DLIST_SEARCH_FOR_EACH(pst_tmp_node,&(g_st_bsd_mgmt.st_bsd_ap_list_head))
    {
        /* 获取bsd ap*/
        pst_bsd_ap = OAL_DLIST_GET_ENTRY(pst_tmp_node,bsd_ap_stru,st_entry);
        if((pst_bsd_ap->past_dmac_vap[0] == pst_src_vap)
          ||(pst_bsd_ap->past_dmac_vap[1] == pst_src_vap))
        {
            return  pst_bsd_ap;
        }
    }

    return OAL_PTR_NULL;
}


OAL_STATIC oal_uint32 dmac_bsd_vap_find_ext(dmac_vap_stru *pst_src_vap)
{
    bsd_ap_stru                 *pst_bsd_ap;
    dmac_vap_stru               *pst_another_vap;
    oal_dlist_head_stru         *pst_tmp_node;
    oal_uint8                   *puc_vap_ssid;
    oal_uint8                   *puc_src_ssid;

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_src_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_find_ext:param null!}");
        return OAL_PTR_NULL;
    }

    /* 遍历所有bsd ap */
    OAL_DLIST_SEARCH_FOR_EACH(pst_tmp_node,&(g_st_bsd_mgmt.st_bsd_ap_list_head))
    {
        /* 获取bsd ap*/
        pst_bsd_ap = OAL_DLIST_GET_ENTRY(pst_tmp_node,bsd_ap_stru,st_entry);
        if((pst_bsd_ap->past_dmac_vap[0] == pst_src_vap)
           ||(pst_bsd_ap->past_dmac_vap[1] == pst_src_vap))
        {
            OAM_WARNING_LOG1(pst_src_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_vap_find_ext:the src vap had add into the bsd ap[%p]!}",pst_bsd_ap);
            return OAL_SUCC;
        }

        if((pst_bsd_ap->past_dmac_vap[0] != OAL_PTR_NULL)&&(pst_bsd_ap->past_dmac_vap[1] != OAL_PTR_NULL))
        {
            OAM_WARNING_LOG1(0, OAM_SF_BSD, "{dmac_bsd_vap_find_ext:the bsd ap[%p] had already match!}",pst_bsd_ap);
            continue;
        }

        pst_another_vap = (pst_bsd_ap->past_dmac_vap[0] != OAL_PTR_NULL) ? pst_bsd_ap->past_dmac_vap[0]:pst_bsd_ap->past_dmac_vap[1];

        puc_src_ssid = mac_mib_get_DesiredSSID(&(pst_src_vap->st_vap_base_info));
        puc_vap_ssid = mac_mib_get_DesiredSSID(&(pst_another_vap->st_vap_base_info));

        if((!oal_strcmp((oal_int8*)puc_src_ssid,(oal_int8*)puc_vap_ssid))
            &&(pst_src_vap->st_vap_base_info.st_channel.en_band != pst_another_vap->st_vap_base_info.st_channel.en_band))
        {
            if(pst_bsd_ap->past_dmac_vap[0] != OAL_PTR_NULL)
            {
                pst_bsd_ap->past_dmac_vap[1] = pst_src_vap;
            }
            else
            {
                pst_bsd_ap->past_dmac_vap[0] = pst_src_vap;
            }

            return OAL_SUCC;
        }
    }
    return OAL_FAIL;
}


OAL_STATIC oal_uint32 dmac_bsd_ap_list_check(oal_void)
{
    bsd_ap_stru                 *pst_bsd_ap;
    oal_dlist_head_stru         *pst_tmp_node;
    oal_dlist_head_stru         *pst_entry_tmp;
    oal_uint32                  ul_bsd_ap_cnt = 0;

    /* 遍历所有bsd ap */
    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_tmp_node,pst_entry_tmp,&(g_st_bsd_mgmt.st_bsd_ap_list_head))
    {
        /* 获取bsd ap*/
        pst_bsd_ap = OAL_DLIST_GET_ENTRY(pst_tmp_node,bsd_ap_stru,st_entry);
        if((pst_bsd_ap->past_dmac_vap[0] == OAL_PTR_NULL)
          ||(pst_bsd_ap->past_dmac_vap[1] == OAL_PTR_NULL))
        {
            OAM_WARNING_LOG0(0, OAM_SF_BSD, "dmac_bsd_ap_list_check:the bsd ap node is not match,will be free");
            oal_dlist_delete_entry(&(pst_bsd_ap->st_entry));
            OAL_MEM_FREE(pst_bsd_ap,OAL_TRUE);
            continue;
        }

        if((dmac_bsd_device_load_monitor_init(pst_bsd_ap->past_dmac_vap[0]) != OAL_SUCC)
          ||(dmac_bsd_device_load_monitor_init(pst_bsd_ap->past_dmac_vap[1]) != OAL_SUCC))
        {
            OAM_ERROR_LOG0(pst_bsd_ap->past_dmac_vap[0]->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_ap_list_check::dmac_bsd_vap_load_monitor_init fail!");
            oal_dlist_delete_entry(&(pst_bsd_ap->st_entry));
            OAL_MEM_FREE(pst_bsd_ap,OAL_TRUE);
            continue;
        }

        if(dmac_bsd_config_set(pst_bsd_ap, &dmac_bsd_default_cfg) != OAL_SUCC)
        {
            OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_ap_list_check::bsd ap set defualt cfg fail!");
            oal_dlist_delete_entry(&(pst_bsd_ap->st_entry));
            OAL_MEM_FREE(pst_bsd_ap,OAL_TRUE);
            continue;
        }

        ul_bsd_ap_cnt++;
    }

    return ul_bsd_ap_cnt;
}


OAL_STATIC oal_uint32  dmac_bsd_config_set_ext(dmac_vap_stru *pst_dmac_vap,
                                               dmac_bsd_cfg_param_index_enum_uint8 en_index,
                                               oal_uint32   ul_value)
{
    bsd_ap_stru                 *pst_bsd_ap;

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(0,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext::param null.");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_bsd_ap = dmac_bsd_vap_find(pst_dmac_vap);
    if(OAL_PTR_NULL == pst_bsd_ap)
    {

        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: the vap is not bsd ap.");
        return OAL_FAIL;
    }

    switch(en_index)
    {
        case CFG_PARAM_INDEX_BUSY_TH:
            if((ul_value < (pst_bsd_ap->st_bsd_config.ul_vap_load_free_th + BSD_CFG_FREE_BUSY_DELTA_TH))
                ||(ul_value > DMAC_BSD_CFG_LOAD_TH_MAX))
            {
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: busy_th shoud not larger than 1000 or lower than (free_th+100).");
                return OAL_FAIL;
            }
            pst_bsd_ap->st_bsd_config.ul_vap_load_busy_th = ul_value;
            break;
        case CFG_PARAM_INDEX_FREE_TH:
            if(ul_value > (pst_bsd_ap->st_bsd_config.ul_vap_load_busy_th - BSD_CFG_FREE_BUSY_DELTA_TH))
            {
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: free_th shoud not larger than (busy_th-100).");
                return OAL_FAIL;
            }
            pst_bsd_ap->st_bsd_config.ul_vap_load_free_th = ul_value;
            break;
        case CFG_PARAM_INDEX_THRU_TH:
            if(0 == ul_value)
            {
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: thru_th shoud not be zero.");
                return OAL_FAIL;
            }
            pst_bsd_ap->st_bsd_config.ul_sta_steering_thr_th = ul_value;
            break;
        case CFG_PARAM_INDEX_RSSI_TH:
            if((ul_value < DMAC_BSD_CFG_RSSI_TH_MIN)||(ul_value > DMAC_BSD_CFG_RSSI_TH_MAX))
            {
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: rssi_th shoud not larger than -20 or lower than -100.");
                return OAL_FAIL;
            }
            pst_bsd_ap->st_bsd_config.c_5g_sta_rssi_th = (oal_int8)(-(oal_uint8)ul_value);
            break;
        case CFG_PARAM_INDEX_SFRE_TH:
            if((ul_value == 0)||(ul_value > 10))
            {
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: sfre_th shoud not larger than 10 or zero.");
                return OAL_FAIL;
            }
            pst_bsd_ap->st_bsd_config.ul_steering_fre_th = ul_value;
            break;
        case CFG_PARAM_INDEX_LOCK_TIME:
            if(ul_value == 0)
            {
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: lock_time shoud not be zero.");
                return OAL_FAIL;
            }
            pst_bsd_ap->st_bsd_config.ul_steering_lock_time = ul_value;
            break;
        case CFG_PARAM_INDEX_NEW_USER_LOCK_TIME:
            if(ul_value == 0)
            {
                OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: new_user_lock_time shoud not be zero.");
                return OAL_FAIL;
            }
            pst_bsd_ap->st_bsd_config.us_new_user_lock_time = (oal_uint16)ul_value;
            break;
         case CFG_PARAM_INDEX_BLOCK_TIME:
            if((ul_value == 0)||(ul_value > 300))
            {
                OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: block_time[%d] not in (0,300].",ul_value);
                return OAL_FAIL;
            }
            pst_bsd_ap->st_bsd_config.ul_steering_blocktime = (oal_uint16)ul_value * 1000;
            break;
        default:
            OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext:: invalid param index(%d).",en_index);
            return OAL_FAIL;
    }

    OAM_WARNING_LOG2(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_BSD,
                               "dmac_bsd_config_set_ext::param(%d)=%d.",en_index,ul_value);
    return OAL_SUCC;
}



oal_uint32  dmac_bsd_config_get(bsd_ap_stru *pst_bsd_ap,bsd_cfg_stru *pst_cfg)
{
    oal_memcopy((oal_void*)pst_cfg,(oal_void*)&(pst_bsd_ap->st_bsd_config),OAL_SIZEOF(bsd_cfg_stru));
    return OAL_SUCC;
}


oal_bool_enum_uint8 dmac_bsd_get_capability(oal_void)
{
    if(g_st_bsd_mgmt.st_bsd_sched_timer.en_is_registerd)
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}




OAL_STATIC oal_uint32  dmac_bsd_vap_load_get(dmac_vap_stru * pst_dmac_vap,oal_uint16 *us_ret_value)
{
    oal_uint16   aus_load[BSD_VAP_LOAD_SAMPLE_MAX_CNT];
    oal_uint32   ul_sample_cnt;
    oal_uint32   ul_index;
    oal_uint32   ul_total;
    dmac_device_stru    *pst_dmac_device;
#ifdef _PRE_DEBUG_MODE
    oal_uint8           uc_2g_dummmy_load;
    oal_uint8           uc_5g_dummmy_load;
    if(g_st_bsd_mgmt.bit_debug_mode)
    {
        uc_2g_dummmy_load = g_st_bsd_mgmt.uc_debug1&0x0f;
        uc_5g_dummmy_load = (g_st_bsd_mgmt.uc_debug1&0xf0) >> 4;

        if((WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)&&(uc_2g_dummmy_load))
        {
            *us_ret_value = uc_2g_dummmy_load*100;
            return OAL_SUCC;
        }

        if((WLAN_BAND_5G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)&&(uc_5g_dummmy_load))
        {
            *us_ret_value = uc_5g_dummmy_load*100;
            return OAL_SUCC;
        }
    }
#endif

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_load_get::pst_dmac_vap null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_vap_load_get::dev null, dev id[%d].}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    ul_sample_cnt = mac_bsd_ringbuf_get(&(pst_dmac_device->st_bsd.st_sample_result_buf),aus_load,BSD_VAP_LOAD_SAMPLE_MAX_CNT);

    if(0 == ul_sample_cnt)
    {
        //如果从ringbuf没有读取到数据，说明ringbuf的数据自最近一次读取之后没有来得及写入新的数据
        //所以取最近一次的数据当做当前的结果
        *us_ret_value = mac_bsd_ringbuf_get_last(&(pst_dmac_device->st_bsd.st_sample_result_buf));
    }
    else
    {
        for(ul_total = 0,ul_index = 0; ul_index < ul_sample_cnt; ul_index++)
        {
            ul_total += aus_load[ul_index];
        }

        *us_ret_value = (oal_uint16)(ul_total/ul_sample_cnt);
    }
    return OAL_SUCC;
}


#define DMAC_BSD_UINT32_MAX   (0xffffffff)
#define DMAC_BSD_CALC_DELTA_UINT32(_l_start, _l_end)  ((DMAC_BSD_UINT32_MAX) - (_l_start) + (_l_end))
#define DMAC_BSD_GET_DELTA_UINT32(_l_start, _l_end) \
    (((_l_start) > (_l_end))?(OAL_TIME_CALC_RUNTIME((_l_start), (_l_end))):((_l_end) - (_l_start)))

#define DMAC_BSD_GET_DELTA_UINT64(_l_start, _l_end) ((_l_end) - (_l_start))


OAL_STATIC oal_void  dmac_bsd_user_state_refresh(dmac_user_stru * pst_dmac_user,bsd_ap_stru *pst_bsd_ap,oal_uint32 ul_valid_period_th)
{
    oal_uint32      ul_timestamp;
    oal_uint64      ull_delta_bytes;
    oal_uint32      ul_delta_time;

    ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    ul_delta_time = OAL_TIME_GET_RUNTIME(pst_dmac_user->st_bsd.ul_associate_timestamp,ul_timestamp);
#ifdef _PRE_DEBUG_MODE
    if(g_st_bsd_mgmt.bit_debug_mode)
    {
        OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_user_state_refresh::user[xx:xx:xx:%02x:%02x:%02x] stat refresh!",
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);

        OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_user_state_refresh::assoc_timestamp=%d,current_timestamp=%d,delta_time=%d",
                         pst_dmac_user->st_bsd.ul_associate_timestamp,
                         ul_timestamp,
                         ul_delta_time);

    }
#endif


#ifdef _PRE_DEBUG_MODE
    if(g_st_bsd_mgmt.bit_debug_mode)
    {
        OAM_WARNING_LOG4(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_user_state_refresh::last_rx_thr=%d,last_tx_thr=%d,current_rx_thr=%d,current_tx_thr=%d",
                             pst_dmac_user->st_bsd.ull_last_rx_bytes,
                             pst_dmac_user->st_bsd.ull_last_tx_bytes,
                             pst_dmac_user->st_dmac_thrpt_stat_info.ull_rx_bytes,
                             pst_dmac_user->st_dmac_thrpt_stat_info.ull_tx_bytes);

    }
#endif
    ull_delta_bytes = DMAC_BSD_GET_DELTA_UINT64(pst_dmac_user->st_bsd.ull_last_rx_bytes,pst_dmac_user->st_dmac_thrpt_stat_info.ull_rx_bytes);
    ull_delta_bytes *=8;
    pst_dmac_user->st_bsd.ul_average_rx = (oal_uint32)oal_div_u64(ull_delta_bytes,BSD_TASK_TIMEOUT);

    ull_delta_bytes = DMAC_BSD_GET_DELTA_UINT64(pst_dmac_user->st_bsd.ull_last_tx_bytes,pst_dmac_user->st_dmac_thrpt_stat_info.ull_tx_bytes);
    ull_delta_bytes *=8;
    pst_dmac_user->st_bsd.ul_average_tx = (oal_uint32)oal_div_u64(ull_delta_bytes,BSD_TASK_TIMEOUT);

    pst_dmac_user->st_bsd.ull_last_rx_bytes = pst_dmac_user->st_dmac_thrpt_stat_info.ull_rx_bytes;
    pst_dmac_user->st_bsd.ull_last_tx_bytes = pst_dmac_user->st_dmac_thrpt_stat_info.ull_tx_bytes;
    pst_dmac_user->st_bsd.uc_valid = 1;
    if((ul_delta_time < ul_valid_period_th)
        ||((ul_delta_time < pst_bsd_ap->st_bsd_config.us_new_user_lock_time*DMAC_BSD_LOCK_TIME_UNIT_MS)
           &&(OAL_TRUE == pst_dmac_user->st_bsd.en_just_assoc_flag)))
    {
 #ifdef _PRE_DEBUG_MODE
        if(g_st_bsd_mgmt.bit_debug_mode)
        {
            OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_user_state_refresh::the user stat period is not enough or just assoc!}");
        }
#endif
        pst_dmac_user->st_bsd.uc_valid = 0;
    }
    else if((ul_delta_time >= pst_bsd_ap->st_bsd_config.us_new_user_lock_time*DMAC_BSD_LOCK_TIME_UNIT_MS)
           &&(OAL_TRUE == pst_dmac_user->st_bsd.en_just_assoc_flag))
    {
        pst_dmac_user->st_bsd.en_just_assoc_flag = OAL_FALSE;
    }
#ifdef _PRE_DEBUG_MODE
    if(g_st_bsd_mgmt.bit_debug_mode)
    {
        OAM_WARNING_LOG2(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_user_state_refresh::rx_thr=%d,tx_thr=%d",
                         pst_dmac_user->st_bsd.ul_average_rx,
                         pst_dmac_user->st_bsd.ul_average_tx);
    }
#endif
    return;
}




OAL_STATIC oal_uint32  dmac_bsd_user_state_get(dmac_user_stru * pst_dmac_user,
                                             dmac_user_bsd_info_stru *pst_user_state_info)
{
    #if 0
    //dummy for debug
    oal_uint32              ul_timestamp;

    //ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    ul_timestamp = (oal_uint32)(OAL_TIME_JIFFY/100);
    pst_user_state_info->c_average_rssi = -10*(4+(ul_timestamp%6));      //[-40 -- -90]
    pst_user_state_info->ul_average_rx_thru = (3+(ul_timestamp%3)); //[3-5]
    pst_user_state_info->ul_average_tx_thru = (3+(ul_timestamp%3)); //[3-5]
    pst_user_state_info->ul_best_rate = 1000*(1+(ul_timestamp%10));                        //[1000-10000]
    #endif

    if(!pst_dmac_user->st_bsd.uc_valid)
    {
        return OAL_FAIL;
    }

    pst_user_state_info->ul_average_tx_thru= pst_dmac_user->st_bsd.ul_average_tx;
    pst_user_state_info->ul_average_rx_thru = pst_dmac_user->st_bsd.ul_average_rx;
    pst_user_state_info->c_average_rssi = oal_get_real_rssi(pst_dmac_user->s_rx_rssi);
    dmac_alg_get_tx_best_rate(&(pst_dmac_user->st_user_base_info),
                              WLAN_WME_AC_BE,
                              &(pst_user_state_info->ul_best_rate));

#ifdef _PRE_DEBUG_MODE
    if(g_st_bsd_mgmt.bit_debug_mode)
    {
        OAM_WARNING_LOG4(0, OAM_SF_BSD, "{dmac_bsd_user_state_get::rssi=-%d,rx_thr=%d,tx_thr=%d,phyrate=%d",
                         (256-(oal_uint8)pst_user_state_info->c_average_rssi),
                         pst_user_state_info->ul_average_rx_thru,
                         pst_user_state_info->ul_average_tx_thru,
                         pst_user_state_info->ul_best_rate);
    }
#endif

    return OAL_SUCC;
}


#define DMAC_BSD_FOR_EACH_CHIP(_uc_chip_idx)   \
    for ((_uc_chip_idx) = 0; (_uc_chip_idx) < WLAN_CHIP_MAX_NUM_PER_BOARD; (_uc_chip_idx)++)

#define DMAC_BSD_FOR_EACH_HAL_DEV(_uc_dev_idx)   \
        for ((_uc_dev_idx) = 0; (_uc_dev_idx) < WLAN_DEVICE_MAX_NUM_PER_CHIP; (_uc_dev_idx)++)


OAL_STATIC dmac_vap_stru *dmac_bsd_vap_search(dmac_vap_stru *pst_src_vap)
{
    oal_uint8                   uc_chip_idx;
    oal_uint8                   uc_device_idx;
    dmac_vap_stru               *pst_dmac_vap;
    oal_uint8                   uc_vap_idx;
    oal_uint8                   *puc_vap_ssid;
    oal_uint8                   *puc_src_ssid;
    mac_vap_stru                *pst_mac_vap;
    oal_uint8                   uc_up_vap_num;
    oal_uint32                  ul_ret;
    hal_to_dmac_device_stru     *pst_hal_device;
    oal_uint8                   auc_mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE];

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_src_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_search:param null!}");
        return OAL_PTR_NULL;
    }

    puc_src_ssid = mac_mib_get_DesiredSSID(&(pst_src_vap->st_vap_base_info));
    if(OAL_PTR_NULL == puc_src_ssid)
    {
        OAM_WARNING_LOG0(pst_src_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_vap_search:src vap ssid null!}");
        return OAL_PTR_NULL;
    }

    DMAC_BSD_FOR_EACH_CHIP(uc_chip_idx)
    {
        DMAC_BSD_FOR_EACH_HAL_DEV(uc_device_idx)
        {
            ul_ret = hal_chip_get_hal_device(uc_chip_idx, uc_device_idx, &pst_hal_device);
            if ((OAL_SUCC != ul_ret) || (OAL_PTR_NULL == pst_hal_device))
            {
                OAM_WARNING_LOG2(0, OAM_SF_BSD, "{dmac_bsd_vap_search::hal_chip_get_hal_device fail!! ret:%d!hal device[%p]}", ul_ret, pst_hal_device);
                continue;
            }

            uc_up_vap_num = hal_device_find_all_up_vap(pst_hal_device, auc_mac_vap_id);
            for (uc_vap_idx = 0; uc_vap_idx < uc_up_vap_num; uc_vap_idx++)
            {
                pst_mac_vap  = (mac_vap_stru *)mac_res_get_mac_vap(auc_mac_vap_id[uc_vap_idx]);
                if (OAL_PTR_NULL == pst_mac_vap)
                {
                    OAM_ERROR_LOG1(0, OAM_SF_BSD, "dmac_bsd_vap_search::pst_mac_vap[%d] IS NULL.", auc_mac_vap_id[uc_vap_idx]);
                    continue;
                }

                pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

                if((pst_dmac_vap == pst_src_vap)||(pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP))
                {
                    continue;
                }

                puc_vap_ssid = mac_mib_get_DesiredSSID(pst_mac_vap);
                if(puc_vap_ssid == OAL_PTR_NULL)
                {
                    //should not be here!
                    continue;
                }

                if(oal_strcmp((oal_int8*)puc_vap_ssid, (oal_int8*)puc_src_ssid))
                {
                    continue;
                }

                if(pst_mac_vap->st_channel.en_band != pst_src_vap->st_vap_base_info.st_channel.en_band)
                {
                    OAM_WARNING_LOG1(0, OAM_SF_BSD, "find bsd vap(%d) in the other band!", pst_src_vap->st_vap_base_info.uc_vap_id);
                    return pst_dmac_vap;
                }
                else
                {
                    //there maybe something error!
                    OAM_ERROR_LOG1(0, OAM_SF_BSD, "find same bsd vap in the same band! vap id is %d", auc_mac_vap_id[uc_vap_idx]);
                }
            }
        }
    }

    OAM_WARNING_LOG1(pst_src_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                     "not find bsd vap in the other band! src vap is %dG vap",
                     (WLAN_BAND_2G == pst_src_vap->st_vap_base_info.st_channel.en_band)?2:5);
    return OAL_PTR_NULL;
}


OAL_STATIC oal_uint32 dmac_bsd_vap_match(oal_void)
{
    oal_uint8                   uc_chip_idx;
    oal_uint8                   uc_device_idx;
    dmac_vap_stru               *pst_dmac_vap;
    oal_uint8                   uc_vap_idx;
    mac_vap_stru                *pst_mac_vap;
    oal_uint8                   uc_up_vap_num;
    oal_uint32                  ul_ret;
    hal_to_dmac_device_stru     *pst_hal_device;
    oal_uint8                   auc_mac_vap_id[WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE];
    bsd_ap_stru                 *pst_bsd_ap;


    DMAC_BSD_FOR_EACH_CHIP(uc_chip_idx)
    {
        DMAC_BSD_FOR_EACH_HAL_DEV(uc_device_idx)
        {
            ul_ret = hal_chip_get_hal_device(uc_chip_idx, uc_device_idx, &pst_hal_device);
            if ((OAL_SUCC != ul_ret) || (OAL_PTR_NULL == pst_hal_device))
            {
                OAM_WARNING_LOG2(0, OAM_SF_BSD, "{dmac_bsd_vap_match::hal_chip_get_hal_device fail!! ret:%d!hal device[%p]}", ul_ret, pst_hal_device);
                continue;
            }

            uc_up_vap_num = hal_device_find_all_up_vap(pst_hal_device, auc_mac_vap_id);
            for (uc_vap_idx = 0; uc_vap_idx < uc_up_vap_num; uc_vap_idx++)
            {
                pst_mac_vap  = (mac_vap_stru *)mac_res_get_mac_vap(auc_mac_vap_id[uc_vap_idx]);
                if (OAL_PTR_NULL == pst_mac_vap)
                {
                    OAM_ERROR_LOG1(0, OAM_SF_BSD, "dmac_bsd_vap_match::pst_mac_vap[%d] IS NULL.", auc_mac_vap_id[uc_vap_idx]);
                    continue;
                }

                pst_dmac_vap = MAC_GET_DMAC_VAP(pst_mac_vap);

                if(pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP)
                {
                    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_BSD, "dmac_bsd_vap_match:vap state[%d] or vap mode[%d] error", pst_mac_vap->en_vap_state,pst_mac_vap->en_vap_mode);
                    continue;
                }

                if(OAL_PTR_NULL == mac_mib_get_DesiredSSID(pst_mac_vap))
                {
                    //should not be here!
                    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BSD, "dmac_bsd_vap_match:get vap ssid fail");
                    continue;
                }

                //搜索bsd ap链表，看是否能配对组成一个bsd ap
                ul_ret = dmac_bsd_vap_find_ext(pst_dmac_vap);
                if(OAL_FAIL == ul_ret)
                {
                    OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BSD, "dmac_bsd_vap_match:no bsd ap node found,add a new bsd ap node");
                    //将此vap暂时挂在bsd ap链表上
                    pst_bsd_ap = (bsd_ap_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(bsd_ap_stru), OAL_TRUE);
                    if(pst_bsd_ap == OAL_PTR_NULL)
                    {
                        continue;
                    }
                    OAL_MEMZERO((oal_void*)pst_bsd_ap,OAL_SIZEOF(bsd_ap_stru));

                    if(WLAN_BAND_5G == pst_mac_vap->st_channel.en_band)
                    {
                        pst_bsd_ap->past_dmac_vap[1] = pst_dmac_vap;
                    }
                    else
                    {
                        pst_bsd_ap->past_dmac_vap[0] = pst_dmac_vap;
                    }

                    oal_dlist_add_tail(&(pst_bsd_ap->st_entry), &g_st_bsd_mgmt.st_bsd_ap_list_head);
                }
            }
        }
    }
    //整理bsd ap的链表，将没有配对的节点从链表删除
    return dmac_bsd_ap_list_check();
}



OAL_STATIC oal_void dmac_bsd_steering_end(bsd_steering_stru *pst_steering)
{
    OAL_MEMZERO(pst_steering->auc_mac_addr, WLAN_MAC_ADDR_LEN);
    pst_steering->uc_state = 0;

    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_steering->st_block_timer));

    pst_steering->pst_bsd_ap = OAL_PTR_NULL;
    pst_steering->pst_source_vap = OAL_PTR_NULL;

    return;
}



OAL_STATIC oal_uint32 dmac_bsd_steering_timeout_fn(oal_void *prg)
{
    bsd_steering_stru   *pst_steering;

    if (OAL_PTR_NULL == prg)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_steering_timeout_fn: input pointer is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_steering = (bsd_steering_stru  *)prg;

    OAM_WARNING_LOG3(0, OAM_SF_BSD, "{dmac_bsd_steering_timeout_fn::user[xx:xx:xx:%02x:%02x:%02x]* steering timeout!",
                                 pst_steering->auc_mac_addr[WLAN_MAC_ADDR_LEN-3],
                                 pst_steering->auc_mac_addr[WLAN_MAC_ADDR_LEN-2],
                                 pst_steering->auc_mac_addr[WLAN_MAC_ADDR_LEN-1]);


    dmac_bsd_steering_end(pst_steering);

    return OAL_SUCC;
}



OAL_STATIC bsd_sta_info_item_stru *dmac_bsd_user_info_search(oal_uint8 *puc_addr)
{
    bsd_sta_info_item_stru  *pst_user_info;
    oal_uint32              ul_user_hash_value;
    oal_dlist_head_stru     *pst_entry;

    ul_user_hash_value = DMAC_BSD_CALCULATE_HASH_VALUE(puc_addr);

    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(g_st_bsd_mgmt.st_sta_info_tbl.ast_hash_tbl[ul_user_hash_value]))
    {
        pst_user_info = (bsd_sta_info_item_stru *)OAL_DLIST_GET_ENTRY(pst_entry, bsd_sta_info_item_stru, st_by_hash_list);
        if(!oal_compare_mac_addr(pst_user_info->auc_user_mac_addr,puc_addr))
        {
#ifdef _PRE_DEBUG_MODE
            if(g_st_bsd_mgmt.bit_debug_mode)
            {
                OAM_WARNING_LOG3(0, OAM_SF_BSD, "{dmac_bsd_user_info_search::user[xx:xx:xx:%02x:%02x:%02x] info:",
                                pst_user_info->auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                                pst_user_info->auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                                pst_user_info->auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);

                OAM_WARNING_LOG4(0, OAM_SF_BSD, "{dmac_bsd_user_info_search::steer_enable=%d,steer_lock=%d,steer_unfriendly=%d,last_assoc_flag=%d",
                                pst_user_info->bit_steering_cfg_enable,
                                pst_user_info->bit_lock_state,
                                pst_user_info->bit_steering_unfriendly_flag,
                                pst_user_info->en_last_assoc_by_bsd);
            }
#endif
            return pst_user_info;
        }
    }

    return (bsd_sta_info_item_stru *)OAL_PTR_NULL;
}

#ifdef _PRE_DEBUG_MODE

OAL_STATIC oal_uint32  dmac_bsd_lock_user(oal_uint8 *puc_addr,oal_uint32   ul_state)
{
    bsd_sta_info_item_stru      *pst_user;
    pst_user = dmac_bsd_user_info_search(puc_addr);
    if(OAL_PTR_NULL == pst_user)
    {
        return OAL_FAIL;
    }

    OAM_WARNING_LOG4(0, OAM_SF_BSD,"{dmac_bsd_lock_user::set user[xx:xx:xx:%02x:%02x:%02x]* lock=%d}",
        puc_addr[3],puc_addr[4],puc_addr[5],ul_state);
    pst_user->bit_lock_state = (oal_uint8)ul_state;
    return OAL_SUCC;
}
#endif



OAL_STATIC  bsd_sta_info_item_stru * dmac_bsd_match_steering_condition(mac_vap_stru *pst_mac_vap,
                                                                       bsd_ap_stru   *pst_bsd_ap,
                                                                       dmac_user_stru *pst_dmac_user,
                                                                       dmac_user_bsd_info_stru *pst_user_stat,
                                                                       oal_uint32 ul_rssi_check_flag)
{
    bsd_sta_info_item_stru      *pst_user_info;
    oal_uint32                  ul_timestamp;
    oal_uint32                  ul_delta_time;

    //搜索该用户的能力信息和状态信息
    pst_user_info = dmac_bsd_user_info_search(pst_dmac_user->st_user_base_info.auc_user_mac_addr);
    if(OAL_PTR_NULL == pst_user_info)
    {
        //如果用户已经关联上，再打开bsd动态开关的场景下，那么有可能出现bsd任务调度时没有该用户的信息的情况
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_BSD, "{dmac_bsd_match_steering_condition::cant't search user[xx:xx:xx:%02x:%02x:%02x] info!",
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);
        return OAL_PTR_NULL;
    }

    if(pst_mac_vap->uc_vap_id != pst_user_info->uc_last_assoc_vap_id)
    {
        //当前vap与最后实际关联的vap不同，那么此时该STA实际上已经不再关联在该vap
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_bsd_task_timeout_fn::the user actual assoc vap[%d],skip!}",
        pst_user_info->uc_last_assoc_vap_id);
        return OAL_PTR_NULL;
    }

    if((!pst_user_info->bit_steering_cfg_enable)||(pst_user_info->bit_steering_unfriendly_flag)
      ||(pst_user_info->en_band_cap != DMAC_BSD_STA_CAP_DUAL_BAND))
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_bsd_task_timeout_fn::the user not enbale bsd or unfrendly,skip!}");
        return OAL_PTR_NULL;
    }

    ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    pst_user_info->bit_pre_steer_fail_flag = 0;

    if(pst_user_info->bit_lock_state)
    {
        ul_delta_time = (oal_uint32)OAL_TIME_GET_RUNTIME(pst_user_info->ul_last_lock_timestamp, ul_timestamp);
        if(ul_delta_time <= pst_bsd_ap->st_bsd_config.ul_steering_lock_time*60*1000)
        {
            return OAL_PTR_NULL;
        }
        else
        {
            OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_BSD, "{dmac_bsd_task_timeout_fn::user[xx:xx:xx:%02x:%02x:%02x]* steer unlock!",
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);

            pst_user_info->bit_lock_state = 0;
        }
    }

    if(dmac_bsd_user_state_get(pst_dmac_user, pst_user_stat) != OAL_SUCC)
    {
        return OAL_PTR_NULL;
    }

    if((pst_user_stat->ul_average_rx_thru > BSD_THRU_TH(pst_bsd_ap))
        ||(pst_user_stat->ul_average_tx_thru > BSD_THRU_TH(pst_bsd_ap))
        ||((ul_rssi_check_flag)&&(pst_user_stat->c_average_rssi > BSD_RSSI_TH(pst_bsd_ap))))
    {
        return OAL_PTR_NULL;
    }

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_BSD, "{dmac_bsd_task_timeout_fn::user[xx:xx:xx:%02x:%02x:%02x]* match steering condition!",
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                         pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);

    return pst_user_info;
}


OAL_STATIC OAL_INLINE oal_void dmac_bsd_force_user_switch_by_blacklist(bsd_steering_stru *pst_steering_ctl,dmac_user_stru *pst_dmac_user)
{
    dmac_vap_stru *pst_dmac_vap = pst_steering_ctl->pst_source_vap;
    //创建拉黑定时器
    FRW_TIMER_CREATE_TIMER(&(pst_steering_ctl->st_block_timer),
                        dmac_bsd_steering_timeout_fn,
                        pst_steering_ctl->pst_bsd_ap->st_bsd_config.ul_steering_blocktime,
                        (void *)pst_steering_ctl,
                        OAL_FALSE,
                        OAM_MODULE_ID_DMAC,
                        pst_dmac_vap->st_vap_base_info.ul_core_id);

    //发送一个kick用户的事件到hmac层，由hmac层开始用户删除
    dmac_send_disasoc_misc_event(&(pst_dmac_vap->st_vap_base_info),pst_dmac_user->st_user_base_info.us_assoc_id, DMAC_DISASOC_MISC_BSD);
}


#ifdef _PRE_WLAN_FEATURE_11V

OAL_STATIC oal_int32 dmac_bsd_tx_11v_request_cb(oal_void *p,
                                     oal_uint8 uc_event_type,
                                     oal_void* param)
{
    dmac_user_stru  *pst_dmac_user = (dmac_user_stru*)p;

    switch(uc_event_type)
    {
        case DMAC_11V_CALLBACK_RETURN_REICEVE_RSP:
            if(((dmac_bsst_rsp_info_stru*)param)->uc_status_code)
            {
#ifdef _PRE_DEBUG_MODE
                if(g_st_bsd_mgmt.bit_debug_mode)
                {
                    OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD,
                        "{dmac_bsd_tx_11v_request_cb::user[xx:xx:xx:%02x:%02x:%02x]* reject change to dest vap,force kick it!",
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);

                }
#endif
                //STA回复拒绝切换，直接通过强制提出拉黑的方式迫使STA切换到新的AP
                //dmac_bsd_force_user_switch_by_blacklist(&g_st_bsd_mgmt.st_steering_ctl,pst_dmac_user);
            }
#ifdef _PRE_DEBUG_MODE
            else
            {
                //STA回复接受切换
                if(g_st_bsd_mgmt.bit_debug_mode)
                {
                    OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD,
                        "{dmac_bsd_tx_11v_request_cb::user[xx:xx:xx:%02x:%02x:%02x]* will change to dest vap triged by 11v request!",
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);

                }
            }
#endif
            break;
        case DMAC_11V_CALLBACK_RETURN_WAIT_RSP_TIMEOUT:
#ifdef _PRE_DEBUG_MODE
                if(g_st_bsd_mgmt.bit_debug_mode)
                {
                    OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD,
                        "{dmac_bsd_tx_11v_request_cb::user[xx:xx:xx:%02x:%02x:%02x]* not response to 11v request,force kick it!",
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);

                }
#endif
            //STA在超时时间内不响应11v req帧,直接通过强制提出拉黑的方式迫使STA切换到新的AP
            //dmac_bsd_force_user_switch_by_blacklist(&g_st_bsd_mgmt.st_steering_ctl,pst_dmac_user);
            break;
        default:
            OAM_WARNING_LOG0(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD,
            "{dmac_bsd_tx_11v_request_cb::en_event_type error!");
            break;
    }

    //不管终端是否能正确响应11v BSS Transition request,都强制将该终端从源vap去关联.
    dmac_bsd_force_user_switch_by_blacklist(&g_st_bsd_mgmt.st_steering_ctl,pst_dmac_user);
    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE oal_void dmac_bsd_set_dest_vap_info(mac_vap_stru* pst_mac_vap,
                                               dmac_neighbor_bss_info_stru *pst_neighbor_bss_list)
{
    oal_memcopy(pst_neighbor_bss_list->auc_mac_addr, mac_vap_get_mac_addr(pst_mac_vap), WLAN_MAC_ADDR_LEN);
    pst_neighbor_bss_list->uc_candidate_perf = 0xff;
    pst_neighbor_bss_list->uc_opt_class = 0;        //与标杆保持一致
    pst_neighbor_bss_list->uc_phy_type = (WLAN_LEGACY_11B_MODE == pst_mac_vap->en_protocol) ? 1 : 4;       //OFDM

    pst_neighbor_bss_list->uc_chl_num = pst_mac_vap->st_channel.uc_chan_number;

    pst_neighbor_bss_list->st_bssid_info.bit_ap_reachability = 0;     //unknow
    pst_neighbor_bss_list->st_bssid_info.bit_security = 0;            //加密规则一致
    pst_neighbor_bss_list->st_bssid_info.bit_key_scope = 0;

    /* Spectrum Management */
    pst_neighbor_bss_list->st_bssid_info.bit_spectrum_mgmt = mac_mib_get_dot11SpectrumManagementRequired(pst_mac_vap);
    /* QoS subfield */
    //pst_neighbor_bss_list->st_bssid_info.bit_qos = mac_mib_get_dot11QosOptionImplemented(pst_mac_vap);
    pst_neighbor_bss_list->st_bssid_info.bit_qos = 0;
    /* APSD */
    pst_neighbor_bss_list->st_bssid_info.bit_apsd = mac_mib_get_dot11APSDOptionImplemented(pst_mac_vap);
    /* Radio Measurement */
    pst_neighbor_bss_list->st_bssid_info.bit_radio_meas = mac_mib_get_dot11RadioMeasurementActivated(pst_mac_vap);
    /* Delayed BA */
    pst_neighbor_bss_list->st_bssid_info.bit_delay_block_ack = mac_mib_get_dot11DelayedBlockAckOptionImplemented(pst_mac_vap);
    /* Immediate Block Ack 参考STA及AP标杆，此能力一直为0,实际通过addba协商。此处修改为标杆一致。mib值不修改 */
    /*pst_neighbor_bss_list->st_bssid_info.bit_immediate_block_ack = pst_mib->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented;*/
    pst_neighbor_bss_list->st_bssid_info.bit_immediate_block_ack = 0;

    pst_neighbor_bss_list->st_bssid_info.bit_high_throughput = 0;
    pst_neighbor_bss_list->st_bssid_info.bit_mobility_domain = 0;
    pst_neighbor_bss_list->st_bssid_info.bit_resv1 = 0;
    pst_neighbor_bss_list->st_bssid_info.bit_resv2 = 0;
    pst_neighbor_bss_list->st_bssid_info.bit_resv3 = 0;

    OAL_MEMZERO(&pst_neighbor_bss_list->st_term_duration.auc_termination_tsf,DMAC_11V_TERMINATION_TSF_LENGTH);
    //pst_neighbor_bss_list->st_term_duration.us_duration_min = 0xffff;
    //pst_neighbor_bss_list->st_term_duration.uc_sub_ie_id = 4;
}


OAL_STATIC OAL_INLINE oal_void dmac_bsd_notify_user_switch_by_11v_req(bsd_steering_stru *pst_steering_ctl,dmac_user_stru *pst_dmac_user)
{
    dmac_vap_stru *pst_source_vap;
    dmac_vap_stru *pst_dest_vap;
    dmac_neighbor_bss_info_stru st_neighbor_bss_list;

    pst_source_vap = pst_steering_ctl->pst_source_vap;
    if(pst_steering_ctl->pst_bsd_ap->past_dmac_vap[0] == pst_source_vap)
    {
        pst_dest_vap = pst_steering_ctl->pst_bsd_ap->past_dmac_vap[1];
    }
    else
    {
        pst_dest_vap = pst_steering_ctl->pst_bsd_ap->past_dmac_vap[0];
    }

    dmac_bsd_set_dest_vap_info(&(pst_dest_vap->st_vap_base_info),&st_neighbor_bss_list);

    if(dmac_tx_bsst_req_action_one_bss(pst_source_vap,pst_dmac_user,&st_neighbor_bss_list,(dmac_user_callback_func_11v)dmac_bsd_tx_11v_request_cb)!= OAL_SUCC)
    {
#ifdef _PRE_DEBUG_MODE
        if(g_st_bsd_mgmt.bit_debug_mode)
        {
            OAM_WARNING_LOG3(pst_dmac_user->st_user_base_info.uc_vap_id, OAM_SF_BSD,
                        "{dmac_bsd_notify_user_switch_by_11v_req::user[xx:xx:xx:%02x:%02x:%02x]* send 11v request fail,force kick it!",
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                        pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1]);

        }
#endif
        //通过11v req的方式通知用户切换AP失败，就采用踢掉拉黑的方式迫使用户切换AP
        dmac_bsd_force_user_switch_by_blacklist(pst_steering_ctl,pst_dmac_user);
    }
}


#endif


OAL_STATIC oal_uint32 dmac_bsd_user_steering_process(bsd_steering_stru *pst_steering_ctl,dmac_user_stru *pst_dmac_user)
{
    oal_uint32                  ul_timestamp;
    oal_uint32                  ul_delta_time;
    bsd_sta_info_item_stru      *pst_user_info = pst_steering_ctl->pst_user_info;
    dmac_vap_stru               *pst_dest_vap;
    dmac_vap_stru               *pst_source_vap;
    oal_uint16                  us_user_idx;
    dmac_user_stru              *pst_dmac_user_another;

    OAM_WARNING_LOG4(0, OAM_SF_BSD, "{dmac_bsd_user_steering_process::user[xx:xx:xx:%02x:%02x:%02x]* be selected to steering to %dG Band!",
                     pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                     pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                     pst_dmac_user->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1],
                     (WLAN_BAND_2G==pst_steering_ctl->pst_source_vap->st_vap_base_info.st_channel.en_band)?5:2);

    //note:如果切换频度的阀值大于2次/30分钟,此段代码需要整改
    ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    pst_user_info->ul_steering_cnt++;    //该用户被调度出来进行切换的次数统计+1
    if(pst_user_info->ul_steering_last_timestamp)
    {
        ul_delta_time = (oal_uint32)OAL_TIME_GET_RUNTIME(pst_user_info->ul_steering_last_timestamp, ul_timestamp);
        if(ul_delta_time < DMAC_BSD_STEER_FRE_PERIOD)
        {
            //切换频度等于阀值了，需要锁定一段时间
            pst_user_info->bit_lock_state = 1;
            pst_user_info->ul_last_lock_timestamp = ul_timestamp;
        }
    }

    pst_user_info->ul_steering_last_timestamp = ul_timestamp;
    pst_steering_ctl->uc_state = 1;
    oal_memcopy(pst_steering_ctl->auc_mac_addr,pst_dmac_user->st_user_base_info.auc_user_mac_addr,WLAN_MAC_ADDR_LEN);
    pst_user_info->en_last_assoc_by_bsd = OAL_FALSE;
    pst_user_info->pst_bsd_source_vap = OAL_PTR_NULL;

    //若该用户还关联在目标vap上，需要先通知hmac层将该用户删除
    pst_source_vap = pst_steering_ctl->pst_source_vap;
    if(pst_steering_ctl->pst_bsd_ap->past_dmac_vap[0] == pst_source_vap)
    {
        pst_dest_vap = pst_steering_ctl->pst_bsd_ap->past_dmac_vap[1];
    }
    else
    {
        pst_dest_vap = pst_steering_ctl->pst_bsd_ap->past_dmac_vap[0];
    }

    if(OAL_SUCC == mac_vap_find_user_by_macaddr(&(pst_dest_vap->st_vap_base_info), pst_dmac_user->st_user_base_info.auc_user_mac_addr, &us_user_idx))
    {
        pst_dmac_user_another = (dmac_user_stru *)mac_res_get_dmac_user(us_user_idx);

        OAM_WARNING_LOG4(pst_dest_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_user_steering_process::user[xx:xx:xx:%02x:%02x:%02x]* will del from dest vap(%dG Band) first!",
                        pst_dmac_user_another->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-3],
                        pst_dmac_user_another->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-2],
                        pst_dmac_user_another->st_user_base_info.auc_user_mac_addr[WLAN_MAC_ADDR_LEN-1],
                        (WLAN_BAND_2G==pst_dest_vap->st_vap_base_info.st_channel.en_band)?2:5);
        //发送一个kick用户的事件到hmac层，由hmac层开始用户删除
        dmac_send_disasoc_misc_event(&(pst_dest_vap->st_vap_base_info),pst_dmac_user_another->st_user_base_info.us_assoc_id, DMAC_DISASOC_MISC_BSD);
    }

#ifdef _PRE_WLAN_FEATURE_11V
    dmac_bsd_notify_user_switch_by_11v_req(pst_steering_ctl,pst_dmac_user);
#else
    dmac_bsd_force_user_switch_by_blacklist(pst_steering_ctl,pst_dmac_user);
#endif
    return OAL_SUCC;
}



OAL_STATIC oal_uint32 dmac_bsd_task_timeout_fn(oal_void *prg)
{
    oal_uint16                  us_2g_load = 0;
    oal_uint16                  us_5g_load = 0;
    bsd_ap_stru                 *pst_bsd_ap;
    dmac_vap_stru               *pst_busy_vap;
    oal_dlist_head_stru         *pst_bsd_ap_list;
    oal_dlist_head_stru         *pst_entry;
    dmac_user_stru              *pst_dmac_user;
    bsd_sta_info_item_stru      *pst_user_info;
    dmac_user_bsd_info_stru     st_user_stat;
    dmac_user_stru              *pst_lowest_phyrate_user = OAL_PTR_NULL;
    oal_uint32                  ul_lowest_phyrate = ~0;
    oal_uint32                  ul_rssi_check_flag = 0;
    oal_uint32                  ul_idx = 0;
#ifdef _PRE_DEBUG_MODE
    if(g_st_bsd_mgmt.bit_debug_mode)
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_task_timeout_fn: bsd task scheduled!}");
    }
#endif
    if(g_st_bsd_mgmt.st_steering_ctl.uc_state)
    {
#ifdef _PRE_DEBUG_MODE
        if(g_st_bsd_mgmt.bit_debug_mode)
        {
            OAM_WARNING_LOG3(0, OAM_SF_BSD, "{dmac_bsd_task_timeout_fn::user[xx:xx:xx:%02x:%02x:%02x] is steering,no need new judgement!",
                                 g_st_bsd_mgmt.st_steering_ctl.auc_mac_addr[WLAN_MAC_ADDR_LEN-3],
                                 g_st_bsd_mgmt.st_steering_ctl.auc_mac_addr[WLAN_MAC_ADDR_LEN-2],
                                 g_st_bsd_mgmt.st_steering_ctl.auc_mac_addr[WLAN_MAC_ADDR_LEN-1]);

        }
#endif
        return OAL_SUCC;
    }
    OAL_DLIST_SEARCH_FOR_EACH(pst_bsd_ap_list, &(g_st_bsd_mgmt.st_bsd_ap_list_head))
    {
        pst_bsd_ap = (bsd_ap_stru*)OAL_DLIST_GET_ENTRY(pst_bsd_ap_list, bsd_ap_stru, st_entry);
        if(0 == ul_idx)
        {
            //只需要获取一次的频段负载
            if(dmac_bsd_vap_load_get(pst_bsd_ap->past_dmac_vap[0],&us_2g_load) != OAL_SUCC)
            {
                OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_task_timeout_fn: got 2G bsd_ap load failed!}");
                continue;
            }
            if(dmac_bsd_vap_load_get(pst_bsd_ap->past_dmac_vap[1],&us_5g_load) != OAL_SUCC)
            {
                OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_task_timeout_fn: got 5G bsd_ap load failed!}");
                continue;
            }
#ifdef _PRE_DEBUG_MODE
            if(g_st_bsd_mgmt.bit_debug_mode)
            {
                OAM_WARNING_LOG2(0, OAM_SF_BSD, "{dmac_bsd_task_timeout_fn: bsd_ap load(2G)=%d,load(5G)=%d!}",
                                 us_2g_load,us_5g_load);
            }

            if(g_st_bsd_mgmt.bit_load_status_switch)
            {
                OAL_IO_PRINT("2G BSD_AP Load=%4d, 5G BSD_AP Load=%4d\r\n",us_2g_load,us_5g_load);
            }
#endif
            ul_idx ++;
        }

        //需要刷新每个bsd ap下关联用户的统计状态
        OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bsd_ap->past_dmac_vap[0]->st_vap_base_info.st_mac_user_list_head))
        {
            pst_dmac_user = (dmac_user_stru *)pst_entry;
            dmac_bsd_user_state_refresh(pst_dmac_user, pst_bsd_ap,BSD_TASK_TIMEOUT/2);
        }

        OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_bsd_ap->past_dmac_vap[1]->st_vap_base_info.st_mac_user_list_head))
        {
            pst_dmac_user = (dmac_user_stru *)pst_entry;
            dmac_bsd_user_state_refresh(pst_dmac_user, pst_bsd_ap,BSD_TASK_TIMEOUT/2);
        }


        if(((us_2g_load < BSD_FREE_TH(pst_bsd_ap))&&(us_5g_load > BSD_BUSY_TH(pst_bsd_ap)))
           ||((us_5g_load < BSD_FREE_TH(pst_bsd_ap))&&(us_2g_load > BSD_BUSY_TH(pst_bsd_ap))))
        {
            //负载失衡
            OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_task_timeout_fn: bsd_ap load not enbalance!}");
            if(us_5g_load > BSD_BUSY_TH(pst_bsd_ap))
            {
                pst_busy_vap = pst_bsd_ap->past_dmac_vap[1];  //5G vap繁忙
            }
            else
            {
                pst_busy_vap = pst_bsd_ap->past_dmac_vap[0];  //2.4G vap繁忙
            }
        }
        else
        {
            ul_rssi_check_flag = 1;
            if(us_2g_load < BSD_FREE_TH(pst_bsd_ap))
            {
                pst_busy_vap = pst_bsd_ap->past_dmac_vap[1];
            }
            else
            {
                continue;
            }
        }


        /* 遍历vap下所有用户 */
        OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_busy_vap->st_vap_base_info.st_mac_user_list_head))
        {
            pst_dmac_user = (dmac_user_stru *)pst_entry;

            //判断该用户是否满足切换条件
            pst_user_info = dmac_bsd_match_steering_condition(&(pst_busy_vap->st_vap_base_info),
                                                              pst_bsd_ap,pst_dmac_user,
                                                              &st_user_stat,
                                                              ul_rssi_check_flag);
            if(pst_user_info)
            {
                if(st_user_stat.ul_best_rate < ul_lowest_phyrate)
                {
                    ul_lowest_phyrate = st_user_stat.ul_best_rate;
                    pst_lowest_phyrate_user = pst_dmac_user;
                    g_st_bsd_mgmt.st_steering_ctl.pst_bsd_ap = pst_bsd_ap;
                    g_st_bsd_mgmt.st_steering_ctl.pst_source_vap = pst_busy_vap;
                    g_st_bsd_mgmt.st_steering_ctl.pst_user_info = pst_user_info;
                }
            }
        }
    }

    if(pst_lowest_phyrate_user)
    {
        return dmac_bsd_user_steering_process(&g_st_bsd_mgmt.st_steering_ctl,pst_lowest_phyrate_user);
    }

    return OAL_SUCC;
}



OAL_STATIC oal_void  dmac_bsd_task_start(oal_void)
{
    oal_uint32      ul_core_id;
    bsd_ap_stru                 *pst_bsd_ap;
    oal_dlist_head_stru         *pst_bsd_ap_list;

    if(g_st_bsd_mgmt.st_bsd_sched_timer.en_is_registerd)
    {
        return;
    }

    pst_bsd_ap_list = g_st_bsd_mgmt.st_bsd_ap_list_head.pst_next;
    if(OAL_PTR_NULL == pst_bsd_ap_list)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_task_start::pst_bsd_ap_list NULL PTR!");
        return;
    }
    pst_bsd_ap = (bsd_ap_stru*)OAL_DLIST_GET_ENTRY(pst_bsd_ap_list, bsd_ap_stru, st_entry);
    ul_core_id = pst_bsd_ap->past_dmac_vap[0]->st_vap_base_info.ul_core_id;

    OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_task_start::bsd task start up!");

    //创建bsd task定时器
    FRW_TIMER_CREATE_TIMER(&g_st_bsd_mgmt.st_bsd_sched_timer,
                           dmac_bsd_task_timeout_fn,
                           BSD_TASK_TIMEOUT,
                           (void *)&g_st_bsd_mgmt,
                           OAL_TRUE,
                           OAM_MODULE_ID_DMAC,
                           ul_core_id);
    return;
}



OAL_STATIC oal_void  dmac_bsd_task_stop(void)
{
    OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_task_stop::bsd task stop!}");

    //删除bsd task定时器
    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&g_st_bsd_mgmt.st_bsd_sched_timer);

	dmac_bsd_steering_end(&g_st_bsd_mgmt.st_steering_ctl);
    return;
}



oal_void  dmac_bsd_vap_up_handle(dmac_vap_stru * pst_dmac_vap)
{
    bsd_ap_stru                 *pst_bsd_ap;
    mac_vap_stru                *pst_mac_vap;
    dmac_vap_stru               *pst_another_vap;

    if(!g_st_bsd_mgmt.en_switch)
    {
        return;
    }

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_up_handle:param null!}");
        return;
    }

    //防止dmac重复收到同一个vap的启动事件，acs的运行时扫描适配的接口导致dmac层重复收到vap的启动事件
    if(dmac_bsd_vap_find(pst_dmac_vap))
    {
        return;
    }

    pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);
    if((pst_mac_vap->en_vap_state != MAC_VAP_STATE_UP)
        ||(pst_mac_vap->en_vap_mode != WLAN_VAP_MODE_BSS_AP))
    {
        return;
    }

    pst_another_vap = dmac_bsd_vap_search(pst_dmac_vap);
    if(OAL_PTR_NULL == pst_another_vap)
    {
        return;
    }

    pst_bsd_ap = (bsd_ap_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(bsd_ap_stru), OAL_TRUE);
    if(pst_bsd_ap == OAL_PTR_NULL)
    {
        return;
    }

    OAL_MEMZERO((oal_void*)pst_bsd_ap,OAL_SIZEOF(bsd_ap_stru));

    if(WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
    {
        pst_bsd_ap->past_dmac_vap[0] = pst_dmac_vap;
        pst_bsd_ap->past_dmac_vap[1] = pst_another_vap;
    }
    else
    {
        pst_bsd_ap->past_dmac_vap[0] = pst_another_vap;
        pst_bsd_ap->past_dmac_vap[1] = pst_dmac_vap;
    }

    if(dmac_bsd_device_load_monitor_init(pst_dmac_vap) != OAL_SUCC)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_vap_up_handle::dmac_bsd_vap_load_monitor_init fail!");
        return;
    }

    if(dmac_bsd_device_load_monitor_init(pst_another_vap) != OAL_SUCC)
    {
        OAM_ERROR_LOG0(pst_another_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_vap_up_handle::dmac_bsd_vap_load_monitor_init fail!");
        return;
    }

    if(dmac_bsd_config_set(pst_bsd_ap, &dmac_bsd_default_cfg) != OAL_SUCC)
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_up_handle::bsd ap set defualt cfg fail!");
        return;
    }

    oal_dlist_add_tail(&(pst_bsd_ap->st_entry), &g_st_bsd_mgmt.st_bsd_ap_list_head);

    if(g_st_bsd_mgmt.bit_cfg_switch)
    {
        //启动BSD_Task
        dmac_bsd_task_start();
    }

    return;
}



oal_void  dmac_bsd_vap_down_handle(dmac_vap_stru * pst_dmac_vap)
{
    bsd_ap_stru                 *pst_bsd_ap;

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_vap_down_handle:param null!}");
        return;
    }

    if(!g_st_bsd_mgmt.en_switch)
    {
        return;
    }

    pst_bsd_ap = dmac_bsd_vap_find(pst_dmac_vap);
    if(OAL_PTR_NULL == pst_bsd_ap)
    {
        return;
    }

    oal_dlist_delete_entry(&(pst_bsd_ap->st_entry));
    OAL_MEM_FREE(pst_bsd_ap,OAL_TRUE);

    if(OAL_TRUE == oal_dlist_is_empty(&(g_st_bsd_mgmt.st_bsd_ap_list_head)))
    {
        //没有bsd ap了
        dmac_bsd_task_stop();

        //释放每个频段的负载采样定时资源
        dmac_bsd_device_load_monitor_deinit(pst_bsd_ap->past_dmac_vap[0]);

        dmac_bsd_device_load_monitor_deinit(pst_bsd_ap->past_dmac_vap[1]);
    }

    return;
}



OAL_STATIC bsd_pre_steering_node_stru  *dmac_bsd_find_pre_steering_user(oal_uint8 *puc_mac_addr)
{
    oal_dlist_head_stru        *pst_entry;
    bsd_pre_steering_node_stru  *pst_bsd_pre_steering_user = (bsd_pre_steering_node_stru*)OAL_PTR_NULL;

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_mac_addr))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_find_pre_steering_user::param null.}");

        return pst_bsd_pre_steering_user;
    }

    /* 与cache user对比 , 相等则直接返回cache user id*/
    if(g_st_bsd_mgmt.st_pre_steering_ctl.pst_cache_user_node)
    {
        if (!oal_compare_mac_addr(g_st_bsd_mgmt.st_pre_steering_ctl.pst_cache_user_node->auc_mac_addr, puc_mac_addr))
        {
            return g_st_bsd_mgmt.st_pre_steering_ctl.pst_cache_user_node;
        }
    }

    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(g_st_bsd_mgmt.st_pre_steering_ctl.st_pre_steering_list))
    {
        pst_bsd_pre_steering_user = (bsd_pre_steering_node_stru *)OAL_DLIST_GET_ENTRY(pst_entry, bsd_pre_steering_node_stru, st_list_entry);

        /* 相同的MAC地址 */
        if (!oal_compare_mac_addr(pst_bsd_pre_steering_user->auc_mac_addr, puc_mac_addr))
        {
            /*更新cache user*/
            g_st_bsd_mgmt.st_pre_steering_ctl.pst_cache_user_node = pst_bsd_pre_steering_user;
            return pst_bsd_pre_steering_user;
        }
    }
    return (bsd_pre_steering_node_stru*)OAL_PTR_NULL;
}


OAL_STATIC oal_void dmac_bsd_pre_steer_user_del(bsd_pre_steering_node_stru *pst_pre_steering_user)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_pre_steering_user))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD,"{dmac_bsd_pre_steer_user_del::arg null.}");
        return;
    }

#ifdef _PRE_DEBUG_MODE
    if(g_st_bsd_mgmt.bit_debug_mode)
    {
        OAM_INFO_LOG3(0, OAM_SF_BSD, "{dmac_bsd_pre_steer_user_del::user[xx:xx:xx:%02x:%02x:%02x] delet from pre-steer dlist!",
                      pst_pre_steering_user->auc_mac_addr[WLAN_MAC_ADDR_LEN-3],
                      pst_pre_steering_user->auc_mac_addr[WLAN_MAC_ADDR_LEN-2],
                      pst_pre_steering_user->auc_mac_addr[WLAN_MAC_ADDR_LEN-1]);
    }
#endif

    if(g_st_bsd_mgmt.st_pre_steering_ctl.pst_cache_user_node == pst_pre_steering_user)
    {
        g_st_bsd_mgmt.st_pre_steering_ctl.pst_cache_user_node = OAL_PTR_NULL;
    }

    oal_dlist_delete_entry(&(pst_pre_steering_user->st_list_entry));
    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_pre_steering_user->st_block_timer));
    OAL_MEM_FREE(pst_pre_steering_user, OAL_TRUE);

    return;

}


OAL_STATIC oal_void dmac_bsd_pre_steer_list_clear(bsd_pre_steering_stru *pst_pre_steer)
{
    oal_dlist_head_stru             *pst_entry;
    oal_dlist_head_stru             *pst_entry_tmp;
    bsd_pre_steering_node_stru      *pst_bsd_pre_steering_user;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_pre_steer))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_pre_steer_list_clear::param null.}");

        return;
    }

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry,pst_entry_tmp,&(pst_pre_steer->st_pre_steering_list))
    {
        pst_bsd_pre_steering_user = (bsd_pre_steering_node_stru *)OAL_DLIST_GET_ENTRY(pst_entry, bsd_pre_steering_node_stru, st_list_entry);
        dmac_bsd_pre_steer_user_del(pst_bsd_pre_steering_user);
    }

    return;
}



OAL_STATIC OAL_INLINE oal_uint32  dmac_bsd_steering_result_handle(dmac_vap_stru * pst_dmac_vap,
                                                    oal_uint8   *puc_addr,
                                                    oal_uint8 uc_result)
{
    oal_uint32          ul_flag = 1;

    if(uc_result)
    {
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                         "{dmac_bsd_user_add_handle::user[xx:xx:xx:%02x:%02x:%02x]* steering to %dG Band succ.}",
                         puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                         (WLAN_BAND_2G==pst_dmac_vap->st_vap_base_info.st_channel.en_band)?2:5);


        g_st_bsd_mgmt.st_steering_ctl.pst_user_info->ul_steering_succ_cnt++;
        g_st_bsd_mgmt.st_steering_ctl.pst_user_info->en_last_assoc_by_bsd = OAL_TRUE;
        g_st_bsd_mgmt.st_steering_ctl.pst_user_info->pst_bsd_source_vap = g_st_bsd_mgmt.st_steering_ctl.pst_source_vap;
    }
    else
    {
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                         "{dmac_bsd_user_add_handle::user[xx:xx:xx:%02x:%02x:%02x]* steering to %dG Band failed.}",
                         puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                         (WLAN_BAND_2G==pst_dmac_vap->st_vap_base_info.st_channel.en_band)?2:5);
        if(!g_st_bsd_mgmt.st_steering_ctl.pst_user_info->ul_steering_succ_cnt)
        {
            if(g_st_bsd_mgmt.st_steering_ctl.pst_user_info->ul_steering_cnt >= BSD_STEER_UNFRIENDLY_TH)
            {
                //试图steer该STA N次，但是1次都没有成功，将该STA标记为steer-unfriendly
                g_st_bsd_mgmt.st_steering_ctl.pst_user_info->bit_steering_unfriendly_flag = 1;
                ul_flag = 0;
                OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                         "{dmac_bsd_user_add_handle::user[xx:xx:xx:%02x:%02x:%02x]* is set steer-unfriendly.}",
                         puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);

            }
        }
    }

    //无论steering结果如何，该sta的steering过程结束，如下结构体清空后，又可以调度新的sta进行steering
    OAL_MEMZERO(g_st_bsd_mgmt.st_steering_ctl.auc_mac_addr, WLAN_MAC_ADDR_LEN);
    g_st_bsd_mgmt.st_steering_ctl.uc_state = 0;

    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&g_st_bsd_mgmt.st_steering_ctl.st_block_timer);

    g_st_bsd_mgmt.st_steering_ctl.pst_bsd_ap = OAL_PTR_NULL;
    g_st_bsd_mgmt.st_steering_ctl.pst_source_vap = OAL_PTR_NULL;
    return ul_flag;
}



OAL_STATIC OAL_INLINE oal_uint32  dmac_bsd_pre_steer_result_handle(dmac_vap_stru * pst_dmac_vap,
                                                        bsd_pre_steering_node_stru  *pst_pre_steering_user,
                                                        oal_uint8 uc_result)
{
    oal_uint32      ul_flag = 1;
    oal_uint8       *puc_addr = pst_pre_steering_user->auc_mac_addr;

    if(uc_result)
    {
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                                 "{dmac_bsd_user_add_handle::user[xx:xx:xx:%02x:%02x:%02x]* pre-steer to %dG Band succ.}",
                                 puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                                 (WLAN_BAND_2G==pst_dmac_vap->st_vap_base_info.st_channel.en_band)?2:5);
        pst_pre_steering_user->pst_sta_info->ul_pre_steer_succ_cnt++;
        pst_pre_steering_user->pst_sta_info->bit_steering_unfriendly_flag = 0;      //清除引导不友好的用户的标记
    }
    else
    {
        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                                 "{dmac_bsd_user_add_handle::user[xx:xx:xx:%02x:%02x:%02x]* pre-steer to %dG failed.}",
                                 puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                                 (WLAN_BAND_2G==pst_dmac_vap->st_vap_base_info.st_channel.en_band)?5:2);
        if(pst_dmac_vap == pst_pre_steering_user->pst_source_vap)
        {
            pst_pre_steering_user->pst_sta_info->bit_pre_steer_fail_flag = 1;      //标记为pre-steer引导不友好的用户
        }
    }

    //无论pre-steer结果如何，该sta的pre-steer过程结束
    dmac_bsd_pre_steer_user_del(pst_pre_steering_user);
    return ul_flag;
}

#if 0
int __attribute__((__no_instrument_function__))dmac_bsd_check(long this_func, long call_func, long dir)
{
    if (p_check_addr && (OAL_PTR_NULL == dmac_bsd_user_info_search(((bsd_sta_info_item_stru*)p_check_addr)->auc_user_mac_addr)))
    {
        return 1;
    }
    return 0;
}
#endif


oal_void  dmac_bsd_user_add_handle(dmac_vap_stru * pst_dmac_vap,dmac_user_stru *pst_dmac_user)
{
    bsd_pre_steering_node_stru  *pst_pre_steering_user;
    oal_uint8                   *puc_addr;
    oal_uint32                  ul_need_init_flag = 1;
    oal_uint8                   uc_result = 0;
    bsd_sta_info_item_stru      *pst_user_info;
    oal_uint32                  ul_steering_user_flag = 0;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_dmac_vap)||(OAL_PTR_NULL == pst_dmac_user)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD,"{dmac_bsd_user_add_handle::arg null.}");
        return;
    }

    if(!g_st_bsd_mgmt.en_switch)
    {
        return;
    }

    puc_addr = pst_dmac_user->st_user_base_info.auc_user_mac_addr;

    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                     "{dmac_bsd_user_add_handle::user[xx:xx:xx:%02x:%02x:%02x]* add to %dG vap.}",
                     puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                     (WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)?2:5);

    if(dmac_bsd_vap_find(pst_dmac_vap)&&(g_st_bsd_mgmt.bit_cfg_switch))
    {
        //判断当前关联成功的用户是否是在pre-steer阶段引导的用户
        pst_pre_steering_user = dmac_bsd_find_pre_steering_user(puc_addr);
        if(pst_pre_steering_user == OAL_PTR_NULL)
        {
            //不是关联阶段引导的用户
            if(!oal_compare_mac_addr(puc_addr,g_st_bsd_mgmt.st_steering_ctl.auc_mac_addr))
            {
                //该用户是在关联之后正在引导的用户
                if(((g_st_bsd_mgmt.st_steering_ctl.pst_bsd_ap->past_dmac_vap[0] == pst_dmac_vap)
                   ||(g_st_bsd_mgmt.st_steering_ctl.pst_bsd_ap->past_dmac_vap[1] == pst_dmac_vap))
                   &&(pst_dmac_vap != g_st_bsd_mgmt.st_steering_ctl.pst_source_vap))
                {
                   //steering成功
                   uc_result = 1;
                }

                ul_need_init_flag = dmac_bsd_steering_result_handle(pst_dmac_vap,puc_addr,uc_result);
                pst_user_info = g_st_bsd_mgmt.st_steering_ctl.pst_user_info;
                if(uc_result)
                {
                    ul_steering_user_flag = 1;
                }
            }
            else
            {
                pst_user_info = dmac_bsd_user_info_search(puc_addr);
                if(OAL_PTR_NULL == pst_user_info)
                {
                    OAM_ERROR_LOG0(0, OAM_SF_BSD,"{dmac_bsd_user_add_handle::user info search fail.}");
                    return;
                }
            }
        }
        else
        {
            //该用户是在关联阶段引导的用户
            if(((pst_pre_steering_user->pst_bsd_ap->past_dmac_vap[0] == pst_dmac_vap)
               ||(pst_pre_steering_user->pst_bsd_ap->past_dmac_vap[1] == pst_dmac_vap))
               &&(pst_dmac_vap != pst_pre_steering_user->pst_source_vap)
               &&(!pst_pre_steering_user->uc_block_timeout))
            {
                //pre-steer成功
                uc_result = 1;
            }

            //所以需要重新搜索该用户对应的用户信息节点
            pst_user_info = dmac_bsd_user_info_search(puc_addr);
            if(OAL_PTR_NULL == pst_user_info)
            {
                OAM_ERROR_LOG0(0, OAM_SF_BSD,"{dmac_bsd_user_add_handle::user info search fail.}");
                return;
            }
            pst_pre_steering_user->pst_sta_info = pst_user_info;
            ul_need_init_flag = dmac_bsd_pre_steer_result_handle(pst_dmac_vap,pst_pre_steering_user,uc_result);
        }
    }
    else
    {
        pst_user_info = dmac_bsd_user_info_search(puc_addr);
        if(OAL_PTR_NULL == pst_user_info)
        {
            OAM_ERROR_LOG0(0, OAM_SF_BSD,"{dmac_bsd_user_add_handle::user info search fail.}");
            return;
        }
    }


    pst_user_info->uc_associate_cnt++;
    pst_user_info->uc_last_assoc_vap_id = pst_dmac_vap->st_vap_base_info.uc_vap_id;
    if(!ul_steering_user_flag)
    {
        pst_user_info->en_last_assoc_by_bsd = OAL_FALSE;
        pst_user_info->pst_bsd_source_vap =  OAL_PTR_NULL;
    }

    //用户关联之后，将该用户对应的用户信息从timestamp链表中删除，防止已关联的用户的信息被覆盖了
    if((pst_user_info->st_by_timestamp_list.pst_next != OAL_PTR_NULL)
        &&(pst_user_info->st_by_timestamp_list.pst_prev != OAL_PTR_NULL))
    {
        oal_dlist_delete_entry(&(pst_user_info->st_by_timestamp_list));
    }
#if 0
    if(OAL_PTR_NULL == p_check_addr)
    {
        p_check_addr = pst_user_info;
        //__cyg_profile_func_register(dmac_bsd_check);
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                     "{dmac_bsd_user_add_handle::profiling check func register!}");
        dmac_bsd_dump_nodeinfo((oal_uint8*)p_check_addr);
    }
#endif

    if(ul_need_init_flag)
    {
        //允许切换的用户才需要初始化该用户的状态监测需要使用的资源
        dmac_bsd_user_state_init(pst_dmac_vap, pst_dmac_user);
    }
}



oal_void  dmac_bsd_user_del_handle(dmac_vap_stru * pst_dmac_vap,dmac_user_stru *pst_dmac_user)
{
    oal_uint8   *puc_addr;
    bsd_sta_info_item_stru  *pst_user_info;
    //mac_vap_stru        *pst_mac_vap;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_dmac_vap)||(OAL_PTR_NULL == pst_dmac_user)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD,"{dmac_bsd_user_del_handle::arg null.}");
        return;
    }

    if(!g_st_bsd_mgmt.en_switch)
    {
        return;
    }

    puc_addr = pst_dmac_user->st_user_base_info.auc_user_mac_addr;

    OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                     "{dmac_bsd_user_del_handle::user[xx:xx:xx:%02x:%02x:%02x]* del from %dG vap.}",
                      puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                      (WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)?2:5);

    pst_user_info = dmac_bsd_user_info_search(puc_addr);
    if(OAL_PTR_NULL == pst_user_info)
    {
        OAM_ERROR_LOG3(0, OAM_SF_BSD,
                     "{dmac_bsd_user_del_handle::user[xx:xx:xx:%02x:%02x:%02x] info not found.}",
                      puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
        return;
    }

    //pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);
    if(pst_user_info->uc_associate_cnt)
    {
        pst_user_info->uc_associate_cnt--;
    }

    if(0 == pst_user_info->uc_associate_cnt)
    {
        //该用户从所有vap都去关联之后，需要将该用户的用户信息重新加入timestamp链表，可以重新被使用
        oal_dlist_add_tail(&(pst_user_info->st_by_timestamp_list), &g_st_bsd_mgmt.st_sta_info_tbl.st_list_head);
    }

    //去初始化该用户的状态监测需要使用的资源
    //dmac_bsd_user_state_deinit(pst_dmac_vap,pst_dmac_user);
    return;
}


OAL_STATIC oal_uint32 dmac_bsd_pre_steer_timeout_fn(void *prg)
{
    bsd_pre_steering_node_stru  *pst_pre_steering_user;

    if(OAL_UNLIKELY(OAL_PTR_NULL == prg))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD,"{dmac_bsd_pre_steer_timeout_fn::arg null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_pre_steering_user = (bsd_pre_steering_node_stru  *)prg;

    if(pst_pre_steering_user->uc_block_timeout)
    {
        OAM_WARNING_LOG3(0, OAM_SF_BSD,"{dmac_bsd_pre_steer_timeout_fn::user[xx:xx:xx:%02x:%02x:%02x]*'s pre-steer block timeout,the user will be re-steer!}",
            pst_pre_steering_user->auc_mac_addr[WLAN_MAC_ADDR_LEN-3],
            pst_pre_steering_user->auc_mac_addr[WLAN_MAC_ADDR_LEN-2],
            pst_pre_steering_user->auc_mac_addr[WLAN_MAC_ADDR_LEN-1]);

        dmac_bsd_pre_steer_user_del(pst_pre_steering_user);
    }
    else
    {
        OAM_WARNING_LOG3(0, OAM_SF_BSD,"{dmac_bsd_pre_steer_timeout_fn::user[xx:xx:xx:%02x:%02x:%02x]*'s pre-steer timeout,restart timer for block timeout!}",
            pst_pre_steering_user->auc_mac_addr[WLAN_MAC_ADDR_LEN-3],
            pst_pre_steering_user->auc_mac_addr[WLAN_MAC_ADDR_LEN-2],
            pst_pre_steering_user->auc_mac_addr[WLAN_MAC_ADDR_LEN-1]);

        pst_pre_steering_user->uc_block_timeout = 1;
        FRW_TIMER_RESTART_TIMER(&(pst_pre_steering_user->st_block_timer), DMAC_BSD_PRE_STEER_INVALID_TIMEOUT, OAL_FALSE);
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32 dmac_bsd_pre_steer_user_add(bsd_ap_stru *pst_bsd_ap,dmac_vap_stru *pst_dmac_vap,oal_uint8 *puc_addr,bsd_sta_info_item_stru* pst_sta)
{
    bsd_pre_steering_node_stru  *pst_pre_steering_user;
    dmac_device_stru            *pst_dmac_device;

    if(OAL_UNLIKELY((OAL_PTR_NULL == pst_dmac_vap)||(OAL_PTR_NULL == pst_bsd_ap)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_pre_steer_user_add::param null.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_pre_steer_user_add::dev null, dev id[%d].}", pst_dmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_pre_steering_user = (bsd_pre_steering_node_stru*)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(bsd_pre_steering_node_stru), OAL_TRUE);
    if (OAL_PTR_NULL == pst_pre_steering_user)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                         "{dmac_bsd_pre_steer_user_add::mem alloc fail.}");
        return OAL_FAIL;
    }


    OAM_WARNING_LOG3(0, OAM_SF_BSD,"{dmac_bsd_pre_steer_user_add::user[xx:xx:xx:%02x:%02x:%02x]* add into pre-steer dlist.}",
                      puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);


    OAL_MEMZERO(pst_pre_steering_user, OAL_SIZEOF(bsd_pre_steering_node_stru));

    oal_memcopy(pst_pre_steering_user->auc_mac_addr, puc_addr, WLAN_MAC_ADDR_LEN);
    pst_pre_steering_user->pst_bsd_ap = pst_bsd_ap;
    pst_pre_steering_user->pst_source_vap = pst_dmac_vap;
    pst_pre_steering_user->uc_block_timeout = 0;
    pst_pre_steering_user->pst_sta_info = pst_sta;
    pst_sta->ul_pre_steer_cnt++;
    oal_dlist_add_head(&(pst_pre_steering_user->st_list_entry), &g_st_bsd_mgmt.st_pre_steering_ctl.st_pre_steering_list);
    /*更新cache user*/
    g_st_bsd_mgmt.st_pre_steering_ctl.pst_cache_user_node = pst_pre_steering_user;


    FRW_TIMER_CREATE_TIMER(&(pst_pre_steering_user->st_block_timer),
                           dmac_bsd_pre_steer_timeout_fn,
                           DMAC_BSD_PRE_STEER_TIMEOUT,
                           (void *)pst_pre_steering_user,
                           OAL_TRUE,
                           OAM_MODULE_ID_DMAC,
                           pst_dmac_device->pst_device_base_info->ul_core_id);
    return OAL_SUCC;

}




OAL_STATIC bsd_sta_info_item_stru *dmac_bsd_user_info_alloc(oal_void)
{
    bsd_sta_info_item_stru *pst_user_info;
    oal_uint32              ul_offset;
    oal_dlist_head_stru     *pst_entry;

    if(g_st_bsd_mgmt.st_sta_info_tbl.ul_item_cnt < BSD_STA_INFO_MAX_CNT)
    {
        ul_offset = g_st_bsd_mgmt.st_sta_info_tbl.ul_item_cnt;
        pst_user_info = &g_st_bsd_mgmt.st_sta_info_tbl.ast_array[ul_offset];
        g_st_bsd_mgmt.st_sta_info_tbl.ul_item_cnt++;
        OAL_MEMZERO(pst_user_info, OAL_SIZEOF(bsd_sta_info_item_stru));
        oal_dlist_add_tail(&(pst_user_info->st_by_timestamp_list), &g_st_bsd_mgmt.st_sta_info_tbl.st_list_head);
    }
    else
    {
        if(oal_dlist_is_empty(&g_st_bsd_mgmt.st_sta_info_tbl.st_list_head))
        {
            OAM_ERROR_LOG1(0, OAM_SF_BSD,"{dmac_bsd_user_info_alloc::BSD_STA_INFO_MAX_CNT(%d) maybe not enough or the memory crashed!}",BSD_STA_INFO_MAX_CNT);
            return (bsd_sta_info_item_stru *)OAL_PTR_NULL;
        }

        pst_entry = g_st_bsd_mgmt.st_sta_info_tbl.st_list_head.pst_next;        //最老的记录
        pst_user_info = (bsd_sta_info_item_stru *)OAL_DLIST_GET_ENTRY(pst_entry, bsd_sta_info_item_stru, st_by_timestamp_list);

        //将最老的记录从timestamp链表删除
        oal_dlist_delete_entry(pst_entry);

        //将最老的记录从hash链表删除
        oal_dlist_delete_entry(&(pst_user_info->st_by_hash_list));

        OAL_MEMZERO(pst_user_info, OAL_SIZEOF(bsd_sta_info_item_stru));
        //成为最新的记录了
        oal_dlist_add_tail(pst_entry, &g_st_bsd_mgmt.st_sta_info_tbl.st_list_head);
    }
    return pst_user_info;
}


OAL_STATIC oal_void dmac_bsd_user_info_add(bsd_sta_info_item_stru * pst_user_info)
{
    oal_uint32              ul_user_hash_value;
    oal_dlist_head_stru     *pst_entry;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_user_info))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD,
                         "{dmac_bsd_user_info_add::arg null.}");
        return;
    }

    ul_user_hash_value = DMAC_BSD_CALCULATE_HASH_VALUE(pst_user_info->auc_user_mac_addr);

    pst_entry = &(pst_user_info->st_by_hash_list);
    oal_dlist_add_tail(pst_entry,&g_st_bsd_mgmt.st_sta_info_tbl.ast_hash_tbl[ul_user_hash_value]);
    return;
}


OAL_STATIC oal_void dmac_bsd_user_info_update(bsd_sta_info_item_stru * pst_user_info,oal_uint32 ul_new_timestamp)
{
    oal_dlist_head_stru     *pst_entry;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_user_info))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD,
                         "{dmac_bsd_user_info_update::arg null.}");
        return;
    }

    pst_entry = &(pst_user_info->st_by_timestamp_list);


    if((pst_entry->pst_next != OAL_PTR_NULL)
      &&(pst_entry->pst_prev != OAL_PTR_NULL))
    {
        //从原来的位置删除
        oal_dlist_delete_entry(pst_entry);

        //更新为最新的记录
        oal_dlist_add_tail(pst_entry, &g_st_bsd_mgmt.st_sta_info_tbl.st_list_head);
    }
    pst_user_info->ul_last_active_timestamp = ul_new_timestamp;
    return;
}



OAL_STATIC dmac_bsd_sta_cap_enum_uint8 dmac_bsd_user_cap_classify(dmac_vap_stru *pst_dmac_vap, bsd_sta_info_item_stru  *pst_user_info)
{
    oal_uint32              ul_timestamp;
    oal_uint32              ul_delta_time;
    oal_uint8               *puc_addr;

    if(OAL_UNLIKELY((OAL_PTR_NULL == pst_dmac_vap)||(OAL_PTR_NULL == pst_user_info)))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD,"{dmac_bsd_user_cap_classify::param null!}");
        return DMAC_BSD_STA_CAP_BUTT;
    }

    puc_addr = pst_user_info->auc_user_mac_addr;

    ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    if(pst_user_info->st_cap_classify_ctl.ul_last_rec_band != pst_dmac_vap->st_vap_base_info.st_channel.en_band)
    {
        ul_delta_time = (oal_uint32)OAL_TIME_GET_RUNTIME(pst_user_info->st_cap_classify_ctl.ul_last_rec_timestamp, ul_timestamp);
        if(ul_delta_time < DMAC_BSD_STA_CAP_CLASSIFY_TIME_TH*5)
        {
            if(DMAC_BSD_STA_CAP_SINGLE_BAND == pst_user_info->en_band_cap)
            {
                OAM_WARNING_LOG4(0, OAM_SF_BSD,
                     "{dmac_bsd_user_cap_classify::the user[xx:xx:xx:%02x:%02x:%02x]* cap update to dual band,delat_time=%d!}",
                    puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                    ul_delta_time);
            }
            else
            {
                OAM_WARNING_LOG4(0, OAM_SF_BSD,
                     "{dmac_bsd_user_cap_classify::the user[xx:xx:xx:%02x:%02x:%02x]* cap is dual band,delat_time=%d!}",
                    puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                    ul_delta_time);
            }
            pst_user_info->en_band_cap = DMAC_BSD_STA_CAP_DUAL_BAND;
        }
        else
        {
            pst_user_info->st_cap_classify_ctl.ul_last_rec_timestamp = ul_timestamp;
            pst_user_info->st_cap_classify_ctl.ul_last_rec_band = pst_dmac_vap->st_vap_base_info.st_channel.en_band;
            pst_user_info->st_cap_classify_ctl.ul_block_cnt = 1;
        }
    }
    else
    {
        ul_delta_time = (oal_uint32)OAL_TIME_GET_RUNTIME(pst_user_info->st_cap_classify_ctl.ul_last_rec_timestamp, ul_timestamp);
        if(ul_delta_time < DMAC_BSD_STA_CAP_CLASSIFY_TIME_TH)
        {
            pst_user_info->st_cap_classify_ctl.ul_block_cnt++;
            if(pst_user_info->st_cap_classify_ctl.ul_block_cnt >= DMAC_BSD_STA_CAP_CLASSIFY_CNT_TH)
            {
#ifdef _PRE_DEBUG_MODE
                if(g_st_bsd_mgmt.bit_debug_mode)
                {
                    OAM_WARNING_LOG4(0, OAM_SF_BSD,
                                    "{dmac_bsd_user_cap_classify::the user[xx:xx:xx:%02x:%02x:%02x]* cap is single band,delat_time=%d!}",
                                    puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                                    ul_delta_time);
                }
#endif
                pst_user_info->en_band_cap = DMAC_BSD_STA_CAP_SINGLE_BAND;
            }
        }
        else
        {
#ifdef _PRE_DEBUG_MODE
            if(g_st_bsd_mgmt.bit_debug_mode)
            {
                OAM_WARNING_LOG4(0, OAM_SF_BSD,
                                "{dmac_bsd_user_cap_classify::the user[xx:xx:xx:%02x:%02x:%02x] cap is single band,delat_time=%d!}",
                                puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                                ul_delta_time);
            }
#endif
            pst_user_info->en_band_cap = DMAC_BSD_STA_CAP_SINGLE_BAND;
            pst_user_info->st_cap_classify_ctl.ul_last_rec_timestamp = ul_timestamp;
            pst_user_info->st_cap_classify_ctl.ul_last_rec_band = pst_dmac_vap->st_vap_base_info.st_channel.en_band;
            pst_user_info->st_cap_classify_ctl.ul_block_cnt = 1;
        }
    }

    return pst_user_info->en_band_cap;
}


OAL_STATIC OAL_INLINE bsd_sta_info_item_stru* dmac_bsd_add_user(dmac_vap_stru *pst_dmac_vap,oal_uint8 *puc_addr,oal_uint32 ul_timestamp)
{
    bsd_sta_info_item_stru      *pst_user_info;

    pst_user_info = dmac_bsd_user_info_alloc();
    if(pst_user_info)
    {
        oal_memcopy(pst_user_info->auc_user_mac_addr, puc_addr, WLAN_MAC_ADDR_LEN);
        pst_user_info->en_band_cap = DMAC_BSD_STA_CAP_UNKOWN;

        pst_user_info->st_cap_classify_ctl.ul_last_rec_timestamp = ul_timestamp;
        pst_user_info->st_cap_classify_ctl.ul_last_rec_band = pst_dmac_vap->st_vap_base_info.st_channel.en_band;
        pst_user_info->st_cap_classify_ctl.ul_block_cnt = 1;


        pst_user_info->bit_steering_cfg_enable = OAL_TRUE;      //默认都允许切换
        pst_user_info->ul_last_active_timestamp = ul_timestamp;
        dmac_bsd_user_info_add(pst_user_info);
    }
    return pst_user_info;
}

dmac_bsd_handle_result_enum_uint8 dmac_bsd_rx_probe_req_frame_handle(dmac_vap_stru *pst_dmac_vap,oal_uint8 *puc_addr,oal_int8  c_rssi)
{
    bsd_ap_stru                 *pst_bsd_ap;
    bsd_pre_steering_node_stru  *pst_pre_steering_user;
    oal_uint16                  us_2g_load;
    oal_uint16                  us_5g_load;
    bsd_sta_info_item_stru      *pst_user_info;
    oal_uint32                  ul_timestamp;

    if(!g_st_bsd_mgmt.en_switch)
    {
        return DMAC_BSD_RET_CONTINUE;
    }

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_dmac_vap)||(OAL_PTR_NULL == puc_addr)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_rx_probe_req_frame_handle::param null.");
        return DMAC_BSD_RET_CONTINUE;
    }

#ifdef _PRE_DEBUG_MODE
    if(g_st_bsd_mgmt.bit_debug_mode)
    {
        OAM_WARNING_LOG4(0, OAM_SF_BSD,
                      "{dmac_bsd_rx_probe_req_frame_handle::%dG device capture [xx:xx:xx:%02x:%02x:%02x]'s probe req!}",
                      ((pst_dmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G)?2:5),
                      puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
    }
#endif

    ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    //判断该用户是否已经存在
    pst_user_info = dmac_bsd_user_info_search(puc_addr);
    if(OAL_PTR_NULL == pst_user_info)
    {

#ifdef _PRE_DEBUG_MODE
        if(g_st_bsd_mgmt.bit_debug_mode)
        {
            OAM_WARNING_LOG4(0, OAM_SF_BSD,
                          "{dmac_bsd_rx_probe_req_frame_handle::%dG device capture new user[xx:xx:xx:%02x:%02x:%02x]!}",
                          ((pst_dmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G)?2:5),
                          puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
        }
#endif
        pst_user_info = dmac_bsd_add_user(pst_dmac_vap,puc_addr,ul_timestamp);
        if(!pst_user_info)
        {
            return DMAC_BSD_RET_CONTINUE;
        }
    }
    else
    {
        dmac_bsd_user_info_update(pst_user_info,ul_timestamp);

        if(pst_user_info->en_band_cap != DMAC_BSD_STA_CAP_DUAL_BAND)
        {
            dmac_bsd_user_cap_classify(pst_dmac_vap,pst_user_info);
        }
    }

    //判断当前vap是否属于bsd ap
    pst_bsd_ap = dmac_bsd_vap_find(pst_dmac_vap);
    if((OAL_PTR_NULL == pst_bsd_ap)||(!g_st_bsd_mgmt.bit_cfg_switch))
    {
        return DMAC_BSD_RET_CONTINUE;
    }

    if(DMAC_BSD_STA_CAP_UNKOWN == pst_user_info->en_band_cap)
    {
        return DMAC_BSD_RET_BLOCK;
    }
    else if(DMAC_BSD_STA_CAP_SINGLE_BAND == pst_user_info->en_band_cap)
    {
       return DMAC_BSD_RET_CONTINUE;
    }

    if(g_st_bsd_mgmt.bit_steer_mode)
    {
        return DMAC_BSD_RET_CONTINUE;
    }

    /*因为BSD导致切换到目标vap,就暂时不响应源vap发送的probe/auth req,防止STA自己的行为逻辑导致又切回原来的频段，此策略待讨论!!!*/
#if 0
    if((OAL_TRUE == pst_user_info->en_last_assoc_by_bsd)&&(pst_dmac_vap == pst_user_info->pst_bsd_source_vap))
    {
#ifdef _PRE_DEBUG_MODE
        if(g_st_bsd_mgmt.bit_debug_mode)
        {
            OAM_WARNING_LOG4(0, OAM_SF_BSD,
                          "{dmac_bsd_rx_probe_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x]is already steered to %dG Band,Block!}",
                          puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                          (WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)?5:2);
        }
#endif
        return DMAC_BSD_RET_BLOCK;
    }
    else
#endif
    {
        if(pst_user_info->uc_associate_cnt)
        {
#ifdef _PRE_DEBUG_MODE
            if(g_st_bsd_mgmt.bit_debug_mode)
            {
                OAM_WARNING_LOG3(0, OAM_SF_BSD,
                          "{dmac_bsd_rx_probe_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x]is already associate,continue!}",
                          puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
            }
#endif
            return DMAC_BSD_RET_CONTINUE;
        }
    }


    if(0 != oal_compare_mac_addr(g_st_bsd_mgmt.st_steering_ctl.auc_mac_addr, puc_addr))
    {
        //判断当前用户是否是正在被关联阶段引导
        if(!g_st_bsd_mgmt.bit_pre_steer_swtich)
        {
            //关联阶段的引导如果关闭，那就直接返回
            return DMAC_BSD_RET_CONTINUE;
        }
        pst_pre_steering_user = dmac_bsd_find_pre_steering_user(puc_addr);
        if(pst_pre_steering_user == OAL_PTR_NULL)
        {
            //该用户还没有处于引导阶段
#ifdef _PRE_DEBUG_MODE
            if(g_st_bsd_mgmt.bit_debug_mode)
            {
                OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                              "{dmac_bsd_rx_probe_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x] is not in pre-steer state yet.}",
                              puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
            }
#endif

            if(dmac_bsd_vap_load_get(pst_bsd_ap->past_dmac_vap[0],&us_2g_load) != OAL_SUCC)
            {
                OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                                 "{dmac_bsd_rx_probe_req_frame_handle::got 2G bsd_ap load failed.}");


                return DMAC_BSD_RET_CONTINUE;
            }

            if(dmac_bsd_vap_load_get(pst_bsd_ap->past_dmac_vap[1],&us_5g_load) != OAL_SUCC)
            {
                OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                                 "{dmac_bsd_rx_probe_req_frame_handle::got 5G bsd_ap load failed.}");
                return DMAC_BSD_RET_CONTINUE;
            }

#ifdef _PRE_DEBUG_MODE
            if(g_st_bsd_mgmt.bit_debug_mode)
            {
                OAM_WARNING_LOG2(0, OAM_SF_BSD, "{dmac_bsd_rx_probe_req_frame_handle: bsd_ap load(2G)=%d,load(5G)=%d!}",
                                 us_2g_load,us_5g_load);
            }
#endif

            if(((us_2g_load < BSD_FREE_TH(pst_bsd_ap))&&(us_5g_load > BSD_BUSY_TH(pst_bsd_ap)))
               ||((us_5g_load < BSD_FREE_TH(pst_bsd_ap))&&(us_2g_load > BSD_BUSY_TH(pst_bsd_ap))))
            {
                //负载失衡
                if(((us_5g_load > BSD_BUSY_TH(pst_bsd_ap))&&(pst_dmac_vap == pst_bsd_ap->past_dmac_vap[1]))
                   ||((us_2g_load > BSD_BUSY_TH(pst_bsd_ap))&&(pst_dmac_vap == pst_bsd_ap->past_dmac_vap[0])))
                {

                    //当前vap所在频段繁忙
                    if(OAL_SUCC == dmac_bsd_pre_steer_user_add(pst_bsd_ap,pst_dmac_vap,puc_addr,pst_user_info))
                    {
                        OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                                         "{dmac_bsd_rx_probe_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x]* will be pre-steer to another %dG band.}",
                                         puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                                         ((pst_dmac_vap == pst_bsd_ap->past_dmac_vap[0])?5:2));
                        return DMAC_BSD_RET_BLOCK;
                    }
                }
            }
            else
            {
                //负载没有失衡
                if(pst_dmac_vap == pst_bsd_ap->past_dmac_vap[1])
                {
                    //当前是5G 频段的vap
                    //根据该probe req帧的rssi
                    if(c_rssi < BSD_RSSI_TH(pst_bsd_ap))
                    {
                        //rssi低于阀值，需要引导到2.4G
                        if(OAL_SUCC == dmac_bsd_pre_steer_user_add(pst_bsd_ap,pst_dmac_vap,puc_addr,pst_user_info))
                        {
                            OAM_WARNING_LOG4(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                                             "{dmac_bsd_rx_probe_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x]* will be pre-steer 2.4G due to rssi[-%d].}",
                                             puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],(256-(oal_uint8)c_rssi));
                            return DMAC_BSD_RET_BLOCK;
                        }
                    }
                }
            }
        }
        else
        {
#ifdef _PRE_DEBUG_MODE
            if(g_st_bsd_mgmt.bit_debug_mode)
            {
                OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                              "{dmac_bsd_rx_probe_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x] is in pre-steer state.}",
                              puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);

            }
#endif
            //该用户已经处于引导阶段
            if(pst_dmac_vap == pst_pre_steering_user->pst_source_vap)
            {
                 //仍旧是pre-steer的源vap接收到请求
                if(!pst_pre_steering_user->uc_block_timeout)
                {
                    return DMAC_BSD_RET_BLOCK;
                }
#ifdef _PRE_DEBUG_MODE
                else
                {
                    //该用户的关联阶段引导超时，不再阻塞该用户
                    if(g_st_bsd_mgmt.bit_debug_mode)
                    {
                        OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                                      "{dmac_bsd_rx_probe_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x] is in pre-steer state,but block timeout.}",
                                      puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);

                    }
                }
#endif
            }
        }
    }
    else
    {
#ifdef _PRE_DEBUG_MODE
        if(g_st_bsd_mgmt.bit_debug_mode)
        {
            OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                            "{dmac_bsd_rx_probe_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x] is steering sta.}",
                            puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
        }
#endif
        //判断当前vap是否是此用户需要steering的源vap
        if(g_st_bsd_mgmt.st_steering_ctl.pst_source_vap == pst_dmac_vap)
        {
            return DMAC_BSD_RET_BLOCK;
        }
    }

    return DMAC_BSD_RET_CONTINUE;
}


oal_uint16  dmac_bsd_encap_auth_rsp(mac_vap_stru *pst_mac_vap, oal_netbuf_stru *pst_auth_rsp, oal_netbuf_stru *pst_auth_req)
{
    oal_uint16       us_auth_rsp_len        = 0;
    oal_uint8       *puc_frame              = OAL_PTR_NULL;
    oal_uint16       us_index               = 0;
    oal_uint16       us_auth_type           = 0;
    oal_uint16       us_auth_seq            = 0;
    oal_uint8        auc_addr2[6]           = {0};
    oal_uint8       *puc_data;
    mac_tx_ctl_stru *pst_tx_ctl;
    oal_uint32       ul_ret;


    if (OAL_PTR_NULL == pst_mac_vap || OAL_PTR_NULL == pst_auth_rsp || OAL_PTR_NULL == pst_auth_req )
    {
        OAM_ERROR_LOG3(0, OAM_SF_AUTH,"{dmac_bsd_encap_auth_rsp::pst_mac_vap[0x%x], puc_data[0x%x], puc_auth_req[0x%x] }", pst_mac_vap, pst_auth_rsp, pst_auth_req);
        return us_auth_rsp_len;
    }

    puc_data    = (oal_uint8 *)OAL_NETBUF_HEADER(pst_auth_rsp);
    pst_tx_ctl  = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_auth_rsp);
    OAL_MEMZERO(pst_tx_ctl, sizeof(mac_tx_ctl_stru));
    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /*************************************************************************/
    /*                Set the fields in the frame header                     */
    /*************************************************************************/

    /* 设置函数头的frame control字段 */
    mac_hdr_set_frame_control(puc_data, WLAN_FC0_SUBTYPE_AUTH);

    /* 获取STA的地址 */
    mac_get_address2(oal_netbuf_header(pst_auth_req), auc_addr2);

    /* 将DA设置为STA的地址 */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_data)->auc_address1, auc_addr2);

    /* 将SA设置为dot11MacAddress */
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_data)->auc_address2, mac_mib_get_StationID(pst_mac_vap));
    oal_set_mac_addr(((mac_ieee80211_frame_stru *)puc_data)->auc_address3, pst_mac_vap->auc_bssid);

    /*************************************************************************/
    /*                Set the contents of the frame body                     */
    /*************************************************************************/

    /*************************************************************************/
    /*              Authentication Frame - Frame Body                        */
    /* --------------------------------------------------------------------- */
    /* |Auth Algo Number|Auth Trans Seq Number|Status Code| Challenge Text | */
    /* --------------------------------------------------------------------- */
    /* | 2              |2                    |2          | 3 - 256        | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    us_index = MAC_80211_FRAME_LEN;
    puc_frame = (oal_uint8 *)(puc_data + us_index);

    /* 计算认证相应帧的长度 */
    us_auth_rsp_len = MAC_80211_FRAME_LEN + MAC_AUTH_ALG_LEN + MAC_AUTH_TRANS_SEQ_NUM_LEN +
                      MAC_STATUS_CODE_LEN;

    /* 解析认证类型 */
    us_auth_type = mac_get_auth_algo_num(pst_auth_req);

    /* 解析auth transaction number */
    us_auth_seq  = mac_get_auth_seq_num(oal_netbuf_header(pst_auth_req));
    if (us_auth_seq > 4)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_AUTH,"{dmac_bsd_encap_auth_rsp::auth recieve invalid seq, auth seq [%d]}", us_auth_seq);
        return 0;
    }

    /* 设置认证类型IE */
    puc_frame[0] = (us_auth_type & 0x00FF);
    puc_frame[1] = (us_auth_type & 0xFF00) >> 8;

    /* 将收到的transaction number + 1后复制给新的认证响应帧 */
    puc_frame[2] = ((us_auth_seq + 1) & 0x00FF);
    puc_frame[3] = ((us_auth_seq + 1) & 0xFF00) >> 8;

    puc_frame[4] = MAC_UNSPEC_FAIL;
    puc_frame[5] = 0;

    ul_ret = mac_vap_set_cb_tx_user_idx(pst_mac_vap, pst_tx_ctl, auc_addr2);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "(dmac_bsd_encap_auth_rsp::fail to find user by xx:xx:xx:0x:0x:0x.}",
        auc_addr2[3],
        auc_addr2[4],
        auc_addr2[5]);
    }
    MAC_GET_CB_MPDU_LEN(pst_tx_ctl)    = us_auth_rsp_len;
    MAC_GET_CB_WME_AC_TYPE(pst_tx_ctl) = WLAN_WME_AC_MGMT;

    OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_AUTH, "{dmac_bsd_encap_auth_rsp::user mac:[xx:xx:xx:%02X:%02X:%02X]*}",
                     auc_addr2[3],auc_addr2[4],auc_addr2[5]);

    return us_auth_rsp_len;
}





OAL_STATIC oal_uint32 dmac_bsd_send_auth_res_frame(dmac_vap_stru *pst_dmac_vap,oal_netbuf_stru *pst_auth_req)
{
    oal_netbuf_stru            *pst_mgmt_buf;
    oal_uint16                  us_mgmt_len;
    oal_uint32                  ul_ret;

    /* 申请管理帧内存 */
    pst_mgmt_buf = OAL_MEM_NETBUF_ALLOC(OAL_MGMT_NETBUF, WLAN_MGMT_NETBUF_SIZE, OAL_NETBUF_PRIORITY_HIGH);
    if (OAL_PTR_NULL == pst_mgmt_buf)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN, "{dmac_bsd_send_auth_res_frame::pst_mgmt_buf null.}");
        OAL_MEM_INFO_PRINT(OAL_MEM_POOL_ID_NETBUF);
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_set_netbuf_prev(pst_mgmt_buf, OAL_PTR_NULL);
    oal_set_netbuf_next(pst_mgmt_buf, OAL_PTR_NULL);

    OAL_MEM_NETBUF_TRACE(pst_mgmt_buf, OAL_TRUE);

    us_mgmt_len = dmac_bsd_encap_auth_rsp(&pst_dmac_vap->st_vap_base_info, pst_mgmt_buf, pst_auth_req);
    if(0 == us_mgmt_len)
    {
        OAM_WARNING_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{dmac_bsd_send_auth_res_frame::auth response frame len is zero.");
        oal_netbuf_free(pst_mgmt_buf);
        return OAL_FAIL;
    }

    /* 调用发送管理帧接口 */
    ul_ret = dmac_tx_mgmt(pst_dmac_vap, pst_mgmt_buf, us_mgmt_len);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_RX, "{dmac_bsd_send_auth_res_frame::dmac_tx_mgmt failed[%d].", ul_ret);
        oal_netbuf_free(pst_mgmt_buf);
        return ul_ret;
    }

    return OAL_SUCC;
}


dmac_bsd_handle_result_enum_uint8 dmac_bsd_rx_auth_req_frame_handle(dmac_vap_stru *pst_dmac_vap,
                                                                    oal_uint8 *puc_addr,
                                                                    oal_netbuf_stru *pst_netbuf)
{
    bsd_ap_stru                 *pst_bsd_ap;
    bsd_sta_info_item_stru      *pst_user_info;
    oal_uint32                  ul_timestamp;

    if(!g_st_bsd_mgmt.en_switch)
    {
        return DMAC_BSD_RET_CONTINUE;
    }

    ul_timestamp = (oal_uint32)OAL_TIME_GET_STAMP_MS();
    pst_user_info = dmac_bsd_user_info_search(puc_addr);
    if(OAL_PTR_NULL == pst_user_info)
    {
        //测试时碰到有终端没有发送probe req，直接发送auth req就开始关联入网流程
        //所以此时需要新增该用户的信息。
        pst_user_info = dmac_bsd_add_user(pst_dmac_vap,puc_addr,ul_timestamp);
        if(!pst_user_info)
        {
            return DMAC_BSD_RET_CONTINUE;
        }
    }

    if(!g_st_bsd_mgmt.st_bsd_sched_timer.en_is_registerd)
    {
        return DMAC_BSD_RET_CONTINUE;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_rx_auth_req_frame_handle::pst_dmac_vap null.");
        return DMAC_BSD_RET_CONTINUE;
    }

#ifdef _PRE_DEBUG_MODE
    if(g_st_bsd_mgmt.bit_debug_mode)
    {
        OAM_WARNING_LOG4(0, OAM_SF_BSD,
                    "{dmac_bsd_rx_auth_req_frame_handle::%dG device capture [xx:xx:xx:%02x:%02x:%02x]'s auth req!}",
                    ((pst_dmac_vap->st_vap_base_info.st_channel.en_band == WLAN_BAND_2G)?2:5),
                    puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
    }
#endif

    /*因为BSD导致切换到目标vap,就暂时不响应源vap发送的probe/auth req,防止STA自己的行为逻辑导致又切回原来的频段，此策略待讨论!!!*/
    if((OAL_TRUE == pst_user_info->en_last_assoc_by_bsd)&&(pst_dmac_vap == pst_user_info->pst_bsd_source_vap))
    {
#ifdef _PRE_DEBUG_MODE
            if(g_st_bsd_mgmt.bit_debug_mode)
            {
                OAM_WARNING_LOG4(0, OAM_SF_BSD,
                          "{dmac_bsd_rx_auth_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x]is already steered to %dG Band,Block!}",
                          puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1],
                          (WLAN_BAND_2G == pst_dmac_vap->st_vap_base_info.st_channel.en_band)?5:2);
            }
#endif
            return DMAC_BSD_RET_BLOCK;
    }


    //判断当前vap是否属于bsd ap
    pst_bsd_ap = dmac_bsd_vap_find(pst_dmac_vap);
    if(OAL_PTR_NULL == pst_bsd_ap)
    {
        return DMAC_BSD_RET_CONTINUE;
    }

    if(!oal_compare_mac_addr(g_st_bsd_mgmt.st_steering_ctl.auc_mac_addr, puc_addr))
    {
        //判断当前vap是否是此用户需要steering的源vap
        if(g_st_bsd_mgmt.st_steering_ctl.pst_source_vap == pst_dmac_vap)
        {
            if(g_st_bsd_mgmt.bit_steer_mode)
            {
#ifdef _PRE_DEBUG_MODE
                if(g_st_bsd_mgmt.bit_debug_mode)
                {
                    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                                    "{dmac_bsd_rx_auth_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x] is steering sta,current vap is source vap,will send auth fail resp.}",
                                    puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
                }
#endif
                dmac_bsd_send_auth_res_frame(pst_dmac_vap,pst_netbuf);
            }
#ifdef _PRE_DEBUG_MODE
            else
            {
                if(g_st_bsd_mgmt.bit_debug_mode)
                {
                    OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                                    "{dmac_bsd_rx_auth_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x] is steering sta,current vap is source vap,auth process block.}",
                                    puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
                }
            }
#endif
            return DMAC_BSD_RET_BLOCK;
        }

        OAM_WARNING_LOG3(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD,
                         "{dmac_bsd_rx_auth_req_frame_handle::user[xx:xx:xx:%02x:%02x:%02x]* is steering sta,current vap is dest vap,auth process continue.}",
                         puc_addr[WLAN_MAC_ADDR_LEN-3],puc_addr[WLAN_MAC_ADDR_LEN-2],puc_addr[WLAN_MAC_ADDR_LEN-1]);
    }

    return DMAC_BSD_RET_CONTINUE;
}


oal_void  dmac_bsd_channel_switch_handle(dmac_vap_stru * pst_dmac_vap)
{
    dmac_device_stru    *pst_dmac_device;

    if(!g_st_bsd_mgmt.en_switch)
    {
        return;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_BSD, "{dmac_bsd_channel_switch_handle::pst_dmac_vap null.");
        return;
    }

    pst_dmac_device = dmac_res_get_mac_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_BSD, "{dmac_bsd_channel_switch_handle::dev null, dev id[%d].}",
                       pst_dmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    dmac_bsd_ringbuf_init(&(pst_dmac_device->st_bsd.st_sample_result_buf));
    return;
}


OAL_STATIC oal_uint32  dmac_bsd_cfg_switch_handle(dmac_vap_stru * pst_dmac_vap,oal_bool_enum_uint8 en_switch)
{
    oal_uint32      ul_ret = OAL_SUCC;

    if(OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_cfg_switch_handle:param null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if(en_switch)
    {
        if(!g_st_bsd_mgmt.st_bsd_sched_timer.en_is_registerd)
        {
            //bsd还未运行时，查找是否存在可以配对的vap，启动bsd ap
            if(dmac_bsd_vap_match())
            {
                //启动BSD_Task
                dmac_bsd_task_start();
            }
            else
            {
                //there is no bsd ap matched!
                ul_ret = OAL_FAIL;
                OAM_WARNING_LOG0(0, OAM_SF_BSD, "{dmac_bsd_cfg_switch_handle:no bsd ap matched!}");
            }
        }
    }
    else
    {
        //关闭bsd特性
        dmac_bsd_task_stop();
        dmac_bsd_pre_steer_list_clear(&g_st_bsd_mgmt.st_pre_steering_ctl);
    }

    g_st_bsd_mgmt.bit_cfg_switch = (OAL_TRUE == en_switch)?1:0;
    return ul_ret;
}



OAL_STATIC dmac_bsd_cfg_param_index_enum_uint8  dmac_config_param_search(OAL_CONST oal_uint8 *puc_str)
{
    oal_uint32   ul_index;

    for(ul_index = 0; ul_index < OAL_ARRAY_SIZE(ast_dmac_bsd_cfg_str);ul_index++)
    {
        if(!oal_strcmp((oal_int8*)puc_str,ast_dmac_bsd_cfg_str[ul_index].puc_param_str))
        {
            return ast_dmac_bsd_cfg_str[ul_index].en_param_index;
        }
    }

    return CFG_PARAM_INDEX_BUTT;
}



oal_uint32  dmac_config_bsd(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param)
{
    oal_int8              *pc_token;
    oal_int8              *pc_end;
    oal_int8              *pc_ctx;
    oal_int8              *pc_sep = " ";
    oal_bool_enum_uint8    en_val;
    oal_uint32             ul_value;
    dmac_bsd_cfg_param_index_enum_uint8 en_param_index;
#ifdef _PRE_DEBUG_MODE
    oal_uint8              auc_addr[WLAN_MAC_ADDR_LEN];
#endif

    /* 获取命令类型 */
    pc_token = oal_strtok((oal_int8 *)puc_param, pc_sep, &pc_ctx);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (0 == oal_strcmp(pc_token, "sw"))
    {
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd cfg sw=%d}",en_val);
        if (OAL_TRUE != en_val && OAL_FALSE != en_val)
        {
            return OAL_EINVAL;
        }
        dmac_bsd_cfg_switch_handle((dmac_vap_stru*)pst_mac_vap,en_val);
        return OAL_SUCC;
    }
    else if (0 == oal_strcmp(pc_token, "pre_steer"))
    {
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd pre-steer sw=%d}",en_val);
        if (OAL_TRUE != en_val && OAL_FALSE != en_val)
        {
            return OAL_EINVAL;
        }
        g_st_bsd_mgmt.bit_pre_steer_swtich = (0 == en_val)?0:1;
        if(0 == en_val)
        {
            dmac_bsd_pre_steer_list_clear(&g_st_bsd_mgmt.st_pre_steering_ctl);
        }
    }
    else if (0 == oal_strcmp(pc_token, "cfg"))
    {
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_param_index = dmac_config_param_search((oal_uint8*)pc_token);
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_value = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);

        return dmac_bsd_config_set_ext(MAC_GET_DMAC_VAP(pst_mac_vap),en_param_index,ul_value);
    }
    else if (0 == oal_strcmp(pc_token, "enable"))
    {
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd enable =%d}",en_val);
        if (OAL_TRUE != en_val && OAL_FALSE != en_val)
        {
            return OAL_EINVAL;
        }
        g_st_bsd_mgmt.en_switch = en_val;
        return OAL_SUCC;
    }
#ifdef _PRE_DEBUG_MODE
    else if (0 == oal_strcmp(pc_token, "load_status"))
    {
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd sw=%d}",en_val);
        if (OAL_TRUE != en_val && OAL_FALSE != en_val)
        {
            return OAL_EINVAL;
        }
        g_st_bsd_mgmt.bit_load_status_switch = (OAL_TRUE == en_val) ? 1 : 0;
    }
    else if (0 == oal_strcmp(pc_token, "debug_mode"))
    {
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd debug_mode=%d}",en_val);
        if (OAL_TRUE != en_val && OAL_FALSE != en_val)
        {
            return OAL_EINVAL;
        }
        g_st_bsd_mgmt.bit_debug_mode = (OAL_TRUE == en_val) ? 1 : 0;
    }
    else if (0 == oal_strcmp(pc_token, "2G_load"))
    {
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_value = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);
        if((ul_value == 0)||(ul_value > 9))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd 2G_load invalid}");
            return OAL_FAIL;
        }

        g_st_bsd_mgmt.uc_debug1 &= 0xf0;
        g_st_bsd_mgmt.uc_debug1 |= (oal_uint8)ul_value;
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd 2G_load=%d}",ul_value);
    }
    else if (0 == oal_strcmp(pc_token, "5G_load"))
    {
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_value = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);
        if((ul_value == 0)||(ul_value > 9))
        {
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd 5G_load invalid}");
            return OAL_FAIL;
        }

        g_st_bsd_mgmt.uc_debug1 &= 0x0f;
        g_st_bsd_mgmt.uc_debug1 |= (oal_uint8)(ul_value<<4);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd 5G_load=%d}",ul_value);
    }
    else if (0 == oal_strcmp(pc_token, "lock"))
    {
        /*hipriv "vap0 bsd lock xx:xx:xx:xx:xx:xx 0/1"   0-- 解锁  1:锁定*/
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        oal_strtoaddr(pc_token,auc_addr);
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        ul_value = (oal_uint32)oal_strtol(pc_token, &pc_end, 10);
        if (OAL_TRUE != ul_value && OAL_FALSE != ul_value)
        {
            return OAL_EINVAL;
        }
        return dmac_bsd_lock_user(auc_addr,ul_value);
    }
    else if (0 == oal_strcmp(pc_token, "steer_mode"))
    {
        /*hipriv "vap0 bsd steer_mode 0/1"   0-- 阻塞单播的probe req处理流程  1:通过回复失败的auth response来阻塞入网流程*/
        pc_token = oal_strtok(OAL_PTR_NULL, pc_sep, &pc_ctx);
        if (OAL_UNLIKELY(OAL_PTR_NULL == pc_token))
        {
            return OAL_ERR_CODE_PTR_NULL;
        }

        en_val = (oal_bool_enum_uint8)oal_strtol(pc_token, &pc_end, 10);
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_BSD,"{dmac_config_bsd::set bsd steer_mode=%d}",en_val);
        if (OAL_TRUE != en_val && OAL_FALSE != en_val)
        {
            return OAL_EINVAL;
        }
        g_st_bsd_mgmt.bit_steer_mode = (OAL_TRUE == en_val) ? 1 : 0;
    }
#endif
    else
    {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


#ifdef _PRE_DEBUG_MODE
oal_bool_enum_uint8 dmac_bsd_debug_switch(oal_void)
{
    return (oal_bool_enum_uint8)g_st_bsd_mgmt.bit_debug_mode;
}


#if 0
oal_void dmac_bsd_dump_nodeinfo(oal_uint8 *puc_node_addr)
{
    oal_uint32              i;
    oal_uint8               j;
    bsd_sta_info_item_stru  *pst_user_info;
    oal_dlist_head_stru     *pst_entry;
    oal_uint32              ul_sta_cnt = g_st_bsd_mgmt.st_sta_info_tbl.ul_item_cnt;
    oal_uint32              ul_node_index;
    oal_uint32              ul_base_addr = (oal_uint32)&g_st_bsd_mgmt.st_sta_info_tbl.ast_array;

    ul_node_index = ((oal_uint32)puc_node_addr - ul_base_addr)/OAL_SIZEOF(bsd_sta_info_item_stru);
    OAL_IO_PRINT("BSD Module Node(%p) Info dump,index=%d ",(oal_void*)puc_node_addr,ul_node_index);

    OAL_IO_PRINT("\r\nHex value:\r\n");
    for(i = 0; i < OAL_SIZEOF(bsd_sta_info_item_stru); i+=8)
    {
        OAL_IO_PRINT("%02x %02x %02x %02x %02x %02x %02x %02x\r\n",
        puc_node_addr[i],puc_node_addr[i+1],puc_node_addr[i+2],puc_node_addr[i+3],
        puc_node_addr[i+4],puc_node_addr[i+5],puc_node_addr[i+6],puc_node_addr[i+7]);
    }
    OAL_IO_PRINT("\r\n=======================\r\n");

    j = 0;
    for(i = 0; i < BSD_STA_INFO_HASH_TBL_SIZE; i++)
    {
        OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(g_st_bsd_mgmt.st_sta_info_tbl.ast_hash_tbl[i]))
        {
            pst_user_info = (bsd_sta_info_item_stru *)OAL_DLIST_GET_ENTRY(pst_entry, bsd_sta_info_item_stru, st_by_hash_list);
            if((oal_uint32)pst_user_info == (oal_uint32)puc_node_addr)
            {
                OAL_IO_PRINT("the current node in hash tbl[%d] dlist\r\n",i);
                j = 1;
                break;
            }
        }

        if(j)
        {
            break;
        }
    }

    j = 0;
    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(g_st_bsd_mgmt.st_sta_info_tbl.st_list_head))
    {
        pst_user_info = (bsd_sta_info_item_stru *)OAL_DLIST_GET_ENTRY(pst_entry, bsd_sta_info_item_stru, st_by_timestamp_list);
        if((oal_uint32)pst_user_info == (oal_uint32)puc_node_addr)
        {
            OAL_IO_PRINT("the current node in timestamp dlist\r\n");
            j = 1;
            break;
        }
    }

    if(0 == j)
    {
        OAL_IO_PRINT("the current node not in the timestamp dlist\r\n");
    }
    OAL_IO_PRINT("\r\n================ dump end (total node cnt=%d)================\r\n",ul_sta_cnt);
}
#endif


#endif

#endif/* #ifdef _PRE_WLAN_FEATURE_BAND_STEERING */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

