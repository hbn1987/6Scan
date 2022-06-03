# Reflections NTP monlist Lab Setup #

To set up an effect NTP monlist lab, you will need the following:

*	CLOSED network
*	Reflector NTP server with > 600 NTP clients. Packets from fake clients can be preloaded to generate large monlist responses.
	*	Reflector should be provisioned on a 1000 Mbps or 10 Gbps port
	*	Should make a script to preload an NTP server with random requests from random client IPs
*	Attacker on a 100 or 1000 Mbps port (smaller port than reflector or victim)
*	Reflector on a 1000 Mbps or 10 Gbps port
*	Victim also on a 1000 Mbps or 10 Gbps port (to measure traffic as accurately as possible)

