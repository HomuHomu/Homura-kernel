#
# Makefile for the Cypress touchkey driver
#
ifeq ($(CONFIG_CM7_LED),y)
obj-$(CONFIG_KEYBOARD_CYPRESS_TOUCH) += cypress-touchkey-cm7sp.o issp_driver_routines.o issp_main.o issp_routines.o
else
obj-$(CONFIG_KEYBOARD_CYPRESS_TOUCH) += cypress-touchkey.o issp_driver_routines.o issp_main.o issp_routines.o
endif
