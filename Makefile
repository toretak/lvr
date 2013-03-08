#******************************************************************************
#
# Makefile - Rules for building the Sample Ethernet I/O Control Application using lwIP 1.3.2.
#
# Copyright (c) 2008-2012 Texas Instruments Incorporated.  All rights reserved.
# Software License Agreement
# 
# Texas Instruments (TI) is supplying this software for use solely and
# exclusively on TI's microcontroller products. The software is owned by
# TI and/or its suppliers, and is protected under applicable copyright
# laws. You may not combine this software with "viral" open-source
# software in order to form a larger program.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
# NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
# NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
# CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
# DAMAGES, FOR ANY REASON WHATSOEVER.
# 
# This is part of revision 9453 of the EK-LM3S8962 Firmware Package.
#
#******************************************************************************

#
# Defines the part type that this project uses.
#
PART=LM3S8962

#
# Set the processor variant.
#
VARIANT=cm3

#
# The base directory for StellarisWare.
#
ROOT=/home/toretak/stellaris/stellarisware

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
#VPATH=../drivers
#VPATH+=../../../third_party/lwip-1.3.2/apps/httpserver_raw
#VPATH+=../../../utils
VPATH=${ROOT}/boards/ek-lm3s8962/drivers
VPATH+=${ROOT}/third_party/lwip-1.3.2/apps/httpserver_raw
VPATH+=${ROOT}/utils


#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=${ROOT}/boards/ek-lm3s8962
IPATH+=${ROOT}
IPATH+=${ROOT}/net
IPATH+=${ROOT}/utils
IPATH+=${ROOT}/myutils
IPATH+=${ROOT}/third_party/lwip-1.3.2/apps
IPATH+=${ROOT}/third_party/lwip-1.3.2/ports/stellaris/include
IPATH+=${ROOT}/third_party/lwip-1.3.2/src/include
IPATH+=${ROOT}/third_party/lwip-1.3.2/src/include/ipv4

#IPATH+=..
#IPATH+=../../..
#IPATH+=../../../third_party/lwip-1.3.2/apps
#IPATH+=../../../third_party/lwip-1.3.2/ports/stellaris/include
#IPATH+=../../../third_party/lwip-1.3.2/src/include
#IPATH+=../../../third_party/lwip-1.3.2/src/include/ipv4

#
# The default rule, which causes the Sample Ethernet I/O Control Application using lwIP 1.3.2 to be built.
#
all: ${COMPILER}
all: ${COMPILER}/enet_io.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}
	@mkdir -p ${COMPILER}/utils
	@mkdir -p ${COMPILER}/myutils
	@mkdir -p ${COMPILER}/net
	@mkdir -p ${COMPILER}/fpga

#
# Rules for building the Sample Ethernet I/O Control Application using lwIP 1.3.2.
#
${COMPILER}/enet_io.axf: ${COMPILER}/fpga/FPGA_IO.o
${COMPILER}/enet_io.axf: ${COMPILER}/myutils/uartstdio.o
${COMPILER}/enet_io.axf: ${COMPILER}/myutils/crc.o
${COMPILER}/enet_io.axf: ${COMPILER}/utils/softeeprom.o
${COMPILER}/enet_io.axf: ${COMPILER}/net/cgifuncs.o
${COMPILER}/enet_io.axf: ${COMPILER}/net/ethernetwrapper.o
${COMPILER}/enet_io.axf: ${COMPILER}/net/http_conf.o
${COMPILER}/enet_io.axf: ${COMPILER}/net/httpd.o
${COMPILER}/enet_io.axf: ${COMPILER}/net/tcp_conn.o
${COMPILER}/enet_io.axf: ${COMPILER}/device.o
${COMPILER}/enet_io.axf: ${COMPILER}/lmi_fs.o
${COMPILER}/enet_io.axf: ${COMPILER}/locator.o
${COMPILER}/enet_io.axf: ${COMPILER}/lwiplib.o
${COMPILER}/enet_io.axf: ${COMPILER}/rit128x96x4.o
${COMPILER}/enet_io.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/enet_io.axf: ${COMPILER}/ustdlib.o
${COMPILER}/enet_io.axf: ${ROOT}/driverlib/${COMPILER}-cm3/libdriver-cm3.a
${COMPILER}/enet_io.axf: enet_io.ld
SCATTERgcc_enet_io=enet_io.ld
ENTRY_enet_io=ResetISR

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
