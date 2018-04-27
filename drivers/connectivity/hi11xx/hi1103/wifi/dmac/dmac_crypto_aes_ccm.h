

#ifndef __DMAC_CRYPTO_AES_CCM_H__
#define __DMAC_CRYPTO_AES_CCM_H__

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
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CRYPTO_AES_CCM_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define ETH_ALEN            6

#define AES_BLOCK_LEN       16
#define ALG_CCMP_KEY_LEN	16
#define CCMP_HDR_LEN		8
#define CCMP_MIC_LEN		8
#define CCMP_TK_LEN		    16
#define CCMP_PN_LEN		    6
#define CMAC_PN_LEN		    6
#define NUM_RX_DATA_QUEUES	17

#define IEEE80211_QOS_CTL_LEN	    2
#ifndef IEEE80211_QOS_CTL_TID_MASK
#define IEEE80211_QOS_CTL_TID_MASK	0x000F
#endif

#define IEEE80211_FC_SUBTYPE	    0x00f0
#define IEEE80211_FC_TODS		    0x0100
#define IEEE80211_FC_FROMDS		    0x0200
#define IEEE80211_FC_PM		        0x1000
#define IEEE80211_FC_RETRY		    0x0800
#define IEEE80211_FC_MOREDATA		0x2000
#define IEEE80211_FC_TYPE		    0x000c
#define IEEE80211_FC_PROTECTED	    0x4000
#define IEEE80211_FC_ORDER		    0x8000

#define IEEE80211_TYPE_DATA		    0x0008
#define IEEE80211_TYPE_MGMT	        0x0000
#define IEEE80211_TYPE_CTL		    0x0004
#define IEEE80211_SUBTYPE_QOS_DATA  0x0080


/* For AES GENERIC ROUTINES */
#define AES_MIN_KEY_SIZE	    16
#define AES_MAX_KEY_SIZE	    32
#define AES_KEYSIZE_128		    16
#define AES_KEYSIZE_192		    24
#define AES_KEYSIZE_256		    32
#define AES_BLOCK_SIZE		    16
#define AES_MAX_KEYLENGTH	    (15 * 16)
#define AES_MAX_KEYLENGTH_U32	(AES_MAX_KEYLENGTH / sizeof(u32))


#define CRYPTO_TFM_REQ_MASK		        0x000fff00
#define CRYPTO_TFM_RES_MASK		        0xfff00000

#define CRYPTO_TFM_REQ_WEAK_KEY		    0x00000100
#define CRYPTO_TFM_REQ_MAY_SLEEP	    0x00000200
#define CRYPTO_TFM_REQ_MAY_BACKLOG	    0x00000400
#define CRYPTO_TFM_RES_WEAK_KEY		    0x00100000
#define CRYPTO_TFM_RES_BAD_KEY_LEN   	0x00200000
#define CRYPTO_TFM_RES_BAD_KEY_SCHED 	0x00400000
#define CRYPTO_TFM_RES_BAD_BLOCK_LEN 	0x00800000
#define CRYPTO_TFM_RES_BAD_FLAGS 	    0x01000000

#ifndef	DIV_ROUND_UP
#define	DIV_ROUND_UP(x,y)	(((x) + ((y) - 1)) / (y))
#endif

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
struct aes_ccm_key_info_t
{
    oal_uint8 auc_tx_pn[6];
    oal_uint8 auc_rx_pn[6];
    oal_uint32 ul_replays; /* dot11RSNAStatsCCMPReplays */
    oal_int8 c_key_idx;
    oal_uint8 uc_key_valid;

    /* scratch buffers for virt_to_page() (crypto API) */
    oal_uint8 auc_tx_crypto_buf[6 * AES_BLOCK_LEN];
    oal_uint8 auc_rx_crypto_buf[6 * AES_BLOCK_LEN];
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


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32 dmac_ieee80211_aes_ccmp_encrypt(oal_uint8 *puc_mac_hdr_addr,
                                                            oal_uint8 uc_mac_header_len,
                                                            oal_uint8 *puc_mac_body_addr,
                                                            oal_uint16 us_mac_frame_len,
                                                            oal_uint8 uc_ta_user_key_idx,
                                                            oal_uint32 ul_ta_user_key_len,
                                                            oal_uint8 *puc_ta_user_key,
                                                            oal_int32 spp_amsdu_en,
                                                            oal_uint8 *puc_tx_pn);
extern oal_uint32 dmac_ieee80211_aes_ccmp_decrypt(oal_uint8 *puc_mac_hdr_addr,
                                                            oal_uint8 uc_mac_header_len,
                                                            oal_uint8 *puc_mac_body_addr,
                                                            oal_uint16 us_mac_frame_len,
                                                            oal_uint8 uc_ta_user_key_idx,
                                                            oal_uint32 ul_ta_user_key_len,
                                                            oal_uint8 *puc_ta_user_key,
                                                            hal_rx_status_enum_uint8 *puc_rx_status,
                                                            oal_int32 spp_amsdu_en,
                                                            oal_int32 replay_detect_en);


#endif /* _PRE_WLAN_FEATURE_SOFT_CRYPTO */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* #ifndef __DMAC_CRYPTO_AES_CCM_H__ */
