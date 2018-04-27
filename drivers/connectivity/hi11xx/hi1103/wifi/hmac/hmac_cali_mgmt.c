

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)

/*****************************************************************************
  1 篓陋隆陇???t隆茫篓鹿o?
*****************************************************************************/
#include "hmac_cali_mgmt.h"
#include "oal_kernel_file.h"
#include "plat_cali.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_MGMT_C

extern oal_uint32 wlan_pm_close(oal_void);

#ifdef _PRE_WLAN_ONLINE_DPD
oal_uint32 gul_corram_data[DPD_CORRAM_DATA_NUM] =
{
    0x1FBD017C,
    0xCD00F7,
    0x104009D,
    0x600034,
    0x3C0004,
    0x18006E,
    0x1EA30142,
    0x1D42023A,
    0x1E970376,
    0x1B30448,
    0x242039F,
    0x1F3A0158,
    0x1D021F16,
    0x1E8B1E89,
    0x1241F62,
    0x1620046,
    0x5E00FD,
    0x8101F7,
    0xB50266,
    0x1EED00D3,
    0x1D241E24,
    0x1F061D79,
    0x38C1FBD,
    0x62C015E,
    0x5421F54,
    0x3A51C0C,
    0x2EF1CAF,
    0x162012A,
    0x1E8B03DD,
    0x1D79012E,
    0x1F621C4A,
    0xEB1A81,
    0x1F7D1C61,
    0x1D891E87,
    0x1E2C1F58,
    0x1FD51FED,
    0x1FA700B9,
    0x1EF81FED,
    0x781CF2,
    0x1EF1A2E,
    0x1F301A06,
    0x19F81B87,
    0x19C11C8F,
    0x4E1D38,
    0x5ED1EFC,
    0x3C501AF,
    0x1D320378,
    0x1A7F03CD,
    0x1DC903BB,
    0x1A703F9,
    0x1780336,
    0x1EB900C9,
    0x1D891E83,
    0x1F791E3A,
    0x28B1F14,
    0x3A51EDD,
    0x1581D1A,
    0x1DD11BC1,
    0x1D461C2E,
    0x1121E1C,
    0x4A50040,
    0x2BF01FD,
    0x1D1C0354,
    0x1A91043E,
    0x1E0604E9,
    0x2B104FD,
    0x2A703C5,
    0x1ECF0100,
    0x1CD71E54,
    0x1F521E14,
    0x31C0008,
    0x472011A,
    0x3321F0C,
    0x1891C10,
    0x1201C16,
    0x1E91FA1,
    0x22C02C9,
    0x2C028B,
    0x1CD3009D,
    0x1B6D0064,
    0x1D810270,
    0x9503DD,
    0x13402E3,
    0x1F8B009F,
    0x1EB11F46,
    0x1F851F93,
    0x1FED0087,
    0x1F40016A,
    0x1FCB0228,
    0x1E902B7,
    0x26C0299,
    0x1F6C0197,
    0x1C6D1FFE,
    0x1DB71E77,
    0x1F91DA9,
    0x3F91D4C,
    0x1A71D08,
    0x1E9F1D0C,
    0x1E971DEE,
    0xAD1F83,
    0x19F00D3,
    0x5600FD,
    0x1E52006A,
    0x1DB7008B,
    0x1F5801B7,
    0x1F102A1,
    0x27A0295,
    0x1F3C021C,
    0x1B790154,
    0x1C691EB7,
    0x18D19EE,
    0x3481675,
    0x1D4017E8,
    0x15AD1CA1,
    0x16161EC3,
    0x1E6C1C3E,
    0x5561A57,
    0x5261E34,
    0x25A051A,
    0x2AB06F9,
    0x4660195,
    0x34E1B54,
    0xED1AA9,
    0x1D91E62,
    0x49D0183,
    0x32802F3,
    0x1D000513,
    0x1A6B07FB,
    0x970813,
    0x8E3050F,
    0x8DB02A7,
    0xD70350,
    0x1A6D03DB,
    0x1B6500B1,
    0x1F811BB3,
    0x361A2E,
    0x1DFE1D64,
    0x1D2C00E7,
    0x1EC50162,
    0x1FC90068,
    0x1F360024,
    0x1EC11FC7,
    0x1F4E1E1C,
    0x1F321CAF,
    0x1DB31E50,
    0x1C8F0195,
    0x1D2E0289,
    0x1E441FC7,
    0x1EF41D04,
    0x6A1D85,
    0x3051FC1,
    0x48A1FFE,
    0x2B11D81,
    0x1F621B7D,
    0x1E8D1C61,
    0xC11F0E,
    0x250010E,
    0xF70122,
    0x1EDD0012,
    0x1E811F3E,
    0x1EC51F48,
    0x1E001FAB,
    0x1D991F6A,
    0x1FCF1E81,
    0x2BD1E56,
    0x284005A,
    0x1F700378,
    0x1E08051E,
    0xC3040D,
    0x4160201,
    0x39F0108,
    0x160128,
    0x1DC500C7,
    0x1EBD1FAF,
    0xF91F2C,
    0x1CD1F7F,
    0x12A1F58,
    0xB71E7B,
    0xE91E75,
    0xF70024,
    0x5801CB,
    0x1F500138,
    0x1E9F1F22,
    0x1E681E22,
    0x1E561F46,
    0x1E420078,
    0x1EC50036,
    0x321F6A,
    0x1911FB7,
    0xE100D3,
    0x1E0E0187,
    0x1C3201F3,
    0x1DE10326,
    0xF70492,
    0x15A0405,
    0x1ECD010C,
    0x1D501E2C,
    0x1F301E10,
    0x1E90010,
    0x28D0164,
    0x1F500DB,
    0x23A0014,
    0x2340083,
    0x1FDD015C,
    0x1D060100,
    0x1DBB1F02,
    0x1BB1CCF,
    0x3E31BC3,
    0x1341C04,
    0x1D481C8F,
    0x1D0A1D32,
    0x381E64,
    0x28F004E,
    0x1A9017A,
    0x1EE700BD,
    0x1C951F7D,
    0x1BBD0030,
    0x1C140268,
    0x1D1E02F7,
    0x1E2E0087,
    0x1F1A1DD5,
    0x3A1DB5,
    0x1661F26,
    0x1741FB1,
    0x1FC31F28,
    0x1D9D1F3A,
    0x1D541FA7,
    0x1F661F1E,
    0x18D1E44,
    0x1A51F3C,
    0x3E01C9,
    0x1F7F0330,
    0x58028F,
    0x19301C7,
    0x11001A5,
    0x1F000087,
    0x1DDB1E5A,
    0x1FBD1DE1,
    0x3601FFC,
    0x5540148,
    0x41C1ECF,
    0x1C31B4C,
    0xE51B9D,
    0x1971FA5,
    0x1A50297,
    0x1601D7,
    0x1E641FE7,
    0x1E501F99,
    0x1F8F0062,
    0x780042,
    0x851FA5,
    0x6C1FF8,
    0x7D00DF,
    0x102,
    0x1EB70044,
    0x1DCF0030,
    0x1ED50132,
    0xE30228,
    0x17E01F3,
    0x1F9B0118,
    0x1D970046,
    0x1E4A1F2C,
    0x11E1D9F,
    0x2661C52,
    0x1FE31C1E,
    0x1C521CA9,
    0x1C7D1D50,
    0xDF1E5E,
    0x4620042,
    0x22E0201,
    0x1C28027E,
    0x191002C9,
    0x1C2604C3,
    0x10A0740,
    0x1830670,
    0x1D9F0174,
    0x1B711CB5,
    0x1ECB1C73,
    0x4381F0E,
    0x5E91FFA,
    0x3091E12,
    0x1FCF1CA1,
    0x1F3E1DB3,
    0x401F93,
    0x890014,
    0x4C1FB1,
    0xFB1FE5,
    0x21E0000,
    0x1AB1F02,
    0x1FFA1D97,
    0x1F401DB5,
    0x661F0E,
    0x1990012,
    0x138009B,
    0x260176,
    0x30022E,
    0x18F012C,
    0x27C1F26,
    0x1F71E95,
    0xBF001A,
    0x1FE70154,
    0x1FBD00CF,
    0x2A0020,
    0x1040046,
    0x1D31F8B,
    0x1FB1D52,
    0x1E71CA7,
    0x2400006,
    0x2140420,
    0x1F6E03B9,
    0x1ADC1FB9,
    0x184F1E79,
    0x1AC50295,
    0x1F9106EB,
    0x146061B,
    0x1E8301BF,
    0x1C041EC9,
    0x1D9F1E24,
    0x10E1D4A,
    0x1701C12,
    0x1E581D22,
    0x1BAF0052,
    0x1C4A01B3,
    0x1E931F85,
    0x161D1A,
    0xDF1DDF,
    0x1FD0064,
    0x2AD00FD,
    0x1991F50,
    0x141E36,
    0x1F911F0C,
    0x1F951FE5,
    0x1F3E1F9D,
    0x1FD51EE5,
    0x2841E81,
    0x4B71D66,
    0x2061AE6,
    0x1C0E189F,
    0x1ADE18FA,
    0x2361C32,
    0x96C0072,
    0x5A30462,
    0x199B07D5,
    0x143F0ADB,
    0x1D520C60,
    0x9E70B7E,
    0xA4C0894,
    0x1E8B04F9,
    0x15C20124,
    0x19DA1D2A,
    0x39719AD,
    0x630186D,
    0x3819F4,
    0x1ABF1CCD,
    0x1BC51EA1,
    0x1FE71F16,
    0x18F1F42,
    0x741F9B,
    0x1FCF1F83,
    0xC1EB3,
    0x1F2A1EE7,
    0x1D930024,
    0x1D6200C7,
    0x1EA71F1C,
    0x121D48,
    0x1541E2E,
    0x2DB00D7,
    0x3F50162,
    0x2CF1E7D,
    0x1FBD1BCF,
    0x1E4E1C28,
    0xC31E20,
    0x3BF1F68,
    0x2F90102,
    0x1FAF0454,
    0x1F4206CF,
    0x36204B3,
    0x7821FD9,
    0x6A51E50,
    0x1CD01D1,
    0x1DDB04AD,
    0x1D420232,
    0x1E321D4C,
    0x1E561BFC,
    0x1E0E1EAB,
    0x1E8B00EB,
    0x1FB5009B,
    0x60001A,
    0x2A00C9,
    0x1F5000E3,
    0x1E5A1F77,
    0x1DB51E97,
    0x1E180024,
    0x1F42023A,
    0xE0246,
    0x1F6200BB,
    0x1E6F1FB1,
    0x1EEB1F46,
    0x4E1E66,
    0x8B1D8F,
    0x1F021E1A,
    0x1DBD1FB3,
    0x1E440074,
    0x1F8F1FE9,
    0x1FFE1F89,
    0x1FC71F7F,
    0x141E9D,
    0x281D22,
    0x1E2A1D3C,
    0x1B101F14,
    0x1ADA1FBD,
    0x1F301DC1,
    0x3E91C6F,
    0x3DD1F7F,
    0x1FE90513,
    0x1E2E07F3,
    0x1C50609,
    0x65802DF,
    0x58E01DB,
    0x1FBF022A,
    0x1B750128,
    0x1D4E1ECB,
    0x2641D0E,
    0x46C1CAB,
    0x1851C85,
    0x1DE91C4E,
    0x1DD71D00,
    0xCD1EBB,
    0x2AF0050,
    0x1500114,
    0x1EE301A1,
    0x1E850206,
    0x7201BB,
    0x22C0099,
    0x1ED0004,
    0x54008F,
    0x1F460110,
    0x1F66007D,
    0x1FAB1FA1,
    0x1FA51FC7,
    0x1C0040,
    0x1340024,
    0x1B31FF1,
    0x9500F7,
    0x1EC9022A,
    0x1E1200D5,
    0x1EA11D22,
    0x1E951AF2,
    0x1CC71D10,
    0x1AE800B3,
    0x1BBB014A,
    0x1F1C1ECB,
    0x1F91D7D,
    0x1C91F68,
    0x1F8301E3,
    0x1E2A026A,
    0x1F060214,
    0x640278,
    0x360276,
    0x1EC10081,
    0x1E3A1E2E,
    0x1FD71E18,
    0x2281F9D,
    0x2BF0018,
    0xF71F1C,
    0x1ECB1EC7,
    0x1E321F5C,
    0x1F641EEF,
    0xDB1DB9,
    0x1B11EB5,
    0x2740254,
    0x3A5042A,
    0x474011C,
    0x3F51C85,
    0x2931C1E,
    0x19B1FD5,
    0x1240291,
    0x3201F9,
    0x1E9F00DD,
    0x1E5201A5,
    0xA70268,
    0x3B7014A,
    0x3F1003C,
    0xAF016E,
    0x1D6F02A7,
    0x1D6600BF,
    0x1FC31CE8,
    0xA71B14,
    0x1EB71CAD,
    0x1C5E1F00,
    0x1CE41F7D,
    0x1FE51E9F,
    0x2361E5E,
    0x1DB1FFC,
    0x640309,
    0xC704AD,
    0x3AD026A,
    0x6741E24,
    0x6171D71,
    0x262023E,
    0x1E20068C,
    0x1C240340,
    0x1C041ACD,
    0x1B971730,
    0x1A281BF6,
    0x19B70201,
    0x1BF80262,
    0x1F7D1FBD,
    0x1201FB1,
    0x1F99009B,
    0x1CC11DE1,
    0x1A2C1A87

};
#endif
/*****************************************************************************
  2 o隆楼篓潞y篓娄篓麓?隆脗
*****************************************************************************/

