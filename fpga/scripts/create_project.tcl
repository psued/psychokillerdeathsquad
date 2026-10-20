
# ============================================================
# TDT4295 FPGA Project
# Vivado 2023.1 - Arty A7-35T
# ============================================================

# Locate the script and FPGA directory.
set script_dir [file dirname [file normalize [info script]]]
set fpga_dir [file normalize [file join $script_dir ..]]

set project_name "psychokillerdeathsquad"
set part_name "xc7a35ticsg324-1L"
set board_name "digilentinc.com:arty-a7-35:part0:1.1"
set top_module "top"

set build_dir [file join $fpga_dir "build"]

create_project $project_name $build_dir -part $part_name

set_property board_part $board_name [current_project]

# ============================================================
# Design sources
# ============================================================

set rtl_dir [file join $fpga_dir rtl]

set rtl_files [glob -nocomplain -directory $rtl_dir *.sv *.v]

add_files -fileset sources_1 $rtl_files

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

set constraints_file [file join $fpga_dir "constraints" "arty_vga.xdc"]

if {![file exists $constraints_file]} {
    error "Constraints file not found: $constraints_file"
}

add_files -fileset constrs_1 $constraints_file

# ============================================================
# Clock Wizard IP
# ============================================================

set clock_ip [file join $fpga_dir "ip" "clk_wiz_vga.xci"]

import_ip -srcset sources_1 $clock_ip

generate_target all [get_ips clk_wiz_vga]

# ============================================================
# Project configuration
# ============================================================

set_property top $top_module [get_filesets sources_1]

update_compile_order -fileset sources_1
update_compile_order -fileset sim_1

puts "Project created successfully."
puts "Project location: $build_dir"
puts "FPGA part: $part_name"
puts "Top module: $top_module"
