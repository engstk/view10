/*
 * fs/f2fs/gc.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/backing-dev.h>
#include <linux/init.h>
#include <linux/f2fs_fs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/timer.h>
#include <linux/blkdev.h>

#include "f2fs.h"
#include "node.h"
#include "segment.h"
#include "gc.h"
#include <trace/events/f2fs.h>

#define IDLE_WT 1000
#define MIN_WT 100
#define DEF_GC_BALANCE_MIN_SLEEP_TIME	10000	/* milliseconds */

/*
 * GC tuning ratio [0, 100] in performance mode
 */
static inline int gc_perf_ratio(struct f2fs_sb_info *sbi)
{
	block_t reclaimable_user_blocks = sbi->user_block_count -
						written_block_count(sbi);
	return reclaimable_user_blocks == 0 ? 100 :
			100ULL * free_user_blocks(sbi) / reclaimable_user_blocks;
}

static inline int __gc_thread_wait_timeout(struct f2fs_sb_info *sbi, struct f2fs_gc_kthread *gc_th,
	int timeout)
{
	wait_queue_head_t *wq = &gc_th->gc_wait_queue_head;
	wait_queue_head_t *fg_gc_wait = &gc_th->fg_gc_wait;

	return wait_event_interruptible_timeout(*wq,
		freezing(current) ||
		kthread_should_stop() || waitqueue_active(fg_gc_wait) || atomic_read(&sbi->need_ssr_gc),
		timeout);
}

static int gc_thread_func(void *data)
{
	struct f2fs_sb_info *sbi = data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	long wait_ms;

	wait_ms = gc_th->min_sleep_time;

	current->flags |= PF_MUTEX_GC;

	set_freezable();
	do {
		/*lint -save -e574 -e666 */
		int ret;

		if (100 * written_block_count(sbi) / sbi->user_block_count > 90)
			gc_th->gc_preference = GC_LIFETIME;
		else if (gc_perf_ratio(sbi) < 10 && free_segments(sbi) <
						3 * overprovision_segments(sbi))
			gc_th->gc_preference = GC_PERF;
		else
			gc_th->gc_preference = GC_BALANCE;

		if (gc_th->gc_preference == GC_PERF)
			wait_ms = max(DEF_GC_BALANCE_MIN_SLEEP_TIME *
					gc_perf_ratio(sbi) / 100, MIN_WT);
		else if (gc_th->gc_preference == GC_BALANCE)
			gc_th->min_sleep_time = DEF_GC_BALANCE_MIN_SLEEP_TIME;
		else
			gc_th->min_sleep_time = DEF_GC_THREAD_MIN_SLEEP_TIME;
		/*lint -restore*/

#ifdef CONFIG_F2FS_FAULT_INJECTION
		if (time_to_inject(sbi, FAULT_CHECKPOINT)) {
			f2fs_show_injection_info(FAULT_CHECKPOINT);
			f2fs_stop_checkpoint(sbi, false);
		}
#endif

		/*lint -save -e454 -e456 -e666*/
		if (!(ret = __gc_thread_wait_timeout(sbi, gc_th,
			msecs_to_jiffies(wait_ms)))) {
			if (sbi->sb->s_writers.frozen >= SB_FREEZE_WRITE) {
				increase_sleep_time(gc_th, &wait_ms);
				continue;
			}

			if (!mutex_trylock(&sbi->gc_mutex))
				continue;

			/* time runs out - must be background GC */
		} else if (try_to_freeze()) {
			continue;
		} else if (kthread_should_stop()) {
			break;
		} else if (ret < 0) {
			pr_err("f2fs-gc: some signals have been received...\n");
			continue;
		} else {
			int ssr_gc_count;
			ssr_gc_count = atomic_read(&sbi->need_ssr_gc);
			if (ssr_gc_count) {
				mutex_lock(&sbi->gc_mutex);
				f2fs_gc(sbi, true, false);
				atomic_sub(ssr_gc_count, &sbi->need_ssr_gc);
			}
			if (!has_not_enough_free_secs(sbi, 0, 0)) {
				wake_up_all(&gc_th->fg_gc_wait);
				continue;
			}

			/* run into FG_GC
			   we must wait & take sbi->gc_mutex before FG_GC */
			mutex_lock(&sbi->gc_mutex);

			f2fs_gc(sbi, false, false);
			wake_up_all(&gc_th->fg_gc_wait);
			continue;
		}

		/*
		 * [GC triggering condition]
		 * 0. GC is not conducted currently.
		 * 1. There are enough dirty segments.
		 * 2. IO subsystem is idle by checking the # of writeback pages.
		 * 3. IO subsystem is idle by checking the # of requests in
		 *    bdev's request list.
		 *
		 * Note) We have to avoid triggering GCs frequently.
		 * Because it is possible that some segments can be
		 * invalidated soon after by user update or deletion.
		 * So, I'd like to wait some time to collect dirty segments.
		 */

#ifdef CONFIG_HISI_BLK_CORE
		if (!gc_th->block_idle) {
#else
		if (!is_idle(sbi)) {
#endif
			increase_sleep_time(gc_th, &wait_ms);
			/*lint -save -e455*/
			mutex_unlock(&sbi->gc_mutex);
			/*lint -restore*/
			continue;
		}


		if (has_enough_invalid_blocks(sbi))
			decrease_sleep_time(gc_th, &wait_ms);
		else
			increase_sleep_time(gc_th, &wait_ms);

		stat_inc_bggc_count(sbi);

#ifdef CONFIG_F2FS_STAT_FS
	{
		static DEFINE_RATELIMIT_STATE(bg_gc_rs, F2FS_GC_DSM_INTERVAL, F2FS_GC_DSM_BURST);
		if (unlikely(__ratelimit(&bg_gc_rs))) {
			f2fs_msg(sbi->sb, KERN_NOTICE,
				"BG_GC: Size=%lluMB,Free=%lluMB,count=%d,free_sec=%u,reserved_sec=%u,node_secs=%d,dent_secs=%d\n",
				(le64_to_cpu(sbi->user_block_count) * sbi->blocksize) / 1024 / 1024,
				(le64_to_cpu(sbi->user_block_count - valid_user_blocks(sbi)) * sbi->blocksize) / 1024 / 1024,
				sbi->bg_gc, free_sections(sbi), reserved_sections(sbi),
				get_blocktype_secs(sbi, F2FS_DIRTY_NODES), get_blocktype_secs(sbi, F2FS_DIRTY_DENTS));
		}
	}
#endif

		/* if return value is not zero, no victim was selected */
		if (f2fs_gc(sbi, test_opt(sbi, FORCE_FG_GC), true))
			wait_ms = gc_th->no_gc_sleep_time;

		trace_f2fs_background_gc(sbi->sb, wait_ms,
				prefree_segments(sbi), free_segments(sbi));

		/* balancing f2fs's metadata periodically */
		f2fs_balance_fs_bg(sbi);

		/*lint -restore*/
	} while (!kthread_should_stop());
	return 0;
}

#ifdef CONFIG_HISI_BLK_CORE
static void set_block_idle(unsigned long data)
{
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	gc_th->block_idle = true;
}

static enum blk_busy_idle_callback_return gc_io_busy_idle_notify_handler(struct blk_busy_idle_nb *nb,
											enum blk_idle_notify_state state)
{
	enum blk_busy_idle_callback_return ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
	struct f2fs_sb_info *sbi = (struct f2fs_sb_info *)nb->param_data;
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;

	if (gc_th->f2fs_gc_task == NULL)
		return ret;
	switch(state) {

	case BLK_IDLE_NOTIFY:
		mod_timer(&gc_th->nb_timer, jiffies + msecs_to_jiffies(IDLE_WT));
		ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
		break;
	case BLK_BUSY_NOTIFY:
		del_timer_sync(&gc_th->nb_timer);
		gc_th->block_idle = false;
		ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
		break;
	}

	return ret;
}
#endif

int start_gc_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	struct task_struct *f2fs_gc_task;
	dev_t dev = sbi->sb->s_bdev->bd_dev;
	int err = 0;

	ACCESS_ONCE(gc_th->f2fs_gc_task) = NULL;

	gc_th->min_sleep_time = DEF_GC_BALANCE_MIN_SLEEP_TIME;
	gc_th->max_sleep_time = DEF_GC_THREAD_MAX_SLEEP_TIME;
	gc_th->no_gc_sleep_time = DEF_GC_THREAD_NOGC_SLEEP_TIME;

	gc_th->gc_idle = 0;
	gc_th->gc_preference = GC_BALANCE;

#ifdef CONFIG_HISI_BLK_CORE
	gc_th->block_idle = false;
	gc_th->gc_nb.subscriber_name = "f2fs_gc";
	gc_th->gc_nb.blk_busy_idle_notifier_callback = gc_io_busy_idle_notify_handler;
	gc_th->gc_nb.param_data = sbi;
	err = blk_busy_idle_event_subscriber(sbi->sb->s_bdev, &gc_th->gc_nb);
	setup_timer(&gc_th->nb_timer, set_block_idle, (unsigned long)sbi);
#endif

	init_waitqueue_head(&gc_th->gc_wait_queue_head);
	init_waitqueue_head(&gc_th->fg_gc_wait);
	f2fs_gc_task = kthread_run(gc_thread_func, sbi,
		"f2fs_gc-%u:%u", MAJOR(dev), MINOR(dev));
	if (IS_ERR(f2fs_gc_task))
		err = PTR_ERR(f2fs_gc_task);
	else
		ACCESS_ONCE(gc_th->f2fs_gc_task) = f2fs_gc_task;
	return err;
}

