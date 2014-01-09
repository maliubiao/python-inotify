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
    if event.mask not in consts:
        print "ignore event", event.mask 
        return 
    if event.wd in fds:
        print "file:%s\twd:%d\t%s" % (fds[event.wd], event.wd, consts[event.mask] )

#if you want to add something to the mainloop
def extracode():
    pass   


def handler(signum, frame):
    print "received SIGINT" 
    inotify.stoploop()
    exit(0)

signal.signal(signal.SIGINT, handler) 


for file in os.listdir("."): 
    fds[inotify.watch(file, inotify.IN_ALL_EVENTS)] = file 


#nonblock
while True:
    inotify.startloop(callback = print_info, timeout=100, block=False) 
"""
#block
inotify.startloop(callback = print_info, extra=extracode, timeout=100, block=True)
"""
```
##API
###startloop(callback, extra=func, timeout=100, block=True)
#####Start the main loop, callback is a function with only one parameter event, there are five members in event, only two: mask and wd are valid so far.
#####wd is the identifier returned by inotify.watch, use mask & const in intoify to decide which event is triggered
###stoploop()
####Stop the main loop, release resources, this must be done if you don't use inotify anymore.
###watch(path, mask)
####Path is the path of the file you want to watch, mask decides what kind event you want to receive, eg: inotify.IN_MODIFY | intoify.IN_OPEN. 
####This function returns a identifier
###unwatch(identifier) 
####Don't watch a file anymore.
## Examples
    
    see examples/ 

## Install 
    #git clone https://github.com/maliubiao/python-inotify.git
    #cd python-inotify
    #sudo python setup.py install

