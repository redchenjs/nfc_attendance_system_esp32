#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := nfc_attendence_system

include $(IDF_PATH)/make/project.mk

sdkconfig: sdkconfig.defaults
	$(Q) cp $< $@

menuconfig: sdkconfig
defconfig:  sdkconfig
