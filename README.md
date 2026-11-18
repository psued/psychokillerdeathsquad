# Psycho Killer Death Squad - FPGA
Branch for Vivado project and docs/utilities for the FPGA.

## Vivado project setup
1. Inside of `fpga/scripts`, run the following command:
```
vivado -mode batch -source create_project.tcl -tclargs custom
```
2. In Vivado, open the .xpr file found inside `fpga/build/custom`
