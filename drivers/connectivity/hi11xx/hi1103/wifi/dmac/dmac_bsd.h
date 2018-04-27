

#ifndef __DMAC_BSD_H__
#define __DMAC_BSD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "wlan_types.h"
#include "oal_list.h"
#include "dmac_vap.h"
#include "dmac_device.h"
#include "oam_wdk.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_BSD_H

typedef struct{ 
    oal_uint32  ul_average_tx_thru;              //用户在统计的时间窗内的平均发送吞吐量
    oal_uint32  ul_average_rx_thru;              //用户在统计的时间窗内的平均接收吞吐量
    oal_uint32  ul_best_rate;                    //用户在决策时的最优速率，从autorate算法获取
    oal_int8    c_average_rssi;                  //5G用户在统计时间窗内的平均接收rssi
    oal_uint8   uc_reserv[3];
}dmac_user_bsd_info_stru;


typedef enum
{
    DMAC_BSD_STA_CAP_UNKOWN     = 0x0,      //还未识别
    DMAC_BSD_STA_CAP_SINGLE_BAND = 0x1,     //单频能力
    DMAC_BSD_STA_CAP_DUAL_BAND  = 0x2,      //双频能力

    DMAC_BSD_STA_CAP_BUTT
} dmac_bsd_sta_cap_enum;
typedef oal_uint8 dmac_bsd_sta_cap_enum_uint8;



//BSD模块的双频能力识别流程控制数据结构
typedef struct
{
    oal_uint32      ul_block_cnt;
    oal_uint32      ul_last_rec_timestamp;      //最近一次接收到该用户帧的时间戳
    oal_uint32      ul_last_rec_band;           //最近一次接收到该用户帧的频段
} bsd_sta_cap_classify_stru;

//BSD模块的STA信息
typedef struct
{
    oal_uint8                   auc_user_mac_addr[WLAN_MAC_ADDR_LEN];   //MAC地址
    dmac_bsd_sta_cap_enum_uint8 en_band_cap;                            
    oal_uint8                   bit_steering_cfg_enable:1;              //用户是否配置为允许切换
    oal_uint8                   bit_lock_state:1;                       //用户是否处于切换锁定状态
    oal_uint8                   bit_steering_unfriendly_flag:1;         //用户是否是切换不友好用户
    oal_uint8                   bit_associate_flag:1;                   //该用户是否已经关联 
    oal_uint8                   bit_pre_steer_fail_flag:1;
    oal_bool_enum_uint8         en_last_assoc_by_bsd:1;                  //表明STA最近一次关联是否是因为BSD切换关联进来的
    oal_uint8                   bit_reserv:2;
    oal_uint8                   uc_associate_cnt;
    oal_uint8                   uc_last_assoc_vap_id;
    oal_uint8                   auc_reserv[2];
    dmac_vap_stru               *pst_bsd_source_vap;    
    bsd_sta_cap_classify_stru   st_cap_classify_ctl;
    oal_uint32                  ul_pre_steer_succ_cnt;                  //用户关联阶段引导成功次数
    oal_uint32                  ul_pre_steer_cnt;                       //用户关联阶段引导次数
    oal_uint32                  ul_steering_succ_cnt;                   //用户关联之后引导切换成功次数
    oal_uint32                  ul_steering_cnt;                        //用户关联之后引导切换次数
    oal_uint32                  ul_steering_last_timestamp;             //用户最近一次切换的时间戳
    oal_uint32                  ul_last_active_timestamp;               //最近更新的时间戳
    oal_uint32                  ul_last_lock_timestamp;                 //最近锁定的时间戳
    oal_dlist_head_stru         st_by_hash_list;                        //根据hash值形成的链表
    oal_dlist_head_stru         st_by_timestamp_list;                   //根据时间戳形成的链表
} bsd_sta_info_item_stru;

/*TBD 保存用户记录的表格后续可以考虑根据mac_chip_get_max_asoc_user 获取运行时支持的最大关联用户数
      来动态申请该内存资源，暂时为静态分配*/
#define BSD_STA_INFO_MAX_CNT            (128*2+10)   //每个芯片规格最大支持128个用户同时关联，额外增加10个用于保存其余STA的信息
#define BSD_STA_INFO_HASH_TBL_SIZE      64         //hash表64个记录项

typedef struct
{
    bsd_sta_info_item_stru   ast_array[BSD_STA_INFO_MAX_CNT];           //保存记录的内存池
    oal_uint32               ul_item_cnt;                               //记录已保存的用户数
    oal_dlist_head_stru      ast_hash_tbl[BSD_STA_INFO_HASH_TBL_SIZE];  //用来搜索
    oal_dlist_head_stru      st_list_head;                              //整个链表，根据更新时间排序,head->next表示最老的记录
} bsd_sta_tbl_stru;