/*****************************************************************************
  3 o隆楼篓潞y篓潞娄脤??
*****************************************************************************/
oal_void hmac_add_bound(oal_uint32 *pul_number, oal_uint32 ul_bound)
{
    *pul_number = *pul_number + 1;

    if (*pul_number > (ul_bound - 1))
    {
        *pul_number -= ul_bound;
    }
}

/*lint -e571*/
/*lint -e801*/
/*lint -e416*/

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
OAL_STATIC oal_int32 hmac_print_2g_cail_result(oal_uint8 uc_cali_chn_idx,
                                                  oal_int8 *pc_print_buff,
                                                  oal_int32 l_remainder_len,
                                                  hi1103_cali_param_stru *pst_cali_data)
{

    oal_int8 *pc_string;

    if (l_remainder_len <= 0)
    {
        OAM_ERROR_LOG3(0, OAM_SF_CALIBRATE, "hmac_print_2g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
                        l_remainder_len, OAM_REPORT_MAX_STRING_LEN, OAL_SIZEOF(hi1103_cali_param_stru));
        return l_remainder_len;
    }

    pc_string = "2G: cali data index[%u]\n"
                "siso_rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]digital_rxdc_i_q[0x%x 0x%x]\n"
                "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
                "tx_power[ppa:%u atx_pwr:%u dtx_pwr:%u dp_init:%d]\n"
                "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
                "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return OAL_SPRINTF(pc_print_buff, (oal_uint32)l_remainder_len, pc_string,
    uc_cali_chn_idx,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[0],
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[1],
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[2],
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[3],
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[4],
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[5],
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[6],
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[7],
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_q,

    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_logen_cmp.uc_ssb_cmp,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_logen_cmp.uc_buf0_cmp,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_logen_cmp.uc_buf1_cmp,

    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_2G.uc_ppa_cmp,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_2G.uc_atx_pwr_cmp,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_2G.uc_dtx_pwr_cmp,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_2G.c_dp_init,

    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[0].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[0].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[1].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[1].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[2].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[2].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[3].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[3].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[4].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[4].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[5].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[5].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[6].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[6].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[7].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[7].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[8].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[8].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[9].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[9].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[10].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[10].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[11].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[11].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[12].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[12].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[13].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[13].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[14].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[14].us_txdc_cmp_q,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[15].us_txdc_cmp_i,
    pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[15].us_txdc_cmp_q);
}

