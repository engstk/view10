


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_net.h"
#include "oam_ext_if.h"
#include "hal_ext_if.h"
#include "mac_resource.h"
#include "dmac_dft.h"
#include "dmac_ext_if.h"
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
#include "hal_phy_reg.h"
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#include "oal_mem.h"
#include "frw_event_main.h"
#include "pm_extern.h"
#ifdef _PRE_WLAN_FEATURE_STA_PM
#include "dmac_sta_pm.h"
#endif
#endif
#include "dmac_config.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_DFT_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
/* PHY内置LNA增益与档位的对应表,单位是1dB, 真实值是小数，以下做了乘100处理，第一个值0为无效值 */
OAL_STATIC oal_int32   g_al_intlna_gain[DMAC_DFT_PHY_INTLAN_GAIN_NUM] =
{0, -310, 290, 890, 1490, 2090, 2690, 3290};

/* PHY VGA增益与档位的对应表，单位是1dB, 做乘100处理 */
OAL_STATIC oal_int32   g_al_vga_gain[DMAC_DFT_PHY_VGA_GAIN_NUM] =
{0, 100, 200, 300, 400, 500, 600, 700, 800, 900,
1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900,
2000, 2100, 2200, 2300, 2400, 2500, 2600, -500, -400, -300,
-200, -100};
#endif
/*****************************************************************************
  3 函数实现
*****************************************************************************/
#ifdef _PRE_WLAN_DFT_STAT
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

