#!/usr/bin/python
import logging,os,sys
logging.getLogger("scapy.runtime").setLevel(logging.ERROR)
from scapy.all import *
import socket

## VARIABLES ##

global TARGET_IPv4
global TARGET_PORT

## FUNCTIONS ##

def getGateway():
	get = "route -n | grep 'UG' | awk '{print $2}'"
	gateway = os.popen(get).read().rstrip("\r\n")
	return str(gateway)

def getTargetMAC(host):
	ans,unans = srp(Ether(dst="ff:ff:ff:ff:ff:ff")/ARP(pdst=host),timeout=1,verbose=0)
	for snd,rcv in ans:
		mac = rcv.sprintf("%Ether.src%")
	try:
		return str(mac)
	except NameError:
		return False

def getBanner(host,port):
	socket.setdefaulttimeout(2)
	s = socket.socket()
	s.connect((host,port))
	try:
		r = s.recv(1024)
		return str(r).rstrip("\r\n")
	except socket.timeout:
		return str("(Socket timed out. Service is running but no banner was received.)")

def portScan(host,port):
	ip = IP(dst=host)
	TCP_SYN = TCP(sport=RandShort(),dport=port,flags="S",seq=40)
	TCP_SYNACK = sr1(ip/TCP_SYN,timeout=1,verbose=0)
	try:
		if not TCP_SYNACK or TCP_SYNACK.getlayer(TCP).flags != 0x12:
			print "\t%s : closed." % port
			return
		else:
			print "\t%s : open and got banner : %s" % (port,getBanner(host,int(port)))
	except Exception:
		print "\tError scanning port : %s." % port

def scan(host):
	targetMAC = getTargetMAC(host)
	if targetMAC == False:
		print "%s is down." % host
		return False
	else:
		print "%s is at %s." % (host,targetMAC)

## MAIN ##

if __name__ == '__main__':
	if os.getuid() != 0:
		print "You must be root to run this script."
		exit(1)
	if len(sys.argv) < 1:
		print "Gateway : %s" % getGateway()
		print "Usage: %s <pdst> <ports>" % sys.argv[0]
		exit(1)
	try:
		TARGET_IPv4 = str(sys.argv[1])
	except:
		q1,q2,q3,q4 = getGateway().split(".")
		TARGET_IPv4 = str(q1)+"."+str(q2)+"."+str(q3)+".1-255"
	try:
		TARGET_PORT = sys.argv[2]
	except:
		TARGET_PORT = "20-26"
	if any(c.isalpha() for c in str(TARGET_PORT)):
		try:
			proto = socket.getservbyname(str(TARGET_PORT))
			TARGET_PORT = int(proto)
		except socket.error:
			print "Protocol not recognized."
			exit(1)

	try:
		if str(TARGET_IPv4).find("-") != -1:
			b1,b2,b3,b4 = TARGET_IPv4.split(".") # need more than 1 value to unpack
			target_ip = b4.split("-",1)
			for x in range(int(target_ip[0]),int(target_ip[1])):
				addr = str(b1)+"."+str(b2)+"."+str(b3)+"."+str(x)
				if scan(addr) != False:
					if str(TARGET_PORT).find("-") != -1:
						target = str(TARGET_PORT).split("-",1)
						for targets in range(int(target[0]),int(target[1])):
							portScan(addr,int(targets))
					elif str(TARGET_PORT).find(",") != -1:
						target4 = str(TARGET_PORT).split(",",10)
						for n in range(len(target4)):
							portScan(TARGET_IPv4,int(target4[n]))
					else: 
						portScan(addr,int(TARGET_PORT))
		elif str(TARGET_PORT).find("-") != -1:
			if scan(TARGET_IPv4) != False:
				target2 = str(TARGET_PORT).split("-",1)
				for targets2 in range(int(target2[0]),int(target2[1])):
					portScan(TARGET_IPv4,int(targets2))
		elif str(TARGET_PORT).find(",") != -1:
			if scan(TARGET_IPv4) != False:
				target3 = str(TARGET_PORT).split(",",10)
				for i in range(len(target3)):
					portScan(TARGET_IPv4,int(target3[i]))
		else:
			scan(TARGET_IPv4)
			portScan(TARGET_IPv4,int(TARGET_PORT))
	except KeyboardInterrupt:
		print 
