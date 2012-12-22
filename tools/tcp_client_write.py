#!/usr/bin/env python
#
# Tool to write to a TCP socket.  Read from STDIN and dump
# to a TCP socket.  
#
# The host information is hardcoded to match the tcp test 
# server program

from optparse import OptionParser
import socket
import sys
import time
import errno
 
usage = \
"""
%prog [options] 
"""
description = \
"""Use this program to write data to a TCP port and echo it back to the same port
"""
parser = OptionParser(usage=usage, description=description)
parser.add_option("-p", "--port", dest="port", default=40000, help="specify a port", action="store", type=int, metavar="PORT")
parser.add_option("-f", "--datafile", dest="data", help="filename containing data to send", action="store")
parser.add_option("-r", "--responsefile", dest="response", help="wait for a response and store output in file", action="store")
parser.add_option("-t", "--timeout", dest="timeout", help="read timeout", type=int, action="store")

(options, args) = parser.parse_args()

HOST = 'localhost'
PORT = options.port
ADDR = (HOST,PORT)
BUFSIZE = 1024
 
cli = socket.socket(socket.AF_INET,socket.SOCK_STREAM)

if(options.timeout):
    cli.settimeout(options.timeout)

cli.connect((ADDR))

time.sleep(1);

FILE = open(options.data);

while(1):
	if(options.data):
		line = FILE.readline()
	else:
		line = sys.stdin.readline()

	if not line:
		break

        print "send (%d): '%s'" % (options.port, line)
	cli.send(line)

# wait for a response if asked too
if(options.response):
    while True:
        try:
            response = cli.recv(BUFSIZE)
            if(response):
                print "recv: " + response
                break;
        except socket.error as e:
            # [Errno 35] Resource temporarily unavailable.
            if e.errno == errno.EAGAIN:
                pass
            else:
                raise e

    FILE = open(options.response, "w");
    FILE.write(response);
    FILE.close

cli.close()
FILE.close()