OAL_STATIC oal_uint32  dmac_dft_get_adc_pin(oal_uint32 ul_adc_pin_code_rpt,
                                                  oal_int32 *pl_adc_pin)
{
    oal_int32       l_adc_pin_phy;       /* 从phy寄存器中读取出来的adc口功率值，此时单位是0.25dB */

    /* ul_adc_pin_code_rpt的低9bit值是ADC口功率，有符号数，单位0.25dB,根据符号位
       来确定真实值
    */
    if (BIT8 != (BIT8 & ul_adc_pin_code_rpt))
    {
        /* 正数，直接转化为单位为1dB的值,然后乘以100保证接下来处理的是整数 */
        *pl_adc_pin = (ul_adc_pin_code_rpt & 0x1FF) * 100 / 4;
    }
    else
    {

        l_adc_pin_phy = (oal_int32)ul_adc_pin_code_rpt;
        /*lint -e701*//*lint -e702*/
        l_adc_pin_phy = (oal_int32)(l_adc_pin_phy << 23);
        l_adc_pin_phy = (oal_int32)(l_adc_pin_phy >> 23);
        /*lint +e701*//*lint +e702*/
        *pl_adc_pin = l_adc_pin_phy * 100 / 4;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_extlna_gain(hal_to_dmac_device_stru *pst_hal_device,
                                                        oal_uint32                ul_adc_pin_code_rpt,
                                                        oal_int32                *pl_extlna_gain)
{
    oal_uint32          ul_extlna_gain0_cfg = 0;
    oal_uint32          ul_extlna_gain1_cfg = 0;
    oal_int32           l_extlna_gain0_cfg;
    oal_int32           l_extlna_gain1_cfg;
    oal_uint8           uc_lna_code_out_3;
    oal_int32           l_extlna_gain;

    /* 读取外置LNA为0和1时增益 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_PHY_EXTLNA_GAIN0_CFG_REG, &ul_extlna_gain0_cfg);
    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_PHY_EXTLNA_GAIN1_CFG_REG, &ul_extlna_gain1_cfg);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_PHY_EXTLNA_GAIN0_CFG_REG, &ul_extlna_gain0_cfg);
    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_PHY_EXTLNA_GAIN1_CFG_REG, &ul_extlna_gain1_cfg);
#endif
    /*lint -e701*//*lint -e702*/
    /* 计算外置LNA为0时的增益值 */
    l_extlna_gain0_cfg = (oal_int32)(ul_extlna_gain0_cfg);
    l_extlna_gain0_cfg = (oal_int32)(l_extlna_gain0_cfg << 22);
    l_extlna_gain0_cfg = (oal_int32)(l_extlna_gain0_cfg >> 22);

    /* 计算外置LNA为1时的增益值 */
    l_extlna_gain1_cfg = (oal_int32)(ul_extlna_gain1_cfg);
    l_extlna_gain1_cfg = (oal_int32)(l_extlna_gain1_cfg << 22);
    l_extlna_gain1_cfg = (oal_int32)(l_extlna_gain1_cfg >> 22);
    /*lint +e701*//*lint +e702*/
    /* 获取外置LNA档位值 */
    uc_lna_code_out_3 = (ul_adc_pin_code_rpt & BIT15) >> 15;

    /* 计算外置LNA增益 */
    l_extlna_gain = (uc_lna_code_out_3 == 0) ? l_extlna_gain0_cfg : l_extlna_gain1_cfg;

    *pl_extlna_gain = l_extlna_gain * 100 / 4;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_intlna_gain(oal_uint32 ul_adc_pin_code_rpt,
                                                      oal_int32 *pl_intlna_gain)
{
    oal_uint8       uc_lna_code_out_0_2;

    /* 获取内置LNA档位值，并计算内置LNA增益 */
    uc_lna_code_out_0_2 = (ul_adc_pin_code_rpt & (BIT12 | BIT13 | BIT14)) >> 12;

    if (0 == uc_lna_code_out_0_2 || DMAC_DFT_PHY_INTLAN_GAIN_NUM <= uc_lna_code_out_0_2)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_dft_get_intlna_gain:: lna_code_out[2:0] val=[%d] invalid.}",
                         uc_lna_code_out_0_2);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    *pl_intlna_gain = g_al_intlna_gain[uc_lna_code_out_0_2];

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_vga_gain(oal_uint32 ul_adc_pin_code_rpt,
                                                   oal_int32 *pl_vga_gain)
{
    oal_uint8       uc_vga_coude_out_0_4;

    /* 获取VGA档位值，并计算VGA增益 */
    uc_vga_coude_out_0_4 = (ul_adc_pin_code_rpt & (BIT16 | BIT17 | BIT18 | BIT19 | BIT20)) >> 16;

    if (DMAC_DFT_PHY_VGA_GAIN_NUM <= uc_vga_coude_out_0_4)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_dft_get_vga_gain:: vga_coude_out[4:0] val=[%d] invalid.}",
                         uc_vga_coude_out_0_4);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    *pl_vga_gain = g_al_vga_gain[uc_vga_coude_out_0_4];

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_reference0(hal_to_dmac_device_stru *pst_hal_device,
                                                       oal_int32               *pl_reference0)
{
    oal_uint32      ul_cfg_power0_ref = 0;
    oal_int32       l_cfg_power0_ref;

    /* 计算接收天线口功率计算参考值 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_PHY_POWER0_REF_REG, &ul_cfg_power0_ref);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_PHY_POWER0_REF_REG, &ul_cfg_power0_ref);
#endif
    /*lint -e701*//*lint -e702*/
    l_cfg_power0_ref = (oal_int32)ul_cfg_power0_ref;
    l_cfg_power0_ref = (oal_int32)(l_cfg_power0_ref << 24);
    l_cfg_power0_ref = (oal_int32)(l_cfg_power0_ref >> 24);
    /*lint +e701*//*lint +e702*/
    *pl_reference0 = l_cfg_power0_ref * 100 / 4;

    return OAL_SUCC;
}


oal_void  dmac_dft_calcu_antenna_power(hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint32          ul_adc_pin_code_rpt;       /* PHY bank2 ADC_PIN_CODE_RPT寄存器的值 */
    oal_int32           l_adc_pin = 0;
    oal_int32           l_extlna_gain = 0;             /* 外置LNA增益 */
    oal_int32           l_intlna_gain = 0;             /* 内置LNA增益 */
    oal_int32           l_vga_gain = 0;                /* VGA增益 */
    oal_int32           l_reference0 = 0;              /* 接收天线口功率计算参考值 */

    /* phy提供的底噪计算公式: adc_pin - (extlna_gain + intlna_gain + vga_gain + reference_0)*/

    /* 读取PHY AGC ADC口功率与输出档位值 */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1102_PHY_ADC_PIN_CODE_RPT_REG, &ul_adc_pin_code_rpt);
#endif
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    hal_reg_info(pst_hal_device, (oal_uint32)HI1103_PHY_ADC_PIN_CODE_RPT_REG, &ul_adc_pin_code_rpt);
#endif

    /* 获取ADC口功率 */
    dmac_dft_get_adc_pin(ul_adc_pin_code_rpt, &l_adc_pin);

    /* 获取外置LNA增益 */
    dmac_dft_get_extlna_gain(pst_hal_device, ul_adc_pin_code_rpt, &l_extlna_gain);

    /* 获取内置LNA增益 */
    dmac_dft_get_intlna_gain(ul_adc_pin_code_rpt, &l_intlna_gain);

    /* 获取VGA增益 */
    dmac_dft_get_vga_gain(ul_adc_pin_code_rpt, &l_vga_gain);

    /* 获取接收天线口功率计算参考值 */
    dmac_dft_get_reference0(pst_hal_device, &l_reference0);

    /* 存到device下，供维测获取 */
    //pst_hal_device->st_dbb_env_param_ctx.s_ant_power = (oal_int16)(l_adc_pin - (l_extlna_gain + l_intlna_gain + l_vga_gain + l_reference0));
}

oal_uint32  dmac_dft_get_antenna_power(mac_device_stru *pst_macdev,
                                                          oal_int32 *pl_antenna_power)
{
    *pl_antenna_power = (oal_int32)pst_macdev->st_dbb_env_param_ctx.s_ant_power;

    return OAL_SUCC;
}

