#!/bin/bash
sdcard="/dev/sdb"
sdcard_fat32="/dev/sdb1"
sdcard_ext3="/dev/sdb2"
sdcard_a2="/dev/sdb3"
pathsd="/home/anderson/projects/ex_codesign"
echo "FORMATTING SD CARD...."
sudo dd if=/dev/zero of=$sdcard bs=512 count=3
(echo n; echo p; echo 3; echo ; echo 4095; echo t; echo a2; \
echo n; echo p; echo 1; echo ; echo +32M; echo t; echo 1; echo b; \
echo n; echo p; echo 2; echo ; echo +512M; echo t; echo 2; echo 83; echo w;) | sudo fdisk $sdcard;
sudo mkfs.vfat $sdcard_fat32;
sudo mkfs.ext3 -F $sdcard_ext3;
sync
echo "COPYING SOURCES..."
sudo dd if=$pathsd/sd/a2/preloader-mkpimage.bin of=$sdcard_a2 bs=64K
sudo dd if=$pathsd/sd/fat32/soc_system.rbf of=$sdcard_fat32
sudo dd if=$pathsd/sd/fat32/u-boot.img of=$sdcard_fat32
sudo dd if=$pathsd/sd/fat32/u-boot.scr of=$sdcard_fat32
sudo dd if=$pathsd/sd/fat32/zImage of=$sdcard_fat32
sudo tar -xvf $pathsd/sd/ext3/rootfs.tar | sudo dd of=$sdcard_ext3
sync
