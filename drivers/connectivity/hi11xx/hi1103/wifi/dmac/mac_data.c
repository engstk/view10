


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_mem.h"
#include "oal_net.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "mac_data.h"
#include "dmac_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DATA_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint8 mac_get_data_type_from_80211(oal_netbuf_stru *pst_netbuff, oal_uint16 us_mac_hdr_len)
{
    oal_uint8               uc_datatype = MAC_DATA_BUTT;
    mac_llc_snap_stru      *pst_snap;

    if(OAL_PTR_NULL == pst_netbuff)
    {
        return MAC_DATA_BUTT;
    }

    pst_snap = (mac_llc_snap_stru *)(OAL_NETBUF_DATA(pst_netbuff) + us_mac_hdr_len);

    uc_datatype = mac_get_data_type_from_8023((oal_uint8 *)pst_snap, MAC_NETBUFF_PAYLOAD_SNAP);

    return uc_datatype;

}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

oal_uint16 mac_rx_get_eapol_keyinfo_51(oal_netbuf_stru *pst_netbuff)
{
    oal_uint8                      uc_datatype = MAC_DATA_BUTT;
    oal_uint8                     *puc_payload;
    mac_rx_ctl_stru               *pst_rx_info;
    dmac_rx_ctl_stru              *pst_cb_ctrl;

    pst_cb_ctrl = (dmac_rx_ctl_stru*)oal_netbuf_cb(pst_netbuff);

    pst_rx_info = &(pst_cb_ctrl->st_rx_info);

    uc_datatype = mac_get_data_type_from_80211(pst_netbuff, pst_rx_info->uc_mac_header_len);
    if(MAC_DATA_EAPOL != uc_datatype)
    {
        return 0;
    }

    puc_payload = MAC_GET_RX_PAYLOAD_ADDR(pst_rx_info, pst_netbuff);
    if(OAL_PTR_NULL == puc_payload)
    {
        return 0;
    }

    return *(oal_uint16 *)(puc_payload + OAL_EAPOL_INFO_POS);

}
#endif

oal_bool_enum_uint8 mac_is_eapol_key_ptk(mac_eapol_header_stru  *pst_eapol_header)
{
    mac_eapol_key_stru *pst_key;

    if (IEEE802_1X_TYPE_EAPOL_KEY == pst_eapol_header->uc_type)
    {
        if ((oal_uint16)(OAL_NET2HOST_SHORT(pst_eapol_header->us_length)) >= (oal_uint16)OAL_SIZEOF(mac_eapol_key_stru))
        {
            pst_key = (mac_eapol_key_stru *)(pst_eapol_header + 1);

            if (pst_key->auc_key_info[1] & WPA_KEY_INFO_KEY_TYPE)
            {
                return OAL_TRUE;
            }
        }
    }
    return OAL_FALSE;
}


oal_bool_enum_uint8 mac_is_eapol_key_ptk_4_4(oal_netbuf_stru *pst_netbuff)
{
    mac_eapol_header_stru   *pst_eapol_header;
    mac_eapol_key_stru      *pst_eapol_key;

    if ((mac_get_data_type(pst_netbuff) == MAC_DATA_EAPOL))
    {
        pst_eapol_header = (mac_eapol_header_stru *)(oal_netbuf_payload(pst_netbuff) + OAL_SIZEOF(mac_llc_snap_stru));
        if (mac_is_eapol_key_ptk(pst_eapol_header) == OAL_TRUE)
        {
            pst_eapol_key = (mac_eapol_key_stru *)(pst_eapol_header + 1);
            if (pst_eapol_key->auc_key_data_length[0] == 0
                && pst_eapol_key->auc_key_data_length[1] == 0)
            {
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}


/*lint -e19*/
oal_module_symbol(mac_get_data_type_from_80211);
oal_module_symbol(mac_is_eapol_key_ptk);

/*lint +e19*/




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


