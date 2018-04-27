

/*****************************************************************************
  1 Header File Including
*****************************************************************************/
#ifdef _PRE_CONFIG_USE_DTS
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif
/*lint -e322*//*lint -e7*/
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#ifdef CONFIG_PINCTRL
#include <linux/pinctrl/consumer.h>
#endif
#include <linux/fs.h>

/*lint +e322*//*lint +e7*/

#include "board.h"
#include "plat_debug.h"
#include "oal_ext_if.h"
#include "board-hi1102.h"
#include "board-hi1103.h"
#include "oal_sdio_host_if.h"
#include "plat_firmware.h"
#include "oal_hcc_bus.h"

/*****************************************************************************
  2 Global Variable Definition
*****************************************************************************/
BOARD_INFO g_board_info = {.ssi_gpio_clk = 0, .ssi_gpio_data = 0};
EXPORT_SYMBOL(g_board_info);

DEVICE_BOARD_VERSION device_board_version_list[BOARD_VERSION_BOTT] = {
    {.index = BOARD_VERSION_HI1102, .name = BOARD_VERSION_NAME_HI1102},
    {.index = BOARD_VERSION_HI1103, .name = BOARD_VERSION_NAME_HI1103},
};

DOWNLOAD_MODE device_download_mode_list[MODE_DOWNLOAD_BUTT] = {
    {.index = MODE_SDIO, .name = DOWNlOAD_MODE_SDIO},
    {.index = MODE_PCIE, .name = DOWNlOAD_MODE_PCIE},
    {.index = MODE_UART, .name = DOWNlOAD_MODE_UART},
};

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
#if 0
/*full version*/
ssi_file_st g_aSsiFile[2] =
{
    {"/system/vendor/firmware/CPU_RAM_SCHED.bin", 0x00010000},
    {"/system/vendor/firmware/BOOT_CALLBACK.bin", 0x0001d200},
};
#else
/*gnss_only uart_cfg*/
ssi_file_st g_aSsiFile[] =
{
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /*gnss only*/
    {"/system/vendor/firmware/RAM_VECTOR.bin",    0x80100800},
    {"/system/vendor/firmware/CPU_RAM_SCHED.bin", 0x80004000},
#else
    /*mpw2*/
    {"/system/vendor/firmware/BCPU_ROM.bin",           0x80000000},
    {"/system/vendor/firmware/VECTORS.bin",            0x80010000},
    {"/system/vendor/firmware/RAM_VECTOR.bin",         0x80105c00},
    {"/system/vendor/firmware/WCPU_ROM.bin",           0x4000},
    {"/system/vendor/firmware/WL_ITCM.bin",            0x10000},
    {"/system/vendor/firmware/PLAT_RAM_EXCEPTION.bin", 0x20002800},
#endif
};
#endif
#endif
/*
*
***************************************************************************
  3
 Function Definition
***
**************************************************************************/
extern irqreturn_t bfg_wake_host_isr(int irq, void *dev_id);

inline BOARD_INFO * get_hi110x_board_info(void)
{
	return &g_board_info;
}

int isAsic(void)
{
    if (VERSION_ASIC == g_board_info.is_asic)
    {
        return VERSION_ASIC;
    }
    else
    {
        return VERSION_FPGA;
    }
}
EXPORT_SYMBOL_GPL(isAsic);

int32 get_hi110x_subchip_type(void)
{
    BOARD_INFO * bd_info = NULL;

    bd_info = get_hi110x_board_info();
    if (unlikely(NULL == bd_info))
    {
        PS_PRINT_ERR("board info is err\n");
        return -EFAIL;
    }

    return bd_info->chip_nr;
}

#ifdef _PRE_CONFIG_USE_DTS
int32 get_board_dts_node(struct device_node ** np, const char * node_prop)
{
	if (NULL ==np || NULL == node_prop)
	{
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, node_prop=%p\n", np, node_prop);
		return BOARD_FAIL;
	}

	*np = of_find_compatible_node(NULL, NULL, node_prop);
	if (NULL == *np)
	{
		PS_PRINT_ERR("HISI IPC:No compatible node found.\n");
		return BOARD_FAIL;
	}

	return BOARD_SUCC;
}

int32 get_board_dts_prop(struct device_node *np, const char * dts_prop, const char ** prop_val)
{
	int32 ret = BOARD_FAIL;

	if (NULL == np || NULL == dts_prop || NULL == prop_val)
	{
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
		return BOARD_FAIL;
	}

	ret = of_property_read_string(np, dts_prop, prop_val);
    if (ret)
    {
        PS_PRINT_ERR("can't get dts_prop value: dts_prop=%s\n", dts_prop);
        return ret;
    }

	PS_PRINT_SUC("have get dts_prop and prop_val: %s=%s\n", dts_prop, *prop_val);

	return BOARD_SUCC;
}

int32 get_board_dts_gpio_prop(struct device_node *np, const char * dts_prop, int32 * prop_val)
{
	int32 ret = BOARD_FAIL;

	if (NULL == np || NULL == dts_prop || NULL == prop_val)
	{
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
		return BOARD_FAIL;
	}

    ret = of_get_named_gpio(np, dts_prop, 0);
    if (ret < 0)
    {
        PS_PRINT_ERR("can't get dts_prop value: dts_prop=%s, ret=%d\n", dts_prop, ret);
        return ret;
    }

    *prop_val = ret;
	PS_PRINT_SUC("have get dts_prop and prop_val: %s=%d\n", dts_prop, *prop_val);

	return BOARD_SUCC;
}

#endif

int32 get_board_gpio(const char * gpio_node, const char * gpio_prop, int32 *physical_gpio)
{
#ifdef _PRE_CONFIG_USE_DTS
	int32 ret= BOARD_FAIL;
	struct device_node * np = NULL;

	ret = get_board_dts_node(&np, gpio_node);
	if(BOARD_SUCC != ret)
	{
		return BOARD_FAIL;
	}

	ret = get_board_dts_gpio_prop(np, gpio_prop, physical_gpio);
	if(BOARD_SUCC != ret)
	{
		return BOARD_FAIL;
	}

	return BOARD_SUCC;
#else
	return BOARD_SUCC;
#endif
}

int32 get_board_custmize(const char * cust_node, const char * cust_prop, const char **cust_prop_val)
{
#ifdef _PRE_CONFIG_USE_DTS
	int32 ret= BOARD_FAIL;
	struct device_node * np = NULL;

	if (NULL == cust_node || NULL == cust_prop || NULL == cust_prop_val)
	{
        PS_PRINT_ERR("func has NULL input param!!!\n");
		return BOARD_FAIL;
	}

	ret = get_board_dts_node(&np, cust_node);
	if(BOARD_SUCC != ret)
	{
		return BOARD_FAIL;
	}

	ret = get_board_dts_prop(np, cust_prop, cust_prop_val);
	if(BOARD_SUCC != ret)
	{
		return BOARD_FAIL;
	}

	PS_PRINT_INFO("get board customize info %s=%s\n", cust_prop, *cust_prop_val);

	return BOARD_SUCC;
#else
	return BOARD_FAIL;
#endif
}

int32 get_board_pmu_clk32k(void)
{
    return g_board_info.bd_ops.get_board_pmu_clk32k();
}

