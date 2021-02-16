# KVS

The Key Value Store (KVS) is a service level component that allows storage and retreival of predefined key-value pairs. By utilizing the NVS of the ESP32 it is possible to store persistent data, so that a reconfigured device will stay reconfigured over power-cycles.

## Features
- Persistent data storage in NVS Flash
- RAM-based fast-access storage to minimize read disturbs
- Dictionary style key-value put/get operations
- Predefined pairs

## Entries

The defined entries of the KVS can be seen below

Key Name | Key Id | Desrcription
------ | ------ | ------
WiFi SSID | 0 | Name of the WiFi to conncet to
WiFi Password | 1 | WiFI Password to use
Image Report Interval | 2 | Interval in seconds to upload image to AWS
Info Report Interval | 3 | Interval in seconds to upload diagnostics to AWS
