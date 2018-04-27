


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SOFT_CRYPTO
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_aes.h"
#include "dmac_crypto_aes_ccm.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CRYPTO_AES_CCM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
static oal_aes_key_stru aes_ctx_root;
static struct aes_ccm_key_info_t  aes_ccmp_root;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
static int tl_ieee80211_has_a4(oal_uint16 fc)
{
    oal_uint16 tmp = (IEEE80211_FC_TODS | IEEE80211_FC_FROMDS);
    return (fc & tmp) == tmp;
}


static  int tl_ieee80211_is_data(oal_uint16 fc)
{
    return (fc & (IEEE80211_FC_TYPE)) == (IEEE80211_TYPE_DATA);
}

static  int tl_ieee80211_is_mgmt(oal_uint16 fc)
{
    return (fc & (IEEE80211_FC_TYPE)) == (IEEE80211_TYPE_MGMT);
}


static int tl_ieee80211_is_data_qos(oal_uint16 fc)
{
    return (fc & (IEEE80211_FC_TYPE | IEEE80211_SUBTYPE_QOS_DATA)) ==
           (IEEE80211_TYPE_DATA | IEEE80211_SUBTYPE_QOS_DATA);

}

static unsigned int cal_ieee80211_hdrlen(oal_uint16 fc)
{
    unsigned int hdrlen = 24;

    if ( (fc & IEEE80211_FC_TYPE) == IEEE80211_TYPE_DATA)
    {
        if (tl_ieee80211_has_a4(fc))
        {
            hdrlen = 30;
        }

        if (tl_ieee80211_is_data_qos(fc))
        {
            hdrlen += IEEE80211_QOS_CTL_LEN;

            if (fc & IEEE80211_FC_ORDER)
            {

                hdrlen += 4; /* HT HEADER */
            }
        }
    }
    else if ((fc & IEEE80211_FC_TYPE) == IEEE80211_TYPE_CTL)
    {
        /*
         * ACK and CTS are 10 bytes, all others 16. To see how
         * to get this condition consider
         *   subtype mask:   0b0000000011110000 (0x00F0)
         *   ACK subtype:    0b0000000011010000 (0x00D0)
         *   CTS subtype:    0b0000000011000000 (0x00C0)
         *   bits that matter:         ^^^      (0x00E0)
         *   value of those: 0b0000000011000000 (0x00C0)
         */
        if ((fc & (0x00E0)) == (0x00C0))
        { hdrlen = 10; }
        else
        { hdrlen = 16; }

        OAL_IO_PRINT("cal_ieee80211_hdrlen: Error! Attempt to encrypt IEEE80211 control packet!\n");
    }
    else if ((fc & IEEE80211_FC_TYPE) == IEEE80211_TYPE_MGMT)
    {
        if (fc & IEEE80211_FC_ORDER)
        {

            hdrlen = 28;
        }
        else
        {
            hdrlen = 24;
        }
    }

    return hdrlen;
}


static void ccmp_pn2hdr(oal_uint8* hdr, oal_uint8* pn, int key_id)
{
    /* struct aes_ccm_key_info_t* key_info = &aes_ccmp_root; */

    hdr[0] = pn[5];
    hdr[1] = pn[4];
    hdr[2] = 0;
    hdr[3] = 0x20 | (key_id << 6);
    hdr[4] = pn[3];
    hdr[5] = pn[2];
    hdr[6] = pn[1];
    hdr[7] = pn[0];
}

static  int ccmp_hdr2pn(oal_uint8* pn, oal_uint8* hdr)
{
    pn[0] = hdr[7];
    pn[1] = hdr[6];
    pn[2] = hdr[5];
    pn[3] = hdr[4];
    pn[4] = hdr[1];
    pn[5] = hdr[0];
    return (hdr[3] >> 6) & 0x03;
}

static void tl_put_unaligned_be16(oal_uint16 val, void* p)
{
    oal_uint8* oo = (oal_uint8*)p;
    oal_uint8* ii = (oal_uint8*)&val;

    oo[0] = ii[1];
    oo[1] = ii[0];
}


