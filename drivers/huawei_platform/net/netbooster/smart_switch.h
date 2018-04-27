
#ifndef _SMART_SWITCH
#define _SMART_SWITCH
/**********************************
*	Macro definition
***********************************/

/*Absolute value function*/
#define ABS(x) (((int)(x))>0?(x):0-(x))

/*Statistics duration*/
#define MAX_STAT_SEC 32

/*Filter coefficient*/
#define MUL128 (128)
#define FILT_20 (20)
#define FILT_16 (16)

/*Time length threshold*/
#define MAX_THRED (20)
#define EXTERN_TIME (15)

/*Amplitude threshold*/
#define KSI_THRED (70*128)

/*RTT threshold*/
#define RTT_THRED (255)

/*Minimum report period*/
#define REPORT_TIMER (40)

/*US turn into MS*/
#define US_MS (1000)

/*Data packet length validity limit*/
#define MAX_PKT_LEN (1500)
#define MIN_PKT_LEN (0)

/*Data service string and length*/
#define DS_NET	("rmnet0")
#define DS_NET_LEN (6)

/*Maximum KSI legal value*/
#define MAX_KSI (200*128)

/**********************************
*	Parameter and structure definition
***********************************/
enum module_type_enum {
	S_MODULE_DISABLE = 0,
	S_MODULE_ENABLE = 1,
	S_MODULE_INVALID = 2,
	S_MODULE_BUFF
};

enum hook_type_enum {
	S_HOOK_DISABLE = 0,
	S_HOOK_ENABLE = 1,
	S_HOOK_INVALID = 2,
	S_HOOK_BUFF
};

enum rtn_para_enum
{
	S_NEW_SECOND =0,
	S_OLD_SECOND =1
};

struct pkt_cnt_swth {
	unsigned int in_pkt;
	unsigned int out_pkt;
	unsigned int in_len;
	unsigned int out_len;
	unsigned int dupack;
	unsigned int rts;
	unsigned int syn;
	unsigned int rtt;
};

struct norm_idx {
	unsigned int in_pkt;
	unsigned int out_pkt;
	unsigned int in_len;
	unsigned int out_len;
	unsigned int dupack;
	unsigned int rts;
	unsigned int syn;
	unsigned int rtt;
	unsigned int inout;
	unsigned int ksi;
	unsigned int flt_ksi;
};

struct pkt_stat_swth{
	struct pkt_cnt_swth stat[MAX_STAT_SEC];
	struct norm_idx norm_idx[MAX_STAT_SEC];
	unsigned long time_stamp;
	unsigned int idx;
};

struct report_slow_para {
	unsigned char slowType;
	unsigned char avgAmp;
	unsigned char duration;
	unsigned char timeStart;
};

#ifdef CONFIG_CHR_NETLINK_MODULE
/*Minimum report period*/
#define CHR_REPORT_TIMER (25*HZ)
#define MAX_RTT (255)
/*RAT handover scenario CHR Report Structure*/
struct chr_para {
	struct pkt_cnt_swth stat_old;
	struct pkt_cnt_swth stat_new;
	unsigned int nsi_old;
	unsigned int nsi_new;
};

struct report_chr_stru {
	unsigned char slowType;
	unsigned char avgAmp;
	unsigned char oldRtt;
	unsigned char newRtt;
};
extern unsigned int chr_smart_switch(struct chr_para *report);
#endif

/**********************************
*	Export the definition of external interfaces
***********************************/
extern int set_ksi_enable(uint8_t nf_hook_enable, uint8_t nl_event_enable);
extern int smart_switch_init(void);
extern void smart_switch_exit(void);
#endif 