void stop_gc_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	if (gc_th->f2fs_gc_task != NULL) {
		kthread_stop(gc_th->f2fs_gc_task);
		ACCESS_ONCE(gc_th->f2fs_gc_task) = NULL;
		wake_up_all(&gc_th->fg_gc_wait);
#ifdef CONFIG_HISI_BLK_CORE
		del_timer_sync(&gc_th->nb_timer);
retry:
		if (blk_busy_idle_event_unsubscriber(sbi->sb->s_bdev, &gc_th->gc_nb))
			goto retry;
#endif

	}
}

static int select_gc_type(struct f2fs_gc_kthread *gc_th, int gc_type)
{
	int gc_mode = (gc_type == BG_GC) ? GC_CB : GC_GREEDY;

	if (gc_th && gc_th->gc_idle) {
		if (gc_th->gc_idle == 1)
			gc_mode = GC_CB;
		else if (gc_th->gc_idle == 2)
			gc_mode = GC_GREEDY;
	}
	return gc_mode;
}

static void select_policy(struct f2fs_sb_info *sbi, int gc_type,
			int type, struct victim_sel_policy *p)
{
	struct dirty_seglist_info *dirty_i = DIRTY_I(sbi);

	if (p->alloc_mode == SSR) {
		p->gc_mode = GC_GREEDY;
		p->dirty_segmap = dirty_i->dirty_segmap[type];
		p->max_search = dirty_i->nr_dirty[type];
		p->ofs_unit = 1;
	} else {
		p->gc_mode = select_gc_type(&sbi->gc_thread, gc_type);
		p->dirty_segmap = dirty_i->dirty_segmap[DIRTY];
		p->max_search = dirty_i->nr_dirty[DIRTY];
		p->ofs_unit = sbi->segs_per_sec;
	}

	/* we need to check every dirty segments in the FG_GC case */
	if (gc_type != FG_GC && p->max_search > sbi->max_victim_search)
		p->max_search = sbi->max_victim_search;

	p->offset = sbi->last_victim[p->gc_mode];
}

static unsigned int get_max_cost(struct f2fs_sb_info *sbi,
				struct victim_sel_policy *p)
{
	/* SSR allocates in a segment unit */
	if (p->alloc_mode == SSR)
		return sbi->blocks_per_seg;
	if (p->gc_mode == GC_GREEDY)
		return 2 * sbi->blocks_per_seg * p->ofs_unit;
	else if (p->gc_mode == GC_CB)
		return UINT_MAX;
	else /* No other gc_mode */
		return 0;
}

static unsigned int check_bg_victims(struct f2fs_sb_info *sbi)
{
	struct dirty_seglist_info *dirty_i = DIRTY_I(sbi);
	unsigned int secno;

	/*
	 * If the gc_type is FG_GC, we can select victim segments
	 * selected by background GC before.
	 * Those segments guarantee they have small valid blocks.
	 */
	for_each_set_bit(secno, dirty_i->victim_secmap, MAIN_SECS(sbi)) {
		if (sec_usage_check(sbi, secno))
			continue;

		if (no_fggc_candidate(sbi, secno))
			continue;

		if (sbi->gc_loop.segmap &&
			test_bit(secno * sbi->segs_per_sec, sbi->gc_loop.segmap))
			continue;

		clear_bit(secno, dirty_i->victim_secmap);
		return secno * sbi->segs_per_sec;
	}
	return NULL_SEGNO;
}

static unsigned int get_cb_cost(struct f2fs_sb_info *sbi, unsigned int segno,
				unsigned int *valid_blocks)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct f2fs_gc_kthread *gc_th = &sbi->gc_thread;
	unsigned int secno = GET_SECNO(sbi, segno);
	unsigned int start = secno * sbi->segs_per_sec;
	unsigned long long mtime = 0;
	unsigned int vblocks;
	unsigned int max_age;
	unsigned char age = 0;
	unsigned char u;
	unsigned int i;

	for (i = 0; i < sbi->segs_per_sec; i++)
		mtime += get_seg_entry(sbi, start + i)->mtime;
	vblocks = get_valid_blocks(sbi, segno, sbi->segs_per_sec);
	*valid_blocks = vblocks;

	mtime = div_u64(mtime, sbi->segs_per_sec);
	vblocks = div_u64(vblocks, sbi->segs_per_sec);

	u = (vblocks * 100) >> sbi->log_blocks_per_seg;

	/* Handle if the system time has changed by the user */
	if (mtime < sit_i->min_mtime)
		sit_i->min_mtime = mtime;
	if (mtime > sit_i->max_mtime)
		sit_i->max_mtime = mtime;
	/*lint -save -e613 -e666 */
	/* Reduce the cost weight of age when free blocks less than 10% */
	max_age = (gc_th && gc_th->gc_preference != GC_LIFETIME &&
		gc_perf_ratio(sbi) < 10) ? max(10 * gc_perf_ratio(sbi), 1) : 100;
	/*lint -restore*/
	if (sit_i->max_mtime != sit_i->min_mtime)
		age = max_age - div64_u64(max_age * (mtime - sit_i->min_mtime),
				sit_i->max_mtime - sit_i->min_mtime);

	return UINT_MAX - ((100 * (100 - u) * age) / (100 + u));
}

static unsigned int get_greedy_cost(struct f2fs_sb_info *sbi,
						unsigned int segno)
{
	unsigned int valid_blocks =
			get_valid_blocks(sbi, segno, sbi->segs_per_sec);

	return IS_DATASEG(get_seg_entry(sbi, segno)->type) ?
				valid_blocks * 2 : valid_blocks;
}

static inline unsigned int get_gc_cost(struct f2fs_sb_info *sbi,
			unsigned int segno, struct victim_sel_policy *p,
			unsigned int *vblocks)
{
	if (p->alloc_mode == SSR)
		return get_seg_entry(sbi, segno)->ckpt_valid_blocks;

	/* alloc_mode == LFS */
	if (p->gc_mode == GC_GREEDY)
		return get_greedy_cost(sbi, segno);
	else
		return get_cb_cost(sbi, segno, vblocks);
}

static unsigned int count_bits(const unsigned long *addr,
				unsigned int offset, unsigned int len)
{
	unsigned int end = offset + len, sum = 0;

	while (offset < end) {
		if (test_bit(offset++, addr))
			++sum;
	}
	return sum;
}

/*
 * This function is called from two paths.
 * One is garbage collection and the other is SSR segment selection.
 * When it is called during GC, it just gets a victim segment
 * and it does not remove it from dirty seglist.
 * When it is called from SSR segment selection, it finds a segment
 * which has minimum valid blocks and removes it from dirty seglist.
 */
