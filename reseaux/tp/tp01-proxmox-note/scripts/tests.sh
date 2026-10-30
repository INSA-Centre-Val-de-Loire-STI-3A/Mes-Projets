#!/bin/sh
# Commandes de vérification, à lancer sur la machine indiquée.
# Usage : sh tests.sh <client1|client2|cloneTest>
set -u

case "${1:-}" in
client1)
    ip -br addr
    ip route
    sysctl net.ipv4.ip_forward
    nft list ruleset
    systemctl --no-pager status isc-dhcp-server ssh
    ping -c 2 10.10.2.2
    ping -c 2 10.10.3.2
    ;;
client2)
    ip route
    ping -c 2 10.10.3.2
    ping -c 2 8.8.8.8
    named-checkconf
    named-checkzone tienou-tp.insa-cvl.fr /etc/bind/db.tienou-tp.insa-cvl.fr
    dig @127.0.0.1 client3.tienou-tp.insa-cvl.fr +short
    dig @127.0.0.1 dns.google.com +short
    ;;
cloneTest)
    ip -br addr
    ip route
    cat /etc/resolv.conf
    ping -c 2 dns.google.com
    dig client3.tienou-tp.insa-cvl.fr +short
    curl -s http://client3.tienou-tp.insa-cvl.fr | head -5
    ;;
*)
    echo "usage: $0 <client1|client2|cloneTest>" >&2
    exit 1
    ;;
esac
