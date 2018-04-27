#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/wired_channel_switch.h>

static struct wired_chsw_device_ops *g_chsw_ops;

#define HWLOG_TAG wired_channel_switch
HWLOG_REGIST();

int wired_chsw_ops_register(struct wired_chsw_device_ops *ops)
{
	if (ops != NULL) {
		g_chsw_ops = ops;
		return 0;
	} else {
		hwlog_err("wired_chgsw ops register fail!\n");
		return -EPERM;
	}
}
int wired_chsw_set_wired_channel(int flag) {
	if (!g_chsw_ops ||!g_chsw_ops->set_wired_channel) {
		hwlog_err("%s ops in null\n", __func__);
		return -1;
	}
	g_chsw_ops->set_wired_channel(flag);
	return 0;
}
static int wired_chsw_check_ops(void)
{
	int ret = 0;

	if ((NULL == g_chsw_ops) || (NULL == g_chsw_ops->set_wired_channel)) {
		hwlog_err("wired_chsw ops is NULL!\n");
		ret = -EINVAL;
	}

	return ret;
}
static int wired_chsw_probe(struct platform_device *pdev)
{
	int ret;
	ret = wired_chsw_check_ops();
	if (ret) {
		return -1;
	}
	hwlog_info("wired_channel_switch probe ok.\n");
	return 0;
}
static struct of_device_id wired_chsw_match_table[] = {
	{
	 .compatible = "huawei,wired_channel_switch",
	 .data = NULL,
	},
	{},
};

static struct platform_driver wired_chsw_driver = {
	.probe = wired_chsw_probe,
	.driver = {
		.name = "huawei,wired_channel_switch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wired_chsw_match_table),
	},
};
static int __init wired_chsw_init(void)
{
	hwlog_info("wired_chgsw init ok.\n");

	return platform_driver_register(&wired_chsw_driver);
}
static void __exit wired_chsw_exit(void)
{
	platform_driver_unregister(&wired_chsw_driver);
}

module_init(wired_chsw_init);
module_exit(wired_chsw_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wired charge switch module driver");
MODULE_AUTHOR("HUAWEI Inc");
