#!/usr/bin/python
import logging,sys,os
logging.getLogger("scapy.runtime").setLevel(logging.ERROR)
from scapy.all import *
import socket

## VARIABLES ##

global TARGET_IPv4
global TARGET_PORT

## FUNCTIONS ##

def getBanner(host,port):
	socket.setdefaulttimeout(2)
	s = socket.socket()
	s.connect((host,port))
	try:
		r = s.recv(1024)
		return str(r).rstrip("\r\n")
	except socket.timeout:
		return("(Socket timed out. Service is running but no banner was received.)")

def scan(host,port):
	ip = IP(dst=host)
	TCP_SYN = TCP(sport=RandShort(),dport=int(port),flags="S",seq=40)
	TCP_SYNACK = sr1(ip/TCP_SYN,timeout=1,verbose=0)
	if not TCP_SYNACK or TCP_SYNACK.getlayer(TCP).flags != 0x12:
		print "%s:%s is closed." % (host,port)
		return False
	else:
		print "%s:%s is open and got banner : %s" % (host,port,getBanner(host,int(port)))

## MAIN ##

if __name__ == '__main__':
	if os.getuid() != 0:
		print "You must be root to run this script."
		exit(1)
	if len(sys.argv) < 2:
		print "Usage: %s <pdst> <dport>" % sys.argv[0]
		exit(1)

	TARGET_IPv4 = sys.argv[1]
	if any(c.isalpha() for c in str(TARGET_IPv4)):
		res = socket.gethostbyname(str(TARGET_IPv4))
		print "%s resolved to %s." % (TARGET_IPv4,res)
		TARGET_IPv4 = str(res)
	try:
		TARGET_PORT = sys.argv[2]
	except:
		TARGET_PORT = "21,22,23,25,80"
	if any(c.isalpha() for c in str(TARGET_PORT)):
		try:
			proto = socket.getservbyname(str(TARGET_PORT))
			TARGET_PORT = proto
		except socket.error:
			print "Protocol not recognized."
			exit(1)

	try:
		if str(TARGET_PORT).find("-") != -1:
			target = str(TARGET_PORT).split("-",1)
			for targets in range(int(target[0]),int(target[1])):
				scan(TARGET_IPv4,targets)
		elif str(TARGET_PORT).find(",") != -1:
			target2 = str(TARGET_PORT).split(",",10)
			for targets in range(len(target2)):
				scan(TARGET_IPv4,int(target2[targets]))
		else:
			scan(TARGET_IPv4,TARGET_PORT)
	except KeyboardInterrupt:
		print 