int32 set_board_pmu_clk32k(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
	int32 ret= BOARD_FAIL;
	const char * clk_name = NULL;
	struct clk* clk = NULL;
	struct device *dev = NULL;

	dev = &pdev->dev;
	clk_name = g_board_info.clk_32k_name;
	if (BOARD_VERSION_HI1102 == get_hi110x_subchip_type())
	{
        clk = devm_clk_get(dev, "clk_pmu32kb");
	}
	else
	{
        clk = devm_clk_get(dev, clk_name);
	}

    if (NULL == clk)
	{
        PS_PRINT_ERR("Get 32k clk %s failed!!!\n", clk_name);
        CHR_EXCEPTION(CHR_WIFI_DEV(CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_32K_CLK));
        return BOARD_FAIL;
	}
	g_board_info.clk_32k = clk;

	ret = clk_prepare_enable(clk);
    if (unlikely(ret < 0))
    {
		devm_clk_put(dev, clk);
        PS_PRINT_ERR("enable 32K clk %s failed!!!", clk_name);
        CHR_EXCEPTION(CHR_WIFI_DEV(CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_32K_CLK));
        return BOARD_FAIL;
    }
#endif
	return BOARD_SUCC;
}

int32 get_board_uart_port(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_board_info.bd_ops.get_board_uart_port();
#else
    return BOARD_SUCC;
#endif
}

int32 check_evb_or_fpga(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_board_info.bd_ops.check_evb_or_fpga();
#else
    return BOARD_SUCC;
#endif
}

int32 board_get_power_pinctrl(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_board_info.bd_ops.board_get_power_pinctrl(pdev);
#else
    return BOARD_SUCC;
#endif
}

int32 board_power_gpio_init(void)
{
    return g_board_info.bd_ops.get_board_power_gpio();
}
void free_board_power_gpio(void)
{
    g_board_info.bd_ops.free_board_power_gpio();
}
#ifdef HAVE_HISI_IR
void free_board_ir_gpio(void)
{
    if (g_board_info.bfgx_ir_ctrl_gpio > -1)
    {
        gpio_free(g_board_info.bfgx_ir_ctrl_gpio);
    }
}
#endif
void free_board_wakeup_gpio(void)
{
    g_board_info.bd_ops.free_board_wakeup_gpio();
}

int32 board_wakeup_gpio_init(void)
{
    return g_board_info.bd_ops.board_wakeup_gpio_init();
}

#ifdef HAVE_HISI_IR
int32 board_ir_ctrl_init(struct platform_device *pdev)
{
    return g_board_info.bd_ops.board_ir_ctrl_init(pdev);
}
#endif

int32 board_gpio_init(struct platform_device *pdev)
{
    int32 ret= BOARD_FAIL;

    /*power on gpio request*/
    ret = board_power_gpio_init();
    if(BOARD_SUCC != ret)
    {
        PS_PRINT_ERR("get power_on dts prop failed\n");
        goto err_get_power_on_gpio;
    }

    ret = board_wakeup_gpio_init();
    if(BOARD_SUCC != ret)
    {
        PS_PRINT_ERR("get wakeup prop failed\n");
        goto oal_board_wakup_gpio_fail;
    }

#ifdef HAVE_HISI_IR
    ret = board_ir_ctrl_init(pdev);
    if(BOARD_SUCC != ret)
    {
        PS_PRINT_ERR("get ir dts prop failed\n");
        goto err_get_ir_ctrl_gpio;
    }
#endif

    return BOARD_SUCC;

#ifdef HAVE_HISI_IR
err_get_ir_ctrl_gpio:
    free_board_wakeup_gpio();
#endif
oal_board_wakup_gpio_fail:
    free_board_power_gpio();
err_get_power_on_gpio:

    CHR_EXCEPTION(CHR_WIFI_DEV(CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_GPIO));

    return BOARD_FAIL;
}

int board_get_bwkup_gpio_val(void)
{
    return gpio_get_value(g_board_info.bfgn_wakeup_host);
}

int board_get_wlan_wkup_gpio_val(void)
{
    return gpio_get_value(g_board_info.wlan_wakeup_host);
}

#if defined(PLATFORM_DEBUG_ENABLE) && defined(_PRE_CONFIG_GPIO_TO_SSI_DEBUG)
#define SSI_REG_BUFF_SIZE  (4096)
static void* g_ssi_dump_buff = NULL;
int32 board_ssi_res_init(void)
{
    g_ssi_dump_buff = oal_memalloc(SSI_REG_BUFF_SIZE);
    if(NULL == g_ssi_dump_buff)
        return BOARD_FAIL;
    return BOARD_SUCC;
}

void board_ssi_res_exit(void)
{
    oal_free(g_ssi_dump_buff);
	g_ssi_dump_buff = NULL;
}
#endif

int32 board_irq_init(void)
{
    uint32 irq = 0;
    int32 gpio = 0;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    gpio = g_board_info.wlan_wakeup_host;
    irq = gpio_to_irq(gpio);
    g_board_info.wlan_irq = irq;

    PS_PRINT_INFO("wlan_irq is %d\n", g_board_info.wlan_irq);
#endif

    gpio = g_board_info.bfgn_wakeup_host;
    irq = gpio_to_irq(gpio);
    g_board_info.bfgx_irq = irq;


    PS_PRINT_INFO("bfgx_irq is %d\n", g_board_info.bfgx_irq);

    return BOARD_SUCC;
}

