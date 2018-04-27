/*******************************************************************************
 * All rights reserved, Copyright (C) huawei LIMITED 2012
 *
 * This source code has been made available to you by HUAWEI on an
 * AS-IS basis. Anyone receiving this source code is licensed under HUAWEI
 * copyrights to use it in any way he or she deems fit, including copying it,
 * modifying it, compiling it, and redistributing it either with or without
 * modifications. Any person who transfers this source code or any derivative
 * work must include the HUAWEI copyright notice and this paragraph in
 * the transferred software.
*******************************************************************************/
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/mempool.h>
#include <linux/vmalloc.h>
#include <linux/of_reserved_mem.h>

#include "mem.h"
#include "smc.h"
#include "tc_ns_client.h"
#include "teek_ns_client.h"
#include "agent.h"
#include "securec.h"
#include "tc_ns_log.h"
#include "mailbox_mempool.h"

void tc_mem_free(TC_NS_Shared_MEM *shared_mem)
{
	if (NULL == shared_mem)
		return;

	if (shared_mem->from_mailbox)
		mailbox_free(shared_mem->kernel_addr);
	else
		vfree(shared_mem->kernel_addr);

	kfree(shared_mem);
}

TC_NS_Shared_MEM *tc_mem_allocate(size_t len, bool from_mailbox)
{
	TC_NS_Shared_MEM *shared_mem = NULL;
	void *addr = NULL;


	shared_mem = kmalloc(sizeof(TC_NS_Shared_MEM), GFP_KERNEL|__GFP_ZERO);
	if (!shared_mem) {
		tloge("shared_mem kmalloc failed\n");
		return ERR_PTR(-ENOMEM);
	}

	if (from_mailbox)
		addr = mailbox_alloc(len, MB_FLAG_ZERO);
	else {
		len = ALIGN(len, SZ_4K);
		if (len > MAILBOX_POOL_SIZE) {
			tloge("alloc sharemem size(%zu) is too large\n", len);
			kfree(shared_mem);
			return ERR_PTR(-EINVAL);
		}
		addr = vmalloc_user(len);
	}

	if (!addr) {
		tloge("alloc maibox failed\n");
		kfree(shared_mem);
		return ERR_PTR(-ENOMEM);
	}

	shared_mem->from_mailbox = from_mailbox;
	shared_mem->kernel_addr = addr;
	shared_mem->len = len;

	return shared_mem;
}

static u64 g_ion_mem_addr;
static u64 g_ion_mem_size;

static int supersonic_reserve_tee_mem(struct reserved_mem *rmem)
{
	g_ion_mem_addr = rmem->base;
	g_ion_mem_size = rmem->size;

	return 0;
}
RESERVEDMEM_OF_DECLARE(supersonic, "hisi-supersonic", supersonic_reserve_tee_mem); /*lint !e611 */


/*send the ion static memory to tee.*/
int TC_NS_register_ion_mem(void)
{
	TC_NS_SMC_CMD smc_cmd = {0};
	int ret = 0;
	struct mb_cmd_pack *mb_pack;

	if ((u64)0 == g_ion_mem_addr || (u64)0 == g_ion_mem_size){
		tloge("No ion mem static reserved for tee.\n");
		return 0;
	}

	mb_pack = mailbox_alloc_cmd_pack();
	if (!mb_pack) {
		return -ENOMEM;
	}

	mb_pack->uuid[0] = 1;
	smc_cmd.uuid_phys = virt_to_phys(mb_pack->uuid);
	smc_cmd.uuid_h_phys = virt_to_phys(mb_pack->uuid) >> 32; /*lint !e572*/
	smc_cmd.cmd_id = GLOBAL_CMD_ID_REGISTER_ION_MEM;

	mb_pack->operation.paramTypes = TEE_PARAM_TYPE_VALUE_INPUT | TEE_PARAM_TYPE_VALUE_INPUT << 4;
	mb_pack->operation.params[0].value.a = (u32)g_ion_mem_addr;
	mb_pack->operation.params[0].value.b = (u32)(g_ion_mem_addr >> 32);
	mb_pack->operation.params[1].value.a = (u32)g_ion_mem_size;

	smc_cmd.operation_phys = virt_to_phys(&mb_pack->operation);
	smc_cmd.operation_h_phys = virt_to_phys(&mb_pack->operation) >> 32; /*lint !e572*/

	ret = TC_NS_SMC(&smc_cmd, 0);
	mailbox_free(mb_pack);
	if (ret) {
	    tloge("Send ion mem info failed.\n");
	}

	return ret;
}

int tc_mem_init(void)
{
	int ret;

	tlogi("tc_mem_init\n");

	ret = mailbox_mempool_init();
	if (ret) {
		tloge("tz mailbox init failed\n");
		return -ENOMEM;
	}

	return 0;
}

void tc_mem_destroy(void)
{
	tlogi("tc_client exit\n");

	mailbox_mempool_destroy();
}
