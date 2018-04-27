
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
#include "mac_vap.h"
#include "dmac_resource.h"
#include "dmac_device.h"
#include "dmac_tx_bss_comm.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_DEVICE_ROM_C

oal_uint32  dmac_mac_error_overload(mac_device_stru *pst_mac_device, hal_mac_error_type_enum_uint8 en_error_id)
{
    if (pst_mac_device->aul_mac_err_cnt[en_error_id] > MAX_MAC_ERR_IN_TBTT)
    {
        return 1;
    }
    return 0;
}

oal_void  dmac_mac_error_cnt_clr(mac_device_stru *pst_mac_device)
{
    oal_memset(pst_mac_device->aul_mac_err_cnt, 0, sizeof(pst_mac_device->aul_mac_err_cnt));
}
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
