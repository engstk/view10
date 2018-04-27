#undef TRACE_SYSTEM
#define TRACE_SYSTEM l3_share

#if !defined(_TRACE_L3_SHARE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_L3_SAHRE_H

#include <linux/tracepoint.h>

TRACE_EVENT(l3_cache_request_enter,
	TP_PROTO(int request_id, unsigned int size, unsigned int g_acp_flag),
	TP_ARGS(request_id, size, g_acp_flag),
	TP_STRUCT__entry(
		__field(int, request_id)
		__field(unsigned int, size)
		__field(unsigned int, g_acp_flag)
	),
	TP_fast_assign(
		__entry->request_id = request_id;
		__entry->size = size;
		__entry->g_acp_flag = g_acp_flag;
	),

	TP_printk("enter request id=%d size=%d g_acp_flag=0x%x",
		  __entry->request_id, __entry->size, __entry->g_acp_flag)
);

TRACE_EVENT(l3_cache_request_succ,
	TP_PROTO(int request_id, unsigned int g_acp_flag),
	TP_ARGS(request_id, g_acp_flag),
	TP_STRUCT__entry(
		__field(int, request_id)
		__field(unsigned int, g_acp_flag)
	),
	TP_fast_assign(
		__entry->request_id = request_id;
		__entry->g_acp_flag = g_acp_flag;
	),

	TP_printk("succ request id=%d g_acp_flag=0x%x",
		  __entry->request_id, __entry->g_acp_flag)
);


TRACE_EVENT(l3_cache_release_enter,
	TP_PROTO(int request_id, unsigned int g_acp_flag),
	TP_ARGS(request_id, g_acp_flag),
	TP_STRUCT__entry(
		__field(int, request_id)
		__field(unsigned int, g_acp_flag)
	),
	TP_fast_assign(
		__entry->request_id = request_id;
		__entry->g_acp_flag = g_acp_flag;
	),

	TP_printk("enter release id=%d g_acp_flag=0x%x",
		  __entry->request_id, __entry->g_acp_flag)
);

TRACE_EVENT(l3_cache_release_succ,
	TP_PROTO(int request_id, unsigned int g_acp_flag),
	TP_ARGS(request_id, g_acp_flag),
	TP_STRUCT__entry(
		__field(int, request_id)
		__field(unsigned int, g_acp_flag)
	),
	TP_fast_assign(
		__entry->request_id = request_id;
		__entry->g_acp_flag = g_acp_flag;
	),

	TP_printk("succ release id=%d g_acp_flag=0x%x",
		  __entry->request_id, __entry->g_acp_flag)
);

#endif /* _TRACE_L3_SHARE_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
