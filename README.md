# HMap

Towards Comprehensive and Efficient Internet-wide Scan in IPv6 Networks.

## Description

Efficient IPv6 scanner integrates ping (HMap-ping6), traceroute (HMap-trace6), and target generation algorithms (HMap-tga6).

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
-s [HMap6/6Scan/6Hit/6Tree/6Gen] # Set TGA searching strategy, e.g., '-s HMap6' denotes HMap-tga6 method.
-A [country-xx/as-xxxx] # Perform HMap-ping6 in a designated region.
-U [low/ran] # Specify the target address generation types for HMap-ping6 as "low" for low-byte and "ran" for randomized.
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
./6scan -A country_CN -t ICMP6 -I [interface e.g., eth0] -k 10 -d -U ran -b 500
# Performing dynamic ping (HMap-ping6) with a de-aliasing strategy on Chinese IPv6 prefixes using ICMPv6 probes, utilizing a maximum of 500 million ICMPv6 probes with randomized pattern target addresses. This comprises 10 probes in each subprefix, with an emphasis on identifying the responding probe type.
./6scan -A global -t ICMP6 -I eno1 -k 4 -d -U low -b 500
# Conducting dynamic ping (HMap-ping6) with a de-aliasing strategy on global IPv6 prefixes, utilizing a maximum of 500 million ICMPv6 probes with low-byte pattern target addresses. This comprises 4 probes in each subprefix, with an emphasis on identifying the responding probe type.

./6scan -t ICMP6 -I [interface e.g., eth0] -s HMap6
# IPv6 Internet-wide scanning with HMap-tga6 method using the latest local ICMPv6 seedset.
./6scan -t UDP6 -I [interface e.g., eth0] -s 6Hit
# IPv6 Internet-wide scanning with 6Hit strategy using the latest local UDPv6 seedset.
./6scan -t UDP6 -I [interface e.g., eth0] -s 6Scan -F [seedfile]
# Specifies the seedset for scanning using 6Scan strategy.

./6scan -C [address file e.g., ./output/raw_6Scan_ICMP6_yyyymmdd] 
# Remove the alias address in the file of raw_6Scan_ICMP6_yyyymmdd.
```

### Scan in a run

```shell
# Step 1: Heuristic seed collection (HMap-ping6)
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
