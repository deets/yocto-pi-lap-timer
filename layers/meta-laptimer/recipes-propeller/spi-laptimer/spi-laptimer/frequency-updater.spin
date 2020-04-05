{{

  Frequency Updates

  Author: Diez Roggisch

  Version 1.0

  Date: 08 April 2020

}}

CON
  RTC_CLK = 19
  RTC_DATA = 18
  ' Start of the RTC CS
  RTC_CS = 20

  TX_PIN  = 30
  RX_PIN  = 31
  SERIAL_BPS = 115200

OBJ
  rtc6715: "RTC6715"
  'serial: "FullDuplexSerial"

VAR
   word frequencies[8]
   long stackspace[640]
   byte cog_id

PUB start(frequency_table_p, rtc_count)
  cog_id := cognew(run(frequency_table_p, rtc_count), @stackspace)


PUB stop
  if cog_id > -1
    cogstop(cog_id)

PRI run(frequency_table, rtc_count)  | i, freq
  'serial.Start(RX_PIN, TX_PIN, 0, SERIAL_BPS)
  rtc6715.init(RTC_CLK, RTC_DATA)

  repeat i from 0 to rtc_count - 1
    frequencies[i] := raceband[i]
    rtc6715.set_frequency(RTC_CS + i, raceband[i])
  repeat

    repeat i from 0 to rtc_count - 1
      freq := long[frequency_table + i] & $ffff ' we only expect the lower word
      'serial.str(string("incoming:"))
      'serial.dec(freq)
      'nl
      if freq <> frequencies[i]
        frequencies[i] := freq
        rtc6715.set_frequency(RTC_CS + i, freq)
        'serial.dec(freq)
        'nl

PRI nl
    'serial.tx(13)
    'serial.tx(10)


DAT
raceband word 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917


 {{
┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                                                   TERMS OF USE: MIT License                                                  │
├──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
│Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    │
│files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    │
│modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software│
│is furnished to do so, subject to the following conditions:                                                                   │
│                                                                                                                              │
│The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.│
│                                                                                                                              │
│THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          │
│WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         │
│COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   │
│ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         │
└──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
}}
