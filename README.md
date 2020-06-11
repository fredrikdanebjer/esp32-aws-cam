# FSU-Eye

Frellie-Sees-You is a home survelleince project, intended to surveil your home with cameras.
This project part implements the Eye unit (imagination!) controllong the camera. It runs on a ESP32, connects to AWS and uploads images taken with its camera.

## Prerequsites

- CMAKE 3.13 or higher
- Python 2.7.10 or higher
- AWS Cli

This project is based on AWS Demo Application, for more detailed prerequisite description please visit:
https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_espressif.html

## Initialization

```bash
    git clone https://github.com/sturmgans/esp32-aws-cam esp32-aws-cam
    cd esp32-aws-cam
    git submodule init
    git submodule update
```

## Build

WIP
