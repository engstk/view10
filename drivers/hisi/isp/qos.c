/*
 * hisilicon ISP driver, qos.c
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/iommu.h>
#include "hisp_internel.h"
#include "soc_vivo_bus_interface.h"
#include "soc_acpu_baseaddr_interface.h"

/* QOS */
#define QOS_FIX_MODE 0x0
#define QOS_BYPASS_MODE 0x2
#define QOS_PRIO_1 0x0
#define QOS_PRIO_2 0x101
#define QOS_PRIO_3 0x202
#define QOS_PRIO_3_PLUS_RD 0x302
#define QOS_PRIO_4 0x303
#define QOS_BANDWIDTH_ISP  0x1000
#define QOS_SATURATION_ISP 0x20

int ispcpu_qos_cfg(void)
{
    void __iomem* vivobus_base;

    pr_info("[%s] +\n", __func__);

    vivobus_base = get_regaddr_by_pa(SOC_ACPU_NOC_ISP_Service_Target_BASE_ADDR);
    if (vivobus_base == NULL) {
        pr_err("[%s] vivobus_base remap fail\n", __func__);
        return -ENOMEM;
    }
    pr_info("[%s]  vivobus_base.%pK, ", __func__, vivobus_base);

    __raw_writel(QOS_PRIO_3,      (volatile void __iomem*)(SOC_VIVO_BUS_ISP_RD_QOS_PRIORITY_ADDR(vivobus_base)));
    __raw_writel(QOS_BYPASS_MODE, (volatile void __iomem*)(SOC_VIVO_BUS_ISP_RD_QOS_MODE_ADDR(vivobus_base)));
    __raw_writel(QOS_PRIO_3,      (volatile void __iomem*)(SOC_VIVO_BUS_ISP_WR_QOS_PRIORITY_ADDR(vivobus_base)));
    __raw_writel(QOS_BYPASS_MODE, (volatile void __iomem*)(SOC_VIVO_BUS_ISP_WR_QOS_MODE_ADDR(vivobus_base)));
    __raw_writel(QOS_PRIO_4,      (volatile void __iomem*)(SOC_VIVO_BUS_A7T0VIVOBUS_RD_QOS_PRIORITY_ADDR(vivobus_base)));
    __raw_writel(QOS_FIX_MODE,    (volatile void __iomem*)(SOC_VIVO_BUS_A7T0VIVOBUS_RD_QOS_MODE_ADDR(vivobus_base)));
    __raw_writel(QOS_PRIO_4,      (volatile void __iomem*)(SOC_VIVO_BUS_A7T0VIVOBUS_WR_QOS_PRIORITY_ADDR(vivobus_base)));
    __raw_writel(QOS_FIX_MODE,    (volatile void __iomem*)(SOC_VIVO_BUS_A7T0VIVOBUS_WR_QOS_MODE_ADDR(vivobus_base)));
    __raw_writel(QOS_PRIO_3,      (volatile void __iomem*)(SOC_VIVO_BUS_ISP1_RD_QOS_PRIORITY_ADDR(vivobus_base)));
    __raw_writel(QOS_BYPASS_MODE, (volatile void __iomem*)(SOC_VIVO_BUS_ISP1_RD_QOS_MODE_ADDR(vivobus_base)));
    __raw_writel(QOS_PRIO_3,      (volatile void __iomem*)(SOC_VIVO_BUS_ISP1_WR_QOS_PRIORITY_ADDR(vivobus_base)));
    __raw_writel(QOS_BYPASS_MODE, (volatile void __iomem*)(SOC_VIVO_BUS_ISP1_WR_QOS_MODE_ADDR(vivobus_base)));


    pr_info("QOS : ISP.rd.(prio.0x%x, mode.0x%x), ISP.wr.(prio.0x%x, mode.0x%x), A7.rd.(prio.0x%x, mode.0x%x), A7.wr.(prio.0x%x, mode.0x%x)\n",
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_ISP_RD_QOS_PRIORITY_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_ISP_RD_QOS_MODE_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_ISP_WR_QOS_PRIORITY_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_ISP_WR_QOS_MODE_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_A7T0VIVOBUS_RD_QOS_PRIORITY_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_A7T0VIVOBUS_RD_QOS_MODE_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_A7T0VIVOBUS_WR_QOS_PRIORITY_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_A7T0VIVOBUS_WR_QOS_MODE_ADDR(vivobus_base))));

    pr_info("QOS : ISP1.rd.(prio.0x%x, mode.0x%x), ISP1.wr.(prio.0x%x, mode.0x%x)\n",
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_ISP1_RD_QOS_PRIORITY_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_ISP1_RD_QOS_MODE_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_ISP1_WR_QOS_PRIORITY_ADDR(vivobus_base))),
        __raw_readl((volatile void __iomem*)(SOC_VIVO_BUS_ISP1_WR_QOS_MODE_ADDR(vivobus_base))));


    return 0;
}