static int get_victim_by_default(struct f2fs_sb_info *sbi,
		unsigned int *result, int gc_type, int type, char alloc_mode)
{
	struct dirty_seglist_info *dirty_i = DIRTY_I(sbi);
	struct victim_sel_policy p;
	unsigned int secno, last_victim;
	unsigned int last_segment = MAIN_SEGS(sbi);
	unsigned int nsearched = 0;
#ifdef CONFIG_F2FS_REMAP_GC
	bool for_remap = false;
	unsigned int remap_min_segno = NULL_SEGNO;
	unsigned int remap_min_cost = UINT_MAX;
#endif
	unsigned int vblocks;

	mutex_lock(&dirty_i->seglist_lock);

	p.alloc_mode = alloc_mode;
	select_policy(sbi, gc_type, type, &p);

	p.min_segno = NULL_SEGNO;
	p.min_cost = get_max_cost(sbi, &p);

	if (p.max_search == 0)
		goto out;

	last_victim = sbi->last_victim[p.gc_mode];
	if (p.alloc_mode == LFS && gc_type == FG_GC) {
		p.min_segno = check_bg_victims(sbi);
		if (p.min_segno != NULL_SEGNO)
			goto got_it;
	}

#ifdef CONFIG_F2FS_REMAP_GC
	if (f2fs_remap_gc_on(sbi) &&
	    atomic_read(&F2FS_RMPGC(sbi)->rmpgc_cblks) == 0)
		for_remap = true;
#endif

	while (1) {
		unsigned long cost;
		unsigned int segno;

		segno = find_next_bit(p.dirty_segmap, last_segment, p.offset);
		if (segno >= last_segment) {
			if (sbi->last_victim[p.gc_mode]) {
				last_segment = sbi->last_victim[p.gc_mode];
				sbi->last_victim[p.gc_mode] = 0;
				p.offset = 0;
				continue;
			}
			break;
		}

		p.offset = segno + p.ofs_unit;
		if (p.ofs_unit > 1) {
			p.offset -= segno % p.ofs_unit;
			nsearched += count_bits(p.dirty_segmap,
						p.offset - p.ofs_unit,
						p.ofs_unit);
		} else {
			nsearched++;
		}

		secno = GET_SECNO(sbi, segno);

		if (sec_usage_check(sbi, secno))
			goto next;
		if (gc_type == BG_GC && test_bit(secno, dirty_i->victim_secmap))
			goto next;
		if (gc_type == FG_GC && p.alloc_mode == LFS &&
					no_fggc_candidate(sbi, secno))
			goto next;
		if (gc_type == FG_GC && p.alloc_mode == LFS &&
			sbi->gc_loop.segmap && test_bit(segno, sbi->gc_loop.segmap))
			goto next;

		vblocks = 0;
		cost = get_gc_cost(sbi, segno, &p, &vblocks);

		if (p.min_cost > cost) {
			p.min_segno = segno;
			p.min_cost = cost;
		}

#ifdef CONFIG_F2FS_REMAP_GC
		if (for_remap && vblocks > F2FS_RMPGC_VBLK_WATERLINE)
			if (remap_min_cost > cost) {
				remap_min_segno = segno;
				remap_min_cost = cost;
			}
#endif
next:
		if (nsearched >= p.max_search) {
			if (!sbi->last_victim[p.gc_mode] && segno <= last_victim)
				sbi->last_victim[p.gc_mode] = last_victim + 1;
			else
				sbi->last_victim[p.gc_mode] = segno + 1;
			break;
		}
	}

#ifdef CONFIG_F2FS_REMAP_GC
	if (remap_min_segno != NULL_SEGNO)
		p.min_segno = remap_min_segno;
#endif

	if (p.min_segno != NULL_SEGNO) {
got_it:
		if (p.alloc_mode == LFS) {
			secno = GET_SECNO(sbi, p.min_segno);
			if (gc_type == FG_GC)
				sbi->cur_victim_sec = secno;
			else
				set_bit(secno, dirty_i->victim_secmap);
		}
		*result = (p.min_segno / p.ofs_unit) * p.ofs_unit;

		trace_f2fs_get_victim(sbi->sb, type, gc_type, &p,
				sbi->cur_victim_sec,
				prefree_segments(sbi), free_segments(sbi));
	}
out:
	mutex_unlock(&dirty_i->seglist_lock);

	return (p.min_segno == NULL_SEGNO) ? 0 : 1;
}

static const struct victim_selection default_v_ops = {
	.get_victim = get_victim_by_default,
};

static struct inode *find_gc_inode(struct gc_inode_list *gc_list, nid_t ino)
{
	struct inode_entry *ie;

	ie = radix_tree_lookup(&gc_list->iroot, ino);
	if (ie)
		return ie->inode;
	return NULL;
}

static void add_gc_inode(struct gc_inode_list *gc_list, struct inode *inode)
{
	struct inode_entry *new_ie;

	if (inode == find_gc_inode(gc_list, inode->i_ino)) {
		iput(inode);
		return;
	}
	new_ie = f2fs_kmem_cache_alloc(inode_entry_slab, GFP_NOFS);
	new_ie->inode = inode;

	f2fs_radix_tree_insert(&gc_list->iroot, inode->i_ino, new_ie);
	list_add_tail(&new_ie->list, &gc_list->ilist);
}

static void put_gc_inode(struct gc_inode_list *gc_list)
{
	struct inode_entry *ie, *next_ie;
	list_for_each_entry_safe(ie, next_ie, &gc_list->ilist, list) {
		radix_tree_delete(&gc_list->iroot, ie->inode->i_ino);
		iput(ie->inode);
		list_del(&ie->list);
		kmem_cache_free(inode_entry_slab, ie);
	}
}

static int check_valid_map(struct f2fs_sb_info *sbi,
				unsigned int segno, int offset)
{
	struct sit_info *sit_i = SIT_I(sbi);
	struct seg_entry *sentry;
	int ret;

	mutex_lock(&sit_i->sentry_lock);
	sentry = get_seg_entry(sbi, segno);
	ret = f2fs_test_bit(offset, sentry->cur_valid_map);
	mutex_unlock(&sit_i->sentry_lock);
	return ret;
}

/*
 * This function compares node address got in summary with that in NAT.
 * On validity, copy that node with cold status, otherwise (invalid node)
 * ignore that.
 */
static void gc_node_segment(struct f2fs_sb_info *sbi,
		struct f2fs_summary *sum, unsigned int segno, int gc_type,
		bool remap_gc)
{
	struct f2fs_summary *entry;
	block_t start_addr;
	int off;
	int phase = 0, gc_cnt = 0;

	start_addr = START_BLOCK(sbi, segno);

next_step:
	entry = sum;

	for (off = 0; off < sbi->blocks_per_seg; off++, entry++) {
		nid_t nid = le32_to_cpu(entry->nid);
		struct page *node_page;
		struct node_info ni;

		/* stop BG_GC if there is not enough free sections. */
		if (gc_type == BG_GC && has_not_enough_free_secs(sbi, 0, 0)) {
			bd_mutex_lock(&sbi->bd_mutex);
			inc_bd_array_val(sbi, gc_node_blk_cnt, gc_type, gc_cnt);
			bd_mutex_unlock(&sbi->bd_mutex);
			return;
		}

		if (check_valid_map(sbi, segno, off) == 0)
			continue;

		if (phase == 0) {
			ra_meta_pages(sbi, NAT_BLOCK_OFFSET(nid), 1,
							META_NAT, true);
			continue;
		}

		if (phase == 1) {
			ra_node_page(sbi, nid);
			continue;
		}

		/* phase == 2 */
		node_page = get_node_page(sbi, nid);
		if (IS_ERR(node_page)) {
			f2fs_gc_loop_debug(sbi);
			continue;
		}

		/* block may become invalid during get_node_page */
		if (check_valid_map(sbi, segno, off) == 0) {
			f2fs_put_page(node_page, 1);
			continue;
		}

		get_node_info(sbi, nid, &ni);
		if (ni.blk_addr != start_addr + off) {
			f2fs_put_page(node_page, 1);
			f2fs_gc_loop_debug(sbi);
			continue;
		}

		if (move_node_page(node_page, gc_type, remap_gc) == 0)
			gc_cnt++;
		stat_inc_node_blk_count(sbi, 1, gc_type);
	}

	if (++phase < 3)
		goto next_step;

	bd_mutex_lock(&sbi->bd_mutex);
	inc_bd_array_val(sbi, gc_node_blk_cnt, gc_type, gc_cnt);
	bd_mutex_unlock(&sbi->bd_mutex);
}

