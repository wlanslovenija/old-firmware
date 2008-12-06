#!/bin/sh -e

PROGRAM="/usr/bin/solar"
BAUD="9600"
DEVICE="/dev/tts/1"

[ -x $PROGRAM ] || { echo "Missing program $PROGRAM."; exit 1; }

BATVOLTAGE=`$PROGRAM -d $DEVICE -b $BAUD -p batvoltage 2> /dev/null`
SOLVOLTAGE=`$PROGRAM -d $DEVICE -b $BAUD -p solvoltage 2> /dev/null`
CHARGE=`$PROGRAM -d $DEVICE -b $BAUD -p charge 2> /dev/null`
LOAD=`$PROGRAM -d $DEVICE -b $BAUD -p load 2> /dev/null`
STATE=`$PROGRAM -d $DEVICE -b $BAUD -p state 2> /dev/null`

/usr/bin/wget -O - -q "http://10.16.0.1/traffic/wifi/feed?solar=1&batvoltage=${BATVOLTAGE}&solvoltage=${SOLVOLTAGE}&charge=${CHARGE}&load=${LOAD}&state=${STATE}" >/dev/null
