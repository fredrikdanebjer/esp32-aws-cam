# FSU-Eye

Frellie-Sees-You is a home survelleince project, intended to surveil your home with cameras.
This project part implements the Eye unit (imagination!) controllong the camera. It runs on a ESP32, connects to AWS and uploads images taken with its camera.

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

## Flashing

### ESP32 Physical Connection

In order to flash the ESP32 you will need to connect to the ESP32 over UART, as well as powering it up with 3V3.

The ESP32 UART pinout is as follows:

ESP32 UART GND GND U0T RX U0R TX

### Flash Script

Once the physical connection is setup you can flash using the provided bash script in the following way:

```
./build.sh flash
```
