import dpkt
import sys
import socket
from collections import Counter


path = sys.argv[1]
file = open(path,'rb')
pcap = dpkt.pcap.Reader(file)

syn_list = list()
ack_list = list()

for time_stamp, buffer in pcap:

    try:
        eth = dpkt.ethernet.Ethernet(buffer)
    except (dpkt.dpkt.UnpackError, IndexError):
        continue #malformed or not ethernet


    ip = eth.data
    if not isinstance(eth.data, dpkt.ip.IP):
        continue #not IP


    tcp = ip.data
    if type(tcp) != dpkt.tcp.TCP:
        continue #not tcp


    if (tcp.flags & dpkt.tcp.TH_SYN) and (tcp.flags & dpkt.tcp.TH_ACK):
        ack_list.append(socket.inet_ntop(socket.AF_INET, ip.dst))
        
    elif tcp.flags & dpkt.tcp.TH_SYN:
        syn_list.append(socket.inet_ntop(socket.AF_INET, ip.src))


syn_count_bag = Counter(syn_list)
ack_count_bag = Counter(ack_list)


for s in (syn_count_bag.keys() | ack_count_bag.keys()):
    if syn_count_bag[s] > ack_count_bag[s]*3:
        print(s)
    #print(s, syn_count_bag[s], ack_count_bag[s])

