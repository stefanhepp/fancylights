#!/bin/bash
#
# Flash script for AtMega48 using avrdude on RaspberryPi
#
if [ "x$1" == "x" ]; then
    echo "Usage: $0 <hexfile>"
    exit 1
fi

sudo avrdude -c linuxgpio -p atmega48 -v -U flash:w:$1:i
