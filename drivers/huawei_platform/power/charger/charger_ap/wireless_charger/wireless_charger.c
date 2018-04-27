#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless_charger.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <../charging_core.h>
#ifdef CONFIG_DIRECT_CHARGER
#include <huawei_platform/power/direct_charger.h>
#endif
#include <huawei_platform/power/wired_channel_switch.h>
#include <huawei_platform/power/wireless_otg.h>

#define HWLOG_TAG wireless_charger
HWLOG_REGIST();

static struct wireless_charge_device_ops *g_wireless_ops;
static struct wireless_charge_device_info *g_wireless_di;
static int g_wireless_channel_state = WIRELESS_CHANNEL_OFF;
static int g_wired_channel_state = WIRED_CHANNEL_OFF;
static enum wireless_charge_stage g_wireless_charge_stage = WIRELESS_STAGE_DEFAULT;
static int wireless_fast_charge_flag = 0;
static int wireless_start_sample_flag = 0;
static struct wake_lock g_rx_con_wakelock;
static int g_rx_gain_ratio = RX_DEFAULT_GAIN_RATIO;
static int rx_iout_samples[RX_IOUT_SAMPLE_LEN];
extern unsigned int runmode_is_factory(void);
static int g_fop_fixed_flag = 0;
static u8 last_chrg_state = 0;
static int g_rx_vrect_low_cnt = 0;
static int g_rx_vrect_err_cnt = 0;
static int g_rx_ocp_cnt = 0;
static int g_rx_ovp_cnt = 0;
static int g_rx_otp_cnt = 0;

BLOCKING_NOTIFIER_HEAD(rx_event_nh);

static char chrg_stage[WIRELESS_STAGE_TOTAL][WIRELESS_STAGE_STR_LEN] = {
	{"DEFAULT"}, {"CHECK_TX_ID"}, {"CHECK_TX_ABILITY"},
	{"CHECK_RX_ABILITY"}, {"CABLE_DETECT"}, {"CERTIFICATION"},
	{"CHECK_FWUPDATE"}, {"CHARGING"}, {"REGULATION"}
};
int wireless_charge_ops_register(struct wireless_charge_device_ops *ops)
{
	int ret = 0;

	if (ops != NULL) {
		g_wireless_ops = ops;
	} else {
		hwlog_err("charge ops register fail!\n");
		ret = -EPERM;
	}

	return ret;
}
int register_wireless_charger_vbus_notifier(struct notifier_block *nb)
{
	if(g_wireless_di && nb)
		return blocking_notifier_chain_register(&g_wireless_di->wireless_charge_evt_nh, nb);

	return -EINVAL;
}
static void wireless_charge_wake_lock(void)
{
	if (!wake_lock_active(&g_rx_con_wakelock)) {
		wake_lock(&g_rx_con_wakelock);
		hwlog_info("wireless_charge wake lock\n");
	}
}
static void wireless_charge_wake_unlock(void)
{
	if (wake_lock_active(&g_rx_con_wakelock)) {
		wake_unlock(&g_rx_con_wakelock);
		hwlog_info("wireless_charge wake unlock\n");
	}
}
static void wireless_charge_en_enable(int enable)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	di->ops->rx_enable(enable);
}
static void wireless_charge_sleep_en_enable(int enable)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	di->ops->rx_sleep_enable(enable);
}
int wireless_charge_get_wireless_channel_state(void)
{
	return g_wireless_channel_state;
}
void wireless_charge_set_wireless_channel_state(int state)
{
	g_wireless_channel_state = state;
}
static void wireless_charge_set_wired_channel_state(int state)
{
	hwlog_info("%s %d\n", __func__, state);
	g_wired_channel_state = state;
}
int wireless_charge_get_wired_channel_state(void)
{
	return g_wired_channel_state;
}
int wireless_charge_get_fast_charge_flag(void)
{
	return wireless_fast_charge_flag;
}
static void wireless_charge_send_normal_charge_uevent(void)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging, return\n", __func__);
		return;
	}

	wireless_fast_charge_flag = 0;
	hwlog_info("%s\n", __func__);
	wireless_charge_connect_send_uevent();
}
static void wireless_charge_send_fast_charge_uevent(void)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging, return\n", __func__);
		return;
	}

	wireless_fast_charge_flag = 1;
	hwlog_info("%s\n", __func__);
	wireless_charge_connect_send_uevent();
}
int wireless_charge_get_rx_iout_limit(void)
{
	int iin_set = RX_IOUT_MIN;
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return iin_set;
	}
	iin_set = min(di->rx_iout_max, di->rx_iout_limit);
	if (di->sysfs_data.rx_iout_max > 0)
		iin_set = min(iin_set, di->sysfs_data.rx_iout_max);
	return iin_set;
}
bool wireless_charge_check_tx_exist(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return false;
	}

	return di->ops->check_tx_exist();
}
static int wireless_charge_send_ept
	(struct wireless_charge_device_info *di, enum wireless_etp_type type)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging\n", __func__);
		return -1;
	}
	return di->ops->send_ept(type);
}
static void wireless_charge_set_input_current(struct wireless_charge_device_info *di)
{
	int iin_set = wireless_charge_get_rx_iout_limit();
	charge_set_input_current(iin_set);
}
static int wireless_charge_get_tx_id(struct wireless_charge_device_info *di)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging\n", __func__);
		return -1;
	}
	return di->ops->get_tx_id();
}
static int wireless_charge_fix_tx_fop(struct wireless_charge_device_info *di)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging\n", __func__);
		return -1;
	}
	if (di && di->ops && di->ops->fix_tx_fop) {
		return di->ops->fix_tx_fop(di->sysfs_data.tx_fixed_fop);
	}
}
static int wireless_charge_unfix_tx_fop(struct wireless_charge_device_info *di)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging\n", __func__);
		return -1;
	}
	if (di && di->ops && di->ops->unfix_tx_fop) {
		return di->ops->unfix_tx_fop();
	}
}
static int wireless_charge_kick_watchdog(struct wireless_charge_device_info *di)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging\n", __func__);
		return -1;
	}
	return di->ops->kick_watchdog();
}
static int wireless_charge_set_tx_vout(struct wireless_charge_device_info *di)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging\n", __func__);
		return -1;
	}
	hwlog_info("%s tx_vout is set to %dmV\n", __func__, di->tx_vout_max);
	return di->ops->set_tx_vout(di->tx_vout_max);
}
static int wireless_charge_set_rx_vout(struct wireless_charge_device_info *di)
{
	int rx_vout = di->tx_vout_max *di->rx_gain_ratio/PERCENT;
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging\n", __func__);
		return -1;
	}
	hwlog_info("%s rx_vout is set to %dmV\n", __func__, rx_vout);
	return di->ops->set_rx_vout(rx_vout);
}
static int wireless_charge_get_rx_vout(struct wireless_charge_device_info *di)
{
	return di->ops->get_rx_vout();
}
static int wireless_charge_get_rx_vrect(struct wireless_charge_device_info *di)
{
	return di->ops->get_rx_vrect();
}
static int wireless_charge_get_rx_iout(struct wireless_charge_device_info *di)
{
	return di->ops->get_rx_iout();
}
static int wireless_charge_get_rx_fop(struct wireless_charge_device_info *di)
{
	return di->ops->get_rx_fop();
}

