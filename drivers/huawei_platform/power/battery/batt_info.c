#include "batt_info.h"

#define HWLOG_TAG HW_BATT_INFO
HWLOG_REGIST();

/*
 *Driver Limitations:
 *  Support 32 and 64 bits process only (u32 == unsigned int)
 *  Support ds28el15 only: only this IC spportted now
 */


/* battery certification operations */
static batt_ct_ops bco = {
    .get_ic_dev_node  = NULL,
    .get_id           = NULL,
    .get_id_len       = NULL,
    .get_mac          = NULL,
    .get_mac_len      = NULL,
    .get_mac_resource = NULL,
};

/* battery check result */
static battery_check_result batt_chk_rlt = {
    .official       = OFFICAL_BATTERY,
    .binding        = BINDING_BATTERY,
    .lim_en         = BATT_LIM_ON,
    .check_ready    = CHECK_RUNNING,
    .limit_current  = MAX_CHARGE_CURRENT,
    .limit_voltage  = MAX_CHARGE_VOLTAGE,
};

/* battery constraints */
static battery_constraint batt_cons = {
    .id_mask        = NULL,
    .id_example     = NULL,
    .mac            = NULL,
    .bind_id        = NULL,
    .limit_sw       = NULL,
    .no_limit       = NULL,
    .id_len         = 0,
    .mac_len        = 0,
    .srv_on         = 0,
    .validity       = 0,
    .limit_current  = MAX_CHARGE_CURRENT,
    .limit_voltage  = MAX_CHARGE_VOLTAGE,
};

struct list_head batt_ct_head = LIST_HEAD_INIT(batt_ct_head);

/*-----------------------------------------------------------------------------------------------*/
/*-------------------------------------Generic netlink start-------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

/* attribute policy */
static struct nla_policy batt_genl_policy[__BATT_MESG_MAX] = {
    // [BATT_MAC_MESSAGE] = {},
    // [BATT_RAW_MESSAGE] = {},
};

/* family definition */
static struct genl_family batt_genl_family = {
    .id      = GENL_ID_GENERATE,
    .hdrsize = BATT_GNNL_HEAD_LEN,
    .name    = BATT_GNNL_FAMILY,
    .version = BATT_GNNL_VERSION,
    .maxattr = __BATT_MESG_MAX,
};

/* BATT_SERVICE_ON handler */
static int batt_srv_on_cb(struct sk_buff *skb_in, struct genl_info *info)
{
    void *msg_head;
    int i;
    int ret;
    struct sk_buff *skb;
    mac_resource mac_res;

    batt_cons.srv_on += 1;

    /* set random first */
    for(i = 0; i < MAX_RETRY_TIME; i++ ) {
        ret = bco.set_random();
        if(!ret) break;
    }
    if(ret){
        hwlog_info("Set random for certification failed in %s", __func__);
    }

    for(i = 0; i < MAX_RETRY_TIME; i++ ) {
        ret = bco.get_mac_resource(&mac_res, CT_MAC_TPYE0);
        if(!ret) break;
    }

    if(!ret) {
        /* send a message back*/
        /* allocate some memory, since the size is not yet known use NLMSG_GOODSIZE*/
        skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
        if(skb == NULL) {
            hwlog_err("sk_buff is NULL found in %s.\n", __func__);
            return 0;
        }
        /* create the message headers */
        msg_head = genlmsg_put(skb, BATT_GNNL_PORTID, info->snd_seq + 1, &batt_genl_family,
                               BATT_COMMON_FLAG, mac_res.ic_type | BATT_CERTIFICAION_MAC);
        if (msg_head == NULL) {
            hwlog_err("Create message for genlmsg failed in %s.\n", __func__);
            goto nosend;
        }
        /* add a TEST_ATTR_MSG attribute (actual value to be sent) */
        ret = nla_put(skb, BATT_RAW_MESSAGE, mac_res.len, mac_res.datum);
        if (ret != 0) {
            hwlog_err("Add attribute to genlmsg failed in %s. Error: %d.\n", __func__, ret);
            goto nosend;
        }

        /* finalize the message */
        genlmsg_end(skb, msg_head);

        /* send the message back */
        ret = genlmsg_unicast(&init_net, skb, info->snd_portid);
        if (ret != 0) {
            hwlog_err("Unicast genlmsg failed in %s. Error: %d.\n", __func__, ret);
            return 0;
        }
        hwlog_info("MAC source data is sent.");
    } else {
        hwlog_err("Synchronizing battery information failed in %s.\n", __func__);
    }
    return 0;

nosend:
    kfree_skb(skb);
    return 0;
}