/*
 * Calculate start block index indicating the given node offset.
 * Be careful, caller should give this node offset only indicating direct node
 * blocks. If any node offsets, which point the other types of node blocks such
 * as indirect or double indirect node blocks, are given, it must be a caller's
 * bug.
 */
block_t start_bidx_of_node(unsigned int node_ofs, struct inode *inode)
{
	unsigned int indirect_blks = 2 * NIDS_PER_BLOCK + 4;
	unsigned int bidx;

	if (node_ofs == 0)
		return 0;

	if (node_ofs <= 2) {
		bidx = node_ofs - 1;
	} else if (node_ofs <= indirect_blks) {
		int dec = (node_ofs - 4) / (NIDS_PER_BLOCK + 1);
		bidx = node_ofs - 2 - dec;
	} else {
		int dec = (node_ofs - indirect_blks - 3) / (NIDS_PER_BLOCK + 1);
		bidx = node_ofs - 5 - dec;
	}
	return bidx * ADDRS_PER_BLOCK + ADDRS_PER_INODE(inode);
}

static bool is_alive(struct f2fs_sb_info *sbi, struct f2fs_summary *sum,
		struct node_info *dni, block_t blkaddr, unsigned int *nofs)
{
	struct page *node_page;
	nid_t nid;
	unsigned int ofs_in_node;
	block_t source_blkaddr;

	nid = le32_to_cpu(sum->nid);
	ofs_in_node = le16_to_cpu(sum->ofs_in_node);

	node_page = get_node_page(sbi, nid);
	if (IS_ERR(node_page))
		return false;

	get_node_info(sbi, nid, dni);

	if (sum->version != dni->version) {
		f2fs_put_page(node_page, 1);
		return false;
	}

	*nofs = ofs_of_node(node_page);
	source_blkaddr = datablock_addr(node_page, ofs_in_node);
	f2fs_put_page(node_page, 1);

	if (source_blkaddr != blkaddr)
		return false;
	return true;
}

static int move_encrypted_block(struct inode *inode, block_t bidx,
				unsigned int segno, int off, bool remap_gc)
{
	struct f2fs_io_info fio = {
		.sbi = F2FS_I_SB(inode),
		.type = DATA,
		.op = REQ_OP_READ,
		.op_flags = REQ_SYNC,
		.encrypted_page = NULL,
#ifdef CONFIG_F2FS_HOTCOLD_DRIVER
		.sp_type = SP_UNSET,
#endif
	};
	struct dnode_of_data dn;
	struct f2fs_summary sum;
	struct node_info ni;
	struct page *page;
	block_t newaddr;
	int err, ret = -1;

	/* do not read out */
	page = f2fs_grab_cache_page(inode->i_mapping, bidx, false);
	if (!page) {
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		return ret;
	}

	if (!check_valid_map(F2FS_I_SB(inode), segno, off))
		goto out;

	if (f2fs_is_atomic_file(inode)) {
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto out;
	}

	set_new_dnode(&dn, inode, NULL, NULL, 0);
	err = get_dnode_of_data(&dn, bidx, LOOKUP_NODE);
	if (err) {
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto out;
	}

	if (unlikely(dn.data_blkaddr == NULL_ADDR)) {
		ClearPageUptodate(page);
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_out;
	}

	if (unlikely(dn.data_blkaddr == NEW_ADDR)) {
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_out;
	}

	/*
	 * don't cache encrypted data into meta inode until previous dirty
	 * data were writebacked to avoid racing between GC and flush.
	 */
	f2fs_wait_on_page_writeback(page, DATA, true);

	get_node_info(fio.sbi, dn.nid, &ni);
	set_summary(&sum, dn.nid, dn.ofs_in_node, ni.version);

	/* read page */
	fio.page = page;
	fio.new_blkaddr = fio.old_blkaddr = dn.data_blkaddr;

	allocate_data_block(fio.sbi, NULL, fio.old_blkaddr, &newaddr,
							&sum, CURSEG_COLD_DATA);

	fio.encrypted_page = pagecache_get_page(META_MAPPING(fio.sbi), newaddr,
					FGP_LOCK | FGP_CREAT, GFP_NOFS);
	if (!fio.encrypted_page) {
		err = -ENOMEM;
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto recover_block;
	}

	f2fs_io_set_remap_gc_check(&fio, false);
	err = f2fs_submit_page_bio(&fio);
	if (err) {
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_page_out;
	}
	f2fs_io_clear_remap_gc_check(&fio);

	/* write page */
	lock_page(fio.encrypted_page);

	if (unlikely(fio.encrypted_page->mapping != META_MAPPING(fio.sbi))) {
		err = -EIO;
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_page_out;
	}
	if (unlikely(!PageUptodate(fio.encrypted_page))) {
		err = -EIO;
		f2fs_gc_loop_debug(F2FS_I_SB(inode)); /*lint !e666*/
		goto put_page_out;
	}

	set_page_dirty(fio.encrypted_page);
	f2fs_wait_on_page_writeback(fio.encrypted_page, DATA, true);
	if (clear_page_dirty_for_io(fio.encrypted_page))
		dec_page_count(fio.sbi, F2FS_DIRTY_META);

	/* allocate block address */
	f2fs_wait_on_page_writeback(dn.node_page, NODE, true);

	if (remap_gc) {
		/*
		 * fix me: we only need allocate block address in this routine,
		 * reading page can be removed.
		 */
		f2fs_gc_add_remap_entry(fio.sbi, fio.old_blkaddr, newaddr);
	} else {
		set_page_writeback(fio.encrypted_page);

		fio.op = REQ_OP_WRITE;
		fio.op_flags = REQ_SYNC | REQ_NOIDLE;
		fio.new_blkaddr = newaddr;
#ifdef CONFIG_F2FS_HOTCOLD_DRIVER
		fio.sp_type = SP_COLD_DATA,
#endif
		f2fs_submit_page_mbio(&fio);

		/*
		 * If writing page fails, cp will be stopped, and last cp
		 * will be used, so removing remap here will not lead to
		 * corruption.
		 */
		f2fs_gc_remove_remap_entry(fio.sbi, fio.old_blkaddr);
	}

	f2fs_update_data_blkaddr(&dn, newaddr);
	set_inode_flag(inode, FI_APPEND_WRITE);
	if (page->index == 0)
		set_inode_flag(inode, FI_FIRST_BLOCK_WRITTEN);
	ret = 0;
put_page_out:
	f2fs_put_page(fio.encrypted_page, 1);
recover_block:
	if (err)
		__f2fs_replace_block(fio.sbi, &sum, newaddr, fio.old_blkaddr,
								true, true);
put_out:
	f2fs_put_dnode(&dn);
out:
	f2fs_put_page(page, 1);
	return ret;
}

