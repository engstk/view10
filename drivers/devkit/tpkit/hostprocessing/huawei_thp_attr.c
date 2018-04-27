/*
 * Huawei Touchscreen Driver
 *
 * Copyright (C) 2017 Huawei Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/ctype.h>
#include "huawei_thp.h"
#include "huawei_thp_attr.h"

#define SYSFS_PROPERTY_PATH	  "afe_properties"
#define SYSFS_TOUCH_PATH   "touchscreen"
#define SYSFS_PLAT_TOUCH_PATH	"huawei_touch"

u8 g_thp_log_cfg = 0;
EXPORT_SYMBOL(g_thp_log_cfg);

static ssize_t thp_tui_wake_up_enable_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data();

	ret = strncmp(buf, "open", sizeof("open"));
	if(ret == 0){
		cd->thp_ta_waitq_flag = WAITQ_WAKEUP;
		wake_up_interruptible(&(cd->thp_ta_waitq));
		THP_LOG_ERR("%s wake up thp_ta_flag\n", __func__);
	}
	return count;
}

static ssize_t thp_tui_wake_up_enable_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t thp_hostprocessing_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "hostprocessing\n");
}

static ssize_t thp_status_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "status=0x%x\n", thp_get_status_all());
}

/*
 * If not config ic_name in dts, it will be "unknown"
 */
static ssize_t thp_chip_info_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	return snprintf(buf, PAGE_SIZE, "%s-%s-%s\n", cd->ic_name,
					cd->project_id, cd->vendor_name);
}

static ssize_t thp_loglevel_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	u8 new_level = g_thp_log_cfg ? 0 : 1;

	int len = snprintf(buf, PAGE_SIZE, "%d -> %d\n",
				g_thp_log_cfg, new_level);

	g_thp_log_cfg = new_level;

	return len;
}

#if defined(THP_CHARGER_FB)
static ssize_t thp_host_charger_state_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct thp_core_data *ts = thp_get_core_data();

	THP_LOG_DEBUG("%s called\n", __func__);

	return snprintf(buf, 32, "%d\n", thp_get_status(THP_STATUS_CHARGER));
}
static ssize_t thp_host_charger_state_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct thp_core_data *ts = thp_get_core_data();

	/*
	 * get value of charger status from first byte of buf
	 */
	unsigned int value = buf[0] - '0';

	THP_LOG_INFO("%s: input value is %d\n", __func__, value);

	thp_set_status(THP_STATUS_CHARGER, value);

	return count;
}
#endif

static ssize_t thp_roi_data_show(struct kobject* kobj,
				struct kobj_attribute* attr, char* buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	short *host_roi_data_report = cd->host_roi_data_report;

	memcpy(buf, host_roi_data_report, ROI_DATA_LENGTH * sizeof(short));

	return ROI_DATA_LENGTH * sizeof(short);
}

static const char* move_to_next_number(const char* str_in)
{
	const char * str = str_in;
	const char *next_num;

	str = skip_spaces(str);
	next_num = strchr(str, ' ');
	if (next_num)
		return next_num;

	return str_in;
}

static ssize_t thp_roi_data_store(struct kobject* kobj,
		struct kobj_attribute* attr, const char* buf, size_t count)
{
	int i = 0;
	int num;
	struct thp_core_data *cd = thp_get_core_data();
	short *host_roi_data_report = cd->host_roi_data_report;
	const char *str = buf;

	while (i < ROI_DATA_LENGTH && *str) {
		if (sscanf(str, "%7d", &num) < 0)
			break;

		str = move_to_next_number(str);
		host_roi_data_report[i++] = (short)num;
	}

	return count;
}

static ssize_t thp_roi_data_debug_show(struct kobject* kobj,
				struct kobj_attribute* attr, char* buf)
{
	int count = 0;
	int i;
	struct thp_core_data *cd = thp_get_core_data();
	short *host_roi_data_report = cd->host_roi_data_report;

	for (i = 0; i < ROI_DATA_LENGTH; ++i) {
		count += snprintf(buf + count, PAGE_SIZE - count,
				"%4d ", host_roi_data_report[i]);
		/* every 7 data is a row */
		if (!((i + 1) % 7))
			count += snprintf(buf + count, PAGE_SIZE - count, "\n");
	}

	return count;
}

static ssize_t thp_roi_enable_store(struct kobject* kobj,
		struct kobj_attribute* attr, const char* buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	long status = 0;
	int ret;


	ret = strict_strtoul(buf, 10, &status);
	if (ret) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_ROI, !!status);
	THP_LOG_INFO("%s: set roi enable status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_roi_enable_show(struct kobject* kobj,
					struct kobj_attribute* attr, char* buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
			thp_get_status(THP_STATUS_ROI));
}

static ssize_t thp_holster_enable_store(struct kobject* kobj,
		struct kobj_attribute* attr, const char* buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	long status = 0;
	int ret;


	ret = strict_strtoul(buf, 10, &status);
	if (ret) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_HOLSTER, !!status);
	THP_LOG_INFO("%s: set holster status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_holster_enable_show(struct kobject* kobj,
					struct kobj_attribute* attr, char* buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
			thp_get_status(THP_STATUS_HOLSTER));
}

static ssize_t thp_glove_enable_store(struct kobject* kobj,
		struct kobj_attribute* attr, const char* buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	long status = 0;
	int ret;


	ret = strict_strtoul(buf, 10, &status);
	if (ret) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_GLOVE, !!status);
	THP_LOG_INFO("%s: set glove status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_glove_enable_show(struct kobject* kobj,
					struct kobj_attribute* attr, char* buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
			thp_get_status(THP_STATUS_GLOVE));
}