OAL_STATIC oal_int32 hmac_print_5g_cail_result(oal_uint8 uc_cali_chn_idx,
                                                  oal_int8 *pc_print_buff,
                                                  oal_int32 l_remainder_len,
                                                  hi1103_cali_param_stru *pst_cali_data)
{

    oal_int8 *pc_string;

    if (l_remainder_len <= 0)
    {
        OAM_ERROR_LOG3(0, OAM_SF_CALIBRATE, "hmac_print_5g_cail_result:check size remain len[%d] max size[%d] check cali_parm[%d]",
                        l_remainder_len, OAM_REPORT_MAX_STRING_LEN, OAL_SIZEOF(hi1103_cali_param_stru));
        return l_remainder_len;
    }

    pc_string = (uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM)
        ? "5G 20M: cali data index[%u]\n"
          "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "digital_rxdc_i_q[0x%x 0x%x]\n"
          "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
          "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
          "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
          "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n"
        : "5G 80M: cali data index[%u]\n"
          "siso rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] digital_rxdc_i_q[0x%x 0x%x]\n"
          "mimo_extlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "mimo_intlna rx_dc_comp analog0~7[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] \n"
          "digital_rxdc_i_q[0x%x 0x%x]\n"
          "cali_logen_cmp ssb[%u]buf_0_1[%u %u]\n"
          "tx_power[ppa:%u mx:%u atx_pwr:%u, dtx_pwr:%u] ppf:%u\n"
          "tx_dc siso_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]"
          "mimo_i_q[0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x][0x%x 0x%x]\n";

    return OAL_SPRINTF(pc_print_buff, (oal_uint32)l_remainder_len, pc_string,
    uc_cali_chn_idx,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[0],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[1],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[2],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[3],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[4],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[5],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[6],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_siso_cmp[7],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[0],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[1],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[2],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[3],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[4],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[5],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[6],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_extlna_cmp[7],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[0],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[1],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[2],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[3],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[4],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[5],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[6],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.aus_analog_rxdc_mimo_intlna_cmp[7],
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_q,

    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_logen_cmp.uc_ssb_cmp,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_logen_cmp.uc_buf0_cmp,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_logen_cmp.uc_buf1_cmp,

    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_5G.uc_ppa_cmp,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_5G.uc_mx_cmp,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_5G.uc_atx_pwr_cmp,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_5G.uc_dtx_pwr_cmp,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_ppf_cmp_val.uc_ppf_val,

    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[0].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[0].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[1].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[1].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[2].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[2].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[3].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[3].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[4].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[4].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[5].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[5].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[6].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[6].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[7].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[7].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[8].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[8].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[9].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[9].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[10].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[10].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[11].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[11].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[12].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[12].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[13].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[13].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[14].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[14].us_txdc_cmp_q,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[15].us_txdc_cmp_i,
    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].ast_txdc_cmp_val[15].us_txdc_cmp_q);
}

oal_void hmac_dump_cali_result(oal_void)
{
    hi1103_cali_param_stru *pst_cali_data;
    oal_uint8               uc_cali_chn_idx_1;
    oal_uint8               uc_cali_chn_idx;
    oal_int8               *pc_print_buff;
    oal_int32               l_string_tmp_len = 0;
    oal_uint32              ul_string_len    = 0;
    oal_uint8               uc_chan_idx;

    OAM_INFO_LOG0(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::START.}");

    /* OTA上报 */
    pc_print_buff = (oal_int8 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (OAL_PTR_NULL == pc_print_buff)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CALIBRATE, "{hmac_dump_cali_result::pc_print_buff null.}", OAM_REPORT_MAX_STRING_LEN);
        return;
    }

    pst_cali_data = (hi1103_cali_param_stru *)get_cali_data_buf_addr();

    /*lint -e734*/
    for (uc_chan_idx = 0; uc_chan_idx < WLAN_RF_CHANNEL_NUMS; uc_chan_idx++)
    {
        pst_cali_data += uc_chan_idx;

        OAM_WARNING_LOG3(0, OAM_SF_CALIBRATE, "chan:%d, cali_data dog_tag:%d, check_hw_status:%d",
                uc_chan_idx, pst_cali_data->ul_dog_tag, pst_cali_data->ul_check_hw_status);
        OAM_WARNING_LOG4(0, OAM_SF_CALIBRATE, "update info tmp:%d, chan_idx_1_2[%d %d],cali_time:%d",
                pst_cali_data->st_cali_update_info.bit_temperature,
                pst_cali_data->st_cali_update_info.uc_5g_chan_idx1,
                pst_cali_data->st_cali_update_info.uc_5g_chan_idx2,
                pst_cali_data->st_cali_update_info.ul_cali_time);
        OAM_WARNING_LOG4(0, OAM_SF_CALIBRATE, "st_rc_r_c_cali_data save_all:%d, c_code[0x%x], rc_code[0x%x] r_code[0x%x]",
                pst_cali_data->en_save_all,
                pst_cali_data->st_rc_r_c_cali_data.uc_c_cmp_code,
                pst_cali_data->st_rc_r_c_cali_data.uc_rc_cmp_code,
                pst_cali_data->st_rc_r_c_cali_data.uc_r_cmp_code);

        OAM_WARNING_LOG2(0, OAM_SF_CALIBRATE, "st_rc_r_c_cali_data save_all:classa[0x%x], classb[0x%x]",
                pst_cali_data->st_pa_ical_cmp.uc_classa_cmp,
                pst_cali_data->st_pa_ical_cmp.uc_classb_cmp);

        /*TBD: new_txiq_comp_val_stru TO BE ADDED*/

        /* 2.4g 不超过ota上报最大字节分3次输出 */
        for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 <= OAL_2G_CHANNEL_NUM/3; uc_cali_chn_idx_1++)
        {
            for (uc_cali_chn_idx = uc_cali_chn_idx_1*3; uc_cali_chn_idx < OAL_MIN(OAL_2G_CHANNEL_NUM, (uc_cali_chn_idx_1+1)*3); uc_cali_chn_idx++)
            {
                l_string_tmp_len = hmac_print_2g_cail_result(uc_cali_chn_idx, pc_print_buff + ul_string_len,
                                                             (oal_int32)(OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1), pst_cali_data);

                if (l_string_tmp_len <= 0)
                {
                    goto sprint_fail;
                }
                ul_string_len += (oal_uint32)l_string_tmp_len;
            }

            pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
            oam_print(pc_print_buff);
            OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
            ul_string_len = 0;
        }

       /* 5g */
        if (OAL_FALSE == mac_device_check_5g_enable_per_chip())
        {
            continue;
        }

        /* 5g 不超过ota上报最大字节分3次输出 */
        for (uc_cali_chn_idx_1 = 0; uc_cali_chn_idx_1 <= OAL_5G_CHANNEL_NUM/3; uc_cali_chn_idx_1++)
        {
            for (uc_cali_chn_idx = uc_cali_chn_idx_1*3; uc_cali_chn_idx < OAL_MIN(OAL_5G_CHANNEL_NUM, (uc_cali_chn_idx_1+1)*3); uc_cali_chn_idx++)
            {
                l_string_tmp_len = hmac_print_5g_cail_result(uc_cali_chn_idx, pc_print_buff + ul_string_len,
                                                        (oal_int32)(OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1), pst_cali_data);

                if (l_string_tmp_len <= 0)
                {
                    goto sprint_fail;
                }
                ul_string_len += (oal_uint32)l_string_tmp_len;
            }

            pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
            oam_print(pc_print_buff);
            OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
            ul_string_len = 0;
        }
    }
    /*lint +e734*/

    OAL_MEM_FREE(pc_print_buff, OAL_TRUE);
    return;

