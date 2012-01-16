#!/sbin/busybox sh

/sbin/busybox mount -o remount,rw /system /system

/sbin/busybox rm /system/lib/hw/sensors.smdkv310.so
/sbin/busybox cat /vendor/files/sensors.smdkv310.so > /system/lib/hw/sensors.smdkv310.so
/sbin/busybox chown 0.0 /system/lib/hw/sensors.smdkv310.so
/sbin/busybox chmod 644 /system/lib/hw/sensors.smdkv310.so

lightsmd5sum=`/sbin/busybox md5sum /system/lib/hw/lights.smdkv310.so | /sbin/busybox awk '{print $1}'`
blnlightsmd5sum=`/sbin/busybox md5sum /vendor/files/lights.bln.so | /sbin/busybox awk '{print $1}'`
if [ "${lightsmd5sum}a" != "${blnlightsmd5sum}a" ]; then
    echo "Copying liblights"
    /sbin/busybox mv /system/lib/hw/lights.smdkv310.so /system/lib/hw/lights.smdkv310.so.BAK
    /sbin/busybox cp /vendor/files/lights.bln.so /system/lib/hw/lights.smdkv310.so
    /sbin/busybox chown 0.0 /system/lib/hw/lights.smdkv310.so
    /sbin/busybox chmod 644 /system/lib/hw/lights.smdkv310.so
fi;

/sbin/busybox rm /system/fonts/truetypeEmoji.ttf
/sbin/busybox cat /vendor/files/truetypeEmoji.ttf > /system/fonts/truetypeEmoji.ttf
/sbin/busybox chown 0.0 /system/fonts/truetypeEmoji.ttf
/sbin/busybox chmod 644 /system/fonts/truetypeEmoji.ttf

/sbin/busybox rm /system/app/CWMManager.apk
/sbin/busybox rm /data/dalvik-cache/*CWMManager*
/sbin/busybox rm /data/app/eu.chainfire.cfroot.cwmmanager*.apk
/sbin/busybox cat /vendor/files/CWMManager.apk > /system/app/CWMManager.apk
/sbin/busybox chown 0.0 /system/app/CWMManager.apk
/sbin/busybox chmod 644 /system/app/CWMManager.apk

/sbin/busybox rm /system/app/TweakGS2.apk
/sbin/busybox rm /data/dalvik-cache/*TweakGS2*
/sbin/busybox rm /data/app/net.sakuramilk.TweakGS2*.apk
/sbin/busybox cat /vendor/files/TweakGS2.apk > /system/app/TweakGS2.apk
/sbin/busybox chown 0.0 /system/app/TweakGS2.apk
/sbin/busybox chmod 644 /system/app/TweakGS2.apk

/sbin/busybox rm /system/app/SharedAppManager.apk
/sbin/busybox rm /data/dalvik-cache/*SharedAppManager*
/sbin/busybox rm /data/app/jp.ma34.SharedAppManager*.apk
/sbin/busybox cat /vendor/files/SharedAppManager.apk > /system/app/SharedAppManager.apk
/sbin/busybox chown 0.0 /system/app/SharedAppManager.apk
/sbin/busybox chmod 644 /system/app/SharedAppManager.apk

# copy sqlite3
/sbin/busybox cat /vendor/files/sqlite3 > /system/bin/sqlite3

# set sqlite3 permissions
/sbin/busybox chown 0.2000 /system/bin/sqlite3
/sbin/busybox chmod 775 /system/bin/sqlite3

# tether_dun_required 0
r=`/system/bin/sqlite3 /data/data/com.android.providers.settings/databases/settings.db "select * from secure where name='tether_dun_required';"`
if [ "$r" == "" ]; then
/system/bin/sqlite3 /data/data/com.android.providers.settings/databases/settings.db "insert into secure values (null, 'tether_dun_required','0');"
else
/system/bin/sqlite3 /data/data/com.android.providers.settings/databases/settings.db "update secure set value='0' where name='tether_dun_required';"
fi
/sbin/busybox rm /system/bin/sqlite3

# Bravia Engine Install
/sbin/busybox rm /system/etc/be_movie
/sbin/busybox rm /system/etc/be_photo
/sbin/busybox cat /vendor/files/be_movie > /system/etc/be_movie
/sbin/busybox cat /vendor/files/be_photo > /system/etc/be_photo
/sbin/busybox chmod 0755 /system/etc/be_movie
/sbin/busybox chmod 0755 /system/etc/be_photo
/sbin/busybox chown 0.0 /system/etc/be_movie
/sbin/busybox chown 0.0 /system/etc/be_photo

# Carrier indicate 1line
/sbin/busybox rm  /system/etc/spn-conf.xml
/sbin/busybox cat /vendor/files/spn-conf.xml > /system/etc/spn-conf.xml
/sbin/busybox chown 0.0 /system/etc/spn-conf.xml
/sbin/busybox chmod 644 /system/etc/spn-conf.xml

/sbin/busybox mount -o remount,ro /system /system

