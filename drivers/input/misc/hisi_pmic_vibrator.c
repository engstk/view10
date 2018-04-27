/*
 * Hisi pmic vibrator driver for Hisilicon Hi64xx pmic vibrator.
 *
 * Copyright (c) 2017 Hisilicon Technologies CO.Ltd.
 *		http://www.hisilicon.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/syscalls.h>
#include <linux/err.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/cdev.h>
#include <linux/switch.h>
#include <linux/wakelock.h>
#include "../../staging/android/timed_output.h"
#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include "securec.h"
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/of_device.h>
#include <linux/hisi/hisi_vibrator.h>

#define HISI_PMIC_VIBRATOR_DEFAULT_NAME		"vibrator"
#define HISI_PMIC_VIBRATOR_CDEVIE_NAME		"haptics"
#define HISI_PMIC_VIBRATOR_HAPTIC_CFG_STR	"haptics-cfg"

#define HISI_PMIC_VIBRATOR_ON			0xA000
#define HISI_PMIC_VIBRATOR_BRAKE		0xA001

#define HISI_PMIC_VIBRATOR_DUTY_NORMAL_CFG_L		0xA010
#define HISI_PMIC_VIBRATOR_DUTY_NORMAL_CFG_H		0xA011

#define HISI_PMIC_VIBRATOR_SPEL_TIME_REG	0xA050
#define HISI_PMIC_VIBRATOR_SPEL_TIME_REG_NUM	10
#define HISI_PMIC_VIBRATOR_SPEL_DUTY_REG	0xA05B
#define HISI_PMIC_VIBRATOR_SPEL_DUTY_REG_NUM	(HISI_PMIC_VIBRATOR_SPEL_TIME_REG_NUM * 2)
#define HISI_PMIC_VIBRATOR_REG_CFG_NUM 		HISI_PMIC_VIBRATOR_SPEL_TIME_REG_NUM

#define HISI_PMIC_VIBRATOR_MODE_STANDBY		0x00
#define HISI_PMIC_VIBRATOR_MODE_RTP		0x01
#define HISI_PMIC_VIBRATOR_MODE_HAPTICS		0x02
#define HISI_PMIC_VIBRATOR_BRK_EN		0x01


#define MAX_TIMEOUT		10000	/* 10s */

struct hisi_pmic_vibrator_haptics_cfg {
	u32 spel_time;
	u32 spel_duty[2];
};


struct hisi_pmic_vibrator_haptics_lib {
	struct hisi_pmic_vibrator_haptics_cfg cfg[HISI_PMIC_VIBRATOR_REG_CFG_NUM];
};

struct hisi_pmic_vibrator_dev {
	struct device* dev;
	struct class* class;
	struct cdev cdev;
	struct switch_dev sw_dev;
	struct timed_output_dev todev;
	struct hrtimer timer;
	struct mutex lock;
	struct wake_lock wakelock;
	dev_t version;
	struct hisi_pmic_vibrator_haptics_lib *haptics_lib;
	u32 haptics_counts;
	u32 vibrator_reg_on;
	u32 vibrator_reg_off;
	u32 bemf_l;
	u32 bemf_h;
	s32 max_timeout_ms;
	s32 irq[6];
	s8 name[32];
};

static struct hisi_pmic_vibrator_dev *g_vdev;

#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
static u8 reg_value;
#define MAX_INPUT_SIZE  63
#endif

/* read register  */
static u8 hisi_pmic_vibrator_read_u8(const u32 vibrator_address)
{
	return hisi_pmic_reg_read(vibrator_address);
}

/* write register  */
static void hisi_pmic_vibrator_write_u8(u8 vibrator_set, const u32 vibrator_address)
{
	hisi_pmic_reg_write(vibrator_address, vibrator_set);
}

static void hisi_pmic_vibrator_set_mode(u8 mode)
{
	hisi_pmic_vibrator_write_u8(mode, HISI_PMIC_VIBRATOR_ON);
}

