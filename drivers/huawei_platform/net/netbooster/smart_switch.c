
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/memory.h>
#include <linux/uaccess.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/time.h>
#include <linux/kernel.h>/* add for log */
#include <linux/ctype.h>/* add for tolower */
#include <linux/spinlock.h>/* add for spinlock */
#include <linux/netlink.h>/* add for thread */
#include <uapi/linux/netlink.h>/* add for netlink */
#include <linux/kthread.h>/* add for thread */
#include <linux/jiffies.h>/* add for jiffies */
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <linux/skbuff.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/version.h>

#include "smart_switch.h"
#include "nb_netlink.h"
#include "constant.h"

#ifndef DEBUG
#define DEBUG
#endif



/********************************
*	Defines global variables
*********************************/
/*smart switch statistic variable*/
static struct pkt_stat_swth g_stat;

/*Input and output data processing of the lock*/
static spinlock_t g_smart_switch_lock;

/*Result escalation Variable*/
static struct report_slow_para g_swth_report;

/*report time Variable*/
static long g_report_time;

/*RTT filter Variable*/
static unsigned int g_rtt;

/*KSI is enabled identifier*/
static uint8_t g_report_flag;

/*Hook is enabled identifier*/
static uint8_t g_hook_flag;
#ifdef CONFIG_CHR_NETLINK_MODULE
static struct chr_para g_chr_data;
static struct timer_list g_chr_rpt_timer;
static struct report_chr_stru g_chr_report;
#endif



/********************************
*	Function variables
*********************************/
/*check index*/
static int idx_check(int index)
{
	if(index < 0)
		index = index + MAX_STAT_SEC;

	if(index < 0)
		return  0;

	index = index % MAX_STAT_SEC;

	return  index;
}

/*Smart switching CHR*/
#ifdef CONFIG_CHR_NETLINK_MODULE
unsigned int chr_smart_switch(struct chr_para *report)
{
	if (report != NULL && virt_addr_valid(report)) {
		memcpy(report, &g_chr_data, sizeof(g_chr_data));
		memset(&g_chr_data, 0, sizeof(g_chr_data));
	}
	return 0;
}

static void mean_stat_old(void)
{
	int cnt = 0;

	g_stat.idx = idx_check(g_stat.idx);
	g_chr_data.nsi_old = g_stat.norm_idx[g_stat.idx].flt_ksi;
	memset(&(g_chr_data.stat_old), 0, sizeof(struct pkt_cnt_swth));
	for (cnt =0; cnt < MAX_STAT_SEC; cnt++) {
		g_chr_data.stat_old.in_pkt += g_stat.stat[cnt].in_pkt;
		g_chr_data.stat_old.out_pkt += g_stat.stat[cnt].out_pkt;
		g_chr_data.stat_old.in_len += g_stat.stat[cnt].in_len;
		g_chr_data.stat_old.out_len += g_stat.stat[cnt].out_len;
		g_chr_data.stat_old.dupack += g_stat.stat[cnt].dupack;
		g_chr_data.stat_old.rts += g_stat.stat[cnt].rts;
		g_chr_data.stat_old.syn += g_stat.stat[cnt].syn;
		g_chr_data.stat_old.rtt += g_stat.stat[cnt].rtt;
	}
}

static void mean_stat_new(void)
{
	int cnt = 0;

	g_stat.idx = idx_check(g_stat.idx);
	g_chr_data.nsi_new = g_stat.norm_idx[g_stat.idx].flt_ksi;
	memset(&(g_chr_data.stat_new), 0, sizeof(struct pkt_cnt_swth));
	for (cnt = 0; cnt < MAX_STAT_SEC; cnt++) {
		g_chr_data.stat_new.in_pkt += g_stat.stat[cnt].in_pkt;
		g_chr_data.stat_new.out_pkt += g_stat.stat[cnt].out_pkt;
		g_chr_data.stat_new.in_len += g_stat.stat[cnt].in_len;
		g_chr_data.stat_new.out_len += g_stat.stat[cnt].out_len;
		g_chr_data.stat_new.dupack += g_stat.stat[cnt].dupack;
		g_chr_data.stat_new.rts += g_stat.stat[cnt].rts;
		g_chr_data.stat_new.syn += g_stat.stat[cnt].syn;
		g_chr_data.stat_new.rtt += g_stat.stat[cnt].rtt;
	}
}

