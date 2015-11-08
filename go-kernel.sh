#!/bin/sh
#


#_BTYPE_=android
_ARCH_=arm
#_CC_=/home/span/workshop/bin/toolchains/poky-1.6.2/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
_CC_=/home/span/workshop/bin/poky-x86_64-core-image/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
_CCC_="ccache ${_CC_}"

_GITREPO_=https://spanliu@bitbucket.org/sp_linuxkernel/linux-2.6-imx.git
_GITBRANCH_=rel_imx_3.10.53_1.1.0_ga_n488

_DATE_=`date +%y%m%d-%H%M%S`
#_DEFC_=imx_v7_defconfig
_DEFC_=imx_v7_N488_defconfig

if [ "${_BTYPE_}" = "android" ]; then
	_GITBRANCH_=kk4.4.3_2.0.0-ga_n488
	_DEFC_=imx_v7_android_defconfig
fi

_BASEDIR_=`pwd`
#_KERNDIR_=${_BASEDIR_}/3.10.53
_KERNFOLD_=fsl-yocto-kernel_3.10.53_n488
_KERNDIR_=${_BASEDIR_}/${_KERNFOLD_}
_LOGNAME_=3.10.53.n488.build-${_DATE_}.log
_LOGFILE_=${_BASEDIR_}/${_LOGNAME_}

cd ${_BASEDIR_}
echo "clear file..." > ${_LOGFILE_}
if [ ! -d ${_KERNFOLD_}/.git ]; then
	rm -rf ${_KERNFOLD_}
	#git clone -b ${_GITBRANCH_} --single-branch ${_GITREPO_} ${_KERNFOLD_}
	git clone ${_GITREPO_} ${_KERNFOLD_}
fi

cd ${_KERNFOLD_}
git checkout ${_GITBRANCH_}

cd ${_KERNDIR_}
echo ""
echo "now @`pwd`"
basename `pwd`

echo ""
echo "<<< make kernel image >>>"
echo "<<< use defconfig=${_DEFC_} >>>"
echo "<<< make kernel image >>>" >> ${_LOGFILE_}
echo "<<< use defconfig=${_DEFC_} >>>" >> ${_LOGFILE_}
make distclean
make clean
find ./ -name \zImage -exec rm {} \+
find ./ -name \*.dtb -exec rm {} \+
echo ">>>" >> ${_LOGFILE_}
echo "make ${_DEFC_} ARCH=${_ARCH_} CROSS_COMPILE=${_CCC_}" >> ${_LOGFILE_}
echo "<<<" >> ${_LOGFILE_}
echo "------------------------------------------------------------------------------------" >> ${_LOGFILE_}
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
