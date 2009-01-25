#!/bin/sh

# Sleep a random amount of time
TIME=$RANDOM
let "TIME = TIME % 30"
sleep $TIME

# Restart nodogsplash if down
NDS_CHECK=`ndsctl -s /var/run/nodogsplash.sock status 2>&1 | grep "not started"`
if [ "$NDS_CHECK" != "" ]; then
  /etc/init.d/nodogsplash stop
  sleep 3
  /etc/init.d/nodogsplash start
  sleep 10
fi

# Push AP status
LOCALTIME=`date +%s`
UPTIME=`cat /proc/uptime | awk '{ print $1 }'`
TRANSFERRED=`cat /proc/net/dev | awk -F: '!/\|/ { gsub(/[[:space:]]*/, "", $1); split($2, a, " "); printf("&iface_%s=%d", $1, a[1]) }'`
BSSID=`iwconfig 2>/dev/null | grep Cell | awk '{ print $5 }'`
ESSID=`iwconfig | grep ESSID | awk '{ split($4, a, "\""); printf("%s", a[2]); }'`
VERSION=`cat /etc/version`
wget -O - -q "http://10.16.0.1/traffic/wifi/feed?status=1&bssid=${BSSID}&essid=${ESSID}&version=${VERSION}&local_time=${LOCALTIME}&uptime=${UPTIME}${TRANSFERRED}" >/dev/null

# Push client list
CMDS=`ndsctl -s /var/run/nodogsplash.sock clients | awk -f /etc/nodogsplash/parse_nds.awk`
eval "$CMDS"