static void ccmp_special_blocks(oal_uint8* mac_hdr, oal_uint8* pn, oal_uint8* scratch,
                                int len, int encrypted, int spp_amsdu_en)
{
    oal_uint16 mask_fc , fc;
    int a4_included;
    oal_uint8 qos_tid;
    oal_uint8 qos_amsdu_flag;
    oal_uint8* b_0, *aad;
    oal_uint16 data_len, len_a;
    mac_ieee80211_frame_stru* hdr = (mac_ieee80211_frame_stru*)mac_hdr;
    unsigned int hdrlen;
    /* oal_uint8 itr; */

    b_0 = scratch + 3 * AES_BLOCK_LEN;
    aad = scratch + 4 * AES_BLOCK_LEN;

    /*
     * Mask FC: zero subtype b4 b5 b6
     * Retry, PwrMgt, MoreData; set Protected
     */
    mask_fc = *(oal_uint16*)&hdr->st_frame_control;
    fc      = mask_fc;
    /*
    mask_fc &= ~(0x0070 | IEEE80211_FC_RETRY | IEEE80211_FC_PM | IEEE80211_FC_MOREDATA);
    mask_fc |= (IEEE80211_FC_PROTECTED);
    */

    /* Mask subtype bits(bits 4 5 6) in Data MPDU to 0 */
    if ((fc & IEEE80211_FC_TYPE) == IEEE80211_TYPE_DATA)
    {
        mask_fc &= ~(0x0070);
    }
    /* Mask Retry,PwrMgt,MoreData bits to 0 */
    mask_fc &= ~(IEEE80211_FC_RETRY | IEEE80211_FC_PM | IEEE80211_FC_MOREDATA);
    /* Set Protected bit to 1 */
    mask_fc |= (IEEE80211_FC_PROTECTED);
    /* Mask order bit to 0 in QoS DATA MPDU */
    if (((fc & IEEE80211_FC_TYPE) == IEEE80211_TYPE_DATA)
        && ((fc & IEEE80211_SUBTYPE_QOS_DATA) == IEEE80211_SUBTYPE_QOS_DATA))
    {
        mask_fc &= ~(IEEE80211_FC_ORDER);
    }

    hdrlen = cal_ieee80211_hdrlen(fc);
    //OAL_IO_PRINT("AES-CCM IEEE80211 HDR Length: %d.\n", hdrlen);
    len_a = hdrlen - 2;
    a4_included = tl_ieee80211_has_a4(fc);

    /* Get qos tid and qos amsdu flag field */
    if (tl_ieee80211_is_data_qos(fc))
    {
        mac_ieee80211_qos_frame_stru* qos_hdr = (mac_ieee80211_qos_frame_stru*)hdr;
        //qos_tid = *ieee80211_get_qos_ctl(hdr) & IEEE80211_QOS_CTL_TID_MASK;
        qos_tid = (oal_uint8)qos_hdr->bit_qc_tid;
        qos_amsdu_flag = qos_hdr->bit_qc_amsdu << 7;
    }
    else
    {
        qos_tid = 0;
        qos_amsdu_flag = 0;
    }

    data_len = len - CCMP_HDR_LEN;
    if (encrypted)
    { data_len -= CCMP_MIC_LEN; }

    /* First block, b_0 */
    b_0[0] = 0x59; /* flags: Adata: 1, M: 011, L: 001 */
    /* Nonce: QoS Priority | A2 | PN */
    b_0[1] = qos_tid;
    /* PMF add 20140211 */
    if (0 == hdr->st_frame_control.bit_type)
    {
        b_0[1] |= 0x10;
    }
    oal_memcopy(&b_0[2], hdr->auc_address2, ETH_ALEN);
    oal_memcopy(&b_0[8], pn, CCMP_PN_LEN);
    /* l(m) */
    tl_put_unaligned_be16(data_len, &b_0[14]);

    /* AAD (extra authenticate-only data) / masked 802.11 header
     * FC | A1 | A2 | A3 | SC | [A4] | [QC] */
    tl_put_unaligned_be16(len_a, &aad[0]);
    //put_unaligned_le16(mask_fc, (oal_uint16 *)&aad[2]);
    *(oal_uint16*)&aad[2] = mask_fc;
    oal_memcopy(&aad[4], &hdr->auc_address1, 3 * ETH_ALEN);

    /* Mask Seq#, leave Frag# */
    aad[22] = ((oal_uint8)hdr->bit_frag_num) & 0x0f;
    aad[23] = 0;

    if (a4_included)
    {
        mac_ieee80211_frame_addr4_stru* addr4_hdr = (mac_ieee80211_frame_addr4_stru*)hdr;
        oal_memcopy(&aad[24], addr4_hdr->auc_address4, ETH_ALEN);
        aad[30] = qos_tid;
        if (spp_amsdu_en)
        {
            /* when SPP A-MSDU is enabled, use qos_amsdu_flag field to constuct AAD */
            aad[30] |= qos_amsdu_flag;
        }
        aad[31] = 0;
    }
    else
    {
        oal_memset(&aad[24], 0, ETH_ALEN + IEEE80211_QOS_CTL_LEN);
        aad[24] = qos_tid;
        if (spp_amsdu_en)
        {
            /* when SPP A-MSDU is enabled, use qos_amsdu_flag field to constuct AAD */
            aad[24] |= qos_amsdu_flag;
        }
    }

#if 0
    OAL_IO_PRINT("AES-CCM Nonce:");
    for (itr = 0; itr < AES_BLOCK_LEN; itr++)
    {

        OAL_IO_PRINT("%02x ", b_0[itr]);
    }
    OAL_IO_PRINT(".\n");

    OAL_IO_PRINT("AES-CCM AAD:");
    for (itr = 0; itr < 2 * AES_BLOCK_LEN; itr++)
    {

        OAL_IO_PRINT("%02x ", aad[itr]);
    }
    OAL_IO_PRINT(".\n");
#endif
}


