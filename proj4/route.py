import socket

# the public network interface
HOST = socket.gethostbyname(socket.gethostname())

# create a raw socket and bind it to the public interface
s = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_IP)
s.bind((HOST, 0))

# Include IP headers
s.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)

# receive all packages
s.ioctl(socket.SIO_RCVALL, socket.RCVALL_ON)

# receive a package
print s.recvfrom(65565)

# disabled promiscuous mode
s.ioctl(socket.SIO_RCVALL, socket.RCVALL_OFF)

#import socket
#import binascii
#import select
#
#HOST = socket.gethostbyname(socket.gethostname())
#
#s = socket.sock(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_IP)
#s.bind((HOST,0)
#
#s.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL,1)
#
#s.ioctl(socket.SIO_RCVALL, socket.RCVALL_ON)
#
#print s.recvfrom(65565)
#
#s.ioctl(socket.SIO.RCVALL, socket.RCVALL_OFF)
##s.connect(
##family, stype, proto, cname, saddr = socket.getaddrinfo()
##print socket.family
#
##print family, stype, proto, came, saddr
