


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include "oam_ext_if.h"
#include "mac_data.h"
#include "dmac_arp_offload.h"
#include "dmac_tx_bss_comm.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_ARP_OFFLOAD_C


#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/


dmac_rx_frame_ctrl_enum_uint8 dmac_ao_process_arp_and_mcast(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_netbuf)
{
    oal_uint8           uc_data_type;
    mac_device_stru    *pst_mac_device;
    oal_netbuf_stru    *pst_tx_buf = OAL_PTR_NULL;
    oal_uint32          ul_ret = OAL_SUCC;
    dmac_rx_frame_ctrl_enum_uint8 en_ret;

    pst_mac_device = mac_res_get_dev(pst_dmac_vap->st_vap_base_info.uc_device_id);
    if (OAL_PTR_NULL  == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id,OAM_SF_PWR,"mac device is null");
        return DMAC_RX_FRAME_CTRL_DROP;
    }

    /* 未开开关 */
    if (OAL_FALSE == pst_mac_device->uc_arpoffload_switch)
    {
        return DMAC_RX_FRAME_CTRL_GOON;
    }

    uc_data_type = mac_get_data_type(pst_netbuf);

    /* 特殊帧/报文先过滤 */
    switch (uc_data_type)
    {
        case MAC_DATA_EAPOL:
        {
            return DMAC_RX_FRAME_CTRL_GOON;
        }

        case MAC_DATA_ARP_REQ:
        case MAC_DATA_ARP_RSP:
        {
            /* 组帧 */
            en_ret = dmac_ao_process_arp_offload(pst_dmac_vap, pst_dmac_user, pst_netbuf, &pst_tx_buf);
            if (OAL_PTR_NULL != pst_tx_buf)
            {
                ul_ret = dmac_tx_process_data(pst_dmac_vap->pst_hal_device, pst_dmac_vap, pst_tx_buf);
                if (OAL_SUCC != ul_ret)
                {
                    OAM_WARNING_LOG1(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_ao_process_arp_and_mcast::Dmac tx process data failed. ret[%d]}",ul_ret);
                    dmac_tx_excp_free_netbuf(pst_tx_buf);
                    en_ret = DMAC_RX_FRAME_CTRL_GOON;
                }
                g_ul_arpoffload_send_arprsp++;
            }

            return en_ret;
        }

        case MAC_DATA_ND:
        {
            /* 组帧 */
            en_ret = dmac_ao_process_nd_offload(pst_dmac_vap, pst_dmac_user, pst_netbuf, &pst_tx_buf);
            if (OAL_PTR_NULL != pst_tx_buf)
            {
                if (OAL_SUCC != dmac_tx_process_data(pst_dmac_vap->pst_hal_device, pst_dmac_vap, pst_tx_buf))
                {
                    OAM_ERROR_LOG0(pst_dmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_PWR, "{dmac_ao_process_arp_and_mcast::Dmac tx data failed.}");
                    dmac_tx_excp_free_netbuf(pst_tx_buf);
                    en_ret = DMAC_RX_FRAME_CTRL_GOON;
                }
            }

            return en_ret;
        }

        case MAC_DATA_DHCP:
        {
            return dmac_ao_process_dhcp_filter(pst_dmac_vap, pst_netbuf);
        }

        case MAC_DATA_DHCPV6:
        {
            return dmac_ao_process_dhcpv6_filter(pst_dmac_vap, pst_netbuf);
        }

        default:
        {
            break;
        }
    }

    /* STAUT的IP广播/组播过滤、MAC广播/组播过滤 */
    if (WLAN_VAP_MODE_BSS_STA == pst_dmac_vap->st_vap_base_info.en_vap_mode)
    {
        return dmac_ao_sta_process_multicast_filter(pst_dmac_vap, pst_netbuf);
    }

    return DMAC_RX_FRAME_CTRL_GOON;
}

#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


