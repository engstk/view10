


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_SOFT_CRYPTO
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "dmac_crypto_comm.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_CRYPTO_COMM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
OAL_STATIC oal_uint32 g_ul_crc_table[256] = {0};
OAL_STATIC struct arc4_ctx arc4_ctx_root;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
oal_void arc4_set_key(OAL_CONST oal_uint8 *puc_key, oal_uint32 ul_key_len)
{
    struct arc4_ctx *pst_ctx = &arc4_ctx_root;
    oal_uint32 i, j = 0, k = 0;

    pst_ctx->x = 1;
    pst_ctx->y = 0;

    for (i = 0; i < 256; i++)
    { pst_ctx->S[i] = i; }

    for (i = 0; i < 256; i++)
    {
        oal_uint8 a = pst_ctx->S[i];
        j = (j + puc_key[k] + a) & 0xff;
        pst_ctx->S[i] = pst_ctx->S[j];
        pst_ctx->S[j] = a;
        if (++k >= ul_key_len)
        { k = 0; }
    }
}

oal_void arc4_crypt(oal_uint8 *out, OAL_CONST oal_uint8 *in)
{
    struct arc4_ctx *pst_ctx = &arc4_ctx_root;

    oal_uint8* const S = pst_ctx->S;
    oal_uint8 x = pst_ctx->x;
    oal_uint8 y = pst_ctx->y;
    oal_uint8 a, b;

    a = S[x];
    y = (y + a) & 0xff;
    b = S[y];
    S[x] = b;
    S[y] = a;
    x = (x + 1) & 0xff;
    *out = *in ^ S[(a + b) & 0xff];

    pst_ctx->x = x;
    pst_ctx->y = y;
}

oal_uint32 do_crc32(oal_uint8 *puc_data, oal_uint32 ul_data_len, oal_uint8 uc_comp)
{
    oal_uint32 i = 0 , j = 0;
    oal_uint32 ul_crc  = 0;
    oal_uint32 ul_poly = 0xEDB88320;

    /* Generate table for all 8 bit numbers i.e., 256 entries                */
    for (i = 0; i < 256; i++)
    {
        ul_crc = i;

        for (j = 8; j > 0; j--)
        {
            if (ul_crc & 1)
            { ul_crc = (ul_crc >> 1) ^ ul_poly; }
            else
            { ul_crc >>= 1; }
        }

        /* Update CRC table for ith entry                                    */
        g_ul_crc_table[i] = ul_crc;
    }

    ul_crc = 0xFFFFFFFF;
    /* Byte by byte processing of the input data using CRC table to          */
    /* accumulate resultant CRC                                              */
    for (i = 0; i < ul_data_len; i++)
        ul_crc = ((ul_crc >> 8) & 0x00FFFFFF) ^
              g_ul_crc_table[ (ul_crc ^ puc_data[i]) & 0xFF ];

    if (uc_comp)
    { ul_crc = ul_crc ^ 0xFFFFFFFF; }

    return ul_crc;
}

#endif /* #ifdef _PRE_WLAN_FEATURE_SOFT_CRYPTO */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
