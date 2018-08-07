TARGET = i2c_test

ALT_DEVICE_FAMILY ?= soc_cv_av

CROSS_COMPILE = arm-linux-gnueabihf-

SOCEDS_DEST_ROOT = /home/anderson/intelFPGA/18.0/embedded

HWLIBS_ROOT = $(SOCEDS_DEST_ROOT)/ip/altera/hps/altera_hps/hwlib

CFLAGS = -Wall -std=gnu11 -ggdb -g -Wall -Werror -I$(HWLIBS_ROOT)/include -I$(HWLIBS_ROOT)/include/$(ALT_DEVICE_FAMILY) -D$(ALT_DEVICE_FAMILY)
LDFLAGS = -g -Wall -Werror

CROSS_COMPILE = arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
ARCH = arm

.PHONY: build
build: $(TARGET)

$(TARGET): test.o
	$(LD) $(LDFLAGS) $^ -o $@
	rm test.o

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~