static void hisi_pmic_vibrator_off(struct hisi_pmic_vibrator_dev *vdev, s32 value)
{
	hisi_pmic_vibrator_write_u8(vdev->vibrator_reg_off, HISI_PMIC_VIBRATOR_BRK_EN);
	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_STANDBY);

}

static enum hrtimer_restart hisi_pmic_vibrator_timer_func(struct hrtimer *timer)
{
		struct hisi_pmic_vibrator_dev *vdev =
			container_of(timer, struct hisi_pmic_vibrator_dev, timer);

	hisi_pmic_vibrator_off(vdev, 0);

	return HRTIMER_NORESTART;
}

static s32 hisi_pmic_vibrator_get_time(struct timed_output_dev *todev)
{

	struct hisi_pmic_vibrator_dev *vdev =
		container_of(todev, struct hisi_pmic_vibrator_dev, todev);

	if (hrtimer_active(&(vdev->timer))) {
		ktime_t r = hrtimer_get_remaining(&vdev->timer);
		return ktime_to_ms(r);
	}

	return 0;
}

static void hisi_pmic_vibrator_enable(struct timed_output_dev *todev, s32 value)
{
	struct hisi_pmic_vibrator_dev *vdev =
		container_of(todev, struct hisi_pmic_vibrator_dev, todev);
	s32 wake_time = 0;

	dev_info(vdev->dev, "enable[%d]\n", value);

	if (value < 0) {
		dev_err(vdev->dev, "invalid value[%d]\n", value);
		return;
	}

	mutex_lock(&vdev->lock);
	if (hrtimer_active(&vdev->timer))
		hrtimer_cancel(&vdev->timer);
	if (value > 0) {
		hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_STANDBY);
		if (value > vdev->max_timeout_ms) {
			value = vdev->max_timeout_ms;
		}
		hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_RTP);
		wake_time = value + 50;
		wake_lock_timeout(&vdev->wakelock,
				  msecs_to_jiffies(wake_time));
		hrtimer_start(&vdev->timer,
			      ns_to_ktime((u64) value * NSEC_PER_MSEC), /*lint !e571 */
			      HRTIMER_MODE_REL);
		dev_info(vdev->dev, "hisi_pmic_vibrator_RTP is running\n");
	} else {
		hisi_pmic_vibrator_off(vdev, 0);
	}

	mutex_unlock(&vdev->lock);
}

static s32 hisi_pmic_vibrator_haptic_cfg(struct hisi_pmic_vibrator_dev *vdev, u32 type)
{
	u32 time_reg, duty_reg0, duty_reg1;
	u32 i, idx;

	if (!vdev)
		return -EINVAL;

	if (type > vdev->haptics_counts || !type) {
		dev_err(vdev->dev, "type:%d is invaild\n", type);
		return -EINVAL;
	}

	idx = type - 1;
	hrtimer_cancel(&vdev->timer);
	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_STANDBY);

	for (i = 0; i < HISI_PMIC_VIBRATOR_REG_CFG_NUM; i++) {
		time_reg = HISI_PMIC_VIBRATOR_SPEL_TIME_REG + i;
		duty_reg0 = HISI_PMIC_VIBRATOR_SPEL_DUTY_REG + (i << 1);
		duty_reg1 = duty_reg0 + 1;

		/* configs time register and duty registers */
		hisi_pmic_vibrator_write_u8(vdev->haptics_lib[idx].cfg[i].spel_time,time_reg);
		hisi_pmic_vibrator_write_u8(vdev->haptics_lib[idx].cfg[i].spel_duty[0],duty_reg0);
		hisi_pmic_vibrator_write_u8(vdev->haptics_lib[idx].cfg[i].spel_duty[1],duty_reg1);
	}

	dev_info(vdev->dev, "hisi_pmic_vibrator_haptic_cfg complete\n");

	return 0;
}