static void wireless_charge_count_avg_iout(struct wireless_charge_device_info *di)
{
	int cnt_max = RX_AVG_IOUT_TIME/di->monitor_interval;

	if (di->iout_avg < RX_LOW_IOUT) {
		di->iout_low_cnt++;
		if (di->iout_low_cnt >= cnt_max)
			di->iout_low_cnt = cnt_max;
		return;
	} else {
		di->iout_low_cnt = 0;
	}

	if (di->iout_avg > RX_HIGH_IOUT) {
		di->iout_high_cnt++;
		if (di->iout_high_cnt >= cnt_max)
			di->iout_high_cnt = cnt_max;
		return;
	} else {
		di->iout_high_cnt = 0;
	}
}
static void wireless_charge_calc_avg_iout(struct wireless_charge_device_info *di, int iout)
{
	int i;
	static int index = 0;
	int iout_sum = 0;

	rx_iout_samples[index] = iout;
	index = (index+1) % RX_IOUT_SAMPLE_LEN;
	for (i = 0; i < RX_IOUT_SAMPLE_LEN; i++) {
		iout_sum += rx_iout_samples[i];
	}
	di->iout_avg = iout_sum/RX_IOUT_SAMPLE_LEN;
}
static void wireless_charge_reset_avg_iout(struct wireless_charge_device_info *di)
{
	int i;
	for (i = 0; i < RX_IOUT_SAMPLE_LEN; i++) {
		rx_iout_samples[i] = RX_IOUT_MIN;
	}
	di->iout_avg = RX_IOUT_MIN;
}
static void wireless_charge_set_charge_stage(enum wireless_charge_stage charge_stage)
{
	if (charge_stage < WIRELESS_STAGE_TOTAL) {
		g_wireless_charge_stage = charge_stage;
		hwlog_info("%s set charge stage to %s\n", __func__, chrg_stage[charge_stage]);
	}
}
static enum wireless_charge_stage wireless_charge_get_charge_stage(void)
{
	return g_wireless_charge_stage;
}
static int  wireless_charge_check_fast_charge_succ(struct wireless_charge_device_info *di)
{
	if (wireless_fast_charge_flag &&
		g_wireless_charge_stage >= WIRELESS_STAGE_CHARGING)
		return WIRELESS_CHRG_SUCC;
	else
		return WIRELESS_CHRG_FAIL;
}
static int  wireless_charge_check_normal_charge_succ(struct wireless_charge_device_info *di)
{
	if (WIRELESS_TYPE_ERR != di->tx_cap->type && !wireless_fast_charge_flag &&
		g_wireless_charge_stage >= WIRELESS_STAGE_CHARGING)
		return WIRELESS_CHRG_SUCC;
	else
		return WIRELESS_CHRG_FAIL;
}
static int  wireless_charge_check_fac_test_succ(struct wireless_charge_device_info *di)
{
	if (di->tx_cap->type == di->standard_tx_adaptor) {
		if (!di->rx_cap->fast_chrg_flag) {
			return  wireless_charge_check_normal_charge_succ(di);
		} else {
			return wireless_charge_check_fast_charge_succ(di);
		}
	}
	return WIRELESS_CHRG_FAIL;
}
static void wireless_charge_dsm_dump(struct wireless_charge_device_info *di, char* dsm_buff)
{
	int i, soc, vrect, vout, iout, tbatt;
	char buff[ERR_NO_STRING_SIZE] = {0};
	soc = hisi_battery_capacity();
	tbatt = hisi_battery_temperature();
	vrect = wireless_charge_get_rx_vrect(di);
	vout = wireless_charge_get_rx_vout(di);
	iout = wireless_charge_get_rx_iout(di);
	snprintf(buff, sizeof(buff),
		"soc = %d, vrect = %dmV, vout = %dmV, iout = %dmA, iout_avg = %dmA, tbatt = %d\n",
		soc, vrect, vout, iout, di->iout_avg, tbatt);
	strncat(dsm_buff, buff, strlen(buff));
	snprintf(buff, ERR_NO_STRING_SIZE, "iout(mA): ");
	strncat(dsm_buff, buff, strlen(buff));
	for (i = 0; i < RX_IOUT_SAMPLE_LEN; i++) {
		snprintf(buff, ERR_NO_STRING_SIZE, "%d ", rx_iout_samples[i]);
		strncat(dsm_buff, buff, strlen(buff));
	}
}
static void wireless_charge_dsm_report(struct wireless_charge_device_info *di, int err_no, char* dsm_buff)
{
	if (di->tx_cap->type == WIRELESS_QC) {
		hwlog_info("[%s] ignore err_no: %d, tx_type: %d \n", __func__, err_no, di->tx_cap->type);
		return;
	}
	msleep(di->monitor_interval);
	if (g_wireless_channel_state == WIRELESS_CHANNEL_ON) {
		wireless_charge_dsm_dump(di, dsm_buff);
		dsm_report(err_no, dsm_buff);
	}
}
static void wireless_charge_get_rx_capability(struct wireless_charge_device_info *di)
{
	int i;
	static struct wireless_core_data rx_cap;

	if (di->core_data->total_type <= 0) {
		hwlog_err("%s total_type is %d\n", __func__, di->core_data->total_type);
		return;
	}
	for (i = 0; i < di->core_data->total_type; i++) {
		if (di->tx_cap->type == di->core_data->rx_cap[i].charger_type) {
			memcpy(&rx_cap, &di->core_data->rx_cap[i], sizeof(rx_cap));
			break;
		}
	}
	if (i == di->core_data->total_type) {
		memcpy(&rx_cap, &di->core_data->rx_cap[i-1], sizeof(rx_cap));
	}
	di->rx_cap = &rx_cap;
	hwlog_info("wireless_charge_para(dts): charger_type: 0x%x cable_detect: %d certification: %d fast_chrg_flag: %d "
				"fast_vout_max: %d fast_iout_max: %d normal_vout_max: %d normal_iout_max: %d\n",
				di->rx_cap->charger_type, di->rx_cap->cable_detect,
				di->rx_cap->certification, di->rx_cap->fast_chrg_flag,
				di->rx_cap->fast_vout_max, di->rx_cap->fast_iout_max,
				di->rx_cap->normal_vout_max, di->rx_cap->normal_iout_max);

	if (di->tx_cap->vout_max) {
		di->rx_cap->normal_vout_max = min(di->rx_cap->normal_vout_max, di->tx_cap->vout_max);
	}
	if (di->tx_cap->iout_max) {
		di->rx_cap->normal_iout_max = min(di->rx_cap->normal_iout_max, di->tx_cap->iout_max);
	}

	if (di->tx_cap->boost) {
		di->rx_cap->fast_vout_max = min(di->rx_cap->fast_vout_max, TX_BOOST_VOUT);
	} else if (di->tx_cap->vout_max) {
		di->rx_cap->fast_vout_max = min(di->rx_cap->fast_vout_max, di->tx_cap->vout_max);
	} else {
		//do nothing
	}
	if (di->tx_cap->iout_max) {
		di->rx_cap->fast_iout_max = min(di->rx_cap->fast_iout_max, di->tx_cap->iout_max);
	}
	di->rx_cap->fast_iout_max =
		min(di->rx_cap->fast_iout_max, di->max_power/(di->rx_cap->fast_vout_max * di->rx_gain_ratio/PERCENT));

	hwlog_info("wireless_charge_para(final): charger_type: 0x%x cable_detect: %d certification: %d fast_chrg_flag: %d "
				"fast_vout_max: %d fast_iout_max: %d normal_vout_max: %d normal_iout_max: %d\n",
				di->rx_cap->charger_type, di->rx_cap->cable_detect,
				di->rx_cap->certification, di->rx_cap->fast_chrg_flag,
				di->rx_cap->fast_vout_max * di->rx_gain_ratio/PERCENT, di->rx_cap->fast_iout_max,
				di->rx_cap->normal_vout_max * di->rx_gain_ratio/PERCENT, di->rx_cap->normal_iout_max);
}
static void wireless_charge_get_tx_capability(struct wireless_charge_device_info *di)
{
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_err("%s not in wireless charging\n", __func__);
		return ;
	}
	di->tx_cap = di->ops->get_tx_capability();
}
static void wireless_charge_get_tx_info(struct wireless_charge_device_info *di)
{
	u8 *tx_fw_version;
	if(!runmode_is_factory()) {
		tx_fw_version = di->ops->get_tx_fw_version();
		hwlog_info("[%s]tx_fw_version = %s\n", __func__, tx_fw_version);
	}
}
static void wireless_charge_set_default_tx_capability(struct wireless_charge_device_info *di)
{
	di->tx_cap->type = WIRELESS_TYPE_ERR;
	di->tx_cap->vout_max = ADAPTER_5V*MVOLT_PER_VOLT;
	di->tx_cap->iout_max = CHARGE_CURRENT_1000_MA;
	di->tx_cap->boost = 0;
	di->tx_cap->cable = 1;
	di->tx_cap->fan = 0;
	di->tx_cap->tec = 0;
}

static int wireless_charge_tx_certification(struct wireless_charge_device_info *di)
{
	int ret;
	int i;
	int cert_flag = CERTI_SUCC;
	u8 random[WIRELESS_RANDOM_LEN] = {0x0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30}; //random
	u8 tx_cipherkey[WIRELESS_TX_KEY_LEN] = {0};
	u8 rx_cipherkey[WIRELESS_RX_KEY_LEN]= {0x56, 0x6d, 0x79, 0x7f, 0x2e, 0x5a, 0xf8, 0x92}; //rx_cipherkey

	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_info("%s not in wireless charging\n", __func__);
		return CERTI_FAIL;
	}

	ret = di->ops->get_tx_cert(random, WIRELESS_RANDOM_LEN, tx_cipherkey, WIRELESS_TX_KEY_LEN);
	if(ret) {
		cert_flag = CERTI_FAIL;
		hwlog_err("%s: get certification ,fail!\n", __func__);
		goto FuncEnd;
	}

	for(i = 0; i < WIRELESS_TX_KEY_LEN && i < WIRELESS_RX_KEY_LEN; i++) {
		if(rx_cipherkey[i] != tx_cipherkey[i]){
			hwlog_info("[%s] fail\n", __func__);
			cert_flag = CERTI_FAIL;
			goto FuncEnd;
		}
	}
