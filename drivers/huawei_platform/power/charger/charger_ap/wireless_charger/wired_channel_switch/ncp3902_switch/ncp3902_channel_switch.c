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

static int support_cutoff_wired_channel = 0;
static int gpio_chgsw_en = 0;

#define HWLOG_TAG ncp3902_channel_switch
HWLOG_REGIST();

static int ncp3902_chgsw_set_wired_channel(int flag)
{
	if (support_cutoff_wired_channel && gpio_chgsw_en) {
		if (WIRED_CHANNEL_CUTOFF == flag) {
			hwlog_info("%s set ncp3902 en high\n", __func__);
			gpio_set_value(gpio_chgsw_en, 1);  //cutoff
		} else {
			hwlog_info("%s set ncp3902 en low\n", __func__);
			gpio_set_value(gpio_chgsw_en, 0);  //restore
		}
	}
}
static struct wired_chsw_device_ops chsw_ops = {
	.set_wired_channel = ncp3902_chgsw_set_wired_channel,
};
static void ncp3902_chgsw_parse_dts(struct device_node *np)
{
	int ret = 0;
	ret = of_property_read_u32(np, "support_cutoff_wired_channel", &support_cutoff_wired_channel);
	if (ret) {
		hwlog_err("%s: get support_cutoff_wired_channel failed\n", __func__);
		support_cutoff_wired_channel = 0;
	}
	hwlog_info("%s:  support_cutoff_wired_channel  = %d.\n", __func__, support_cutoff_wired_channel);
}
static int ncp3902_chgsw_gpio_init(struct device_node *np)
{
	gpio_chgsw_en = of_get_named_gpio(np, "gpio_chgsw_en", 0);
	hwlog_info("ncp3902 gpio_chgsw_en %d\n", gpio_chgsw_en);
	if (!gpio_is_valid(gpio_chgsw_en)) {
		hwlog_err("gpio_chgsw_en is not valid\n");
		return -EINVAL;
	}
	if (gpio_request(gpio_chgsw_en, "gpio_chgsw_en")) {
		hwlog_err("could not request gpio_chgsw_en\n");
		return  -ENOMEM;
	}
	gpio_direction_output(gpio_chgsw_en, 0);  //0:enable 1:disable

	return 0;
}

static int ncp3902_chgsw_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = (&pdev->dev)->of_node;

	ncp3902_chgsw_parse_dts(np);
	ncp3902_chgsw_gpio_init(np);
	ret = wired_chsw_ops_register(&chsw_ops);
	if (ret) {
		hwlog_err("register wireless charge ops failed!\n");
		gpio_free(gpio_chgsw_en);
		return -1;
	}

	hwlog_info("ncp3902_chgsw probe ok.\n");
	return 0;
}
static struct of_device_id ncp3902_chgsw_match_table[] = {
	{
	 .compatible = "huawei,ncp3902_channel_switch",
	 .data = NULL,
	},
	{},
};

static struct platform_driver ncp3902_chgsw_driver = {
	.probe = ncp3902_chgsw_probe,
	.driver = {
		.name = "huawei,ncp3902_channel_switch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ncp3902_chgsw_match_table),
	},
};
static int __init ncp3902_chsw_init(void)
{
	hwlog_info("ncp3902_chgsw init ok.\n");

	return platform_driver_register(&ncp3902_chgsw_driver);
}
static void __exit ncp3902_chsw_exit(void)
{
	platform_driver_unregister(&ncp3902_chgsw_driver);
}

fs_initcall_sync(ncp3902_chsw_init);
module_exit(ncp3902_chsw_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ncp3902 switch module driver");
MODULE_AUTHOR("HUAWEI Inc");