int32 board_clk_init(struct platform_device *pdev)
{
    int32 ret= BOARD_FAIL;

    if (NULL == pdev)
    {
        PS_PRINT_ERR("func has NULL input param!!!\n");
        return BOARD_FAIL;
    }

    ret = g_board_info.bd_ops.get_board_pmu_clk32k();
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = set_board_pmu_clk32k(pdev);
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

void power_state_change(int32 gpio, int32 flag)
{
    PS_PRINT_INFO("power_state_change,gpio=%d,flag=%d\n", gpio, flag);

    if (BOARD_POWER_ON == flag)
    {
        gpio_direction_output(gpio, GPIO_LOWLEVEL);
        mdelay(10);
        gpio_direction_output(gpio, GPIO_HIGHLEVEL);
        mdelay(30);
    }
    else if (BOARD_POWER_OFF == flag)
    {
        gpio_direction_output(gpio, GPIO_LOWLEVEL);
    }
}

int32 board_wlan_gpio_power_on(void* data)
{
    int32 gpio = (int32)(long)(data);
    if(g_board_info.host_wakeup_wlan)
    {
        /*host wakeup dev gpio pinmux to jtag when w boot,
          must gpio low when bootup*/
        board_host_wakeup_dev_set(0);
    }
    power_state_change(gpio, BOARD_POWER_ON);
    return 0;
}

int32 board_wlan_gpio_power_off(void* data)
{
    int32 gpio = (int32)(long)(data);
    power_state_change(gpio, BOARD_POWER_OFF);
    return 0;
}

int32 board_host_wakeup_dev_set(int value)
{
    if(value)
    {
        return gpio_direction_output(g_board_info.host_wakeup_wlan, GPIO_HIGHLEVEL);
    }
    else
    {
        return gpio_direction_output(g_board_info.host_wakeup_wlan, GPIO_LOWLEVEL);
    }
}

int32 board_get_host_wakeup_dev_stat(void)
{
    return gpio_get_value(g_board_info.host_wakeup_wlan);
}

void board_power_on(uint32 ul_subsystem)
{
    return g_board_info.bd_ops.board_power_on(ul_subsystem);
}
void board_power_off(uint32 ul_subsystem)
{
    return g_board_info.bd_ops.board_power_off(ul_subsystem);
}

void board_power_reset(uint32 ul_subsystem)
{
    return g_board_info.bd_ops.board_power_reset(ul_subsystem);
}

int32 find_device_board_version(void)
{
    int32 ret= BOARD_FAIL;
    const char *device_version = NULL;

    ret = get_board_custmize(DTS_NODE_HISI_HI110X, DTS_PROP_SUBCHIP_TYPE_VERSION, &device_version);
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    g_board_info.chip_type = device_version;
    return BOARD_SUCC;
}

int32 board_func_init(void)
{
    int32 ret= BOARD_FAIL;
    //board init
    memset(&g_board_info, 0 ,sizeof(BOARD_INFO));

    ret = find_device_board_version();
    if(BOARD_SUCC != ret)
    {
        /*兼容1102*/
        g_board_info.chip_nr = BOARD_VERSION_HI1102;
        g_board_info.chip_type = device_board_version_list[g_board_info.chip_nr].name;
        PS_PRINT_WARNING("can not find device_board_version ,choose default:%s\n", g_board_info.chip_type);
    }
    else
    {
        ret = check_device_board_name();
        if (BOARD_SUCC != ret)
        {
            return BOARD_FAIL;
        }
    }

    switch (g_board_info.chip_nr)
    {
        case  BOARD_VERSION_HI1102:
            g_board_info.bd_ops.get_board_power_gpio       =hi1102_get_board_power_gpio;
            g_board_info.bd_ops.free_board_power_gpio      =hi1102_free_board_power_gpio;
            g_board_info.bd_ops.board_wakeup_gpio_init     =hi1102_board_wakeup_gpio_init;
            g_board_info.bd_ops.free_board_wakeup_gpio     =hi1102_free_board_wakeup_gpio;
            g_board_info.bd_ops.bfgx_dev_power_on          =hi1102_bfgx_dev_power_on;
            g_board_info.bd_ops.bfgx_dev_power_off         =hi1102_bfgx_dev_power_off;
            g_board_info.bd_ops.wlan_power_off             =hi1102_wlan_power_off;
            g_board_info.bd_ops.wlan_power_on              =hi1102_wlan_power_on;
            g_board_info.bd_ops.board_power_on             =hi1102_board_power_on;
            g_board_info.bd_ops.board_power_off            =hi1102_board_power_off;
            g_board_info.bd_ops.board_power_reset          =hi1102_board_power_reset;
            g_board_info.bd_ops.get_board_pmu_clk32k       =hi1102_get_board_pmu_clk32k;
            g_board_info.bd_ops.get_board_uart_port        =hi1102_get_board_uart_port;
            g_board_info.bd_ops.board_ir_ctrl_init         =hi1102_board_ir_ctrl_init;
            g_board_info.bd_ops.check_evb_or_fpga          =hi1102_check_evb_or_fpga;
            g_board_info.bd_ops.board_get_power_pinctrl    =hi1102_board_get_power_pinctrl;
            g_board_info.bd_ops.get_ini_file_name_from_dts =hi1102_get_ini_file_name_from_dts;
            break;
        case BOARD_VERSION_HI1103:
            g_board_info.bd_ops.get_board_power_gpio       =hi1103_get_board_power_gpio;
            g_board_info.bd_ops.free_board_power_gpio      =hi1103_free_board_power_gpio;
            g_board_info.bd_ops.board_wakeup_gpio_init     =hi1103_board_wakeup_gpio_init;
            g_board_info.bd_ops.free_board_wakeup_gpio     =hi1103_free_board_wakeup_gpio;
            g_board_info.bd_ops.bfgx_dev_power_on          =hi1103_bfgx_dev_power_on;
            g_board_info.bd_ops.bfgx_dev_power_off         =hi1103_bfgx_dev_power_off;
            g_board_info.bd_ops.wlan_power_off             =hi1103_wlan_power_off;
            g_board_info.bd_ops.wlan_power_on              =hi1103_wlan_power_on;
            g_board_info.bd_ops.board_power_on             =hi1103_board_power_on;
            g_board_info.bd_ops.board_power_off            =hi1103_board_power_off;
            g_board_info.bd_ops.board_power_reset          =hi1103_board_power_reset;
            g_board_info.bd_ops.get_board_pmu_clk32k       =hi1103_get_board_pmu_clk32k;
            g_board_info.bd_ops.get_board_uart_port        =hi1103_get_board_uart_port;
            g_board_info.bd_ops.board_ir_ctrl_init         =hi1103_board_ir_ctrl_init;
            g_board_info.bd_ops.check_evb_or_fpga          =hi1103_check_evb_or_fpga;
            g_board_info.bd_ops.board_get_power_pinctrl    =hi1103_board_get_power_pinctrl;
            g_board_info.bd_ops.get_ini_file_name_from_dts =hi1103_get_ini_file_name_from_dts;
            break;
        default:
            PS_PRINT_ERR("g_board_info.chip_nr=%d is illegal\n", g_board_info.chip_nr);
            return BOARD_FAIL;
    }


    PS_PRINT_INFO("g_board_info.chip_nr=%d, device_board_version is %s\n", g_board_info.chip_nr, g_board_info.chip_type);
    return BOARD_SUCC;
}

int32 check_download_channel_name(uint8* wlan_buff, int32* index)
{
    int32 i = 0;
    for (i = 0; i < MODE_DOWNLOAD_BUTT; i++)
    {
        if (0 == strncmp(device_download_mode_list[i].name, wlan_buff, strlen(device_download_mode_list[i].name)))
        {
            *index = i;
            return BOARD_SUCC;
        }
    }
    return BOARD_FAIL;
}

int32 get_download_channel(void)
{
    int32 ret= BOARD_FAIL;
    uint8 wlan_mode[DOWNLOAD_CHANNEL_LEN]={0};
    uint8 bfgn_mode[DOWNLOAD_CHANNEL_LEN]={0};

    /*wlan channel*/
    ret = find_download_channel(wlan_mode, INI_WLAN_DOWNLOAD_CHANNEL);
    if (BOARD_SUCC != ret)
    {
        /*兼容1102,1102无此配置项*/
        g_board_info.wlan_download_channel = MODE_SDIO;
        PS_PRINT_WARNING("can not find wlan_download_channel ,choose default:%s\n", device_download_mode_list[0].name);
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, NULL);
    }
    else
    {
        if (BOARD_SUCC != check_download_channel_name(wlan_mode, &(g_board_info.wlan_download_channel)))
        {
            PS_PRINT_ERR("check wlan download channel:%s error\n", bfgn_mode);
            return BOARD_FAIL;
        }
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, wlan_mode);
    }


    /*bfgn channel*/
    ret = find_download_channel(bfgn_mode, INI_BFGX_DOWNLOAD_CHANNEL);
    if (BOARD_SUCC != ret)
    {
        /*如果不存在该项，则默认保持和wlan一致*/
        g_board_info.bfgn_download_channel = g_board_info.wlan_download_channel;
        PS_PRINT_WARNING("can not find bfgn_download_channel ,choose default:%s\n", device_download_mode_list[0].name);
        return BOARD_SUCC;
    }

    if (BOARD_SUCC != check_download_channel_name(bfgn_mode, &(g_board_info.bfgn_download_channel)))
    {
        PS_PRINT_ERR("check bfgn download channel:%s error\n", bfgn_mode);
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("wlan_download_channel index:%d, bfgn_download_channel index:%d\n",
                        g_board_info.wlan_download_channel, g_board_info.bfgn_download_channel);

    return BOARD_SUCC;
}

int32 check_device_board_name(void)
{
    int32 i = 0;
    for (i = 0; i < BOARD_VERSION_BOTT; i++)
    {
        if (0 == strncmp(device_board_version_list[i].name, g_board_info.chip_type, strlen(device_board_version_list[i].name)))
        {
            g_board_info.chip_nr = i;
            return BOARD_SUCC;
        }
    }

    return BOARD_FAIL;
}

int32 get_uart_pclk_source(void)
{
    return g_board_info.uart_pclk;
}

