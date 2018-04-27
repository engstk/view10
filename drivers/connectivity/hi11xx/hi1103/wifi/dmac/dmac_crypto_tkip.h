

#ifndef __DMAC_CRYPTO_TKIP_H__
#define __DMAC_CRYPTO_TKIP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SOFT_CRYPTO

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "dmac_crypto_comm.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CRYPTO_TKIP_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define TKIP_KEY_LEN        32
#define PHASE1_LOOP_COUNT   8

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
enum
{
    IEEE80211_CRYPTO_TKIP_COUNTERMEASURES = (1 << 0),
};

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
struct michael_mic_ctx
{
    oal_uint32 l, r;
};

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
struct ieee80211_tkip_data
{

    oal_uint8 auc_key[TKIP_KEY_LEN];
    oal_int32 l_key_set;

    oal_uint32 ul_tx_iv32;
    oal_uint16 us_tx_iv16;
    oal_uint16 aus_tx_ttak[5];
    oal_int32  l_tx_phase1_done;

    oal_uint32 ul_rx_iv32;
    oal_uint16 us_rx_iv16;
    oal_uint16 aus_rx_ttak[5];
    oal_int32  l_rx_phase1_done;
    oal_uint32 ul_rx_iv32_new;
    oal_uint16 us_rx_iv16_new;

    oal_uint32 ul_dot11RSNAStatsTKIPReplays;
    oal_uint32 ul_dot11RSNAStatsTKIPICVErrors;
    oal_uint32 ul_dot11RSNAStatsTKIPLocalMICFailures;

    oal_int32 l_key_idx;
    unsigned long flags;

    oal_uint32 ul_auth_supp_flag;
};


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
OAL_STATIC OAL_INLINE oal_uint16 RotR1(oal_uint16 us_val)
{
    return (us_val >> 1) | (us_val << 15);
}
OAL_STATIC OAL_INLINE oal_uint8 Lo8(oal_uint16 us_val)
{
    return us_val & 0xff;
}
OAL_STATIC OAL_INLINE oal_uint8 Hi8(oal_uint16 us_val)
{
    return us_val >> 8;
}
OAL_STATIC OAL_INLINE oal_uint16 Lo16(oal_uint32 ul_val)
{
    return ul_val & 0xffff;
}
OAL_STATIC OAL_INLINE oal_uint16 Hi16(oal_uint32 ul_val)
{
    return ul_val >> 16;
}
OAL_STATIC OAL_INLINE oal_uint16 Mk16(oal_uint8 uc_hi, oal_uint8 uc_lo)
{
    return uc_lo | (((oal_uint16) uc_hi) << 8);
}
OAL_STATIC OAL_INLINE oal_uint32 RoL32(oal_uint32 ul_word, oal_uint32 ul_shift)
{
    return (ul_word << ul_shift) | (ul_word >> (32 - ul_shift));
}
OAL_STATIC OAL_INLINE oal_uint32 RoR32(oal_uint32 ul_word, oal_uint32 ul_shift)
{
    return (ul_word >> ul_shift) | (ul_word << (32 - ul_shift));
}
OAL_STATIC OAL_INLINE oal_int32 tkip_replay_check(oal_uint32 ul_iv32_n, oal_uint16 us_iv16_n,
                                    oal_uint32 ul_iv32_o, oal_uint16 us_iv16_o)
{
    if (ul_iv32_n < ul_iv32_o ||
        (ul_iv32_n == ul_iv32_o && us_iv16_n <= us_iv16_o))
    { return 1; }
    return 0;
}
/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32 dmac_ieee80211_tkip_encrypt(oal_uint8 *puc_mac_hdr_addr,
                                                            oal_uint8 uc_mac_header_len,
                                                            oal_uint8 *puc_mac_body_addr,
                                                            oal_uint16 us_mac_frame_len,
                                                            oal_uint8 uc_ta_user_key_idx,
                                                            oal_uint32 ul_ta_user_key_len,
                                                            oal_uint8 *puc_ta_user_key,
                                                            oal_uint32 ul_vap_auth_supp_flag,
                                                            hal_rx_status_enum_uint8 *puc_rx_status,
                                                            oal_int32 encrypt_no_frag,
                                                            oal_uint8 *puc_tx_pn);
extern oal_uint32 dmac_ieee80211_tkip_decrypt(oal_uint8 *puc_mac_hdr_addr,
                                                            oal_uint8 uc_mac_header_len,
                                                            oal_uint8 *puc_mac_body_addr,
                                                            oal_uint16 us_mac_frame_len,
                                                            oal_uint8 uc_ta_user_key_idx,
                                                            oal_uint32 ul_ta_user_key_len,
                                                            oal_uint8 *puc_ta_user_key,
                                                            oal_uint32 ul_vap_auth_supp_flag,
                                                            hal_rx_status_enum_uint8 *puc_rx_status,
                                                            oal_int32 decrypt_no_frag,
                                                            oal_int32 replay_detect_en);


#endif /* _PRE_WLAN_FEATURE_SOFT_CRYPTO */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* #ifndef __DMAC_CRYPTO_TKIP_H__ */