typedef enum
{
    CFG_PARAM_INDEX_BUSY_TH = 0x0,      //负载繁忙阀值参数索引
    CFG_PARAM_INDEX_FREE_TH = 0x1,      //负载空闲阀值参数索引
    CFG_PARAM_INDEX_THRU_TH  = 0x2,     //允许切换的流量阀值参数索引
    CFG_PARAM_INDEX_RSSI_TH  = 0x3,     //5G STA触发切换的RSSI阀值参数索引
    CFG_PARAM_INDEX_SFRE_TH  = 0x4,     //导致锁定的切换频度阀值参数索引
    CFG_PARAM_INDEX_LOCK_TIME  = 0x5,   //锁定时长参数索引
    CFG_PARAM_INDEX_NEW_USER_LOCK_TIME  = 0x6,   //新用户锁定时长参数索引
    CFG_PARAM_INDEX_BLOCK_TIME  = 0x7,   //切换时拉黑时长参数索引
    CFG_PARAM_INDEX_BUTT
} dmac_bsd_cfg_param_index_enum;
typedef oal_uint8 dmac_bsd_cfg_param_index_enum_uint8;


typedef struct
{
    oal_int8                                *puc_param_str;
    dmac_bsd_cfg_param_index_enum_uint8     en_param_index;
} bsd_cfg_param_index_stru;


#define DMAC_BSD_CFG_LOAD_TH_MAX               1000
#define DMAC_BSD_LOCK_TIME_UNIT_MS             1000

#define DMAC_BSD_CFG_RSSI_TH_MAX               100
#define DMAC_BSD_CFG_RSSI_TH_MIN               20


//BSD特性的配置参数
typedef struct
{
    oal_uint32      ul_vap_load_busy_th;     //vap繁忙阀值
    oal_uint32      ul_vap_load_free_th;     //vap空闲阀值
    oal_uint32      ul_sta_steering_thr_th;  //允许STA切换的流量阀值 kbps
    oal_uint32      ul_steering_fre_th;      //切换频度阀值，单位：次/30min
    oal_uint32      ul_steering_lock_time;   //切换锁定时间，单位：min
    oal_int8        c_5g_sta_rssi_th;        //5G STA触发steering的RSSI阀值
    oal_uint8       uc_reserv;
    oal_uint16      us_new_user_lock_time; //新用户刚关联时的切换锁定时间，单位:s
    oal_uint32      ul_steering_blocktime;  //配置的拉黑定时器的超时时间
    
} bsd_cfg_stru;

//bsd ap的数据结构,一个bsd ap实际上对应了不同频段的2个vap
typedef struct
{
    oal_dlist_head_stru st_entry;           //形成所有的bsd ap链表
    dmac_vap_stru       *past_dmac_vap[2];   //指向两个处于Steering模式的vap，
                                            //在2个vap都up时才正真关联到BSD模块，偏移0表示2G的vap 偏移1表示5G的vap
    bsd_cfg_stru        st_bsd_config;      //bsd ap的steering配置参数
} bsd_ap_stru;

//BSD模块的关联之后steering实施模块的控制结构体
typedef struct
{
    bsd_ap_stru             *pst_bsd_ap;         //正在实施steering的bsd ap
    dmac_vap_stru           *pst_source_vap;     //源vap
    bsd_sta_info_item_stru  *pst_user_info;      //正在实施steering的sta的信息
    frw_timeout_stru        st_block_timer;      //拉黑定时器
    oal_uint8               auc_mac_addr[WLAN_MAC_ADDR_LEN];       //正在被Steering的user mac地址
    oal_uint8               uc_state;
    oal_uint8               uc_reserv;
} bsd_steering_stru;


//BSD模块的关联阶段steering实施模块的控制节点
typedef struct
{
    oal_dlist_head_stru st_list_entry;                      //遍历
    bsd_ap_stru         *pst_bsd_ap;                        //正在实施pre-steer的bsd ap
    dmac_vap_stru       *pst_source_vap;                    //源vap
    frw_timeout_stru    st_block_timer;                     //关联阶段每个用户阻塞的定时器
    oal_uint8           auc_mac_addr[WLAN_MAC_ADDR_LEN];    //正在被pre-Steer的user mac地址
    oal_uint8           uc_block_timeout;                   //标记是否已经阻塞超时
    oal_uint8           uc_reserv;
    bsd_sta_info_item_stru  *pst_sta_info;
} bsd_pre_steering_node_stru;

