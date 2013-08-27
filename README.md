python-inotify
==============
python binding for linux inotify 


* License     :MIT

## Dependencies
* kernel >= 2.6.13
* cPython >= 2.7

## Guide

    #echo "">tmp
    
    import os
    imort inotify
    
    def print_event(event):
        print("wd:%d\tmask:%d" % (event.wd, event.mask))
        
    inotify.watch("tmp", inotify.IN_ALL_EVENTS)
    inotify.startloop(print_event)
    
## Demo
    
    #python test.py    

## Install
### Clone this repo and install it with `python setup.py install`

    #git clone https://github.com/maliubiao/python-inotify.git
    #cd python-inotify
    #sudo python setup.py install