static ssize_t hisi_pmic_vibrator_haptics_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	struct hisi_pmic_vibrator_dev *vdev = (struct hisi_pmic_vibrator_dev *)filp->private_data;
	uint64_t type = 0;
	s32 ret;

	mutex_lock(&vdev->lock);

	if ((len > 32) || (strict_strtoull(buff, 10, &type))) {
		dev_err(vdev->dev, "invaild parameters\n");
		mutex_unlock(&vdev->lock);
		return len;
	}
	ret = hisi_pmic_vibrator_haptic_cfg(vdev, (u32)type);
	if (ret) {
		mutex_unlock(&vdev->lock);
		return ret;
	}

	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_HAPTICS);

	mutex_unlock(&vdev->lock);

	return len;
}

#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG

static ssize_t hisi_pmic_vibrator_calib_value_show (struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	u32 val = 0;

	val = g_vdev->bemf_h << 8 | g_vdev->bemf_l;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "0x%x\n", val);
}

static ssize_t hisi_pmic_vibrator_get_reg_value_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	u32 val = reg_value;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "0x%x\n", val);
}

static ssize_t hisi_pmic_vibrator_get_reg_value_store(struct device *dev,
				     struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t value = 0;

	if ((count > MAX_INPUT_SIZE) || (strict_strtoull(buf, 16, &value))) {
		dev_err(g_vdev->dev,"vibrator_get_reg_store read value error\n");
		return count;
	}

	reg_value = hisi_pmic_vibrator_read_u8(value);
	dev_info(g_vdev->dev,"reg_value is 0x%x.\n", reg_value); /*lint !e559 */

	return count;
}

static ssize_t  hisi_pmic_vibrator_duty_modify_store(struct device *dev,
				     struct device_attribute *attr, const char *buf, size_t count)
{

	uint64_t value = 0;
	u32 duty_l;
	u32 duty_h;

	if ((count > MAX_INPUT_SIZE) || (strict_strtoull(buf, 16, &value))) {
		dev_err(g_vdev->dev,
			 "vibrator_get_reg_store read value error\n");
			return count;
	}

	duty_l = value & 0xff; // low 8 bit
	duty_h = value >> 8;   //high 8 bit

	hisi_pmic_vibrator_write_u8(duty_l, HISI_PMIC_VIBRATOR_DUTY_NORMAL_CFG_L);
	hisi_pmic_vibrator_write_u8(duty_h, HISI_PMIC_VIBRATOR_DUTY_NORMAL_CFG_H);

	dev_info(g_vdev->dev,"user buf L is 0x%x \n", hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_DUTY_NORMAL_CFG_L));
	dev_info(g_vdev->dev,"user buf H is 0x%x \n", hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_DUTY_NORMAL_CFG_H));

	return count;
}

static ssize_t  hisi_pmic_vibrator_change_mode_test_store(struct device *dev,
				     struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t val = 0;

	if((count > MAX_INPUT_SIZE) || (strict_strtoull(buf, 10, &val))) {
		dev_err(g_vdev->dev, "invaild val\n");
		return -EINVAL;
	}

	switch(val) {
	case HISI_PMIC_VIBRATOR_MODE_STANDBY:
		hisi_pmic_vibrator_set_mode(val);
		dev_info(g_vdev->dev, "hisi_pmic_vibrator mode is standby \n");
		break;
	case HISI_PMIC_VIBRATOR_MODE_RTP:
		hisi_pmic_vibrator_set_mode(val);
		dev_info(g_vdev->dev, "hisi_pmic_vibrator mode is rtp \n");
		break;
	case HISI_PMIC_VIBRATOR_MODE_HAPTICS:
		hisi_pmic_vibrator_set_mode(val);
		dev_info(g_vdev->dev, "hisi_pmic_vibrator mode is haptics \n");
		break;
	default:
		dev_err(g_vdev->dev,"input val is error!");
	}
	return count;
}

