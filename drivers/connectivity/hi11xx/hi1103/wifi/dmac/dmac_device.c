
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"

#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_vap.h"
#ifdef _PRE_WLAN_FEATURE_AP_PM
#include "mac_pm.h"
#endif
#include "dmac_resource.h"
#include "dmac_device.h"
#include "dmac_reset.h"
#include "dmac_blockack.h"
#include "dmac_scan.h"
#include "dmac_alg.h"
#ifdef _PRE_SUPPORT_ACS
#include "dmac_acs.h"
#endif
#include "dmac_fcs.h"
#include "dmac_dfx.h"
#include "dmac_beacon.h"

#ifdef _PRE_WLAN_DFT_STAT
#include "dmac_dft.h"
#endif

#ifdef _PRE_WLAN_FEATURE_GREEN_AP
#include "dmac_green_ap.h"
#endif

#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
#include "dmac_auto_cali.h"
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
#include "dmac_m2s.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_DEVICE_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_IP_FILTER


OAL_STATIC oal_void dmac_ip_filter_exit(oal_void)
{
    g_st_dmac_board.st_rx_ip_filter.en_state = MAC_RX_IP_FILTER_STOPED;
    g_st_dmac_board.st_rx_ip_filter.uc_btable_size      = 0;
    g_st_dmac_board.st_rx_ip_filter.uc_btable_items_num = 0;

    OAL_MEMZERO(g_auc_ip_filter_btable, OAL_SIZEOF(g_auc_ip_filter_btable));

    g_st_dmac_board.st_rx_ip_filter.pst_filter_btable = OAL_PTR_NULL;
    return;
}

OAL_STATIC oal_void dmac_ip_filter_init(oal_void)
{
    /* 初始化功能控制变量 */
    g_pst_mac_board->st_rx_ip_filter.en_state = MAC_RX_IP_FILTER_STOPED;
    g_pst_mac_board->st_rx_ip_filter.uc_btable_size      = OAL_SIZEOF(g_auc_ip_filter_btable) / OAL_SIZEOF(mac_ip_filter_item_stru);
    g_pst_mac_board->st_rx_ip_filter.uc_btable_items_num = 0;
    g_pst_mac_board->st_rx_ip_filter.pst_filter_btable   = OAL_PTR_NULL;

    OAL_MEMZERO(g_auc_ip_filter_btable, OAL_SIZEOF(g_auc_ip_filter_btable));
    g_pst_mac_board->st_rx_ip_filter.pst_filter_btable = (mac_ip_filter_item_stru *)g_auc_ip_filter_btable;

    return;
}

#endif //_PRE_WLAN_FEATURE_IP_FILTER


oal_void  dmac_free_hd_tx_dscr_queue(hal_to_dmac_device_stru *pst_hal_dev_stru)
{
    oal_uint8                        uc_index;
    hal_tx_dscr_stru                *pst_tx_dscr;
    oal_dlist_head_stru             *pst_hal_dscr_header;
    oal_netbuf_stru                 *pst_buf = OAL_PTR_NULL;


    for (uc_index = 0; uc_index < HAL_TX_QUEUE_NUM; uc_index++)
    {
        pst_hal_dscr_header = &(pst_hal_dev_stru->ast_tx_dscr_queue[uc_index].st_header);

        while(OAL_TRUE != oal_dlist_is_empty(pst_hal_dscr_header))
        {
            pst_tx_dscr = OAL_DLIST_GET_ENTRY(pst_hal_dscr_header->pst_next, hal_tx_dscr_stru, st_entry);
            pst_buf = pst_tx_dscr->pst_skb_start_addr;

            oal_dlist_delete_entry(&pst_tx_dscr->st_entry);

            OAL_MEM_FREE(pst_tx_dscr, OAL_TRUE);
            dmac_tx_excp_free_netbuf(pst_buf);
        }
    }
}

oal_uint32  dmac_device_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip, dmac_device_stru *pst_dmac_device)
{
    mac_device_stru         *pst_device;
    hal_to_dmac_device_stru *pst_hal_device;
    oal_uint8                uc_hal_dev_num_per_chip;
    oal_uint8                uc_hal_device_idx;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_device_exit::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device = pst_dmac_device->pst_device_base_info;
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        return OAL_SUCC;
    }

#ifdef _PRE_WLAN_RF_CALI
    dmac_auto_cali_exit(pst_device);
#endif

    /* 不对外直接暴露规格宏 */
    hal_chip_get_device_num(pst_chip->uc_chip_id, &uc_hal_dev_num_per_chip);
    if(uc_hal_dev_num_per_chip > WLAN_DEVICE_MAX_NUM_PER_CHIP)
    {
        OAM_ERROR_LOG2(0, OAM_SF_CFG, "{dmac_device_exit::uc_hal_dev_num_per_chip is %d,more than %d",
                    uc_hal_dev_num_per_chip, WLAN_DEVICE_MAX_NUM_PER_CHIP);
        return OAL_FAIL;
    }

    for (uc_hal_device_idx = 0; uc_hal_device_idx < uc_hal_dev_num_per_chip; uc_hal_device_idx++)
    {
        pst_hal_device = pst_dmac_device->past_hal_device[uc_hal_device_idx];
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        dmac_free_hd_tx_dscr_queue(pst_hal_device);

#ifdef _PRE_WLAN_FEATURE_M2S
        /* m2s 状态机注销 */
        dmac_m2s_fsm_detach(pst_hal_device);
#endif

#ifdef _PRE_WLAN_FEATURE_DFR
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
        /* 注销无法送完成中断检测定时器 */
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hal_device->st_dfr_tx_prot.st_tx_prot_timer));

        /* 注销pcie错误检测定时器 */
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hal_device->st_pcie_err_timer));
#endif
#endif

#ifdef _PRE_DEBUG_MODE
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hal_device->st_exception_report_timer));
#endif

#ifdef _PRE_WLAN_DFT_REG
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hal_device->st_reg_prd_timer)); /* 寄存器读取删除定时器 */
#endif

#ifdef _PRE_WLAN_FEATURE_BTCOEX
        if(OAL_TRUE == pst_hal_device->st_btcoex_powersave_timer.en_is_registerd)
        {
            FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_hal_device->st_btcoex_powersave_timer));
        }
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
        pst_hal_device->p_tbtt_update_beacon_func = OAL_PTR_NULL;
