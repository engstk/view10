#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include "blk.h"
#include "blk-mq.h"
#include "blk-mq-tag.h"
#include <linux/module.h>

#ifdef CONFIG_HISI_DEBUG_FS
extern void blk_dump_counted_io(struct blk_lld_func *lld, struct request_queue *q);
#else
static void blk_mq_dump_queued_request(struct request_queue *q)
{
	if(!list_empty(&q->high_prio_sync_dispatch_list)){
		struct request *pos;
		pr_err("high_prio_sync_dispatch_list: \n");
		list_for_each_entry(pos, &q->high_prio_sync_dispatch_list, queuelist) { /*lint !e64 !e826 */
			blk_dump_request(pos);
		}
	}
	if(!list_empty(&q->sync_dispatch_list)){
		struct request *pos;
		pr_err("sync_dispatch_list: \n");
		list_for_each_entry(pos, &q->sync_dispatch_list, queuelist) { /*lint !e64 !e826 */
			blk_dump_request(pos);
		}
	}
	if(!list_empty(&q->async_fifo_list)){
		struct request *pos;
		pr_err("async_fifo_list: \n");
		list_for_each_entry(pos, &q->async_fifo_list, async_fifo_queuelist) { /*lint !e64 !e826 */
			blk_dump_request(pos);
		}
	}
}
#endif

static void hisi_blk_dump_queue(struct request_queue *q)
{
	struct blk_lld_func *lld = blk_get_lld(q);

	if (lld->dump_status) {
		pr_err("io_count = %d\n", atomic_read(&lld->blk_idle.io_count));
	#if defined(CONFIG_HISI_DEBUG_FS)
		pr_err("blk_idle.bio_count = %d lld->blk_idle.req_count = %d\n", atomic_read(&lld->blk_idle.bio_count), atomic_read(&lld->blk_idle.req_count));
	#endif
		lld->dump_status(q, BLK_DUMP_PANIC);
		lld->dump_status = NULL;
	}

	if(q->request_queue_disk){
		pr_err("block device: %s, q = 0x%llx read inflight = %d write inflight = %d\n", q->request_queue_disk->disk_name, (unsigned long long)q,
			atomic_read(&q->request_queue_disk->part0.in_flight[0]), atomic_read(&q->request_queue_disk->part0.in_flight[1]));
	#if defined(CONFIG_HISI_BLK_MQ)
		if (q->mq_ops) {
			pr_err("total_tags_used_count = %d total_reserved_tags_used_count = %d total_high_tags_used_count = %d \n",
				atomic_read(&q->total_tags_used_count), atomic_read(&q->total_reserved_tags_used_count),
				atomic_read(&q->total_high_tags_used_count));
			pr_err("high_prio_sync_io_inflight_count = %d sync_io_inflight_count = %d async_io_inflight_count = %d \n",
				atomic_read(&q->high_prio_sync_io_inflight_count), atomic_read(&q->sync_io_inflight_count),
				atomic_read(&q->async_io_inflight_count));
		}
	#endif
#if defined(CONFIG_HISI_DEBUG_FS)
		blk_dump_counted_io(lld, q);
#else
		if (q->mq_ops && q->hisi_mq_dispatch_decision)
			blk_mq_dump_queued_request(q);
#endif/* CONFIG_HISI_DEBUG_FS */
	}
}

static LIST_HEAD(dump_list);
static  DEFINE_SPINLOCK(dump_list_lock); /*lint !e708 !e570 !e64 !e785 */

void hisi_blk_dump_register_queue(struct request_queue *q)
{
	spin_lock(&dump_list_lock);
	list_add_tail(&q->dump_list, &dump_list);
	spin_unlock(&dump_list_lock);
} /*lint !e429*/

void hisi_blk_dump_unregister_queue(struct request_queue *q)
{
	struct request_queue *q_node, *q_backup;

	spin_lock(&dump_list_lock);
	list_for_each_entry_safe(q_node, q_backup, &dump_list, dump_list) { /*lint !e64 !e826 !e530 */
		if (q_node == q) {
			list_del(&q_node->dump_list);
			break;
		}
	}
	spin_unlock(&dump_list_lock);
}

static void hisi_blk_dump(void)
{
	struct request_queue *q;

	list_for_each_entry(q, &dump_list, dump_list)  /*lint !e64 !e826 */
			hisi_blk_dump_queue(q);
}

static int hisi_blk_dump_panic_notify(struct notifier_block *nb,
				   unsigned long event, void *buf)
{
	ktime_t now = ktime_get();
	pr_err("[%s]+ current = %llu \n", __func__, now.tv64);
	hisi_blk_dump();
	pr_err("[%s]-\n", __func__);
	return 0;
} /*lint !e715 */

static struct notifier_block hisi_blk_dump_nb = {
	.notifier_call = hisi_blk_dump_panic_notify,
	.priority = 0,
};

static int __init hisi_blk_dump_init(void)
{
	return atomic_notifier_chain_register(&panic_notifier_list,
				       &hisi_blk_dump_nb);
}

static void __exit hisi_blk_dump_exit(void){}

/*lint -e528 -esym(528,*)*/
module_init(hisi_blk_dump_init);
module_exit(hisi_blk_dump_exit);
/*lint -e528 +esym(528,*)*/