sprint_fail:

    OAM_WARNING_LOG0(0, OAM_SF_CFG, "{hmac_dump_cali_result:: OAL_SPRINTF return error!}");
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);
    OAL_MEM_FREE(pc_print_buff, OAL_TRUE);

    return;
}

oal_uint32 hmac_send_cali_data(mac_vap_stru *pst_mac_vap)
{
    frw_event_mem_stru       *pst_event_mem;
    frw_event_stru           *pst_event;
    dmac_tx_event_stru       *pst_h2d_cali_event;
    oal_netbuf_stru          *pst_netbuf_cali_data;
    oal_uint8                *puc_cali_data;
    hi1103_cali_param_stru   *pst_cali_data;
    oal_uint8                *puc_param;
    oal_uint32                ul_ret;
    oal_uint16                us_frame_len;
    oal_int32                 l_remain_len;
    oal_uint8                 uc_chan_idx;

    OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "hmac_send_cali_data:start!");

    pst_cali_data = (hi1103_cali_param_stru *)get_cali_data_buf_addr();

    hmac_dump_cali_result();

    pst_netbuf_cali_data = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF, WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);

    if (OAL_PTR_NULL == pst_netbuf_cali_data)
    {
       OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "{hmac_send_cali_data::pst_netbuf alloc null SIZE[%d].}", WLAN_LARGE_NETBUF_SIZE);
       return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    for (uc_chan_idx = 0; uc_chan_idx < WLAN_RF_CHANNEL_NUMS; uc_chan_idx++)
    {
        l_remain_len  = RF_SINGLE_CHAN_CALI_DATA_BUF_LEN; /* 单通道下发wifi校准长度 */
        pst_cali_data += uc_chan_idx;

        if (OAL_FALSE == pst_cali_data->en_save_all)
        {
            /* FPGA上校准数据不做上传下发，直接返回 */
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "hmac_send_cali_data::ch[%d] no cali data is delivered to host yet", uc_chan_idx);
            return OAL_FAIL;
        }
        puc_param     = (oal_uint8 *)(pst_cali_data->ast_2Gcali_param) + l_remain_len;

        while (l_remain_len > 0)
        {
            pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
            if (OAL_PTR_NULL == pst_event_mem)
            {
                OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "{hmac_send_cali_data::pst_event_mem null.}");
                oal_netbuf_free(pst_netbuf_cali_data);

                return OAL_ERR_CODE_PTR_NULL;
            }

            pst_event = frw_get_event_stru(pst_event_mem);
            FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                               FRW_EVENT_TYPE_WLAN_CTX,
                               DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_HMAC2DMAC,
                               OAL_SIZEOF(dmac_tx_event_stru),
                               FRW_EVENT_PIPELINE_STAGE_1,
                               pst_mac_vap->uc_chip_id,
                               pst_mac_vap->uc_device_id,
                               pst_mac_vap->uc_vap_id);

            OAL_MEMZERO(oal_netbuf_cb(pst_netbuf_cali_data), OAL_TX_CB_LEN);
            puc_cali_data = (oal_uint8 *)(OAL_NETBUF_DATA(pst_netbuf_cali_data));

            /* netbuf len 不能超过1544字节 */
            us_frame_len = (oal_uint16)OAL_MIN(WLAN_LARGE_NETBUF_SIZE - 104, l_remain_len);
            /*lint -e662*/
            oal_memcopy(puc_cali_data, puc_param - l_remain_len, (oal_uint32)us_frame_len);
            /*lint +e662*/
            pst_h2d_cali_event               = (dmac_tx_event_stru *)pst_event->auc_event_data;
            pst_h2d_cali_event->pst_netbuf   = pst_netbuf_cali_data;
            pst_h2d_cali_event->us_frame_len = us_frame_len;
            pst_h2d_cali_event->us_remain    = (oal_uint16)l_remain_len;

            l_remain_len -= (WLAN_LARGE_NETBUF_SIZE - 104);

            ul_ret = frw_event_dispatch_event(pst_event_mem);
            if (OAL_UNLIKELY(OAL_SUCC != ul_ret))
            {
                OAM_ERROR_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE,
                            "{hmac_send_cali_data::dispatch event fail ret[%d],l_remain_len[%d]frame_len[%d].}",
                            ul_ret, l_remain_len, us_frame_len);

                oal_netbuf_free(pst_netbuf_cali_data);
                FRW_EVENT_FREE(pst_event_mem);
                return OAL_FAIL;
            }

            FRW_EVENT_FREE(pst_event_mem);
        }
    }

    OAM_INFO_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "hmac_send_cali_data:end!");
    oal_netbuf_free(pst_netbuf_cali_data);

    return OAL_SUCC;
}

