#include "../cpu-ns32k/kernel-ns32k.def"
#
#	Called with interrupts off and in supervisor mode
#

		.globl __end
		.globl __bss_start

		.text
/*
head:
		.ascii "BOOTNS32"

		addr	head(pc),r3

		# Relocate ourselves down correctly
		movd	0x10,r0
		addr	relocate(pc),r1
		movd	0x1f000,r2
		movsd
		jump	0x1f000
#
#	This little block is run high up
#
relocate:
		movd	0x4000,r0
		movd	r3,r1
		movd	0x8000,r2
		movsd
		jump	start
*/
#
#	And this is where we enter the code in proper location
#
start:
		setcfg []			# Set CFG to all zeros to start with
		lprd sp,0x7fffc		# Set stack to top of memory

		# Stick our exception handler pointer in 0x80000
		# FIXME
		# Set up the ROM int and map tables.
		# addr inttab,r0
		# lprd intbase,r0
		# And modules - not that we use them
		# Second module entry uses start of RAM

		# Setup module table
		#
		# At present we just have 2 module Interrupts and Kernel (that's us)
		# Interrupt mod table is at 0x0000 (move to 0x0020)
		# Kernel mod table is at 0x0x0010
		#
		# At present both modules are set to all zeros.
		# Which is fine for now
		#

		movd 0,r0

		# Setup Interrupt module descriptor
		#
		movd r0, @0x000000 	# Static Base 
		movd r0, @0x000004 	# Link Base
		movd r0, @0x000008 	# Program Base
		movd r0, @0x00000C 	# Reserved


		# Setup Kernel module descriptor
		#
		movd r0, @0x000010 	# Static Base 
		movd r0, @0x000014 	# Link Base
		movd r0, @0x000018 	# Program Base
		movd r0, @0x00001C 	# Reserved


		movd 0x0x0010, r0	
		lprw mod, r0		# Set mod register to kernel module descriptor



		#movd 0x08,r0
		#lprw mod,r0


		#movd sbase,r0
		#lprd sb,r0
		

		addr __bss_start,r0
		addr __end,r1

wipebss:
		movqd 0,0(r0)
		addqd 4,r0
		cmpd r0,r1
		blt wipebss		

		/* FIXME: hard coded ugly */
		addr _udata_block+1016,r0
		lprd sp,r0
		/* udata global */
		addr _udata_block,r6

		bsr init_early
		bsr init_hardware
		bsr init_icu
		bsr _fuzix_main
		
		sprw psr,r0		# Interrupts back off
		andw 0xF7FF,r0
		lprw psr,r0
stop:		
		br stop

		.data

		.long 0xAABBCCDD