static int chr_report(unsigned long data)
{
	int rtt = 0;

	g_stat.idx = idx_check(g_stat.idx);
	mean_stat_new();
	g_chr_report.slowType = 3;
	g_chr_report.avgAmp = g_stat.norm_idx[g_stat.idx].flt_ksi;
	rtt = g_chr_data.stat_old.rtt / MAX_STAT_SEC / 10;
	g_chr_report.oldRtt = rtt > MAX_RTT ? MAX_RTT : rtt;
	rtt = g_chr_data.stat_new.rtt / MAX_STAT_SEC / 10;
	g_chr_report.newRtt = rtt > MAX_RTT ? MAX_RTT : rtt;
	nb_notify_event(NBMSG_KSI_EVT, &g_chr_report,
		sizeof(g_chr_report));
	pr_info("KSI chr slow timer report\n");
	return 0;
}

#endif

/*Normalized parameters*/
static int norm_para(const unsigned char norm[],
	const unsigned short idx[], int len, int value)
{
	int cnt = 0;
	int val = 0;
	int min_cnt = 0;
	int min_val;

	min_val = ABS(value - idx[0]);
	for (cnt = 1; cnt < len; cnt++) {
		val = ABS(value - idx[cnt]);
		if (val < min_val) {
			min_cnt = cnt;
			min_val = val;
		}
	}
	return norm[min_cnt];
}

/*Calculate the normalized values of each factor*/
static void calc_norm(unsigned int idx)
{
	unsigned int div = 0;

	idx = idx_check(idx);

	div = g_stat.stat[idx].in_len * MUL128;
	if(g_stat.stat[idx].out_len == 0)
		g_stat.stat[idx].out_len++;

	div = div / g_stat.stat[idx].out_len;
	if (div > MUL128)
		div = MUL128;

	g_stat.norm_idx[idx].inout
		= norm_para(inout_norm, inout_idx, INOUT_LEN, div);

	g_stat.norm_idx[idx].rtt
		= norm_para(rtt_norm, rtt_idx, RTT_LEN, g_stat.stat[idx].rtt);

	g_stat.norm_idx[idx].in_len
		= norm_para(in_norm, in_idx, IN_LEN, g_stat.stat[idx].in_len);

	g_stat.norm_idx[idx].out_len
		= norm_para(out_norm, out_idx, OUT_LEN, g_stat.stat[idx].out_len);

	g_stat.norm_idx[idx].in_pkt
		= norm_para(inpkt_norm, inpkt_idx, INPKT_LEN, g_stat.stat[idx].in_pkt);

	g_stat.norm_idx[idx].out_pkt
		= norm_para(outpkt_norm, outpkt_idx, OUTPKT_LEN, g_stat.stat[idx].out_pkt);

	g_stat.norm_idx[idx].syn
		= norm_para(syn_norm, syn_idx, SYN_LEN, g_stat.stat[idx].syn);

	g_stat.norm_idx[idx].rts
		= norm_para(rts_norm, rts_idx, RTS_LEN, g_stat.stat[idx].rts);

	g_stat.norm_idx[idx].dupack
		= norm_para(dupack_norm, dupack_idx, DUPACK_LEN, g_stat.stat[idx].dupack);
}

/*Calculate KSI values*/
static void calc_ksi(unsigned int idx)
{
	idx = idx_check(idx);

	g_stat.norm_idx[idx].ksi =
		(MUL128 - g_stat.norm_idx[idx].inout) * coef[0] +
		g_stat.norm_idx[idx].rtt * coef[1] +
		(MUL128 - g_stat.norm_idx[idx].in_len) * coef[2] +
		(MUL128 - g_stat.norm_idx[idx].in_pkt) * coef[3] +
		(MUL128 - g_stat.norm_idx[idx].out_len) * coef[4] +
		(MUL128 - g_stat.norm_idx[idx].out_pkt) * coef[5] +
		g_stat.norm_idx[idx].dupack * coef[6] +
		g_stat.norm_idx[idx].rts * coef[7] +
		g_stat.norm_idx[idx].syn * coef[8];

	if (g_stat.norm_idx[idx].ksi > MAX_KSI)
		g_stat.norm_idx[idx].ksi = 0;
}

