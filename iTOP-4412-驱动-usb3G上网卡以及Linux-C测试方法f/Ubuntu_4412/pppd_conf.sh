#!/bin/sh

pppd call wcdma  | tee  /usr/ppp.log &
sleep 8 
vgw=`tail  /usr/ppp.log |   grep  'remote IP address' | grep -m 1 -o '\([0-9]\{1,3\}\.\)\{3\}[0-9]\{1,3\}'`
vnamenserver=`tail   /usr/ppp.log |  grep  primary | grep -m 1  -o '\([0-9]\{1,3\}\.\)\{3\}[0-9]\{1,3\}'`
echo
echo
echo $vgw
echo $vnamenserver
echo "nameserver $vnamenserver" > /etc/resolv.conf
route add default gw $vgw

ifconfig eth0 down
