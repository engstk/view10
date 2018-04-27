ifeq ($(es_low_freq),true)
EXTRA_CFLAGS += -DCONFIG_ES_VDEC_LOW_FREQ
endif

obj-$(CONFIG_HISI_ICS_IPU) += cambricon_ipu.o
obj-$(CONFIG_HISI_ICS_IPU) += ipu_smmu_drv.o
obj-$(CONFIG_HISI_ICS_IPU) += ipu_clock.o
obj-$(CONFIG_HISI_ICS_IPU) += ipu_mntn.o
ifneq ($(TARGET_BUILD_VARIANT),user)
obj-$(CONFIG_HISI_ICS_IPU) += ics_debug_proxy.o
obj-m += ics_debug.o
endif