/*KSI Filter*/
static void filter_ksi(unsigned int idx, unsigned int len)
{
	int cnt = 0;

	idx = idx_check(idx);
	if (len > MAX_STAT_SEC)
		len = MAX_STAT_SEC;

	for (cnt = 0; cnt <= len; cnt++) {
		g_stat.norm_idx[idx_check(idx+cnt)].flt_ksi =
			g_stat.norm_idx[idx_check(idx+cnt-1)].flt_ksi
			- g_stat.norm_idx[idx_check(idx+cnt-1)].flt_ksi / FILT_20
			+ g_stat.norm_idx[idx_check(idx+cnt-1)].ksi / FILT_20;
	}
}

/*Calculate KSI values*/
static void detect_ksi(void)
{
	int cnt = 0;
	int idx_start = -1;
	int idx_dura = -1;
	int idx_tmp = 0;
	int Amp = 0;

	g_stat.idx = idx_check(g_stat.idx);
	if (g_stat.norm_idx[g_stat.idx].flt_ksi > KSI_THRED)
		idx_start = g_stat.idx - MAX_STAT_SEC;

	for (cnt = 1; cnt < MAX_STAT_SEC - 1; cnt++) {
		idx_tmp = idx_check(g_stat.idx - cnt);
		if (g_stat.norm_idx[idx_tmp].flt_ksi <= KSI_THRED &&
			g_stat.norm_idx[idx_check(idx_tmp - 1)].flt_ksi > KSI_THRED) {

			Amp = g_stat.norm_idx[idx_tmp].flt_ksi;
			idx_start = idx_tmp;
			idx_dura = 1;
		}

		if (g_stat.norm_idx[idx_tmp].flt_ksi >= KSI_THRED &&
			g_stat.norm_idx[idx_check(idx_tmp-1)].flt_ksi < KSI_THRED) {

			if (idx_dura < MAX_THRED) {
				Amp = 0;
				idx_start = -1;
				idx_dura = -1;
			} else {
				break;
			}
		}

		if (g_stat.norm_idx[idx_tmp].flt_ksi >= KSI_THRED) {
			Amp += g_stat.norm_idx[idx_tmp].flt_ksi;
			idx_dura++;
		}
	}

	if (idx_dura >= MAX_THRED) {
		memset(&g_swth_report, 0, sizeof(g_swth_report));
		g_swth_report.slowType = 1;
		g_swth_report.avgAmp = Amp / idx_dura / MUL128;
		g_swth_report.duration = idx_dura;
		g_swth_report.timeStart = idx_check(g_stat.idx - idx_start);
		if (before(g_report_time + REPORT_TIMER*HZ, jiffies) && g_report_flag) {

			nb_notify_event(NBMSG_KSI_EVT, &g_swth_report,
				sizeof(g_swth_report));
			pr_info("KSI network slow %d\n", g_swth_report.avgAmp);
			g_report_time = jiffies;
#ifdef CONFIG_CHR_NETLINK_MODULE
			mean_stat_old();
			mod_timer(&g_chr_rpt_timer, jiffies + CHR_REPORT_TIMER);
#endif
		}
	}
}

/*Update KSI values*/
static int ksi_update(void)
{
	int cnt = 0;
	int idx_cur = 0;
	int idx_prv = 0;

	g_stat.idx = idx_check(g_stat.idx);
	calc_norm(g_stat.idx);
	calc_ksi(g_stat.idx);
	if (g_stat.stat[g_stat.idx].rtt < RTT_THRED) {

		filter_ksi(g_stat.idx, 0);
		return -1;
	}

	for (cnt = 0; cnt < EXTERN_TIME; cnt++) {
		idx_cur = idx_check(g_stat.idx - cnt);
		idx_prv = idx_check(g_stat.idx - cnt - 1);
		if (g_stat.stat[idx_prv].rtt == 0) {
			g_stat.stat[idx_prv].rtt = g_stat.stat[g_stat.idx].rtt;
			calc_norm(idx_prv);
			calc_ksi(idx_prv);
		} else {
			filter_ksi(idx_cur, cnt);
			break;
		}
	}
	detect_ksi();
	return 0;
}

