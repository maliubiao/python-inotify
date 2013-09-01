python-inotify
==============
python binding for linux inotify 

* License     :MIT 

## Guide 
monitor all files and directories in current directory
```py
#! /usr/bin/env python
import os
import signal
import inotify

i = 0
fds = {}
consts = {
        inotify.IN_ACCESS: "File was accessed (read)",
        inotify.IN_ATTRIB: "Metadata changed ",
        inotify.IN_CLOSE_WRITE: "File opened for writing was closed",
        inotify.IN_CLOSE_NOWRITE: "File not opened for writing was closed",  
        inotify.IN_CREATE: "File/directory created in wached directory",
        inotify.IN_DELETE: "File/directory deleted from watched directory",
        inotify.IN_DELETE_SELF: "Watched file/directory was itself deleted",
        inotify.IN_MODIFY: "File was modified",
        inotify.IN_MOVE_SELF: "Watched file/directory was itself moved",
        inotify.IN_MOVED_FROM: "File moved out of watched directory",
        inotify.IN_MOVED_TO: "File moved into watched directory",
        inotify.IN_OPEN: "File was opened",
        inotify.IN_IGNORED: "Watch was removed explictily",
        inotify.IN_ISDIR: "Subject of this event is a directory",
        inotify.IN_Q_OVERFLOW: "Event queue overflowed",
        inotify.IN_UNMOUNT: "File system containing watched object was unmounted",
        inotify.IN_IGNORED: "Watch was removed explictly or automatically",
        inotify.IN_ISDIR: "Subject of this event is a directory",
        inotify.IN_Q_OVERFLOW: "Event queue overflowed",
        inotify.IN_UNMOUNT: "File system containing watched object was unmounted"
} 

def print_info(event): 
    if event.wd in fds.keys():
        print "file:%s\twd:%d\t%s" % (fds[event.wd], event.wd, consts[event.mask] )

def extracode():
    global i
    i = i + 1
    if i > 999999:
        print "extra code"
        i = 1
    
def handler(signum, frame):
    print "received SIGINT" 
    inotify.stoploop()
    exit(0)

for file in os.listdir("."): 
    fds[inotify.watch(file, inotify.IN_ALL_EVENTS)] = file 

signal.signal(signal.SIGINT, handler) 
inotify.startloop(callback = print_info, extra=extracode)
```
## Demo
    
    see examples/

## Install 
    #git clone https://github.com/maliubiao/python-inotify.git
    #cd python-inotify
    #sudo python setup.py install




