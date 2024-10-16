# Makefile for the ADC driver

# Kernel build path
KDIR := /lib/modules/$(shell uname -r)/build

# Module name
MODULE := ADC_driver

# Source files
obj-m := $(MODULE).o

# Default target
all:
	make -C $(KDIR) M=$(PWD) modules

# Clean target
clean:
	make -C $(KDIR) M=$(PWD) clean


