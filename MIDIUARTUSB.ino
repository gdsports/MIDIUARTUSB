/*
MIT License

Copyright (c) 2018 gdsports625@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
   MIDIUARTUSB converter for 32u4 such as Leonardo, Pro Micro, Itsy Bitsy.

   Works on Arduino Zero and Adafruit Trinket M0 that has more RAM so larger
   Sys Ex is possible. Other SAMD21 boards might work.
*/

#include <MIDI.h>
#include <MIDIUSB.h>

// 1 turns on debug, 0 off
#define DBGSERIAL if (0) SERIAL_PORT_MONITOR

struct MySettings : public midi::DefaultSettings
{
    static const bool Use1ByteParsing = false;
    static const unsigned SysExMaxSize = 1026; // Accept SysEx messages up to 1024 bytes long.
    static const long BaudRate = 31250;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, MIDI, MySettings);

void USBSystemExclusive(unsigned size, byte *data, bool markersIncluded) {
  uint8_t bytesOut;
  midiEventPacket_t evt;

  while (size > 0) {
    bytesOut = min(3, size);
    evt.byte1 = *data++;
    size -= bytesOut;
    switch (bytesOut) {
      case 1:
        evt.header = 0x05;
        evt.byte2 = evt.byte3 = 0;
        break;
      case 2:
        evt.header = 0x06;
        evt.byte2 = *data++;
        evt.byte3 = 0;
        break;
      case 3:
        evt.header = (size > 0) ? 0x04 : 0x07;
        evt.byte2 = *data++;
        evt.byte3 = *data++;
        break;
      default:
        break;
    }
    MidiUSB.sendMIDI(evt);
  }
}

void setup() {
  DBGSERIAL.begin(115200);

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
}

uint32_t sysexSize = 0;

void loop()
{
  /* MIDI UART -> MIDI USB */
  if (MIDI.read()) {
    midi::MidiType msgType = MIDI.getType();
    DBGSERIAL.print(msgType, HEX);
    DBGSERIAL.print(' ');
    DBGSERIAL.print(MIDI.getData1(), HEX);
    DBGSERIAL.print(' ');
    DBGSERIAL.println(MIDI.getData2(), HEX);
    switch (msgType) {
      case midi::InvalidType:
        break;
      case midi::NoteOff:
      case midi::NoteOn:
      case midi::AfterTouchPoly:
      case midi::ControlChange:
      case midi::ProgramChange:
      case midi::AfterTouchChannel:
      case midi::PitchBend:
        {
          midiEventPacket_t tx = {
            (byte)(msgType>>4),
            (byte)((msgType & 0xF0) | ((MIDI.getChannel()-1) & 0x0F)), /* getChannel() returns values from 1 to 16 */
            MIDI.getData1(),
            MIDI.getData2()
          };
          MidiUSB.sendMIDI(tx);
          MidiUSB.flush();
          break;
        }
      case midi::SystemExclusive:
        USBSystemExclusive(MIDI.getSysExArrayLength(),
            (byte *)MIDI.getSysExArray(), true);
        MidiUSB.flush();
        break;
      case midi::TuneRequest:
      case midi::Clock:
      case midi::Start:
      case midi::Continue:
      case midi::Stop:
      case midi::ActiveSensing:
      case midi::SystemReset:
        {
          midiEventPacket_t tx = { 0x0F, (byte)(msgType), 0, 0 };
          MidiUSB.sendMIDI(tx);
          MidiUSB.flush();
          break;
        }
      case midi::TimeCodeQuarterFrame:
      case midi::SongSelect:
        {
          midiEventPacket_t tx = { 0x02, (byte)(msgType), MIDI.getData1(), 0 };
          MidiUSB.sendMIDI(tx);
          MidiUSB.flush();
          break;
        }
      case midi::SongPosition:
        {
          midiEventPacket_t tx = { 0x03, (byte)(msgType), MIDI.getData1(), MIDI.getData2() };
          MidiUSB.sendMIDI(tx);
          MidiUSB.flush();
          break;
        }
      default:
        break;
    }
  }

  /* MIDI USB -> MIDI UART */
  midiEventPacket_t rx = MidiUSB.read();
  if (rx.header != 0) {
    midi::Channel channel = (rx.byte1 & 0x0F) + 1;
    midi::MidiType midiType = (midi::MidiType)(rx.byte1 & 0xF0);
    switch (rx.header & 0x0F) {
      case 0x00:  // Misc. Reserved for future extensions.
        break;
      case 0x01:  // Cable events. Reserved for future expansion.
        break;
      case 0x02:  // Two-byte System Common messages
        switch (rx.byte1) {
          case midi::SongSelect:
            MIDI.sendSongSelect(rx.byte2);
            break;
          case midi::TimeCodeQuarterFrame:
            MIDI.sendTimeCodeQuarterFrame(rx.byte2);
            break;
          default:
            break;
        }
        break;
      case 0x03:  // Three-byte System Common messages
        if (rx.byte1 == midi::SongPosition) {
          MIDI.sendSongPosition(rx.byte2 | ((unsigned) rx.byte3 << 7));
        }
        break;
      case 0x04:  // SysEx starts or continues
        sysexSize += 3;
        MIDI.sendSysEx(3, &rx.byte1, true);
        break;
      case 0x05:  // Single-byte System Common Message or SysEx ends with the following single byte
        sysexSize += 1;
        DBGSERIAL.print("sysexSize=");
        DBGSERIAL.println(sysexSize);
        sysexSize = 0;
        MIDI.sendSysEx(1, &rx.byte1, true);
        break;
      case 0x06:  // SysEx ends with the following two bytes
        sysexSize += 2;
        DBGSERIAL.print("sysexSize=");
        DBGSERIAL.println(sysexSize);
        sysexSize = 0;
        MIDI.sendSysEx(2, &rx.byte1, true);
        break;
      case 0x07:  // SysEx ends with the following three bytes
        sysexSize += 3;
        DBGSERIAL.print("sysexSize=");
        DBGSERIAL.println(sysexSize);
        sysexSize = 0;
        MIDI.sendSysEx(3, &rx.byte1, true);
        break;
      case 0x08:  // Note-off
      case 0x09:  // Note-on
      case 0x0A:  // Poly-KeyPress
      case 0x0B:  // Control Change
      case 0x0C:  // Program Change
      case 0x0D:  // Channel Pressure
      case 0x0E:  // PitchBend Change
        MIDI.send(midiType, rx.byte2, rx.byte3, channel);
        break;
      case 0x0F:  // Single Byte, TuneRequest, Clock, Start, Continue, Stop, etc.
        MIDI.sendRealTime((midi::MidiType)rx.byte1);
        break;
    }
  }
}