static  ssize_t  hisi_haptic_test_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{

	s32 ret = 0;
	uint64_t val = 0;

	if ((count > MAX_INPUT_SIZE) || (strict_strtoull(buf, 10, &val))) {
		dev_err(g_vdev->dev, "invaild parameters\n");
		return -EINVAL;
	}

	mutex_lock(&g_vdev->lock);

	ret = hisi_pmic_vibrator_haptic_cfg(g_vdev,val);
	if (ret) {
		dev_err(g_vdev->dev,"hisi_haptic_test error\n");
		mutex_unlock(&g_vdev->lock);
		return -EINVAL;
	}

	msleep(500);
	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_HAPTICS);
	dev_info(g_vdev->dev, "hisi_pmic_vibrator_haptic is running\n");

	mutex_unlock(&g_vdev->lock);
	return count;
}


static DEVICE_ATTR(vibrator_calib_value_show, S_IRUSR | S_IWUSR, hisi_pmic_vibrator_calib_value_show,
		   NULL);
static DEVICE_ATTR(vibrator_get_reg_value, S_IRUSR | S_IWUSR, hisi_pmic_vibrator_get_reg_value_show,
		   hisi_pmic_vibrator_get_reg_value_store);
static DEVICE_ATTR(hisi_vibrator_duty_change, S_IRUSR | S_IWUSR, NULL,
		   hisi_pmic_vibrator_duty_modify_store);
static DEVICE_ATTR(vibrator_change_mode_test, S_IRUSR | S_IWUSR, NULL,
		   hisi_pmic_vibrator_change_mode_test_store);
static DEVICE_ATTR(hisi_haptic_test, S_IRUSR|S_IWUSR, NULL,
		   hisi_haptic_test_store);

static struct attribute *hisi_vb_attributes[] = {
	&dev_attr_vibrator_calib_value_show.attr,
	&dev_attr_vibrator_get_reg_value.attr,
	&dev_attr_hisi_vibrator_duty_change.attr,
	&dev_attr_vibrator_change_mode_test.attr,
	&dev_attr_hisi_haptic_test.attr,
	NULL
};

static const struct attribute_group hisi_vb_attr_group = {
	.attrs = hisi_vb_attributes,
};

#endif

static s32 hisi_pmic_vibrator_haptics_open(struct inode * i_node, struct file * filp)
{
	filp->private_data = g_vdev;
	pr_err("%s:haptics open\n", __func__);
	return 0;
}

static struct file_operations hisi_pmic_vibrator_fops = {
	.open = hisi_pmic_vibrator_haptics_open,
	.write = hisi_pmic_vibrator_haptics_write,
};

static irqreturn_t hisi_pmic_vibrator_handler(int irq, void *data)
{
	struct hisi_pmic_vibrator_dev *vdev = (struct hisi_pmic_vibrator_dev *)data;

	if (irq == vdev->irq[0] || irq == vdev->irq[1] || irq == vdev->irq[2] || irq == vdev->irq[3] || irq == vdev->irq[4]) {
		dev_err(g_vdev->dev,"[%s] pmic vibrator interrupt happend !\n",__func__ );
		hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_STANDBY);
	} else {
		dev_err(g_vdev->dev,"[%s]invalid irq %d!\n", __func__, irq);
	}

	return IRQ_HANDLED;
}

