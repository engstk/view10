/*
 * blackbox cleartext. (kernel run data recorder clear text recording.)
 *
 * Copyright (c) 2013 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_pub.h>

#include "rdr_hisi_ap_adapter.h"
#include "../rdr_inner.h"

/*
 * func name: rdr_hisiap_cleartext_print
 *
 * The clear text printing for the reserved debug memory of core AP
 *
 * func args:
 * @dir_path: the file directory of saved clear text
 * @log_addr: the start address of reserved memory for specified core
 * @log_len: the length of reserved memory for specified core
 *
 * return value
 *
 */
int rdr_hisiap_cleartext_print(char *dir_path, u64 log_addr, u32 log_len)
{
	struct file *fp;
	bool error;
	u32  i;
	AP_EH_ROOT *ap_root;

	if ( unlikely(IS_ERR_OR_NULL(dir_path) || IS_ERR_OR_NULL((void *)log_addr)) ) {
		printk(KERN_ERR "%s() error:dir_path 0x%pK log_addr 0x%pK.\n", 
			__func__, dir_path, (void *)log_addr);
		return -1;
	}

	if ( unlikely(log_len < (sizeof(AP_EH_ROOT) + PMU_RESET_RECORD_DDR_AREA_SIZE)) ) {
		printk(KERN_ERR "%s() error:log_len %u not enough.\n",
			__func__, log_len);
		return -1;
	}

	/* get the file descriptor from the specified directory path */
	fp = bbox_cleartext_get_filep(dir_path, "AP.txt");
	if (unlikely(IS_ERR_OR_NULL(fp))) {
		printk(KERN_ERR "%s() error:fp 0x%pK.\n", __func__, fp);
		return -1;
	}

	ap_root = (AP_EH_ROOT *)(log_addr + PMU_RESET_RECORD_DDR_AREA_SIZE);
	error = false;

	ap_root->version[PRODUCT_VERSION_LEN - 1] = '\0';
	ap_root->device_id[PRODUCT_DEVICE_LEN - 1] = '\0';

	rdr_cleartext_print(fp, &error, "=================AP_EH_ROOT START================\n");
	rdr_cleartext_print(fp, &error, "dump_magic [0x%x]\n", ap_root->dump_magic);
	rdr_cleartext_print(fp, &error, "version [%s]\n", ap_root->version);
	rdr_cleartext_print(fp, &error, "modid [0x%x]\n", ap_root->modid);
	rdr_cleartext_print(fp, &error, "e_exce_type [0x%x],\n", ap_root->e_exce_type);
	rdr_cleartext_print(fp, &error, "e_exce_subtype [0x%x],\n", ap_root->e_exce_subtype);
	rdr_cleartext_print(fp, &error, "coreid [0x%llx]\n", ap_root->coreid);
	rdr_cleartext_print(fp, &error, "slice [%llu]\n", ap_root->slice);
	rdr_cleartext_print(fp, &error, "enter_times [0x%x]\n", ap_root->enter_times);
	rdr_cleartext_print(fp, &error, "bbox_version [0x%llx]\n", ap_root->bbox_version);
	rdr_cleartext_print(fp, &error, "num_reg_regions [0x%x]\n", ap_root->num_reg_regions);

	rdr_cleartext_print(fp, &error, "wdt_kick_slice:");
	for (i = 0; i < WDT_KICK_SLICE_TIMES; i++) {
		rdr_cleartext_print(fp, &error, "%llu ", ap_root->wdt_kick_slice[i]);
	}
	rdr_cleartext_print(fp, &error, "\n");
	rdr_cleartext_print(fp, &error, "device_id %s\n", (char *)(ap_root->device_id));
	rdr_cleartext_print(fp, &error, "=================AP_EH_ROOT END--================\n");

	/* For the formal commercial version, hook trace&last task trace& is closed */

	/* the cleaning of specified file descriptor */
	bbox_cleartext_end_filep(fp, dir_path, "AP.txt");

	if (unlikely(true == error)) {
		return -1;
	}

	return 0;
}