oal_uint32 hmac_save_cali_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru                  *pst_event;
    hal_cali_hal2hmac_event_stru    *pst_cali_save_event;
    hal_cali_hal2hmac_payload_stru  *pst_pay_load;
    oal_uint8                       *puc_start_addr;
    oal_uint32                       ul_copy_len;
    hi1103_update_cali_channel_stru *pst_update_cali_channel;
    hi1103_cali_param_stru          *pst_cali_data;
    oal_uint32                       ul_remain_len;         /* 每个通道剩余的校准长度 */
    oal_uint32                       ul_frame_netbuf_len;
    OAL_STATIC oal_uint32            ul_cali_data_len = 0;
    //oal_uint8  *puc_content;
    //oal_uint32 ul_byte;


    OAM_INFO_LOG0(0, OAM_SF_CALIBRATE, "hmac_save_cali_event:start!");

    pst_cali_data = (hi1103_cali_param_stru *)get_cali_data_buf_addr();

    if (OAL_PTR_NULL == pst_cali_data)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CALIBRATE, "hmac_save_cali_event:pst_cali_data is null!");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (RF_SINGLE_CHAN_CALI_DATA_BUF_LEN <= ul_cali_data_len)
    {
        /* 通道1的校准数据 */
        pst_cali_data++;
        ul_remain_len  = RF_CALI_DATA_BUF_LEN - ul_cali_data_len;
        OAM_INFO_LOG2(0, OAM_SF_CALIBRATE, "hmac_save_cali_event:goto chn1[%p]remain len[%d]!", pst_cali_data,
                         ul_remain_len);
    }
    else
    {
        ul_remain_len  = RF_SINGLE_CHAN_CALI_DATA_BUF_LEN - ul_cali_data_len;
        OAM_INFO_LOG2(0, OAM_SF_CALIBRATE, "hmac_save_cali_event:goto chn0[%p]remain len[%d]!", pst_cali_data,
                         ul_remain_len);

    }

    pst_event               = frw_get_event_stru(pst_event_mem);
    pst_cali_save_event     = (hal_cali_hal2hmac_event_stru *)pst_event->auc_event_data;
    pst_pay_load            = (hal_cali_hal2hmac_payload_stru *)OAL_NETBUF_DATA(pst_cali_save_event->pst_netbuf);
    //pst_update_cali_channel->ul_cali_time = 0;


    //OAL_IO_PRINT("pst_update_cali_channel->ul_cali_time %d : \r\n", pst_update_cali_channel->ul_cali_time);
    ul_frame_netbuf_len = OAL_NETBUF_LEN(pst_cali_save_event->pst_netbuf) - OAL_SIZEOF(pst_pay_load->ul_packet_idx);

    //OAL_IO_PRINT("hmac_save_cali_event : first cali packet idx:pst_pay_load->ul_packet_idx %d\r\n", pst_pay_load->ul_packet_idx);
    OAM_INFO_LOG1(0, OAM_SF_CALIBRATE, "hmac_save_cali_event:first cali packet idx:pst_pay_load packet_idx %d", pst_pay_load->ul_packet_idx);

    puc_start_addr = ((oal_uint8 *)pst_cali_data->ast_2Gcali_param) + (ul_frame_netbuf_len * pst_pay_load->ul_packet_idx);
    //OAL_IO_PRINT("hmac_save_cali_event : ul_remain_len%d \r\n", ul_remain_len);

    //OAM_ERROR_LOG1(0, 0, "hmac_save_cali_event : ul_remain_len%d \r\n", ul_remain_len);
    //OAL_IO_PRINT("hmac_save_cali_event : WLAN_LARGE_NETBUF_SIZE %d\r\n", ul_netbuf_len);
    ul_copy_len = OAL_MIN(ul_remain_len, ul_frame_netbuf_len);
    OAM_INFO_LOG2(0, OAM_SF_CALIBRATE, "hmac_save_cali_event:remain len[%d] frame len[%d]!",
           ul_remain_len, ul_frame_netbuf_len);

    ul_cali_data_len += ul_copy_len;

    if (RF_CALI_DATA_BUF_LEN == ul_cali_data_len)
    {
        pst_update_cali_channel = &pst_cali_data->st_cali_update_info;

        if ((pst_update_cali_channel->ul_cali_time == 0) && (g_uc_netdev_is_open == OAL_FALSE))
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_save_cali_event:wlan_pm_close,g_uc_netdev_is_open = FALSE!.}",
                             pst_update_cali_channel->ul_cali_time);
            wlan_pm_close();
        }

        pst_update_cali_channel->ul_cali_time++;
        g_ul_cali_update_channel_info = *(oal_uint32 *)pst_update_cali_channel;

        OAM_INFO_LOG1(0, OAM_SF_ANY, "{hmac_save_cali_event::last packet ul_cali_time %d}", pst_update_cali_channel->ul_cali_time);
    }

    oal_memcopy(puc_start_addr, pst_pay_load->auc_payoald, ul_copy_len);

    if (RF_CALI_DATA_BUF_LEN == ul_cali_data_len)
    {
        ul_cali_data_len = 0;
        hmac_dump_cali_result();
    }


#if 0
    puc_content = (oal_uint8 *)pst_cali_data->ast_2Gcali_param;

    OAM_ERROR_LOG1(0, 0, "hmac_save_cali_event : calidata len%d \r\n", OAL_SIZEOF(oal_cali_param_stru) - 8);
    for (ul_byte = 0; ul_byte < OAL_SIZEOF(oal_cali_param_stru) - 8; ul_byte+=4)
    {

        OAL_IO_PRINT("%02X %02X %02X %02X\r\n", puc_content[ul_byte], puc_content[ul_byte+1],
                      puc_content[ul_byte+2], puc_content[ul_byte+3]);
    }

    else
    {
        puc_start_addr = pst_pay_load->auc_payoald;
        oal_memcopy(&g_st_cali_control.g_ast_5Gcali_param[pst_update_cali_channel->uc_5g_chan_idx1],
                    puc_start_addr,
                    OAL_SIZEOF(oal_5Gcali_param_stru));

        puc_start_addr += OAL_SIZEOF(oal_5Gcali_param_stru);
        oal_memcopy(&g_st_cali_control.g_ast_5Gcali_param[pst_update_cali_channel->uc_5g_chan_idx1 + HAL_5G_20M_CHANNEL_NUM],
                    puc_start_addr,
                    OAL_SIZEOF(oal_5Gcali_param_stru));

        if (pst_update_cali_channel->en_update_bt == OAL_TRUE)
        {
            puc_start_addr += OAL_SIZEOF(oal_5Gcali_param_stru);
            oal_memcopy(&g_st_cali_control.st_bt_cali_comp,
                        pst_pay_load,
                        OAL_SIZEOF(oal_bt_cali_comp_stru));
        }

        pst_update_cali_channel->ul_cali_time++;
        hmac_add_bound((oal_uint32 *)&pst_update_cali_channel->uc_2g_band_idx, HAL_2G_CHANNEL_NUM + 1);

        if (HAL_2G_CHANNEL_NUM == pst_update_cali_channel->uc_2g_band_idx)
        {
            pst_update_cali_channel->en_update_bt = OAL_TRUE;
        }
        else
        {
            pst_update_cali_channel->en_update_bt = OAL_FALSE;
        }

        hmac_add_bound((oal_uint32 *)&pst_update_cali_channel->uc_5g_chan_idx1, HAL_5G_20M_CHANNEL_NUM);

        while (mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, pst_update_cali_channel->uc_5g_chan_idx1))
        {
            hmac_add_bound((oal_uint32 *)&pst_update_cali_channel->uc_5g_chan_idx1, HAL_5G_20M_CHANNEL_NUM);
        }
    }