typedef struct
{
    oal_dlist_head_stru         st_pre_steering_list;    /* 关联用户节点双向链表,使用bsd_pre_steering_node_stru结构内的DLIST */
    bsd_pre_steering_node_stru  *pst_cache_user_node;
} bsd_pre_steering_stru;



//BSD模块全局的管理结构体
typedef struct
{
    oal_bool_enum_uint8      en_switch;                       //bsd特性的动态开关，此开关关闭时，BandSteering模块不再捕获关键事件
    oal_uint8                bit_load_status_switch:1;        //输出bsd ap的负载状态开关    
    oal_uint8                bit_debug_mode:1;
    oal_uint8                bit_pre_steer_swtich:1;          //关联阶段引导的动态开关  
    oal_uint8                bit_steer_mode:1;                //steer模式，0:阻塞单播的probe req  1:回复失败的auth resp  
    oal_uint8                bit_cfg_switch:1;                //bsd特性的配置开关，网页通过iwpriv下发命令开启此特性
    oal_uint8                bit_reserv:3;
    oal_uint8                uc_debug1;                        //for debug
    oal_uint8                uc_debug2;
    oal_dlist_head_stru      st_bsd_ap_list_head;             //用来遍历所有的bsd ap，对应于bsd_ap_stru的st_entry
    bsd_sta_tbl_stru         st_sta_info_tbl;                 //本模块维护的STA信息(用户级别双频能力、切换统计、切换配置)
    bsd_steering_stru        st_steering_ctl;                 //关联后steering实施模块的控制结构体
    bsd_pre_steering_stru    st_pre_steering_ctl;             //关联阶段steering实施模块的控制结构体
    frw_timeout_stru         st_bsd_sched_timer;              //bsd task的调度定时器
} bsd_mgmt_stru;


typedef enum
{
    DMAC_BSD_RET_BLOCK = 0x0,      //阻塞流程
    DMAC_BSD_RET_CONTINUE = 0x1,   //继续流程
    DMAC_BSD_RET_BUTT
}dmac_bsd_handle_result_enum;
typedef oal_uint8 dmac_bsd_handle_result_enum_uint8;

typedef enum
{
    DMAC_BSD_PRE_STEER = 0x0,       //关联阶段的引导
    DMAC_BSD_STEERING = 0x1,        //关联成功之后的引导
    DMAC_BSD_STEER_BUTT
}dmac_bsd_steer_type_enum;
typedef oal_uint8 dmac_bsd_steer_type_enum_uint8;




extern oal_void     dmac_bsd_init(void);
extern oal_void     dmac_bsd_exit(void);
extern oal_uint32   dmac_bsd_config_get(bsd_ap_stru *pst_bsd_ap,bsd_cfg_stru *pst_cfg);
//extern oal_void     dmac_bsd_vap_add_handle(dmac_vap_stru * pst_dmac_vap,mac_cfg_add_vap_param_stru *pst_param);
//extern oal_void     dmac_bsd_vap_del_handle(dmac_vap_stru * pst_dmac_vap);
extern oal_void     dmac_bsd_vap_up_handle(dmac_vap_stru * pst_dmac_vap);
extern oal_void     dmac_bsd_vap_down_handle(dmac_vap_stru * pst_dmac_vap);
extern oal_void     dmac_bsd_user_add_handle(dmac_vap_stru * pst_dmac_vap,dmac_user_stru *pst_dmac_user);
extern oal_void     dmac_bsd_user_del_handle(dmac_vap_stru * pst_dmac_vap,dmac_user_stru *pst_dmac_user);
extern oal_void     dmac_bsd_channel_switch_handle(dmac_vap_stru * pst_dmac_vap);
extern oal_void     dmac_bsd_device_load_scan_cb(oal_void *p_param);
extern oal_uint32   dmac_config_bsd(mac_vap_stru *pst_mac_vap, oal_uint8 us_len, oal_uint8 *puc_param);
extern oal_bool_enum_uint8 dmac_bsd_debug_switch(oal_void);
extern dmac_bsd_handle_result_enum_uint8   dmac_bsd_rx_probe_req_frame_handle(dmac_vap_stru *pst_dmac_vap,oal_uint8 *puc_addr,oal_int8  c_rssi);
extern dmac_bsd_handle_result_enum_uint8   dmac_bsd_rx_auth_req_frame_handle(dmac_vap_stru *pst_dmac_vap,oal_uint8 *puc_addr,oal_netbuf_stru *pst_netbuf);
extern oal_bool_enum_uint8 dmac_bsd_get_capability(oal_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
