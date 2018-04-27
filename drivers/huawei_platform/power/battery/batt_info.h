#ifndef BATT_INFO_H
#define BATT_INFO_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include <linux/list.h>
#include <uapi/asm-generic/ioctl.h>
#include <net/genetlink.h>
#include <net/genetlink.h>
#include <net/netlink.h>
#include <huawei_platform/log/hw_log.h>

#include "onewire_common.h"

typedef struct{
    char official;
    char binding;
    char lim_en;
    char check_ready;
    unsigned int limit_current;
    unsigned int limit_voltage;
}battery_check_result;

typedef struct{
    unsigned char *id_mask;
    unsigned char *id_example;
    unsigned char *mac;
    unsigned char *bind_id;
    char *limit_sw;
    char *no_limit;
    unsigned int id_len;
    unsigned int mac_len;
    unsigned int validity;
    unsigned int srv_on;
    unsigned int limit_current;
    unsigned int limit_voltage;
}battery_constraint;

typedef struct{
    unsigned char *datum;
    unsigned int len;
    unsigned int ic_type;
}mac_resource;

typedef struct{
    struct device_node *(*get_ic_dev_node)(void);
    int (*get_id)(unsigned char **);
    unsigned int (*get_id_len)(void);
    int (*get_mac)(unsigned char **);
    unsigned int (*get_mac_len)(void);
    int (*get_mac_resource)(mac_resource *, unsigned int);
    int (*set_random)(void);
}batt_ct_ops;

typedef struct{
    struct list_head node;
    int (*ct_ops_register)(batt_ct_ops *);
    void (*ic_memory_release)(void);
}ct_ops_reg_list;

extern struct list_head batt_ct_head;

/* return macro */
#define BATTERY_DRIVER_FAIL                         1
#define BATTERY_DRIVER_SUCCESS                      0

/* Battery constrain validity macro */
#define BIND_ID_VALIDITY_BIT                        0x01
#define LIM_SW_VALIDITY_BIT                         0x02

/* Battery certification result macro */
#define BATTERY_LIMITATION_SWITCH_OFF               0xFF
#define OFFICAL_BATTERY                             0xFF
#define ILLEGAL_BATTERY                             0x00
#define BINDING_BATTERY                             0xFF
#define UNBIND_BATTERY                              0x00
#define BATT_LIM_OFF                                0xFF
#define BATT_LIM_ON                                 0
#define CHECK_FINISHED                              0xFF
#define CHECK_RUNNING                               0

/* sys node information show macro */
#define BATT_ID_PRINT_SIZE_PER_CHAR                 3

/* NVME macro */
#define BBINFO_NV_NUMBER                            389
#define BLIMSW_NV_NUMBER                            388
#define BBINFO_NV_NAME                              "BBINFO"
#define BLIMSW_NV_NAME                              "BLIMSW"
#define BBINFO_BATT_ID_OFFSET                       8
#define LIMIT_SWITCH_SIZE                           32

/* Battery maxium current&voltage initialization value */
#define MAX_CHARGE_CURRENT                          10000
#define MAX_CHARGE_VOLTAGE                          10000

/* cdev ioctl cmd */
#define BATT_BIND_CMD                               0x000000BA
#define BATT_LIM_OFF_CMD                            0x000001BA
#define BATT_LIM_ON_CMD                             0x000002BA

/* Communication retry limitation */
#define MAX_RETRY_TIME                              10
#define IC_DECT_RETRY_NUM                           5

/* NULL pointer process*/
#define BATTERY_DRIVE_NULL_POINT(x)                                                     \
    do{                                                                                 \
        if(!x){                                                                         \
            hwlog_err("Battery-driver: NULL point: "#x", found in %s.",__func__);       \
            return BATTERY_DRIVER_FAIL;                                                 \
        }                                                                               \
    }while(0)

/* dts read property error process*/
#define BATTERY_DRIVE_DTS_READ_ERROR(x)                                                 \
    do{                                                                                 \
        if(ret){                                                                        \
            hwlog_err("Battery-driver: DTS do not have "x", needed in %s.",__func__);   \
            return BATTERY_DRIVER_FAIL;                                                 \
        }                                                                               \
    }while(0)

/* dts read property error process*/
#define BATTERY_DRIVE_COMMUNICATION_ERROR(x)                                            \
    do{                                                                                 \
        if(ret){                                                                        \
            hwlog_err("Battery-driver: Getting "#x" failed, needed in %s.",__func__);   \
        }                                                                               \
    }while(0)

/* Battery drive necessary function fail process*/
#define BATTERY_DRIVE_FUNC_PROCESS(x)                                                       \
    do{                                                                                     \
        if(ret){                                                                            \
            hwlog_err("Battery-driver: necessary function "x" failed in %s.",__func__);     \
            return BATTERY_DRIVER_FAIL;                                                     \
        } else {                                                                            \
            hwlog_info("Battery-driver: necessary function "x" finished in %s.",__func__);  \
        }                                                                                   \
    }while(0)

/* generic netlink macro */
#define     BATT_GNNL_FAMILY                            "BATT_GNNL"
#define     BATT_GNNL_HEAD_LEN                          0
#define     BATT_GNNL_VERSION                           1
#define     BATT_GNNL_CB_NUM                            2
#define     BATT_GNNL_PORTID                            0
#define     BATT_COMMON_FLAG                            0
#define     GNNL_INFO_SIZE                              128

/* wait for NVME ready */
#define     WAIT_FOR_NVME_MS                            3000

#define     CT_MAC_TPYE0                                0

/* attributes */
enum {
   BATT_UNSPEC_MESSAGE = 0,
   BATT_RAW_MESSAGE,
   __BATT_MESG_MAX,
};
#define     BATT_MESG_MAX       (__BATT_MESG_MAX - 1)

/* commands */
enum {
   BATT_SERVICE_ON = 0,
   BATT_CERTIFICAION_MAC,
   BATT_WRITE_MEMORY_MAC,
   BATT_CHANGE_STATUS_MAC,
   __BATT_CMD_MAX,
};
#define     BATT_CMD_MAX        (__BATT_CMD_MAX - 1)

#endif