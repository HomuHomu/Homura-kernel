#!/system/bin/sh

for x in vendor/scripts_aosp/init*; do
	$x
done
