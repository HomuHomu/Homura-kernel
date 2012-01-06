#
# Makefile for the VooDoo Screen Tuner Module
#

obj-$(CONFIG_VOODOO_SCREEN_TUNER) += ld9040_voodoo.o

ld9040_voodoo-obj += ld9040_voodoo.o

all:
	@echo "$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules"
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules

clean:
	rm -f *.o *.ko *.mod.c *.mod.o *~ .*.cmd .*.o.cmd Module.symvers modules.order

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install
