#!/bin/bash

cd /sys/kernel/config/usb_gadget/
mkdir -p loopback
cd loopback
echo 0xc0de > idVendor # code
echo 0xb10b > idProduct # blob
echo 0x0100 > bcdDevice # v1.0.0
echo 0x0200 > bcdUSB # USB2  
mkdir -p strings/0x409
echo "C0CAC01AADD511FE" > strings/0x409/serialnumber #cocacola adds life
echo "Tristan Dobrowolski" > strings/0x409/manufacturer
echo "Miniature Multi-functional Liquid Crystal Display" > strings/0x409/product
mkdir -p configs/c.1/strings/0x409
echo "16x2 LCD + 3 pushbuttons + buzzer" > configs/c.1/strings/0x409/configuration
echo 250 > configs/c.1/MaxPower

# Add functions here
mkdir -p functions/ffs.usb0
ln -s functions/ffs.usb0 configs/c.1/
# End functions