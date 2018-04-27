#ifndef _LIBRA_PARTITION_H_
#define _LIBRA_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER_A,        0,         2*1024,        EMMC_BOOT_MAJOR_PART},
  {PART_XLOADER_B,        0,         2*1024,        EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,           0,         512,           EMMC_USER_PART},/* ptable          512K */
  {PART_FRP,              512,       512,           EMMC_USER_PART},/* frp             512K   p1*/
  {PART_PERSIST,          1024,      2048,          EMMC_USER_PART},/* persist         2048K  p2*/
  {PART_RESERVED1,        3072,      5120,          EMMC_USER_PART},/* reserved1       5120K  p3*/
  {PART_RESERVED6,        8*1024,    512,           EMMC_USER_PART},/* reserved6       512K   p4*/
  {PART_VRL,              8704,      512,           EMMC_USER_PART},/* vrl             512K   p5*/
  {PART_VRL_BACKUP,       9216,      512,           EMMC_USER_PART},/* vrl backup      512K   p6*/
  {PART_MODEM_SECURE,     9728,      8704,          EMMC_USER_PART},/* modem_secure    8704k  p7*/
  {PART_NVME,             18*1024,   5*1024,        EMMC_USER_PART},/* nvme            5M     p8*/
  {PART_CTF,              23*1024,   1*1024,        EMMC_USER_PART},/* PART_CTF        1M     p9*/
  {PART_OEMINFO,          24*1024,   64*1024,       EMMC_USER_PART},/* oeminfo         64M    p10*/
  {PART_SECURE_STORAGE,   88*1024,   32*1024,       EMMC_USER_PART},/* secure storage  32M    p11*/
  {PART_MODEM_OM,         120*1024,  32*1024,       EMMC_USER_PART},/* modem om        32M    p12*/
  {PART_MODEMNVM_FACTORY, 152*1024,  16*1024,       EMMC_USER_PART},/* modemnvm factory16M    p13*/
  {PART_MODEMNVM_BACKUP,  168*1024,  16*1024,       EMMC_USER_PART},/* modemnvm backup 16M    p14*/
  {PART_MODEMNVM_IMG,     184*1024,  34*1024,       EMMC_USER_PART},/* modemnvm img    34M    p15*/
  {PART_RESERVED7,        218*1024,  2*1024,        EMMC_USER_PART},/* reserved7       2M     p16*/
  {PART_HISEE_ENCOS,      220*1024,  4*1024,        EMMC_USER_PART},/* hisee_encos     4M     p17*/
  {PART_VERITYKEY,        224*1024,  1*1024,        EMMC_USER_PART},/* reserved2       32M    p18*/
  {PART_DDR_PARA,         225*1024,  1*1024,        EMMC_USER_PART},/* DDR_PARA        1M     p19*/
  {PART_RESERVED2,        226*1024,  27*1024,       EMMC_USER_PART},/* reserved2       32M    p20*/
  {PART_SPLASH2,          253*1024,  80*1024,       EMMC_USER_PART},/* splash2         80M    p21*/
  {PART_BOOTFAIL_INFO,    333*1024,  2*1024,        EMMC_USER_PART},/* bootfail info   2MB    p22*/
  {PART_MISC,             335*1024,  2*1024,        EMMC_USER_PART},/* misc            2M     p23*/
  {PART_DFX,              337*1024,  16*1024,       EMMC_USER_PART},/* dfx             16M    p24*/
  {PART_RRECORD,          353*1024,  16*1024,       EMMC_USER_PART},/* rrecord         16M    p25*/
  {PART_FW_LPM3_A,        369*1024,  256,           EMMC_USER_PART},/* mcuimage        256K   p26*/
  {PART_RESERVED3_A,      378112,    3840,          EMMC_USER_PART},/* reserved3A      3840KB p27*/
  {PART_HDCP_A,           373*1024,  1*1024,        EMMC_USER_PART},/* PART_HDCP_A      1M    p28*/
  {PART_HISEE_IMG_A,      374*1024,  4*1024,        EMMC_USER_PART},/* part_hisee_img_a 4M    p29*/
  {PART_HHEE_A,           378*1024,  4*1024,        EMMC_USER_PART},/* PART_RESERVED10  4M    p30*/
  {PART_HISEE_FS_A,       382*1024,  8*1024,        EMMC_USER_PART},/* hisee_fs        8M     p31*/
  {PART_FASTBOOT_A,       390*1024,  12*1024,       EMMC_USER_PART},/* fastboot        12M    p32*/
  {PART_VECTOR_A,         402*1024,  4*1024,        EMMC_USER_PART},/* avs vector      4M     p33*/
  {PART_ISP_BOOT_A,       406*1024,  2*1024,        EMMC_USER_PART},/* isp_boot        2M     p34*/
  {PART_ISP_FIRMWARE_A,   408*1024,  14*1024,       EMMC_USER_PART},/* isp_firmware    14M    p35*/
  {PART_FW_HIFI_A,        422*1024,  12*1024,       EMMC_USER_PART},/* hifi            12M    p36*/
  {PART_TEEOS_A,          434*1024,  8*1024,        EMMC_USER_PART},/* teeos           8M     p37*/
  {PART_SENSORHUB_A,      442*1024,  16*1024,       EMMC_USER_PART},/* sensorhub       16M    p38*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A,  458*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p39*/
  {PART_ERECOVERY_RAMDISK_A, 482*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p40*/
  {PART_ERECOVERY_VENDOR_A,  514*1024,  8*1024,     EMMC_USER_PART},/* erecovery_vendor  8M     p41*/
  {PART_KERNEL_A,            522*1024,  32*1024,    EMMC_USER_PART},/* kernel            32M    p42*/
#else
  {PART_ERECOVERY_KERNEL_A,  458*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p39*/
  {PART_ERECOVERY_RAMDISK_A, 482*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p40*/
  {PART_ERECOVERY_VENDOR_A,  514*1024,  16*1024,    EMMC_USER_PART},/* erecovery_vendor  16M    p41*/
  {PART_KERNEL_A,            530*1024,  24*1024,    EMMC_USER_PART},/* kernel            24M    p42*/
#endif
  {PART_RAMDISK_A,          554*1024,  16*1024,     EMMC_USER_PART},/* ramdisk         16M    p43*/
  {PART_RECOVERY_RAMDISK_A, 570*1024,  32*1024,     EMMC_USER_PART},/* recovery_ramdisk 32M   p44*/
  {PART_RECOVERY_VENDOR_A,  602*1024,  16*1024,     EMMC_USER_PART},/* recovery_vendor 16M    p45*/
  {PART_DTS_A,              618*1024,  24*1024,     EMMC_USER_PART},/* dtimage         24M    p46*/
  {PART_DTO_A,              642*1024,  8*1024,      EMMC_USER_PART},/* dtoimage        8M     p47*/
  {PART_TRUSTFIRMWARE_A,    650*1024,  2*1024,      EMMC_USER_PART},/* trustfirmware   2M     p48*/
  {PART_MODEM_FW_A,         652*1024,  56*1024,     EMMC_USER_PART},/* modem_fw        56M    p49*/
  {PART_RESERVED4_A,        708*1024,  20*1024,     EMMC_USER_PART},/* reserved4A      20M    p50*/
  {PART_RECOVERY_VBMETA_A,  728*1024,  2*1024,      EMMC_USER_PART},/* recovery_vbmeta_a 2M   p51*/
  {PART_ERECOVERY_VBMETA_A, 730*1024,  2*1024,      EMMC_USER_PART},/* erecovery_vbmeta_a 2M  p52*/
  {PART_VBMETA_A,           732*1024,  4*1024,      EMMC_USER_PART},/* PART_VBMETA_A   4M     p53*/
  {PART_MODEMNVM_UPDATE_A,  736*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_update_a 16M  p54*/
  {PART_MODEMNVM_CUST_A,    752*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_cust_a 16M    p55*/
  {PART_PATCH_A,            768*1024,  32*1024,     EMMC_USER_PART},/* patch           32M    p56*/
  {PART_VERSION_A,          800*1024,  32*1024,     EMMC_USER_PART},/* version         32M    p57*/
  {PART_VENDOR_A,           832*1024,  760*1024,    EMMC_USER_PART},/* vendor          760M   p58*/
  {PART_PRODUCT_A,          1592*1024, 192*1024,    EMMC_USER_PART},/* product         192M   p59*/
  {PART_CUST_A,             1784*1024, 192*1024,    EMMC_USER_PART},/* cust            192M   p60*/
  {PART_ODM_A,              1976*1024, 176*1024,    EMMC_USER_PART},/* odm             176M   p61*/
  {PART_CACHE,              2152*1024, 104*1024,    EMMC_USER_PART},/* cache           104M   p62*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM_A,           2256*1024, 3852*1024,      EMMC_USER_PART},/* system          3852M  p63*/
  {PART_USERDATA,           6108*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p66*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM_A,           2256*1024, 5632*1024,      EMMC_USER_PART},/* system          5632M  p63*/
  {PART_USERDATA,           7888*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p66*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM_A,           2256*1024, 4752*1024,      EMMC_USER_PART},/* system          4752M  p63*/
  {PART_USERDATA,           7008*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p66*/
  #else
  {PART_SYSTEM_A,           2256*1024, 4988*1024,      EMMC_USER_PART},/* system          4988M  p63*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_RESERVED5,          7244*1024, 128*1024,       EMMC_USER_PART},/* reserved5      128M    p64*/
  {PART_HIBENCH_DATA,       7372*1024, 512*1024,       EMMC_USER_PART},/* hibench_data   512M    p65*/
  {PART_USERDATA,           7884*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p66*/
  #else
  {PART_USERDATA,           7244*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p66*/
  #endif
  #endif
  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER_A,        0,         2*1024,        UFS_PART_0},
  {PART_XLOADER_B,        0,         2*1024,        UFS_PART_1},
  {PART_PTABLE,           0,         512,           UFS_PART_2},/* ptable          512K     */
  {PART_FRP,              512,       512,           UFS_PART_2},/* frp             512K   p1*/
  {PART_PERSIST,          1024,      2048,          UFS_PART_2},/* persist         2048K  p2*/
  {PART_RESERVED1,        3072,      5120,          UFS_PART_2},/* reserved1       5120K  p3*/
  {PART_PTABLE_LU3,       0,         512,           UFS_PART_3},/* ptable_lu3      512K   p0*/
  {PART_VRL,              512,       512,           UFS_PART_3},/* vrl             512K   p1*/
  {PART_VRL_BACKUP,       1024,      512,           UFS_PART_3},/* vrl backup      512K   p2*/
  {PART_MODEM_SECURE,     1536,      8704,          UFS_PART_3},/* modem_secure    8704k  p3*/
  {PART_NVME,             10*1024,   5*1024,        UFS_PART_3},/* nvme            5M     p4*/
  {PART_CTF,              15*1024,   1*1024,        UFS_PART_3},/* PART_CTF        1M     p5*/
  {PART_OEMINFO,          16*1024,   64*1024,       UFS_PART_3},/* oeminfo         64M    p6*/
  {PART_SECURE_STORAGE,   80*1024,   32*1024,       UFS_PART_3},/* secure storage  32M    p7*/
  {PART_MODEM_OM,         112*1024,  32*1024,       UFS_PART_3},/* modem om        32M    p8*/
  {PART_MODEMNVM_FACTORY, 144*1024,  16*1024,       UFS_PART_3},/* modemnvm factory16M    p9*/
  {PART_MODEMNVM_BACKUP,  160*1024,  16*1024,       UFS_PART_3},/* modemnvm backup 16M    p10*/
  {PART_MODEMNVM_IMG,     176*1024,  34*1024,       UFS_PART_3},/* modemnvm img    34M    p11*/
  {PART_RESERVED7,        210*1024,  2*1024,        UFS_PART_3},/* reserved7       2M     p12*/
  {PART_HISEE_ENCOS,      212*1024,  4*1024,        UFS_PART_3},/* hisee_encos     4M     p13*/
  {PART_VERITYKEY,        216*1024,  1*1024,        UFS_PART_3},/* reserved2       32M    p14*/
  {PART_DDR_PARA,         217*1024,  1*1024,        UFS_PART_3},/* DDR_PARA        1M     p15*/
  {PART_RESERVED2,        218*1024,  27*1024,       UFS_PART_3},/* reserved2       32M    p16*/
  {PART_SPLASH2,          245*1024,  80*1024,       UFS_PART_3},/* splash2         80M    p17*/
  {PART_BOOTFAIL_INFO,    325*1024,  2*1024,        UFS_PART_3},/* bootfail info   2MB    p18*/
  {PART_MISC,             327*1024,  2*1024,        UFS_PART_3},/* misc            2M     p19*/
  {PART_DFX,              329*1024,  16*1024,       UFS_PART_3},/* dfx             16M    p20*/
  {PART_RRECORD,          345*1024,  16*1024,       UFS_PART_3},/* rrecord         16M    p21*/
  {PART_FW_LPM3_A,        361*1024,  256,           UFS_PART_3},/* mcuimage        256K   p22*/
  {PART_RESERVED3_A,      369920,    3840,          UFS_PART_3},/* reserved3A      3840K  p23*/
  {PART_HDCP_A,           365*1024,  1*1024,        UFS_PART_3},/* PART_HDCP_A     1M     p24*/
  {PART_HISEE_IMG_A,      366*1024,  4*1024,        UFS_PART_3},/* part_hisee_img_a 4M    p25*/
  {PART_HHEE_A,           370*1024,  4*1024,        UFS_PART_3},/* PART_RESERVED10  4M    p26*/
  {PART_HISEE_FS_A,       374*1024,  8*1024,        UFS_PART_3},/* hisee_fs        8M     p27*/
  {PART_FASTBOOT_A,       382*1024,  12*1024,       UFS_PART_3},/* fastboot        12M    p28*/
  {PART_VECTOR_A,         394*1024,  4*1024,        UFS_PART_3},/* avs vector      4M     p29*/
  {PART_ISP_BOOT_A,       398*1024,  2*1024,        UFS_PART_3},/* isp_boot        2M     p30*/
  {PART_ISP_FIRMWARE_A,   400*1024,  14*1024,       UFS_PART_3},/* isp_firmware    14M    p31*/
  {PART_FW_HIFI_A,        414*1024,  12*1024,       UFS_PART_3},/* hifi            12M    p32*/
  {PART_TEEOS_A,          426*1024,  8*1024,        UFS_PART_3},/* teeos           8M     p33*/
  {PART_SENSORHUB_A,      434*1024,  16*1024,       UFS_PART_3},/* sensorhub       16M    p34*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A,  450*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M   p35*/
  {PART_ERECOVERY_RAMDISK_A, 474*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M   p36*/
  {PART_ERECOVERY_VENDOR_A,  506*1024,  8*1024,     UFS_PART_3},/* erecovery_vendor  8M    p37*/
  {PART_KERNEL_A,            514*1024,  32*1024,    UFS_PART_3},/* kernel            32M   p38*/
#else
  {PART_ERECOVERY_KERNEL_A,  450*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M   p35*/
  {PART_ERECOVERY_RAMDISK_A, 474*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M   p36*/
  {PART_ERECOVERY_VENDOR_A,  506*1024,  16*1024,    UFS_PART_3},/* erecovery_vendor  16M   p37*/
  {PART_KERNEL_A,            522*1024,  24*1024,    UFS_PART_3},/* kernel            24M   p38*/
#endif
  {PART_RAMDISK_A,          546*1024,  16*1024,     UFS_PART_3},/* ramdisk          16M    p39*/
  {PART_RECOVERY_RAMDISK_A, 562*1024,  32*1024,     UFS_PART_3},/* recovery_ramdisk 32M    p40*/
  {PART_RECOVERY_VENDOR_A,  594*1024,  16*1024,     UFS_PART_3},/* recovery_vendor  16M    p41*/
  {PART_DTS_A,              610*1024,  24*1024,     UFS_PART_3},/* dtimage          24M    p42*/
  {PART_DTO_A,              634*1024,  8*1024,      UFS_PART_3},/* dtoimage         8M     p43*/
  {PART_TRUSTFIRMWARE_A,    642*1024,  2*1024,      UFS_PART_3},/* trustfirmware    2M     p44*/
  {PART_MODEM_FW_A,         644*1024,  56*1024,     UFS_PART_3},/* modem_fw         56M    p45*/
  {PART_RESERVED4_A,        700*1024,  20*1024,     UFS_PART_3},/* reserved4A       20M    p46*/
  {PART_RECOVERY_VBMETA_A,  720*1024,  2*1024,      UFS_PART_3},/* recovery_vbmeta_a  2M   p47*/
  {PART_ERECOVERY_VBMETA_A, 722*1024,  2*1024,      UFS_PART_3},/* erecovery_vbmeta_a 2M   p48*/
  {PART_VBMETA_A,           724*1024,  4*1024,      UFS_PART_3},/* vbmeta_a         4M     p49*/
  {PART_MODEMNVM_UPDATE_A,  728*1024,  16*1024,     UFS_PART_3},/* modemnvm_update_a  16M  p50*/
  {PART_MODEMNVM_CUST_A,    744*1024,  16*1024,     UFS_PART_3},/* modemnvm_cust_a    16M  p51*/
  {PART_PATCH_A,            760*1024,  32*1024,     UFS_PART_3},/* patch            32M    p52*/
  {PART_VERSION_A,          792*1024,  32*1024,     UFS_PART_3},/* version          32M    p53*/
  {PART_VENDOR_A,           824*1024,  760*1024,    UFS_PART_3},/* vendor           760M   p54*/
  {PART_PRODUCT_A,          1584*1024, 192*1024,    UFS_PART_3},/* product          192M   p55*/
  {PART_CUST_A,             1776*1024, 192*1024,    UFS_PART_3},/* cust             192M   p56*/
  {PART_ODM_A,              1968*1024, 176*1024,    UFS_PART_3},/* odm              176M   p57*/
  {PART_CACHE,              2144*1024, 104*1024,    UFS_PART_3},/* cache            104M   p58*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM_A,           2248*1024, 3852*1024,      UFS_PART_3},/* system          3852M  p59*/
  {PART_USERDATA,           6100*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G     p62*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM_A,           2248*1024, 5632*1024,      UFS_PART_3},/* system          5632M  p59*/
  {PART_USERDATA,           7880*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G     p62*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM_A,           2248*1024, 4752*1024,      UFS_PART_3},/* system          4752M  p59*/
  {PART_USERDATA,           7000*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G     p62*/
  #else
  {PART_SYSTEM_A,           2248*1024, 4988*1024,      UFS_PART_3},/* system          4988M  p59*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_RESERVED5,          7236*1024, 128*1024,       UFS_PART_3},/* reserved5        128M   p60*/
  {PART_HIBENCH_DATA,       7364*1024, 512*1024,       UFS_PART_3},/* hibench_data     512M   p61*/
  {PART_USERDATA,           7876*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G     p62*/
  #else
  {PART_USERDATA,           7236*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G     p62*/
  #endif
  #endif
  {"0", 0, 0, 0},
};
#endif

#endif