/*Statistic Input Package Parameters*/
static void stat_pkt_in(struct tcp_sock *sk,
	struct tcphdr *th, unsigned int len)
{
	if(th == NULL || sk == NULL)
		return;

	g_stat.idx = idx_check(g_stat.idx);
	g_stat.stat[g_stat.idx].in_pkt++;
	g_stat.stat[g_stat.idx].in_len += len;

	if (before(th->seq, sk->rcv_nxt) || before(sk->rcv_nxt+len, th->seq))
		g_stat.stat[g_stat.idx].dupack++;

	/*RT update*/
	g_rtt = g_rtt + (sk->srtt_us) / US_MS / FILT_16 - g_rtt / FILT_16;

	if (g_stat.stat[g_stat.idx].rtt < g_rtt)
		g_stat.stat[g_stat.idx].rtt = g_rtt;
}

/*Statistic Output Package Parameters*/
static void stat_pkt_out(struct tcp_sock *sk,
	struct tcphdr *th, unsigned int len)
{
	if(th == NULL || sk == NULL)
		return;

	g_stat.idx = idx_check(g_stat.idx);
	g_stat.stat[g_stat.idx].out_pkt++;
	g_stat.stat[g_stat.idx].out_len += len;

	if (before(th->seq, sk->snd_nxt) || before(sk->snd_nxt+len, th->seq))
		g_stat.stat[g_stat.idx].rts++;
}

/*Update the index for the statistics array*/
static unsigned int idx_update(void)
{
	unsigned long time_stamp;
	unsigned long time_stamp_tmp;
	unsigned int index;
	unsigned int index_tmp;
	int cnt = 0;

	time_stamp = jiffies / HZ;
	time_stamp_tmp = time_stamp;
	index = time_stamp % MAX_STAT_SEC;
	index_tmp = index;

	if (index != g_stat.idx || time_stamp != g_stat.time_stamp) {

		for (cnt = 1; cnt <= time_stamp - g_stat.time_stamp; cnt++) {

			index_tmp = (g_stat.idx+cnt) % MAX_STAT_SEC;
			time_stamp_tmp = g_stat.time_stamp+cnt;

			memset(&(g_stat.stat[index_tmp]), 0,
				sizeof(struct pkt_cnt_swth));

			if (time_stamp_tmp == time_stamp || cnt >= MAX_STAT_SEC)
				break;
		}
		ksi_update();

		g_stat.idx = index;
		g_stat.time_stamp = time_stamp;

		return S_NEW_SECOND;
	}

	return S_OLD_SECOND;
}

/*Local out hook function*/
static unsigned int hook_out(const struct nf_hook_ops *ops,
		struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	char *pTcpData = NULL;
	u32 totalLen, ipHdrLen, dataLen;

	if (skb == NULL)
		return NF_ACCEPT;

	if (g_hook_flag == S_HOOK_DISABLE)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;

	if (iph->protocol == IPPROTO_TCP) {

		tcph = tcp_hdr(skb);

		if (NULL == tcph || NULL == skb->data || 0 == tcph->doff)
			return NF_ACCEPT;

		pTcpData = (char *)((u32 *)tcph + tcph->doff);
		if (!virt_addr_valid(pTcpData) && !virt_addr_valid(pTcpData + MAX_PKT_LEN))
			return NF_ACCEPT;

		totalLen = skb->len - skb->data_len;
		ipHdrLen = (pTcpData - (char *)iph);
		dataLen = totalLen - ipHdrLen;

		if (totalLen > MAX_PKT_LEN || ipHdrLen > MAX_PKT_LEN || dataLen > MAX_PKT_LEN)
			return NF_ACCEPT;

		if (NULL == skb->dev || NULL == skb->dev->name)
			return NF_ACCEPT;

		if (strncmp(skb->dev->name, DS_NET, DS_NET_LEN))
			return NF_ACCEPT;

		if (skb->sk == NULL)
			return NF_ACCEPT;

		/*When the lock is not locked, the lock is triggered*/
		if (!spin_trylock_bh(&g_smart_switch_lock))
			return NF_ACCEPT;

		if (skb->sk->sk_state == TCP_ESTABLISHED) {
			idx_update();
			stat_pkt_out(skb->sk, tcph, dataLen);
		} else if (skb->sk->sk_state == TCP_SYN_SENT) {
			if (tcph->syn == 1)
				g_stat.stat[g_stat.idx].syn++;
		}

		spin_unlock_bh(&g_smart_switch_lock);
	}
	return NF_ACCEPT;
}

