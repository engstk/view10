#ifndef _NB_NETLINK_H
#define _NB_NETLINK_H

enum nb_cmd_type
{
	NBMSG_REG = 0x10,  /* NLMSG_MIN_TYPE */
	NBMSG_UNREG,
	NBMSG_VOD_REQ,
	NBMSG_KSI_REQ,
	NBMSG_REQ_BUTT,
};

enum nb_evt_type
{
	NBMSG_EVT_INVALID = 0,
	NBMSG_VOD_EVT,
	NBMSG_KSI_EVT,
	NBMSG_EVT_BUTT,
};

struct vod_event {
	uint8_t videoSegState;
	uint8_t videoProtocol;
	uint8_t videoRemainingPlayTime;
	uint8_t videoStatus;
	uint16_t aveCodeRate;
	uint16_t segSize;
	uint32_t videoIP;
	uint16_t videoPort;
	uint8_t segDuration;
	uint8_t segIndex;
};

struct ksi_event {
	uint8_t slowType;
	uint8_t avgAmp;
	uint8_t duration;
	uint8_t timeStart;
};

struct ksi_requst {
	int8_t nf_hook_enable;
	int8_t nl_event_enable;
};

struct vod_requst {
	int8_t nf_hook_enable;
	int8_t nl_event_enable;
};

void nb_notify_event(enum nb_evt_type event_type, void *data, int size);

#endif /*_NB_NETLINK_H*/
