# MIDI UART USB converter

Bi-directional converter between MIDI USB and MIDI UART. There are many pages
that describe how to connect a UART port to DIN connectors so it is not covered
here.

Maximum System Exclusive size is 1024 bytes. This can be bigger on SAMD boards
which have more RAM.

Install MIDI Library and MIDIUSB libraries using the Arduino IDE library manager.

Tested in 32u4 boards such as Arduino Leonardo, SparkFun Pro Micro, and Adafuit
Itsy Bitsy 32u4.

Tested on Arduino Zero and Adafruit Trinket M0. It might work on other SAMD21
boards.

## Testing

Tested in with hardware loop back on UART Tx/Rx using SendMIDI and ReceiveMIDI.

In one command line window run receivemidi like this:

$ receivemidi dev arduino

In another command line window run sendmidi with the test script
receivemidi.txt like this:

$ sendmidi dev arduino file receivemidi.txt

Compare the output of receivemidi (redirect output to a file) with the
receivemidi.txt test script.

## References

https://www.midi.org/specifications/

http://www.usb.org/developers/docs/devclass_docs/midi10.pdf

https://github.com/gbevin/ReceiveMIDI

https://github.com/gbevin/SendMIDI
