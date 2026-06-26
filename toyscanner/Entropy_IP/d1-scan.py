#!/usr/bin/python
import os
from multiping import multi_ping
import logging

if __name__ == "__main__":
    # Scanning with icmpv6
    logging.basicConfig(filename='./Sim_experiment_downsampling3k_9k/run.log', level=logging.INFO)
    seeds=open("./Sim_experiment_downsampling3k_9k/target","r")
    lines = seeds.readlines()
    seeds.close()
    colons=[line.strip() for line in lines]

    n = 5000
    print "begin pinging ..."
    resfile = open("./Sim_experiment_downsampling3k_9k/allresult", "a")
    for addrs in [colons[i:i + n] for i in range(0, len(colons), n)]:
        try:
            responses, no_responses = multi_ping(addrs, timeout=2, retry=3)
        except:
            pass
        logging.info("%d responses in 5000 addresses" % len(responses))
        for addr, rtt in responses.items():
            # print "%s responded in %f seconds" % (addr, rtt)
            resfile.write(addr + "\n")
    resfile.close()

