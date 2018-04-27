ifneq ($(TARGET_BUILD_VARIANT),user)
ifneq ($(kcov),true)
obj-$(CONFIG_HISI_ISP_RDR) += ispperf.o ispmonitor.o ispcoresight.o
subdir-ccflags-y += -DDEBUG_HISI_ISP
endif
endif

obj-$(CONFIG_HISI_ISP_RDR) += isprdr.o isplog.o

subdir-ccflags-y += -I$(srctree)/drivers/hisi/mntn/blackbox/   \
                    -Idrivers/hisi/tzdriver/libhwsecurec \
