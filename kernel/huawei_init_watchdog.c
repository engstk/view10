
#include <linux/mm.h>
#include <linux/cpu.h>
#include <linux/nmi.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/lockdep.h>
#include <linux/export.h>
#include <linux/sysctl.h>
#include <linux/utsname.h>
#include <trace/events/sched.h>
#include <linux/ptrace.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <asm/traps.h>
#include <linux/suspend.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>


#define ENABLE_SHOW_LEN  6
#define SIG_TO_INIT      40
#define SIG_INT_VALUE 1234
#define WATCHDOG_CHECK_TIME 30

static unsigned int init_watchdog_enable = 0;
static unsigned int init_watchdog_firstkick = 0;
static unsigned int init_watchdog_status = 1;
static unsigned int init_in_d_state = 0;
static struct timer_list init_watchdog_timer;


static void send_signal_to_init(int signum){
    int pid = 1;
    int ret;
    struct siginfo info;
    struct task_struct *t;
    memset(&info, 0, sizeof(struct siginfo));
    info.si_signo = signum;
    info.si_code = SI_QUEUE;
    info.si_int = SIG_INT_VALUE;
    rcu_read_lock();
    t = find_task_by_vpid(pid);
    if(t == NULL){
        pr_err("hw_init: no such pid\n");
        rcu_read_unlock();
    }
    else{
        rcu_read_unlock();
        ret = send_sig_info(signum, &info, t);
        if (ret < 0) {
        pr_err("hw_init: error sending signal\n");
        }
        else{
        pr_err("hw_init: sending signal success\n");
        }
    }
}

static void init_watchdog_check(void)
{
    if (!init_watchdog_enable || !init_watchdog_firstkick){
        pr_info("hw_init: init_watchdog is closed!");
        mod_timer(&init_watchdog_timer, jiffies+msecs_to_jiffies(1000 * WATCHDOG_CHECK_TIME));
        init_in_d_state = 0;
        return;
    }

    if (init_watchdog_status) {
        init_watchdog_status = 0;
        mod_timer(&init_watchdog_timer, jiffies+msecs_to_jiffies(1000 * WATCHDOG_CHECK_TIME));
        init_in_d_state = 0;
        return;
    }

    struct task_struct *p = pid_task(find_vpid(1), PIDTYPE_PID);

    if(p->flags & PF_FROZEN){
        pr_err("hw_init: init process is frozen!\n");
        mod_timer(&init_watchdog_timer, jiffies+msecs_to_jiffies(1000 * WATCHDOG_CHECK_TIME));
        init_in_d_state = 0;
        return;
    }

    if(p->state == TASK_UNINTERRUPTIBLE){
        pr_err("hw_init: init process is in D state!\n");
        init_in_d_state ++;
        if(init_in_d_state == 2){
            panic("Init process blocked for more than 60s");
        }
        mod_timer(&init_watchdog_timer, jiffies+msecs_to_jiffies(1000 * WATCHDOG_CHECK_TIME));
        return;
    }

    pr_err("hw_init: init_watchdog is  long time not kicked!\n");
    send_signal_to_init(SIG_TO_INIT);
    init_in_d_state = 0;
    mod_timer(&init_watchdog_timer, jiffies+msecs_to_jiffies(1000 * WATCHDOG_CHECK_TIME));
}


static ssize_t init_watchdog_show(struct kobject *kobj, struct kobj_attribute *attr,
                    char *buf)
{
    if (init_watchdog_status)
        return snprintf(buf, ENABLE_SHOW_LEN, "kick\n");
    if (init_watchdog_enable)
        return snprintf(buf, ENABLE_SHOW_LEN, "on\n");
    else
        return snprintf(buf, ENABLE_SHOW_LEN, "off\n");
}

static ssize_t init_watchdog_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count)
{
    char tmp[6];
    size_t len = 0;
    char *p;
    if ((count < 2) || (count > (sizeof(tmp) - 1))) {
        pr_err("hw_init: string too long or too short for init_watchdog.\n");
        return -EINVAL;
    }
    if (!buf)
        return -EINVAL;
    p = memchr(buf, '\n', count);
    len = p ? (size_t) (p - buf) : count;
    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp, buf, len);
    if (strncmp(tmp, "on", strlen(tmp)) == 0) {
        init_watchdog_enable = 1;
        pr_info("hw_init: init_watchdog_enable is set to enable.\n");
    } else if (unlikely(strncmp(tmp, "off", strlen(tmp)) == 0)) {
        init_watchdog_enable = 0;
        pr_info("hw_init: init_watchdog_enable is set to disable.\n");
    } else if (likely(strncmp(tmp, "kick", strlen(tmp)) == 0)) {
        init_watchdog_firstkick = 1;
        init_watchdog_status = 1;
        pr_err("hw_init: init_watchdog is kicked.\n");
    } else
        pr_err("hw_init: only accept on off or kick !\n");
    return (ssize_t) count;
}


static struct kobj_attribute init_watchdog_attribute = {
    .attr = {
        .name = "init",
        .mode = 0640,
    },
    .show = init_watchdog_show,
    .store = init_watchdog_store,
};



static struct attribute *attrs[] = {
    &init_watchdog_attribute.attr,
    NULL
};


static struct attribute_group init_attr_group = {
    .attrs = attrs,
};

struct kobject *init_kobj;
int create_sysfs_init(void)
{
    pr_err("hw_init: init_kobj created!");
    int retval = 0;
    while (kernel_kobj == NULL)
        msleep(1000);

    init_kobj = kobject_create_and_add("init_watchdog", kernel_kobj);
    if (!init_kobj){
        pr_err("hw_init: create init_watchdog failed.\n");
        return -ENOMEM;
    }

    retval = sysfs_create_group(init_kobj, &init_attr_group);
    if (retval){
        kobject_put(init_kobj);
    }
    return retval;
}

static void init_watchdog_handler(unsigned long time) {
    init_watchdog_check();
}

static int __init init_watchdog_init(void)
{
    int init_ret=0;
    init_ret=create_sysfs_init();
    if (init_ret){
        pr_err("hw_init: create_sysfs_init fail.\n");
        return 0;
    }
    init_timer(&init_watchdog_timer);
    init_watchdog_timer.function = init_watchdog_handler;
    init_watchdog_timer.expires = jiffies + HZ * WATCHDOG_CHECK_TIME;
    add_timer(&init_watchdog_timer);
    return 0;
}

subsys_initcall(init_watchdog_init);
