# Custom PCB: XC7A100T-1FTG256C.
# Banks 0, 14, and 15 are connected to +3.3V.
# CFGBVS (E7) is connected to +3.3V.
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]

# Define the shared 100 MHz clock at the FPGA input.
create_clock -name sys_clk_pin -period 10.00 -waveform {0 5} [get_ports clk]

# 100 MHz oscillator: bank 14, MRCC input.
set_property -dict { PACKAGE_PIN N14 IOSTANDARD LVCMOS33 } [get_ports clk]

# Active-high user reset: bank 15.
set_property -dict { PACKAGE_PIN D16 IOSTANDARD LVCMOS33 } [get_ports reset]

# VGA red: MSB to LSB, bank 15.
set_property -dict { PACKAGE_PIN C8 IOSTANDARD LVCMOS33 } [get_ports {red[3]}]
set_property -dict { PACKAGE_PIN C9 IOSTANDARD LVCMOS33 } [get_ports {red[2]}]
set_property -dict { PACKAGE_PIN A8 IOSTANDARD LVCMOS33 } [get_ports {red[1]}]
set_property -dict { PACKAGE_PIN A9 IOSTANDARD LVCMOS33 } [get_ports {red[0]}]

# VGA green: MSB to LSB, bank 15.
set_property -dict { PACKAGE_PIN B9  IOSTANDARD LVCMOS33 } [get_ports {green[3]}]
set_property -dict { PACKAGE_PIN A10 IOSTANDARD LVCMOS33 } [get_ports {green[2]}]
set_property -dict { PACKAGE_PIN B10 IOSTANDARD LVCMOS33 } [get_ports {green[1]}]
set_property -dict { PACKAGE_PIN B11 IOSTANDARD LVCMOS33 } [get_ports {green[0]}]

# VGA blue: MSB to LSB, bank 15.
set_property -dict { PACKAGE_PIN B12 IOSTANDARD LVCMOS33 } [get_ports {blue[3]}]
set_property -dict { PACKAGE_PIN A12 IOSTANDARD LVCMOS33 } [get_ports {blue[2]}]
set_property -dict { PACKAGE_PIN A13 IOSTANDARD LVCMOS33 } [get_ports {blue[1]}]
set_property -dict { PACKAGE_PIN A14 IOSTANDARD LVCMOS33 } [get_ports {blue[0]}]

# Active-low VGA synchronization: bank 15.
set_property -dict { PACKAGE_PIN C14 IOSTANDARD LVCMOS33 } [get_ports hsync]
set_property -dict { PACKAGE_PIN B14 IOSTANDARD LVCMOS33 } [get_ports vsync]

