


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SOFT_CRYPTO
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_crypto_tkip.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CRYPTO_TKIP_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
static struct michael_mic_ctx michael_root;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
OAL_STATIC struct ieee80211_tkip_data  g_st_tkip_root;

OAL_STATIC OAL_CONST oal_uint16 tkip_sbox[256] =
{
    0xC6A5, 0xF884, 0xEE99, 0xF68D, 0xFF0D, 0xD6BD, 0xDEB1, 0x9154,
    0x6050, 0x0203, 0xCEA9, 0x567D, 0xE719, 0xB562, 0x4DE6, 0xEC9A,
    0x8F45, 0x1F9D, 0x8940, 0xFA87, 0xEF15, 0xB2EB, 0x8EC9, 0xFB0B,
    0x41EC, 0xB367, 0x5FFD, 0x45EA, 0x23BF, 0x53F7, 0xE496, 0x9B5B,
    0x75C2, 0xE11C, 0x3DAE, 0x4C6A, 0x6C5A, 0x7E41, 0xF502, 0x834F,
    0x685C, 0x51F4, 0xD134, 0xF908, 0xE293, 0xAB73, 0x6253, 0x2A3F,
    0x080C, 0x9552, 0x4665, 0x9D5E, 0x3028, 0x37A1, 0x0A0F, 0x2FB5,
    0x0E09, 0x2436, 0x1B9B, 0xDF3D, 0xCD26, 0x4E69, 0x7FCD, 0xEA9F,
    0x121B, 0x1D9E, 0x5874, 0x342E, 0x362D, 0xDCB2, 0xB4EE, 0x5BFB,
    0xA4F6, 0x764D, 0xB761, 0x7DCE, 0x527B, 0xDD3E, 0x5E71, 0x1397,
    0xA6F5, 0xB968, 0x0000, 0xC12C, 0x4060, 0xE31F, 0x79C8, 0xB6ED,
    0xD4BE, 0x8D46, 0x67D9, 0x724B, 0x94DE, 0x98D4, 0xB0E8, 0x854A,
    0xBB6B, 0xC52A, 0x4FE5, 0xED16, 0x86C5, 0x9AD7, 0x6655, 0x1194,
    0x8ACF, 0xE910, 0x0406, 0xFE81, 0xA0F0, 0x7844, 0x25BA, 0x4BE3,
    0xA2F3, 0x5DFE, 0x80C0, 0x058A, 0x3FAD, 0x21BC, 0x7048, 0xF104,
    0x63DF, 0x77C1, 0xAF75, 0x4263, 0x2030, 0xE51A, 0xFD0E, 0xBF6D,
    0x814C, 0x1814, 0x2635, 0xC32F, 0xBEE1, 0x35A2, 0x88CC, 0x2E39,
    0x9357, 0x55F2, 0xFC82, 0x7A47, 0xC8AC, 0xBAE7, 0x322B, 0xE695,
    0xC0A0, 0x1998, 0x9ED1, 0xA37F, 0x4466, 0x547E, 0x3BAB, 0x0B83,
    0x8CCA, 0xC729, 0x6BD3, 0x283C, 0xA779, 0xBCE2, 0x161D, 0xAD76,
    0xDB3B, 0x6456, 0x744E, 0x141E, 0x92DB, 0x0C0A, 0x486C, 0xB8E4,
    0x9F5D, 0xBD6E, 0x43EF, 0xC4A6, 0x39A8, 0x31A4, 0xD337, 0xF28B,
    0xD532, 0x8B43, 0x6E59, 0xDAB7, 0x018C, 0xB164, 0x9CD2, 0x49E0,
    0xD8B4, 0xACFA, 0xF307, 0xCF25, 0xCAAF, 0xF48E, 0x47E9, 0x1018,
    0x6FD5, 0xF088, 0x4A6F, 0x5C72, 0x3824, 0x57F1, 0x73C7, 0x9751,
    0xCB23, 0xA17C, 0xE89C, 0x3E21, 0x96DD, 0x61DC, 0x0D86, 0x0F85,
    0xE090, 0x7C42, 0x71C4, 0xCCAA, 0x90D8, 0x0605, 0xF701, 0x1C12,
    0xC2A3, 0x6A5F, 0xAEF9, 0x69D0, 0x1791, 0x9958, 0x3A27, 0x27B9,
    0xD938, 0xEB13, 0x2BB3, 0x2233, 0xD2BB, 0xA970, 0x0789, 0x33A7,
    0x2DB6, 0x3C22, 0x1592, 0xC920, 0x8749, 0xAAFF, 0x5078, 0xA57A,
    0x038F, 0x59F8, 0x0980, 0x1A17, 0x65DA, 0xD731, 0x84C6, 0xD0B8,
    0x82C3, 0x29B0, 0x5A77, 0x1E11, 0x7BCB, 0xA8FC, 0x6DD6, 0x2C3A,
};