static s32 hisi_pmic_vibrator_interrupt_init(struct spmi_device *pdev)
{
	struct hisi_pmic_vibrator_dev *vdev;
	s32 ret;

	vdev = pdev->dev.driver_data;
	if (!vdev)
		return -EINVAL;

	vdev->irq[0] = spmi_get_irq_byname(pdev, NULL, "ocp");
	if (vdev->irq[0] < 0) {
		dev_err(vdev->dev, "failed to get ocp irq id\n");
	}

	ret = devm_request_irq(&pdev->dev, vdev->irq[0], hisi_pmic_vibrator_handler,
			       0, "ocp", vdev);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to request ocp irq\n");
	}

	vdev->irq[1] = spmi_get_irq_byname(pdev, NULL, "out");
	if (vdev->irq[1] < 0) {
		dev_err(vdev->dev, "failed to get out irq id\n");
	}

	ret = devm_request_irq(&pdev->dev, vdev->irq[1], hisi_pmic_vibrator_handler,
			       0, "out", vdev);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to request out irq\n");
	}

	vdev->irq[2] = spmi_get_irq_byname(pdev, NULL, "undervol");
	if (vdev->irq[2] < 0) {
		dev_err(vdev->dev, "failed to get undervol irq id\n");
	}

	ret = devm_request_irq(&pdev->dev, vdev->irq[2], hisi_pmic_vibrator_handler,
			       0, "undervol", vdev);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to request undervol irq\n");
	}

	vdev->irq[3] = spmi_get_irq_byname(pdev, NULL, "overvol");
	if (vdev->irq[3] < 0) {
		dev_err(vdev->dev, "failed to get overvol irq id\n");
	}

	ret = devm_request_irq(&pdev->dev, vdev->irq[3], hisi_pmic_vibrator_handler,
			       0, "overvol", vdev);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to request overvol irq\n");
	}

	vdev->irq[4] = spmi_get_irq_byname(pdev, NULL, "adc");
	if (vdev->irq[4] < 0) {
		dev_err(vdev->dev, "failed to get adc irq id\n");
	}

	ret = devm_request_irq(&pdev->dev, vdev->irq[4], hisi_pmic_vibrator_handler,
			       0, "adc", vdev);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to request adc irq\n");
	}

	return 0;
}

static s32 hisi_pmic_vibrator_parse_dt(struct hisi_pmic_vibrator_dev *vdev)
{
	struct device *dev = vdev->dev;
	const __be32 *mux;
	/*lint -e429 */
	struct hisi_pmic_vibrator_haptics_lib *table;
	int ret, size, rows, temp = 0, params = 30;

	/* configure minimum idle timeout */
	ret = of_property_read_u32(dev->of_node, "hisi_pmic_vibrator,max-timeout-ms", &temp);
	if (ret < 0) {
		vdev->max_timeout_ms = MAX_TIMEOUT;
		dev_info(dev, "use default max timeout[%d]\n", MAX_TIMEOUT);
	} else {
		vdev->max_timeout_ms = temp;
	}

	ret = of_property_read_u32(dev->of_node, "vibrator-reg-on", &temp);
	if (ret < 0) {
		vdev->vibrator_reg_on = HISI_PMIC_VIBRATOR_ON;
		dev_info(dev, "use default ON register[0x%x]\n", HISI_PMIC_VIBRATOR_ON);
	} else  {
		vdev->vibrator_reg_on = temp;
	}

	ret = of_property_read_u32(dev->of_node, "vibrator-reg-off", &temp);
	if (ret < 0) {
		vdev->vibrator_reg_off = HISI_PMIC_VIBRATOR_BRAKE;
		dev_info(dev, "use default ON register[0x%x]\n", HISI_PMIC_VIBRATOR_BRAKE);
	} else  {
		vdev->vibrator_reg_off = temp;
	}

	ret = of_property_read_u32(dev->of_node, "vibrator-bemf-l", &temp);
	if (ret < 0) {
		dev_info(dev, "get bemf-l failed!\n");
	}else {
		vdev->bemf_l = temp;
	}

	ret = of_property_read_u32(dev->of_node, "vibrator-bemf-h", &temp);
	if (ret < 0) {
		dev_info(dev, "get bemf-h failed!\n");
	} else {
		vdev->bemf_h = temp;
	}

	mux = of_get_property(dev->of_node, "haptics-cfg", &size);
	if (!mux) {
		dev_info(dev, "could not support haptic lib\n");
		return 0;
	}

	if (size < (sizeof(*mux) * params)) { /*lint !e574 */
		dev_err(dev, "haptic lib data is bad\n");
		return -EINVAL;
	}

	size /= sizeof(*mux); /*lint !e573 */ /* Number of elements in array */
	rows = size / params;

	dev_info(dev, "number of elements is %d, rows is %d\n", size, rows);

	table = devm_kzalloc(dev,
		sizeof(struct hisi_pmic_vibrator_haptics_lib) * rows,
		GFP_KERNEL); /*lint !e429 */
	if (!table) {
		dev_err(dev,"failed to allocate haptics cfg table\n");
		return -ENOMEM;
	}

	ret = of_property_read_u32_array(dev->of_node, "haptics-cfg", (u32 *)table, size);
	if (ret) {
		dev_err(dev, "could not read 'haptics-cfg' table\n");
		return ret;
	}

	vdev->haptics_counts = rows;
	vdev->haptics_lib = table;
	/*lint +e429 */

	return 0;
}

