subdir-ccflags-y += -Wall -Werror

ifeq ($(CLT_IV), true)
EXTRA_CFLAGS += -DCLT_AUDIO
endif

EXTRA_CFLAGS += -Isound/soc/codecs/

snd-soc-hi6xxx_mbhc-objs := hi6xxx_mbhc.o

obj-$(CONFIG_SND_SOC_CODEC_HI6555V2) += snd-soc-hi6xxx_mbhc.o


