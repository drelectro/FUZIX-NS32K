#include "../cpu-ns32k/kernel-ns32k.def"
#
#	Called with interrupts off and in supervisor mode
#

		.globl __end
		.globl __bss_start
		.globl __data_load
		.globl __data_start
		.globl __data_end
		.globl start

		.text

#
#	
#
start:
		setcfg [i,f]			# Set CFG to reflect system configuration
								# WARNING: lower case only !

		#
		# Configure sp, intbase and sbase to suit our desired memory map
		#
		lprd sp, 0x07fffc		# Set stack to top of memory
		lprd intbase, 0x001000	# Set intbase
		lprd sb, 0x0			# Set sbase (compiler expects this to be zero)


		# Setup module table
		#
		# At present we just have 2 modules, Interrupts and Kernel (that's us)
		# Interrupt mod table is at 0x0020 
		# Kernel mod table is at 0x0x0010
		#
		# At present both modules set static base and link base to zero.
		# Program base is set to our load address.
		#

		movd 0,r0
		movd start, r1 

		# Setup Interrupt module descriptor
		#
		movd r0, @0x000020 	# Static Base 
		movd r0, @0x000024 	# Link Base
		movd r1, @0x000028 	# Program Base
		movd r0, @0x00002C 	# Reserved


		# Setup Kernel module descriptor
		#
		movd r0, @0x000010 	# Static Base 
		movd r0, @0x000014 	# Link Base
		movd r1, @0x000018 	# Program Base
		movd r0, @0x00001C 	# Reserved


		movd 0x0x0010, r0	
		lprw mod, r0		# Set mod register to kernel module descriptor


		# Clear BSS
		#
		# Initialise .data
		#
		# The kernel image is stored in "ROM", but the .data section runs in RAM.
		# The linker provides:
		#   __data_load  = ROM source (LMA)
		#   __data_start = RAM destination (VMA)
		#   __data_end   = end of .data in RAM
		#
		addr __data_load,r1
		addr __data_start,r2
		addr __data_end,r3
		cmpd r2,r3
		beq datacopied

		movd r3,r4
		subd r2,r4			# r4 = length in bytes

		movd r4,r0
		lshd -2,r0			# r0 = length in dwords
		cmpqd 0,r0
		beq datatail
		movsd				# copies r0 dwords from r1 -> r2

datatail:
		movd r4,r0
		andb 3,r0			# r0 = remaining bytes (0..3)
		cmpqb 0,r0
		beq datacopied

datatailloop:
		movb 0(r1),r5
		movb r5,0(r2)
		addqd 1,r1
		addqd 1,r2
		addqb -1,r0
		bne datatailloop

datacopied:

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
