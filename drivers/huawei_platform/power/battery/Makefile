ifeq ($(CONFIG_ONEWIRE_PHY_PL061_GPIO), y)
ccflags-y += -I$(srctree)/drivers/gpio
endif
obj-$(CONFIG_CERTIFICATION_IC_DS28EL15) += ds28el15.o maxim_onewire.o onewire_common.o
obj-$(CONFIG_ONEWIRE_PHY_PL061_GPIO) += onewire_phy_pl061_gpio.o
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION) += batt_info.o