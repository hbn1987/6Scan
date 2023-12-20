# HScan6

Towards Efficient Internet-wide Probing in IPv6 Networks.

## Description

Efficient IPv6 Ping Scanner Integrates Dynamic Search Algorithms (HScan6-DSA4p) and Target Generation Algorithms (HScan6-TGA).

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
-s [HMap6/6Scan/6Hit/6Tree/6Gen] # Set TGA searching strategy, e.g., '-s HMap6' denotes HScan6-TGA method.
-A [country-xx/as-xxxx] # Perform HScan6-DSA4p in a designated region.
-F # Specifies the seedset file.
-d # Output with the probe type i.e., (Target, Responder, Probe).
-r # Probing rate, 100Kpps by default.
-b # Probe budget, 100M by default.
-k # Specifies the probe number for each iteration in the heuristic ping.
```

### Example

```shell
./6scan -D hitlist
# Download the IPv6 hitlist from https://ipv6hitlist.github.io/.
./6scan -D alias
# Download the aliased prefixes from https://ipv6hitlist.github.io/.
./6scan -D country_CN
# Download ASNs, IPv4 ranges and IPv6 prefixes associated with China from https://stat.ripe.net/docs/data_api/.
./6scan -D as_3333
# Download all announced prefixes associated with AS 3333 from https://stat.ripe.net/docs/data_api/.

./6scan -P -t ICMP6 -I [interface e.g., eth0]
# Pre-scan the latest local hitlist to make the seedset using ICMPv6 probes.
./6scan -A country_CN -t ICMP6 -I [interface e.g., eth0] -k 10 -d
# Performing dynamic ping (HScan6-DSA4p) with a de-aliasing strategy on Chinese IPv6 prefixes using ICMPv6 probes, consisting of 10 probes in each subprefix, and identifying the responding probe type.

./6scan -t ICMP6 -I [interface e.g., eth0] -s HMap6
# IPv6 Internet-wide scanning with HScan6-TGA method using the latest local ICMPv6 seedset.
./6scan -t UDP6 -I [interface e.g., eth0] -s 6Hit
# IPv6 Internet-wide scanning with 6Hit strategy using the latest local UDPv6 seedset.
./6scan -t UDP6 -I [interface e.g., eth0] -s 6Scan -F [seedfile]
# Specifies the seedset for scanning using 6Scan strategy.

./6scan -H -t ICMP6 -l country_CN
# Get the Chinese seeds from Gasser et al's hitlist and our heuristic search strategy (i.e., mixed seedset).

./6scan -C [address file e.g., ./output/raw_6Scan_ICMP6_yyyymmdd] 
# Remove the alias address in the file of raw_6Scan_ICMP6_yyyymmdd.
```

### Scan in a run

```shell
# Step 1: Heuristic seed collection (HScan6-ping)
python3 runner/1Hseed.py
# Step 2: Organize alias prefix list 
python3 runner/2Aliasp.py
# Step 3: Create the seedset 
python3 runner/3Mseed.py
# Step 4: Evaluate the scan results 
python3 runner/4Evalu.py
```

### Distributed-scan

```shell
# Step 1: Dispatch the seeds and alias list
python3 scheduling/seeds_dispatch.py
# Step 2: Pre-scan the seeds and ping-like scanning
python3 scheduling/ssh_runner.py
# Step 3: Data pullback
python3 scheduling/data_pullback.py
```

## Reference

### Scanner

>R. Beverly, R. Durairajan, D. Plonka, and J. P. Rohrer, “In the IP of the Beholder: Strategies for Active IPv6 Topology Discovery,” in IMC, 2018.

### TGAs

>B. Hou, Z. Cai, K. Wu, T. Yang, and T. Zhou, “Search in the Expanse: Towards Active and Global IPv6 Hitlists,” in INFOCOM, 2023. \
>B. Hou, Z. Cai, K. Wu, T. Yang, and T. Zhou, “6Scan: A High-Efficiency Dynamic Internet-wide IPv6 Scanner with Regional Encoding,” IEEE/ACM Transections on Networking, 2023. \
>B. Hou, Z. Cai, K. Wu, J. Su, and Y. Xiong, “6Hit: A Reinforcement Learning-based Approach to Target Generation for Internet-wide IPv6 Scanning,” in INFOCOM, 2021. \
>Z. Liu, Y. Xiong, X. Liu, W. Xie, and P. Zhu, “6Tree: Efficient Dynamic Discovery of Active Addresses in the IPv6 Address Space,” Computer Networks, 2019. \
>A. Murdock, F. Li, P. Bramsen, Z. Durumeric, and V. Paxson, “Target Generation for Internet-Wide IPv6 Scanning,” in IMC, 2017.

### Hitlists & Aliases

>O. Gasser et al., “Clusters in the Expanse: Understanding and Unbiasing IPv6 Hitlists,” in IMC, 2018.
