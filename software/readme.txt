ESP32 Einrichten

- Drauf achten das genau der richtige Microprocessor ausgewählt ist. esp32-c3-devkitm-1
-



Config in: platformio.ini 

[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200