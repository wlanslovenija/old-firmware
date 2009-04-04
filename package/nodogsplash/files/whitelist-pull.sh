#!/bin/sh -e

trap 'killall ndsctl; exit' INT QUIT TERM EXIT

NDSCTL="/usr/bin/ndsctl -s /var/run/nodogsplash.sock"
WHITELIST_URL="http://10.14.0.2/feeds/whitelist"
WHITELIST="`wget -q -O - ${WHITELIST_URL} 2>/dev/null`"
CURRENT_TRUSTS="`${NDSCTL} status | awk 'BEGIN { in_trust = 0 } !/^=/ && in_trust == 1 { print $1 } /^Trusted MAC addresses/ { in_trust = 1 } /^=/ { in_trust = 0 }'`"

# Remove some MACs from trusted list that were removed from the whitelist
for trust in $CURRENT_TRUSTS; do
  if [ "`expr match \"${WHITELIST}\" \".*${trust}\"`" == "0" ]; then
    ${NDSCTL} untrust ${trust}
  fi
done

# Add MACs that are on the whitelist
for mac in $WHITELIST; do
  if [ "`expr match \"${CURRENT_TRUSTS}\" \".*${mac}\"`" == "0" ]; then
    ${NDSCTL} trust ${mac}
  fi
done
