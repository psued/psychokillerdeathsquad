
# ============================================================
# TDT4295 FPGA Project
# Vivado 2023.1 - Arty A7-35T / custom XC7A100T-1FTG256C
# ============================================================

# Locate the script and FPGA directory.
set script_dir [file dirname [file normalize [info script]]]
set fpga_dir [file normalize [file join $script_dir ..]]

# GUI: set fpga_target custom before sourcing this script.
# Batch: vivado -mode batch -source create_project.tcl -tclargs custom
# An explicit fpga_target takes precedence over command-line arguments.
if {![info exists fpga_target]} {
    set fpga_target "arty"
    if {[info exists argv] && [llength $argv] > 0} {
        set fpga_target [lindex $argv 0]
    }
}

set project_name "psychokillerdeathsquad"
set top_module "top"
set board_name ""

switch -- $fpga_target {
    arty {
        set part_name "xc7a35ticsg324-1L"
        set board_name "digilentinc.com:arty-a7-35:part0:1.1"
        set constraints_name "arty_vga.xdc"
    }
    custom {
        set part_name "xc7a100tftg256-1"
        set constraints_name "custom_board.xdc"
    }
    default {
        error "Unknown FPGA target '$fpga_target'; use arty or custom"
    }
}

set constraints_file [file join $fpga_dir "constraints" $constraints_name]
if {![file exists $constraints_file]} {
    error "Constraints file not found: $constraints_file"
}

# Keep projects and generated IP separate for each target.
set build_dir [file join $fpga_dir "build" $fpga_target]

create_project $project_name $build_dir -part $part_name

if {$board_name ne ""} {
    set_property board_part $board_name [current_project]
}

# ============================================================
# Design sources
# ============================================================

set rtl_dir [file join $fpga_dir rtl]

add_files -fileset sources_1 $rtl_dir

# ============================================================
# Simulation sources
# ============================================================

set tb_dir [file join $fpga_dir "tb"]
set tb_files [glob -nocomplain -directory $tb_dir *.sv *.v]

if {[llength $tb_files] == 0} {
    error "No testbenches found in $tb_dir"
}

add_files -fileset sim_1 $tb_files

set_property top tb_vga_frame [get_filesets sim_1]

# ============================================================
# Constraints
# ============================================================

add_files -fileset constrs_1 $constraints_file

# ============================================================
# Clock Wizard IP
# ============================================================

# Generate clock IP for the selected FPGA, preserving the RTL interface.
create_ip -name clk_wiz -vendor xilinx.com -library ip -version 6.0 \
    -module_name clk_wiz_vga

set_property -dict [list \
    CONFIG.PRIMITIVE {MMCM} \
    CONFIG.PRIM_SOURCE {No_buffer} \
    CONFIG.PRIM_IN_FREQ {100.000} \
    CONFIG.NUM_OUT_CLKS {1} \
    CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {25.000} \
    CONFIG.PRIMARY_PORT {clk_in1} \
    CONFIG.CLK_OUT1_PORT {clk_out1} \
    CONFIG.USE_RESET {true} \
    CONFIG.RESET_TYPE {ACTIVE_HIGH} \
    CONFIG.RESET_PORT {reset} \
    CONFIG.USE_LOCKED {true} \
    CONFIG.LOCKED_PORT {locked} \
] [get_ips clk_wiz_vga]

generate_target all [get_ips clk_wiz_vga]

# ============================================================
# Project configuration
# ============================================================

set_property top $top_module [get_filesets sources_1]

update_compile_order -fileset sources_1
update_compile_order -fileset sim_1

puts "Project created successfully."
puts "FPGA target: $fpga_target"
puts "Project location: $build_dir"
puts "FPGA part: $part_name"
puts "Top module: $top_module"