oal_uint32  dmac_dft_get_machw_stat_info(
                                    hal_to_dmac_device_stru   *pst_dev,
                                    oam_stats_machw_stat_stru *pst_machw_stat)
{
#if 0
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dev || OAL_PTR_NULL == pst_machw_stat))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_dft_get_machw_stat_info:: pst_dev = [%d], \
                       pst_machw_stat = [%d]}", pst_dev, pst_machw_stat);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 依次读取MAC 的收发包数目统计寄存器 */
    hal_dft_get_machw_stat_info_ext(pst_dev, pst_machw_stat);
#endif
    return OAL_SUCC;
}

#else

oal_uint32  dmac_dft_set_phy_stat_node(hal_to_dmac_device_stru     *pst_hal_device,
                                                 oam_stats_phy_node_idx_stru *pst_phy_node_idx)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device || OAL_PTR_NULL == pst_phy_node_idx))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_phy_stat_node_init::param is null!}.");
        return OAL_ERR_CODE_PTR_NULL;
    }

    hal_dft_set_phy_stat_node(pst_hal_device, pst_phy_node_idx);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_adc_pin(oal_uint32 ul_adc_pin_code_rpt,
                                                  oal_int32 *pl_adc_pin)
{
    oal_int32       l_adc_pin_phy;       /* 从phy寄存器中读取出来的adc口功率值，此时单位是0.25dB */

    /* ul_adc_pin_code_rpt的低9bit值是ADC口功率，有符号数，单位0.25dB,根据符号位
       来确定真实值
    */
    if (BIT8 != (BIT8 & ul_adc_pin_code_rpt))
    {
        /* 正数，直接转化为单位为1dB的值,然后乘以100保证接下来处理的是整数 */
        *pl_adc_pin = (ul_adc_pin_code_rpt & 0x1FF) * 100 / 4;
    }
    else
    {
         /*lint -e701*//*lint -e702*/ /* 注释掉有符号数移位错误 */
        l_adc_pin_phy = (oal_int32)ul_adc_pin_code_rpt;
        l_adc_pin_phy = (oal_int32)(l_adc_pin_phy << 23);
        l_adc_pin_phy = (oal_int32)(l_adc_pin_phy >> 23);
        /*lint +e701*//*lint +e702*/

        *pl_adc_pin = l_adc_pin_phy * 100 / 4;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_extlna_gain(hal_to_dmac_device_stru *pst_hal_device,
                                                         oal_uint32               ul_adc_pin_code_rpt,
                                                         oal_int32               *pl_extlna_gain)
{
    oal_uint32          ul_extlna_gain0_cfg = 0;
    oal_uint32          ul_extlna_gain1_cfg = 0;
    oal_int32           l_extlna_gain0_cfg;
    oal_int32           l_extlna_gain1_cfg;
    oal_uint8           uc_lna_code_out_3;
    oal_int32           l_extlna_gain;

    /* 读取外置LNA为0和1时增益 */
    hal_dft_get_extlna_gain(pst_hal_device, &ul_extlna_gain0_cfg, &ul_extlna_gain1_cfg);

    /*lint -e701*//*lint -e702*/
    /* 计算外置LNA为0时的增益值 */
    l_extlna_gain0_cfg = (oal_int32)(ul_extlna_gain0_cfg);
    l_extlna_gain0_cfg = (oal_int32)(l_extlna_gain0_cfg << 22);
    l_extlna_gain0_cfg = (oal_int32)(l_extlna_gain0_cfg >> 22);

    /* 计算外置LNA为1时的增益值 */
    l_extlna_gain1_cfg = (oal_int32)(ul_extlna_gain1_cfg);
    l_extlna_gain1_cfg = (oal_int32)(l_extlna_gain1_cfg << 22);
    l_extlna_gain1_cfg = (oal_int32)(l_extlna_gain1_cfg >> 22);
    /*lint +e701*//*lint +e702*/

    /* 获取外置LNA档位值 */
    uc_lna_code_out_3 = (ul_adc_pin_code_rpt & BIT15) >> 15;

    /* 计算外置LNA增益 */
    l_extlna_gain = (uc_lna_code_out_3 == 0) ? l_extlna_gain0_cfg : l_extlna_gain1_cfg;

    *pl_extlna_gain = l_extlna_gain * 100 / 4;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_intlna_gain(oal_uint32 ul_adc_pin_code_rpt,
                                                      oal_int32 *pl_intlna_gain)
{
    oal_uint8       uc_lna_code_out_0_2;

    /* 获取内置LNA档位值，并计算内置LNA增益 */
    uc_lna_code_out_0_2 = (ul_adc_pin_code_rpt & (BIT12 | BIT13 | BIT14)) >> 12;

    if (0 == uc_lna_code_out_0_2 || DMAC_DFT_PHY_INTLAN_GAIN_NUM <= uc_lna_code_out_0_2)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_dft_get_intlna_gain:: lna_code_out[2:0] val=[%d] invalid.}",
                         uc_lna_code_out_0_2);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    *pl_intlna_gain = g_al_intlna_gain[uc_lna_code_out_0_2];

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_vga_gain(oal_uint32 ul_adc_pin_code_rpt,
                                                   oal_int32 *pl_vga_gain)
{
    oal_uint8       uc_vga_coude_out_0_4;

    /* 获取VGA档位值，并计算VGA增益 */
    uc_vga_coude_out_0_4 = (ul_adc_pin_code_rpt & (BIT16 | BIT17 | BIT18 | BIT19 | BIT20)) >> 16;

    if (DMAC_DFT_PHY_VGA_GAIN_NUM <= uc_vga_coude_out_0_4)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{dmac_dft_get_vga_gain:: vga_coude_out[4:0] val=[%d] invalid.}",
                         uc_vga_coude_out_0_4);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    *pl_vga_gain = g_al_vga_gain[uc_vga_coude_out_0_4];

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_reference0(hal_to_dmac_device_stru *pst_hal_device,
                                                       oal_int32                *pl_reference0)
{
    oal_uint32      ul_cfg_power0_ref = 0;
    oal_int32       l_cfg_power0_ref;

    /* 计算接收天线口功率计算参考值 */
    hal_dft_get_power0_ref(pst_hal_device, &ul_cfg_power0_ref);

    /*lint -e701*//*lint -e702*/
    l_cfg_power0_ref = (oal_int32)ul_cfg_power0_ref;
    l_cfg_power0_ref = (oal_int32)(l_cfg_power0_ref << 24);
    l_cfg_power0_ref = (oal_int32)(l_cfg_power0_ref >> 24);
    /*lint +e701*//*lint +e702*/

    *pl_reference0 = l_cfg_power0_ref * 100 / 4;

    return OAL_SUCC;
}


oal_void  dmac_dft_calcu_antenna_power(hal_to_dmac_device_stru *pst_hal_device)
{
    oal_uint32          ul_adc_pin_code_rpt;       /* PHY bank2 ADC_PIN_CODE_RPT寄存器的值 */
    oal_int32           l_adc_pin = 0;
    oal_int32           l_extlna_gain = 0;             /* 外置LNA增益 */
    oal_int32           l_intlna_gain = 0;             /* 内置LNA增益 */
    oal_int32           l_vga_gain = 0;                /* VGA增益 */
    oal_int32           l_reference0 = 0;              /* 接收天线口功率计算参考值 */

    /* phy提供的底噪计算公式: adc_pin - (extlna_gain + intlna_gain + vga_gain + reference_0)*/

    /* 读取PHY AGC ADC口功率与输出档位值 */
    hal_dft_get_phy_pin_code_rpt(pst_hal_device, &ul_adc_pin_code_rpt);

    /* 获取ADC口功率 */
    dmac_dft_get_adc_pin(ul_adc_pin_code_rpt, &l_adc_pin);

    /* 获取外置LNA增益 */
    dmac_dft_get_extlna_gain(pst_hal_device, ul_adc_pin_code_rpt, &l_extlna_gain);

    /* 获取内置LNA增益 */
    dmac_dft_get_intlna_gain(ul_adc_pin_code_rpt, &l_intlna_gain);

    /* 获取VGA增益 */
    dmac_dft_get_vga_gain(ul_adc_pin_code_rpt, &l_vga_gain);

    /* 获取接收天线口功率计算参考值 */
    dmac_dft_get_reference0(pst_hal_device, &l_reference0);

    /* 存到device下，供维测获取 */
    //pst_macdev->st_dbb_env_param_ctx.s_ant_power = (oal_int16)(l_adc_pin - (l_extlna_gain + l_intlna_gain + l_vga_gain + l_reference0));
}



oal_uint32  dmac_dft_get_antenna_power(mac_device_stru *pst_macdev,
                                                          oal_int32 *pl_antenna_power)
{
    *pl_antenna_power = (oal_int32)pst_macdev->st_dbb_env_param_ctx.s_ant_power;

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_get_chan_stat_result(hal_to_dmac_device_stru       *pst_hal_device,
                                                               oam_stats_dbb_env_param_stru  *pst_dbb_env_param)
{
    hal_dft_get_chan_stat_result(pst_hal_device, pst_dbb_env_param);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  dmac_dft_report_dbb_env_param(mac_device_stru *pst_macdev, hal_to_dmac_device_stru *pst_hal_device)
{
    oam_stats_dbb_env_param_stru        st_dbb_env_param;

    OAL_MEMZERO(&st_dbb_env_param, OAL_SIZEOF(oam_stats_dbb_env_param_stru));

    /* 获取接收到非本机地址的帧个数，单位是: 个/s */
    st_dbb_env_param.ul_non_directed_frame_num = (pst_macdev->st_dbb_env_param_ctx.ul_non_directed_frame_num
                                               * DMAC_DFT_REPORT_TO_COLLECT_TIMES) >> 1;
    /* 清零超时次数和非本机地址帧个数 */
    pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt = 0;
    pst_macdev->st_dbb_env_param_ctx.ul_non_directed_frame_num = 0;

    /* 获取每一个ap beacon miss最大次数 */
    hal_dft_get_beacon_miss_stat_info(pst_hal_device, &st_dbb_env_param);

    /* 获取MAC PHY测量信道结果 */
    dmac_dft_get_chan_stat_result(pst_hal_device, &st_dbb_env_param);

    /* 上报参数 */
    return oam_report_dft_params(BROADCAST_MACADDR, (oal_uint8 *)&st_dbb_env_param,(oal_uint16)OAL_SIZEOF(oam_stats_dbb_env_param_stru), OAM_OTA_TYPE_DBB_ENV_PARAM);

}



OAL_STATIC oal_uint32  dmac_dft_collect_dbb_env_param_timeout(oal_void *p_arg)
{
    mac_vap_stru            *pst_mac_vap;
    mac_device_stru         *pst_macdev;
    hal_to_dmac_device_stru *pst_hal_device;

    pst_mac_vap = (mac_vap_stru *)p_arg;

    pst_macdev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_macdev))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_collect_dbb_env_param_timeout::device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_hal_device = DMAC_VAP_GET_HAL_DEVICE(pst_mac_vap);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_hal_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_collect_dbb_env_param_timeout::pst_hal_device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 增加超时次数的计数 */
    pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt++;

    /* 如果本次是第98次超时，通知DBB MAC开始统计CCA空闲率，DBB PHY开始统计信道空闲功率 */
    if (DMAC_DFT_REPORT_TO_COLLECT_TIMES - 2 == pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt)
    {
        /* 配置DBB MAC统计周期,20ms */
        hal_set_ch_statics_period(pst_hal_device, DMAC_DFT_MAC_CHAN_STAT_PERIOD);

        /* 配置DBB PHY统计周期 */
        hal_set_ch_measurement_period(pst_hal_device, DMAC_DFT_PHY_CHAN_MEASUREMENT_PERIOD);

        /* 使能DBB MAC和DBB PHY开始统计 */
        hal_enable_ch_statics(pst_hal_device, 1);

        return OAL_SUCC;
    }
    /* 如果本次是第99次超时，开始收集一个周期(20ms)数据(接收到非本机地址的帧个数) */
    else if (DMAC_DFT_REPORT_TO_COLLECT_TIMES - 1 == pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt)
    {
        /* 配置mac不过滤，持续20ms */
        hal_dft_enable_mac_filter(pst_hal_device, 0);

        return OAL_SUCC;
    }
    else if (DMAC_DFT_REPORT_TO_COLLECT_TIMES == pst_macdev->st_dbb_env_param_ctx.uc_collect_period_cnt)
    {
        /* 配置mac过滤 */
        hal_dft_enable_mac_filter(pst_hal_device, 1);

        return dmac_dft_report_dbb_env_param(pst_macdev, pst_hal_device);
    }
    else
    {
        return OAL_SUCC;
    }
}



oal_uint32  dmac_dft_start_report_dbb_env(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_device;

    pst_device = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_start_report_param::device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 启动采集维测参数定时器，定时器超时100次的时候上报维测参数 */
    FRW_TIMER_CREATE_TIMER(&pst_device->st_dbb_env_param_ctx.st_collect_period_timer,
                           dmac_dft_collect_dbb_env_param_timeout,
                           DMAC_DFT_COLLECT_DBB_ENV_PARAM_TIMEOUT,
                           (oal_void *)pst_mac_vap,
                           OAL_TRUE,
                           OAM_MODULE_ID_DMAC,
                           pst_device->ul_core_id);

    pst_device->st_dbb_env_param_ctx.en_non_directed_frame_stat_flg = OAL_TRUE;

    return OAL_SUCC;
}


oal_uint32  dmac_dft_stop_report_dbb_env(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_device;

    pst_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_stop_report_param::device is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 删除定时器 */
    if (OAL_TRUE == pst_device->st_dbb_env_param_ctx.st_collect_period_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_device->st_dbb_env_param_ctx.st_collect_period_timer);
    }

    /* 清零计数器 */
    pst_device->st_dbb_env_param_ctx.uc_collect_period_cnt = 0;
    pst_device->st_dbb_env_param_ctx.s_ant_power           = 0;
    pst_device->st_dbb_env_param_ctx.en_non_directed_frame_stat_flg = 0;
    pst_device->st_dbb_env_param_ctx.ul_non_directed_frame_num = 0;

    return OAL_SUCC;
}


oal_uint32  dmac_dft_clear_usr_queue_stat(dmac_user_stru  *pst_dmac_user)
{
    oal_uint8    uc_tid_no;

    /* 清零节能队列统计信息 */
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user->st_ps_structure.pst_psm_stats
        || OAL_PTR_NULL == pst_dmac_user->st_uapsd_stru.pst_uapsd_statis))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_dft_clear_usr_queue_stat::ps stats is null! psm=[%d],uapsd=[%d].}",
                       (oal_uint32)pst_dmac_user->st_ps_structure.pst_psm_stats,
                       (oal_uint32)pst_dmac_user->st_uapsd_stru.pst_uapsd_statis);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(pst_dmac_user->st_ps_structure.pst_psm_stats, OAL_SIZEOF(dmac_user_psm_stats_stru));
    OAL_MEMZERO(pst_dmac_user->st_uapsd_stru.pst_uapsd_statis, OAL_SIZEOF(dmac_usr_uapsd_statis_stru));

    /* 清零tid队列统计信息 */
    for (uc_tid_no = 0; uc_tid_no < WLAN_TID_MAX_NUM; uc_tid_no++)
    {
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user->ast_tx_tid_queue[uc_tid_no].pst_tid_stats))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_dft_clear_usr_queue_stat::pst_tid_stats is null.tid_no=[%d].}",
                           uc_tid_no);
            return OAL_ERR_CODE_PTR_NULL;
        }

        OAL_MEMZERO(pst_dmac_user->ast_tx_tid_queue[uc_tid_no].pst_tid_stats, OAL_SIZEOF(dmac_tid_stats_stru));
    }

    return OAL_SUCC;
}


