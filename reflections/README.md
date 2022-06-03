# Reflections #

Various reflected attacks used in DDoS.

## Requirements ##

Python
Scapy (2.3.x)

## dns.py / dns6.py ##

Uses spoofed packets to request large responses from a recursive/cacheing DNS server.

## ntp.py / ntp6.py ##

Uses spoofed packets to request monlist from affected NTP servers.

## snmp.py / snmp6.py ##

Uses spoofed packets to make large snmpbulk requests from a server.

*Uses logic from http://www.prolexic.com/kcresources/white-paper/white-paper-snmp-ntp-chargen-reflection-attacks-drdos/analysis-of-drdos-dns-reflection-white-paper_i.html
*Uses improved request from http://www.nothink.org/misc/snmp_reflected.php