#endif /* #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)   */

    }


#ifdef _PRE_WLAN_FEATURE_AP_PM
    mac_pm_arbiter_destroy(pst_device);
#endif

    /*释放公共结构体*/
    /*释放公共结构体 以及 对应衍生特性*/
    mac_device_exit(pst_device);

#ifdef _PRE_SUPPORT_ACS
    /* 清除dmac acs */
    dmac_acs_exit(pst_device);
#endif

#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    /* green ap 注销函数 */
    dmac_green_ap_exit(pst_dmac_device->pst_device_base_info);
#endif

    /* 取消keepalive定时器 */
    if(OAL_TRUE == pst_device->st_keepalive_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_device->st_keepalive_timer));
    }

#ifdef _PRE_WLAN_DFT_STAT
    if (OAL_TRUE == pst_device->st_dbb_env_param_ctx.st_collect_period_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_device->st_dbb_env_param_ctx.st_collect_period_timer));
    }
#endif

/* 删除动态校准定时器 */
#ifdef _PRE_WLAN_REALTIME_CALI
    if (OAL_TRUE == pst_device->st_realtime_cali_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&(pst_device->st_realtime_cali_timer));
    }
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    if (OAL_TRUE == pst_device->st_obss_scan_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_device->st_obss_scan_timer);
    }
#endif

    /* 指向基础mac device的指针为空，待定时解耦后，本处理提前 */
    pst_dmac_device->pst_device_base_info  = OAL_PTR_NULL;
    pst_dmac_device->pst_hal_chip          = OAL_PTR_NULL;
    OAL_MEMZERO(pst_dmac_device->past_hal_device, OAL_SIZEOF(pst_dmac_device->past_hal_device[0])*WLAN_DEVICE_MAX_NUM_PER_CHIP);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_chip_exit(mac_board_stru *pst_board, mac_chip_stru *pst_chip)
{
    dmac_device_stru  *pst_dmac_device;
    oal_uint32         ul_ret;
    oal_uint8          uc_device;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_chip || OAL_PTR_NULL == pst_board))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_chip_exit::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    dmac_chip_alg_exit(pst_chip);
    if(pst_chip->uc_device_nums > WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_chip_exit::uc_device_nums is %d,more than %d.}", pst_chip->uc_device_nums, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);
        return OAL_FAIL;
    }

    for (uc_device = 0; uc_device < pst_chip->uc_device_nums; uc_device++)
    {
         pst_dmac_device = dmac_res_get_mac_dev(pst_chip->auc_device_id[uc_device]);

         /* TBD 置换位置 释放资源 */
         dmac_res_free_mac_dev(pst_chip->auc_device_id[uc_device]);

         ul_ret = dmac_device_exit(pst_board, pst_chip, pst_dmac_device);
         if (OAL_SUCC != ul_ret)
         {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_chip_exit::dmac_chip_exit failed[%d].}", ul_ret);
             return ul_ret;
         }
    }

    /*释放基础结构*/
    ul_ret = mac_chip_exit(pst_board, pst_chip);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_chip_exit::mac_chip_exit failed[%d].}", ul_ret);
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32  dmac_board_exit(mac_board_stru *pst_board)
{
    oal_uint8        uc_chip_idx;
    oal_uint32       ul_ret;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_board))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_board_exit::pst_board null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    while (0 != pst_board->uc_chip_id_bitmap)
    {
        /* 获取最右边一位为1的位数，此值即为chip的数组下标 */
        uc_chip_idx = oal_bit_find_first_bit_one_byte(pst_board->uc_chip_id_bitmap);
        if (OAL_UNLIKELY(uc_chip_idx >= WLAN_CHIP_MAX_NUM_PER_BOARD))
        {
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "{hmac_board_exit::invalid uc_chip_idx[%d] uc_chip_id_bitmap=%d.}",
                           uc_chip_idx, pst_board->uc_chip_id_bitmap);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        ul_ret = dmac_chip_exit(pst_board, &(pst_board->ast_chip[uc_chip_idx]));
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_board_exit::mac_chip_exit failed[%d].}", ul_ret);
            return ul_ret;
        }

        /* 清除对应的bitmap位 */
        oal_bit_clear_bit_one_byte(&pst_board->uc_chip_id_bitmap, uc_chip_idx);
    }

    /*公共部分的初始化*/
    mac_board_exit(pst_board);

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    /* rx ip数据包过滤功能的去初始化 */
    dmac_ip_filter_exit();
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */

    return OAL_SUCC;
}




#ifdef _PRE_DEBUG_MODE

oal_uint32  dmac_device_exception_report_timeout_fn(oal_void *p_arg)
{
#if ((_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE))
    oal_uint8                   uc_pci_device_id = 0;
    oal_uint32                  ul_reg_pci_rpt_val  = 0;
    oal_uint32                  ul_pci_warn_clear_cfg_val = 0xFFFFFFFF;             /* 写1清 */
    oal_uint32                  ul_reg_pci_rpt_addr_offset = 0x110;                 /* PCIE 0x110寄存器 */
    oal_bus_chip_stru           *pst_bus_chip = OAL_PTR_NULL;
    hal_to_dmac_device_stru     *pst_hal_device = OAL_PTR_NULL;

    pst_hal_device = (hal_to_dmac_device_stru *)p_arg;

    /* 获取chip id值 */
    uc_pci_device_id = pst_hal_device->uc_chip_id;

    oal_bus_get_chip_instance(&pst_bus_chip, uc_pci_device_id);
    if(OAL_PTR_NULL == pst_bus_chip)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY,"{dmac_device_exception_report_timeout_fn:: pst_bus_chip null");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 然后读取1151侧的 PCIE */
    ul_reg_pci_rpt_val   = 0;
    oal_pci_read_config_dword(pst_bus_chip->pst_pci_device, ul_reg_pci_rpt_addr_offset, &ul_reg_pci_rpt_val);

    pst_hal_device->ul_pcie_read_counter++;

    /* Bit[12]: Timer Timeout Status， 判断是否发生 timeout异常 */
    if(0 != (ul_reg_pci_rpt_val & 0x1000))
    {
        pst_hal_device->ul_pcie_reg110_timeout_counter++;

        OAM_WARNING_LOG4(0, OAM_SF_ANY,
         "{dmac_device_exception_report_timeout_fn:: read 1151 pcie reg0x110 timeout, chip id = %d, device id = %d, reg0x110 = [0x%08x]}, timeout counter: %d.",
            pst_hal_device->uc_chip_id, pst_hal_device->uc_mac_device_id, ul_reg_pci_rpt_val, pst_hal_device->ul_pcie_reg110_timeout_counter);

        oal_pci_write_config_dword(pst_bus_chip->pst_pci_device, ul_reg_pci_rpt_addr_offset, ul_pci_warn_clear_cfg_val);
    }
    /* 为防止出现 timeout异常后，长时间再未出现异常，每隔 10 * 64秒打印一次 timeout异常统计 */
    else if(0 == (pst_hal_device->ul_pcie_read_counter & 0x3F))
    {
        OAM_WARNING_LOG3(0, OAM_SF_ANY,
             "{dmac_device_exception_report_timeout_fn:: chip id = %d, device id = %d, reg0x110 read timeout counter: %d.",
                pst_hal_device->uc_chip_id, pst_hal_device->uc_mac_device_id, pst_hal_device->ul_pcie_reg110_timeout_counter);
    }
