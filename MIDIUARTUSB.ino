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
    static const unsigned SysExMaxSize = 1024; // Accept SysEx messages up to 1024 bytes long.
    static const long BaudRate = 31250;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, MIDI, MySettings);

/*************** MIDI USB functions ****************************/

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void USBNoteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t evt = {0x08, (byte)(0x80 | (channel - 1)), pitch, velocity};
  MidiUSB.sendMIDI(evt);
}

void USBNoteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t evt = {0x09, (byte)(0x90 | (channel - 1)), pitch, velocity};
  MidiUSB.sendMIDI(evt);
}

// First parameter is the event type (0x0A = polyphonic key pressure)
// Second parameter is poly-keypress, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the pressure on the key after it "bottoms out"

void USBAfterTouchPoly(byte channel, byte pitch, byte pressure) {
  midiEventPacket_t evt = {0x0A, (byte)(0xA0 | (channel - 1)), pitch, pressure};
  MidiUSB.sendMIDI(evt);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void USBControlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, (byte)(0xB0 | (channel - 1)), control, value};
  MidiUSB.sendMIDI(event);
}

// First parameter is the event type (0x0C = program change)
// Second parameter is program number, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the program number.
// Fourth parameter is 0.

void USBProgramChange(byte channel, byte number) {
  midiEventPacket_t evt = {0x0C, (byte)(0xC0 | (channel - 1)), number, 0};
  MidiUSB.sendMIDI(evt);
}

// First parameter is the event type (0x0D = after touch channel pressure)
// Second parameter is channel pressure, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the channel pressure value.
// Fourth parameter is 0.

void USBAfterTouchChannel(byte channel, byte pressure) {
  midiEventPacket_t evt = {0x0D, (byte)(0xD0 | (channel - 1)), pressure, 0};
  MidiUSB.sendMIDI(evt);
}

// First parameter is the event type (0x0E = pitch bend)
// Second parameter is pitch bend, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the least significant 7 bits of pitch bend
// Fourth parameter is the most significant 7 bits of pitch bend

void USBPitchBend(byte channel, int bend) {
  uint16_t uint_bend = (uint16_t)(bend - MIDI_PITCHBEND_MIN);
  midiEventPacket_t evt = {0x0E, (byte)(0xE0 | (channel - 1)),
    (byte)(uint_bend & 0x7F), (byte)((uint_bend >> 7) & 0x7F)};
  MidiUSB.sendMIDI(evt);
}

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

void USBTimeCodeQuarterFrame(byte data)
{
  midiEventPacket_t evt = {0x02, midi::TimeCodeQuarterFrame, data, 0};
  MidiUSB.sendMIDI(evt);
}

void USBSongSelect(byte songnumber)
{
  midiEventPacket_t evt = {0x02, midi::SongSelect, songnumber, 0};
  MidiUSB.sendMIDI(evt);
}

void USBSongPosition(unsigned beats)
{
  midiEventPacket_t evt = {0x03, midi::SongPosition, (byte)(beats & 0x7F), 
    (byte)((beats >> 7) & 0x7F)};
  MidiUSB.sendMIDI(evt);
}

inline void USBRealTime(midi::MidiType midiType)
{
  midiEventPacket_t evt = {0x0F, (byte)midiType, 0, 0};
  MidiUSB.sendMIDI(evt);
}

void USBTuneRequest(void)
{
  USBRealTime(midi::TuneRequest);
}

void USBTimingClock(void)
{
  USBRealTime(midi::Clock);
}

void USBStart(void)
{
  USBRealTime(midi::Start);
}

void USBContinue(void)
{
  USBRealTime(midi::Continue);
}

void USBStop(void)
{
  USBRealTime(midi::Stop);
}

void USBActiveSensing(void)
{
  USBRealTime(midi::ActiveSensing);
}

void USBSystemReset(void)
{
  USBRealTime(midi::SystemReset);
}

/*************** MIDI UART callback functions ****************************/

void UARThandleNoteOn(byte inChannel, byte inNumber, byte inVelocity)
{
  DBGSERIAL.print(F("NoteOn  note: "));
  DBGSERIAL.print(inNumber);
  DBGSERIAL.print(F("\tvelocity: "));
  DBGSERIAL.print(inVelocity);
  DBGSERIAL.print(F("\tchannel: "));
  DBGSERIAL.println(inChannel);
  USBNoteOn(inChannel, inNumber, inVelocity);
}
void UARThandleNoteOff(byte inChannel, byte inNumber, byte inVelocity)
{
  DBGSERIAL.print(F("NoteOff note: "));
  DBGSERIAL.print(inNumber);
  DBGSERIAL.print(F("\tvelocity: "));
  DBGSERIAL.print(inVelocity);
  DBGSERIAL.print(F("\tchannel: "));
  DBGSERIAL.println(inChannel);
  USBNoteOff(inChannel, inNumber, inVelocity);
}