oal_uint32  dmac_dft_report_usr_queue_stat(dmac_user_stru  *pst_dmac_user)
{
    oam_stats_usr_queue_stat_stru       st_usr_queue_stats;
    oal_uint8                           uc_tid_no;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user->st_ps_structure.pst_psm_stats
        || OAL_PTR_NULL == pst_dmac_user->st_uapsd_stru.pst_uapsd_statis))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_dft_report_usr_queue_stat::ps stats is null! psm=[%d],uapsd=[%d].}",
                       (oal_uint32)pst_dmac_user->st_ps_structure.pst_psm_stats,
                       (oal_uint32)pst_dmac_user->st_uapsd_stru.pst_uapsd_statis);
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(&st_usr_queue_stats, OAL_SIZEOF(oam_stats_usr_queue_stat_stru));

    /* 获取用户uapsd队列下的统计信息 */
    oal_memcopy(st_usr_queue_stats.aul_uapsd_stat,
                pst_dmac_user->st_uapsd_stru.pst_uapsd_statis,
                OAL_SIZEOF(oal_uint32) * OAM_UAPSD_STAT_CNT);

    /* 获取用户psm队列下的统计信息 */
    oal_memcopy(st_usr_queue_stats.aul_psm_stat,
                pst_dmac_user->st_ps_structure.pst_psm_stats,
                OAL_SIZEOF(oal_uint32) * OAM_PSM_STAT_CNT);

    /* 获取用户tid队列下的统计信息 */
    for (uc_tid_no = 0; uc_tid_no < WLAN_TID_MAX_NUM; uc_tid_no++)
    {
        if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user->ast_tx_tid_queue[uc_tid_no].pst_tid_stats))
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_dft_report_usr_queue_stat::tid_stats is null! tid_no=[%d].}", uc_tid_no);
            return OAL_ERR_CODE_PTR_NULL;
        }
        oal_memcopy(&st_usr_queue_stats.aul_tid_stat[uc_tid_no][0],
                    pst_dmac_user->ast_tx_tid_queue[uc_tid_no].pst_tid_stats,
                    OAL_SIZEOF(oal_uint32) * OAM_TID_STAT_CNT);
    }

    return oam_report_dft_params(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                                 (oal_uint8 *)&st_usr_queue_stats,
                                 (oal_uint16)OAL_SIZEOF(oam_stats_usr_queue_stat_stru),
                                 OAM_OTA_TYPE_USR_QUEUE_STAT);
}


