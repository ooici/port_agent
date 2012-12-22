#!/usr/bin/env python
#
# This tool it used to test TCP listeners.  It connects to a tcp port.
# bytes are read, then written to an output file.
#
# Options:
# 
#  -h, --help - Display the program help screen
#  -f, --datafile - File to dump output too
#  -h, --host - Host to connect too
#  -p PORT, --port PORT - Port to connect too
#  -t TIMEOUT, --timeout TIMEOUT - Timeout duration 
#

import socket
import fcntl
import errno 
import sys
import argparse
import time
import os

##### MAIN #####

print "-->> DHE: testing..."

data = "DHE TEST:\n"
data += "\r\nversion: " + str(sys.version)
data +="\r\npath: " + str(sys.path) + "\r\n"

datafile = "deletemeohyeah.txt"

FILE = open(datafile, "w");
FILE.write(data);
print "Write: " + data;
FILE.close();
print "-->> DHE: file should be there"

