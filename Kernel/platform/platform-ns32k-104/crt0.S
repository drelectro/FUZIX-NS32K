#include "../cpu-ns32k/kernel-ns32k.def"
#
#	Called with interrupts off and in supervisor mode
#

		.globl __end
		.globl __bss_start

		.text

#
#	
#
start:
		setcfg [i]				# Set CFG to reflect system configuration
								# WARNING: lower case only !

		#
		# Configure sp, intbase and sbase to suit our desired memory map
		#
		lprd sp, 0x07fffc		# Set stack to top of memory
		lprd intbase, 0x001000	# Set intbase
		lprd sb, 0x002000		# Set sbase (not used)


		# Setup module table
		#
		# At present we just have 2 modules, Interrupts and Kernel (that's us)
		# Interrupt mod table is at 0x0020 
		# Kernel mod table is at 0x0x0010
		#
		# At present both modules are set to all zeros.
		# We should set the Interrupt mod table base to reflect the actual base 
		# address of the interrupt routines
		#

		movd 0,r0

		# Setup Interrupt module descriptor
		#
		movd r0, @0x000020 	# Static Base 
		movd r0, @0x000024 	# Link Base
		movd r0, @0x000028 	# Program Base
		movd r0, @0x00002C 	# Reserved


		# Setup Kernel module descriptor
		#
		movd r0, @0x000010 	# Static Base 
		movd r0, @0x000014 	# Link Base
		movd r0, @0x000018 	# Program Base
		movd r0, @0x00001C 	# Reserved


		movd 0x0x0010, r0	
		lprw mod, r0		# Set mod register to kernel module descriptor


		# Clear BSS
		#
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