#endif

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_DFT_REG

OAL_STATIC oal_uint32  dmac_reg_timeout(void *p_arg)
{
    hal_to_dmac_device_stru    *pst_device;
    oal_uint32                  ul_ret;

    pst_device = (hal_to_dmac_device_stru *)p_arg;
    if (OAL_PTR_NULL == pst_device)
    {
        MAC_ERR_LOG(0, "ptr is null !");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_reg_timeout::pst_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_debug_refresh_reg_ext(pst_device, OAM_REG_EVT_PRD, &ul_ret);
    if(OAL_SUCC != ul_ret)
    {
        return OAL_FAIL;
    }
    hal_debug_frw_evt(pst_device);

    return OAL_SUCC;
}

#endif


OAL_STATIC oal_uint32 dmac_cfg_vap_init(mac_device_stru *pst_device)
{
    dmac_vap_stru              *pst_dmac_vap;
    mac_cfg_add_vap_param_stru  st_param = {0};       /* 构造配置VAP参数结构体 */
    dmac_device_stru           *pst_dmac_device;

    /* 分配dmac vap内存空间 */
    mac_res_alloc_dmac_vap(pst_device->uc_device_id);
    pst_device->uc_cfg_vap_id = pst_device->uc_device_id;

    pst_dmac_device = dmac_res_get_mac_dev(pst_device->uc_device_id);
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_device->uc_cfg_vap_id);
    pst_dmac_vap->pst_hal_device = DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device); //cfg vap都默认挂在自己的主路上
    pst_dmac_vap->pst_hal_vap    = OAL_PTR_NULL;

    st_param.en_vap_mode = WLAN_VAP_MODE_CONFIG;
    mac_vap_init(&(pst_dmac_vap->st_vap_base_info), pst_device->uc_chip_id, pst_device->uc_device_id, pst_device->uc_cfg_vap_id, &st_param);

    return OAL_SUCC;
}

#ifdef _PRE_FEATURE_FAST_AGING

oal_uint32 dmac_fast_aging_timeout(oal_void *pst_void_dmac_dev)
{
    dmac_device_stru                *pst_dmac_device;
    dmac_user_stru                  *pst_dmac_user;
    dmac_user_query_stats_stru      *pst_query_stats;
    oal_uint32                       ul_tx_succ_gap;
    oal_uint32                       ul_tx_fail_gap;
    oal_uint32                       ul_rx_gap;
    dmac_user_fast_aging_stru       *pst_user_fast_aging;
    oal_dlist_head_stru             *pst_entry;
    oal_dlist_head_stru             *pst_dlist_tmp;
    mac_user_stru                   *pst_mac_user;
    mac_vap_stru                    *pst_mac_vap;
    oal_uint8                        uc_vap_index;
    mac_device_stru                 *pst_mac_device;

    pst_dmac_device = (dmac_device_stru*)pst_void_dmac_dev;
    if (OAL_PTR_NULL == pst_dmac_device)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_fast_aging_timeout::pst_hmac_dev null}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    //遍历device下所有vap的所有user
    /* 遍历device下所有vap */
    pst_mac_device = pst_dmac_device->pst_device_base_info;
    for (uc_vap_index = 0; uc_vap_index <pst_mac_device->uc_vap_num; uc_vap_index++)
    {
        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_index]);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_fast_aging_timeout::pst_vap null,vap_id=%d.}", pst_mac_device->auc_vap_id[uc_vap_index]);
            continue;
        }

        /* VAP模式判断, 只有ap模式 */
        if (WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)
        {
             continue;
        }

        /* 遍历vap下所有user */
        //MAC_VAP_FOREACH_USER_SAFE(pst_mac_user, pst_mac_vap, pst_list_pos)
        OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, &(pst_mac_vap->st_mac_user_list_head))
        {
            pst_mac_user = OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_dlist);
            /*lint -save -e774 */
            if (OAL_PTR_NULL == pst_mac_user)
            {
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_KEEPALIVE, "pst_user_tmp is null.");
                return OAL_ERR_CODE_PTR_NULL;
            }

            /* 如果用户尚未关联成功，则不访问私有信息结构体 */
            if (MAC_USER_STATE_ASSOC != pst_mac_user->en_user_asoc_state)
            {
                continue;
            }

            //获取dmac user
            pst_dmac_user = (dmac_user_stru*)mac_res_get_dmac_user(pst_mac_user->us_assoc_id);
            if (OAL_PTR_NULL == pst_dmac_user)
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_fast_aging_timeout::pst_dmac_user null}");
                return OAL_ERR_CODE_PTR_NULL;
            }

            //判断定时器周期内，该用户发包收包情况
            pst_query_stats = &pst_dmac_user->st_query_stats;
            if (OAL_PTR_NULL == pst_query_stats)
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_fast_aging_timeout::pst_query_stats null}");
                return OAL_ERR_CODE_PTR_NULL;
            }
            //tx发送失败个数; 发送成功个数不增，发送失败个数增长，说明tx全失败了

            pst_user_fast_aging = &pst_dmac_user->st_user_fast_aging;
            if (OAL_PTR_NULL == pst_user_fast_aging)
            {
                OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_fast_aging_timeout::pst_user_fast_aging null}");
                return OAL_ERR_CODE_PTR_NULL;
            }

            //user下记录上次定时器超时时候的发送成功和失败个数
            ul_tx_succ_gap = pst_query_stats->ul_hw_tx_pkts - pst_user_fast_aging->ul_tx_msdu_succ;
            ul_tx_fail_gap = pst_query_stats->ul_hw_tx_failed - pst_user_fast_aging->ul_tx_msdu_fail;
            ul_rx_gap = pst_query_stats->ul_drv_rx_pkts - pst_user_fast_aging->ul_rx_msdu;

            OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{dmac_fast_aging_timeout:: ul_tx_succ_gap=%d, ul_tx_fail_gap=%d, ul_rx_gap=%d, uc_fast_aging_num=%d}",
                ul_tx_succ_gap, ul_tx_fail_gap, ul_rx_gap, pst_user_fast_aging->uc_fast_aging_num);

            //ul_tx_succ_gap为0，ul_tx_fail_gap不为0，同时rx为0, 说明此时用户异常
            if (0 == ul_tx_succ_gap && ul_tx_fail_gap && 0 == ul_rx_gap)
            {
                pst_user_fast_aging->uc_fast_aging_num++;
            }
            else if (ul_tx_succ_gap || ul_rx_gap) //ul_tx_succ_gap非0，或ul_rx_gap非0就清空
            {
                pst_user_fast_aging->uc_fast_aging_num = 0;
            }
            else
            {
                ;
            }

            //超过10次，直接踢用户
            if ((pst_user_fast_aging->uc_fast_aging_num) >= pst_dmac_device->st_fast_aging.uc_count_limit)
            {
                OAM_WARNING_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_CONN, "{dmac_fast_aging_timeout:: because DMAC_DISASOC_MISC_FAST_AGINIG, user[%2X:XX:XX:XX:%2X:%2X] exceed limit[%d].}",
                    pst_mac_user->auc_user_mac_addr[0], pst_mac_user->auc_user_mac_addr[4],
                    pst_mac_user->auc_user_mac_addr[5],pst_dmac_device->st_fast_aging.uc_count_limit);

                pst_user_fast_aging->uc_fast_aging_num = 0;
                dmac_send_disasoc_misc_event(pst_mac_vap, pst_dmac_user->st_user_base_info.us_assoc_id, DMAC_DISASOC_MISC_FAST_AGINIG);
                continue;
            }

            //将本次定时器超时后的数据记录在pst_user_fast_aging，供下次使用
            pst_user_fast_aging->ul_tx_msdu_succ = pst_query_stats->ul_hw_tx_pkts;
            pst_user_fast_aging->ul_tx_msdu_fail = pst_query_stats->ul_hw_tx_failed;
            pst_user_fast_aging->ul_rx_msdu = pst_query_stats->ul_drv_rx_pkts;
        }
    }

    return OAL_SUCC;
}

