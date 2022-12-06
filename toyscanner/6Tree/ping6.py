from multiping import multi_ping
import sys

if __name__=="__main__":
    input=sys.argv[1]
    output=sys.argv[2]
    alloutput="./Sim_experiment_downsampling3k_9k/all"

    fin = open(input)
    fout = open(output, "w")
    allfout = open(alloutput, "a")
    lines = fin.readlines()
    lines = [line[:-1] for line in lines]
    fin.close()
    n = 5000
    for addrs in [lines[i:i + n] for i in range(0, len(lines), n)]:
        responses, no_responses = multi_ping(addrs, timeout=2, retry=3)
        for addr, rtt in responses.items():
            # print "%s responded in %f seconds" % (addr, rtt)
            fout.writelines(addr+"\n")
            allfout.writelines(addr+"\n")
    fout.close()
    allfout.close()
