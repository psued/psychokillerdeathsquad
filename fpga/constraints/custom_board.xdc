# Custom PCB: XC7A100T-1FTG256C.
# Banks 0, 14, and 15 are connected to +3.3V.
# CFGBVS (E7) is connected to +3.3V.
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]

# Define the shared 100 MHz clock at the FPGA input.
create_clock -period 10.000 -name sys_clk_pin -waveform {0.000 5.000} [get_ports clk]

# 100 MHz oscillator: bank 14, MRCC input.
set_property -dict {PACKAGE_PIN N14 IOSTANDARD LVCMOS33} [get_ports clk]

# Active-high user reset: bank 15.
set_property -dict {PACKAGE_PIN D16 IOSTANDARD LVCMOS33} [get_ports reset]

# VGA red: MSB to LSB, bank 15.
set_property -dict {PACKAGE_PIN C8 IOSTANDARD LVCMOS33} [get_ports {red[3]}]
set_property -dict {PACKAGE_PIN C9 IOSTANDARD LVCMOS33} [get_ports {red[2]}]
set_property -dict {PACKAGE_PIN A8 IOSTANDARD LVCMOS33} [get_ports {red[1]}]
set_property -dict {PACKAGE_PIN A9 IOSTANDARD LVCMOS33} [get_ports {red[0]}]

# VGA green: MSB to LSB, bank 15.
set_property -dict {PACKAGE_PIN B9 IOSTANDARD LVCMOS33} [get_ports {green[3]}]
set_property -dict {PACKAGE_PIN A10 IOSTANDARD LVCMOS33} [get_ports {green[2]}]
set_property -dict {PACKAGE_PIN B10 IOSTANDARD LVCMOS33} [get_ports {green[1]}]
set_property -dict {PACKAGE_PIN B11 IOSTANDARD LVCMOS33} [get_ports {green[0]}]

# VGA blue: MSB to LSB, bank 15.
set_property -dict {PACKAGE_PIN B12 IOSTANDARD LVCMOS33} [get_ports {blue[3]}]
set_property -dict {PACKAGE_PIN A12 IOSTANDARD LVCMOS33} [get_ports {blue[2]}]
set_property -dict {PACKAGE_PIN A13 IOSTANDARD LVCMOS33} [get_ports {blue[1]}]
set_property -dict {PACKAGE_PIN A14 IOSTANDARD LVCMOS33} [get_ports {blue[0]}]

# Active-low VGA synchronization: bank 15.
set_property -dict {PACKAGE_PIN C14 IOSTANDARD LVCMOS33} [get_ports hsync]
set_property -dict {PACKAGE_PIN B14 IOSTANDARD LVCMOS33} [get_ports vsync]

# SD card
set_property -dict {PACKAGE_PIN B15 IOSTANDARD LVCMOS33} [get_ports SD_SCLK]
set_property -dict {PACKAGE_PIN A15 IOSTANDARD LVCMOS33} [get_ports SD_DI]
set_property -dict {PACKAGE_PIN C16 IOSTANDARD LVCMOS33} [get_ports SD_DO]
set_property -dict {PACKAGE_PIN C12 IOSTANDARD LVCMOS33} [get_ports SD_CS]

# MCU communication
set_property -dict {PACKAGE_PIN T9 IOSTANDARD LVCMOS33} [get_ports MCU_SCLK]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets MCU_SCLK_IBUF]
set_property -dict {PACKAGE_PIN P8 IOSTANDARD LVCMOS33} [get_ports MCU_MISO]
set_property -dict {PACKAGE_PIN R8 IOSTANDARD LVCMOS33} [get_ports MCU_MOSI]
set_property -dict {PACKAGE_PIN T10 IOSTANDARD LVCMOS33} [get_ports MCU_CS]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets MCU_CS_IBUF]
set_property -dict {PACKAGE_PIN R7 IOSTANDARD LVCMOS33} [get_ports MCU_READY]


connect_debug_port u_ila_0/probe0 [get_nets [list {renderer_inst/scheduler/state_1[0]} {renderer_inst/scheduler/state_1[1]} {renderer_inst/scheduler/state_1[2]} {renderer_inst/scheduler/state_1[3]}]]


