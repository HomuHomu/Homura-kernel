#!/sbin/busybox sh

MMC=`ls -d /sys/block/mmc*`;

/sbin/busybox cp /data/user.log /data/user.log.bak
/sbin/busybox rm /data/user.log
exec >>/data/user.log
exec 2>&1

echo $(date) START of post-init.sh

##### Early-init phase #####

# IPv6 privacy tweak
  echo "2" > /proc/sys/net/ipv6/conf/all/use_tempaddr

# Remount all partitions with noatime
  for k in $(/sbin/busybox mount | /sbin/busybox grep relatime | /sbin/busybox cut -d " " -f3)
  do
        sync
        /sbin/busybox mount -o remount,noatime $k
  done

# Remount ext4 partitions with optimizations
  for k in $(/sbin/busybox mount | /sbin/busybox grep ext4 | /sbin/busybox cut -d " " -f3)
  do
        sync
        /sbin/busybox mount -o remount,commit=15 $k
  done
  
# EXT4 Speed Tweaks
/sbin/busybox mount -o noatime,remount,rw,discard,barrier=0,commit=60,noauto_da_alloc,delalloc /cache /cache;
/sbin/busybox mount -o noatime,remount,rw,discard,barrier=0,commit=60,noauto_da_alloc,delalloc /data /data;
  
# Miscellaneous tweaks
  echo "1500" > /proc/sys/vm/dirty_writeback_centisecs
  echo "200" > /proc/sys/vm/dirty_expire_centisecs
  echo "0" > /proc/sys/vm/swappiness
  
# Thunderbolt! CFS Tweaks - by pikachu01
  sysctl -w kernel.sched_min_granularity_ns=200000;
  sysctl -w kernel.sched_latency_ns=400000;
  sysctl -w kernel.sched_wakeup_granularity_ns=100000;

# SD cards (mmcblk) read ahead tweaks
  echo "1024" > /sys/devices/virtual/bdi/179:0/read_ahead_kb
  echo "1024" > /sys/devices/virtual/bdi/179:16/read_ahead_kb

# TCP tweaks
  echo "0" > /proc/sys/net/ipv4/tcp_timestamps;
  echo "1" > /proc/sys/net/ipv4/tcp_tw_reuse;
  echo "1" > /proc/sys/net/ipv4/tcp_sack;
  echo "1" > /proc/sys/net/ipv4/tcp_tw_recycle;
  echo "1" > /proc/sys/net/ipv4/tcp_window_scaling;
  echo "5" > /proc/sys/net/ipv4/tcp_keepalive_probes;
  echo "30" > /proc/sys/net/ipv4/tcp_keepalive_intvl;
  echo "30" > /proc/sys/net/ipv4/tcp_fin_timeout;
  echo "404480" > /proc/sys/net/core/wmem_max;
  echo "404480" > /proc/sys/net/core/rmem_max;
  echo "256960" > /proc/sys/net/core/rmem_default;
  echo "256960" > /proc/sys/net/core/wmem_default;
  echo "4096 16384 404480" > /proc/sys/net/ipv4/tcp_wmem;
  echo "4096 87380 404480" > /proc/sys/net/ipv4/tcp_rmem;

# UI tweaks
setprop debug.performance.tuning 1; 
setprop video.accelerate.hw 1;
setprop debug.sf.hw 1;

# Enable SCHED_MC
  echo "1" > /sys/devices/system/cpu/sched_mc_power_savings

# Enable AFTR
echo "3" > /sys/module/cpuidle/parameters/enable_mask

# Hotplug thresholds
echo "35" > /sys/module/pm_hotplug/parameters/loadl
echo "75" > /sys/module/pm_hotplug/parameters/loadh
echo "200" > /sys/module/pm_hotplug/parameters/rate

# Renice kswapd0 - kernel thread responsible for managing the memory
renice 2 `pidof kswapd0`

# New scheduler tweaks + readahead tweaks
for k in $MMC;
do
	if [ -e $i/queue/iostats ];
	then
		echo "0" > $k/queue/iostats;
	fi;
	if [ -e $i/queue/read_ahead_kb ];
	then
		echo "256" >  $i/queue/read_ahead_kb;
	fi;
done;

# Misc Kernel Tweaks
sysctl -w vm.vfs_cache_pressure=70
echo "8" > /proc/sys/vm/page-cluster;
echo "64000" > /proc/sys/kernel/msgmni;
echo "64000" > /proc/sys/kernel/msgmax;
echo "10" > /proc/sys/fs/lease-break-time;
echo "500 512000 64 2048" > /proc/sys/kernel/sem;

