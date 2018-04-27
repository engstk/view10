#ifndef _WIRELESS_CHARGER
#define _WIRELESS_CHARGER

#define I2C_RETRY_CNT               3
#define I2C_OPS_SLEEP_TIME          5
#define BYTE_MASK                   0xff
#define WORD_MASK                   0xffff
#define BYTE_LEN                    1
#define WORD_LEN                    2

#define WIRELESS_CHANNEL_ON         1
#define WIRELESS_CHANNEL_OFF        0
#define WIRED_CHANNEL_ON            1
#define WIRED_CHANNEL_OFF           0

#define RX_EN_ENABLE                0
#define RX_EN_DISABLE               1
#define RX_SLEEP_EN_ENABLE          1
#define RX_SLEEP_EN_DISABLE         0

#define CERTI_SUCC                  0
#define CERTI_FAIL                  1

#define WIRELESS_CHRG_SUCC          0
#define WIRELESS_CHRG_FAIL          1

#define MSEC_PER_SEC                1000
#define MVOLT_PER_VOLT              1000
#define PERCENT                     100
#define RX_IOUT_MIN                 150
#define RX_VRECT_MIN                4500
#define TX_BOOST_VOUT               12000
#define TX_DEFAULT_VOUT             5000
#define RX_DEFAULT_GAIN_RATIO       100
#define CHANNEL_SW_TIME             50
#define CHANNEL_SW_TIME_2           150


#define RX_HIGH_IOUT                850
#define RX_LOW_IOUT                 300
#define RX_AVG_IOUT_TIME            60*1000
#define RX_IOUT_REG_GAP1            100
#define RX_IOUT_REG_GAP2            200
#define RX_IOUT_REG_STEP            100
#define RX_VRECT_LOW_RESTORE_TIME   10000
#define RX_VRECT_LOW_TH             0
#define RX_VRECT_LOW_IOUT_MIN       300
#define RX_VRECT_ERR_CHECK_TIME     1000

#define TX_ID_HW                    0x8866

#define RX_CON_WORK_DELAY           3000
#define CONTROL_INTERVAL_NORMAL     300
#define CONTROL_INTERVAL_FAST       100
#define MONITOR_INTERVAL            100
#define MONITOR_LOG_INTERVAL        5000

#define RX_IOUT_SAMPLE_LEN          10
#define WIRELESS_STAGE_STR_LEN      32
#define WIRELESS_TMP_STR_LEN        16

#define WIRELESS_DEFAULT_POWER      5000000 //mV*mA

#define TX_ID_ERR_CNT_MAX           3
#define TX_ABILITY_ERR_CNT_MAX      3
#define CERTI_ERR_CNT_MAX           3
#define BOOST_ERR_CNT_MAX           5


#define RX_OCP_CNT_MAX              3
#define RX_OVP_CNT_MAX              3
#define RX_OTP_CNT_MAX              3

#define SET_CURRENT_LIMIT_STEP      100
#define RX_SAMPLE_WORK_DELAY        500
#define SERIALNO_LEN                16
#define WIRELESS_RANDOM_LEN         8
#define WIRELESS_TX_KEY_LEN         8
#define WIRELESS_RX_KEY_LEN         8

/*rx charge state*/
#define WIRELESS_CHRG_STATE_FULL    0x01

#define WIRELESS_INTERFER_PARA_LEVEL        8
#define WIRELESS_CORE_PARA_LEVEL            10
#define WIRELESS_SEGMENT_PARA_LEVEL         5
#define WIRELESS_CHIP_INIT                  0

#define WIRELESS_INT_CNT_TH                 10
#define WIRELESS_INT_TIMEOUT_TH             15*1000 //ms

enum tx_adaptor_type {
	WIRELESS_UNKOWN   = 0x00,
	WIRELESS_SDP      = 0x01,
	WIRELESS_CDP      = 0x02,
	WIRELESS_NON_STD  = 0x03,
	WIRELESS_DCP      = 0x04,
	WIRELESS_FCP      = 0x05,
	WIRELESS_SCP      = 0x06,
	WIRELESS_PD       = 0x07,
	WIRELESS_QC       = 0x08,
	WIRELESS_TYPE_ERR = 0xff,
};
enum wireless_etp_type {
	WIRELESS_EPT_UNKOWN         = 0x00,
	WIRELESS_EPT_CHRG_COMPLETE  = 0x01,
	WIRELESS_EPT_INTERNAL_FAULT = 0x02,
	WIRELESS_EPT_OTP            = 0x03,
	WIRELESS_EPT_OVP            = 0x04,
	WIRELESS_EPT_OCP            = 0x05,
	WIRELESS_EPT_BATT_FAILURE   = 0x06,
	WIRELESS_EPT_RESERVED       = 0x07,
	WIRELESS_EPT_NO_RESPONSE    = 0x08,
	WIRELESS_EPT_ERR_VRECT      = 0xA0,
};
enum wireless_charge_stage {
	WIRELESS_STAGE_DEFAULT = 0,
	WIRELESS_STAGE_CHECK_TX_ID,
	WIRELESS_STAGE_CHECK_TX_ABILITY,
	WIRELESS_STAGE_CHECK_RX_ABILITY,
	WIRELESS_STAGE_CABLE_DETECT,
	WIRELESS_STAGE_CERTIFICATION,
	WIRELESS_STAGE_CHECK_FWUPDATE,
	WIRELESS_STAGE_CHARGING,
	WIRELESS_STAGE_REGULATION,
	WIRELESS_STAGE_TOTAL,
};
enum wireless_charge_state {
	WIRELESS_NORMAL_CHARGE = 0,
	WIRELESS_FAST_CHARGE,
};
struct tx_capability {
	u8 type;
	int vout_max;
	int iout_max;
	u8 boost;
	u8 cable;
	u8 fan;
	u8 tec;
};
enum tx_cap_info {
	TX_CAP_TYPE = 1,
	TX_CAP_VOUT_MAX,
	TX_CAP_IOUT_MAX,
	TX_CAP_ATTR,
	TX_CAP_TOTAL,
};

