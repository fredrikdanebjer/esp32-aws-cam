# FSU-Eye

Frellie-Sees-yoU is a home survelleince project, intended to surveil your home with cameras.
This project-part implements the Eye unit (imagination!) controllong the camera. It runs on a ESP32, hosts a web server with video stream, connects to AWS and uploads images taken with its camera as well as exposes an interface from which it can be controlled.

## Features
- HTTP Webserver with Camera Stream (to be used with e.g. Home Assistant)
- AWS IoT MQTT based OTA Job
- AWS IoT MQTT based periodic camera upload
- AWS IoT MQTT based periodic diagnostic message upload
- AWS IoT MQTT based control interface for receiving commands

## Prerequsites

The following are confirmed working versions

- CMAKE 3.13 or higher
- Python 3.8.2 or higher
- AWS Cli 2
- Ninja 1.10.0

Python 2.7 should work, but if using Ubuntu 20.04 pip is no longer available, causing issues. This can be solved by ensuring Python3 is invoked through 'python' calls from PATH.

When running under Linux it may be required to symlink 'ninja' to 'ninja-build'.

### Notes

This project is based on AWS Demo Application, for more detailed prerequisite description please visit:
https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_espressif.html

## Initialization

```bash
    git submodule update --init --recursive
```

## Build

The provided bash script can be used to build on Linux

```
./build.sh
```

### Populate Credentials

The FSU-Eye needs some credentials in order to work, as it is intended to connect over WiFi and then further connect to a webservice.

#### WiFi

The FSU-Eye needs to connect to the local WiFi in order to operate correctly, in order to this the appropriate WiFi seetings needs to be provided. The programmer needs to edit the file config/wifi/fsu_eye_wifi_credentials.h with appropriate values, mainly for SSID and password, but in case other settings for your local WiFi differs from default, change these too.

#### AWS

In order to connect to AWS the device must first have been provisioned in on your AWS account - the necessary credentials needs then to be populated in the config/aws/fsu_eye_aws_credentials.h file. CAUTION: Never, ever, ever, upload any AWS credentials online as it can seriously jeopordize your wallet!

## Hardware

This project uses an ESP32-CAM with an OV2640 Camera. Example pictures can be seen below

![ESP32-CAM Front](/images/esp32-s_front.jpg)
![ESP32-CAM Back](/images/esp32-s_back_described.jpg)

NOTE: The pinout description on the board is on the reveresed side of the pins, which is why the handy pinout is written in the images.


## ESP32 Physical Connection

The physical connection to the ESP32-S can be done by connecting over UART using 3V3 logic, and powering it up with 5V. NOTE: During flashing the the pin IO0 needs to be connected to GND.

The pin connections to UART can be summarized as:
| ESP32-S | UART 3V3 Logic |
|---------|----------------|
|   GND   |      GND       |
|   U0R   |      TX        |
|   U0T   |      RX        |

In addition the power supply needs to connect:

| ESP32-S |  Power Supply |
|---------|---------------|
|   5V    |       5V      |

### Flashing

When flashing IO0 needs also to be connected to GND.

| ESP32-S | ESP32-S |
|---------|---------|
|   IO0   |   GND   |

An example picture of a connected device in flash mode can be seen below.
![ESP32-CAM Connected Flash Mode](/images/esp32-s_flash_connector_described.jpg)

Once the physical connection is setup you can flash using the provided bash script in the following way:

```
./build.sh flash
```

### OTA

Once you have flashed your initial firmware over UART you can, if desired, start to flash with OTA instead. There are some things that needs to be done in AWS, out of scope of this project, please read more in docs/OTA.md.

### Monitor

The device can be monitored from a HOST PC with the above explained physical connection. It is important that if the device was just flashed, the IO0 pin is disconnected from GND and the ESP32-S then reset.

The provided build.sh implements a wrapper for the monitor functionality. It is by default hardcoded to run on ttyUSB0, you may need to adjust this on your setup.
To monitor the device you can run the provided bash script in the following way:
```
./build.sh monitor
```

## Versioning

Git Tags are used in order to keep some kind of versioning of milestones. The format is  `<MAJOR>.<MINOR>`. Example on how to create and push a tag:

git tag -a 0.1 -m "Hello World!"

Once the tag is created it can be pushed to origin:

git push origin --tags

## Useful Links

https://randomnerdtutorials.com/esp32-cam-video-streaming-face-recognition-arduino-ide/

## TODO / Wanted list
- Doxygen
- Add a command-list to process in System Controller which it polls, callback should not do heavy processing
- All configurations to be placed in NVS, to be configurable over AWS connection
- WiFi setup over BLE
