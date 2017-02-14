#!/usr/bin/python
import zipfile
import sys
from threading import Thread

def extractFile(zFile,password):
	try:
		zFile.extractall(pwd=password)
		print "Password found : "+password
		exit(0)
	except:
		pass

def main():
	zname = sys.argv[1]
	dname = sys.argv[2]
	zFile = zipfile.ZipFile(zname)
	passFile = open(dname)
	for line in passFile.readlines():
		password = line.strip('\r\n')
		t = Thread(target=extractFile,args=(zFile,password))
		t.start()

if __name__ == '__main__':
	if len(sys.argv) < 3:
		print "Usage: %s <zipfile> <dict>" % sys.argv[0]
		exit(1)
	main()
