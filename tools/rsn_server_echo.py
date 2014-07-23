#!/usr/bin/env python
#
# This tool replicates an RSN serial server.It listens on a tcp port and 
# allows clients to connect.  Bytes are read a single character at a time
# until the buffer is empty.  Then the bytes read are wrapped in a port 
# agent packet and sent back to the original sender.
#
# Options:
# 
#  -h, --help - Display the program help screen
#  -c, --continuous - Do not close the connection after each read
#  -s, --single - Accept one client connection then kill the server.
#  -p PORT, --port PORT - Indicate what port to listen on.  Default: 40000
#  -t TIMEOUT, --timeout TIMEOUT - How long should we wait for a client connection?
#                                  set to 0 for no timeout, default 30
#  -d DELAY, --delay DELAY - sleep X number of seconds (fractions ok) before reading
#                            the net byte of data.  This is useful for testing 
#                            blocking vs. non-blocking reads.
#  -o, --stdout - enable this option if the server should output the buffer read to 
#                 stdout after it is read.
#  -n, --no_packet - Do not wrap data in port agent packet.

import socket
import fcntl
import errno 
import sys
import argparse
import time
import os
import binascii
from struct import Struct
from math import modf
import datetime

DEFAULT_TIMEOUT = 30
DEFAULT_PORT    = 40000
DEFAULT_DELAY   = 0

"""
Port Agent Packet Types
"""
UNKNOWN = 0
DATA_FROM_INSTRUMENT = 1
DATA_FROM_DRIVER = 2
PORT_AGENT_COMMAND = 3
PORT_AGENT_STATUS = 4
PORT_AGENT_FAULT = 5
INSTRUMENT_COMMAND = 6
HEARTBEAT = 7
PICKLED_DATA_FROM_INSTRUMENT = 8
PICKLED_DATA_FROM_DRIVER = 9

def gettime():
    (fraction, seconds) = modf(time.time());
    seconds = int(seconds) + 2208988800
    fraction = int(fraction)
    return (seconds, fraction)

def parseArgs():
    parser = argparse.ArgumentParser(description="TCP Echo Server")
    parser.add_argument("-s", '--single', dest='single', action="store_true",
        help="only accept one connection then quit" )
    parser.add_argument("-c", '--continuous', dest='continuous', action="store_true",
        help="do not close between reads" )
    parser.add_argument('-p', '--port', dest='port', type=int, default=40000, help='specify the INET port to bind')
    parser.add_argument('-t', '--timeout', dest='timeout', type=int, default=DEFAULT_TIMEOUT, help='listen timeout')
    parser.add_argument('-d', '--delay', dest='delay', type=float, default=DEFAULT_DELAY, help='delay between reading characters from the stream')
    parser.add_argument('-o', '--stdout', dest='stdout', action="store_true", help='write bytes read to stdout')
    parser.add_argument("-n", '--no_header', dest='header', action="store_false",
        help="do not wrap data in port agent packet" )
    return parser.parse_args()

def makepacket(msgtype, data, seconds, fraction):

    SYNC = (0xA3, 0x9D, 0x7A)
    HEADER_FORMAT = "!BBBBHHII"

    header_struct = Struct(HEADER_FORMAT)
    HEADER_SIZE = header_struct.size

    def calculateChecksum(data, seed=0):
        n = seed
        for datum in data:
            n ^= datum
        return n

    def pack_header(buf, msgtype, pktsize, checksum, seconds, fraction):
        sync1, sync2, sync3 = SYNC
        header_struct.pack_into(buf, 0, sync1, sync2, sync3, msgtype, pktsize,
                                checksum, seconds, fraction)

    pktsize = HEADER_SIZE + len(data)
    pkt = bytearray(pktsize)
    pack_header(pkt, msgtype, pktsize, 0, seconds, fraction)
    pkt[HEADER_SIZE:] = data
    checksum = calculateChecksum(pkt)
    pack_header(pkt, msgtype, pktsize, checksum, seconds, fraction)
    return pkt

def run_server(serv, opts):
    conn,addr = serv.accept() #accept the connection
    print("client connected")
    
    time_out = 0.1
    
    if (opts.timeout):
        time_out = opts.timeout
    
    print("timeout: %s" % time_out)
    
    try:
        while True:
            data = read_tcp(conn, opts.delay, time_out)
            if(data):
                if(opts.stdout): print "Read: " + binascii.hexlify(data)
                time.sleep(1)
                if (opts.header):
                    seconds, fraction = gettime()
                    data = makepacket(msgtype=DATA_FROM_INSTRUMENT, data=data, seconds=seconds, fraction=fraction)
                send_tcp(conn, data)
                time.sleep(1)

            if(not opts.continuous): break

        print "closing connection"
        conn.close()
        
    except Exception, e:
        raise e

def send_tcp(conn, buffer):
    ## TODO: Send in pieces
    print("Sending data: " + binascii.hexlify(buffer))
    result = conn.send(buffer)
    print("result: %d" % result)
    

def read_tcp(conn, read_delay, time_out):
    result = ""

    ## conn.settimeout(0.1)
    conn.settimeout(time_out)
    
    while True:
        try:
            data = conn.recv(1)
            if(read_delay): time.sleep(read_delay)
            if(data):
                print("READ: " + data)
                result = result + data
            else:
                break
        except socket.timeout:
            print "read_tcp timeout"
            break
        
        except socket.error as e:
            # [Errno 35] Resource temporarily unavailable.
            if e.errno == errno.EAGAIN:
                print("eagain")
                pass
            
            # [Errno 54] Connection reset by peer.  Because we are reading a single
            # byte at a time we read until we fail.
            elif e.errno == errno.ECONNRESET:
                print("connection reset")
                pass
            else:
                print("read/write error (errno: %d)" % e.errno)
                raise e

    return result

opts = parseArgs()

print "options: " + str(opts)

##let's set up some constants
hostname = ''    #we are the host
portnum = opts.port    #arbitrary port not currently in use
address = (hostname,portnum)    #we need a tuple for the address
buffer_size = 1    #reasonably sized buffer for data
timeout = opts.timeout         # Socket timeout

## now we create a new socket object (serv)
## see the python docs for more information on the socket types/flags
serv = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
serv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

##bind our socket to the address
serv.bind((address))    #the double parens are to create a tuple with one element
print("listening")
serv.listen(5)
print("ready for connections")

if(timeout):
    serv.settimeout(timeout)

while(1):
    run_server(serv, opts)
    if opts.single:
        break
