ESP32 Einrichten

- Drauf achten das genau der richtige Microprocessor ausgewählt ist. esp32-c3-devkitm-1
-



Config in: platformio.ini 

[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200






IRemote


Protocol=NEC Address=0x1308 Command=0x87 Repeat gap=96350us