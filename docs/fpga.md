# FPGA Interface

The STM32U545 owns game state and simulation. The FPGA owns rendering and video output.

The MCU sends complete render snapshots to the FPGA over SPI.

## Signals

| Signal | Direction | Behaviour |
| --- | --- | --- |
| SCK | MCU → FPGA | SPI clock |
| MISO | FPGA → MCU | Reserved |
| MOSI | MCU → FPGA | Snapshot data |
| CS | MCU → FPGA | Active low |
| READY | FPGA → MCU | Active-high readiness level |
| IRQ | FPGA → MCU | Reserved, active-high held level |
| FRAME | FPGA → MCU | Active-high frame-boundary pulse |
| AUX0 | MCU → FPGA | Reserved, initially low |
| AUX1 | MCU → FPGA | Reserved, initially low |

Pin mapping and electrical connections are documented in [`hardware.md`](hardware.md).

## SPI

The STM32U545 is the SPI master.

| Property | Value |
| --- | --- |
| Mode | 0 |
| Data size | 8 bits |
| Bit order | MSB first |
| Chip select | Software controlled, active low |
| Baseline clock | 10 MHz |

The MCU checks `READY` before starting a snapshot transfer. Once a transfer begins, it completes even if `READY` becomes low.

A transfer begins when `CS` goes low and ends when `CS` goes high.

`FRAME` produces one pulse at each video frame boundary.

`IRQ` remains low in the baseline implementation.

## Rendering

The logical rendering area is **320 × 240 pixels**, displayed at **640 × 480** using 2× scaling.

Each snapshot contains 24 fixed layer slots. Each layer describes an image, source rectangle, destination position, depth and rendering flags.

Enabled layers have equal source and destination dimensions. Horizontal mirroring is supported.

The FPGA uses the latest valid snapshot for rendering without applying partially received data.

## Render Snapshot

Each snapshot contains 24 layer descriptors of 16 bytes each.

| Component | Size |
| --- | ---: |
| Layer descriptors | 384 bytes |
| CRC-16 | 2 bytes |
| Total | 386 bytes |

All 24 slots are transmitted in order.

### Layer Descriptor

| Offset | Field | Size | Description |
| ---: | --- | ---: | --- |
| 0 | `image_id` | 2 | Image identifier |
| 2 | `src_x` | 2 | Source X |
| 4 | `src_y` | 2 | Source Y |
| 6 | `width` | 2 | Rectangle width |
| 8 | `height` | 2 | Rectangle height |
| 10 | `dst_x` | 2 | Destination X |
| 12 | `dst_y` | 2 | Destination Y |
| 14 | `z` | 1 | Layer depth |
| 15 | `flags` | 1 | Layer flags |

All 16-bit fields are unsigned and little-endian.

`image_id` identifies an image available to the FPGA. `src_x` and `src_y` specify the source rectangle within that image. `dst_x` and `dst_y` specify its position in the logical rendering area.

### Flags

| Bit | Name | Behaviour |
| ---: | --- | --- |
| 0 | ENABLED | Layer is rendered |
| 1 | FLIP_X | Mirror horizontally |
| 2–7 | Reserved | Zero |

A disabled layer is transmitted as 16 zero bytes.

### Geometry

For enabled layers:

- `width` and `height` are greater than zero.
- `width` is at most 320 and `height` is at most 240.
- `dst_x + width` is at most 320.
- `dst_y + height` is at most 240.

The MCU clips enabled layers to the logical rendering area before transmission.

## Snapshot Transfer

Each SPI transfer contains the 384-byte snapshot payload followed by a two-byte CRC.

The CRC covers the payload in transmission order and is transmitted low byte first.

| CRC property | Value |
| --- | --- |
| Algorithm | CRC-16/IBM-3740 (CCITT-FALSE) |
| Polynomial | `0x1021` |
| Initial value | `0xFFFF` |
| Input reflection | False |
| Output reflection | False |
| Final XOR | `0x0000` |
| Check value for `123456789` | `0x29B1` |

An incomplete transfer or a CRC mismatch does not produce a new valid snapshot.

## Startup

`CS` is initially high. `READY`, `IRQ` and `FRAME` are initially low. `AUX0` and `AUX1` are initially low after MCU GPIO initialization.

The FPGA asserts `READY` when it can receive snapshot transfers. The MCU does not start a transfer while `READY` is low.
