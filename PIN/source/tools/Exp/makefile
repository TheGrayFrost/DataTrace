##############################################################
#
#                   DO NOT EDIT THIS FILE!
#
##############################################################

# If the tool is built out of the kit, PIN_ROOT must be specified in the make invo#cation and point to the kit root.

PIN_ROOT := $(shell cd ../PIN && pwd)
CONFIG_ROOT := $(PIN_ROOT)/source/tools/Config
include $(CONFIG_ROOT)/makefile.config
include makefile.rules
include $(CONFIG_ROOT)/makefile.default.rules

CC := gcc
CXX := g++
CFLAGS := -g -pthread
CXXFLAGS := -g -pthread

.PRECIOUS : %.out %.dump
memtracker.cpp: memtracker.h
	@touch memtracker.cpp
%.out: %.c
	$(CC) $(CFLAGS) $< -o $@
%.out: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@
%.dump: %.out ./obj-intel64/memtracker.so
	../../../pin -t obj-intel64/memtracker.so -- ./$< > $@
	# ../../../pin -t obj-intel64/memtracker.so -- ./$<
	# cat Thread*.dump > $@
	# rm Thread*.dump
cleaner:
	-rm *.dump
	make clean

##############################################################
#
#                   DO NOT EDIT THIS FILE!
#
##############################################################