/* BATT_CERTIFICAION_MAC handler */
static int batt_mac_mesg_cb(struct sk_buff *skb_in, struct genl_info *info)
{
    int ret;
    struct nlattr *nla;
    char * mydata;
    unsigned char *ic_mac;

    hwlog_info("Battery-driver:Certification running!\n");

    BATTERY_DRIVE_NULL_POINT(info);

    ret = bco.get_mac(&ic_mac);
    if(ret) {
        hwlog_err("Get mac failed(%d) in %s", ret, __func__);
        return 0;
    }

    /* certification MAC */
    nla = info->attrs[BATT_RAW_MESSAGE];
    if (nla) {
        mydata = (char *)nla_data(nla);
        if ( (mydata == NULL) || (nla->nla_len != batt_cons.mac_len + NLA_HDRLEN) ) {
            hwlog_err("Error receiving data, %s\n", __func__);
        } else {
            if(memcmp(mydata, ic_mac, batt_cons.mac_len)) {
                batt_chk_rlt.official = OFFICAL_BATTERY;
                hwlog_info("Battery-driver:Certification Failed!\n");
            } else {
                batt_chk_rlt.official = !OFFICAL_BATTERY;
                hwlog_info("Battery-driver:Certification Success!\n");
            }
        }
    } else {
        hwlog_err("no info->attrs %i\n, %s", BATT_RAW_MESSAGE, __func__);
    }
    return 0;
}

