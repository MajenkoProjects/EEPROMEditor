EEPROMEditor
============

This is a simple editor and programmer for 24L02 (256 byte) I2C EEPROM chips.

It makes use of the advanced filesystem and USB stack in chipKIT to provide an internal
flash-based filesystem for storing EEPROM images, and also export that filesystem
as a USB mass storage device so your computer can access the images and copy new images
into the programmer.

Control is through a USB serial (CDC/ACM) interface with a simple command-line interface.
Commands include:

* Filesystem comands: save, load, ls, rm
* EEPROM commands: scan, read, write
* Data manipulation commands: print, set

Equipment
---------

This sketch is designed to run on a [chipKIT Lenny](https://majenko.co.uk/lenny)
with an 8-pin SOIC clip attached to A4 and A5 with pullups to 3.3V.

```
         .--_--.
        o|     |o 3.3V
        o|     |o
        o|     |o A5 + pullup
    GND o|     |o A4 + pullup
         '-----'
```

