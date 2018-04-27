#include <linux/kthread.h>
#include <linux/sched/rt.h>
#include <linux/random.h>
#include <huawei_platform/log/hw_log.h>
#include <dsm/dsm_pub.h>
#include <linux/module.h>
#include <linux/printk.h>
#define HWLOG_TAG    DSM_KIRIN_CHIPSETS
HWLOG_REGIST();

#define DSM_LOG_INFO(x...)      _hwlog_info(HWLOG_TAG, ##x)
#define DSM_LOG_ERR(x...)       _hwlog_err(HWLOG_TAG, ##x)
#define DSM_LOG_DEBUG(x...)     _hwlog_debug(HWLOG_TAG, ##x)

struct dsm_dev dsm_kirin_chipsets = {
    .name = "dsm_kirin_chipsets",
    .device_name = NULL,
    .ic_name = NULL,
    .module_name = NULL,
    .fops = NULL,
    .buff_size = 1024,
};

struct dsm_client *kirin_chipsets_client = NULL;

static int __init dsm_kirin_chipsets_init(void)
{
    if (!kirin_chipsets_client) {
        kirin_chipsets_client = dsm_register_client(&dsm_kirin_chipsets);
    }
    if(!kirin_chipsets_client){
        DSM_LOG_ERR("kirin_chipsets_client reg failed\n");
    }
    return 0;
}
module_init(dsm_kirin_chipsets_init);
