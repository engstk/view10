#undef TRACE_SYSTEM
#define TRACE_SYSTEM shell_temp

#if !defined(_SHELL_TEMP_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_SHELL_TEMP_H

#include <linux/tracepoint.h>

TRACE_EVENT(calc_shell_temp,	/* [false alarm]: native interface */
	TP_PROTO(struct thermal_zone_device *tz, int i, int j,
			int coef, int temp,
			long sum),
	TP_ARGS(tz, i, j, coef, temp, sum),

	TP_STRUCT__entry(
		__string(thermal_zone, tz->type)
		__field(int, i)
		__field(int, j)
		__field(int, coef)
		__field(int, temp)
		__field(long, sum)
	),

	TP_fast_assign(
		__assign_str(thermal_zone, tz->type);
		__entry->i = i;
		__entry->j = j;
		__entry->coef = coef;
		__entry->temp = temp;
		__entry->sum = sum;
	),

	TP_printk("%s: sensor=%d time=%d coef=%d temp=%d sum=%ld",
			__get_str(thermal_zone), __entry->i, __entry->j,
			__entry->coef, __entry->temp, __entry->sum)
);

TRACE_EVENT(shell_temp,		/* [false alarm]: native interface */
	TP_PROTO(struct thermal_zone_device *tz, int temp),
	TP_ARGS(tz, temp),

	TP_STRUCT__entry(
		__string(thermal_zone, tz->type)
		__field(int, temp)
	),

	TP_fast_assign(
		__assign_str(thermal_zone, tz->type);
		__entry->temp = temp;
	),

	TP_printk("%s: shell temp=%d", __get_str(thermal_zone), __entry->temp)
);
#endif

/* This part must be outside protection */
#include <trace/define_trace.h>
