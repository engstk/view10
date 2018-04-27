#ifndef _WIRELESS_OTG
#define _WIRELESS_OTG

#define OTG_SWITCH_ENABLE 1
#define OTG_SWITCH_DISABLE 0
#define OTG_5VBOOST_ENABLE 1
#define OTG_5VBOOST_DISABLE 0

#define NOT_IN_OTG_MODE 0
#define IN_OTG_MODE 1

extern void wireless_otg_detach_handler(int);
extern void wireless_otg_attach_handler(int);
extern int wireless_otg_get_mode(void);
typedef enum boost_ctrl_type {
	BOOST_CTRL_BEGIN = 0,
	BOOST_CTRL_WIRELESS_OTG = BOOST_CTRL_BEGIN,
	BOOST_CTRL_PD_VCONN,
	BOOST_CTRL_MAX,
} boost_ctrl_type_t;

int boost_enable(boost_ctrl_type_t type);
int boost_disable(boost_ctrl_type_t type);
#endif
