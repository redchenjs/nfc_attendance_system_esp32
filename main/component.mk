#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the 
# src/ directory, compile them and link them into lib(subdirectory_name).a 
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#

COMPONENT_SRCDIRS := src src/tasks src/system src/driver src/device
COMPONENT_ADD_INCLUDEDIRS := inc
COMPONENT_EMBED_TXTFILES := res/cert/cert0.pem
COMPONENT_EMBED_FILES := res/snd/snd0.mp3 res/snd/snd1.mp3 res/snd/snd2.mp3 res/snd/snd3.mp3 res/snd/snd4.mp3 res/snd/snd5.mp3 res/snd/snd6.mp3
COMPONENT_EMBED_FILES += res/ani/ani0_96x64.gif res/ani/ani1_96x64.gif res/ani/ani2_96x64.gif res/ani/ani3_96x64.gif res/ani/ani4_96x64.gif res/ani/ani5_96x64.gif res/ani/ani6_96x64.gif res/ani/ani7_96x64.gif res/ani/ani8_96x64.gif
COMPONENT_EMBED_FILES += res/ani/ani0_160x80.gif res/ani/ani1_160x80.gif res/ani/ani2_160x80.gif res/ani/ani3_160x80.gif res/ani/ani4_160x80.gif res/ani/ani5_160x80.gif res/ani/ani6_160x80.gif res/ani/ani7_160x80.gif res/ani/ani8_160x80.gif
