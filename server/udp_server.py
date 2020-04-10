#!/usr/bin/env python3

import datetime
import socket

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Bind the socket to the port
server_address = ("0.0.0.0", 42501)
sock.bind(server_address)

print("Server started on port %d\n" % server_address[1])
while True:
    data, address = sock.recvfrom(512)

    tstr = datetime.datetime.now().isoformat()[0:len("2019-09-12T11:25:41")]
    msg = data.decode("utf-8").strip()
    print('%s\t%s\t%s' % (tstr, address, msg))

    sock.sendto(data, address);
