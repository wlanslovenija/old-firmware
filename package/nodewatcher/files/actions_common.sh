#
# Common functions for nodewatcher OLSR actions.
#
GW_LOCK="/var/lock/gw_redirection"
MARK_TRAFFIC="/tmp/traffic_redirection_enabled"
MARK_DNS="/tmp/dns_redirection_enabled"
MARK_DNS_DOWN="/tmp/dns_servers_down"

iptables_retry()
{
  RESULT=0
  for _ in $(seq 1 5); do
    iptables $@ 2>&1
    RESULT=$?
    
    if [[ "${RESULT}" == "4" ]]; then
      # Error code 4 indicates a temporary resource problem, retry
      sleep 1
    else
      break
    fi
  done
  
  if [[ "${RESULT}" == "4" ]]; then
    # Unable to modify netfilter rules, state is undefined and we must reboot
    reboot
  elif [[ "${RESULT}" != "0" ]]; then
    # Some other error has ocurred, report it so nodewatcher will notice it
    echo "${RESULT}" > /tmp/iptables_redirection_problem
  fi
}

generate_rules()
{
  NF_ACTION="$1"
  
  iptables_retry -t nat -${NF_ACTION} PREROUTING -p tcp --dport 80 -s ${CLIENT_SUBNET} -d ${LOCAL_IP} -j CLIENT_REDIRECT
  iptables_retry -t nat -${NF_ACTION} PREROUTING -p tcp --dport 80 -s ${CLIENT_SUBNET} ! -d 10.254.0.0/16 -j CLIENT_REDIRECT
}

start_traffic_redirection()
{
  # Insert iptables rule to forward incoming HTTP traffic (only from client subnet)
  iptables_retry -t nat -N CLIENT_REDIRECT
  generate_rules "I"
  iptables_retry -t nat -A CLIENT_REDIRECT -p tcp --dport 80 -j DNAT --to-destination ${LOCAL_IP}:2051
  iptables_retry -I INPUT -p tcp --dport 2051 -j ACCEPT
  
  # Setup redirection enabled mark
  touch ${MARK_TRAFFIC}
}

stop_traffic_redirection()
{
  # Remove iptables redirection rule
  generate_rules "D"
  iptables_retry -t nat -F CLIENT_REDIRECT
  iptables_retry -t nat -X CLIENT_REDIRECT
  iptables_retry -D INPUT -p tcp --dport 2051 -j ACCEPT
  
  # Remove redirection enabled mark
  rm -f ${MARK_TRAFFIC}
}

start_dnsmasq()
{
  # Start dnsmasq and ensure that it has started
  for _ in $(seq 1 5); do
    /usr/sbin/dnsmasq $1
    
    if [[ "$?" == "0" ]]; then
      break
    else
      # Unable to start, sleep a second and retry
      sleep 1
    fi
  done
}

try_dns_redirection()
{
  WHICH="$1"
  
  # We only start redirection when both conditions are true
  if [[ "${WHICH}" == "traffic" ]]; then
    MARK="${MARK_DNS_DOWN}"
  elif [[ "${WHICH}" == "dns" ]]; then 
    MARK="${MARK_TRAFFIC}"
    
    # Setup DNS down flag
    touch ${MARK_DNS_DOWN}
  fi
  
  if [[ -f ${MARK} ]]; then
    start_dns_redirection
  fi
}

unmark_dns_down()
{
  rm -f ${MARK_DNS_DOWN}
}

start_dns_redirection()
{
  # Setup redirection enabled mark
  touch ${MARK_DNS}
  
  # Put dnsmasq into redirection mode
  LOCAL_IP="`uci get network.subnet0.ipaddr`"
  killall dnsmasq
  start_dnsmasq --address=/#/${LOCAL_IP}  
}

stop_dns_redirection()
{
  # Put dnsmasq into normal mode
  killall dnsmasq
  start_dnsmasq
  
  # Remove redirection enabled mark
  rm -f ${MARK_DNS}
}

init_action()
{
  # Acquire lock
  lock ${GW_LOCK}
  trap "lock -u ${GW_LOCK}" INT TERM EXIT
}

