from scapy.all import *

pkts = sniff(iface="eth0", prn=lambda x:x.summary(), filter="host 240e:688:400:bf::2 or 240e:688:400:bf::3", count =3)

for item in pkts:
    print(item.show())
    print("IP.payload",item.payload,len(item.payload))
    print("UDP.payload",item.payload.payload,len(item.payload.payload))
    print("RaW.payload",item.payload.payload.payload,len(item.payload.payload.payload))
    print("====================")