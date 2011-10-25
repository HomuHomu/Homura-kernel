#!/bin/bash

# Set Default Path
TOP_DIR=$PWD
KERNEL_PATH=$TOP_DIR/kernel

# TODO: Set toolchain and root filesystem path
TOOLCHAIN="/home/ecoco/CodeSourcery452/bin/arm-none-eabi-"
ROOTFS_PATH="$TOP_DIR/initramfs_aosp"
export LOCALVERSION="-Homura-kernel"
export KBUILD_BUILD_VERSION="AOSP2"
export WHOAMI_MOD="Homura"
export HOSTNAME_MOD="Akemi"
TAR_NAME=Homura_kernel_$KBUILD_BUILD_VERSION.tar
ZIP_NAME=Homura_kernel_$KBUILD_BUILD_VERSION.zip

cd $KERNEL_PATH
make -j8 clean

# Copy Kernel Configuration#  File
# Generic Custum SC02C
cp -f $KERNEL_PATH/arch/arm/configs/Homura_aosp_defconfig $KERNEL_PATH/.config
make -j8 -C $KERNEL_PATH oldconfig || exit -1

find -name '*.ko' -exec cp -av {} $ROOTFS_PATH/lib/modules/ \;
cd $TOP_DIR
make -j8 -C $KERNEL_PATH ARCH=arm CROSS_COMPILE=$TOOLCHAIN CONFIG_INITRAMFS_SOURCE="$ROOTFS_PATH" || exit -1
cd $KERNEL_PATH
find -name '*.ko' -exec cp -av {} $ROOTFS_PATH/lib/modules/ \;
cd $TOP_DIR
make -j8 -C $KERNEL_PATH ARCH=arm CROSS_COMPILE=$TOOLCHAIN CONFIG_INITRAMFS_SOURCE="$ROOTFS_PATH" || exit -1

# Copy Kernel Image
cp -f $KERNEL_PATH/arch/arm/boot/zImage .
rm $TAR_NAME
rm $ZIP_NAME
tar --format=ustar -cf $TAR_NAME zImage
md5sum -t $TAR_NAME >> $TAR_NAME
mv $TAR_NAME $TAR_NAME.md5
sudo heimdall flash --kernel zImage --verbose
mv zImage ./Auto-sign/zImage
cd Auto-sign
cp aosp.zip $ZIP_NAME
zip $ZIP_NAME zImage
rm zImage
java -jar signapk.jar testkey.x509.pem testkey.pk8 $ZIP_NAME ../$ZIP_NAME
rm $ZIP_NAME
cd ../