static int move_data_page(struct inode *inode, block_t bidx, int gc_type,
			  unsigned int segno, int off, bool remap_gc)
{
	struct page *page;
	int ret = -1;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	page = get_lock_data_page(inode, bidx, true);
	if (IS_ERR(page)) {
		f2fs_gc_loop_debug(sbi);
		return ret;
	}

	if (!check_valid_map(sbi, segno, off))
		goto out;

	if (f2fs_is_atomic_file(inode)) {
		f2fs_gc_loop_debug(sbi);
		goto out;
	}

	if (gc_type == BG_GC && !remap_gc) {
		ret = 0;
		if (PageWriteback(page)) {
			f2fs_gc_loop_debug(sbi);
			goto out;
		}
		set_page_dirty(page);
		set_cold_data(page);
	} else {
		/*lint -save -e446*/
		struct f2fs_io_info fio = {
			.sbi = F2FS_I_SB(inode),
			.type = DATA,
			.op = REQ_OP_WRITE,
			.op_flags = REQ_SYNC | REQ_NOIDLE,
			.page = page,
			.encrypted_page = NULL,
#ifdef CONFIG_F2FS_HOTCOLD_DRIVER
			.sp_type = SP_UNSET,
#endif
		};
		/*lint -restore*/
		bool is_dirty = PageDirty(page);
		int err;
		unsigned long cnt = 0;

		f2fs_set_remap_gc_write(&fio, remap_gc);
retry:
		set_page_dirty(page);
		f2fs_wait_on_page_writeback(page, DATA, true);
		if (clear_page_dirty_for_io(page)) {
			inode_dec_dirty_pages(inode);
			remove_dirty_inode(inode);
		}

		set_cold_data(page);

		err = do_write_data_page(&fio);
		if (err == -ENOMEM && is_dirty) {
			cnt++;
			if (!(cnt % F2FS_GC_LOOP_NOMEM_MOD))
				 f2fs_msg(sbi->sb, KERN_ERR,
				 "f2fs_gc_loop nomem retry:%lu in %s:%d\n",
				 cnt, __func__, __LINE__);
			congestion_wait(BLK_RW_ASYNC, HZ/50);
			goto retry;
		}
		if (!err)
			ret = 0;
		else
			f2fs_gc_loop_debug(sbi);

		if (!ret && remap_gc)
			f2fs_gc_add_remap_entry(sbi,
					fio.old_blkaddr,
					fio.new_blkaddr);
	}
out:
	f2fs_put_page(page, 1);
	return ret;
}

/*
 * This function tries to get parent node of victim data block, and identifies
 * data block validity. If the block is valid, copy that with cold status and
 * modify parent node.
 * If the parent node is not valid or the data block address is different,
 * the victim data block is ignored.
 */
static void gc_data_segment(struct f2fs_sb_info *sbi, struct f2fs_summary *sum,
		struct gc_inode_list *gc_list, unsigned int segno, int gc_type,
		bool remap_gc)
{
	struct super_block *sb = sbi->sb;
	struct f2fs_summary *entry;
	block_t start_addr;
	int off;
	int phase = 0, gc_cnt = 0;

	start_addr = START_BLOCK(sbi, segno);

next_step:
	entry = sum;

	for (off = 0; off < sbi->blocks_per_seg; off++, entry++) {
		struct page *data_page;
		struct inode *inode;
		struct node_info dni; /* dnode info for the data */
		unsigned int ofs_in_node, nofs;
		block_t start_bidx;
		nid_t nid = le32_to_cpu(entry->nid);

		/* stop BG_GC if there is not enough free sections. */
		if (gc_type == BG_GC && has_not_enough_free_secs(sbi, 0, 0)) {
			bd_mutex_lock(&sbi->bd_mutex);
			inc_bd_array_val(sbi, gc_data_blk_cnt, gc_type, gc_cnt);
			inc_bd_array_val(sbi, hotcold_cnt, HC_GC_COLD_DATA, gc_cnt);
			bd_mutex_unlock(&sbi->bd_mutex);
			return;
		}

		if (check_valid_map(sbi, segno, off) == 0)
			continue;

		if (phase == 0) {
			ra_meta_pages(sbi, NAT_BLOCK_OFFSET(nid), 1,
							META_NAT, true);
			continue;
		}

		if (phase == 1) {
			ra_node_page(sbi, nid);
			continue;
		}

		/* Get an inode by ino with checking validity */
		if (!is_alive(sbi, entry, &dni, start_addr + off, &nofs)) {
			f2fs_gc_loop_debug(sbi);
			continue;
		}

		if (phase == 2) {
			ra_node_page(sbi, dni.ino);
			continue;
		}

		ofs_in_node = le16_to_cpu(entry->ofs_in_node);

		if (phase == 3) {
			inode = f2fs_iget(sb, dni.ino);
			if (IS_ERR(inode) || is_bad_inode(inode)) {
				f2fs_gc_loop_debug(sbi);
				continue;
			}

			/* if encrypted inode, let's go phase 3 */
			if (f2fs_encrypted_inode(inode) &&
						S_ISREG(inode->i_mode)) {
				add_gc_inode(gc_list, inode);
				continue;
			}

			start_bidx = start_bidx_of_node(nofs, inode);
			data_page = get_read_data_page(inode,
					start_bidx + ofs_in_node, REQ_RAHEAD,
					true);
			if (IS_ERR(data_page)) {
				iput(inode);
				f2fs_gc_loop_debug(sbi);
				continue;
			}

			f2fs_put_page(data_page, 0);
			add_gc_inode(gc_list, inode);
			continue;
		}

		/* phase 4 */
		inode = find_gc_inode(gc_list, dni.ino);
		if (inode) {
			struct f2fs_inode_info *fi = F2FS_I(inode);
			bool locked = false;
			int ret;

			if (S_ISREG(inode->i_mode)) {
				if (!down_write_trylock(&fi->dio_rwsem[READ])) {
					f2fs_gc_loop_debug(sbi);
					continue;
				}
				if (!down_write_trylock(
						&fi->dio_rwsem[WRITE])) {
					up_write(&fi->dio_rwsem[READ]);
					f2fs_gc_loop_debug(sbi);
					continue;
				}
				locked = true;
			}

			start_bidx = start_bidx_of_node(nofs, inode)
								+ ofs_in_node;
			if (f2fs_encrypted_inode(inode) && S_ISREG(inode->i_mode))
				ret = move_encrypted_block(inode, start_bidx,
							   segno, off, remap_gc);
			else
				ret = move_data_page(inode, start_bidx, gc_type,
						     segno, off, remap_gc);

			if (locked) {
				up_write(&fi->dio_rwsem[WRITE]);
				up_write(&fi->dio_rwsem[READ]);
			}

			stat_inc_data_blk_count(sbi, 1, gc_type);
			if (!ret)
				gc_cnt++;
		} else
			f2fs_gc_loop_debug(sbi);
	}

	if (++phase < 5)
		goto next_step;

	bd_mutex_lock(&sbi->bd_mutex);
	inc_bd_array_val(sbi, gc_data_blk_cnt, gc_type, gc_cnt);
	inc_bd_array_val(sbi, hotcold_cnt, HC_GC_COLD_DATA, gc_cnt);
	bd_mutex_unlock(&sbi->bd_mutex);
}

static int __get_victim(struct f2fs_sb_info *sbi, unsigned int *victim,
			int gc_type)
{
	struct sit_info *sit_i = SIT_I(sbi);
	int ret;

	mutex_lock(&sit_i->sentry_lock);
	ret = DIRTY_I(sbi)->v_ops->get_victim(sbi, victim, gc_type,
					      NO_CHECK_TYPE, LFS);
	mutex_unlock(&sit_i->sentry_lock);
	return ret;
}

static int do_garbage_collect(struct f2fs_sb_info *sbi,
				unsigned int start_segno,
				struct gc_inode_list *gc_list, int gc_type)
{
	struct page *sum_page;
	struct f2fs_summary_block *sum;
	struct blk_plug plug;
	unsigned int segno = start_segno;
	unsigned int end_segno = start_segno + sbi->segs_per_sec;
	int sec_freed = 0;
	int hotcold_type = get_seg_entry(sbi, segno)->type;
	unsigned char type = IS_DATASEG(hotcold_type) ?
						SUM_TYPE_DATA : SUM_TYPE_NODE;
	unsigned int vblocks;
	bool remap_gc = false;

	/* readahead multi ssa blocks those have contiguous address */
	if (sbi->segs_per_sec > 1)
		ra_meta_pages(sbi, GET_SUM_BLOCK(sbi, segno),
					sbi->segs_per_sec, META_SSA, true);

	/* reference all summary page */
	while (segno < end_segno) {
		sum_page = get_sum_page(sbi, segno++);
		unlock_page(sum_page);
	}

	blk_start_plug(&plug);

