# Output name

PROJECT = Executables/sjsud_exec

# All Target
all: $(PROJECT).elf secondary-outputs


# Tool invocations
$(PROJECT).elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM GCC C++ Linker'
	arm-none-eabi-g++ -T"../loader.ld" -nostartfiles -Wl,-Map,$(PROJECT).map -mcpu=cortex-m3 -mthumb -o "$(PROJECT).elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-@echo ' Nothing to clean at the moment [DOES NOT EXIST]'