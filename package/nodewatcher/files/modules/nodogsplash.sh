#
# nodewatcher module
# NODOGSPLASH STATUS MODULE
#

# TODO This module should be moved to nodewatcher-nodogsplash package

# Module metadata
MODULE_ID="net.captive.nodogsplash"
MODULE_SERIAL=1

#
# Report output function
#
report()
{
  if [ ! -x /usr/bin/ndsctl ]; then
    return
  fi
  
  /usr/bin/ndsctl -s /var/run/nodogsplash.sock status >/dev/null 2>/dev/null

  if [ "$?" == "0" ]; then
    # TODO parse_nds.awk script should be moved to nodewatcher-nodogsplash package when this module
    #      is also moved
    /usr/bin/ndsctl -s /var/run/nodogsplash.sock clients | awk -f /etc/nodewatcher/parse_nds.awk
    show_entry_from_file "nds.down" /var/nds_status "0"
  else
    show_entry "nds.down" 1
  fi
}

