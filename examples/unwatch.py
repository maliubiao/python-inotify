#! /usr/bin/python

import signal
import inotify

def handler(signum, frame):
    print "received SIGINT"
    inotify.stoploop()
    exit(0)

signal.signal(signal.SIGINT, handler)

i = 0

def response(event):
    '''unwatch after 5 event''' 
    global i 
    print "event",i
    i = i + 1
    if i > 4: 
        inotify.unwatch(event.wd) 
        i = 0 



inotify.watch("example", inotify.IN_ALL_EVENTS)

inotify.startloop(response)
 