static ssize_t thp_holster_window_store(struct kobject* kobj,
		struct kobj_attribute* attr, const char* buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	int ret;

	int window_enable;
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;

	ret = sscanf(buf,"%4d %4d %4d %4d %4d", &window_enable, &x0,&y0,&x1, &y1);
	if (ret <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}
	thp_set_status(THP_STATUS_HOLSTER, !!window_enable);
	thp_set_status(THP_STAUTS_WINDOW_UPDATE, !thp_get_status(THP_STAUTS_WINDOW_UPDATE));
	THP_LOG_INFO("%s: update window %d %d %d %d %d\n",
			__func__, window_enable, x0, y0, x1, y1);

	cd->window.x0 = x0;
	cd->window.y0 = y0;
	cd->window.x1 = x1;
	cd->window.y1 = y1;

	return count;
}

static ssize_t thp_holster_window_show(struct kobject* kobj,
					struct kobj_attribute* attr, char* buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_window_info *window = &cd->window;

	return snprintf(buf, PAGE_SIZE - 1, "%d %d %d %d %d\n",
			thp_get_status(THP_STATUS_HOLSTER),
			window->x0, window->y0, window->x1, window->y1);
}



static DEVICE_ATTR(thp_status, S_IRUGO, thp_status_show, NULL);
static DEVICE_ATTR(touch_chip_info, S_IRUGO, thp_chip_info_show, NULL);
static DEVICE_ATTR(hostprocessing, S_IRUGO, thp_hostprocessing_show, NULL);
static DEVICE_ATTR(loglevel, S_IRUGO, thp_loglevel_show, NULL);
static DEVICE_ATTR(charger_state, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_host_charger_state_show, thp_host_charger_state_store);
static DEVICE_ATTR(roi_data, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			 thp_roi_data_show, thp_roi_data_store);
static DEVICE_ATTR(roi_data_internal, S_IRUGO, thp_roi_data_debug_show, NULL);
static DEVICE_ATTR(roi_enable, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_roi_enable_show, thp_roi_enable_store);
static DEVICE_ATTR(touch_sensitivity, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_holster_enable_show, thp_holster_enable_store);
static DEVICE_ATTR(touch_glove, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_glove_enable_show, thp_glove_enable_store);
static DEVICE_ATTR(touch_window, (S_IRUSR | S_IWUSR),
			thp_holster_window_show, thp_holster_window_store);

static struct attribute *thp_ts_attributes[] = {
	&dev_attr_thp_status.attr,
	&dev_attr_touch_chip_info.attr,
	&dev_attr_hostprocessing.attr,
	&dev_attr_loglevel.attr,
#if defined(THP_CHARGER_FB)
	&dev_attr_charger_state.attr,
#endif
	&dev_attr_roi_data.attr,
	&dev_attr_roi_data_internal.attr,
	&dev_attr_roi_enable.attr,
	&dev_attr_touch_sensitivity.attr,
	&dev_attr_touch_glove.attr,
	&dev_attr_touch_window.attr,
	NULL,
};

static const struct attribute_group thp_ts_attr_group = {
	.attrs = thp_ts_attributes,
};

static DEVICE_ATTR(tui_wake_up_enable, (S_IRUGO | S_IWUSR | S_IWGRP),
			thp_tui_wake_up_enable_show, thp_tui_wake_up_enable_store);


static struct attribute *thp_prop_attrs[] = {
	&dev_attr_tui_wake_up_enable.attr,
	NULL
};

static const struct attribute_group thp_ts_prop_attr_group = {
	.attrs = thp_prop_attrs,
};

int thp_init_sysfs(struct thp_core_data *cd)
{
	int rc;

	if (!cd) {
		THP_LOG_ERR("%s: core data null\n", __func__);
		return -EINVAL;
	}

	rc = sysfs_create_group(&cd->sdev->dev.kobj, &thp_ts_attr_group);
	if (rc) {
		THP_LOG_ERR("%s:can't create ts's sysfs\n", __func__);
		return rc;
	}

	rc = sysfs_create_link(NULL, &cd->sdev->dev.kobj, SYSFS_TOUCH_PATH);
	if (rc) {
		THP_LOG_ERR("%s: fail create link error = %d\n", __func__, rc);
		goto err_create_ts_sysfs;
	}

	/* add sys/afe_properties/ sysfs */
	cd->thp_obj = kobject_create_and_add(SYSFS_PROPERTY_PATH, NULL);
	if (!cd->thp_obj) {
		THP_LOG_ERR("%s:unable to create kobject\n", __func__);
		rc = -EINVAL;
		goto err_create_ts_sysfs;
	}
	rc = sysfs_create_group(cd->thp_obj, &thp_ts_prop_attr_group);
	if (rc) {
		THP_LOG_ERR("%s:failed to create attributes\n", __func__);
		goto err_create_ts_sysfs;
	}

	return 0;

err_create_ts_sysfs:
	sysfs_remove_group(&cd->sdev->dev.kobj, &thp_ts_attr_group);
	return rc;
}


void thp_sysfs_release(struct thp_core_data *cd)
{
	if (!cd) {
		THP_LOG_ERR("%s: core data null\n", __func__);
		return;
	}

	sysfs_remove_group(cd->thp_obj, &thp_ts_prop_attr_group);
	sysfs_remove_group(&cd->sdev->dev.kobj, &thp_ts_attr_group);
}


