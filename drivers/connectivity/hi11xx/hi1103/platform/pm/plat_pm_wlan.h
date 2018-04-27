

#ifndef __PLAT_PM_WLAN_H__
#define __PLAT_PM_WLAN_H__

/*****************************************************************************
  1 Include other Head file
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/mutex.h>
#include <linux/kernel.h>
#ifdef CONFIG_WAKELOCK
#include <linux/wakelock.h>
#endif
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>

#include <linux/fb.h>
#endif
#include "oal_hcc_bus.h"

#include "oal_ext_if.h"



#define WLAN_WAKUP_MSG_WAIT_TIMEOUT     (1000)
#define WLAN_SLEEP_MSG_WAIT_TIMEOUT     (10000)
#define WLAN_POWEROFF_ACK_WAIT_TIMEOUT  (1000)
#define WLAN_OPEN_BCPU_WAIT_TIMEOUT     (1000)
#define WLAN_HALT_BCPU_TIMEOUT          (1000)
#define WLAN_SLEEP_TIMER_PERIOD         (20)    /*睡眠定时器50ms定时*/
#define WLAN_SLEEP_DEFAULT_CHECK_CNT    (5)     /*默认检查2次，即100ms*/
#define WLAN_SLEEP_LONG_CHECK_CNT       (20)    /*入网阶段,延长至400ms*/
#define WLAN_SLEEP_FAST_CHECK_CNT       (1)     /*fast sleep,20ms*/

#define WLAN_SDIO_MSG_RETRY_NUM         (3)
#define WLAN_WAKEUP_FAIL_MAX_TIMES      (1)  /* 连续多少次wakeup失败，可进入DFR流程 */

#define WLAN_PM_MODULE               "[wlan]"


enum WLAN_PM_CPU_FREQ_ENUM
{
    WLCPU_40MHZ     =   1,
    WLCPU_80MHZ     =   2,
    WLCPU_160MHZ    =   3,
    WLCPU_240MHZ    =   4,
    WLCPU_320MHZ    =   5,
    WLCPU_480MHZ    =   6,
};

enum WLAN_PM_SLEEP_STAGE
{
    SLEEP_STAGE_INIT    = 0,  //初始
    SLEEP_REQ_SND       = 1,  //sleep request发送完成
    SLEEP_ALLOW_RCV     = 2,  //收到allow sleep response
    SLEEP_DISALLOW_RCV  = 3,  //收到allow sleep response
    SLEEP_CMD_SND       = 4,  //允许睡眠reg设置完成
};

#define ALLOW_IDLESLEEP     (1)
#define DISALLOW_IDLESLEEP  (0)

#define WIFI_PM_POWERUP_EVENT          (3)
#define WIFI_PM_POWERDOWN_EVENT        (2)
#define WIFI_PM_SLEEP_EVENT            (1)
#define WIFI_PM_WAKEUP_EVENT           (0)

/*****************************************************************************
  3 STRUCT DEFINE
*****************************************************************************/
typedef oal_bool_enum_uint8 (*wifi_srv_get_pm_pause_func)(oal_void);
typedef oal_void (*wifi_srv_open_notify)(oal_bool_enum_uint8);
typedef oal_void (*wifi_srv_pm_state_notify)(oal_bool_enum_uint8);

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
typedef oal_void (*wifi_srv_host_wkup_dev_print_en_func)(oal_bool_enum_uint8);
#endif
struct wifi_srv_callback_handler
{
    wifi_srv_get_pm_pause_func     p_wifi_srv_get_pm_pause_func;
    wifi_srv_open_notify           p_wifi_srv_open_notify;
    wifi_srv_pm_state_notify       p_wifi_srv_pm_state_notify;
#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    wifi_srv_host_wkup_dev_print_en_func     p_host_wkup_dev_print_en_func;
#endif
};


struct wlan_pm_s
{
    hcc_bus                *pst_bus;              //保存oal_bus 的指针

    oal_uint                ul_wlan_pm_enable;    //pm使能开关
    oal_uint                ul_wlan_power_state;  //wlan power on state
    oal_uint                ul_apmode_allow_pm_flag;   /* ap模式下，是否允许下电操作,1:允许,0:不允许 */

    volatile oal_uint       ul_wlan_dev_state;    //wlan sleep state

    oal_workqueue_stru*     pst_pm_wq;           //pm work quque
    oal_work_stru           st_wakeup_work;       //唤醒work
    oal_work_stru           st_sleep_work;        //sleep work
    oal_work_stru           st_ram_reg_test_work;  //ram_reg_test work

    struct timer_list       st_watchdog_timer;   //sleep watch dog
    oal_uint32              ul_packet_cnt;       //睡眠周期内统计的packet个数
    oal_uint32              ul_wdg_timeout_cnt;  //timeout check cnt
    oal_uint32              ul_wdg_timeout_curr_cnt;  //timeout check current cnt
    volatile oal_uint       ul_sleep_stage;      //睡眠过程阶段标识

