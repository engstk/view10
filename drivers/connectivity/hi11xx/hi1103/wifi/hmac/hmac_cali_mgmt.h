

#ifndef __HMAC_CALI_MGMT_H__
#define __HMAC_CALI_MGMT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "frw_ext_if.h"
#include "dmac_ext_if.h"
#include "hmac_vap.h"
#include "plat_cali.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_MGMT_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define DPD_CORRAM_DATA_NUM (512)
#define DPD_CORRAM_DATA_LEN (1024)

#define HI1103_CALI_SISO_TXDC_GAIN_LVL_NUM (8)       /* SISO tx dc补偿值档位数目 */
#define HI1103_CALI_MIMO_TXDC_GAIN_LVL_NUM (8)       /* MIMO tx dc补偿值档位数目 */
#define HI1103_CALI_TXDC_GAIN_LVL_NUM      (HI1103_CALI_SISO_TXDC_GAIN_LVL_NUM + HI1103_CALI_MIMO_TXDC_GAIN_LVL_NUM)
#define HI1103_CALI_IQ_TONE_NUM            (8)
#define HI1103_CALI_RXDC_GAIN_LVL_NUM      (8)      /* rx dc补偿值档位数目 */
#define HI1103_2G_CHANNEL_NUM              (13)
#define HI1103_5G_20M_CHANNEL_NUM          (7)
#define HI1103_5G_80M_CHANNEL_NUM          (7)
#define HI1103_5G_CHANNEL_NUM              (HI1103_5G_20M_CHANNEL_NUM + HI1103_5G_80M_CHANNEL_NUM)
#define HI1103_CALI_RXIQ_LS_FILTER_TAP_NUM (17)
#define HI1103_CALI_TXIQ_LS_FILTER_TAP_NUM (15)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
/* 校准数据结构 */
typedef struct
{
    oal_uint16  aus_analog_rxdc_siso_cmp[HI1103_CALI_RXDC_GAIN_LVL_NUM];
    oal_uint16  aus_analog_rxdc_mimo_extlna_cmp[HI1103_CALI_RXDC_GAIN_LVL_NUM];
    oal_uint16  aus_analog_rxdc_mimo_intlna_cmp[HI1103_CALI_RXDC_GAIN_LVL_NUM];
    oal_uint16  us_digital_rxdc_cmp_i;
    oal_uint16  us_digital_rxdc_cmp_q;
}hi1103_rx_dc_comp_val_stru;

typedef struct
{
    oal_uint8   uc_ppa_cmp;
    oal_uint8   uc_atx_pwr_cmp;
    oal_uint8   uc_dtx_pwr_cmp;
    oal_int8    c_dp_init;
    oal_uint8   auc_reserve[2];
    oal_int16   s_2g_tx_power_dc;
}hi1103_2G_tx_power_comp_val_stru;

typedef struct
{
    oal_uint8   uc_ppa_cmp;
    oal_uint8   uc_mx_cmp;
    oal_uint8   uc_atx_pwr_cmp;
    oal_uint8   uc_dtx_pwr_cmp;
    oal_int16   s_5g_tx_power_dc;
    oal_uint8   auc_reserve[2];
}hi1103_5G_tx_power_comp_val_stru;

typedef struct
{
    oal_uint8   uc_ssb_cmp;
    oal_uint8   uc_buf0_cmp;
    oal_uint8   uc_buf1_cmp;
    oal_uint8   uc_resv;
}hi1103_logen_comp_val_stru;

typedef struct
{
    oal_uint8   uc_classa_cmp;
    oal_uint8   uc_classb_cmp;
    oal_uint8   auc_reserve[2];
}hi1103_pa_ical_val_stru;

typedef struct
{
    oal_uint16  us_txdc_cmp_i;
    oal_uint16  us_txdc_cmp_q;
}hi1103_txdc_comp_val_stru;

typedef struct
{
    oal_uint8   uc_ppf_val;
    oal_uint8   auc_reserve[3];
}hi1103_ppf_comp_val_stru;

typedef struct
{
    oal_uint16   us_txiq_cmp_p;
    oal_uint16   us_txiq_cmp_e;
}hi1103_txiq_comp_val_stru;

typedef struct
{
    oal_int32  l_txiq_cmp_alpha_p;
    oal_int32  l_txiq_cmp_beta_p;
    oal_int32  l_txiq_cmp_alpha_n;
    oal_int32  l_txiq_cmp_beta_n;
}hi1103_new_txiq_comp_val_stru;

