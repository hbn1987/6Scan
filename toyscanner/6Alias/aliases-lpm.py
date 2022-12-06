#!/usr/bin/env python

from __future__ import print_function

import argparse
import sys
import SubnetTree


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
    #parser.add_argument("-n", "--non-aliased-file", required=True, type=argparse.FileType('r'), help="File containing non-aliased prefixes")
    parser.add_argument("-i", "--ip-address-file", required=True, type=argparse.FileType('r'), help="File containing IP addresses to be matched against (non-)aliased prefixes")
    args = parser.parse_args()

    # Store aliased and non-aliased prefixes in a single subnet tree
    tree = SubnetTree.SubnetTree()

    # Read aliased and non-aliased prefixes
    tree = read_aliased(tree, args.aliased_file)
    #tree = read_non_aliased(tree, args.non_aliased_file)

    # Read IP address file, match each address to longest prefix and print Sim_experiment_downsampling3k_9k
    fp = open("./Sim_experiment_downsampling3k_9k/aaddrs", "w")
    for line in args.ip_address_file:
        line = line.strip()
        try:
            fp.writelines(line + "," + tree[line] + "\n")
        except KeyError as e:
            #print("Skipped line '" + line + "'", file=sys.stderr)
            pass
    fp.close()
if __name__ == "__main__":
    main()
