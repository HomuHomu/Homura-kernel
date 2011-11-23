#!/system/bin/sh
export PATH=/system/bin:/sbin:/vendor/bin:/system/sbin:/system/xbin
export LD_LIBRARY_PATH=/vendor/lib:/system/lib

for x in /system/etc/init.d/*; do
	logwrapper "$x"
done
