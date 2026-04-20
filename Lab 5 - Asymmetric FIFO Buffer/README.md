# Lab 5 Assymetric FIFO Buffer

## Overview

Please refer to the following PDF file for detailed instructions and description of the lab:
- [Lab Instructions](images/Lab%205%20-%20Asymmetric%20FIFO%20Buffer.pdf)

##  Schematic

![Lab 5 Schematic](images/Lab%204%20Circuit%20Schematic.png)

## Simulation Waveform

![Simulation Waveform](images/Screenshot%202026-03-02%20120932.png)

In short, a write command will write 16 bits of data into the buffer and a read comand will read 8 bits, but the slots are only 8 bits lon.g Thus, the following are specific cases I took into account of:

1.  FIFO full when < 2 slots are open
      - Since write commands wil write 16 bits of data and slots are 8 bits long, issuing one read command after a FIFO is full is not enough for a write to occur. Thus it remains full.
        
2. Empty + (Simultanoues Read + Write) = Write and Don't Move Read Ptr
      - When empty, a Simultanoues Read + Write only moves the write pointer, but not the read. It allows for the lower byte of the just written word to be read and then be able to read the upper byte on the next read command.

3. Full + (Simultanoues Read + Write) = Only Read
      - When full, a Simultanoues Read + Write only reads because write commands are 16 bits long, so FIFO is full when < 2 slots are open. Until > 2 slots are open, that is when a write can be done
