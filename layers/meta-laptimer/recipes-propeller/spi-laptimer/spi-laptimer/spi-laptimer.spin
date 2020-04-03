{{
SPI Laptimer using RX5808/RTC6715

(c) Diez Roggisch
}}

CON
  _clkmode = xtal1 + pll16x
  _xinfreq = 6_000_000

  DATAGRAM_SIZE = 8
  BUFSIZE = 256 * DATAGRAM_SIZE
  SAMPLERATE = 2

  MPC_DATA_PIN = 17
  MPC_CLK_PIN = 16
  MPC_CS_PIN = 15

  RTC_CLK = 22
  RTC_DATA = 24
  RTC_COUNT = 1

  RTC_CS = 20 ' for now just one RTC

  ADC_ADJUST_B = 93 ' see scripts/adc-spread-calc.py
  ADJ_ADJUST_A = 46942

  ' for debugging
  DEBUGPIN = 10
  TX_PIN  = 30
  RX_PIN  = 31
  SERIAL_BPS = 115200

VAR
    long cog
    long write_pos
    long ring_buffer[BUFSIZE]

OBJ
  mcp3008: "MCP3008"
  serial: "FullDuplexSerial"

PUB main | h, start_ts, rssi, loopcount
    mcp3008.start(MPC_DATA_PIN, MPC_CLK_PIN, MPC_CS_PIN, (|< RTC_COUNT) - 1 )
    serial.Start(RX_PIN, TX_PIN, 0, SERIAL_BPS)
    dira[DEBUGPIN]~~
    write_pos := 0
    start(@write_pos)
    start_ts := cnt
    repeat
      serial.str(string("loop:"))
      nl
      start_ts += clkfreq / SAMPLERATE
      waitcnt(start_ts)
      rssi := mcp3008.in(0)
      ' rssi -= ADC_ADJUST_B
      ' rssi *= ADJ_ADJUST_A

      ' we use h here to write
      ' write_pos only after a full datagram
      ' has been written
      h := write_pos
      ring_buffer[h++] := cnt
      ring_buffer[h++] := rssi
      repeat DATAGRAM_SIZE - 2
        ring_buffer[h] := cnt
        h := (h + 1) // BUFSIZE
      write_pos := h

PRI start(wp): okay
    okay := cognew(@spi_main, wp) + 1

PRI stop
    if cog
       cogstop(cog~ - 1)

PRI nl
    serial.tx(13)
    serial.tx(10)

DAT org 0

spi_main
             mov       write_pos_addr, par
             mov       miso_mask, #1        nr, wz ' force wz to 0
             muxz      outa, miso_mask             ' set miso low
             muxnz     dira, miso_mask             ' and turn it output
:cs_loop
             call       #fill_buffer
             waitpne  cs_mask, cs_mask
             mov      wordcounter, #DATAGRAM_SIZE + 1
             mov      d0, #out_buf
             movs     :word_read, d0
:word_loop
             mov      bitcount, #32
:word_read   mov      buffer, d0
:bit_loop
             shl      buffer, #1       wc
             muxc     outa, miso_mask
             waitpeq  clk_mask, clk_mask    ' clk high
             test     mosi_mask, ina   wc
             rcl      incoming, #1
             waitpne  clk_mask, clk_mask    ' clk low
             sub      bitcount, #1       wz
       if_nz jmp      #:bit_loop
             add      d0, #1
             movs     :word_read, d0
             djnz     wordcounter, #:word_loop

             waitpeq  cs_mask, cs_mask
             mov      buffer, #1                wz
             muxz     outa, miso_mask
             jmp      #:cs_loop

' Our buffer looks like this:
'
' size
' data[0..DATAGRAM_SIZE)
'
fill_buffer
             mov      read_pointer, write_pos_addr
             rdlong   size, read_pointer
             ' compute the amount of data in the ringbuffer
             sub      size, read_pos wc, wz
             ' ensure we correct for wrap-around
if_c         add      size, buf_size
             mov       out_buf, size
             ' if the size is zero, we don't have any
             ' data to copy. the transaction will still
             ' copy over the old data, but we don't have to
             ' care about that.
if_z         jmp      #fill_buffer_ret
             ' adjust our copy instruction to
             ' the beginning of our buffer beyond size
             mov      out_pos, #out_buf
             mov      wordcounter, #DATAGRAM_SIZE
:copy_loop
             ' point to the next buffer element
             add      out_pos, #1
             movd     :read_long, out_pos
             ' move forward to our actual
             ' read position
             mov      read_pointer, write_pos_addr
             mov      d0, read_pos
             add      d0, #1 ' offset the write pos
             shl      d0, #2
             add      read_pointer, d0
:read_long   rdlong   0, read_pointer
             ' increment the read-pos and
             ' wrap it around at the end
             add      read_pos, #1
             cmp      read_pos, buf_size wz
if_z         mov      read_pos, #0
             djnz     wordcounter, #:copy_loop
fill_buffer_ret ret


cs_mask  long $1           ' A0 .. A3
clk_mask long $2
mosi_mask long $4
miso_mask long $8
buf_size long BUFSIZE

wordcounter long 0
bitcount long 0
buffer   long 0
incoming long 0

write_pos_addr long 0
' this is our position in the ringbuffer
read_pos long 0
' the pointer into the main memory, at
' first the write position, then the ringbuffer
' itself.
read_pointer long 0
' data registers
d0       long 0

size     long 0
out_pos  long $0

'out_buf  res DATAGRAM_SIZE + 1
out_buf  long 9
         long 8
         long 7
         long 6
         long 5
         long 4
         long 3
         long 2
         long 1