static void aes_ccm_prepare(oal_uint8* scratch, oal_uint8* a)
{
    int i;
    oal_uint8* b_0, *aad, *b, *s_0;
    oal_aes_key_stru* ctx = &aes_ctx_root;

    b_0 = scratch + 3 * AES_BLOCK_LEN;
    aad = scratch + 4 * AES_BLOCK_LEN;
    b = scratch;
    s_0 = scratch + AES_BLOCK_LEN;

    oal_aes_encrypt(ctx, b, b_0);

    /* Extra Authenticate-only data (always two AES blocks) */
    for (i = 0; i < AES_BLOCK_LEN; i++)
    { aad[i] ^= b[i]; }
    oal_aes_encrypt(ctx, b, aad);

    aad += AES_BLOCK_LEN;

    for (i = 0; i < AES_BLOCK_LEN; i++)
    { aad[i] ^= b[i]; }
    oal_aes_encrypt(ctx, a, aad);

    /* Mask out bits from auth-only-b_0 */
    b_0[0] &= 0x07;

    /* S_0 is used to encrypt T (= MIC) */
    b_0[14] = 0;
    b_0[15] = 0;
    oal_aes_encrypt(ctx, s_0, b_0);
}

/**
 * ieee80211_aes_ccm_set_pn - Set pn.
 */
int ieee80211_aes_ccm_set_pn(unsigned int ul_pn_h, unsigned int ul_pn_l)
{
    struct aes_ccm_key_info_t* key_info = &aes_ccmp_root;
    oal_uint8* p;
    int i;
    /* unsigned int ul_tmp_pn; */

    p = (oal_uint8*)(&ul_pn_h);
    for (i = 1; i >= 0; i--)
    {
        key_info->auc_tx_pn[i] = *p;
        p++;
    }

    p = (oal_uint8*)(&ul_pn_l);
    for (i = 3; i >= 0; i--)
    {
        key_info->auc_tx_pn[i + 2] = *p;
        p++;
    }

    p = NULL;

    return 0;
}

