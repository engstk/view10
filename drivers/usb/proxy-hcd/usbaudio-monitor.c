#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/audio.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi/usb/hisi_hifi_usb.h>

#define ERR(format, arg...) pr_err("[usbaudio-minitor]%s: " format, __func__, ##arg)
#define DBG(format, arg...) pr_info("[usbaudio-minitor]%s: " format, __func__, ##arg)


static bool is_usbaudio_device(struct usb_device *udev, int configuration)
{
	struct usb_host_config *config = NULL;
	int hid_intf_num = 0, audio_intf_num = 0, other_intf_num = 0;
	int nintf;
	int i;

	for (i = 0; i < udev->descriptor.bNumConfigurations; i++) {
		if (udev->config[i].desc.bConfigurationValue == configuration) {
			config = &udev->config[i];
			break;
		}
	}

	if (!config) {
		WARN_ON(true);
		return false;
	}

	DBG("configuration %d %d\n", configuration, config->desc.bConfigurationValue);

	nintf = config->desc.bNumInterfaces;
	if ((nintf < 0) || (nintf > USB_MAXINTERFACES)) {
		ERR("nintf invalid %d\n", nintf);
		return false;
	}

	for (i = 0; i < nintf; ++i) {
		struct usb_interface_cache *intfc;
		struct usb_host_interface *alt;

		intfc = config->intf_cache[i];
		alt = &intfc->altsetting[0];

		if (alt->desc.bInterfaceClass == USB_CLASS_AUDIO) {
			if (alt->desc.bInterfaceSubClass == USB_SUBCLASS_AUDIOCONTROL)
				audio_intf_num++;
		} else if (alt->desc.bInterfaceClass == USB_CLASS_HID)
			hid_intf_num++;
		else
			other_intf_num++;
	}

	DBG("audio_intf_num %d, hid_intf_num %d, other_intf_num %d\n",
		audio_intf_num, hid_intf_num, other_intf_num);

	if ((audio_intf_num == 1) && (hid_intf_num <= 1) && (other_intf_num == 0)) {
		DBG("[%s]this is usb addio device\n", __func__);
		return true;
	}

	return false;
}

static bool is_non_usbaudio_device(struct usb_device *udev, int configuration)
{
	DBG("\n");

	if (udev->parent == NULL) {
		WARN_ON(1);
		return false;
	}
	if (udev->parent->parent != NULL) {
		WARN_ON(1);
		return false;
	}

	if (configuration <= 0) {
		WARN_ON(1);
		return false;
	}

	return !is_usbaudio_device(udev, configuration);
}

bool check_non_usbaudio_device(struct usb_device *udev, int configuration)
{
	if (is_non_usbaudio_device(udev, configuration)) {
		DBG("it need call hisi_usb_stop_hifi_usb\n");
		hisi_usb_stop_hifi_usb();
		return true;
	}
	return false;
}

static int usb_notifier_call(struct notifier_block *nb,
				unsigned long action, void *data)
{
	struct usb_device *udev = (struct usb_device *)data;
	int configuration;

	if (action == USB_DEVICE_ADD) {
		if ((udev->parent != NULL) && (udev->parent->parent == NULL)
				&& hisi_usb_using_hifi_usb(udev)
				&& udev->actconfig) {
			configuration = udev->actconfig->desc.bConfigurationValue;
			DBG("configuration %d\n", configuration);
			check_non_usbaudio_device(udev, configuration);
		}
	}

	return 0;
}

struct notifier_block usbaudio_monirot_nb = {
	.notifier_call = usb_notifier_call,
};

static int __init usbaudio_monitor_init(void)
{
	usb_register_notify(&usbaudio_monirot_nb);
	return 0;
}

static void __exit usbaudio_monitor_exit(void)
{
	usb_unregister_notify(&usbaudio_monirot_nb);
}

module_init(usbaudio_monitor_init);
module_exit(usbaudio_monitor_exit);
