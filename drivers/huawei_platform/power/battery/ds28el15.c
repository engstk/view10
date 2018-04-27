#include "ds28el15.h"

#define HWLOG_TAG HW_ONEWIRE_DS28EL15
HWLOG_REGIST();

/* */
static ds28el15_des ds28el15;

/* random 32 bytes for sram */
static unsigned char *random_bytes = NULL;

static unsigned char mac_datum[MAX_MAC_SOURCE_SIZE];

static ct_ops_reg_list ds28el15_ct_node;

static struct device_node * ds28el15_get_ic_dev_node(void)
{
    return ds28el15.pdev->dev.of_node;
}

static int ds28el15_get_id(unsigned char **id)
{
    int ret;
    if( !(ds28el15.ic_des.memory.validity & ROM_ID_VALIDITY_BIT) )
    {
        ret = ds28el15.ic_des.mem_ops.get_rom_id(&ds28el15.ic_des);
        DS28EL15_COMMUNICATION_INFO(ds28el15.ic_des.memory.rom_id);
    }
    if( ds28el15.ic_des.memory.validity & ROM_ID_VALIDITY_BIT)
    {
        *id = ds28el15.ic_des.memory.rom_id;
        return DS28EL15_SUCCESS;
    }
    return DS28EL15_FAIL;
}

static unsigned int ds28el15_get_id_len(void)
{
    return ds28el15.ic_des.memory.rom_id_length;
}

static int ds28el15_get_mac(unsigned char **mac)
{
    int ret;

    if( !(ds28el15.ic_des.memory.validity & MAC_VALIDITY_BIT) )
    {
        ret = ds28el15.ic_des.mem_ops.get_mac(&ds28el15.ic_des, !ANONYMOUS_MODE, MAXIM_PAGE0);
        DS28EL15_COMMUNICATION_INFO(ds28el15.ic_des.memory.mac);
    }
    if( ds28el15.ic_des.memory.validity & MAC_VALIDITY_BIT ){
        *mac = ds28el15.ic_des.memory.mac;
        return DS28EL15_SUCCESS;
    }
    return DS28EL15_FAIL;
}

static unsigned int ds28el15_get_mac_len(void)
{
    return ds28el15.ic_des.memory.mac_length;
}

static int ds28el15_get_mac_resource(mac_resource *res, unsigned int type)
{
    int ret;

    switch (type) {
    case DS28EL15_CT_MAC_PAGE0:
        if( !(ds28el15.ic_des.memory.validity & ROM_ID_VALIDITY_BIT) ) {
            ret = ds28el15.ic_des.mem_ops.get_rom_id(&ds28el15.ic_des);
            DS28EL15_COMMUNICATION_INFO(ds28el15.ic_des.memory.rom_id);
        }
        if( !(ds28el15.ic_des.memory.validity & PERSONALITY_VALIDITY_BIT) ) {
            ret = ds28el15.ic_des.mem_ops.get_personality(&ds28el15.ic_des);
            DS28EL15_COMMUNICATION_INFO(ds28el15.ic_des.memory.sram);
        }
        if( !(ds28el15.ic_des.memory.validity & SRAM_VALIDITY_BIT) ) {
            ret = ds28el15.ic_des.mem_ops.get_sram(&ds28el15.ic_des);
            DS28EL15_COMMUNICATION_INFO(ds28el15.ic_des.memory.sram);
        }
        if( !(ds28el15.ic_des.memory.validity & USER_MEMORY_VALIDITY_BIT) ) {
            ret = ds28el15.ic_des.mem_ops.get_user_memory(&ds28el15.ic_des, MAXIM_PAGE0);
            DS28EL15_COMMUNICATION_INFO(ds28el15.ic_des.memory.user_memory);
        }
        if( (ds28el15.ic_des.memory.validity & ROM_ID_VALIDITY_BIT) &&
            (ds28el15.ic_des.memory.validity & PERSONALITY_VALIDITY_BIT) &&
            (ds28el15.ic_des.memory.validity & SRAM_VALIDITY_BIT) &&
            (ds28el15.ic_des.memory.validity & USER_MEMORY_VALIDITY_BIT)) {
            memset(mac_datum, 0, MAX_MAC_SOURCE_SIZE);
            memcpy(mac_datum + AUTH_MAC_ROM_ID_OFFSET, ds28el15.ic_des.memory.rom_id,
                   ds28el15.ic_des.memory.rom_id_length);
            memcpy(mac_datum + AUTH_MAC_PAGE_OFFSET, ds28el15.ic_des.memory.user_memory,
                   ds28el15.ic_des.memory.page_size);
            memcpy(mac_datum + AUTH_MAC_SRAM_OFFSET, ds28el15.ic_des.memory.sram,
                   ds28el15.ic_des.memory.sram_length);
            mac_datum[AUTH_MAC_PAGE_NUM_OFFSET] = MAXIM_PAGE0;
            memcpy(mac_datum + AUTH_MAC_MAN_ID_OFFSET,
                   ds28el15.ic_des.memory.personality + MAXIM_MAN_ID_OFFSET, MAXIM_MAN_ID_SIZE);
            res->datum = mac_datum;
            res->len = DS28EL15_CT_MAC_SIZE;
            res->ic_type = DS28EL15_MAC_RES;
            return DS28EL15_SUCCESS;
        }
        break;
    default:
        break;
    }

    return DS28EL15_FAIL;
}

