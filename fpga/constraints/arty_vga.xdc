## ============================================================
## FPGA configuration voltage
## ============================================================

set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]

## ============================================================
## 100 MHz system clock
## ============================================================

set_property -dict { PACKAGE_PIN E3 IOSTANDARD LVCMOS33 } [get_ports clk]
create_clock -name sys_clk_pin -period 10.00 -waveform {0 5} [get_ports clk]


## ============================================================
## Reset button - BTN0
## ============================================================

set_property -dict { PACKAGE_PIN D9 IOSTANDARD LVCMOS33 } [get_ports reset]


## ============================================================
## Pmod VGA - Header JB
##
## JB carries:
##   Red[3:0]
##   Blue[3:0]
## ============================================================

# Red
set_property -dict { PACKAGE_PIN E15 IOSTANDARD LVCMOS33 } [get_ports { red[0] }]
set_property -dict { PACKAGE_PIN E16 IOSTANDARD LVCMOS33 } [get_ports { red[1] }]
set_property -dict { PACKAGE_PIN D15 IOSTANDARD LVCMOS33 } [get_ports { red[2] }]
set_property -dict { PACKAGE_PIN C15 IOSTANDARD LVCMOS33 } [get_ports { red[3] }]

# Blue
set_property -dict { PACKAGE_PIN J17 IOSTANDARD LVCMOS33 } [get_ports { blue[0] }]
set_property -dict { PACKAGE_PIN J18 IOSTANDARD LVCMOS33 } [get_ports { blue[1] }]
set_property -dict { PACKAGE_PIN K15 IOSTANDARD LVCMOS33 } [get_ports { blue[2] }]
set_property -dict { PACKAGE_PIN J15 IOSTANDARD LVCMOS33 } [get_ports { blue[3] }]


## ============================================================
## Pmod VGA - Header JC
##
## JC carries:
##   Green[3:0]
##   HSYNC
##   VSYNC
## ============================================================

# Green
set_property -dict { PACKAGE_PIN U12 IOSTANDARD LVCMOS33 } [get_ports { green[0] }]
set_property -dict { PACKAGE_PIN V12 IOSTANDARD LVCMOS33 } [get_ports { green[1] }]
set_property -dict { PACKAGE_PIN V10 IOSTANDARD LVCMOS33 } [get_ports { green[2] }]
set_property -dict { PACKAGE_PIN V11 IOSTANDARD LVCMOS33 } [get_ports { green[3] }]

# Synchronization
set_property -dict { PACKAGE_PIN U14 IOSTANDARD LVCMOS33 } [get_ports hsync]
set_property -dict { PACKAGE_PIN V14 IOSTANDARD LVCMOS33 } [get_ports vsync]