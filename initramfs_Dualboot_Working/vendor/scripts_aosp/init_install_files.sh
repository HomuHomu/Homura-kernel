#!/system/bin/sh

mount -o remount,rw /system /system

rm /system/lib/hw/sensors.smdkv310.so
cat /vendor/files/sensors.smdkv310.so > /system/lib/hw/sensors.smdkv310.so
chown 0.0 /system/lib/hw/sensors.smdkv310.so
chmod 644 /system/lib/hw/sensors.smdkv310.so

rm /system/lib/hw/*lights.smdkv310.so
cat /vendor/files/lights.smdkv310.so > /system/lib/hw/lights.smdkv310.so
chown 0.0 /system/lib/hw/lights.smdkv310.so
chmod 644 /system/lib/hw/lights.smdkv310.so

rm /system/fonts/truetypeEmoji.ttf
cat /vendor/files/truetypeEmoji.ttf > /system/fonts/truetypeEmoji.ttf
chown 0.0 /system/fonts/truetypeEmoji.ttf
chmod 644 /system/fonts/truetypeEmoji.ttf

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

mount -o remount,ro /system /system

