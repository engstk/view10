

#ifndef __DMAC_ARP_OFFLOAD_H__
#define __DMAC_ARP_OFFLOAD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "oal_net.h"
#include "dmac_vap.h"
#include "dmac_user.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_ARP_OFFLOAD_H

#ifdef _PRE_WLAN_FEATURE_ARP_OFFLOAD

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/


/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DMAC_AO_UNKNOWN_TYPE        0xFF

/*****************************************************************************
  3 枚举定义
*****************************************************************************/



/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern    oal_uint32  g_ul_arpoffload_send_arprsp;
extern    oal_uint32  g_ul_arpoffload_drop_frame;

typedef oal_uint32 (*ao_encap_cb)(dmac_vap_stru *pst_dmac_vap,
                                       dmac_user_stru *pst_dmac_user,
                                       oal_netbuf_stru *pst_tx_buf,
                                       oal_uint16 us_payload_len,
                                       oal_uint8 *puc_dst_mac_addr);
typedef oal_uint32 (*ao_encap_ieee80211_head)(dmac_vap_stru *pst_dmac_vap,
                                                     dmac_user_stru *pst_dmac_user,
                                                     mac_ieee80211_frame_stru *pst_rx_hdr,
                                                     oal_netbuf_stru *pst_tx_buf);
typedef oal_bool_enum_uint8 (*ao_is_ipv4_addr)(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_ipv4_addr);
typedef oal_bool_enum_uint8 (*ao_is_ipv6_addr)(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_ipv6_addr);

typedef struct
{
    ao_encap_cb               ao_cb_encap;
    ao_encap_ieee80211_head   ao_encap_ieee80211_head_cb;
    ao_is_ipv4_addr           ao_is_ipv4_addr_cb;
    ao_is_ipv6_addr           ao_is_ipv6_addr_cb;
}dmac_arp_offload_cb;
extern dmac_arp_offload_cb  g_st_dmac_arp_offload_rom_cb;
/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern dmac_rx_frame_ctrl_enum_uint8 dmac_ao_process_arp_and_mcast(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_netbuf);
extern oal_bool_enum_uint8 dmac_ao_is_ipv4_addr_owner(dmac_vap_stru *pst_dmac_vap, oal_uint8 *puc_ipv4_addr);
extern dmac_rx_frame_ctrl_enum_uint8 dmac_ao_sta_process_multicast_filter(dmac_vap_stru * pst_dmac_vap , oal_netbuf_stru *pst_netbuf);
extern dmac_rx_frame_ctrl_enum_uint8 dmac_ao_process_dhcpv6_filter(dmac_vap_stru *pst_dmac_vap , oal_netbuf_stru *pst_netbuf);
extern dmac_rx_frame_ctrl_enum_uint8 dmac_ao_process_dhcp_filter(dmac_vap_stru *pst_dmac_vap, oal_netbuf_stru *pst_netbuf);
extern dmac_rx_frame_ctrl_enum_uint8 dmac_ao_process_nd_offload(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_netbuf, oal_netbuf_stru** ppst_tx_buf);
extern dmac_rx_frame_ctrl_enum_uint8 dmac_ao_process_arp_offload(dmac_vap_stru *pst_dmac_vap, dmac_user_stru *pst_dmac_user, oal_netbuf_stru *pst_netbuf, oal_netbuf_stru** ppst_tx_buf);
oal_bool_enum_uint8 dmac_ao_is_ipv4_broadcast(dmac_vap_stru *pst_dmac_vap, oal_uint32 ul_ipv4_addr);
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_arp_offload.h */