int ds28el15_set_random(void)
{
    int ret;

    if( !(ds28el15.ic_des.memory.validity & SRAM_VALIDITY_BIT) )
    {
        ret = ds28el15.ic_des.mem_ops.set_sram(&ds28el15.ic_des, random_bytes);
        DS28EL15_COMMUNICATION_INFO(ds28el15.ic_des.memory.sram);
    }
    if(ds28el15.ic_des.memory.validity & SRAM_VALIDITY_BIT)
    {
        return DS28EL15_SUCCESS;
    }

    return DS28EL15_FAIL;
}

#ifdef CONFIG_DS28EL15_COMMUNICATION_DEBUG

static unsigned int com_err;

static ssize_t get_rom_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;

    ret = ds28el15.ic_des.mem_ops.get_rom_id(&ds28el15.ic_des);
    if(ret) {
        com_err++;
    }

    return 0;
}

static ssize_t get_personality_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;

    ret = ds28el15.ic_des.mem_ops.get_personality(&ds28el15.ic_des);
    if(ret) {
        com_err++;
    }

    return 0;
}

static ssize_t get_user_memory_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;

    ret = ds28el15.ic_des.mem_ops.get_user_memory(&ds28el15.ic_des, MAXIM_PAGE0);
    if(ret) {
        com_err++;
    }

    return 0;
}

static ssize_t set_user_memory_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;

    ret = ds28el15.ic_des.mem_ops.set_user_memory(&ds28el15.ic_des, "\0\0\0\0",
                                                    MAXIM_SEGMENT0, MAXIM_PAGE0);
    if(ret) {
        com_err++;
    }

    return 0;
}

static ssize_t get_sram_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;

    ret = ds28el15.ic_des.mem_ops.get_sram(&ds28el15.ic_des);
    if(ret) {
        com_err++;
    }

    return 0;
}

static ssize_t set_sram_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;

    ret = ds28el15.ic_des.mem_ops.set_sram(&ds28el15.ic_des, random_bytes);
    if(ret) {
        com_err++;
    }

    return 0;
}

static ssize_t get_mac_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;

    ret = ds28el15.ic_des.mem_ops.get_mac(&ds28el15.ic_des,!ANONYMOUS_MODE,MAXIM_PAGE0);
    if(ret) {
        com_err++;
    }

    return 0;
}

static ssize_t com_err_store(struct device *dev, struct device_attribute *attr, const char *buf,
                               size_t count)
{
    com_err = 0;
    return count;
}

static ssize_t com_err_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d", com_err);
}