	for (segno = start_segno; segno < end_segno; segno++) {

		/* find segment summary of victim */
		sum_page = find_get_page(META_MAPPING(sbi),
					GET_SUM_BLOCK(sbi, segno));
		f2fs_put_page(sum_page, 0);

		vblocks = get_valid_blocks(sbi, segno, 1);
		if (vblocks == 0 || !PageUptodate(sum_page) ||
				unlikely(f2fs_cp_error(sbi)))
			goto next;

		sum = page_address(sum_page);
		f2fs_bug_on(sbi, type != GET_SUM_TYPE((&sum->footer)));

#ifdef CONFIG_F2FS_REMAP_GC
		if (vblocks + atomic_read(&F2FS_RMPGC(sbi)->rmpgc_cblks) >
		    F2FS_RMPGC_MAX_BLKS) {
			struct cp_control cpc;

			cpc.reason = __get_cp_reason(sbi);
			write_checkpoint(sbi, &cpc);
		}

		if (f2fs_remap_gc_on(sbi) &&
		    vblocks + atomic_read(&F2FS_RMPGC(sbi)->rmpgc_cblks) >
		    F2FS_RMPGC_VBLK_WATERLINE)
			remap_gc = true;
#endif

		/*
		 * this is to avoid deadlock:
		 * - lock_page(sum_page)         - f2fs_replace_block
		 *  - check_valid_map()            - mutex_lock(sentry_lock)
		 *   - mutex_lock(sentry_lock)     - change_curseg()
		 *                                  - lock_page(sum_page)
		 */

		if (type == SUM_TYPE_NODE)
			gc_node_segment(sbi, sum->entries, segno, gc_type,
					remap_gc);
		else
			gc_data_segment(sbi, sum->entries, gc_list, segno,
					gc_type, remap_gc);

		stat_inc_seg_count(sbi, type, gc_type);
		bd_mutex_lock(&sbi->bd_mutex);
		if (gc_type == BG_GC || get_valid_blocks(sbi, segno, 1) == 0) {
			if (type == SUM_TYPE_NODE)
				inc_bd_array_val(sbi, gc_node_seg_cnt, gc_type, 1);
			else
				inc_bd_array_val(sbi, gc_data_seg_cnt, gc_type, 1);
			inc_bd_array_val(sbi, hotcold_gc_seg_cnt, hotcold_type + 1, 1UL);/*lint !e679*/
		}
		inc_bd_array_val(sbi, hotcold_gc_blk_cnt, hotcold_type + 1,
					(unsigned long)get_valid_blocks(sbi, segno, 1));/*lint !e679*/
		bd_mutex_unlock(&sbi->bd_mutex);
next:
		f2fs_put_page(sum_page, 0);
	}

	if (gc_type == FG_GC)
		f2fs_submit_merged_bio(sbi,
				(type == SUM_TYPE_NODE) ? NODE : DATA, WRITE);

	blk_finish_plug(&plug);

	if (gc_type == FG_GC &&
		get_valid_blocks(sbi, start_segno, sbi->segs_per_sec) == 0)
		sec_freed = 1;

	stat_inc_call_count(sbi->stat_info);

	return sec_freed;
}

int f2fs_gc(struct f2fs_sb_info *sbi, bool sync, bool background)
{
	unsigned int segno;
	int gc_type = sync ? FG_GC : BG_GC;
	int sec_freed = 0, seg_freed;
	int ret = -EINVAL;
	struct cp_control cpc;
	struct gc_inode_list gc_list = {
		.ilist = LIST_HEAD_INIT(gc_list.ilist),
		.iroot = RADIX_TREE_INIT(GFP_NOFS),
	};
	int gc_completed = 0;
	u64 fggc_begin = 0, fggc_end;

	fggc_begin = local_clock();
	cpc.reason = __get_cp_reason(sbi);
gc_more:
	if (unlikely(!(sbi->sb->s_flags & MS_ACTIVE)))
		goto stop;
	if (unlikely(f2fs_cp_error(sbi))) {
		ret = -EIO;
		goto stop;
	}

	if (gc_type == BG_GC && has_not_enough_free_secs(sbi, 0, 0)) {
		/*
		 * For example, if there are many prefree_segments below given
		 * threshold, we can make them free by checkpoint. Then, we
		 * secure free segments which doesn't need fggc any more.
		 */
		ret = write_checkpoint(sbi, &cpc);
		if (ret)
			goto stop;
		if (has_not_enough_free_secs(sbi, 0, 0))
			gc_type = FG_GC;
	}

	/* f2fs_balance_fs doesn't need to do BG_GC in critical path. */
	if (gc_type == BG_GC && !background)
		goto stop;
	if (!__get_victim(sbi, &segno, gc_type))
		goto stop;
	ret = 0;

	if (unlikely(sbi->gc_loop.check && segno != sbi->gc_loop.segno))
		init_f2fs_gc_loop(sbi);
	seg_freed = do_garbage_collect(sbi, segno, &gc_list, gc_type);
	if (seg_freed && gc_type == FG_GC)
		sec_freed++;
	else if (unlikely(!seg_freed && gc_type == FG_GC)) {
		if (!sbi->gc_loop.check) {
			sbi->gc_loop.check = true;
			sbi->gc_loop.count = 1;
			sbi->gc_loop.segno = segno;
		}
		if (!(sbi->gc_loop.count % F2FS_GC_LOOP_MOD))
			f2fs_msg(sbi->sb, KERN_ERR,
				"f2fs_gc_loop same victim retry:%lu in %s:%d "
				"segno:%u type:%d blocks:%u "
				"free:%u prefree:%u rsvd:%u\n",
				sbi->gc_loop.count, __func__, __LINE__,
				segno, get_seg_entry(sbi, segno)->type,
				get_valid_blocks(sbi, segno, sbi->segs_per_sec),
				free_segments(sbi), prefree_segments(sbi),
				reserved_segments(sbi));
		sbi->gc_loop.count++;
		if (sbi->gc_loop.count > F2FS_GC_LOOP_MAX) {
			if (!sbi->gc_loop.segmap)
				sbi->gc_loop.segmap =
					f2fs_kvzalloc(f2fs_bitmap_size(MAIN_SEGS(sbi)), GFP_KERNEL);
			if (sbi->gc_loop.segmap)
				set_bit(segno, sbi->gc_loop.segmap);
		}
	}
	gc_completed = 1;

	if (gc_type == FG_GC)
		sbi->cur_victim_sec = NULL_SEGNO;

	if (!sync) {
		if (has_not_enough_free_secs(sbi, sec_freed, 0)) {
			if (prefree_segments(sbi) &&
				has_not_enough_free_secs(sbi,
				reserved_sections(sbi), 0)) {
				ret = write_checkpoint(sbi, &cpc);
				if (ret)
					goto stop;
				sec_freed = 0;
			}
			goto gc_more;
		}

		if (gc_type == FG_GC)
			ret = write_checkpoint(sbi, &cpc);
	}
stop:
	/*lint -save -e455 -e647*/
	mutex_unlock(&sbi->gc_mutex);
	if (gc_completed) {
		bd_mutex_lock(&sbi->bd_mutex);
		if (gc_type == FG_GC && fggc_begin) {
			fggc_end = local_clock();
			inc_bd_val(sbi, fggc_time, fggc_end - fggc_begin);
		}
		inc_bd_array_val(sbi, gc_cnt, gc_type, 1);
		if (ret)
			inc_bd_array_val(sbi, gc_fail_cnt, gc_type, 1);
		bd_mutex_unlock(&sbi->bd_mutex);
	}

	put_gc_inode(&gc_list);

	if (sync)
		ret = sec_freed ? 0 : -EAGAIN;
	if (unlikely(sbi->gc_loop.segmap)) {
		kvfree(sbi->gc_loop.segmap);
		sbi->gc_loop.segmap = NULL;
	}
	if (unlikely(sbi->gc_loop.check))
		init_f2fs_gc_loop(sbi);
	return ret;
}