create_debug_core u_ila_0 ila
set_property ALL_PROBE_SAME_MU true [get_debug_cores u_ila_0]
set_property ALL_PROBE_SAME_MU_CNT 1 [get_debug_cores u_ila_0]
set_property C_ADV_TRIGGER false [get_debug_cores u_ila_0]
set_property C_DATA_DEPTH 1024 [get_debug_cores u_ila_0]
set_property C_EN_STRG_QUAL false [get_debug_cores u_ila_0]
set_property C_INPUT_PIPE_STAGES 0 [get_debug_cores u_ila_0]
set_property C_TRIGIN_EN false [get_debug_cores u_ila_0]
set_property C_TRIGOUT_EN false [get_debug_cores u_ila_0]
set_property port_width 1 [get_debug_ports u_ila_0/clk]
connect_debug_port u_ila_0/clk [get_nets [list system_clk]]
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe0]
set_property port_width 2 [get_debug_ports u_ila_0/probe0]
connect_debug_port u_ila_0/probe0 [get_nets [list {snapshot_cache_inst/state[0]} {snapshot_cache_inst/state[1]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe1]
set_property port_width 6 [get_debug_ports u_ila_0/probe1]
connect_debug_port u_ila_0/probe1 [get_nets [list {debug_sd_cmd12_bits_left[0]} {debug_sd_cmd12_bits_left[1]} {debug_sd_cmd12_bits_left[2]} {debug_sd_cmd12_bits_left[3]} {debug_sd_cmd12_bits_left[4]} {debug_sd_cmd12_bits_left[5]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe2]
set_property port_width 5 [get_debug_ports u_ila_0/probe2]
connect_debug_port u_ila_0/probe2 [get_nets [list {gamestate_layer[0]} {gamestate_layer[1]} {gamestate_layer[2]} {gamestate_layer[3]} {gamestate_layer[4]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe3]
set_property port_width 5 [get_debug_ports u_ila_0/probe3]
connect_debug_port u_ila_0/probe3 [get_nets [list {layer_counter[0]} {layer_counter[1]} {layer_counter[2]} {layer_counter[3]} {layer_counter[4]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe4]
set_property port_width 8 [get_debug_ports u_ila_0/probe4]
connect_debug_port u_ila_0/probe4 [get_nets [list {debug_cached_flags[0]} {debug_cached_flags[1]} {debug_cached_flags[2]} {debug_cached_flags[3]} {debug_cached_flags[4]} {debug_cached_flags[5]} {debug_cached_flags[6]} {debug_cached_flags[7]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe5]
set_property port_width 16 [get_debug_ports u_ila_0/probe5]
connect_debug_port u_ila_0/probe5 [get_nets [list {debug_mmu_width[0]} {debug_mmu_width[1]} {debug_mmu_width[2]} {debug_mmu_width[3]} {debug_mmu_width[4]} {debug_mmu_width[5]} {debug_mmu_width[6]} {debug_mmu_width[7]} {debug_mmu_width[8]} {debug_mmu_width[9]} {debug_mmu_width[10]} {debug_mmu_width[11]} {debug_mmu_width[12]} {debug_mmu_width[13]} {debug_mmu_width[14]} {debug_mmu_width[15]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe6]
set_property port_width 16 [get_debug_ports u_ila_0/probe6]
connect_debug_port u_ila_0/probe6 [get_nets [list {debug_mmu_file_name[0]} {debug_mmu_file_name[1]} {debug_mmu_file_name[2]} {debug_mmu_file_name[3]} {debug_mmu_file_name[4]} {debug_mmu_file_name[5]} {debug_mmu_file_name[6]} {debug_mmu_file_name[7]} {debug_mmu_file_name[8]} {debug_mmu_file_name[9]} {debug_mmu_file_name[10]} {debug_mmu_file_name[11]} {debug_mmu_file_name[12]} {debug_mmu_file_name[13]} {debug_mmu_file_name[14]} {debug_mmu_file_name[15]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe7]
set_property port_width 32 [get_debug_ports u_ila_0/probe7]
connect_debug_port u_ila_0/probe7 [get_nets [list {debug_mmu_file_size[0]} {debug_mmu_file_size[1]} {debug_mmu_file_size[2]} {debug_mmu_file_size[3]} {debug_mmu_file_size[4]} {debug_mmu_file_size[5]} {debug_mmu_file_size[6]} {debug_mmu_file_size[7]} {debug_mmu_file_size[8]} {debug_mmu_file_size[9]} {debug_mmu_file_size[10]} {debug_mmu_file_size[11]} {debug_mmu_file_size[12]} {debug_mmu_file_size[13]} {debug_mmu_file_size[14]} {debug_mmu_file_size[15]} {debug_mmu_file_size[16]} {debug_mmu_file_size[17]} {debug_mmu_file_size[18]} {debug_mmu_file_size[19]} {debug_mmu_file_size[20]} {debug_mmu_file_size[21]} {debug_mmu_file_size[22]} {debug_mmu_file_size[23]} {debug_mmu_file_size[24]} {debug_mmu_file_size[25]} {debug_mmu_file_size[26]} {debug_mmu_file_size[27]} {debug_mmu_file_size[28]} {debug_mmu_file_size[29]} {debug_mmu_file_size[30]} {debug_mmu_file_size[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe8]
set_property port_width 32 [get_debug_ports u_ila_0/probe8]
connect_debug_port u_ila_0/probe8 [get_nets [list {debug_mmu_sd_byte_counter[0]} {debug_mmu_sd_byte_counter[1]} {debug_mmu_sd_byte_counter[2]} {debug_mmu_sd_byte_counter[3]} {debug_mmu_sd_byte_counter[4]} {debug_mmu_sd_byte_counter[5]} {debug_mmu_sd_byte_counter[6]} {debug_mmu_sd_byte_counter[7]} {debug_mmu_sd_byte_counter[8]} {debug_mmu_sd_byte_counter[9]} {debug_mmu_sd_byte_counter[10]} {debug_mmu_sd_byte_counter[11]} {debug_mmu_sd_byte_counter[12]} {debug_mmu_sd_byte_counter[13]} {debug_mmu_sd_byte_counter[14]} {debug_mmu_sd_byte_counter[15]} {debug_mmu_sd_byte_counter[16]} {debug_mmu_sd_byte_counter[17]} {debug_mmu_sd_byte_counter[18]} {debug_mmu_sd_byte_counter[19]} {debug_mmu_sd_byte_counter[20]} {debug_mmu_sd_byte_counter[21]} {debug_mmu_sd_byte_counter[22]} {debug_mmu_sd_byte_counter[23]} {debug_mmu_sd_byte_counter[24]} {debug_mmu_sd_byte_counter[25]} {debug_mmu_sd_byte_counter[26]} {debug_mmu_sd_byte_counter[27]} {debug_mmu_sd_byte_counter[28]} {debug_mmu_sd_byte_counter[29]} {debug_mmu_sd_byte_counter[30]} {debug_mmu_sd_byte_counter[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe9]
set_property port_width 3 [get_debug_ports u_ila_0/probe9]
connect_debug_port u_ila_0/probe9 [get_nets [list {debug_sd_cmd12_state[0]} {debug_sd_cmd12_state[1]} {debug_sd_cmd12_state[2]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe10]
set_property port_width 16 [get_debug_ports u_ila_0/probe10]
connect_debug_port u_ila_0/probe10 [get_nets [list {debug_cached_width[0]} {debug_cached_width[1]} {debug_cached_width[2]} {debug_cached_width[3]} {debug_cached_width[4]} {debug_cached_width[5]} {debug_cached_width[6]} {debug_cached_width[7]} {debug_cached_width[8]} {debug_cached_width[9]} {debug_cached_width[10]} {debug_cached_width[11]} {debug_cached_width[12]} {debug_cached_width[13]} {debug_cached_width[14]} {debug_cached_width[15]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe11]
set_property port_width 8 [get_debug_ports u_ila_0/probe11]
connect_debug_port u_ila_0/probe11 [get_nets [list {mmu_y_select[0]} {mmu_y_select[1]} {mmu_y_select[2]} {mmu_y_select[3]} {mmu_y_select[4]} {mmu_y_select[5]} {mmu_y_select[6]} {mmu_y_select[7]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe12]
set_property port_width 5 [get_debug_ports u_ila_0/probe12]
connect_debug_port u_ila_0/probe12 [get_nets [list {mmu_layer[0]} {mmu_layer[1]} {mmu_layer[2]} {mmu_layer[3]} {mmu_layer[4]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe13]
set_property port_width 4 [get_debug_ports u_ila_0/probe13]
connect_debug_port u_ila_0/probe13 [get_nets [list {renderer_inst/scheduler/state[0]} {renderer_inst/scheduler/state[1]} {renderer_inst/scheduler/state[2]} {renderer_inst/scheduler/state[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe14]
set_property port_width 16 [get_debug_ports u_ila_0/probe14]
connect_debug_port u_ila_0/probe14 [get_nets [list {asset_fetcher_inst/width_reg[0]} {asset_fetcher_inst/width_reg[1]} {asset_fetcher_inst/width_reg[2]} {asset_fetcher_inst/width_reg[3]} {asset_fetcher_inst/width_reg[4]} {asset_fetcher_inst/width_reg[5]} {asset_fetcher_inst/width_reg[6]} {asset_fetcher_inst/width_reg[7]} {asset_fetcher_inst/width_reg[8]} {asset_fetcher_inst/width_reg[9]} {asset_fetcher_inst/width_reg[10]} {asset_fetcher_inst/width_reg[11]} {asset_fetcher_inst/width_reg[12]} {asset_fetcher_inst/width_reg[13]} {asset_fetcher_inst/width_reg[14]} {asset_fetcher_inst/width_reg[15]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe15]
set_property port_width 3 [get_debug_ports u_ila_0/probe15]
connect_debug_port u_ila_0/probe15 [get_nets [list {asset_fetcher_inst/state[0]} {asset_fetcher_inst/state[1]} {asset_fetcher_inst/state[2]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe16]
set_property port_width 1 [get_debug_ports u_ila_0/probe16]
connect_debug_port u_ila_0/probe16 [get_nets [list real_memory.state/fs/addr_fetch_failed]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe17]
set_property port_width 1 [get_debug_ports u_ila_0/probe17]
connect_debug_port u_ila_0/probe17 [get_nets [list debug_file_addr_fetched]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe18]
set_property port_width 1 [get_debug_ports u_ila_0/probe18]
connect_debug_port u_ila_0/probe18 [get_nets [list debug_fs_read]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe19]
set_property port_width 1 [get_debug_ports u_ila_0/probe19]
connect_debug_port u_ila_0/probe19 [get_nets [list debug_mcu_ready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe20]
set_property port_width 1 [get_debug_ports u_ila_0/probe20]
connect_debug_port u_ila_0/probe20 [get_nets [list debug_mmu_reading_file]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe21]
set_property port_width 1 [get_debug_ports u_ila_0/probe21]
connect_debug_port u_ila_0/probe21 [get_nets [list debug_mmu_selected_mem]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe22]
set_property port_width 1 [get_debug_ports u_ila_0/probe22]
connect_debug_port u_ila_0/probe22 [get_nets [list debug_mmu_state_written]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe23]
set_property port_width 1 [get_debug_ports u_ila_0/probe23]
connect_debug_port u_ila_0/probe23 [get_nets [list debug_sd_awaiting_resp]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe24]
set_property port_width 1 [get_debug_ports u_ila_0/probe24]
connect_debug_port u_ila_0/probe24 [get_nets [list debug_sd_byte_ready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe25]
set_property port_width 1 [get_debug_ports u_ila_0/probe25]
connect_debug_port u_ila_0/probe25 [get_nets [list debug_sd_cmd_ready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe26]
set_property port_width 1 [get_debug_ports u_ila_0/probe26]
connect_debug_port u_ila_0/probe26 [get_nets [list debug_sd_error]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe27]
set_property port_width 1 [get_debug_ports u_ila_0/probe27]
connect_debug_port u_ila_0/probe27 [get_nets [list debug_sd_reading]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe28]
set_property port_width 1 [get_debug_ports u_ila_0/probe28]
connect_debug_port u_ila_0/probe28 [get_nets [list fetcher_idle]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe29]
set_property port_width 1 [get_debug_ports u_ila_0/probe29]
connect_debug_port u_ila_0/probe29 [get_nets [list frame_done]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe30]
set_property port_width 1 [get_debug_ports u_ila_0/probe30]
connect_debug_port u_ila_0/probe30 [get_nets [list frame_setup]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe31]
set_property port_width 1 [get_debug_ports u_ila_0/probe31]
connect_debug_port u_ila_0/probe31 [get_nets [list real_memory.state/fs_reading]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe32]
set_property port_width 1 [get_debug_ports u_ila_0/probe32]
connect_debug_port u_ila_0/probe32 [get_nets [list renderer_inst/scheduler/layer_disabled]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe33]
set_property port_width 1 [get_debug_ports u_ila_0/probe33]
connect_debug_port u_ila_0/probe33 [get_nets [list renderer_inst/scheduler/layer_row_intersect]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe34]
set_property port_width 1 [get_debug_ports u_ila_0/probe34]
connect_debug_port u_ila_0/probe34 [get_nets [list renderer_inst/scheduler/layer_skip]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe35]
set_property port_width 1 [get_debug_ports u_ila_0/probe35]
connect_debug_port u_ila_0/probe35 [get_nets [list mmu_enabled]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe36]
set_property port_width 1 [get_debug_ports u_ila_0/probe36]
connect_debug_port u_ila_0/probe36 [get_nets [list mmu_prepare_ready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe37]
set_property port_width 1 [get_debug_ports u_ila_0/probe37]
connect_debug_port u_ila_0/probe37 [get_nets [list mmu_prepare_valid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe38]
set_property port_width 1 [get_debug_ports u_ila_0/probe38]
connect_debug_port u_ila_0/probe38 [get_nets [list snapshot_ready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe39]
set_property port_width 1 [get_debug_ports u_ila_0/probe39]
connect_debug_port u_ila_0/probe39 [get_nets [list snapshot_start]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe40]
set_property port_width 1 [get_debug_ports u_ila_0/probe40]
connect_debug_port u_ila_0/probe40 [get_nets [list stall]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe41]
set_property port_width 1 [get_debug_ports u_ila_0/probe41]
connect_debug_port u_ila_0/probe41 [get_nets [list system_reset]]
set_property C_CLK_INPUT_FREQ_HZ 300000000 [get_debug_cores dbg_hub]
set_property C_ENABLE_CLK_DIVIDER false [get_debug_cores dbg_hub]
set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]
connect_debug_port dbg_hub/clk [get_nets system_clk]
