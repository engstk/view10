#ifndef DS28EL15_H
#define DS28EL15_H

#include "batt_info.h"
#include "maxim_onewire.h"

typedef struct {
    struct platform_device *pdev;
    maxim_onewire_ic_des ic_des;
}ds28el15_des;

/* MAC types */
#define DS28EL15_CT_MAC_PAGE0                       CT_MAC_TPYE0
#define DS28EL15_CT_MAC_SIZE                        128
#define DS28EL15_MAC_RES                            0x00

/* ds28el15 return value */
#define DS28EL15_SUCCESS                            0
#define DS28EL15_FAIL                               1


/* onewire communication error process*/
#define DS28EL15_COMMUNICATION_INFO(x)                                             \
    do{                                                                             \
        if(ret){                                                                    \
            hwlog_info("Communication: get "#x" failed in %s.",__func__);           \
        }                                                                           \
    }while(0)

#endif