void build_gc_manager(struct f2fs_sb_info *sbi)
{
	u64 main_count, resv_count, ovp_count, blocks_per_sec;

	DIRTY_I(sbi)->v_ops = &default_v_ops;

	/* threshold of # of valid blocks in a section for victims of FG_GC */
	main_count = SM_I(sbi)->main_segments << sbi->log_blocks_per_seg;
	resv_count = SM_I(sbi)->reserved_segments << sbi->log_blocks_per_seg;
	ovp_count = SM_I(sbi)->ovp_segments << sbi->log_blocks_per_seg;
	blocks_per_sec = sbi->blocks_per_seg * sbi->segs_per_sec;

	sbi->fggc_threshold = div64_u64((main_count - ovp_count) * blocks_per_sec,
					(main_count - resv_count));
	/*lint -restore*/
}

#ifdef CONFIG_F2FS_REMAP_GC
#define F2FS_GC_REMAP_ENTRY_TAG		(0)

static inline void __gc_remove_remap_entry(struct f2fs_remap_gc *rmpgc,
						block_t blkaddr,
						struct f2fs_gc_remap_entry *e)
{
	radix_tree_delete(&rmpgc->rmpgc_root, blkaddr);
	kfree(e);
	atomic_dec(&rmpgc->rmpgc_cblks);
}

static int f2fs_gc_copy_all_remap_entries(struct f2fs_sb_info *sbi)
{
	struct f2fs_gc_remap_entry *entries[16];
	struct f2fs_remap_gc *remap_gc = F2FS_RMPGC(sbi);
	struct page *page;
	struct f2fs_io_info fio = {
		.sbi = sbi,
		.type = DATA,
		.op_flags = REQ_SYNC,
		.encrypted_page = NULL,
#ifdef CONFIG_F2FS_HOTCOLD_DRIVER
		.sp_type = SP_UNSET,
#endif
	};
	unsigned long first_index = 0;
	int i;
	int ret, found;

	while (1) {
		found = radix_tree_gang_lookup_tag(&remap_gc->rmpgc_root,
						 (void **)entries, first_index,
						 ARRAY_SIZE(entries),
						 F2FS_GC_REMAP_ENTRY_TAG);
		if (!found)
			break;
		first_index = entries[found - 1]->dst_blkaddr + 1;

		for (i = 0; i < found; i++) {
			/* we should roll back in error path */
			page = pagecache_get_page(META_MAPPING(sbi),
					entries[i]->src_blkaddr,
					FGP_LOCK | FGP_CREAT,
					GFP_NOFS);
			if (!page) {
				f2fs_stop_checkpoint(sbi, false);
				f2fs_msg(sbi->sb, KERN_ERR,
					"get page error in copping remap page, %d",
					entries[i]->src_blkaddr);
				return -ENOMEM;
			}

			fio.page = page;
			fio.new_blkaddr = fio.old_blkaddr =
						entries[i]->src_blkaddr;
			fio.op = REQ_OP_READ;
			fio.op_flags = REQ_SYNC;
#ifdef CONFIG_F2FS_HOTCOLD_DRIVER
			fio.sp_type = SP_COLD_DATA,
#endif
			ret = f2fs_submit_page_bio(&fio);
			if (ret) {
				f2fs_put_page(page, 1);
				f2fs_stop_checkpoint(sbi, false);
				f2fs_msg(sbi->sb, KERN_ERR,
					"read io error in copping remap page");
				return ret;
			}

			lock_page(page);

			set_page_dirty(page);
			f2fs_wait_on_page_writeback(page, DATA, true);
			if (clear_page_dirty_for_io(page))
				dec_page_count(sbi, F2FS_DIRTY_META);

			fio.page = page;
			fio.new_blkaddr = fio.old_blkaddr =
						entries[i]->dst_blkaddr;
			fio.op = REQ_OP_WRITE;
			fio.op_flags = REQ_SYNC | REQ_NOIDLE;
#ifdef CONFIG_F2FS_HOTCOLD_DRIVER
			fio.sp_type = SP_UNSET,
#endif
			set_page_writeback(page);
			inc_page_count(sbi, F2FS_WB_CP_DATA);
			ret = f2fs_submit_page_bio(&fio);
			f2fs_put_page(page, 1);
			if (ret) {
				f2fs_stop_checkpoint(sbi, false);
				f2fs_msg(sbi->sb, KERN_ERR,
					"write io error in copping remap page");
				return ret;
			}
		}
	}

	filemap_fdatawait_range(META_MAPPING(sbi), 0, LLONG_MAX);

	/* If copying data fails in IO path, CP should be undone */
	if (unlikely(f2fs_cp_error(sbi)))
		return -EIO;

	return 0;
}

void f2fs_gc_drop_all_remap_entries(struct f2fs_remap_gc *remap_gc)
{
	struct f2fs_gc_remap_entry *entries[16];
	unsigned long first_index = 0;
	int i;
	int ret;

	while (1) {
		ret = radix_tree_gang_lookup_tag(&remap_gc->rmpgc_root,
						 (void **)entries, first_index,
						 ARRAY_SIZE(entries),
						 F2FS_GC_REMAP_ENTRY_TAG);
		if (!ret)
			break;

		first_index = entries[ret - 1]->dst_blkaddr + 1;

		for (i = 0; i < ret; i++)
			__gc_remove_remap_entry(remap_gc,
						     entries[i]->dst_blkaddr,
						     entries[i]);
	}

	BUG_ON(atomic_read(&remap_gc->rmpgc_cblks));
}

struct remap_mapping {
	unsigned int size;
	unsigned int old_addr;
	unsigned int new_addr;
};

extern int ufshcd_issue_remap_command(struct request_queue *q, u8 *buf, u32 size);

static int f2fs_gc_load_remap_table(struct f2fs_sb_info *sbi)
{
	struct remap_mapping *mapping;
	struct request_queue *q;
	struct f2fs_gc_remap_entry *entries[16];
	struct f2fs_remap_gc *remap_gc = F2FS_RMPGC(sbi);
	unsigned long first_index = 0;
	unsigned int index = 0;
	int i;
	int ret;

	mapping = kzalloc(sizeof(struct remap_mapping) * F2FS_RMPGC_MAX_BLKS,
				GFP_NOIO);
	if (!mapping)
		return -ENOMEM;

	while (1) {
		ret = radix_tree_gang_lookup_tag(&remap_gc->rmpgc_root,
						 (void **)entries, first_index,
						 ARRAY_SIZE(entries),
						 F2FS_GC_REMAP_ENTRY_TAG);
		if (!ret)
			break;

		first_index = entries[ret - 1]->dst_blkaddr + 1;

		for (i = 0; i < ret; i++) {
			if (index > 0 &&
			    (entries[i]->src_blkaddr ==
			     (mapping[index - 1].old_addr + mapping[index - 1].size)) &&
			    (entries[i]->dst_blkaddr ==
			     (mapping[index - 1].new_addr + mapping[index - 1].size))) {
				mapping[index - 1].size++;
				continue;
			}

			mapping[index].old_addr = entries[i]->src_blkaddr;
			mapping[index].new_addr = entries[i]->dst_blkaddr;
			mapping[index].size = 1;
			index++;
		}
	}

	q = bdev_get_queue(sbi->sb->s_bdev);
	BUG_ON(!q);

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
	ret = ufshcd_issue_remap_command(q, mapping, index);
	if (ret)
		pr_err("f2fs remap-gc: load remap table failed\n");
#else
	ret = -ENODEV;
	pr_err("f2fs remap-gc: not support\n");
#endif

	kfree(mapping);
	return ret;
}

int __f2fs_gc_lock_and_submit_remap_table(struct f2fs_sb_info *sbi,
					   long long *latency __maybe_unused)
{
#ifdef CONFIG_F2FS_STAT_FS
	ktime_t	start, stop;

	start = ktime_get();
#endif
	bool	need_copy = false;
	int	ret = 0;

	down_write(&F2FS_RMPGC(sbi)->rmpgc_sem);

	if (atomic_read(&F2FS_RMPGC(sbi)->rmpgc_refs))
		io_wait_event(F2FS_RMPGC(sbi)->rmpgc_wait,
			      !atomic_read(&F2FS_RMPGC(sbi)->rmpgc_refs));

	/* start transaction */

	/* write remap table to device */
	ret = f2fs_gc_load_remap_table(sbi);
	if (ret)
		need_copy = true;

	if (need_copy)
		ret = f2fs_gc_copy_all_remap_entries(sbi);

#ifdef CONFIG_F2FS_STAT_FS
	stop = ktime_get();

	*latency = (long long)ktime_to_ns(ktime_sub(stop, start));
	if (*latency < 0) {
		pr_err("f2fs: Submit Remap Info latency is negative.\n");
		*latency = 0;
	}
#endif

	return ret;
}

