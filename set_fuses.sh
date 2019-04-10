#!/bin/bash
avrdude -p m32u4 -P usb -c usbasp  -v -v -V -U efuse:w:0xC8:m -U hfuse:w:0xD8:m -U lfuse:w:0xFF:m
