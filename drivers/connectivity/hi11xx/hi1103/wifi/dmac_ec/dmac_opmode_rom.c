


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_opmode.h"
#include "dmac_vap.h"
#include "oal_types.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "dmac_tx_bss_comm.h"
#include "dmac_config.h"
#include "dmac_psm_ap.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_OPMODE_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
dmac_opmode_cb g_st_dmac_opmode_rom_cb = {OAL_PTR_NULL};


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_void dmac_mgmt_encap_opmode_notify_action(mac_vap_stru *pst_mac_vap,
    oal_netbuf_stru *pst_netbuf, oal_uint16 *puc_len, oal_bool_enum_uint8 en_bool, wlan_nss_enum_uint8 en_nss)
{
    oal_uint16                  us_index = 0;
    mac_ieee80211_frame_stru   *pst_mac_header = (mac_ieee80211_frame_stru *)OAL_NETBUF_HEADER(pst_netbuf);
    mac_opmode_notify_action_mgt_stru   *puc_payload_addr = (mac_opmode_notify_action_mgt_stru *)oal_netbuf_data(pst_netbuf);

    *puc_len = 0;

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
    mac_hdr_set_frame_control((oal_uint8 *)pst_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /*  power management bit is never sent by an AP */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        pst_mac_header->st_frame_control.bit_power_mgmt = en_bool;
    }

    /* 设置分片序号为0 */
    mac_hdr_set_fragment_number((oal_uint8 *)pst_mac_header, 0);

    /* 设置 address1(接收端): AP MAC地址 (BSSID)*/
    oal_set_mac_addr((oal_uint8 *)pst_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_mac_vap->auc_bssid);

    /* 设置 address2(发送端): dot11StationID */
    oal_set_mac_addr((oal_uint8 *)pst_mac_header + WLAN_HDR_ADDR2_OFFSET, pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID);

    /* 设置 address3: AP MAC地址 (BSSID) */
    oal_set_mac_addr((oal_uint8 *)pst_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    /*************************************************************************/
    /*                  operating mode notify Management frame - Frame Body  */
    /* ----------------------------------------------------------------------*/
    /* |Category |VHT Action |Operating mode field|                          */
    /* ----------------------------------------------------------------------*/
    /* |1        |1          |1                   |                          */
    /* ----------------------------------------------------------------------*/
    /*                                                                       */
    /*************************************************************************/
    us_index = MAC_80211_FRAME_LEN;

    puc_payload_addr->category = MAC_ACTION_CATEGORY_VHT;            /* VHT Category */
    puc_payload_addr->action = MAC_VHT_ACTION_OPREATING_MODE_NOTIFICATION;               /* VHT Action */

    us_index += MAC_IE_HDR_LEN;

    mac_set_opmode_field((oal_uint8 *)pst_mac_vap, &(puc_payload_addr->opmode_ctl), en_nss);

    us_index += MAC_OPMODE_NOTIFY_LEN;

    *puc_len = us_index;

    if (OAL_PTR_NULL != g_st_dmac_opmode_rom_cb.encap_opmode_action_cb)
    {
        g_st_dmac_opmode_rom_cb.encap_opmode_action_cb(pst_mac_vap, pst_netbuf, puc_len, en_nss);
    }
}
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

