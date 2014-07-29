import socket
import struct
import fileinput
import sys
import select
import os

DIR = os.path.dirname(os.path.realpath(__file__))
GBL = "/tmp/SERVER_URL"

MESSAGE = "REQUEST_SERVER_IP"
MCAST_GRP = '224.1.1.1'
MCAST_PORT = 10010

data = None
SERVER_IP = None

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', MCAST_PORT))
mreq = struct.pack("4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)

sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

while not SERVER_IP and not (data == "TEMPMON_SERVER_IP"):
  data, SERVER_IP = sock.recvfrom(1024)

serverURL = "http://"+str(SERVER_IP[0])+":5005"
print "Server URL discovered: ", serverURL
print "Recording server URL for client use."

try:
  f = open(GBL, "r")
  lines = f.readlines()
  f.close()
  
  f = open(GBL, "w")
  for line in lines:
    if "url" in line:
      f.write("url: " + serverURL + '\n')
    else:
      f.write(line)
except IOError:
  f = open(GBL, "w")
  f.write("url: " + serverURL + '\n')

f.close()
    
print ""