/* generic netlink operation definition */
static struct genl_ops batt_genl_ops_cb[BATT_GNNL_CB_NUM] = {
    {
        .cmd = BATT_CERTIFICAION_MAC,
        .flags = 0,
        .policy = batt_genl_policy,
        .doit = batt_mac_mesg_cb,
        .dumpit = NULL,
    },
    {
        .cmd = BATT_SERVICE_ON,
        .flags = 0,
        .policy = batt_genl_policy,
        .doit = batt_srv_on_cb,
        .dumpit = NULL,
    },
};
/*-----------------------------------------------------------------------------------------------*/
/*--------------------------------------Generic netlink end--------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------*/
/*-------------------------------------NVME operations start-------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
static int get_bind_id(void)
{
    int ret;
    struct hisi_nve_info_user bbinfo;
    BATTERY_DRIVE_NULL_POINT(batt_cons.bind_id);
    bbinfo.nv_operation = NV_READ;
    bbinfo.nv_number = BBINFO_NV_NUMBER;
    strncpy(bbinfo.nv_name, BBINFO_NV_NAME, NV_NAME_LENGTH);
    bbinfo.valid_size = NVE_NV_DATA_SIZE;
    ret = hisi_nve_direct_access(&bbinfo);
    if(ret) {
        hwlog_err("Battery-driver: read BBINFO NV failed in %s, error code %d.\n", __func__, ret);
        return BATTERY_DRIVER_FAIL;
    }
    memcpy(batt_cons.bind_id, bbinfo.nv_data + BBINFO_BATT_ID_OFFSET, batt_cons.id_len);
    return BATTERY_DRIVER_SUCCESS;
}

static int set_bind_id(void)
{
    int ret;
    struct hisi_nve_info_user bbinfo;
    unsigned char *id;

    ret = bco.get_id(&id);
    if(ret) {
        hwlog_err("Get ic id failed in %s.\n", __func__);
        return BATTERY_DRIVER_FAIL;
    }

    bbinfo.nv_operation = NV_WRITE;
    bbinfo.nv_number = BBINFO_NV_NUMBER;
    strncpy(bbinfo.nv_name, BBINFO_NV_NAME, NV_NAME_LENGTH);
    bbinfo.valid_size = NVE_NV_DATA_SIZE;
    memset(bbinfo.nv_data, 0, BBINFO_BATT_ID_OFFSET);
    memcpy(bbinfo.nv_data + BBINFO_BATT_ID_OFFSET, id, batt_cons.id_len);
    ret = hisi_nve_direct_access(&bbinfo);
    if(ret) {
        hwlog_err("Battery-driver: write BBINFO NV failed in %s, error code %d.\n", __func__, ret);
        return BATTERY_DRIVER_FAIL;
    }
    return BATTERY_DRIVER_SUCCESS;
}

static void check_batt_id(void)
{
    unsigned char *id = NULL;
    int ret;

    ret = bco.get_id(&id);
    if(ret) {
        hwlog_err("Get ic id failed in %s.\n", __func__);
        return;
    }

    if((!batt_cons.bind_id) || (!id)) {
        batt_chk_rlt.binding = UNBIND_BATTERY;
        return;
    }

    if(memcmp(batt_cons.bind_id, id, batt_cons.id_len)) {
        batt_chk_rlt.binding = UNBIND_BATTERY;
    } else {
        batt_chk_rlt.binding = BINDING_BATTERY;
    }
}

static int get_lim_sw(void)
{
    int ret;
    struct hisi_nve_info_user blimsw;
    BATTERY_DRIVE_NULL_POINT(batt_cons.limit_sw);
    blimsw.nv_operation = NV_READ;
    blimsw.nv_number = BLIMSW_NV_NUMBER;
    strncpy(blimsw.nv_name, BLIMSW_NV_NAME, NV_NAME_LENGTH);
    blimsw.valid_size = NVE_NV_DATA_SIZE;
    ret = hisi_nve_direct_access(&blimsw);
    if(ret) {
        hwlog_err("Battery-driver: read BLIMSW NV failed in %s, error code %d.\n", __func__, ret);
        return BATTERY_DRIVER_FAIL;
    }
    memcpy(batt_cons.limit_sw, blimsw.nv_data, LIMIT_SWITCH_SIZE);
    return BATTERY_DRIVER_SUCCESS;
}

static int set_lim_sw(int mode)
{
    int ret;
    struct hisi_nve_info_user blimsw;

    BATTERY_DRIVE_NULL_POINT(batt_cons.no_limit);

    blimsw.nv_operation = NV_WRITE;
    blimsw.nv_number = BLIMSW_NV_NUMBER;
    strncpy(blimsw.nv_name, BLIMSW_NV_NAME, NV_NAME_LENGTH);
    blimsw.valid_size = NVE_NV_DATA_SIZE;

    if(mode == BATT_LIM_ON) {
        memset(blimsw.nv_data, 0, LIMIT_SWITCH_SIZE);
    } else {
        memcpy(blimsw.nv_data, batt_cons.no_limit, LIMIT_SWITCH_SIZE);
    }

    ret = hisi_nve_direct_access(&blimsw);
    if(ret) {
        hwlog_err("Battery-driver: write BLIMSW NV failed in %s, error code %d.\n", __func__, ret);
        return BATTERY_DRIVER_FAIL;
    }
    return BATTERY_DRIVER_SUCCESS;
}

static void check_lim_sw(void)
{
    if((!batt_cons.no_limit) || (!batt_cons.limit_sw)) {
        batt_chk_rlt.lim_en = BATT_LIM_ON;
        return;
    }
    if(memcmp(batt_cons.no_limit, batt_cons.limit_sw, LIMIT_SWITCH_SIZE)) {
        batt_chk_rlt.lim_en = BATT_LIM_ON;
    } else {
        batt_chk_rlt.lim_en = BATT_LIM_OFF;
    }
}
/*-----------------------------------------------------------------------------------------------*/
/*--------------------------------------NVME operations end--------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------*/
/*----------------------------------------sys node start-----------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
static ssize_t batt_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i;
    int total_size = 0;
    unsigned char *id;
    int ret;

    ret = bco.get_id(&id);
    if(ret) {
        hwlog_err("Get ic id failed in %s.\n", __func__);
        return 0;
    }

    if(!id) {
        return snprintf(buf, PAGE_SIZE, "ERROR:NULL ID");
    }
    if(PAGE_SIZE <= (batt_cons.id_len * BATT_ID_PRINT_SIZE_PER_CHAR)) {
        return snprintf(buf, PAGE_SIZE, "ERROR:ID SIZE");
    }
    for(i = 0; i < batt_cons.id_len; i++) {
        total_size += snprintf(&(buf[total_size]), BATT_ID_PRINT_SIZE_PER_CHAR, "%02x", id[i]);
    }
    return total_size;
}

static ssize_t official_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    if(batt_chk_rlt.official == OFFICAL_BATTERY) {
        return snprintf(buf, PAGE_SIZE, "1");
    } else {
        return snprintf(buf, PAGE_SIZE, "0");
    }
}

static ssize_t bind_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i;
    int total_size = 0;

    if(!batt_cons.bind_id) {
        return snprintf(buf, PAGE_SIZE, "ERROR:NULL ID");
    }
    if(PAGE_SIZE < (batt_cons.id_len * BATT_ID_PRINT_SIZE_PER_CHAR)) {
        return snprintf(buf, PAGE_SIZE, "ERROR:ID SIZE");
    }
    for(i = 0; i < batt_cons.id_len; i++) {
        total_size += snprintf(&(buf[total_size]), BATT_ID_PRINT_SIZE_PER_CHAR, "%02x",
                               batt_cons.bind_id[i]);
    }
    return total_size;
}

static ssize_t binding_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    if(batt_chk_rlt.binding == BINDING_BATTERY
       && batt_chk_rlt.official == OFFICAL_BATTERY) {
        return snprintf(buf, PAGE_SIZE, "1");
    } else {
        return snprintf(buf, PAGE_SIZE, "0");
    }
}

static ssize_t lim_sw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    if(batt_chk_rlt.lim_en == BATT_LIM_OFF) {
        return snprintf(buf, PAGE_SIZE, "0");
    } else {
        return snprintf(buf, PAGE_SIZE, "1");
    }
}

static ssize_t service_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%02x", batt_cons.srv_on);
}


static DEVICE_ATTR_RO(batt_id);
static DEVICE_ATTR_RO(official);
static DEVICE_ATTR_RO(bind_id);
static DEVICE_ATTR_RO(binding);
static DEVICE_ATTR_RO(lim_sw);
static DEVICE_ATTR_RO(service);

static struct attribute *batt_ic_attrs[] = {
    &dev_attr_batt_id.attr,
    &dev_attr_official.attr,
    &dev_attr_bind_id.attr,
    &dev_attr_binding.attr,
    &dev_attr_lim_sw.attr,
    &dev_attr_service.attr,
    NULL, /* sysfs_create_files need last one be NULL */
};
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------sys node end------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------*/
/*----------------------------------------cdev node start----------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
static int batt_cdev_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int batt_cdev_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t batt_cdev_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    return 0;
}

long batt_cdev_unlocked_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;

    switch (cmd) {
    case BATT_BIND_CMD:
        ret = set_bind_id();
        if(ret) {
            hwlog_err("Battery-driver: set bind id fail in %s. Error: %d.\n", __func__, ret);
            return -EINVAL;
        }
        ret = get_bind_id();
        if(ret) {
            hwlog_err("Battery-driver: get bind id fail in %s. Error: %d.\n", __func__, ret);
            return -EINVAL;
        }
        check_batt_id();
        if(batt_chk_rlt.binding != BINDING_BATTERY) {
            hwlog_err("Battery-driver: BATT_BIND_CMD fail in %s.\n", __func__);
            return -EINVAL;
        }
        break;
    case BATT_LIM_ON_CMD:
        ret = set_lim_sw(BATT_LIM_ON);
        if(ret) {
            hwlog_err("Battery-driver: set lim sw fail in %s. Error: %d.\n", __func__, ret);
            return -EINVAL;
        }
        ret = get_lim_sw();
        if(ret) {
            hwlog_err("Battery-driver: get lim sw fail in %s. Error: %d.\n", __func__, ret);
            return -EINVAL;
        }
        check_lim_sw();
        if(batt_chk_rlt.lim_en != BATT_LIM_ON) {
            hwlog_err("Battery-driver: BATT_LIM_ON fail in %s.\n", __func__);
            return -EINVAL;
        }
        break;
    case BATT_LIM_OFF_CMD:
        ret = set_lim_sw(BATT_LIM_OFF);
        if(ret) {
            hwlog_err("Battery-driver: set lim sw fail in %s. Error: %d.\n", __func__, ret);
            return -EINVAL;
        }
        ret = get_lim_sw();
        if(ret) {
            hwlog_err("Battery-driver: get lim sw fail in %s. Error: %d.\n", __func__, ret);
            return -EINVAL;
        }
        check_lim_sw();
        if(batt_chk_rlt.lim_en != BATT_LIM_OFF) {
            hwlog_err("Battery-driver: BATT_LIM_ON fail in %s.\n", __func__);
            return -EINVAL;
        }
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static struct file_operations batt_cdev_fops = {
    .owner = THIS_MODULE,
    .open = batt_cdev_open,
    .release = batt_cdev_release,
    .unlocked_ioctl = batt_cdev_unlocked_ioctl,
    .read = batt_cdev_read,
};
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------cdev node end-----------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------*/
/*---------------------------------------work process start--------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
static struct delayed_work batt_nvme_work;

static void checkout_batt_nvme(struct work_struct *work)
{
    int ret;
    hwlog_info("Checking battery NVME...\n");
    if(!(BIND_ID_VALIDITY_BIT&batt_cons.validity)){
        ret = get_bind_id();
        if(ret) {
            hwlog_err("Battery-driver: get bind id fail in %s. Error: %d.\n", __func__, ret);
            schedule_delayed_work(&batt_nvme_work, msecs_to_jiffies(WAIT_FOR_NVME_MS));
            return ;
        }
        check_batt_id();
        batt_cons.validity |= BIND_ID_VALIDITY_BIT;
    }

    if(!(LIM_SW_VALIDITY_BIT&batt_cons.validity)){
        ret = get_lim_sw();
        if(ret) {
            hwlog_err("Battery-driver: get lim sw fail in %s. Error: %d.\n", __func__, ret);
            schedule_delayed_work(&batt_nvme_work, msecs_to_jiffies(WAIT_FOR_NVME_MS));
            return ;
        }
        check_lim_sw();
        batt_cons.validity |= LIM_SW_VALIDITY_BIT;
    }
    hwlog_info("Checking battery NVME finined.\n");
}

/*-----------------------------------------------------------------------------------------------*/
/*----------------------------------------work process end---------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------*/
/*----------------------------------------driver init start--------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
static dev_t devno;
static struct class *batt_class;
static struct device *batt_device;
static struct cdev *batt_cdev;

/* create all node needed by driver */
static int batt_node_create(struct platform_device *pdev)
{
    int ret;
    batt_cdev = cdev_alloc();
    if(!batt_cdev) {
        hwlog_err("Battery-driver: cdev alloc failed for batt_cdev.\n");
        return BATTERY_DRIVER_FAIL;
    }
    batt_cdev->ops = &batt_cdev_fops;
    batt_cdev->owner = THIS_MODULE;
    ret = alloc_chrdev_region(&devno, 0, 1, "BattDRV");
    if (ret < 0) {
        hwlog_err("Can't get devno for cdev.\n");
        return BATTERY_DRIVER_FAIL;
    }
    ret = cdev_add(batt_cdev, devno, 1);
    if(ret < 0) {
        hwlog_err("Can't add cdev in battery information driver.\n");
        return BATTERY_DRIVER_FAIL;
    }
    batt_class = class_create(THIS_MODULE, "battery");
    if(!batt_class) {
        hwlog_err("Can't create class: battery.\n");
        return BATTERY_DRIVER_FAIL;
    }
    batt_device = device_create(batt_class, NULL, devno, NULL, "batt_info");
    if(!batt_device) {
        hwlog_err("Can't create /dev/batt_info.\n");
        return BATTERY_DRIVER_FAIL;
    }

    ret = sysfs_create_files(&pdev->dev.kobj, batt_ic_attrs);
    if(ret) {
        hwlog_err("Can't create nodes under /sys/devices/platform/huawei_batt_info.\n");
        return BATTERY_DRIVER_FAIL;
    }

    return BATTERY_DRIVER_SUCCESS;
}