#endif

    oal_netbuf_free(pst_cali_save_event->pst_netbuf);

    OAM_INFO_LOG0(0, OAM_SF_CALIBRATE, "hmac_save_cali_event:end!");

    return OAL_SUCC;
}

#else //#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)

OAL_STATIC oal_int32 hmac_print_cail_result(oal_uint8 uc_cali_chn_idx,
                                                  oal_int8 *pc_print_buff,
                                                  oal_uint32 ul_remainder_len,
                                                  oal_cali_param_stru *pst_cali_data)
{

    oal_int8 *pc_string;

    pc_string = (uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM)
        ? "5G 20M cali data index:%d, rx_dc_comp:0x%x, s_digital_rxdc_cmp_i:0x%x, us_digital_rxdc_cmp_q:0x%x \n"
          "5G 20M tx_power upc_ppa_cmp:0x%x, upc_mx_cmp:0x%x, atx_pwr_cmp:0x%x,  ppf:0x%x \n"
          "5G 20M tx_dc i:%u,  q:%u, tx_iq p:%u  e:%u \n"
        : "5G 80M cali data index:%d, rx_dc_comp:0x%x, s_digital_rxdc_cmp_i:0x%x, us_digital_rxdc_cmp_q:0x%x \n"
          "5G 80M tx_power upc_ppa_cmp:0x%x, upc_mx_cmp:0x%x, atx_pwr_cmp:0x%x,  ppf:0x%x \n"
          "5G 80M tx_dc i:%u,  q:%u, tx_iq p:%u  e:%u \n" ;


    return OAL_SPRINTF(pc_print_buff, ul_remainder_len, pc_string,
                    uc_cali_chn_idx,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_analog_rxdc_cmp,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_i,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_q,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_5G.upc_ppa_cmp,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_5G.upc_mx_cmp,
                    (oal_uint8)pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_5G.ac_atx_pwr_cmp,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_ppf_cmp_val.uc_ppf_val,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_txdc_cmp_val.us_txdc_cmp_i,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_txdc_cmp_val.us_txdc_cmp_q,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_txiq_cmp_val.us_txiq_cmp_p,
                    pst_cali_data->ast_5Gcali_param[uc_cali_chn_idx].st_txiq_cmp_val.us_txiq_cmp_e);
}

oal_void hmac_dump_cali_result(oal_void)
{
    oal_cali_param_stru *pst_cali_data;
    oal_uint8            uc_cali_chn_idx;
    oal_int32            l_string_tmp_len;
    oal_int8            *pc_print_buff;
    oal_uint32           ul_string_len    = 0;

    pst_cali_data = (oal_cali_param_stru *)get_cali_data_buf_addr();

    OAM_WARNING_LOG4(0, OAM_SF_CFG, "rc code RC:0x%x, R:0x%x, C:0x%x, check_hw_status:0x%x",
            pst_cali_data->st_bfgn_cali_data.g_uc_rc_cmp_code,
            pst_cali_data->st_bfgn_cali_data.g_uc_r_cmp_code,
            pst_cali_data->st_bfgn_cali_data.g_uc_c_cmp_code,
            pst_cali_data->ul_check_hw_status);

    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_2G_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        OAM_WARNING_LOG4(0, OAM_SF_CFG, "2G cali data index:%u, rx_dc_comp: 0x%x, us_digital_rxdc_cmp_i:0x%x, us_digital_rxdc_cmp_q:0x%x",
                uc_cali_chn_idx,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_analog_rxdc_cmp,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_i,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_rx_dc_cmp.us_digital_rxdc_cmp_q);

        OAM_WARNING_LOG4(0, OAM_SF_CFG, "2G cali data index:%u, upc_ppa_cmp:0x%x, ac_atx_pwr_cmp:0x%x, dtx_pwr_cmp:0x%x",
                uc_cali_chn_idx,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_2G.upc_ppa_cmp,
                (oal_uint8)pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_2G.ac_atx_pwr_cmp,
                (oal_uint8)pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_cali_tx_power_cmp_2G.dtx_pwr_cmp);

        OAM_WARNING_LOG4(0, OAM_SF_CFG, "2G tx_dc i:%u q:%u, tx_iq p:%u e:%u",
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_txdc_cmp_val.us_txdc_cmp_i,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_txdc_cmp_val.us_txdc_cmp_q,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_txiq_cmp_val.us_txiq_cmp_p,
                pst_cali_data->ast_2Gcali_param[uc_cali_chn_idx].st_txiq_cmp_val.us_txiq_cmp_e);

    }

    if (OAL_FALSE == mac_device_check_5g_enable_per_chip())
    {
        return;
    }

    pc_print_buff = (oal_int8 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);
    if (OAL_PTR_NULL == pc_print_buff)
    {
        OAM_WARNING_LOG1(0, OAM_SF_CFG, "{hmac_dump_cali_result::pc_print_buff null.}", OAM_REPORT_MAX_STRING_LEN);
        return;
    }

    OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);

    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = hmac_print_cail_result(uc_cali_chn_idx, pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1), pst_cali_data);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;

    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);

    OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len    = 0;

    for (uc_cali_chn_idx = OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx < OAL_5G_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = hmac_print_cail_result(uc_cali_chn_idx, pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1), pst_cali_data);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;

    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);

#ifdef _PRE_WLAN_NEW_IQ
    OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len = 0;
    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = OAL_SPRINTF(pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            "uc_cali_chn_idx:%d \n"
            "RX_IQ udelay1:0x%x, udelay2:0x%x, alpha:0x%x, beta:0x%x \n",
            uc_cali_chn_idx,
            pst_cali_data->ast_new_rxiq_cmp_val_5G[uc_cali_chn_idx].ul_rxiq_cmp_u1,
            pst_cali_data->ast_new_rxiq_cmp_val_5G[uc_cali_chn_idx].ul_rxiq_cmp_u2,
            pst_cali_data->ast_new_rxiq_cmp_val_5G[uc_cali_chn_idx].ul_rxiq_cmp_alpha,
            pst_cali_data->ast_new_rxiq_cmp_val_5G[uc_cali_chn_idx].ul_rxiq_cmp_beta);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;
    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);

    OAL_MEMZERO(pc_print_buff, OAM_REPORT_MAX_STRING_LEN);
    ul_string_len    = 0;
    for (uc_cali_chn_idx = 0; uc_cali_chn_idx < OAL_5G_20M_CHANNEL_NUM; uc_cali_chn_idx++)
    {
        l_string_tmp_len = OAL_SPRINTF(pc_print_buff + ul_string_len, (OAM_REPORT_MAX_STRING_LEN - ul_string_len - 1),
            "uc_cali_chn_idx:%d \n"
            "TX_IQ alpha0:0x%x, alpha:0x%x, beta0:0x%x, beta:0x%x \n",
            uc_cali_chn_idx,
            pst_cali_data->ast_new_txiq_cmp_val_5G[uc_cali_chn_idx].aul_alpha0_reg_val[7],
            pst_cali_data->ast_new_txiq_cmp_val_5G[uc_cali_chn_idx].aul_alpha_reg_val[7],
            pst_cali_data->ast_new_txiq_cmp_val_5G[uc_cali_chn_idx].aul_beta0_reg_val[7],
            pst_cali_data->ast_new_txiq_cmp_val_5G[uc_cali_chn_idx].aul_beta_reg_val[7]);
        if (l_string_tmp_len < 0)
        {
            goto sprint_fail;
        }
        ul_string_len += (oal_uint32)l_string_tmp_len;
    }
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);
#endif

    OAL_MEM_FREE(pc_print_buff, OAL_TRUE);
    return;