FuncEnd:
	hwlog_info("[%s] cert_flag = %d \n", __func__, cert_flag);
	if(runmode_is_factory()){
		ret = di->ops->send_msg_cert_confirm(cert_flag);
		if (ret){
			hwlog_err("%s send confirm message\n", __func__);
			return CERTI_FAIL;
		}
	}
	return cert_flag;
}
static int wireless_charge_get_serialno(char *serial_no, unsigned int len)
{
	char *pstr,*dstr;
	if(!serial_no) {
		hwlog_info("%s: NULL pointer!\n", __func__);
		return -1;
	}
	pstr = strstr(saved_command_line,"androidboot.serialno=");
	if (!pstr) {
		hwlog_err("No androidboot.serialno info\n");
		return -1;
	}
	pstr += strlen("androidboot.serialno=");
	dstr = strstr(pstr, " ");

	if(dstr - pstr >= len)
		return -1;
	memcpy(serial_no, pstr, (dstr - pstr));
	serial_no[dstr - pstr] = '\0';
	hwlog_info("[%s] cmdline: androidboot.serialno=%s \n", __func__, serial_no);
	return 0;
}
static void wireless_charge_set_ctrl_interval(struct wireless_charge_device_info *di)
{
	if (g_wireless_charge_stage < WIRELESS_STAGE_REGULATION) {
		di->ctrl_interval = CONTROL_INTERVAL_NORMAL;
	} else {
		di->ctrl_interval = CONTROL_INTERVAL_FAST;
	}
}
static void wireless_charge_chip_init(struct wireless_charge_device_info *di, int tx_vset)
{
	int ret;
	ret = di->ops->chip_init(tx_vset);
	if(ret < 0) {
		hwlog_err("%s rx chip init failed\n", __func__);
	}
}
static bool wireless_charge_can_boost(struct wireless_charge_device_info *di, bool ignore_cnt_flag)
{
	int cnt_max = RX_AVG_IOUT_TIME/di->monitor_interval;
	int soc = hisi_battery_capacity();
	int i;

	if (!wireless_fast_charge_flag)
		return false;
	if (di->sysfs_data.rx_vout_reset)
		return false;
	if (IN_OTG_MODE == wireless_otg_get_mode())
		return false;

	/*check whether boost is allowed*/
	for(i = 0; i < di->segment_data.segment_para_level; i++) {
		if(soc >= di->segment_data.segment_para[i].soc_min && soc <= di->segment_data.segment_para[i].soc_max) {
			if(!di->segment_data.segment_para[i].boost_flag) {
				return false;
			}
			break;
		}
	}

	if (ignore_cnt_flag || di->iout_high_cnt >= cnt_max)
		return true;

	return false;
}
static bool wireless_charge_can_reset(struct wireless_charge_device_info *di)
{
	int cnt_max = RX_AVG_IOUT_TIME/di->monitor_interval;

	if (di->sysfs_data.rx_vout_reset)
		return true;
	if (IN_OTG_MODE == wireless_otg_get_mode())
		return true;
	if (di->iout_low_cnt >= cnt_max)
		return true;

	return false;
}
static int wireless_charge_boost_control(struct wireless_charge_device_info *di)
{
	int ret = 0;
	int tx_vout_max = di->tx_vout_max;
	static int dsm_report_flag = false;
	char dsm_buff[CHARGE_DMDLOG_SIZE] = {0};

	if (di->tx_vout_max != di->rx_cap->fast_vout_max) {
		di->tx_vout_max = di->rx_cap->fast_vout_max;
		ret = wireless_charge_set_tx_vout(di);
		if (ret) {
			hwlog_err("%s set tx vout fail\n", __func__);
			di->tx_vout_max = tx_vout_max;
			wireless_charge_set_tx_vout(di);
			wireless_charge_chip_init(di, di->tx_vout_max);
			goto FuncEnd;
		}
		wireless_charge_chip_init(di, di->tx_vout_max);
		ret = wireless_charge_set_rx_vout(di);
		if (ret) {
			hwlog_err("%s set rx vout fail\n", __func__);
			di->tx_vout_max = tx_vout_max;
			goto FuncEnd;
		}
	}
FuncEnd:
	if (ret && ++di->boost_err_cnt) {
		hwlog_err("%s: boost fail\n", __func__);
		if (di->boost_err_cnt >= BOOST_ERR_CNT_MAX) {
			di->boost_err_cnt = BOOST_ERR_CNT_MAX;
			if (!dsm_report_flag) {
				wireless_charge_dsm_report(di,ERROR_WIRELESS_BOOSTING_FAIL, dsm_buff);
				dsm_report_flag = true;
			}
		} else {
			dsm_report_flag = false;
		}
	} else {
		dsm_report_flag = false;
		di->boost_err_cnt = 0;
	}
	return ret;
}
static int wireless_charge_reset_control(struct wireless_charge_device_info *di)
{
	int ret = 0;
	int tx_vout_max = di->tx_vout_max;

	if (di->tx_vout_max != di->rx_cap->normal_vout_max) {
		di->tx_vout_max = di->rx_cap->normal_vout_max;
		ret = wireless_charge_set_tx_vout(di);
		if (ret) {
			hwlog_err("%s set tx vout fail\n", __func__);
			di->tx_vout_max = tx_vout_max;
			wireless_charge_set_tx_vout(di);
			wireless_charge_chip_init(di, di->tx_vout_max);
			return ret;
		}
		wireless_charge_chip_init(di, di->tx_vout_max);
		ret = wireless_charge_set_rx_vout(di);
		if (ret) {
			hwlog_err("%s set rx vout fail\n", __func__);
			di->tx_vout_max = tx_vout_max;
			return ret;
		}
	}
	return ret;
}
static void wireless_charge_vout_control(struct wireless_charge_device_info *di)
{
	if (!wireless_start_sample_flag && wireless_fast_charge_flag) {
		if (wireless_charge_can_boost(di, false)) {
			di->iout_low_cnt = 0;
			wireless_charge_boost_control(di);
		}
		if (wireless_charge_can_reset(di)) {
			di->iout_high_cnt = 0;
			wireless_charge_reset_control(di);
		}
	}
}
static void wireless_charge_iout_control(struct wireless_charge_device_info *di)
{
	int charger_iin_regval;
	int vrect;
	int cnt_max = RX_VRECT_LOW_RESTORE_TIME/di->ctrl_interval;
	int soc = hisi_battery_capacity();
	int i;

	if (di->max_power_time_out && time_after(jiffies, di->max_power_time_out)) {
		di->rx_cap->fast_iout_max = min(di->rx_cap->fast_iout_max,
			di->fast_charge_power/(di->rx_cap->fast_vout_max * di->rx_gain_ratio/PERCENT));
		di->max_power_time_out = 0;
	}
	if (di->tx_vout_max == di->rx_cap->fast_vout_max) {
		di->rx_iout_max = di->rx_cap->fast_iout_max;
	} else {
		di->rx_iout_max = di->rx_cap->normal_iout_max;
	}
	if (wireless_start_sample_flag) {
		di->rx_iout_limit = di->rx_iout_max;
		wireless_charge_set_input_current(di);
		return;
	}

	/*check charge segment para*/
	for(i = 0; i < di->segment_data.segment_para_level; i++) {
		if(soc >= di->segment_data.segment_para[i].soc_min && soc <= di->segment_data.segment_para[i].soc_max) {
			di->rx_iout_max = min(di->segment_data.segment_para[i].iout_max, di->rx_iout_max);
			break;
		}
	}
	charger_iin_regval = charge_get_charger_iinlim_regval();
	vrect = wireless_charge_get_rx_vrect(di);
	if (di->tx_vout_max - vrect > RX_VRECT_LOW_TH) {
		hwlog_err("%s: tx_vout_max(%dmV)-vrect(%dmV)> %dmV, decrease rx_iout %dmA \n",
			__func__, di->tx_vout_max, vrect, RX_VRECT_LOW_TH, RX_IOUT_REG_STEP);
		di->rx_iout_limit = max(charger_iin_regval - RX_IOUT_REG_STEP, RX_VRECT_LOW_IOUT_MIN);
		wireless_charge_set_input_current(di);
		g_rx_vrect_low_cnt = cnt_max;
		return;
	} else if (g_rx_vrect_low_cnt > 0){
		g_rx_vrect_low_cnt--;
	} else {
		/*do nothing*/
	}

	if (!g_rx_vrect_low_cnt && charger_iin_regval - di->iout_avg > RX_IOUT_REG_GAP2) {
		di->rx_iout_limit = max(charger_iin_regval - RX_IOUT_REG_STEP, RX_IOUT_MIN);
		wireless_charge_set_input_current(di);
	} else if (!g_rx_vrect_low_cnt && charger_iin_regval -di->iout_avg < RX_IOUT_REG_GAP1) {
		di->rx_iout_limit = min(charger_iin_regval + RX_IOUT_REG_STEP, di->rx_iout_max);
		wireless_charge_set_input_current(di);
	} else {
		/*do nothing*/
	}
}
static void wireless_charger_update_interference_settings
	(struct wireless_charge_device_info *di, u8 interfer_src_state)
{
	int i;
	int tx_fixed_fop = 0;
	int rx_vout_reset = 0;
	int rx_iout_max = 0;

	for (i = 0; i < di->interfer_data.total_src; i++) {
		if (di->interfer_data.interfer_para[i].src_open == interfer_src_state) {
			di->interfer_data.interfer_src_state |= BIT(i);
			break;
		} else if (di->interfer_data.interfer_para[i].src_close == interfer_src_state) {
			di->interfer_data.interfer_src_state &= ~ BIT(i);
			break;
		} else {
			/*do nothing*/
		}
	}
	if (i == di->interfer_data.total_src) {
		hwlog_err("%s: interference settings error\n", __func__);
		return;
	}

	for (i = 0; i < di->interfer_data.total_src; i++) {
		if (di->interfer_data.interfer_src_state & BIT(i)) {
			if (di->interfer_data.interfer_para[i].tx_fixed_fop) {
				tx_fixed_fop = di->interfer_data.interfer_para[i].tx_fixed_fop;
			}
			rx_vout_reset |= di->interfer_data.interfer_para[i].rx_vout_reset;
			if (!rx_iout_max) {
				rx_iout_max = di->interfer_data.interfer_para[i].rx_iout_max;
			} else {
				rx_iout_max = min(rx_iout_max, di->interfer_data.interfer_para[i].rx_iout_max);
			}
		}
	}
	di->sysfs_data.tx_fixed_fop = tx_fixed_fop;
	di->sysfs_data.rx_vout_reset = rx_vout_reset;
	di->sysfs_data.rx_iout_max = rx_iout_max;
	hwlog_info("[%s] sysfs_data: tx_fixed_fop = %d, rx_vout_reset = %d, rx_iout_max = %dmA",
		__func__, di->sysfs_data.tx_fixed_fop, di->sysfs_data.rx_vout_reset, di->sysfs_data.rx_iout_max);
}
static void wireless_charge_update_fop(struct wireless_charge_device_info *di)
{
	int ret;
	if (di->sysfs_data.tx_fixed_fop > 0 && !g_fop_fixed_flag) {
		ret = wireless_charge_fix_tx_fop(di);
		if (ret) {
			hwlog_err("%s: fix tx_fop fail\n", __func__);
			return;
		}
		hwlog_info("[%s] fop fixed to %dkHZ\n", __func__, di->sysfs_data.tx_fixed_fop);
		g_fop_fixed_flag = 1;
	}
	if (di->sysfs_data.tx_fixed_fop == 0 && g_fop_fixed_flag) {
		ret = wireless_charge_unfix_tx_fop(di);
		if (ret) {
			hwlog_err("%s: unfix tx_fop fail", __func__);
			return;
		}
		hwlog_info("[%s] fop unfixed  succ \n", __func__);
		g_fop_fixed_flag = 0;
	}
}
static void wireless_charge_send_charge_state(struct wireless_charge_device_info *di)
{
	int ret;
	u8 cur_chrg_state = 0;
	int soc = hisi_battery_capacity();

	if (soc >= CAPACITY_FULL) {
		cur_chrg_state |= WIRELESS_CHRG_STATE_FULL;
	} else {
		cur_chrg_state &= ~WIRELESS_CHRG_STATE_FULL;
	}

	if (cur_chrg_state != last_chrg_state) {
		ret = di->ops->send_chrg_state(cur_chrg_state);
		if (ret) {
			hwlog_err("%s: send charge_state fail\n", __func__);
			return;
		}
		last_chrg_state = cur_chrg_state;
	}
}
static void wireless_charge_check_voltage(struct wireless_charge_device_info *di)
{
	int cnt_max = RX_VRECT_ERR_CHECK_TIME/di->ctrl_interval;
	int vrect = wireless_charge_get_rx_vrect(di);
	if (vrect > 0 && vrect < di->rx_vrect_min) {
		g_rx_vrect_err_cnt++;
		if (g_rx_vrect_err_cnt >= cnt_max){
			g_rx_vrect_err_cnt = cnt_max;
			hwlog_info("%s vrect lower than %dmV for %dms, send ept_ocp\n",
				__func__, di->rx_vrect_min, RX_VRECT_ERR_CHECK_TIME);
			wireless_charge_send_ept(di, WIRELESS_EPT_ERR_VRECT);
		}
	} else {
		g_rx_vrect_err_cnt = 0;
	}
}
static void wireless_charge_update_status(struct wireless_charge_device_info *di)
{
	if ((g_wireless_charge_stage < WIRELESS_STAGE_REGULATION) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	wireless_charge_update_fop(di);
	wireless_charge_send_charge_state(di);
}
static void wireless_charge_regulation(struct wireless_charge_device_info *di)
{
	if ((WIRELESS_STAGE_REGULATION != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	wireless_charge_vout_control(di);
	wireless_charge_iout_control(di);
}
static void wireless_charge_start_charging(struct wireless_charge_device_info *di)
{
	int ret;
	if ((WIRELESS_STAGE_CHARGING != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	di->tx_vout_max = di->rx_cap->normal_vout_max;
	wireless_charge_set_tx_vout(di);
	wireless_charge_set_rx_vout(di);
	if (wireless_fast_charge_flag)
		di->max_power_time_out = jiffies +msecs_to_jiffies(di->max_power_time * MSEC_PER_SEC);

	wireless_charge_get_tx_info(di);

	if (wireless_charge_can_boost(di, true)) {
		ret = wireless_charge_boost_control(di);
		if (!ret) {
			if(runmode_is_factory()){
				ret = di->ops->send_msg_rx_boost_succ();
				if (!ret){
					hwlog_info("[%s] send cmd rx_boost_succ ok!\n", __func__);
				}
			}
		}
	}
	wireless_charge_set_charge_stage(WIRELESS_STAGE_REGULATION);
}
static void wireless_charge_check_fwupdate(struct wireless_charge_device_info *di)
{
	if ((WIRELESS_STAGE_CHECK_FWUPDATE != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state)){
		return;
	}
	di->ops->check_fwupdate();
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CHARGING);
}
static void wireless_charge_check_certification(struct wireless_charge_device_info *di)
{
	int ret;
	if ((WIRELESS_STAGE_CERTIFICATION != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	hwlog_info("%s ++\n", __func__);
	if (di->certi_err_cnt >= CERTI_ERR_CNT_MAX) {
		hwlog_err("%s error exceed %d times, fast charge is disabled\n", __func__, CERTI_ERR_CNT_MAX);
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_FWUPDATE);
		wireless_charge_send_normal_charge_uevent();
		return;
	}
	wireless_charge_set_input_current(di);
	if (di->rx_cap->certification) {
		ret = wireless_charge_tx_certification(di);
		if (ret < 0) {
			hwlog_err("%s certification communication failed\n", __func__);
			di->certi_err_cnt ++;
			return;
		}
		if (CERTI_SUCC != ret) {
			hwlog_err("%s: certification fail\n", __func__);
			di->certi_err_cnt++;
			return;
		}
	}
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_FWUPDATE);
	hwlog_info("%s --\n", __func__);
}
static void wireless_charge_cable_detect(struct wireless_charge_device_info *di)
{
	if ((WIRELESS_STAGE_CABLE_DETECT != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	hwlog_info("%s ++\n", __func__);
	if (di->rx_cap->cable_detect && !di->tx_cap->cable) {
		hwlog_err("%s cable detect fail\n", __func__);
		wireless_charge_send_normal_charge_uevent();
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CHARGING);
		return;
	}
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CERTIFICATION);
	hwlog_info("%s --\n", __func__);
}
static void wireless_charge_check_rx_ability(struct wireless_charge_device_info *di)
{
	if ((WIRELESS_STAGE_CHECK_RX_ABILITY != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	hwlog_info("%s ++\n", __func__);
	wireless_charge_get_rx_capability(di);
	if (di->rx_cap->fast_chrg_flag) {
		wireless_charge_send_fast_charge_uevent();
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CABLE_DETECT);
	} else {
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_FWUPDATE);
	}
	hwlog_info("%s --\n", __func__);
}
static void wireless_charge_check_tx_ability(struct wireless_charge_device_info *di)
{
	char dsm_buff[CHARGE_DMDLOG_SIZE] = {0};
	if ((WIRELESS_STAGE_CHECK_TX_ABILITY != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	hwlog_info("%s ++\n", __func__);
	if (di->tx_ability_err_cnt >= TX_ABILITY_ERR_CNT_MAX) {
		hwlog_err("%s error exceed %d times, fast charge is disabled\n", __func__, TX_ABILITY_ERR_CNT_MAX);
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_RX_ABILITY);
		if (di->standard_tx) {
			wireless_charge_dsm_report(di,ERROR_WIRELESS_CHECK_TX_ABILITY_FAIL, dsm_buff);
		}
		return;
	}
	wireless_charge_set_input_current(di);
	wireless_charge_get_tx_capability(di);
	if (WIRELESS_TYPE_ERR == di->tx_cap->type) {
		hwlog_err("%s get tx ability failed\n", __func__);
		di->tx_ability_err_cnt ++;
		return;
	}
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_RX_ABILITY);
	hwlog_info("%s --\n", __func__);
}
static void wireless_charge_check_tx_id(struct wireless_charge_device_info *di)
{
	int tx_id;
	if ((WIRELESS_STAGE_CHECK_TX_ID != g_wireless_charge_stage) ||
		(WIRELESS_CHANNEL_OFF == g_wireless_channel_state))
		return;

	hwlog_info("[%s] ++\n", __func__);
	if (di->tx_id_err_cnt >= TX_ID_ERR_CNT_MAX) {
		hwlog_err("%s: error exceed %d times, fast charge is disabled\n", __func__, TX_ID_ERR_CNT_MAX);
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_RX_ABILITY);
		return;
	}
	wireless_charge_set_input_current(di);
	tx_id = wireless_charge_get_tx_id(di);
	if (tx_id < 0) {
		hwlog_err("%s: get id failed\n", __func__);
		di->tx_id_err_cnt++;
		return;
	}
	if (TX_ID_HW != tx_id) {
		hwlog_err("%s: id(0x%x) is not correct(0x%x)\n", __func__, tx_id, TX_ID_HW);
		wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_RX_ABILITY);
		return;
	}
	di->standard_tx = 1;
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_TX_ABILITY);
	hwlog_info("[%s] --\n", __func__);
	return;
}
static void wireless_charge_rx_stop_charing_config(struct wireless_charge_device_info *di)
{
	int ret;
	ret = di->ops->stop_charging();
	if (ret < 0) {
		hwlog_err("%s rx stop charing config failed\n", __func__);
	}
}
static void wireless_charge_para_init(struct wireless_charge_device_info *di)
{
	di->ctrl_interval = CONTROL_INTERVAL_NORMAL;
	di->monitor_interval = MONITOR_INTERVAL;
	di->tx_vout_max = TX_DEFAULT_VOUT;
	di->rx_iout_max = RX_IOUT_MIN;
	di->rx_iout_limit = RX_IOUT_MIN;
	di->standard_tx = 0;
	di->tx_id_err_cnt = 0;
	di->tx_ability_err_cnt = 0;
	di->certi_err_cnt = 0;
	di->boost_err_cnt = 0;
	di->sysfs_data.en_enable = 0;
	di->max_power_time_out = 0;
	di->iout_high_cnt = 0;
	di->iout_low_cnt = 0;
	di->rx_gain_ratio = g_rx_gain_ratio;
	last_chrg_state = 0;
	g_rx_vrect_low_cnt = 0;
	g_rx_vrect_err_cnt = 0;
	g_rx_ocp_cnt = 0;
	g_rx_ovp_cnt = 0;
	g_rx_otp_cnt = 0;
	wireless_charge_reset_avg_iout(di);
	wireless_charge_set_default_tx_capability(di);
	charge_set_input_current_prop(CHARGE_CURRENT_STEP, CHARGE_CURRENT_DELAY);
	wireless_charge_set_input_current(di);
	wireless_charge_set_tx_vout(di);
	wireless_charge_set_rx_vout(di);
}
static void wireless_charge_control_work(struct work_struct *work)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}

	wireless_charge_check_tx_id(di);
	wireless_charge_check_tx_ability(di);
	wireless_charge_check_rx_ability(di);
	wireless_charge_cable_detect(di);
	wireless_charge_check_certification(di);
	wireless_charge_check_fwupdate(di);
	wireless_charge_start_charging(di);
	wireless_charge_regulation(di);
	wireless_charge_update_status(di);
	wireless_charge_set_ctrl_interval(di);

	if (WIRELESS_CHANNEL_ON == g_wireless_channel_state) {
		schedule_delayed_work(&di->wireless_ctrl_work, msecs_to_jiffies(di->ctrl_interval));
	}
}
static void wireless_charge_rx_program_otp_work(struct work_struct *work)
{
	int ret;
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s wireless charge di null\n", __func__);
		return;
	}
	hwlog_info("%s ++\n", __func__);

	ret = di->ops->rx_program_otp();
	if (ret) {
		hwlog_err("%s wireless rx program otp fail\n", __func__);
	}
	hwlog_info("%s --\n", __func__);
}

