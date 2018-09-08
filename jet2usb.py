#!/usr/bin/python
#
import sys
import os
import argparse
import syslog
import socket
import struct
import select
import glob
import subprocess


parser = argparse.ArgumentParser()

parser.add_argument('-d', '--usbdevice', help="use Device with this serial number", default="/dev/null")
parser.add_argument('-e', '--sendextraeof', help="send an eof on close", default='0')
parser.add_argument('-u', '--senduel', help="send an <esc>%-12345X", default='0')
parser.add_argument('-L', '--debuglog', help="write Debuglogfile", default="/tmp/usbjet_dbg.log")
parser.add_argument('-D', '--debug', help='turn on debug', default='0')

args = parser.parse_args()

syslog.openlog(logoption=syslog.LOG_PID, facility=syslog.LOG_DAEMON)
syslog.syslog(syslog.LOG_INFO, "jet2usb starting")


if args.debug != 0:
    try:
        debugfile=open(args.debuglog,"wa")
    except IOError as e:
        syslog.syslog(syslog.LOG_ERR, "dbgopen error "+ str(e.errno))
        syslog.syslog(syslog.LOG_ERR, "dbgopen: " + e.strerror)


syslog.syslog(syslog.LOG_INFO, "trying device with serial '" + str(args.usbdevice) + "'")

port=0

os.chdir("/sys/bus/usb/devices/")
for a in glob.glob("*"):
   if (subprocess.call(["grep -q " + args.usbdevice + " " + a + "/serial 2>/dev/null"], shell=True) == 0 ):
       port=a
       syslog.syslog(syslog.LOG_INFO,"printing to usb device " + a)
if port == 0:
    syslog.syslog(syslog.LOG_ERR, "NO PRINTER found")
    exit(1)

pn=subprocess.check_output(["ls -1 " + port + "/" + port + ":*/usbmisc | tr -d '\n'"],shell=True)
printer="/dev/usb/" + pn
syslog.syslog(syslog.LOG_INFO,"printing to printer at " + printer)

try:
  usbhandle=open(printer,"r+b")
except IOError as e:
  syslog.syslog(syslog.LOG_ERR, "open error "+ str(e.errno))
  syslog.syslog(syslog.LOG_ERR, "open: " + e.strerror)
usedihandles=[usbhandle, sys.stdin]
notdidsendpostcmds=1
running=1
netrunning = 1
while running and netrunning:
  timeout = 10
  inputready, outputready, excptready = select.select(usedihandles, [], usedihandles, timeout)
  if ( inputready or outputready or excptready ) :
    for s in inputready:
      if s == usbhandle:
        data = s.read(1024)
        if data:
          sys.stdout.write(data)
#          print(data)
          if args.debug != 0:
              debugfile.write(data)
          syslog.syslog(syslog.LOG_INFO,"received data from printer ")
          syslog.syslog(syslog.LOG_INFO,data)
        else:
          running = 1
          syslog.syslog(syslog.LOG_INFO,"received empty input from printer")
      elif s == sys.stdin:
        data=sys.stdin.read()
        if data:
          usbhandle.write(data)
          if args.debug != 0:
              debugfile.write(data)
        else: 
          syslog.syslog(syslog.LOG_INFO,"end of input from net")
          netrunning=0
          if notdidsendpostcmds:
            notdidsendpostcmds=0
            if args.sendextraeof :
              usbhandle.write("")
              syslog.syslog(syslog.LOG_INFO,"sent additional eof")
            if args.senduel :
              usbhandle.write("%-12345X")
              syslog.syslog(syslog.LOG_INFO,"sent additional uel")
      else:
        syslog.syslog(syslo.LOG_INFO,"received select from unknown")
    for s in excptready:
      syslog.syslog(syslog.LOG_INFO, "exception from " + s.getpeername())
  else:
    syslog.syslog(syslog.LOG_INFO,"timeout on select")
syslog.syslog(syslog.LOG_INFO,"done printing closing connection")
usbhandle.close()
syslog.syslog(syslog.LOG_INFO,"closed usb - bye.")
if args.debug != 0:
    debugfile.close()

exit(0)
