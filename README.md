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

## Hardware

All of the following run MIDIUARTUSB.

Arduino Leonardo (32u4) with SparkFun MIDI shield. The buttons and pots are not
used in this program.

![Arudino Leonardo with SparkFun MIDI shield](https://github.com/gdsports/MIDIUARTUSB/blob/master/images/leonardo_midi.jpg)

Pro Micro (32u4) 5V with MIDI breakout board.

![Pro Micro 5V with MIDI breakout board](https://github.com/gdsports/MIDIUARTUSB/blob/master/images/promicro_midi.jpg)

Adafruit Trinket M0 (SAMD21) 3.3V with MIDI breakout board. Note the different
resistor values on the MIDI board for 3.3V operation.

![Trinket M0 3.3V with MIDI breakout board](https://github.com/gdsports/MIDIUARTUSB/blob/master/images/trinketm0_midi.jpg)

## References

https://www.midi.org/specifications/

http://www.usb.org/developers/docs/devclass_docs/midi10.pdf

https://github.com/gbevin/ReceiveMIDI

https://github.com/gbevin/SendMIDI