oal_uint32  dmac_dft_mgmt_stat_incr(
                                    mac_device_stru   *pst_mac_dev,
                                    oal_uint8   *puc_mac_hdr_addr,
                                    mac_dev_mgmt_stat_type_enum_uint8 en_type)
{
    oal_uint8           uc_subtype;

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_mac_hdr_addr || OAL_PTR_NULL == pst_mac_dev))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_dft_mgmt_stat_incr::param is null. puc_mac_hdr_addr = [%d],\
                       mac_dev = [%d]}", puc_mac_hdr_addr, pst_mac_dev);
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (MAC_DEV_MGMT_STAT_TYPE_BUTT <= en_type)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_dft_mgmt_stat_incr::en_type exceeds! en_type = [%d].}", en_type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (WLAN_MANAGEMENT == mac_frame_get_type_value(puc_mac_hdr_addr))
    {
        uc_subtype = mac_frame_get_subtype_value(puc_mac_hdr_addr);
        if (uc_subtype >= WLAN_MGMT_SUBTYPE_BUTT)
        {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{dmac_dft_mgmt_stat_incr::uc_subtype exceeds! uc_subtype = [%d].}",
                           uc_subtype);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        switch (en_type)
        {
            case MAC_DEV_MGMT_STAT_TYPE_TX:
                DMAC_DFT_MGMT_STAT_INCR(pst_mac_dev->st_mgmt_stat.aul_tx_mgmt_soft[uc_subtype]);
            break;

            case MAC_DEV_MGMT_STAT_TYPE_RX:
                DMAC_DFT_MGMT_STAT_INCR(pst_mac_dev->st_mgmt_stat.aul_rx_mgmt[uc_subtype]);
            break;

            case MAC_DEV_MGMT_STAT_TYPE_TX_COMPLETE:
                DMAC_DFT_MGMT_STAT_INCR(pst_mac_dev->st_mgmt_stat.aul_tx_mgmt_hardware[uc_subtype]);
            break;

            default:
            break;
        }
    }

    return OAL_SUCC;
}

