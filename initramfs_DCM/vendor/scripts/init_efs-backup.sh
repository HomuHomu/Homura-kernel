#!/sbin/busybox sh
# EFS Backup script

if [ ! -d /sdcard/Homura ]; then
  /sbin/busybox mkdir  /sdcard/Homura
else
	echo "Homura folder already exists"
fi;

if [ ! -f /sdcard/Homura/efsbackup.tar.gz ];
then
  /sbin/busybox tar zcvf /sdcard/Homura/efsbackup.tar.gz /efs
  /sbin/busybox dd if=/dev/block/mmcblk0p1 of=/sdcard/Homura/efsdev-mmcblk0p1.img bs=4096
  /sbin/busybox gzip /sdcard/Homura/efsdev-mmcblk0p1.img
else
	echo "efs backup already exists"
fi;
