NFC Attendance System
=====================

NFC Attendance System based on ESP32 chip.

## Main Features

* NFC token exchange
* WiFi SmartConfig support
* Audio prompt (I2S output)
* GUI output (GIF animations)
* OTA firmware update (HTTP / HTTPS)

## Preparing

### Obtain the source

```
git clone --recursive --shallow-submodules https://github.com/redchenjs/nfc_attendance_system_esp32.git
```

### Update an existing repository

```
git pull
git submodule update --init --recursive --depth 1 --recommend-shallow
```

### Setup the tools

```
./esp-idf/install.sh
```

## Building

### Setup the environment variables

```
export IDF_PATH=$PWD/esp-idf
source ./esp-idf/export.sh
```

### Configure

```
idf.py menuconfig
```

* All project configurations are under the `NFC Attendance System` menu.

### Flash & Monitor

```
idf.py flash monitor
```

## Board

<img src="docs/board.png">

## Mini-Program Code

<img src="docs/acode.jpg">

## Videos Links

* [智慧校园NFC考勤系统简介](https://www.bilibili.com/video/av64088862) ([YouTube](https://www.youtube.com/watch?v=l8kSf4VVHyo))
* [智慧校园NFC考勤系统配套微信小程序](https://www.bilibili.com/video/av83055533) ([YouTube](https://www.youtube.com/watch?v=4vxJgOV0nS0))