OAL_STATIC oal_uint32 dmac_fast_aging_init(dmac_device_stru *pst_dmac_device)
{
    pst_dmac_device->st_fast_aging.en_enable = OAL_TRUE;
    pst_dmac_device->st_fast_aging.us_timeout_ms = DMAC_FAST_AGING_TIMEOUT;
    pst_dmac_device->st_fast_aging.uc_count_limit = DMAC_FAST_AGING_NUM_LIMIT;

    if (pst_dmac_device->pst_device_base_info && pst_dmac_device->st_fast_aging.en_enable)
    {
        FRW_TIMER_CREATE_TIMER(&pst_dmac_device->st_fast_aging.st_timer,
                               dmac_fast_aging_timeout,
                               pst_dmac_device->st_fast_aging.us_timeout_ms,
                               (oal_void *)pst_dmac_device,
                               OAL_TRUE,
                               OAM_MODULE_ID_DMAC,
                               pst_dmac_device->pst_device_base_info->ul_core_id);
    }
    return OAL_SUCC;
}
#endif


oal_bool_enum_uint8 dmac_device_is_dynamic_dbdc(dmac_device_stru *pst_dmac_device)
{
    oal_uint8          uc_mac_dev_num_per_chip;
    oal_uint8          uc_hal_dev_num_per_chip;
    mac_chip_stru     *pst_mac_chip;

    pst_mac_chip = dmac_res_get_mac_chip(pst_dmac_device->pst_device_base_info->uc_chip_id);
    if (OAL_PTR_NULL == pst_mac_chip)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_device_is_dynamic_dbdc::pst_mac_chip[%d] null.}",
                        pst_dmac_device->pst_device_base_info->uc_chip_id);
        return OAL_FALSE;
    }

    hal_chip_get_device_num(pst_dmac_device->pst_device_base_info->uc_chip_id, &uc_hal_dev_num_per_chip);

    uc_mac_dev_num_per_chip = oal_chip_get_device_num(pst_mac_chip->ul_chip_ver);

    if ((uc_mac_dev_num_per_chip > 0) && (uc_hal_dev_num_per_chip > uc_mac_dev_num_per_chip))
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

oal_void dmac_register_hal_dev_cb(hal_to_dmac_device_stru *pst_hal_device)
{
#ifdef _PRE_WLAN_FEATURE_DBDC
    pst_hal_device->st_hal_dev_fsm.st_hal_device_fsm_cb.p_dbdc_handle_stop_event = dmac_dbdc_handle_stop_event;
#endif
    pst_hal_device->st_hal_dev_fsm.st_hal_device_fsm_cb.p_dmac_scan_one_channel_start = dmac_scan_one_channel_start;
}