sprint_fail:

    OAM_WARNING_LOG0(0, OAM_SF_CFG, "{hmac_dump_cali_result:: OAL_SPRINTF return error!}");
    pc_print_buff[OAM_REPORT_MAX_STRING_LEN-1] = '\0';
    oam_print(pc_print_buff);
    OAL_MEM_FREE(pc_print_buff, OAL_TRUE);

    return;
}


oal_uint32 hmac_send_cali_data(mac_vap_stru *pst_mac_vap)
{
    frw_event_mem_stru       *pst_event_mem;
    frw_event_stru           *pst_event;
    dmac_tx_event_stru       *pst_h2d_cali_event;
    oal_netbuf_stru          *pst_netbuf_cali_data;
    oal_uint8                *puc_cali_data;
    oal_cali_param_stru      *pst_cali_data;
    oal_int16                 l_remain_len;
    oal_uint8                *puc_param = OAL_PTR_NULL;

    l_remain_len = OAL_WIFI_CALI_DATA_DOWNLOAD_LEN;
    pst_cali_data = (oal_cali_param_stru *)get_cali_data_buf_addr();

    hmac_dump_cali_result();

    while (l_remain_len > 0)
    {
        pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
        if (OAL_PTR_NULL == pst_event_mem)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "{hmac_send_cali_data::pst_event_mem null.}");

            return OAL_ERR_CODE_PTR_NULL;
        }

        OAL_IO_PRINT("{hmac_send_cali_data.start}\r\n");

        pst_event = frw_get_event_stru(pst_event_mem);

        FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                           FRW_EVENT_TYPE_WLAN_CTX,
                           DMAC_WLAN_CTX_EVENT_SUB_TYPE_CALI_HMAC2DMAC,
                           OAL_SIZEOF(dmac_tx_event_stru),
                           FRW_EVENT_PIPELINE_STAGE_1,
                           pst_mac_vap->uc_chip_id,
                           pst_mac_vap->uc_device_id,
                           pst_mac_vap->uc_vap_id);

        pst_netbuf_cali_data = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
        if (OAL_PTR_NULL == pst_netbuf_cali_data)
        {
           FRW_EVENT_FREE(pst_event_mem);
           OAM_ERROR_LOG0(0, OAM_SF_CALIBRATE, "{hmac_send_cali_data::pst_netbuf_scan_result null.}");

           return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        OAL_MEMZERO(oal_netbuf_cb(pst_netbuf_cali_data), OAL_TX_CB_LEN);

        puc_param = (oal_uint8 *)(pst_cali_data->ast_2Gcali_param) + OAL_WIFI_CALI_DATA_DOWNLOAD_LEN - l_remain_len;
        puc_cali_data = (oal_uint8 *)(OAL_NETBUF_DATA(pst_netbuf_cali_data));
        oal_memcopy(puc_cali_data, puc_param, (oal_uint32)(OAL_MIN(WLAN_LARGE_NETBUF_SIZE - 104, l_remain_len)));

        pst_h2d_cali_event               = (dmac_tx_event_stru *)pst_event->auc_event_data;
        pst_h2d_cali_event->pst_netbuf   = pst_netbuf_cali_data;
        pst_h2d_cali_event->us_frame_len = (oal_uint16)(OAL_MIN(WLAN_LARGE_NETBUF_SIZE - 104, l_remain_len));
        pst_h2d_cali_event->us_remain    = (oal_uint16)l_remain_len;

        l_remain_len -= (WLAN_LARGE_NETBUF_SIZE - 104);

        frw_event_dispatch_event(pst_event_mem);

        oal_netbuf_free(pst_netbuf_cali_data);
        FRW_EVENT_FREE(pst_event_mem);

    }


    return OAL_SUCC;
}

oal_uint32 hmac_save_cali_event(frw_event_mem_stru *pst_event_mem)
{
    frw_event_stru          *pst_event;
    hal_cali_hal2hmac_event_stru    *pst_cali_save_event;
    hal_cali_hal2hmac_payload_stru *pst_pay_load;
    oal_uint8 *puc_start_addr;
    oal_uint32 ul_copy_len;
    oal_update_cali_channel_stru *pst_update_cali_channel;
    oal_cali_param_stru *pst_cali_data;
    oal_uint32                      ul_remain_len;
    oal_uint32                      ul_netbuf_len;
    //oal_uint8  *puc_content;
    //oal_uint32 ul_byte;

    pst_cali_data = (oal_cali_param_stru *)get_cali_data_buf_addr();

    pst_event = frw_get_event_stru(pst_event_mem);
    pst_cali_save_event = (hal_cali_hal2hmac_event_stru *)pst_event->auc_event_data;

    pst_pay_load  = (hal_cali_hal2hmac_payload_stru *)OAL_NETBUF_DATA(pst_cali_save_event->pst_netbuf);
    pst_update_cali_channel = &pst_cali_data->st_cali_update_info;
    //pst_update_cali_channel->ul_cali_time = 0;


    //OAL_IO_PRINT("pst_update_cali_channel->ul_cali_time %d : \r\n", pst_update_cali_channel->ul_cali_time);
    ul_netbuf_len = OAL_NETBUF_LEN(pst_cali_save_event->pst_netbuf);

    //OAL_IO_PRINT("hmac_save_cali_event : first cali packet idx:pst_pay_load->ul_packet_idx %d\r\n", pst_pay_load->ul_packet_idx);
    OAM_WARNING_LOG1(0, 0, "hmac_save_cali_event : first cali packet idx:pst_pay_load->ul_packet_idx %d\r\n", pst_pay_load->ul_packet_idx);

    puc_start_addr = ((oal_uint8 *)pst_cali_data->ast_2Gcali_param) + ((ul_netbuf_len - 4) * pst_pay_load->ul_packet_idx);
    ul_remain_len = OAL_WIFI_CALI_DATA_UPLOAD_LEN;
    ul_remain_len = ul_remain_len - (ul_netbuf_len - 4) * pst_pay_load->ul_packet_idx;
    //OAL_IO_PRINT("hmac_save_cali_event : ul_remain_len%d \r\n", ul_remain_len);

    //OAM_ERROR_LOG1(0, 0, "hmac_save_cali_event : ul_remain_len%d \r\n", ul_remain_len);
    //OAL_IO_PRINT("hmac_save_cali_event : WLAN_LARGE_NETBUF_SIZE %d\r\n", ul_netbuf_len);

    if (ul_remain_len <= ul_netbuf_len - 4)
    {
        //OAL_IO_PRINT("hmac_save_cali_event : last packet");
        ul_copy_len = ul_remain_len;

        if ((pst_update_cali_channel->ul_cali_time == 0) && (g_uc_netdev_is_open == OAL_FALSE))
        {
            OAM_WARNING_LOG0(0, OAM_SF_ANY, "{hmac_save_cali_event: wlan_pm_close,g_uc_netdev_is_open = FALSE!\r\n.}");
            wlan_pm_close();
        }
    }
    else
    {
        ul_copy_len = ul_netbuf_len - 4;
    }

    oal_memcopy(puc_start_addr, pst_pay_load->auc_payoald, ul_copy_len);

    if (ul_remain_len <= ul_netbuf_len - 4)
    {
        pst_update_cali_channel->ul_cali_time++;
        pst_update_cali_channel->en_update_bt = OAL_FALSE;
        g_ul_cali_update_channel_info = *(oal_uint32 *)pst_update_cali_channel;
        hmac_dump_cali_result();

        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_save_cali_event::last packet ul_cali_time %d}", pst_update_cali_channel->ul_cali_time);
    }

