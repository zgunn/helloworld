#!/usr/bin/python
from Crypto.Cipher import AES
import os, random, struct
import hashlib, base64
import sys

### Since it might have to pad the file to fit inot mult of 16,
### function saves original file size in the first 8 bytes of
### output file (first sizeof(long,long) bytes)
### Randomly generates 16 byte IV and stores in in the file as well
### Then, reads input file chunk by chunk (chunk sizes config'able),
### encrypts the chunk, and writes it to output
### Last chunk is padded with spaces, if needed
def encfile(infname,key=None,ofname=None,chunksiz=64*1024):
	if not key:
		salt = base64.b64encode(os.urandom(32))
		key = hashlib.pbkdf2_hmac('sha256',(raw_input("Enter password to encrypt file with: ")),salt,100000)
	if not ofname:
		ofname = infname+'.enc'

	iv = ''.join(chr(random.randint(0,0xFF)) for i in range(16))
	encryptor = AES.new(key,AES.MODE_CBC,iv)
	filesiz = os.path.getsize(infname)

	with open(infname,'rb') as infp:
		with open(ofname,'wb') as ofp:
			ofp.write(struct.pack('<Q',filesiz))
			ofp.write(iv)

			while True:
				chunk = infp.read(chunksiz)
				if len(chunk) == 0:
					break
				elif len(chunk) % 16 != 0:
					chunk += ' ' * (16 - len(chunk) % 16)

				ofp.write(encryptor.encrypt(chunk))



### First the original file size is read from the first 8 bytes of the
### encrypted file. The IV is read next to correctly init AES obj
### Then file is decrypted in chunks, finally truncated to orig size
def decfile(infname,key=None,ofname=None,chunksiz=24*1024): # why 24*1024?
	if not key:
		key = raw_input("Enter password to decrypt file: ")
	if not ofname:
		ofname = os.path.splitext(infname)[0]

	with open(infname,'rb') as infp:
		origsiz = struct.unpack('<Q',infp.read(struct.calcsize('Q')))[0]
		iv = infp.read(16)
		decryptor = AES.new(key,AES.MODE_CBC,iv)

		with open(ofname,'wb') as ofp:
			while True:
				chunk = infp.read(chunksiz)
				if len(chunk) == 0:
					break
				ofp.write(decryptor.decrypt(chunk))

			ofp.truncate(origsiz)

try:
	op = sys.argv[1]
	infilename = sys.argv[2]
except:
	print "Usage:",sys.argv[0],"<operation [e,d]> <input filename>"
	sys.exit(1)

if op == "e":
	encfile(infilename)
elif op == "d":
	decfile(infilename)
else:
	print "Invalid op"
