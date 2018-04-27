#
# Main Makefile for building the corresponding module functioned target
#
#
#
# set the coorespondig path to the project root directory
# DO NOT change the variable value, or else if YOU know what you are changing
export ROOT_DEPTH=../../../..



DMAC_SCRDIR=$(WIFI_SRCDIR)\dmac_rom

# set the default make target
# NOTICE: make sure that the target name here is same with the module
# build target defined at the bottom
.PHONY:module_build

# include the config file to setup the compiling environment
#include $(PWD)/$(ROOT_DEPTH)/Script/Build/1151_Host_WiFi/env.config

# set the module objects files
obj-m :=dmac.o
dmac-rom-objs := dmac_main_rom.o dmac_mgmt_classifier_rom.o dmac_psm_ap_rom.o dmac_rx_data_rom.o dmac_tid_rom.o dmac_tx_bss_comm_rom.o dmac_smps_rom.o dmac_opmode_rom.o dmac_m2s_rom.o \
			 dmac_11w_rom.o dmac_tx_complete_rom.o dmac_user_rom.o dmac_vap_rom.o \
			 dmac_data_acq_rom.o dmac_uapsd_rom.o dmac_mgmt_bss_comm_rom.o dmac_beacon_rom.o dmac_alg_rom.o	\
			 dmac_blockack_rom.o dmac_mgmt_ap_rom.o dmac_mgmt_sta_rom.o dmac_wep_rom.o dmac_11i_rom.o dmac_11k_rom.o	\
			 dmac_scan_rom.o dmac_dfx_rom.o dmac_reset_rom.o dmac_config_rom.o dmac_config_debug_rom.o dmac_stat_rom.o dmac_fcs_rom.o \
			 dmac_acs_rom.o dmac_chan_mgmt_rom.o dmac_user_track_rom.o dmac_rx_filter_rom.o dmac_txopps_rom.o	\
			 dmac_dft_rom.o dmac_device_rom.o dmac_resource_rom.o dmac_ap_pm_rom.o dmac_hcc_adapt_rom.o dmac_psm_sta_rom.o dmac_sta_pm_rom.o dmac_ftm_rom.o dmac_csa_sta_rom.o
dmac-rom-objs += dmac_p2p_rom.o dmac_uapsd_sta_rom.o dmac_pm_sta_rom.o dmac_power_rom.o
dmac-rom-objs += dmac_btcoex_rom.o dmac_ltecoex_verify_rom.o dmac_arp_offload_rom.o dmac_auto_adjust_freq_rom.o
dmac-rom-objs += mac_data_rom.o mac_frame_rom.o mac_ie_rom.o mac_regdomain_rom.o mac_device_rom.o mac_resource_rom.o mac_board_rom.o mac_user_rom.o mac_vap_rom.o
dmac-rom-objs := $(addprefix $(DMAC_SCRDIR)\,$(dmac-rom-objs))
dmac-rom-dump-objs := $(dmac-rom-objs)

# set the feature options
#include $(PWD)/$(ROOT_DEPTH)/Script/Build/1151_Host_WiFi/caps.config

# for example:
# ifeq ($(SUPPORT_VIDEO_OPTIMIZE),1)
#	alg-objs += alg_video.o
# endif

# set the module name
MODULE_NAME=$(subst .o,.ko,$(obj-m))

BACKUP_OBJFILE_FOLDER=$(strip $(subst .o,,$(obj-m)))_objfile

BACKUP_OBJFILE_DIR=$(PWD)/$(BACKUP_OBJFILE_FOLDER)

OBJDUMP_FILE=$(subst .o,.objdump,$(obj-m))
OBJDUMP_TXT_FILE=$(subst .o,.txt,$(obj-m))

# module's elf header file name
MODULE_ELF_HEADER_FILE=$(subst .o,.elf_header,$(obj-m))