static void wireless_charge_stop_charging(struct wireless_charge_device_info *di)
{
	hwlog_info("%s ++\n", __func__);
	wireless_charge_set_wireless_channel_state(WIRELESS_CHANNEL_OFF);
	wireless_charge_sleep_en_enable(RX_SLEEP_EN_ENABLE);
	wireless_charge_set_charge_stage(WIRELESS_STAGE_DEFAULT);
	charge_set_input_current_prop(0, 0);
	wireless_charge_rx_stop_charing_config(di);
	wireless_fast_charge_flag = 0;
	g_fop_fixed_flag = 0;
	cancel_delayed_work_sync(&di->rx_sample_work);
	cancel_delayed_work_sync(&di->wireless_ctrl_work);
	hwlog_info("%s --\n", __func__);
}
static void wireless_charge_initialization(struct wireless_charge_device_info *di)
{
	hwlog_info("%s ++\n", __func__);
	wireless_charge_chip_init(di,WIRELESS_CHIP_INIT);
	wireless_charge_para_init(di);
	wireless_charge_set_charge_stage(WIRELESS_STAGE_CHECK_TX_ID);

	if (WIRELESS_CHANNEL_ON == g_wireless_channel_state) {
		mod_delayed_work(system_wq, &di->wireless_ctrl_work, msecs_to_jiffies(di->ctrl_interval));
		mod_delayed_work(system_wq, &di->wireless_monitor_work, msecs_to_jiffies(0));
		hwlog_info("%s --\n", __func__);
	}
}
static void wireless_charge_wireless_vbus_disconnect_handler(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}

	if (NOT_IN_OTG_MODE == wireless_otg_get_mode()) {
		msleep(CHANNEL_SW_TIME);
		wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);
	}
	charger_source_sink_event(STOP_SINK_WIRELESS);
	wireless_charge_stop_charging(di);
}
static void wireless_charge_wireless_vbus_connect_handler(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}

	wireless_charge_set_wireless_channel_state(WIRELESS_CHANNEL_ON);
	wired_chsw_set_wired_channel(WIRED_CHANNEL_CUTOFF);
	charger_source_sink_event(START_SINK_WIRELESS);
	wireless_charge_initialization(di);
}
static void wireless_charge_wired_vbus_connect_work(struct work_struct *work)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	if (wireless_fast_charge_flag) {
		di->tx_vout_max = TX_DEFAULT_VOUT;
		wireless_charge_set_tx_vout(di);
		wireless_charge_en_enable(RX_EN_DISABLE);
		msleep(CHANNEL_SW_TIME_2);
	} else {
		wireless_charge_en_enable(RX_EN_DISABLE);
	}
	msleep(CHANNEL_SW_TIME);
	wired_chsw_set_wired_channel(WIRED_CHANNEL_RESTORE);
	hwlog_info("wired vbus connect, turn off wireless channel\n");
	wireless_charge_stop_charging(di);
}
static void wireless_charge_wired_vbus_disconnect_work(struct work_struct *work)
{
	wireless_charge_en_enable(RX_EN_ENABLE);
	hwlog_info("wired vbus disconnect, turn on wireless channel\n");
}
void wireless_charge_wired_vbus_connect_handler(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	if (WIRED_CHANNEL_ON == g_wired_channel_state) {
		hwlog_err("%s already in sink_vbus state, ignore\n", __func__);
		return;
	}
	hwlog_info("%s wired vbus connect\n", __func__);
	wireless_charge_set_wired_channel_state(WIRED_CHANNEL_ON);
	schedule_work(&di->wired_vbus_connect_work);
}
void wireless_charge_wired_vbus_disconnect_handler(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	if (WIRED_CHANNEL_OFF == g_wired_channel_state) {
		hwlog_err("%s not in sink_vbus state, ignore\n", __func__);
		return;
	}
	hwlog_info("%s wired vbus disconnect\n", __func__);
	wireless_charge_set_wired_channel_state(WIRED_CHANNEL_OFF);
	schedule_work(&di->wired_vbus_disconnect_work);
}
#ifdef CONFIG_DIRECT_CHARGER
void direct_charger_disconnect_event(void)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	hwlog_info("wired vbus disconnect in scp charging mode\n");
	wireless_charge_set_wired_channel_state(WIRED_CHANNEL_OFF);
	schedule_work(&di->wired_vbus_disconnect_work);
}
#endif
void wireless_charger_pmic_vbus_handler(bool vbus_state)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	if (di && di->ops && di->ops->pmic_vbus_handler) {
		di->ops->pmic_vbus_handler(vbus_state);
	}
}
static void wireless_charge_monitor_work(struct work_struct *work)
{
	struct wireless_charge_device_info *di = g_wireless_di;
	int soc, iout, vout, vrect, fop, iin_regval = 0;
	static int cnt = 0;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	if (WIRELESS_CHANNEL_OFF == g_wireless_channel_state) {
		hwlog_info("%s wireless charge stopped, return \n");
		return;
	}
	if (!wireless_charge_check_tx_exist()){
		wireless_charge_wireless_vbus_disconnect_handler();
		return;
	}
	soc = hisi_battery_capacity();
	vrect = wireless_charge_get_rx_vrect(di);
	vout = wireless_charge_get_rx_vout(di);
	iout = wireless_charge_get_rx_iout(di);
	fop = wireless_charge_get_rx_fop(di);
	iin_regval = charge_get_charger_iinlim_regval();
	wireless_charge_calc_avg_iout(di, iout);
	wireless_charge_count_avg_iout(di);
	wireless_charge_kick_watchdog(di);
	wireless_charge_check_voltage(di);
	if (g_wireless_charge_stage <= WIRELESS_STAGE_CHARGING ||
		cnt++ == MONITOR_LOG_INTERVAL/di->monitor_interval) {
		hwlog_info("[%s] soc = %d, vrect = %dmV, vout = %dmV, iout = %dmA, iout_avg = %dmA, fop = %dkHZ, iin_regval = %d, "
			"sysfs_fixed_fop = %d, sysfs_vout_reset = %d, sysfs_iout_limit = %d\n",
			__func__, soc, vrect, vout, iout, di->iout_avg, fop, iin_regval,
			di->sysfs_data.tx_fixed_fop, di->sysfs_data.rx_vout_reset, di->sysfs_data.rx_iout_max);
		cnt = 0;
	}
	schedule_delayed_work(&di->wireless_monitor_work, msecs_to_jiffies(di->monitor_interval));
}
static void wireless_charge_rx_sample_work(struct work_struct *work)
{
	struct wireless_charge_device_info *di = container_of(work, struct wireless_charge_device_info, rx_sample_work.work);
	int ret = 0;
	int rx_vout;
	int rx_iout;
	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	/*send confirm message to TX */
	rx_vout = di->ops->get_rx_vout();
	rx_iout = di->ops->get_rx_iout();
	ret = di->ops->send_msg_rx_vout(rx_vout);
	ret |= di->ops->send_msg_rx_iout(rx_iout);
	if(ret) {
		hwlog_err("%s: RX send message to TX failed!\n", __func__);
	}
	hwlog_info("[%s]rx_vout = %d, rx_iout = %d\n", __func__, rx_vout,rx_iout);

	schedule_delayed_work(&di->rx_sample_work, msecs_to_jiffies(RX_SAMPLE_WORK_DELAY));
}
static void wireless_charge_rx_connect_work(struct work_struct *work)
{
	wireless_charge_wake_unlock();
}
static void wireless_charge_rx_event_work(struct work_struct *work)
{
	int ret;
	char serial_no[SERIALNO_LEN + 1] = {0};
	char dsm_buff[CHARGE_DMDLOG_SIZE] = {0};
	int batt_temp;
	int batt_capacity;
	struct wireless_charge_device_info *di =
		container_of(work, struct wireless_charge_device_info, wireless_rx_event_work);

	if (!di) {
		hwlog_err("%s di null\n", __func__);
		return;
	}
	hwlog_info("[%s] event_type = %d, event_data = %d \n", __func__, di->rx_event_type, di->rx_event_data);

	switch(di->rx_event_type) {
		case WIRELESS_CHARGE_RX_POWER_ON:
			hwlog_info("[%s] RX power on \n",__func__);
			break;
		case WIRELESS_CHARGE_RX_READY:
			hwlog_info("[%s] RX ready \n",__func__);
			wireless_charge_wake_lock();
			wireless_charge_wireless_vbus_connect_handler();
			mod_delayed_work(system_wq, &di->wireless_rx_con_work, msecs_to_jiffies(RX_CON_WORK_DELAY));
			break;
		case WIRELESS_CHARGE_GET_BATTERY_TEMP:
			hwlog_info("[%s] batt temp \n",__func__);
			batt_temp = hisi_battery_temperature();
			ret = di->ops->send_msg_batt_temp(batt_temp);
			break;
		case WIRELESS_CHARGE_GET_SERIAL_NO:
			hwlog_info("[%s] serialno \n",__func__);
			ret = wireless_charge_get_serialno(serial_no, SERIALNO_LEN + 1);
			ret |= di->ops->send_msg_serialno(serial_no, SERIALNO_LEN);
			break;
		case WIRELESS_CHARGE_GET_BATTERY_CAPACITY:
			hwlog_info("[%s] batt capacity \n",__func__);
			batt_capacity = hisi_battery_capacity();
			ret = di->ops->send_msg_batt_capacity(batt_capacity);
			break;
		case WIRELESS_CHARGE_SET_CURRENT_LIMIT:
			hwlog_info("[%s] set current limit = %dmA\n",__func__, di->rx_event_data * SET_CURRENT_LIMIT_STEP);
			di->rx_iout_limit = di->rx_event_data * SET_CURRENT_LIMIT_STEP;
			wireless_charge_set_input_current(di);
			break;
		case WIRELESS_CHARGE_START_SAMPLE:
			hwlog_info("[%s] RX start sample \n",__func__);
			wireless_start_sample_flag = 1;
			di->rx_iout_limit = di->rx_iout_max;
			wireless_charge_set_input_current(di);
			mod_delayed_work(system_wq, &di->rx_sample_work, msecs_to_jiffies(0));
			break;
		case WIRELESS_CHARGE_STOP_SAMPLE:
			hwlog_info("[%s] RX stop sample \n",__func__);
			wireless_start_sample_flag = 0;
			cancel_delayed_work_sync(&di->rx_sample_work);
			break;
		case WIRELESS_CHARGE_RX_OCP:
			if (g_wireless_charge_stage >= WIRELESS_STAGE_REGULATION) {
				hwlog_err("RX ocp happend \n");
				g_rx_ocp_cnt++;
			}
			if (g_rx_ocp_cnt >= RX_OCP_CNT_MAX) {
				g_rx_ocp_cnt = RX_OCP_CNT_MAX;
				wireless_charge_dsm_report(di,ERROR_WIRELESS_RX_OCP, dsm_buff);
			}
			break;
		case WIRELESS_CHARGE_RX_OVP:
			if (g_wireless_charge_stage >= WIRELESS_STAGE_REGULATION) {
				hwlog_err("RX ovp happend \n");
				g_rx_ovp_cnt++;
			}
			if (g_rx_ovp_cnt >= RX_OVP_CNT_MAX) {
				g_rx_ovp_cnt = RX_OVP_CNT_MAX;
				wireless_charge_dsm_report(di,ERROR_WIRELESS_RX_OVP, dsm_buff);
			}
			break;
		case WIRELESS_CHARGE_RX_OTP:
			if (g_wireless_charge_stage >= WIRELESS_STAGE_REGULATION) {
				hwlog_err("RX otp happend \n");
				g_rx_otp_cnt++;
			}
			if (g_rx_otp_cnt >= RX_OTP_CNT_MAX) {
				g_rx_otp_cnt = RX_OTP_CNT_MAX;
				wireless_charge_dsm_report(di,ERROR_WIRELESS_RX_OTP, dsm_buff);
			}
			break;
		default:
			hwlog_err("%s has no this event_type(%d)\n", __func__, di->rx_event_type);
			break;
	}
}

