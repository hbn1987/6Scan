# 6Scan Analysis Toolset

```shell
Legal_IP.py # Delete invalid IP addresses.

APD.py # APD algorithm.

AS_statistics_IPASN.py # AS statistics using Caida's IP2ASN.
CC_statistics_Maxmind.py # IPs to countries using Maxmind.

EUI64_analysis.py # EUI64 addresses analysis on the output of addr6 [cat <filename> | ipv6toolkit/addr6 -i -s -q].
EmbedIPv4_analysis.py # Testing the embedded-IPv4 addresses for AS consistency on the output of addr6.
non-Target_analysis.py # Non-Target reply analysis.
Entropy.py # Give the nybble-by-nybble entropy of IID

HitRate.py # Hit rate analysis.
ProbeType.py # Analyze the percentage of each type of returned probes.

data # IP2ASN, Maxmind, and IEEE OUI database.
```