static s32 hisi_pmic_vibrator_haptics_probe(struct hisi_pmic_vibrator_dev *vdev)
{
	s32 ret;

	vdev->version = MKDEV(0,0);
	ret = alloc_chrdev_region(&vdev->version, 0, 1, HISI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to alloc chrdev region, ret[%d]\n", ret);
		return ret;
	}

	vdev->class = class_create(THIS_MODULE, HISI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (!vdev->class) {
		dev_err(vdev->dev, "failed to create class\n");
		ret = ENOMEM;;
		goto unregister_cdev_region;
	}

	vdev->dev = device_create(vdev->class, NULL, vdev->version, NULL, HISI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (!vdev->dev) {
		ret = ENOMEM;;
		dev_err(vdev->dev, "failed to create device\n");
		goto destory_class;
	}

	cdev_init(&vdev->cdev, &hisi_pmic_vibrator_fops);
	vdev->cdev.owner = THIS_MODULE;
	vdev->cdev.ops = &hisi_pmic_vibrator_fops;
	ret = cdev_add(&vdev->cdev, vdev->version, 1);
	if (ret) {
		dev_err(vdev->dev, "failed to add cdev\n");
		goto destory_device;
	}

	vdev->sw_dev.name = "haptics";
	ret = switch_dev_register(&vdev->sw_dev);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to register sw_dev\n");
		goto unregister_cdev;
	}

	dev_info(vdev->dev, "haptics setup ok\n");

	return 0;

unregister_cdev:
	cdev_del(&vdev->cdev);
destory_device:
	device_destroy(vdev->class, vdev->version);
destory_class:
	class_destroy(vdev->class);
unregister_cdev_region:
	unregister_chrdev_region(vdev->version, 1);
	return ret;
}

static void hisi_pmic_vibrator_haptics_remove(struct hisi_pmic_vibrator_dev *vdev)
{
	cdev_del(&vdev->cdev);
	device_destroy(vdev->class, vdev->version);
	class_destroy(vdev->class);
	unregister_chrdev_region(vdev->version, 1);
	switch_dev_unregister(&vdev->sw_dev);
}


static int hisi_pmic_vibrator_probe(struct spmi_device *pdev)
{
	struct hisi_pmic_vibrator_dev *vdev;
	s32 ret;

	vdev = devm_kzalloc(&pdev->dev, sizeof(struct hisi_pmic_vibrator_dev), GFP_KERNEL);
	if (!vdev) {
		dev_err(&pdev->dev,"failed to allocate vibrator device\n");
		return -ENOMEM;
	}

	vdev->dev = &pdev->dev;
	g_vdev = vdev;
	dev_set_drvdata(&pdev->dev, vdev);

	/* parse DT */
	ret = hisi_pmic_vibrator_parse_dt(vdev);
	if (ret) {
		dev_err(&pdev->dev,"DT parsing failed\n");
		return ret;
	}

	wake_lock_init(&vdev->wakelock, WAKE_LOCK_SUSPEND, "his_pmic_vibrator");
	mutex_init(&vdev->lock);
	hrtimer_init(&vdev->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vdev->timer.function = hisi_pmic_vibrator_timer_func;
	vdev->todev.name = HISI_PMIC_VIBRATOR_DEFAULT_NAME;
	vdev->todev.get_time = hisi_pmic_vibrator_get_time;
	vdev->todev.enable = hisi_pmic_vibrator_enable;
	ret = timed_output_dev_register(&vdev->todev);
	if (ret) {
		dev_err(&pdev->dev,"unable to register with timed_output\n");
		goto unregister_timed_output_dev;
	}
#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
	ret = sysfs_create_group(&vdev->todev.dev->kobj, &hisi_vb_attr_group);
	if (ret) {
		dev_err(vdev->dev,"unable create vibrator's\n");
	}
#endif
	ret = hisi_pmic_vibrator_haptics_probe(vdev);
	if (ret) {
		dev_err(&pdev->dev,"failed to register timeout ouput dev\n");
		goto remove_hisi_pmic_vibrator_haptics;
	}

	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_STANDBY);

	/*init interrupts */
	ret = hisi_pmic_vibrator_interrupt_init(pdev);
	if (ret) {
		dev_err(&pdev->dev,"interrupts init failed\n");
	}

	dev_info(&pdev->dev,"hisi_pmic_vibrator probe succeed\n");

	return 0;

remove_hisi_pmic_vibrator_haptics:
	timed_output_dev_unregister(&vdev->todev);
unregister_timed_output_dev:
	hrtimer_cancel(&vdev->timer);
	mutex_destroy(&vdev->lock);
	wake_lock_destroy(&vdev->wakelock);
	return ret;
}

static s32 hisi_pmic_vibrator_remove(struct spmi_device *pdev)
{
	struct hisi_pmic_vibrator_dev *vdev;

	vdev = dev_get_drvdata(&pdev->dev);
	if (!vdev) {
		pr_err("%s:failed to get drvdata\n", __func__);
		return -ENODEV;
	}

	if (hrtimer_active(&vdev->timer))
		hrtimer_cancel(&vdev->timer);
#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
	sysfs_remove_group(&vdev->todev.dev->kobj, &hisi_vb_attr_group);
#endif
	hisi_pmic_vibrator_haptics_remove(vdev);
	timed_output_dev_unregister(&vdev->todev);
	mutex_destroy(&vdev->lock);
	wake_lock_destroy(&vdev->wakelock);
	dev_set_drvdata(&pdev->dev, NULL);

	return 0;
}

static void hisi_pmic_vibrator_shutdown(struct spmi_device *pdev)
{
	struct hisi_pmic_vibrator_dev *vdev;

	vdev = dev_get_drvdata(&pdev->dev);
	if (!vdev) {
		pr_err("%s:failed to get drvdata\n", __func__);
		return;
	}

	if (hrtimer_active(&vdev->timer))
		hrtimer_cancel(&vdev->timer);

	hisi_pmic_vibrator_off(vdev, 0);

	return ;
}

static const struct of_device_id hisi_pmic_vibrator_match[] = {
	{ .compatible = "hisilicon,pmic-vibrator",},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_pmic_vibrator_match);

static struct spmi_device_id pmic_vibrator_id[] = {
	{"hisilicon,pmic-vibrator", 0},
	{},
};

static struct spmi_driver hisi_pmic_vibrator_driver = {
	.probe  = hisi_pmic_vibrator_probe,
	.remove = hisi_pmic_vibrator_remove,
	.shutdown	= hisi_pmic_vibrator_shutdown,
	.id_table = pmic_vibrator_id,
	.driver = {
		.name   = "hisi-pmic-vibrator",
		.owner  = THIS_MODULE,
		.of_match_table =of_match_ptr(hisi_pmic_vibrator_match),
	},
};

static int __init hisi_pmic_vibrator_init(void)
{

	return spmi_driver_register(&hisi_pmic_vibrator_driver);

}

static void __exit hisi_pmic_vibrator_exit(void)
{
        spmi_driver_unregister(&hisi_pmic_vibrator_driver);
}

module_init(hisi_pmic_vibrator_init);
module_exit(hisi_pmic_vibrator_exit);

MODULE_AUTHOR("Wang Xiaoyin <hw.wangxiaoyin@hisilicon.com>");
MODULE_DESCRIPTION("HISI PMIC Vibrator driver");
MODULE_LICENSE("GPL");
