

#ifndef __DMAC_RADAR_H__
#define __DMAC_RADAR_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_DFS_OPTIMIZE

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hal_ext_if.h"
#include "mac_device.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DMAC_RADAR_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define STAGGER_MAX_DURATION                100         /* STAGGER类型雷达最大宽度 */
#define STAGGER_PULSE_MARGIN                4           /* STAGGER类型雷达相同脉冲间隔误差 */
#define RADAR_PULSE_MARGIN_ETSI             4           /* ETSI脉冲间隔最小误差 */
#define RADAR_PULSE_MARGIN_FCC              3           /* FCC脉冲间隔最小误差 */
#define RADAR_PULSE_MARGIN_FCC_LOW_TYPE     3           /* FCC TYPE0~TYPE2脉冲间隔最小误差 */

#define RADAR_PULSE_DURATION_MARGIN            20          /* 脉冲宽度最小误差 */
#define RADAR_PULSE_POWER_MARGIN               20          /* 脉冲功率最小误差 */
#define RADAR_PULSE_DURATION_MARGIN_ERROR     100          /* 脉冲宽度最大误差 */
#define RADAR_FCC_CHIRP_PULSE_DURATION_MARGIN   5          /* 脉冲宽度最小误差 */

#define MAX_RADAR_NORMAL_PULSE_ANA_CNT                  5   /* 非chirp雷达脉冲信息分析最大个数 */
#define MAX_RADAR_NORMAL_PULSE_ANA_CNT_ETSI_TYPE3       8   /* ETSI type3雷达脉冲信息分析最大个数 */
#define EXTRA_PULSE_CNT                                 3   /* buf中额外查询的脉冲个数 */

#define MAX_RADAR_STAGGER_NUM               3           /* STAGGER类型脉冲最大个数 */
#define MIN_RADAR_STAGGER_DURATION          9           /* STAGGER类型脉冲最小宽度 */
#define MEAN_RADAR_STAGGER_DURATION         80          /* STAGGER类型脉冲最大宽度 */

#define MIN_RADAR_NORMAL_DURATION           9           /* normal类型脉冲最小宽度 */
#define MIN_RADAR_NORMAL_DURATION_MKK       8           /* normal类型脉冲最小宽度 */

#define MIN_RADAR_NORMAL_DURATION_ETSI_TYPE3    180     /* ETSI TYPE3 最小宽度*/
#define MAX_RADAR_NORMAL_DURATION_FCC_TYPE2 110         /* FCC TYPE2  最大宽度 */
#define MIN_RADAR_NORMAL_DURATION_FCC_TYPE4 40          /* FCC TYPE4  最小宽度 */

#define MIN_RADAR_PULSE_PRI                 148         /* 雷达脉冲最小间距(us) */
#define MIN_ETSI_PULSE_PRI                  248         /* ETSI雷达脉冲最小间距(us) */

#define MIN_ETSI_CHIRP_PRI                  245         /* ETSI chirp雷达最小脉冲间隔(us) */
#define MIN_FCC_CHIRP_PRI                   990         /* FCC chirp雷达最小脉冲间隔(us) */
#define MIN_MKK_CHIRP_PRI                   990         /* MKK chirp雷达最小脉冲间隔(us) */
#define MIN_ETSI_CHIRP_PRI_NUM              4           /* ETSI chirp雷达最小脉冲间隔个数 */
#define MIN_FCC_CHIRP_PRI_NUM               1           /* FCC chirp雷达最小脉冲间隔个数 */
#define MIN_MKK_CHIRP_PRI_NUM               1           /* MKK chirp雷达最小脉冲间隔个数 */

#define MIN_RADAR_PULSE_POWER               394         /* 脉冲power最小值 */
#define MIN_RADAR_PULSE_POWER_FCC_TYPE0     390         /* FCC type0脉冲power最小值 */
#define MIN_RADAR_PULSE_POWER_ETSI_STAGGER  394         /* ETSI Stagger脉冲power最小值 */

#define RADAR_NORMAL_PULSE_TYPE             0           /* 非chirp雷达脉冲类型 */
#define RADAR_CHIRP_PULSE_TYPE              1           /* chirp雷达脉冲类型 */

#define MAX_PULSE_TIMES                     4           /* 脉冲间隔之间最大倍数 */

#define MAX_STAGGER_PULSE_TIMES             4           /* stagger脉冲间隔相差的最大倍数关系 */
#define MIN_FCC_TOW_TIMES_INT_PRI           100//200    /* FCC chirp雷达两次中断的最小时间间隔(ms) */
#define MAX_FCC_TOW_TIMES_INT_PRI           8000        /* FCC chirp雷达两次中断的最大时间间隔(ms) */
#define MAX_FCC_CHIRP_PULSE_CNT_IN_600US    5           /* FCC chirp雷达一个中断600ms内最大脉冲个数*/
#define MAX_FCC_CHIRP_EQ_DURATION_NUM       3           /* FCC chirp连续相同duration的最大个数*/

