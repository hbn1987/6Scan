
# Toy Scanners Implemented using Python

## 6Hit
### Build
```shell
cd tree
compile 's2_entrance()' and 's3_entrance()' in the main.cpp 
g++ definition.hpp definition.cpp main.cpp m2.hpp m2.cpp m3.hpp m3.cpp -o tree
```
### Usage
```shell
python2 main.py
```

## 6Tree
### Build
```shell
g++ definition.hpp definition.cpp main.cpp m1.hpp m1.cpp m2.hpp m2.cpp m3.hpp m3.cpp m4.hpp m4.cpp -o 6tree
```
### Usage
```shell
./6tree
```

## 6Gen
### Usage
```shell
python2 6gen.py
```

## Entropy/IP
### Usage
```shell
./ALL.sh <num>`, where `<num>` is the number of IPv6 addresses you want to generate
```

## 6Alias
### Usage
```shell
python2 aliasdetection.py
```