oal_uint32  dmac_dft_get_machw_stat_info(
                                    hal_to_dmac_device_stru   *pst_dev,
                                    oam_stats_machw_stat_stru *pst_machw_stat)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dev || OAL_PTR_NULL == pst_machw_stat))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{dmac_dft_get_machw_stat_info:: pst_dev = [%d], \
                       pst_machw_stat = [%d]}", pst_dev, pst_machw_stat);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 依次读取MAC 的收发包数目统计寄存器 */
    hal_dft_get_machw_stat_info_ext(pst_dev, pst_machw_stat);

    return OAL_SUCC;
}


oal_void  dmac_dft_report_all_ota_state(mac_vap_stru *pst_mac_sta)
{
}

oal_void dmac_dft_report_all_para(dmac_vap_stru *pst_dmac_sta,oal_uint8 uc_ota_switch)
{
    /* TBD */
}

#endif
#if 0

OAL_STATIC oal_uint32  dmac_dft_report_vap_stat(oal_void *p_arg)
{
    mac_vap_stru            *pst_mac_vap;
    oam_stats_vap_stat_stru  st_vap_dft_stats;
    oal_uint8                auc_user_macaddr[WLAN_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    pst_mac_vap = (mac_vap_stru  *)p_arg;

    OAL_MEMZERO(&st_vap_dft_stats, OAL_SIZEOF(oam_stats_vap_stat_stru));

    /* 先停止统计 */
    pst_mac_vap->st_vap_dft.ul_flg = OAL_FALSE;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap->st_vap_dft.pst_vap_dft_stats))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_report_vap_stat:: vap_dft_stats is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_memcopy(&st_vap_dft_stats,
                pst_mac_vap->st_vap_dft.pst_vap_dft_stats,
                OAL_SIZEOF(mac_vap_dft_stats_stru));
    OAL_MEMZERO(pst_mac_vap->st_vap_dft.pst_vap_dft_stats, OAL_SIZEOF(mac_vap_dft_stats_stru));

    /* 计算5个平均速率,单位是 kbps，上报周期是2s */
    st_vap_dft_stats.ul_lan_tx_rate = st_vap_dft_stats.ul_lan_tx_bytes / DMAC_DFT_VAP_STAT_RATE_TO_KBPS;
    st_vap_dft_stats.ul_drv_tx_rate = st_vap_dft_stats.ul_drv_tx_bytes / DMAC_DFT_VAP_STAT_RATE_TO_KBPS;
    st_vap_dft_stats.ul_hw_tx_rate  = st_vap_dft_stats.ul_hw_tx_bytes  / DMAC_DFT_VAP_STAT_RATE_TO_KBPS;
    st_vap_dft_stats.ul_drv_rx_rate = st_vap_dft_stats.ul_drv_rx_bytes / DMAC_DFT_VAP_STAT_RATE_TO_KBPS;
    st_vap_dft_stats.ul_lan_rx_rate = st_vap_dft_stats.ul_lan_rx_bytes / DMAC_DFT_VAP_STAT_RATE_TO_KBPS;

    pst_mac_vap->st_vap_dft.ul_flg = OAL_TRUE;

    return oam_report_dft_params(auc_user_macaddr, (oal_uint8 *)&st_vap_dft_stats, OAM_OTA_TYPE_VAP_STAT);
}



