
KinXOS
======
An experimental attempt at making a tiny operating system. The majority of the OS was developed using resources at:
* [OS-Dev.org]
* [BrokenThorn Entertainment]
* [OS Dever]

Programming Languages Used
-----
* Nasm 
* C

Kernel Wish List and TODO:
-----

#### Create a ext2 filesystem with typical directories
* Create drivers (or kernel functions) to make files, directories, etc.
* Create directories
* Install GRUB
* Get memory usage

#### Create a means of allocating ram
* Memory Management System
* Malloc, Dealloc, Realloc, Callac
* Ram usage 

#### Create a system that can read the CPU Usage
* a program like Linux 'top'

#### KinSHELL
* make an evironment variable system so programs can access their arguments from there.
* allow KinDos to execute programs that are externel to Kernel. 

#### Create fully functioning DATE program.
* ~~Can request CMOS date and time~~
* display it in proper format

#### Create a means of executing sound.
* A system beep sound.
* Play music (WAV, mp2, mp3)

#### Integrate USB drivers
* EXT2 and FAT32 Filesystems

#### Create a means of networking 
* TCP/IP
* WebSockets


Version
----
0.5.0


Running KinX OS
-----------
* Install QEMU
* Clone/Download Repo

```sh
cd KinXOS
make
qemu -fda KinXOS.img
```
if **qemu** is not available you can use **qemu-system-i386** instead.


License
----
MIT

[BrokenThorn Entertainment]:http://www.brokenthorn.com/
[OS Dever]:http://www.osdever.net/tutorials/
[OS-Dev.org]:http://wiki.osdev.org/