enum wireless_interfer_info {
	WIRELESS_INTERFER_SRC_OPEN = 0,
	WIRELESS_INTERFER_SRC_CLOSE,
	WIRELESS_INTERFER_TX_FIXED_FOP,
	WIRELESS_INTERFER_RX_VOUT_RESET,
	WIRELESS_INTERFER_RX_IOUT_LIMIT,
	WIRELESS_INTERFER_TOTAL,
};
struct wireless_interfer_para {
	u8 src_open;
	u8 src_close;
	int tx_fixed_fop;
	int rx_vout_reset;
	int rx_iout_max;
};
struct wireless_interfer_data {
	int total_src;
	u8 interfer_src_state;
	struct wireless_interfer_para
		interfer_para[WIRELESS_INTERFER_PARA_LEVEL];
};

enum wireless_segment_info {
	WIRELESS_SEGMENT_PARA_SOC_MIN = 0,
	WIRELESS_SEGMENT_PARA_SOC_MAX,
	WIRELESS_SEGMENT_PARA_BOOST_FLAG,
	WIRELESS_SEGMENT_PARA_IOUT_MAX,
	WIRELESS_SEGMENT_PARA_TOTAL,
};
struct wireless_segment_para {
	int soc_min;
	int soc_max;
	int boost_flag;
	int iout_max;
};
struct wireless_segment_data {
	int segment_para_level;
	struct wireless_segment_para segment_para[WIRELESS_SEGMENT_PARA_LEVEL];
};