OAL_STATIC oal_void dmac_set_hal_device_params(dmac_device_stru *pst_dmac_device, hal_to_dmac_device_stru *pst_hal_device)
{
    hal_cfg_rts_tx_param_stru  st_hal_rts_tx_param;

#ifdef _PRE_WLAN_DFT_STAT
    oam_stats_phy_node_idx_stru st_phy_node_idx = {{OAM_STATS_PHY_NODE_TX_CNT,
                                                    OAM_STATS_PHY_NODE_RX_OK_CNT,
                                                    OAM_STATS_PHY_NODE_11B_HDR_ERR_CNT,
                                                    OAM_STATS_PHY_NODE_OFDM_HDR_ERR_CNT}};
#endif

    /* 初始化2.4G和5G的RTS速率, RTS[0~2]设为24Mbps,  RTS[3]在2.4G设为1Mbps、5G设为24Mbps*/
    st_hal_rts_tx_param.auc_protocol_mode[0]    = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    st_hal_rts_tx_param.auc_rate[0]             = WLAN_LEGACY_OFDM_24M_BPS;
    st_hal_rts_tx_param.auc_protocol_mode[1]    = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    st_hal_rts_tx_param.auc_rate[1]             = WLAN_LEGACY_OFDM_24M_BPS;
    st_hal_rts_tx_param.auc_protocol_mode[2]    = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    st_hal_rts_tx_param.auc_rate[2]             = WLAN_LEGACY_OFDM_24M_BPS;

    st_hal_rts_tx_param.en_band                 = WLAN_BAND_2G;
    st_hal_rts_tx_param.auc_protocol_mode[3]    = WLAN_11B_PHY_PROTOCOL_MODE;
    st_hal_rts_tx_param.auc_rate[3]             = WLAN_LONG_11b_1_M_BPS;
    hal_set_rts_rate_params(pst_hal_device, &st_hal_rts_tx_param);

    st_hal_rts_tx_param.en_band                 = WLAN_BAND_5G;
    st_hal_rts_tx_param.auc_protocol_mode[3]    = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
    st_hal_rts_tx_param.auc_rate[3]             = WLAN_LEGACY_OFDM_24M_BPS;
    hal_set_rts_rate_params(pst_hal_device, &st_hal_rts_tx_param);

#ifdef _PRE_WLAN_DFT_STAT
        /* 初始化phy统计节点 */
    dmac_dft_set_phy_stat_node(pst_hal_device, &st_phy_node_idx);
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    pst_hal_device->p_tbtt_update_beacon_func = dmac_irq_tbtt_ap_isr;
#endif /* #if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)  */

#ifdef _PRE_DEBUG_MODE
    pst_hal_device->ul_pcie_read_counter           = 0;
    pst_hal_device->ul_pcie_reg110_timeout_counter = 0;
    OAL_MEMZERO(&(pst_hal_device->st_exception_report_timer), OAL_SIZEOF(frw_timeout_stru));
    FRW_TIMER_CREATE_TIMER(&(pst_hal_device->st_exception_report_timer),
                            dmac_device_exception_report_timeout_fn,
                            MAC_EXCEPTION_TIME_OUT,
                            pst_hal_device,
                            OAL_TRUE,
                            OAM_MODULE_ID_DMAC,
                            pst_hal_device->ul_core_id);
#endif

#ifdef _PRE_WLAN_DFT_REG
    /* 初始化寄存器读取定时器 */
    FRW_TIMER_CREATE_TIMER(&(pst_hal_device->st_reg_prd_timer),
                               dmac_reg_timeout,
                               OAM_REGSTER_REFRESH_TIME_MS,
                               pst_hal_device,
                               OAL_TRUE,
                               OAM_MODULE_ID_DMAC,
                               pst_hal_device->ul_core_id);
#endif

    /* 保存mac层的device id */
    pst_hal_device->uc_mac_device_id = pst_dmac_device->pst_device_base_info->uc_device_id;

#ifdef _PRE_WLAN_FEATURE_DFR
    dmac_dfr_init(pst_hal_device);
#endif
    dmac_register_hal_dev_cb(pst_hal_device);
}

oal_uint32 dmac_device_add_hal_device(dmac_device_stru *pst_dmac_device, oal_uint8 uc_chip_id, oal_uint8 uc_hal_dev_id)
{
    oal_uint32                 ul_ret;
    oal_uint8                  uc_hal_dev_idx;
    hal_to_dmac_device_stru   *pst_hal_device;
    oal_uint8                  uc_mac_device_id = pst_dmac_device->pst_device_base_info->uc_device_id;

    if (OAL_TRUE == dmac_device_is_dynamic_dbdc(pst_dmac_device))
    {
        for (uc_hal_dev_idx = 0; uc_hal_dev_idx < WLAN_DEVICE_MAX_NUM_PER_CHIP; uc_hal_dev_idx++)
        {
            ul_ret = hal_chip_get_hal_device(uc_chip_id, uc_hal_dev_idx, &pst_hal_device);
            if ((OAL_SUCC != ul_ret) || (OAL_PTR_NULL == pst_hal_device))
            {
                OAM_WARNING_LOG2(0, OAM_SF_CFG, "{dmac_device_init::hal_chip_get_hal_device[%d]hal device[%p]}", ul_ret, pst_hal_device);
                dmac_res_free_mac_dev(uc_mac_device_id);
                mac_res_free_dev(uc_mac_device_id);
                return ul_ret;
            }
            /* 动态DBDC,dmac device会保存两个hal device的指针 */
            dmac_set_hal_device_params(pst_dmac_device, pst_hal_device);
            pst_dmac_device->past_hal_device[uc_hal_dev_idx] = pst_hal_device;

#ifdef _PRE_WLAN_FEATURE_M2S
            /* m2s 状态机初始化 */
            dmac_m2s_fsm_attach(pst_hal_device);
#endif
        }
    }
    else //static DBDC/NOT DBDC
    {
        ul_ret = hal_chip_get_hal_device(uc_chip_id, uc_hal_dev_id, &pst_hal_device); //只能用入参来取hal device!!!
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{dmac_device_init::hal_chip_get_device failed[%d].}", ul_ret);

            dmac_res_free_mac_dev(uc_mac_device_id);
            mac_res_free_dev(uc_mac_device_id);
            return ul_ret;
        }

        dmac_set_hal_device_params(pst_dmac_device, pst_hal_device);

        /* 无论是主路或是辅路的hal device, en_is_master_hal_device都置成TRUE */
        pst_hal_device->en_is_master_hal_device = OAL_TRUE;

        for (uc_hal_dev_idx = 0; uc_hal_dev_idx < WLAN_DEVICE_MAX_NUM_PER_CHIP; uc_hal_dev_idx++)
        {
            pst_dmac_device->past_hal_device[uc_hal_dev_idx] = (0 == uc_hal_dev_idx) ? pst_hal_device : OAL_PTR_NULL;
        }
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_device_init(mac_chip_stru *pst_chip, oal_uint8 *puc_device_id, oal_uint8 uc_chip_id, oal_uint8 uc_device_id, oal_uint32 ul_chip_ver)
{
    oal_uint32                 ul_ret;
    oal_uint8                  uc_dev_id;
    mac_device_stru           *pst_device;
    mac_data_rate_stru        *pst_data_rate = OAL_PTR_NULL;
    oal_uint32                 ul_rate_num = 0;
    oal_uint32                 ul_idx;
    oal_uint32                 ul_loop = 0;
    dmac_device_stru          *pst_dmac_device;
    hal_to_dmac_device_stru   *pst_hal_device;

    /*申请公共mac device结构体，修改下 名称*/
    ul_ret = mac_res_alloc_dmac_dev(&uc_dev_id);
    if(OAL_UNLIKELY(ul_ret != OAL_SUCC))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_device_init::mac_res_alloc_dmac_dev failed[%d].}", ul_ret);

        return OAL_FAIL;
    }

    pst_device = mac_res_get_dev(uc_dev_id);

    if (OAL_PTR_NULL == pst_device)
    {
       OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_device_init::pst_device null.}");

       return OAL_ERR_CODE_PTR_NULL;
    }

    /* 申请dmac device资源 */
    if(OAL_UNLIKELY(dmac_res_alloc_mac_dev(uc_dev_id) != OAL_SUCC))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_device_init::dmac_res_alloc_mac_dev failed.}");
        return OAL_FAIL;
    }

    /* 初始化device级别的一些参数 */
    ul_ret = mac_device_init(pst_device, ul_chip_ver, uc_chip_id, uc_dev_id);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_device_init::mac_device_init failed[%d], chip_ver[0x%x].}", ul_ret, ul_chip_ver);
        dmac_res_free_mac_dev(uc_dev_id);
        mac_res_free_dev(uc_dev_id);
        return ul_ret;
    }

    /* 获取dmac device，并进行相关参数赋值 */
    pst_dmac_device = dmac_res_get_mac_dev(uc_dev_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_device))
    {
       OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_device_init::pst_dmac_device null.}");
       mac_res_free_dev(uc_dev_id);
       return OAL_ERR_CODE_PTR_NULL;
    }

    /* 结构体初始化 */
    OAL_MEMZERO(pst_dmac_device, OAL_SIZEOF(*pst_dmac_device));
    pst_dmac_device->pst_device_base_info = pst_device;

