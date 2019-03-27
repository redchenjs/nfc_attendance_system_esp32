IDF_PATH := $(PWD)/esp-idf

PROJECT_NAME := nfc_attendance_system

include $(IDF_PATH)/make/project.mk

sdkconfig: sdkconfig.defaults
	$(Q) cp $< $@

menuconfig: sdkconfig
defconfig:  sdkconfig
