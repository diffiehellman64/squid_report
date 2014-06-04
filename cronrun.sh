#!/bin/bash
date=`date --date="1 month ago" +%Y%m`
log_dir="/var/log/squid3/archive/`date --date="1 month ago" +%Y`/`date --date="1 month ago" +%m`"

mkdir -p $log_dir
mv /var/log/squid3/access.log-$date*.gz $log_dir
zcat $log_dir/*.gz | squid_report -m /home/diff/sites.list | csvparseldap /home/samba/$date.csv `cat /etc/squid3/squid_pass`
