#
# Makefile for the Cypress touchkey driver
#
ifeq ($(CONFIG_BUILD_TARGET_CM7),y)
obj-$(CONFIG_KEYBOARD_CYPRESS_TOUCH) += cypress-touchkey.o issp_driver_routines.o issp_main.o issp_routines.o
else
obj-$(CONFIG_KEYBOARD_CYPRESS_TOUCH) += cypress-touchkey-creams.o issp_driver_routines.o issp_main.o issp_routines.o
endif