#ifdef _PRE_WLAN_MAC_BUGFIX_SW_CTRL_RSP
    pst_dmac_device->en_usr_bw_mode = WLAN_BAND_ASSEMBLE_20M;
#endif

#if (WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS)
    pst_dmac_device->uc_fake_vap_id =0xFF;
#endif

    dmac_device_add_hal_device(pst_dmac_device, uc_chip_id, uc_device_id);

    /* 使用master进行下面的初始化 */
    pst_hal_device = DMAC_DEV_GET_MST_HAL_DEV(pst_dmac_device);
    pst_device->ul_core_id = pst_hal_device->ul_core_id;

    if(OAL_SUCC != mac_fcs_init(&pst_device->st_fcs_mgr, pst_device->uc_chip_id, uc_dev_id))
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{dmac_device_init::mac_fcs_init failed.}");
        dmac_res_free_mac_dev(uc_dev_id);
        mac_res_free_dev(uc_dev_id);
        return OAL_FAIL;
    }

#if (defined _PRE_WLAN_RF_CALI) || (defined _PRE_WLAN_RF_CALI_1151V2)
    dmac_auto_cali_init(pst_device);
#endif

    pst_device->us_total_mpdu_num = 0;

    /* 错误计数初始化 */
    OAL_MEMZERO(pst_device->aul_mac_err_cnt, OAL_SIZEOF(pst_device->aul_mac_err_cnt));

    /*初始化记录性能统计第一次时间戳*/
    pst_device->ul_first_timestamp = 0;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    pst_device->st_dataflow_brk_bypass.en_brk_limit_aggr_enable = OAL_FALSE;
    pst_device->st_dataflow_brk_bypass.ul_tx_dataflow_brk_cnt = 0;
    pst_device->st_dataflow_brk_bypass.ul_last_tx_complete_isr_cnt = 0;
#endif
    for (ul_loop = 0; ul_loop < WLAN_WME_AC_BUTT; ul_loop++)
    {
        pst_device->aus_ac_mpdu_num[ul_loop] = 0;
    }

    for (ul_loop = 0; ul_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; ul_loop++)
    {
        pst_device->aus_vap_mpdu_num[ul_loop] = 0;
    }

    for (ul_loop = WLAN_WME_AC_BE; ul_loop < WLAN_WME_AC_BUTT; ul_loop++)
    {
        pst_device->aus_ac_mpdu_num[ul_loop] = 0;
    }

#ifdef _PRE_WLAN_DFT_STAT
    /* 初始化维测参数，空口环境类参数 */
    OAL_MEMZERO(&(pst_device->st_dbb_env_param_ctx), OAL_SIZEOF(mac_device_dbb_env_param_ctx_stru));
#endif


#ifdef _PRE_WLAN_FEATURE_AP_PM
#if (_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5)
    pst_device->en_pm_enable   = OAL_TRUE;
#else
    pst_device->en_pm_enable   = OAL_FALSE;
#endif
    mac_pm_arbiter_init(pst_device);
