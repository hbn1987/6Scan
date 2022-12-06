#!/usr/bin/env python

from __future__ import print_function

import argparse
import sys

try:
    import SubnetTree
except Exception as e:
    print(e, file=sys.stderr)
    print("Use `pip install pysubnettree` to install the required module", file=sys.stderr)
    sys.exit(1)


def read_non_aliased(tree, fh):
    return fill_tree(tree, fh, ",0")

def read_aliased(tree, fh):
    return fill_tree(tree, fh, ",1")

def fill_tree(tree, fh, suffix):
    for line in fh:
        line = line.strip()
        try:
            tree[line] = line + suffix
        except ValueError as e:
            print("Skipped line '" + line + "'", file=sys.stderr)
    return tree


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--aliased-file", required=True, type=argparse.FileType('r'), help="File containing aliased prefixes")
    # parser.add_argument("-n", "--non-aliased-file", required=True, type=argparse.FileType('r'), help="File containing non-aliased prefixes")
    parser.add_argument("-i", "--ip-address-file", required=True, type=argparse.FileType('r'), help="File containing IP addresses to be matched against (non-)aliased prefixes")
    parser.add_argument("-o", "--output", required=False, type=argparse.FileType('w'), help="Output non-alias addresses")
    args = parser.parse_args()

    # Store aliased and non-aliased prefixes in a single subnet tree
    tree = SubnetTree.SubnetTree()

    # Read aliased and non-aliased prefixes
    tree = read_aliased(tree, args.aliased_file)
    # tree = read_non_aliased(tree, args.non_aliased_file)

    # Read IP address file, match each address to longest prefix and print output
    alias_list = list()
    for line in args.ip_address_file:
        line = line.strip()
        if (line[0] != '#'):    
            index = line.find(',')
            if  index != -1:
                ip = line[:index]
            else: 
                ip = line
            try:
                prefix = tree[ip]
                # print(line, prefix)
                alias_list.append(ip)
            except KeyError as e:
                pass
                # args.output.write(line + '\n')
        else:
            pass
            # args.output.write(line + '\n')
    print('Alias num:', len(set(alias_list)))

if __name__ == "__main__":
    main()
