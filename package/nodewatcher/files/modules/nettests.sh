#
# nodewatcher module
# NETWORK TESTS REPORTING MODULE
#

# Module metadata
MODULE_ID="core.nettests"
MODULE_SERIAL=1

#
# Report output function
#
report()
{
  # DNS tests
  show_entry_from_file "dns.local" /var/dns_test_local "0"
  show_entry_from_file "dns.remote" /var/dns_test_remote "0"
  
  # Firewall problems
  show_entry_from_file "iptables.redirection_problem" /tmp/iptables_redirection_problem "0"
  
  # Connectivity loss counter
  show_entry_from_file "net.losses" /tmp/loss_counter "0"
}

