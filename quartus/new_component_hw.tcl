# TCL File Generated by Component Editor 23.1
# Wed Dec 04 15:45:39 MST 2024
# DO NOT MODIFY


# 
# RGB_LED_Control "RGB_LED_Control" v1.0
#  2024.12.04.15:45:39
# 
# 

# 
# request TCL package from ACDS 16.1
# 
package require -exact qsys 16.1


# 
# module RGB_LED_Control
# 
set_module_property DESCRIPTION ""
set_module_property NAME RGB_LED_Control
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR ""
set_module_property DISPLAY_NAME RGB_LED_Control
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property REPORT_HIERARCHY false


# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL RGB_LED_Control
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE true
add_fileset_file pwm_controller.vhd VHDL PATH ../hdl/pwm/pwm_controller.vhd
add_fileset_file RGB_LED_Control.vhd VHDL PATH ../hdl/RGB-LED-Control/RGB_LED_Control.vhd TOP_LEVEL_FILE


# 
# parameters
# 


# 
# display items
# 


# 
# connection point clk
# 
add_interface clk clock end
set_interface_property clk clockRate 0
set_interface_property clk ENABLED true
set_interface_property clk EXPORT_OF ""
set_interface_property clk PORT_NAME_MAP ""
set_interface_property clk CMSIS_SVD_VARIABLES ""
set_interface_property clk SVD_ADDRESS_GROUP ""

add_interface_port clk clk clk Input 1


# 
# connection point rst
# 
add_interface rst reset end
set_interface_property rst associatedClock clk
set_interface_property rst synchronousEdges DEASSERT
set_interface_property rst ENABLED true
set_interface_property rst EXPORT_OF ""
set_interface_property rst PORT_NAME_MAP ""
set_interface_property rst CMSIS_SVD_VARIABLES ""
set_interface_property rst SVD_ADDRESS_GROUP ""

add_interface_port rst rst reset Input 1


# 
# connection point RGB_LED_Control
# 
add_interface RGB_LED_Control avalon end
set_interface_property RGB_LED_Control addressUnits WORDS
set_interface_property RGB_LED_Control associatedClock clk
set_interface_property RGB_LED_Control associatedReset rst
set_interface_property RGB_LED_Control bitsPerSymbol 8
set_interface_property RGB_LED_Control burstOnBurstBoundariesOnly false
set_interface_property RGB_LED_Control burstcountUnits WORDS
set_interface_property RGB_LED_Control explicitAddressSpan 0
set_interface_property RGB_LED_Control holdTime 0
set_interface_property RGB_LED_Control linewrapBursts false
set_interface_property RGB_LED_Control maximumPendingReadTransactions 0
set_interface_property RGB_LED_Control maximumPendingWriteTransactions 0
set_interface_property RGB_LED_Control readLatency 0
set_interface_property RGB_LED_Control readWaitTime 1
set_interface_property RGB_LED_Control setupTime 0
set_interface_property RGB_LED_Control timingUnits Cycles
set_interface_property RGB_LED_Control writeWaitTime 0
set_interface_property RGB_LED_Control ENABLED true
set_interface_property RGB_LED_Control EXPORT_OF ""
set_interface_property RGB_LED_Control PORT_NAME_MAP ""
set_interface_property RGB_LED_Control CMSIS_SVD_VARIABLES ""
set_interface_property RGB_LED_Control SVD_ADDRESS_GROUP ""

add_interface_port RGB_LED_Control avs_address address Input 2
add_interface_port RGB_LED_Control avs_read read Input 1
add_interface_port RGB_LED_Control avs_readdata readdata Output 32
add_interface_port RGB_LED_Control avs_write write Input 1
add_interface_port RGB_LED_Control avs_writedata writedata Input 32
set_interface_assignment RGB_LED_Control embeddedsw.configuration.isFlash 0
set_interface_assignment RGB_LED_Control embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment RGB_LED_Control embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment RGB_LED_Control embeddedsw.configuration.isPrintableDevice 0


# 
# connection point export
# 
add_interface export conduit end
set_interface_property export associatedClock clk
set_interface_property export associatedReset ""
set_interface_property export ENABLED true
set_interface_property export EXPORT_OF ""
set_interface_property export PORT_NAME_MAP ""
set_interface_property export CMSIS_SVD_VARIABLES ""
set_interface_property export SVD_ADDRESS_GROUP ""

add_interface_port export blue_out blue_out Output 1
add_interface_port export green_out green_out Output 1
add_interface_port export red_out red_out Output 1