#if 0

    puc_content = (oal_uint8 *)pst_cali_data->ast_2Gcali_param;

    OAM_ERROR_LOG1(0, 0, "hmac_save_cali_event : calidata len%d \r\n", OAL_SIZEOF(oal_cali_param_stru) - 8);
    for (ul_byte = 0; ul_byte < OAL_SIZEOF(oal_cali_param_stru) - 8; ul_byte+=4)
    {

        OAL_IO_PRINT("%02X %02X %02X %02X\r\n", puc_content[ul_byte], puc_content[ul_byte+1],
                      puc_content[ul_byte+2], puc_content[ul_byte+3]);
    }

    else
    {
        puc_start_addr = pst_pay_load->auc_payoald;
        oal_memcopy(&g_st_cali_control.g_ast_5Gcali_param[pst_update_cali_channel->uc_5g_chan_idx1],
                    puc_start_addr,
                    OAL_SIZEOF(oal_5Gcali_param_stru));

        puc_start_addr += OAL_SIZEOF(oal_5Gcali_param_stru);
        oal_memcopy(&g_st_cali_control.g_ast_5Gcali_param[pst_update_cali_channel->uc_5g_chan_idx1 + HAL_5G_20M_CHANNEL_NUM],
                    puc_start_addr,
                    OAL_SIZEOF(oal_5Gcali_param_stru));

        if (pst_update_cali_channel->en_update_bt == OAL_TRUE)
        {
            puc_start_addr += OAL_SIZEOF(oal_5Gcali_param_stru);
            oal_memcopy(&g_st_cali_control.st_bt_cali_comp,
                        pst_pay_load,
                        OAL_SIZEOF(oal_bt_cali_comp_stru));
        }

        pst_update_cali_channel->ul_cali_time++;
        hmac_add_bound((oal_uint32 *)&pst_update_cali_channel->uc_2g_band_idx, HAL_2G_CHANNEL_NUM + 1);

        if (HAL_2G_CHANNEL_NUM == pst_update_cali_channel->uc_2g_band_idx)
        {
            pst_update_cali_channel->en_update_bt = OAL_TRUE;
        }
        else
        {
            pst_update_cali_channel->en_update_bt = OAL_FALSE;
        }

        hmac_add_bound((oal_uint32 *)&pst_update_cali_channel->uc_5g_chan_idx1, HAL_5G_20M_CHANNEL_NUM);

        while (mac_is_channel_idx_valid(MAC_RC_START_FREQ_5, pst_update_cali_channel->uc_5g_chan_idx1))
        {
            hmac_add_bound((oal_uint32 *)&pst_update_cali_channel->uc_5g_chan_idx1, HAL_5G_20M_CHANNEL_NUM);
        }
    }

#endif

    oal_netbuf_free(pst_cali_save_event->pst_netbuf);

    return OAL_SUCC;
}
#endif
/*lint +e801*/

#ifdef _PRE_WLAN_ONLINE_DPD


oal_uint32 hmac_send_corram_data(mac_vap_stru *pst_mac_vap)
{
    frw_event_mem_stru       *pst_event_mem;
    frw_event_stru           *pst_event;
    dmac_tx_event_stru       *pst_dpd_event;
    oal_netbuf_stru          *pst_netbuf_corram_data;
    oal_uint8                *puc_corram_data;
    oal_int16                 l_remain_len;
    oal_int16                 l_total_len;
    oal_uint8                *puc_param;

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG1(0, OAM_SF_CALIBRATE, "{hmac_dpd_data_processed_send::param null, %p %p.}", pst_mac_vap);
        return OAL_ERR_CODE_PTR_NULL;
    }


    l_total_len = OAL_SIZEOF(gul_corram_data);

    l_remain_len = OAL_SIZEOF(gul_corram_data);

    puc_corram_data = (oal_uint8 *)gul_corram_data;

    while (l_remain_len > 0)
    {
        pst_event_mem = FRW_EVENT_ALLOC(OAL_SIZEOF(dmac_tx_event_stru));
        if (OAL_PTR_NULL == pst_event_mem)
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CALIBRATE, "{hmac_scan_proc_scan_req_event::pst_event_mem null.}");

            return OAL_ERR_CODE_PTR_NULL;
        }


        pst_event = frw_get_event_stru(pst_event_mem);

        FRW_EVENT_HDR_INIT(&(pst_event->st_event_hdr),
                           FRW_EVENT_TYPE_WLAN_CTX,
                           DMAC_WLAN_CTX_EVENT_SUB_TYPE_DPD_HMAC2DMAC,
                           OAL_SIZEOF(dmac_tx_event_stru),
                           FRW_EVENT_PIPELINE_STAGE_1,
                           pst_mac_vap->uc_chip_id,
                           pst_mac_vap->uc_device_id,
                           pst_mac_vap->uc_vap_id);

        pst_netbuf_corram_data = OAL_MEM_NETBUF_ALLOC(OAL_NORMAL_NETBUF,WLAN_LARGE_NETBUF_SIZE, OAL_NETBUF_PRIORITY_MID);
        if (OAL_PTR_NULL == pst_netbuf_corram_data)
        {
           FRW_EVENT_FREE(pst_event_mem);
           OAM_ERROR_LOG0(0, OAM_SF_CALIBRATE,"{hmac_dpd_data_processed_send::pst_netbuf_scan_result null.}");

           return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        OAL_MEMZERO(oal_netbuf_cb(pst_netbuf_corram_data), OAL_TX_CB_LEN);

        puc_param = puc_corram_data + l_total_len - l_remain_len;

        oal_memcopy((oal_uint8 *)(OAL_NETBUF_DATA(pst_netbuf_corram_data)), puc_param, DPD_CORRAM_DATA_LEN);
        l_remain_len -= DPD_CORRAM_DATA_LEN;

        pst_dpd_event               = (dmac_tx_event_stru *)pst_event->auc_event_data;
        pst_dpd_event->pst_netbuf   = pst_netbuf_corram_data;
        pst_dpd_event->us_frame_len = DPD_CORRAM_DATA_LEN;
        pst_dpd_event->us_remain    = (oal_uint16)l_remain_len;


        frw_event_dispatch_event(pst_event_mem);

        oal_netbuf_free(pst_netbuf_corram_data);
        FRW_EVENT_FREE(pst_event_mem);

    }


    return OAL_SUCC;

}
#endif

/*lint +e571*/
/*lint +e416*/

#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif



