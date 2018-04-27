#ifndef ONEWIRE_PHY_PL061_GPIO_H
#define ONEWIRE_PHY_PL061_GPIO_H

#include <linux/delay.h>
#include <linux/timex.h>
#include <generated/uapi/linux/version.h>
#include <hisi_gpio.h>
#include "onewire_phy_common.h"

#if ( LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0) )

typedef struct {
    spinlock_t *lock;
    unsigned long gpio_phy_base_addr;
    unsigned long length;
    unsigned int offset;
    volatile void *gpio_data_addr;
    volatile void *gpio_dir_addr;
    unsigned char gpio_dir_out_data;
    unsigned char gpio_dir_in_data;
}onewire_gpio_des;

#endif/* version macro */

/* PL061 */
#define PL061_DIR_OFFSET                    0x400
#define PL061_DATA_OFFSET                   2

/* device tree bind */
#define GPIO_CHIP_PHANDLE_INDEX             0
#define GPIO_REG_PROPERTY_SIZE              4
#define ONEWIRE_GPIO_OFFSET_INDEX           1
#define ADDRESS_HIGH32BIT                   0
#define ADDRESS_LOW32BIT                    1
#define LENGTH_HIGH32BIT                    2
#define LENGTH_LOW32BIT                     3
#define SHIFT_32                            32
#define GPIO_INDEX                          0


#endif