static int wireless_charge_rx_event_notifier_call(struct notifier_block *rx_event_nb, unsigned int event, void *data)
{
	struct wireless_charge_device_info *di =
	    container_of(rx_event_nb, struct wireless_charge_device_info, rx_event_nb);
	if(!di) {
		hwlog_err("%s: di is NULL\n", __func__);
		return NOTIFY_OK;
	}
	if(data) {
		u8 *rx_notify_data = (u8 *)data;
		di->rx_event_data = *rx_notify_data;
	}
	di->rx_event_type = (enum rx_event_type)event;

	schedule_work(&di->wireless_rx_event_work);
	return NOTIFY_OK;
}
static void wireless_charge_parse_interfer_para
		(struct device_node* np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	unsigned int i = 0;
	int array_len = 0;
	u32 tmp_para[WIRELESS_INTERFER_TOTAL*WIRELESS_INTERFER_PARA_LEVEL];

	array_len = of_property_count_u32_elems(np, "interference_para");
	if ((array_len <= 0) ||(array_len % WIRELESS_INTERFER_TOTAL != 0)) {
		di->interfer_data.total_src = 0;
		hwlog_err("%s: para is invaild, please check wireless_para number!!\n", __func__);
	} else if (array_len > WIRELESS_INTERFER_PARA_LEVEL * WIRELESS_INTERFER_TOTAL) {
		di->interfer_data.total_src = 0;
		hwlog_err("%s: para is too long(%d)!!\n", __func__, array_len);
	} else {
		ret = of_property_read_u32_array(np, "interference_para", tmp_para, array_len);
		if (ret) {
			di->interfer_data.total_src = 0;
			hwlog_err("%s: get para fail!\n",  __func__);
		} else {
			di->interfer_data.interfer_src_state = 0;
			di->interfer_data.total_src = array_len / WIRELESS_INTERFER_TOTAL;
			for (i = 0; i < di->interfer_data.total_src; i++) {
				di->interfer_data.interfer_para[i].src_open =
					tmp_para[(u8)(WIRELESS_INTERFER_SRC_OPEN+WIRELESS_INTERFER_TOTAL*i)];
				di->interfer_data.interfer_para[i].src_close =
					tmp_para[(u8)(WIRELESS_INTERFER_SRC_CLOSE+WIRELESS_INTERFER_TOTAL*i)];
				di->interfer_data.interfer_para[i].tx_fixed_fop =
					tmp_para[(int)(WIRELESS_INTERFER_TX_FIXED_FOP+WIRELESS_INTERFER_TOTAL*i)];
				di->interfer_data.interfer_para[i].rx_vout_reset =
					tmp_para[(int)(WIRELESS_INTERFER_RX_VOUT_RESET+WIRELESS_INTERFER_TOTAL*i)];
				di->interfer_data.interfer_para[i].rx_iout_max =
					tmp_para[(int)(WIRELESS_INTERFER_RX_IOUT_LIMIT+WIRELESS_INTERFER_TOTAL*i)];
				hwlog_info("wireless_interfer_para[%d], src_open: 0x%-2x src_close: 0x%-2x "
					"tx_fixed_fop: %-3d rx_vout_reset: %d rx_iout_max: %-3d\n",
					i, di->interfer_data.interfer_para[i].src_open, di->interfer_data.interfer_para[i].src_close,
					di->interfer_data.interfer_para[i].tx_fixed_fop, di->interfer_data.interfer_para[i].rx_vout_reset,
					di->interfer_data.interfer_para[i].rx_iout_max);
			}
		}
	}
}
static void wireless_charge_parse_segment_para
			(struct device_node* np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	int i = 0;
	int array_len = 0;
	u32 temp_para[WIRELESS_SEGMENT_PARA_TOTAL * WIRELESS_SEGMENT_PARA_LEVEL];

	array_len = of_property_count_u32_elems(np, "segment_para");
	if ((array_len <= 0) || (array_len % WIRELESS_SEGMENT_PARA_TOTAL != 0)) {
		di->segment_data.segment_para_level = 0;
		hwlog_err("%s: para is invaild, please check!\n", __func__);
	} else if (array_len > WIRELESS_SEGMENT_PARA_LEVEL * WIRELESS_SEGMENT_PARA_TOTAL) {
		di->segment_data.segment_para_level = 0;
		hwlog_err("%s: para is too long, please check!!\n", __func__);
	} else {
		ret = of_property_read_u32_array(np, "segment_para", temp_para, array_len);
		if (ret) {
			di->segment_data.segment_para_level = 0;
			hwlog_err("%s: get para fail!\n", __func__);
		} else {
			di->segment_data.segment_para_level = array_len / WIRELESS_SEGMENT_PARA_TOTAL;
			for (i = 0; i < di->segment_data.segment_para_level; i++) {
				di->segment_data.segment_para[i].soc_min = temp_para[(int)(WIRELESS_SEGMENT_PARA_SOC_MIN + WIRELESS_SEGMENT_PARA_TOTAL * i)];
				di->segment_data.segment_para[i].soc_max = temp_para[(int)(WIRELESS_SEGMENT_PARA_SOC_MAX + WIRELESS_SEGMENT_PARA_TOTAL * i)];
				di->segment_data.segment_para[i].boost_flag = temp_para[(int)(WIRELESS_SEGMENT_PARA_BOOST_FLAG + WIRELESS_SEGMENT_PARA_TOTAL * i)];
				di->segment_data.segment_para[i].iout_max = temp_para[(int)(WIRELESS_SEGMENT_PARA_IOUT_MAX + WIRELESS_SEGMENT_PARA_TOTAL * i)];
				hwlog_info("wireless_segment_para[%d], soc_min: %d soc_max: %d boost_flag: %d current_limit: %d\n",
					i, di->segment_data.segment_para[i].soc_min, di->segment_data.segment_para[i].soc_max,
					di->segment_data.segment_para[i].boost_flag, di->segment_data.segment_para[i].iout_max);
			}
		}
	}
}
static void wireless_charge_parse_dts(struct device_node *np, struct wireless_charge_device_info *di)
{
	int ret = 0;
	ret = of_property_read_u32(np, "standard_tx_adaptor", &di->standard_tx_adaptor);
	if (ret) {
		hwlog_err("%s: get standard_tx_adaptor failed\n", __func__);
		di->standard_tx_adaptor = WIRELESS_UNKOWN;
	}
	hwlog_info("%s:  standard_tx_adaptor  = %d.\n", __func__, di->standard_tx_adaptor);
	ret = of_property_read_u32(np, "max_power_time", &di->max_power_time);
	if (ret) {
		hwlog_err("%s: get max_power_time failed\n", __func__);
		di->max_power_time = 0;
	}
	hwlog_info("%s:  max_power_time  = %d.\n", __func__, di->max_power_time);
	ret = of_property_read_u32(np, "max_power", &di->max_power);
	if (ret) {
		hwlog_err("%s: get max_power failed\n", __func__);
		di->max_power = WIRELESS_DEFAULT_POWER;
	}
	hwlog_info("%s:  max_power  = %d.\n", __func__, di->max_power);
	ret = of_property_read_u32(np, "fast_charge_power", &di->fast_charge_power);
	if (ret) {
		hwlog_err("%s: get fast_charge_power failed\n", __func__);
		di->fast_charge_power = WIRELESS_DEFAULT_POWER;
	}
	hwlog_info("%s:  fast_charge_power  = %d.\n", __func__, di->fast_charge_power);
	ret = of_property_read_u32(np, "rx_gain_ratio", &g_rx_gain_ratio);
	if (ret) {
		hwlog_err("%s: get rx_gain_ratio failed\n", __func__);
		g_rx_gain_ratio = RX_DEFAULT_GAIN_RATIO;
	}
	hwlog_info("%s:  rx_gain_ratio  = %d.\n", __func__, g_rx_gain_ratio);
	ret = of_property_read_u32(np, "rx_vrect_min", &di->rx_vrect_min);
	if (ret) {
		hwlog_err("%s: get rx_vrect_min failed\n", __func__);
		di->rx_vrect_min = RX_VRECT_MIN;
	}
	hwlog_info("%s:  rx_vrect_min  = %dmV.\n", __func__, di->rx_vrect_min);
	wireless_charge_parse_interfer_para(np,di);
	wireless_charge_parse_segment_para(np,di);
}

