#!/bin/bash

# Set Default Path
TOP_DIR=$PWD
KERNEL_PATH=$TOP_DIR/kernel

# TODO: Set toolchain and root filesystem path
TOOLCHAIN="$TOP_DIR/arm-2011.03/bin/arm-none-eabi-"
ROOTFS_PATH="$TOP_DIR/initramfs_DCM"

KERNEL_NAME="Homura-Kernel"
CONFIG_FILE="$KERNEL_PATH/arch/arm/configs/Homura_defconfig"

export LOCALVERSION="-$KERNEL_NAME"
export KBUILD_BUILD_VERSION="DCM21b"
export WHOAMI_MOD="Homura"
export HOSTNAME_MOD="Akemi"

TAR_NAME="$KERNEL_NAME-$KBUILD_BUILD_VERSION.tar"
ZIP_NAME="$KERNEL_NAME-$KBUILD_BUILD_VERSION.zip"
CWM_ZIP="DCM.cwmzip"

cd $KERNEL_PATH
make -j8 clean
# Copy Kernel Configuration File
cp -f $CONFIG_FILE $KERNEL_PATH/.config
make -j8 -C $KERNEL_PATH oldconfig || exit -1

# Build Kernel
find -name '*.ko' -exec cp -av {} $ROOTFS_PATH/lib/modules/ \;
cd $TOP_DIR
make -j8 -C $KERNEL_PATH ARCH=arm CROSS_COMPILE=$TOOLCHAIN CONFIG_INITRAMFS_SOURCE="$ROOTFS_PATH" || exit -1
cd $KERNEL_PATH
find -name '*.ko' -exec cp -av {} $ROOTFS_PATH/lib/modules/ \;
cd $TOP_DIR
make -j8 -C $KERNEL_PATH ARCH=arm CROSS_COMPILE=$TOOLCHAIN CONFIG_INITRAMFS_SOURCE="$ROOTFS_PATH" || exit -1

# Copy Kernel Image
cp -f $KERNEL_PATH/arch/arm/boot/zImage .

rm $TAR_NAME.md5
rm $ZIP_NAME

# Create Odin File 
tar --format=ustar -cf $TAR_NAME zImage
md5sum -t $TAR_NAME >> $TAR_NAME
mv $TAR_NAME $TAR_NAME.md5

# Create CWM File
mv zImage ./Auto-sign/zImage
cd Auto-sign
cp $CWM_ZIP $ZIP_NAME
zip $ZIP_NAME zImage
java -jar signapk.jar testkey.x509.pem testkey.pk8 $ZIP_NAME ../$ZIP_NAME
rm $ZIP_NAME
mv zImage $ROOTFS_PATH

# Make Clean
cd $ROOTFS_PATH/lib/modules/
rm *
cd $KERNEL_PATH
make -j8 clean

# Heimdall Flash!
cd $ROOTFS_PATH
sudo heimdall flash --kernel zImage --verbose
rm zImage