enum wireless_core_para_info {
	WIRELESS_PARA_CHARGER_TYPE = 0,
	WIRELESS_PARA_CABLE_DETECT,
	WIRELESS_PARA_CERTIFICATION,
	WIRELESS_PARA_FAST_CHRG_FLAG,
	WIRELESS_PARA_FAST_VOUT_MAX,
	WIRELESS_PARA_FAST_IOUT_MAX,
	WIRELESS_PARA_NORMAL_VOUT_MAX,
	WIRELESS_PARA_NORMAL_IOUT_MAX,
	WIRELESS_PARA_TOTAL,
};
struct wireless_core_para {
	u8 charger_type;
	u8 cable_detect;
	u8 certification;
	int fast_chrg_flag;
	int fast_vout_max;
	int fast_iout_max;
	int normal_vout_max;
	int normal_iout_max;
};
struct wireless_core_data {
	int total_type;
	struct wireless_core_para rx_cap[WIRELESS_CORE_PARA_LEVEL];
};
struct wireless_charge_device_ops {
	int (*chip_init)(int);
	void (*rx_enable)(int);
	void (*rx_sleep_enable)(int);
	bool (*check_tx_exist)(void);
	int (*send_chrg_state)(int);
	int (*kick_watchdog)(void);
	int (*get_rx_vrect)(void);
	int (*get_rx_vout)(void);
	int (*get_rx_iout)(void);
	int (*get_rx_fop)(void);
	int (*set_tx_vout)(int);
	int (*set_rx_vout)(int);
	int (*set_rx_fod_coef)(char *);
	int (*fix_tx_fop)(int);
	int (*unfix_tx_fop)(void);
	int (*get_tx_id)(void);
	u8* (*get_rx_chip_id)(void);
	u8* (*get_rx_fw_version)(void);
	char* (*get_rx_fod_coef)(void);
	enum tx_adaptor_type (*get_tx_adaptor_type)(void);
	int (*rx_program_otp)(void);
	int (*check_fwupdate)(void);
	struct tx_capability * (*get_tx_capability)(void);
	u8* (*get_tx_fw_version)(void);
	int (*check_ac_power)(void);
	int (*send_ept)(enum wireless_etp_type);
	int (*stop_charging)(void);
	int (*check_is_otp_exist)(void);
	int (*get_tx_cert)(u8*, unsigned int, u8*, unsigned int);
	int (*send_msg_rx_vout)(int);
	int (*send_msg_rx_iout)(int);
	int (*send_msg_serialno)(char*, unsigned int);
	int (*send_msg_batt_temp)(int);
	int (*send_msg_batt_capacity)(int);
	int (*send_msg_cert_confirm)(bool);
	int (*send_msg_rx_boost_succ)(void);
	void (*pmic_vbus_handler)(bool);
};
struct wireless_charge_sysfs_data {
	int en_enable;
	int tx_fixed_fop;
	int rx_vout_reset;
	int rx_iout_max;
};
struct wireless_charge_device_info {
	struct device *dev;
	struct notifier_block rx_event_nb;
	struct blocking_notifier_head wireless_charge_evt_nh;
	struct work_struct wired_vbus_connect_work;
	struct work_struct wired_vbus_disconnect_work;
	struct work_struct rx_program_otp_work;
	struct work_struct wireless_rx_event_work;
	struct delayed_work wireless_ctrl_work;
	struct delayed_work wireless_monitor_work;
	struct delayed_work wireless_rx_con_work;
	struct delayed_work rx_sample_work;
	struct tx_capability *tx_cap;
	struct wireless_core_para *rx_cap;
	struct wireless_charge_device_ops *ops;
	struct wireless_core_data *core_data;
	struct wireless_charge_sysfs_data sysfs_data;
	struct wireless_interfer_data interfer_data;
	struct wireless_segment_data segment_data;
	enum tx_adaptor_type standard_tx_adaptor;
	int standard_tx;
	int max_power;
	int fast_charge_power;
	int fast_charge_power_now;
	int max_power_time;
	unsigned long max_power_time_out;
	int rx_gain_ratio;
	int tx_vout_max;
	int rx_iout_max;
	int rx_vrect_min;
	enum wireless_charge_stage stage;
	int ctrl_interval;
	int monitor_interval;
	int tx_id_err_cnt;
	int tx_ability_err_cnt;
	int certi_err_cnt;
	int boost_err_cnt;
	int rx_event_type;
	int rx_event_data;
	int rx_iout_limit;
	int iout_avg;
	int iout_high_cnt;
	int iout_low_cnt;
};
enum wireless_charge_sysfs_type {
	WIRELESS_CHARGE_SYSFS_CHIP_ID = 0,
	WIRELESS_CHARGE_SYSFS_FW_VERSION,
	WIRELESS_CHARGE_SYSFS_PROGRAM_OTP,
	WIRELESS_CHARGE_SYSFS_TX_ADAPTOR_TYPE,
	WIRELESS_CHARGE_SYSFS_VOUT,
	WIRELESS_CHARGE_SYSFS_IOUT,
	WIRELESS_CHARGE_SYSFS_VRECT,
	WIRELESS_CHARGE_SYSFS_EN_ENABLE,
	WIRELESS_CHARGE_SYSFS_WIRELESS_SUCC,
	WIRELESS_CHARGE_SYSFS_NORMAL_CHRG_SUCC,
	WIRELESS_CHARGE_SYSFS_FAST_CHRG_SUCC,
	WIRELESS_CHARGE_SYSFS_FOD_COEF,
	WIRELESS_CHARGE_SYSFS_INTERFERENCE_SETTING,
};
enum rx_event_type{
	WIRELESS_CHARGE_RX_POWER_ON = 0,
	WIRELESS_CHARGE_RX_READY,
	WIRELESS_CHARGE_GET_SERIAL_NO,
	WIRELESS_CHARGE_GET_BATTERY_TEMP,
	WIRELESS_CHARGE_GET_BATTERY_CAPACITY,
	WIRELESS_CHARGE_SET_CURRENT_LIMIT,
	WIRELESS_CHARGE_START_SAMPLE,
	WIRELESS_CHARGE_STOP_SAMPLE,
	WIRELESS_CHARGE_RX_OCP,
	WIRELESS_CHARGE_RX_OVP,
	WIRELESS_CHARGE_RX_OTP,
};

extern struct blocking_notifier_head rx_event_nh;
extern int wireless_charge_ops_register(struct wireless_charge_device_ops *ops);
extern int register_wireless_charger_vbus_notifier(struct notifier_block *nb);
extern void wireless_charge_wired_vbus_connect_handler(void);
extern void wireless_charge_wired_vbus_disconnect_handler(void);
extern int wireless_charge_get_wireless_channel_state(void);
extern void wireless_charge_set_wireless_channel_state(int state);
extern int wireless_charge_get_wired_channel_state(void);
extern void direct_charger_disconnect_event(void);
extern int wireless_charge_get_fast_charge_flag(void);
extern int wireless_charge_get_rx_iout_limit(void);
extern bool wireless_charge_check_tx_exist(void);
extern void wireless_charger_pmic_vbus_handler(bool);

#endif
