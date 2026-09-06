# FPGA Interface

The STM32U545 owns game state and simulation. The FPGA owns rendering and video timing.

The MCU sends complete render snapshots to the FPGA over SPI.

## Signals

| Signal | Direction | Behaviour |
| --- | --- | --- |
| SCK | MCU → FPGA | SPI clock |
| MISO | FPGA → MCU | SPI data |
| MOSI | MCU → FPGA | SPI data |
| CS | MCU → FPGA | active low |
| READY | FPGA → MCU | active high level |
| IRQ | FPGA → MCU | active high held level |
| FRAME | FPGA → MCU | active high pulse |
| AUX0 | MCU → FPGA | reserved |
| AUX1 | MCU → FPGA | reserved |

Pin mapping and electrical biasing are documented in [`hardware.md`](hardware.md).

## SPI

The STM32U545 is the SPI master.

| Property | Value |
| --- | --- |
| Mode | 0 |
| Data size | 8 bit |
| Bit order | MSB first |
| Chip select | software controlled, active low |
| Baseline clock | 10 MHz |

A transaction is active while `CS` is low.

## READY

`READY` indicates whether the FPGA is initialized and available for communication.

| READY | State |
| --- | --- |
| LOW | not ready |
| HIGH | ready |

Communication begins after `READY` becomes high.

## IRQ

`IRQ` indicates a pending FPGA condition requiring MCU service.

The FPGA holds `IRQ` high until the condition has been serviced. The MCU detects the rising edge, while the held level preserves the pending state if servicing is delayed.

`IRQ` remains asserted until the pending condition is serviced. The conditions represented by `IRQ` and their acknowledgement through the SPI protocol are subject to revision.

## FRAME

`FRAME` produces one active-high pulse at each video frame boundary.

The MCU detects the rising edge.

`FRAME` reports FPGA video timing and does not drive the MCU game simulation.

## AUX0 and AUX1

`AUX0` and `AUX1` are reserved MCU outputs and are initially low.

No baseline protocol depends on them.

## Timing and State Updates

The game simulation runs independently at approximately 60 Hz.

Each update produces a complete render snapshot:

    game simulation
          │
          ▼
    render snapshot
          │
         SPI
          │
          ▼
    pending FPGA state
          │
     frame boundary
          │
          ▼
     active FPGA state
          │
          ▼
       rendering

Receiving a snapshot updates pending state only. At a suitable frame boundary, the FPGA atomically makes the complete pending snapshot active.

If no new snapshot is available, the current active state remains in use.

A snapshot is expected to be approximately 64 bytes. At 60 updates per second this is approximately 3.84 kB/s. A 64-byte transfer at 10 MHz takes approximately 51.2 µs.

## Render Snapshot

A render snapshot contains the state required for rendering, including information such as:

- game state
- player positions
- orientation
- animation state
- health
- map state
- effects

The exact representation is subject to revision.

## Startup and Failure Behaviour

The interface starts with `CS` high and `READY`, `IRQ` and `FRAME` low through their external biasing. `AUX0` and `AUX1` are driven low after MCU GPIO initialization.

Neither side depends on a particular reset completion order.

An incomplete or invalid transfer does not replace active render state. If no new valid snapshot is available, the FPGA continues rendering the current state.

## Protocol

The SPI payload format is subject to revision.

It will specify:

- message type and payload length
- render snapshot layout
- field representation and byte order
- FPGA status
- IRQ conditions and acknowledgement
- validation and versioning

Protocol fields use fixed-size representations rather than compiler-dependent structure layout.
