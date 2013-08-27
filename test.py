#! /usr/bin/env python
import os
import pyinotify

fds = {}
consts = {
        pyinotify.IN_ACCESS: "File was accessed (read)",
        pyinotify.IN_ATTRIB: "Metadata changed ",
        pyinotify.IN_CLOSE_WRITE: "File opened for writing was closed",
        pyinotify.IN_CLOSE_NOWRITE: "File not opened for writing was closed",  
        pyinotify.IN_CREATE: "File/directory created in wached directory",
        pyinotify.IN_DELETE: "File/directory deleted from watched directory",
        pyinotify.IN_DELETE_SELF: "Watched file/directory was itself deleted",
        pyinotify.IN_MODIFY: "File was modified",
        pyinotify.IN_MOVE_SELF: "Watched file/directory was itself moved",
        pyinotify.IN_MOVED_FROM: "File moved out of watched directory",
        pyinotify.IN_MOVED_TO: "File moved into watched directory",
        pyinotify.IN_OPEN: "File was opened",
        pyinotify.IN_IGNORED: "Watch was removed explictily",
        pyinotify.IN_ISDIR: "Subject of this event is a directory",
        pyinotify.IN_Q_OVERFLOW: "Event queue overflowed",
        pyinotify.IN_UNMOUNT: "File system containing watched object was unmounted"
}
       

def print_info(event): 
    if event.wd in fds.keys():
        print "file:%s\twd:%d\t%s" % (fds[event.wd], event.wd, consts[event.mask] )

for file in os.listdir("."): 
    fds[pyinotify.watch(file, pyinotify.IN_ALL_EVENTS)] = file 

pyinotify.startloop(print_info)