#endif

    /* 初始化TXOP特性相关值 */
    pst_device->en_txop_enable       = OAL_FALSE;
    pst_device->uc_tx_ba_num = 0;

    /* 从eeprom或flash获得MAC地址 */
    hal_get_hw_addr(pst_hal_device, pst_device->auc_hw_addr);

    /* 初始化DEVICE下的速率集 */
    hal_get_rate_80211g_table(pst_hal_device, (oal_void *)&pst_data_rate);
    if(OAL_PTR_NULL == pst_data_rate)
    {
       OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_device_init::pst_data_rate null.}");
       return OAL_ERR_CODE_PTR_NULL;
    }

    hal_get_rate_80211g_num(pst_hal_device, &ul_rate_num);

    for (ul_idx = 0; ul_idx < ul_rate_num; ul_idx++)
    {
        oal_memcopy(&(pst_device->st_mac_rates_11g[ul_idx]),&pst_data_rate[ul_idx],OAL_SIZEOF(mac_data_rate_stru));
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)||(_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
    /* TBD  */
    /* hal_enable_radar_det(pst_device->pst_device_stru, OAL_TRUE); */
    pst_device->us_dfs_timeout = 0;
#endif

    /* 初始化DMAC SCANNER */
    dmac_scan_init(pst_device);

#ifdef _PRE_SUPPORT_ACS
    /* 初始化ACS结构体 */
    dmac_acs_init(pst_device);
#endif

#ifdef _PRE_WLAN_FEATURE_GREEN_AP
    /* Green AP特性结构体初始化 */
    dmac_green_ap_init(pst_dmac_device->pst_device_base_info);
#endif

    pst_dmac_device->en_fast_scan_enable = OAL_FALSE;      //默认软件不支持快速扫描,调通后打开
    pst_dmac_device->en_dbdc_enable      = OAL_FALSE;      //默认软件不支持dbdc,调通后打开
    pst_dmac_device->uc_gscan_mac_vap_id = 0xff;           //gscan的vap id，add delvap时更新

    /* 出参赋值，CHIP中需要保存该device id */
    *puc_device_id = uc_dev_id;

    ul_ret = dmac_cfg_vap_init(pst_device);
    if(OAL_UNLIKELY(ul_ret != OAL_SUCC))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_device_init::hmac_cfg_vap_init failed[%d].}", ul_ret);
        return OAL_FAIL;
    }

    /* 初始化chip指针 */
    pst_dmac_device->pst_hal_chip = MAC_CHIP_GET_HAL_CHIP(pst_chip);

    //rssi limit 初始化, 默认关闭
    pst_dmac_device->st_rssi_limit.en_rssi_limit_enable_flag = OAL_FALSE;
    pst_dmac_device->st_rssi_limit.c_rssi = WAL_HIPRIV_RSSI_DEFAULT_THRESHOLD;
    pst_dmac_device->st_rssi_limit.c_rssi_delta = DMAC_RSSI_LIMIT_DELTA;

    /* vap创建之后，mib能力需要根据hal device重新刷新并同步到host侧 用于切换到siso之后，go等vap重新创建 */
    /* 用于切换到siso之后，go等vap重新创建 ,开关机时候，vap m2s能力同步到host */
#ifdef _PRE_WLAN_FEATURE_M2S
    /* 刷新m2s切换mimo nss能力，如果初始mac device nss等于single，后续不支持m2s动态切换 */
    if (OAL_SUCC != dmac_m2s_d2h_device_info_syn(pst_device))
    {
        OAM_WARNING_LOG0(0, OAM_SF_M2S,
               "{dmac_device_init::dmac_m2s_d2h_device_info_syn failed.}");
    }
#endif

#ifdef _PRE_FEATURE_FAST_AGING
    dmac_fast_aging_init(pst_dmac_device);
#endif


    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_chip_init(mac_chip_stru *pst_chip, oal_uint8 uc_chip_id)
{
    oal_uint8  uc_device;
    oal_uint8  uc_device_max;
    oal_uint32 ul_ret;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_chip))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_chip_init::pst_chip null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_chip->uc_chip_id = uc_chip_id;

    /*根据芯片id，获取hal_to_dmac_chip_stru结构体*/
    ul_ret = hal_chip_get_chip(uc_chip_id, &pst_chip->pst_hal_chip);
    if (OAL_SUCC != ul_ret)
    {
       OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_chip_init::hal_chip_get_chip failed[%d].}", ul_ret);
       return ul_ret;
    }

    /* CHIP调用接口 hal_get_chip_version*/
    hal_get_chip_version(pst_chip->pst_hal_chip, &pst_chip->ul_chip_ver);

    /* OAL接口获取支持业务device个数 */
    uc_device_max = oal_chip_get_device_num(pst_chip->ul_chip_ver);
    if (!uc_device_max)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_chip_init::hal_chip_get_device_num is zero.}");
        return OAL_FAIL;
    }

    dmac_chip_alg_init(pst_chip);

    for (uc_device = 0; uc_device < uc_device_max; uc_device++)
    {
        ul_ret = dmac_device_init(pst_chip, &pst_chip->auc_device_id[uc_device], uc_chip_id, uc_device, pst_chip->ul_chip_ver);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_chip_init::dmac_device_init failed[%d].}", ul_ret);

            return ul_ret;
        }
    }

    /*公共部分初始化,dmac执行*/
    mac_chip_init(pst_chip, uc_device_max);

    return OAL_SUCC;
}


oal_uint32  dmac_board_init(mac_board_stru *pst_board)
{
    oal_uint8  uc_chip;
    oal_uint32 ul_ret;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_board))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_board_init::pst_board null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    /*公共部分初始化*/
    mac_board_init();

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    /* rx ip数据包过滤功能的初始化 */
    dmac_ip_filter_init();
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */

    /* chip支持的最大数由PCIe总线处理提供 */
    for (uc_chip = 0; uc_chip < oal_bus_get_chip_num(); uc_chip++)
    {
        ul_ret = dmac_chip_init(&pst_board->ast_chip[uc_chip], uc_chip);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_board_init::dmac_chip_init failed[%d].}", ul_ret);

            return ul_ret;
        }

        oal_bit_set_bit_one_byte(&pst_board->uc_chip_id_bitmap, uc_chip);
    }

    return OAL_SUCC;
}

hal_to_dmac_device_stru*  dmac_device_get_another_h2d_dev(dmac_device_stru *pst_dmac_device, hal_to_dmac_device_stru *pst_ori_hal_dev)
{
    oal_uint8                   uc_hal_device_idx;

    for (uc_hal_device_idx = 0; uc_hal_device_idx < WLAN_DEVICE_MAX_NUM_PER_CHIP; uc_hal_device_idx++)
    {
        if (pst_dmac_device->past_hal_device[uc_hal_device_idx] != pst_ori_hal_dev)
        {
            return pst_dmac_device->past_hal_device[uc_hal_device_idx];
        }
    }

    return OAL_PTR_NULL;
}

