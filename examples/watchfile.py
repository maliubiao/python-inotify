if __name__ == "__main__":
    import inotify 
    import sys
    if len(sys.argv) < 2:
        print("require a file.")
        exit()

    target = sys.argv[1]
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
            
    def callback(event): 
        if event.mask in consts:
            print("%s: %s" %(target, consts[event.mask]))

    inotify.watch(target, inotify.IN_ALL_EVENTS)    
    inotify.startloop(callback = callback)
