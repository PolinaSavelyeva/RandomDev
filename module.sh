#!/bin/bash
module_name="mymodule"
device_name="randomdev"
path_dev="/dev/$device_name"

sudo rmmod "$module_name"
sudo rm "$path_dev"
make &&
sudo dmesg -C &&
sudo insmod "$module_name".ko &&
sudo dmesg &&
sudo mknod "$path_dev" c 238 0 &&
sudo chmod a+w "$path_dev" &&
printf "\x03\x05\x02\x01\x03\x03\x07\x04" > "$path_dev" &&
cat "$path_dev"
