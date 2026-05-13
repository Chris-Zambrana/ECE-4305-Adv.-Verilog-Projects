# Sonar Object Detection System

## Overview

This final project is a Sonar Object Detection system implemented on a custom embedded SoC pipeline on the Nexys A7-100T FPGA board where:
- MicroBlaze software controls MMIO and video cores via a custom bridge and slot-based bus
- custom HDL modules implement radar overlay, sensor UART intake, and servo PWM actuation
- custom C++ drivers expose the registers to those cores cleanly to software
- the main application fuses keyboard input, compass/I2C, ultrasonic UART distance, servo sweep, and VGA rendering into one real-time radar-style interface

## Video Demo
https://youtu.be/jW2u8r9myFY


## SoC Breakdown

This document provides a full breakdown of the SoC system used in this final project, including:
- HDL subsystems
- Custom bus/addressing structure
- Subsystem Components and Cores
- Core drivers
- Custom HDL cores added for this project
- Custom C++ drivers and the full application flow

---

### 1) SoC Architecture (Top-Level System)

The top-level integrates:
- **MicroBlaze MCS** soft processor (`cpu.xci`) running at 100 MHz
- **Clock generation** via `mmcm_fpro` (100 MHz system and 25 MHz VGA)
- **MCS-to-FPGA bridge** (`chu_mcs_bridge`) that maps CPU I/O transactions into:
  - MMIO subsystem space
  - Video subsystem space
- **MMIO subsystem** (`mmio_sys_sampler`)
- **Video subsystem** (`video_sys_daisy`)

Major board I/O tied into the SoC:
- UART (console + sensor UART)
- switches/LEDs/buttons
- VGA
- SPI, I2C, PS/2
- XADC analog channels
- PWM outputs (RGB LEDs + servo)
- audio output path (DDFS + ADSR/PDM)
- Seven Segment Display

---

### 2) Custom Bus / Addressing Model

#### 2.1 MCS Bridge Layer

- Bridge base address: `0xC0000000` (`BRIDGE_BASE`)
- `io_address[23]` selects target domain:
  - `0` -> MMIO subsystem
  - `1` -> Video subsystem
- Read/write strobes and data are translated directly to FPGA-side signals.

#### 2.2 MMIO Slot Bus

MMIO addressing:
- `mmio_addr[10:5]` = slot number (64 slots max)
- `mmio_addr[4:0]`  = register index in slot (32 registers/slot)

Controller behavior:
- decodes one slot chip-select
- broadcasts read/write/address/write-data to all slots
- returns read data from selected slot via mux

C++ helper mapping:
- `get_slot_addr(base,slot) = base + slot*32*4`

#### 2.3 Video Bus

Video addressing:
- `video_addr[20]` selects memory region:
  - `1` = framebuffer
  - `0` = video core slot space
- `video_addr[16:14]` = video slot index (8 slots)
- `video_addr[13:0]` = slot-local register address

C++ helpers:
- `FRAME_BASE = BRIDGE_BASE + 0x00c00000`
- `get_sprite_addr(base,sprite) = base + 0x00800000 + sprite*16384*4`

---

### 3) HDL Subsystems and Core Slot Mapping

#### 3.1 MMIO Subsystem Slots (`mmio_sys_sampler.sv`)

- **S0**  -> `chu_timer` (system timer)
- **S1**  -> `chu_uart` (console UART)
- **S2**  -> `chu_gpo` (LED output)
- **S3**  -> `chu_gpi` (switch input)
- **S4**  -> reserved user slot (currently returns 0)
- **S5**  -> `chu_xadc_core`
- **S6**  -> `chu_io_pwm_core` (RGB PWM)
- **S7**  -> `chu_debounce_core` (buttons)
- **S8**  -> `chu_led_mux_core` (7-seg)
- **S9**  -> `chu_spi_core`
- **S10** -> `chu_i2c_core`
- **S11** -> `chu_ps2_core`
- **S12** -> `chu_ddfs_core`
- **S13** -> `chu_adsr_core`
- **S14** -> `chu_uart_sensor` (**custom ultrasonic sensor UART path**)
- **S15** -> `chu_io_pwm_servo` (**custom servo PWM core**)

#### 3.2 Video Subsystem Pipeline (`video_sys_daisy.sv`)

Pixel stream chain:

