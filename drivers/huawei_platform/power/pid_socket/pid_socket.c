/*copyright (c) Huawei Technologies Co., Ltd. 1998-2014. All rights reserved.
 *
 * File name: pid_socket.c
 * Description: This file use to record pid and socket
 * Author: xishiying@huawei.com
 * Version: 0.1
 * Date:  2014/11/27
 */

#include <net/tcp.h>

#include <linux/sched.h>
#include <linux/rcupdate.h>

#include "pid_socket.h"


void print_process_pid_name(struct inet_sock *inet)
{
	if (NULL == inet || NULL == inet->sk.sk_socket) {
		return;
	}

	int pid = inet->sk.sk_socket->pid;
	unsigned short source_port = inet->inet_sport;

	rcu_read_lock();
	struct task_struct * task=find_task_by_vpid(pid);
	if (NULL == task) {
		rcu_read_unlock();
		return;
	}
	get_task_struct(task);
	rcu_read_unlock();

	source_port = htons(source_port);
	pr_crit("%s: pid:(%d),name:(%s),port:(%d)\n", __func__, pid, task->comm, source_port);
	put_task_struct(task);
}

/*20150114 add,get pid and process name of the app who used connect function.*/