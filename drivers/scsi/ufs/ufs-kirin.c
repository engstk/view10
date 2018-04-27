/*
 * Copyright (c) 2013-2015, Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/bootdevice.h>
#include <linux/time.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <soc_crgperiph_interface.h>
#include <linux/gpio.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>
#include <linux/hisi/hisi_idle_sleep.h>
#include <linux/vmalloc.h>
#include <linux/kthread.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include "ufshcd.h"
#include "unipro.h"
#include "ufs-kirin.h"
#include "ufshci.h"
#include "dsm_ufs.h"
#ifdef CONFIG_HISI_UFS_MANUAL_BKOPS
#include "hisi_ufs_bkops.h"
#endif

struct st_caps_map {
	char *caps_name;
	uint64_t cap_bit;
};
static char ufs_product_name[32] = {0};
static int __init early_parse_ufs_product_name_cmdline(char *arg)
{
	if (arg) {
		strncpy(ufs_product_name, arg, strnlen(arg, sizeof(ufs_product_name)));
		pr_info("cmdline ufs_product_name=%s\n", ufs_product_name);
	} else {
		pr_info("no ufs_product_name cmdline\n");
	}
	return 0;
}
/*lint -e528 -esym(528,*)*/
early_param("ufs_product_name", early_parse_ufs_product_name_cmdline);
/*lint -e528 +esym(528,*)*/
/* Here external BL31 function declaration for UFS inline encrypt*/
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
noinline int atfd_hisi_uie_smc(u64 function_id, u64 arg0, u64 arg1, u64 arg2)
{
	asm volatile(
		__asmeq("%0", "x0")
		__asmeq("%1", "x1")
		__asmeq("%2", "x2")
		__asmeq("%3", "x3")
		"smc    #0\n"
	: "+r" (function_id)
	: "r" (arg0), "r" (arg1), "r" (arg2));

	return (int)function_id;
}
#endif

static u64 kirin_ufs_dma_mask = DMA_BIT_MASK(64);/*lint !e598 !e648*/

/*lint -e648 -e845*/
uint16_t ufs_kirin_mphy_read(struct ufs_hba *hba, uint16_t addr)
{
	uint16_t result;
	uint32_t value;
	/*DME_SET(16'h8117, cr_para_addr.MSB );*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x8117), (addr & 0xFF00) >> 8);
	/*DME_SET(16'h8116, cr_para_addr.LSB);*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x8116), (addr & 0xFF));
	/*DME_SET(16'h811c, 0x0);*//*trigger read*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x811C), 0);
	/*DME_GET(16'h811b, read_data.MSB);*/
	ufshcd_dme_get(hba, UIC_ARG_MIB(0x811B), &value); /* Unipro VS_mphy_disable */
	result = (uint16_t)(value & 0xFF);
	result <<= 8;
	/*DME_GET(16'h811a, read_data.LSB);*/
	ufshcd_dme_get(hba, UIC_ARG_MIB(0x811A), &value); /* Unipro VS_mphy_disable */
	result |= (uint16_t)(value & 0xFF);
	return result;
}

void ufs_kirin_mphy_write(struct ufs_hba *hba, uint16_t addr, uint16_t value)
{
	/*DME_SET(16'h8117, cr_para_addr.MSB );*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x8117), (addr & 0xFF00) >> 8);
	/*DME_SET(16'h8116, cr_para_addr.LSB);*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x8116), (addr & 0xFF));
	/*DME_SET(16'h8119, cr_para_wr_data.MSB);*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x8119), (value & 0xFF00) >> 8);
	/*DME_SET(16'h8118, cr_para_wr_data.LSB );*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x8118), (value & 0xFF));
	/*DME_SET(16'h811c, 0x0);*//*trigger write*/
	ufshcd_dme_set(hba, UIC_ARG_MIB(0x811C), 1);
}

/* Not use interrupt, instead, use polling UIC_COMMAND_COMPL to save time */
static inline int ufshcd_polling_dme_set(struct ufs_hba *hba, u32 attr_sel,
					  u32 mib_val)
{
	int retry;

	/* 100ms polling */
	retry = 0x5A9BE;
	while (--retry) {
		if (ufshcd_readl(hba, REG_CONTROLLER_STATUS) & UIC_COMMAND_READY)
			break;
	}
	if (unlikely(retry == 0)) {
		    dev_err(hba->dev,"polling UCRDY timeout\n");
			return -EIO;
	}
	ufshcd_writel(hba, UIC_COMMAND_COMPL, REG_INTERRUPT_STATUS);
	/* Write Args */
	ufshcd_writel(hba, attr_sel, REG_UIC_COMMAND_ARG_1);
	ufshcd_writel(hba, mib_val, REG_UIC_COMMAND_ARG_3);

	/* Write UIC Cmd */
	ufshcd_writel(hba, UIC_CMD_DME_SET & COMMAND_OPCODE_MASK,
		      REG_UIC_COMMAND);
	/* 500ms polling */
	retry = 0x1C50B6;
	while (--retry) {
		if (ufshcd_readl(hba, REG_INTERRUPT_STATUS) & UIC_COMMAND_COMPL)
			break;
	}
	if (unlikely(retry == 0)) {
		    dev_err(hba->dev,"polling UCCS timeout\n");
			return -ETIMEDOUT;
	}
	return 0;
}

/* polling uic method, can use in phy firmware update, which is too many uic command */
int ufs_kirin_polling_mphy_write(struct ufs_hba *hba, uint16_t addr,
				  uint16_t value)
{
	int err = 0;
	/*DME_SET(16'h8117, cr_para_addr.MSB ); */
	err |= ufshcd_polling_dme_set(hba, UIC_ARG_MIB(0x8117), (addr & 0xFF00) >> 8);
	/*DME_SET(16'h8116, cr_para_addr.LSB); */
	err |= ufshcd_polling_dme_set(hba, UIC_ARG_MIB(0x8116), (addr & 0xFF));
	/*DME_SET(16'h8119, cr_para_wr_data.MSB); */
	err |= ufshcd_polling_dme_set(hba, UIC_ARG_MIB(0x8119), (value & 0xFF00) >> 8);
	/*DME_SET(16'h8118, cr_para_wr_data.LSB ); */
	err |= ufshcd_polling_dme_set(hba, UIC_ARG_MIB(0x8118), (value & 0xFF));
	/*DME_SET(16'h811c, 0x0); *//*trigger write */
	err |= ufshcd_polling_dme_set(hba, UIC_ARG_MIB(0x811C), 1);

	return err;
}

/*lint -restore*/

