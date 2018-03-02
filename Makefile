#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := nfc_attendence_system

include $(IDF_PATH)/make/project.mk

sdkconfig_defaults:
	cp sdkconfig sdkconfig.old
	cp sdkconfig.defaults sdkconfig
	make menuconfig

spiffs_bin:
	mkspiffs -c main/res -b 4096 -p 256 -s 0x100000 spiffs.bin

spiffs_flash: spiffs_bin
	esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 2000000 write_flash -z 0x210000 spiffs.bin