oal_bool_enum_uint8 dmac_device_is_support_double_hal_device(dmac_device_stru *pst_dmac_device)
{
    /* MASTER ID 默认挂接不会为空 */
    if (OAL_PTR_NULL == pst_dmac_device->past_hal_device[HAL_DEVICE_ID_MASTER])
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_device_is_support_double_hal_device::dmac device[%d]master hal device is null",
                        pst_dmac_device->pst_device_base_info->uc_device_id);
        return OAL_FALSE;
    }

    /* 再去检查dmac device挂接的另外一路是否为空 */
    if (OAL_PTR_NULL == dmac_device_get_another_h2d_dev(pst_dmac_device, pst_dmac_device->past_hal_device[HAL_DEVICE_ID_MASTER]))
    {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

hal_to_dmac_device_stru *dmac_device_get_work_hal_device(dmac_device_stru *pst_dmac_device)
{
    oal_uint8                               uc_idx;
    oal_uint8                               uc_dev_num_per_chip;
    hal_to_dmac_device_stru                *pst_hal_device;

    hal_chip_get_device_num(pst_dmac_device->pst_device_base_info->uc_chip_id, &uc_dev_num_per_chip);
    if(uc_dev_num_per_chip > WLAN_DEVICE_MAX_NUM_PER_CHIP)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "dmac_device_get_work_hal_device::uc_dev_num_per_chip is %d,more than %d", uc_dev_num_per_chip, WLAN_DEVICE_MAX_NUM_PER_CHIP);
        return OAL_PTR_NULL;
    }

    for (uc_idx = 0; uc_idx < uc_dev_num_per_chip; uc_idx++)
    {
        pst_hal_device = pst_dmac_device->past_hal_device[uc_idx];
        if (OAL_PTR_NULL == pst_hal_device)
        {
            continue;
        }

        if (HAL_DEVICE_WORK_STATE == GET_HAL_DEVICE_STATE(pst_hal_device))
        {
            return pst_hal_device;
        }
    }
    return OAL_PTR_NULL;
}

oal_uint8  dmac_device_check_fake_queues_empty(oal_uint8 uc_device_id)
{
    mac_device_stru             *pst_mac_device;
    mac_vap_stru                *pst_mac_vap;
    oal_uint8                    uc_vap_idx;

    pst_mac_device = mac_res_get_dev(uc_device_id);
    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "dmac_device_is_fake_queues_empty::dmac device[%d]is null",uc_device_id);
        return OAL_FALSE;
    }

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_mac_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_device_is_fake_queues_empty::pst_mac_vap null, vap id is %d.}",
                           pst_mac_device->auc_vap_id[uc_vap_idx]);
            continue;
        }

        if (dmac_vap_fake_queue_empty_assert(pst_mac_vap, THIS_FILE_ID) != OAL_TRUE)
        {
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}

oal_void  dmac_mac_error_cnt_inc(mac_device_stru *pst_mac_device, oal_uint8 uc_mac_int_bit)
{
    pst_mac_device->aul_mac_err_cnt[uc_mac_int_bit] += 1;
}


oal_bool_enum_uint8 dmac_device_check_is_vap_in_assoc(mac_device_stru  *pst_mac_device)
{
    oal_uint8              uc_vap_idx;
#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
    oal_uint8              ul_loop;
    oal_bool_enum_uint8    en_get_ip;
#endif
    mac_user_stru         *pst_mac_user;
    oal_dlist_head_stru   *pst_list_pos;
    mac_vap_stru          *pst_mac_vap;
    dmac_vap_stru         *pst_dmac_vap;

    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_dmac_vap = (dmac_vap_stru *)mac_res_get_dmac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        pst_mac_vap = &(pst_dmac_vap->st_vap_base_info);
        if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
        {
            /* JOIN->UP */
            if(((pst_mac_vap->en_vap_state >= MAC_VAP_STATE_STA_JOIN_COMP) && (pst_mac_vap->en_vap_state <= MAC_VAP_STATE_STA_WAIT_ASOC))
#ifdef _PRE_WLAN_FEATURE_ROAM
            || (MAC_VAP_STATE_ROAMING == pst_mac_vap->en_vap_state)
#endif
                )
            {
                OAM_WARNING_LOG2(uc_vap_idx, OAM_SF_ANY,"{dmac_device_check_is_vap_in_assoc::vap[%d]state[%d]}",
                    pst_mac_vap->uc_vap_id, pst_mac_vap->en_vap_state);
                return OAL_TRUE;
            }
            /* UP后eapol + DHCP */
            else if ((MAC_VAP_STATE_UP == pst_mac_vap->en_vap_state) || (MAC_VAP_STATE_PAUSE == pst_mac_vap->en_vap_state))
            {
            #ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
                en_get_ip = OAL_FALSE;//开始置false，拿到ip置true
                for (ul_loop = 0; ul_loop < DMAC_MAX_IPV4_ENTRIES; ul_loop++)
                {
                    /* ip地址存储数组有一个不为0就当做拿到了ip地址 */
                    if ((pst_dmac_vap->pst_ip_addr_info->ast_ipv4_entry[ul_loop].un_local_ip.ul_value) != 0)
                    {
                       en_get_ip = OAL_TRUE;
                       break;
                    }
                }

                /* 任何一个没获取到ip地址,返回还在入网中 */
                if (OAL_FALSE == en_get_ip)
                {
                    return OAL_TRUE;
                }
            #endif
            }
        }
        else if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
        {
            /* APUT启动中当做还没准备好 */
            if (pst_mac_vap->en_vap_state != MAC_VAP_STATE_UP)
            {
                return OAL_TRUE;
            }

            MAC_VAP_FOREACH_USER(pst_mac_user, pst_mac_vap, pst_list_pos)
            {
                /* sta正在关联过程中,这里只保护到关联响应结束 */
                if(pst_mac_user->en_user_asoc_state != MAC_USER_STATE_ASSOC)
                {
                    OAM_WARNING_LOG2(uc_vap_idx, OAM_SF_ANY,"{dmac_device_check_is_vap_in_assoc::user[%d] asoc state[%d]",
                        pst_mac_user->us_assoc_id, pst_mac_user->en_user_asoc_state);
                    return OAL_TRUE;
                }
            }
        }
    }
    return OAL_FALSE;
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
