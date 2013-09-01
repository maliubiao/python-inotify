#! /usr/bin/python
import signal
import inotify

i = 1 

def response(event):
    '''stop after 5 event'''
    global i 
    if i > 4: 
        inotify.stoploop()
    print "event",i
    i = i + 1

   

def handler(signum, frame):
    print "received SIGINT"
    inotify.stoploop()
    exit(0)
signal.signal(signal.SIGINT, handler)

inotify.watch("example", inotify.IN_ALL_EVENTS)

inotify.startloop(response)