STATIC int32 hi110x_board_probe(struct platform_device *pdev)
{
    int ret = BOARD_FAIL;

    PS_PRINT_INFO("hi110x board init\n");
    ret = board_func_init();
    if (BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = ini_cfg_init();
    if (BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = check_evb_or_fpga();
    if (BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = get_download_channel();
    if (BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = board_clk_init(pdev);
    if (BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = get_board_uart_port();
    if (BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = board_gpio_init(pdev);
    if (BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = board_irq_init();
    if (BOARD_SUCC != ret)
    {
        goto err_gpio_source;
    }

    ret = board_get_power_pinctrl(pdev);
    if (BOARD_SUCC != ret)
    {
        goto err_get_power_pinctrl;
    }

#if defined(PLATFORM_DEBUG_ENABLE) && defined(_PRE_CONFIG_GPIO_TO_SSI_DEBUG)
    ret = board_ssi_res_init();
    if (BOARD_SUCC != ret)
    {
        PS_PRINT_ERR("board_ssi_res_init failed\n");
    }
#endif

    PS_PRINT_INFO("board init ok\n");

    return BOARD_SUCC;

err_get_power_pinctrl:
    free_irq(g_board_info.bfgx_irq, NULL);
    err_gpio_source:
#ifdef HAVE_HISI_IR
    free_board_ir_gpio();
#endif
    free_board_wakeup_gpio();
    free_board_power_gpio();

    return BOARD_FAIL;
}

STATIC int32 hi110x_board_remove(struct platform_device *pdev)
{
    PS_PRINT_INFO("hi110x board exit\n");

#if defined(PLATFORM_DEBUG_ENABLE) && defined(_PRE_CONFIG_GPIO_TO_SSI_DEBUG)
    board_ssi_res_exit();
#endif

#ifdef _PRE_CONFIG_USE_DTS
    if (NEED_POWER_PREPARE == g_board_info.need_power_prepare)
    {
        devm_pinctrl_put(g_board_info.pctrl);
    }
#endif

    free_irq(g_board_info.bfgx_irq, NULL);

#ifdef HAVE_HISI_IR
    free_board_ir_gpio();
#endif

    free_board_wakeup_gpio();
    free_board_power_gpio();

    return BOARD_SUCC;
}

int32 hi110x_board_suspend(struct platform_device *pdev, pm_message_t state)
{
    return BOARD_SUCC;
}

int32 hi110x_board_resume(struct platform_device *pdev)
{
    return BOARD_SUCC;
}


/*********************************************************************/
/********************   SSI调试代码start   ***************************/
/*********************************************************************/

#ifdef PLATFORM_DEBUG_ENABLE
#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG

#define HI110X_SSI_CLK_GPIO_NAME  ("hi110x ssi clk")
#define HI110X_SSI_DATA_GPIO_NAME ("hi110x ssi data")
#define INTERVAL_TIME             (10)
#define SSI_DATA_LEN              (16)

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#define SSI_CLK_GPIO  89
#define SSI_DATA_GPIO 91
#else
#define SSI_CLK_GPIO  75
#define SSI_DATA_GPIO 77
#endif

#define SSI_WRITE_DATA 0x5a5a
ssi_trans_test_st ssi_test_st = {0};

uint32 g_ssi_clk  = 0;              /*模拟ssi时钟的GPIO管脚号*/
uint32 g_ssi_data = 0;              /*模拟ssi数据线的GPIO管脚号*/
uint16 g_ssi_base = 0x8000;         /*ssi基址*/
uint32 g_interval = INTERVAL_TIME;  /*GPIO拉出来的波形保持时间，单位us*/
uint32 g_delay    = 5;

int32 ssi_show_setup(void)
{
    PS_PRINT_INFO("clk=%d, data=%d, interval=%d us, ssi base=0x%x, r/w delay=%d cycle\n",
                    g_ssi_clk, g_ssi_data, g_interval, g_ssi_base, g_delay);
    return BOARD_SUCC;
}

int32 ssi_setup(uint32 interval, uint32 delay, uint16 ssi_base)
{
    g_interval    = interval;
    g_delay       = delay;
    g_ssi_base    = ssi_base;

    return BOARD_SUCC;
}

int32 ssi_request_gpio(uint32 clk, uint32 data)
{
    int32 ret = BOARD_FAIL;

    PS_PRINT_DBG("request hi110x ssi GPIO\n");
#ifdef GPIOF_OUT_INIT_LOW
    ret = gpio_request_one(clk, GPIOF_OUT_INIT_LOW, HI110X_SSI_CLK_GPIO_NAME);
    if (ret)
    {
        PS_PRINT_ERR("%s gpio_request_one failed ret=%d\n", HI110X_SSI_CLK_GPIO_NAME, ret);
        goto err_get_ssi_clk_gpio;
    }

    g_ssi_clk = clk;

    ret = gpio_request_one(data, GPIOF_OUT_INIT_LOW, HI110X_SSI_DATA_GPIO_NAME);
    if (ret)
    {
        PS_PRINT_ERR("%s gpio_request_one failed ret=%d\n", HI110X_SSI_DATA_GPIO_NAME, ret);
        goto err_get_ssi_data_gpio;
    }
#else
    ret = gpio_request(clk,  HI110X_SSI_CLK_GPIO_NAME);
    if(ret)
    {
        PS_PRINT_ERR("%s gpio_request failed  ret=%d\n", HI110X_SSI_CLK_GPIO_NAME, ret);
        goto err_get_ssi_clk_gpio;
    }

    gpio_direction_output(clk, 0);

    ret = gpio_request(data, HI110X_SSI_DATA_GPIO_NAME);
    if (ret)
    {
        PS_PRINT_ERR("%s gpio_request failed  ret=%d\n", HI110X_SSI_DATA_GPIO_NAME, ret);
        goto err_get_ssi_data_gpio;
    }

    gpio_direction_output(data, 0);
#endif
    g_ssi_data = data;

    return BOARD_SUCC;

err_get_ssi_data_gpio:
    gpio_free(clk);
    g_ssi_clk = 0;
err_get_ssi_clk_gpio:

    CHR_EXCEPTION(CHR_WIFI_DEV(CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_GPIO));

    return ret;
}

int32 ssi_free_gpio(void)
{
    PS_PRINT_DBG("free hi110x ssi GPIO\n");

    if (0 != g_ssi_clk)
    {
        gpio_free(g_ssi_clk);
        g_ssi_clk = 0;
    }

    if (0 != g_ssi_data)
    {
        gpio_free(g_ssi_data);
        g_ssi_data = 0;
    }

    return BOARD_SUCC;
}

void ssi_clk_output(void)
{
    gpio_direction_output(g_ssi_clk, GPIO_LOWLEVEL);
    SSI_DELAY(g_interval);
    gpio_direction_output(g_ssi_clk, GPIO_HIGHLEVEL);
}

void ssi_data_output(uint16 data)
{
    SSI_DELAY(5);
    if (data)
    {
        gpio_direction_output(g_ssi_data, GPIO_HIGHLEVEL);
    }
    else
    {
        gpio_direction_output(g_ssi_data, GPIO_LOWLEVEL);
    }

    SSI_DELAY(g_interval);
}

int32 ssi_write_data(uint16 addr, uint16 value)
{
    uint16 tx;
    uint32 i;

    for (i = 0; i < g_delay; i++)
    {
        ssi_clk_output();
        ssi_data_output(0);
    }

    /*发送SYNC位*/
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output();
    ssi_data_output(1);

    /*指示本次操作为写，高读低写*/
    PS_PRINT_DBG("tx r/w->w\n");
    ssi_clk_output();
    ssi_data_output(0);

    /*发送地址*/
    PS_PRINT_DBG("write addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /*发送数据*/
    PS_PRINT_DBG("write value:0x%x\n", value);
    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        tx = (value >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx data bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /*数据发送完成以后，保持delay个周期的0*/
    PS_PRINT_DBG("ssi write:finish, delay %d cycle\n", g_delay);
    for (i = 0; i < g_delay; i++)
    {
        ssi_clk_output();
        ssi_data_output(0);
    }

    return BOARD_SUCC;
}

uint16 ssi_read_data(uint16 addr)
{
#define SSI_READ_RETTY (1000)
    uint16 tx;
    uint32 i;
    uint32 retry = 0;
    uint16 rx;
    uint16 data = 0;

    for (i = 0; i < g_delay; i++)
    {
        ssi_clk_output();
        ssi_data_output(0);
    }

    /*发送SYNC位*/
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output();
    ssi_data_output(1);

    /*指示本次操作为读，高读低写*/
    PS_PRINT_DBG("tx r/w->r\n");
    ssi_clk_output();
    ssi_data_output(1);

    /*发送地址*/
    PS_PRINT_DBG("read addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /*延迟一个clk，否则上一个数据只保持了半个时钟周期*/
    ssi_clk_output();

    /*设置data线GPIO为输入，准备读取数据*/
    gpio_direction_input(g_ssi_data);

    PS_PRINT_DBG("data in mod, current gpio level is %d\n", gpio_get_value(g_ssi_data));

    /*读取SYNC同步位*/
    do
    {
        ssi_clk_output();
        SSI_DELAY(g_interval);
        if(gpio_get_value(g_ssi_data))
        {
            PS_PRINT_DBG("read data sync bit ok, retry=%d\n", retry);
            break;
        }
        retry++;
    }while(SSI_READ_RETTY != retry);

    if (SSI_READ_RETTY == retry)
    {
        PS_PRINT_ERR("ssi read sync bit timeout\n");
        ssi_data_output(0);
        return data;
    }

    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        ssi_clk_output();
        SSI_DELAY(g_interval);
        rx = gpio_get_value(g_ssi_data);
        PS_PRINT_DBG("rx data bit %d:%d\n", SSI_DATA_LEN - i - 1, rx);
        data = data | (rx << (SSI_DATA_LEN - i - 1));
    }

    /*恢复data线GPIO为输出，并输出0*/
    ssi_data_output(0);

    return data;
}

int32 ssi_write16(uint16 addr, uint16 value)
{
#define write_retry   (3)
    uint32 retry = 0;

    do
    {
        ssi_write_data(addr, value);
        if (value == ssi_read_data(addr))
        {
            PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);
            return BOARD_SUCC;
        }
        retry++;
    }while(retry < write_retry);

    PS_PRINT_ERR("ssi write: 0x%x=0x%x fail\n", addr, value);

    return BOARD_FAIL;
}

uint16 ssi_read16(uint16 addr)
{
    uint16 data;

    data = ssi_read_data(addr);

    PS_PRINT_SUC("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

int32 ssi_write32(uint32 addr, uint16 value)
{
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low  = (addr & 0xffff) >> 1;

    /*往基地址写地址的高16位*/
    if (ssi_write16(g_ssi_base, addr_half_word_high) < 0)
    {
        PS_PRINT_ERR("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    /*低地址写实际要写入的value*/
    if (ssi_write16(addr_half_word_low, value) < 0)
    {
        PS_PRINT_ERR("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);

    return BOARD_SUCC;
}

int32 ssi_read32(uint32 addr)
{
    uint16  data = 0;
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low  = (addr & 0xffff) >> 1;

    if (ssi_write16(g_ssi_base, addr_half_word_high) < 0)
    {
        PS_PRINT_ERR("ssi read 0x%x fail\n", addr);
        return BOARD_FAIL;
    }

    data = ssi_read_data(addr_half_word_low);

    PS_PRINT_DBG("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

int32 do_ssi_file_test(ssi_file_st *file_st, ssi_trans_test_st* pst_ssi_test)
{
    OS_KERNEL_FILE_STRU        *fp;
    uint16 data_buf = 0;
    int32 rdlen = 0;
    uint32 ul_addr = 0;
    int32 l_ret = BOARD_FAIL;

    if ((NULL == pst_ssi_test) || ( NULL == file_st))
    {
        return BOARD_FAIL;
    }
    fp = filp_open(file_st->file_name, O_RDONLY, 0);
    if (IS_ERR(fp))
    {
        fp = NULL;
        PS_PRINT_ERR("filp_open %s fail!!\n",  file_st->file_name);
        return -EFAIL;
    }
    ul_addr = file_st->write_addr;
    PS_PRINT_INFO("begin file:%s", file_st->file_name);
    while(1)
    {
        data_buf = 0;
        rdlen = kernel_read(fp, fp->f_pos, (uint8 *)&data_buf, 2);
        if (rdlen > 0)
        {
            fp->f_pos += rdlen;
        }
        else if (0 == rdlen)
        {
            PS_PRINT_INFO("file read over:%s!!\n",  file_st->file_name);
            break;
        }
        else
        {
            PS_PRINT_ERR("file read ERROR:%d!!\n", rdlen);
            goto test_fail;
        }
        l_ret = ssi_write32(ul_addr, data_buf);
        if (BOARD_SUCC != l_ret)
        {
            PS_PRINT_ERR(" write data error, ul_addr=0x%x, l_ret=%d\n", ul_addr, l_ret);
            goto test_fail;
        }
        pst_ssi_test->trans_len += 2;
        ul_addr +=2;
    }
    filp_close(fp, NULL);
    fp = NULL;
    PS_PRINT_INFO("%s send finish\n",  file_st->file_name);
    return BOARD_SUCC;
test_fail:
    filp_close(fp, NULL);
    fp = NULL;
    return BOARD_FAIL;
}
typedef struct ht_test_s {
    int32 add;
    int32 data;
}ht_test_t;

ht_test_t ht_cnt[]={
    {0x50000314,    0x0D00},
    {0x50002724,    0x0022},
    {0x50002720,    0x0033},
};
int32 test_hd_ssi_write(void)
{
    int32 i;
    if (BOARD_SUCC != ssi_request_gpio(SSI_CLK_GPIO, SSI_DATA_GPIO))
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }

    if (BOARD_SUCC != ssi_write16(0x8007,1))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    for (i=0;i<sizeof(ht_cnt)/sizeof(ht_test_t);i++)
    {
        if (0 != ssi_write32(ht_cnt[i].add, ht_cnt[i].data))
        {
            PS_PRINT_ERR("error: ssi write fail s_addr:0x%x s_data:0x%x\n", ht_cnt[i].add,ht_cnt[i].data);
            //return BOARD_FAIL;
        }
        else
        {
            //PS_PRINT_ERR("0x%x:0x%x succ\n", ht_cnt[i].add,ht_cnt[i].data);
        }
    }

    /*reset clk*/
    if (BOARD_SUCC != ssi_write16(0x8007,0))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    if (BOARD_SUCC != ssi_free_gpio())
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }
    PS_PRINT_ERR("ALL reg finish---------------------");
    return 0;
err_exit:
    PS_PRINT_ERR("test reg fail---------------------");
    ssi_free_gpio();
    return BOARD_FAIL;

}
int32 ssi_single_write(int32 addr, int16 data)
{
    if (BOARD_SUCC != ssi_request_gpio(SSI_CLK_GPIO, SSI_DATA_GPIO))
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }

    if (BOARD_SUCC != ssi_write16(0x8007,1))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    /*set wcpu wait*/
    if (BOARD_SUCC != ssi_write32(addr, data))
    {
         goto err_exit;
    }
    /*reset clk*/
    if (BOARD_SUCC != ssi_write16(0x8007,0))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    if (BOARD_SUCC != ssi_free_gpio())
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }
    return 0;
err_exit:
    ssi_free_gpio();
    return BOARD_FAIL;
}
int32 ssi_single_read(int32 addr)
{
    int32 ret;
    if (BOARD_SUCC != ssi_request_gpio(SSI_CLK_GPIO, SSI_DATA_GPIO))
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }
    if (BOARD_SUCC != ssi_write16(0x8007,1))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    ret = ssi_read32(addr);
    /*reset clk*/
    if (BOARD_SUCC != ssi_write16(0x8007,0))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    if (BOARD_SUCC != ssi_free_gpio())
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }
    return ret;
err_exit:
    ssi_free_gpio();
    return BOARD_FAIL;
}
int32 ssi_file_test(ssi_trans_test_st* pst_ssi_test)
{
    int32 i = 0;
    if (NULL == pst_ssi_test)
    {
        return BOARD_FAIL;
    }
    pst_ssi_test->trans_len = 0;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    hi1103_chip_power_on();
    hi1103_bfgx_enable();
    hi1103_wifi_enable();
#endif

    //ssi_setup(20, 10, g_ssi_base);
    //waring: fpga version should set 300801c0 1 to let host control ssi
    /*first set ssi clk ctl*/
    if (BOARD_SUCC != ssi_write16(0x8007,1))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         return BOARD_FAIL;
    }
    //env init
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /*set bootloader deadbeaf*/
    if (BOARD_SUCC != ssi_write32(0x8010010c, 0xbeaf))
    {
         PS_PRINT_ERR("set flag:beaf fail\n");
         return BOARD_FAIL;
    }
    if (BOARD_SUCC != ssi_write32(0x8010010e, 0xdead))
    {
         PS_PRINT_ERR("set flag:dead fail\n");
         return BOARD_FAIL;
    }
#else
    /*set wcpu wait*/
    if (BOARD_SUCC != ssi_write32(0x50000e00, 0x1))
    {
         PS_PRINT_ERR("set wcpu wait fail\n");
         return BOARD_FAIL;
    }

    /*reset wcpu */
    if (BOARD_SUCC != ssi_write32(0x40000030, 0xfe5e))
    {
         //脉冲复位
         //PS_PRINT_ERR("reset wcpu fail\n");
         //return BOARD_FAIL;
    }
    /*boot flag*/
    if (BOARD_SUCC != ssi_write32(0x50000200, 0xbeaf))
    {
         PS_PRINT_ERR("set boot flag fail\n");
         return BOARD_FAIL;
    }
    /*dereset bcpu*/
    if (BOARD_SUCC != ssi_write32(0x50000094, 1))
    {
         PS_PRINT_ERR("dereset bcpu\n");
         return BOARD_FAIL;
    }
#endif
    /*file download*/
    for (i = 0; i < sizeof(g_aSsiFile)/sizeof(ssi_file_st); i++)
    {
        if (BOARD_SUCC != do_ssi_file_test(&g_aSsiFile[i], pst_ssi_test))
        {
            PS_PRINT_ERR("%s write %d error\n", g_aSsiFile[i].file_name, g_aSsiFile[i].write_addr);
            return BOARD_FAIL;
        }
    }
    /*let cpu go*/
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /*reset bcpu*/
    if (BOARD_SUCC != ssi_write32(0x50000094, 0))
    {
         PS_PRINT_ERR("reset bcpu set 0 fail\n");
         return BOARD_FAIL;
    }
    if (BOARD_SUCC != ssi_write32(0x50000094, 1))
    {
         PS_PRINT_ERR("reset bcpu set 1 fail\n");
         return BOARD_FAIL;
    }
#else
    /*clear b wait*/
    if (BOARD_SUCC != ssi_write32(0x50000e04, 0x0))
    {
         PS_PRINT_ERR("clear b wait\n");
         return BOARD_FAIL;
    }
    /*clear w wait*/
    if (BOARD_SUCC != ssi_write32(0x50000e00, 0x0))
    {
         PS_PRINT_ERR("clear w wait\n");
         return BOARD_FAIL;
    }
#endif
    /*reset clk*/
    if (BOARD_SUCC != ssi_write16(0x8007,0))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         return BOARD_FAIL;
    }
    return BOARD_SUCC;
}
int32 do_ssi_mem_test(ssi_trans_test_st* pst_ssi_test)
{
    uint32 i = 0;
    uint32 ul_write_base = 0x0;
    uint32 ul_addr;
    int32 l_ret = BOARD_FAIL;
    if (NULL == pst_ssi_test)
    {
        return BOARD_FAIL;
    }

    for (i = 0; i < pst_ssi_test->trans_len; i++ )
    {
        ul_addr = ul_write_base + 2*i;  //按2字节读写
        l_ret = ssi_write32(ul_addr, SSI_WRITE_DATA);
        if (BOARD_SUCC != l_ret)
        {
            PS_PRINT_ERR(" write data error, ul_addr=0x%x, l_ret=%d\n", ul_addr, l_ret);
            return l_ret;
        }
        l_ret = ssi_read32(ul_addr);
        if (SSI_WRITE_DATA != l_ret)
        {
            PS_PRINT_ERR("read write 0x%x error, expect:0x5a5a,actual:0x%x\n",ul_addr, l_ret);
            return l_ret;
        }
    }
    return BOARD_SUCC;
}
int32 ssi_download_test(ssi_trans_test_st* pst_ssi_test)
{

    int32 l_ret = BOARD_FAIL;

    struct timeval stime,etime;

    if (NULL == pst_ssi_test)
    {
        return BOARD_FAIL;
    }
    pst_ssi_test->trans_len = 1024;
    if (BOARD_SUCC != ssi_request_gpio(SSI_CLK_GPIO, SSI_DATA_GPIO))
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        goto fail_process;
    }

    do_gettimeofday(&stime);
    switch (pst_ssi_test->test_type)
    {
        case SSI_MEM_TEST:
            l_ret = do_ssi_mem_test(pst_ssi_test);
            break;
        case SSI_FILE_TEST:
            l_ret = ssi_file_test(pst_ssi_test);
            break;
        default:
            PS_PRINT_ERR("error type=%d\n", pst_ssi_test->test_type);
            break;
    }
    do_gettimeofday(&etime);
    ssi_free_gpio();
    if (BOARD_SUCC != l_ret)
    {
        goto fail_process;
    }
    pst_ssi_test->used_time = (etime.tv_sec - stime.tv_sec)*1000 + (etime.tv_usec - stime.tv_usec)/1000;
    pst_ssi_test->send_status = 0;
    return BOARD_SUCC;
fail_process:
    pst_ssi_test->used_time = 0;
    pst_ssi_test->send_status = -1;
    return BOARD_FAIL;

}

uint32 g_hi1103_mpw2_regs[][2] =
{
    {0x50000000, 0x1000},/*GLB_CTRL*/
    {0x50002000, 0xb00},/*PMU_CMU_CTRL*/
    {0x50003000, 0xa20},/*PMU2_CMU_IR_CTRL*/
};

uint32 g_hi1103_mpw2_wcpu_regs[][2] =
{
    {0x40000000, 0x408},/*W_CTRL*/
    {0x40007000, 0x488},/*PCIE_CTRL*/
};

uint32 g_hi1103_mpw2_bcpu_regs[][2] =
{
    {0x48000000, 0x40c},/*B_CTRL*/
};

int ssi_check_device_isalive(void)
{
    int i;
    uint32 reg;
    for(i = 0; i < 2; i++)
    {
        reg = (uint32)ssi_read32(0x50000000);
        if(0x101 == reg)
        {
            PS_PRINT_INFO("reg is 0x%x\n", reg);
            break;
        }
    }

    if(2 == i)
    {
        PS_PRINT_INFO("ssi is fail, gpio-ssi did't support, reg=0x%x\n", reg);
        return -1;
    }
    return 0;
}

int ssi_read_wpcu_pc_lr_sp(void)
{
    int i;
    uint32 reg_low, reg_high, pc, lr, sp;

    /*read pc twice check whether wcpu is runing*/
    for(i = 0; i < 2; i++)
    {
        ssi_write32(0x50000400, 0x1);
        oal_mdelay(1);

        reg_low = (uint32)ssi_read32(0x50000404);
        reg_high = (uint32)ssi_read32(0x50000408);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        pc = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32(0x5000040c);
        reg_high = (uint32)ssi_read32(0x50000410);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        lr = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32(0x50000414);
        reg_high = (uint32)ssi_read32(0x50000418);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        sp = reg_low | (reg_high << 16);

        PS_PRINT_INFO("gpio-ssi:read wcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if(!pc && !lr && !sp)
        {
            PS_PRINT_INFO("gpio-ssi: wcpu deepsleep or power down!\n");
            return 0;
        }
        oal_mdelay(10);
    }

    return 0;
}

int ssi_read_bpcu_pc_lr_sp(void)
{
    int i;
    uint32 reg_low, reg_high, pc, lr, sp;

    /*read pc twice check whether wcpu is runing*/
    for(i = 0; i < 2; i++)
    {
        ssi_write32(0x50000420, 0x1);
        oal_mdelay(1);

        reg_low = (uint32)ssi_read32(0x50000424);
        reg_high = (uint32)ssi_read32(0x50000428);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        pc = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32(0x5000042c);
        reg_high = (uint32)ssi_read32(0x50000430);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        lr = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32(0x50000434);
        reg_high = (uint32)ssi_read32(0x50000438);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        sp = reg_low | (reg_high << 16);

        PS_PRINT_INFO("gpio-ssi:read bcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if(!pc && !lr && !sp)
        {
            PS_PRINT_INFO("gpio-ssi: bcpu deepsleep or power down!\n");
            return 0;
        }
        oal_mdelay(10);
    }

    return 0;
}

int ssi_check_wcpu_is_working(void)
{
    uint32 reg = (uint32)ssi_read32(0x50002240);
    return (0x3 == (reg & 0x3 ));
}

int ssi_check_bcpu_is_working(void)
{
    uint32 reg = (uint32)ssi_read32(0x50002240);
    reg >>= 2;
    return (0x3 == (reg & 0x3 ));
}

int ssi_read_wpcu_reg_arrays( void*buf, int32 buf_max_len)
{
    int ret;
    int i,j;
    uint32 reg;
    int min_size;
    int32 reg_nums = OAL_SIZEOF(g_hi1103_mpw2_wcpu_regs)/OAL_SIZEOF(g_hi1103_mpw2_wcpu_regs[0]);


    for(i = 0; i < reg_nums; i++)
    {
        min_size = OAL_MIN(g_hi1103_mpw2_wcpu_regs[i][1], buf_max_len);

        ret = ssi_check_device_isalive();
        if(ret)
        {
            PS_PRINT_INFO("gpio-ssi dead before read 0x%x\n", g_hi1103_mpw2_wcpu_regs[i][0]);
            return 0;
        }

        for(j = 0; j < min_size; j += 4)
        {
            reg = (uint32)ssi_read32(g_hi1103_mpw2_wcpu_regs[i][0]+j);
            oal_writel(reg, buf+j);
        }
        PS_PRINT_INFO("dump reg:0x%x, len:%d\n", g_hi1103_mpw2_wcpu_regs[i][0], min_size);
#ifdef CONFIG_PRINTK
        print_hex_dump(KERN_DEBUG, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
		       buf, min_size, false);
#endif
    }

    return 0;
}

int ssi_read_aon_reg_arrays( void*buf, int32 buf_max_len)
{
    int ret;
    int i,j;
    uint32 reg;
    int min_size;
    int32 reg_nums = OAL_SIZEOF(g_hi1103_mpw2_regs)/OAL_SIZEOF(g_hi1103_mpw2_regs[0]);


    for(i = 0; i < reg_nums; i++)
    {
        min_size = OAL_MIN(g_hi1103_mpw2_regs[i][1], buf_max_len);

        ret = ssi_check_device_isalive();
        if(ret)
        {
            PS_PRINT_INFO("gpio-ssi dead before read 0x%x\n", g_hi1103_mpw2_regs[i][0]);
            return 0;
        }

        for(j = 0; j < min_size; j += 4)
        {
            reg = (uint32)ssi_read32(g_hi1103_mpw2_regs[i][0]+j);
            oal_writel(reg, buf+j);
        }
        PS_PRINT_INFO("dump reg:0x%x, len:%d\n", g_hi1103_mpw2_regs[i][0], min_size);
#ifdef CONFIG_PRINTK
        print_hex_dump(KERN_DEBUG, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
		       buf, min_size, false);
#endif
    }

    return 0;
}

int ssi_read_bpcu_reg_arrays( void*buf, int32 buf_max_len)
{
    int ret;
    int i,j;
    uint32 reg;
    int min_size;
    int32 reg_nums = OAL_SIZEOF(g_hi1103_mpw2_bcpu_regs)/OAL_SIZEOF(g_hi1103_mpw2_bcpu_regs[0]);


    for(i = 0; i < reg_nums; i++)
    {
        min_size = OAL_MIN(g_hi1103_mpw2_bcpu_regs[i][1], buf_max_len);

        ret = ssi_check_device_isalive();
        if(ret)
        {
            PS_PRINT_INFO("gpio-ssi dead before read 0x%x\n", g_hi1103_mpw2_bcpu_regs[i][0]);
            return 0;
        }

        for(j = 0; j < min_size; j += 4)
        {
            reg = (uint32)ssi_read32(g_hi1103_mpw2_bcpu_regs[i][0]+j);
            oal_writel(reg, buf+j);
        }
        PS_PRINT_INFO("dump reg:0x%x, len:%d\n", g_hi1103_mpw2_bcpu_regs[i][0], min_size);
#ifdef CONFIG_PRINTK
        print_hex_dump(KERN_DEBUG, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
		       buf, min_size, false);
#endif
    }

    return 0;
}

int ssi_force_reset_aon(void)
{
    int ret;
    /*request  ssi's gpio */

    if((0 == g_board_info.ssi_gpio_clk) || (0 == g_board_info.ssi_gpio_data))
    {
        PS_PRINT_INFO("reset aon, gpio ssi don't support\n");
        return -1;
    }

    ret = ssi_request_gpio(g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data);
    if(ret)
    {
        PS_PRINT_INFO("ssi_force_reset_aon request failed:%d, data:%d, ret=%d\n",
                    g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data, ret);
        return ret;
    }

    /*try to reset aon*/
    ssi_write16(0x8009, 0x60);
    ssi_write16(0x8008, 0x60);

    PS_PRINT_INFO("ssi_force_reset_aon");

    ssi_free_gpio();

    return 0;
}

/*print device keyinfo when crash*/
int ssi_save_device_keyinfo(void)
{
    int ret;
    /*request  ssi's gpio */

    if((0 == g_board_info.ssi_gpio_clk) || (0 == g_board_info.ssi_gpio_data))
    {
        PS_PRINT_INFO("gpio ssi don't support\n");
        return -1;
    }

    PS_PRINT_INFO("gpio ssi clk:%d, data:%d\n", g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data);

    ret = ssi_request_gpio(g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data);
    if(ret)
    {
        return ret;
    }

    ssi_read16(0x8009);
    ssi_read16(0x8008);

    ssi_write16(0x8007, 0x1);/*switch to ssi clk, wcpu hold*/

    ret = ssi_check_device_isalive();
    if(ret)
    {
        /*try to reset aon*/
        ssi_write16(0x8008, 0x60);
        ssi_write16(0x8009, 0x60);
        ssi_write16(0x8007, 0x1);
        if(ssi_check_device_isalive())
        {
            PS_PRINT_INFO("after reset aon, ssi still can't work\n");
        }
        else
        {
            PS_PRINT_INFO("after reset aon, ssi ok, aon is useless now, return!!\n");
            ssi_read_wpcu_pc_lr_sp();
            ssi_read_bpcu_pc_lr_sp();
        }

        ssi_free_gpio();
        return ret;
    }

    PS_PRINT_INFO("ssi is ok, glb_ctrl is ready\n");

    /*read PC*/
    ssi_read_wpcu_pc_lr_sp();
    ssi_read_bpcu_pc_lr_sp();

    ret = ssi_check_device_isalive();
    if(ret)
    {
        ssi_free_gpio();
        return ret;
    }

    if(NULL == g_ssi_dump_buff)
    {
        PS_PRINT_INFO("g_ssi_dump_buff is null\n");
        ssi_free_gpio();
        return -1;
    }

    ssi_write16(0x8007, 0x0);/*switch from ssi clk, wcpu continue*/

    ssi_free_gpio();
    return 0;
}


int ssi_save_device_regs(void)
{
    int ret;
    /*request  ssi's gpio */

    if((0 == g_board_info.ssi_gpio_clk) || (0 == g_board_info.ssi_gpio_data))
    {
        PS_PRINT_INFO("gpio ssi don't support\n");
        return -1;
    }

    PS_PRINT_INFO("gpio ssi clk:%d, data:%d\n", g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data);

    ret = ssi_request_gpio(g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data);
    if(ret)
    {
        return ret;
    }

    ssi_read16(0x8009);
    ssi_read16(0x8008);

    ssi_write16(0x8007, 0x1);/*switch to ssi clk, wcpu hold*/

    ret = ssi_check_device_isalive();
    if(ret)
    {
        /*try to reset aon*/
        ssi_write16(0x8008, 0x60);
        ssi_write16(0x8009, 0x60);
        ssi_write16(0x8007, 0x1);
        if(ssi_check_device_isalive())
        {
            PS_PRINT_INFO("after reset aon, ssi still can't work\n");
        }
        else
        {
            PS_PRINT_INFO("after reset aon, ssi ok\n");
            ssi_read_wpcu_pc_lr_sp();
            ssi_read_bpcu_pc_lr_sp();
        }

        ssi_free_gpio();
        return ret;
    }

    PS_PRINT_INFO("ssi is ok, glb_ctrl is ready\n");

    /*read PC*/
    ssi_read_wpcu_pc_lr_sp();
    ssi_read_bpcu_pc_lr_sp();

    ret = ssi_check_device_isalive();
    if(ret)
    {
        ssi_free_gpio();
        return ret;
    }

    if(NULL == g_ssi_dump_buff)
    {
        PS_PRINT_INFO("g_ssi_dump_buff is null\n");
        ssi_free_gpio();
        return -1;
    }

    ret = ssi_read_aon_reg_arrays(g_ssi_dump_buff, SSI_REG_BUFF_SIZE);
    if(ret)
    {
        ssi_free_gpio();
        return -1;
    }

    if(ssi_check_wcpu_is_working())
    {
        ret = ssi_read_wpcu_reg_arrays(g_ssi_dump_buff, SSI_REG_BUFF_SIZE);
        if(ret)
        {
            ssi_free_gpio();
            return -1;
        }
    }

    if(ssi_check_bcpu_is_working())
    {
        ret = ssi_read_bpcu_reg_arrays(g_ssi_dump_buff, SSI_REG_BUFF_SIZE);
        if(ret)
        {
            ssi_free_gpio();
            return -1;
        }
    }

    ssi_write16(0x8007, 0x0);/*switch from ssi clk, wcpu continue*/

    ssi_free_gpio();
    return 0;
}

#endif
#endif

/*********************************************************************/
/********************   SSI调试代码end    ****************************/
/*********************************************************************/

#ifdef _PRE_CONFIG_USE_DTS
static struct of_device_id hi110x_board_match_table[] = {
    {
        .compatible = DTS_COMP_HI110X_BOARD_NAME,
        .data = NULL,
    },
    {
        .compatible = DTS_COMP_HISI_HI110X_BOARD_NAME,
        .data = NULL,
    },
    { },
};
#endif

STATIC struct platform_driver hi110x_board_driver = {
        .probe      = hi110x_board_probe,
        .remove     = hi110x_board_remove,
        .suspend    = hi110x_board_suspend,
        .resume     = hi110x_board_resume,
        .driver     = {
            .name   = "hi110x_board",
            .owner  = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
			.of_match_table	= hi110x_board_match_table,
#endif
        },
};

int32 hi110x_board_init(void)
{
    int32 ret = BOARD_FAIL;

    ret = platform_driver_register(&hi110x_board_driver);
    if (ret)
    {
        PS_PRINT_ERR("Unable to register hisi connectivity board driver.\n");
    }

    return ret;
}

void hi110x_board_exit(void)
{
    platform_driver_unregister(&hi110x_board_driver);
}


oal_uint8 g_bus_type = HCC_BUS_SDIO;
oal_int32 g_wifi_plat_dev_probe_state;

extern int create_hwconn_proc_file(void);
static int hisi_wifi_plat_dev_drv_probe(struct platform_device *pdev)
{
   int ret = 0;
    /*TBD:bus type should be defined in dts and read during probe*/
   if(HCC_BUS_SDIO == g_bus_type)
   {
        ret = oal_wifi_platform_load_sdio();
        if(ret)
        {
            printk(KERN_ERR "[HW_CONN] oal_wifi_platform_load_sdio failed.\n");
            g_wifi_plat_dev_probe_state = -OAL_FAIL;
            return ret;
        }
   }

#ifdef CONFIG_HWCONNECTIVITY
  ret = create_hwconn_proc_file();
  if (ret)
  {
      printk(KERN_ERR "[HW_CONN] create proc file failed.\n");
      g_wifi_plat_dev_probe_state = -OAL_FAIL;
      return ret;
  }
#endif
  return ret;
}

static int hisi_wifi_plat_dev_drv_remove(struct platform_device *pdev)
{
    printk(KERN_ERR "[HW_CONN] hisi_wifi_plat_dev_drv_remove.\n");
    return  OAL_SUCC;
}


#ifdef _PRE_CONFIG_USE_DTS
static const struct of_device_id hisi_wifi_match_table[] = {
    {
        .compatible = DTS_NODE_HI110X_WIFI,   // compatible must match with which defined in dts
        .data = NULL,
    },
    {},
};
#endif

static struct platform_driver hisi_wifi_platform_dev_driver = {
    .probe          = hisi_wifi_plat_dev_drv_probe,
    .remove         = hisi_wifi_plat_dev_drv_remove,
    .suspend        = NULL,
    .shutdown       = NULL,
    .driver = {
        .name = DTS_NODE_HI110X_WIFI,
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = hisi_wifi_match_table, // dts required code
#endif
    },
};

int32 hisi_wifi_platform_register_drv(void)
{
    int32 ret = BOARD_FAIL;
    PS_PRINT_FUNCTION_NAME;

    g_wifi_plat_dev_probe_state = OAL_SUCC;

    ret = platform_driver_register(&hisi_wifi_platform_dev_driver);
    if (ret)
    {
        PS_PRINT_ERR("Unable to register hisi wifi driver.\n");
    }
    /*platform_driver_register return always true*/
    return g_wifi_plat_dev_probe_state;
}

void hisi_wifi_platform_unregister_drv(void)
{
    PS_PRINT_FUNCTION_NAME;

    platform_driver_unregister(&hisi_wifi_platform_dev_driver);

    return;
}