`FrameBuffer -> V7 Radar -> V6 Gray -> V5 Dummy -> V4 Dummy -> V3 Ghost -> V2 OSD -> V1 Mouse -> V0 VGA Sync`

Video slots:
- **V0** VGA sync/output core
- **V1** mouse sprite core (Unused)
- **V2** OSD core
- **V3** ghost sprite core (Unused)
- **V4** dummy (user placeholder)
- **V5** dummy (user placeholder)
- **V6** RGB-to-gray core
- **V7** **custom radar overlay core**

---

### 4) Driver Layer (C++)

Core driver classes include:
- `TimerCore`, `UartCore`
- `GpiCore`, `GpoCore`, `PwmCore`, `DebounceCore`
- `SsegCore`, `XadcCore`
- `SpiCore`, `I2cCore`, `Ps2Core`
- `DdfsCore`, `AdsrCore`
- Video-side classes: `FrameCore`, `GpvCore`, `SpriteCore`, `OsdCore`, `RadarCore`
- System mapping/utilities: `chu_io_map.h`, `chu_io_rw.h`, `chu_init.h/.cpp`

These drivers map software operations directly onto the MMIO and video register maps used by the HDL subsystems.

---

### 5) Custom HDL Cores Added for This Project

#### 5.1 Custom Radar Video Core
**Files:**
- `.../sources_1/new/chu_vga_radar_core.sv`
- `.../sources_1/new/radar_overlay_engine.sv`
- `.../sources_1/new/radar_angle_lut.sv`

Function:
- Procedural radar overlay in the video stream (without modifying frame memory directly)
- Supports:
  - 180-degree and 360-degree scan behavior
  - manual heading mode
  - active sweep line
  - fading sweep trail (history-based)
  - red object-detection beam
- Controlled by memory-mapped registers in video slot V7 (origin, angle, distance, mode, color, thickness, fade config, bypass)

Register Map:
- [Radar Video Core Register Map](images/Lab%2011%20-%20Keyboard%20or%20mouse%20controlled%20chasing%20LED.pdf)

#### 5.2 Custom Sensor UART Core
**Files:**
- `.../sources_1/imports/new/chu_uart_sensor.sv`
- `.../sources_1/new/uart_sensor.sv`
- `.../sources_1/new/uart_rx_sensor.sv`

Function:
- Dedicated UART core path for ultrasonic sensor input in MMIO slot S14
- Maintains UART-like register model (baud, RX/TX data/status, remove-read)
- Includes project-specific RX behavior in `uart_rx_sensor` to handle inverse RS232

Register Map:
- [Sensor UART Core Register Map](images/Lab%2011%20-%20Keyboard%20or%20mouse%20controlled%20chasing%20LED.pdf)

##### UART Polarity Note
- The sensor outputs the **inverse** of standard RS232 (UART) polarity.
- In other words, instead of normal UART assumptions (`idle = 1`, `start_bit = 0`, and data as listed directly in the datasheet), the ultrasonic output is the opposite polarity (`idle = 0`, `start_bit = 1`, and inverted data bits).
- Example: if the sensor sends `R222<CR>` in ASCII, each byte appears inverted on the wire.

Standard RS232:
- `'R' = 0x52`
- `'2' = 0x32`
- `<CR> = 0x0D`

LV-MaxSonar (inverted):
- `'R' = ~0x52 = 0xAD`
- `'2' = ~0x32 = 0xCD`
- `<CR> = ~0x0D = 0xF2`

#### 5.3 Custom Servo PWM Core
**File:**
- `.../sources_1/imports/new/chu_io_pwm_servo.sv`

Function:
- Dedicated PWM core for servo control in slot S15
- Configured with:
  - `W=1` output channel
  - `R=21` resolution bits
- Supports stable duty-cycle control appropriate for servo pulse timing

Register Map:
- [Servo PWM Core Register Map](images/Lab%2011%20-%20Keyboard%20or%20mouse%20controlled%20chasing%20LED.pdf)

##### PWM Resolution Note
- PWM period is standardized to 20 ms (50 Hz).
- We need to determine PWM resolution (`R`):

`f_pwm = f_sys / 2^R`

`50 Hz = 100 MHz / 2^R`

`2^R = 100 MHz / 50 Hz`

`2^R = 2,000,000`

`log2(2^R) = log2(2,000,000) ≈ 20.93 -> R = 21`