/**
 * crypto_aes_set_key - Set the AES key.
 * @tfm:	The %crypto_tfm that is used in the context.
 * @in_key:	The input key.
 * @key_len:	The size of the key.
 *
 * Returns 0 on success, on failure the %CRYPTO_TFM_RES_BAD_KEY_LEN flag in tfm
 * is set. The function uses crypto_aes_expand_key() to expand the key.
 * &crypto_aes_ctx _must_ be the private data embedded in @tfm which is
 * retrieved with crypto_tfm_ctx().
 */
int crypto_aes_set_key(const oal_uint8* in_key,	unsigned int key_len)
{
    oal_aes_key_stru* ctx = &aes_ctx_root;
    int ret;

    ret = oal_aes_expand_key(ctx, in_key, key_len);
    if (!ret)
    {
        return 0;
    }

    return -1;
}

int ieee80211_aes_ccm_set_key(const oal_uint8* in_key,	unsigned int key_len, oal_uint8 key_id)
{
    struct aes_ccm_key_info_t* key_info = &aes_ccmp_root;
    int ret;
    /* oal_uint32 i; */

    //for (i = 0; i < 6; i++)
    //key_info->tx_pn[i] = pn[i];

    key_info->c_key_idx = key_id;

    ret = crypto_aes_set_key(in_key, key_len);
    if (ret)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "AES set key failed!\n");
        key_info->uc_key_valid = 0;
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "AES set key OK!\n");
        key_info->uc_key_valid = 1;
    }

    return ret;
}

static void aes_ccm_encrypt(oal_uint8* scratch,
                            oal_uint8* data, oal_uint32 data_len,
                            oal_uint8* cdata, oal_uint8* mic)
{
    int i, j, last_len, num_blocks;
    oal_uint8* pos, *cpos, *b, *s_0, *e, *b_0;
    oal_aes_key_stru* ctx = &aes_ctx_root;

    b = scratch;
    s_0 = scratch + AES_BLOCK_LEN;
    e = scratch + 2 * AES_BLOCK_LEN;
    b_0 = scratch + 3 * AES_BLOCK_LEN;

    num_blocks = DIV_ROUND_UP(data_len, AES_BLOCK_LEN);
    //OAL_IO_PRINT("AES-CCM blocks: %d.\n", num_blocks);
    last_len = data_len % AES_BLOCK_LEN;
    aes_ccm_prepare(scratch, b);

    /* Process payload blocks */
    pos = data;
    cpos = cdata;
    for (j = 1; j <= num_blocks; j++)
    {
        int blen = (j == num_blocks && last_len) ?
                   last_len : AES_BLOCK_LEN;

        /* 1. Authentication (MIC calculation) */

        for (i = 0; i < blen; i++)
        { b[i] ^= pos[i]; }

        oal_aes_encrypt(ctx, b, b);

        /* AES ctr mode encryption(note 'j' acts as counter) */
        b_0[14] = (j >> 8) & 0xff;
        b_0[15] = j & 0xff;
        oal_aes_encrypt(ctx, e, b_0);

        /* AES encrypts the message body */
        for (i = 0; i < blen; i++)
        { *cpos++ = *pos++ ^ e[i]; }

    }

    /* CCM outputs the MIC code */
    for (i = 0; i < CCMP_MIC_LEN; i++)
    { mic[i] = b[i] ^ s_0[i]; }

}