/* Battery constraints initialization */
static int batt_cons_init(struct platform_device *pdev,
                          struct device_node *batt_ic_np)
{
    int ret;

    /* get certification information length */
    batt_cons.id_len = bco.get_id_len();
    hwlog_info("ID length:0x%x",batt_cons.id_len);
    batt_cons.mac_len = bco.get_mac_len();
    hwlog_info("MAC length:0x%x",batt_cons.mac_len);

    /* Allocate memory for battery constraint infomation */
    batt_cons.id_mask = (unsigned char *)devm_kzalloc(&pdev->dev,
                                                      batt_cons.id_len,
                                                      GFP_KERNEL);
    BATTERY_DRIVE_NULL_POINT(batt_cons.id_mask);
    batt_cons.id_example = (unsigned char *)devm_kzalloc(&pdev->dev,
                                                         batt_cons.id_len,
                                                         GFP_KERNEL);
    BATTERY_DRIVE_NULL_POINT(batt_cons.id_example);
    batt_cons.mac = (unsigned char *)devm_kzalloc(&pdev->dev,
                                                  batt_cons.mac_len,
                                                  GFP_KERNEL);
    BATTERY_DRIVE_NULL_POINT(batt_cons.mac);
    batt_cons.bind_id = (unsigned char *)devm_kzalloc(&pdev->dev,
                                                      batt_cons.id_len,
                                                      GFP_KERNEL);
    BATTERY_DRIVE_NULL_POINT(batt_cons.bind_id);
    batt_cons.limit_sw = (unsigned char *)devm_kzalloc(&pdev->dev,
                                                       LIMIT_SWITCH_SIZE,
                                                       GFP_KERNEL);
    BATTERY_DRIVE_NULL_POINT(batt_cons.limit_sw);

