#!/bin/sh
#

_ARCH_=arm
_CC_=/home/span/workshop/bin/toolchains/poky-1.6.2/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
_CCC_="ccache ${_CC_}"
#_DEFC_=imx_v7_defconfig
_DEFC_=imx_v7_N488_defconfig
_BASEDIR_=`pwd`
_KERNDIR_=${_BASEDIR_}/3.10.53
_LOGNAME_=3.10.53.n488.build.log
_LOGFILE_=${_BASEDIR_}/3.10.53.n488.build.log

echo "clear file..." > ${_LOGFILE_}
cd ${_KERNDIR_}
echo ""
echo "now @`pwd`"
basename `pwd`

echo ""
echo "<<< make kernel image >>>"
echo "<<< use defconfig=${_DEFC_} >>>"
make distclean
make clean
find ./ -name \zImage -exec rm {} \+
find ./ -name \*.dtb -exec rm {} \+
echo ">>>" >> ${_LOGFILE_}
echo "make ${_DEFC_} ARCH=${_ARCH_} CROSS_COMPILE=${_CCC_}" >> ${_LOGFILE_}
echo "<<<" >> ${_LOGFILE_}
make ${_DEFC_} ARCH=${_ARCH_} CROSS_COMPILE="${_CCC_}" 2>&1 | tee -a ${_LOGFILE_}
echo ">>>" >> ${_LOGFILE_}
echo "make -j4 ARCH=${_ARCH_} CROSS_COMPILE=${_CCC_}" >> ${_LOGFILE_}
echo "<<<" >> ${_LOGFILE_}
make -j4 ARCH=${_ARCH_} CROSS_COMPILE="${_CCC_}" 2>&1 | tee -a ${_LOGFILE_}
echo "<<< make kernel image done >>>"
echo "<<< use defconfig=${_DEFC_} >>>"
echo ""


echo ""
echo "<<< make kernel device tree >>>"
echo ">>>" >> ${_LOGFILE_}
echo "make dtbs -j4 ARCH=${_ARCH_} CROSS_COMPILE=${_CCC_}" >> ${_LOGFILE_}
echo "<<<" >> ${_LOGFILE_}
make dtbs -j4 ARCH=${_ARCH_} CROSS_COMPILE="${_CCC_}" 2>&1 | tee -a ${_LOGFILE_}
echo "<<< make kernel device tree done >>>"
echo ""

cd ${BASEDIR}
echo ""
echo "now @`pwd`"
basename `pwd`
find ${_KERNDIR_} -name \zImage
find ${_KERNDIR_} -name \*n488*.dtb

##_E_O_F_
