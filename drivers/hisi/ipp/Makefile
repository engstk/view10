# Hisilicon IPP Makefile

EXTRA_CFLAGS += -DCONFIG_IPP_DEBUG

EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/ipp/
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/ipp/v200
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/ipp/device
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/ipp/include
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/ipp/v200/include
EXTRA_CFLAGS += -I$(srctree)/drivers/hisi/ipp/device/include

obj-y += adapter_common.o
obj-y += adapter_mcf.o
obj-y += adapter_mfnr.o
obj-y += adapter_slam.o
obj-y += adapter_slamtest.o
obj-y += ipp_core.o
obj-y += memory.o

obj-y += v200/
obj-y += device/