oal_uint32 dmac_ieee80211_aes_ccmp_encrypt(oal_uint8 *puc_mac_hdr_addr,
                                                            oal_uint8 uc_mac_header_len,
                                                            oal_uint8 *puc_mac_body_addr,
                                                            oal_uint16 us_mac_frame_len,
                                                            oal_uint8 uc_ta_user_key_idx,
                                                            oal_uint32 ul_ta_user_key_len,
                                                            oal_uint8 *puc_ta_user_key,
                                                            oal_int32 spp_amsdu_en,
                                                            oal_uint8 *puc_tx_pn)
{
    struct aes_ccm_key_info_t* key_info = &aes_ccmp_root;
    oal_uint16 us_len;
    oal_uint8* puc_pos, *puc_pn;
    int i;
    oal_uint16 us_data_len = us_mac_frame_len - uc_mac_header_len;

    OAM_WARNING_LOG1(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_aes_ccmp_encrypt AES-CCM payload length: %d.\n", us_data_len);

    /* set key */
    if (ieee80211_aes_ccm_set_key(puc_ta_user_key, ul_ta_user_key_len, uc_ta_user_key_idx) != OAL_SUCC)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_aes_ccmp_encrypt AES-CCM set key failed!\n");
        return 0;
    }
    /* set pn */
    ieee80211_aes_ccm_set_pn(*(oal_uint32*)(puc_tx_pn+4), *(oal_uint32*)puc_tx_pn);

    us_len = us_data_len;

    puc_pos = puc_mac_body_addr + us_len + CCMP_HDR_LEN - 1;
    for (i = 0; i < us_len; i++)
    {
        *(puc_pos - i) = *(puc_pos - CCMP_HDR_LEN - i);
    }

    /* POS points to the CCMP header */
    puc_pos = puc_mac_body_addr;

    /* PN = PN + 1 */
    puc_pn = key_info->auc_tx_pn;

    //good algorithem
    for (i = CCMP_PN_LEN - 1; i >= 0; i--)
    {
        puc_pn[i]++;
        if (puc_pn[i])
        { break; }
    }

    for (i = 0; i < 6; i++)
    {
        puc_tx_pn[i] = puc_pn[5 - i];
    }

    /* Generate CCMP header */
    ccmp_pn2hdr(puc_pos, puc_pn, key_info->c_key_idx);

    puc_pos += CCMP_HDR_LEN;
    ccmp_special_blocks(puc_mac_hdr_addr, puc_pn, key_info->auc_tx_crypto_buf, us_len + CCMP_HDR_LEN, 0, spp_amsdu_en);
    aes_ccm_encrypt(key_info->auc_tx_crypto_buf, puc_pos, us_len,
                    puc_pos, puc_pos + us_len);

    return CCMP_HDR_LEN + us_data_len + CCMP_MIC_LEN;
}


static int aes_ccm_decrypt(oal_uint8* scratch, oal_uint8* cdata, oal_uint32 data_len,
                           oal_uint8* mic, oal_uint8* data)
{
    int i, j, last_len, num_blocks;
    oal_uint8* pos, *cpos, *b, *s_0, *a, *b_0, *aad;
    oal_aes_key_stru* ctx = &aes_ctx_root;

    b = scratch;
    s_0 = scratch + AES_BLOCK_LEN;
    a = scratch + 2 * AES_BLOCK_LEN;
    b_0 = scratch + 3 * AES_BLOCK_LEN;
    aad = scratch + 4 * AES_BLOCK_LEN;

    num_blocks = DIV_ROUND_UP(data_len, AES_BLOCK_LEN);
    last_len = data_len % AES_BLOCK_LEN;
    aes_ccm_prepare(scratch, a);

    /* Process payload blocks */
    cpos = cdata;
    pos = data;
    for (j = 1; j <= num_blocks; j++)
    {
        int blen = (j == num_blocks && last_len) ?
                   last_len : AES_BLOCK_LEN;

        /* Decryption followed by authentication */
        b_0[14] = (j >> 8) & 0xff;
        b_0[15] = j & 0xff;
        oal_aes_encrypt(ctx, b, b_0);
        for (i = 0; i < blen; i++)
        {
            *pos = *cpos++ ^ b[i];
            a[i] ^= *pos++;
        }
        oal_aes_encrypt(ctx, a, a);
    }

    for (i = 0; i < CCMP_MIC_LEN; i++)
    {
        if ((mic[i] ^ s_0[i]) != a[i])
        {
            OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "aes_ccm_decrypt AES-CCM DECRYPT: MIC CHECK Failed!\n");
            return HAL_RX_CCMP_MIC_FAILURE;
        }
    }

    return HAL_RX_SUCCESS;
}


