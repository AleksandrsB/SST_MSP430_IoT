################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Communication/%.obj: ../Communication/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1240/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmsp -Ooff --use_hw_mpy=none --include_path="C:/ti/ccs1240/ccs/ccs_base/msp430/include" --include_path="C:/Users/burak/SST/SST_LD_User" --include_path="C:/ti/ccs1240/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power=all --define=__MSP430F2274__ -g --c11 --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU19 --preproc_with_compile --preproc_dependency="Communication/$(basename $(<F)).d_raw" --obj_directory="Communication" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