void UARThandleAfterTouchPoly(byte channel, byte note, byte pressure)
{
  DBGSERIAL.print(F("AfterTouchPoly note: "));
  DBGSERIAL.print(note);
  DBGSERIAL.print(F("\tpressure: "));
  DBGSERIAL.print(pressure);
  DBGSERIAL.print(F("\tchannel: "));
  DBGSERIAL.println(channel);
  USBAfterTouchPoly(channel, note, pressure);
}

void UARThandleControlChange(byte channel, byte number, byte value)
{
  DBGSERIAL.print(F("ControlChange number: "));
  DBGSERIAL.print(number);
  DBGSERIAL.print(F("\tvalue: "));
  DBGSERIAL.print(value);
  DBGSERIAL.print(F("\tchannel: "));
  DBGSERIAL.println(channel);
  USBControlChange(channel, number, value);
}

void UARThandleProgramChange(byte channel, byte number)
{
  DBGSERIAL.print(F("ProgramChange number: "));
  DBGSERIAL.print(number);
  DBGSERIAL.print(F("\tchannel: "));
  DBGSERIAL.println(channel);
  USBProgramChange(channel, number);
}

void UARThandleAfterTouchChannel(byte channel, byte pressure)
{
  DBGSERIAL.print(F("AfterTouchChannel pressure: "));
  DBGSERIAL.print(pressure);
  DBGSERIAL.print(F("\tchannel: "));
  DBGSERIAL.println(channel);
  USBAfterTouchChannel(channel, pressure);
}

void UARThandlePitchBend(byte channel, int bend)
{
  DBGSERIAL.print(F("PitchBend bend: "));
  DBGSERIAL.print(bend & 0x3FFF, HEX);
  DBGSERIAL.print(F("\tchannel: "));
  DBGSERIAL.println(channel);
  USBPitchBend(channel, bend);
}

void UARThandleSystemExclusive(byte* array, unsigned size)
{
  DBGSERIAL.print(F("SystemExclusive size: "));
  DBGSERIAL.println(size);
  USBSystemExclusive(size, array, true);
}

void UARThandleTimeCodeQuarterFrame(byte data)
{
  DBGSERIAL.print(F("TimeCodeQuarterFrame data: "));
  DBGSERIAL.println(data);
  //MIDI.sendTimeCodeQuarterFrame((data & 0x70) >> 4, data & 0x0F);
}

void UARThandleSongPosition(unsigned beats)
{
  DBGSERIAL.print(F("SongPosition beats: "));
  DBGSERIAL.println(beats);
  USBSongPosition(beats);
}

void UARThandleSongSelect(byte songnumber)
{
  DBGSERIAL.print(F("SongSelect number: "));
  DBGSERIAL.println(songnumber);
  USBSongSelect(songnumber);
}

void UARThandleTuneRequest(void)
{
  DBGSERIAL.println(F("TuneRequest"));
  USBTuneRequest();
}

void UARThandleClock(void)
{
  DBGSERIAL.println(F("Clock"));
  USBTimingClock();
}

void UARThandleStart(void)
{
  DBGSERIAL.println(F("Start"));
  USBStart();
}

void UARThandleContinue(void)
{
  DBGSERIAL.println(F("Continue"));
  USBContinue();
}

void UARThandleStop(void)
{
  DBGSERIAL.println(F("Stop"));
  USBStop();
}

void UARThandleActiveSensing(void)
{
  DBGSERIAL.println(F("ActiveSensing"));
  USBActiveSensing();
}

void UARThandleSystemReset(void)
{
  DBGSERIAL.println(F("SystemReset"));
  USBSystemReset();
}

void setup() {
  DBGSERIAL.begin(115200);

  MIDI.begin();
  MIDI.turnThruOff();
  MIDI.setHandleNoteOn(UARThandleNoteOn);
  MIDI.setHandleNoteOff(UARThandleNoteOff);
  MIDI.setHandleAfterTouchPoly(UARThandleAfterTouchPoly);
  MIDI.setHandleControlChange(UARThandleControlChange);
  MIDI.setHandleProgramChange(UARThandleProgramChange);
  MIDI.setHandleAfterTouchChannel(UARThandleAfterTouchChannel);
  MIDI.setHandlePitchBend(UARThandlePitchBend);
  MIDI.setHandleSystemExclusive(UARThandleSystemExclusive);
  MIDI.setHandleTimeCodeQuarterFrame(UARThandleTimeCodeQuarterFrame);
  MIDI.setHandleSongPosition(UARThandleSongPosition);
  MIDI.setHandleSongSelect(UARThandleSongSelect);
  MIDI.setHandleTuneRequest(UARThandleTuneRequest);
  MIDI.setHandleClock(UARThandleClock);
  MIDI.setHandleStart(UARThandleStart);
  MIDI.setHandleContinue(UARThandleContinue);
  MIDI.setHandleStop(UARThandleStop);
  MIDI.setHandleActiveSensing(UARThandleActiveSensing);
  MIDI.setHandleSystemReset(UARThandleSystemReset);
}

uint32_t sysexSize = 0;

void loop()
{
  // MIDI UART -> MIDI USB
  MIDI.read();
  MidiUSB.flush();

  // MIDI USB -> MIDI UART
  midiEventPacket_t rx;
  rx = MidiUSB.read();
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