static int wireless_charge_check_ops(struct wireless_charge_device_info *di)
{
	int ret = 0;

	if ((NULL == di->ops) || (di->ops->chip_init == NULL)
		||(di->ops->rx_program_otp == NULL)
		||(di->ops->check_fwupdate == NULL)
		|| (di->ops->set_tx_vout == NULL)
		|| (di->ops->set_rx_vout == NULL)
		|| (di->ops->get_rx_vout == NULL)
		|| (di->ops->get_rx_iout == NULL)
		|| (di->ops->rx_enable == NULL)
		|| (di->ops->rx_sleep_enable == NULL)
		|| (di->ops->check_tx_exist == NULL)
		|| (di->ops->send_chrg_state == NULL)
		|| (di->ops->kick_watchdog == NULL)
		|| (di->ops->set_rx_fod_coef == NULL)
		|| (di->ops->get_rx_fod_coef == NULL)
		|| (di->ops->get_rx_chip_id == NULL)
		|| (di->ops->get_tx_id == NULL)
		|| (di->ops->get_rx_fw_version == NULL)
		|| (di->ops->get_tx_adaptor_type == NULL)
		|| (di->ops->get_tx_capability == NULL)
		|| (di->ops->get_tx_fw_version == NULL)
		|| (di->ops->check_is_otp_exist == NULL)
		|| (di->ops->get_tx_cert == NULL)
		|| (di->ops->send_msg_rx_vout == NULL)
		|| (di->ops->send_msg_rx_iout == NULL)
		|| (di->ops->send_msg_serialno == NULL)
		|| (di->ops->send_msg_batt_temp == NULL)
		|| (di->ops->send_msg_batt_capacity == NULL)
		|| (di->ops->send_msg_cert_confirm == NULL)
		|| (di->ops->send_ept == NULL)
		|| (di->ops->pmic_vbus_handler == NULL)
	) {
		hwlog_err("wireless_charge ops is NULL!\n");
		ret = -EINVAL;
	}

	return ret;
}
/*
 * There are a numerous options that are configurable on the wireless receiver
 * that go well beyond what the power_supply properties provide access to.
 * Provide sysfs access to them so they can be examined and possibly modified
 * on the fly.
 */
 #ifdef CONFIG_SYSFS
