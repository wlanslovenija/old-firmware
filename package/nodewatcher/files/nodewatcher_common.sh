#
# Nodewatcher functions library
#
. /etc/functions.sh

get_client_subnets()
{
  client_subnets=
  
  config_cb() {
    local ipaddr netmask
    config_get enttype "$CONFIG_SECTION" TYPE

    if [[ "$enttype" == "alias" && "`echo $CONFIG_SECTION | grep subnet`" != "" ]]; then
      config_get ipaddr "$CONFIG_SECTION" ipaddr
      config_get netmask "$CONFIG_SECTION" netmask
      append client_subnets "$ipaddr/$netmask"
    fi
  }

  config_load network
}

get_local_ip()
{
  LOCAL_IP="`uci get network.subnet0.ipaddr`"
}

