# ============================================================================
# create_project.tcl - Generate the Vivado project for the FPGA coprocessor
# Usage: vivado -mode batch -source vivado/create_project.tcl
# Target: Artix-7 XC7A35T-FTG256 (国产备选 EG4S20 需另行评估)
# ============================================================================

set proj_name "bt_fpga"
set part "xc7a35tftg256-1"
set proj_dir "./vivado_out"
set rtl_src [glob ./src/*.v]
set tb_src  [glob ./tb/*.v]

create_project -name $proj_name -dir $proj_dir -part $part -force
set_property target_language Verilog [current_project]
set_property default_lib work [current_project]

# RTL sources
foreach f $rtl_src {
    add_files -norecurse [file normalize $f]
}
set_property top top [current_fileset]

# Constraints
add_files -fileset constrs_1 -norecurse [file normalize ./constraints/top.xdc]

# Simulation sources
if {[llength $tb_src] > 0} {
    set sim_fileset [get_filesets sim_1]
    foreach f $tb_src {
        add_files -norecurse -fileset $sim_fileset [file normalize $f]
    }
}

# Synthesis / implementation strategy (default balanced)
set_property strategy "Performance_Explore" [get_runs synth_1]
set_property strategy "Performance_Explore" [get_runs impl_1]

puts "== Project '$proj_name' generated. Run: vivado -mode batch -source <run_flow> =="
close_project
