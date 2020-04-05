{{
SPI Laptimer using RX5808/RTC6715

(c) Diez Roggisch
}}

CON
  _clkmode = xtal1 + pll16x
  _xinfreq = 6_000_000

  DATAGRAM_SIZE = 9 ' including the leading control word
  BUFSIZE = 256 * DATAGRAM_SIZE
  SAMPLERATE = 2

  MPC_DATA_PIN = 17
  MPC_CLK_PIN = 16
  MPC_CS_PIN = 15

  RTC_COUNT = 1

  ' for debugging
  DEBUGPIN = 10

  TX_PIN  = 30
  RX_PIN  = 31
  SERIAL_BPS = 115200

VAR
    long cog
    long write_pos
    long ring_buffer[BUFSIZE]
    long incoming_buffer[DATAGRAM_SIZE]
OBJ
  mcp3008: "MCP3008"
  fu: "frequency-updater"
  serial: "FullDuplexSerial"

PUB main | h, start_ts, rssi, loopcount
    mcp3008.start(MPC_DATA_PIN, MPC_CLK_PIN, MPC_CS_PIN, (|< RTC_COUNT) - 1 )
    serial.Start(RX_PIN, TX_PIN, 0, SERIAL_BPS)

    fu.start(@incoming_buffer + 4, RTC_COUNT)
    dira[DEBUGPIN]~~
    write_pos := 0
    start(@write_pos)
    start_ts := cnt
    repeat
      start_ts += clkfreq / SAMPLERATE
      waitcnt(start_ts)
      rssi := mcp3008.in(0)

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

      print_incoming_buffer

PRI start(wp): okay
    okay := cognew(@spi_main, wp) + 1

PRI stop
    if cog
       cogstop(cog~ - 1)

PRI print_incoming_buffer | i
    repeat i from 0 to DATAGRAM_SIZE - 1
      serial.hex(incoming_buffer[i], 8)
      serial.tx(58)
    nl

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
             call      #fill_out_buffer
             call      #copy_input_buffer
             waitpne  cs_mask, cs_mask
             mov      wordcounter, #DATAGRAM_SIZE
             mov      d0, #out_buf
             movs     :word_read, d0
             mov      d1, #in_buf
             movd     :word_write, d1
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
:word_write  mov      0, incoming
             add      d1, #1
             movd     :word_write, d1
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
fill_out_buffer
             mov      mem_pointer, write_pos_addr
             rdlong   size, mem_pointer
             ' compute the amount of data in the ringbuffer
             sub      size, read_pos wc, wz
             ' ensure we correct for wrap-around
if_c         add      size, buf_size
             mov      out_buf, size
             ' if the size is zero, we don't have any
             ' data to copy. the transaction will still
             ' copy over the old data, but we don't have to
             ' care about that.
if_z         jmp      #fill_out_buffer_ret
             ' adjust our copy instruction to
             ' the beginning of our buffer beyond size
             mov      buf_pos, #out_buf
             mov      wordcounter, #DATAGRAM_SIZE
:copy_loop
             ' point to the next buffer element
             add      buf_pos, #1
             movd     :read_long, buf_pos
             ' move forward to our actual
             ' read position
             mov      mem_pointer, write_pos_addr
             mov      d0, read_pos
             add      d0, #1 ' offset the write pos
             shl      d0, #2
             add      mem_pointer, d0
:read_long   rdlong   0, mem_pointer
             ' increment the read-pos and
             ' wrap it around at the end
             add      read_pos, #1
             cmp      read_pos, buf_size wz
if_z         mov      read_pos, #0
             djnz     wordcounter, #:copy_loop
fill_out_buffer_ret ret


copy_input_buffer
             mov      mem_pointer, write_pos_addr
             mov      d0, #1 ' skip write_pos
             add      d0, buf_size ' skip the ring-buffer
             shl      d0, #2
             add      mem_pointer, d0
             mov      buf_pos, #in_buf
             movd     :write_long, buf_pos
             mov      wordcounter, #DATAGRAM_SIZE
:copy_loop
:write_long  wrlong   0, mem_pointer
             add      buf_pos, #1
             movd     :write_long, buf_pos
             add      mem_pointer, #4
             djnz     wordcounter, #:copy_loop
copy_input_buffer_ret ret


cs_mask  long $1           ' A0 .. A3
clk_mask long $2
mosi_mask long $4
miso_mask long $8
' buffer size doesn't fit into a 9-bit
' space anymore, thus needs to be held here
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
' itself, then the incoming buffer
mem_pointer long 0
' data registers
d0       long 0
d1       long 0

size     long 0
' points to our current offset into
' either out_buf or in_buf
buf_pos  long 0

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
in_buf   long 100
         long 200
         long 300
         long 400
         long 500
         long 600
         long 700
         long 800
         long 900