    /* Get battery id mask & id example */
    if(batt_ic_np) {
        ret = of_property_read_u8_array(batt_ic_np, "id-mask",
                                        batt_cons.id_mask,
                                        batt_cons.id_len);
        BATTERY_DRIVE_DTS_READ_ERROR("id-mask");
        ret = of_property_read_u8_array(batt_ic_np, "id-example",
                                        batt_cons.id_example,
                                        batt_cons.id_len);
        BATTERY_DRIVE_DTS_READ_ERROR("id-example");
    } else {
        memset(batt_cons.id_mask,0,batt_cons.id_len);
        memset(batt_cons.id_example,0,batt_cons.id_len);
    }

    /* get battery no limitation flags */
    ret = of_property_read_string(pdev->dev.of_node, "no-limit", &(batt_cons.no_limit));
    BATTERY_DRIVE_DTS_READ_ERROR("no-limit");

    /* get battery voltage limitation */
    ret = of_property_read_u32(pdev->dev.of_node, "limit-voltage", &(batt_cons.limit_voltage));
    BATTERY_DRIVE_DTS_READ_ERROR("limit-voltage");

    /* get battery current limitation */
    ret = of_property_read_u32(pdev->dev.of_node, "limit-current", &(batt_cons.limit_current));
    BATTERY_DRIVE_DTS_READ_ERROR("limit-current");

