6Scan
=====
Towards Efficient Internet-wide Scanning in IPv6 Network

## Description

Efficient network scanner which could scan the IPv6 network using various search strategies.

## Build

```shell
./bootstrap
./configure
make
```

## Usage

### Option
```shell
-D [hitlist/alias] # Download the IPv6 hitlist/aliased prefixes.
-P # Pre-scan the hitlist.
-t [ICMP6/TCP6_SYN/TCP6_ACK/UDP6] # Set probe type.
-I # Network interface to use.
-s [6Scan/6Tree/6Gen/Edgy] # Set IPv6 searching strategy. 
-r # Set max probing rate, 50k pps by default.
-C [Active address file e.g., ./output/raw_ICMP6_yyyymmdd] # Address classification.
```
### Example
```shell
./6scan -D hitlist
# Download the IPv6 hitlist from https://ipv6hitlist.github.io/.
./6scan -D alias
# Download the aliased prefixes from https://ipv6hitlist.github.io/.
./6scan -P -t ICMP6 -I [interface e.g., enp2s0f0]
# Pre-scan the latest local hitlist to make the seedset using ICMPv6 packet.
./6scan -t ICMP6 -I [interface e.g., enp2s0f0] -s 6Scan
# IPv6 Internet-wide scanning with 6Scan strategy using the latest local ICMPv6 seedset.
./6scan -t UDP6 -I [interface e.g., enp2s0f0] -s 6Tree
# IPv6 Internet-wide scanning with 6Tree strategy using the latest local UDPv6 seedset.
./6scan -C [Active address file e.g., ./output/raw_ICMP6_yyyymmdd] 
# Classify the active address in the file of raw_ICMP6_yyyymmdd.
```

## Output

6Scan outputs the collected active addresses, the scanning information in the file that begin with raw_ \
and the address classification results in the file that begin with results_.

|Information|Description|
|:---:|---|
|Strategy|searching strategy|
|Packet type|packet type|
|Budget|probing packet limit|
|Probing rate|number of packets sent per second|
|Received ratio|ratio of total number of received packets to the budget|
|Hit active addresses|hit active addresses|
|Discovered new addresses|found active addresses after removing the seed addresses|
|Time cost|time cost of the preparation and scanning time|
|Address classification|IID allocation schemas i.e., Alias, Small-integer, Randomized, Embedded-IPv4, EUI-64, and Other|


## Reference

### Scanner
>R. Beverly, R. Durairajan, D. Plonka, and J. P. Rohrer, “In the IP of the Beholder: Strategies for Active IPv6 Topology Discovery,” in IMC, 2018.
### IPv6 hitlist
>O. Gasser, Q. Scheitle, S. Gebhard, and G. Carle, “Scanning the IPv6 Internet: Towards a Comprehensive Hitlist,” in TMA, 2016.
### Search strategy (6Scan, 6Tree, 6Gen, Edgy)
>B. Hou, Z. Cai, K. Wu, J. Su, and Y. Xiong, “6Hit: A Reinforcement Learning-based Approach to Target Generation for Internet-wide IPv6 Scanning,” in INFOCOM, 2021. \
>Z. Liu, Y. Xiong, X. Liu, W. Xie, and P. Zhu, “6Tree: Efficient Dynamic Discovery of Active Addresses in the IPv6 Address Space,” Comput. Networks, 2019. \
>A. Murdock, F. Li, P. Bramsen, Z. Durumeric, and V. Paxson, “Target Generation for Internet-Wide IPv6 Scanning,” in IMC, 2017. \
>E. C. Rye and R. Beverly, “Discovering the IPv6 Network Periphery,” in PAM, 2020.
### Prefix Sketch
>L. Tang, Q. Huang, and P.P.C. Lee, “MV-Sketch: A Fast and Compact Invertible Sketch for Heavy Flow Detection in Network Data Streams,” in INFOCOM, 2019.
### Address classification
>Q. Hu and N. Brownlee, “How Interface ID Allocation Mechanisms are Performed in IPv6,” in CoNEXT Student Workshop, 2014.
### Alias resolution
>O. Gasser et al., “Clusters in the Expanse: Understanding and Unbiasing IPv6 Hitlists,” in IMC, 2018.