void kirin_ufs_uic_log(struct ufs_hba *hba)
{
	unsigned int reg, reg_lane1, index;

	struct st_register_dump unipro_reg[] = {
		{0x15A70000, "PA_Hibern8Time"},
		{0x15AA0000, "PA_Granularity"},
		{0x15c00000, "PA_PACPFrameCount"},
		{0x15c10000, "PA_PACPErrorCount"},

		{0xD0300000, "DME_HibernateEnter"},
		{0xD0310000, "DME_HibernateEnterInd"},
		{0xD0320000, "DME_HibernateExit"},
		{0xD0330000, "DME_HibernateExitInd"},
		{0xD0600000, "DME_ErrorPHYInd"},
		{0xD0610000, "DME_ErrorPAInd"},
		{0xD0620000, "DME_ErrorDInd"},
		{0xD0630000, "DME_ErrorNInd"},
		{0xD0640000, "DME_ErrorTInd"},
		{0xD0820000, "VS_L2Status"},
		{0xD0830000, "VS_PowerState"},
		{0xd0920000, "VS_DebugTxByteCount"},
		{0xd0930000, "VS_DebugRxByteCount"},
		{0xd0940000, "VS_DebugInvalidByteEnable"},
		{0xd0950000, "VS_DebugLinkStartup"},
		{0xd0960000, "VS_DebugPwrChg"},
		{0xd0970000, "VS_DebugStates"},
		{0xd0980000, "VS_DebugCounter0"},
		{0xd0990000, "VS_DebugCounter1"},
		{0xd09a0000, "VS_DebugCounter0Mask"},
		{0xd09b0000, "VS_DebugCounter1Mask"},
		{0xd09d0000, "VS_DebugCounterOverflow"},
		{0xd09f0000, "VS_DebugCounterBMask"},
		{0xd0a00000, "VS_DebugSaveConfigTime"},
		{0xd0a10000, "VS_DebugLoopback"},
	};

	struct st_register_dump tx_phy[] = {
		{0x00210000, "TX_MODE"},
		{0x00220000, "TX_HSRATE_SERIES"},
		{0x00230000, "TX_HSGEAR"},
		{0x00240000, "TX_PWMGEAR"},
		{0x00410000, "TX_FSM_STATE"},
	};

	struct st_register_dump rx_phy[] = {
		{0x00A10000, "RX_MODE"},
		{0x00A20000, "RX_HSRATE_SERIES"},
		{0x00A30000, "RX_HSGEAR"},
		{0x00A40000, "RX_PWMGEAR"},
		{0x00C10000, "RX_FSM_STATE"},
	};

	for (index = 0;
		index < (sizeof(unipro_reg) / sizeof(struct st_register_dump));
		index++) {
		/* dont print more info if one uic cmd failed */
		if (ufshcd_dme_get(hba, unipro_reg[index].addr, &reg))
			goto out;

		dev_err(hba->dev, ": %s: 0x%08x\n", unipro_reg[index].name,
			reg);
	}

	for (index = 0;
		index < (sizeof(tx_phy) / sizeof(struct st_register_dump));
		index++) {
		/* dont print more info if one uic cmd failed */
		if (ufshcd_dme_get(hba, tx_phy[index].addr, &reg))
			goto out;

		if (ufshcd_dme_get(hba, tx_phy[index].addr | 0x1, &reg_lane1))
			goto out;

		dev_err(hba->dev, ": %s: LANE0: 0x%08x, LANE1: 0x%08x\n",
			tx_phy[index].name, reg, reg_lane1);
	}

	for (index = 0;
		index < (sizeof(rx_phy) / sizeof(struct st_register_dump));
		index++) {
		/* dont print more info if one uic cmd failed */
		if (ufshcd_dme_get(hba, rx_phy[index].addr | 0x4, &reg))
			goto out;

		if (ufshcd_dme_get(hba, rx_phy[index].addr | 0x5, &reg_lane1))
			goto out;

		dev_err(hba->dev, ": %s: LANE0: 0x%08x, LANE1: 0x%08x\n",
			rx_phy[index].name, reg, reg_lane1);
	}

out:
	return;
}

