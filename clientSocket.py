#!/usr/bin/python2.7

#=====================================
# This is a file for Client connection
#======================================

import socket 
import sys

size = 1024

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

host = socket.gethostbyname(sys.argv[1])
port = int(sys.argv[2])
serverAddress = (host,port)
sock.connect(serverAddress)

data = sock.recv(size)
print data

#========================================
# Hardcoding of input data begins from here
#===========================================
#New user login

#sock.send("1")

#existing user login
sock.send("2")

data = sock.recv(size)
print data

sock.send("test3")

data = sock.recv(size)
print data

sock.send("test3")


data = sock.recv(size)
print data

sock.send("test3")


data = sock.recv(size)
print data

data = sock.recv(size)
print data
sock.send("test3")

data = sock.recv(size)
print data

sock.send("test3")
data = sock.recv(size)
print data

#sock.close()