void __f2fs_gc_unlock_and_clear_remap_table(struct f2fs_sb_info *sbi,
					    long long slat __maybe_unused)
{
#ifdef CONFIG_F2FS_STAT_FS
	struct f2fs_stat_info	*si = F2FS_STAT(sbi);
	int			orders;
	int			nblocks;
	long long		latency;
	ktime_t			start, stop;

	start = ktime_get();
	nblocks = atomic_read(&F2FS_RMPGC(sbi)->rmpgc_cblks);
#endif

	/* end transaction */

	f2fs_gc_drop_all_remap_entries(F2FS_RMPGC(sbi));
	up_write(&F2FS_RMPGC(sbi)->rmpgc_sem);

#ifdef CONFIG_F2FS_STAT_FS
	orders = 0;
	while (nblocks >>= 1)
		orders++;
	if (orders >= F2FS_RMPGC_STATS_NGROUPS)
		orders = F2FS_RMPGC_STATS_NGROUPS - 1;

	si->rmpgc_cp_count++;
	si->rmpgc_block_stats[orders]++;

	stop = ktime_get();

	latency = (long long)ktime_to_ns(ktime_sub(stop, start));
	if (latency < 0) {
		pr_err("f2fs: Clear Remap Info latency is negative.\n");
		latency = 0;
	}

	latency += slat;
	if (latency <= 0) {
		pr_err("f2fs: Submit and Clear Remap Info latency is invalid.\n");
		return;
	}

	if (latency > si->rmpgc_max_lat)
		si->rmpgc_max_lat = latency;
	if (latency < si->rmpgc_min_lat)
		si->rmpgc_min_lat = latency;
	si->rmpgc_total_lat += latency;
	si->rmpgc_nlats++;

	orders = -1;
	do {
		orders++;
		do_div(latency, 10);
	} while (latency);
	if (orders >= F2FS_RMPGC_STATS_NGROUPS)
		orders = F2FS_RMPGC_STATS_NGROUPS - 1;

	si->rmpgc_time_stats[orders]++;
#endif
}

void f2fs_init_remap_gc(struct f2fs_sb_info *sbi)
{
	struct f2fs_remap_gc *rmpgc = F2FS_RMPGC(sbi);

	/* Init remap gc structure */
	atomic_set(&rmpgc->rmpgc_cblks, 0);
	INIT_RADIX_TREE(&rmpgc->rmpgc_root, GFP_ATOMIC);
	init_rwsem(&rmpgc->rmpgc_sem);
	init_waitqueue_head(&rmpgc->rmpgc_wait);
	atomic_set(&rmpgc->rmpgc_refs, 0);

	if (test_hw_opt(sbi, REMAP_GC))
		set_sbi_flag(sbi, SBI_REMAP_GC);
}

void __f2fs_gc_remove_remap_entry(struct f2fs_sb_info *sbi,
				  block_t blkaddr)
{
	struct f2fs_remap_gc *rmpgc = F2FS_RMPGC(sbi);
	struct f2fs_gc_remap_entry *e;

	rcu_read_lock();
	e = radix_tree_lookup(&rmpgc->rmpgc_root, blkaddr);
	rcu_read_unlock();
	if (!e)
		return;

	down_write(&rmpgc->rmpgc_sem);
	e = radix_tree_lookup(&rmpgc->rmpgc_root, blkaddr);
	if (e)
		__gc_remove_remap_entry(rmpgc, blkaddr, e);

	up_write(&rmpgc->rmpgc_sem);
}

void __f2fs_gc_add_remap_entry(struct f2fs_sb_info *sbi, block_t src,
			       block_t dst)
{
	struct f2fs_remap_gc *rmpgc = F2FS_RMPGC(sbi);
	struct f2fs_gc_remap_entry *e, *orig;
	block_t orig_src;
	int ret;

	e = kmalloc(sizeof(struct f2fs_gc_remap_entry),
		    GFP_NOFS | __GFP_NOFAIL);
	BUG_ON(!e);

	ret = radix_tree_preload(GFP_NOFS | __GFP_NOFAIL);
	BUG_ON(ret);

	down_write(&rmpgc->rmpgc_sem);
	orig = radix_tree_lookup(&rmpgc->rmpgc_root, (unsigned long)src);
	if (orig) {
		orig_src = orig->src_blkaddr;
		__gc_remove_remap_entry(rmpgc, src, orig);
		src = orig_src;
	}

	e->src_blkaddr = src;
	e->dst_blkaddr = dst;

	/*
	 * It should not fail here because we already allocate memory for
	 * node insertion and dst block is a new block, it is impossible
	 * that it is in the tree.
	 */
	ret = radix_tree_insert(&rmpgc->rmpgc_root, dst, e);
	BUG_ON(ret);

	radix_tree_tag_set(&rmpgc->rmpgc_root, dst, F2FS_GC_REMAP_ENTRY_TAG);
	atomic_inc(&rmpgc->rmpgc_cblks);
	up_write(&rmpgc->rmpgc_sem);
	radix_tree_preload_end();
}

bool f2fs_remap_gc_get_mapped_blk(struct f2fs_sb_info *sbi, block_t dst_blk,
				  block_t *src_blk, bool need_remove)
{
	struct f2fs_remap_gc *rmpgc;
	struct f2fs_gc_remap_entry *e;

	if (!f2fs_remap_gc_on(sbi))
		return false;

	if (dst_blk < MAIN_BLKADDR(sbi))
		return false;

	rmpgc = F2FS_RMPGC(sbi);

	rcu_read_lock();
	e = radix_tree_lookup(&rmpgc->rmpgc_root, dst_blk);
	rcu_read_unlock();
	if (!e)
		return false;

	down_read(&rmpgc->rmpgc_sem);
	e = radix_tree_lookup(&rmpgc->rmpgc_root, dst_blk);
	if (!e) {
		up_read(&rmpgc->rmpgc_sem);
		return false;
	}

	BUG_ON(dst_blk != e->dst_blkaddr);

	if (need_remove)
		__gc_remove_remap_entry(rmpgc, dst_blk, e);

	*src_blk = e->src_blkaddr;
	atomic_inc(&rmpgc->rmpgc_refs);
	up_read(&rmpgc->rmpgc_sem);
	return true;
}

void f2fs_remap_gc_put_mapped_blk(struct f2fs_sb_info *sbi)
{
	struct f2fs_remap_gc *rmpgc;

	rmpgc = F2FS_RMPGC(sbi);
	if (likely(!atomic_dec_and_test(&rmpgc->rmpgc_refs)))
		return;

	wake_up(&rmpgc->rmpgc_wait);
}

void f2fs_remap_gc_add_to_bio(struct f2fs_sb_info *sbi, struct bio *bio)
{
	struct f2fs_bio *fbio = F2FS_BIO(bio);
	struct f2fs_remap_gc *rmpgc;

	rmpgc = F2FS_RMPGC(sbi);

	if (!(fbio->flags & F2FS_BIO_REMAP_GC_REF)) {
		fbio->flags |= F2FS_BIO_REMAP_GC_REF;
		fbio->sbi = sbi;
	}

	fbio->mapped_blks++;
}

void f2fs_remap_gc_check_mapped_blk(struct f2fs_sb_info *sbi, struct bio *bio,
				    block_t dst_blk, bool need_remove)
{
	block_t src_blk = dst_blk;
	bool blk_mapped;

	blk_mapped = f2fs_remap_gc_get_mapped_blk(sbi, dst_blk, &src_blk,
						  need_remove);
	if (likely(!blk_mapped))
		return;

	f2fs_remap_gc_add_to_bio(sbi, bio);
}
#endif
