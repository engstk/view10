/*
 * hw_rscan_module.c
 *
 * the hw_rscan_module.c for root scanner kernel space init and deinit
 *
 * likun <quentin.lee@huawei.com>
 * likan <likan82@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/types.h>
#include "./include/hw_rscan_utils.h"
#include "./include/hw_rscan_data_uploader.h"
#include "./include/hw_rscan_scanner.h"
#include "./include/hw_rscan_proc.h"

static int __init rscan_module_init(void);
static void __exit rscan_module_exit(void);
static const char *TAG = "hw_rscan_module";

struct rscan_module_work {
	struct workqueue_struct *rscan_wq;
	struct work_struct rscan_work;
};

static struct rscan_module_work rscan_work_data;

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
static const umode_t FILE_CREAT_RW_MODE = 0660;
static const kuid_t ROOT_UID =  KUIDT_INIT((uid_t)0);
static const kgid_t SYSTEM_GID = KGIDT_INIT((gid_t)1000);
static int rs_eng_enable = RSCAN_UNINIT;
static struct proc_dir_entry *rs_eng_proc;

static ssize_t rs_eng_proc_write(struct file *file,const char __user *buffer,size_t count,loff_t *pos)
{
	char mode;
	if(count > 0) {
		if(get_user(mode,buffer))
			return EFAULT;
		if((mode != '0') && RSCAN_UNINIT == rs_eng_enable){
			/* set flag and queue init workqueue */
			rs_eng_enable = RSCAN_INIT;
			queue_work(rscan_work_data.rscan_wq, &(rscan_work_data.rscan_work));
		}
	}
	return count;
}

static int rs_eng_proc_show(struct seq_file *seq, void *v)
{
	seq_puts(seq, rs_eng_enable ? "RS ENG Enable.\n" : "RS ENG Disable.\n");
	return 0;
}

static int rs_eng_proc_open(struct inode *inode,struct file *file)
{
	return single_open(file, rs_eng_proc_show, inode->i_private);
}

static const struct file_operations eng_debug_proc_fops = {
	.open		= rs_eng_proc_open,
	.read		= seq_read,
	.write		= rs_eng_proc_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

int is_rs_eng_enable(void)
{
	return rs_eng_enable;
}
EXPORT_SYMBOL(is_rs_eng_enable);

static int create_debug_interface(void)
{
	/* create debug init interface, use procfs for compatible */
	rs_eng_proc = proc_create("rs_eng_debug", FILE_CREAT_RW_MODE, NULL, &eng_debug_proc_fops);
	if (rs_eng_proc == NULL) {
		RSLogError(TAG, "eng debug proc entry create failed");
		return -ENOMEM;
	}
	proc_set_user(rs_eng_proc, ROOT_UID, SYSTEM_GID);
	return 0;
}
#endif

static void rscan_work_init(struct work_struct *data)
{
	int result = 0;

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	if (RO_NORMAL != get_ro_secure() && RSCAN_UNINIT == rs_eng_enable) {
		/* eng mode default off, create debug interface for late init */
		create_debug_interface();
		RSLogTrace(TAG, "in engneering mode, root scan stopped");
		return;
	}
#endif

	RSLogDebug(TAG, "rscan work init.");

	do {
		/* init uploader */
		result = rscan_uploader_init();
		if (result != 0)
			break;

		/* init dynamic scanner */
		result = rscan_dynamic_init();
		if (result != 0) {
			RSLogError(TAG, "dynamic scanner init failed: %d",
						result);
			break;
		}

		/* init proc file */
		result = rscan_proc_init();
		if (result != 0) {
			RSLogError(TAG, "rscan_proc_init init failed.");
			break;
		}
	} while (0);

	if (0 != result) {
		/* The function __init should not references __exit*/
		/*rscan_module_exit();*/
		rscan_proc_deinit();
		rscan_uploader_deinit();
	}

	RSLogTrace(TAG, "+++root scan init end, result:%d", result);
}

static int __init rscan_module_init(void)
{
	rscan_work_data.rscan_wq =
				create_singlethread_workqueue("HW_ROOT_SCAN");

	if (rscan_work_data.rscan_wq == NULL) {
		RSLogError(TAG, "rscan module wq error, no mem");
		return -ENOMEM;
	}

	INIT_WORK(&(rscan_work_data.rscan_work), rscan_work_init);
	queue_work(rscan_work_data.rscan_wq, &(rscan_work_data.rscan_work));

	return 0;
}

static void __exit rscan_module_exit(void)
{
	rscan_proc_deinit();
	rscan_uploader_deinit();
	destroy_workqueue(rscan_work_data.rscan_wq);
}

late_initcall(rscan_module_init);   /* lint -save -e528 */
module_exit(rscan_module_exit);   /* lint -save -e528 */

MODULE_AUTHOR("likun <quentin.lee@huawei.com>");
MODULE_DESCRIPTION("Huawei root scanner");