#define WIRELESS_CHARGE_SYSFS_FIELD(_name, n, m, store)	\
{					\
	.attr = __ATTR(_name, m, wireless_charge_sysfs_show, store),	\
	.name = WIRELESS_CHARGE_SYSFS_##n,		\
}
#define WIRELESS_CHARGE_SYSFS_FIELD_RW(_name, n)               \
	WIRELESS_CHARGE_SYSFS_FIELD(_name, n, S_IWUSR | S_IRUGO, wireless_charge_sysfs_store)
#define WIRELESS_CHARGE_SYSFS_FIELD_RO(_name, n)               \
	WIRELESS_CHARGE_SYSFS_FIELD(_name, n, S_IRUGO, NULL)
static ssize_t wireless_charge_sysfs_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t wireless_charge_sysfs_store(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count);
struct wireless_charge_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};
static struct wireless_charge_sysfs_field_info wireless_charge_sysfs_field_tbl[] = {
	WIRELESS_CHARGE_SYSFS_FIELD_RO(chip_id, CHIP_ID),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(fw_version, FW_VERSION),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(program_otp, PROGRAM_OTP),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(tx_adaptor_type, TX_ADAPTOR_TYPE),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(vout, VOUT),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(iout, IOUT),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(vrect, VRECT),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(en_enable, EN_ENABLE),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(wireless_succ, WIRELESS_SUCC),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(normal_chrg_succ, NORMAL_CHRG_SUCC),
	WIRELESS_CHARGE_SYSFS_FIELD_RO(fast_chrg_succ, FAST_CHRG_SUCC),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(fod_coef, FOD_COEF),
	WIRELESS_CHARGE_SYSFS_FIELD_RW(interference_setting, INTERFERENCE_SETTING),
};
static struct attribute *wireless_charge_sysfs_attrs[ARRAY_SIZE(wireless_charge_sysfs_field_tbl) + 1];
static const struct attribute_group wireless_charge_sysfs_attr_group = {
	.attrs = wireless_charge_sysfs_attrs,
};
/**********************************************************
*  Function:       wireless_charge_sysfs_init_attrs
*  Discription:    initialize wireless_charge_sysfs_attrs[] for wireless_charge attribute
*  Parameters:   NULL
*  return value:  NULL
**********************************************************/
static void wireless_charge_sysfs_init_attrs(void)
{
	int i, limit = ARRAY_SIZE(wireless_charge_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		wireless_charge_sysfs_attrs[i] = &wireless_charge_sysfs_field_tbl[i].attr.attr;

	wireless_charge_sysfs_attrs[limit] = NULL;
}
/**********************************************************
*  Function:       wireless_charge_sysfs_field_lookup
*  Discription:    get the current device_attribute from wireless_charge_sysfs_field_tbl by attr's name
*  Parameters:   name:evice attribute name
*  return value:  wireless_charge_sysfs_field_tbl[]
**********************************************************/
static struct wireless_charge_sysfs_field_info *wireless_charge_sysfs_field_lookup(const char *name)
{
	int i, limit = ARRAY_SIZE(wireless_charge_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, wireless_charge_sysfs_field_tbl[i].attr.attr.name, strlen(name)))
			break;
	}
	if (i >= limit)
		return NULL;

	return &wireless_charge_sysfs_field_tbl[i];
}
/**********************************************************
*  Function:       wireless_charge_sysfs_create_group
*  Discription:    create the wireless_charge device sysfs group
*  Parameters:   di:wireless_charge_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int wireless_charge_sysfs_create_group(struct wireless_charge_device_info *di)
{
	wireless_charge_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj, &wireless_charge_sysfs_attr_group);
}

/**********************************************************
*  Function:       wireless_charge_sysfs_remove_group
*  Discription:    remove the wireless_charge device sysfs group
*  Parameters:   di:wireless_charge_device_info
*  return value:  NULL
**********************************************************/
static void wireless_charge_sysfs_remove_group(struct wireless_charge_device_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &wireless_charge_sysfs_attr_group);
}
#else
static int wireless_charge_sysfs_create_group(struct wireless_charge_device_info *di)
{
	return 0;
}
static void wireless_charge_sysfs_remove_group(struct wireless_charge_device_info *di)
{
}
#endif

/**********************************************************
*  Function:       wireless_charge_create_sysfs
*  Discription:    create the wireless_charge device sysfs group
*  Parameters:   di:wireless_charge_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int wireless_charge_create_sysfs(struct wireless_charge_device_info *di)
{
	int ret = 0;
	struct class *power_class = NULL;

	ret = wireless_charge_sysfs_create_group(di);
	if (ret) {
		hwlog_err("create sysfs entries failed!\n");
		return ret;
	}
	power_class = hw_power_get_class();
	if (power_class) {
		if (charge_dev == NULL)
			charge_dev = device_create(power_class, NULL, 0, NULL, "charger");
		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj, "wireless_charger");
		if (ret) {
			hwlog_err("create link to wireless_charge fail.\n");
			wireless_charge_sysfs_remove_group(di);
			return ret;
		}
	}

	return 0;
}
/**********************************************************
*  Function:       wireless_charge_sysfs_show
*  Discription:    show the value for all wireless charge nodes
*  Parameters:   dev:device
*                      attr:device_attribute
*                      buf:string of node value
*  return value:  0-sucess or others-fail
**********************************************************/
static ssize_t wireless_charge_sysfs_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct wireless_charge_sysfs_field_info *info = NULL;
	struct wireless_charge_device_info *di = g_wireless_di;
	int chrg_succ = WIRELESS_CHRG_FAIL;
	char tmp_buff[WIRELESS_TMP_STR_LEN] = {0};
	int i;

	info = wireless_charge_sysfs_field_lookup(attr->attr.name);
	if (!info ||!di)
		return -EINVAL;

	switch (info->name) {
	case WIRELESS_CHARGE_SYSFS_CHIP_ID:
		return snprintf(buf, PAGE_SIZE, "%s\n", di->ops->get_rx_chip_id());
	case WIRELESS_CHARGE_SYSFS_FW_VERSION:
		return snprintf(buf, PAGE_SIZE, "%s\n", di->ops->get_rx_fw_version());
	case WIRELESS_CHARGE_SYSFS_PROGRAM_OTP:
		if (!runmode_is_factory()){
			hwlog_info("%s: runmode is not factory, return!\n", __func__);
			return -EINVAL;
		}
		hwlog_info("%s: wireless rx, check whether otp is already exist!\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", di->ops->check_is_otp_exist());
	case WIRELESS_CHARGE_SYSFS_TX_ADAPTOR_TYPE:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->tx_cap->type);
	case WIRELESS_CHARGE_SYSFS_VOUT:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->ops->get_rx_vout());
	case WIRELESS_CHARGE_SYSFS_IOUT:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->ops->get_rx_iout());
	case WIRELESS_CHARGE_SYSFS_VRECT:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->ops->get_rx_vrect());
	case WIRELESS_CHARGE_SYSFS_EN_ENABLE:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_data.en_enable);
	case WIRELESS_CHARGE_SYSFS_WIRELESS_SUCC:
		chrg_succ = wireless_charge_check_fac_test_succ(di);
		if (g_wireless_charge_stage < WIRELESS_STAGE_TOTAL) {
			hwlog_info("%s tx_type = %d, chrg_stage = %s\n",
				__func__, di->tx_cap->type, chrg_stage[g_wireless_charge_stage]);
		}
		return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
	case WIRELESS_CHARGE_SYSFS_NORMAL_CHRG_SUCC:
		chrg_succ = wireless_charge_check_normal_charge_succ(di);
		if (g_wireless_charge_stage < WIRELESS_STAGE_TOTAL) {
			hwlog_info("%s tx_type = %d, chrg_stage = %s\n",
				__func__, di->tx_cap->type, chrg_stage[g_wireless_charge_stage]);
		}
		return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
	case WIRELESS_CHARGE_SYSFS_FAST_CHRG_SUCC:
		chrg_succ = wireless_charge_check_fast_charge_succ(di);
		if (g_wireless_charge_stage < WIRELESS_STAGE_TOTAL) {
			hwlog_info("%s tx_type = %d, chrg_stage = %s\n",
				__func__, di->tx_cap->type, chrg_stage[g_wireless_charge_stage]);
		}
		return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
	case WIRELESS_CHARGE_SYSFS_FOD_COEF:
		return snprintf(buf, PAGE_SIZE, "%s\n", di->ops->get_rx_fod_coef());
	case WIRELESS_CHARGE_SYSFS_INTERFERENCE_SETTING:
		return snprintf(buf, PAGE_SIZE, "%u\n", di->interfer_data.interfer_src_state);
	default:
		hwlog_err("(%s)NODE ERR!!HAVE NO THIS NODE:(%d)\n", __func__, info->name);
		break;
	}
	return 0;
}
/**********************************************************
*  Function:       wireless_charge_sysfs_store
*  Discription:    set the value for all wireless charge nodes
*  Parameters:   dev:device
*                      attr:device_attribute
*                      buf:string of node value
*                      count:unused
*  return value:  0-sucess or others-fail
**********************************************************/
static ssize_t wireless_charge_sysfs_store(struct device *dev,
				   struct device_attribute *attr, const char *buf, size_t count)
{
	struct wireless_charge_sysfs_field_info *info = NULL;
	struct wireless_charge_device_info *di = g_wireless_di;
	long val = 0;
	int ret;

