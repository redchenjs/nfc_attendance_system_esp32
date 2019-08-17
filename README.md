NFC Attendance System
=====================

NFC Attendance System based on ESP32 chip.

## Main Features

* NFC-Token Exchange (via PN532)
* GUI Output (GIF Animations)
* Audio Prompt (I2S Output)
* Wifi SmartConfig (with External Key)
* OTA Firmware Update (via HTTP/HTTPS)

## Obtaining

```
git clone --recursive https://github.com/redchenjs/nfc_attendance_system_esp32.git
```

* Set up the Toolchain: <https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html>

## Configure

```
./esp-idf/tools/idf.py menuconfig
```

* All project configurations are under the `NFC Attendance System` menu.

## Build & Flash & Monitor

```
./esp-idf/tools/idf.py flash monitor
```

## Board

<img src="docs/board.png">