oal_uint32 dmac_ieee80211_aes_ccmp_decrypt(oal_uint8 *puc_mac_hdr_addr,
                                                            oal_uint8 uc_mac_header_len,
                                                            oal_uint8 *puc_mac_body_addr,
                                                            oal_uint16 us_mac_frame_len,
                                                            oal_uint8 uc_ta_user_key_idx,
                                                            oal_uint32 ul_ta_user_key_len,
                                                            oal_uint8 *puc_ta_user_key,
                                                            hal_rx_status_enum_uint8 *puc_rx_status,
                                                            oal_int32 spp_amsdu_en,
                                                            oal_int32 replay_detect_en)
{
    mac_ieee80211_frame_stru* pst_hdr = (mac_ieee80211_frame_stru*)puc_mac_hdr_addr;
    struct aes_ccm_key_info_t* pst_key_info = &aes_ccmp_root;
    oal_uint8 auc_pn[CCMP_PN_LEN];
    oal_int16 us_len;
    hal_rx_status_enum_uint8 ret;
    oal_uint16 us_data_len = us_mac_frame_len - uc_mac_header_len;

    OAM_INFO_LOG1(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_aes_ccmp_decrypt AES-CCM Decryption. Payload Length: %d.\n", us_data_len);

    /* set key */
    ieee80211_aes_ccm_set_key(puc_ta_user_key, ul_ta_user_key_len, uc_ta_user_key_idx);

    if ((!tl_ieee80211_is_data(*(oal_uint16*)&pst_hdr->st_frame_control))
        && (!tl_ieee80211_is_mgmt(*(oal_uint16*)&pst_hdr->st_frame_control)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_aes_ccmp_decrypt AES-CCM DECRYPT: This is not a data or mgmt packet!\n");
        //may not suitable to return 0xf
        *puc_rx_status = 0xF;
        return 0;
    }

    if (pst_key_info->uc_key_valid == 0)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_aes_ccmp_decrypt Found no valid AES-CCM key!\n");
        *puc_rx_status = HAL_RX_KEY_SEARCH_FAILURE;
        return 0;
    }

    us_len = us_data_len - CCMP_HDR_LEN - CCMP_MIC_LEN;
    ccmp_hdr2pn(auc_pn, puc_mac_body_addr);

    if (replay_detect_en && (!pst_hdr->st_frame_control.bit_retry) && (oal_memcmp(auc_pn, pst_key_info->auc_rx_pn, CCMP_PN_LEN) <= 0))
    {
        pst_key_info->ul_replays++;
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "AES-CCM DECRYPT: Detected PN replay attack!\n");
        if (HAL_RX_SUCCESS == *puc_rx_status)
        {
            *puc_rx_status = HAL_RX_CCMP_REPLAY_FAILURE;
        }
    }

    /* hardware didn't decrypt/verify MIC */
    /* construct aad & nonce */
    ccmp_special_blocks(puc_mac_hdr_addr, auc_pn, pst_key_info->auc_rx_crypto_buf, us_data_len, 1, spp_amsdu_en);

    ret = aes_ccm_decrypt(pst_key_info->auc_rx_crypto_buf,
                          puc_mac_body_addr + CCMP_HDR_LEN, us_len,
                          puc_mac_body_addr + us_data_len - CCMP_MIC_LEN,
                          puc_mac_body_addr + CCMP_HDR_LEN);
    if (HAL_RX_SUCCESS != ret)
    {
        *puc_rx_status = ret;
        //return -1;
    }

    /* update pn if all succ */
    if ((HAL_RX_SUCCESS == ret)	&& (HAL_RX_SUCCESS == *puc_rx_status))
    {
        oal_memcopy(pst_key_info->auc_rx_pn, auc_pn, CCMP_PN_LEN);
    }

    /* Remove CCMP header and MIC */
    oal_memmove(puc_mac_body_addr, puc_mac_body_addr + CCMP_HDR_LEN, us_len);

    if (HAL_RX_SUCCESS == *puc_rx_status)
    {
        *puc_rx_status = HAL_RX_SUCCESS;
    }

    return us_len;
}

#endif /* #ifdef _PRE_WLAN_FEATURE_SOFT_CRYPTO */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