void kirin_ufs_hci_log(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;

	dev_err(hba->dev, ": --------------------------------------------------- \n");
	dev_err(hba->dev, ": \t\tHCI STANDARD REGISTER DUMP\n");
	dev_err(hba->dev, ": --------------------------------------------------- \n");
	dev_err(hba->dev, ": CAPABILITIES:                 0x%08x\n", ufshcd_readl(hba, REG_CONTROLLER_CAPABILITIES));
	dev_err(hba->dev, ": UFS VERSION:                  0x%08x\n", ufshcd_readl(hba, REG_UFS_VERSION));
	dev_err(hba->dev, ": PRODUCT ID:                   0x%08x\n", ufshcd_readl(hba, REG_CONTROLLER_DEV_ID));
	dev_err(hba->dev, ": MANUFACTURE ID:               0x%08x\n", ufshcd_readl(hba, REG_CONTROLLER_PROD_ID));
	dev_err(hba->dev, ": INTERRUPT STATUS:             0x%08x\n", ufshcd_readl(hba, REG_INTERRUPT_STATUS));
	dev_err(hba->dev, ": INTERRUPT ENABLE:             0x%08x\n", ufshcd_readl(hba, REG_INTERRUPT_ENABLE));
	dev_err(hba->dev, ": CONTROLLER STATUS:            0x%08x\n", ufshcd_readl(hba, REG_CONTROLLER_STATUS));
	dev_err(hba->dev, ": CONTROLLER ENABLE:            0x%08x\n", ufshcd_readl(hba, REG_CONTROLLER_ENABLE));
	dev_err(hba->dev, ": UIC ERR PHY ADAPTER LAYER:    0x%08x\n", ufshcd_readl(hba, REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER));
	dev_err(hba->dev, ": UIC ERR DATA LINK LAYER:      0x%08x\n", ufshcd_readl(hba, REG_UIC_ERROR_CODE_DATA_LINK_LAYER));
	dev_err(hba->dev, ": UIC ERR NETWORK LATER:        0x%08x\n", ufshcd_readl(hba, REG_UIC_ERROR_CODE_NETWORK_LAYER));
	dev_err(hba->dev, ": UIC ERR TRANSPORT LAYER:      0x%08x\n", ufshcd_readl(hba, REG_UIC_ERROR_CODE_TRANSPORT_LAYER));
	dev_err(hba->dev, ": UIC ERR DME:                  0x%08x\n", ufshcd_readl(hba, REG_UIC_ERROR_CODE_DME));
	dev_err(hba->dev, ": UTP TRANSF REQ INT AGG CNTRL: 0x%08x\n", ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL));
	dev_err(hba->dev, ": UTP TRANSF REQ LIST BASE L:   0x%08x\n", ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_BASE_L));
	dev_err(hba->dev, ": UTP TRANSF REQ LIST BASE H:   0x%08x\n", ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_BASE_H));
	dev_err(hba->dev, ": UTP TRANSF REQ DOOR BELL:     0x%08x\n", ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL));
	dev_err(hba->dev, ": UTP TRANSF REQ LIST CLEAR:    0x%08x\n", ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_CLEAR));
	dev_err(hba->dev, ": UTP TRANSF REQ LIST RUN STOP: 0x%08x\n", ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_RUN_STOP));
	dev_err(hba->dev, ": UTP TASK REQ LIST BASE L:     0x%08x\n", ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_BASE_L));
	dev_err(hba->dev, ": UTP TASK REQ LIST BASE H:     0x%08x\n", ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_BASE_H));
	dev_err(hba->dev, ": UTP TASK REQ DOOR BELL:       0x%08x\n", ufshcd_readl(hba, REG_UTP_TASK_REQ_DOOR_BELL));
	dev_err(hba->dev, ": UTP TASK REQ LIST CLEAR:      0x%08x\n", ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_CLEAR));
	dev_err(hba->dev, ": UTP TASK REQ LIST RUN STOP:   0x%08x\n", ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_RUN_STOP));
	dev_err(hba->dev, ": UIC COMMAND:                  0x%08x\n", ufshcd_readl(hba, REG_UIC_COMMAND));
	dev_err(hba->dev, ": UIC COMMAND ARG1:             0x%08x\n", ufshcd_readl(hba, REG_UIC_COMMAND_ARG_1));
	dev_err(hba->dev, ": UIC COMMAND ARG2:             0x%08x\n", ufshcd_readl(hba, REG_UIC_COMMAND_ARG_2));
	dev_err(hba->dev, ": UIC COMMAND ARG3:             0x%08x\n", ufshcd_readl(hba, REG_UIC_COMMAND_ARG_3));
	dev_err(hba->dev, ": DWC BUSTHRTL:                 0x%08x\n", ufshcd_readl(hba, UFS_REG_OCPTHRTL));
	dev_err(hba->dev, ": DWC HCLKDIV:                  0x%08x\n", ufshcd_readl(hba, UFS_REG_HCLKDIV));

	dev_err(hba->dev, ": --------------------------------------------------- \n");
	dev_err(hba->dev, ": \t\tUFS SYSCTRL REGISTER DUMP\n");
	dev_err(hba->dev, ": --------------------------------------------------- \n");
	dev_err(hba->dev, ": UFSSYS_MEMORY_CTRL:             0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_MEMORY_CTRL));
	dev_err(hba->dev, ": UFSSYS_PSW_POWER_CTRL:          0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_PSW_POWER_CTRL));
	dev_err(hba->dev, ": UFSSYS_PHY_ISO_EN:              0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_PHY_ISO_EN));
	dev_err(hba->dev, ": UFSSYS_HC_LP_CTRL:              0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_HC_LP_CTRL));
	dev_err(hba->dev, ": UFSSYS_PHY_CLK_CTRL:            0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_PHY_CLK_CTRL));
	dev_err(hba->dev, ": UFSSYS_PSW_CLK_CTRL:            0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_PSW_CLK_CTRL));
	dev_err(hba->dev, ": UFSSYS_CLOCK_GATE_BYPASS:       0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_CLOCK_GATE_BYPASS));
	dev_err(hba->dev, ": UFSSYS_RESET_CTRL_EN:           0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_RESET_CTRL_EN));
	dev_err(hba->dev, ": UFSSYS_PHY_RESET_STATUS:        0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_PHY_RESET_STATUS));
	dev_err(hba->dev, ": UFSSYS_HC_DEBUG:                0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_HC_DEBUG));
	dev_err(hba->dev, ": UFSSYS_PHY_MPX_TEST_CTRL:       0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_PHY_MPX_TEST_CTRL));
	dev_err(hba->dev, ": UFSSYS_PHY_MPX_TEST_OBSV:       0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_PHY_MPX_TEST_OBSV));
	dev_err(hba->dev, ": UFSSYS_PHY_DTB_OUT:             0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_PHY_DTB_OUT));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_HH:        0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_HH));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_H:         0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_H));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_L:         0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_L));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITORUP_H:       0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITORUP_H));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITORUP_L:       0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITORUP_L));
	dev_err(hba->dev, ": UFSSYS_MK2_CTRL:                0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_MK2_CTRL));
	dev_err(hba->dev, ": UFSSYS_UFS_SYSCTRL:             0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_UFS_SYSCTRL));
	dev_err(hba->dev, ": UFSSYS_UFS_RESET_CTRL:          0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_UFS_RESET_CTRL));
	dev_err(hba->dev, ": UFSSYS_UFS_UMECTRL:             0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_UFS_UMECTRL));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_UME_HH:    0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_UME_HH));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_UME_H:     0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_UME_H));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_UME_L:     0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_UME_L));
	dev_err(hba->dev, ": UFSSYS_UFS_MEM_CLK_GATE_BYPASS: 0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_UFS_MEM_CLK_GATE_BYPASS));
	dev_err(hba->dev, ": UFSSYS_CRG_UFS_CFG:             0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_CRG_UFS_CFG));
	dev_err(hba->dev, ": UFSSYS_CRG_UFS_CFG1:            0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_CRG_UFS_CFG1));
	dev_err(hba->dev, ": UFSSYS_UFSAXI_W_QOS_LMTR:       0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_UFSAXI_W_QOS_LMTR));
	dev_err(hba->dev, ": UFSSYS_UFSAXI_R_QOS_LMTR:       0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_UFSAXI_R_QOS_LMTR));
	dev_err(hba->dev, ": UFSSYS_CRG_UFS_STAT:            0x%08x\n", ufs_sys_ctrl_readl(host, UFSSYS_CRG_UFS_STAT));
}

int ufs_kirin_check_hibern8(struct ufs_hba *hba)
{
	int err = 0;
	u32 tx_fsm_val_0 = 0;
	u32 tx_fsm_val_1 = 0;
	unsigned long timeout = jiffies + msecs_to_jiffies(HBRN8_POLL_TOUT_MS);

	do {
		err = ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE, 0),
				     &tx_fsm_val_0);
		err |= ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE, 1),
				     &tx_fsm_val_1);
		if (err || (tx_fsm_val_0 == TX_FSM_HIBERN8 && tx_fsm_val_1 == TX_FSM_HIBERN8))
			break;

		/* sleep for max. 200us */
		usleep_range(100, 200);
	} while (time_before(jiffies, timeout));

	/*
	 * we might have scheduled out for long during polling so
	 * check the state again.
	 */
	if (time_after(jiffies, timeout)) {
		err = ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE, 0),
				     &tx_fsm_val_0);
		err |= ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE, 1),
				     &tx_fsm_val_1);
	}

	if (err) {
		dev_err(hba->dev, "%s: unable to get TX_FSM_STATE, err %d\n",
			__func__, err);
	} else if (tx_fsm_val_0 != TX_FSM_HIBERN8 || tx_fsm_val_1 != TX_FSM_HIBERN8) {
		err = -1;
		dev_err(hba->dev, "%s: invalid TX_FSM_STATE, lane0 = %d, lane1 = %d\n",
			__func__, tx_fsm_val_0, tx_fsm_val_1);
	}

	return err;
}

#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
int ufs_kirin_uie_config_init(struct ufs_hba *hba)
{
	int reg_value = 0;
	int err = 0;

	/* enable UFS cryptographic operations on transactions */
	reg_value = ufshcd_readl(hba, REG_CONTROLLER_ENABLE);
	reg_value |= CRYPTO_GENERAL_ENABLE;
	ufshcd_writel(hba, reg_value, REG_CONTROLLER_ENABLE);

	/* Here UFS driver, which set SECURITY reg 0x1 in BL31,
	 * has the permission to write scurity key registers.
	 */
	err = atfd_hisi_uie_smc(RPMB_SVC_UFS_TEST, 0x0, 0x0, 0x0);
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
	if(err) {
		dev_err(hba->dev, "%s: first set ufs inline key failed,try again.\n", __func__);
		err = atfd_hisi_uie_smc(RPMB_SVC_UFS_TEST, 0x0, 0x0, 0x0);
		if(err)
			BUG_ON(1);
	}
#endif

	return err;
}

#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
/*generate the key index use bkdrhash alg,we limit
 *the result in the range of 0~29 */
static u32 bkdrhash_alg(u8 *str, int len)
{
	u32 seed = 131;
	u32 hash = 0;
	int i;

	for (i = 0; i < len; i++) {
		hash = hash * seed + str[i];
	}

	return (hash & 0xFFFFFFFF);
}

#ifdef CONFIG_HISI_DEBUG_FS
static void test_generate_cci_dun_use_bkdrhash(u8 *key, int key_len)
{
	u32 crypto_cci;
	u64 dun;
	u32 hash_res;

	hash_res = bkdrhash_alg(key, key_len);
	crypto_cci = hash_res % MAX_CRYPTO_KEY_INDEX;
	dun = (u64)hash_res;
	pr_err("%s: ufs crypto key index is %d, dun is 0x%llx\n", __func__, crypto_cci, dun);
}
#endif
#endif

/* configure UTRD to enable cryptographic operations for this transaction. */
void ufs_kirin_uie_utrd_prepare(struct ufs_hba *hba,
		struct ufshcd_lrb *lrbp)
{
	struct utp_transfer_req_desc *req_desc = lrbp->utr_descriptor_ptr;
	u32 dword_0, dword_1, dword_3;
	u64 dun;
	u32 crypto_enable;
	u32 crypto_cci;
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
	u32 hash_res;
#else
	unsigned long flags;
#endif
	/*
	 * According to UFS 2.1 SPEC
	 * decrypte incoming payload if the command is SCSI READ operation
	 * encrypte outgoing payload if the command is SCSI WRITE operation
	 * And Kirin UFS controller only support SCSI cmd as below:
	 * READ_6/READ_10/WRITE_6/WRITE_10
	 */
	switch (lrbp->cmd->cmnd[0]) {
	case READ_10:
	case WRITE_10:
		crypto_enable = UTP_REQ_DESC_CRYPTO_ENABLE;
		break;
	default:
		return;
	}

	if (lrbp->cmd->request && lrbp->cmd->request->ci_key) {
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
		hash_res = bkdrhash_alg((u8 *)lrbp->cmd->request->ci_key,
				lrbp->cmd->request->ci_key_len);
		crypto_cci = hash_res % MAX_CRYPTO_KEY_INDEX;

#ifdef CONFIG_HISI_DEBUG_FS
		if(hba->inline_debug_flag == DEBUG_LOG_ON)
			dev_err(hba->dev, "%s: key index is %d\n", __func__, crypto_cci);
#endif
#else
		crypto_cci = lrbp->task_tag;
		spin_lock_irqsave(hba->host->host_lock, flags);
		ufs_kirin_uie_key_prepare(hba, crypto_cci, lrbp->cmd->request->ci_key);
		spin_unlock_irqrestore(hba->host->host_lock, flags);
#endif
	} else {
		return;
	}

	dun = (u64)lrbp->cmd->request->bio->index;

#if defined(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO) && defined(CONFIG_HISI_DEBUG_FS)
	if(hba->inline_debug_flag == DEBUG_LOG_ON) {
		dev_err(hba->dev, "%s: dun is 0x%llx\n", __func__, ((u64)hash_res) << 32 | dun);
	}
	if(hba->inline_debug_flag == DEBUG_CRYPTO_ON) {
		crypto_enable = UTP_REQ_DESC_CRYPTO_ENABLE;
	} else if(hba->inline_debug_flag == DEBUG_CRYPTO_OFF) {
		crypto_enable = 0x0;
	}
#endif

	dword_0 = crypto_enable | crypto_cci;
	dword_1 = (u32)(dun & 0xffffffff);
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
	dword_3 = (u32)((dun >> 32) | hash_res);
#else
	dword_3 = (u32)((dun >> 32) & 0xffffffff);
#endif

	req_desc->header.dword_0 |= cpu_to_le32(dword_0);
	req_desc->header.dword_1 = cpu_to_le32(dword_1);
	req_desc->header.dword_3 = cpu_to_le32(dword_3);
}
#endif

void ufs_kirin_regulator_init(struct ufs_hba *hba)
{
	struct device *dev = hba->dev;

	hba->vreg_info.vcc =
		devm_kzalloc(dev, sizeof(struct ufs_vreg), GFP_KERNEL);
	if (!hba->vreg_info.vcc) {
		dev_err(dev, "vcc alloc error\n");
		goto error;
	}

	hba->vreg_info.vcc->reg = devm_regulator_get(dev, "vcc");
	if (!hba->vreg_info.vcc->reg) {
		dev_err(dev, "get regulator vcc failed\n");
		goto error;
	}

	if (regulator_set_voltage(hba->vreg_info.vcc->reg, 2950000, 2950000)) {
		dev_err(dev, "set vcc voltage failed\n");
		goto error;
	}

	if (regulator_enable(hba->vreg_info.vcc->reg))
		dev_err(dev, "regulator vcc enable failed\n");

error:
	return;
}

static void ufs_kirin_pre_hce_notify(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;

	BUG_ON(!host->pericrg || !host->ufs_sys_ctrl ||
	    !host->pctrl || !host->sysctrl || !host->pmctrl);

	return;
}


int ufs_kirin_hce_enable_notify(struct ufs_hba *hba, bool status)
{
	int err = 0;

	switch (status) {/*lint !e483*/
	case PRE_CHANGE:
		ufs_kirin_pre_hce_notify(hba);
		break;
	case POST_CHANGE:
		break;
	default:
		dev_err(hba->dev, "%s: invalid status %d\n", __func__, status);
		err = -EINVAL;
		break;
	}
	return err;
}

void hisi_mphy_busdly_config(struct ufs_hba *hba,
			struct ufs_kirin_host *host)
{
	uint32_t reg = 0;
	if (host->caps & USE_HISI_MPHY_TC) {
		/*UFS_BUSTHRTL_OFF*/
		reg = ufshcd_readl(hba, UFS_REG_OCPTHRTL);
		reg &= (~((0x3f << 18) | (0xff << 0)));
		reg |= ((0x10 << 18) | (0xff << 0));
		ufshcd_writel(hba, reg, UFS_REG_OCPTHRTL);
		reg = ufshcd_readl(hba, UFS_REG_OCPTHRTL);
	}
}

/*lint -save -e483*/
int ufs_kirin_link_startup_notify(struct ufs_hba *hba, bool status)
{
	int err = 0;
	switch (status) {
	case PRE_CHANGE:
		err = ufs_kirin_link_startup_pre_change(hba);
		break;
	case POST_CHANGE:
		err = ufs_kirin_link_startup_post_change(hba);
		break;
	default:
		break;
	}

	return err;
}
/*lint -restore*/

static int ufs_kirin_get_pwr_dev_param(struct ufs_kirin_dev_params *kirin_param,
				       struct ufs_pa_layer_attr *dev_max,
				       struct ufs_pa_layer_attr *agreed_pwr)
{
	int min_kirin_gear;
	int min_dev_gear;
	bool is_dev_sup_hs = false;
	bool is_kirin_max_hs = false;

	if (dev_max->pwr_rx == FASTAUTO_MODE || dev_max->pwr_rx == FAST_MODE)
		is_dev_sup_hs = true;

	if (kirin_param->desired_working_mode == FAST) {
		is_kirin_max_hs = true;
		min_kirin_gear = min_t(u32, kirin_param->hs_rx_gear,
				       kirin_param->hs_tx_gear);
	} else {
		min_kirin_gear = min_t(u32, kirin_param->pwm_rx_gear,
				       kirin_param->pwm_tx_gear);
	}

	/*
	 * device doesn't support HS but kirin_param->desired_working_mode is
	 * HS, thus device and kirin_param don't agree
	 */
	if (!is_dev_sup_hs && is_kirin_max_hs) {
		pr_err("%s: failed to agree on power mode (device doesn't "
		       "support HS but requested power is HS)\n",
		       __func__);
		return -ENOTSUPP;
	} else if (is_dev_sup_hs && is_kirin_max_hs) {
		/*
		 * since device supports HS, it supports FAST_MODE.
		 * since kirin_param->desired_working_mode is also HS
		 * then final decision (FAST/FASTAUTO) is done according
		 * to kirin_params as it is the restricting factor
		 */
		agreed_pwr->pwr_rx = agreed_pwr->pwr_tx =
			kirin_param->rx_pwr_hs;
	} else {
		/*
		 * here kirin_param->desired_working_mode is PWM.
		 * it doesn't matter whether device supports HS or PWM,
		 * in both cases kirin_param->desired_working_mode will
		 * determine the mode
		 */
		agreed_pwr->pwr_rx = agreed_pwr->pwr_tx =
			kirin_param->rx_pwr_pwm;
	}

	/*
	 * we would like tx to work in the minimum number of lanes
	 * between device capability and vendor preferences.
	 * the same decision will be made for rx
	 */
	agreed_pwr->lane_tx =
		min_t(u32, dev_max->lane_tx, kirin_param->tx_lanes);
	agreed_pwr->lane_rx =
		min_t(u32, dev_max->lane_rx, kirin_param->rx_lanes);

	/* device maximum gear is the minimum between device rx and tx gears */
	min_dev_gear = min_t(u32, dev_max->gear_rx, dev_max->gear_tx);

	/*
	 * if both device capabilities and vendor pre-defined preferences are
	 * both HS or both PWM then set the minimum gear to be the chosen
	 * working gear.
	 * if one is PWM and one is HS then the one that is PWM get to decide
	 * what is the gear, as it is the one that also decided previously what
	 * pwr the device will be configured to.
	 */
	if ((is_dev_sup_hs && is_kirin_max_hs) ||
	    (!is_dev_sup_hs && !is_kirin_max_hs))
		agreed_pwr->gear_rx = agreed_pwr->gear_tx =
			min_t(u32, min_dev_gear, min_kirin_gear);
	else
		agreed_pwr->gear_rx = agreed_pwr->gear_tx = min_kirin_gear;

	agreed_pwr->hs_rate = kirin_param->hs_rate;

	pr_err("ufs final power mode: gear = %d, lane = %d, pwr = %d, "
		"rate = %d\n",
		agreed_pwr->gear_rx, agreed_pwr->lane_rx, agreed_pwr->pwr_rx,
		agreed_pwr->hs_rate);
	return 0;
}

void ufs_kirin_cap_fill(struct ufs_kirin_host *host, struct ufs_kirin_dev_params *ufs_kirin_cap)
{
	if (host->caps & USE_HS_GEAR4) {
		ufs_kirin_cap->hs_rx_gear = UFS_HS_G4;
		ufs_kirin_cap->hs_tx_gear = UFS_HS_G4;
		ufs_kirin_cap->desired_working_mode = FAST;
	} else if (host->caps & USE_HS_GEAR3) {
		ufs_kirin_cap->hs_rx_gear = UFS_HS_G3;
		ufs_kirin_cap->hs_tx_gear = UFS_HS_G3;
		ufs_kirin_cap->desired_working_mode = FAST;
	} else if (host->caps & USE_HS_GEAR2) {
		ufs_kirin_cap->hs_rx_gear = UFS_HS_G2;
		ufs_kirin_cap->hs_tx_gear = UFS_HS_G2;
		ufs_kirin_cap->desired_working_mode = FAST;
	} else if (host->caps & USE_HS_GEAR1) {
		ufs_kirin_cap->hs_rx_gear = UFS_HS_G1;
		ufs_kirin_cap->hs_tx_gear = UFS_HS_G1;
		ufs_kirin_cap->desired_working_mode = FAST;
	} else {
		ufs_kirin_cap->hs_rx_gear = 0;
		ufs_kirin_cap->hs_tx_gear = 0;
		ufs_kirin_cap->desired_working_mode = SLOW;
	}
}

int ufs_kirin_pwr_change_notify(struct ufs_hba *hba, bool status,
				       struct ufs_pa_layer_attr *dev_max_params,
				       struct ufs_pa_layer_attr *dev_req_params)
{
	struct ufs_kirin_dev_params ufs_kirin_cap;
	struct ufs_kirin_host *host = hba->priv;
	int ret = 0;
#if 0
	uint32_t value;
#endif

	if (!dev_req_params) {
		pr_err("%s: incoming dev_req_params is NULL\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	switch (status) {/*lint !e483*/
	case PRE_CHANGE:
		if (host->caps & USE_ONE_LANE) {
			ufs_kirin_cap.tx_lanes = 1;
			ufs_kirin_cap.rx_lanes = 1;
		} else {
			ufs_kirin_cap.tx_lanes = 2;
			ufs_kirin_cap.rx_lanes = 2;
		}

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
		if (hba->hs_single_lane) {
			ufs_kirin_cap.tx_lanes = 1;
			ufs_kirin_cap.rx_lanes = 1;
		}
#endif

		ufs_kirin_cap_fill(host, &ufs_kirin_cap);

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
		if (hba->use_pwm_mode)
			ufs_kirin_cap.desired_working_mode = SLOW;
#endif

		ufs_kirin_cap.pwm_rx_gear = UFS_KIRIN_LIMIT_PWMGEAR_RX;
		ufs_kirin_cap.pwm_tx_gear = UFS_KIRIN_LIMIT_PWMGEAR_TX;
		ufs_kirin_cap.rx_pwr_pwm = UFS_KIRIN_LIMIT_RX_PWR_PWM;
		ufs_kirin_cap.tx_pwr_pwm = UFS_KIRIN_LIMIT_TX_PWR_PWM;
		/*hynix not support fastauto now*/
		if (host->caps & BROKEN_FASTAUTO) {
			ufs_kirin_cap.rx_pwr_hs = FAST_MODE;
			ufs_kirin_cap.tx_pwr_hs = FAST_MODE;
		} else {
			ufs_kirin_cap.rx_pwr_hs = FASTAUTO_MODE;
			ufs_kirin_cap.tx_pwr_hs = FASTAUTO_MODE;
		}

		if (host->caps & USE_RATE_B)
			ufs_kirin_cap.hs_rate = PA_HS_MODE_B;
		else
			ufs_kirin_cap.hs_rate = PA_HS_MODE_A;

		ret = ufs_kirin_get_pwr_dev_param(
			&ufs_kirin_cap, dev_max_params, dev_req_params);
		if (ret) {
			pr_err("%s: failed to determine capabilities\n",
			       __func__);
			goto out;
		}
		/*for hisi MPHY*/
		deemphasis_config(host, hba, dev_req_params);

		if (host->caps & USE_HISI_MPHY_TC) {
			adapt_pll_to_power_mode(hba);
		}

		ufs_kirin_pwr_change_pre_change(hba);
		break;
	case POST_CHANGE:
		break;
	default:
		ret = -EINVAL;
		break;
	}
out:
	return ret;
}

/* platform_get_resource will require resource exclusively, ufs_sys_ctrl used
 * for ufs only, but pctrl and pericrg are common resource */
int ufs_kirin_get_resource(struct ufs_kirin_host *host)
{
	struct resource *mem_res;
	struct device_node *np = NULL;
	struct device *dev = host->hba->dev;
	struct platform_device *pdev = to_platform_device(dev);

	/* get resource of ufs sys ctrl */
	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	host->ufs_sys_ctrl = devm_ioremap_resource(dev, mem_res);
	if (!host->ufs_sys_ctrl) {
		dev_err(dev, "cannot ioremap for ufs sys ctrl register\n");
		return -ENOMEM;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
	if (!np) {
		dev_err(host->hba->dev,
			"can't find device node \"hisilicon,crgctrl\"\n");
		return -ENXIO;
	}

	host->pericrg = of_iomap(np, 0);
	if (!host->pericrg) {
		dev_err(host->hba->dev, "crgctrl iomap error!\n");
		return -ENOMEM;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pctrl");
	if (!np) {
		dev_err(host->hba->dev,
			"can't find device node \"hisilicon,pctrl\"\n");
		return -ENXIO;
	}

	host->pctrl = of_iomap(np, 0);
	if (!host->pctrl) {
		dev_err(host->hba->dev, "pctrl iomap error!\n");
		return -ENOMEM;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	if (!np) {
		dev_err(host->hba->dev,
			"can't find device node \"hisilicon,pmctrl\"\n");
		return -ENXIO;
	}

	host->pmctrl = of_iomap(np, 0);
	if (!host->pmctrl) {
		dev_err(host->hba->dev, "pmctrl iomap error!\n");
		return -ENOMEM;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (!np) {
		dev_err(host->hba->dev,
			"can't find device node \"hisilicon,sysctrl\"\n");
		return -ENXIO;
	}

	host->sysctrl = of_iomap(np, 0);
	if (!host->sysctrl) {
		dev_err(host->hba->dev, "sysctrl iomap error!\n");
		return -ENOMEM;
	}

	/* we only use 64 bit dma */
	dev->dma_mask = &kirin_ufs_dma_mask;

	return 0;
}

/*lint -save -e715*/
static ssize_t ufs_kirin_inline_stat_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	struct ufs_hba *hba = dev_get_drvdata(dev);
#endif
	int ret_show = 0;

#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	if (ufshcd_readl(hba, REG_CONTROLLER_CAPABILITIES)
					& MASK_INLINE_ENCRYPTO_SUPPORT)
		ret_show = 1;
#endif

	return snprintf(buf, PAGE_SIZE, "%d\n", ret_show);
}
/*lint -restore*/

void ufs_kirin_inline_crypto_attr(struct ufs_hba *hba)
{
	hba->inline_state.inline_attr.show = ufs_kirin_inline_stat_show;

	sysfs_attr_init(&hba->inline_state.inline_attr.attr);
	hba->inline_state.inline_attr.attr.name = "ufs_inline_stat";
	hba->inline_state.inline_attr.attr.mode = S_IRUGO;
	if (device_create_file(hba->dev, &hba->inline_state.inline_attr))
		dev_err(hba->dev, "Failed to create sysfs for ufs_inline_state\n");
}

#if defined(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO) && defined(CONFIG_HISI_DEBUG_FS)
static ssize_t ufs_kirin_inline_debug_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);

	if (hba->inline_debug_flag == DEBUG_LOG_ON || hba->inline_debug_flag == DEBUG_CRYPTO_ON) {
		return snprintf(buf, PAGE_SIZE, "%s\n", "on");
	} else {
		return snprintf(buf, PAGE_SIZE, "%s\n", "off");
	}
}

static ssize_t ufs_kirin_inline_debug_store(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);

	if (sysfs_streq(buf, "off")) {
		hba->inline_debug_flag = DEBUG_LOG_OFF;
	} else if (sysfs_streq(buf, "on")) {
		hba->inline_debug_flag = DEBUG_LOG_ON;
	} else if(sysfs_streq(buf, "crypto_on")) {
		hba->inline_debug_flag = DEBUG_CRYPTO_ON;
	} else if(sysfs_streq(buf, "crypto_off")) {
		hba->inline_debug_flag = DEBUG_CRYPTO_OFF;
	} else {
		pr_err("%s: invalid input debug parameter.\n", __func__);
		return -EINVAL;
	}

	return count;
}

static ssize_t ufs_kirin_inline_dun_cci_test(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count)
{
	int i;
	char buf_temp[65] = {0};

	if(count != 65) {
		pr_err("%s: the input key len is not 64.\n", __func__);
		return count;
	}

	for(i = 0; i < 64; i++) {
		buf_temp[i] = buf[i];
	}
	buf_temp[64] = '\0';
	pr_err("%s: input key is %s\n", __func__, buf_temp);
	test_generate_cci_dun_use_bkdrhash((u8 *)buf_temp, 64);
	return count;
}

static void ufs_kirin_inline_crypto_debug_init(struct ufs_hba *hba)
{
	hba->inline_debug_flag = DEBUG_LOG_OFF;

	hba->inline_debug_state.inline_attr.show = ufs_kirin_inline_debug_show;
	hba->inline_debug_state.inline_attr.store = ufs_kirin_inline_debug_store;
	sysfs_attr_init(&hba->inline_debug_state.inline_attr.attr);
	hba->inline_debug_state.inline_attr.attr.name = "ufs_inline_debug";
	hba->inline_debug_state.inline_attr.attr.mode = S_IRUGO | S_IWUSR;
	if (device_create_file(hba->dev, &hba->inline_debug_state.inline_attr))
		dev_err(hba->dev, "Failed to create sysfs for inline_debug_state\n");

	hba->inline_dun_cci_test.inline_attr.store = ufs_kirin_inline_dun_cci_test;
	sysfs_attr_init(&hba->inline_dun_cci_test.inline_attr.attr);
	hba->inline_dun_cci_test.inline_attr.attr.name = "ufs_inline_dun_cci_test";
	hba->inline_dun_cci_test.inline_attr.attr.mode = S_IWUSR;
	if (device_create_file(hba->dev, &hba->inline_dun_cci_test.inline_attr))
		dev_err(hba->dev, "Failed to create sysfs for inline_dun_cci_test\n");
}
#endif

void ufs_kirin_set_pm_lvl(struct ufs_hba *hba)
{
	hba->rpm_lvl = UFS_PM_LVL_1;
	hba->spm_lvl = UFS_PM_LVL_3;
}

/**
 * ufs_kirin_advertise_quirks - advertise the known KIRIN UFS controller quirks
 * @hba: host controller instance
 *
 * KIRIN UFS host controller might have some non standard behaviours (quirks)
 * than what is specified by UFSHCI specification. Advertise all such
 * quirks to standard UFS host controller driver so standard takes them into
 * account.
 */
void ufs_kirin_advertise_quirks(struct ufs_hba *hba)
{
	/* put all our quirks here */
	/*hba->quirks |= UFSHCD_QUIRK_BROKEN_LCC;*/
}

static inline void ufs_kirin_populate_caps_dt(struct device_node *np,
					      struct ufs_kirin_host *host)
{
	unsigned int idx;
	struct st_caps_map caps_arry[] = {
		{"ufs-kirin-use-rate-B", USE_RATE_B},
		{"ufs-kirin-broken-fastauto", BROKEN_FASTAUTO},
		{"ufs-kirin-use-one-line", USE_ONE_LANE},
		{"ufs-kirin-use-HS-GEAR4", USE_HS_GEAR4},
		{"ufs-kirin-use-HS-GEAR3", USE_HS_GEAR3},
		{"ufs-kirin-use-HS-GEAR2", USE_HS_GEAR2},
		{"ufs-kirin-use-HS-GEAR1", USE_HS_GEAR1},
		{"ufs-kirin-use-hisi-mphy-tc", USE_HISI_MPHY_TC},
		{"ufs-kirin-broken-clk-gate-bypass", BROKEN_CLK_GATE_BYPASS},
	};

	for (idx = 0; idx < sizeof(caps_arry) / sizeof(struct st_caps_map);
	     idx++) {
		if (of_find_property(np, caps_arry[idx].caps_name, NULL))
			host->caps |= caps_arry[idx].cap_bit;
	}
}

static void ufs_kirin_populate_quirks_dt(struct device_node *np,
					struct ufs_kirin_host *host)
{
	if (of_find_property(np, "ufs-kirin-unipro-termination", NULL))
		host->hba->quirks |= UFSHCD_QUIRK_UNIPRO_TERMINATION;

	if (of_find_property(np, "ufs-kirin-unipro-scrambing", NULL))
		host->hba->quirks |= UFSHCD_QUIRK_UNIPRO_SCRAMBLING;

}

#ifdef CONFIG_HISI_UFS_MANUAL_BKOPS
static void ufs_kirin_populate_mgc_dt(struct device_node *parent_np,
					struct ufs_kirin_host *host)
{
	struct device_node *child_np;
	char *compatible;
	char *model;
	char *rev;
	unsigned int man_id = 0;
	int ret = 0;
	int is_white;
	struct hisi_ufs_bkops_id *bkops_id;
	struct ufs_hba *hba = host->hba;
	struct device *dev = hba->dev;

	INIT_LIST_HEAD(&host->hba->bkops_whitelist);
	INIT_LIST_HEAD(&host->hba->bkops_blacklist);

	for_each_child_of_node(parent_np, child_np) {
		ret = of_property_read_string(child_np, "compatible", (const char **)(&compatible));
		if (ret) {
			pr_err("check the compatible %s\n", child_np->name);
			continue;
		} else {
			if (!strcmp("white", compatible))/*lint !e421*/
				is_white = 1;
			else if (!strcmp("black", compatible))/*lint !e421*/
				is_white = 0;
			else {
				pr_err("check the compatible %s\n", child_np->name);
				continue;
			}
		}

		ret = of_property_read_u32(child_np, "manufacturer_id", &man_id);
		if (ret) {
			pr_err("check the manufacturer_id %s\n", child_np->name);
			continue;
		}

		ret = of_property_read_string(child_np, "model", (const char **)(&model));
		if (ret) {
			pr_err("check the model %s\n", child_np->name);
			continue;
		}

		ret = of_property_read_string(child_np, "rev", (const char **)(&rev));
		if (ret) {
			pr_err("check the rev %s\n", child_np->name);
			continue;
		}

		bkops_id = devm_kzalloc(dev, sizeof(*bkops_id), GFP_KERNEL);
		if (!bkops_id) {
			pr_err("%s %d Failed to alloc bkops_id\n", __func__, __LINE__);
			return;
		}

		bkops_id->manufacturer_id = man_id;
		bkops_id->ufs_model = model;
		bkops_id->ufs_rev = rev;
		INIT_LIST_HEAD(&bkops_id->p);
		if (is_white)
			list_add(&bkops_id->p, &hba->bkops_whitelist);
		else
			list_add(&bkops_id->p, &hba->bkops_blacklist);
	}
} /*lint !e429*/
#endif /* CONFIG_HISI_UFS_MANUAL_BKOPS */

void ufs_kirin_populate_dt(struct device *dev,
				  struct ufs_kirin_host *host)
{
	struct device_node *np = dev->of_node;
	int ret;

	if (!np) {
		dev_err(dev, "can not find device node\n");
		return;
	}

	ufs_kirin_populate_caps_dt(np, host);
#ifdef CONFIG_HISI_UFS_MANUAL_BKOPS
	ufs_kirin_populate_mgc_dt(np, host);
#endif
#ifdef CONFIG_SCSI_UFS_KIRIN_LINERESET_CHECK
	if (of_find_property(np, "ufs-kirin-linereset-check-disable", NULL))
		host->hba->bg_task_enable = false;
	else
		host->hba->bg_task_enable = true;
#endif

	if (of_find_property(np, "ufs-kirin-use-auto-H8", NULL)) {
		host->caps |= USE_AUTO_H8;
		host->hba->caps |= UFSHCD_CAP_AUTO_HIBERN8;	/*lint !e648 */
		host->hba->ahit_ts = 4;	/* 4 -> 10ms */
		host->hba->ahit_ah8itv = 1;
	}

	ufs_kirin_populate_quirks_dt(np, host);
	if (!of_property_read_u32(np, "reset-gpio", &(host->reset_gpio))) {
		if (0 > gpio_request(host->reset_gpio, "ufs_device_reset")) {
			pr_err("%s: could not request gpio %d\n", __func__,
			       host->reset_gpio);
			host->reset_gpio = 0xFFFFFFFF;
		}
	} else {
		host->reset_gpio = 0xFFFFFFFF;
	}

	if (of_find_property(np, "ufs-kirin-ssu-by-self", NULL))
		host->hba->caps |= UFSHCD_CAP_SSU_BY_SELF;

	if (of_find_property(np, "ufs-on-emulator", NULL))
		host->hba->host->is_emulator = 1;
	else
		host->hba->host->is_emulator = 0;

	ret =
	    of_property_match_string(np, "ufs-0db-equalizer-product-names",
				     ufs_product_name);
	if (ret >= 0) {
		dev_info(dev, "find %s in dts\n", ufs_product_name);
		host->tx_equalizer = 0;
	} else {
#ifdef UFS_TX_EQUALIZER_35DB
		host->tx_equalizer = 35;
#endif
#ifdef UFS_TX_EQUALIZER_60DB
		host->tx_equalizer = 60;
#endif
	}

	/*ufs reset retry num*/
	if (of_property_read_u32(np, "reset_retry_max",
				&(host->hba->reset_retry_max))) {
		host->hba->reset_retry_max =
			MAX_HOST_RESET_RETRIES;
		dev_info(dev, "can not find retry max, use default val\n");
	}

	if (of_find_property(np, "broken-hce", NULL))
		host->hba->caps |= UFSHCD_QUIRK_BROKEN_HCE;
}

/*lint +e648 +e845*/

/**
 * ufs_kirin_init
 * @hba: host controller instance
 */
int ufs_kirin_init(struct ufs_hba *hba)
{
	int err = 0;
	struct device *dev = hba->dev;
	struct ufs_kirin_host *host;

#ifdef CONFIG_HISI_BOOTDEVICE
	if (get_bootdevice_type() == BOOT_DEVICE_UFS)
		set_bootdevice_name(dev);
#endif

	host = devm_kzalloc(dev, sizeof(*host), GFP_KERNEL);
	if (!host) {
		err = -ENOMEM;
		dev_err(dev, "%s: no memory for kirin ufs host\n", __func__);
		goto out;
	}

	host->hba = hba;
	hba->priv = (void *)host;

	host->clk_ufsio_ref = devm_clk_get(dev, "clk_ufsio_ref");
	if (IS_ERR(host->clk_ufsio_ref)) {
		err = PTR_ERR(host->clk_ufsio_ref);
		dev_err(dev, "clk_ufsio_ref not found.\n");
		goto out;
	}

	ufs_kirin_advertise_quirks(hba);

	ufs_kirin_set_pm_lvl(hba);

	ufs_kirin_populate_dt(dev, host);
	if ((host->caps & USE_HISI_MPHY_TC) && !host->i2c_client) {
		i2c_chipsel_gpio_config(host, dev);
		err = create_i2c_client(hba);
		if (err) {
			dev_err(dev, "create i2c client error\n");
			goto host_free;
		}
	}
	err = ufs_kirin_get_resource(host);
	if (err)
		goto host_free;

	ufs_kirin_regulator_init(hba);

	ufs_clk_init(hba);

	ufs_soc_init(hba);

#ifdef CONFIG_HISI_BOOTDEVICE
	if (get_bootdevice_type() == BOOT_DEVICE_UFS) {
#endif
		ufs_kirin_inline_crypto_attr(hba);

#if defined(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO) && defined(CONFIG_HISI_DEBUG_FS)
		ufs_kirin_inline_crypto_debug_init(hba);
#endif

#ifdef CONFIG_HISI_BOOTDEVICE
	}
#endif

	goto out;

host_free:
	devm_kfree(dev, host);
	hba->priv = NULL;
out:
	return err;
}

void ufs_kirin_exit(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = hba->priv;

	if ((host->caps & USE_HISI_MPHY_TC) && host->i2c_client) {
		i2c_unregister_device(host->i2c_client);
		host->i2c_client = NULL;
	}

	return;
}

/*lint -save -e529 -e438 -e732 -e845*/
#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
int ufs_kirin_get_pwr_by_sysctrl(struct ufs_hba *hba)
{
	struct ufs_kirin_host *host = (struct ufs_kirin_host *)hba->priv;
	u32 link_state;
	u32 lane0_rx_state;

	ufs_sys_ctrl_writel(host, 0x08081010, PHY_MPX_TEST_CTRL);
	wmb();
	link_state = ufs_sys_ctrl_readl(host, PHY_MPX_TEST_OBSV);
	lane0_rx_state = (link_state & (0x7 << 8)) >> 8;

	if (lane0_rx_state == 2)
		return SLOW;
	else if (lane0_rx_state == 3)
		return FAST;
	else
		return -1;
}
#endif
/*lint -restore*/


/**
 * struct ufs_hba_kirin_vops - UFS KIRIN specific variant operations
 *
 * The variant operations configure the necessary controller and PHY
 * handshake during initialization.
 */
/*lint -save -e785*/
const struct ufs_hba_variant_ops ufs_hba_kirin_vops = {
	.name = "kirin",
	.init = ufs_kirin_init,
	.exit = ufs_kirin_exit,
	.setup_clocks = NULL,
	.hce_enable_notify = ufs_kirin_hce_enable_notify,
	.link_startup_notify = ufs_kirin_link_startup_notify,
	.pwr_change_notify = ufs_kirin_pwr_change_notify,
	.full_reset = ufs_kirin_full_reset,
	.device_reset = ufs_kirin_device_hw_reset,
	.suspend_before_set_link_state = ufs_kirin_suspend_before_set_link_state,
	.suspend = ufs_kirin_suspend,
	.resume = ufs_kirin_resume,
	.resume_after_set_link_state = ufs_kirin_resume_after_set_link_state,
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	.uie_config_init = ufs_kirin_uie_config_init,
	.uie_utrd_pre = ufs_kirin_uie_utrd_prepare,
#endif
	.dbg_hci_dump = kirin_ufs_hci_log,
	.dbg_uic_dump = kirin_ufs_uic_log,
#ifdef CONFIG_SCSI_UFS_KIRIN_LINERESET_CHECK
	.background_thread = ufs_kirin_daemon_thread,
#endif
#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
	.get_pwr_by_debug_register = ufs_kirin_get_pwr_by_sysctrl,
#endif
};
/*lint -restore*/

EXPORT_SYMBOL(ufs_hba_kirin_vops);
