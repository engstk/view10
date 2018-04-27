


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hal_ext_if.h"
#include "mac_board.h"
#include "dmac_ext_if.h"
#include "dmac_device.h"
#include "dmac_main.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_MAIN_ROM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

oal_uint32  dmac_sdt_recv_reg_cmd(frw_event_mem_stru *pst_event_mem)
{
    oal_uint32                ul_reg_val = 0;
    oal_uint32               *pst_reg_val;
    frw_event_stru           *pst_event_down;
    frw_event_stru           *pst_event_up;
    dmac_vap_stru            *pst_dmac_vap;
    dmac_sdt_reg_frame_stru  *pst_reg_frame;
    frw_event_mem_stru       *pst_event_memory;

    if (OAL_PTR_NULL == pst_event_mem)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_reg_cmd::pst_event_mem null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_event_down     = frw_get_event_stru(pst_event_mem);
    pst_reg_frame = (dmac_sdt_reg_frame_stru *)pst_event_down->auc_event_data;

    pst_dmac_vap  = mac_res_get_dmac_vap(pst_event_down->st_event_hdr.uc_vap_id);
    if (OAL_PTR_NULL == pst_dmac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_reg_cmd::pst_mac_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
    if (pst_reg_frame->uc_mode >= MAC_SDT_MODE_BUTT)
    {
        return OAL_FAIL;
    }
    if (MAC_SDT_MODE_WRITE == pst_reg_frame->uc_mode)
    {

        hal_reg_write(pst_dmac_vap->pst_hal_device, pst_reg_frame->ul_addr, pst_reg_frame->ul_reg_val);
        return OAL_SUCC;
    }
    else if(MAC_SDT_MODE_WRITE16 == pst_reg_frame->uc_mode)
    {
        hal_reg_write16(pst_dmac_vap->pst_hal_device, pst_reg_frame->ul_addr, (oal_uint16)pst_reg_frame->ul_reg_val);
        return OAL_SUCC;
    }
    else if (MAC_SDT_MODE_READ == pst_reg_frame->uc_mode || MAC_SDT_MODE_READ16 == pst_reg_frame->uc_mode)
    {
        if(MAC_SDT_MODE_READ == pst_reg_frame->uc_mode)
        {
            hal_reg_info(pst_dmac_vap->pst_hal_device, pst_reg_frame->ul_addr, &ul_reg_val);
        }
        else if(MAC_SDT_MODE_READ16 == pst_reg_frame->uc_mode)
        {
            hal_reg_info16(pst_dmac_vap->pst_hal_device, pst_reg_frame->ul_addr, (oal_uint16*)(&ul_reg_val));
        }

        /* 将读取到的寄存器值抛事件给hmac */
        pst_event_memory = FRW_EVENT_ALLOC(OAL_SIZEOF(oal_uint32));
        if (OAL_PTR_NULL == pst_event_memory)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_sdt_recv_reg_cmd::pst_event_memory null.}");

            return OAL_ERR_CODE_PTR_NULL;
        }

        pst_event_up = frw_get_event_stru(pst_event_memory);

        /* 填写事件头 */
        FRW_EVENT_HDR_INIT(&(pst_event_up->st_event_hdr),
                       FRW_EVENT_TYPE_HOST_SDT_REG,
                       DMAC_TO_HMAC_SYN_UP_REG_VAL,
                       OAL_SIZEOF(oal_uint32),
                       FRW_EVENT_PIPELINE_STAGE_1,
                       pst_event_down->st_event_hdr.uc_chip_id,
                       pst_event_down->st_event_hdr.uc_device_id,
                       pst_event_down->st_event_hdr.uc_vap_id);

        pst_reg_val = (oal_uint32 *)pst_event_up->auc_event_data;
       *pst_reg_val = ul_reg_val;

        frw_event_dispatch_event(pst_event_memory);
        FRW_EVENT_FREE(pst_event_memory);
    }

    return OAL_SUCC;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_void dmac_timestamp_get(oal_time_us_stru *pst_usec)
{
    oal_uint64  ull_ts_ms = 0;
    ull_ts_ms = OAL_TIME_GET_STAMP_MS();

    pst_usec->i_sec     = (oal_int) (ull_ts_ms / 1000);
    pst_usec->i_usec    = (oal_int) (ull_ts_ms % 1000) * 1000;
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


