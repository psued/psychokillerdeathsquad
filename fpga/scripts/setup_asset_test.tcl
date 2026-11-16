# Source this with a Vivado project open and simulation closed.
if {[current_project -quiet] eq ""} {
    error "Open your Vivado project before running setup_asset_test.tcl"
}

set asset_test_script_dir [file dirname [file normalize [info script]]]
set asset_test_fpga_dir [file normalize [file join $asset_test_script_dir ..]]
set asset_test_rtl_dir [file join $asset_test_fpga_dir rtl]
set asset_test_data_dir [file join $asset_test_fpga_dir build asset_test_data]

puts [exec python -E [file join $asset_test_script_dir prepare_asset_test.py]]

# Remove project entries for the earlier source locations.
foreach source_file [get_files -of_objects [get_filesets sources_1]] {
    if {[file extension $source_file] in {.sv .v} && ![file exists $source_file]} {
        remove_files $source_file
    }
}

set asset_test_sources [list \
    [file join $asset_test_rtl_dir renderer_pkg.sv] \
    [file join $asset_test_rtl_dir top.sv]]
foreach folder {renderer mock_modules line_buffer vga} {
    set asset_test_sources [concat $asset_test_sources \
        [glob -directory [file join $asset_test_rtl_dir $folder] *.sv]]
}
foreach source_file $asset_test_sources {
    if {[llength [get_files -quiet $source_file]] == 0} {
        add_files -fileset sources_1 $source_file
    }
}
foreach name {chinabg fredrikidle_comp roundovertext} {
    set memory_file [file join $asset_test_data_dir "$name.mem"]
    if {[llength [get_files -quiet $memory_file]] == 0} {
        add_files -fileset sources_1 $memory_file
    }
}
foreach source_file [list \
    [file join $asset_test_fpga_dir tb tb_vga_frame.sv] \
    [file join $asset_test_data_dir expected_frame.mem]] {
    if {[llength [get_files -quiet $source_file]] == 0} {
        add_files -fileset sim_1 $source_file
    }
}
set_property generic {USE_MOCK_MMU=1} [get_filesets sources_1]
set_property top tb_vga_frame [get_filesets sim_1]
puts "Asset test ready. Run: launch_simulation -mode behavioral; run all"
