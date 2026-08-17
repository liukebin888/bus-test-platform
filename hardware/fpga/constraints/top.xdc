# ============================================================================
# top.xdc - Top-level timing & pin constraints (Artix-7 XC7A35T-FTG256)
# Adjust pin locations to the actual board schematic (Phase A W01-04).
# ============================================================================

# ---- Clock: TCXO 10 MHz on a clock-capable pin ----
set_property PACKAGE_PIN R4 [get_ports clk]
set_property IOSTANDARD LVCMOS33 [get_ports clk]
create_clock -period 100.000 -name clk_tcxo [get_ports clk]

# ---- Reset (active low) ----
set_property PACKAGE_PIN T5 [get_ports rst_n]
set_property IOSTANDARD LVCMOS33 [get_ports rst_n]

# ---- PPS input ----
set_property PACKAGE_PIN U6 [get_ports pps_in]
set_property IOSTANDARD LVCMOS33 [get_ports pps_in]
set_input_delay -clock clk_tcxo -max 5.0 [get_ports pps_in]
set_input_delay -clock clk_tcxo -min 1.0 [get_ports pps_in]

# ---- MCU register interface (LVCMOS33, 3.3V MCU domain) ----
set_property PACKAGE_PIN V7  [get_ports {mcu_addr[0]}]
set_property PACKAGE_PIN V8  [get_ports {mcu_addr[1]}]
set_property PACKAGE_PIN W7  [get_ports {mcu_addr[2]}]
set_property PACKAGE_PIN W8  [get_ports {mcu_addr[3]}]
# ... remaining pins per board schematic (placeholder)
set_property IOSTANDARD LVCMOS33 [get_ports {mcu_addr[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {mcu_wdata[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports mcu_we]
set_property IOSTANDARD LVCMOS33 [get_ports {mcu_rdata[*]}]

# ---- Clock groups (avoid false cross-domain warnings between domains) ----
set_clock_groups -asynchronous \
    -group [get_clocks clk_tcxo]

# ---- Timing exceptions ----
# Register interface is asynchronous relative to TCXO; constrain by mcu_we.
set_false_path -from [get_ports mcu_we]