    oal_completion          st_open_bcpu_done;
    oal_completion          st_close_bcpu_done;
    oal_completion          st_close_done;
//    oal_completion          st_device_ready;
    oal_completion          st_wakeup_done;
    oal_completion          st_sleep_request_ack;
    oal_completion          st_halt_bcpu_done;
    oal_completion          st_wifi_powerup_done;


    struct wifi_srv_callback_handler st_wifi_srv_handler;

    /* 维测统计 */
    oal_uint32              ul_open_cnt;
    oal_uint32              ul_open_bcpu_done_callback;
    oal_uint32              ul_close_bcpu_done_callback;
    oal_uint32              ul_close_cnt;
    oal_uint32              ul_close_done_callback;
    oal_uint32              ul_wakeup_succ;
    oal_uint32              ul_wakeup_succ_work_submit;
	oal_uint32              ul_wakeup_dev_ack;
    oal_uint32              ul_wakeup_done_callback;
    oal_uint32              ul_wakeup_fail_wait_sdio;
    oal_uint32              ul_wakeup_fail_timeout;
    oal_uint32              ul_wakeup_fail_set_reg;
    oal_uint32              ul_wakeup_fail_submit_work;

    oal_uint32              ul_sleep_succ;
    oal_uint32              ul_sleep_feed_wdg_cnt;
    oal_uint32              ul_sleep_fail_request;
    oal_uint32              ul_sleep_fail_wait_timeout;
    oal_uint32              ul_sleep_fail_set_reg;
    oal_uint32              ul_sleep_fail_forbid;
    oal_uint32              ul_sleep_work_submit;


};
typedef struct wlan_memdump_s{
    int32 addr;
    int32 len;
    int32 en;
}wlan_memdump_t;

/*****************************************************************************
  4 EXTERN VARIABLE
*****************************************************************************/
extern oal_bool_enum g_wlan_pm_switch;
extern oal_uint8 g_wlan_device_pm_switch;
extern oal_uint8 g_wlan_fast_check_cnt;
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern oal_uint8 g_uc_custom_cali_done;
#endif

/*****************************************************************************
  5 EXTERN FUNCTION
*****************************************************************************/
extern struct wlan_pm_s*  wlan_pm_get_drv(oal_void);
extern oal_void wlan_pm_debug_sleep(void);
extern oal_void wlan_pm_debug_wakeup(void);
extern void wlan_pm_dump_host_info(void);
extern oal_int32 wlan_pm_host_info_print(struct wlan_pm_s *pst_wlan_pm,char* buf,oal_int32 buf_len);
extern void wlan_pm_dump_device_info(void);
extern oal_void wlan_pm_debug_wake_lock(void);
extern oal_void wlan_pm_debug_wake_unlock(void);
extern struct wlan_pm_s*  wlan_pm_init(oal_void);
extern oal_uint  wlan_pm_exit(oal_void);
extern oal_uint32 wlan_pm_is_poweron(oal_void);
extern oal_int32 wlan_pm_open(oal_void);
extern oal_uint32 wlan_pm_close(oal_void);
extern oal_uint wlan_pm_init_dev(void);
extern oal_uint wlan_pm_wakeup_dev(oal_void);
extern oal_uint wlan_pm_wakeup_host(void);
extern oal_uint  wlan_pm_open_bcpu(oal_void);
extern oal_uint wlan_pm_state_get(void);
extern oal_uint32 wlan_pm_enable(oal_void);
extern oal_uint32 wlan_pm_disable(oal_void);
extern oal_uint32 wlan_pm_statesave(oal_void);
extern oal_uint32 wlan_pm_staterestore(oal_void);
extern oal_uint32 wlan_pm_disable_check_wakeup(oal_int32 flag);
struct wifi_srv_callback_handler* wlan_pm_get_wifi_srv_handler(oal_void);
extern oal_void wlan_pm_wakeup_dev_ack(oal_void);
extern oal_void  wlan_pm_set_timeout(oal_uint32 ul_timeout);
extern oal_int32 wlan_pm_poweroff_cmd(oal_void);
extern oal_int32 wlan_pm_shutdown_bcpu_cmd(oal_void);
extern oal_void  wlan_pm_feed_wdg(oal_void);
extern oal_int32 wlan_pm_stop_wdg(struct wlan_pm_s *pst_wlan_pm_info);
extern void wlan_pm_info_clean(void);
extern wlan_memdump_t* get_wlan_memdump_cfg(void);
extern oal_int32     wlan_mem_check_mdelay;
extern wlan_memdump_t* get_wlan_memdump_cfg(void);

#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
extern oal_int32 wlan_device_mem_check(void);
extern oal_int32 wlan_device_mem_check_result(unsigned long long *time);
extern oal_void wlan_device_mem_check_work(oal_work_stru *pst_worker);
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_int32 wlan_pm_register_notifier(struct notifier_block *nb);
oal_void wlan_pm_unregister_notifier(struct notifier_block *nb);
#endif

#endif