    /* NVME not finish at now, so delay 3ms to get bind ID and limitation switch */
    INIT_DELAYED_WORK(&batt_nvme_work, checkout_batt_nvme);
    schedule_delayed_work(&batt_nvme_work, msecs_to_jiffies(WAIT_FOR_NVME_MS));

    return BATTERY_DRIVER_SUCCESS;
}

static int battery_driver_probe(struct platform_device *pdev)
{
    int ret;
    int i;
    ct_ops_reg_list *pos;
    struct device_node *batt_ic_np;

    hwlog_info("Battery information driver is going to probing...\n");

    /* find the working battery ic */
    for( i = 0; i < IC_DECT_RETRY_NUM; i++) {
        list_for_each_entry(pos, &batt_ct_head, node) {
            if(pos->ct_ops_register != NULL) {
                ret = pos->ct_ops_register(&bco);
            } else {
                ret = BATTERY_DRIVER_FAIL;
            }
            if(!ret) {
                hwlog_info("Find valid battery certification ic.");
                break;
            }
        }
        if(!ret) {
            list_del_init(&pos->node);
            break;
        }
    }

    /* check battery certification interface */
    BATTERY_DRIVE_NULL_POINT(bco.get_ic_dev_node);
    BATTERY_DRIVE_NULL_POINT(bco.get_id);
    BATTERY_DRIVE_NULL_POINT(bco.get_id_len);
    BATTERY_DRIVE_NULL_POINT(bco.get_mac);
    BATTERY_DRIVE_NULL_POINT(bco.get_mac_len);
    BATTERY_DRIVE_NULL_POINT(bco.get_mac_resource);
    BATTERY_DRIVE_NULL_POINT(bco.set_random);

    /* battery constraints initialization */
    batt_ic_np = bco.get_ic_dev_node();
    ret = batt_cons_init(pdev, batt_ic_np);
    BATTERY_DRIVE_FUNC_PROCESS("batt_cons_init");

    /* battery node initialization */
    ret = batt_node_create(pdev);
    BATTERY_DRIVE_FUNC_PROCESS("batt_node_create");

    /* register netlink  */
    ret = genl_register_family_with_ops(&batt_genl_family, batt_genl_ops_cb);
    if(ret) {
        hwlog_err("Battery-driver: register generic netlink failed. Error:%d.\n", ret);
    }
    BATTERY_DRIVE_FUNC_PROCESS("genl_register_family_with_ops");

    /* free memory of other battery ic */
    list_for_each_entry(pos, &batt_ct_head, node) {
        if(pos->ic_memory_release != NULL) {
            pos->ic_memory_release();
        }
    }

    hwlog_info("Battery information driver was probed.\n");
    return BATTERY_DRIVER_SUCCESS;
}

static int  battery_driver_remove(struct platform_device *pdev)
{
    return BATTERY_DRIVER_SUCCESS;
}

static struct of_device_id huawei_battery_match_table[] = {
    {
        .compatible = "huawei,battery-information",
    },
    { /*end*/},
};

static struct platform_driver huawei_battery_driver = {
    .probe		= battery_driver_probe,
    .remove		= battery_driver_remove,
    .driver		= {
        .name = "huawei_battery",
        .owner = THIS_MODULE,
        .of_match_table = huawei_battery_match_table,
    },
};

int __init battery_driver_init(void)
{
    hwlog_info("battery information driver init...\n");
    return platform_driver_register(&huawei_battery_driver);
}

void __exit battery_driver_exit(void)
{
    hwlog_info("battery information driver exit...\n");
    platform_driver_unregister(&huawei_battery_driver);
}

late_initcall(battery_driver_init);
module_exit(battery_driver_exit);
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------driver init end---------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

MODULE_LICENSE("GPL");
