#ifndef _WIRED_CHANNEL_SWITCH
#define _WIRED_CHANNEL_SWITCH

#define WIRED_CHANNEL_CUTOFF 1
#define WIRED_CHANNEL_RESTORE 0

struct wired_chsw_device_ops {
	int (*set_wired_channel)(int);
};
extern int wired_chsw_ops_register(struct wired_chsw_device_ops *ops);
extern int wired_chsw_set_wired_channel(int flag);

#endif
