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
    git submodule init
    git submodule update
```

## Build

The provided bash script can be used to build on Linux

```
./build.sh
```
