#ifndef ONEWIRE_PHY_COMMON_H
#define ONEWIRE_PHY_COMMON_H

#include <linux/of.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/of_gpio.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/consumer.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include "onewire_common.h"

#define LOW_VOLTAGE                         0
#define HIGH_VOLTAGE                        1

#define SHIFT_0                             0
#define SHIFT_1                             1
#define SHIFT_2                             2
#define SHIFT_3                             3
#define SHIFT_4                             4
#define SHIFT_5                             5
#define SHIFT_6                             6
#define SHIFT_7                             7

#define BIT_0                               0x01
#define BIT_1                               0x02
#define BIT_2                               0x04
#define BIT_3                               0x08
#define BIT_4                               0x10
#define BIT_5                               0x20
#define BIT_6                               0x40
#define BIT_7                               0x80

#define FIRST_TIME_PROPERTY                 0
#define SECOND_TIME_PROPERTY                1
#define THIRD_TIME_PROPERTY                 2

/* */
#define ONEWIRE_PHY_SUCCESS                 0
#define ONEWIRE_GPIO_FAIL                   1
#define ONEWIRE_PHY_MATCH_FAIL              2

/* dts read property error process*/
#define ONEWIRE_PHY_DTS_READ_ERROR(x)                                               \
    do{                                                                             \
        if(ret){                                                                    \
            hwlog_err("DTS do not have "x", needed in %s.",__func__);               \
            return ONEWIRE_DTS_FAIL;                                                \
        }                                                                           \
    }while(0)

/* NULL point process*/
#define ONEWIRE_PHY_NULL_POINT(x)                                                   \
    do{                                                                             \
        if(!x){                                                                     \
            hwlog_err("NULL point: "#x", found in %s.",__func__);                   \
            return ONEWIRE_NULL_INPARA;                                             \
        }                                                                           \
    }while(0)

#endif