typedef struct
{
    oal_int32  l_txiq_cmp_alpha;
    oal_int32  l_txiq_cmp_beta;
    oal_int16  as_txiq_comp_ls_filt[HI1103_CALI_TXIQ_LS_FILTER_TAP_NUM];
    oal_uint8  auc_resev[2];
}hi1103_new_txiq_time_comp_val_stru;

typedef struct
{
    oal_int32  l_rxiq_cmp_u1;
    oal_int32  l_rxiq_cmp_u2;
    oal_int32  l_rxiq_cmp_alpha;
    oal_int32  l_rxiq_cmp_beta;
    oal_int16  as_rxiq_comp_ls_filt[HI1103_CALI_RXIQ_LS_FILTER_TAP_NUM];
    oal_uint8  auc_resev[2];
}hi1103_new_rxiq_comp_val_stru;

typedef struct
{
    hi1103_rx_dc_comp_val_stru         st_cali_rx_dc_cmp;
    hi1103_logen_comp_val_stru         st_cali_logen_cmp;
    hi1103_2G_tx_power_comp_val_stru   st_cali_tx_power_cmp_2G;
    hi1103_txdc_comp_val_stru          ast_txdc_cmp_val[HI1103_CALI_TXDC_GAIN_LVL_NUM];
#ifdef _PRE_WLAN_NEW_IQ
    hi1103_new_txiq_comp_val_stru      ast_new_txiq_cmp_val[HI1103_CALI_IQ_TONE_NUM];
    hi1103_new_txiq_time_comp_val_stru st_new_txiq_time_cmp_val;
    hi1103_new_rxiq_comp_val_stru      st_new_rxiq_cmp_val;
#endif
}hi1103_2Gcali_param_stru;

typedef struct
{
    hi1103_rx_dc_comp_val_stru         st_cali_rx_dc_cmp;
    hi1103_logen_comp_val_stru         st_cali_logen_cmp;
    hi1103_5G_tx_power_comp_val_stru   st_cali_tx_power_cmp_5G;
    hi1103_ppf_comp_val_stru           st_ppf_cmp_val;
    hi1103_txdc_comp_val_stru          ast_txdc_cmp_val[HI1103_CALI_TXDC_GAIN_LVL_NUM];
#ifdef _PRE_WLAN_NEW_IQ
    hi1103_new_txiq_comp_val_stru      ast_new_txiq_cmp_val[HI1103_CALI_IQ_TONE_NUM];
    hi1103_new_txiq_time_comp_val_stru st_new_txiq_time_cmp_val;
    hi1103_new_rxiq_comp_val_stru      st_new_rxiq_cmp_val;
#endif
}hi1103_5Gcali_param_stru;

typedef struct
{
    hi1103_2Gcali_param_stru st_2Gcali_param[HI1103_2G_CHANNEL_NUM];
    hi1103_5Gcali_param_stru st_5Gcali_param[HI1103_5G_CHANNEL_NUM];
}hi1103_wifi_cali_param_stru;

typedef struct
{
    oal_uint16  ul_cali_time;
    oal_uint16  bit_temperature     : 3,
                uc_5g_chan_idx1     : 5,
                uc_5g_chan_idx2     : 5,
                bit_rev             : 3;
}hi1103_update_cali_channel_stru;

typedef struct
{
    oal_uint8              uc_rc_cmp_code;
    oal_uint8              uc_r_cmp_code;
    oal_uint8              uc_c_cmp_code;
    oal_uint8              uc_20M_rc_cmp_code;
}hi1103_rc_r_c_cali_param_stru;

typedef struct
{
    oal_uint32                      ul_dog_tag;
    hi1103_2Gcali_param_stru        ast_2Gcali_param[HI1103_2G_CHANNEL_NUM];
    hi1103_5Gcali_param_stru        ast_5Gcali_param[HI1103_5G_CHANNEL_NUM];
    hi1103_ppf_comp_val_stru        st_165chan_ppf_comp;
    hi1103_update_cali_channel_stru st_cali_update_info;
    oal_uint32                      ul_check_hw_status;
    hi1103_pa_ical_val_stru         st_pa_ical_cmp;
    hi1103_rc_r_c_cali_param_stru   st_rc_r_c_cali_data;
    oal_bool_enum_uint8             en_save_all;
    oal_uint8                       auc_resv[3];
}hi1103_cali_param_stru;
#endif

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32  hmac_save_cali_event(frw_event_mem_stru *pst_event_mem);
extern oal_uint32 hmac_send_cali_data(mac_vap_stru *pst_mac_vap);
extern oal_uint32 hmac_send_corram_data(mac_vap_stru *pst_mac_vap);

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of hmac_mgmt_classifier.h */


