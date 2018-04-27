#ifndef _HI3660_PARTITION_H_
#define _HI3660_PARTITION_H_

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
  {PART_NVME,             18*1024,   6*1024,        EMMC_USER_PART},/* nvme            6M     p8*/
  {PART_OEMINFO,          24*1024,   64*1024,       EMMC_USER_PART},/* oeminfo         64M    p9*/
  {PART_SECURE_STORAGE,   88*1024,   32*1024,       EMMC_USER_PART},/* secure storage  32M    p10*/
  {PART_MODEM_OM,         120*1024,  32*1024,       EMMC_USER_PART},/* modem om        32M    p11*/
  {PART_MODEMNVM_FACTORY, 152*1024,  4*1024,        EMMC_USER_PART},/* modemnvm factory4M     p12*/
  {PART_MODEMNVM_BACKUP,  156*1024,  4*1024,        EMMC_USER_PART},/* modemnvm backup 4M     p13*/
  {PART_MODEMNVM_IMG,     160*1024,  12*1024,       EMMC_USER_PART},/* modemnvm img    12M    p14*/
  {PART_MODEMNVM_SYSTEM,  172*1024,  4*1024,        EMMC_USER_PART},/* modemnvm system 4M     p15*/
  {PART_SPLASH2,          176*1024,  80*1024,       EMMC_USER_PART},/* splash2         80M    p16*/
  {PART_CACHE,            256*1024,  128*1024,      EMMC_USER_PART},/* cache         128M     p17*/
  {PART_ODM_A,              384*1024,  128*1024,      EMMC_USER_PART},/* odm           128M     p18*/
  {PART_BOOTFAIL_INFO,    512*1024,  2*1024,        EMMC_USER_PART},/* bootfail info   2MB    p19*/
  {PART_MISC,             514*1024,  2*1024,        EMMC_USER_PART},/* misc            2M     p20*/
  {PART_RESERVED2,        516*1024,  32*1024,       EMMC_USER_PART},/* reserved2       32M    p21*/
  {PART_RESERVED10,        548*1024,  4*1024,       EMMC_USER_PART},/* PART_RESERVED10 4M     p22*/
  {PART_HISEE_FS,         552*1024,  8*1024,        EMMC_USER_PART},/* hisee_fs        8M     p23*/
  {PART_DFX,              560*1024,  16*1024,       EMMC_USER_PART},/* dfx             16M    p24*/
  {PART_RRECORD,          576*1024,  16*1024,       EMMC_USER_PART},/* rrecord         16M    p25*/
  {PART_FW_LPM3_A,        592*1024,  256,           EMMC_USER_PART},/* mcuimage        256K   p26*/
  {PART_RESERVED3_A,      606464,    3840,          EMMC_USER_PART},/* reserved3A      3840KB p27*/
  {PART_HISEE_IMG_A,      596*1024,    4*1024,      EMMC_USER_PART},/* part_hisee_img_a   4*1024KB p28*/
  {PART_FASTBOOT_A,       600*1024,  12*1024,       EMMC_USER_PART},/* fastboot        12M    p29*/
  {PART_VECTOR_A,         612*1024,  4*1024,        EMMC_USER_PART},/* avs vector      4M     p30*/
  {PART_ISP_BOOT_A,       616*1024,  2*1024,        EMMC_USER_PART},/* isp_boot        2M     p31*/
  {PART_ISP_FIRMWARE_A,   618*1024,  14*1024,       EMMC_USER_PART},/* isp_firmware    14M    p32*/
  {PART_FW_HIFI_A,        632*1024,  12*1024,       EMMC_USER_PART},/* hifi            12M    p33*/
  {PART_TEEOS_A,          644*1024,  8*1024,        EMMC_USER_PART},/* teeos           8M     p34*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A, 652*1024,  24*1024,     EMMC_USER_PART},/* erecovery_kernel  24M  p35*/
  {PART_ERECOVERY_RAMDISK_A, 676*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M  p36*/
  {PART_ERECOVERY_VENDOR_A, 708*1024,  8*1024,     EMMC_USER_PART},/* erecovery_vendor  8M    p37*/
  {PART_SENSORHUB_A,      716*1024,  16*1024,       EMMC_USER_PART},/* sensorhub       16M    p38*/
  {PART_KERNEL_A,         732*1024,  32*1024,       EMMC_USER_PART},/* kernel          32M    p39*/
#else
  {PART_ERECOVERY_KERNEL_A, 652*1024,  24*1024,     EMMC_USER_PART},/* erecovery_kernel  24M  p35*/
  {PART_ERECOVERY_RAMDISK_A, 676*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M  p36*/
  {PART_ERECOVERY_VENDOR_A, 708*1024,  16*1024,     EMMC_USER_PART},/* erecovery_vendor 16M   p37*/
  {PART_SENSORHUB_A,      724*1024,  16*1024,       EMMC_USER_PART},/* sensorhub       16M    p38*/
  {PART_KERNEL_A,         740*1024,  24*1024,       EMMC_USER_PART},/* kernel          24M    p39*/
#endif
  {PART_RAMDISK_A,        764*1024,  16*1024,       EMMC_USER_PART},/* ramdisk         16M    p40*/
  {PART_RECOVERY_RAMDISK_A, 780*1024,  32*1024,     EMMC_USER_PART},/* recovery_ramdisk 32M   p41*/
  {PART_RECOVERY_VENDOR_A, 812*1024,  16*1024,      EMMC_USER_PART},/* recovery_vendor 16M    p42*/
  {PART_DTS_A,            828*1024,  14*1024,       EMMC_USER_PART},/* dtimage         14M    p43*/
  {PART_DTO_A,            842*1024,   2*1024,       EMMC_USER_PART},/* dtoimage        2M     p44*/
  {PART_TRUSTFIRMWARE_A,  844*1024,  2*1024,        EMMC_USER_PART},/* trustfirmware   2M     p45*/
  {PART_MODEM_FW_A,       846*1024,  56*1024,       EMMC_USER_PART},/* modem_fw        56M    p46*/
  {PART_RESERVED4_A,      902*1024,  42*1024,       EMMC_USER_PART},/* reserved4A      42M    p47*/
  {PART_RECOVERY_VBMETA_A,  944*1024, 2*1024,       EMMC_USER_PART},/* recovery_vbmeta_a   2M    p48*/
  {PART_ERECOVERY_VBMETA_A, 946*1024, 2*1024,       EMMC_USER_PART},/* erecovery_vbmeta_a  2M    p49*/
  {PART_VBMETA_A,         948*1024,  4*1024,        EMMC_USER_PART},/* PART_VBMETA_A   4M     p50*/
  {PART_MODEMNVM_UPDATE_A,952*1024,  80*1024,       EMMC_USER_PART},/* modemnvm update 80M    p51*/
  {PART_PATCH_A,          1032 *1024,  32*1024,       EMMC_USER_PART},/* patch         32M    p52*/
  {PART_VERSION_A,        1064*1024,  32*1024,        EMMC_USER_PART},/* version       32M    p53*/
  {PART_VENDOR_A,         1096*1024,  784*1024,      EMMC_USER_PART},/* vendor         784M   p54*/
  {PART_PRODUCT_A,        1880*1024, 192*1024,      EMMC_USER_PART},/* product         192M   p55*/
  {PART_CUST_A,           2072*1024, 192*1024,      EMMC_USER_PART},/* cust            192M   p56*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM_A,         2264*1024, 3552*1024,     EMMC_USER_PART},/* system          3552M  p57*/
  {PART_RESERVED5,        5816*1024, 128*1024,       EMMC_USER_PART},/* reserved5      128M   p58*/
  {PART_USERDATA,         5944*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G     p59*/
  #else
  {PART_SYSTEM_A,         2264*1024, 4688*1024,     EMMC_USER_PART},/* system          4688M  p57*/
  {PART_RESERVED5,        6952*1024, 128*1024,       EMMC_USER_PART},/* reserved5      128M   p58*/
  {PART_USERDATA,         7080*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G     p59*/
  #endif
  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER_A,        0,         2*1024,        UFS_PART_0},
  {PART_XLOADER_B,        0,         2*1024,        UFS_PART_1},
  {PART_PTABLE,           0,         512,           UFS_PART_2},/* ptable          512K */
  {PART_FRP,              512,       512,           UFS_PART_2},/* frp             512K   p1*/
  {PART_PERSIST,          1024,      2048,          UFS_PART_2},/* persist         2048K  p2*/
  {PART_RESERVED1,        3072,      5120,          UFS_PART_2},/* reserved1       5120K  p3*/
  {PART_PTABLE_LU3,       0,         512,           UFS_PART_3},/* ptable_lu3      512K   p0*/
  {PART_VRL,              512,       512,           UFS_PART_3},/* vrl             512K   p1*/
  {PART_VRL_BACKUP,       1024,      512,           UFS_PART_3},/* vrl backup      512K   p2*/
  {PART_MODEM_SECURE,     1536,      8704,          UFS_PART_3},/* modem_secure    8704k  p3*/
  {PART_NVME,             10*1024,   6*1024,        UFS_PART_3},/* nvme            6M     p4*/
  {PART_OEMINFO,          16*1024,   64*1024,       UFS_PART_3},/* oeminfo         64M    p5*/
  {PART_SECURE_STORAGE,   80*1024,   32*1024,       UFS_PART_3},/* secure storage  32M    p6*/
  {PART_MODEM_OM,         112*1024,  32*1024,       UFS_PART_3},/* modem om        32M    p7*/
  {PART_MODEMNVM_FACTORY, 144*1024,  4*1024,        UFS_PART_3},/* modemnvm factory4M     p8*/
  {PART_MODEMNVM_BACKUP,  148*1024,  4*1024,        UFS_PART_3},/* modemnvm backup 4M     p9*/
  {PART_MODEMNVM_IMG,     152*1024,  12*1024,       UFS_PART_3},/* modemnvm img    12M    p10*/
  {PART_MODEMNVM_SYSTEM,  164*1024,  4*1024,        UFS_PART_3},/* modemnvm system 4M     p11*/
  {PART_SPLASH2,          168*1024,  80*1024,       UFS_PART_3},/* splash2         80M    p12*/
  {PART_CACHE,            248*1024,  128*1024,      UFS_PART_3},/* cache           128M     p13*/
  {PART_ODM_A,              376*1024,  128*1024,      UFS_PART_3},/* odm             128M     p14*/
  {PART_BOOTFAIL_INFO,    504*1024,  2*1024,        UFS_PART_3},/* bootfail info   2MB    p15*/
  {PART_MISC,             506*1024,  2*1024,        UFS_PART_3},/* misc            2M     p16*/
  {PART_RESERVED2,        508*1024,  32*1024,       UFS_PART_3},/* reserved2       32M    p17*/
  {PART_RESERVED10,        540*1024,  4*1024,        UFS_PART_3},/* PART_RESERVED10       4M     p18*/
  {PART_HISEE_FS,         544*1024,  8*1024,        UFS_PART_3},/* hisee_fs        8M     p19*/
  {PART_DFX,              552*1024,  16*1024,       UFS_PART_3},/* dfx             16M    p20*/
  {PART_RRECORD,          568*1024,  16*1024,       UFS_PART_3},/* rrecord         16M    p21*/
  {PART_FW_LPM3_A,        584*1024,  256,           UFS_PART_3},/* mcuimage        256K   p22*/
  {PART_RESERVED3_A,      598272,    3840,          UFS_PART_3},/*reserved3A      3840KB  p23*/
  {PART_HISEE_IMG_A,      588*1024,    4*1024,          UFS_PART_3},/*part_hisee_img_a   4*1024KB  p24*/
  {PART_FASTBOOT_A,       592*1024,  12*1024,       UFS_PART_3},/* fastboot        12M    p25*/
  {PART_VECTOR_A,         604*1024,  4*1024,        UFS_PART_3},/* avs vector      4M     p26*/
  {PART_ISP_BOOT_A,       608*1024,  2*1024,        UFS_PART_3},/* isp_boot        2M     p27*/
  {PART_ISP_FIRMWARE_A,   610*1024,  14*1024,       UFS_PART_3},/* isp_firmware    14M    p28*/
  {PART_FW_HIFI_A,        624*1024,  12*1024,       UFS_PART_3},/* hifi            12M    p29*/
  {PART_TEEOS_A,          636*1024,  8*1024,        UFS_PART_3},/* teeos           8M     p30*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A, 644*1024,  24*1024,     UFS_PART_3},/* erecovery_kernel 24M   p31*/
  {PART_ERECOVERY_RAMDISK_A, 668*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M  p32*/
  {PART_ERECOVERY_VENDOR_A, 700*1024,  8*1024,     UFS_PART_3},/* erecovery_vendor 8M     p33*/
  {PART_SENSORHUB_A,      708*1024,  16*1024,       UFS_PART_3},/* sensorhub       16M    p34*/
  {PART_KERNEL_A,           724*1024,  32*1024,     UFS_PART_3},/* kernel            32M  p35*/
#else
  {PART_ERECOVERY_KERNEL_A, 644*1024,  24*1024,     UFS_PART_3},/* erecovery_kernel 24M   p31*/
  {PART_ERECOVERY_RAMDISK_A, 668*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M  p32*/
  {PART_ERECOVERY_VENDOR_A, 700*1024,  16*1024,     UFS_PART_3},/* erecovery_vendor 16M   p33*/
  {PART_SENSORHUB_A,      716*1024,  16*1024,       UFS_PART_3},/* sensorhub       16M    p34*/
  {PART_KERNEL_A,           732*1024,  24*1024,     UFS_PART_3},/* kernel            24M    p35*/
#endif
  {PART_RAMDISK_A,        756*1024,  16*1024,       UFS_PART_3},/* ramdisk         16M    p36*/
  {PART_RECOVERY_RAMDISK_A, 772*1024,  32*1024,     UFS_PART_3},/* recovery_ramdisk 32M p37*/
  {PART_RECOVERY_VENDOR_A, 804*1024,  16*1024,      UFS_PART_3},/* recovery_vendor 16M   p38*/
  {PART_DTS_A,            820*1024,  14*1024,       UFS_PART_3},/* dtimage         14M    p39*/
  {PART_DTO_A,            834*1024,   2*1024,       UFS_PART_3},/* dtoimage         2M    p40*/
  {PART_TRUSTFIRMWARE_A,  836*1024,  2*1024,        UFS_PART_3},/* trustfirmware   2M     p41*/
  {PART_MODEM_FW_A,       838*1024,  56*1024,       UFS_PART_3},/* modem_fw        56M    p42*/
  {PART_RESERVED4_A,      894*1024,  42*1024,       UFS_PART_3},/* reserved4_A      42M    p43*/
  {PART_RECOVERY_VBMETA_A,  936*1024, 2*1024,       UFS_PART_3},/* recovery_vbmeta_a  2M    p44*/
  {PART_ERECOVERY_VBMETA_A, 938*1024, 2*1024,       UFS_PART_3},/* erecovery_vbmeta_a  2M    p45*/
  {PART_VBMETA_A,         940*1024,  4*1024,        UFS_PART_3},/* PART_VBMETA_A    4M    p46*/
  {PART_MODEMNVM_UPDATE_A,944*1024,  80*1024,       UFS_PART_3},/* modemnvm update 80M    p47*/
  {PART_PATCH_A,          1024*1024,  32*1024,      UFS_PART_3},/* patch           32M    p48*/
  {PART_VERSION_A,        1056*1024,  32*1024,      UFS_PART_3},/* version         24M    p49*/
  {PART_VENDOR_A,         1088*1024,  784*1024,     UFS_PART_3},/* vendor          784M   p50*/
  {PART_PRODUCT_A,        1872*1024, 192*1024,      UFS_PART_3},/* product         192M   p51*/
  {PART_CUST_A,           2064*1024, 192*1024,      UFS_PART_3},/* cust            192M   p52*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM_A,         2256*1024, 3552*1024,     UFS_PART_3},/* system          3552M  p53*/
  {PART_RESERVED5,        5808*1024, 128*1024,      UFS_PART_3},/* reserved5       64M    p54*/
  {PART_USERDATA,         5936*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G     p55*/
  #else
  {PART_SYSTEM_A,         2256*1024, 4688*1024,     UFS_PART_3},/* system          4688M  p53*/
  {PART_RESERVED5,        6944*1024, 128*1024,      UFS_PART_3},/* reserved5       64M    p54*/
  {PART_USERDATA,         7072*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G     p55*/
  #endif
  {"0", 0, 0, 0},
};
#endif

#endif
