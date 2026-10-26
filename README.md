Firmware

Testing
load esp-idf : ~/esp/esp-idf/export.sh (getidf)

clean previous build    : idf.py fullclean
set target              : idf.py set-target <esp32s3>
build                   : idf.py build
flash / run             : idf.py -p /dev/cu.usbxxx flash monitor