oal_uint32  dmac_dft_start_report_vap_stat(mac_vap_stru *pst_mac_vap)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_start_report_vap_stat::vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 置标志位，业务流程开始统计 */
    pst_mac_vap->st_vap_dft.ul_flg = 1;

    /* 申请内存 */
    pst_mac_vap->st_vap_dft.pst_vap_dft_stats = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL,
                                                              OAL_SIZEOF(mac_vap_dft_stats_stru),
                                                              OAL_TRUE);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap->st_vap_dft.pst_vap_dft_stats))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_start_report_vap_stat::alloc fail.}");
        pst_mac_vap->st_vap_dft.ul_flg = 0;

        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    OAL_MEMZERO(pst_mac_vap->st_vap_dft.pst_vap_dft_stats, OAL_SIZEOF(mac_vap_dft_stats_stru));

    /* 启动周期上报定时器 */
    FRW_TIMER_CREATE_TIMER(&pst_mac_vap->st_vap_dft.st_vap_dft_timer,
                           dmac_dft_report_vap_stat,
                           DMAC_DFT_REPORT_VAP_STAT_TIMEOUT,
                           (oal_void *)pst_mac_vap,
                           OAL_TRUE,
                           OAM_MODULE_ID_DMAC,
                           pst_mac_vap->ul_core_id);

    return OAL_SUCC;
}