/*Local in hook function*/
static unsigned int hook_in(const struct nf_hook_ops *ops,
		struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	char *pTcpData = NULL;
	u32 totalLen, ipHdrLen, dataLen;

	if (skb == NULL)
		return NF_ACCEPT;

	if (g_hook_flag == S_HOOK_DISABLE)
		return NF_ACCEPT;

	iph = ip_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;

	if (iph->protocol == IPPROTO_TCP) {

		tcph = tcp_hdr(skb);

		if (NULL == tcph || NULL == skb->data || 0 == tcph->doff)
			return NF_ACCEPT;

		pTcpData = (char *)((u32 *)tcph + tcph->doff);
		if (!virt_addr_valid(pTcpData) && !virt_addr_valid(pTcpData + MAX_PKT_LEN))
			return NF_ACCEPT;

		totalLen = skb->len - skb->data_len;
		ipHdrLen = (pTcpData - (char *)iph);
		dataLen = totalLen - ipHdrLen;

		if (totalLen > MAX_PKT_LEN || ipHdrLen > MAX_PKT_LEN || dataLen > MAX_PKT_LEN)
			return NF_ACCEPT;

		if (NULL == skb->dev || NULL == skb->dev->name)
			return NF_ACCEPT;

		if (strncmp(skb->dev->name, DS_NET, DS_NET_LEN))
			return NF_ACCEPT;

		if (skb->sk == NULL)
			return NF_ACCEPT;

		if (skb->sk->sk_state != TCP_ESTABLISHED)
			return NF_ACCEPT;

		if (!spin_trylock_bh(&g_smart_switch_lock))
			return NF_ACCEPT;

		idx_update();
		stat_pkt_in(skb->sk, tcph, dataLen);

		spin_unlock_bh(&g_smart_switch_lock);
	}
	return NF_ACCEPT;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook		= hook_in,
		.pf		= PF_INET,
		.hooknum	= NF_INET_LOCAL_IN,
		.priority	= NF_IP_PRI_FILTER - 1,
	},
	{
		.hook		= hook_out,
		.pf		= PF_INET,
		.hooknum	= NF_INET_POST_ROUTING,
		.priority	= NF_IP_PRI_FILTER - 1,
	}
};

/*Set KSI and hook-and-package points to enable*/
int set_ksi_enable(uint8_t nf_hook_enable, uint8_t nl_event_enable)
{
	spin_lock_bh(&g_smart_switch_lock);
	g_report_flag = nl_event_enable;
	g_hook_flag = nf_hook_enable;
	spin_unlock_bh(&g_smart_switch_lock);

	return 0;
}

/*initialization function*/
int smart_switch_init(void)
{
	int ret = -1;

	g_report_time = jiffies;
	g_report_flag = S_MODULE_DISABLE;
	g_hook_flag = S_HOOK_DISABLE;
	g_rtt = 0;
	memset(&g_stat, 0, sizeof(g_stat));
	memset(&g_swth_report, 0, sizeof(g_swth_report));

	spin_lock_init(&g_smart_switch_lock);
	/*Registration hook function*/
	ret = nf_register_hooks(net_hooks, ARRAY_SIZE(net_hooks));
	if (ret) {
		pr_err("KSI init fail ret=%d\n", ret);
		return ret;
	}

#ifdef CONFIG_CHR_NETLINK_MODULE
	/*Timer initialization*/
	init_timer(&g_chr_rpt_timer);
	g_chr_rpt_timer.data = 0;
	g_chr_rpt_timer.function = chr_report;
	g_chr_rpt_timer.expires = jiffies + CHR_REPORT_TIMER;
	memset(&g_chr_report, 0, sizeof(g_chr_report));
#endif

	pr_info("KSI init success\n");
	return 0;
}

/*Exit function*/
void smart_switch_exit(void)
{
	nf_unregister_hooks(net_hooks, ARRAY_SIZE(net_hooks));
	pr_err("KSI exit success\n");
}

#undef DEBUG