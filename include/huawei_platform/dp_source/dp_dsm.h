/* Copyright (c) 2008-2019, Huawei Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
#ifndef __DP_DSM_H__
#define __DP_DSM_H__

#ifdef CONFIG_HUAWEI_DSM
#define DP_DSM_ENABLE
#endif

#define DP_DSM_EDID_BLOCK_SIZE   (128)
#define DP_DSM_DPCD_RX_CAPS_SIZE (256)

#define DP_DSM_VS_PE_NUM         (4)
#define DP_DSM_VS_PE_MASK        (0xF)

typedef enum {
	DP_PARAM_TIME,
	DP_PARAM_TIME_START = DP_PARAM_TIME,
	DP_PARAM_TIME_LINK_SUCC,
	DP_PARAM_TIME_STOP,
	DP_PARAM_TIME_MAX,

	DP_PARAM_BASIC,
	DP_PARAM_WIDTH = DP_PARAM_BASIC,
	DP_PARAM_HIGH,
	DP_PARAM_MAX_WIDTH,
	DP_PARAM_MAX_HIGH,
	DP_PARAM_PIXEL_CLOCK,
	DP_PARAM_FPS,
	DP_PARAM_MAX_RATE,
	DP_PARAM_MAX_LANES,
	DP_PARAM_LINK_RATE,   // current rate
	DP_PARAM_LINK_LANES,  // current lanes
	DP_PARAM_TU,
	DP_PARAM_SOURCE_MODE, // same or diff source mode
	DP_PARAM_USER_MODE,   // vesa_id
	DP_PARAM_USER_FORMAT,
	DP_PARAM_BASIC_AUDIO, // whether or not to support audio(only video)
	DP_PARAM_BASIC_MAX,

	DP_PARAM_EXTEND,
	DP_PARAM_EDID = DP_PARAM_EXTEND,
	DP_PARAM_EDID_BLOCK0 = DP_PARAM_EDID,
	DP_PARAM_EDID_BLOCK1,
	DP_PARAM_EDID_BLOCK2,
	DP_PARAM_EDID_BLOCK3,
	DP_PARAM_EDID_MAX,
	DP_PARAM_DPCD_RX_CAPS,
	DP_PARAM_EXTEND_MAX,

	DP_PARAM_HDCP,
	DP_PARAM_HDCP_VERSION = DP_PARAM_HDCP,
	DP_PARAM_HDCP_KEY_S, // hdcp key authentication success
	DP_PARAM_HDCP_KEY_F, // hdcp key authentication failed
	DP_PARAM_HDCP_MAX,

	DP_PARAM_DIAG,
	DP_PARAM_LINK_RETRAINING = DP_PARAM_DIAG,
	DP_PARAM_SAFE_MODE,            // whether or not to be safe_mode(force display)
	DP_PARAM_READ_EDID_FAILED,
	DP_PARAM_LINK_TRAINING_FAILED,
	DP_PARAM_HOTPLUG_RETVAL,       // hotplug success or failed
	DP_PARAM_IRQ_VECTOR,
	DP_PARAM_SOURCE_SWITCH,
	DP_PARAM_DIAG_MAX,

	DP_PARAM_NUM,
	DP_PARAM_TIME_NUM = DP_PARAM_TIME_MAX - DP_PARAM_TIME,
	DP_PARAM_EDID_NUM = DP_PARAM_EDID_MAX - DP_PARAM_EDID,
} dp_imonitor_param_t;

typedef enum {
	DP_IMONITOR_TYPE,
	// link success
	DP_IMONITOR_BASIC_INFO = DP_IMONITOR_TYPE,
	DP_IMONITOR_TIME_INFO,
	DP_IMONITOR_EXTEND_INFO,
	// link failed
	DP_IMONITOR_HPD,
	DP_IMONITOR_LINK_TRAINING,
	DP_IMONITOR_HOTPLUG,
	DP_IMONITOR_HDCP,

	DP_IMONITOR_TYPE_NUM,
} dp_imonitor_type_t;

typedef enum {
	DP_DMD_TYPE,
	DP_DMD_LINK_SUCCESS = DP_DMD_TYPE,
	DP_DMD_LINK_FAILED,

	DP_DMD_TYPE_NUM,
} dp_dmd_type_t;

#ifdef DP_DSM_ENABLE
void dp_imonitor_set_pd_event(uint8_t irq_type, uint8_t cur_mode, uint8_t mode_type, uint8_t dev_type, uint8_t typec_orien);
void dp_imonitor_set_param(dp_imonitor_param_t param, void *data);
void dp_imonitor_set_param_aux_rw(bool rw, bool i2c, uint32_t addr, uint32_t len, int retval);

// NOTE:
// 1. record vs and pe for imonitor
// 2. reset vs and pe force when to debug(eng build)
void dp_imonitor_set_param_vs_pe(int index, uint8_t *vs, uint8_t *pe);

// NOTE: DON'T CALL THE FUNCTION(dp_imonitor_report) IN IRQ HANDLER, otherwise it might be panic.
// REASON: can't use vmalloc in irq handler, but use vmalloc in imonitor_create_eventobj func.
// for info report by imonitor
void dp_imonitor_report(dp_imonitor_type_t type, void *data);

// for info report by dmd
void dp_dmd_report(dp_dmd_type_t type, const char *fmt, ...);

#ifdef DP_DEBUG_ENABLE
int dp_get_pd_event_result(char *buffer, int size);
int dp_get_hotplug_result(char *buffer, int size);
int dp_get_vs_pe_result(char *buffer, int size);
#endif // DP_DEBUG_ENABLE
#else
static inline void dp_imonitor_set_pd_event(uint8_t irq_type, uint8_t cur_mode, uint8_t mode_type, uint8_t dev_type, uint8_t typec_orien) {}
static inline void dp_imonitor_set_param(dp_imonitor_param_t param, void *data) {}
static inline void dp_imonitor_set_param_aux_rw(bool rw, bool i2c, uint32_t addr, uint32_t len, int retval) {}
static inline void dp_imonitor_set_param_vs_pe(int index, uint8_t *vs, uint8_t *pe) {}

static inline void dp_imonitor_report(dp_imonitor_type_t type, void *data) {}
static inline void dp_dmd_report(dp_dmd_type_t type, const char *fmt, ...) {}

#ifdef DP_DEBUG_ENABLE
static inline int dp_get_pd_event_result(char *buffer, int size) { return 0; }
static inline int dp_get_hotplug_result(char *buffer, int size) { return 0; }
static inline int dp_get_vs_pe_result(char *buffer, int size) { return 0; }
#endif // DP_DEBUG_ENABLE
#endif // DP_DSM_ENABLE

#endif // __DP_DSM_H__

