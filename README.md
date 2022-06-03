# 6Scan

Towards Efficient Internet-wide Scanning in IPv6 Network

## Description

Efficient network scanner which could scan the IPv6 network using various search strategies.

## Require

```shell
apt-get install libcurl4-openssl-dev
apt-get install libjsoncpp-dev
apt-get install autoconf
apt-get install zlib1g-dev
```

## Build

```shell
./bootstrap
./configure
make
```

## Usage

### Option

```shell
-D [hitlist/alias/country_xx/as_xxxx] # Download the IPv6 hitlist/aliased prefixes/country prefix resource list/AS announced prefixes.
-P # Pre-scan the hitlist.
-t [ICMP6/TCP6_SYN/TCP6_ACK/UDP6] # Set probe type.
-I # Network interface to use.
-l [country_xx/as_xxxx] # Specify the region of the seeds using 2-digit ISO-3166 country code or AS number.
-s [HMap6/6Scan/6Hit/6Tree/6Gen/Heuristic] # Set IPv6 searching strategy.
-A [country-xx/as-xxxx] # Active search and alias resolution within a region using heuristic algorithm.
-C [Active address file e.g., ./output/raw_ICMP6_yyyymmdd] # Address classification.
-H [country/as] # Set the regional level for data analysis.
-d # Output with the probe type
-r # Probing rate, 100Kpps by default.
-b # Probe budget, 10M by default.
```

### Example

```shell
./6scan -D hitlist
# Download the IPv6 hitlist from https://ipv6hitlist.github.io/.
./6scan -D alias
# Download the aliased prefixes from https://ipv6hitlist.github.io/.
./6scan -D country_cn
# Download ASNs, IPv4 ranges and IPv6 prefixes associated with China from https://stat.ripe.net/docs/data_api/.
./6scan -D as_3333
# Download all announced prefixes associated with AS 3333 from https://stat.ripe.net/docs/data_api/.

./6scan -P -t ICMP6 -I [interface e.g., enp2s0f0]
# Pre-scan the latest local hitlist to make the seedset using ICMPv6 packet.
./6scan -A country_cn -t ICMP6 -I [interface e.g., enp2s0f0]
# Active search and alias resolution of Chinese IPv6 address resources with ICMPv6 packet.

./6scan -t ICMP6 -I [interface e.g., enp2s0f0] -s 6Scan
# IPv6 Internet-wide scanning with 6Scan strategy using the latest local ICMPv6 seedset.
./6scan -t UDP6 -I [interface e.g., enp2s0f0] -s 6Hit
# IPv6 Internet-wide scanning with 6Hit strategy using the latest local UDPv6 seedset.
./6scan -t UDP6 -I [interface e.g., eth0] -s 6Tree -l country_cn
# IPv6 country-level scanning with 6Tree strategy using the latest local seedset collected from the anounced BGP prefixes of China.
./6scan -t UDP6 -I [interface e.g., eth0] -s 6Scan -F [seedfile e.g., Gasser_country_in_ICMP6_yyyymmdd]
# Specifies the seedset for scanning using 6Scan strategy.

./6scan -R [Reginal hitlist file e.g., ./output/hitlist_country_in_ICMP6_yyyymmdd]
# Remove the alias addresses in the reginal hitlist file.
./6scan -H -t ICMP6 -L country
# Analyze the regional distribution of addresses that respond to ICMPv6 packets in the Gasser's hitlist(seedset).
./6scan -H -L alias
# Compare the alias prefixes we obtained with Gasser's alias prefix list.

./6scan -H -t ICMP6 -l country_cn
# Get the Chinese seeds from Gasser's hitlist and our heuristic search strategy (i.e., mixed seedset).
./6scan -C [Active address file e.g., ./output/raw_6Scan_ICMP6_yyyymmdd] 
# Classify the active address in the file of raw_6Scan_ICMP6_yyyymmdd.
```

## Output

6Scan outputs the collected active addresses, the scanning information in the file that begin with raw_\
and the address classification results in the file that begin with results_.

|Information|Description|
|:---:|---|
|Strategy|searching strategy|
|Packet type|packet type|
|Budget|probing packet limit|
|Probing rate|number of packets sent per second|
|Received ratio|ratio of total number of received packets to the budget|
|Hit active addresses|hit active addresses|
|Discovered new addresses|found active addresses after removing the seeds and the aliased addresses|
|Time cost|time cost of the preparation and scanning time|
|Address classification|IID allocation schemas i.e., Alias, Small-integer, Embedded-IPv4, EUI-64, and Other|

## Reference

### Scanner

>R. Beverly, R. Durairajan, D. Plonka, and J. P. Rohrer, “In the IP of the Beholder: Strategies for Active IPv6 Topology Discovery,” in IMC, 2018.

### IPv6 hitlist

>O. Gasser, Q. Scheitle, S. Gebhard, and G. Carle, “Scanning the IPv6 Internet: Towards a Comprehensive Hitlist,” in TMA, 2016.

### Search strategy

>B. Hou, Z. Cai, K. Wu, J. Su, and Y. Xiong, “6Hit: A Reinforcement Learning-based Approach to Target Generation for Internet-wide IPv6 Scanning,” in INFOCOM, 2021. \
>Z. Liu, Y. Xiong, X. Liu, W. Xie, and P. Zhu, “6Tree: Efficient Dynamic Discovery of Active Addresses in the IPv6 Address Space,” Comput. Networks, 2019. \
>A. Murdock, F. Li, P. Bramsen, Z. Durumeric, and V. Paxson, “Target Generation for Internet-Wide IPv6 Scanning,” in IMC, 2017.

### Alias resolution

>O. Gasser et al., “Clusters in the Expanse: Understanding and Unbiasing IPv6 Hitlists,” in IMC, 2018.
