EXTRA_CFLAGS 	+= -Idrivers/hisi/mntn/blackbox/platform_lpm3
#LOCAL_C_INCLUDES += vendor/huawei/chipset_common/modules/libc_sec/include
#LOCAL_CFLAGS += -Ivendor/huawei/chipset_common/modules/libc_sec/include

subdir-ccflags-y              += -Wall -Werror

#if support casd key, open the macro
#EXTRA_CFLAGS += -DHISEE_CHIPTEST_SUPPORT_CASDKEY

ifeq ($(TARGET_VERSION_MODE),factory)
EXTRA_CFLAGS += -DCONFIG_HISI_HISEE_CHIPTEST_RT
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
EXTRA_CFLAGS += -DCONFIG_HISI_HISEE_CHIPTEST_SLT
endif

#Support to disable HiSEE.
ifeq ($(hisee),false)
EXTRA_CFLAGS += -DCONFIG_HISEE_DISABLE
endif

obj-$(CONFIG_HISI_HISEE)  += hisi_hisee.o
obj-$(CONFIG_HISI_HISEE)  += hisi_hisee_power.o
obj-$(CONFIG_HISI_HISEE)  += hisi_hisee_upgrade.o
obj-$(CONFIG_HISI_HISEE)  += hisi_hisee_chip_test.o
obj-$(CONFIG_HISI_HISEE)  += hisi_hisee_fs.o
obj-$(CONFIG_HISI_HISEE)  += hisi_hisee_autotest.o