static DEVICE_ATTR_RO(get_rom_id);
static DEVICE_ATTR_RO(get_personality);
static DEVICE_ATTR_RO(get_user_memory);
static DEVICE_ATTR_RO(set_user_memory);
static DEVICE_ATTR_RO(get_sram);
static DEVICE_ATTR_RO(set_sram);
static DEVICE_ATTR_RO(get_mac);
static DEVICE_ATTR_RW(com_err);

static struct attribute *batt_ic_attrs[] = {
    &dev_attr_get_rom_id.attr,
    &dev_attr_get_personality.attr,
    &dev_attr_get_user_memory.attr,
    &dev_attr_set_user_memory.attr,
    &dev_attr_get_sram.attr,
    &dev_attr_set_sram.attr,
    &dev_attr_get_mac.attr,
    &dev_attr_com_err.attr,
    NULL, /* sysfs_create_files need last one be NULL */
};

#endif

int ds28el15_ct_ops_register(batt_ct_ops *bco)
{
    int ret;

    ret = ds28el15.ic_des.mem_ops.valid_mem_ops(&ds28el15.ic_des, ds28el15.pdev);

    if(!ret){
        get_random_bytes(random_bytes, ds28el15.ic_des.memory.sram_length);
        bco->get_ic_dev_node    = ds28el15_get_ic_dev_node;
        bco->get_id             = ds28el15_get_id;
        bco->get_id_len         = ds28el15_get_id_len;
        bco->get_mac            = ds28el15_get_mac;
        bco->get_mac_len        = ds28el15_get_mac_len;
        bco->get_mac_resource   = ds28el15_get_mac_resource;
        bco->set_random         = ds28el15_set_random;
    }

    return ret;
}

static int ds28el15_driver_probe(struct platform_device *pdev)
{
    int ret;

    hwlog_info("ds28el15: probing...\n");

    /* record ds28el15 platform_device */
    ds28el15.pdev = pdev;

    /* ds28el15--maxim onewire ic and use gpio as communication controller*/
    ret = maxim_onewire_register(&ds28el15.ic_des, ds28el15.pdev);
    if(ret){
        hwlog_err("maxim onewire ic register failed(%d) in %s.",ret,__func__);
        return DS28EL15_FAIL;
    }

    /* get random 32 bytes */
    random_bytes = (unsigned char *)devm_kmalloc(&pdev->dev,
                                                 ds28el15.ic_des.memory.sram_length,
                                                 GFP_KERNEL);
    if(!random_bytes)
    {
        hwlog_err("Kalloc for random bytes failed in %s.", __func__);
        return DS28EL15_FAIL;
    }

    /* add ds28el15_ct_ops_register to ct_ops_reg_list */
    ds28el15_ct_node.ic_memory_release = NULL;
    ds28el15_ct_node.ct_ops_register = ds28el15_ct_ops_register;
    list_add_tail(&ds28el15_ct_node.node, &batt_ct_head);

#ifdef CONFIG_DS28EL15_COMMUNICATION_DEBUG
    com_err = 0;
    sysfs_create_files(&pdev->dev.kobj, batt_ic_attrs);
#endif

    hwlog_info("ds28el15: probing success.\n");

    return DS28EL15_SUCCESS;
}

static int  ds28el15_driver_remove(struct platform_device *pdev)
{
    return DS28EL15_SUCCESS;
}

static struct of_device_id ds28el15_match_table[] = {
    {
        .name = "ds28el15",
        .compatible = "maxim,onewire-sha",
    },
    { /*end*/},
};

static struct platform_driver ds28el15_driver = {
    .probe		= ds28el15_driver_probe,
    .remove		= ds28el15_driver_remove,
    .driver		= {
        .name = "ds28el15",
        .owner = THIS_MODULE,
        .of_match_table = ds28el15_match_table,
    },
};

int __init ds28el15_driver_init(void)
{
    hwlog_info("ds28el15 driver init...\n");
    return platform_driver_register(&ds28el15_driver);
}

void __exit ds28el15_driver_exit(void)
{
    hwlog_info("ds28el15 driver exit...\n");
    platform_driver_unregister(&ds28el15_driver);
}

device_initcall_sync(ds28el15_driver_init);
module_exit(ds28el15_driver_exit);

MODULE_LICENSE("GPL");
