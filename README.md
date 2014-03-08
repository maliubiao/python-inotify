python-inotify
==============
python binding for linux inotify 

* License     :MIT 

## Guide 
monitor all files and directories in current directory
```shell 
python test.py
{'cookie': 0L, 'wd': 1, 'mask': 32L, 'name': '', 'len': 0L}
{'cookie': 0L, 'wd': 1, 'mask': 1L, 'name': '', 'len': 0L}
{'cookie': 0L, 'wd': 1, 'mask': 16L, 'name': '', 'len': 0L}
{'cookie': 0L, 'wd': 1, 'mask': 32L, 'name': '', 'len': 0L}
{'cookie': 0L, 'wd': 1, 'mask': 1L, 'name': '', 'len': 0L}
{'cookie': 0L, 'wd': 1, 'mask': 16L, 'name': '', 'len': 0L}
```
## Examples
    
    see examples/ 

## Install 
    #git clone https://github.com/maliubiao/python-inotify.git
    #cd python-inotify
    #sudo python setup.py install

