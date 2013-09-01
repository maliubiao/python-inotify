#! /usr/bin/python

import signal
import inotify


def on_write(event):
    print('write')

def on_read(event):
    print("read")

def on_attrib(event):
    print("attribute change")

def on_move(event):
    print("move")

def on_open(event):
    print("open")

def on_close(event):
    print("close")

def on_delete_self(event):
    print("delete_self")

def on_create(event):
    print("create")

def on_delete(event):
    print("delete")

""" 
inotify.IN_MOVE = inotify.IN_MOVED_FROM | inotify.MOVED_TO
inotify.IN_CLOSE = inotify.IN_CLOSE_WRITE | inotify.IN_CLOSE_NOWRITE

inotify.IN_MOVE, IN_MOVED_FROM, IN_MOVED_TO, IN_DELETE
IN_CREATE are  directory only masks
inotify.IN_ACCESS, IN_MODIFY, IN_OPEN, IN_CLOSE,
IN_CLOSE_WRITE, IN_CLOSE_NOWRITE are file only masks.
"""

def response(event): 
    mask = event.mask
    if mask & inotify.IN_ACCESS:
        on_read(event)
    elif mask & inotify.IN_MODIFY:
        on_write(event)
    elif mask & inotify.IN_ATTRIB:
        on_attrib(event)
    elif mask & inotify.IN_OPEN:
        on_open(event)
    elif mask & inotify.IN_CLOSE:
        on_close(event) 
    elif mask & inotify.IN_MOVE:
        on_move(event)
    elif mask & inotify.IN_DELETE_SELF:
        on_delete_self(event)
    elif mask & inotify.IN_DELETE:
        on_delete(event)
    elif mask & inotify.IN_CREATE:
        on_create(event)
    

def handler(signum, frame):
    print "received SIGINT"
    inotify.stoploop()
    exit(0)

signal.signal(signal.SIGINT, handler)

inotify.watch("example", inotify.IN_ALL_EVENTS) 

inotify.startloop(response)

