# Htrace6

Efficient Internet-wide IPv6 Topology Discovery with Dynamic Traceroute
___

Htrace6 is an efficient tool specifically crafted for the measurement of IPv6 network topology. It builds upon the foundations laid by previous achievements [1-3] and introduces the innovative concept of dynamic probing. This novel approach significantly minimizes its impact on the Internet, thereby enabling the discovery of a larger number of router interfaces within the vast IPv6 address space in the same amount of time when compared to other available alternatives.

## Installation

trace6 has been developed using C++ 14 and incorporates several third-party libraries, such as Boost, GFlag, GLog, and other C++ libraries.

### 1. GFlags

GFlags is an open-source project from Google, which supports flags parsing functionalities.

```
sudo apt-get install libgflags-dev
```

### 2. GLog

GLog is also an Google's open source project to provide logging functionalities.

```
sudo apt install libgoogle-glog-dev
```

### 3. Bazel

Bazel is an open-source build system, developed by Google.

```
wget https://github.com/bazelbuild/bazel/releases/download/3.7.2/bazel-3.7.2-installer-linux-x86_64.sh

chmod +x bazel-3.7.2-installer-linux-x86_64.sh
./bazel-3.7.2-installer-linux-x86_64.sh --user
export PATH="$PATH:$HOME/bin"
export BAZEL_CXXOPTS="-std=c++14" 
```

## Compiling

```
bazel build --cxxopt="--std=c++14" trace6
```

To optimize the program during the compilation process, you can utilize the following command.

```
bazel build --cxxopt="--std=c++14" --compilation_mode=opt trace6
```

## Flags

`--split_ttl` Specify initial TTL to start Scan. By default, 16.

`--granularity` Setting the prefix length for probe termination.

`--gaplimit` Specify the number of consecutive silent interfaces to halt the forward probing. By default, 5.

`--remove_redundancy` Enable Doubletree-based redundancy removal in backward probing. By default, enabled.

`--interface` Specify the interface used by probing. By default, eth0.

`--probing_rate` Specify the probes sending rate in the unit of packet per second. By deafult, 100Kpps.

`--dst_port` Specify the destination port for probing.

`--targets` Specify the file path to target list.

## Usage

1. Probe an IPv4 address.

```
./bazel-bin/trace6/trace6 --interface eth0 192.168.1.1
```

2. Probe an IPv6 address.

```
./bazel-bin/trace6/trace6 --interface eth0 2607:f8b0:4009:805::200e
```

3. Probe the prefixes from the prefix32 file with dynamic traceroute.

```
./bazel-bin/trace6/trace6 --targets ./prefix32 --output ./output48 --granularity 48 
```

## Result 

Output file format   <destination, responder, distance>

```
bazel run parsers/route_path -- --file [filepath]
```

## References

[1] B. Donnet, P. Raoult, T. Friedman, and M. Crovella, “Efficient Algorithms for Large-scale Topology Discovery,” in SIGMETRICS, 2005.

[2] R. Beverly, “Yarrp’ing the Internet: Randomized High-Speed Active Topology Discovery,” in IMC, 2016.

[3] Y. Huang, M. Rabinovich, and R. Al-Dalky, “FlashRoute: Efficient Traceroute on a Massive Scale,” in IMC, 2020.