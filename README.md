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

Commands
--------

`load <filename>`

Load a file off the internal flash into the 256 byte buffer

`save <filename>`

Save the internal buffer to a file in internal flash

`ls`

List the contents of the internal flash

`rm <filename>`

Remove a file from flash

`read`

Read the EEPROM data into the internal buffer

`write`

Write the internal buffer out to the EEPROM

`print`

Display the content of the internal buffer

`set <addr> <val>`

Set the buffer address `<addr>` to `<val>`.  Both address and value can be in an of the formats:

* Hex: `0x12`
* Binary: `0b1001100`
* ASCII: `'Q`
* Decimal: `123`