#define RADAR_PULSE_NO_PERIOD               0           /* 脉冲不具备周期性 */
#define RADAR_PULSE_NORMAL_PERIOD           1           /* 脉冲具备周期性 */
#define RADAR_PULSE_BIG_PERIOD              2           /* 脉冲间隔相差较大 */
#define RADAR_PULSE_ONE_DIFF_PRI            3           /* 等间隔脉冲中出现一个不等间隔 */

#define RADAR_ETSI_PPS_MARGIN               2
#define RADAR_ETSI_TYPE5_MIN_PPS_DIFF       (20 - RADAR_ETSI_PPS_MARGIN)          /* ETSI type5不同PRI之间最小PPS偏差度 */
#define RADAR_ETSI_TYPE5_MAX_PPS_DIFF       (50 + RADAR_ETSI_PPS_MARGIN)          /* ETSI type5不同PRI之间最大PPS偏差度 */
#define RADAR_ETSI_TYPE6_MIN_PPS_DIFF       (80 - RADAR_ETSI_PPS_MARGIN)          /* ETSI type6不同PRI之间最小PPS偏差度 */
#define RADAR_ETSI_TYPE6_MAX_PSS_DIFF       (400 + RADAR_ETSI_PPS_MARGIN)         /* ETSI type6不同PRI之间最大PPS偏差度 */

#define RADAR_ONE_SEC_IN_US                 1000000

#define CHECK_RADAR_ETSI_TYPE1_PRI(_a)      (((_a) >= 998 && (_a) <= 5002) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_TYPE2_PRI(_a)      (((_a) >= 623  && (_a) <= 5002) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_TYPE3_PRI(_a)      (((_a) >= 248  && (_a) <= 437) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_TYPE5_PRI(_a)      (((_a) >= 2500 && (_a) <= 3333) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_TYPE6_PRI(_a)      (((_a) >= 833 && (_a) <= 2500) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_STAGGER_PRI(_a)    (((_a) >= 833 && (_a) <= 3333) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_TYPE3_FILTER_DURATION(_a)      (((_a) >= 120  && (_a) <= 180) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_TYPE3_FILTER_POWER(_a)         (((_a) <= 7) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_TYPE6_DURATION_DIFF(_a)   (((_a) <= 47) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_SHORT_PULSE(_a)    (((_a) >= 10 && (_a) <= 30) ? OAL_TRUE : OAL_FALSE)

#define CHECK_RADAR_FCC_TYPE0_PRI(_a)       (((_a) >= 1426  && (_a) <= 1430) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE1_PRI(_a)       (((_a) >= 516  && (_a) <= 3068) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE2_PRI(_a)       (((_a) >= 148  && (_a) <= 232) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE3_PRI(_a)       (((_a) >= 198  && (_a) <= 502) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE4_PRI(_a)       (((_a) >= 198  && (_a) <= 502) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE4_PRI_SMALL(_a) ((((_a) >= 233  && (_a) <= 330) || ((_a) >= 336  && (_a) <= 502)) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE5_PRI(_a)       (((_a) >= 998  && (_a) <= 2002*2) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE6_PRI(_a)       (((_a) >= 331  && (_a) <= 335) ? OAL_TRUE : OAL_FALSE)

#define CHECK_RADAR_FCC_TYPE4_DURATION(_a)  (((_a) >= 40  && (_a) <= 220) ? OAL_TRUE : OAL_FALSE)

#define CHECK_RADAR_FCC_TYPE2_PRI_SMALL(_a) (((_a) >= 148  && (_a) <= 198) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE2_FILTER_CASE_DURATION(_a) (((_a) >= 160  && (_a) <= 180) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE2_FILTER_CASE_PRI(_a)       (((_a) >= 238  && (_a) <= 244) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE2_FILTER_CASE_MIN_DURATION  60
#define CHECK_RADAR_FCC_TYPE2_FILTER_CASE_DURATION_DIFF 15
#define CHECK_RADAR_FCC_CHIRP_TOTAL_CNT(_a)         (((_a) >= 3  && (_a) <= 15) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE4_DURATION_DIFF(_a)     (((_a) <= 30) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE4_POW_DIFF(_a)          (((_a) <= 2) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_CHIRP_PRI(_a)               (((_a) >= 7000  && (_a) <= 80000) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_CHIRP_POW_DIFF(_a)          (((_a) <= 8) ? OAL_TRUE : OAL_FALSE)

#define TWO_TIMES(_a)                       ((_a) << 1)
#define THREE_TIMES(_a)                     ((_a)*3)

#define TWO_TIMES_STAGGER_PULSE_MARGIN      (2 * STAGGER_PULSE_MARGIN)
#define THREE_TIMES_STAGGER_PULSE_MARGIN    (3 * STAGGER_PULSE_MARGIN)
#define FOUR_TIMES_STAGGER_PULSE_MARGIN     (4 * STAGGER_PULSE_MARGIN)

#define CHECK_STAGGER_PRI_ABSUB(_a)       (((_a) <= RADAR_PULSE_MARGIN_ETSI) || ((_a)>50))

