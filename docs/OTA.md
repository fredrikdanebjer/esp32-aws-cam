# AWS OTA

FSU-Eye offers OTA functionality over AWS IoT Core Jobs, with rollback functionality if device fails to apply the OTA. Once you have flashed your initial firmware over UART you can, if desired, start to flash with OTA instead. To do so it is required that you have set up AWS structures accordingly.

## Prerequisites

There are several resources that needs to be created on your AWS in order to be able to apply the OTA, please read more at https://docs.aws.amazon.com/freertos/latest/userguide/ota-prereqs.html

## Versioning

The emebedded OTA Agent of Amazon FreeRTOS uses a version format of '<MAJOR>.<MINOR>.<BUILD>' and requires that a new firmware has a strictly higher version number than the currently operating one. This project uses the latest git tag for <MAJOR> and <MINOR>, then counts the number of commits made after that tag and uses for <BUILD>.
Note: For testing purposes the build command 'release' can be used to force a specific <BUILD> number.

## Code Signing

The FSU-Eye requires that any firmware image is signed and that the signing certificate is placed in 'config/aws/aws_ota_codesigner_certificate.h'.

For more information on how to generate the certificates and key, please refer to https://docs.aws.amazon.com/freertos/latest/userguide/burn-initial-firmware-esp.html

## Filepath

The filepath to apply the OTA Job to is the root, i.e. '.'.
