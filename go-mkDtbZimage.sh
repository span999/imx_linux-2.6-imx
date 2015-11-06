#!/bin/sh
#
# This script helps to create binary of zImage+dtb image ...
#
# flash zImage_dtb image to sd card (6.5MB should be good)
# sudo dd if=./zImage-dtb.bin of=/dev/sd? bs=512 seek=2048 count=13312 conv=fsync
# 
# for coresponding uboot flashing
# sudo dd if=./u-boot_derek0206.imx of=/dev/sd? bs=512 seek=2 conv=fsync# uboot parameters
# setenv sdbootargs 'setenv bootargs console=${console},${baudrate} ${smp} root=/dev/mmcblk0p1 rootwait rw'
# setenv sdboot 'echo Booting from sdcard ...; run sdbootargs; mmc dev 0; mmc read ${loadaddr} 0x800 0x3800; bootz ${loadaddr} - 0x12600000;'
# 
# 


CURROOT=`pwd`
BUILDDIR=${CURROOT}/../kernel/fsl-yocto-kernel_3.10.53
TARDTB=imx6q-sabresd-ldo_n488.dtb
TARZMG=zImage
TARBIN=zImage-dtb.bin


# remove old target file.
rm -f ${TARBIN}

# copy source binary
find ../kernel/fsl-yocto-kernel_3.10.53 -name ${TARZMG} -exec cp -f {} ./ \;
find ../kernel/fsl-yocto-kernel_3.10.53 -name ${TARDTB} -exec cp -f {} ./ \;

if [ ! -f ${TARZMG} ]; then
	echo "exit!! ${TARZMG} not exist!!"
	exit 1
fi
if [ ! -f ${TARDTB} ]; then
	echo "exit!! ${TARDTB} not exist!!"
	exit 2
fi

# create empty target file - 7MB
dd if=/dev/zero of=./zImage-dtb.bin bs=512 count=14336 conv=fsync

# copy kernel image into target file @ 0block=0MB
dd if=./zImage of=./zImage-dtb.bin bs=512 conv=fsync,notrunc

# copy device tree dtb file into target file @ 12288block=6MB
dd if=./imx6q-sabresd-ldo_n488.dtb of=./zImage-dtb.bin bs=512 seek=12288 conv=fsync,notrunc


# _E_O_F_