	info = wireless_charge_sysfs_field_lookup(attr->attr.name);
	if (!info ||!di)
		return -EINVAL;

	switch (info->name) {
	case WIRELESS_CHARGE_SYSFS_PROGRAM_OTP:
		if (!runmode_is_factory() || (strict_strtol(buf, 10, &val) < 0) || val != 1){
			hwlog_info("%s: runmode is not factory,  or val is not valid!\n", __func__);
			return -EINVAL;
		}
		schedule_work(&di->rx_program_otp_work);
		hwlog_info("%s: wireless rx program otp\n", __func__);
		break;
	case WIRELESS_CHARGE_SYSFS_EN_ENABLE:
		if ((strict_strtol(buf, 10, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;
		di->sysfs_data.en_enable = val;
		hwlog_info("set rx en_enable = %d\n", di->sysfs_data.en_enable);
		wireless_charge_en_enable(!di->sysfs_data.en_enable);
		break;
	case WIRELESS_CHARGE_SYSFS_FOD_COEF:
		hwlog_info("%s set fod_coef:  %s\n", __func__, buf);
		ret = di->ops->set_rx_fod_coef((char*)buf);
		if (ret)
			hwlog_err("%s set fod_coef fail\n", __func__);
		break;
	case WIRELESS_CHARGE_SYSFS_INTERFERENCE_SETTING:
		hwlog_info("%s interference_settings:  %s", __func__, buf);
		if (strict_strtol(buf, 10, &val) < 0)
			return -EINVAL;
		wireless_charger_update_interference_settings(di, (u8)val);
		break;
	default:
		hwlog_err("(%s)NODE ERR!!HAVE NO THIS NODE:(%d)\n", __func__, info->name);
		break;
	}
	return count;
}
static struct wireless_charge_device_info *wireless_charge_device_info_alloc(void)
{
	static struct wireless_charge_device_info *di;
	static struct tx_capability *tx_cap;
	static struct wireless_core_para *rx_cap;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di) {
		hwlog_err("alloc di failed\n");
		return NULL;
	}
	tx_cap = kzalloc(sizeof(*tx_cap), GFP_KERNEL);
	if (!tx_cap) {
		hwlog_err("alloc tx_cap failed\n");
		goto alloc_fail_1;
	}
	rx_cap = kzalloc(sizeof(*rx_cap), GFP_KERNEL);
	if (!rx_cap) {
		hwlog_err("alloc rx_cap failed\n");
		goto alloc_fail_2;
	}
	di->rx_cap = rx_cap;
	di->tx_cap = tx_cap;
	return di;
alloc_fail_2:
	kfree(tx_cap);
	tx_cap = NULL;
alloc_fail_1:
	kfree(di);
	return NULL;
}
static void wireless_charge_device_info_free(struct wireless_charge_device_info *di)
{
	if(di) {
		if(di->rx_cap) {
			kfree(di->rx_cap);
			di->rx_cap = NULL;
		}
		if(di->tx_cap) {
			kfree(di->tx_cap);
			di->tx_cap = NULL;
		}
		kfree(di);
		di = NULL;
	}
}
static int wireless_charge_shutdown(struct platform_device *pdev)
{
	struct wireless_charge_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	if (NULL == di) {
		hwlog_err("%s di is null\n", __func__);
		return 0;
	}
	cancel_delayed_work(&di->rx_sample_work);
	cancel_delayed_work(&di->wireless_rx_con_work);
	cancel_delayed_work(&di->wireless_ctrl_work);
	cancel_delayed_work(&di->wireless_monitor_work);
	hwlog_info("%s --\n", __func__);

	return 0;
}

static int wireless_charge_remove(struct platform_device *pdev)
{
	struct wireless_charge_device_info *di = platform_get_drvdata(pdev);

	hwlog_info("%s ++\n", __func__);
	if (NULL == di) {
		hwlog_err("%s di is null\n", __func__);
		return 0;
	}

	hwlog_info("%s --\n", __func__);

	return 0;
}
static int wireless_charge_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct wireless_charge_device_info *di = NULL;
	struct device_node *np = NULL;

	di = wireless_charge_device_info_alloc();
	if(!di) {
		hwlog_err("alloc di failed\n");
		return -ENOMEM;
	}

	g_wireless_di = di;
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	di->ops = g_wireless_ops;
	platform_set_drvdata(pdev, di);
	di->core_data = charge_core_get_wireless_params();
	if (NULL == di->core_data){
	    hwlog_err("%s di->core_data is NULL\n", __func__);
	    ret = -EINVAL;
	    goto wireless_charge_fail_0;
	}
	wake_lock_init(&g_rx_con_wakelock, WAKE_LOCK_SUSPEND, "rx_con_wakelock");

	ret = wireless_charge_check_ops(di);
	if (ret)
		goto wireless_charge_fail_0;

	wireless_charge_parse_dts(np,di);

	INIT_WORK(&di->wired_vbus_connect_work, wireless_charge_wired_vbus_connect_work);
	INIT_WORK(&di->wired_vbus_disconnect_work, wireless_charge_wired_vbus_disconnect_work);
	INIT_WORK(&di->rx_program_otp_work, wireless_charge_rx_program_otp_work);
	INIT_WORK(&di->wireless_rx_event_work, wireless_charge_rx_event_work);
	INIT_DELAYED_WORK(&di->wireless_ctrl_work, wireless_charge_control_work);
	INIT_DELAYED_WORK(&di->wireless_rx_con_work, wireless_charge_rx_connect_work);
	INIT_DELAYED_WORK(&di->rx_sample_work, wireless_charge_rx_sample_work);
	INIT_DELAYED_WORK(&di->wireless_monitor_work, wireless_charge_monitor_work);

	BLOCKING_INIT_NOTIFIER_HEAD(&di->wireless_charge_evt_nh);
	di->rx_event_nb.notifier_call = wireless_charge_rx_event_notifier_call;
	ret = blocking_notifier_chain_register(&rx_event_nh, &di->rx_event_nb);
	if (ret < 0) {
		hwlog_err("register rx_connect notifier failed\n");
		goto  wireless_charge_fail_0;
	}
	if (wireless_charge_check_tx_exist()) {
		wireless_charge_wireless_vbus_connect_handler();
		schedule_delayed_work(&di->wireless_monitor_work, msecs_to_jiffies(0));
	}
	ret = wireless_charge_create_sysfs(di);
	if (ret)
		hwlog_err("wirelss_charge create sysfs fail.\n");

	hwlog_info("wireless_charger probe ok.\n");
	return 0;

wireless_charge_fail_0:
	di->ops = NULL;
	wireless_charge_device_info_free(di);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static struct of_device_id wireless_charge_match_table[] = {
	{
	 .compatible = "huawei,wireless_charger",
	 .data = NULL,
	},
	{},
};

static struct platform_driver wireless_charge_driver = {
	.probe = wireless_charge_probe,
	.remove = wireless_charge_remove,
	.shutdown = wireless_charge_shutdown,
	.driver = {
		.name = "huawei,wireless_charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wireless_charge_match_table),
	},
};
/**********************************************************
*  Function:       wireless_charge_init
*  Description:    wireless charge module initialization
*  Parameters:   NULL
*  return value:  0-sucess or others-fail
**********************************************************/
static int __init wireless_charge_init(void)
{
	hwlog_info("wireless_charger init ok.\n");

	return platform_driver_register(&wireless_charge_driver);
}
/**********************************************************
*  Function:       wireless_charge_exit
*  Description:    wireless charge module exit
*  Parameters:   NULL
*  return value:  NULL
**********************************************************/
static void __exit wireless_charge_exit(void)
{
	platform_driver_unregister(&wireless_charge_driver);
}

device_initcall_sync(wireless_charge_init);
module_exit(wireless_charge_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wireless charge module driver");
MODULE_AUTHOR("HUAWEI Inc");
