from scapy.all import *
fakesrc = 'x:x:x::x'
server = '240e:688:400:bf::2'
device = 'eth0'
snmpver = 'v2c'
snmpcomm = 'public'

mypacket = IPv6(dst=server, tc = 1)/fuzz(UDP(dport=161))/SNMP(version=snmpver, community=snmpcomm, PDU=SNMPbulk(id=RandNum(1,200000000),max_repetitions=70,varbindlist=[SNMPvarbind(oid=ASN1_OID('1.3.6'))]))
l2packet = Ether()/mypacket
# l2packet.show()

t = AsyncSniffer(iface="eth0", prn=lambda x:x.summary(), filter="host 240e:688:400:bf::2")

t.start()
# sendpfast( l2packet*2500, pps=10000, loop=100, iface=device )
sendpfast( l2packet, pps=10000, loop=1, iface=device )
t.stop()

results = t.results
for item in results:
    print(item.show())
    print("IP.payload",item.payload,len(item.payload))
    print("UDP.payload",item.payload.payload,len(item.payload.payload))
    print("RaW.payload",item.payload.payload.payload,len(item.payload.payload.payload))
    print("====================")