#define CHECK_RADAR_ETSI_TYPE2_HW(_country, _num)   ((2 == (_num)) && HAL_DFS_RADAR_TYPE_ETSI == (_country))
#define CHECK_RADAR_ETSI_TYPE3_HW(_country, _num)   ((3 == (_num)) && HAL_DFS_RADAR_TYPE_ETSI == (_country))
#define CHECK_RADAR_FCC_TYPE4_HW(_country, _num)    ((4 == (_num)) && HAL_DFS_RADAR_TYPE_FCC == (_country))
#define CHECK_RADAR_FCC_TYPE3_HW(_country, _num)    ((3 == (_num)) && HAL_DFS_RADAR_TYPE_FCC == (_country))

#define CHECK_RADAR_ETSI_TYPE23_OR_FCC_TYPE34_HW(_country, _num)    \
        ((HAL_DFS_RADAR_TYPE_ETSI == (_country) && (2 == (_num) || 3 == (_num))) \
        || (HAL_DFS_RADAR_TYPE_FCC == (_country) && (3 == (_num) || 4 == (_num))))

#define CHECK_RADAR_ETSI_TYPE2_IRQ_NUM(_a, _b)  (((_a) >= 1  && (_a) <= 3 && (0 == (_b) ||((_b) >= 1  && (_b) <= 8))) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_ETSI_TYPE3_IRQ_NUM(_a, _b)  (((_a) >= 1  && (_a) <= 5 && (0 == (_b) ||((_b) >= 1  && (_b) <= 8))) ? OAL_TRUE : OAL_FALSE)

#define CHECK_RADAR_FCC_TYPE4_IRQ_NUM(_a, _b)   (((_a) >= 1  && (_a) <= 3 && (0 == (_b) ||((_b) >= 1  && (_b) <= 4))) ? OAL_TRUE : OAL_FALSE)
#define CHECK_RADAR_FCC_TYPE3_IRQ_NUM(_a, _b)   (((_a) >= 1  && (_a) <= 4 && (0 == (_b) ||((_b) >= 1  && (_b) <= 4))) ? OAL_TRUE : OAL_FALSE)

/*chirp crazy repot*/
#define MAX_IRQ_CNT_IN_CHIRP_CRAZY_REPORT_DET_FCC     100
#define MAX_IRQ_CNT_IN_CHIRP_CRAZY_REPORT_DET_ETSI    40

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* stagger模式周期检测类型 */
typedef enum
{
    DMAC_RADAR_STAGGER_PERIOD_PRI_EQUAL,            /* 存在相同的PRI，且PRI符合范围要求 */
    DMAC_RADAR_STAGGER_PERIOD_PRI_ERR,              /* 存在相同的PRI，但PRI不符合范围要求 */
    DMAC_RADAR_STAGGER_PERIOD_NOT_DEC,              /* 不存在相同的PRI，无法确定周期性，需做进一步检查 */

    DMAC_RADAR_STAGGER_PERIOD_BUTT
} dmac_radar_stagger_period_enum;

typedef oal_uint8 dmac_radar_stagger_period_enum_uint8;

/* stagger模式radar类型 */
typedef enum
{
    DMAC_RADAR_STAGGER_TYPE_INVALID,
    DMAC_RADAR_STAGGER_TYPE5,
    DMAC_RADAR_STAGGER_TYPE6,

    DMAC_RADAR_STAGGER_TYPE_BUTT
} dmac_radar_stagger_type_enum;

typedef oal_uint8 dmac_radar_stagger_type_enum_uint8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/* 脉冲信息分析结果 */
typedef struct
{
    oal_uint32      ul_min_pri;                         /* 最小脉冲间隔 */
    oal_uint16      us_min_duration;                    /* 最小脉冲宽度 */
    oal_uint16      us_max_power;                       /* 脉冲power最大值 */
    oal_uint32      aul_pri[MAX_RADAR_PULSE_NUM];       /* 所有脉冲间隔 */

    oal_uint8       uc_pri_cnt;                         /* 脉冲间隔个数 */
    oal_uint8       uc_stagger_cnt;                     /* stagger脉冲数目 */
    oal_uint8       uc_begin;                           /* 分析脉冲信息的起始index */
    oal_uint8       auc_resv1[1];                       /* 保留位 */

    oal_uint16      us_avrg_power;                      /* 平均脉冲功率 */
    oal_uint16      us_duration_diff;                   /* 最小脉冲宽度 */
    oal_uint32      ul_extra_pri;                       /* stagger type6额外读取一个pri */
    oal_uint32      ul_pri_diff;                        /* 最小脉冲间隔 */

    oal_uint16      us_power_diff;                      /* 脉冲power最大值 */
    oal_uint16      us_avrg_duration;                   /* 平均脉冲间隔 */
} dmac_radar_pulse_analysis_result_stru;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

extern oal_bool_enum_uint8 dmac_radar_filter(mac_device_stru *pst_mac_device, hal_radar_det_event_stru *pst_radar_det_info);
extern oal_bool_enum_uint8 dmac_radar_crazy_report_det_timer(hal_to_dmac_device_stru *pst_hal_device);

#endif /* _PRE_WLAN_FEATURE_DFS_OPTIMIZE */


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of dmac_vap.h */
