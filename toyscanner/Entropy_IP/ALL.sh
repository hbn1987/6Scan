#!/bin/bash
#
# This code runs all parts of Entropy/IP in one shot.
#
#

function ips()
{
	if [ "${IPS##*.}" = "gz" ]; then
		cat $IPS | zcat
	else
		cat $IPS
	fi
}

if [ $# -ne 1 ]; then
	echo "usage: ALL.sh n(number of IP generated)"
	echo
	echo "Entropy/IP: do all steps and generate IPv6 addrs"
	echo
	exit 1
fi >&2

IPS="./output/seeds"
DIR="output"
N="$1"
set -o pipefail
set -o errexit
set -o nounset

mkdir -p $DIR
[ -d "$DIR" ] || exit 1

echo "0. transform ipv6 addresses"
./iptrans.py || exit 2

echo -e "\n1. segments"
ips | ./a1-segments.py /dev/stdin \
	>$DIR/segments || exit 3

echo -e "\n2. segment mining"
ips | ./a2-mining.py /dev/stdin $DIR/segments \
	>$DIR/analysis || exit 4

echo -e "\n3. bayes model"
ips | ./a3-encode.py /dev/stdin $DIR/analysis \
	| ./a4-bayes-prepare.sh /dev/stdin \
	>$DIR/bnfinput || exit 5
./a5-bayes.sh $DIR/bnfinput \
	>$DIR/cpd || exit 6

#echo -e "\n4. web report"
#./b1-webreport.sh $DIR $DIR/segments $DIR/analysis $DIR/cpd \
#	|| exit 6

echo -e "\n5. generate encoded IPs"
./c1-gen.py $DIR/cpd -n $N \
    >$DIR/genIPs || exit 7
echo -e "encoded IPs written to $DIR/genIPs"

echo -e "\n6. decode IPs"
./c2-decode.py $DIR/genIPs $DIR/analysis --colons \
    >$DIR/target || exit 8
echo -e "generated IPs written to target"

echo -e "\n7. scanning target IPs"
./d1-scan.py || exit 9
echo -e "scanning finished"

exit 0
