#! /usr/bin/python

import signal
import inotify



def handler(signum, frame):
    print "received SIGINT"
    inotify.stoploop()
    exit(0)

signal.signal(signal.SIGINT, handler)

i = 1

def response(event):
    '''unwatch after 5 event''' 
    global i 
    print "event",i
    if i > 4: 
        #if we receive more than one event in the same time.
        try:
            inotify.unwatch(event.wd) 
        except OSError:
            pass 
        i = 0

    i = i + 1


inotify.watch("example", inotify.IN_ALL_EVENTS)

inotify.startloop(response)
 
