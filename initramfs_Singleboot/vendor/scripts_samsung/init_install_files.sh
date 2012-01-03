#!/system/bin/sh
DEST=/system/xbin/su

mount -o remount,rw /system /system

rm /system/bin/su
rm /system/xbin/su
cat /vendor/files/su-3.0.3 > $DEST
chown 0.0 /system/xbin/su
chmod 6755 /system/xbin/su

rm /system/lib/hw/lights.SC-02C.so
cat /vendor/files/lights.bln.so > /system/lib/hw/lights.SC-02C.so
chown 0.0 /system/lib/hw/lights.SC-02C.so
chmod 644 /system/lib/hw/lights.SC-02C.so

rm /system/app/CWMManager.apk
rm /data/dalvik-cache/*CWMManager.apk*
rm /data/app/eu.chainfire.cfroot.cwmmanager*.apk
cat /vendor/files/CWMManager.apk > /system/app/CWMManager.apk
chown 0.0 /system/app/CWMManager.apk
chmod 644 /system/app/CWMManager.apk

rm /system/app/TweakGS2.apk
rm /data/dalvik-cache/*TweakGS2.apk*
rm /data/app/net.sakuramilk.TweakGS2*.apk
cat /vendor/files/TweakGS2.apk > /system/app/TweakGS2.apk
chown 0.0 /system/app/TweakGS2.apk
chmod 644 /system/app/TweakGS2.apk

rm /system/app/AppWidgetPicker.apk
rm /data/dalvik-cache/*AppWidgetPicker.apk*
cat /vendor/files/AppWidgetPicker-1.2.3.apk > /system/app/AppWidgetPicker.apk
chown 0.0 /system/app/AppWidgetPicker.apk
chmod 644 /system/app/AppWidgetPicker.apk

# copy sqlite3
cat /vendor/files/sqlite3 > /system/bin/sqlite3

# set sqlite3 permissions
chown 0.2000 /system/bin/sqlite3
chmod 775 /system/bin/sqlite3

# tether_dun_required 0
r=`/system/bin/sqlite3 /data/data/com.android.providers.settings/databases/settings.db "select * from secure where name='tether_dun_required';"`
if [ "$r" == "" ]; then
/system/bin/sqlite3 /data/data/com.android.providers.settings/databases/settings.db "insert into secure values (null, 'tether_dun_required','0');"
else
/system/bin/sqlite3 /data/data/com.android.providers.settings/databases/settings.db "update secure set value='0' where name='tether_dun_required';"
fi
rm /system/bin/sqlite3

# Bravia Engine Install
rm /system/etc/be_movie
rm /system/etc/be_photo
cat /vendor/files/be_movie > /system/etc/be_movie
cat /vendor/files/be_photo > /system/etc/be_photo
chmod 0755 /system/etc/be_movie
chmod 0755 /system/etc/be_photo
chown 0.0 /system/etc/be_movie
chown 0.0 /system/etc/be_photo

# Carrier indicate 1line
rm  /system/etc/spn-conf.xml
cat /vendor/files/spn-conf.xml > /system/etc/spn-conf.xml
chown 0.0 /system/etc/spn-conf.xml
chmod 644 /system/etc/spn-conf.xml

mount -o remount,ro /system /system