OAL_STATIC OAL_INLINE oal_uint16 _S_(oal_uint16 v)
{
    oal_uint16 t = tkip_sbox[Hi8(v)];
    return tkip_sbox[Lo8(v)] ^ ((t << 8) | (t >> 8));
}


OAL_STATIC oal_void tkip_mixing_phase1(oal_uint16* TTAK,
                               OAL_CONST oal_uint8* TK,
                               OAL_CONST oal_uint8* TA,
                               oal_uint32 IV32)
{
    oal_int32 i, j;

    /* Initialize the 80-bit TTAK from TSC (IV32) and TA[0..5] */
    TTAK[0] = IV32 & 0xFFFF;
    TTAK[1] = IV32 >> 16;
    TTAK[2] = ((oal_uint16)TA[1] << 8) | TA[0];
    TTAK[3] = ((oal_uint16)TA[3] << 8) | TA[2];
    TTAK[4] = ((oal_uint16)TA[5] << 8) | TA[4];

    for (i = 0; i < PHASE1_LOOP_COUNT; i++)
    {
        j = 2 * (i & 1);
        TTAK[0] += _S_(TTAK[4] ^ Mk16(TK[1 + j], TK[0 + j]));
        TTAK[1] += _S_(TTAK[0] ^ Mk16(TK[5 + j], TK[4 + j]));
        TTAK[2] += _S_(TTAK[1] ^ Mk16(TK[9 + j], TK[8 + j]));
        TTAK[3] += _S_(TTAK[2] ^ Mk16(TK[13 + j], TK[12 + j]));
        TTAK[4] += _S_(TTAK[3] ^ Mk16(TK[1 + j], TK[0 + j])) + i;
    }
}


OAL_STATIC oal_void tkip_mixing_phase2(oal_uint8* WEPSeed, OAL_CONST oal_uint8* TK, OAL_CONST oal_uint16* TTAK,
                               oal_uint16 IV16)
{
    /* Make temporary area overlap WEP seed so that the final copy can be
     * aoal_voided on little endian hosts. */
    oal_uint16* PPK = (oal_uint16*) & WEPSeed[4];

    /* Step 1 - make copy of TTAK and bring in TSC */
    PPK[0] = TTAK[0];
    PPK[1] = TTAK[1];
    PPK[2] = TTAK[2];
    PPK[3] = TTAK[3];
    PPK[4] = TTAK[4];
    PPK[5] = TTAK[4] + IV16;

    /* Step 2 - 96-bit bijective mixing using S-box */
    PPK[0] += _S_(PPK[5] ^ Mk16(TK[1], TK[0]));
    PPK[1] += _S_(PPK[0] ^ Mk16(TK[3], TK[2]));
    PPK[2] += _S_(PPK[1] ^ Mk16(TK[5], TK[4]));
    PPK[3] += _S_(PPK[2] ^ Mk16(TK[7], TK[6]));
    PPK[4] += _S_(PPK[3] ^ Mk16(TK[9], TK[8]));
    PPK[5] += _S_(PPK[4] ^ Mk16(TK[11], TK[10]));

    PPK[0] += RotR1(PPK[5] ^ Mk16(TK[13], TK[12]));
    PPK[1] += RotR1(PPK[0] ^ Mk16(TK[15], TK[14]));
    PPK[2] += RotR1(PPK[1]);
    PPK[3] += RotR1(PPK[2]);
    PPK[4] += RotR1(PPK[3]);
    PPK[5] += RotR1(PPK[4]);

    /* Step 3 - bring in last of TK bits, assign 24-bit WEP IV value
     * WEPSeed[0..2] is transmitted as WEP IV */
    WEPSeed[0] = Hi8(IV16);
    WEPSeed[1] = (Hi8(IV16) | 0x20) & 0x7F;
    WEPSeed[2] = Lo8(IV16);
    WEPSeed[3] = Lo8((PPK[5] ^ Mk16(TK[1], TK[0])) >> 1);
#if 0
    {
        oal_int32 i;
        for (i=0; i<5; ++i)
        {
            WEPSeed[4+(2-i)] = Lo8(PPK[i]);
            WEPSeed[5+(2-i)] = Hi8(PPK[i]);
        }
    }
#endif
}


