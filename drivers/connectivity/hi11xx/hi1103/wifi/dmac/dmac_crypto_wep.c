


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SOFT_CRYPTO
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_crypto_wep.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CRYPTO_WEP_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
OAL_STATIC struct wep_data wep_root;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/* Copy necessary key info to wep_root */
OAL_STATIC oal_uint32 wep_set_key(oal_void* p_key, oal_uint32 ul_len, oal_uint8 uc_key_idx, oal_uint32 ul_iv)
{
    if (ul_len > WEP_KEY_LEN)
    {
        OAM_WARNING_LOG0(0,OAM_SF_CRYPTO,"wep_set_key Invalid WEP key length!\n");
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    if ((ul_len != 5) && (ul_len != 13))
    {
        OAM_WARNING_LOG0(0,OAM_SF_CRYPTO,"wep_set_key Key length must be 5(WEP40) or 13(WEP104)!\n");
        return OAL_ERR_CODE_CONFIG_EXCEED_SPEC;
    }

    wep_root.uc_key_idx = uc_key_idx;

    oal_memcopy(wep_root.auc_key, p_key, ul_len);
    wep_root.auc_key[ul_len] = '\0';
    wep_root.uc_key_len = (oal_uint8)ul_len;
    wep_root.ul_iv = ul_iv - 1;

    OAM_WARNING_LOG2(0,OAM_SF_CRYPTO,"wep_set_key: key_idx=%d, key_len=%d.\n", uc_key_idx, ul_len);

    return OAL_SUCC;
}

/* Add WEP IV/key info to a frame that has at least 4 bytes of headroom */
OAL_STATIC oal_void wep_build_iv(oal_uint8 *puc_mac_body_addr, oal_uint16 us_mac_data_len)
{
    oal_uint32      ul_klen;
    oal_uint8      *puc_pos;
    oal_uint32      loop;
    /* oal_uint32     *ul_pos_iv; */

    /* move 4 byetes backwards */
    puc_pos = puc_mac_body_addr + us_mac_data_len + WEP_IV_LEN - 1;
    for (loop=0; loop<us_mac_data_len; loop++)
    {
        *(puc_pos - loop) = *(puc_pos - WEP_IV_LEN - loop);
    }

    ul_klen = WEP_IV_LEN - 1 + wep_root.uc_key_len;

    wep_root.ul_iv++;

    //Hi1151 do not support
#if 0
    /* Fluhrer, Mantin, and Shamir have reported weaknesses in the key
     * scheduling algorithm of RC4. At least IVs (KeyByte + 3, 0xff, N)
     * can be used to speedup attacks, so avoid using them. */
    if ((*pul_iv & 0xff00) == 0xff00)
    {
        u8 B = (*pul_iv >> 16) & 0xff;
        if (B >= 3 && B < ul_klen)
        { *pul_iv += 0x0100; }
    }
#endif

    puc_pos = puc_mac_body_addr;
    /* Prepend 24-bit IV to RC4 key and TX frame */
    *puc_pos++ = wep_root.ul_iv & 0xff;
    *puc_pos++ = (wep_root.ul_iv >> 8) & 0xff;
    *puc_pos++ = (wep_root.ul_iv >> 16) & 0xff;

    *puc_pos++ = wep_root.uc_key_idx << 6;
}

/* Perform WEP encryption on given skb that has at least 4 bytes of headroom
 * for IV and 4 bytes of tailroom for ICV. Both IV and ICV will be transmitted,
 * so the payload length increases with 8 bytes.
 *
 * WEP frame payload: IV + TX key idx, RC4(data), ICV = RC4(CRC32(data))

 * puc_mac_hdr_addr:帧头起始虚拟地址
 * uc_mac_header_len:帧头长度
 * puc_mac_body_addr:帧体起始虚拟地址
 * us_mac_frame_len:整帧长度(包括帧头和帧体)
 * uc_tx_key_idx:ta user key idx
 * ul_tx_key_len:ta user key len
 * puc_tx_key:ta user key
 * pul_iv:ta user iv
 *
 * |本函数不作任何入参校验，外部保证正确性|
 */
oal_int32 dmac_ieee80211_wep_encrypt(oal_uint8 *puc_mac_hdr_addr,
                                                    oal_uint8 uc_mac_header_len,
                                                    oal_uint8 *puc_mac_body_addr,
                                                    oal_uint16 us_mac_frame_len,
                                                    oal_uint8 uc_tx_key_idx,
                                                    oal_uint32 ul_tx_key_len,
                                                    oal_uint8 *puc_tx_key,
                                                    oal_uint32* pul_iv)
{
    oal_uint32      ul_crc;
    oal_uint32      ul_klen;
    oal_uint8      *puc_pos;
    oal_uint8      *puc_icv;
    oal_uint8       auc_key[WEP_KEY_LEN + WEP_IV_LEN - 1];
    oal_uint32      us_mac_data_len = us_mac_frame_len - uc_mac_header_len;
    oal_uint16      us_encrypt_len;

    OAM_WARNING_LOG2(0, 0, "CRYPTO WEP Encryption. head len=%d, Payload len=%d.\n",
        uc_mac_header_len,
        us_mac_frame_len);

    /* copy info to wep_root */
    if (OAL_SUCC != wep_set_key(puc_tx_key, ul_tx_key_len, uc_tx_key_idx, *pul_iv))
    {
        return -1;
    }

    /* add the IV to the frame */
    wep_build_iv(puc_mac_body_addr, us_mac_data_len);

    /* Copy the IV into the first 3 bytes of the key */
    oal_memcopy(auc_key, puc_mac_body_addr, WEP_IV_LEN - 1);

    /* Copy rest of the WEP key (the secret part) */
    oal_memcopy(auc_key + WEP_IV_LEN - 1, wep_root.auc_key, wep_root.uc_key_len);

    puc_pos = puc_mac_body_addr + WEP_IV_LEN;
    ul_klen =  WEP_IV_LEN - 1 + wep_root.uc_key_len;

    /* Append little-endian CRC32 over only the data and encrypt it to produce ICV */
    ul_crc = do_crc32(puc_pos, us_mac_data_len, 1);
    puc_icv = puc_mac_body_addr + WEP_IV_LEN + us_mac_data_len;
    puc_icv[0] = ul_crc;
    puc_icv[1] = ul_crc >> 8;
    puc_icv[2] = ul_crc >> 16;
    puc_icv[3] = ul_crc >> 24;

    arc4_set_key(auc_key, ul_klen);

    us_encrypt_len = us_mac_data_len + WEP_ICV_LEN;
    puc_pos = puc_mac_body_addr + WEP_IV_LEN;
    do
    {
        arc4_crypt(puc_pos, puc_pos);
        puc_pos += 1;
    }
    while (us_encrypt_len--);

    return us_mac_data_len + WEP_IV_LEN + WEP_ICV_LEN;
}

/* Perform WEP decryption on given buffer. Buffer includes whole WEP part of
 * the frame: IV (4 bytes), encrypted payload (including SNAP header),
 * ICV (4 bytes). len includes both IV and ICV.
 *
 * Returns 0 if frame was decrypted successfully and ICV was correct and -1 on
 * failure. If frame is OK, IV and ICV will be removed.

 * puc_mac_hdr_addr:帧头起始虚拟地址
 * uc_mac_header_len:帧头长度
 * puc_mac_body_addr:帧体起始虚拟地址
 * us_mac_frame_len:整帧长度(包括帧头和帧体)
 * uc_key_idx:ta user key idx
 * ul_key_len:ta user key len
 * puc_key:ta user key
 * puc_rx_status:rx status
 *
 * |本函数不作任何入参校验，外部保证正确性|
 */
oal_int32 dmac_ieee80211_wep_decrypt(oal_uint8 *puc_mac_hdr_addr,
                                                    oal_uint8 uc_mac_header_len,
                                                    oal_uint8 *puc_mac_body_addr,
                                                    oal_uint16 us_mac_frame_len,
                                                    oal_uint8 uc_ta_user_key_idx,
                                                    oal_uint32 ul_ta_user_key_len,
                                                    oal_uint8 *puc_ta_user_key,
                                                    hal_rx_status_enum_uint8 *puc_rx_status)
{
    oal_uint32      ul_crc;
    oal_uint32      ul_klen;
    oal_uint8       auc_key[WEP_KEY_LEN + WEP_IV_LEN - 1];
    oal_uint8       uc_key_idx;
    oal_uint8      *puc_pos;
    oal_uint8       auc_icv[WEP_ICV_LEN];
    oal_uint16      us_body_len = us_mac_frame_len - uc_mac_header_len;     /* iv + data + icv */
    oal_uint16      us_encrypt_len;     /* 加密域长度 = frame_len - header_len - iv */

    OAM_INFO_LOG2(0, 0, "CRYPTO WEP Decryption. head len=%d, Payload len(include iv & icv)=%d.\n",
        uc_mac_header_len,
        us_mac_frame_len);

    /* copy info to wep_root */
    if (OAL_SUCC != wep_set_key(puc_ta_user_key, ul_ta_user_key_len, uc_ta_user_key_idx, 0))
    {
        return 0;
    }

    /* move to body */
    puc_pos = puc_mac_body_addr;
    auc_key[0] = *puc_pos++;
    auc_key[1] = *puc_pos++;
    auc_key[2] = *puc_pos++;
    uc_key_idx = *puc_pos++ >> 6;

    /* check key idx */
    if (wep_root.uc_key_idx != uc_key_idx)
    {
        *puc_rx_status = HAL_RX_KEY_SEARCH_FAILURE;
        return OAL_FAIL;
    }

    /* Copy rest of the WEP key (the secret part) */
    ul_klen = WEP_IV_LEN - 1 + wep_root.uc_key_len;    //+iv 3octects
    oal_memcopy(auc_key + WEP_IV_LEN - 1, wep_root.auc_key, wep_root.uc_key_len);

    /* Apply RC4 to data and compute CRC32 over decrypted data */
    arc4_set_key(auc_key, ul_klen);

    /* encrypted data len */
    us_encrypt_len = us_body_len - WEP_IV_LEN;
    /* move to data */
    puc_pos = puc_mac_body_addr + WEP_IV_LEN;
    do
    {
        arc4_crypt(puc_pos, puc_pos);
        puc_pos += 1;
    }
    while(us_encrypt_len--);

    /* do_crc32，notice not include mac head */
    puc_pos = puc_mac_body_addr + WEP_IV_LEN;
    ul_crc = do_crc32(puc_pos, us_body_len - WEP_IV_LEN - WEP_ICV_LEN, 1);
    auc_icv[0] = ul_crc;
    auc_icv[1] = ul_crc >> 8;
    auc_icv[2] = ul_crc >> 16;
    auc_icv[3] = ul_crc >> 24;

    if (oal_memcmp(auc_icv, puc_pos + us_body_len - WEP_IV_LEN - WEP_ICV_LEN, 4))
    {
        /* ICV mismatch - drop frame */
        OAM_WARNING_LOG0(0, 0, "CRYPTO icv mismatch.\n");
        *puc_rx_status = HAL_RX_ICV_FAILURE;

        /* do not return fail */
    }

    /* Remove IV and ICV */
    oal_memmove(puc_mac_body_addr, puc_mac_body_addr + WEP_IV_LEN, us_body_len - WEP_IV_LEN - WEP_ICV_LEN);

    if (*puc_rx_status != HAL_RX_ICV_FAILURE)
    {
        *puc_rx_status = HAL_RX_SUCCESS;
    }

    return us_body_len - WEP_IV_LEN - WEP_ICV_LEN;
}



#endif /* #ifdef _PRE_WLAN_FEATURE_SOFT_CRYPTO */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