oal_uint32  dmac_dft_stop_report_vap_stat(mac_vap_stru *pst_mac_vap)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_start_report_vap_stat::vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    return mac_vap_dft_stat_clear(pst_mac_vap);
}
#endif
#endif
#ifdef  _PRE_DEBUG_MODE

oal_uint32  dmac_dft_report_all_ampdu_stat(dmac_user_stru  *pst_dmac_user,
                                                   oal_uint8        uc_param)
{
    oal_uint8                  uc_tid_no;
    dmac_tid_stru             *pst_tid;
    oam_stats_ampdu_stat_stru  st_ampdu_stat;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dmac_user))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_report_all_ampdu_stat::dmac_user is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_TRUE == uc_param)
    {
        for (uc_tid_no = 0; uc_tid_no < WLAN_TID_MAX_NUM; uc_tid_no++)
        {
            pst_tid = &pst_dmac_user->ast_tx_tid_queue[uc_tid_no];

            if (OAL_LIKELY(OAL_PTR_NULL != pst_tid->pst_tid_ampdu_stat))
            {
                OAL_MEMZERO(&st_ampdu_stat, OAL_SIZEOF(oam_stats_ampdu_stat_stru));
                oal_memcopy(&st_ampdu_stat, pst_tid->pst_tid_ampdu_stat, OAL_SIZEOF(oam_stats_ampdu_stat_stru));

                oam_report_dft_params(pst_dmac_user->st_user_base_info.auc_user_mac_addr,
                                      (oal_uint8 *)&st_ampdu_stat,
                                      (oal_uint16)OAL_SIZEOF(oam_stats_ampdu_stat_stru),
                                      OAM_OTA_TYPE_AMPDU_STAT);
            }

        }
    }
    else
    {
        for (uc_tid_no = 0; uc_tid_no < WLAN_TID_MAX_NUM; uc_tid_no++)
        {
            pst_tid = &pst_dmac_user->ast_tx_tid_queue[uc_tid_no];

            if (OAL_LIKELY(OAL_PTR_NULL != pst_tid->pst_tid_ampdu_stat))
            {
                OAL_MEMZERO(pst_tid->pst_tid_ampdu_stat, OAL_SIZEOF(oam_stats_ampdu_stat_stru));
            }

        }
    }

    return OAL_SUCC;
}


oal_uint32  dmac_dft_report_ampdu_stat(dmac_tid_stru  *pst_tid,
                                               oal_uint8      auc_macaddr[],
                                               oal_uint8      uc_param)
{
    oam_stats_ampdu_stat_stru  st_ampdu_stat;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_tid))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_dft_report_ampdu_stat::pst_tid is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_TRUE == uc_param)
    {
        if (OAL_LIKELY(OAL_PTR_NULL != pst_tid->pst_tid_ampdu_stat))
        {
            OAL_MEMZERO(&st_ampdu_stat, OAL_SIZEOF(oam_stats_ampdu_stat_stru));
            oal_memcopy(&st_ampdu_stat, pst_tid->pst_tid_ampdu_stat, OAL_SIZEOF(oam_stats_ampdu_stat_stru));

            oam_report_dft_params(auc_macaddr,
                                  (oal_uint8 *)&st_ampdu_stat,
                                  (oal_uint16)OAL_SIZEOF(oam_stats_ampdu_stat_stru),
                                  OAM_OTA_TYPE_AMPDU_STAT);
        }


    }
    else
    {
        if (OAL_LIKELY(OAL_PTR_NULL != pst_tid->pst_tid_ampdu_stat))
        {
            OAL_MEMZERO(pst_tid->pst_tid_ampdu_stat, OAL_SIZEOF(oam_stats_ampdu_stat_stru));
        }
    }

    return OAL_SUCC;
}

#endif








#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