OAL_STATIC oal_void ieee80211_tkip_set_tsc(oal_uint32 ul_tsc_h, oal_uint32 ul_tsc_l)
{
    oal_uint32 ul_tsc_msb;
    oal_uint32 ul_tsc_lsb;

    struct ieee80211_tkip_data* pst_tkey = &g_st_tkip_root;

    ul_tsc_lsb = (++ul_tsc_l) & 0xFFFFFFFF;
    if (0 == ul_tsc_lsb)
    {
        ul_tsc_msb = (++ul_tsc_h) & 0xFFFF;
    }
    else
    {
        ul_tsc_msb = ul_tsc_h;
    }

    pst_tkey->us_tx_iv16 = ul_tsc_lsb & 0xFFFF;
    pst_tkey->ul_tx_iv32 = (ul_tsc_msb << 16) | (ul_tsc_lsb >> 16);
}


OAL_STATIC oal_uint32 ieee80211_tkip_set_key(oal_uint8* puc_key,
                                   oal_int32 klen,
                                   oal_uint8* puc_seq,
                                   oal_int32 l_key_idx,
                                   oal_uint32 ul_auth_supp_flag)
{
    struct ieee80211_tkip_data* pst_tkey = &g_st_tkip_root;

    oal_memset(pst_tkey, 0, sizeof(*pst_tkey));
    pst_tkey->l_key_idx = l_key_idx;

    if (klen == TKIP_KEY_LEN)
    {
        /* 00 20 01 20 00 00 00 00 TC1082 IV */
        oal_memcopy(pst_tkey->auc_key, puc_key, TKIP_KEY_LEN);

        pst_tkey->l_key_set = 1;
        //tkey->tx_iv16 = 1;	/* TSC is initialized to 1 */
        pst_tkey->ul_auth_supp_flag = ul_auth_supp_flag;
        if (puc_seq)
        {
            pst_tkey->ul_rx_iv32 = (puc_seq[5] << 24) | (puc_seq[4] << 16) |
                                   (puc_seq[3] << 8) | puc_seq[2];
            pst_tkey->us_rx_iv16 = (puc_seq[1] << 8) | puc_seq[0];
        }
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "ieee80211_tkip_set_key key length neq 32, set key fail, will not encrypt(decrypt) packet.");
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_int32 ieee80211_tkip_hdr(oal_uint8* puc_mac_hdr_addr,
                                            oal_uint16 us_data_len,
                                            oal_uint8* puc_mac_body_addr,
                                            oal_uint8* puc_rc4key, oal_int32 l_keylen, oal_void* priv)
{
    struct ieee80211_tkip_data* pst_tkey = (struct ieee80211_tkip_data*)priv;
    mac_ieee80211_frame_stru*   pst_hdr = (mac_ieee80211_frame_stru*)puc_mac_hdr_addr;
    oal_uint8* puc_pos;
    oal_uint8* puc_payload;
    int i;
    //oal_uint32 *pos_iv;

    if (puc_rc4key == NULL || l_keylen < 16)
    { return -1; }

    if (0 == pst_tkey->l_tx_phase1_done)
    {
        tkip_mixing_phase1(pst_tkey->aus_tx_ttak, pst_tkey->auc_key, pst_hdr->auc_address2, pst_tkey->ul_tx_iv32);
        pst_tkey->l_tx_phase1_done = 1;
    }
    tkip_mixing_phase2(puc_rc4key, pst_tkey->auc_key, pst_tkey->aus_tx_ttak, pst_tkey->us_tx_iv16);

    /* Move the payload back of 8 bytes */
    puc_payload = puc_mac_body_addr;
    puc_pos = puc_payload + us_data_len + 8 - 1;
    for (i = 0; i < us_data_len; i++)
    { *(puc_pos - i) = *(puc_pos - 8 - i); }

    /* Build TKIP header */
    puc_pos = puc_payload;

    //tkip head前3 octects: iv_hi8,wepseed[1],iv_lo8，其实就是wepseed[0-2],因为wepseed[0]=iv_hi8,wepseed[2]=iv_lo8
    //here do not need hdr bypass
    //octect1
    *puc_pos++ = *puc_rc4key;
    //octect2
    *puc_pos++ = *(puc_rc4key + 1);
    //octect3
    *puc_pos++ = *(puc_rc4key + 2);
    //octect4
    *puc_pos++ = (pst_tkey->l_key_idx << 6) | (1 << 5); /* Ext IV included */
    *puc_pos++ = pst_tkey->ul_tx_iv32 & 0xff;
    *puc_pos++ = (pst_tkey->ul_tx_iv32 >> 8) & 0xff;
    *puc_pos++ = (pst_tkey->ul_tx_iv32 >> 16) & 0xff;
    *puc_pos++ = (pst_tkey->ul_tx_iv32 >> 24) & 0xff;

    pst_tkey->us_tx_iv16++;
    if (pst_tkey->us_tx_iv16 == 0)
    {
        pst_tkey->l_tx_phase1_done = 0;
        //tkey->tx_iv32++;
    }

    return 0;
}


OAL_STATIC oal_void michael_mic_hdr(oal_uint8* puc_mac_hdr_addr, oal_uint8* puc_mic_hdr)
{
    mac_ieee80211_frame_stru*       pst_hdr = (mac_ieee80211_frame_stru*)puc_mac_hdr_addr;
    mac_header_frame_control_stru*  pst_fc = &pst_hdr->st_frame_control;
    oal_uint16 us_stype;

    us_stype = pst_fc->bit_sub_type;

    if (pst_fc->bit_from_ds || pst_fc->bit_to_ds)
    {
        if ((pst_fc->bit_to_ds) && (!pst_fc->bit_from_ds))
        {
            oal_memcopy(puc_mic_hdr, pst_hdr->auc_address3, 6);	/* DA */
            oal_memcopy(puc_mic_hdr + 6, pst_hdr->auc_address2, 6);	/* SA */
        }
        else if ((pst_fc->bit_from_ds) && (!pst_fc->bit_to_ds))
        {
            oal_memcopy(puc_mic_hdr, pst_hdr->auc_address1, 6);	/* DA */
            oal_memcopy(puc_mic_hdr + 6, pst_hdr->auc_address3, 6);	/* SA */
        }
        else if ((pst_fc->bit_from_ds) && (pst_fc->bit_to_ds))
        {
            oal_memcopy(puc_mic_hdr, ((mac_ieee80211_frame_addr4_stru*)pst_hdr)->auc_address3, 6);	/* DA */
            oal_memcopy(puc_mic_hdr + 6, ((mac_ieee80211_frame_addr4_stru*)pst_hdr)->auc_address4, 6);	/* SA */
        }
    }
    else
    {
        oal_memcopy(puc_mic_hdr, pst_hdr->auc_address1, 6);	/* DA */
        oal_memcopy(puc_mic_hdr + 6, pst_hdr->auc_address2, 6);	/* SA */
    }

    if (us_stype & 0x8 /* WLAN_QOS_DATA */)
    {
        mac_ieee80211_qos_frame_stru* pst_qoshdr = (mac_ieee80211_qos_frame_stru*)pst_hdr;
        puc_mic_hdr[12] = (pst_qoshdr->bit_qc_tid) & 0x000F /* IEEE80211_QCTL_TID */;
    }
    else
    { puc_mic_hdr[12] = 0; }		/* priority */

    /* padding 3 zero */
    puc_mic_hdr[13] = puc_mic_hdr[14] = puc_mic_hdr[15] = 0;	/* reserved */
}


OAL_STATIC oal_void michael_block(struct michael_mic_ctx* mctx, oal_uint32 val)
{
    mctx->l ^= val;
    mctx->r ^= RoL32(mctx->l, 17);
    mctx->l += mctx->r;
    mctx->r ^= ((mctx->l & 0xff00ff00) >> 8) |
               ((mctx->l & 0x00ff00ff) << 8);
    mctx->l += mctx->r;
    mctx->r ^= RoL32(mctx->l, 3);
    mctx->l += mctx->r;
    mctx->r ^= RoR32(mctx->l, 2);
    mctx->l += mctx->r;
}


OAL_STATIC oal_void michael_mic(OAL_CONST oal_uint8* puc_mic_key, oal_uint8* puc_mac_hdr_addr,
                        OAL_CONST oal_uint8* puc_data, oal_uint32 ul_data_len, oal_uint8* puc_mic)
{
    oal_uint32 ul_val;
    unsigned long block, blocks, left;
    oal_uint8 auc_mic_hdr[16];

    /* MIC头，包括DA SA PRIORITY，16 octects */
    //see Figure 11-9—TKIP MIC processing format
    //(da(6) + sa(6) + priority(1) + 0(3)) + data(M) + MIC(8)
    michael_mic_hdr(puc_mac_hdr_addr, auc_mic_hdr);

    michael_root.l = *(oal_uint32*)(puc_mic_key);
    michael_root.r = *(oal_uint32*)(puc_mic_key + 4);

    michael_block(&michael_root, (*(oal_uint32*)&auc_mic_hdr[0]));
    michael_block(&michael_root, (*(oal_uint16*)&auc_mic_hdr[4]) | ((*(oal_uint16*)&auc_mic_hdr[6]) << 16));
    michael_block(&michael_root, (*(oal_uint32*)&auc_mic_hdr[8]));
    michael_block(&michael_root, auc_mic_hdr[12]);

    /* Real data */
    blocks = ul_data_len >> 2;
    left = ul_data_len % 4;

    for (block = 0; block < blocks; block++)
    { michael_block(&michael_root, (*(oal_uint32*)&puc_data[block * 4])); }

    /* Partial block of 0..3 bytes and padding: 0x5a + 4..7 zeros to make
     * total length a multiple of 4. */
    ul_val = 0x5a;
    while (left > 0)
    {
        ul_val <<= 8;
        left--;
        ul_val |= puc_data[blocks * 4 + left];
    }

    michael_block(&michael_root, ul_val);

    //这一步的0是做什么的
    michael_block(&michael_root, 0);

    *(oal_uint32*)puc_mic = michael_root.l;
    *(oal_uint32*)(puc_mic + 4) = michael_root.r;
}


oal_uint32 dmac_ieee80211_tkip_encrypt(oal_uint8 *puc_mac_hdr_addr,
                                                    oal_uint8 uc_mac_header_len,
                                                    oal_uint8 *puc_mac_body_addr,
                                                    oal_uint16 us_mac_frame_len,
                                                    oal_uint8 uc_ta_user_key_idx,
                                                    oal_uint32 ul_ta_user_key_len,
                                                    oal_uint8 *puc_ta_user_key,
                                                    oal_uint32 ul_vap_auth_supp_flag,
                                                    hal_rx_status_enum_uint8 *puc_rx_status,
                                                    oal_int32 encrypt_no_frag,
                                                    oal_uint8 *puc_tx_pn)
{
    struct ieee80211_tkip_data* pst_tkey = &g_st_tkip_root;
    oal_uint8 auc_rc4key[16], *puc_pos, *puc_icv;
    oal_uint32 ul_crc;
    oal_uint16 us_data_len = us_mac_frame_len - uc_mac_header_len;
    oal_uint32 ul_auth_supp_flag;
    oal_uint16 us_encrypt_len;

    /* not sure about that */
    ieee80211_tkip_set_tsc(*(oal_uint32*)(puc_tx_pn+4), *(oal_uint32*)puc_tx_pn);
    /* copy info to g_st_tkip_root */
    if (OAL_SUCC != ieee80211_tkip_set_key(puc_ta_user_key, ul_ta_user_key_len, OAL_PTR_NULL, uc_ta_user_key_idx, ul_vap_auth_supp_flag))
    {
        return 0;
    }

    ul_auth_supp_flag = pst_tkey->ul_auth_supp_flag;

    if (pst_tkey->flags & IEEE80211_CRYPTO_TKIP_COUNTERMEASURES)
    {
        return 0;
    }

    /* Calculate TKIP Header */
    /* tkip head need wepseed[1] */
    ieee80211_tkip_hdr(puc_mac_hdr_addr, us_data_len, puc_mac_body_addr, auc_rc4key, 16, &g_st_tkip_root);

    puc_pos = puc_mac_hdr_addr;

    puc_tx_pn[0]    = puc_pos[2];
    puc_tx_pn[1]    = puc_pos[0];
    puc_tx_pn[2]    = puc_pos[4];
    puc_tx_pn[3]    = puc_pos[5];
    puc_tx_pn[4]    = puc_pos[6];
    puc_tx_pn[5]    = puc_pos[7];

    if (encrypt_no_frag)
    {
        /* Calcuate Michael MIC */
        puc_pos = puc_mac_body_addr + 8 + us_data_len;

        //see 11.7.1 Mapping PTK to TKIP keys
        /* A STA shall use bits 0–127 of the temporal key as its input to the TKIP Phase 1 and Phase 2 mixing functions
         * A STA shall use bits 128–191 of the temporal key as the Michael key for MSDUs from the Authenticator’s
         *       STA to the Supplicant’s STA
         * A STA shall use bits 192–255 of the temporal key as the Michael key for MSDUs from the Supplicant’s
         *       STA to the Authenticator’s STA
         */
        //auth_supp_flag 0:auth，1:supp
        michael_mic(&pst_tkey->auc_key[16 + 8 * ul_auth_supp_flag], puc_mac_hdr_addr, puc_mac_body_addr + 8, us_data_len, puc_pos);
    }
    else
    {
        puc_pos = puc_mac_body_addr + 8 + us_data_len - 8;
    }

    puc_icv = puc_pos + 8; /* ICV appended after MIC */

    puc_pos = puc_mac_body_addr + 8; /* Point to payload when do CRC */

    //encrypt_no_frag 1:mic校验，0:没有mic
    if (encrypt_no_frag)
    {
        ul_crc = do_crc32(puc_pos, us_data_len + 8, 1);
    }
    else
    {
        ul_crc = do_crc32(puc_pos, us_data_len, 1);
    }
    puc_icv[0] = ul_crc & 0xff;
    puc_icv[1] = (ul_crc >> 8) & 0xff;
    puc_icv[2] = (ul_crc >> 16) & 0xff;
    puc_icv[3] = (ul_crc >> 24) & 0xff;

    arc4_set_key(auc_rc4key, 16);

    if (encrypt_no_frag)
    { us_encrypt_len = us_data_len + 8 + 4; }
    else
    { us_encrypt_len = us_data_len + 4; }
    puc_pos = puc_mac_body_addr + 8;
    do
    {
        arc4_crypt(puc_pos, puc_pos);
        puc_pos += 1;

    }
    while (us_encrypt_len--);

    if (encrypt_no_frag)
    { return us_data_len + 8 + 8 + 4; }
    else
    { return us_data_len + 8 + 4; }
}


oal_uint32 dmac_ieee80211_tkip_encrypt_msdu(oal_uint8 *puc_mac_hdr_addr,
                                                            oal_uint8 uc_mac_header_len,
                                                            oal_uint8 *puc_mac_body_addr,
                                                            oal_uint16 us_mac_frame_len)
{
    struct ieee80211_tkip_data* pst_tkey = &g_st_tkip_root;
    oal_uint8* puc_pos;
    oal_uint16 us_data_len = us_mac_frame_len - uc_mac_header_len;

    puc_pos = puc_mac_body_addr + us_data_len;
    michael_mic(&pst_tkey->auc_key[16], puc_mac_hdr_addr, puc_mac_body_addr, us_data_len, puc_pos);

    return us_data_len + 8;
}


oal_uint32 dmac_ieee80211_tkip_decrypt(oal_uint8 *puc_mac_hdr_addr,
                                                    oal_uint8 uc_mac_header_len,
                                                    oal_uint8 *puc_mac_body_addr,
                                                    oal_uint16 us_mac_frame_len,
                                                    oal_uint8 uc_ta_user_key_idx,
                                                    oal_uint32 ul_ta_user_key_len,
                                                    oal_uint8 *puc_ta_user_key,
                                                    oal_uint32 ul_vap_auth_supp_flag,
                                                    hal_rx_status_enum_uint8 *puc_rx_status,
                                                    oal_int32 decrypt_no_frag,
                                                    oal_int32 replay_detect_en)
{
    struct ieee80211_tkip_data* pst_tkey = &g_st_tkip_root;
    oal_uint8 auc_rc4key[16];
    oal_uint8 uc_keyidx, *puc_pos;
    oal_uint32 ul_iv32;
    oal_uint16 us_iv16;
    oal_uint8 auc_icv[4];
    oal_uint32 ul_crc;
    oal_uint16 us_plen;     /* encrypted data - icv */
    oal_uint16 us_data_len = us_mac_frame_len - uc_mac_header_len;
    oal_uint16 us_encrypted_len;
    oal_uint32 ul_auth_supp_flag;

    OAM_INFO_LOG2(0, OAM_SF_SOFT_CRYPTO, "CRYPTO TKIP Decryption. head len=%d, Payload len(include iv & tkip_header & mic & icv)=%d.\n",
        uc_mac_header_len,
        us_mac_frame_len);

    /* copy info to g_st_tkip_root */
    if (OAL_SUCC != ieee80211_tkip_set_key(puc_ta_user_key, ul_ta_user_key_len, OAL_PTR_NULL, uc_ta_user_key_idx, ul_vap_auth_supp_flag))
    {
        return 0;
    }

    /* get ta auth supp flag */
    ul_auth_supp_flag = !(pst_tkey->ul_auth_supp_flag);

    puc_pos = puc_mac_body_addr;
    /* octect 3, include resv(bit0-4)+ext_iv(bit5)+key_id(bit6-7) */
    uc_keyidx = puc_pos[3];

    /* check ext iv */
    if (!(uc_keyidx & (1 << 5)))
    {
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_tkip_decrypt received packet without ExtIV. drop it.\n");
        *puc_rx_status = HAL_RX_KEY_SEARCH_FAILURE;
        return 0;
    }

    uc_keyidx >>= 6;
    if (pst_tkey->l_key_idx != uc_keyidx)
    {
        OAM_WARNING_LOG2(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_tkip_decrypt frame key_idx neq, own key_idx=%d, frame "
               "keyidx=%d \n", pst_tkey->l_key_idx, uc_keyidx);
        *puc_rx_status = HAL_RX_KEY_SEARCH_FAILURE;
        return 0;
    }

    /* check g_st_tkip_root key info has been copied */
    if (!pst_tkey->l_key_set)
    {
        OAM_WARNING_LOG1(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_tkip_decrypt received packet"
               " with keyid=%d that does not have a configured"
               " key\n", uc_keyidx);
        *puc_rx_status = HAL_RX_KEY_SEARCH_FAILURE;
        return 0;
    }
    /* cal tsc */
    us_iv16 = (puc_pos[0] << 8) | puc_pos[2];
    ul_iv32 = puc_pos[4] | (puc_pos[5] << 8) | (puc_pos[6] << 16) | (puc_pos[7] << 24);
    /* Skip TKIP header */
    puc_pos += 8;

    /* replay detect on && not a retry frame && replay check return 1 */
    if (replay_detect_en
        && (!(((mac_ieee80211_frame_stru*)puc_mac_hdr_addr)->st_frame_control.bit_retry))
        && (tkip_replay_check(ul_iv32, us_iv16, pst_tkey->ul_rx_iv32, pst_tkey->us_rx_iv16)))
    {
        OAM_WARNING_LOG4(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_tkip_decrypt replay detected: "
               " previous TSC %08x%04x received TSC "
               "%08x%04x\n", pst_tkey->ul_rx_iv32, pst_tkey->us_rx_iv16, ul_iv32, us_iv16);

        pst_tkey->ul_dot11RSNAStatsTKIPReplays++;
        *puc_rx_status = HAL_RX_TKIP_REPLAY_FAILURE;
        return 0;
    }

    /* almost every 2**16=65535 frames need recal ttak or phase1 never done before */
    if (ul_iv32 != pst_tkey->ul_rx_iv32 || !pst_tkey->l_rx_phase1_done)
    {
        /* generate ttak */
        tkip_mixing_phase1(pst_tkey->aus_rx_ttak, pst_tkey->auc_key, ((mac_ieee80211_frame_stru*)puc_mac_hdr_addr)->auc_address2, ul_iv32);
        pst_tkey->l_rx_phase1_done = 1;
    }
    /* generate wepseed */
    tkip_mixing_phase2(auc_rc4key, pst_tkey->auc_key, pst_tkey->aus_rx_ttak, us_iv16);

    arc4_set_key(auc_rc4key, 16);

    us_encrypted_len = us_data_len - 8;
    puc_pos = puc_mac_body_addr + 8;
    do
    {
        arc4_crypt(puc_pos, puc_pos);
        puc_pos += 1;
    }
    while (us_encrypted_len--);

    us_plen = us_data_len - 8 - 4;

    puc_pos = puc_mac_body_addr + 8; /* Point to payload when do CRC */
    ul_crc = do_crc32(puc_pos, us_plen, 1);
    auc_icv[0] = ul_crc & 0xff;
    auc_icv[1] = (ul_crc >> 8) & 0xff;
    auc_icv[2] = (ul_crc >> 16) & 0xff;
    auc_icv[3] = (ul_crc >> 24) & 0xff;

    if (oal_memcmp(auc_icv, puc_pos + us_plen, 4) != 0)
    {
        if (ul_iv32 != pst_tkey->ul_rx_iv32)
        {
            /* Previously cached Phase1 result was already lost, so
             * it needs to be recalculated for the next packet. */
            pst_tkey->l_rx_phase1_done = 0;
        }
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_tkip_decrypt ICV error detected!\n");

        pst_tkey->ul_dot11RSNAStatsTKIPICVErrors++;
        if (HAL_RX_SUCCESS == *puc_rx_status)
        {
            *puc_rx_status = HAL_RX_ICV_FAILURE;
        }
    }

    /* Update real counters only after Michael MIC verification has
     * completed */
    pst_tkey->ul_rx_iv32_new = ul_iv32;
    pst_tkey->us_rx_iv16_new = us_iv16;

    /* Remove IV and ICV */
    oal_memmove(puc_mac_body_addr, puc_mac_body_addr + 8, us_data_len - 4 - 8);

    /* check mic field */
    if (decrypt_no_frag)
    {
        /* Calculate Michael MIC */
        oal_uint8 auc_mic[8];
        puc_pos = puc_mac_body_addr;
        /* auth use hi8, supplicant use lo8 */
        michael_mic(&pst_tkey->auc_key[16 + 8 * ul_auth_supp_flag], puc_mac_hdr_addr, puc_pos, us_data_len - 8 - 8 - 4, auc_mic);

        if (oal_memcmp(auc_mic, puc_pos + us_data_len - 8 - 8 - 4, 8) != 0)
        {
            OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_tkip_decrypt Michael MIC verification failed!\n");
            pst_tkey->ul_dot11RSNAStatsTKIPLocalMICFailures++;
            pst_tkey->flags |= IEEE80211_CRYPTO_TKIP_COUNTERMEASURES;
            if (HAL_RX_SUCCESS == *puc_rx_status)
            {
                *puc_rx_status  = HAL_RX_TKIP_MIC_FAILURE;
            }
        }
    }

    /* Update TSC counters for RX now that the packet verification has
     * completed. */
    if (HAL_RX_SUCCESS == *puc_rx_status)	//20140811
    {
        pst_tkey->ul_rx_iv32 = pst_tkey->ul_rx_iv32_new;
        pst_tkey->us_rx_iv16 = pst_tkey->us_rx_iv16_new;
    }

    if (decrypt_no_frag)
    { return us_data_len - 8 - 8 - 4; } /* TKIP header, MIC, ICV */
    else
    { return us_data_len - 8 - 4; } /* TKIP header, ICV */
}



oal_int32 dmac_ieee80211_tkip_decrypt_msdu(oal_uint8 *puc_mac_hdr_addr,
                                                            oal_uint8 uc_mac_header_len,
                                                            oal_uint8 *puc_mac_body_addr,
                                                            oal_uint16 us_mac_frame_len,
                                                            hal_rx_status_enum_uint8 *puc_rx_status)
{
    struct ieee80211_tkip_data* pst_tkey = &g_st_tkip_root;
    oal_uint8 auc_mic[8];
    oal_uint8* puc_pos;
    oal_uint16 us_data_len = us_mac_frame_len - uc_mac_header_len;

    if (!pst_tkey->l_key_set)
    {
        *puc_rx_status = HAL_RX_KEY_SEARCH_FAILURE;
        return 0;
    }

    puc_pos = puc_mac_body_addr;
    michael_mic(&pst_tkey->auc_key[24], puc_mac_hdr_addr, puc_pos, us_data_len - 8, auc_mic);

    if (oal_memcmp(auc_mic, puc_mac_body_addr + us_data_len - 8, 8) != 0)
    {
        OAM_WARNING_LOG0(0, OAM_SF_SOFT_CRYPTO, "dmac_ieee80211_tkip_decrypt_msdu Michael MIC verification failed for MSDU.\n");
        pst_tkey->ul_dot11RSNAStatsTKIPLocalMICFailures++;
        *puc_rx_status = HAL_RX_TKIP_MIC_FAILURE;
        return 0;
    }

    /* Update TSC counters for RX now that the packet verification has
     * completed. */
    pst_tkey->ul_rx_iv32 = pst_tkey->ul_rx_iv32_new;
    pst_tkey->us_rx_iv16 = pst_tkey->us_rx_iv16_new;

    *puc_rx_status = HAL_RX_SUCCESS;

    return us_data_len - 8;
}


#endif /* #ifdef _PRE_WLAN_FEATURE_SOFT_CRYPTO */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