### Note
For more information regarding the specifications of the subsystems, cores, and SoC system in general, refer to my notebook with notes on the system:

https://livecsupomona-my.sharepoint.com/:o:/r/personal/czambrana_cpp_edu/Documents/Notebooks/ECE4305%20Advanced%20Verilog%20Notes?d=w7deaf97db06445ac8aac9f42d94dc5be&csf=1&web=1&e=hlKGyK

---

### 6) Custom Drivers and Software Extensions Added for This Project

#### 6.1 `SensorUartCore` (`sensor_uart_core.h/.cpp`)
- Dedicated driver for slot S14.
- Parses ultrasonic packet format:
  - `'R' + 3 ASCII digits + CR`
- Includes packet resynchronization logic for corrupted/misaligned bytes.
- Returns distance value in inches.

#### 6.2 `ServoPwmCore` (`servo_pwm_core.h/.cpp`)
- Dedicated driver for slot S15 servo PWM.
- Sets servo PWM frequency (50 Hz used by application).
- Supports duty control by integer value and normalized floating-point value.

#### 6.3 `RadarCore` extension (`vga_core.h/.cpp`)
- Added as a custom video driver class for V7 radar overlay.
- Exposes APIs for:
  - `set_origin()`
  - `set_angle()`
  - `set_distance()`
  - `set_mode()`
  - `set_color()`
  - `set_thickness()`
  - `set_fade()`
  - `update_scan()`
  - object-detection flag control

#### 6.4 `FrameCore` extension (`vga_core.h/.cpp`)
- Added helper drawing functions used by radar background rendering:
  - `plot_half_circle()`
  - `fill_half_circle()`

---

### 7) Application Process (Main Program)

Main application file:  
`/home/runner/work/ECE-4305-Adv.-Verilog-Projects/ECE-4305-Adv.-Verilog-Projects/Final Project /code/c++/Main_App/main_video_test.cpp`

#### 7.1 Runtime Initialization
- Instantiates MMIO and video drivers mapped to the slot table.
- Enables/disables bypass on video modules (frame/radar/osd active initially).
- Configures radar overlay defaults:
  - center/origin
  - color
  - mode
  - line thickness and fade settings
- Initializes servo PWM at 50 Hz.
- Initializes PS/2 interface for keyboard input.

#### 7.2 Core Loop Sequence
In each loop iteration:
1. `keyboard_check(...)`
   - Reads keyboard commands over PS/2
   - Updates operation mode, sweep speed, display color, and pause state
2. `scanning_check(...)`
   - Selects behavior based on mode:
     - mode 0: 180-degree servo scan
     - mode 1: 360-degree scan
     - mode 2: manual compass mode
   - Updates heading/servo/OSD/7-seg state as needed
3. `ultrasonic_check(...)`
   - Reads distance via `SensorUartCore`
   - Updates OSD and 7-seg display
   - Updates RGB LED state
   - Pushes angle+distance to radar overlay via `RadarCore::update_scan(...)`
4. Short timing delay (`sleep_ms(15)`)

#### 7.3 User Interaction Model
Keyboard controls include:
- `M/m` -> select operation mode
- `S/s` -> set sweep speed (non-manual modes)
- `C/c` -> select radar color theme
- `P/p` -> pause/resume

---

## References

Nexys A7-100T Reference Manual:
https://digilent.com/reference/programmable-logic/nexys-a7/reference-manual?redirect=1&__cf_chl_tk=3olcm7D2jZhVYePhm8Eoqw8_Waufl0ESAK.RQkqlD1s-1777770536-1.0.1.1-zqbEIc2h6hBOkQ8BjLl6Ye0USDcfKN67HLgPPyNxlCY

Maxsonar Ultrasonic Senesor Datasheet:
https://maxbotix.com/pages/lv-maxsonar-ez-datasheet

3-axis Compass Pmod Reference Manual
https://digilent.com/reference/pmod/pmodcmps2/reference-manual?srsltid=AfmBOoqXetBwuNnBnsHk4Tevmn4Yf_rbjkUW1X1OsUz7Hqp8ADtMFAe5

3-axis Compass Mangometer Datasheet
https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/1793/MMC3416xPJ.pdf

180 Degree Positional Servo 
https://www.amazon.de/-/en/Miuzei-Walking-Helicopter-Vehicle-Control/dp/B0G3WWTXLV?th=1
