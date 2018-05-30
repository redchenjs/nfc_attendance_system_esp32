#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

IDF_PATH ?= ./esp-idf

PROJECT_NAME := nfc_attendance_system

include $(IDF_PATH)/make/project.mk

sdkconfig: sdkconfig.defaults
	$(Q) cp $< $@

menuconfig: sdkconfig
defconfig:  sdkconfig
