#
# Makefile for ams TCS3430 device.
#
ifdef CONFIG_SENSORS_COLOR_AP

#
# Feature Options
#

ccflags-$(CONFIG_SENSORS_COLOR_AP)	+= -DCONFIG_AMS_OPTICAL_SENSOR_ALS
ccflags-$(CONFIG_SENSORS_COLOR_AP)	+= -DCONFIG_AMS_OPTICAL_SENSOR_ALS_XYZ
ccflags-$(CONFIG_SENSORS_COLOR_AP)	+= -DCONFIG_AMS_DEBUG_LOG

#
# Include Paths
#
ccflags-$(CONFIG_SENSORS_COLOR_AP)	+= -I$(src)

#
# Object Artifacts
#
obj-$(CONFIG_SENSORS_COLOR_AP)	+= ams_tcs3430.o color_sensor.o

endif
